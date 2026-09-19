/* reuse-lint.c - REUSE / SPDX compliance checker (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#ifdef __LINUX__
#include <unistd.h>
#else
#include <io.h>
#endif

#include <reuse.h>
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "reuse_discover.h"
#include "reuse_lic.h"
#include "spdx_tag.h"
#include "git.h"

/**
 * @file reuse-lint.c
 * @brief REUSE / SPDX compliance checker.
 *
 * Walks the project, resolves the license and copyright of every
 * file, validates SPDX expressions against the SPDX License List,
 * and verifies the LICENSES/ directory.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - SPDX 2.3, Annex D (license expression grammar).
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - OS/2 Control Program Interface (naming, types, conventions).
 */

#define PATH_BUF 1024

/* ------------------------------------------------------------------ */
/* Global state                                                        */
/* ------------------------------------------------------------------ */

static ULONG       g_ulErrorCount = 0;
static ULONG       g_ulWarningCount = 0;

static PCSZ        g_pszDefaultLicense = NULL;
static PCSZ        g_pszDefaultCopyright = NULL;

static ULONG       g_ulTotalFiles = 0;
static ULONG       g_ulFilesWithLicense = 0;
static ULONG       g_ulFilesWithCopyright = 0;
static ULONG       g_ulReadErrors = 0;

static ULONG       g_ulTotalSnippets = 0;
static ULONG       g_ulSnippetsWithLicense = 0;

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Check whether a file name has an extension.
 *
 * @param[in] pszName  File name. Not NULL.
 *
 * @return TRUE_ if the name contains a '.' that is not at position 0.
 */
static BOOL HasExtension(PCSZ pszName) {
    PCSZ pszDot = strrchr(pszName, '.');
    if (!pszDot) return FALSE_;
    return (pszDot != pszName) ? TRUE_ : FALSE_;
}

/**
 * @brief Check whether a name is a valid SPDX license or exception.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE_ if the name is valid.
 */
static BOOL IsValidSpdxName(PCSZ pszName) {
    BOOL fValid = FALSE_;

    if (SpdxQueryLicenseValid(pszName, &fValid) == NO_ERROR && fValid)
        return TRUE_;
    fValid = FALSE_;
    if (SpdxQueryExceptionValid(pszName, &fValid) == NO_ERROR && fValid)
        return TRUE_;
    return FALSE_;
}

/**
 * @brief Print the offending token of an SPDX expression to stderr.
 *
 * @param[in] pszStart  Start of the token. May be NULL.
 */
static void PrintBadToken(PCSZ pszStart) {
    PCSZ pszPos = pszStart;
    if (!pszPos) { fprintf(stderr, "(null)"); return; }
    while (*pszPos && !isspace((unsigned char)*pszPos) &&
           *pszPos != '(' && *pszPos != ')')
        pszPos++;
    fwrite(pszStart, 1, (size_t)(pszPos - pszStart), stderr);
}

/* ------------------------------------------------------------------ */
/* Expression validation                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Validate one SPDX expression and collect its identifiers.
 *
 * @param[in] pszFullPath  File path used in diagnostics. Not NULL.
 * @param[in] pszLicense   Expression. Not NULL.
 * @param[in] hUsedIds     Set of used identifiers. May be NULLHANDLE.
 */
