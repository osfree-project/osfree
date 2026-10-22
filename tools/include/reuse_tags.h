/* reuse_tags.h - REUSE 3.3 file tag reader
 * (C89 + Watcom extensions) */
#ifndef REUSE_TAGS_H
#define REUSE_TAGS_H

/**
 * @file reuse_tags.h
 * @brief Public interface of the REUSE file tag reader.
 *
 * Part of the REUSE library, sub-layer L2.1 (tags).
 * Umbrella header: reuse.h.
 *
 * Declares the tag-value reader for in-file SPDX tags, the snippet
 * reader, and the warning report accumulator.
 */

#include "os2types.h"
#include "os2err.h"
#include "reuseerr.h"
#include "reuse_comments.h"
#include "spdx_tag.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Warning codes for the in-file tag report
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
 * Snippet types
 *
 * SPDX snippet as seen by REUSE consumers: a line range with a
 * license expression and copyright notice. Constructed by
 * ReuseQueryFileSnippets according to the REUSE rules (comment
 * markers removed, REUSE-IgnoreStart/End honoured, multiple license
 * tags joined with AND, multiple copyright lines joined with '\n').
 *
 * The strings pszLicense and pszCopyright are owned by the list and
 * released by ReuseSnippetListFree.
 * ================================================================== */

/**
 * @struct _SPDXSNIPPET
 * @brief One SPDX snippet extracted from a file.
 */
typedef struct _SPDXSNIPPET {
    ULONG ulLineStart;    /**< Line number of SPDX-SnippetBegin. */
    ULONG ulLineEnd;      /**< Line number of SPDX-SnippetEnd.   */
    PSZ   pszLicense;     /**< SPDX-License-Identifier, or NULL. */
    PSZ   pszCopyright;   /**< SPDX-SnippetCopyrightText, lines
                               joined with '\n', or NULL.       */
} SPDXSNIPPET, *PSPDXSNIPPET;

/**
 * @struct _SPDXSNIPPETLIST
 * @brief A list of SPDX snippets collected from one file.
 */
typedef struct _SPDXSNIPPETLIST {
    PSPDXSNIPPET pItems;     /**< Array of snippets.      */
    ULONG        ulCount;    /**< Number of used entries. */
    ULONG        ulCapacity; /**< Allocated capacity.     */
} SPDXSNIPPETLIST, *PSPDXSNIPPETLIST;

/* ==================================================================
 * Snippet list lifecycle
 * ================================================================== */

/**
 * @brief Initialize a snippet list.
 *
 * Sets all fields to zero. No memory is allocated.
 *
 * @param[in] pList  List. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList is NULL.
 */
APIRET APIENTRY ReuseSnippetListInit(PSPDXSNIPPETLIST pList);

/**
 * @brief Release all memory owned by a snippet list.
 *
 * Frees the strings of every snippet, then the backing array, and
 * reinitializes the structure. Passing NULL is a no-op.
 *
 * @param[in] pList  List. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Success. Also for NULL.
 */
APIRET APIENTRY ReuseSnippetListFree(PSPDXSNIPPETLIST pList);

/* ==================================================================
 * Tags report
 * ================================================================== */

/**
 * @typedef HREUSETAGSREPORT
 * @brief Handle to an accumulated in-file tag warnings report.
 */
typedef HANDLE HREUSETAGSREPORT;

/**
 * @typedef PHREUSETAGSREPORT
 * @brief Pointer to a HREUSETAGSREPORT receiver.
 */
typedef HANDLE *PHREUSETAGSREPORT;

/**
 * @brief Create an empty tags report.
 *
 * @param[out] phReport  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseCreateTagsReport(PHREUSETAGSREPORT phReport);

/**
 * @brief Close a tags report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseCloseTagsReport(HREUSETAGSREPORT hReport);

/**
 * @brief Number of records in a tags report.
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
APIRET APIENTRY ReuseQueryTagsReportCount(HREUSETAGSREPORT hReport,
                                          PULONG pulCount);

/**
 * @brief Retrieve one tags report record by index.
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
APIRET APIENTRY ReuseQueryTagsReport(HREUSETAGSREPORT hReport,
                                     ULONG ulIndex,
                                     PREUSEERR pErr);

/* ==================================================================
 * File tag reader
 * ================================================================== */

/**
 * @brief Read the file-level license and copyright of a file.
 *
 * @param[in]  pszPath       Path to the file. Not NULL.
 * @param[in]  hStyle        Comment style, or NULLHANDLE for a
 *                           sidecar file (raw tag-value, no comment
 *                           marker stripping).
 * @param[out] pszLicense    License buffer. Not NULL unless size-query.
 * @param[in]  ulLicSize     Size of pszLicense in bytes.
 * @param[out] pulLicUsed    Optional. Receives the used size.
 * @param[out] pszCopyright  Copyright buffer. Not NULL unless size-query.
 * @param[in]  ulCopSize     Size of pszCopyright in bytes.
 * @param[out] pulCopUsed    Optional. Receives the used size.
 * @param[in]  hReport       Warning report, or NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 At least one tag was found.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
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
 * @brief Read all SPDX snippets from a file.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style. Not NULLHANDLE.
 * @param[in]  hReport  Warning report, or NULLHANDLE.
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
