/* spdx.h - shared SPDX utility functions.
 * Part of the reuse support layer. */
#ifndef SPDX_H
#define SPDX_H

#include "os2types.h"
#include "os2err.h"
#include "strset.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx.h
 * @brief Shared SPDX utility functions.
 *
 * File helpers used by the REUSE resolver and the SBOM generator,
 * text normalization for license-text comparison, and a helper
 * that extracts SPDX identifiers from a license expression.
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

/* ==================================================================
 * File helpers
 * ================================================================== */

/**
 * @brief Read an entire file into a caller-supplied buffer.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *     NUL) is written, no buffer touched.
 *   - ulSize large enough: file content copied and NUL-terminated;
 *     *pulUsed is the number of bytes read, not counting the NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxReadFileAll(PCSZ pszPath, PSZ pszBuf,
                                ULONG ulSize, PULONG pulUsed);

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
 * Removes a leading UTF-8 BOM, converts CRLF and lone CR to LF,
 * removes trailing spaces and tabs on each line, and removes
 * trailing empty lines.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *     NUL) is written.
 *   - ulSize large enough: normalized text copied and
 *     NUL-terminated; *pulUsed is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszSrc   Input text. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxNormalizeText(PCSZ pszSrc, PSZ pszBuf,
                                  ULONG ulSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_H */
