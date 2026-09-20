/* spdx_sbom_scan.h - file and snippet collection for SBOM (C89) */
#ifndef SPDX_SBOM_SCAN_H
#define SPDX_SBOM_SCAN_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_scan.h
 * @brief File and snippet collection for the SBOM generator.
 *
 * The module consumes a list of SPDXFILEINPUT entries produced by
 * the CLI after ReuseResolveLicense, computes the SHA-1 and the
 * SPDX file type of each file, validates its license expression,
 * and appends a SPDXFILEINFO entry to the file list. For each file,
 * SPDX snippets are extracted and appended to the snippet list.
 *
 * The module does not depend on the REUSE resolver. Licensing
 * information arrives pre-resolved through SPDXFILEINPUT.
 *
 * Conforms to:
 *   - SPDX 2.3, §8 (File information), §9 (Snippet information).
 *     https://spdx.github.io/spdx-spec/v2.3/
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
 * Iterates over @p hResolved, and for every entry:
 *   1. fills a SPDXFILEINFO entry via SbomFillFileBasic;
 *   2. validates the license expression;
 *   3. appends the entry to @p hFiles;
 *   4. extracts SPDX snippets and appends them to @p hSnippets.
 *
 * The function stops at the first error and returns the
 * corresponding APIRET.
 *
 * @param[in]  hResolved   List of SPDXFILEINPUT entries produced by
 *                         the CLI after ReuseResolveLicense. Not
 *                         NULLHANDLE.
 * @param[in]  hFiles      Destination file list. Not NULLHANDLE.
 *                         Must have been created by
 *                         SbomCreateFileList.
 * @param[in]  hSnippets   Destination snippet list, or NULLHANDLE
 *                         to skip snippet collection.
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
    HVECTOR hSnippets);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_SCAN_H */
