/* sha1.c - SHA1 implementation (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1.h"

/* ==================================================================
 * Internal context (private)
 * ================================================================== */

typedef struct {
    unsigned int  state[5];
    unsigned int  count[2];
    unsigned char buffer[64];
} SHA1CONTEXT;

/* ==================================================================
 * Internal helpers (private)
 * ================================================================== */

static unsigned int sha1_rotl(unsigned int a, unsigned int b) {
    if (b == 0) return a;
    if (b >= 32) return a;
    return (a << b) | (a >> (32 - b));
}

static void sha1_transform(unsigned int state[5],
                           const unsigned char buffer[64]) {
    unsigned int a, b, c, d, e;
    unsigned int w[80];
    int i;

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

static void sha1_init(SHA1CONTEXT *pCtx) {
    pCtx->state[0] = 0x67452301u;
    pCtx->state[1] = 0xEFCDAB89u;
    pCtx->state[2] = 0x98BADCFEu;
    pCtx->state[3] = 0x10325476u;
    pCtx->state[4] = 0xC3D2E1F0u;
    pCtx->count[0] = 0;
    pCtx->count[1] = 0;
}

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

    partLen = 64 - index;
    if (cbLen >= partLen) {
        memcpy(&pCtx->buffer[index], pData, partLen);
        sha1_transform(pCtx->state, pCtx->buffer);
        for (i = partLen; i + 63 < cbLen; i += 64)
            sha1_transform(pCtx->state, &pData[i]);
        index = 0;
    } else {
        i = 0;
    }
    memcpy(&pCtx->buffer[index], &pData[i], cbLen - i);
}

static void sha1_final(SHA1CONTEXT *pCtx, unsigned char pDigest[20]) {
    unsigned char bits[8];
    unsigned int index, padLen;
    static const unsigned char padding[64] = {0x80};

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

APIRET APIENTRY Sha1File(PCSZ pszPath, PSZ *ppszHex) {
    FILE *f;
    SHA1CONTEXT ctx;
    unsigned char buffer[8192];
    size_t n;
    unsigned char digest[20];
    char *hex;
    int i;

    if (!pszPath || !ppszHex) return SHA1_ERROR_INVALID_PARAM;
    *ppszHex = NULL;

    f = fopen(pszPath, "rb");
    if (!f) return SHA1_ERROR_OPEN_FAILED;

    sha1_init(&ctx);
    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha1_update(&ctx, buffer, n);
    }
    if (ferror(f)) { fclose(f); return SHA1_ERROR_READ_FAILED; }
    fclose(f);
    sha1_final(&ctx, digest);

    hex = (char *)malloc(41);
    if (!hex) return SHA1_ERROR_OUT_OF_MEMORY;
    for (i = 0; i < 20; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[40] = '\0';
    *ppszHex = hex;
    return SHA1_NO_ERROR;
}

APIRET APIENTRY Sha1String(PCSZ pszStr, PSZ pszHex) {
    SHA1CONTEXT ctx;
    unsigned char digest[20];
    int i;

    if (!pszStr || !pszHex) return SHA1_ERROR_INVALID_PARAM;

    sha1_init(&ctx);
    sha1_update(&ctx, (const unsigned char *)pszStr, strlen(pszStr));
    sha1_final(&ctx, digest);

    for (i = 0; i < 20; i++) {
        sprintf(pszHex + i*2, "%02x", digest[i]);
    }
    pszHex[40] = '\0';
    return SHA1_NO_ERROR;
}
