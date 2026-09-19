/* spdx_sbom_scan.c - file and snippet collection for SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_scan.h"
#include "spdx_sbom_utils.h"
#include "reuse_lic.h"
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "spdx_tag.h"
#include "sha1.h"

/**
 * @file spdx_sbom_scan.c
 * @brief Implementation of the SBOM file and snippet collection.
 */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Copy a NUL-terminated string into a fixed buffer.
 *
 * If @p pszSrc is NULL, the destination is set to an empty string.
 * If the source does not fit, it is truncated.
 *
 * @param[out] pszDst     Destination buffer. Not NULL.
 * @param[in]  ulDstSize  Size of pszDst in bytes. Must be > 0.
 * @param[in]  pszSrc     Source string, or NULL.
 */
static void copy_field(PSZ pszDst, ULONG ulDstSize, PCSZ pszSrc) {
    if (!pszSrc) { pszDst[0] = '\0'; return; }
    strncpy(pszDst, pszSrc, ulDstSize - 1);
    pszDst[ulDstSize - 1] = '\0';
}

/* ------------------------------------------------------------------ */
/* License validation                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Validate the license expression of one file.
 *
 * @param[in] pszFullPath  File path. Not NULL.
 * @param[in] pLic         Resolved license information. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     License is empty.
 * @retval ERROR_INVALID_DATA       Expression is invalid.
 */
