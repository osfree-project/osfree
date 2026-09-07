/* sha1_utils.h - вычисление SHA1 (C89) */
#ifndef SHA1_UTILS_H
#define SHA1_UTILS_H

#include <stddef.h>

typedef struct {
    unsigned int state[5];
    unsigned int count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void sha1_init(SHA1_CTX *ctx);
void sha1_update(SHA1_CTX *ctx, const unsigned char *data, size_t len);
void sha1_final(SHA1_CTX *ctx, unsigned char digest[20]);

/* Вычисляет SHA1 файла, возвращает hex-строку (malloc) */
char *sha1_file(const char *path);

/* Вычисляет SHA1 строки, записывает hex в hex_out (минимум 41 байт) */
void sha1_string(const char *str, char *hex_out);

#endif
