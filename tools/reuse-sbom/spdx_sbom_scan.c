/* spdx_sbom_scan.c - file and snippet collection for SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_scan.h"
#include "spdx_sbom_utils.h"
#include "reuse_lic.h"
#include "spdx_utils.h"
#include "spdx_db.h"
#include "spdx_tag.h"
#include "sha1_utils.h"

int sbom_fill_file_basic(const char *fullpath,
                         const char *display_name,
                         FileInfo *out) {
    char *sha1;

    memset(out, 0, sizeof(*out));
    strncpy(out->name, display_name, sizeof(out->name) - 1);

    sha1 = sha1_file(fullpath);
    if (!sha1) {
        fprintf(stderr,
                "ERROR: cannot compute SHA1 for %s\n"
                "       Check that the file exists and is readable.\n",
                fullpath);
        return -1;
    }
    strncpy(out->sha1, sha1, sizeof(out->sha1) - 1);
    free(sha1);

    strncpy(out->file_type, sbom_get_file_type(display_name),
            sizeof(out->file_type) - 1);
    return 0;
}

static int validate_license(const char *fullpath, REUSELICENSEINFO *lic) {
    const char *bad = NULL;
    int rc;

    if (lic->license[0] == '\0') {
        fprintf(stderr,
                "ERROR: no license information for file: %s\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier: <id>' tag in the "
                "file;\n"
                "         - or create '<file>.license' sidecar;\n"
                "         - or add a [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        return -1;
    }

    rc = spdx_expression_validate(lic->license, &bad);
    if (rc == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "ERROR: %s: invalid SPDX license expression: '%s'\n"
                "       Fix the expression according to the SPDX grammar:\n"
                "         https://spdx.github.io/spdx-spec/v2.3/"
                "SPDX-license-expressions/\n",
                fullpath, lic->license);
        return -1;
    }
    if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
        const char *p = bad;
        while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
        fprintf(stderr,
                "ERROR: %s: unknown SPDX identifier: '", fullpath);
        fwrite(bad, 1, (size_t)(p - bad), stderr);
        fprintf(stderr,
                "'\n"
                "       Not present in SPDX License List. Fix one of:\n"
                "         - correct the identifier;\n"
                "         - or use a 'LicenseRef-' identifier for a custom "
                "license.\n"
                "       See https://spdx.org/licenses/ for the full list.\n");
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
                            "WARNING: %s: deprecated SPDX identifier '%s'.\n"
                            "         The SPDX License List marks this "
                            "identifier deprecated.\n"
                            "         Replace it with the current identifier\n"
                            "         (usually a '-only' or '-or-later' "
                            "variant).\n"
                            "         See https://spdx.org/licenses/ for the "
                            "recommended replacement.\n",
                            fullpath, p);
                }
            }
            p = strtok(NULL, " \t()");
        }
    }
    return 0;
}

/* Collect all snippets of one file into the shared SBOM list.
 * For each snippet, verify that a license is present (error if not)
 * and fill the SnippetInfo fields. */
static int collect_file_snippets(const char *fullpath,
                                 const char *display_name,
                                 SnippetList *snippets) {
    SPDXSNIPPETLIST raw;
    int i;
    int rc = 0;

    if (SpdxFileGetSnippets(fullpath, &raw) != SPDX_TAG_NO_ERROR) {
        return -1;
    }

    for (i = 0; i < raw.nCount; i++) {
        SPDXSNIPPET *rs = &raw.paItems[i];
        SnippetInfo *s;

        if (!rs->pszLicense || rs->pszLicense[0] == '\0') {
            fprintf(stderr,
                    "ERROR: %s:%d-%d: snippet has no "
                    "SPDX-License-Identifier.\n"
                    "       Fix one of:\n"
                    "         - add 'SPDX-License-Identifier: <id>' inside "
                    "the snippet block;\n"
                    "         - or remove SPDX-SnippetBegin/SPDX-SnippetEnd "
                    "if the code is not a snippet.\n",
                    fullpath, rs->nLineStart, rs->nLineEnd);
            rc = -1;
            continue;
        }

        {
            const char *bad = NULL;
            int vrc = spdx_expression_validate(rs->pszLicense, &bad);
            if (vrc != SPDX_EXPR_OK) {
                const char *p = bad;
                fprintf(stderr,
                        "ERROR: %s:%d-%d: invalid SPDX license expression: '",
                        fullpath, rs->nLineStart, rs->nLineEnd);
                if (bad) {
                    while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
                    fwrite(bad, 1, (size_t)(p - bad), stderr);
                }
                fprintf(stderr,
                        "'\n"
                        "       See https://spdx.github.io/spdx-spec/v2.3/"
                        "SPDX-license-expressions/\n");
                rc = -1;
                continue;
            }
        }

        s = snippetlist_add(snippets);
        snprintf(s->spdx_id, sizeof(s->spdx_id),
                 "SPDXRef-Snippet-%d", snippets->count);
        snprintf(s->from_file_id, sizeof(s->from_file_id),
                 "SPDXRef-File-%s", display_name);
        strncpy(s->from_file_name, display_name,
                sizeof(s->from_file_name) - 1);
        s->line_start = rs->nLineStart;
        s->line_end   = rs->nLineEnd;
        strncpy(s->license, rs->pszLicense, sizeof(s->license) - 1);
        if (rs->pszCopyright)
            strncpy(s->copyright, rs->pszCopyright,
                    sizeof(s->copyright) - 1);
    }

    SpdxSnippetListFree(&raw);
    return rc;
}

static int process_one_file(const char *fullpath,
                            const char *display_name,
                            HREUSETREE hTree,
                            const char *default_license,
                            const char *default_copyright,
                            FileList *out,
                            SnippetList *snippets) {
    FileInfo info;
    REUSELICENSEINFO lic;

    if (sbom_fill_file_basic(fullpath, display_name, &info) != 0)
        return -1;

    if (ReuseResolveLicense(hTree, fullpath,
                            default_license, default_copyright,
                            &lic) != REUSE_NO_ERROR) {
        fprintf(stderr,
                "ERROR: no license information for file: %s\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier: <id>' tag in the "
                "file;\n"
                "         - or create '<file>.license' sidecar;\n"
                "         - or add a [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        return -1;
    }

    if (validate_license(fullpath, &lic) != 0)
        return -1;

    strncpy(info.license, lic.license, sizeof(info.license) - 1);
    strncpy(info.copyright, lic.copyright, sizeof(info.copyright) - 1);

    filelist_add(out, &info);

    if (snippets) {
        if (collect_file_snippets(fullpath, display_name, snippets) != 0)
            return -1;
    }
    return 0;
}

int sbom_collect_files(const SpdxStrList *paths,
                       HREUSETREE hTree,
                       const char *default_license,
                       const char *default_copyright,
                       FileList *out,
                       SnippetList *snippets) {
    int i;
    for (i = 0; i < paths->count; i++) {
        const char *full = paths->items[i];
        const char *name = spdx_get_file_name(full);
        if (process_one_file(full, name, hTree,
                             default_license, default_copyright,
                             out, snippets) != 0)
            return -1;
    }
    return 0;
}