static APIRET validate_license(PCSZ pszFullPath,
                               const REUSELICENSEINFO *pLic) {
    PCSZ pszBad = NULL;
    APIRET rc;

    if (pLic->achLicense[0] == '\0') {
        return ERROR_FILE_NOT_FOUND;
    }

    rc = SpdxQueryExpression(pLic->achLicense, &pszBad);
    if (rc != NO_ERROR) return ERROR_INVALID_DATA;

    (void)pszFullPath;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Snippet collection                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Collect all snippets of one file into the snippet list.
 *
 * For each snippet, the function requires an SPDX-License-Identifier
 * and validates it. Snippets without a license are rejected.
 *
 * @param[in] pszFullPath     Path to the file on disk. Not NULL.
 * @param[in] pszDisplayName  Base name to store. Not NULL.
 * @param[in] hSnippets       Destination snippet list. Not
 *                            NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     A snippet has no license.
 * @retval ERROR_INVALID_DATA       A snippet license is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET collect_file_snippets(PCSZ pszFullPath,
                                    PCSZ pszDisplayName,
                                    HVECTOR hSnippets) {
    SPDXSNIPPETLIST raw;
    APIRET rc = NO_ERROR;
    ULONG ulIdx;

    if (SpdxQueryFileSnippets(pszFullPath, &raw) != NO_ERROR)
        return NO_ERROR;

    for (ulIdx = 0; ulIdx < raw.ulCount; ulIdx++) {
        PSPDXSNIPPET pRaw = &raw.pItems[ulIdx];
        SPDXSNIPPETINFO info;
        PCSZ pszBad = NULL;
        APIRET rcVal;

        if (!pRaw->pszLicense || pRaw->pszLicense[0] == '\0') {
            rc = ERROR_FILE_NOT_FOUND;
            break;
        }
        rcVal = SpdxQueryExpression(pRaw->pszLicense, &pszBad);
        if (rcVal != NO_ERROR) {
            rc = ERROR_INVALID_DATA;
            break;
        }

        memset(&info, 0, sizeof(info));
        snprintf(info.achSpdxId, sizeof(info.achSpdxId),
                 "SPDXRef-Snippet-%lu", (unsigned long)ulIdx);
        snprintf(info.achFromFileId, sizeof(info.achFromFileId),
                 "SPDXRef-File-%s", pszDisplayName);
        copy_field(info.achFromFileName, sizeof(info.achFromFileName),
                   pszDisplayName);
        info.ulLineStart = pRaw->ulLineStart;
        info.ulLineEnd   = pRaw->ulLineEnd;
        copy_field(info.achLicense, sizeof(info.achLicense),
                   pRaw->pszLicense);
        if (pRaw->pszCopyright)
            copy_field(info.achCopyright, sizeof(info.achCopyright),
                       pRaw->pszCopyright);

        rc = SbomAddSnippet(hSnippets, &info);
        if (rc != NO_ERROR) break;
    }

    SpdxSnippetListFree(&raw);
    return rc;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Fill the basic fields of a file entry.
 *
 * @param[in]  pszFullPath     Path to the file on disk. Not NULL.
 * @param[in]  pszDisplayName  Base name to store. Not NULL.
 * @param[out] pInfo           Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error while hashing.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomFillFileBasic(
    PCSZ pszFullPath,
    PCSZ pszDisplayName,
    PSPDXFILEINFO pInfo)
{
    CHAR achHex[41];
    PSZ pszFileType = NULL;
    APIRET rc;

    if (!pszFullPath || !pszDisplayName || !pInfo)
        return ERROR_INVALID_PARAMETER;

    memset(pInfo, 0, sizeof(*pInfo));
    copy_field(pInfo->achName, sizeof(pInfo->achName), pszDisplayName);

    rc = Sha1File(pszFullPath, achHex, sizeof(achHex), NULL);
    if (rc != NO_ERROR) return rc;

    copy_field(pInfo->achSha1, sizeof(pInfo->achSha1), achHex);

    rc = SbomQueryFileType(pszDisplayName, &pszFileType);
    if (rc != NO_ERROR) return rc;
    if (pszFileType) {
        copy_field(pInfo->achFileType, sizeof(pInfo->achFileType),
                   pszFileType);
        free(pszFileType);
    }
    return NO_ERROR;
}

/**
 * @brief Process one file: fill, resolve, validate, append.
 *
 * @param[in] pszFullPath           Path to the file on disk.
 * @param[in] pszDisplayName        Base name to store.
 * @param[in] hTree                 REUSE project handle, or
 *                                  NULLHANDLE.
 * @param[in] pszDefaultLicense     Fallback license, or NULL.
 * @param[in] pszDefaultCopyright   Fallback copyright, or NULL.
 * @param[in] hFiles                Destination file list.
 * @param[in] hSnippets             Destination snippet list, or
 *                                  NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     File has no license information.
 * @retval ERROR_INVALID_DATA       License expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET process_one_file(
    PCSZ pszFullPath,
    PCSZ pszDisplayName,
    HREUSETREE hTree,
    PCSZ pszDefaultLicense,
    PCSZ pszDefaultCopyright,
    HVECTOR hFiles,
    HVECTOR hSnippets)
{
    SPDXFILEINFO info;
    REUSELICENSEINFO lic;
    APIRET rc;

    rc = SbomFillFileBasic(pszFullPath, pszDisplayName, &info);
    if (rc != NO_ERROR) return rc;

    rc = ReuseResolveLicense(hTree, pszFullPath,
                             pszDefaultLicense, pszDefaultCopyright,
                             &lic);
    if (rc != NO_ERROR) return ERROR_FILE_NOT_FOUND;

    rc = validate_license(pszFullPath, &lic);
    if (rc != NO_ERROR) return rc;

    copy_field(info.achLicense, sizeof(info.achLicense), lic.achLicense);
    copy_field(info.achCopyright, sizeof(info.achCopyright),
               lic.achCopyright);

    rc = SbomAddFile(hFiles, &info);
    if (rc != NO_ERROR) return rc;

    if (hSnippets != NULLHANDLE) {
        rc = collect_file_snippets(pszFullPath, pszDisplayName, hSnippets);
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}

/**
 * @brief Collect all files and snippets for the SBOM.
 *
 * @param[in] hPaths              Set of file paths. Not NULLHANDLE.
 * @param[in] hTree               REUSE project handle, or
 *                                NULLHANDLE.
 * @param[in] pszDefaultLicense   Fallback license, or NULL.
 * @param[in] pszDefaultCopyright Fallback copyright, or NULL.
 * @param[in] hFiles              Destination file list. Not
 *                                NULLHANDLE.
 * @param[in] hSnippets           Destination snippet list, or
 *                                NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hPaths or hFiles is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     hPaths or hFiles is not
 *                                  recognized.
 * @retval ERROR_FILE_NOT_FOUND     A file has no license information.
 * @retval ERROR_INVALID_DATA       A license expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCollectFiles(
    HSTRSET hPaths,
    HREUSETREE hTree,
    PCSZ pszDefaultLicense,
    PCSZ pszDefaultCopyright,
    HVECTOR hFiles,
    HVECTOR hSnippets)
{
    HSTRSETENUM hEnum = NULLHANDLE;
    APIRET rc = NO_ERROR;

    if (hPaths == NULLHANDLE || hFiles == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    if (StrSetEnumFirst(hPaths, &hEnum) != NO_ERROR)
        return NO_ERROR;

    do {
        CHAR achFull[1024];
        PCSZ pszName;

        if (StrSetEnumGet(hEnum, achFull, sizeof(achFull), NULL)
                != NO_ERROR)
            continue;

        pszName = SpdxGetFileName(achFull);
        rc = process_one_file(achFull, pszName, hTree,
                              pszDefaultLicense, pszDefaultCopyright,
                              hFiles, hSnippets);
        if (rc != NO_ERROR) break;
    } while (StrSetEnumNext(hEnum) == NO_ERROR);

    StrSetEnumClose(hEnum);
    return rc;
}
