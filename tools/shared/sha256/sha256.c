/*!
 *
 * @file sha256.c
 *
 * @brief Implementation of SHA-256.
 *
 * SHA-256 hashing (C89, OpenWatcom). Conforms to:
 *   - FIPS PUB 180-2.
 *   - RFC 6234.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256.h"

/* ==================================================================
 * Internal constants and types
 * ================================================================== */

/*!
 * @brief SHA-256 block size in bytes.
 */
#define SHA256_BLOCK_SIZE  64

/*!
 * @brief SHA-256 digest size in bytes.
 */
#define SHA256_DIGEST_SIZE 32

/*!
 * @brief Hex representation length including NUL.
 */
#define SHA256_HEX_SIZE    (SHA256_DIGEST_SIZE * 2 + 1)

/*!
 * @struct _SHA256CTX
 * @brief Working state of the SHA-256 algorithm.
 *
 * The message length is tracked as two 32-bit counters: ulCountLo
 * (low 32 bits of byte count) and ulCountHi (high 32 bits of byte
 * count). This matches the 64-bit length field required by
 * FIPS 180-2.
 */
typedef struct _SHA256CTX {
    ULONG aulState[8];                    /*!< h0..h7.               */
    ULONG ulCountLo;                      /*!< Byte count, low 32.   */
    ULONG ulCountHi;                      /*!< Byte count, high 32.  */
    UCHAR auchBuffer[SHA256_BLOCK_SIZE];  /*!< Partial block buffer. */
    ULONG ulBufLen;                       /*!< Bytes used in buffer. */
} SHA256CTX;

/*!
 * @brief Round constants K[0..63] from FIPS 180-2 §4.2.2.
 *
 * First 32 bits of the fractional parts of the cube roots of the
 * first 64 prime numbers.
 */
