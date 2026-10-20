/* reuse_tags.h - REUSE 3.3 file tag reader (C89) */
#ifndef REUSE_TAGS_H
#define REUSE_TAGS_H

#include "os2types.h"
#include "os2err.h"
#include "reuse_comments.h"
#include "reuse_licenses.h"     /* REUSEERR, REUSE_SEV_* */
#include "spdx_tag.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_tags.h
 * @brief REUSE 3.3 file tag reader.
 *
 * Reads SPDX-License-Identifier, SPDX-FileCopyrightText,
 * SPDX-Snippet* and the REUSE-specific Copyright / © copyright
 * prefixes from the comment header of a source file.
 *
 * Comment prefix stripping is delegated to the comment style
 * (ReuseQueryCommentBody). Tag parsing is delegated to the SPDX 2.3
 * tag-value parser (SpdxQueryTagLine). This module adds only what is
 * specific to REUSE 3.3:
 *
 *   - Accepts only Annex E / Annex H / Annex H.3 tag forms
 *     (SPDX-<Field>:), and reports Clause 8/9 forms
 *     (<Field>:) that must not appear in source files per REUSE 3.3.
 *   - Recognizes the REUSE-specific Copyright and © copyright
 *     prefixes (REUSE 3.3 section 3.2).
 *   - Handles REUSE-IgnoreStart / REUSE-IgnoreEnd blocks.
 *   - Separates file-level copyright, file-level license, snippet
 *     copyright and snippet license.
 *   - Joins multiple SPDX-License-Identifier values with " AND ".
 *   - Joins multiple copyright notices with a newline.
 *
 * Warnings collected during a scan are appended to an
 * HREUSETAGSREPORT instead of being written to stderr. The caller
 * decides whether to print them, store them, or drop them.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - SPDX 2.3, Annex E and Annex H.
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

/* ==================================================================
 * Report codes
 * ================================================================== */

/** @def REUSE_TAGS_DEPRECATED @brief A deprecated SPDX tag was found
 *  in the file. Severity: REUSE_SEV_WARNING. */
#define REUSE_TAGS_DEPRECATED       0xFF10

/** @def REUSE_TAGS_NON_ANNEX_FORM @brief A Clause 8/9 canonical tag
 *  form was used in a source file where only its Annex H spelling is
 *  valid per REUSE 3.3. Severity: REUSE_SEV_WARNING. */
#define REUSE_TAGS_NON_ANNEX_FORM   0xFF11

/** @def REUSE_TAGS_LONG_LINE @brief A tag value exceeded the internal
 *  buffer and was skipped. Severity: REUSE_SEV_WARNING. */
#define REUSE_TAGS_LONG_LINE        0xFF12

/* ==================================================================
 * Report handle
 * ================================================================== */

/**
 * @brief Opaque report handle.
 *
 * Records are REUSEERR entries (shared with reuse_licenses.h). The
 * caller iterates them with ReuseTagsReportGetCount and
 * ReuseTagsReportGet.
 */
typedef HANDLE HREUSETAGSREPORT;

/**
 * @brief Pointer to a report handle.
 *
 * Convenience typedef for the output parameter of
 * ReuseTagsReportCreate.
 */
typedef HANDLE *PHREUSETAGSREPORT;

/**
 * @brief Create an empty report.
 *
 * @param[out] phReport  Receiver. Not NULL. Set to NULLHANDLE on
 *                       error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseTagsReportCreate(PHREUSETAGSREPORT phReport);

/**
 * @brief Release a report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseTagsReportFree(HREUSETAGSREPORT hReport);

/**
 * @brief Number of records in a report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseTagsReportGetCount(HREUSETAGSREPORT hReport,
                                        PULONG pulCount);

/**
 * @brief Retrieve one record by index.
 *
 * @param[in]  hReport  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pErr is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 */
APIRET APIENTRY ReuseTagsReportGet(HREUSETAGSREPORT hReport,
                                   ULONG ulIndex,
                                   PREUSEERR pErr);

/* ==================================================================
 * File-level queries
 * ================================================================== */

