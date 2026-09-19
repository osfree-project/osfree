/* sha1.c - SHA-1 implementation (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1.h"

/**
 * @file sha1.c
 * @brief Implementation of SHA-1.
 *
 * Conforms to:
 *   - FIPS PUB 180-1
 *   - RFC 3174
 */

/* ==================================================================
 * Internal constants and types
 * ================================================================== */

/** @brief SHA-1 block size in bytes. */
#define SHA1_BLOCK_SIZE  64

/** @brief SHA-1 digest size in bytes. */
#define SHA1_DIGEST_SIZE 20

/** @brief Hex representation length including NUL. */
#define SHA1_HEX_SIZE    (SHA1_DIGEST_SIZE * 2 + 1)

/**
 * @struct _SHA1CONTEXT
 * @brief Working state of the SHA-1 algorithm.
 *
 * The message length is tracked as two 32-bit counters: count[0]
 * (low 32 bits of bit count) and count[1] (high 32 bits of bit
 * count). This matches the 64-bit length field required by
 * FIPS 180-1.
 */
typedef struct _SHA1CONTEXT {
    unsigned int  state[5];                     /**< h0..h4.              */
    unsigned int  count[2];                     /**< Bit count, lo/hi.    */
    unsigned char buffer[SHA1_BLOCK_SIZE];      /**< Partial block buffer. */
} SHA1CONTEXT;

/* ==================================================================
 * Internal bit helpers
 * ================================================================== */

/**
 * @brief 32-bit left rotation.
 *
 * @param[in] a  Value.
 * @param[in] b  Rotation amount (0..31).
 *
 * @return Rotated value.
 */
static unsigned int sha1_rotl(unsigned int a, unsigned int b) {
    if (b == 0) return a;
    if (b >= 32) return a;
    return (a << b) | (a >> (32 - b));
}

/* ==================================================================
 * Core algorithm
 * ================================================================== */

/**
 * @brief Compression function: process one 64-byte block.
 *
 * Applies the SHA-1 compression function to @p buffer and updates
 * the five working variables in @p state.
 *
 * @param[in,out] state   Current state. Not NULL.
 * @param[in]     buffer  64-byte block. Not NULL.
 */
