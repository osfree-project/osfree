/* reuse_parser.c - реализация парсера REUSE.toml (C89)
 *
 * Приведён к спецификации REUSE 3.2:
 *   - нет секции [default] (значения по умолчанию задаются аннотацией
 *     с паттерном "**");
 *   - matches_pattern корректно обрабатывает * и **.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __LINUX__
#define PATH_SEPARATOR '/'
#else
#define PATH_SEPARATOR '\\'
#endif

#include "reuse_parser.h"

static char* trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static char* strip_quotes(char *str) {
    char *end;
    if (str[0] == '"' || str[0] == '\'') {
        str++;
        end = str + strlen(str) - 1;
        if (*end == '"' || *end == '\'') *end = '\0';
    }
    return str;
}

static int starts_with(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

static void add_path(Annotation *ann, const char *value) {
    ann->paths = (char**)realloc(ann->paths,
                                 (ann->path_count + 1) * sizeof(char*));
    ann->paths[ann->path_count++] = strdup(value);
}

static void join_path(char *dest, size_t dest_size,
                      const char *dir, const char *name) {
    size_t len = strlen(dir);
    if (len == 0) {
        snprintf(dest, dest_size, "%s", name);
    } else {
        snprintf(dest, dest_size, "%s%c%s", dir, PATH_SEPARATOR, name);
    }
}

/* ------------------------------------------------------------------ */
/* Соответствие паттернов                                              */
/* ------------------------------------------------------------------ */

/* '*' соответствует нулю и более символам, кроме '/' */
static int match_component(const char *pat, const char *str) {
    if (*pat == '\0') return *str == '\0';
    if (*pat == '*') {
        while (1) {
            if (match_component(pat + 1, str)) return 1;
            if (*str == '\0' || *str == '/') return 0;
            str++;
        }
    }
    if (*str == '\0' || *str == '/') return 0;
    if (*pat != *str) return 0;
    return match_component(pat + 1, str + 1);
}

/* Полное сопоставление с поддержкой ** */
static int match_path(const char *pat, const char *path) {
    const char *pat_slash;
    const char *path_slash;

    if (pat[0] == '*' && pat[1] == '*' &&
        (pat[2] == '\0' || pat[2] == '/')) {
        const char *rest = (pat[2] == '/') ? pat + 3 : pat + 2;
        if (match_path(rest, path)) return 1;
        while ((path_slash = strchr(path, '/')) != NULL) {
            path = path_slash + 1;
            if (match_path(rest, path)) return 1;
        }
        return 0;
    }

    pat_slash  = strchr(pat, '/');
    path_slash = strchr(path, '/');

    if (!pat_slash) {
        if (path_slash) return 0;
        return match_component(pat, path);
    }
    if (!path_slash) return 0;

    {
        size_t plen = (size_t)(pat_slash - pat);
        size_t flen = (size_t)(path_slash - path);
        char pat_seg[256];
        char path_seg[256];

        if (plen >= sizeof(pat_seg) || flen >= sizeof(path_seg)) return 0;
        memcpy(pat_seg, pat, plen);
        pat_seg[plen] = '\0';
        memcpy(path_seg, path, flen);
        path_seg[flen] = '\0';

        if (!match_component(pat_seg, path_seg)) return 0;
        return match_path(pat_slash + 1, path_slash + 1);
    }
}

int matches_pattern(const char *pattern, const char *filename) {
    return match_path(pattern, filename);
}

/* ------------------------------------------------------------------ */
/* Разбор REUSE.toml                                                   */
/* ------------------------------------------------------------------ */

