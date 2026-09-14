/* reuse_parser.c - парсер REUSE.toml с поддержкой иерархии (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef __LINUX__
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#endif

#include "reuse_parser.h"

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

static char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static char *strip_quotes(char *str) {
    char *end;
    if (str[0] == '"' || str[0] == '\'') {
        char q = str[0];
        str++;
        end = str + strlen(str) - 1;
        if (*end == q) *end = '\0';
    }
    return str;
}

static int starts_with(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

/* Удаляет inline-комментарий (# вне кавычек). Учитывает экранирование
 * внутри кавычек: \" и \'. */
static void strip_inline_comment(char *line) {
    char *p = line;
    int in_dq = 0;
    int in_sq = 0;
    while (*p) {
        char c = *p;
        if (in_dq) {
            if (c == '\\' && p[1] != '\0') { p += 2; continue; }
            if (c == '"') in_dq = 0;
        } else if (in_sq) {
            if (c == '\\' && p[1] != '\0') { p += 2; continue; }
            if (c == '\'') in_sq = 0;
        } else {
            if (c == '"') in_dq = 1;
            else if (c == '\'') in_sq = 1;
            else if (c == '#') { *p = '\0'; return; }
        }
        p++;
    }
}

/* Сравнение символов пути: регистрозависимо на Linux, регистронезависимо
 * на Windows. */
static int path_char_eq(int a, int b) {
#ifdef _WIN32
    return tolower((unsigned char)a) == tolower((unsigned char)b);
#else
    return (unsigned char)a == (unsigned char)b;
#endif
}

static int path_prefix_eq(const char *s, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (!s[i]) return 0;
        if (!path_char_eq(s[i], prefix[i])) return 0;
    }
    return 1;
}

static void add_path(Annotation *ann, const char *value) {
    char **na = (char**)realloc(ann->paths,
        (size_t)(ann->path_count + 1) * sizeof(char*));
    if (!na) { fprintf(stderr, "OOM\n"); exit(EXIT_FAILURE); }
    ann->paths = na;
    ann->paths[ann->path_count++] = dup_str(value);
}

/* ------------------------------------------------------------------ */
/* Сопоставление паттернов (* и **)                                    */
/* ------------------------------------------------------------------ */

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

static int match_path(const char *pat, const char *path) {
    const char *pat_slash;
    const char *path_slash;

    if (pat[0] == '*' && pat[1] == '*') {
        if (pat[2] == '\0') return 1;
        if (pat[2] == '/') {
            const char *rest = pat + 3;
            if (match_path(rest, path)) return 1;
            while ((path_slash = strchr(path, '/')) != NULL) {
                path = path_slash + 1;
                if (match_path(rest, path)) return 1;
            }
            return 0;
        }
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

/* Возвращает путь filename относительно cfg->source_dir.
 * Проверяет, что совпадение префикса — на границе сегмента. */
static const char *rel_to_config(const ReuseConfig *cfg, const char *filename) {
    const char *base = cfg->source_dir;
    size_t blen;
    if (!base || !base[0]) return filename;
    blen = strlen(base);
    if (path_prefix_eq(filename, base, blen)) {
        const char *rest = filename + blen;
        if (*rest == '\0') return rest;
        if (*rest != '/' && *rest != '\\') return filename;
        while (*rest == '/' || *rest == '\\') rest++;
        return rest;
    }
    return filename;
}

/* ------------------------------------------------------------------ */
/* Разбор TOML-массива строк: ["a", "b", "c"]                          */
/* ------------------------------------------------------------------ */

static char *parse_toml_string_array(const char *val, const char *sep) {
    size_t cap = 64;
    size_t len = 0;
    size_t sep_len = strlen(sep);
    char *out;
    const char *p;

    if (!val || *val != '[') return NULL;
    out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';

    p = val + 1;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ']') break;
        if (*p == '\0') { free(out); return NULL; }
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            const char *start = p;
            size_t seg_len;
            while (*p && *p != quote) p++;
            if (*p != quote) { free(out); return NULL; }
            seg_len = (size_t)(p - start);
            p++;

            if (len > 0) {
                if (len + sep_len + 1 > cap) {
                    size_t ncap = cap * 2 + sep_len + seg_len;
                    char *no = (char*)realloc(out, ncap);
                    if (!no) { free(out); return NULL; }
                    out = no; cap = ncap;
                }
                memcpy(out + len, sep, sep_len);
                len += sep_len;
            }
            if (len + seg_len + 1 > cap) {
                size_t ncap = cap * 2 + seg_len;
                char *no = (char*)realloc(out, ncap);
                if (!no) { free(out); return NULL; }
                out = no; cap = ncap;
            }
            memcpy(out + len, start, seg_len);
            len += seg_len;
            out[len] = '\0';
        } else {
            free(out);
            return NULL;
        }
        while (*p == ' ' || *p == '\t') p++;
        if (*p == ',') { p++; continue; }
        if (*p == ']') break;
    }
    return out;
}

