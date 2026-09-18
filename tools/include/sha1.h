/* sha1.h - SHA1 hashing (C89) */
#ifndef SHA1_H
#define SHA1_H

#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Error codes
 * ================================================================== */

/** @def SHA1_NO_ERROR @brief Success. */
#define SHA1_NO_ERROR            0
/** @def SHA1_ERROR_INVALID_PARAM @brief Invalid parameter. */
#define SHA1_ERROR_INVALID_PARAM 1
/** @def SHA1_ERROR_OPEN_FAILED @brief File cannot be opened. */
#define SHA1_ERROR_OPEN_FAILED   2
/** @def SHA1_ERROR_READ_FAILED @brief Read error. */
#define SHA1_ERROR_READ_FAILED   3
/** @def SHA1_ERROR_OUT_OF_MEMORY @brief Memory allocation failure. */
#define SHA1_ERROR_OUT_OF_MEMORY 4

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Compute SHA1 of a file.
 *
 * On success, @p *ppszHex receives a malloc'd lowercase hex string
 * (40 characters + NUL), owned by the caller. On failure, @p *ppszHex
 * is set to NULL.
 *
 * @param[in]  pszPath    Path to the file. Not NULL.
 * @param[out] ppszHex    Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 * @retval SHA1_NO_ERROR            Success.
 * @retval SHA1_ERROR_INVALID_PARAM pszPath or ppszHex is NULL.
 * @retval SHA1_ERROR_OPEN_FAILED   File cannot be opened.
 * @retval SHA1_ERROR_READ_FAILED   Read error.
 * @retval SHA1_ERROR_OUT_OF_MEMORY Memory allocation failure.
 */
APIRET APIENTRY Sha1File(PCSZ pszPath, PSZ *ppszHex);

/**
 * @brief Compute SHA1 of a NUL-terminated string.
 *
 * @param[in]  pszStr  Input string. Not NULL.
 * @param[out] pszHex  Receiver, at least 41 bytes. Not NULL.
 *
 * @return APIRET
 * @retval SHA1_NO_ERROR            Success.
 * @retval SHA1_ERROR_INVALID_PARAM pszStr or pszHex is NULL.
 */
APIRET APIENTRY Sha1String(PCSZ pszStr, PSZ pszHex);

#ifdef __cplusplus
}
#endif

#endif /* SHA1_H */