/**
 * @brief Read the file-level license and copyright of a file.
 *
 * The function reads the comment header of @p pszPath using the
 * comment style @p hStyle, extracts every recognized SPDX tag and
 * every REUSE-specific Copyright / © notice, and:
 *
 *   - joins multiple SPDX-License-Identifier values with
 *     " AND " into @p pszLicense;
 *   - joins multiple copyright notices with a newline into
 *     @p pszCopyright.
 *
 * Only file-level tags are collected. Tags inside an
 * SPDX-SnippetBegin / SPDX-SnippetEnd block are ignored by this
 * function; use ReuseQueryFileSnippets to read them.
 *
 * Soft warnings (deprecated tags, non-Annex forms, truncated values)
 * are appended to @p hReport when it is not NULLHANDLE. Hard errors
 * (unbalanced snippets, I/O failures) are returned as the function
 * result.
 *
 * Size-query convention:
 *   - pszLicense == NULL, ulLicSize == 0, pszCopyright == NULL,
 *     ulCopSize == 0: only *pulLicUsed and *pulCopUsed are written.
 *   - Otherwise both buffers must be non-NULL. Each receives a
 *     NUL-terminated value or an empty string.
 *   - If either buffer is too small: ERROR_BUFFER_OVERFLOW;
 *     *pulLicUsed and *pulCopUsed receive the required size
 *     including the NUL.
 *
 * @param[in]  pszPath       Path to the file. Not NULL.
 * @param[in]  hStyle        Comment style. Not NULLHANDLE.
 * @param[out] pszLicense    License buffer. Not NULL unless size-query.
 * @param[in]  ulLicSize     Size of pszLicense in bytes.
 * @param[out] pulLicUsed    Optional. Receives the used size.
 * @param[out] pszCopyright  Copyright buffer. Not NULL unless size-query.
 * @param[in]  ulCopSize     Size of pszCopyright in bytes.
 * @param[out] pulCopUsed    Optional. Receives the used size.
 * @param[in]  hReport       Warning report, or NULLHANDLE to discard.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 At least one tag was found.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hStyle or hReport is not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No licensing information found.
 * @retval ERROR_BUFFER_OVERFLOW    A buffer is too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Unmatched snippet delimiters.
 */
APIRET APIENTRY ReuseQueryFileTags(PCSZ pszPath,
                                   HREUSECOMMENTSTYLE hStyle,
                                   PSZ pszLicense, ULONG ulLicSize,
                                   PULONG pulLicUsed,
                                   PSZ pszCopyright, ULONG ulCopSize,
                                   PULONG pulCopUsed,
                                   HREUSETAGSREPORT hReport);

/**
 * @brief Query whether a file has any SPDX-License-Identifier tag
 *        outside a REUSE-IgnoreStart/End block.
 *
 * Faster than ReuseQueryFileTags: stops at the first matching tag.
 * Emits no warnings.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style. Not NULLHANDLE.
 * @param[out] pfHas    Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  pszPath or pfHas is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
APIRET APIENTRY ReuseQueryFileHasTag(PCSZ pszPath,
                                     HREUSECOMMENTSTYLE hStyle,
                                     PBOOL pfHas);

/**
 * @brief Read all SPDX snippets from a file.
 *
 * Each snippet is delimited by SPDX-SnippetBegin / SPDX-SnippetEnd.
 * Within a snippet, SPDX-License-Identifier values are joined with
 * " AND " and SPDX-SnippetCopyrightText values (and REUSE-specific
 * Copyright / © notices) are joined with a newline.
 *
 * Per SPDX 2.3 Annex H.3, snippets may nest. This implementation
 * rejects nesting with SPDX_TAG_ERROR_SYNTAX; callers that need
 * nested snippets should extend the function.
 *
 * Soft warnings (deprecated tags, non-Annex forms, truncated values)
 * are appended to @p hReport when it is not NULLHANDLE.
 *
 * On success, @p pOut is initialized and populated. The caller must
 * release it with SpdxSnippetListFree. On failure, @p pOut is left
 * initialized (empty) and can be passed to SpdxSnippetListFree
 * safely.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style. Not NULLHANDLE.
 * @param[in]  hReport  Warning report, or NULLHANDLE to discard.
 * @param[out] pOut     List receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_INVALID_PARAMETER  pszPath or pOut is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle or hReport is not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Unmatched or nested snippet
 *                                  delimiters.
 */
APIRET APIENTRY ReuseQueryFileSnippets(PCSZ pszPath,
                                       HREUSECOMMENTSTYLE hStyle,
                                       HREUSETAGSREPORT hReport,
                                       PSPDXSNIPPETLIST pOut);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_TAGS_H */