/* ------------------------------------------------------------------ */
/* Разбор REUSE.toml                                                   */
/* ------------------------------------------------------------------ */

static int is_known_annotation_key(const char *s, int *consumed) {
    static const char *known[] = {
        "path",
        "SPDX-License-Identifier",
        "SPDX-FileCopyrightText",
        "precedence",
        "SPDX-FileContributor",
        "SPDX-PackageName",
        "SPDX-PackageSupplier",
        "SPDX-PackageDownloadLocation",
        "SPDX-PackageComment",
        NULL
    };
    int i;
    for (i = 0; known[i]; i++) {
        size_t len = strlen(known[i]);
        if (strncmp(s, known[i], len) == 0) {
            const char *p = s + len;
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '=') {
                *consumed = 1;
                return 1;
            }
        }
    }
    *consumed = 0;
    return 0;
}

ReuseConfig* parse_reuse_toml(const char *filename) {
    FILE *f;
    ReuseConfig *config;
    char line[1024];
    char *s, *p, *val, *start;
    int in_path_array = 0;
    Annotation *current_ann = NULL;
    int order_counter = 0;
    int seen_version = 0;
    int warned_unknown = 0;
    int annotation_capacity = 16;

    f = fopen(filename, "r");
    if (!f) return NULL;

    config = (ReuseConfig*)calloc(1, sizeof(ReuseConfig));
    if (!config) { fclose(f); return NULL; }

    config->annotations = (Annotation*)malloc(
        (size_t)annotation_capacity * sizeof(Annotation));
    if (!config->annotations) {
        free(config);
        fclose(f);
        return NULL;
    }
    config->annotation_count = 0;
    config->source_dir = dup_str(filename);
    if (!config->source_dir) {
        free(config->annotations);
        free(config);
        fclose(f);
        return NULL;
    }

    p = strrchr(config->source_dir, '/');
#ifdef _WIN32
    {
        char *backslash = strrchr(config->source_dir, '\\');
        if (backslash && (!p || backslash > p)) p = backslash;
    }
#endif
    if (p) *p = '\0';

    while (fgets(line, sizeof(line), f)) {
        /* Сначала отсекаем inline-комментарий, потом trim */
        strip_inline_comment(line);
        s = trim(line);
        if (*s == '\0') continue;

        if (strcmp(s, "[[annotations]]") == 0) {
            if (config->annotation_count >= annotation_capacity) {
                Annotation *na;
                annotation_capacity *= 2;
                na = (Annotation*)realloc(config->annotations,
                    (size_t)annotation_capacity * sizeof(Annotation));
                if (!na) {
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
                config->annotations = na;
            }
            current_ann = &config->annotations[config->annotation_count];
            memset(current_ann, 0, sizeof(Annotation));
            current_ann->precedence = REUSE_PRECEDENCE_CLOSEST;
            current_ann->order_in_file = order_counter++;
            config->annotation_count++;
            in_path_array = 0;
            continue;
        }

        if (starts_with(s, "version") && strchr(s, '=')) {
            val = strchr(s, '=');
            val++;
            val = trim(val);
            val = strip_quotes(val);
            if (strcmp(val, "1") != 0) {
                fprintf(stderr,
                        "Warning: %s: unsupported version '%s' "
                        "(expected 1)\n", filename, val);
            } else {
                seen_version = 1;
            }
            continue;
        }

        if (current_ann == NULL) {
            int consumed = 0;
            if (!is_known_annotation_key(s, &consumed) && !warned_unknown) {
                fprintf(stderr,
                        "Warning: %s: unknown key outside [[annotations]]: %s\n",
                        filename, s);
                warned_unknown = 1;
            }
            continue;
        }

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
                    char quote = *p;
                    start = ++p;
                    while (*p && *p != quote) p++;
                    if (*p == quote) {
                        *p = '\0';
                        add_path(current_ann, start);
                        p++;
                    }
                }
                if (*p == ',') p++;
            }
            continue;
        }

        if (starts_with(s, "path = ") || starts_with(s, "path=")) {
            char *eq = strchr(s, '=');
            val = eq + 1;
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
                        char quote = *val;
                        start = ++val;
                        while (*val && *val != quote) val++;
                        if (*val == quote) {
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
        } else if (starts_with(s, "SPDX-License-Identifier") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (current_ann->license) free(current_ann->license);
            if (*val == '[') {
                current_ann->license = parse_toml_string_array(val, " AND ");
            } else {
                val = strip_quotes(val);
                current_ann->license = dup_str(val);
            }
        } else if (starts_with(s, "SPDX-FileCopyrightText") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (current_ann->copyright) free(current_ann->copyright);
            if (*val == '[') {
                current_ann->copyright = parse_toml_string_array(val, "\n");
            } else {
                val = strip_quotes(val);
                current_ann->copyright = dup_str(val);
            }
        } else if (starts_with(s, "precedence") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            val = strip_quotes(val);
            if (strcmp(val, "closest") == 0)
                current_ann->precedence = REUSE_PRECEDENCE_CLOSEST;
            else if (strcmp(val, "aggregate") == 0)
                current_ann->precedence = REUSE_PRECEDENCE_AGGREGATE;
            else if (strcmp(val, "override") == 0)
                current_ann->precedence = REUSE_PRECEDENCE_OVERRIDE;
            else {
                fprintf(stderr,
                        "Warning: %s: unknown precedence '%s' "
                        "(expected closest, aggregate or override)\n",
                        filename, val);
                current_ann->precedence = REUSE_PRECEDENCE_CLOSEST;
            }
        } else {
            int consumed = 0;
            if (!is_known_annotation_key(s, &consumed)) {
                fprintf(stderr,
                        "Warning: %s: unknown key in [[annotations]]: %s\n",
                        filename, s);
            }
        }
    }

    fclose(f);

    if (!seen_version) {
        fprintf(stderr,
                "Warning: %s: missing 'version = 1' at the top\n", filename);
    }

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
    }
    free(config->annotations);
    free(config);
}

