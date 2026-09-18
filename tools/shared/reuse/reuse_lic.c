/* reuse_lic.c - per-file license/copyright resolution (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reuse_lic.h"
#include "spdx_tag.h"
#include "spdx_utils.h"
#include "spdx_db.h"

#ifdef __LINUX__
#include <unistd.h>
#else
#include <io.h>
#endif

/* ------------------------------------------------------------------ */
/* Utilities                                                           */
/* ------------------------------------------------------------------ */

static void copy_safe(char *dst, size_t dst_size, const char *src) {
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

/* Copy with a truncation warning. */
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

/* Remove trailing whitespace and comment-closing markers. */
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
/* In-file sources                                                     */
/* ------------------------------------------------------------------ */

static int read_sidecar(const char *fullpath,
                        char **out_license, char **out_copyright) {
    char sidecar[1024];
    int exists;
    char *lic = NULL, *cop = NULL;

    *out_license = NULL;
    *out_copyright = NULL;

    snprintf(sidecar, sizeof(sidecar), "%s.license", fullpath);
#ifdef __LINUX__
    exists = (access(sidecar, F_OK) == 0);
#else
    exists = (_access(sidecar, 0) == 0);
#endif
    if (!exists) return -1;

    if (SpdxFileGetLicense(sidecar, &lic) != SPDX_TAG_NO_ERROR) lic = NULL;
    if (SpdxFileGetCopyright(sidecar, &cop) != SPDX_TAG_NO_ERROR) cop = NULL;

    if (lic && !lic[0]) { free(lic); lic = NULL; }
    if (cop && !cop[0]) { free(cop); cop = NULL; }

    *out_license = lic;
    *out_copyright = cop;

    if (!*out_license && !*out_copyright) return -1;
    return 0;
}

static int read_tags(const char *fullpath,
                     char **out_license, char **out_copyright) {
    char *lic = NULL, *cop = NULL;

    *out_license = NULL;
    *out_copyright = NULL;

    if (SpdxFileGetLicense(fullpath, &lic) != SPDX_TAG_NO_ERROR) lic = NULL;
    if (SpdxFileGetCopyright(fullpath, &cop) != SPDX_TAG_NO_ERROR) cop = NULL;

    if (lic && !lic[0]) { free(lic); lic = NULL; }
    if (cop && !cop[0]) { free(cop); cop = NULL; }

    *out_license = lic;
    *out_copyright = cop;

    if (!*out_license && !*out_copyright) return -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Resolver field extraction                                           */
/* ------------------------------------------------------------------ */

typedef APIRET (APIENTRY *ReuseFieldFn)(HREUSETREEFILE, PSZ, ULONG, PULONG);

/* Copy a single string field from a resolver handle into a fixed
 * buffer. Silent if the field is absent; warns on truncation. */
static void copy_file_field(HREUSETREEFILE hFile, ReuseFieldFn fn,
                            const char *fullpath, const char *field_name,
                            char *dst, size_t dst_size)
{
    ULONG ulSize = 0;
    APIRET rc;

    dst[0] = '\0';
    if (!hFile) return;

    rc = fn(hFile, NULL, 0, &ulSize);
    if (rc != REUSE_NO_ERROR) return;
    if (ulSize == 0) return;

    if (ulSize > dst_size) {
        char *tmp = (char*)malloc(ulSize);
        if (!tmp) return;
        rc = fn(hFile, tmp, ulSize, NULL);
        if (rc == REUSE_NO_ERROR) {
            fprintf(stderr,
                    "WARNING: %s: %s truncated (%u bytes, buffer %u).\n"
                    "         Consider shortening the annotation.\n",
                    fullpath, field_name,
                    (unsigned)(ulSize - 1), (unsigned)dst_size);
            copy_safe(dst, dst_size, tmp);
        }
        free(tmp);
        return;
    }
    rc = fn(hFile, dst, (ULONG)dst_size, NULL);
    if (rc != REUSE_NO_ERROR) dst[0] = '\0';
}

/* ------------------------------------------------------------------ */
/* Main function                                                       */
/* ------------------------------------------------------------------ */

APIRET APIENTRY ReuseResolveLicense(HREUSETREE hTree,
                                    PCSZ pszFullpath,
                                    PCSZ pszDefaultLicense,
                                    PCSZ pszDefaultCopyright,
                                    PREUSELICENSEINFO pOut)
{
    char *side_license = NULL;
    char *side_copyright = NULL;
    char *tag_license = NULL;
    char *tag_copyright = NULL;
    HREUSETREEFILE hFile = NULLHANDLE;
    BOOL has_reuse = FALSE_;
    ULONG precedence = 0;
    APIRET rc;

    if (!pszFullpath || !pOut) return REUSE_ERROR_INVALID_PARAM;

    /* Initialize the result. */
    pOut->license[0] = '\0';
    pOut->copyright[0] = '\0';
    pOut->contributors[0] = '\0';
    pOut->package_name[0] = '\0';
    pOut->package_supplier[0] = '\0';
    pOut->package_download_location[0] = '\0';
    pOut->package_comment[0] = '\0';
    pOut->has_package_info = 0;
    pOut->source = REUSE_LICENSE_SRC_NONE;
    pOut->license_from_default = 0;
    pOut->copyright_from_default = 0;

    /* 1. Read in-file sources. They are needed by the resolver for
     *    aggregation. Even if a matched annotation has override
     *    precedence, reading them is safe: override ignores in-file
     *    sources. */
    read_sidecar(pszFullpath, &side_license, &side_copyright);
    read_tags(pszFullpath, &tag_license, &tag_copyright);

    /* 2. Resolve REUSE.toml + in-file sources. The precedence rules
     *    (override > aggregate > closest) and aggregation are handled
     *    inside the resolver (see reuse.c). */
    if (hTree != NULLHANDLE) {
        rc = ReuseTreeResolveFile(hTree, pszFullpath, &hFile, NULL);
        if (rc != REUSE_NO_ERROR) hFile = NULLHANDLE;
    }

    /* 3. Copy fields from the resolver handle. */
    if (hFile != NULLHANDLE) {
        copy_file_field(hFile, ReuseTreeFileGetLicense,
                        pszFullpath, "SPDX-License-Identifier",
                        pOut->license, sizeof(pOut->license));
        copy_file_field(hFile, ReuseTreeFileGetCopyright,
                        pszFullpath, "SPDX-FileCopyrightText",
                        pOut->copyright, sizeof(pOut->copyright));
        copy_file_field(hFile, ReuseTreeFileGetContributors,
                        pszFullpath, "SPDX-FileContributor",
                        pOut->contributors, sizeof(pOut->contributors));
        copy_file_field(hFile, ReuseTreeFileGetPackageName,
                        pszFullpath, "SPDX-PackageName",
                        pOut->package_name, sizeof(pOut->package_name));
        copy_file_field(hFile, ReuseTreeFileGetPackageSupplier,
                        pszFullpath, "SPDX-PackageSupplier",
                        pOut->package_supplier,
                        sizeof(pOut->package_supplier));
        copy_file_field(hFile, ReuseTreeFileGetPackageDownloadLocation,
                        pszFullpath, "SPDX-PackageDownloadLocation",
                        pOut->package_download_location,
                        sizeof(pOut->package_download_location));
        copy_file_field(hFile, ReuseTreeFileGetPackageComment,
                        pszFullpath, "SPDX-PackageComment",
                        pOut->package_comment, sizeof(pOut->package_comment));

        if (ReuseTreeFileGetPrecedence(hFile, &precedence) != REUSE_NO_ERROR)
            precedence = 0;
        if (ReuseTreeFileGetHasReuse(hFile, &has_reuse) != REUSE_NO_ERROR)
            has_reuse = FALSE_;

        ReuseTreeFileClose(hFile);
    }

    if (pOut->package_name[0] || pOut->package_supplier[0] ||
        pOut->package_download_location[0] || pOut->package_comment[0]) {
        pOut->has_package_info = 1;
    }

    /* 4. Determine the source.
     *
     * The resolver's bHasReuse is TRUE if any source matched, including
     * sidecar and in-file tags. To attribute a specific origin, prefer
     * the more specific sidecar/tag over a REUSE.toml annotation when
     * sidecar or tag content was found, and fall back to REUSE when
     * only a config matched. */
    if (side_license || side_copyright) {
        pOut->source = REUSE_LICENSE_SRC_SIDECAR;
    } else if (tag_license || tag_copyright) {
        pOut->source = REUSE_LICENSE_SRC_TAG;
    } else if (has_reuse) {
        pOut->source = REUSE_LICENSE_SRC_REUSE;
    }

    /* 5. Fallback to --default-*. */
    if (pOut->license[0] == '\0' &&
        pszDefaultLicense && pszDefaultLicense[0]) {
        copy_safe(pOut->license, sizeof(pOut->license), pszDefaultLicense);
        pOut->license_from_default = 1;
        if (pOut->source == REUSE_LICENSE_SRC_NONE)
            pOut->source = REUSE_LICENSE_SRC_DEFAULT;
    }
    if (pOut->copyright[0] == '\0' &&
        pszDefaultCopyright && pszDefaultCopyright[0]) {
        copy_safe(pOut->copyright, sizeof(pOut->copyright),
                  pszDefaultCopyright);
        pOut->copyright_from_default = 1;
        if (pOut->source == REUSE_LICENSE_SRC_NONE)
            pOut->source = REUSE_LICENSE_SRC_DEFAULT;
    }

    /* 6. Normalize the SPDX expression (canonical case). */
    if (pOut->license[0] != '\0') {
        char *normalized = spdx_normalize_license_expression(pOut->license);
        if (normalized) {
            copy_safe(pOut->license, sizeof(pOut->license), normalized);
            free(normalized);
        }
    }

    /* Defensive cleanup of trailing markers (sidecar with a comment). */
    strip_trailing_comments(pOut->license);
    strip_trailing_comments(pOut->copyright);

    /* 7. Release temporaries. */
    if (side_license) free(side_license);
    if (side_copyright) free(side_copyright);
    if (tag_license) free(tag_license);
    if (tag_copyright) free(tag_copyright);

    if (pOut->license[0] == '\0' && pOut->copyright[0] == '\0')
        return REUSE_ERROR_NOT_FOUND;
    return REUSE_NO_ERROR;
}
