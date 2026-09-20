/* spdx_sbom_extracted.c - LicenseRef-* text collection (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_sbom_extracted.h"
#include "spdx_sbom_utils.h"
#include "spdx.h"
#include "spdx_db.h"

/**
 * @file spdx_sbom_extracted.c
 * @brief Implementation of the LicenseRef-* text collection.
 */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a whole file into a heap string.
 *
 * Uses the size-query convention of SpdxReadFileAll.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return malloc'd NUL-terminated content, or NULL on error.
 */
static PSZ read_file_to_heap(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxReadFileAll(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxReadFileAll(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    return pszOut;
}

/* ------------------------------------------------------------------ */
/* Text lookup                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Look up a LicenseRef text in the command-line sources.
 *
 * @param[in] paExtra      Array of sources, or NULL.
 * @param[in] ulExtraCount Number of entries in @p paExtra.
 * @param[in] pszId        Identifier. Not NULL.
 *
 * @return malloc'd text, or NULL if no source matched or on read
 *         error.
 */
static PSZ find_text_in_extra(
    const SPDXEXTRACTEDLICENSESOURCE *paExtra,
    ULONG ulExtraCount,
    PCSZ pszId)
{
    ULONG ulIdx;
    PSZ pszText;

    if (!paExtra) return NULL;
    for (ulIdx = 0; ulIdx < ulExtraCount; ulIdx++) {
        if (strcmp(paExtra[ulIdx].achId, pszId) == 0) {
            pszText = read_file_to_heap(paExtra[ulIdx].achPath);
            return pszText;
        }
    }
    return NULL;
}

/**
 * @brief Look up a LicenseRef text under <project_dir>/LICENSES/.
 *
 * Tries four variants: <id>.txt and <id>, each with '/' and '\\'
 * as separator.
 *
 * @param[in] pszProjectDir  Project root. Not NULL.
 * @param[in] pszId          Identifier. Not NULL.
 *
 * @return malloc'd text, or NULL if no variant matched or on read
 *         error.
 */
static PSZ find_text_in_licenses_dir(PCSZ pszProjectDir, PCSZ pszId) {
    CHAR achPath[2048];
    PSZ pszText;

    snprintf(achPath, sizeof(achPath), "%s/LICENSES/%s.txt",
             pszProjectDir, pszId);
    pszText = read_file_to_heap(achPath);
    if (pszText) return pszText;

    snprintf(achPath, sizeof(achPath), "%s\\LICENSES\\%s.txt",
             pszProjectDir, pszId);
    pszText = read_file_to_heap(achPath);
    if (pszText) return pszText;

    snprintf(achPath, sizeof(achPath), "%s/LICENSES/%s",
             pszProjectDir, pszId);
    pszText = read_file_to_heap(achPath);
    if (pszText) return pszText;

    snprintf(achPath, sizeof(achPath), "%s\\LICENSES\\%s",
             pszProjectDir, pszId);
    pszText = read_file_to_heap(achPath);
    if (pszText) return pszText;

    return NULL;
}

/* ------------------------------------------------------------------ */
/* Expression scanning                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Collect LicenseRef-* identifiers from one expression.
 *
 * The expression is first passed through SpdxExpressionCollectIds,
 * which validates it and gathers every identifier. Tokens that do
 * not start with "LicenseRef-" are dropped; duplicates are removed
 * by the destination set.
 *
 * @param[in] pszExpr  Expression. May be NULL or empty.
 * @param[in] hIds     Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hIds is not recognized.
 * @retval SPDX_EXPR_SYNTAX_ERROR   Grammar violation.
 * @retval SPDX_EXPR_UNKNOWN_TOKEN  Unknown SPDX identifier.
 * @retval SPDXDB_ERROR_LICENSES    The license index is not loaded.
 * @retval SPDXDB_ERROR_EXCEPTIONS  The exception index is not loaded.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET scan_expression_ids(PCSZ pszExpr, HSTRSET hIds) {
    HSTRSET hAll = NULLHANDLE;
    HSTRSETENUM hEnum = NULLHANDLE;
    APIRET rc;

    if (!pszExpr || !pszExpr[0]) return NO_ERROR;

    rc = StrSetCreate(&hAll);
    if (rc != NO_ERROR) return rc;

    rc = SpdxExpressionCollectIds(pszExpr, hAll, NULL);
    if (rc != NO_ERROR) {
        StrSetDestroy(hAll);
        return rc;
    }

    if (StrSetEnumFirst(hAll, &hEnum) == NO_ERROR) {
        do {
            CHAR achTok[256];
            if (StrSetEnumGet(hEnum, achTok, sizeof(achTok), NULL)
                    != NO_ERROR)
                continue;
            if (strncmp(achTok, "LicenseRef-", 11) == 0) {
                rc = StrSetAdd(hIds, achTok);
                if (rc != NO_ERROR) {
                    StrSetEnumClose(hEnum);
                    StrSetDestroy(hAll);
                    return rc;
                }
            }
        } while (StrSetEnumNext(hEnum) == NO_ERROR);
        StrSetEnumClose(hEnum);
    }

    StrSetDestroy(hAll);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Collect LicenseRef-* texts for the whole file list.
 *
 * @param[in] hList          Destination list. Not NULLHANDLE.
 * @param[in] hFiles         File list to scan. Not NULLHANDLE.
 * @param[in] pszProjectDir  Project root. Not NULL.
 * @param[in] paExtra        Array of command-line sources, or NULL.
 * @param[in] ulExtraCount   Number of entries in @p paExtra.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList or hFiles is NULLHANDLE,
 *                                  or pszProjectDir is NULL.
 * @retval ERROR_INVALID_HANDLE     hList or hFiles is not
 *                                  recognized.
 * @retval ERROR_FILE_NOT_FOUND     Text not available for one of
 *                                  the LicenseRef-* identifiers.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCollectExtractedLicenses(
    HVECTOR hList,
    HVECTOR hFiles,
    PCSZ pszProjectDir,
    const SPDXEXTRACTEDLICENSESOURCE *paExtra,
    ULONG ulExtraCount)
{
    HSTRSET hIds = NULLHANDLE;
    HSTRSETENUM hEnum = NULLHANDLE;
    ULONG ulFiles = 0, ulIdx;
    APIRET rc;

    if (hList == NULLHANDLE || hFiles == NULLHANDLE || !pszProjectDir)
        return ERROR_INVALID_PARAMETER;

    rc = StrSetCreate(&hIds);
    if (rc != NO_ERROR) return rc;

    rc = VectorGetCount(hFiles, &ulFiles);
    if (rc != NO_ERROR) {
        StrSetDestroy(hIds);
        return rc;
    }

    /* Phase 1: collect unique LicenseRef-* identifiers. */
    for (ulIdx = 0; ulIdx < ulFiles; ulIdx++) {
        SPDXFILEINFO info;
        if (VectorGetItem(hFiles, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;
        rc = scan_expression_ids(info.achLicense, hIds);
        if (rc != NO_ERROR) {
            StrSetDestroy(hIds);
            return rc;
        }
    }

    /* Phase 2: resolve text for each identifier. */
    if (StrSetEnumFirst(hIds, &hEnum) == NO_ERROR) {
        do {
            CHAR achId[256];
            PSZ pszText = NULL;
            SPDXEXTRACTEDLICENSEINFO info;

            if (StrSetEnumGet(hEnum, achId, sizeof(achId), NULL) != NO_ERROR)
                continue;

            pszText = find_text_in_extra(paExtra, ulExtraCount, achId);
            if (!pszText)
                pszText = find_text_in_licenses_dir(pszProjectDir, achId);
            if (!pszText) {
                StrSetEnumClose(hEnum);
                StrSetDestroy(hIds);
                return ERROR_FILE_NOT_FOUND;
            }

            memset(&info, 0, sizeof(info));
            info.pszLicenseId = strdup(achId);
            info.pszExtractedText = pszText;
            if (!info.pszLicenseId) {
                free(pszText);
                StrSetEnumClose(hEnum);
                StrSetDestroy(hIds);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            rc = SbomAddExtractedLicense(hList, &info);
            if (rc != NO_ERROR) {
                free(info.pszLicenseId);
                free(pszText);
                StrSetEnumClose(hEnum);
                StrSetDestroy(hIds);
                return rc;
            }
        } while (StrSetEnumNext(hEnum) == NO_ERROR);
        StrSetEnumClose(hEnum);
    }

    StrSetDestroy(hIds);
    return NO_ERROR;
}
