#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha256_utils.h"

#define SHA256_BLOCK_SIZE 64
#define SHA256_DIGEST_SIZE 32

typedef struct {
    unsigned long state[8];
    unsigned long count[2];
    unsigned char buffer[SHA256_BLOCK_SIZE];
} SHA256_CTX;

static const unsigned long K[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

static unsigned long rotr(unsigned long x, unsigned int n) {
    return (x >> n) | (x << (32 - n));
}
static unsigned long shr(unsigned long x, unsigned int n) { return x >> n; }
static unsigned long ch(unsigned long x, unsigned long y, unsigned long z) { return (x & y) ^ (~x & z); }
static unsigned long maj(unsigned long x, unsigned long y, unsigned long z) { return (x & y) ^ (x & z) ^ (y & z); }
static unsigned long sigma0(unsigned long x) { return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static unsigned long sigma1(unsigned long x) { return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static unsigned long sigma0_small(unsigned long x) { return rotr(x,7) ^ rotr(x,18) ^ shr(x,3); }
static unsigned long sigma1_small(unsigned long x) { return rotr(x,17) ^ rotr(x,19) ^ shr(x,10); }

static void sha256_transform(SHA256_CTX *ctx, const unsigned char data[SHA256_BLOCK_SIZE]) {
    unsigned long a, b, c, d, e, f, g, h;
    unsigned long w[64];
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = ((unsigned long)data[i*4] << 24) | ((unsigned long)data[i*4+1] << 16) |
               ((unsigned long)data[i*4+2] << 8) | ((unsigned long)data[i*4+3]);
    }
    for (i = 16; i < 64; i++) {
        w[i] = sigma1_small(w[i-2]) + w[i-7] + sigma0_small(w[i-15]) + w[i-16];
    }
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0; i < 64; i++) {
        unsigned long t1 = h + sigma1(e) + ch(e,f,g) + K[i] + w[i];
        unsigned long t2 = sigma0(a) + maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->state[0]=0x6a09e667UL; ctx->state[1]=0xbb67ae85UL;
    ctx->state[2]=0x3c6ef372UL; ctx->state[3]=0xa54ff53aUL;
    ctx->state[4]=0x510e527fUL; ctx->state[5]=0x9b05688cUL;
    ctx->state[6]=0x1f83d9abUL; ctx->state[7]=0x5be0cd19UL;
    ctx->count[0]=0; ctx->count[1]=0;
}

static void sha256_update(SHA256_CTX *ctx, const unsigned char *data, size_t len) {
    unsigned long old_count = ctx->count[0];
    ctx->count[0] += (unsigned long)len;
    if (ctx->count[0] < old_count) ctx->count[1]++;
    while (len >= SHA256_BLOCK_SIZE) {
        sha256_transform(ctx, data);
        data += SHA256_BLOCK_SIZE;
        len -= SHA256_BLOCK_SIZE;
    }
    if (len > 0) memcpy(ctx->buffer, data, len);
}

static void sha256_final(SHA256_CTX *ctx, unsigned char digest[SHA256_DIGEST_SIZE]) {
    unsigned char padding[SHA256_BLOCK_SIZE * 2];
    unsigned long bit_count_hi = ctx->count[1];
    unsigned long bit_count_lo = ctx->count[0];
    size_t pad_len;
    int i;
    pad_len = (ctx->count[0] % SHA256_BLOCK_SIZE < 56) ?
              (56 - ctx->count[0] % SHA256_BLOCK_SIZE) :
              (120 - ctx->count[0] % SHA256_BLOCK_SIZE);
    memset(padding, 0, pad_len);
    padding[0] = 0x80;
    for (i = 0; i < 4; i++) padding[pad_len+i] = (unsigned char)(bit_count_hi >> (24 - i*8));
    for (i = 0; i < 4; i++) padding[pad_len+4+i] = (unsigned char)(bit_count_lo >> (24 - i*8));
    sha256_update(ctx, padding, pad_len + 8);
    for (i = 0; i < 8; i++) {
        digest[i*4]   = (unsigned char)(ctx->state[i] >> 24);
        digest[i*4+1] = (unsigned char)(ctx->state[i] >> 16);
        digest[i*4+2] = (unsigned char)(ctx->state[i] >> 8);
        digest[i*4+3] = (unsigned char)(ctx->state[i]);
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
    while ((nread = fread(buf,1,sizeof(buf),fp)) > 0) sha256_update(&ctx, buf, nread);
    fclose(fp);
    sha256_final(&ctx, digest);
    hex = (char*)malloc(SHA256_DIGEST_SIZE*2+1);
    if (!hex) return NULL;
    for (i=0;i<SHA256_DIGEST_SIZE;i++) sprintf(hex+i*2,"%02x",digest[i]);
    hex[SHA256_DIGEST_SIZE*2] = '\0';
    return hex;
}

char *sha256_string(const char *input, char *output) {
    SHA256_CTX ctx;
    unsigned char digest[SHA256_DIGEST_SIZE];
    int i;
    sha256_init(&ctx);
    sha256_update(&ctx, (const unsigned char*)input, strlen(input));
    sha256_final(&ctx, digest);
    for (i=0;i<SHA256_DIGEST_SIZE;i++) sprintf(output+i*2,"%02x",digest[i]);
    output[SHA256_DIGEST_SIZE*2]='\0';
    return output;
}
