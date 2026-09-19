/* spdx_sbom_scan.h - file and snippet collection for SBOM (C89) */
#ifndef SPDX_SBOM_SCAN_H
#define SPDX_SBOM_SCAN_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "spdx_sbom_types.h"
#include <reuse.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_scan.h
 * @brief File and snippet collection for the SBOM generator.
 *
 * Walks a set of file paths, resolves the license and copyright of
 * each file through the REUSE resolver, computes the SHA-1 and the
 * SPDX file type, and appends a SPDXFILEINFO entry to the file
 * list. For each file, SPDX snippets are extracted and appended to
 * the snippet list.
 *
 * @par Architecture
 * This module currently calls into the REUSE resolver directly.
 * Unlinking the SPDX layer from the REUSE layer is planned as a
 * separate change; see the architectural analysis.
 *
 * Conforms to:
 *   - SPDX 2.3, §8 (File information), §9 (Snippet information).
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - OS/2 Control Program Interface (naming, types, conventions).
 */

/* ==================================================================
 * File entry helpers
 * ================================================================== */

/**
 * @brief Fill the basic fields of a file entry.
 *
 * Computes the SHA-1 of the file at @p pszFullPath, stores the base
 * name @p pszDisplayName, and derives the SPDX file type from the
 * extension.
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
    PSPDXFILEINFO pInfo);

/* ==================================================================
 * Collection
 * ================================================================== */

/**
 * @brief Collect all files and snippets for the SBOM.
 *
 * Iterates over @p hPaths, and for every path:
 *   1. fills a SPDXFILEINFO entry via SbomFillFileBasic;
 *   2. resolves license and copyright through the REUSE resolver;
 *   3. validates the license expression;
 *   4. appends the entry to @p hFiles;
 *   5. extracts SPDX snippets and appends them to @p hSnippets.
 *
 * The function stops at the first error and returns the
 * corresponding APIRET.
 *
 * @param[in]  hPaths              Set of file paths (HSTRSET).
 *                                 Not NULLHANDLE.
 * @param[in]  hTree               REUSE project handle. May be
 *                                 NULLHANDLE; in that case only
 *                                 sidecar and in-file tags are used.
 * @param[in]  pszDefaultLicense   Fallback license, or NULL.
 * @param[in]  pszDefaultCopyright Fallback copyright, or NULL.
 * @param[in]  hFiles              Destination file list. Not
 *                                 NULLHANDLE. Must have been
 *                                 created by SbomCreateFileList.
 * @param[in]  hSnippets           Destination snippet list, or
 *                                 NULLHANDLE to skip snippet
 *                                 collection.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hPaths or hFiles is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     hPaths or hFiles is not
 *                                  recognized.
 * @retval ERROR_FILE_NOT_FOUND     A file has no license information
 *                                  or its SPDX identifier is unknown.
 * @retval ERROR_INVALID_DATA       A license expression is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCollectFiles(
    HSTRSET hPaths,
    HREUSETREE hTree,
    PCSZ pszDefaultLicense,
    PCSZ pszDefaultCopyright,
    HVECTOR hFiles,
    HVECTOR hSnippets);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_SCAN_H */
