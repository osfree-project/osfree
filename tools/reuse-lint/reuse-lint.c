/* reuse-lint.c - REUSE / SPDX compliance checker (C89, OpenWatcom) */
/*!
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
#include "osadl.h"

/*!
 * @def PATH_BUF
 * @brief Maximum size in bytes of a path buffer used by the linter.
 */
#define PATH_BUF 1024

/*!
 * @def COMPAT_MSG_BUF
 * @brief Maximum size in bytes of one buffered compatibility message.
 */
#define COMPAT_MSG_BUF (PATH_BUF + 512)

#ifdef __LINUX__
/*!
 * @def WCC_CMD
 * @brief Build command wrapper used on Linux hosts.
 */
#define WCC_CMD "_wcc.sh"
#else
/*!
 * @def WCC_CMD
 * @brief Build command wrapper used on non-Linux hosts.
 */
#define WCC_CMD "_wcc.cmd"
#endif

/* ------------------------------------------------------------------ */
/* Global state                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Number of errors reported so far.
 */
static ULONG       g_ulErrorCount = 0;
/*!
 * @brief Number of warnings reported so far.
 */
static ULONG       g_ulWarningCount = 0;

/*!
 * @brief Default SPDX license identifier used as a fallback.
 */
static PCSZ        g_pszDefaultLicense = NULL;
/*!
 * @brief Default copyright text used as a fallback.
 */
static PCSZ        g_pszDefaultCopyright = NULL;

/*!
 * @brief Total number of files walked.
 */
static ULONG       g_ulTotalFiles = 0;
/*!
 * @brief Number of files whose license was resolved from the file itself.
 */
static ULONG       g_ulFilesWithLicense = 0;
/*!
 * @brief Number of files whose copyright was resolved from the file itself.
 */
static ULONG       g_ulFilesWithCopyright = 0;
/*!
 * @brief Number of files that could not be read.
 */
static ULONG       g_ulReadErrors = 0;

/*!
 * @brief Total number of SPDX snippets encountered.
 */
static ULONG       g_ulTotalSnippets = 0;
/*!
 * @brief Number of SPDX snippets that carry a license identifier.
 */
static ULONG       g_ulSnippetsWithLicense = 0;

/*!
 * @brief OSADL matrix handle, or NULLHANDLE when the compatibility
 *        check is not enabled.
 */
static HOSADLMATRIX g_hCompat = NULLHANDLE;
/*!
 * @brief TRUE if --compat-strict was given: OSADL_COMPAT_NO becomes
 *        an ERROR rather than a WARNING.
 */
static BOOL        g_fCompatStrict = FALSE;

/*!
 * @brief Number of files whose license was compatible with the
 *        project license (Same or Yes).
 */
static ULONG       g_ulCompatCompatible = 0;
/*!
 * @brief Number of files whose license requires a dependency check.
 */
static ULONG       g_ulCompatCheckDep = 0;
/*!
 * @brief Number of files whose license is not compatible.
 */
static ULONG       g_ulCompatNo = 0;
/*!
 * @brief Number of files whose compatibility is unknown.
 */
static ULONG       g_ulCompatUnknown = 0;
/*!
 * @brief Number of files whose license is absent from the matrix.
 */
static ULONG       g_ulCompatUndef = 0;

/*!
 * @brief Buffered compatibility messages, printed inside the
 *        "=== Compatibility ===" section.
 *
 * The compatibility checks run during the file walk, which happens
 * before "=== Compatibility ===" is printed. Messages are buffered
 * here so that they appear in their own section rather than
 * interleaved with unrelated output.
 */
static PSZ        *g_papszCompatMessages = NULL;
/*!
 * @brief Number of entries in g_papszCompatMessages.
 */
static ULONG       g_ulCompatMessageCount = 0;
/*!
 * @brief Allocated capacity of g_papszCompatMessages.
 */
static ULONG       g_ulCompatMessageCapacity = 0;

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Append one message line to the compatibility buffer.
 *
 * @param[in] pszLine  NUL-terminated line, typically ending with
 *                     '\n'. Not NULL. Copied into the buffer.
 */
static void CompatRecord(PCSZ pszLine) {
    PSZ pszDup;

    if (pszLine == NULL) return;
    pszDup = strdup(pszLine);
    if (pszDup == NULL) return;

    if (g_ulCompatMessageCount >= g_ulCompatMessageCapacity) {
        ULONG ulNewCap = g_ulCompatMessageCapacity
                         ? g_ulCompatMessageCapacity * 2
                         : 32;
        PSZ *paNew = (PSZ *)realloc(g_papszCompatMessages,
                                    (size_t)ulNewCap * sizeof(PSZ));
        if (paNew == NULL) { free(pszDup); return; }
        g_papszCompatMessages = paNew;
        g_ulCompatMessageCapacity = ulNewCap;
    }
    g_papszCompatMessages[g_ulCompatMessageCount++] = pszDup;
}

