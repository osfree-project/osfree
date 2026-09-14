/* spdx_sbom_utils.c - вспомогательные функции SPDX SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_sbom_utils.h"
#include "spdx_utils.h"

void sbom_remove_extension(char *str) {
    char *dot = strrchr(str, '.');
    if (dot) *dot = '\0';
}

void sbom_sanitize_id(const char *src, char *dst, size_t dst_size) {
    size_t i = 0, j = 0, start, end;
    for (i = 0; src[i] != '\0' && j < dst_size - 1; i++) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c) || c == '.' || c == '-')
            dst[j++] = (char)c;
        else
            dst[j++] = '-';
    }
    dst[j] = '\0';
    start = 0;
    end = j;
    while (start < end && dst[start] == '-') start++;
    while (end > start && dst[end - 1] == '-') end--;
    if (start > 0 || end < j) {
        memmove(dst, dst + start, end - start);
        dst[end - start] = '\0';
    }
}

void sbom_make_package_id(const char *base_name, const char *suffix,
                          char *buf, size_t buf_size) {
    char sanitized[256];
    sbom_sanitize_id(base_name, sanitized, sizeof(sanitized));
    if (suffix && *suffix)
        snprintf(buf, buf_size, "SPDXRef-Package-%s-%s", sanitized, suffix);
    else
        snprintf(buf, buf_size, "SPDXRef-Package-%s", sanitized);
}

char *sbom_json_escape(const char *src) {
    size_t len, extra, i, j;
    char *dst;
    len = strlen(src);
    extra = 0;
    for (i = 0; i < len; i++) {
        if (src[i] == '"' || src[i] == '\\' || src[i] == '\n' ||
            src[i] == '\r' || src[i] == '\t')
            extra++;
    }
    dst = (char*)malloc(len + extra + 1);
    if (!dst) return NULL;
    j = 0;
    for (i = 0; i < len; i++) {
        switch (src[i]) {
            case '"':  dst[j++] = '\\'; dst[j++] = '"'; break;
            case '\\': dst[j++] = '\\'; dst[j++] = '\\'; break;
            case '\n': dst[j++] = '\\'; dst[j++] = 'n'; break;
            case '\r': dst[j++] = '\\'; dst[j++] = 'r'; break;
            case '\t': dst[j++] = '\\'; dst[j++] = 't'; break;
            default:   dst[j++] = src[i]; break;
        }
    }
    dst[j] = '\0';
    return dst;
}

const char *sbom_get_file_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "OTHER";
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 ||
        strcmp(ext, ".h") == 0 || strcmp(ext, ".asm") == 0 ||
        strcmp(ext, ".rc") == 0) return "SOURCE";
    if (strcmp(ext, ".ico") == 0 || strcmp(ext, ".bmp") == 0 ||
        strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0) return "IMAGE";
    if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".md") == 0) return "TEXT";
    if (strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
        strcmp(ext, ".sys") == 0 || strcmp(ext, ".lib") == 0) return "BINARY";
    return "OTHER";
}

void filelist_init(FileList *list) {
    list->count = 0;
    list->capacity = 16;
    list->items = (FileInfo*)malloc(list->capacity * sizeof(FileInfo));
    if (!list->items) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

void filelist_add(FileList *list, const FileInfo *info) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (FileInfo*)realloc(list->items,
                                         list->capacity * sizeof(FileInfo));
        if (!list->items) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    list->items[list->count++] = *info;
}

void filelist_free(FileList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}