static void sha1_transform(unsigned int state[5],
                           const unsigned char buffer[SHA1_BLOCK_SIZE]) {
    unsigned int a, b, c, d, e;
    unsigned int w[80];
    int i;

    /* Prepare the message schedule. */
    for (i = 0; i < 16; i++) {
        w[i] = ((unsigned int)buffer[i*4]     << 24) |
               ((unsigned int)buffer[i*4 + 1] << 16) |
               ((unsigned int)buffer[i*4 + 2] << 8)  |
               ((unsigned int)buffer[i*4 + 3]);
    }
    for (i = 16; i < 80; i++) {
        unsigned int tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = sha1_rotl(tmp, 1);
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    for (i = 0; i < 80; i++) {
        unsigned int f, k, temp;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }
        temp = sha1_rotl(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = sha1_rotl(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

/**
 * @brief Initialize the working state with FIPS 180-1 initial values.
 *
 * @param[out] pCtx  Context to initialize. Not NULL.
 */
static void sha1_init(SHA1CONTEXT *pCtx) {
    pCtx->state[0] = 0x67452301u;
    pCtx->state[1] = 0xEFCDAB89u;
    pCtx->state[2] = 0x98BADCFEu;
    pCtx->state[3] = 0x10325476u;
    pCtx->state[4] = 0xC3D2E1F0u;
    pCtx->count[0] = 0;
    pCtx->count[1] = 0;
}

/**
 * @brief Absorb more input data into the context.
 *
 * Buffers partial blocks; full blocks are passed to the compression
 * function immediately.
 *
 * @param[in,out] pCtx   Context. Not NULL.
 * @param[in]     pData  Input data. Not NULL.
 * @param[in]     cbLen  Number of bytes to absorb.
 */
static void sha1_update(SHA1CONTEXT *pCtx,
                        const unsigned char *pData, size_t cbLen) {
    size_t i, index, partLen;
    unsigned int bits_lo;
    unsigned int bits_hi;
    unsigned int prev;

    /* Compute bit count as (len * 8), split into two 32-bit words.
     * For len < 2^29 (always true for real read blocks) the result is
     * correct on any platform. */
    bits_lo = (unsigned int)cbLen << 3;
    bits_hi = (unsigned int)(cbLen >> 29);

    index = (pCtx->count[0] >> 3) & 0x3F;
    prev = pCtx->count[0];
    pCtx->count[0] += bits_lo;
    if (pCtx->count[0] < prev) pCtx->count[1]++;
    pCtx->count[1] += bits_hi;

    partLen = SHA1_BLOCK_SIZE - index;
    if (cbLen >= partLen) {
        memcpy(&pCtx->buffer[index], pData, partLen);
        sha1_transform(pCtx->state, pCtx->buffer);
        for (i = partLen; i + 63 < cbLen; i += SHA1_BLOCK_SIZE)
            sha1_transform(pCtx->state, &pData[i]);
        index = 0;
    } else {
        i = 0;
    }
    memcpy(&pCtx->buffer[index], &pData[i], cbLen - i);
}

/**
 * @brief Finish the hash and produce the digest.
 *
 * Applies the padding scheme from FIPS 180-1 §4: a single 0x80
 * byte, zero bytes, and the 64-bit message length in bits as an
 * 8-byte big-endian value.
 *
 * @param[in,out] pCtx     Context. Not NULL.
 * @param[out]    pDigest  20-byte output. Not NULL.
 */
static void sha1_final(SHA1CONTEXT *pCtx,
                       unsigned char pDigest[SHA1_DIGEST_SIZE]) {
    unsigned char bits[8];
    unsigned int index, padLen;
    static const unsigned char padding[SHA1_BLOCK_SIZE] = {0x80};

    bits[0] = (unsigned char)(pCtx->count[1] >> 24);
    bits[1] = (unsigned char)(pCtx->count[1] >> 16);
    bits[2] = (unsigned char)(pCtx->count[1] >> 8);
    bits[3] = (unsigned char)(pCtx->count[1]);
    bits[4] = (unsigned char)(pCtx->count[0] >> 24);
    bits[5] = (unsigned char)(pCtx->count[0] >> 16);
    bits[6] = (unsigned char)(pCtx->count[0] >> 8);
    bits[7] = (unsigned char)(pCtx->count[0]);

    index = (pCtx->count[0] >> 3) & 0x3F;
    padLen = (index < 56) ? (56 - index) : (120 - index);
    sha1_update(pCtx, padding, padLen);
    sha1_update(pCtx, bits, 8);

    for (index = 0; index < 5; index++) {
        pDigest[index*4]     = (unsigned char)(pCtx->state[index] >> 24);
        pDigest[index*4 + 1] = (unsigned char)(pCtx->state[index] >> 16);
        pDigest[index*4 + 2] = (unsigned char)(pCtx->state[index] >> 8);
        pDigest[index*4 + 3] = (unsigned char)(pCtx->state[index]);
    }
}

/* ==================================================================
 * Public API
 * ================================================================== */

APIRET APIENTRY Sha1File(PCSZ pszPath, PSZ pszBuf, ULONG ulSize,
                         PULONG pulUsed) {
    FILE *f;
    SHA1CONTEXT ctx;
    unsigned char buffer[8192];
    size_t n;
    unsigned char digest[SHA1_DIGEST_SIZE];
    char hex[SHA1_HEX_SIZE];
    int i;

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

    f = fopen(pszPath, "rb");
    if (!f) return ERROR_OPEN_FAILED;

    sha1_init(&ctx);
    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha1_update(&ctx, buffer, n);
    }
    if (ferror(f)) {
        fclose(f);
        return ERROR_READ_FAULT;
    }
    fclose(f);

    sha1_final(&ctx, digest);

    for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[SHA1_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, hex, SHA1_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA1_HEX_SIZE - 1);
    return NO_ERROR;
}

APIRET APIENTRY Sha1String(PCSZ pszStr, PSZ pszBuf, ULONG ulSize,
                           PULONG pulUsed) {
    SHA1CONTEXT ctx;
    unsigned char digest[SHA1_DIGEST_SIZE];
    char hex[SHA1_HEX_SIZE];
    int i;

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
    sha1_update(&ctx, (const unsigned char*)pszStr, strlen(pszStr));
    sha1_final(&ctx, digest);

    for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[SHA1_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, hex, SHA1_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA1_HEX_SIZE - 1);
    return NO_ERROR;
}