/*!
 * @brief Release the compatibility message buffer.
 */
static void CompatRelease(void) {
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < g_ulCompatMessageCount; ulIdx++) {
        if (g_papszCompatMessages[ulIdx] != NULL)
            free(g_papszCompatMessages[ulIdx]);
    }
    if (g_papszCompatMessages != NULL) free(g_papszCompatMessages);
    g_papszCompatMessages = NULL;
    g_ulCompatMessageCount = 0;
    g_ulCompatMessageCapacity = 0;
}

/*!
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

/*!
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
        return "license file is not used by any source file";
    case REUSE_LICENSES_BAD_ID:
        return "identifier is not in the SPDX License List";
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

/*!
 * @brief Specification reference for a LICENSES/ reason code.
 *
 * @param[in] ulCode  One of REUSE_LICENSES_*.
 *
 * @return Static reference text, or "" for unknown codes.
 */
static PCSZ LicensesSpecRef(ULONG ulCode) {
    switch (ulCode) {
    case REUSE_LICENSES_DIR_MISSING:
    case REUSE_LICENSES_DIR_CREATED:
    case REUSE_LICENSES_BAD_NAME:
    case REUSE_LICENSES_UNUSED_FILE:
    case REUSE_LICENSES_MISSING_FILE:
    case REUSE_LICENSES_NO_EXTENSION:
    case REUSE_LICENSES_TEXT_MISMATCH:
    case REUSE_LICENSES_FILE_CREATED:
    case REUSE_LICENSES_FILE_UPDATED:
    case REUSE_LICENSES_FILE_UP_TO_DATE:
    case REUSE_LICENSES_FILE_OUTDATED:
    case REUSE_LICENSES_MANUAL_REQUIRED:
        return "REUSE 3.3 License Files";
    case REUSE_LICENSES_BAD_ID:
        return "REUSE 3.3 Licensing Information";
    case REUSE_LICENSES_DEPRECATED_ID:
        return "SPDX 2.3 License List";
    case REUSE_LICENSES_NO_DB_TEXT:
        return "not in spec";
    default:
        return "";
    }
}

/*!
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
        PCSZ pszRef;

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
            fprintf(stderr, "%s: %s: %s\n",
                    pszSev, err.achDetail, pszReason);

        pszRef = LicensesSpecRef(err.ulCode);
        if (pszRef[0])
            fprintf(stderr, "       [%s]\n", pszRef);

        if (err.ulCode == REUSE_LICENSES_DIR_MISSING) {
            fprintf(stderr,
                    "       Run '%s annotate' to create it.\n",
                    WCC_CMD);
        }
    }
}

/*!
 * @brief Print every project-level diagnostic record to stderr.
 *
 * @param[in]  hProject     Project handle. Not NULLHANDLE.
 * @param[out] pulErrors    Receiver for the error count. Not NULL.
 * @param[out] pulWarnings  Receiver for the warning count. Not NULL.
 */
static void PrintProjectReport(HREUSEPROJECT hProject,
                               PULONG pulErrors, PULONG pulWarnings) {
    ULONG ulCount = 0, ulIdx;

    *pulErrors = 0;
    *pulWarnings = 0;

    if (hProject == NULLHANDLE) return;
    if (ReuseProjectGetRecordCount(hProject, &ulCount) != NO_ERROR)
        return;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        REUSEERR err;
        PCSZ pszSev;

        if (ReuseProjectGetError(hProject, ulIdx, &err) != NO_ERROR)
            continue;

        switch (err.ulSeverity) {
        case REUSE_SEV_ERROR:
            pszSev = "ERROR"; (*pulErrors)++; break;
        case REUSE_SEV_WARNING:
            pszSev = "WARNING"; (*pulWarnings)++; break;
        default:
            pszSev = "INFO"; break;
        }

        if (err.achFile[0] && err.achDetail[0])
            fprintf(stderr, "%s: %s: %s\n",
                    pszSev, err.achFile, err.achDetail);
        else if (err.achFile[0])
            fprintf(stderr, "%s: %s\n", pszSev, err.achFile);
        else if (err.achDetail[0])
            fprintf(stderr, "%s: %s\n", pszSev, err.achDetail);
    }
}

/*!
 * @brief Human-readable text for a REUSE_TAGS_* reason code.
 *
 * @param[in] ulCode  One of REUSE_TAGS_*.
 *
 * @return Static description.
 */
