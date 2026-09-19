/* spdx_tag.h - SPDX tags and snippet parser (C89) */
#ifndef SPDX_TAG_H
#define SPDX_TAG_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_tag.h
 * @brief SPDX tags and snippet parser.
 *
 * Reads SPDX-License-Identifier, SPDX-FileCopyrightText and
 * SPDX-Snippet* tags from a file. Tags inside REUSE-IgnoreStart /
 * REUSE-IgnoreEnd regions are ignored.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - SPDX 2.3, §8 (file information), §9 (snippet information).
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

/* ==================================================================
 * Return codes
 * ================================================================== */

/** @def SPDX_TAG_ERROR_SYNTAX @brief Snippet structure is invalid.
 *  User range 0xFF01. */
#define SPDX_TAG_ERROR_SYNTAX        0xFF01

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
 * List lifecycle
 * ================================================================== */

/**
 * @brief Initialize a snippet list.
 *
 * Sets all fields to zero. No memory is allocated.
 *
 * @param[in] pList  List. Not NULL.
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList is NULL.
 */
APIRET APIENTRY SpdxSnippetListInit(SPDXSNIPPETLIST *pList);

/**
 * @brief Release all memory owned by a snippet list.
 *
 * Frees the strings of every snippet, then the backing array, and
 * reinitializes the structure. Passing NULL is a no-op.
 *
 * @param[in] pList  List. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULL.
 */
APIRET APIENTRY SpdxSnippetListFree(SPDXSNIPPETLIST *pList);

/* ==================================================================
 * File-level queries
 * ================================================================== */

/**
 * @brief Query whether a file contains an SPDX-License-Identifier
 *        tag outside any REUSE-IgnoreStart/End block.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pfHasTag     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename or pfHasTag is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
APIRET APIENTRY SpdxQueryFileHasTag(PCSZ pszFilename, PBOOL pfHasTag);

/**
 * @brief Query the value of the SPDX-License-Identifier tag.
 *
 * On success, @p *ppszLicense receives a malloc'd string owned by the
 * caller and must be freed with @c free. On failure, @p *ppszLicense
 * is set to NULL.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     Tag not present.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryFileLicense(PCSZ pszFilename, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed);

/**
 * @brief Query the concatenated SPDX-FileCopyrightText values.
 *
 * Multiple notices are joined with '\n'. On success, @p
 * *ppszCopyright receives a malloc'd string owned by the caller and
 * must be freed with @c free. On failure, @p *ppszCopyright is set
 * to NULL.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No copyright tag present.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryFileCopyright(PCSZ pszFilename, PSZ pszBuf,
                                       ULONG ulSize, PULONG pulUsed);

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
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_INVALID_PARAMETER  pszFilename or pOut is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Nested or unmatched snippet
 *                                  delimiters.
 */
APIRET APIENTRY SpdxQueryFileSnippets(PCSZ pszFilename,
                                      SPDXSNIPPETLIST *pOut);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_TAG_H */