static const ULONG aulK[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

/* ==================================================================
 * Internal bit helpers
 * ================================================================== */

/*!
 * @brief 32-bit right rotation.
 *
 * @param[in] ulVal  Value.
 * @param[in] nBits  Rotation amount (0..31).
 *
 * @return Rotated value.
 */
static ULONG rotr(ULONG ulVal, ULONG nBits) {
    if (nBits == 0) return ulVal;
    return (ulVal >> nBits) | (ulVal << (32 - nBits));
}

/*!
 * @brief 32-bit logical right shift.
 *
 * @param[in] ulVal  Value.
 * @param[in] nBits  Shift amount (0..31).
 *
 * @return Shifted value.
 */
static ULONG shr(ULONG ulVal, ULONG nBits) {
    return ulVal >> nBits;
}

/*!
 * @brief Choose function Ch(x, y, z) = (x & y) ^ (~x & z).
 *
 * @param[in] ulX  First operand.
 * @param[in] ulY  Second operand.
 * @param[in] ulZ  Third operand.
 *
 * @return Ch(x, y, z).
 */
static ULONG ch(ULONG ulX, ULONG ulY, ULONG ulZ) {
    return (ulX & ulY) ^ (~ulX & ulZ);
}

/*!
 * @brief Majority function Maj(x, y, z) = (x&y) ^ (x&z) ^ (y&z).
 *
 * @param[in] ulX  First operand.
 * @param[in] ulY  Second operand.
 * @param[in] ulZ  Third operand.
 *
 * @return Maj(x, y, z).
 */
static ULONG maj(ULONG ulX, ULONG ulY, ULONG ulZ) {
    return (ulX & ulY) ^ (ulX & ulZ) ^ (ulY & ulZ);
}

/*!
 * @brief Upper-case sigma-0 used in the compression function.
 *
 * sigma_0(x) = ROTR^2(x) ^ ROTR^13(x) ^ ROTR^22(x).
 *
 * @param[in] ulX  Value.
 *
 * @return sigma_0(x).
 */
static ULONG sigma0(ULONG ulX) {
    return rotr(ulX, 2) ^ rotr(ulX, 13) ^ rotr(ulX, 22);
}

/*!
 * @brief Upper-case sigma-1 used in the compression function.
 *
 * sigma_1(x) = ROTR^6(x) ^ ROTR^11(x) ^ ROTR^25(x).
 *
 * @param[in] ulX  Value.
 *
 * @return sigma_1(x).
 */
static ULONG sigma1(ULONG ulX) {
    return rotr(ulX, 6) ^ rotr(ulX, 11) ^ rotr(ulX, 25);
}

/*!
 * @brief Lower-case sigma-0 used in message schedule expansion.
 *
 * sigma_0(x) = ROTR^7(x) ^ ROTR^18(x) ^ SHR^3(x).
 *
 * @param[in] ulX  Value.
 *
 * @return sigma_0(x).
 */
static ULONG sigma0_small(ULONG ulX) {
    return rotr(ulX, 7) ^ rotr(ulX, 18) ^ shr(ulX, 3);
}

/*!
 * @brief Lower-case sigma-1 used in message schedule expansion.
 *
 * sigma_1(x) = ROTR^17(x) ^ ROTR^19(x) ^ SHR^10(x).
 *
 * @param[in] ulX  Value.
 *
 * @return sigma_1(x).
 */
static ULONG sigma1_small(ULONG ulX) {
    return rotr(ulX, 17) ^ rotr(ulX, 19) ^ shr(ulX, 10);
}

/* ==================================================================
 * Core algorithm
 * ================================================================== */

/*!
 * @brief Compression function: process one 64-byte block.
 *
 * Applies the SHA-256 compression function to @p puchData and
 * updates the eight working variables in @p pCtx.
 *
 * @param[in,out] pCtx      Current state. Not NULL.
 * @param[in]     puchData  64-byte block. Not NULL.
 */
static void sha256_transform(SHA256CTX *pCtx,
                             const UCHAR puchData[SHA256_BLOCK_SIZE]) {
    ULONG ulA, ulB, ulC, ulD, ulE, ulF, ulG, ulH;
    ULONG aulW[64];
    int nIdx;

    /* Prepare the message schedule. */
    for (nIdx = 0; nIdx < 16; nIdx++) {
        aulW[nIdx] = ((ULONG)puchData[nIdx*4]     << 24) |
                     ((ULONG)puchData[nIdx*4 + 1] << 16) |
                     ((ULONG)puchData[nIdx*4 + 2] << 8)  |
                     ((ULONG)puchData[nIdx*4 + 3]);
    }
    for (nIdx = 16; nIdx < 64; nIdx++) {
        aulW[nIdx] = sigma1_small(aulW[nIdx-2]) + aulW[nIdx-7] +
                     sigma0_small(aulW[nIdx-15]) + aulW[nIdx-16];
    }

    ulA = pCtx->aulState[0]; ulB = pCtx->aulState[1];
    ulC = pCtx->aulState[2]; ulD = pCtx->aulState[3];
    ulE = pCtx->aulState[4]; ulF = pCtx->aulState[5];
    ulG = pCtx->aulState[6]; ulH = pCtx->aulState[7];

    for (nIdx = 0; nIdx < 64; nIdx++) {
        ULONG ulT1 = ulH + sigma1(ulE) + ch(ulE, ulF, ulG) +
                     aulK[nIdx] + aulW[nIdx];
        ULONG ulT2 = sigma0(ulA) + maj(ulA, ulB, ulC);
        ulH = ulG;
        ulG = ulF;
        ulF = ulE;
        ulE = ulD + ulT1;
        ulD = ulC;
        ulC = ulB;
        ulB = ulA;
        ulA = ulT1 + ulT2;
    }

    pCtx->aulState[0] += ulA; pCtx->aulState[1] += ulB;
    pCtx->aulState[2] += ulC; pCtx->aulState[3] += ulD;
    pCtx->aulState[4] += ulE; pCtx->aulState[5] += ulF;
    pCtx->aulState[6] += ulG; pCtx->aulState[7] += ulH;
}

/*!
 * @brief Initialize the working state with FIPS 180-2 initial values.
 *
 * @param[out] pCtx  Context to initialize. Not NULL.
 */
static void sha256_init(SHA256CTX *pCtx) {
    pCtx->aulState[0] = 0x6a09e667u;
    pCtx->aulState[1] = 0xbb67ae85u;
    pCtx->aulState[2] = 0x3c6ef372u;
    pCtx->aulState[3] = 0xa54ff53au;
    pCtx->aulState[4] = 0x510e527fu;
    pCtx->aulState[5] = 0x9b05688cu;
    pCtx->aulState[6] = 0x1f83d9abu;
    pCtx->aulState[7] = 0x5be0cd19u;
    pCtx->ulCountLo = 0;
    pCtx->ulCountHi = 0;
    pCtx->ulBufLen = 0;
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
static void sha256_update(SHA256CTX *pCtx,
                          const UCHAR *puchData, size_t cbLen) {
    /* Update the byte counter. len < 2^29 in practice, so the shift
     * into ulCountHi is safe. */
    {
        ULONG ulPrev = pCtx->ulCountLo;
        pCtx->ulCountLo += (ULONG)cbLen;
        if (pCtx->ulCountLo < ulPrev) pCtx->ulCountHi++;
    }

    /* Top up the partial block first. */
    if (pCtx->ulBufLen > 0) {
        ULONG ulNeed = SHA256_BLOCK_SIZE - pCtx->ulBufLen;
        if (cbLen < ulNeed) {
            memcpy(pCtx->auchBuffer + pCtx->ulBufLen, puchData, cbLen);
            pCtx->ulBufLen += (ULONG)cbLen;
            return;
        }
        memcpy(pCtx->auchBuffer + pCtx->ulBufLen, puchData, ulNeed);
        sha256_transform(pCtx, pCtx->auchBuffer);
        puchData += ulNeed;
        cbLen  -= ulNeed;
        pCtx->ulBufLen = 0;
    }

    /* Full blocks. */
    while (cbLen >= SHA256_BLOCK_SIZE) {
        sha256_transform(pCtx, puchData);
        puchData += SHA256_BLOCK_SIZE;
        cbLen  -= SHA256_BLOCK_SIZE;
    }

    /* Remaining tail. */
    if (cbLen > 0) {
        memcpy(pCtx->auchBuffer, puchData, cbLen);
        pCtx->ulBufLen = (ULONG)cbLen;
    }
}

/*!
 * @brief Finish the hash and produce the digest.
 *
 * Applies the padding scheme from FIPS 180-2 §5.1.1: a single 0x80
 * byte, zero bytes, and the 64-bit message length in bits as an
 * 8-byte big-endian value.
 *
 * @param[in,out] pCtx        Context. Not NULL.
 * @param[out]    puchDigest  32-byte output. Not NULL.
 */
static void sha256_final(SHA256CTX *pCtx,
                         UCHAR puchDigest[SHA256_DIGEST_SIZE]) {
    ULONG ulBitsLo;
    ULONG ulBitsHi;
    UCHAR auchPad[SHA256_BLOCK_SIZE * 2];
    UCHAR auchLenBytes[8];
    size_t cbPadLen;
    int nIdx;

    /* Message length in bits: (count_hi << 32 | count_lo) * 8. */
    ulBitsLo = (pCtx->ulCountLo << 3);
    ulBitsHi = (pCtx->ulCountHi << 3) | (pCtx->ulCountLo >> 29);

    /* Padding: 0x80, then zeros, then 8 bytes of length. The length
     * field ends at offset 56 modulo 64. */
    cbPadLen = (pCtx->ulBufLen < 56)
               ? (56 - pCtx->ulBufLen)
               : (120 - pCtx->ulBufLen);

    memset(auchPad, 0, cbPadLen);
    auchPad[0] = 0x80;

    auchLenBytes[0] = (UCHAR)(ulBitsHi >> 24);
    auchLenBytes[1] = (UCHAR)(ulBitsHi >> 16);
    auchLenBytes[2] = (UCHAR)(ulBitsHi >> 8);
    auchLenBytes[3] = (UCHAR)(ulBitsHi);
    auchLenBytes[4] = (UCHAR)(ulBitsLo >> 24);
    auchLenBytes[5] = (UCHAR)(ulBitsLo >> 16);
    auchLenBytes[6] = (UCHAR)(ulBitsLo >> 8);
    auchLenBytes[7] = (UCHAR)(ulBitsLo);

    sha256_update(pCtx, auchPad, cbPadLen);
    sha256_update(pCtx, auchLenBytes, 8);

    /* Produce the digest in big-endian order. */
    for (nIdx = 0; nIdx < 8; nIdx++) {
        puchDigest[nIdx*4]     = (UCHAR)(pCtx->aulState[nIdx] >> 24);
        puchDigest[nIdx*4 + 1] = (UCHAR)(pCtx->aulState[nIdx] >> 16);
        puchDigest[nIdx*4 + 2] = (UCHAR)(pCtx->aulState[nIdx] >> 8);
        puchDigest[nIdx*4 + 3] = (UCHAR)(pCtx->aulState[nIdx]);
    }
}

/* ==================================================================
 * Public API
 * ================================================================== */

/*!
 * @brief Compute SHA-256 of a file.
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
APIRET APIENTRY Sha256File(PCSZ pszPath, PSZ pszBuf, ULONG ulSize,
                           PULONG pulUsed) {
    FILE *fp;
    SHA256CTX ctx;
    UCHAR auchBuf[4096];
    size_t cbRead;
    UCHAR auchDigest[SHA256_DIGEST_SIZE];
    CHAR achHex[SHA256_HEX_SIZE];
    int nIdx;

    if (!pszPath) return ERROR_INVALID_PARAMETER;

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)SHA256_HEX_SIZE;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < SHA256_HEX_SIZE) {
        if (pulUsed) *pulUsed = (ULONG)SHA256_HEX_SIZE;
        return ERROR_BUFFER_OVERFLOW;
    }

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    sha256_init(&ctx);
    while ((cbRead = fread(auchBuf, 1, sizeof(auchBuf), fp)) > 0) {
        sha256_update(&ctx, auchBuf, cbRead);
    }
    if (ferror(fp)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    fclose(fp);

    sha256_final(&ctx, auchDigest);

    for (nIdx = 0; nIdx < SHA256_DIGEST_SIZE; nIdx++) {
        sprintf(achHex + nIdx*2, "%02x", auchDigest[nIdx]);
    }
    achHex[SHA256_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, achHex, SHA256_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA256_HEX_SIZE - 1);
    return NO_ERROR;
}

/*!
 * @brief Compute SHA-256 of a NUL-terminated string.
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
APIRET APIENTRY Sha256String(PCSZ pszStr, PSZ pszBuf, ULONG ulSize,
                             PULONG pulUsed) {
    SHA256CTX ctx;
    UCHAR auchDigest[SHA256_DIGEST_SIZE];
    CHAR achHex[SHA256_HEX_SIZE];
    int nIdx;

    if (!pszStr) return ERROR_INVALID_PARAMETER;

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)SHA256_HEX_SIZE;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < SHA256_HEX_SIZE) {
        if (pulUsed) *pulUsed = (ULONG)SHA256_HEX_SIZE;
        return ERROR_BUFFER_OVERFLOW;
    }

    sha256_init(&ctx);
    sha256_update(&ctx, (const UCHAR*)pszStr, strlen(pszStr));
    sha256_final(&ctx, auchDigest);

    for (nIdx = 0; nIdx < SHA256_DIGEST_SIZE; nIdx++) {
        sprintf(achHex + nIdx*2, "%02x", auchDigest[nIdx]);
    }
    achHex[SHA256_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, achHex, SHA256_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA256_HEX_SIZE - 1);
    return NO_ERROR;
}