/* ------------------------------------------------------------------ */
/* Поиск одного REUSE.toml вверх                                       */
/* ------------------------------------------------------------------ */

static void join_path_str(char *dest, size_t dest_size,
                          const char *dir, const char *name) {
    size_t len = strlen(dir);
#ifdef __LINUX__
    if (len == 0) snprintf(dest, dest_size, "%s", name);
    else snprintf(dest, dest_size, "%s/%s", dir, name);
#else
    if (len == 0) snprintf(dest, dest_size, "%s", name);
    else snprintf(dest, dest_size, "%s\\%s", dir, name);
#endif
}

char* find_reuse_toml_upwards(const char *start_dir) {
    static char path[1024];
    char *d = dup_str(start_dir);
    char *p;
    FILE *f;
    size_t len;

    if (!d) return NULL;
    len = strlen(d);
    while (len > 1 && (d[len-1] == '/' || d[len-1] == '\\')) d[--len] = '\0';

    while (1) {
        join_path_str(path, sizeof(path), d, "REUSE.toml");
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
        if (p == d) break;
#ifdef _WIN32
        if (p == d + 2 && d[1] == ':') { p[1] = '\\'; p[2] = '\0'; break; }
#endif
        *p = '\0';
    }

    free(d);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Сбор всех REUSE.toml от корня до target_dir                         */
/* ------------------------------------------------------------------ */

static char *rel_from(const char *from, const char *to) {
    size_t flen = strlen(from);
    size_t tlen = strlen(to);
    if (flen > tlen) return NULL;
    if (!path_prefix_eq(to, from, flen)) return NULL;
    if (flen == tlen) return dup_str("");
    if (to[flen] != '/' && to[flen] != '\\') return NULL;
    return dup_str(to + flen + 1);
}

int reuse_find_all_tomls(const char *repo_root,
                         const char *target_dir,
                         SpdxStrList *out_paths) {
    char path[2048];
    FILE *f;
    char *rel;
    char current[2048];

    spdx_strlist_init(out_paths);

    if (!repo_root) {
        join_path_str(path, sizeof(path), target_dir, "REUSE.toml");
        f = fopen(path, "r");
        if (f) { fclose(f); spdx_strlist_add(out_paths, path); }
        return 0;
    }

    rel = rel_from(repo_root, target_dir);
    if (!rel) {
        join_path_str(path, sizeof(path), target_dir, "REUSE.toml");
        f = fopen(path, "r");
        if (f) { fclose(f); spdx_strlist_add(out_paths, path); }
        return 0;
    }

    /* Корневой REUSE.toml */
    join_path_str(path, sizeof(path), repo_root, "REUSE.toml");
    f = fopen(path, "r");
    if (f) { fclose(f); spdx_strlist_add(out_paths, path); }

    if (rel[0] == '\0') {
        free(rel);
        return 0;
    }

    strncpy(current, repo_root, sizeof(current) - 1);
    current[sizeof(current) - 1] = '\0';

    {
        size_t ri = 0, rlen = strlen(rel);
        while (ri < rlen) {
            size_t start = ri;
            size_t seglen;
            while (ri < rlen && rel[ri] != '/' && rel[ri] != '\\') ri++;
            seglen = ri - start;
            if (seglen > 0) {
                size_t clen = strlen(current);
                if (clen + 1 + seglen + 1 > sizeof(current)) {
                    fprintf(stderr,
                            "Warning: path too long, skipping REUSE.toml "
                            "below %s\n", current);
                    break;
                }
                current[clen] = '/';
                memcpy(current + clen + 1, rel + start, seglen);
                current[clen + 1 + seglen] = '\0';

                join_path_str(path, sizeof(path), current, "REUSE.toml");
                f = fopen(path, "r");
                if (f) { fclose(f); spdx_strlist_add(out_paths, path); }
            }
            if (ri < rlen) ri++;
        }
    }

    free(rel);
    return 0;
}

ReuseConfig **reuse_parse_all(const SpdxStrList *paths, int *out_count) {
    ReuseConfig **arr;
    int i, n = 0;

    *out_count = 0;
    if (!paths || paths->count == 0) return NULL;

    arr = (ReuseConfig**)malloc((size_t)paths->count * sizeof(ReuseConfig*));
    if (!arr) return NULL;

    for (i = 0; i < paths->count; i++) {
        ReuseConfig *c = parse_reuse_toml(paths->items[i]);
        if (!c) continue;
        c->depth = i;
        arr[n++] = c;
    }
    *out_count = n;
    return arr;
}

void reuse_free_all(ReuseConfig **configs, int count) {
    int i;
    if (!configs) return;
    for (i = 0; i < count; i++) free_reuse_config(configs[i]);
    free(configs);
}

/* ------------------------------------------------------------------ */
/* Резолвер с иерархией                                                */
/* ------------------------------------------------------------------ */

static int ann_better(const Annotation *a, int a_depth,
                      const Annotation *b, int b_depth) {
    if (a->precedence != b->precedence)
        return a->precedence > b->precedence;
    if (a_depth != b_depth)
        return a_depth > b_depth;
    return a->order_in_file > b->order_in_file;
}

static const Annotation *ann_find_match(ReuseConfig *cfg,
                                        const char *filename) {
    const char *rel = rel_to_config(cfg, filename);
    const Annotation *best = NULL;
    int i, j;
    for (i = 0; i < cfg->annotation_count; i++) {
        for (j = 0; j < cfg->annotations[i].path_count; j++) {
            if (matches_pattern(cfg->annotations[i].paths[j], rel)) {
                if (!best || ann_better(&cfg->annotations[i], cfg->depth,
                                        best, cfg->depth))
                    best = &cfg->annotations[i];
                break;
            }
        }
    }
    return best;
}

/* Склеивает уникальные идентификаторы из двух выражений через AND. */
static char *aggregate_licenses(const char *a, const char *b) {
    char *r;
    size_t alen, blen;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 8);
    if (!r) return NULL;
    snprintf(r, alen + blen + 8, "(%s) AND (%s)", a, b);
    return r;
}

