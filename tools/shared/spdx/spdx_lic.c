/* spdx_lic.c - определение лицензии/копирайта файла (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_lic.h"
#include "spdx_tag.h"

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

/* Убирает хвостовые артефакты комментария и whitespace.
 * Удаляются: закрывающий блочный комментарий, строчный комментарий,
 * пробелы, табуляции, CR, LF. */
static void strip_trailing_comments(char *s) {
    size_t len = strlen(s);
    while (len > 0) {
        if (len >= 2 && s[len-2] == '*' && s[len-1] == '/') {
            len -= 2;
        } else if (len >= 2 && s[len-2] == '/' && s[len-1] == '/') {
            len -= 2;
        } else if (s[len-1] == ' ' || s[len-1] == '\t' ||
                   s[len-1] == '\n' || s[len-1] == '\r') {
            len -= 1;
        } else {
            break;
        }
    }
    s[len] = '\0';
}

/* ------------------------------------------------------------------ */
/* Источники                                                           */
/* ------------------------------------------------------------------ */

/* Sidecar <fullpath>.license. */
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

/* Теги SPDX внутри файла. */
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
                         const char *filename,
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

    /* 1. REUSE.toml: определяем precedence и значения */
    if (configs && config_count > 0) {
        reuse_resolve_for_file(configs, config_count, fullpath,
                               NULL, NULL,
                               &reuse_license, &reuse_copyright,
                               &reuse_precedence, &has_reuse);
    }

    /* 2. Sidecar и теги — только если precedence != override */
    if (reuse_precedence != REUSE_PRECEDENCE_OVERRIDE) {
        read_sidecar(fullpath, &side_license, &side_copyright);
        read_tags(fullpath, &tag_license, &tag_copyright);
    }

    /* 3. Собираем итог в зависимости от precedence */
    if (reuse_precedence == REUSE_PRECEDENCE_OVERRIDE) {
        /* Только REUSE.toml */
        if (reuse_license) {
            copy_safe(out->license, sizeof(out->license), reuse_license);
            out->source = LICENSE_SRC_REUSE;
        }
        if (reuse_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), reuse_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
        }
    } else if (reuse_precedence == REUSE_PRECEDENCE_AGGREGATE) {
        /* REUSE.toml + in-file через AND */
        char *in_lic = side_license ? side_license : tag_license;
        char *in_cop = side_copyright ? side_copyright : tag_copyright;

        if (in_lic && reuse_license) {
            size_t alen = strlen(in_lic);
            size_t blen = strlen(reuse_license);
            char *agg = (char*)malloc(alen + blen + 8);
            if (agg) {
                snprintf(agg, alen + blen + 8, "(%s) AND (%s)",
                         in_lic, reuse_license);
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
            size_t alen = strlen(in_cop);
            size_t blen = strlen(reuse_copyright);
            if (alen + blen + 2 < sizeof(out->copyright)) {
                sprintf(out->copyright, "%s\n%s", in_cop, reuse_copyright);
                if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
            }
        } else if (in_cop) {
            copy_safe(out->copyright, sizeof(out->copyright), in_cop);
            if (out->source == LICENSE_SRC_NONE)
                out->source = side_copyright ? LICENSE_SRC_SIDECAR : LICENSE_SRC_TAG;
        } else if (reuse_copyright) {
            copy_safe(out->copyright, sizeof(out->copyright), reuse_copyright);
            if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_REUSE;
        }
    } else {
        /* closest: sidecar выигрывает у тегов; иначе REUSE.toml */
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

    /* 4. CLI-fallback */
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

    /* Освобождаем */
    if (reuse_license) free(reuse_license);
    if (reuse_copyright) free(reuse_copyright);
    if (side_license) free(side_license);
    if (side_copyright) free(side_copyright);
    if (tag_license) free(tag_license);
    if (tag_copyright) free(tag_copyright);

    /* 5. Санитизация */
    strip_trailing_comments(out->license);
    strip_trailing_comments(out->copyright);

    if (out->license[0] == '\0' && out->copyright[0] == '\0')
        return -1;
    return 0;
}

int spdx_resolve_license_single(ReuseConfig *config,
                                const char *fullpath,
                                const char *filename,
                                const char *default_license,
                                const char *default_copyright,
                                FileLicenseInfo *out) {
    ReuseConfig *arr[1];
    if (config) {
        arr[0] = config;
        return spdx_resolve_license(arr, 1, fullpath, filename,
                                    default_license, default_copyright, out);
    }
    return spdx_resolve_license(NULL, 0, fullpath, filename,
                                default_license, default_copyright, out);
}
