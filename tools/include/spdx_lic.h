/* spdx_lic.h - определение лицензии/копирайта файла (C89) */
#ifndef SPDX_LIC_H
#define SPDX_LIC_H

#include <reuse_parser.h>

/* Источник, откуда взята лицензия/копирайт */
typedef enum {
    LICENSE_SRC_NONE = 0,
    LICENSE_SRC_TAG,
    LICENSE_SRC_SIDECAR,
    LICENSE_SRC_REUSE,
    LICENSE_SRC_DEFAULT
} LicenseSource;

typedef struct {
    char license[256];
    char copyright[512];
    LicenseSource source;
} FileLicenseInfo;

/* Определяет лицензию и копирайт для файла. Возвращает 0 при успехе.
 * Порядок источников:
 *   1. SPDX-теги внутри файла
 *   2. sidecar <fullpath>.license
 *   3. REUSE.toml (включая path="**")
 *   4. default_license / default_copyright (CLI fallback)
 *
 * При неудаче возвращает -1 и выставляет out->source = LICENSE_SRC_NONE. */
int spdx_resolve_license(const char *fullpath,
                         const char *filename,
                         ReuseConfig *config,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out);

#endif /* SPDX_LIC_H */
