/* spdx_tag.c - разбор SPDX-тегов и сниппетов из файлов (C89) */
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

/* Пропускает отступ и маркер комментария. */
static const char *skip_comment_prefix(const char *p) {
    while (*p == ' ' || *p == '\t') p++;
    if (p[0] == '/' && p[1] == '/') { p += 2; }
    else if (p[0] == '/' && p[1] == '*') { p += 2; }
    else if (p[0] == '#') { p++; }
    else if (p[0] == ';') { p++; }
    else if (p[0] == '%') { p++; }
    else if (p[0] == '-' && p[1] == '-') { p += 2; }
    else if (p[0] == '*') { p++; }
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* Проверяет, начинается ли содержательная часть строки с
 * REUSE-IgnoreStart или REUSE-IgnoreEnd. Возвращает 1, 2 или 0. */
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

/* Проверяет, начинается ли содержательная часть строки с указанного
 * тега (tag может оканчиваться двоеточием). После тега должен идти
 * пробел, таб, ':', перевод строки или конец. */
static int line_starts_with_tag(const char *line, const char *tag) {
    size_t tlen = strlen(tag);
    const char *p = skip_comment_prefix(line);
    if (strncmp(p, tag, tlen) != 0) return 0;
    if (p[tlen] == ':' && tag[tlen - 1] != ':') return 1;
    if (p[tlen] == '\0' || p[tlen] == ' ' || p[tlen] == '\t' ||
        p[tlen] == '\n' || p[tlen] == '\r')
        return 1;
    return 0;
}

/* Возвращает указатель на значение после тега (пропустив двоеточие
 * и пробелы). Если тег не найден или без двоеточия - NULL. */
static const char *tag_value(const char *line, const char *tag_with_colon) {
    size_t tlen = strlen(tag_with_colon);
    const char *p = skip_comment_prefix(line);
    if (strncmp(p, tag_with_colon, tlen) != 0) return NULL;
    p += tlen;
    while (*p == ' ' || *p == '\t') p++;
    return p;
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

/* ------------------------------------------------------------------ */
/* Список сниппетов                                                    */
/* ------------------------------------------------------------------ */

void tag_snippets_init(TagSnippetList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void tag_snippets_free(TagSnippetList *list) {
    int i;
    for (i = 0; i < list->count; i++) {
        free(list->items[i].license);
        free(list->items[i].copyright);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static TagSnippet *tag_snippets_add(TagSnippetList *list) {
    TagSnippet *s;
    if (list->count >= list->capacity) {
        int new_cap = list->capacity ? list->capacity * 2 : 4;
        TagSnippet *ni = (TagSnippet*)realloc(list->items,
            (size_t)new_cap * sizeof(TagSnippet));
        if (!ni) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        list->items = ni;
        list->capacity = new_cap;
    }
    s = &list->items[list->count++];
    memset(s, 0, sizeof(*s));
    return s;
}

/* ------------------------------------------------------------------ */
/* Публичные функции                                                   */
/* ------------------------------------------------------------------ */

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

        p = (char *)skip_comment_prefix(p);

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

/* Дописать строку к накопителю через '\n'. */
static char *append_line(char *acc, size_t *acc_len, const char *line) {
    size_t llen = strlen(line);
    char *na;
    size_t new_size;

    if (llen == 0) return acc;

    new_size = (*acc_len ? *acc_len + 1 : 0) + llen + 1;
    na = (char*)realloc(acc, new_size);
    if (!na) return acc;

    if (*acc_len > 0) {
        na[(*acc_len)++] = '\n';
    }
    memcpy(na + *acc_len, line, llen);
    *acc_len += llen;
    na[*acc_len] = '\0';
    return na;
}

int file_get_snippets(const char *filename, TagSnippetList *out) {
    FILE *f;
    char line[MAX_LINE];
    int lineno = 0;
    int first_line = 1;
    int in_snippet = 0;
    int snippet_start_line = 0;
    char *snippet_license = NULL;
    char *snippet_copyright = NULL;
    size_t snippet_copyright_len = 0;
    int ignore = 0;

    tag_snippets_init(out);

    f = fopen(filename, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        char *work = line;
        int marker;

        lineno++;

        if (first_line) {
            work = (char *)skip_bom(line);
            first_line = 0;
        }

        marker = line_ignore_marker(work);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        if (line_starts_with_tag(work, "SPDX-SnippetBegin")) {
            if (in_snippet) {
                fprintf(stderr,
                        "Error: %s:%d: nested SPDX-SnippetBegin\n",
                        filename, lineno);
                free(snippet_license);
                free(snippet_copyright);
                tag_snippets_free(out);
                fclose(f);
                return -1;
            }
            in_snippet = 1;
            snippet_start_line = lineno;
            free(snippet_license); snippet_license = NULL;
            free(snippet_copyright); snippet_copyright = NULL;
            snippet_copyright_len = 0;
            continue;
        }

        if (line_starts_with_tag(work, "SPDX-SnippetEnd")) {
            TagSnippet *s;
            if (!in_snippet) {
                fprintf(stderr,
                        "Error: %s:%d: SPDX-SnippetEnd without matching "
                        "SPDX-SnippetBegin\n", filename, lineno);
                tag_snippets_free(out);
                fclose(f);
                return -1;
            }
            s = tag_snippets_add(out);
            s->line_start = snippet_start_line;
            s->line_end   = lineno;
            s->license    = snippet_license;
            s->copyright  = snippet_copyright;
            snippet_license = NULL;
            snippet_copyright = NULL;
            snippet_copyright_len = 0;
            in_snippet = 0;
            continue;
        }

        if (!in_snippet) continue;

        /* Внутри сниппета: SPDX-SnippetCopyrightText и SPDX-License-Identifier */
        {
            const char *val;

            val = tag_value(work, "SPDX-SnippetCopyrightText:");
            if (val) {
                char tmp[MAX_LINE];
                size_t l = strlen(val);
                if (l >= sizeof(tmp)) l = sizeof(tmp) - 1;
                memcpy(tmp, val, l);
                tmp[l] = '\0';
                strip_trailing_markers(tmp);
                snippet_copyright = append_line(snippet_copyright,
                                                &snippet_copyright_len,
                                                tmp);
                continue;
            }

            val = tag_value(work, "SPDX-License-Identifier:");
            if (val) {
                char tmp[MAX_LINE];
                size_t l = strlen(val);
                if (l >= sizeof(tmp)) l = sizeof(tmp) - 1;
                memcpy(tmp, val, l);
                tmp[l] = '\0';
                strip_trailing_markers(tmp);
                free(snippet_license);
                snippet_license = (char*)malloc(strlen(tmp) + 1);
                if (snippet_license) strcpy(snippet_license, tmp);
                continue;
            }
        }
    }

    fclose(f);

    if (in_snippet) {
        fprintf(stderr,
                "Error: %s: unclosed SPDX-SnippetBegin at line %d\n",
                filename, snippet_start_line);
        free(snippet_license);
        free(snippet_copyright);
        tag_snippets_free(out);
        return -1;
    }

    return 0;
}
