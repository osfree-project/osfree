/* spdx_sbom_extracted.h - LicenseRef-* text collection (C89) */
#ifndef SPDX_SBOM_EXTRACTED_H
#define SPDX_SBOM_EXTRACTED_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_extracted.h
 * @brief Collection of LicenseRef-* license texts.
 *
 * Scans the license expressions of all files for LicenseRef-*
 * identifiers and resolves the text of each one from either the
 * sources given on the command line or from
 * <project_dir>/LICENSES/<id>.txt.
 *
 * The resulting entries are appended to an extracted license list
 * created by SbomCreateExtractedList. The string fields of every
 * appended entry are malloc'd and are released by
 * SbomFreeExtractedList.
 *
 * Conforms to:
 *   - SPDX 2.3, §10 (Other Licensing Information Detected).
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - OS/2 Control Program Interface (naming, types, conventions).
 */

/* ==================================================================
 * LicenseRef text collection
 * ================================================================== */

/**
 * @brief Collect LicenseRef-* texts for the whole file list.
 *
 * The function performs two phases:
 *
 *   1. Every file's license expression is scanned for LicenseRef-*
 *      identifiers. Duplicates are removed.
 *   2. For each unique identifier, the text is read from the first
 *      matching source:
 *        - @p paExtra (command-line sources), by identifier;
 *        - <project_dir>/LICENSES/<id>.txt;
 *        - <project_dir>/LICENSES/<id>.
 *      Directories are tried with both '/' and '\\' separators.
 *
 * Every resolved identifier is appended to @p hList as a new
 * SPDXEXTRACTEDLICENSEINFO entry whose @c pszLicenseId and
 * @c pszExtractedText fields are set.
 *
 * @param[in] hList          Destination list. Not NULLHANDLE. Must
 *                           have been created by
 *                           SbomCreateExtractedList.
 * @param[in] hFiles         File list to scan. Not NULLHANDLE.
 *                           Holds SPDXFILEINFO entries.
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
    ULONG ulExtraCount);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_EXTRACTED_H */
