/* spdx_lic.h - определение лицензии/копирайта файла (C89) */
#ifndef SPDX_LIC_H
#define SPDX_LIC_H

#include <reuse_parser.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LICENSE_SRC_NONE = 0,
    LICENSE_SRC_TAG,
    LICENSE_SRC_SIDECAR,
    LICENSE_SRC_REUSE,
    LICENSE_SRC_DEFAULT
} LicenseSource;

typedef struct {
    /* Основные поля: license и copyright.
     * Заполняются из REUSE.toml, sidecar или тегов файла (в порядке
     * приоритета, определяемом REUSE 3.3 §4.1.3). */
    char license[256];
    char copyright[512];

    /* SPDX-FileContributor: '\n'-separated, дедуплицирован.
     * Источник — только REUSE.toml. Спецификация REUSE 3.3
     * (строки 177-182) относит этот ключ к "other keys" с
     * неопределённой семантикой; проектное решение — агрегировать
     * contributors из closest и aggregate, чтобы не терять людей. */
    char contributors[1024];

    /* SPDX-Package*: заполняются только из REUSE.toml. Спецификация
     * REUSE 3.3 относит их к "other keys" (semantics not defined).
     * Проектное решение: closest > aggregate (более специфичная
     * аннотация выигрывает). override выигрывает у всего. */
    char package_name[256];
    char package_supplier[256];
    char package_download_location[512];
    char package_comment[512];
    int  has_package_info;    /* 1, если хоть одно package_* непусто */

    LicenseSource source;
    int license_from_default;
    int copyright_from_default;
} FileLicenseInfo;

/* Разрешение лицензии с учётом иерархии REUSE.toml.
 *
 * Порядок источников:
 *   1. REUSE.toml (с учётом precedence: override > aggregate > closest)
 *   2. sidecar <file>.license
 *   3. теги SPDX в файле
 *   4. CLI-fallback
 *
 * Sidecar и теги передаются в reuse_resolve_for_file как in-file
 * источники; их агрегация с REUSE.toml происходит внутри.
 *
 * configs может быть NULL (тогда работает только in-file + fallback).
 * Возвращает 0 при успехе, -1 если ни одного источника. */
int spdx_resolve_license(ReuseConfig **configs, int config_count,
                         const char *fullpath,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out);

/* Совместимость: один REUSE.toml. */
int spdx_resolve_license_single(ReuseConfig *config,
                                const char *fullpath,
                                const char *default_license,
                                const char *default_copyright,
                                FileLicenseInfo *out);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_LIC_H */
