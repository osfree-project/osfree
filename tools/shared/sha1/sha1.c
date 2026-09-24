/*!
 *
 * @file sha1.c
 *
 * @brief Implementation of SHA-1.
 *
 * SHA-1 implementation (C89, OpenWatcom). Conforms to:
 *   - FIPS PUB 180-1.
 *   - RFC 3174.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1.h"

/* ==================================================================
 * Internal constants and types
 * ================================================================== */

/*!
 * @def SHA1_BLOCK_SIZE
 * @brief SHA-1 block size in bytes.
 */
#define SHA1_BLOCK_SIZE  64

/*!
 * @def SHA1_DIGEST_SIZE
 * @brief SHA-1 digest size in bytes.
 */
#define SHA1_DIGEST_SIZE 20

/*!
 * @def SHA1_HEX_SIZE
 * @brief Hex representation length including NUL.
 */
#define SHA1_HEX_SIZE    (SHA1_DIGEST_SIZE * 2 + 1)

/*!
 * @struct _SHA1CONTEXT
 * @brief Working state of the SHA-1 algorithm.
 *
 * The message length is tracked as two 32-bit counters: count[0]
 * (low 32 bits of bit count) and count[1] (high 32 bits of bit
 * count). This matches the 64-bit length field required by
 * FIPS 180-1.
 */
typedef struct _SHA1CONTEXT {
    ULONG aulState[5];                  /*!< h0..h4.               */
    ULONG aulCount[2];                  /*!< Bit count, lo/hi.     */
    UCHAR auchBuffer[SHA1_BLOCK_SIZE];  /*!< Partial block buffer. */
} SHA1CONTEXT;

/* ==================================================================
 * Internal bit helpers
 * ================================================================== */

/*!
 * @brief 32-bit left rotation.
 *
 * @param[in] ulVal  Value.
 * @param[in] nBits  Rotation amount (0..31).
 *
 * @return Rotated value.
 */
static ULONG sha1_rotl(ULONG ulVal, ULONG nBits) {
    if (nBits == 0) return ulVal;
    if (nBits >= 32) return ulVal;
    return (ulVal << nBits) | (ulVal >> (32 - nBits));
}

/* ==================================================================
 * Core algorithm
 * ================================================================== */

/*!
 * @brief Compression function: process one 64-byte block.
 *
 * Applies the SHA-1 compression function to @p auchBuffer and
 * updates the five working variables in @p aulState.
 *
 * @param[in,out] aulState    Current state. Not NULL.
 * @param[in]     auchBuffer  64-byte block. Not NULL.
 */
static void sha1_transform(ULONG aulState[5],
                           const UCHAR auchBuffer[SHA1_BLOCK_SIZE]) {
    ULONG ulA, ulB, ulC, ulD, ulE;
    ULONG aulW[80];
    int nIdx;

    /* Prepare the message schedule. */
    for (nIdx = 0; nIdx < 16; nIdx++) {
        aulW[nIdx] = ((ULONG)auchBuffer[nIdx*4]     << 24) |
                     ((ULONG)auchBuffer[nIdx*4 + 1] << 16) |
                     ((ULONG)auchBuffer[nIdx*4 + 2] << 8)  |
                     ((ULONG)auchBuffer[nIdx*4 + 3]);
    }
    for (nIdx = 16; nIdx < 80; nIdx++) {
        ULONG ulTmp = aulW[nIdx-3] ^ aulW[nIdx-8] ^
                      aulW[nIdx-14] ^ aulW[nIdx-16];
        aulW[nIdx] = sha1_rotl(ulTmp, 1);
    }

    ulA = aulState[0];
    ulB = aulState[1];
    ulC = aulState[2];
    ulD = aulState[3];
    ulE = aulState[4];

    for (nIdx = 0; nIdx < 80; nIdx++) {
        ULONG ulF, ulK, ulTemp;
        if (nIdx < 20) {
            ulF = (ulB & ulC) | ((~ulB) & ulD);
            ulK = 0x5A827999u;
        } else if (nIdx < 40) {
            ulF = ulB ^ ulC ^ ulD;
            ulK = 0x6ED9EBA1u;
        } else if (nIdx < 60) {
            ulF = (ulB & ulC) | (ulB & ulD) | (ulC & ulD);
            ulK = 0x8F1BBCDCu;
        } else {
            ulF = ulB ^ ulC ^ ulD;
            ulK = 0xCA62C1D6u;
        }
        ulTemp = sha1_rotl(ulA, 5) + ulF + ulE + ulK + aulW[nIdx];
        ulE = ulD;
        ulD = ulC;
        ulC = sha1_rotl(ulB, 30);
        ulB = ulA;
        ulA = ulTemp;
    }

    aulState[0] += ulA;
    aulState[1] += ulB;
    aulState[2] += ulC;
    aulState[3] += ulD;
    aulState[4] += ulE;
}

