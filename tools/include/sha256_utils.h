#ifndef SHA256_UTILS_H
#define SHA256_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

char *sha256_file(const char *path);
char *sha256_string(const char *input, char *output);

#ifdef __cplusplus
}
#endif

#endif
