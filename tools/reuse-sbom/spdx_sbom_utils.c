/* spdx_sbom_utils.c - SPDX SBOM helper functions (C89) */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_sbom_utils.h"

/**
 * @file spdx_sbom_utils.c
 * @brief Implementation of the SBOM helper functions.
 *
 * No function in this module writes to stdout or stderr. All
 * failures are reported through the returned APIRET code.
 */

/* ------------------------------------------------------------------ */
/* Identifier helpers                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Remove the file extension from a path, in place.
 *
 * @param[in,out] pszPath  Path to modify. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL.
 */
APIRET APIENTRY SbomRemoveExtension(PSZ pszPath) {
    PSZ pszDot;
    if (!pszPath) return ERROR_INVALID_PARAMETER;
    pszDot = strrchr(pszPath, '.');
    if (pszDot) *pszDot = '\0';
    return NO_ERROR;
}

/**
 * @brief Sanitize a string for use as an SPDX identifier fragment.
 *
 * @param[in]  pszSrc     Source string. Not NULL.
 * @param[out] pszDst     Destination buffer. Not NULL.
 * @param[in]  ulDstSize  Size of pszDst in bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc or pszDst is NULL, or
 *                                  ulDstSize is zero.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 */
APIRET APIENTRY SbomSanitizeId(PCSZ pszSrc, PSZ pszDst, ULONG ulDstSize) {
    ULONG ulSrcIdx = 0, ulDstIdx = 0;
    ULONG ulStart, ulEnd;

    if (!pszSrc || !pszDst || ulDstSize == 0)
        return ERROR_INVALID_PARAMETER;

    while (pszSrc[ulSrcIdx] != '\0' && ulDstIdx < ulDstSize - 1) {
        UCHAR uch = (UCHAR)pszSrc[ulSrcIdx];
        if (isalnum(uch) || uch == '.' || uch == '-')
            pszDst[ulDstIdx++] = (CHAR)uch;
        else
            pszDst[ulDstIdx++] = '-';
        ulSrcIdx++;
    }
    pszDst[ulDstIdx] = '\0';

    if (pszSrc[ulSrcIdx] != '\0') return ERROR_BUFFER_OVERFLOW;

    ulStart = 0;
    ulEnd = ulDstIdx;
    while (ulStart < ulEnd && pszDst[ulStart] == '-') ulStart++;
    while (ulEnd > ulStart && pszDst[ulEnd - 1] == '-') ulEnd--;
    if (ulStart > 0 || ulEnd < ulDstIdx) {
        memmove(pszDst, pszDst + ulStart, ulEnd - ulStart);
        pszDst[ulEnd - ulStart] = '\0';
    }
    return NO_ERROR;
}

/**
 * @brief Build an SPDX package identifier.
 *
 * @param[in]  pszBaseName  Base name. Not NULL.
 * @param[in]  pszSuffix    Suffix, or NULL.
 * @param[out] pszBuf       Output buffer. Not NULL.
 * @param[in]  ulBufSize    Size of pszBuf in bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszBaseName or pszBuf is NULL,
 *                                  or ulBufSize is zero.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SbomMakePackageId(PCSZ pszBaseName, PCSZ pszSuffix,
                                  PSZ pszBuf, ULONG ulBufSize) {
    CHAR achSanitized[256];
    APIRET rc;
    LONG lWritten;

    if (!pszBaseName || !pszBuf || ulBufSize == 0)
        return ERROR_INVALID_PARAMETER;

    rc = SbomSanitizeId(pszBaseName, achSanitized, sizeof(achSanitized));
    if (rc != NO_ERROR) return rc;

    if (pszSuffix && pszSuffix[0] != '\0')
        lWritten = (LONG)snprintf(pszBuf, ulBufSize,
                                  "SPDXRef-Package-%s-%s",
                                  achSanitized, pszSuffix);
    else
        lWritten = (LONG)snprintf(pszBuf, ulBufSize,
                                  "SPDXRef-Package-%s", achSanitized);

    if (lWritten < 0 || (ULONG)lWritten >= ulBufSize)
        return ERROR_BUFFER_OVERFLOW;
    return NO_ERROR;
}

/**
 * @brief Query the SPDX file type for a path.
 *
 * @param[in]  pszFilename    Path. Not NULL.
 * @param[out] ppszFileType   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename or ppszFileType is
 *                                  NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomQueryFileType(PCSZ pszFilename, PSZ *ppszFileType) {
    PCSZ pszExt;
    PCSZ pszValue;
    PSZ pszCopy;
    size_t cbLen;

    if (!pszFilename || !ppszFileType) return ERROR_INVALID_PARAMETER;
    *ppszFileType = NULL;

    pszExt = strrchr(pszFilename, '.');
    if (!pszExt) {
        pszValue = "OTHER";
    } else if (strcmp(pszExt, ".c") == 0 || strcmp(pszExt, ".cpp") == 0 ||
               strcmp(pszExt, ".h") == 0 || strcmp(pszExt, ".asm") == 0 ||
               strcmp(pszExt, ".rc") == 0) {
        pszValue = "SOURCE";
    } else if (strcmp(pszExt, ".ico") == 0 || strcmp(pszExt, ".bmp") == 0 ||
               strcmp(pszExt, ".png") == 0 || strcmp(pszExt, ".jpg") == 0) {
        pszValue = "IMAGE";
    } else if (strcmp(pszExt, ".txt") == 0 || strcmp(pszExt, ".md") == 0) {
        pszValue = "TEXT";
    } else if (strcmp(pszExt, ".exe") == 0 || strcmp(pszExt, ".dll") == 0 ||
               strcmp(pszExt, ".sys") == 0 || strcmp(pszExt, ".lib") == 0) {
        pszValue = "BINARY";
    } else {
        pszValue = "OTHER";
    }

    cbLen = strlen(pszValue);
    pszCopy = (PSZ)malloc(cbLen + 1);
    if (!pszCopy) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(pszCopy, pszValue, cbLen + 1);
    *ppszFileType = pszCopy;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* File list                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Create an empty file list.
 *
 * @param[out] phList  Receiver for the HVECTOR. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCreateFileList(PHVECTOR phList) {
    if (!phList) return ERROR_INVALID_PARAMETER;
    return VectorCreate((ULONG)sizeof(SPDXFILEINFO), phList);
}

/**
 * @brief Append a copy of a file entry to the list.
 *
 * @param[in] hList  List. Not NULLHANDLE.
 * @param[in] pInfo  Entry to copy. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList is NULLHANDLE or pInfo is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     hList is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomAddFile(HVECTOR hList, const SPDXFILEINFO *pInfo) {
    if (hList == NULLHANDLE || !pInfo) return ERROR_INVALID_PARAMETER;
    return VectorAdd(hList, (PCVOID)pInfo);
}

/**
 * @brief Release a file list.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeFileList(HVECTOR hList) {
    if (hList == NULLHANDLE) return NO_ERROR;
    return VectorDestroy(hList);
}

/* ------------------------------------------------------------------ */
/* Snippet list                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Create an empty snippet list.
 *
 * @param[out] phList  Receiver for the HVECTOR. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCreateSnippetList(PHVECTOR phList) {
    if (!phList) return ERROR_INVALID_PARAMETER;
    return VectorCreate((ULONG)sizeof(SPDXSNIPPETINFO), phList);
}

/**
 * @brief Append a copy of a snippet entry to the list.
 *
 * @param[in] hList  List. Not NULLHANDLE.
 * @param[in] pInfo  Entry to copy. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList is NULLHANDLE or pInfo is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     hList is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomAddSnippet(HVECTOR hList,
                               const SPDXSNIPPETINFO *pInfo) {
    if (hList == NULLHANDLE || !pInfo) return ERROR_INVALID_PARAMETER;
    return VectorAdd(hList, (PCVOID)pInfo);
}

/**
 * @brief Release a snippet list.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeSnippetList(HVECTOR hList) {
    if (hList == NULLHANDLE) return NO_ERROR;
    return VectorDestroy(hList);
}

/* ------------------------------------------------------------------ */
/* Relationship list                                                   */
/* ------------------------------------------------------------------ */

