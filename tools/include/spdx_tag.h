/* spdx_tag.h - SPDX tags and snippet parser (C89) */
#ifndef SPDX_TAG_H
#define SPDX_TAG_H

#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Error codes
 * ================================================================== */

/** @def SPDX_TAG_NO_ERROR @brief Success. */
#define SPDX_TAG_NO_ERROR            0
/** @def SPDX_TAG_ERROR_INVALID_PARAM @brief Invalid parameter. */
#define SPDX_TAG_ERROR_INVALID_PARAM 1
/** @def SPDX_TAG_ERROR_OPEN_FAILED @brief File cannot be opened. */
#define SPDX_TAG_ERROR_OPEN_FAILED   2
/** @def SPDX_TAG_ERROR_READ_FAILED @brief Read error. */
#define SPDX_TAG_ERROR_READ_FAILED   3
/** @def SPDX_TAG_ERROR_OUT_OF_MEMORY @brief Memory allocation failure. */
#define SPDX_TAG_ERROR_OUT_OF_MEMORY 4
/** @def SPDX_TAG_ERROR_NOT_FOUND @brief Requested value not found. */
#define SPDX_TAG_ERROR_NOT_FOUND     5
/** @def SPDX_TAG_ERROR_SYNTAX @brief Snippet structure is invalid. */
#define SPDX_TAG_ERROR_SYNTAX        6

/* ==================================================================
 * Types
 * ================================================================== */

/**
 * @struct _SPDXSNIPPET
 * @brief One SPDX snippet extracted from a file.
 *
 * The @c pszLicense and @c pszCopyright strings are malloc'd and
 * owned by the containing list. They are released by
 * SpdxSnippetListFree.
 */
typedef struct _SPDXSNIPPET {
    int   nLineStart;   /**< Line number of SPDX-SnippetBegin. */
    int   nLineEnd;     /**< Line number of SPDX-SnippetEnd.   */
    char *pszLicense;   /**< SPDX-License-Identifier, or NULL. */
    char *pszCopyright; /**< SPDX-SnippetCopyrightText, lines
                             joined with '\n', or NULL.      */
} SPDXSNIPPET, *PSPDXSNIPPET;

/**
 * @struct _SPDXSNIPPETLIST
 * @brief A list of SPDX snippets collected from one file.
 */
typedef struct _SPDXSNIPPETLIST {
    PSPDXSNIPPET paItems;    /**< Array of snippets.         */
    int          nCount;     /**< Number of used entries.    */
    int          nCapacity;  /**< Allocated capacity.        */
} SPDXSNIPPETLIST, *PSPDXSNIPPETLIST;

/* ==================================================================
 * List lifecycle
 * ================================================================== */

/**
 * @brief Initialize a snippet list.
 *
 * Sets all fields to zero. No memory is allocated.
 *
 * @param[in] pList  List. Not NULL.
 */
void APIENTRY SpdxSnippetListInit(SPDXSNIPPETLIST *pList);

/**
 * @brief Release all memory owned by a snippet list.
 *
 * Frees the strings of every snippet, then the backing array, and
 * reinitializes the structure. Passing NULL is a no-op.
 *
 * @param[in] pList  List. May be NULL.
 */
void APIENTRY SpdxSnippetListFree(SPDXSNIPPETLIST *pList);

/* ==================================================================
 * File-level queries
 * ================================================================== */

/**
 * @brief Check whether a file contains an SPDX-License-Identifier tag
 *        outside of any REUSE-IgnoreStart/End block.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pfHasTag     Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval SPDX_TAG_NO_ERROR            Success.
 * @retval SPDX_TAG_ERROR_INVALID_PARAM pszFilename or pfHasTag is NULL.
 * @retval SPDX_TAG_ERROR_OPEN_FAILED   File cannot be opened.
 */
APIRET APIENTRY SpdxFileHasTag(PCSZ pszFilename, PBOOL pfHasTag);

/**
 * @brief Retrieve the value of the SPDX-License-Identifier tag.
 *
 * On success, @p *ppszLicense receives a malloc'd string owned by the
 * caller and must be freed with @c free. On failure, @p *ppszLicense
 * is set to NULL.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] ppszLicense  Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 * @retval SPDX_TAG_NO_ERROR            Success.
 * @retval SPDX_TAG_ERROR_INVALID_PARAM pszFilename or ppszLicense
 *                                      is NULL.
 * @retval SPDX_TAG_ERROR_OPEN_FAILED   File cannot be opened.
 * @retval SPDX_TAG_ERROR_OUT_OF_MEMORY Memory allocation failure.
 * @retval SPDX_TAG_ERROR_NOT_FOUND     Tag not present.
 */
APIRET APIENTRY SpdxFileGetLicense(PCSZ pszFilename, PSZ *ppszLicense);

/**
 * @brief Retrieve the concatenated SPDX-FileCopyrightText values.
 *
 * Multiple notices are joined with '\n'. On success, @p
 * *ppszCopyright receives a malloc'd string owned by the caller and
 * must be freed with @c free. On failure, @p *ppszCopyright is set
 * to NULL.
 *
 * @param[in]  pszFilename    Path to the file. Not NULL.
 * @param[out] ppszCopyright  Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 * @retval SPDX_TAG_NO_ERROR            Success.
 * @retval SPDX_TAG_ERROR_INVALID_PARAM pszFilename or ppszCopyright
 *                                      is NULL.
 * @retval SPDX_TAG_ERROR_OPEN_FAILED   File cannot be opened.
 * @retval SPDX_TAG_ERROR_OUT_OF_MEMORY Memory allocation failure.
 * @retval SPDX_TAG_ERROR_NOT_FOUND     No copyright tag present.
 */
APIRET APIENTRY SpdxFileGetCopyright(PCSZ pszFilename, PSZ *ppszCopyright);

/**
 * @brief Extract all SPDX snippets from a file.
 *
 * On success, @p pOut is initialized and populated with all snippets
 * found between matching SPDX-SnippetBegin and SPDX-SnippetEnd tags.
 * The caller must release it with SpdxSnippetListFree.
 *
 * On any failure, @p pOut is left initialized (empty) and the caller
 * may safely call SpdxSnippetListFree.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pOut         List receiver. Not NULL.
 *
 * @return APIRET
 * @retval SPDX_TAG_NO_ERROR            Success (possibly no snippets).
 * @retval SPDX_TAG_ERROR_INVALID_PARAM pszFilename or pOut is NULL.
 * @retval SPDX_TAG_ERROR_OPEN_FAILED   File cannot be opened.
 * @retval SPDX_TAG_ERROR_OUT_OF_MEMORY Memory allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX        Nested or unmatched snippet
 *                                      delimiters.
 */
APIRET APIENTRY SpdxFileGetSnippets(PCSZ pszFilename,
                                    SPDXSNIPPETLIST *pOut);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_TAG_H */
