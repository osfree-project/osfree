/* spdx_sbom_scan.c - сбор списка файлов для SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_scan.h"
#include "spdx_sbom_utils.h"
#include "spdx_lic.h"
#include "spdx_utils.h"
#include "spdx_db.h"
#include "sha1_utils.h"

int sbom_fill_file_basic(const char *fullpath,
                         const char *display_name,
                         FileInfo *out) {
    char *sha1;

    memset(out, 0, sizeof(*out));
    strncpy(out->name, display_name, sizeof(out->name) - 1);

    sha1 = sha1_file(fullpath);
    if (!sha1) {
        fprintf(stderr, "Error: cannot compute SHA1 for %s\n", fullpath);
        return -1;
    }
    strncpy(out->sha1, sha1, sizeof(out->sha1) - 1);
    free(sha1);

    strncpy(out->file_type, sbom_get_file_type(display_name),
            sizeof(out->file_type) - 1);
    return 0;
}

static int validate_license(const char *fullpath, FileLicenseInfo *lic) {
    const char *bad = NULL;
    int rc;

    if (lic->license[0] == '\0') {
        fprintf(stderr, "Error: no license for file: %s\n", fullpath);
        return -1;
    }

    rc = spdx_expression_validate(lic->license, &bad);
    if (rc == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "Error: invalid SPDX license expression in %s: '%s'\n",
                fullpath, lic->license);
        return -1;
    }
    if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
        const char *p = bad;
        while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
        fprintf(stderr, "Error: unknown SPDX identifier in %s: '",
                fullpath);
        fwrite(bad, 1, (size_t)(p - bad), stderr);
        fprintf(stderr, "'\n");
        return -1;
    }

    {
        char buf[256];
        char *p;
        strncpy(buf, lic->license, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        p = strtok(buf, " \t()");
        while (p) {
            if (strcmp(p, "AND") != 0 &&
                strcmp(p, "OR")  != 0 &&
                strcmp(p, "WITH") != 0) {
                if (spdx_license_is_deprecated(p) ||
                    spdx_exception_is_deprecated(p)) {
                    fprintf(stderr,
                            "Warning: %s: deprecated SPDX identifier '%s'\n",
                            fullpath, p);
                }
            }
            p = strtok(NULL, " \t()");
        }
    }
    return 0;
}

static int process_one_file(const char *fullpath,
                            const char *display_name,
                            ReuseConfig *config,
                            const char *default_license,
                            const char *default_copyright,
                            FileList *out) {
    FileInfo info;
    FileLicenseInfo lic;

    if (sbom_fill_file_basic(fullpath, display_name, &info) != 0)
        return -1;

    if (spdx_resolve_license_single(config, fullpath, display_name,
                                    default_license, default_copyright,
                                    &lic) != 0) {
        fprintf(stderr, "Error: no license found for file: %s\n", fullpath);
        return -1;
    }

    if (validate_license(fullpath, &lic) != 0)
        return -1;

    strncpy(info.license, lic.license, sizeof(info.license) - 1);
    strncpy(info.copyright, lic.copyright, sizeof(info.copyright) - 1);

    filelist_add(out, &info);
    return 0;
}

int sbom_collect_files(const SpdxStrList *paths,
                       ReuseConfig *config,
                       const char *default_license,
                       const char *default_copyright,
                       FileList *out) {
    int i;
    for (i = 0; i < paths->count; i++) {
        const char *full = paths->items[i];
        const char *name = spdx_get_file_name(full);
        if (process_one_file(full, name, config,
                             default_license, default_copyright, out) != 0)
            return -1;
    }
    return 0;
}
