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
#include "reuse_licenses.h"
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

/**
 * @brief Human-readable text for a LICENSES/ reason code.
 *
 * @param[in] ulCode  One of REUSE_LICENSES_*.
 *
 * @return Static description, or "" for unknown codes.
 */
static PCSZ LicensesReasonText(ULONG ulCode) {
    switch (ulCode) {
    case REUSE_LICENSES_DIR_MISSING:
        return "LICENSES/ directory is missing";
    case REUSE_LICENSES_DIR_CREATED:
        return "LICENSES/ directory was created";
    case REUSE_LICENSES_BAD_NAME:
        return "file name is not a valid SPDX identifier";
    case REUSE_LICENSES_UNUSED_FILE:
        return "file is not used by any license";
    case REUSE_LICENSES_MISSING_FILE:
        return "used license has no file in LICENSES/";
    case REUSE_LICENSES_DEPRECATED_ID:
        return "identifier is deprecated by SPDX";
    case REUSE_LICENSES_NO_EXTENSION:
        return "license file has no extension";
    case REUSE_LICENSES_TEXT_MISMATCH:
        return "license text does not match the SPDX database";
    case REUSE_LICENSES_FILE_CREATED:
        return "license file was created";
    case REUSE_LICENSES_FILE_UPDATED:
        return "license file was updated";
    case REUSE_LICENSES_FILE_UP_TO_DATE:
        return "license file is up to date";
    case REUSE_LICENSES_FILE_OUTDATED:
        return "license file is outdated; use --force";
    case REUSE_LICENSES_MANUAL_REQUIRED:
        return "LicenseRef-* requires manual text";
    case REUSE_LICENSES_NO_DB_TEXT:
        return "no text in the SPDX database";
    default:
        return "";
    }
}

/**
 * @brief Print every record of a LICENSES/ report to stderr.
 *
 * @param[in]  hReport      Report handle, or NULLHANDLE.
 * @param[out] pulErrors    Receiver for the error count. Not NULL.
 * @param[out] pulWarnings  Receiver for the warning count. Not NULL.
 */
static void PrintLicensesReport(HREUSELICENSEREPORT hReport,
                                PULONG pulErrors, PULONG pulWarnings) {
    ULONG ulCount = 0, ulIdx;

    *pulErrors = 0;
    *pulWarnings = 0;

    if (hReport == NULLHANDLE) return;
    if (ReuseLicensesReportGetCount(hReport, &ulCount) != NO_ERROR)
        return;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        REUSEERR err;
        PCSZ pszSev;
        PCSZ pszReason;

        if (ReuseLicensesReportGet(hReport, ulIdx, &err) != NO_ERROR)
            continue;

        switch (err.ulSeverity) {
        case REUSE_SEV_ERROR:
            pszSev = "ERROR"; (*pulErrors)++; break;
        case REUSE_SEV_WARNING:
            pszSev = "WARNING"; (*pulWarnings)++; break;
        default:
            pszSev = "INFO"; break;
        }

        pszReason = LicensesReasonText(err.ulCode);
        if (!pszReason[0]) pszReason = err.achDetail;

        if (err.achFile[0] && err.achDetail[0] &&
            strcmp(err.achDetail, pszReason) != 0)
            fprintf(stderr, "%s: %s: %s (%s)\n",
                    pszSev, err.achFile, pszReason, err.achDetail);
        else if (err.achFile[0])
            fprintf(stderr, "%s: %s: %s\n",
                    pszSev, err.achFile, pszReason);
        else if (err.achDetail[0])
            fprintf(stderr, "%s: %s\n",
                    pszSev, pszReason);
    }
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
    HREUSELICENSEREPORT hLicReport = NULLHANDLE;
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
    if (ReuseLicensesReportCreate(&hLicReport) == NO_ERROR) {
        ULONG ulLicErrors = 0;
        ULONG ulLicWarnings = 0;
        ReuseLicensesValidate(hTree, hUsedLicenses, hLicReport);
        PrintLicensesReport(hLicReport, &ulLicErrors, &ulLicWarnings);
        g_ulErrorCount += ulLicErrors;
        g_ulWarningCount += ulLicWarnings;
        ReuseLicensesReportFree(hLicReport);
    } else {
        fprintf(stderr, "ERROR: out of memory\n");
        g_ulErrorCount++;
    }

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
