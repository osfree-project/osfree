/* spdx_tag.c - реализация функций для работы с SPDX-тегами (C89) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_tag.h"

#define MAX_LINE 4096

/* Проверяет, находится ли строка между REUSE-IgnoreStart и REUSE-IgnoreEnd */
static int is_ignore_start(const char *line) {
    return strstr(line, "REUSE-IgnoreStart") != NULL;
}

static int is_ignore_end(const char *line) {
    return strstr(line, "REUSE-IgnoreEnd") != NULL;
}

/* Пропускает пробелы и табуляции */
static const char *skip_whitespace(const char *p) {
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* Удаляет хвостовые пробелы и переводы строк */
static void trim_trailing(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r' ||
                       str[len-1] == ' ' || str[len-1] == '\t')) {
        str[--len] = '\0';
    }
}

int file_has_spdx_tag(const char *filename) {
    FILE *f;
    char line[MAX_LINE];
    int ignore = 0;

    f = fopen(filename, "r");
    if (!f) return 0;
    while (fgets(line, sizeof(line), f)) {
        if (ignore) {
            if (is_ignore_end(line)) ignore = 0;
            continue;
        }
        if (is_ignore_start(line)) {
            ignore = 1;
            continue;
        }
        if (strstr(line, "SPDX-License-Identifier:")) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

char *file_get_spdx_license(const char *filename) {
    FILE *f;
    char line[MAX_LINE];
    const char *needle = "SPDX-License-Identifier:";
    const size_t needle_len = strlen(needle);
    char *pos;
    char *result;
    char *start;
    size_t len;
    int ignore = 0;

    f = fopen(filename, "r");
    if (!f) return NULL;

    while (fgets(line, sizeof(line), f)) {
        if (ignore) {
            if (is_ignore_end(line)) ignore = 0;
            continue;
        }
        if (is_ignore_start(line)) {
            ignore = 1;
            continue;
        }
        pos = strstr(line, needle);
        if (pos) {
            start = (char *)skip_whitespace(pos + needle_len);
            trim_trailing(start);
            len = strlen(start);
            if (len > 0) {
                result = (char *)malloc(len + 1);
                if (!result) {
                    fclose(f);
                    return NULL;
                }
                memcpy(result, start, len);
                result[len] = '\0';
                fclose(f);
                return result;
            }
        }
    }
    fclose(f);
    return NULL;
}

char *file_get_spdx_copyright(const char *filename) {
    FILE *f;
    char line[MAX_LINE];
    char *result = NULL;
    size_t result_len = 0;
    int ignore = 0;

    f = fopen(filename, "r");
    if (!f) return NULL;

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char *start;
        size_t len;

        if (ignore) {
            if (is_ignore_end(line)) ignore = 0;
            continue;
        }
        if (is_ignore_start(line)) {
            ignore = 1;
            continue;
        }

        /* Пропускаем ведущие пробелы и распространённые символы комментариев */
        while (*p == ' ' || *p == '\t' || *p == '#') p++;
        if (*p == '/' && p[1] == '/') p += 2;
        else if (*p == '*') p++;
        else if (*p == ';') p++;
        else if (*p == '%') p++;
        else if (p[0] == '-' && p[1] == '-') p += 2;
        while (*p == ' ' || *p == '\t') p++;

        /* Проверяем три разрешённых способа указания копирайта */
        if (strncmp(p, "SPDX-FileCopyrightText:", 23) == 0) {
            p += 23;
            p = (char *)skip_whitespace(p);
            start = p;
        } else if (strncmp(p, "Copyright", 9) == 0 &&
                   (p[9] == ' ' || p[9] == '\t')) {
            p += 9;
            p = (char *)skip_whitespace(p);
            start = p;
        } else if ((unsigned char)*p == 0xC2 && (unsigned char)*(p+1) == 0xA9) {
            /* Символ © в UTF-8 */
            p += 2;
            p = (char *)skip_whitespace(p);
            start = p;
        } else {
            continue;
        }

        trim_trailing(start);
        len = strlen(start);
        if (len > 0) {
            char *new_result;
            size_t new_size = (result ? result_len + 1 + len + 1 : len + 1);
            new_result = (char *)realloc(result, new_size);
            if (!new_result) {
                free(result);
                fclose(f);
                return NULL;
            }
            result = new_result;
            if (result_len > 0) {
                result[result_len++] = '\n';
            }
            memcpy(result + result_len, start, len);
            result_len += len;
            result[result_len] = '\0';
        }
    }
    fclose(f);
    return result;
}
