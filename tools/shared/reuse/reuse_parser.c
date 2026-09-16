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
    if (!na) { fprintf(stderr, "ERROR: out of memory\n"); exit(EXIT_FAILURE); }
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

/* Возвращает путь filename относительно cfg->source_dir. */
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
/* TOML-строки: escape-последовательности                              */
/*                                                                     */
/* TOML v1.0.0, раздел «String»:                                       */
/*   basic string   — двойные кавычки, поддерживают \b \t \n \f \r     */
/*                    \" \\ \uXXXX \UXXXXXXXX                          */
/*   literal string — одинарные кавычки, escape не раскрываются        */
/* ------------------------------------------------------------------ */

static int toml_hex_digit(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int toml_hex_n(const char *p, int n, unsigned long *out) {
    int i;
    unsigned long v = 0;
    for (i = 0; i < n; i++) {
        int d = toml_hex_digit((unsigned char)p[i]);
        if (d < 0) return 0;
        v = (v << 4) | (unsigned long)d;
    }
    *out = v;
    return 1;
}

static int toml_utf8_encode(unsigned long cp, char out[4]) {
    if (cp < 0x80) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

static int toml_buf_append(char **buf, size_t *cap, size_t *len,
                           const char *data, size_t n) {
    if (*len + n + 1 > *cap) {
        size_t ncap = *cap * 2 + n + 16;
        char *nb = (char*)realloc(*buf, ncap);
        if (!nb) return -1;
        *buf = nb;
        *cap = ncap;
    }
    memcpy(*buf + *len, data, n);
    *len += n;
    (*buf)[*len] = '\0';
    return 0;
}

static char *parse_toml_basic_string(const char *p, const char **end) {
    size_t cap = 64;
    size_t len = 0;
    char *out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';

    while (*p && *p != '"') {
        char c;
        if (*p == '\\') {
            p++;
            if (!*p) goto fail;
            switch (*p) {
                case 'b':  c = '\b'; p++; break;
                case 't':  c = '\t'; p++; break;
                case 'n':  c = '\n'; p++; break;
                case 'f':  c = '\f'; p++; break;
                case 'r':  c = '\r'; p++; break;
                case '"':  c = '"';  p++; break;
                case '\\': c = '\\'; p++; break;
                case 'u':
                case 'U': {
                    unsigned long cp;
                    char ubuf[4];
                    int nb;
                    int nhex = (*p == 'u') ? 4 : 8;
                    p++;
                    if (!toml_hex_n(p, nhex, &cp)) goto fail;
                    p += nhex;
                    nb = toml_utf8_encode(cp, ubuf);
                    if (nb == 0) goto fail;
                    if (toml_buf_append(&out, &cap, &len, ubuf,
                                        (size_t)nb) != 0) goto fail;
                    continue;
                }
                default:
                    goto fail;
            }
        } else {
            c = *p++;
        }
        if (toml_buf_append(&out, &cap, &len, &c, 1) != 0) goto fail;
    }
    if (*p != '"') goto fail;
    p++;
    if (end) *end = p;
    return out;
fail:
    free(out);
    return NULL;
}

static char *parse_toml_literal_string(const char *p, const char **end) {
    const char *close = strchr(p, '\'');
    size_t n;
    char *out;
    if (!close) return NULL;
    n = (size_t)(close - p);
    out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, p, n);
    out[n] = '\0';
    if (end) *end = close + 1;
    return out;
}

static char *parse_toml_string_value(const char *val) {
    const char *end;
    while (*val == ' ' || *val == '\t') val++;
    if (*val == '"')
        return parse_toml_basic_string(val + 1, &end);
    if (*val == '\'')
        return parse_toml_literal_string(val + 1, &end);
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Разбор TOML-массивов                                                */
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
        char quote;
        const char *end;
        char *content;

        while (*p == ' ' || *p == '\t') p++;
        if (*p == ']') break;
        if (*p == '\0') { free(out); return NULL; }

        quote = *p;
        if (quote != '"' && quote != '\'') {
            free(out);
            return NULL;
        }
        p++;
        content = (quote == '"')
                  ? parse_toml_basic_string(p, &end)
                  : parse_toml_literal_string(p, &end);
        if (!content) { free(out); return NULL; }
        p = end;

        if (len > 0) {
            if (toml_buf_append(&out, &cap, &len, sep, sep_len) != 0) {
                free(content);
                free(out);
                return NULL;
            }
        }
        if (toml_buf_append(&out, &cap, &len, content,
                            strlen(content)) != 0) {
            free(content);
            free(out);
            return NULL;
        }
        free(content);

        while (*p == ' ' || *p == '\t') p++;
        if (*p == ',') { p++; continue; }
        if (*p == ']') break;
    }
    return out;
}

