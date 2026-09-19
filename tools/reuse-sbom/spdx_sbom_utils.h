/* spdx_sbom_utils.h - SPDX SBOM helper functions (C89) */
#ifndef SPDX_SBOM_UTILS_H
#define SPDX_SBOM_UTILS_H

#include <stddef.h>
#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_utils.h
 * @brief Helper functions for the SBOM generator.
 *
 * Identifier sanitization, package id construction, file type
 * classification, and lifecycle of the container lists used by
 * the document (files, snippets, relationships, extracted
 * licenses).
 *
 * @par Containers
 * All lists are HVECTOR containers from the ccl library. The
 * element type is fixed at creation time. The Free functions
 * release the container; for lists whose elements own memory, the
 * Free function releases the owned strings first.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * container is shared between threads.
 *
 * Conforms to:
 *   - SPDX 2.3.
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - OS/2 Control Program Interface (naming, types, conventions).
 */

/* ==================================================================
 * Identifier helpers
 * ================================================================== */

/**
 * @brief Remove the file extension from a path, in place.
 *
 * Everything from the last '.' to the end of the string is
 * removed. If the path has no '.', the string is left unchanged.
 *
 * @param[in,out] pszPath  Path to modify. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL.
 */
APIRET APIENTRY SbomRemoveExtension(PSZ pszPath);

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
APIRET APIENTRY SbomSanitizeId(PCSZ pszSrc, PSZ pszDst, ULONG ulDstSize);

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
                                  PSZ pszBuf, ULONG ulBufSize);

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
APIRET APIENTRY SbomQueryFileType(PCSZ pszFilename, PSZ *ppszFileType);

/* ==================================================================
 * File list
 * ================================================================== */

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
APIRET APIENTRY SbomCreateFileList(PHVECTOR phList);

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
APIRET APIENTRY SbomAddFile(HVECTOR hList, const SPDXFILEINFO *pInfo);

/**
 * @brief Release a file list.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeFileList(HVECTOR hList);

/* ==================================================================
 * Snippet list
 * ================================================================== */

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
APIRET APIENTRY SbomCreateSnippetList(PHVECTOR phList);

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
                               const SPDXSNIPPETINFO *pInfo);

/**
 * @brief Release a snippet list.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeSnippetList(HVECTOR hList);

/* ==================================================================
 * Relationship list
 * ================================================================== */

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
APIRET APIENTRY SbomCreateRelationshipList(PHVECTOR phList);

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
                                    const SPDXRELATIONSHIP *pRel);

/**
 * @brief Release a relationship list.
 *
 * @param[in] hList  List. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hList is not recognized.
 */
APIRET APIENTRY SbomFreeRelationshipList(HVECTOR hList);

/* ==================================================================
 * Extracted license list
 * ================================================================== */

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
APIRET APIENTRY SbomCreateExtractedList(PHVECTOR phList);

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
    HVECTOR hList, const SPDXEXTRACTEDLICENSEINFO *pInfo);

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
APIRET APIENTRY SbomFreeExtractedList(HVECTOR hList);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_UTILS_H */