/**
 * @brief Create an empty relationship list.
 *
 * @param[out] phList  Receiver for the HVECTOR. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCreateRelationshipList(PHVECTOR phList) {
    if (!phList) return ERROR_INVALID_PARAMETER;
    return VectorCreate((ULONG)sizeof(SPDXRELATIONSHIP), phList);
}

/**
 * @brief Append a copy of a relationship to the list.
 *
 * @param[in] hList  List. Not NULLHANDLE.
 * @param[in] pRel   Relationship to copy. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList is NULLHANDLE or pRel is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     hList is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomAddRelationship(HVECTOR hList,
                                    const SPDXRELATIONSHIP *pRel) {
    if (hList == NULLHANDLE || !pRel) return ERROR_INVALID_PARAMETER;
    return VectorAdd(hList, (PCVOID)pRel);
}

/**
 * @brief Release a relationship list.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeRelationshipList(HVECTOR hList) {
    if (hList == NULLHANDLE) return NO_ERROR;
    return VectorDestroy(hList);
}

/* ------------------------------------------------------------------ */
/* Extracted license list                                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Create an empty extracted license list.
 *
 * @param[out] phList  Receiver for the HVECTOR. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCreateExtractedList(PHVECTOR phList) {
    if (!phList) return ERROR_INVALID_PARAMETER;
    return VectorCreate((ULONG)sizeof(SPDXEXTRACTEDLICENSEINFO), phList);
}

/**
 * @brief Append a copy of an extracted license entry to the list.
 *
 * @param[in] hList  List. Not NULLHANDLE.
 * @param[in] pInfo  Entry to copy. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList is NULLHANDLE or pInfo is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     hList is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomAddExtractedLicense(
    HVECTOR hList, const SPDXEXTRACTEDLICENSEINFO *pInfo) {
    if (hList == NULLHANDLE || !pInfo) return ERROR_INVALID_PARAMETER;
    return VectorAdd(hList, (PCVOID)pInfo);
}

/**
 * @brief Release an extracted license list.
 *
 * Frees every owned string in every entry, then releases the
 * container.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeExtractedList(HVECTOR hList) {
    ULONG ulCount = 0;
    ULONG i;
    APIRET rc;

    if (hList == NULLHANDLE) return NO_ERROR;
    if (VectorGetCount(hList, &ulCount) == NO_ERROR) {
        for (i = 0; i < ulCount; i++) {
            SPDXEXTRACTEDLICENSEINFO info;
            if (VectorGetItem(hList, i, &info,
                              (ULONG)sizeof(info), NULL) == NO_ERROR) {
                free(info.pszLicenseId);
                free(info.pszExtractedText);
                free(info.pszName);
                free(info.pszComment);
            }
        }
    }
    rc = VectorDestroy(hList);
    return rc;
}