static PCSZ TagsReasonText(ULONG ulCode) {
    switch (ulCode) {
    case REUSE_TAGS_DEPRECATED:
        return "deprecated SPDX tag";
    case REUSE_TAGS_NON_ANNEX_FORM:
        return "Clause 8/9 form in source file; "
               "use the Annex H spelling";
    case REUSE_TAGS_LONG_LINE:
        return "tag value truncated";
    default:
        return "tag warning";
    }
}

/*!
 * @brief Print every record of an in-file tags warning report.
 *
 * @param[in]  hReport      Report handle, or NULLHANDLE.
 * @param[out] pulErrors    Receiver for the error count. Not NULL.
 * @param[out] pulWarnings  Receiver for the warning count. Not NULL.
 */
static void PrintTagsReport(HREUSETAGSREPORT hReport,
                            PULONG pulErrors, PULONG pulWarnings) {
    ULONG ulCount = 0, ulIdx;

    *pulErrors = 0;
    *pulWarnings = 0;

    if (hReport == NULLHANDLE) return;
    if (ReuseQueryTagsReportCount(hReport, &ulCount) != NO_ERROR)
        return;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        REUSEERR err;
        PCSZ pszSev;
        PCSZ pszReason;

        if (ReuseQueryTagsReport(hReport, ulIdx, &err) != NO_ERROR)
            continue;

        switch (err.ulSeverity) {
        case REUSE_SEV_ERROR:
            pszSev = "ERROR"; (*pulErrors)++; break;
        case REUSE_SEV_WARNING:
            pszSev = "WARNING"; (*pulWarnings)++; break;
        default:
            pszSev = "INFO"; break;
        }

        pszReason = TagsReasonText(err.ulCode);

        if (err.achFile[0] && err.achDetail[0])
            fprintf(stderr, "%s: %s:%lu: %s (%s)\n",
                    pszSev, err.achFile, (unsigned long)err.ulLine,
                    pszReason, err.achDetail);
        else if (err.achFile[0])
            fprintf(stderr, "%s: %s:%lu: %s\n",
                    pszSev, err.achFile, (unsigned long)err.ulLine,
                    pszReason);
    }
}

/* ------------------------------------------------------------------ */
/* Expression validation                                               */
/* ------------------------------------------------------------------ */

/*!
 * @brief Validate one SPDX expression and collect its identifiers.
 *
 * @param[in] pszFullPath  File path used in diagnostics. Not NULL.
 * @param[in] pszLicense   Expression. Not NULL.
 * @param[in] hUsedIds     Set of used identifiers. May be NULLHANDLE.
 */
