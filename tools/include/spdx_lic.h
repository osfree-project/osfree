/* spdx_lic.h - определение лицензии/копирайта файла (C89) */
#ifndef SPDX_LIC_H
#define SPDX_LIC_H

#include <reuse_parser.h>

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
    LicenseSource source;             /* первый источник, давший хоть что-то */
    int license_from_default;         /* 1, если license вз€та из CLI */
    int copyright_from_default;       /* 1, если copyright вз€т из CLI */
} FileLicenseInfo;

int spdx_resolve_license(const char *fullpath,
                         const char *filename,
                         ReuseConfig *config,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out);

#endif /* SPDX_LIC_H */
