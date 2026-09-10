/* spdx_utils.c - общие утилиты для SPDX-утилит (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_utils.h"

void spdx_strlist_init(SpdxStrList *list) {
    list->count = 0;
    list->capacity = 16;
    list->items = (char**)malloc((size_t)list->capacity * sizeof(char*));
    if (!list->items) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
}

void spdx_strlist_add(SpdxStrList *list, const char *str) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (char**)realloc(list->items,
                                       (size_t)list->capacity * sizeof(char*));
        if (!list->items) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    list->items[list->count] = (char*)malloc(strlen(str) + 1);
    if (!list->items[list->count]) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(list->items[list->count], str);
    list->count++;
}

int spdx_strlist_contains(const SpdxStrList *list, const char *str) {
    int i;
    for (i = 0; i < list->count; i++)
        if (strcmp(list->items[i], str) == 0) return 1;
    return 0;
}

void spdx_strlist_add_unique(SpdxStrList *list, const char *str) {
    if (!str || !str[0]) return;
    if (spdx_strlist_contains(list, str)) return;
    spdx_strlist_add(list, str);
}

void spdx_strlist_free(SpdxStrList *list) {
    int i;
    for (i = 0; i < list->count; i++) free(list->items[i]);
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

char *spdx_read_file_all(const char *path, long *out_size) {
    FILE *f;
    long size;
    char *buf;
    size_t rd;

    f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return NULL; }
    buf = (char*)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    rd = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (rd != (size_t)size) { free(buf); return NULL; }
    buf[size] = '\0';
    if (out_size) *out_size = size;
    return buf;
}

const char *spdx_get_file_name(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    if (backslash && (!slash || backslash > slash))
        return backslash + 1;
    if (slash)
        return slash + 1;
    return path;
}

void spdx_expression_collect_ids(const char *expr, SpdxStrList *out) {
    char *copy;
    char *token;
    const char *delims = " \t()";

    if (!expr) return;
    copy = strdup(expr);
    if (!copy) return;
    token = strtok(copy, delims);
    while (token) {
        if (strcmp(token, "AND") != 0 &&
            strcmp(token, "OR")  != 0 &&
            strcmp(token, "WITH") != 0 &&
            token[0] != '\0') {
            spdx_strlist_add_unique(out, token);
        }
        token = strtok(NULL, delims);
    }
    free(copy);
}