/*!
 * @brief Initialize the working state with FIPS 180-1 initial values.
 *
 * @param[out] pCtx  Context to initialize. Not NULL.
 */
static void sha1_init(SHA1CONTEXT *pCtx) {
    pCtx->aulState[0] = 0x67452301u;
    pCtx->aulState[1] = 0xEFCDAB89u;
    pCtx->aulState[2] = 0x98BADCFEu;
    pCtx->aulState[3] = 0x10325476u;
    pCtx->aulState[4] = 0xC3D2E1F0u;
    pCtx->aulCount[0] = 0;
    pCtx->aulCount[1] = 0;
}

/*!
 * @brief Absorb more input data into the context.
 *
 * Buffers partial blocks; full blocks are passed to the compression
 * function immediately.
 *
 * @param[in,out] pCtx      Context. Not NULL.
 * @param[in]     puchData  Input data. Not NULL.
 * @param[in]     cbLen     Number of bytes to absorb.
 */
static void sha1_update(SHA1CONTEXT *pCtx,
                        const UCHAR *puchData, size_t cbLen) {
    size_t i, cbIndex, cbPartLen;
    ULONG ulBitsLo;
    ULONG ulBitsHi;
    ULONG ulPrev;

    /* Compute bit count as (len * 8), split into two 32-bit words.
     * For len < 2^29 (always true for real read blocks) the result
     * is correct on any platform. */
    ulBitsLo = (ULONG)cbLen << 3;
    ulBitsHi = (ULONG)(cbLen >> 29);

    cbIndex = (pCtx->aulCount[0] >> 3) & 0x3F;
    ulPrev = pCtx->aulCount[0];
    pCtx->aulCount[0] += ulBitsLo;
    if (pCtx->aulCount[0] < ulPrev) pCtx->aulCount[1]++;
    pCtx->aulCount[1] += ulBitsHi;

    cbPartLen = SHA1_BLOCK_SIZE - cbIndex;
    if (cbLen >= cbPartLen) {
        memcpy(&pCtx->auchBuffer[cbIndex], puchData, cbPartLen);
        sha1_transform(pCtx->aulState, pCtx->auchBuffer);
        for (i = cbPartLen; i + 63 < cbLen; i += SHA1_BLOCK_SIZE)
            sha1_transform(pCtx->aulState, &puchData[i]);
        cbIndex = 0;
    } else {
        i = 0;
    }
    memcpy(&pCtx->auchBuffer[cbIndex], &puchData[i], cbLen - i);
}

/*!
 * @brief Finish the hash and produce the digest.
 *
 * Applies the padding scheme from FIPS 180-1 §4: a single 0x80
 * byte, zero bytes, and the 64-bit message length in bits as an
 * 8-byte big-endian value.
 *
 * @param[in,out] pCtx        Context. Not NULL.
 * @param[out]    puchDigest  20-byte output. Not NULL.
 */
