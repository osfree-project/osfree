/* sha256.h - SHA-256 hashing (C89, OpenWatcom) */
#ifndef SHA256_H
#define SHA256_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file sha256.h
 * @brief SHA-256 hashing.
 *
 * Conforms to:
 *   - FIPS PUB 180-2, "Secure Hash Standard".
 *     https://csrc.nist.gov/publications/detail/fips/180/2/archive
 *   - RFC 6234, "US Secure Hash Algorithms (SHA and SHA-based HMAC
 *     and HKDF)".
 *     https://datatracker.ietf.org/doc/html/rfc6234
 *
 * The module provides one-shot hashing of a file or of a
 * NUL-terminated string. Streaming is not exposed.
 *
 * @par Buffer size
 * A SHA-256 digest in hex representation is 64 characters. The
 * output buffer must be at least 65 bytes (64 + NUL).
 */

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Compute SHA-256 of a file.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written, no
 *     buffer touched.
 *   - ulSize >= 65: hex digest is copied into pszBuf and
 *     NUL-terminated; *pulUsed = 64.
 *   - ulSize < 65: ERROR_BUFFER_OVERFLOW; *pulUsed = 65.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY Sha256File(PCSZ pszPath, PSZ pszBuf, ULONG ulSize,
                           PULONG pulUsed);

/**
 * @brief Compute SHA-256 of a NUL-terminated string.
 *
 * Size-query convention is identical to Sha256File.
 *
 * @param[in]  pszStr   Input string. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszStr is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY Sha256String(PCSZ pszStr, PSZ pszBuf, ULONG ulSize,
                             PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* SHA256_H */
