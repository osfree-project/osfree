/* sha256_utils.c - реализация SHA256 (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256_utils.h"

#define SHA256_BLOCK_SIZE  64
#define SHA256_DIGEST_SIZE 32

/* Внутреннее состояние. Все счётчики — 32-битные (unsigned int
 * на всех целевых платформах OpenWatcom), длина сообщения в байтах
 * хранится как два 32-битных слова. */
typedef struct {
    unsigned int state[8];
    unsigned int count_lo;                    /* младшие 32 бита байт */
    unsigned int count_hi;                    /* старшие 32 бита байт */
    unsigned char buffer[SHA256_BLOCK_SIZE];
    unsigned int buflen;                      /* сколько байт в buffer */
} SHA256_CTX;

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

static unsigned int rotr(unsigned int x, unsigned int n) {
    if (n == 0) return x;
    return (x >> n) | (x << (32 - n));
}
static unsigned int shr(unsigned int x, unsigned int n) { return x >> n; }
static unsigned int ch(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (~x & z);
}
static unsigned int maj(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (x & z) ^ (y & z);
}
static unsigned int sigma0(unsigned int x) {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}
static unsigned int sigma1(unsigned int x) {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}
static unsigned int sigma0_small(unsigned int x) {
    return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
}
static unsigned int sigma1_small(unsigned int x) {
    return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
}

static void sha256_transform(SHA256_CTX *ctx,
                             const unsigned char data[SHA256_BLOCK_SIZE]) {
    unsigned int a, b, c, d, e, f, g, h;
    unsigned int w[64];
    int i;

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

static void sha256_init(SHA256_CTX *ctx) {
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

static void sha256_update(SHA256_CTX *ctx,
                          const unsigned char *data, size_t len) {
    /* Обновляем счётчик байт (len приводится к unsigned int; для
     * реальных вызовов len < 2^29, переполнения нет). */
    {
        unsigned int prev = ctx->count_lo;
        ctx->count_lo += (unsigned int)len;
        if (ctx->count_lo < prev) ctx->count_hi++;
    }

    /* Дописываем буфер до полного блока */
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

    /* Полные блоки */
    while (len >= SHA256_BLOCK_SIZE) {
        sha256_transform(ctx, data);
        data += SHA256_BLOCK_SIZE;
        len  -= SHA256_BLOCK_SIZE;
    }

    /* Остаток */
    if (len > 0) {
        memcpy(ctx->buffer, data, len);
        ctx->buflen = (unsigned int)len;
    }
}

static void sha256_final(SHA256_CTX *ctx,
                         unsigned char digest[SHA256_DIGEST_SIZE]) {
    unsigned int bits_lo;
    unsigned int bits_hi;
    unsigned char pad[SHA256_BLOCK_SIZE * 2];
    unsigned char len_bytes[8];
    size_t pad_len;
    int i;

    /* Длина сообщения в битах: (count_hi << 32 | count_lo) * 8 */
    bits_lo = (ctx->count_lo << 3);
    bits_hi = (ctx->count_hi << 3) | (ctx->count_lo >> 29);

    /* Паддинг: 0x80, затем нули, затем 8 байт длины.
     * Нужно дойти до позиции 56 в блоке. */
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

    /* Обновляем буфер паддингом и длиной. sha256_update корректно
     * трансформирует все блоки по мере заполнения. */
    sha256_update(ctx, pad, pad_len);
    sha256_update(ctx, len_bytes, 8);

    for (i = 0; i < 8; i++) {
        digest[i*4]     = (unsigned char)(ctx->state[i] >> 24);
        digest[i*4 + 1] = (unsigned char)(ctx->state[i] >> 16);
        digest[i*4 + 2] = (unsigned char)(ctx->state[i] >> 8);
        digest[i*4 + 3] = (unsigned char)(ctx->state[i]);
    }
}

char *sha256_file(const char *path) {
    FILE *fp;
    SHA256_CTX ctx;
    unsigned char buf[4096];
    size_t nread;
    unsigned char digest[SHA256_DIGEST_SIZE];
    char *hex;
    int i;

    fp = fopen(path, "rb");
    if (!fp) return NULL;

    sha256_init(&ctx);
    while ((nread = fread(buf, 1, sizeof(buf), fp)) > 0) {
        sha256_update(&ctx, buf, nread);
    }
    if (ferror(fp)) { fclose(fp); return NULL; }
    fclose(fp);

    sha256_final(&ctx, digest);

    hex = (char*)malloc(SHA256_DIGEST_SIZE * 2 + 1);
    if (!hex) return NULL;
    for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(hex + i*2, "%02x", digest[i]);
    }
    hex[SHA256_DIGEST_SIZE * 2] = '\0';
    return hex;
}

char *sha256_string(const char *input, char *output) {
    SHA256_CTX ctx;
    unsigned char digest[SHA256_DIGEST_SIZE];
    int i;

    sha256_init(&ctx);
    sha256_update(&ctx, (const unsigned char*)input, strlen(input));
    sha256_final(&ctx, digest);

    for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(output + i*2, "%02x", digest[i]);
    }
    output[SHA256_DIGEST_SIZE * 2] = '\0';
    return output;
}
