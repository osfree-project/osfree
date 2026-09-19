/* sha1.h - SHA-1 hashing (C89, OpenWatcom) */
#ifndef SHA1_H
#define SHA1_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file sha1.h
 * @brief SHA-1 hashing.
 *
 * Conforms to:
 *   - FIPS PUB 180-1, "Secure Hash Standard".
 *     https://csrc.nist.gov/publications/detail/fips/180/1/archive
 *   - RFC 3174, "US Secure Hash Algorithm 1 (SHA1)".
 *     https://datatracker.ietf.org/doc/html/rfc3174
 *
 * The module provides one-shot hashing of a file or of a
 * NUL-terminated string. Streaming is not exposed.
 *
 * @par Buffer size
 * A SHA-1 digest in hex representation is 40 characters. The output
 * buffer must be at least 41 bytes (40 + NUL).
 */

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Compute SHA-1 of a file.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written, no
 *     buffer touched.
 *   - ulSize >= 41: hex digest is copied into pszBuf and
 *     NUL-terminated; *pulUsed = 40.
 *   - ulSize < 41: ERROR_BUFFER_OVERFLOW; *pulUsed = 41.
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
APIRET APIENTRY Sha1File(PCSZ pszPath, PSZ pszBuf, ULONG ulSize,
                         PULONG pulUsed);

/**
 * @brief Compute SHA-1 of a NUL-terminated string.
 *
 * Size-query convention is identical to Sha1File.
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
APIRET APIENTRY Sha1String(PCSZ pszStr, PSZ pszBuf, ULONG ulSize,
                           PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* SHA1_H */