static void CheckLicenseExpression(PCSZ pszFullPath, PCSZ pszLicense,
                                   HSTRSET hUsedIds) {
    PCSZ pszBad = NULL;
    APIRET rcExpr = SpdxQueryExpression(pszLicense, &pszBad);

    if (rcExpr == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "ERROR: %s: invalid SPDX license expression: '%s'\n"
                "       Fix the expression according to the SPDX grammar:\n"
                "         https://spdx.github.io/spdx-spec/v2.3/"
                "SPDX-license-expressions/\n",
                pszFullPath, pszLicense);
        g_ulErrorCount++;
    } else if (rcExpr == SPDX_EXPR_UNKNOWN_TOKEN) {
        fprintf(stderr, "ERROR: %s: unknown SPDX identifier: '",
                pszFullPath);
        PrintBadToken(pszBad);
        fprintf(stderr,
                "'\n"
                "       Not present in SPDX License List. Fix one of:\n"
                "         - correct the identifier;\n"
                "         - if it is a custom license, prefix it with "
                "'LicenseRef-' and add the text to LICENSES/;\n"
                "         - or add a [[annotations]] entry in REUSE.toml.\n"
                "       See https://spdx.org/licenses/ for the full list.\n");
        g_ulErrorCount++;
    }

    if (hUsedIds != NULLHANDLE) {
        HSTRSET hIds = NULLHANDLE;
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetCreate(&hIds) == NO_ERROR) {
            SpdxExpressionCollectIds(pszLicense, hIds);
            if (StrSetEnumFirst(hIds, &hEnum) == NO_ERROR) {
                do {
                    CHAR achId[256];
                    BOOL fDepLic = FALSE_;
                    BOOL fDepExc = FALSE_;

                    if (StrSetEnumGet(hEnum, achId, sizeof(achId), NULL)
                            != NO_ERROR)
                        continue;
                    if (rcExpr == NO_ERROR) {
                        StrSetAdd(hUsedIds, achId);
                    }
                    SpdxQueryLicenseDeprecated(achId, &fDepLic);
                    SpdxQueryExceptionDeprecated(achId, &fDepExc);
                    if (fDepLic || fDepExc) {
                        fprintf(stderr,
                                "WARNING: %s: deprecated SPDX identifier "
                                "'%s'.\n"
                                "         The SPDX License List marks this "
                                "identifier deprecated.\n"
                                "         Replace it with the current "
                                "identifier (usually a '-only' or "
                                "'-or-later' variant).\n"
                                "         See https://spdx.org/licenses/ "
                                "for the recommended replacement.\n",
                                pszFullPath, achId);
                        g_ulWarningCount++;
                    }
                } while (StrSetEnumNext(hEnum) == NO_ERROR);
                StrSetEnumClose(hEnum);
            }
            StrSetDestroy(hIds);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Snippets                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Check every SPDX snippet of one file.
 *
 * @param[in] pszFullPath    File path used in diagnostics. Not NULL.
 * @param[in] hUsedLicenses  Set of used identifiers. Not NULLHANDLE.
 */
static void ProcessSnippets(PCSZ pszFullPath, HSTRSET hUsedLicenses) {
    SPDXSNIPPETLIST snippets;
    ULONG ulIdx;

    if (SpdxQueryFileSnippets(pszFullPath, &snippets) != NO_ERROR) {
        g_ulErrorCount++;
        return;
    }

    for (ulIdx = 0; ulIdx < snippets.ulCount; ulIdx++) {
        PSPDXSNIPPET pS = &snippets.pItems[ulIdx];

        g_ulTotalSnippets++;

        if (!pS->pszLicense || pS->pszLicense[0] == '\0') {
            fprintf(stderr,
                    "ERROR: %s:%lu-%lu: snippet has no "
                    "SPDX-License-Identifier.\n"
                    "       Fix one of:\n"
                    "         - add 'SPDX-License-Identifier: <id>' inside "
                    "the snippet block;\n"
                    "         - or remove SPDX-SnippetBegin/SPDX-SnippetEnd "
                    "if the code is not a snippet.\n",
                    pszFullPath,
                    (unsigned long)pS->ulLineStart,
                    (unsigned long)pS->ulLineEnd);
            g_ulErrorCount++;
            continue;
        }

        g_ulSnippetsWithLicense++;

        {
            CHAR achLabel[1200];
            snprintf(achLabel, sizeof(achLabel), "%s:%lu-%lu",
                     pszFullPath,
                     (unsigned long)pS->ulLineStart,
                     (unsigned long)pS->ulLineEnd);
            CheckLicenseExpression(achLabel, pS->pszLicense, hUsedLicenses);
        }

        if (!pS->pszCopyright || pS->pszCopyright[0] == '\0') {
            fprintf(stderr,
                    "WARNING: %s:%lu-%lu: snippet has no "
                    "SPDX-SnippetCopyrightText.\n"
                    "         REUSE recommends adding a copyright notice\n"
                    "         inside the snippet.\n",
                    pszFullPath,
                    (unsigned long)pS->ulLineStart,
                    (unsigned long)pS->ulLineEnd);
            g_ulWarningCount++;
        }
    }

    SpdxSnippetListFree(&snippets);
}

/* ------------------------------------------------------------------ */
/* Per-file processing                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Process one file: resolve, validate, count.
 *
 * @param[in] pszFullPath     File path. Not NULL.
 * @param[in] hTree           REUSE project handle. Not NULLHANDLE.
 * @param[in] hUsedLicenses   Set of used identifiers. Not NULLHANDLE.
 */
static void ProcessFile(PCSZ pszFullPath, HREUSETREE hTree,
                        HSTRSET hUsedLicenses) {
    REUSELICENSEINFO lic;
    FILE *fp;
    PCSZ pszWcc;

#ifdef __LINUX__
    pszWcc = "_wcc.sh";
#else
    pszWcc = "_wcc.cmd";
#endif

    g_ulTotalFiles++;

    fp = fopen(pszFullPath, "rb");
    if (!fp) {
        fprintf(stderr,
                "ERROR: cannot read file: %s\n"
                "       Fix file permissions or remove it from the "
                "project.\n",
                pszFullPath);
        g_ulErrorCount++;
        g_ulReadErrors++;
        return;
    }
    fclose(fp);

    if (ReuseResolveLicense(hTree, pszFullPath,
                            g_pszDefaultLicense, g_pszDefaultCopyright,
                            &lic) != NO_ERROR) {
        fprintf(stderr,
                "ERROR: %s has no licensing information at all.\n"
                "       REUSE requires each file to carry both license and\n"
                "       copyright information.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier' and "
                "'SPDX-FileCopyrightText' tags in the file header;\n"
                "         - or create a sidecar '<file>.license' next to it;\n"
                "         - or add a [[annotations]] entry in REUSE.toml;\n"
                "         - or run '%s annotate' to write them.\n"
                "       See https://reuse.software/spec/ for details.\n",
                pszFullPath, pszWcc);
        g_ulErrorCount++;
        return;
    }

    if (lic.fLicenseFromDefault) {
        fprintf(stderr,
                "WARNING: %s: license is taken from --default-license.\n"
                "         REUSE does not allow a global CLI fallback.\n"
                "         Run '%s annotate' to write the license\n"
                "         into the file, or add a sidecar/REUSE.toml "
                "entry.\n",
                pszFullPath, pszWcc);
        g_ulWarningCount++;
    }
    if (lic.fCopyrightFromDefault) {
        fprintf(stderr,
                "WARNING: %s: copyright is taken from "
                "--default-copyright.\n"
                "         REUSE does not allow a global CLI fallback.\n"
                "         Run '%s annotate' to write the copyright\n"
                "         into the file, or add a sidecar/REUSE.toml "
                "entry.\n",
                pszFullPath, pszWcc);
        g_ulWarningCount++;
    }

    if (lic.achLicense[0] != '\0') {
        g_ulFilesWithLicense++;
    } else {
        fprintf(stderr,
                "ERROR: %s has copyright information but no license "
                "information.\n"
                "       REUSE requires both.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier: <id>' in the "
                "file header;\n"
                "         - or create '<file>.license' with the same tag;\n"
                "         - or add a [[annotations]] entry in REUSE.toml;\n"
                "         - or run '%s annotate'.\n",
                pszFullPath, pszWcc);
        g_ulErrorCount++;
    }
    if (lic.achCopyright[0] != '\0') {
        g_ulFilesWithCopyright++;
    } else {
        fprintf(stderr,
                "ERROR: %s has license information but no copyright "
                "information.\n"
                "       REUSE requires both.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-FileCopyrightText: <holder>' in the "
                "file header;\n"
                "         - or create '<file>.license' with the same tag;\n"
                "         - or add a [[annotations]] entry in REUSE.toml;\n"
                "         - or run '%s annotate'.\n",
                pszFullPath, pszWcc);
        g_ulErrorCount++;
    }

    if (lic.achLicense[0] != '\0') {
        CheckLicenseExpression(pszFullPath, lic.achLicense, hUsedLicenses);
    }

    ProcessSnippets(pszFullPath, hUsedLicenses);
}

/* ------------------------------------------------------------------ */
/* LICENSES/ directory                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Strip the license extension (or recognise the whole name).
 *
 * @param[in]  pszFname   File name. Not NULL.
 * @param[out] pszBase    Base receiver. Not NULL.
 * @param[in]  ulBaseSize Size of pszBase in bytes.
 */
static void StripLicenseExt(PCSZ pszFname, PSZ pszBase, ULONG ulBaseSize) {
    PSZ pszDot;
    size_t cbLen;

    if (IsValidSpdxName(pszFname)) {
        strncpy(pszBase, pszFname, ulBaseSize - 1);
        pszBase[ulBaseSize - 1] = '\0';
        return;
    }
    strncpy(pszBase, pszFname, ulBaseSize - 1);
    pszBase[ulBaseSize - 1] = '\0';
    pszDot = strrchr(pszBase, '.');
    if (pszDot) *pszDot = '\0';

    cbLen = strlen(pszBase);
    if (cbLen == 0) {
        strncpy(pszBase, pszFname, ulBaseSize - 1);
        pszBase[ulBaseSize - 1] = '\0';
    }
}

/**
 * @brief Check whether a string set contains a string.
 *
 * @param[in] hSet    Set. May be NULLHANDLE.
 * @param[in] pszStr  String. Not NULL.
 *
 * @return TRUE_ if present.
 */
static BOOL StrSetHas(HSTRSET hSet, PCSZ pszStr) {
    BOOL fFound = FALSE_;
    if (hSet == NULLHANDLE) return FALSE_;
    if (StrSetContains(hSet, pszStr, &fFound) != NO_ERROR) return FALSE_;
    return fFound ? TRUE_ : FALSE_;
}

/**
 * @brief Verify the LICENSES/ directory.
 *
 * @param[in] pszProjectDir  Project root. Not NULL.
 * @param[in] hUsedLicenses  Set of used identifiers. Not NULLHANDLE.
 */
static void CheckLicensesDir(PCSZ pszProjectDir, HSTRSET hUsedLicenses) {
    CHAR achLicPath[1024];
    CHAR achExamplePath[1100];
    REUSEDISCOVEROPTIONS opts;
    HSTRSET hPaths = NULLHANDLE;
    HSTRSET hFilesInLic = NULLHANDLE;
    BOOL fDirExists;
    PCSZ pszWcc;

#ifdef __LINUX__
    pszWcc = "_wcc.sh";
    snprintf(achLicPath, sizeof(achLicPath), "%s/LICENSES", pszProjectDir);
    fDirExists = (access(achLicPath, F_OK) == 0) ? TRUE_ : FALSE_;
#else
    pszWcc = "_wcc.cmd";
    snprintf(achLicPath, sizeof(achLicPath), "%s\\LICENSES", pszProjectDir);
    fDirExists = (_access(achLicPath, 0) == 0) ? TRUE_ : FALSE_;
#endif

    if (StrSetCreate(&hFilesInLic) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        g_ulErrorCount++;
        return;
    }

    if (fDirExists) {
        memset(&opts, 0, sizeof(opts));
        opts.fRecursive             = FALSE_;
        opts.fSkipHidden            = TRUE_;
        opts.fSkipVcsDirs           = FALSE_;
        opts.fSkipLicensesDir       = FALSE_;
        opts.fSkipReuseDir          = FALSE_;
        opts.fSkipLicenseSidecars   = FALSE_;
        opts.fSkipReuseToml         = FALSE_;
        opts.fSkipLicenseFiles      = FALSE_;

        if (StrSetCreate(&hPaths) == NO_ERROR) {
            if (ReuseDiscoverWalkTree(achLicPath, &opts, hPaths)
                    == NO_ERROR) {
                HSTRSETENUM hEnum = NULLHANDLE;
                if (StrSetEnumFirst(hPaths, &hEnum) == NO_ERROR) {
                    do {
                        CHAR achFull[PATH_BUF];
                        if (StrSetEnumGet(hEnum, achFull, sizeof(achFull),
                                          NULL) != NO_ERROR)
                            continue;
                        StrSetAdd(hFilesInLic, SpdxGetFileName(achFull));
                    } while (StrSetEnumNext(hEnum) == NO_ERROR);
                    StrSetEnumClose(hEnum);
                }
            }
            StrSetDestroy(hPaths);
        }
    } else {
#ifdef __LINUX__
        snprintf(achExamplePath, sizeof(achExamplePath),
                 "%s/<SPDX-id>.txt", achLicPath);
#else
        snprintf(achExamplePath, sizeof(achExamplePath),
                 "%s\\<SPDX-id>.txt", achLicPath);
#endif
        fprintf(stderr,
                "ERROR: %s is missing.\n"
                "       REUSE requires every license text to be placed in\n"
                "       %s (REUSE Specification 3.3).\n"
                "       Fix one of:\n"
                "         - create the directory and add a text file for "
                "each\n"
                "           license declared by any file in the project;\n"
                "         - or run '%s annotate' to create it "
                "automatically.\n"
                "       See https://reuse.software/spec/ for details.\n",
                achLicPath, achExamplePath, pszWcc);
        g_ulErrorCount++;
    }

    /* First pass: validate each file name in LICENSES/. */
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hFilesInLic, &hEnum) == NO_ERROR) {
            do {
                CHAR achFname[512];
                CHAR achBase[256];

                if (StrSetEnumGet(hEnum, achFname, sizeof(achFname), NULL)
                        != NO_ERROR)
                    continue;

                StripLicenseExt(achFname, achBase, sizeof(achBase));

                if (!IsValidSpdxName(achBase)) {
                    fprintf(stderr,
                            "ERROR: bad license file name: %s\n"
                            "       The name must be a valid SPDX License "
                            "List\n"
                            "       identifier or start with 'LicenseRef-'.\n"
                            "       Fix one of:\n"
                            "         - rename the file to a valid SPDX "
                            "identifier\n"
                            "           (e.g. 'MIT.txt');\n"
                            "         - or use 'LicenseRef-<name>.txt' for a "
                            "custom\n"
                            "           license.\n"
                            "       See https://spdx.org/licenses/ for the "
                            "full list.\n",
                            achFname);
                    g_ulErrorCount++;
                } else {
                    BOOL fDepLic = FALSE_;
                    BOOL fDepExc = FALSE_;

                    if (!HasExtension(achFname)) {
                        fprintf(stderr,
                                "WARNING: license file without extension: "
                                "%s\n"
                                "         REUSE convention is to use "
                                "'.txt'.\n"
                                "         Rename to '%s.txt' to follow the "
                                "convention and avoid ambiguity.\n",
                                achFname, achFname);
                        g_ulWarningCount++;
                    }
                    SpdxQueryLicenseDeprecated(achBase, &fDepLic);
                    SpdxQueryExceptionDeprecated(achBase, &fDepExc);
                    if (fDepLic || fDepExc) {
                        fprintf(stderr,
                                "WARNING: deprecated license file: %s\n"
                                "         SPDX License List marks '%s' as "
                                "deprecated.\n"
                                "         Rename the file to the current "
                                "identifier and update\n"
                                "         all references in source files.\n"
                                "         See https://spdx.org/licenses/ "
                                "for the recommended replacement.\n",
                                achFname, achBase);
                        g_ulWarningCount++;
                    }
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    /* Second pass: every file must be used. */
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hFilesInLic, &hEnum) == NO_ERROR) {
            do {
                CHAR achFname[512];
                CHAR achBase[256];

                if (StrSetEnumGet(hEnum, achFname, sizeof(achFname), NULL)
                        != NO_ERROR)
                    continue;

                StripLicenseExt(achFname, achBase, sizeof(achBase));

                if (!StrSetHas(hUsedLicenses, achBase)) {
                    fprintf(stderr,
                            "ERROR: unused license file: %s\n"
                            "       REUSE Specification 3.3 forbids License "
                            "Files\n"
                            "       for licenses under which none of the "
                            "files in\n"
                            "       the project are licensed.\n"
                            "       Fix one of:\n"
                            "         - remove the file if it is no longer "
                            "needed;\n"
                            "         - or add 'SPDX-License-Identifier: "
                            "%s' to the files it applies to;\n"
                            "         - or add a [[annotations]] entry in "
                            "REUSE.toml referencing this license.\n",
                            achFname, achBase);
                    g_ulErrorCount++;
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    /* Third pass: every used identifier has a file, and the text
     * matches the SPDX database. */
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hUsedLicenses, &hEnum) == NO_ERROR) {
            do {
                CHAR achLic[512];
                CHAR achWithTxt[512];
                CHAR achExpected[1100];
                CHAR achFullPath[1200];
                PCSZ pszActualFname = NULL;
                BOOL fIsRef;
                BOOL fFound;
                BOOL fIsLicValid = FALSE_;
                BOOL fIsExcValid = FALSE_;

                if (StrSetEnumGet(hEnum, achLic, sizeof(achLic), NULL)
                        != NO_ERROR)
                    continue;

                fIsRef = (strncmp(achLic, "LicenseRef-", 11) == 0) ||
                         (strncmp(achLic, "DocumentRef-", 12) == 0);

                snprintf(achWithTxt, sizeof(achWithTxt), "%s.txt", achLic);
                fFound = StrSetHas(hFilesInLic, achLic) ||
                         StrSetHas(hFilesInLic, achWithTxt);

                if (!fIsRef) {
                    SpdxQueryLicenseValid(achLic, &fIsLicValid);
                    SpdxQueryExceptionValid(achLic, &fIsExcValid);
                    if (!fIsLicValid && !fIsExcValid) {
                        fprintf(stderr,
                                "ERROR: '%s' is not a known SPDX "
                                "identifier.\n"
                                "       Check spelling and case against the "
                                "SPDX License List:\n"
                                "         https://spdx.org/licenses/\n",
                                achLic);
                        g_ulErrorCount++;
                        continue;
                    }
                }

                if (!fFound) {
#ifdef __LINUX__
                    snprintf(achExpected, sizeof(achExpected),
                             "%s/%s.txt", achLicPath, achLic);
#else
                    snprintf(achExpected, sizeof(achExpected),
                             "%s\\%s.txt", achLicPath, achLic);
#endif
                    fprintf(stderr,
                            "ERROR: missing license file for %s.\n"
                            "       Files declare this license but\n"
                            "       %s does not exist.\n"
                            "       REUSE requires the full license text in "
                            "LICENSES/\n"
                            "       at the project root (REUSE Specification "
                            "3.3).\n"
                            "       Fix one of:\n"
                            "         - create %s with the license text;\n"
                            "         - or run '%s annotate' to create it "
                            "automatically.\n",
                            achLic, achExpected, achExpected, pszWcc);
                    g_ulErrorCount++;
                    continue;
                }

                if (fIsRef) continue;

                {
                    CHAR achDbText[16384];
                    CHAR achFileText[16384];
                    CHAR achNormFile[16384];
                    CHAR achNormDb[16384];
                    ULONG ulDbSize = sizeof(achDbText);
                    ULONG ulFileSize = sizeof(achFileText);
                    ULONG ulNormFileSize = sizeof(achNormFile);
                    ULONG ulNormDbSize = sizeof(achNormDb);
                    APIRET rcDb;
                    int fEqual = 0;

                    rcDb = SpdxQueryLicenseText(achLic, achDbText,
                                                ulDbSize, NULL);
                    if (rcDb != NO_ERROR)
                        rcDb = SpdxQueryExceptionText(achLic, achDbText,
                                                      ulDbSize, NULL);
                    if (rcDb != NO_ERROR) continue;

                    if (StrSetHas(hFilesInLic, achWithTxt))
                        pszActualFname = achWithTxt;
                    else
                        pszActualFname = achLic;

#ifdef __LINUX__
                    snprintf(achFullPath, sizeof(achFullPath),
                             "%s/%s", achLicPath, pszActualFname);
#else
                    snprintf(achFullPath, sizeof(achFullPath),
                             "%s\\%s", achLicPath, pszActualFname);
#endif

                    if (SpdxReadFileAll(achFullPath, achFileText,
                                        ulFileSize, &ulFileSize) == NO_ERROR) {
                        SpdxNormalizeText(achFileText, achNormFile,
                                          ulNormFileSize,
                                          &ulNormFileSize);
                        SpdxNormalizeText(achDbText, achNormDb,
                                          ulNormDbSize,
                                          &ulNormDbSize);
                        if (strcmp(achNormFile, achNormDb) == 0)
                            fEqual = 1;
                    }

                    if (!fEqual) {
                        fprintf(stderr,
                                "ERROR: license text for %s does not match "
                                "the SPDX License List.\n"
                                "       File: %s\n"
                                "       To update it, run "
                                "'%s annotate'.\n",
                                achLic, achFullPath, pszWcc);
                        g_ulErrorCount++;
                    }
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    StrSetDestroy(hFilesInLic);
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Entry point of the REUSE linter.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    PCSZ pszDir = ".";
    PCSZ pszSpdxDbRoot = NULL;
    PCSZ pszCacheFile = NULL;
    BOOL fNoGitignore = FALSE_;
    int i;
    HREUSETREE hTree = NULLHANDLE;
    HSTRSET hUsedLicenses = NULLHANDLE;
    HSTRSET hPaths = NULLHANDLE;
    APIRET rcDb;
    REUSEDISCOVEROPTIONS walk_opts;
    PSZ pszRepoRoot = NULL;
    GITIGNORELIST gitignore_rules;
    BOOL fHasGitignore = FALSE_;
    int nExit;

    GitIgnoreListInit(&gitignore_rules);

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: reuse-lint [options] [<directory>]\n"
                   "  --spdx-db=<path>           SPDX database root "
                   "(licenses.json,\n"
                   "                             exceptions.json, details/, "
                   "exceptions/)\n"
                   "  --cache=<path>             SPDX database cache file\n"
                   "  --default-license=<id>     Fallback license "
                   "identifier\n"
                   "  --default-copyright=<text> Fallback copyright text\n"
                   "  --no-gitignore             Do not apply .gitignore "
                   "rules\n"
                   "  --help, -h                 Show this help\n");
            GitIgnoreListFree(&gitignore_rules);
            return 0;
        }
        if (strncmp(argv[i], "--spdx-db=", 10) == 0)
            pszSpdxDbRoot = argv[i] + 10;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            pszCacheFile = argv[i] + 8;
        else if (strncmp(argv[i], "--default-license=", 18) == 0)
            g_pszDefaultLicense = argv[i] + 18;
        else if (strncmp(argv[i], "--default-copyright=", 20) == 0)
            g_pszDefaultCopyright = argv[i] + 20;
        else if (strcmp(argv[i], "--no-gitignore") == 0)
            fNoGitignore = TRUE_;
        else if (argv[i][0] != '-')
            pszDir = argv[i];
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s\n"
                    "       Run 'reuse-lint --help' for usage.\n",
                    argv[i]);
            GitIgnoreListFree(&gitignore_rules);
            return 1;
        }
    }

    if (!pszSpdxDbRoot) {
        fprintf(stderr,
                "ERROR: SPDX database is not configured.\n"
                "       --spdx-db=<path> is required.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n"
                "       Run 'reuse-lint --help' for usage.\n");
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }

    rcDb = SpdxOpenDatabase(pszSpdxDbRoot, pszCacheFile);
    if (rcDb & SPDXDB_ERROR_LICENSES) {
        fprintf(stderr,
                "ERROR: SPDX license database is unavailable "
                "(licenses.json not loaded).\n"
                "       Expected at <spdx-db>/licenses.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        SpdxCloseDatabase();
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: SPDX exceptions database is unavailable "
                "(exceptions.json not loaded).\n"
                "       Expected at <spdx-db>/exceptions.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        SpdxCloseDatabase();
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written.\n"
                "         Next run will re-parse JSON indexes.\n");

    {
        ULONG ulSize = 0;
        if (GitFindRepoRoot(pszDir, NULL, 0, &ulSize) == NO_ERROR &&
            ulSize > 0) {
            pszRepoRoot = (PSZ)malloc(ulSize);
            if (pszRepoRoot) {
                if (GitFindRepoRoot(pszDir, pszRepoRoot, ulSize, NULL)
                        != NO_ERROR) {
                    free(pszRepoRoot);
                    pszRepoRoot = NULL;
                }
            }
        }
    }

    {
        APIRET rc = ReuseTreeOpen(pszDir, &hTree);
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot open REUSE project at %s\n"
                    "       The directory is missing or unreadable.\n",
                    pszDir);
            GitIgnoreListFree(&gitignore_rules);
            free(pszRepoRoot);
            SpdxCloseDatabase();
            return 1;
        }
        {
            ULONG ulErrs = 0;
            if (ReuseTreeGetErrorCount(hTree, &ulErrs) == NO_ERROR)
                g_ulErrorCount += (int)ulErrs;
        }
    }

    if (!fNoGitignore) {
        if (GitCollectGitignores(pszRepoRoot, pszDir,
                                 &gitignore_rules) == NO_ERROR &&
            gitignore_rules.ulCount > 0) {
            fHasGitignore = TRUE_;
        }
    }

    if (StrSetCreate(&hUsedLicenses) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        ReuseTreeClose(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(pszRepoRoot);
        SpdxCloseDatabase();
        return 1;
    }

    ReuseSetDiscoverOptionsDefault(&walk_opts);
    walk_opts.fRecursive = FALSE_;
    if (fHasGitignore) {
        walk_opts.fUseGitignore   = TRUE_;
        walk_opts.pszRepoRoot     = pszRepoRoot ? pszRepoRoot : pszDir;
        walk_opts.pGitignoreRules = &gitignore_rules;
    }

    if (StrSetCreate(&hPaths) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        StrSetDestroy(hUsedLicenses);
        ReuseTreeClose(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(pszRepoRoot);
        SpdxCloseDatabase();
        return 1;
    }

    if (ReuseDiscoverWalkTree(pszDir, &walk_opts, hPaths) != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot walk tree: %s\n"
                "       Check that the directory exists and is "
                "readable.\n",
                pszDir);
        StrSetDestroy(hPaths);
        StrSetDestroy(hUsedLicenses);
        ReuseTreeClose(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(pszRepoRoot);
        SpdxCloseDatabase();
        return 1;
    }
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hPaths, &hEnum) == NO_ERROR) {
            do {
                CHAR achFull[PATH_BUF];
                if (StrSetEnumGet(hEnum, achFull, sizeof(achFull), NULL)
                        != NO_ERROR)
                    continue;
                ProcessFile(achFull, hTree, hUsedLicenses);
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }
    StrSetDestroy(hPaths);

    fprintf(stderr, "\n=== LICENSES/ ===\n");
    CheckLicensesDir(pszRepoRoot ? pszRepoRoot : pszDir, hUsedLicenses);

    fprintf(stderr, "\n");
    fprintf(stderr, "REUSE Lint summary for %s:\n", pszDir);
    fprintf(stderr, "  Total files:                %lu\n",
            (unsigned long)g_ulTotalFiles);
    fprintf(stderr, "  Files with license info:    %lu / %lu\n",
            (unsigned long)g_ulFilesWithLicense,
            (unsigned long)g_ulTotalFiles);
    fprintf(stderr, "  Files with copyright info:  %lu / %lu\n",
            (unsigned long)g_ulFilesWithCopyright,
            (unsigned long)g_ulTotalFiles);
    if (g_ulTotalSnippets > 0) {
        fprintf(stderr, "  Snippets:                   %lu\n",
                (unsigned long)g_ulTotalSnippets);
        fprintf(stderr, "  Snippets with license:      %lu / %lu\n",
                (unsigned long)g_ulSnippetsWithLicense,
                (unsigned long)g_ulTotalSnippets);
    }
    fprintf(stderr, "  Read errors:                %lu\n",
            (unsigned long)g_ulReadErrors);
    fprintf(stderr, "  Used licenses:              ");
    {
        ULONG ulUsedCount = 0;
        if (StrSetGetCount(hUsedLicenses, &ulUsedCount) != NO_ERROR)
            ulUsedCount = 0;
        if (ulUsedCount == 0) {
            fprintf(stderr, "(none)\n");
        } else {
            HSTRSETENUM hEnum = NULLHANDLE;
            int fFirst = 1;
            if (StrSetEnumFirst(hUsedLicenses, &hEnum) == NO_ERROR) {
                do {
                    CHAR achLic[512];
                    if (StrSetEnumGet(hEnum, achLic, sizeof(achLic), NULL)
                            != NO_ERROR)
                        continue;
                    fprintf(stderr, "%s%s", fFirst ? "" : ", ", achLic);
                    fFirst = 0;
                } while (StrSetEnumNext(hEnum) == NO_ERROR);
                StrSetEnumClose(hEnum);
            }
            fprintf(stderr, "\n");
        }
    }
    fprintf(stderr, "  Errors:                     %lu\n",
            (unsigned long)g_ulErrorCount);
    fprintf(stderr, "  Warnings:                   %lu\n",
            (unsigned long)g_ulWarningCount);

    StrSetDestroy(hUsedLicenses);
    ReuseTreeClose(hTree);
    GitIgnoreListFree(&gitignore_rules);
    free(pszRepoRoot);
    SpdxCloseDatabase();

    nExit = (g_ulErrorCount > 0) ? 1 : 0;
    return nExit;
}