ReuseConfig* parse_reuse_toml(const char *filename) {
    FILE *f;
    ReuseConfig *config;
    char line[1024];
    char *s, *p, *val, *start;
    int in_path_array = 0;
    Annotation *current_ann = NULL;

    f = fopen(filename, "r");
    if (!f) return NULL;

    config = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    config->annotations = (Annotation*)malloc(100 * sizeof(Annotation));
    config->annotation_count = 0;
    config->source_dir = strdup(filename);

    p = strrchr(config->source_dir, '/');
#ifdef _WIN32
    {
        char *backslash = strrchr(config->source_dir, '\\');
        if (backslash && (!p || backslash > p)) p = backslash;
    }
#endif
    if (p) *p = '\0';

    while (fgets(line, sizeof(line), f)) {
        s = trim(line);
        if (*s == '\0' || *s == '#') continue;

        if (strcmp(s, "[[annotations]]") == 0) {
            current_ann = &config->annotations[config->annotation_count];
            memset(current_ann, 0, sizeof(Annotation));
            current_ann->precedence = strdup("override");
            config->annotation_count++;
            in_path_array = 0;
            continue;
        }

        if (current_ann == NULL) continue;

        if (in_path_array) {
            p = s;
            while (*p) {
                while (isspace((unsigned char)*p)) p++;
                if (*p == ']') {
                    in_path_array = 0;
                    p++;
                    break;
                }
                if (*p == '"' || *p == '\'') {
                    start = ++p;
                    while (*p && *p != '"' && *p != '\'') p++;
                    if (*p) {
                        *p = '\0';
                        add_path(current_ann, start);
                        p++;
                    }
                }
                if (*p == ',') p++;
            }
            continue;
        }

        if (starts_with(s, "path = ")) {
            val = s + 7;
            val = trim(val);
            if (*val == '[') {
                in_path_array = 1;
                val++;
                while (*val) {
                    while (isspace((unsigned char)*val)) val++;
                    if (*val == ']') {
                        in_path_array = 0;
                        val++;
                        break;
                    }
                    if (*val == '"' || *val == '\'') {
                        start = ++val;
                        while (*val && *val != '"' && *val != '\'') val++;
                        if (*val) {
                            *val = '\0';
                            add_path(current_ann, start);
                            val++;
                        }
                    }
                    if (*val == ',') val++;
                }
            } else {
                val = strip_quotes(val);
                add_path(current_ann, val);
            }
        } else if (starts_with(s, "SPDX-License-Identifier = ")) {
            val = s + strlen("SPDX-License-Identifier = ");
            val = trim(val);
            current_ann->license = strdup(strip_quotes(val));
        } else if (starts_with(s, "SPDX-FileCopyrightText = ")) {
            val = s + strlen("SPDX-FileCopyrightText = ");
            val = trim(val);
            current_ann->copyright = strdup(strip_quotes(val));
        } else if (starts_with(s, "precedence = ")) {
            val = s + strlen("precedence = ");
            val = trim(val);
            free(current_ann->precedence);
            current_ann->precedence = strdup(strip_quotes(val));
        }
    }

    fclose(f);
    return config;
}

void free_reuse_config(ReuseConfig *config) {
    int i, j;
    if (!config) return;
    free(config->source_dir);
    for (i = 0; i < config->annotation_count; i++) {
        for (j = 0; j < config->annotations[i].path_count; j++)
            free(config->annotations[i].paths[j]);
        free(config->annotations[i].paths);
        free(config->annotations[i].license);
        free(config->annotations[i].copyright);
        free(config->annotations[i].precedence);
    }
    free(config->annotations);
    free(config);
}

char* find_reuse_toml_upwards(const char *start_dir) {
    static char path[1024];
    char *d, *p;
    FILE *f;
    size_t len;

    d = strdup(start_dir);
    len = strlen(d);
    if (len > 0 && (d[len - 1] == '/' || d[len - 1] == '\\'))
        d[len - 1] = '\0';

    while (1) {
        join_path(path, sizeof(path), d, "REUSE.toml");
        f = fopen(path, "r");
        if (f) {
            fclose(f);
            free(d);
            return path;
        }

        p = strrchr(d, '/');
#ifdef _WIN32
        {
            char *backslash = strrchr(d, '\\');
            if (backslash && (!p || backslash > p)) p = backslash;
        }
#endif
        if (!p) break;
        *p = '\0';
    }

    free(d);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Поиск лицензии/копирайта для файла                                  */
/* ------------------------------------------------------------------ */

static int pick_best(Annotation **matches, int count) {
    int best = 0, i;
    if (count <= 1) return 0;
    for (i = 1; i < count; i++) {
        if (matches[i]->precedence &&
            strcmp(matches[i]->precedence, "closest") == 0) {
            best = i;
            break;
        }
        if (matches[i]->path_count > 0 && matches[best]->path_count > 0 &&
            strlen(matches[i]->paths[0]) > strlen(matches[best]->paths[0]))
            best = i;
    }
    return best;
}

static int collect_matches(ReuseConfig *config, const char *filename,
                           Annotation **matches, int max_matches) {
    int i, j;
    int count = 0;
    if (!config) return 0;
    for (i = 0; i < config->annotation_count; i++) {
        for (j = 0; j < config->annotations[i].path_count; j++) {
            if (matches_pattern(config->annotations[i].paths[j], filename)) {
                if (count < max_matches)
                    matches[count++] = &config->annotations[i];
                break;
            }
        }
    }
    return count;
}

const char* find_license_for_file(ReuseConfig *config, const char *filename) {
    Annotation *matches[100];
    int count = collect_matches(config, filename, matches, 100);
    if (count == 0) return NULL;
    return matches[pick_best(matches, count)]->license;
}

const char* find_copyright_for_file(ReuseConfig *config, const char *filename) {
    Annotation *matches[100];
    int count = collect_matches(config, filename, matches, 100);
    if (count == 0) return NULL;
    return matches[pick_best(matches, count)]->copyright;
}
