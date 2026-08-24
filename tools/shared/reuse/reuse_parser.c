/* reuse_parser.c - реализация парсера REUSE.toml (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

static void parse_annotation(ReuseConfig *config, FILE *f, char *line) {
    Annotation *ann;
    char *p, *val, *start;

    ann = &config->annotations[config->annotation_count];
    memset(ann, 0, sizeof(Annotation));
    ann->precedence = strdup("override");
    ann->paths = NULL;
    ann->path_count = 0;

    while (fgets(line, 1024, f)) {
        p = trim(line);
        if (*p == '[') break;
        if (*p == '\0' || *p == '#') continue;

        if (starts_with(p, "path = ")) {
            val = p + 7;
            val = trim(val);
            if (*val == '[') {
                val++;
                while (*val && *val != ']') {
                    while (*val && isspace(*val)) val++;
                    if (*val == '"' || *val == '\'') {
                        start = ++val;
                        while (*val && *val != '"' && *val != '\'') val++;
                        if (*val) {
                            *val = '\0';
                            ann->paths = (char**)realloc(ann->paths,
                                (ann->path_count + 1) * sizeof(char*));
                            ann->paths[ann->path_count++] = strdup(start);
                            val++;
                        }
                    }
                    if (*val == ',') val++;
                }
            } else {
                val = strip_quotes(val);
                ann->paths = (char**)malloc(sizeof(char*));
                ann->paths[0] = strdup(val);
                ann->path_count = 1;
            }
        } else if (starts_with(p, "SPDX-License-Identifier = ")) {
            val = p + strlen("SPDX-License-Identifier = ");
            val = trim(val);
            ann->license = strdup(strip_quotes(val));
        } else if (starts_with(p, "SPDX-FileCopyrightText = ")) {
            val = p + strlen("SPDX-FileCopyrightText = ");
            val = trim(val);
            ann->copyright = strdup(strip_quotes(val));
        } else if (starts_with(p, "precedence = ")) {
            val = p + strlen("precedence = ");
            val = trim(val);
            free(ann->precedence);
            ann->precedence = strdup(strip_quotes(val));
        }
    }
    config->annotation_count++;
}

ReuseConfig* parse_reuse_toml(const char *filename) {
    FILE *f;
    ReuseConfig *config;
    char line[1024];
    int in_default;
    char *p, *s, *val;

    f = fopen(filename, "r");
    if (!f) return NULL;

    config = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    config->annotations = (Annotation*)malloc(100 * sizeof(Annotation));
    config->annotation_count = 0;
    config->source_dir = strdup(filename);
    p = strrchr(config->source_dir, '/');
    if (p) *p = '\0';

    in_default = 0;

    while (fgets(line, sizeof(line), f)) {
        s = trim(line);
        if (*s == '\0' || *s == '#') continue;

        if (strcmp(s, "[default]") == 0) {
            in_default = 1;
            continue;
        }
        if (strcmp(s, "[[annotations]]") == 0) {
            in_default = 0;
            parse_annotation(config, f, line);
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

    d = strdup(start_dir);
    p = d + strlen(d);

    while (1) {
        snprintf(path, sizeof(path), "%s/REUSE.toml", d);
        f = fopen(path, "r");
        if (f) { fclose(f); free(d); return path; }
        while (p > d && *p != '/') p--;
        if (p == d) break;
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
