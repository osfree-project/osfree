/* spdx_tag.c - реализация функций для работы с SPDX-тегами (C89) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_tag.h"

/* Увеличен с 4096, чтобы длинные copyright-выражения не обрезались. */
#define MAX_LINE 16384

/* Пропускает UTF-8 BOM в начале буфера. */
static const char *skip_bom(const char *p) {
    if ((unsigned char)p[0] == 0xEF &&
        (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) {
        return p + 3;
    }
    return p;
}

/* Пропускает отступ и маркер комментария. Возвращает указатель
 * на первое содержательное содержимое строки. */
static const char *skip_comment_prefix(const char *p) {
    while (*p == ' ' || *p == '\t') p++;
    if (p[0] == '/' && p[1] == '/') { p += 2; }
    else if (p[0] == '/' && p[1] == '*') { p += 2; }
    else if (p[0] == '#') { p++; }
    else if (p[0] == ';') { p++; }
    else if (p[0] == '%') { p++; }
    else if (p[0] == '-' && p[1] == '-') { p += 2; }
    else if (p[0] == '*') { p++; }   /* продолжение блочного комментария */
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* Проверяет, начинается ли содержательная часть строки с
 * REUSE-IgnoreStart или REUSE-IgnoreEnd. После маркера должен идти
 * пробел, таб, перевод строки или конец. Возвращает 1, 2 или 0. */
static int line_ignore_marker(const char *line) {
    const char *p = skip_comment_prefix(line);
    if (strncmp(p, "REUSE-IgnoreStart", 17) == 0) {
        char c = p[17];
        if (c == '\0' || c == ' ' || c == '\t' ||
            c == '\n' || c == '\r')
            return 1;
    }
    if (strncmp(p, "REUSE-IgnoreEnd", 15) == 0) {
        char c = p[15];
        if (c == '\0' || c == ' ' || c == '\t' ||
            c == '\n' || c == '\r')
            return 2;
    }
    return 0;
}

/* Удаляет хвостовые пробелы, переводы строк и маркеры закрытия
 * блочных и строчных комментариев. */
static void strip_trailing_markers(char *str) {
    size_t len = strlen(str);
    while (len > 0) {
        if (len >= 2 && str[len-2] == '*' && str[len-1] == '/') {
            len -= 2;
        } else if (len >= 2 && str[len-2] == '/' && str[len-1] == '/') {
            len -= 2;
        } else if (str[len-1] == ' ' || str[len-1] == '\t' ||
                   str[len-1] == '\n' || str[len-1] == '\r') {
            len -= 1;
        } else {
            break;
        }
    }
    str[len] = '\0';
}

int file_has_spdx_tag(const char *filename) {
    FILE *f;
    char line[MAX_LINE];
    int ignore = 0;
    int first_line = 1;

    f = fopen(filename, "r");
    if (!f) return 0;
    while (fgets(line, sizeof(line), f)) {
        char *work = line;
        int marker;

        if (first_line) {
            work = (char *)skip_bom(line);
            first_line = 0;
        }

        marker = line_ignore_marker(work);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        if (strstr(work, "SPDX-License-Identifier:")) {
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
    int first_line = 1;

    f = fopen(filename, "r");
    if (!f) return NULL;

    while (fgets(line, sizeof(line), f)) {
        char *work = line;
        int marker;

        if (first_line) {
            work = (char *)skip_bom(line);
            first_line = 0;
        }

        marker = line_ignore_marker(work);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        pos = strstr(work, needle);
        if (pos) {
            start = (char *)skip_comment_prefix(pos + needle_len);
            strip_trailing_markers(start);
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
    int first_line = 1;

    f = fopen(filename, "r");
    if (!f) return NULL;

    while (fgets(line, sizeof(line), f)) {
        char *p;
        char *start;
        size_t len;
        int marker;

        if (first_line) {
            p = (char *)skip_bom(line);
            first_line = 0;
        } else {
            p = line;
        }

        marker = line_ignore_marker(p);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        /* Пропускаем отступ и маркер комментария */
        p = (char *)skip_comment_prefix(p);

        /* Три разрешённых способа указания копирайта */
        if (strncmp(p, "SPDX-FileCopyrightText:", 23) == 0) {
            p += 23;
            while (*p == ' ' || *p == '\t') p++;
            start = p;
        } else if (strncmp(p, "Copyright", 9) == 0 &&
                   (p[9] == ' ' || p[9] == '\t')) {
            p += 9;
            while (*p == ' ' || *p == '\t') p++;
            start = p;
        } else if ((unsigned char)*p == 0xC2 &&
                   (unsigned char)*(p+1) == 0xA9) {
            /* © в UTF-8 */
            p += 2;
            while (*p == ' ' || *p == '\t') p++;
            start = p;
        } else {
            continue;
        }

        strip_trailing_markers(start);
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