static void CheckLicenseExpression(PCSZ pszFullPath, PCSZ pszLicense,
                                   HSTRSET hUsedIds) {
    PCSZ pszBad = NULL;
    HSTRSET hIds = NULLHANDLE;
    HSTRSETENUM hEnum = NULLHANDLE;
    APIRET rcExpr;

    if (StrSetCreate(&hIds) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory [not in spec]\n");
        g_ulErrorCount++;
        return;
    }

    rcExpr = SpdxExpressionCollectIds(pszLicense, hIds, &pszBad);

    if (rcExpr == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "ERROR: %s: invalid SPDX license expression: '%s'\n"
                "       [SPDX 2.3 Annex D]\n",
                pszFullPath, pszLicense);
        g_ulErrorCount++;
    } else if (rcExpr == SPDX_EXPR_UNKNOWN_TOKEN) {
        fprintf(stderr, "ERROR: %s: unknown SPDX identifier: '",
                pszFullPath);
        PrintBadToken(pszBad);
        fprintf(stderr,
                "'\n"
                "       [SPDX 2.3 Annex D]\n");
        g_ulErrorCount++;
    } else if (rcExpr == SPDXDB_ERROR_LICENSES ||
               rcExpr == SPDXDB_ERROR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: %s: SPDX database is not loaded. "
                "Cannot validate SPDX identifiers. [not in spec]\n",
                pszFullPath);
        g_ulErrorCount++;
    } else if (rcExpr != NO_ERROR) {
        fprintf(stderr,
                "ERROR: %s: cannot validate SPDX expression "
                "(code %lu). [not in spec]\n",
                pszFullPath, (unsigned long)rcExpr);
        g_ulErrorCount++;
    }

    if (rcExpr == NO_ERROR && hUsedIds != NULLHANDLE) {
        if (StrSetEnumFirst(hIds, &hEnum) == NO_ERROR) {
            do {
                CHAR achId[256];
                BOOL fDepLic = FALSE;
                BOOL fDepExc = FALSE;

                if (StrSetEnumGet(hEnum, achId, sizeof(achId), NULL)
                        != NO_ERROR)
                    continue;
                StrSetAdd(hUsedIds, achId);
                SpdxQueryLicenseDeprecated(achId, &fDepLic);
                SpdxQueryExceptionDeprecated(achId, &fDepExc);
                if (fDepLic || fDepExc) {
                    fprintf(stderr,
                            "WARNING: %s: deprecated SPDX identifier "
                            "'%s'. [SPDX 2.3 License List]\n",
                            pszFullPath, achId);
                    g_ulWarningCount++;
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    StrSetDestroy(hIds);
}

/* ------------------------------------------------------------------ */
/* OSADL compatibility check                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Check one file's license against the project license via
 *        the OSADL matrix.
 *
 * Called from ProcessFile for files whose license did not come from
 * the default fallback. Bumps counters and records diagnostics in
 * the compatibility buffer. Severity: NO -> ERROR (with
 * --compat-strict) or WARNING; CHECK_DEP -> WARNING; UNKNOWN /
 * UNDEF -> INFO.
 *
 * UNDEF here always means the file's license is not in the matrix:
 * if the project license were missing, the matrix would have been
 * closed during startup with a one-time WARNING, and this function
 * would not be reached.
 *
 * @param[in] pszFullPath    File path used in diagnostics. Not NULL.
 * @param[in] pszFileLicense Resolved license expression. Not NULL.
 */
static void CheckCompatibility(PCSZ pszFullPath, PCSZ pszFileLicense) {
    OSADLCOMPAT compat;
    APIRET rc;
    CHAR achMsg[COMPAT_MSG_BUF];

    if (g_hCompat == NULLHANDLE) return;
    if (g_pszDefaultLicense == NULL) return;
    if (pszFileLicense == NULL || pszFileLicense[0] == '\0') return;

    rc = OsadlQueryPair(g_hCompat, g_pszDefaultLicense,
                        pszFileLicense, &compat);
    if (rc != NO_ERROR) {
        snprintf(achMsg, sizeof(achMsg),
                 "WARNING: %s: OSADL compatibility check failed "
                 "(code %lu). [not in spec]\n",
                 pszFullPath, (unsigned long)rc);
        CompatRecord(achMsg);
        g_ulWarningCount++;
        return;
    }

    switch (compat) {
    case OSADL_COMPAT_SAME:
    case OSADL_COMPAT_YES:
        g_ulCompatCompatible++;
        break;
    case OSADL_COMPAT_NO:
        g_ulCompatNo++;
        if (g_fCompatStrict) {
            snprintf(achMsg, sizeof(achMsg),
                     "ERROR: %s: license '%s' is not compatible with "
                     "project license '%s'.\n"
                     "       [OSADL matrix; ISO/IEC 5230:2020 §3.3.2]\n",
                     pszFullPath, pszFileLicense, g_pszDefaultLicense);
            CompatRecord(achMsg);
            g_ulErrorCount++;
        } else {
            snprintf(achMsg, sizeof(achMsg),
                     "WARNING: %s: license '%s' is not compatible with "
                     "project license '%s'.\n"
                     "         [OSADL matrix; ISO/IEC 5230:2020 §3.3.2]\n"
                     "         Pass --compat-strict to treat as ERROR.\n",
                     pszFullPath, pszFileLicense, g_pszDefaultLicense);
            CompatRecord(achMsg);
            g_ulWarningCount++;
        }
        break;
    case OSADL_COMPAT_CHECK_DEP:
        g_ulCompatCheckDep++;
        snprintf(achMsg, sizeof(achMsg),
                 "WARNING: %s: license '%s' requires a dependency "
                 "check against project license '%s'.\n"
                 "         [OSADL matrix; ISO/IEC 5230:2020 §3.3.2]\n",
                 pszFullPath, pszFileLicense, g_pszDefaultLicense);
        CompatRecord(achMsg);
        g_ulWarningCount++;
        break;
    case OSADL_COMPAT_UNKNOWN:
        g_ulCompatUnknown++;
        snprintf(achMsg, sizeof(achMsg),
                 "INFO: %s: OSADL compatibility of '%s' with '%s' "
                 "is unknown.\n"
                 "      [OSADL matrix]\n",
                 pszFullPath, pszFileLicense, g_pszDefaultLicense);
        CompatRecord(achMsg);
        break;
    case OSADL_COMPAT_UNDEF:
        g_ulCompatUndef++;
        snprintf(achMsg, sizeof(achMsg),
                 "INFO: %s: license '%s' is not in the OSADL matrix.\n"
                 "      [OSADL matrix]\n",
                 pszFullPath, pszFileLicense);
        CompatRecord(achMsg);
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Snippets                                                            */
/* ------------------------------------------------------------------ */

/*!
 * @brief Check every SPDX snippet of one file.
 *
 * Reads the snippets through the REUSE layer
 * (ReuseQueryFileSnippets), so comment markers are stripped according
 * to the file's comment style and REUSE-IgnoreStart/End regions are
 * honoured.
 *
 * @param[in] pszFullPath    File path used in diagnostics. Not NULL.
 * @param[in] hUsedLicenses  Set of used identifiers. Not NULLHANDLE.
 * @param[in] hStyle         Comment style. Not NULLHANDLE.
 * @param[in] hReport        Tag warnings report, or NULLHANDLE.
 */
static void ProcessSnippets(PCSZ pszFullPath, HSTRSET hUsedLicenses,
                            HREUSECOMMENTSTYLE hStyle,
                            HREUSETAGSREPORT hReport) {
    SPDXSNIPPETLIST snippets;
    ULONG ulIdx;
    APIRET rc;

    rc = ReuseQueryFileSnippets(pszFullPath, hStyle, hReport, &snippets);
    if (rc != NO_ERROR) {
        fprintf(stderr,
                "ERROR: %s: cannot read SPDX snippets (code %lu). "
                "[not in spec]\n",
                pszFullPath, (unsigned long)rc);
        g_ulErrorCount++;
        return;
    }

    for (ulIdx = 0; ulIdx < snippets.ulCount; ulIdx++) {
        PSPDXSNIPPET pS = &snippets.pItems[ulIdx];

        g_ulTotalSnippets++;

        if (!pS->pszLicense || pS->pszLicense[0] == '\0') {
            fprintf(stderr,
                    "ERROR: %s:%lu-%lu: snippet has no "
                    "SPDX-License-Identifier. "
                    "[REUSE 3.3 In-line Snippet comments]\n",
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
                    "SPDX-SnippetCopyrightText. "
                    "[REUSE 3.3 In-line Snippet comments]\n",
                    pszFullPath,
                    (unsigned long)pS->ulLineStart,
                    (unsigned long)pS->ulLineEnd);
            g_ulWarningCount++;
        }
    }

    ReuseSnippetListFree(&snippets);
}

/* ------------------------------------------------------------------ */
/* Per-file processing                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @brief Process one file: resolve, validate, count.
 *
 * @param[in] pszFullPath     File path. Not NULL.
 * @param[in] hProject        REUSE project handle. Not NULLHANDLE.
 * @param[in] hUsedLicenses   Set of used identifiers. Not NULLHANDLE.
 * @param[in] hTagsReport     Tag warnings report, or NULLHANDLE.
 */
static void ProcessFile(PCSZ pszFullPath, HREUSEPROJECT hProject,
                        HSTRSET hUsedLicenses,
                        HREUSETAGSREPORT hTagsReport) {
    REUSELICENSEINFO lic;
    HREUSECOMMENTSTYLE hStyle = NULLHANDLE;
    BOOL fStyleFound = FALSE;
    FILE *fp;
    APIRET rcResolve;

    g_ulTotalFiles++;

    fp = fopen(pszFullPath, "rb");
    if (!fp) {
        fprintf(stderr,
                "ERROR: cannot read file: %s [not in spec]\n",
                pszFullPath);
        g_ulErrorCount++;
        g_ulReadErrors++;
        return;
    }
    fclose(fp);

    rcResolve = ReuseResolveLicense(hProject, pszFullPath,
                                    g_pszDefaultLicense,
                                    g_pszDefaultCopyright, &lic);

    if (rcResolve == REUSE_ERROR_NO_COMMENT_STYLE) {
        fprintf(stderr,
                "ERROR: %s: no [[map]] entry in comments.toml matches "
                "this file.\n"
                "       Add a [[map]] entry (e.g. glob = \"*.<ext>\", "
                "style = ...).\n",
                pszFullPath);
        g_ulErrorCount++;
        return;
    }

    if (rcResolve != NO_ERROR) {
        fprintf(stderr,
                "ERROR: %s: license and copyright are missing.\n"
                "       Run '%s annotate' to write the license\n"
                "       \"%s\"\n"
                "       and the copyright\n"
                "       \"%s\"\n"
                "       into the file, or add a sidecar/REUSE.toml "
                "entry.\n"
                "       [REUSE 3.3 Licensing Information]\n",
                pszFullPath, WCC_CMD,
                g_pszDefaultLicense ? g_pszDefaultLicense : "",
                g_pszDefaultCopyright ? g_pszDefaultCopyright : "");
        g_ulErrorCount++;
        return;
    }

    if (lic.fLicenseFromDefault) {
        fprintf(stderr,
                "WARNING: %s: license is missing.\n"
                "         Run '%s annotate' to write the license\n"
                "         \"%s\"\n"
                "         into the file, or add a sidecar/REUSE.toml "
                "entry.\n"
                "         [REUSE 3.3 Licensing Information]\n",
                pszFullPath, WCC_CMD,
                g_pszDefaultLicense ? g_pszDefaultLicense : "");
        g_ulWarningCount++;
    }
    if (lic.fCopyrightFromDefault) {
        fprintf(stderr,
                "WARNING: %s: copyright is missing.\n"
                "         Run '%s annotate' to write the copyright\n"
                "         \"%s\"\n"
                "         into the file, or add a sidecar/REUSE.toml "
                "entry.\n"
                "         [REUSE 3.3 Licensing Information]\n",
                pszFullPath, WCC_CMD,
                g_pszDefaultCopyright ? g_pszDefaultCopyright : "");
        g_ulWarningCount++;
    }

    if (lic.fLicenseTruncated) {
        fprintf(stderr,
                "WARNING: %s: license value was truncated. "
                "[not in spec]\n",
                pszFullPath);
        g_ulWarningCount++;
    }
    if (lic.fCopyrightTruncated) {
        fprintf(stderr,
                "WARNING: %s: copyright value was truncated. "
                "[not in spec]\n",
                pszFullPath);
        g_ulWarningCount++;
    }

    if (lic.achLicense[0] != '\0') {
        if (!lic.fLicenseFromDefault)
            g_ulFilesWithLicense++;
    } else {
        fprintf(stderr,
                "ERROR: %s: license is missing.\n"
                "       Run '%s annotate' to write the license\n"
                "       \"%s\"\n"
                "       into the file, or add a sidecar/REUSE.toml "
                "entry.\n"
                "       [REUSE 3.3 Licensing Information]\n",
                pszFullPath, WCC_CMD,
                g_pszDefaultLicense ? g_pszDefaultLicense : "");
        g_ulErrorCount++;
    }
    if (lic.achCopyright[0] != '\0') {
        if (!lic.fCopyrightFromDefault)
            g_ulFilesWithCopyright++;
    } else {
        fprintf(stderr,
                "ERROR: %s: copyright is missing.\n"
                "       Run '%s annotate' to write the copyright\n"
                "       \"%s\"\n"
                "       into the file, or add a sidecar/REUSE.toml "
                "entry.\n"
                "       [REUSE 3.3 Licensing Information]\n",
                pszFullPath, WCC_CMD,
                g_pszDefaultCopyright ? g_pszDefaultCopyright : "");
        g_ulErrorCount++;
    }

    if (lic.achLicense[0] != '\0') {
        CheckLicenseExpression(pszFullPath, lic.achLicense, hUsedLicenses);
        if (!lic.fLicenseFromDefault) {
            CheckCompatibility(pszFullPath, lic.achLicense);
        }
    }

    if (ReuseQueryProjectCommentStyle(hProject, pszFullPath,
                                      &fStyleFound, &hStyle) != NO_ERROR)
        fStyleFound = FALSE;
    if (fStyleFound && hStyle != NULLHANDLE) {
        ProcessSnippets(pszFullPath, hUsedLicenses, hStyle, hTagsReport);
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Entry point of the REUSE linter.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @retval 0 Lint completed without errors.
 * @retval 1 Lint completed with errors.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    PCSZ pszDir = ".";
    PCSZ pszSpdxDbRoot = NULL;
    PCSZ pszCacheFile = NULL;
    PCSZ pszCommentsConfig = NULL;
    PCSZ pszCompatMatrix = NULL;
    int i;
    HREUSEPROJECT hProject = NULLHANDLE;
    HSTRSET hUsedLicenses = NULLHANDLE;
    HSTRSET hPaths = NULLHANDLE;
    HREUSELICENSEREPORT hLicReport = NULLHANDLE;
    HREUSETAGSREPORT hTagsReport = NULLHANDLE;
    APIRET rcDb;
    int nExit;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: reuse-lint [options] [<directory>]\n"
                   "  --spdx-db=<path>           SPDX database root "
                   "(licenses.json,\n"
                   "                             exceptions.json, details/, "
                   "exceptions/)\n"
                   "  --comments-config=<path>   comments.toml describing "
                   "comment styles\n"
                   "                             for in-file SPDX tags "
                   "(required)\n"
                   "  --cache=<path>             SPDX database cache file\n"
                   "  --default-license=<id>     Fallback license "
                   "identifier\n"
                   "  --default-copyright=<text> Fallback copyright text\n"
                   "  --compat-matrix=<path>     OSADL matrix.json; enable "
                   "license\n"
                   "                             compatibility check "
                   "against\n"
                   "                             --default-license\n"
                   "  --compat-strict            Treat incompatible "
                   "license as ERROR\n"
                   "                             (default: WARNING)\n"
                   "  --help, -h                 Show this help\n");
            return 0;
        }
        if (strncmp(argv[i], "--spdx-db=", 10) == 0)
            pszSpdxDbRoot = argv[i] + 10;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            pszCacheFile = argv[i] + 8;
        else if (strncmp(argv[i], "--comments-config=", 18) == 0)
            pszCommentsConfig = argv[i] + 18;
        else if (strncmp(argv[i], "--default-license=", 18) == 0)
            g_pszDefaultLicense = argv[i] + 18;
        else if (strncmp(argv[i], "--default-copyright=", 20) == 0)
            g_pszDefaultCopyright = argv[i] + 20;
        else if (strncmp(argv[i], "--compat-matrix=", 16) == 0)
            pszCompatMatrix = argv[i] + 16;
        else if (strcmp(argv[i], "--compat-strict") == 0)
            g_fCompatStrict = TRUE;
        else if (argv[i][0] != '-')
            pszDir = argv[i];
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s [not in spec]\n"
                    "       Run 'reuse-lint --help' for usage.\n",
                    argv[i]);
            return 1;
        }
    }

    if (!pszSpdxDbRoot) {
        fprintf(stderr,
                "ERROR: SPDX database is not configured. "
                "[not in spec]\n"
                "       --spdx-db=<path> is required.\n"
                "       Run 'reuse-lint --help' for usage.\n");
        return 1;
    }

    rcDb = SpdxOpenDatabase(pszSpdxDbRoot, pszCacheFile);
    if (rcDb & SPDXDB_ERROR_LICENSES) {
        fprintf(stderr,
                "ERROR: SPDX license database is unavailable "
                "(licenses.json not loaded). [not in spec]\n"
                "       Expected at <spdx-db>/licenses.json.\n");
        SpdxCloseDatabase();
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: SPDX exceptions database is unavailable "
                "(exceptions.json not loaded). [not in spec]\n"
                "       Expected at <spdx-db>/exceptions.json.\n");
        SpdxCloseDatabase();
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written. [not in spec]\n"
                "         Next run will re-parse JSON indexes.\n");

    {
        APIRET rc = ReuseOpenProject(pszDir, pszCommentsConfig,
                                     &hProject);
        if (rc == REUSE_ERROR_NO_COMMENT_STYLE) {
            fprintf(stderr,
                    "ERROR: --comments-config=<path> is required.\n"
                    "       It defines the comment styles used to read "
                    "in-file SPDX tags.\n"
                    "       Run 'reuse-lint --help' for usage.\n");
            SpdxCloseDatabase();
            return 1;
        }
        if (rc != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot open REUSE project at %s. "
                    "[not in spec]\n"
                    "       The directory is missing or unreadable.\n",
                    pszDir);
            SpdxCloseDatabase();
            return 1;
        }
        fprintf(stderr, "=== Project ===\n");
        {
            ULONG ulProjErrors = 0;
            ULONG ulProjWarnings = 0;
            PrintProjectReport(hProject, &ulProjErrors, &ulProjWarnings);
            g_ulErrorCount += ulProjErrors;
            g_ulWarningCount += ulProjWarnings;
        }
    }

    if (ReuseQueryProjectTagsReport(hProject, &hTagsReport) != NO_ERROR)
        hTagsReport = NULLHANDLE;

    if (StrSetCreate(&hUsedLicenses) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory [not in spec]\n");
        ReuseCloseProject(hProject);
        SpdxCloseDatabase();
        return 1;
    }

    if (StrSetCreate(&hPaths) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory [not in spec]\n");
        StrSetDestroy(hUsedLicenses);
        ReuseCloseProject(hProject);
        SpdxCloseDatabase();
        return 1;
    }

    if (pszCompatMatrix) {
        if (!g_pszDefaultLicense) {
            CHAR achMsg[COMPAT_MSG_BUF];
            snprintf(achMsg, sizeof(achMsg),
                     "WARNING: --compat-matrix is set but "
                     "--default-license is not.\n"
                     "         Compatibility check will be skipped; "
                     "there is no\n"
                     "         outbound (project) license to compare "
                     "against.\n"
                     "         [ISO/IEC 5230:2020 §3.3.2]\n");
            CompatRecord(achMsg);
            g_ulWarningCount++;
        } else {
            APIRET rcCompat = OsadlOpenMatrix(pszCompatMatrix, &g_hCompat);
            if (rcCompat != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: cannot open OSADL matrix at %s "
                        "(code %lu).\n"
                        "       [not in spec]\n"
                        "       Download matrix.json from\n"
                        "       https://www.osadl.org/fileadmin/"
                        "checklists/matrix.json\n",
                        pszCompatMatrix, (unsigned long)rcCompat);
                CompatRelease();
                StrSetDestroy(hPaths);
                StrSetDestroy(hUsedLicenses);
                ReuseCloseProject(hProject);
                SpdxCloseDatabase();
                return 1;
            }
            {
                BOOL fProjPresent = FALSE;
                if (OsadlQueryHasLicense(g_hCompat, g_pszDefaultLicense,
                                         &fProjPresent) == NO_ERROR
                    && !fProjPresent) {
                    CHAR achMsg[COMPAT_MSG_BUF];
                    snprintf(achMsg, sizeof(achMsg),
                             "WARNING: project license '%s' is not in "
                             "the OSADL matrix.\n"
                             "         Compatibility check will be "
                             "skipped.\n"
                             "         [OSADL matrix]\n",
                             g_pszDefaultLicense);
                    CompatRecord(achMsg);
                    g_ulWarningCount++;
                    OsadlCloseMatrix(g_hCompat);
                    g_hCompat = NULLHANDLE;
                }
            }
        }
    }

    if (ReuseDiscoverFiles(pszDir, NULL, 0, FALSE, hPaths) != NO_ERROR) {
        fprintf(stderr,
                "ERROR: cannot walk tree: %s [not in spec]\n"
                "       Check that the directory exists and is "
                "readable.\n",
                pszDir);
        OsadlCloseMatrix(g_hCompat);
        CompatRelease();
        StrSetDestroy(hPaths);
        StrSetDestroy(hUsedLicenses);
        ReuseCloseProject(hProject);
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
                ProcessFile(achFull, hProject, hUsedLicenses, hTagsReport);
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }
    StrSetDestroy(hPaths);

    fprintf(stderr, "\n=== Tags ===\n");
    {
        ULONG ulTagErrors = 0;
        ULONG ulTagWarnings = 0;
        PrintTagsReport(hTagsReport, &ulTagErrors, &ulTagWarnings);
        g_ulErrorCount += ulTagErrors;
        g_ulWarningCount += ulTagWarnings;
    }

    fprintf(stderr, "\n=== LICENSES/ ===\n");
    if (ReuseLicensesReportCreate(&hLicReport) == NO_ERROR) {
        ULONG ulLicErrors = 0;
        ULONG ulLicWarnings = 0;
        ReuseLicensesValidate(hProject, hUsedLicenses, hLicReport);
        PrintLicensesReport(hLicReport, &ulLicErrors, &ulLicWarnings);
        g_ulErrorCount += ulLicErrors;
        g_ulWarningCount += ulLicWarnings;
        ReuseLicensesReportFree(hLicReport);
    } else {
        fprintf(stderr, "ERROR: out of memory [not in spec]\n");
        g_ulErrorCount++;
    }

    if (g_hCompat != NULLHANDLE || g_ulCompatMessageCount > 0) {
        ULONG ulMsgIdx;
        fprintf(stderr, "\n=== Compatibility ===\n");
        for (ulMsgIdx = 0; ulMsgIdx < g_ulCompatMessageCount; ulMsgIdx++) {
            if (g_papszCompatMessages[ulMsgIdx] != NULL)
                fprintf(stderr, "%s", g_papszCompatMessages[ulMsgIdx]);
        }
        if (g_hCompat != NULLHANDLE) {
            if (g_ulCompatMessageCount > 0) fprintf(stderr, "\n");
            fprintf(stderr, "  Project license:            %s\n",
                    g_pszDefaultLicense ? g_pszDefaultLicense : "(none)");
            {
                CHAR achTs[128];
                ULONG ulTsUsed = 0;
                if (OsadlQueryTimestamp(g_hCompat, achTs,
                                        sizeof(achTs), &ulTsUsed) == NO_ERROR
                    && ulTsUsed > 0) {
                    fprintf(stderr,
                            "  OSADL matrix snapshot:      %s\n", achTs);
                }
            }
            fprintf(stderr, "  Compatible files:           %lu\n",
                    (unsigned long)g_ulCompatCompatible);
            fprintf(stderr, "  Check-dependency files:     %lu\n",
                    (unsigned long)g_ulCompatCheckDep);
            fprintf(stderr, "  Incompatible files:         %lu%s\n",
                    (unsigned long)g_ulCompatNo,
                    g_fCompatStrict ? " (errors)" : " (warnings)");
            fprintf(stderr, "  Unknown compatibility:      %lu\n",
                    (unsigned long)g_ulCompatUnknown);
            fprintf(stderr, "  Not in OSADL matrix:        %lu\n",
                    (unsigned long)g_ulCompatUndef);
        }
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

    OsadlCloseMatrix(g_hCompat);
    CompatRelease();
    StrSetDestroy(hUsedLicenses);
    ReuseCloseProject(hProject);
    SpdxCloseDatabase();

    nExit = (g_ulErrorCount > 0) ? 1 : 0;
    return nExit;
}