static char *aggregate_copyrights(const char *a, const char *b) {
    char *r;
    size_t alen, blen;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 2);
    if (!r) return NULL;
    sprintf(r, "%s\n%s", a, b);
    return r;
}

int reuse_resolve_for_file(ReuseConfig **configs, int count,
                           const char *filename,
                           const char *in_license,
                           const char *in_copyright,
                           char **out_license,
                           char **out_copyright,
                           int *out_precedence,
                           int *out_has_reuse) {
    const Annotation *best_override = NULL;
    const Annotation *best_aggregate = NULL;
    const Annotation *best_closest = NULL;
    int best_override_depth = -1;
    int best_aggregate_depth = -1;
    int best_closest_depth = -1;
    int i;

    *out_license = NULL;
    *out_copyright = NULL;
    *out_precedence = 0;
    *out_has_reuse = 0;

    /* Ищем лучшие аннотации в каждой из трёх групп независимо. */
    for (i = 0; i < count; i++) {
        const Annotation *a = ann_find_match(configs[i], filename);
        if (!a) continue;
        if (a->precedence == REUSE_PRECEDENCE_OVERRIDE) {
            if (!best_override ||
                ann_better(a, configs[i]->depth,
                           best_override, best_override_depth)) {
                best_override = a;
                best_override_depth = configs[i]->depth;
            }
        } else if (a->precedence == REUSE_PRECEDENCE_AGGREGATE) {
            if (!best_aggregate ||
                ann_better(a, configs[i]->depth,
                           best_aggregate, best_aggregate_depth)) {
                best_aggregate = a;
                best_aggregate_depth = configs[i]->depth;
            }
        } else {
            if (!best_closest ||
                ann_better(a, configs[i]->depth,
                           best_closest, best_closest_depth)) {
                best_closest = a;
                best_closest_depth = configs[i]->depth;
            }
        }
    }

    /* 1. override — выигрывает у всего. */
    if (best_override) {
        *out_has_reuse = 1;
        *out_precedence = REUSE_PRECEDENCE_OVERRIDE;
        if (best_override->license) *out_license = dup_str(best_override->license);
        if (best_override->copyright) *out_copyright = dup_str(best_override->copyright);
        return 0;
    }

    /* 2. Собираем базу по closest-логике: in-file выигрывает у best_closest. */
    {
        const char *base_lic = NULL;
        const char *base_cop = NULL;
        int base_has_reuse = 0;

        if (best_closest) {
            base_has_reuse = 1;
            if (best_closest->license) base_lic = best_closest->license;
            if (best_closest->copyright) base_cop = best_closest->copyright;
        }
        if (in_license) base_lic = in_license;
        if (in_copyright) base_cop = in_copyright;

        /* 2a. Без aggregate — возвращаем closest-результат. */
        if (!best_aggregate) {
            if (base_lic || base_cop) {
                *out_has_reuse = base_has_reuse;
                *out_precedence = REUSE_PRECEDENCE_CLOSEST;
                if (base_lic) *out_license = dup_str(base_lic);
                if (base_cop) *out_copyright = dup_str(base_cop);
            }
            return 0;
        }

        /* 2b. С aggregate — добавляем его значения поверх closest-результата. */
        *out_has_reuse = 1;
        *out_precedence = REUSE_PRECEDENCE_AGGREGATE;

        if (base_lic && best_aggregate->license) {
            *out_license = aggregate_licenses(base_lic, best_aggregate->license);
        } else if (base_lic) {
            *out_license = dup_str(base_lic);
        } else if (best_aggregate->license) {
            *out_license = dup_str(best_aggregate->license);
        }

        if (base_cop && best_aggregate->copyright) {
            *out_copyright = aggregate_copyrights(base_cop, best_aggregate->copyright);
        } else if (base_cop) {
            *out_copyright = dup_str(base_cop);
        } else if (best_aggregate->copyright) {
            *out_copyright = dup_str(best_aggregate->copyright);
        }
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Совместимость со старым API                                         */
/* ------------------------------------------------------------------ */

const char* find_license_for_file(ReuseConfig *config, const char *filename) {
    const Annotation *a;
    if (!config) return NULL;
    a = ann_find_match(config, filename);
    return a ? a->license : NULL;
}

const char* find_copyright_for_file(ReuseConfig *config, const char *filename) {
    const Annotation *a;
    if (!config) return NULL;
    a = ann_find_match(config, filename);
    return a ? a->copyright : NULL;
}
