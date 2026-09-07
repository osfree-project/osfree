/* sha1_utils.c - реализация SHA1 (C89) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1_utils.h"

#define SHA1_ROTL(a,b) (((a) << (b)) | ((a) >> (32-(b))))

static void sha1_transform(unsigned int state[5], const unsigned char buffer[64]) {
    unsigned int a, b, c, d, e;
    unsigned int w[80];
    int i;

    for (i = 0; i < 16; i++) {
        w[i] = ((unsigned int)buffer[i*4] << 24) |
               ((unsigned int)buffer[i*4+1] << 16) |
               ((unsigned int)buffer[i*4+2] << 8) |
               (unsigned int)buffer[i*4+3];
    }
    for (i = 16; i < 80; i++) {
        unsigned int tmp = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];
        w[i] = SHA1_ROTL(tmp, 1);
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];

    for (i = 0; i < 80; i++) {
        unsigned int f, k, temp;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        temp = SHA1_ROTL(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = SHA1_ROTL(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void sha1_init(SHA1_CTX *ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

void sha1_update(SHA1_CTX *ctx, const unsigned char *data, size_t len) {
    size_t i, index, partLen;

    index = (ctx->count[0] >> 3) & 0x3F;
    if ((ctx->count[0] += (unsigned int)(len << 3)) < (unsigned int)(len << 3))
        ctx->count[1]++;
    ctx->count[1] += (unsigned int)(len >> 29);

    partLen = 64 - index;
    if (len >= partLen) {
        memcpy(&ctx->buffer[index], data, partLen);
        sha1_transform(ctx->state, ctx->buffer);
        for (i = partLen; i + 63 < len; i += 64)
            sha1_transform(ctx->state, &data[i]);
        index = 0;
    } else {
        i = 0;
    }
    memcpy(&ctx->buffer[index], &data[i], len - i);
}

void sha1_final(SHA1_CTX *ctx, unsigned char digest[20]) {
    unsigned char bits[8];
    unsigned int index, padLen;
    static const unsigned char padding[64] = {0x80};

    bits[0] = (unsigned char)(ctx->count[1] >> 24);
    bits[1] = (unsigned char)(ctx->count[1] >> 16);
    bits[2] = (unsigned char)(ctx->count[1] >> 8);
    bits[3] = (unsigned char)(ctx->count[1]);
    bits[4] = (unsigned char)(ctx->count[0] >> 24);
    bits[5] = (unsigned char)(ctx->count[0] >> 16);
    bits[6] = (unsigned char)(ctx->count[0] >> 8);
    bits[7] = (unsigned char)(ctx->count[0]);

    index = (ctx->count[0] >> 3) & 0x3F;
    padLen = (index < 56) ? (56 - index) : (120 - index);
    sha1_update(ctx, padding, padLen);
    sha1_update(ctx, bits, 8);

    for (index = 0; index < 5; index++) {
        digest[index*4] = (unsigned char)(ctx->state[index] >> 24);
        digest[index*4+1] = (unsigned char)(ctx->state[index] >> 16);
        digest[index*4+2] = (unsigned char)(ctx->state[index] >> 8);
        digest[index*4+3] = (unsigned char)(ctx->state[index]);
    }
}

char *sha1_file(const char *path) {
    FILE *f;
    SHA1_CTX ctx;
    unsigned char buffer[8192];
    size_t n;
    unsigned char digest[20];
    char *hex;
    int i;

    f = fopen(path, "rb");
    if (!f) return NULL;

    sha1_init(&ctx);
    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        sha1_update(&ctx, buffer, n);
    }
    fclose(f);
    sha1_final(&ctx, digest);

    hex = (char *)malloc(41);
    if (!hex) return NULL;
    for (i = 0; i < 20; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[40] = '\0';
    return hex;
}

void sha1_string(const char *str, char *hex_out) {
    SHA1_CTX ctx;
    unsigned char digest[20];
    int i;

    sha1_init(&ctx);
    sha1_update(&ctx, (const unsigned char *)str, strlen(str));
    sha1_final(&ctx, digest);

    for (i = 0; i < 20; i++) {
        sprintf(hex_out + i*2, "%02x", digest[i]);
    }
    hex_out[40] = '\0';
}
