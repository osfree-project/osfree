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

static void copy_safe(char *dst, size_t dst_size, const char *src) {
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static int try_tags(const char *fullpath, FileLicenseInfo *out) {
    char *tag_lic = file_get_spdx_license(fullpath);
    char *tag_copy = file_get_spdx_copyright(fullpath);
    int found = 0;

    if (tag_lic && tag_lic[0]) {
        copy_safe(out->license, sizeof(out->license), tag_lic);
        found = 1;
    }
    if (tag_copy && tag_copy[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), tag_copy);
    }
    if (tag_lic) free(tag_lic);
    if (tag_copy) free(tag_copy);

    if (found) {
        out->source = LICENSE_SRC_TAG;
        return 0;
    }
    return -1;
}

static int try_sidecar(const char *fullpath, FileLicenseInfo *out) {
    char sidecar[1024];
    int exists;
    char *lic, *cop;
    int found = 0;

    snprintf(sidecar, sizeof(sidecar), "%s.license", fullpath);
#ifdef __LINUX__
    exists = (access(sidecar, F_OK) == 0);
#else
    exists = (_access(sidecar, 0) == 0);
#endif
    if (!exists) return -1;

    lic = file_get_spdx_license(sidecar);
    cop = file_get_spdx_copyright(sidecar);
    if (lic && lic[0]) {
        copy_safe(out->license, sizeof(out->license), lic);
        found = 1;
    }
    if (cop && cop[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), cop);
    }
    if (lic) free(lic);
    if (cop) free(cop);

    if (found) {
        out->source = LICENSE_SRC_SIDECAR;
        return 0;
    }
    return -1;
}

static int try_reuse(const char *filename, ReuseConfig *config,
                     FileLicenseInfo *out) {
    const char *lic;
    const char *cop;
    int found = 0;

    if (!config) return -1;
    lic = find_license_for_file(config, filename);
    cop = find_copyright_for_file(config, filename);
    if (lic && lic[0]) {
        copy_safe(out->license, sizeof(out->license), lic);
        found = 1;
    }
    if (cop && cop[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), cop);
    }
    if (found) {
        out->source = LICENSE_SRC_REUSE;
        return 0;
    }
    return -1;
}

static int try_default(const char *default_license,
                       const char *default_copyright,
                       FileLicenseInfo *out) {
    int found = 0;
    if (default_license && default_license[0]) {
        copy_safe(out->license, sizeof(out->license), default_license);
        found = 1;
    }
    if (default_copyright && default_copyright[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), default_copyright);
    }
    if (found) {
        out->source = LICENSE_SRC_DEFAULT;
        return 0;
    }
    return -1;
}

int spdx_resolve_license(const char *fullpath,
                         const char *filename,
                         ReuseConfig *config,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out) {
    out->license[0] = '\0';
    out->copyright[0] = '\0';
    out->source = LICENSE_SRC_NONE;

    if (try_tags(fullpath, out) == 0) return 0;
    if (try_sidecar(fullpath, out) == 0) return 0;
    if (try_reuse(filename, config, out) == 0) return 0;
    if (try_default(default_license, default_copyright, out) == 0) return 0;

    return -1;
}
