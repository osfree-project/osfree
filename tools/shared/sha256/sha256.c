/* sha256.c - SHA-256 hashing (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256.h"

/**
 * @file sha256.c
 * @brief Implementation of SHA-256.
 *
 * Conforms to:
 *   - FIPS PUB 180-2
 *   - RFC 6234
 */

/* ==================================================================
 * Internal constants and types
 * ================================================================== */

/** @brief SHA-256 block size in bytes. */
#define SHA256_BLOCK_SIZE  64

/** @brief SHA-256 digest size in bytes. */
#define SHA256_DIGEST_SIZE 32

/** @brief Hex representation length including NUL. */
#define SHA256_HEX_SIZE    (SHA256_DIGEST_SIZE * 2 + 1)

/**
 * @struct _SHA256CTX
 * @brief Working state of the SHA-256 algorithm.
 *
 * The message length is tracked as two 32-bit counters: count_lo
 * (low 32 bits of byte count) and count_hi (high 32 bits of byte
 * count). This matches the 64-bit length field required by
 * FIPS 180-2.
 */
typedef struct _SHA256CTX {
    unsigned int  state[8];                     /**< h0..h7.                */
    unsigned int  count_lo;                     /**< Byte count, low 32.    */
    unsigned int  count_hi;                     /**< Byte count, high 32.   */
    unsigned char buffer[SHA256_BLOCK_SIZE];    /**< Partial block buffer.  */
    unsigned int  buflen;                       /**< Bytes used in buffer.  */
} SHA256CTX;

/**
 * @brief Round constants K[0..63] from FIPS 180-2 §4.2.2.
 *
 * First 32 bits of the fractional parts of the cube roots of the
 * first 64 prime numbers.
 */
