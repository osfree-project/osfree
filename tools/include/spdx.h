/* spdx.h - shared SPDX utility functions.
 * Part of the reuse support layer. */
#ifndef SPDX_H
#define SPDX_H

#include "os2types.h"
#include "os2err.h"
#include "strset.h"

/**
 * @file spdx.h
 * @brief Shared SPDX utility functions.
 *
 * @par Contents
 * The module provides:
 *   - file helpers used by the REUSE resolver and the SBOM generator;
 *   - text normalization for license-text comparison;
 *   - a helper that extracts SPDX identifiers from a license
 *     expression.
 *
 * @par History
 * The module formerly held a string-list container as well. That
 * container has moved to the ccl library; callers now use HSTRSET
 * from strset.h directly.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * function is used from multiple threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * File helpers
 * ================================================================== */

/**
 * @brief Read an entire file into a malloc'd buffer.
 *
 * On success, @p *ppszText receives a NUL-terminated buffer owned by
 * the caller and @p *pcbSize receives the number of bytes read (not
 * counting the trailing NUL). On failure, @p *ppszText is NULL.
 *
 * @param[in]  pszPath   Path to the file. Not NULL.
 * @param[out] ppszText  Receiver. Not NULL. Set to NULL on error.
 * @param[out] pcbSize   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or ppszText is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxReadFileAll(PCSZ pszPath, PSZ *ppszText, PLONG pcbSize);

/**
 * @brief Return a pointer to the base name inside a path.
 *
 * The returned pointer refers to data inside @p pszPath; the caller
 * must not free it. Never returns NULL when @p pszPath is not NULL.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return Base name, or NULL if pszPath is NULL.
 */
PCSZ APIENTRY SpdxGetFileName(PCSZ pszPath);

/* ==================================================================
 * SPDX expression helpers
 * ================================================================== */

/**
 * @brief Collect the SPDX identifiers from a license expression.
 *
 * AND / OR / WITH and parentheses are stripped; remaining tokens are
 * added to @p hOut with duplicate removal. Case is preserved.
 *
 * The destination is a string set created by StrSetCreate; the
 * caller remains the owner and is responsible for calling
 * StrSetDestroy afterwards.
 *
 * @param[in] pszExpr  Expression. Not NULL.
 * @param[in] hOut     Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszExpr is NULL.
 * @retval ERROR_INVALID_HANDLE     hOut is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxExpressionCollectIds(PCSZ pszExpr, HSTRSET hOut);

/* ==================================================================
 * Text helpers
 * ================================================================== */

/**
 * @brief Normalize text for comparison.
 *
 *   - remove a leading UTF-8 BOM;
 *   - convert CRLF and lone CR to LF;
 *   - remove trailing spaces and tabs on each line;
 *   - remove trailing empty lines.
 *
 * On success, @p *ppszOut receives a malloc'd string owned by the
 * caller. On failure, @p *ppszOut is NULL.
 *
 * @param[in]  pszSrc    Input text. Not NULL.
 * @param[out] ppszOut   Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc or ppszOut is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxNormalizeText(PCSZ pszSrc, PSZ *ppszOut);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_H */
