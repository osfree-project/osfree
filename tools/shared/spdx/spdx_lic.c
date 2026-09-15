/* spdx_lic.c - определение лицензии/копирайта файла (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_lic.h"
#include "spdx_tag.h"
#include "spdx_utils.h"
#include "spdx_db.h"

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

/* Копирует с предупреждением при усечении. */
static void copy_safe_warn(const char *fullpath, const char *field_name,
                           char *dst, size_t dst_size, const char *src) {
    size_t slen;
    if (!src) { dst[0] = '\0'; return; }
    slen = strlen(src);
    if (slen >= dst_size) {
        fprintf(stderr,
                "WARNING: %s: %s truncated (%u bytes, buffer %u).\n"
                "         Consider shortening the annotation.\n",
                fullpath, field_name,
                (unsigned)slen, (unsigned)dst_size);
        copy_safe(dst, dst_size, src);
        return;
    }
    memcpy(dst, src, slen + 1);
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

/* ------------------------------------------------------------------ */
/* In-file источники                                                   */
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
    char *side_license = NULL;
    char *side_copyright = NULL;
    char *tag_license = NULL;
    char *tag_copyright = NULL;
    const char *in_license = NULL;
    const char *in_copyright = NULL;
    ReuseResolved resolved;

    /* Инициализация результата. */
    out->license[0] = '\0';
    out->copyright[0] = '\0';
    out->contributors[0] = '\0';
    out->package_name[0] = '\0';
    out->package_supplier[0] = '\0';
    out->package_download_location[0] = '\0';
    out->package_comment[0] = '\0';
    out->has_package_info = 0;
    out->source = LICENSE_SRC_NONE;
    out->license_from_default = 0;
    out->copyright_from_default = 0;

    /* 1. Читаем in-file источники. Они нужны reuse_resolve_for_file
     *    для aggregate-логики. Даже если matched override — чтение
     *    безопасно: override игнорирует in-file источники. */
    read_sidecar(fullpath, &side_license, &side_copyright);
    read_tags(fullpath, &tag_license, &tag_copyright);

    /* Приоритет in-file источников: sidecar > tags
     * (sidecar физически ближе к файлу). */
    if (side_license) in_license = side_license;
    else if (tag_license) in_license = tag_license;
    if (side_copyright) in_copyright = side_copyright;
    else if (tag_copyright) in_copyright = tag_copyright;

    /* 2. Резолвим REUSE.toml + in-file источники.
     *    Логика precedence (override > aggregate > closest) и
     *    агрегация — внутри reuse_resolve_for_file (см. reuse_parser.c). */
    memset(&resolved, 0, sizeof(resolved));
    reuse_resolve_for_file(configs, config_count, fullpath,
                           in_license, in_copyright, &resolved);

    /* 3. Копируем поля из resolved в FileLicenseInfo. */
    if (resolved.license) {
        copy_safe_warn(fullpath, "SPDX-License-Identifier",
                       out->license, sizeof(out->license),
                       resolved.license);
    }
    if (resolved.copyright) {
        copy_safe_warn(fullpath, "SPDX-FileCopyrightText",
                       out->copyright, sizeof(out->copyright),
                       resolved.copyright);
    }
    if (resolved.contributors) {
        copy_safe_warn(fullpath, "SPDX-FileContributor",
                       out->contributors, sizeof(out->contributors),
                       resolved.contributors);
    }
    if (resolved.package_name) {
        copy_safe_warn(fullpath, "SPDX-PackageName",
                       out->package_name, sizeof(out->package_name),
                       resolved.package_name);
    }
    if (resolved.package_supplier) {
        copy_safe_warn(fullpath, "SPDX-PackageSupplier",
                       out->package_supplier, sizeof(out->package_supplier),
                       resolved.package_supplier);
    }
    if (resolved.package_download_location) {
        copy_safe_warn(fullpath, "SPDX-PackageDownloadLocation",
                       out->package_download_location,
                       sizeof(out->package_download_location),
                       resolved.package_download_location);
    }
    if (resolved.package_comment) {
        copy_safe_warn(fullpath, "SPDX-PackageComment",
                       out->package_comment, sizeof(out->package_comment),
                       resolved.package_comment);
    }

    if (out->package_name[0] || out->package_supplier[0] ||
        out->package_download_location[0] || out->package_comment[0]) {
        out->has_package_info = 1;
    }

    /* 4. Определяем source.
     *    REUSE.toml (если хоть одна аннотация совпала) — REUSE.
     *    Иначе in-file источники: sidecar > tags. */
    if (resolved.has_reuse) {
        out->source = LICENSE_SRC_REUSE;
    } else if (side_license || side_copyright) {
        out->source = LICENSE_SRC_SIDECAR;
    } else if (tag_license || tag_copyright) {
        out->source = LICENSE_SRC_TAG;
    }

    /* 5. Fallback на --default-*. */
    if (out->license[0] == '\0' &&
        default_license && default_license[0]) {
        copy_safe(out->license, sizeof(out->license), default_license);
        out->license_from_default = 1;
        if (out->source == LICENSE_SRC_NONE)
            out->source = LICENSE_SRC_DEFAULT;
    }
    if (out->copyright[0] == '\0' &&
        default_copyright && default_copyright[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), default_copyright);
        out->copyright_from_default = 1;
        if (out->source == LICENSE_SRC_NONE)
            out->source = LICENSE_SRC_DEFAULT;
    }

    /* 6. Нормализация SPDX-выражения (канонический регистр). */
    if (out->license[0] != '\0') {
        char *normalized = spdx_normalize_license_expression(out->license);
        if (normalized) {
            copy_safe(out->license, sizeof(out->license), normalized);
            free(normalized);
        }
    }

    /* Защитная очистка от хвостовых маркеров (sidecar с комментарием). */
    strip_trailing_comments(out->license);
    strip_trailing_comments(out->copyright);

    /* 7. Освобождение временных. */
    reuse_resolved_free(&resolved);
    if (side_license) free(side_license);
    if (side_copyright) free(side_copyright);
    if (tag_license) free(tag_license);
    if (tag_copyright) free(tag_copyright);

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