static const unsigned int K[64] = {
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

/**
 * @brief 32-bit right rotation.
 *
 * @param[in] x  Value.
 * @param[in] n  Rotation amount (0..31).
 *
 * @return Rotated value.
 */
static unsigned int rotr(unsigned int x, unsigned int n) {
    if (n == 0) return x;
    return (x >> n) | (x << (32 - n));
}

/**
 * @brief 32-bit logical right shift.
 *
 * @param[in] x  Value.
 * @param[in] n  Shift amount (0..31).
 *
 * @return Shifted value.
 */
static unsigned int shr(unsigned int x, unsigned int n) {
    return x >> n;
}

/**
 * @brief Choose function Ch(x, y, z) = (x & y) ^ (~x & z).
 */
static unsigned int ch(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (~x & z);
}

/**
 * @brief Majority function Maj(x, y, z) = (x&y) ^ (x&z) ^ (y&z).
 */
static unsigned int maj(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

/**
 * @brief Upper-case sigma-0 used in the compression function.
 *
 * sigma_0(x) = ROTR^2(x) ^ ROTR^13(x) ^ ROTR^22(x).
 */
static unsigned int sigma0(unsigned int x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

/**
 * @brief Upper-case sigma-1 used in the compression function.
 *
 * sigma_1(x) = ROTR^6(x) ^ ROTR^11(x) ^ ROTR^25(x).
 */
static unsigned int sigma1(unsigned int x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

/**
 * @brief Lower-case sigma-0 used in message schedule expansion.
 *
 * sigma_0(x) = ROTR^7(x) ^ ROTR^18(x) ^ SHR^3(x).
 */
static unsigned int sigma0_small(unsigned int x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}

/**
 * @brief Lower-case sigma-1 used in message schedule expansion.
 *
 * sigma_1(x) = ROTR^17(x) ^ ROTR^19(x) ^ SHR^10(x).
 */
static unsigned int sigma1_small(unsigned int x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
}

/* ==================================================================
 * Core algorithm
 * ================================================================== */

/**
 * @brief Compression function: process one 64-byte block.
 *
 * Applies the SHA-256 compression function to @p data and updates
 * the eight working variables in @p ctx.
 *
 * @param[in,out] ctx   Current state. Not NULL.
 * @param[in]     data  64-byte block. Not NULL.
 */
static void sha256_transform(SHA256CTX *ctx,
                             const unsigned char data[SHA256_BLOCK_SIZE]) {
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int w[64];
    int i;

    /* Prepare the message schedule. */
    for (i = 0; i < 16; i++) {
        w[i] = ((unsigned int)data[i*4]     << 24) |
               ((unsigned int)data[i*4 + 1] << 16) |
               ((unsigned int)data[i*4 + 2] << 8)  |
               ((unsigned int)data[i*4 + 3]);
    }
    for (i = 16; i < 64; i++) {
        w[i] = sigma1_small(w[i-2]) + w[i-7] +
               sigma0_small(w[i-15]) + w[i-16];
    }

    a = ctx->state[0]; b = ctx->state[1];
    c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5];
    g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; i++) {
        unsigned int t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
        unsigned int t2 = sigma0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b;
    ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f;
    ctx->state[6] += g; ctx->state[7] += h;
}

/**
 * @brief Initialize the working state with FIPS 180-2 initial values.
 *
 * @param[out] ctx  Context to initialize. Not NULL.
 */
static void sha256_init(SHA256CTX *ctx) {
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
    ctx->count_lo = 0;
    ctx->count_hi = 0;
    ctx->buflen = 0;
}

/**
 * @brief Absorb more input data into the context.
 *
 * Buffers partial blocks; full blocks are passed to the compression
 * function immediately.
 *
 * @param[in,out] ctx   Context. Not NULL.
 * @param[in]     data  Input data. Not NULL.
 * @param[in]     len   Number of bytes to absorb.
 */
static void sha256_update(SHA256CTX *ctx,
                          const unsigned char *data, size_t len) {
    /* Update the byte counter. len < 2^29 in practice, so the shift
     * into count_hi is safe. */
    {
        unsigned int prev = ctx->count_lo;
        ctx->count_lo += (unsigned int)len;
        if (ctx->count_lo < prev) ctx->count_hi++;
    }

    /* Top up the partial block first. */
    if (ctx->buflen > 0) {
        unsigned int need = SHA256_BLOCK_SIZE - ctx->buflen;
        if (len < need) {
            memcpy(ctx->buffer + ctx->buflen, data, len);
            ctx->buflen += (unsigned int)len;
            return;
        }
        memcpy(ctx->buffer + ctx->buflen, data, need);
        sha256_transform(ctx, ctx->buffer);
        data += need;
        len  -= need;
        ctx->buflen = 0;
    }

    /* Full blocks. */
    while (len >= SHA256_BLOCK_SIZE) {
        sha256_transform(ctx, data);
        data += SHA256_BLOCK_SIZE;
        len  -= SHA256_BLOCK_SIZE;
    }

    /* Remaining tail. */
    if (len > 0) {
        memcpy(ctx->buffer, data, len);
        ctx->buflen = (unsigned int)len;
    }
}

/**
 * @brief Finish the hash and produce the digest.
 *
 * Applies the padding scheme from FIPS 180-2 §5.1.1: a single 0x80
 * byte, zero bytes, and the 64-bit message length in bits as an
 * 8-byte big-endian value.
 *
 * @param[in,out] ctx     Context. Not NULL.
 * @param[out]    digest  32-byte output. Not NULL.
 */
static void sha256_final(SHA256CTX *ctx,
                         unsigned char digest[SHA256_DIGEST_SIZE]) {
    unsigned int bits_lo;
    unsigned int bits_hi;
    unsigned char pad[SHA256_BLOCK_SIZE * 2];
    unsigned char len_bytes[8];
    size_t pad_len;
    int i;

    /* Message length in bits: (count_hi << 32 | count_lo) * 8. */
    bits_lo = (ctx->count_lo << 3);
    bits_hi = (ctx->count_hi << 3) | (ctx->count_lo >> 29);

    /* Padding: 0x80, then zeros, then 8 bytes of length. The length
     * field ends at offset 56 modulo 64. */
    pad_len = (ctx->buflen < 56)
              ? (56 - ctx->buflen)
              : (120 - ctx->buflen);

    memset(pad, 0, pad_len);
    pad[0] = 0x80;

    len_bytes[0] = (unsigned char)(bits_hi >> 24);
    len_bytes[1] = (unsigned char)(bits_hi >> 16);
    len_bytes[2] = (unsigned char)(bits_hi >> 8);
    len_bytes[3] = (unsigned char)(bits_hi);
    len_bytes[4] = (unsigned char)(bits_lo >> 24);
    len_bytes[5] = (unsigned char)(bits_lo >> 16);
    len_bytes[6] = (unsigned char)(bits_lo >> 8);
    len_bytes[7] = (unsigned char)(bits_lo);

    /* Feed padding and length through the update function so that
     * full blocks are compressed as they fill up. */
    sha256_update(ctx, pad, pad_len);
    sha256_update(ctx, len_bytes, 8);

    /* Produce the digest in big-endian order. */
    for (i = 0; i < 8; i++) {
        digest[i*4]     = (unsigned char)(ctx->state[i] >> 24);
        digest[i*4 + 1] = (unsigned char)(ctx->state[i] >> 16);
        digest[i*4 + 2] = (unsigned char)(ctx->state[i] >> 8);
        digest[i*4 + 3] = (unsigned char)(ctx->state[i]);
    }
}

/* ==================================================================
 * Public API
 * ================================================================== */

APIRET APIENTRY Sha256File(PCSZ pszPath, PSZ pszBuf, ULONG ulSize,
                           PULONG pulUsed) {
    FILE *fp;
    SHA256CTX ctx;
    unsigned char buf[4096];
    size_t nread;
    unsigned char digest[SHA256_DIGEST_SIZE];
    char hex[SHA256_HEX_SIZE];
    int i;

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
    while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0) {
        sha256_update(&ctx, buf, nread);
    }
    if (ferror(fp)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    fclose(fp);

    sha256_final(&ctx, digest);

    for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[SHA256_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, hex, SHA256_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA256_HEX_SIZE - 1);
    return NO_ERROR;
}

APIRET APIENTRY Sha256String(PCSZ pszStr, PSZ pszBuf, ULONG ulSize,
                             PULONG pulUsed) {
    SHA256CTX ctx;
    unsigned char digest[SHA256_DIGEST_SIZE];
    char hex[SHA256_HEX_SIZE];
    int i;

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
    sha256_update(&ctx, (const unsigned char*)pszStr, strlen(pszStr));
    sha256_final(&ctx, digest);

    for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[SHA256_DIGEST_SIZE * 2] = '\0';

    memcpy(pszBuf, hex, SHA256_HEX_SIZE);
    if (pulUsed) *pulUsed = (ULONG)(SHA256_HEX_SIZE - 1);
    return NO_ERROR;
}
