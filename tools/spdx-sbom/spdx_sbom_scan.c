/* spdx_sbom_scan.c - file and snippet collection for SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_scan.h"
#include "spdx_sbom_utils.h"
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
 * @brief Validate one SPDX license expression.
 *
 * @param[in] pszLicense  Expression. Not NULL. May be empty.
 *
 * @return APIRET
 * @retval NO_ERROR                 Expression is valid.
 * @retval ERROR_FILE_NOT_FOUND     Expression is empty.
 * @retval ERROR_INVALID_DATA       Expression is invalid.
 */
static APIRET validate_license(PCSZ pszLicense) {
    PCSZ pszBad = NULL;
    APIRET rc;

    if (!pszLicense || pszLicense[0] == '\0') {
        return ERROR_FILE_NOT_FOUND;
    }

    rc = SpdxQueryExpression(pszLicense, &pszBad);
    if (rc != NO_ERROR) return ERROR_INVALID_DATA;

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
 * A file that parses cleanly but contains no snippets is not an
 * error. A file that fails to parse (nested or unmatched snippet
 * delimiters, unreadable file, out of memory) is reported through
 * the return value.
 *
 * @param[in] pszFullPath     Path to the file on disk. Not NULL.
 * @param[in] pszDisplayName  Base name to store. Not NULL.
 * @param[in] hSnippets       Destination snippet list. Not
 *                            NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     A snippet has no license.
 * @retval ERROR_INVALID_DATA       A snippet license is invalid, or
 *                                  the snippet structure is broken.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET collect_file_snippets(PCSZ pszFullPath,
                                    PCSZ pszDisplayName,
                                    HVECTOR hSnippets) {
    SPDXSNIPPETLIST raw;
    APIRET rc;
    ULONG ulIdx;

    rc = SpdxQueryFileSnippets(pszFullPath, &raw);
    if (rc != NO_ERROR) {
        /* Parse error, unreadable file, or OOM: propagate. A file
         * with no snippets at all is NOT an error and is handled
         * below (raw.ulCount == 0). */
        return rc;
    }

    rc = NO_ERROR;
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
 * @retval ERROR_BUFFER_OVERFLOW    File type name does not fit the
 *                                  fixed buffer.
 */
APIRET APIENTRY SbomFillFileBasic(
    PCSZ pszFullPath,
    PCSZ pszDisplayName,
    PSPDXFILEINFO pInfo)
{
    CHAR achHex[41];
    APIRET rc;

    if (!pszFullPath || !pszDisplayName || !pInfo)
        return ERROR_INVALID_PARAMETER;

    memset(pInfo, 0, sizeof(*pInfo));
    copy_field(pInfo->achName, sizeof(pInfo->achName), pszDisplayName);

    rc = Sha1File(pszFullPath, achHex, sizeof(achHex), NULL);
    if (rc != NO_ERROR) return rc;

    copy_field(pInfo->achSha1, sizeof(pInfo->achSha1), achHex);

    rc = SbomQueryFileType(pszDisplayName,
                           pInfo->achFileType,
                           sizeof(pInfo->achFileType),
                           NULL);
    if (rc != NO_ERROR) return rc;

    return NO_ERROR;
}

/**
 * @brief Process one resolved file entry.
 *
 * @param[in] pIn          Resolved file entry. Not NULL.
 * @param[in] hFiles       Destination file list.
 * @param[in] hSnippets    Destination snippet list, or NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     File has no license information.
 * @retval ERROR_INVALID_DATA       License expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET process_one_file(
    const SPDXFILEINPUT *pIn,
    HVECTOR hFiles,
    HVECTOR hSnippets)
{
    SPDXFILEINFO info;
    PCSZ pszDisplayName;
    APIRET rc;

    pszDisplayName = SpdxGetFileName(pIn->achFullPath);

    rc = SbomFillFileBasic(pIn->achFullPath, pszDisplayName, &info);
    if (rc != NO_ERROR) return rc;

    rc = validate_license(pIn->achLicense);
    if (rc != NO_ERROR) return rc;

    copy_field(info.achLicense, sizeof(info.achLicense), pIn->achLicense);
    copy_field(info.achCopyright, sizeof(info.achCopyright),
               pIn->achCopyright);

    rc = SbomAddFile(hFiles, &info);
    if (rc != NO_ERROR) return rc;

    if (hSnippets != NULLHANDLE) {
        rc = collect_file_snippets(pIn->achFullPath, pszDisplayName,
                                   hSnippets);
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}

/**
 * @brief Collect all files and snippets for the SBOM.
 *
 * @param[in] hResolved   List of SPDXFILEINPUT entries. Not
 *                        NULLHANDLE.
 * @param[in] hFiles      Destination file list. Not NULLHANDLE.
 * @param[in] hSnippets   Destination snippet list, or NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hResolved or hFiles is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     hResolved or hFiles is not
 *                                  recognized.
 * @retval ERROR_FILE_NOT_FOUND     A file has no license information.
 * @retval ERROR_INVALID_DATA       A license expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCollectFiles(
    HVECTOR hResolved,
    HVECTOR hFiles,
    HVECTOR hSnippets)
{
    ULONG ulCount = 0;
    ULONG ulIdx;
    APIRET rc;

    if (hResolved == NULLHANDLE || hFiles == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    rc = VectorGetCount(hResolved, &ulCount);
    if (rc != NO_ERROR) return rc;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        SPDXFILEINPUT input;

        if (VectorGetItem(hResolved, ulIdx, &input,
                          (ULONG)sizeof(input), NULL) != NO_ERROR)
            continue;

        rc = process_one_file(&input, hFiles, hSnippets);
        if (rc != NO_ERROR) return rc;
    }

    return NO_ERROR;
}