static int parse_toml_string_array_into(const char *val,
                                        char ***list, int *count) {
    const char *p;
    if (!val || *val != '[') return -1;
    p = val + 1;
    while (*p) {
        char quote;
        const char *end;
        char *content;
        char **na;

        while (*p == ' ' || *p == '\t') p++;
        if (*p == ']') break;
        if (*p == '\0') return -1;

        quote = *p;
        if (quote != '"' && quote != '\'') return -1;
        p++;
        content = (quote == '"')
                  ? parse_toml_basic_string(p, &end)
                  : parse_toml_literal_string(p, &end);
        if (!content) return -1;
        p = end;

        na = (char**)realloc(*list, (size_t)(*count + 1) * sizeof(char*));
        if (!na) { free(content); return -1; }
        *list = na;
        (*list)[*count] = content;
        (*count)++;

        while (*p == ' ' || *p == '\t') p++;
        if (*p == ',') { p++; continue; }
        if (*p == ']') break;
    }
    return 0;
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

static int set_scalar_field(const char *filename, char **field,
                            const char *val, const char *key_name) {
    char *content = parse_toml_string_value(val);
    if (!content) {
        fprintf(stderr,
                "ERROR: %s: invalid %s value in [[annotations]].\n"
                "       Expected a quoted string.\n",
                filename, key_name);
        return -1;
    }
    if (*field) free(*field);
    *field = content;
    return 0;
}

ReuseConfig* parse_reuse_toml(const char *filename) {
    FILE *f;
    ReuseConfig *config;
    char line[1024];
    char *s, *p, *val;
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
            char *eq = strchr(s, '=');
            val = trim(eq + 1);

            if (*val == '"' || *val == '\'') {
                fprintf(stderr,
                        "ERROR: %s: 'version' must be an integer, "
                        "not a string.\n"
                        "       TOML v1.0.0 §Integer; REUSE 3.3 §4.1.1 "
                        "requires 'version = 1'.\n"
                        "       Fix: remove the quotes.\n",
                        filename);
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }

            if (strcmp(val, "1") != 0) {
                fprintf(stderr,
                        "ERROR: %s: unsupported version '%s'.\n"
                        "       REUSE 3.3 §4.1.1 defines version 1 only.\n"
                        "       Fix: change to 'version = 1'.\n",
                        filename, val);
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }
            seen_version = 1;
            continue;
        }

        if (current_ann == NULL) {
            int consumed = 0;
            if (!is_known_annotation_key(s, &consumed) && !warned_unknown) {
                fprintf(stderr,
                        "WARNING: %s: unknown key outside [[annotations]]: "
                        "%s\n",
                        filename, s);
                warned_unknown = 1;
            }
            continue;
        }

        if (in_path_array) {
            const char *q = s;
            int closed = 0;
            while (*q) {
                char quote;
                const char *end;
                char *content;

                while (*q == ' ' || *q == '\t') q++;
                if (*q == '\0') break;
                if (*q == ']') { closed = 1; break; }
                if (*q == ',') { q++; continue; }

                quote = *q;
                if (quote != '"' && quote != '\'') break;
                q++;
                content = (quote == '"')
                          ? parse_toml_basic_string(q, &end)
                          : parse_toml_literal_string(q, &end);
                if (!content) {
                    fprintf(stderr,
                            "ERROR: %s: invalid path string in "
                            "[[annotations]].\n"
                            "       Fix the string literal syntax "
                            "(TOML v1.0.0).\n",
                            filename);
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
                add_path(current_ann, content);
                free(content);
                q = end;
            }
            in_path_array = !closed;
            continue;
        }

        if (starts_with(s, "path = ") || starts_with(s, "path=")) {
            char *eq = strchr(s, '=');
            val = eq + 1;
            val = trim(val);
            if (*val == '[') {
                const char *q = val + 1;
                int closed = 0;
                while (*q) {
                    char quote;
                    const char *end;
                    char *content;

                    while (*q == ' ' || *q == '\t') q++;
                    if (*q == '\0') break;
                    if (*q == ']') { closed = 1; break; }
                    if (*q == ',') { q++; continue; }

                    quote = *q;
                    if (quote != '"' && quote != '\'') break;
                    q++;
                    content = (quote == '"')
                              ? parse_toml_basic_string(q, &end)
                              : parse_toml_literal_string(q, &end);
                    if (!content) {
                        fprintf(stderr,
                                "ERROR: %s: invalid path string in "
                                "[[annotations]].\n"
                                "       Fix the string literal syntax "
                                "(TOML v1.0.0).\n",
                                filename);
                        free_reuse_config(config);
                        fclose(f);
                        return NULL;
                    }
                    add_path(current_ann, content);
                    free(content);
                    q = end;
                }
                in_path_array = !closed;
            } else {
                char *content = parse_toml_string_value(val);
                if (!content) {
                    fprintf(stderr,
                            "ERROR: %s: invalid path value in "
                            "[[annotations]].\n"
                            "       Expected a quoted string.\n",
                            filename);
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
                add_path(current_ann, content);
                free(content);
            }
        } else if (starts_with(s, "SPDX-License-Identifier") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (current_ann->license) free(current_ann->license);
            if (*val == '[') {
                current_ann->license = parse_toml_string_array(val, " AND ");
            } else {
                char *content = parse_toml_string_value(val);
                if (!content) {
                    fprintf(stderr,
                            "ERROR: %s: invalid SPDX-License-Identifier "
                            "value in [[annotations]].\n"
                            "       Expected a quoted string.\n",
                            filename);
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
                current_ann->license = content;
            }
        } else if (starts_with(s, "SPDX-FileCopyrightText") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (current_ann->copyright) free(current_ann->copyright);
            if (*val == '[') {
                current_ann->copyright = parse_toml_string_array(val, "\n");
            } else {
                char *content = parse_toml_string_value(val);
                if (!content) {
                    fprintf(stderr,
                            "ERROR: %s: invalid SPDX-FileCopyrightText "
                            "value in [[annotations]].\n"
                            "       Expected a quoted string.\n",
                            filename);
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
                current_ann->copyright = content;
            }
        } else if (starts_with(s, "SPDX-FileContributor") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (*val == '[') {
                if (parse_toml_string_array_into(val,
                        &current_ann->contributors,
                        &current_ann->contributor_count) != 0) {
                    fprintf(stderr,
                            "ERROR: %s: invalid SPDX-FileContributor "
                            "value in [[annotations]].\n"
                            "       Expected an array of quoted strings.\n",
                            filename);
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
            } else {
                char *content = parse_toml_string_value(val);
                if (!content) {
                    fprintf(stderr,
                            "ERROR: %s: invalid SPDX-FileContributor "
                            "value in [[annotations]].\n"
                            "       Expected a quoted string.\n",
                            filename);
                    free_reuse_config(config);
                    fclose(f);
                    return NULL;
                }
                {
                    char **na = (char**)realloc(current_ann->contributors,
                        (size_t)(current_ann->contributor_count + 1) *
                        sizeof(char*));
                    if (!na) {
                        free(content);
                        fprintf(stderr, "ERROR: out of memory\n");
                        free_reuse_config(config);
                        fclose(f);
                        return NULL;
                    }
                    current_ann->contributors = na;
                    current_ann->contributors[
                        current_ann->contributor_count++] = content;
                }
            }
        } else if (starts_with(s, "SPDX-PackageName") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (set_scalar_field(filename, &current_ann->package_name,
                                 val, "SPDX-PackageName") != 0) {
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }
        } else if (starts_with(s, "SPDX-PackageSupplier") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (set_scalar_field(filename, &current_ann->package_supplier,
                                 val, "SPDX-PackageSupplier") != 0) {
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }
        } else if (starts_with(s, "SPDX-PackageDownloadLocation") &&
                   strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (set_scalar_field(filename,
                                 &current_ann->package_download_location,
                                 val, "SPDX-PackageDownloadLocation") != 0) {
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }
        } else if (starts_with(s, "SPDX-PackageComment") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            val = trim(eq + 1);
            if (set_scalar_field(filename, &current_ann->package_comment,
                                 val, "SPDX-PackageComment") != 0) {
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }
        } else if (starts_with(s, "precedence") && strchr(s, '=')) {
            char *eq = strchr(s, '=');
            char *content;
            val = trim(eq + 1);
            content = parse_toml_string_value(val);
            if (!content) {
                fprintf(stderr,
                        "ERROR: %s: invalid precedence value in "
                        "[[annotations]].\n"
                        "       Expected one of: \"closest\", "
                        "\"aggregate\", \"override\".\n",
                        filename);
                free_reuse_config(config);
                fclose(f);
                return NULL;
            }
            if (strcmp(content, "closest") == 0)
                current_ann->precedence = REUSE_PRECEDENCE_CLOSEST;
            else if (strcmp(content, "aggregate") == 0)
                current_ann->precedence = REUSE_PRECEDENCE_AGGREGATE;
            else if (strcmp(content, "override") == 0)
                current_ann->precedence = REUSE_PRECEDENCE_OVERRIDE;
            else {
                fprintf(stderr,
                        "WARNING: %s: unknown precedence '%s'.\n"
                        "         Expected 'closest', 'aggregate' or "
                        "'override'. Using 'closest'.\n",
                        filename, content);
                current_ann->precedence = REUSE_PRECEDENCE_CLOSEST;
            }
            free(content);
        } else {
            int consumed = 0;
            if (!is_known_annotation_key(s, &consumed)) {
                fprintf(stderr,
                        "WARNING: %s: unknown key in [[annotations]]: %s\n",
                        filename, s);
            }
        }
    }

    fclose(f);

    if (!seen_version) {
        fprintf(stderr,
                "ERROR: %s: missing 'version = 1' at the top.\n"
                "       REUSE 3.3 §4.1.1 requires this line in every "
                "REUSE.toml.\n"
                "       Fix: add 'version = 1' as the first non-comment "
                "line.\n",
                filename);
        free_reuse_config(config);
        return NULL;
    }

    return config;
}

void free_reuse_config(ReuseConfig *config) {
    int i, j;
    if (!config) return;
    free(config->source_dir);
    for (i = 0; i < config->annotation_count; i++) {
        Annotation *a = &config->annotations[i];
        for (j = 0; j < a->path_count; j++)
            free(a->paths[j]);
        free(a->paths);
        free(a->license);
        free(a->copyright);
        for (j = 0; j < a->contributor_count; j++)
            free(a->contributors[j]);
        free(a->contributors);
        free(a->package_name);
        free(a->package_supplier);
        free(a->package_download_location);
        free(a->package_comment);
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
                            "WARNING: path too long, skipping REUSE.toml "
                            "below %s\n"
                            "         Increase the buffer size or shorten "
                            "the path.\n", current);
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

ReuseConfig **reuse_parse_all(const SpdxStrList *paths,
                              int *out_count,
                              int *out_error_count) {
    ReuseConfig **arr;
    int i, n = 0;

    *out_count = 0;
    if (out_error_count) *out_error_count = 0;
    if (!paths || paths->count == 0) return NULL;

    arr = (ReuseConfig**)malloc((size_t)paths->count * sizeof(ReuseConfig*));
    if (!arr) return NULL;

    for (i = 0; i < paths->count; i++) {
        ReuseConfig *c = parse_reuse_toml(paths->items[i]);
        if (!c) {
            if (out_error_count) (*out_error_count)++;
            continue;
        }
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
/*                                                                     */
/* Политика разрешения полей:                                          */
/*                                                                     */
/* 1. Licensing Information (copyright, license).                      */
/*    Спецификация REUSE 3.3 (строки 21-23) определяет это как         */
/*    "the information that lists the copyright holders of a file or   */
/*    work, and describes under which licenses the file or work is     */
/*    made available". К этой категории применяется precedence:        */
/*    override > aggregate > closest > depth > order_in_file.          */
/*                                                                     */
/* 2. Contributors (SPDX-FileContributor).                             */
/*    Спецификация REUSE 3.3 (строки 177-182) прямо говорит:           */
/*    "You MAY include other keys and tables to convey additional      */
/*    information. Their semantics are not defined by this             */
/*    specification." SPDX-FileContributor упомянут как пример такого  */
/*    ключа. Поэтому precedence к нему формально НЕ применяется.       */
/*                                                                     */
/*    Проектное решение: contributors собираются с агрегацией         */
/*    (closest + aggregate) с дедупликацией строк. Причина: это       */
/*    список людей, и не терять информацию важнее, чем соблюдать      */
/*    иерархию. При наличии override — берётся только override.        */
/*                                                                     */
/* 3. Package* (SPDX-PackageName, SPDX-PackageSupplier,                */
/*    SPDX-PackageDownloadLocation, SPDX-PackageComment).              */
/*    Спецификация REUSE 3.3 (строки 177-182) относит их к "other      */
/*    keys", семантика не определена. precedence формально НЕ          */
/*    применяется.                                                     */
/*                                                                     */
/*    Проектное решение: closest > aggregate. Причина: package_name   */
/*    и прочие атрибуты — это атрибуты конкретного (под)пакета;       */
/*    более специфичная (ближе к файлу) аннотация должна выигрывать.  */
/*                                                                     */
/*    override по-прежнему выигрывает у всего.                         */
/*                                                                     */
/* Ссылки:                                                             */
/*   REUSE 3.3: https://reuse.software/spec-3.3/                       */
/*     - строки 21-23 (Licensing Information)                          */
/*     - строки 155-171 (precedence)                                   */
/*     - строки 177-182 (other keys)                                   */
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

/* Соединяет массив contributors в одну строку через '\n'. */
static char *join_contributors(char **items, int count) {
    size_t total = 0;
    int i;
    size_t pos = 0;
    char *out;

    if (count == 0) return NULL;
    for (i = 0; i < count; i++) {
        total += strlen(items[i]);
        if (i > 0) total += 1;
    }
    out = (char*)malloc(total + 1);
    if (!out) return NULL;
    for (i = 0; i < count; i++) {
        if (i > 0) out[pos++] = '\n';
        {
            size_t l = strlen(items[i]);
            memcpy(out + pos, items[i], l);
            pos += l;
        }
    }
    out[pos] = '\0';
    return out;
}

/* Проверяет, содержит ли haystack (строки через '\n') строку needle. */
static int contains_line(const char *haystack, const char *needle, size_t nlen) {
    const char *p = haystack;
    while (*p) {
        const char *line_end = strchr(p, '\n');
        size_t len = line_end ? (size_t)(line_end - p) : strlen(p);
        if (len == nlen && memcmp(p, needle, nlen) == 0) return 1;
        if (!line_end) break;
        p = line_end + 1;
    }
    return 0;
}

/* Соединяет две строки-списка ('\n'-separated) с дедупликацией строк. */
static char *join_lines_dedup(const char *a, const char *b) {
    size_t alen, blen, out_len;
    char *out;
    const char *p;

    if (!a && !b) return NULL;
    if (!a) return dup_str(b);
    if (!b) return dup_str(a);

    alen = strlen(a);
    blen = strlen(b);
    out = (char*)malloc(alen + blen + 2);
    if (!out) return NULL;
    memcpy(out, a, alen);
    out[alen] = '\0';
    out_len = alen;

    p = b;
    while (*p) {
        const char *line_end = strchr(p, '\n');
        size_t line_len = line_end ? (size_t)(line_end - p) : strlen(p);
        if (!contains_line(out, p, line_len)) {
            if (out_len + 1 + line_len + 1 > alen + blen + 2) {
                size_t ncap = out_len + 1 + line_len + 2;
                char *no = (char*)realloc(out, ncap);
                if (!no) { free(out); return NULL; }
                out = no;
            }
            out[out_len++] = '\n';
            memcpy(out + out_len, p, line_len);
            out_len += line_len;
            out[out_len] = '\0';
        }
        if (!line_end) break;
        p = line_end + 1;
    }
    return out;
}

void reuse_resolved_free(ReuseResolved *r) {
    if (!r) return;
    free(r->license);                    r->license = NULL;
    free(r->copyright);                  r->copyright = NULL;
    free(r->contributors);               r->contributors = NULL;
    free(r->package_name);               r->package_name = NULL;
    free(r->package_supplier);           r->package_supplier = NULL;
    free(r->package_download_location);  r->package_download_location = NULL;
    free(r->package_comment);            r->package_comment = NULL;
    r->precedence = 0;
    r->has_reuse = 0;
}

int reuse_resolve_for_file(ReuseConfig **configs, int count,
                           const char *filename,
                           const char *in_license,
                           const char *in_copyright,
                           ReuseResolved *out) {
    const Annotation *best_override = NULL;
    const Annotation *best_aggregate = NULL;
    const Annotation *best_closest = NULL;
    int best_override_depth = -1;
    int best_aggregate_depth = -1;
    int best_closest_depth = -1;
    int i;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));

    /* Поиск лучших аннотаций в каждой из трёх групп независимо. */
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

    /* === 1. Override — выигрывает у всего. === */
    if (best_override) {
        out->has_reuse = 1;
        out->precedence = REUSE_PRECEDENCE_OVERRIDE;
        if (best_override->license)
            out->license = dup_str(best_override->license);
        if (best_override->copyright)
            out->copyright = dup_str(best_override->copyright);
        if (best_override->contributor_count > 0)
            out->contributors = join_contributors(
                best_override->contributors,
                best_override->contributor_count);
        if (best_override->package_name)
            out->package_name = dup_str(best_override->package_name);
        if (best_override->package_supplier)
            out->package_supplier = dup_str(best_override->package_supplier);
        if (best_override->package_download_location)
            out->package_download_location =
                dup_str(best_override->package_download_location);
        if (best_override->package_comment)
            out->package_comment = dup_str(best_override->package_comment);
        return 0;
    }

    /* === 2. База: closest + in-file. === */
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

        /* === 2a. Без aggregate. === */
        if (!best_aggregate) {
            out->has_reuse = base_has_reuse;
            out->precedence = REUSE_PRECEDENCE_CLOSEST;
            if (base_lic) out->license = dup_str(base_lic);
            if (base_cop) out->copyright = dup_str(base_cop);
            if (best_closest) {
                if (best_closest->contributor_count > 0)
                    out->contributors = join_contributors(
                        best_closest->contributors,
                        best_closest->contributor_count);
                if (best_closest->package_name)
                    out->package_name = dup_str(best_closest->package_name);
                if (best_closest->package_supplier)
                    out->package_supplier =
                        dup_str(best_closest->package_supplier);
                if (best_closest->package_download_location)
                    out->package_download_location =
                        dup_str(best_closest->package_download_location);
                if (best_closest->package_comment)
                    out->package_comment =
                        dup_str(best_closest->package_comment);
            }
            return 0;
        }

        /* === 2b. Aggregate поверх base. === */
        out->has_reuse = 1;
        out->precedence = REUSE_PRECEDENCE_AGGREGATE;

        /* license / copyright: aggregate применяется (Licensing
         * Information; REUSE 3.3 строки 21-23). */
        if (base_lic && best_aggregate->license)
            out->license = aggregate_licenses(base_lic,
                                              best_aggregate->license);
        else if (base_lic)
            out->license = dup_str(base_lic);
        else if (best_aggregate->license)
            out->license = dup_str(best_aggregate->license);

        if (base_cop && best_aggregate->copyright)
            out->copyright = aggregate_copyrights(base_cop,
                                                  best_aggregate->copyright);
        else if (base_cop)
            out->copyright = dup_str(base_cop);
        else if (best_aggregate->copyright)
            out->copyright = dup_str(best_aggregate->copyright);

        /* contributors: НЕ Licensing Information (REUSE 3.3 строки
         * 177-182). Проектное решение — агрегация с дедупликацией,
         * чтобы не терять людей. */
        {
            char *base_contrib = NULL;
            char *agg_contrib = NULL;
            if (best_closest && best_closest->contributor_count > 0)
                base_contrib = join_contributors(
                    best_closest->contributors,
                    best_closest->contributor_count);
            if (best_aggregate->contributor_count > 0)
                agg_contrib = join_contributors(
                    best_aggregate->contributors,
                    best_aggregate->contributor_count);

            if (base_contrib && agg_contrib) {
                out->contributors = join_lines_dedup(base_contrib,
                                                     agg_contrib);
                free(base_contrib);
                free(agg_contrib);
            } else if (base_contrib) {
                out->contributors = base_contrib;
            } else if (agg_contrib) {
                out->contributors = agg_contrib;
            }
        }

        /* package_*: НЕ Licensing Information (REUSE 3.3 строки
         * 177-182). Проектное решение — closest > aggregate: более
         * специфичная аннотация выигрывает у менее специфичной,
         * независимо от precedence. */
        if (best_closest && best_closest->package_name)
            out->package_name = dup_str(best_closest->package_name);
        else if (best_aggregate->package_name)
            out->package_name = dup_str(best_aggregate->package_name);

        if (best_closest && best_closest->package_supplier)
            out->package_supplier = dup_str(best_closest->package_supplier);
        else if (best_aggregate->package_supplier)
            out->package_supplier = dup_str(best_aggregate->package_supplier);

        if (best_closest && best_closest->package_download_location)
            out->package_download_location =
                dup_str(best_closest->package_download_location);
        else if (best_aggregate->package_download_location)
            out->package_download_location =
                dup_str(best_aggregate->package_download_location);

        if (best_closest && best_closest->package_comment)
            out->package_comment = dup_str(best_closest->package_comment);
        else if (best_aggregate->package_comment)
            out->package_comment = dup_str(best_aggregate->package_comment);
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
