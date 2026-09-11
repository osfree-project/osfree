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
    char license[256];
    char copyright[512];
    LicenseSource source;        /* первый источник, давший хоть что-то */
    int license_from_default;    /* 1, если license взята из CLI */
    int copyright_from_default;  /* 1, если copyright взят из CLI */
} FileLicenseInfo;

/* Разрешение лицензии с учётом иерархии REUSE.toml.
 *
 * Порядок источников:
 *   1. REUSE.toml (для определения precedence)
 *   2. sidecar <file>.license
 *   3. теги SPDX в файле
 *   4. CLI-fallback
 *
 * Precedence:
 *   override  - только REUSE.toml, in-file (sidecar/теги) игнорируется;
 *   aggregate - REUSE.toml + in-file через AND;
 *   closest   - in-file выигрывает, иначе REUSE.toml.
 *
 * configs может быть NULL (тогда REUSE.toml не используется).
 * Возвращает 0 при успехе, -1 если ни одного источника. */
int spdx_resolve_license(ReuseConfig **configs, int config_count,
                         const char *fullpath,
                         const char *filename,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out);

/* Совместимость: один REUSE.toml. */
int spdx_resolve_license_single(ReuseConfig *config,
                                const char *fullpath,
                                const char *filename,
                                const char *default_license,
                                const char *default_copyright,
                                FileLicenseInfo *out);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_LIC_H */
