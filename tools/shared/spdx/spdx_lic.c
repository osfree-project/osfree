/* spdx_lic.c - определение лицензии/копирайта файла (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_lic.h"
#include "spdx_tag.h"
#include "spdx_utils.h"

#ifdef __LINUX__
#include <unistd.h>
#else
#include <io.h>
#endif

/* ------------------------------------------------------------------ */
/* Утилиты                                                             */
/* ------------------------------------------------------------------ */

static void copy_safe(char *dst, size_t dst_size, const char *src) {
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

/* Убирает хвостовые пробелы и маркеры закрытия комментариев. */
static void strip_trailing_comments(char *s) {
    size_t len = strlen(s);
    while (len > 0) {
        if (s[len-1] == ' ' || s[len-1] == '\t' ||
            s[len-1] == '\n' || s[len-1] == '\r') {
            len -= 1;
            continue;
        }
        if (len >= 2 && s[len-2] == '*' && s[len-1] == '/') {
            if (len == 2 || s[len-3] == ' ' || s[len-3] == '\t') {
                len -= 2;
                while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t'))
                    len--;
                continue;
            }
        }
        if (len >= 2 && s[len-2] == '/' && s[len-1] == '/') {
            if (len == 2 || s[len-3] == ' ' || s[len-3] == '\t') {
                len -= 2;
                while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t'))
                    len--;
                continue;
            }
        }
        break;
    }
    s[len] = '\0';
}

/* Проверяет, встречается ли token в expr как отдельное слово. */
static int contains_token(const char *expr, const char *token) {
    size_t tl;
    const char *p;
    int before_ok;
    int after_ok;

    tl = strlen(token);
    p = expr;
    while ((p = strstr(p, token)) != NULL) {
        before_ok = (p == expr) ||
                    (p[-1] == ' ' || p[-1] == '\t' || p[-1] == '(');
        after_ok = (p[tl] == '\0' || p[tl] == ' ' ||
                    p[tl] == '\t' || p[tl] == ')');
        if (before_ok && after_ok) return 1;
        p += tl;
    }
    return 0;
}

static int is_simple_and_chain(const char *expr) {
    if (contains_token(expr, "OR")) return 0;
    if (contains_token(expr, "WITH")) return 0;
    if (strchr(expr, '(') || strchr(expr, ')')) return 0;
    return 1;
}

/* Склеивает два SPDX-выражения через AND с удалением дубликатов. */
static char *merge_and_expressions(const char *a, const char *b) {
    if (!a && !b) return NULL;
    if (!a) return dup_str(b);
    if (!b) return dup_str(a);
    if (strcmp(a, b) == 0) return dup_str(a);

    if (is_simple_and_chain(a) && is_simple_and_chain(b)) {
        SpdxStrList ids;
        int i;
        size_t total;
        char *out;
        size_t pos;

        spdx_strlist_init(&ids);
        spdx_expression_collect_ids(a, &ids);
        spdx_expression_collect_ids(b, &ids);

        if (ids.count == 0) {
            spdx_strlist_free(&ids);
            return NULL;
        }

        total = 0;
        for (i = 0; i < ids.count; i++) {
            if (i > 0) total += 5;
            total += strlen(ids.items[i]);
        }
        out = (char*)malloc(total + 1);
        if (!out) {
            spdx_strlist_free(&ids);
            return NULL;
        }
        pos = 0;
        for (i = 0; i < ids.count; i++) {
            if (i > 0) {
                memcpy(out + pos, " AND ", 5);
                pos += 5;
            }
            {
                size_t l = strlen(ids.items[i]);
                memcpy(out + pos, ids.items[i], l);
                pos += l;
            }
        }
        out[pos] = '\0';
        spdx_strlist_free(&ids);
        return out;
    }

    {
        size_t al = strlen(a);
        size_t bl = strlen(b);
        size_t need = al + bl + 12;
        char *r = (char*)malloc(need);
        if (!r) return NULL;
        snprintf(r, need, "(%s) AND (%s)", a, b);
        return r;
    }
}

/* ------------------------------------------------------------------ */
/* Источники                                                           */
/* ------------------------------------------------------------------ */

static int read_sidecar(const char *fullpath,
                        char **out_license, char **out_copyright) {
    char sidecar[1024];
    int exists;
    char *lic, *cop;

    *out_license = NULL;
    *out_copyright = NULL;

    snprintf(sidecar, sizeof(sidecar), "%s.license", fullpath);
#ifdef __LINUX__
    exists = (access(sidecar, F_OK) == 0);
#else
    exists = (_access(sidecar, 0) == 0);
#endif
    if (!exists) return -1;

    lic = file_get_spdx_license(sidecar);
    cop = file_get_spdx_copyright(sidecar);
    if (lic && lic[0]) *out_license = lic;
    else if (lic) free(lic);
    if (cop && cop[0]) *out_copyright = cop;
    else if (cop) free(cop);

    if (!*out_license && !*out_copyright) return -1;
    return 0;
}

static int read_tags(const char *fullpath,
                     char **out_license, char **out_copyright) {
    char *lic, *cop;

    *out_license = NULL;
    *out_copyright = NULL;

    lic = file_get_spdx_license(fullpath);
    cop = file_get_spdx_copyright(fullpath);
    if (lic && lic[0]) *out_license = lic;
    else if (lic) free(lic);
    if (cop && cop[0]) *out_copyright = cop;
    else if (cop) free(cop);

    if (!*out_license && !*out_copyright) return -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Основная функция                                                    */
/* ------------------------------------------------------------------ */

int spdx_resolve_license(ReuseConfig **configs, int config_count,
                         const char *fullpath,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out) {
    char *reuse_license = NULL;
    char *reuse_copyright = NULL;
    char *side_license = NULL;
    char *side_copyright = NULL;
    char *tag_license = NULL;
    char *tag_copyright = NULL;
    int reuse_precedence = 0;
    int has_reuse = 0;

    out->license[0] = '\0';
    out->copyright[0] = '\0';
    out->source = LICENSE_SRC_NONE;
    out->license_from_default = 0;
    out->copyright_from_default = 0;

    if (configs && config_count > 0) {
        reuse_resolve_for_file(configs, config_count, fullpath,
                               NULL, NULL,
                               &reuse_license, &reuse_copyright,
                               &reuse_precedence, &has_reuse);
    }

    if (reuse_precedence != REUSE_PRECEDENCE_OVERRIDE) {
        read_sidecar(fullpath, &side_license, &side_copyright);
        read_tags(fullpath, &tag_license, &tag_copyright);
    }

    if (reuse_precedence == REUSE_PRECEDENCE_OVERRIDE) {
        if (reuse_license) {
            copy_safe(out->license, sizeof(out->license), reuse_license);
            out->source = LICENSE_SRC_REUSE;
        }
        if (reuse_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), reuse_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
        }
    } else if (reuse_precedence == REUSE_PRECEDENCE_AGGREGATE) {
        char *in_lic = side_license ? side_license : tag_license;
        char *in_cop = side_copyright ? side_copyright : tag_copyright;

        if (in_lic && reuse_license) {
            char *agg = merge_and_expressions(in_lic, reuse_license);
            if (agg) {
                if (strlen(agg) >= sizeof(out->license)) {
                    fprintf(stderr,
                            "WARNING: %s: aggregated license expression "
                            "truncated.\n"
                            "         The combined expression exceeds %d "
                            "characters.\n"
                            "         Consider simplifying the REUSE.toml "
                            "annotation.\n",
                            fullpath, (int)sizeof(out->license));
                }
                copy_safe(out->license, sizeof(out->license), agg);
                free(agg);
                out->source = LICENSE_SRC_REUSE;
            }
        } else if (in_lic) {
            copy_safe(out->license, sizeof(out->license), in_lic);
            out->source = side_license ? LICENSE_SRC_SIDECAR : LICENSE_SRC_TAG;
        } else if (reuse_license) {
            copy_safe(out->license, sizeof(out->license), reuse_license);
            out->source = LICENSE_SRC_REUSE;
        }

        if (in_cop && reuse_copyright) {
            if (strcmp(in_cop, reuse_copyright) == 0) {
                copy_safe(out->copyright, sizeof(out->copyright), in_cop);
            } else {
                size_t alen = strlen(in_cop);
                size_t blen = strlen(reuse_copyright);
                if (alen + blen + 2 < sizeof(out->copyright)) {
                    sprintf(out->copyright, "%s\n%s",
                            in_cop, reuse_copyright);
                } else {
                    fprintf(stderr,
                            "WARNING: %s: aggregated copyright truncated.\n"
                            "         The combined text exceeds %d "
                            "characters.\n"
                            "         Consider simplifying the REUSE.toml "
                            "annotation.\n",
                            fullpath, (int)sizeof(out->copyright));
                    copy_safe(out->copyright, sizeof(out->copyright), in_cop);
                }
            }
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
        } else if (in_cop) {
            copy_safe(out->copyright, sizeof(out->copyright), in_cop);
            if (out->source == LICENSE_SRC_NONE)
                out->source = side_copyright ? LICENSE_SRC_SIDECAR : LICENSE_SRC_TAG;
        } else if (reuse_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), reuse_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
        }
    } else {
        if (side_license) {
            copy_safe(out->license, sizeof(out->license), side_license);
            out->source = LICENSE_SRC_SIDECAR;
        } else if (tag_license) {
            copy_safe(out->license, sizeof(out->license), tag_license);
            out->source = LICENSE_SRC_TAG;
        } else if (reuse_license) {
            copy_safe(out->license, sizeof(out->license), reuse_license);
            out->source = LICENSE_SRC_REUSE;
        }

        if (side_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), side_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_SIDECAR;
        } else if (tag_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), tag_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_TAG;
        } else if (reuse_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), reuse_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
        }
    }

    if (out->license[0] == '\0' && default_license && default_license[0]) {
        copy_safe(out->license, sizeof(out->license), default_license);
        out->license_from_default = 1;
        if (out->source == LICENSE_SRC_NONE)
            out->source = LICENSE_SRC_DEFAULT;
    }
    if (out->copyright[0] == '\0' && default_copyright && default_copyright[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), default_copyright);
        out->copyright_from_default = 1;
        if (out->source == LICENSE_SRC_NONE)
            out->source = LICENSE_SRC_DEFAULT;
    }

    if (reuse_license) free(reuse_license);
    if (reuse_copyright) free(reuse_copyright);
    if (side_license) free(side_license);
    if (side_copyright) free(side_copyright);
    if (tag_license) free(tag_license);
    if (tag_copyright) free(tag_copyright);

    strip_trailing_comments(out->license);
    strip_trailing_comments(out->copyright);

    if (out->license[0] == '\0' && out->copyright[0] == '\0')
        return -1;
    return 0;
}

int spdx_resolve_license_single(ReuseConfig *config,
                                const char *fullpath,
                                const char *default_license,
                                const char *default_copyright,
                                FileLicenseInfo *out) {
    ReuseConfig *arr[1];
    if (config) {
        arr[0] = config;
        return spdx_resolve_license(arr, 1, fullpath,
                                    default_license, default_copyright, out);
    }
    return spdx_resolve_license(NULL, 0, fullpath,
                                default_license, default_copyright, out);
}
