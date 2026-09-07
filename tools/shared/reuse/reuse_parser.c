/* reuse_parser.c - реализаци€ парсера REUSE.toml (C89, исправлено) */

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
    ann->paths = (char**)realloc(ann->paths, (ann->path_count + 1) * sizeof(char*));
    ann->paths[ann->path_count++] = strdup(value);
}

static void join_path(char *dest, size_t dest_size, const char *dir, const char *name) {
    size_t len = strlen(dir);
    if (len == 0) {
        snprintf(dest, dest_size, "%s", name);
    } else if (dir[len-1] == '/' || dir[len-1] == '\\') {
        snprintf(dest, dest_size, "%s%c%s", dir, PATH_SEPARATOR, name);
    } else {
        snprintf(dest, dest_size, "%s%c%s", dir, PATH_SEPARATOR, name);
    }
}

ReuseConfig* parse_reuse_toml(const char *filename) {
    FILE *f;
    ReuseConfig *config;
    char line[1024];
    char *s, *p, *val, *start;
    int in_default = 0;
    int in_path_array = 0;
    Annotation *current_ann = NULL;

    f = fopen(filename, "r");
    if (!f) return NULL;

    config = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    config->annotations = (Annotation*)malloc(100 * sizeof(Annotation));
    config->annotation_count = 0;
    config->source_dir = strdup(filename);

    /* ќпредел€ем каталог, содержащий REUSE.toml */
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

        if (strcmp(s, "[default]") == 0) {
            in_default = 1;
            current_ann = NULL;
            in_path_array = 0;
            continue;
        }

        if (strcmp(s, "[[annotations]]") == 0) {
            in_default = 0;
            current_ann = &config->annotations[config->annotation_count];
            memset(current_ann, 0, sizeof(Annotation));
            current_ann->precedence = strdup("override");
            current_ann->paths = NULL;
            current_ann->path_count = 0;
            config->annotation_count++;
            in_path_array = 0;
            continue;
        }

        if (in_default) {
            if (starts_with(s, "SPDX-License-Identifier = ")) {
                val = s + strlen("SPDX-License-Identifier = ");
                val = trim(val);
                config->default_license = strdup(strip_quotes(val));
            } else if (starts_with(s, "SPDX-FileCopyrightText = ")) {
                val = s + strlen("SPDX-FileCopyrightText = ");
                val = trim(val);
                config->default_copyright = strdup(strip_quotes(val));
            }
            continue;
        }

        if (current_ann != NULL) {
            if (in_path_array) {
                /* ѕродолжаем разбор многострочного массива путей */
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
    }

    fclose(f);
    return config;
}

void free_reuse_config(ReuseConfig *config) {
    int i, j;
    if (!config) return;
    free(config->default_license);
    free(config->default_copyright);
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
    if (len > 0 && (d[len-1] == '/' || d[len-1] == '\\')) {
        d[len-1] = '\0';
    }

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

int matches_pattern(const char *pattern, const char *filename) {
    size_t plen, flen, prefix_len, suffix_len;
    char *star;

    plen = strlen(pattern);
    flen = strlen(filename);

    star = strchr(pattern, '*');
    if (star) {
        prefix_len = star - pattern;
        if (strncmp(pattern, filename, prefix_len) == 0) {
            if (*(star + 1)) {
                suffix_len = strlen(star + 1);
                if (flen >= suffix_len)
                    return strcmp(filename + flen - suffix_len, star + 1) == 0;
                return 0;
            }
            return 1;
        }
        return 0;
    }
    return strcmp(pattern, filename) == 0;
}

const char* find_license_for_file(ReuseConfig *config, const char *filename) {
    Annotation *matches[100];
    int match_count, i, j, best;

    if (!config) return NULL;

    match_count = 0;
    for (i = 0; i < config->annotation_count; i++) {
        for (j = 0; j < config->annotations[i].path_count; j++) {
            if (matches_pattern(config->annotations[i].paths[j], filename)) {
                matches[match_count++] = &config->annotations[i];
                break;
            }
        }
    }

    if (match_count == 0)
        return config->default_license;

    if (match_count > 1) {
        best = 0;
        for (i = 1; i < match_count; i++) {
            if (strcmp(matches[i]->precedence, "closest") == 0) {
                best = i;
                break;
            }
            if (strlen(matches[i]->paths[0]) > strlen(matches[best]->paths[0]))
                best = i;
        }
        return matches[best]->license;
    }
    return matches[0]->license;
}

const char* find_copyright_for_file(ReuseConfig *config, const char *filename) {
    Annotation *matches[100];
    int match_count, i, j, best;

    if (!config) return NULL;

    match_count = 0;
    for (i = 0; i < config->annotation_count; i++) {
        for (j = 0; j < config->annotations[i].path_count; j++) {
            if (matches_pattern(config->annotations[i].paths[j], filename)) {
                matches[match_count++] = &config->annotations[i];
                break;
            }
        }
    }

    if (match_count == 0)
        return config->default_copyright;

    if (match_count > 1) {
        best = 0;
        for (i = 1; i < match_count; i++) {
            if (strcmp(matches[i]->precedence, "closest") == 0) {
                best = i;
                break;
            }
            if (strlen(matches[i]->paths[0]) > strlen(matches[best]->paths[0]))
                best = i;
        }
        return matches[best]->copyright;
    }
    return matches[0]->copyright;
}
