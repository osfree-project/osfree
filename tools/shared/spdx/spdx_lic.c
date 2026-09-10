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

/* ”бирает хвостовые артефакты комментари€ и whitespace.
 * ”дал€ютс€: закрывающий блочный комментарий (звЄздочка плюс слэш),
 * строчный комментарий (два слэша), пробелы, табул€ции, CR, LF. */
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

int spdx_resolve_license(const char *fullpath,
                         const char *filename,
                         ReuseConfig *config,
                         const char *default_license,
                         const char *default_copyright,
                         FileLicenseInfo *out) {
    char *tag_lic = NULL;
    char *tag_copy = NULL;
    char sidecar[1024];
    int sidecar_exists;
    char *side_lic = NULL;
    char *side_copy = NULL;

    out->license[0] = '\0';
    out->copyright[0] = '\0';
    out->source = LICENSE_SRC_NONE;
    out->license_from_default = 0;
    out->copyright_from_default = 0;

    /* 1. “еги в файле */
    tag_lic = file_get_spdx_license(fullpath);
    tag_copy = file_get_spdx_copyright(fullpath);
    if (tag_lic && tag_lic[0]) {
        copy_safe(out->license, sizeof(out->license), tag_lic);
        out->source = LICENSE_SRC_TAG;
    }
    if (tag_copy && tag_copy[0]) {
        copy_safe(out->copyright, sizeof(out->copyright), tag_copy);
        if (out->source == LICENSE_SRC_NONE) out->source = LICENSE_SRC_TAG;
    }
    if (tag_lic) free(tag_lic);
    if (tag_copy) free(tag_copy);

    /* 2. Sidecar */
    if (out->license[0] == '\0' || out->copyright[0] == '\0') {
        snprintf(sidecar, sizeof(sidecar), "%s.license", fullpath);
#ifdef __LINUX__
        sidecar_exists = (access(sidecar, F_OK) == 0);
#else
        sidecar_exists = (_access(sidecar, 0) == 0);
#endif
        if (sidecar_exists) {
            side_lic = file_get_spdx_license(sidecar);
            side_copy = file_get_spdx_copyright(sidecar);
            if (out->license[0] == '\0' && side_lic && side_lic[0]) {
                copy_safe(out->license, sizeof(out->license), side_lic);
                if (out->source == LICENSE_SRC_NONE)
                    out->source = LICENSE_SRC_SIDECAR;
            }
            if (out->copyright[0] == '\0' && side_copy && side_copy[0]) {
                copy_safe(out->copyright, sizeof(out->copyright), side_copy);
                if (out->source == LICENSE_SRC_NONE)
                    out->source = LICENSE_SRC_SIDECAR;
            }
            if (side_lic) free(side_lic);
            if (side_copy) free(side_copy);
        }
    }

    /* 3. REUSE.toml */
    if (config && (out->license[0] == '\0' || out->copyright[0] == '\0')) {
        if (out->license[0] == '\0') {
            const char *lic = find_license_for_file(config, filename);
            if (lic && lic[0]) {
                copy_safe(out->license, sizeof(out->license), lic);
                if (out->source == LICENSE_SRC_NONE)
                    out->source = LICENSE_SRC_REUSE;
            }
        }
        if (out->copyright[0] == '\0') {
            const char *cop = find_copyright_for_file(config, filename);
            if (cop && cop[0]) {
                copy_safe(out->copyright, sizeof(out->copyright), cop);
                if (out->source == LICENSE_SRC_NONE)
                    out->source = LICENSE_SRC_REUSE;
            }
        }
    }

    /* 4. CLI-fallback по каждому полю независимо */
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

    strip_trailing_comments(out->license);
    strip_trailing_comments(out->copyright);

    if (out->license[0] == '\0' && out->copyright[0] == '\0')
        return -1;
    return 0;
}