static void sha1_final(SHA1CONTEXT *pCtx,
                       UCHAR puchDigest[SHA1_DIGEST_SIZE]) {
    UCHAR auchBits[8];
    ULONG ulIndex, ulPadLen;
    static const UCHAR auchPadding[SHA1_BLOCK_SIZE] = {0x80};

    auchBits[0] = (UCHAR)(pCtx->aulCount[1] >> 24);
    auchBits[1] = (UCHAR)(pCtx->aulCount[1] >> 16);
    auchBits[2] = (UCHAR)(pCtx->aulCount[1] >> 8);
    auchBits[3] = (UCHAR)(pCtx->aulCount[1]);
    auchBits[4] = (UCHAR)(pCtx->aulCount[0] >> 24);
    auchBits[5] = (UCHAR)(pCtx->aulCount[0] >> 16);
    auchBits[6] = (UCHAR)(pCtx->aulCount[0] >> 8);
    auchBits[7] = (UCHAR)(pCtx->aulCount[0]);

    ulIndex = (pCtx->aulCount[0] >> 3) & 0x3F;
    ulPadLen = (ulIndex < 56) ? (56 - ulIndex) : (120 - ulIndex);
    sha1_update(pCtx, auchPadding, ulPadLen);
    sha1_update(pCtx, auchBits, 8);

    for (ulIndex = 0; ulIndex < 5; ulIndex++) {
        puchDigest[ulIndex*4]     = (UCHAR)(pCtx->aulState[ulIndex] >> 24);
        puchDigest[ulIndex*4 + 1] = (UCHAR)(pCtx->aulState[ulIndex] >> 16);
        puchDigest[ulIndex*4 + 2] = (UCHAR)(pCtx->aulState[ulIndex] >> 8);
        puchDigest[ulIndex*4 + 3] = (UCHAR)(pCtx->aulState[ulIndex]);
    }
}

/* ==================================================================
 * Public API
 * ================================================================== */

/*!
 * @brief Compute SHA-1 of a file.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY Sha1File(PCSZ pszPath, PSZ pszBuf, ULONG ulSize,
                         PULONG pulUsed) {
    FILE *fp;
    SHA1CONTEXT ctx;
    UCHAR auchBuffer[8192];
    size_t cbRead;
    UCHAR auchDigest[SHA1_DIGEST_SIZE];
    CHAR achHex[SHA1_HEX_SIZE];
    int nIdx;

    if (!pszPath) return ERROR_INVALID_PARAMETER;

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)SHA1_HEX_SIZE;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < SHA1_HEX_SIZE) {
        if (pulUsed) *pulUsed = (ULONG)SHA1_HEX_SIZE;
        return ERROR_BUFFER_OVERFLOW;
    }

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    sha1_init(&ctx);
    while ((cbRead = fread(auchBuffer, 1, sizeof(auchBuffer), fp)) > 0) {
        sha1_update(&ctx, auchBuffer, cbRead);
    }
    if (ferror(fp)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    fclose(fp);

    sha1_final(&ctx, auchDigest);

    for (nIdx = 0; nIdx < SHA1_DIGEST_SIZE; nIdx++) {
        sprintf(achHex + nIdx*2, "%02x", auchDigest[nIdx]);
    }
    achHex[SHA1_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, achHex, SHA1_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA1_HEX_SIZE - 1);
    return NO_ERROR;
}

/*!
 * @brief Compute SHA-1 of a NUL-terminated string.
 *
 * @param[in]  pszStr   Input string. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszStr is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY Sha1String(PCSZ pszStr, PSZ pszBuf, ULONG ulSize,
                           PULONG pulUsed) {
    SHA1CONTEXT ctx;
    UCHAR auchDigest[SHA1_DIGEST_SIZE];
    CHAR achHex[SHA1_HEX_SIZE];
    int nIdx;

    if (!pszStr) return ERROR_INVALID_PARAMETER;

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)SHA1_HEX_SIZE;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < SHA1_HEX_SIZE) {
        if (pulUsed) *pulUsed = (ULONG)SHA1_HEX_SIZE;
        return ERROR_BUFFER_OVERFLOW;
    }

    sha1_init(&ctx);
    sha1_update(&ctx, (const UCHAR*)pszStr, strlen(pszStr));
    sha1_final(&ctx, auchDigest);

    for (nIdx = 0; nIdx < SHA1_DIGEST_SIZE; nIdx++) {
        sprintf(achHex + nIdx*2, "%02x", auchDigest[nIdx]);
    }
    achHex[SHA1_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, achHex, SHA1_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA1_HEX_SIZE - 1);
    return NO_ERROR;
}
