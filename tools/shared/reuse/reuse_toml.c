/* reuse_toml.c - REUSE.toml parser, OS/2 API style
 * (C89 + Watcom extensions) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reuse_toml.h"
#include "reuse_toml_internal.h"
#include "toml.h"

/**
 * @file reuse_toml.c
 * @brief Implementation of the REUSE.toml parser.
 *
 * Conforms to:
 *   - https://reuse.software/spec-3.3/
 */

/* ==================================================================
 * Small helpers
 * ================================================================== */

static char *dup_n(const char *s, size_t n) {
    char *r;
    r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

static char *dup_str(const char *s) {
    return dup_n(s, strlen(s));
}

/* Convert a TOML node holding a string into a malloc'd string. */
static char *node_to_str(HTOMLNODE hNode) {
    ULONG ulSize = 0;
    char *buf;
    if (TomlNodeGetString(hNode, NULL, 0, &ulSize) != TOML_NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    buf = (char*)malloc(ulSize);
    if (!buf) return NULL;
    if (TomlNodeGetString(hNode, buf, ulSize, NULL) != TOML_NO_ERROR) {
        free(buf);
        return NULL;
    }
    return buf;
}

/* ==================================================================
 * Field reading
 * ================================================================== */

static char *read_scalar_string(HTOMLNODE hTable, PCSZ pszKey) {
    HTOMLNODE hChild = NULLHANDLE;
    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != TOML_NO_ERROR)
        return NULL;
    return node_to_str(hChild);
}

/* Read a value that is either a string or an array of strings. Array
 * elements are joined with pszSep. */
static char *read_string_or_join(HTOMLNODE hTable, PCSZ pszKey,
                                 PCSZ pszSep) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;
    char *result = NULL;
    size_t sep_len = strlen(pszSep);

    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != TOML_NO_ERROR)
        return NULL;
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR)
        return NULL;

    if (ulType == TOML_TYPE_STRING) {
        return node_to_str(hChild);
    }
    if (ulType != TOML_TYPE_ARRAY) return NULL;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *part;
        size_t rlen, plen;
        char *nresult;

        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR) {
            free(result);
            return NULL;
        }
        part = node_to_str(hElem);
        if (!part) { free(result); return NULL; }

        rlen = result ? strlen(result) : 0;
        plen = strlen(part);
        nresult = (char*)realloc(result,
            rlen + (rlen ? sep_len : 0) + plen + 1);
        if (!nresult) { free(part); free(result); return NULL; }
        result = nresult;
        if (rlen) {
            memcpy(result + rlen, pszSep, sep_len);
            rlen += sep_len;
        }
        memcpy(result + rlen, part, plen);
        result[rlen + plen] = '\0';
        free(part);
    }
    return result;
}

static int add_path(REUSEANN *ann, const char *value) {
    char **na = (char**)realloc(ann->paPaths,
        (size_t)(ann->ulPathCount + 1) * sizeof(char*));
    if (!na) return -1;
    ann->paPaths = na;
    ann->paPaths[ann->ulPathCount] = dup_str(value);
    if (!ann->paPaths[ann->ulPathCount]) return -1;
    ann->ulPathCount++;
    return 0;
}

static int read_paths_into(HTOMLNODE hTable, REUSEANN *ann) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;

    if (TomlNodeGetTableEntryByKey(hTable, "path", &hChild) != TOML_NO_ERROR)
        return -1;
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR) return -1;

    if (ulType == TOML_TYPE_STRING) {
        char *s = node_to_str(hChild);
        if (!s) return -1;
        if (add_path(ann, s) != 0) { free(s); return -1; }
        free(s);
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -1;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *s;
        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR)
            return -1;
        s = node_to_str(hElem);
        if (!s) return -1;
        if (add_path(ann, s) != 0) { free(s); return -1; }
        free(s);
    }
    return 0;
}

static int read_contributors_into(HTOMLNODE hTable, REUSEANN *ann) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;

    if (TomlNodeGetTableEntryByKey(hTable, "SPDX-FileContributor",
                                   &hChild) != TOML_NO_ERROR)
        return 0;
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR) return -1;

    if (ulType == TOML_TYPE_STRING) {
        char *s = node_to_str(hChild);
        char **na;
        if (!s) return -1;
        na = (char**)realloc(ann->paContributors,
                             (size_t)(ann->ulContributorCount + 1) *
                             sizeof(char*));
        if (!na) { free(s); return -1; }
        ann->paContributors = na;
        ann->paContributors[ann->ulContributorCount++] = s;
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -1;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *s;
        char **na;
        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR)
            return -1;
        s = node_to_str(hElem);
        if (!s) return -1;
        na = (char**)realloc(ann->paContributors,
                             (size_t)(ann->ulContributorCount + 1) *
                             sizeof(char*));
        if (!na) { free(s); return -1; }
        ann->paContributors = na;
        ann->paContributors[ann->ulContributorCount++] = s;
    }
    return 0;
}

static void read_precedence(HTOMLNODE hTable, REUSEANN *ann) {
    char *s = read_scalar_string(hTable, "precedence");
    if (!s) return;
    if (strcmp(s, "closest") == 0)
        ann->ulPrecedence = REUSE_PRECEDENCE_CLOSEST;
    else if (strcmp(s, "aggregate") == 0)
        ann->ulPrecedence = REUSE_PRECEDENCE_AGGREGATE;
    else if (strcmp(s, "override") == 0)
        ann->ulPrecedence = REUSE_PRECEDENCE_OVERRIDE;
    free(s);
}

static int parse_one_annotation(HTOMLNODE hItem, REUSEANN *ann) {
    memset(ann, 0, sizeof(*ann));
    ann->ulPrecedence = REUSE_PRECEDENCE_CLOSEST;

    if (read_paths_into(hItem, ann) != 0) return -1;
    if (ann->ulPathCount == 0) return -1;

    ann->pszLicense   = read_string_or_join(hItem,
                          "SPDX-License-Identifier", " AND ");
    ann->pszCopyright = read_string_or_join(hItem,
                          "SPDX-FileCopyrightText", "\n");
    if (read_contributors_into(hItem, ann) != 0) return -1;
    read_precedence(hItem, ann);

    ann->pszPackageName = read_scalar_string(hItem, "SPDX-PackageName");
    ann->pszPackageSupplier =
        read_scalar_string(hItem, "SPDX-PackageSupplier");
    ann->pszPackageDownloadLocation =
        read_scalar_string(hItem, "SPDX-PackageDownloadLocation");
    ann->pszPackageComment =
        read_scalar_string(hItem, "SPDX-PackageComment");

    return 0;
}

/* ==================================================================
 * Free helpers
 * ================================================================== */

static void ann_free(REUSEANN *ann) {
    ULONG i;
    if (!ann) return;
    for (i = 0; i < ann->ulPathCount; i++)
        free(ann->paPaths[i]);
    free(ann->paPaths);
    free(ann->pszLicense);
    free(ann->pszCopyright);
    for (i = 0; i < ann->ulContributorCount; i++)
        free(ann->paContributors[i]);
    free(ann->paContributors);
    free(ann->pszPackageName);
    free(ann->pszPackageSupplier);
    free(ann->pszPackageDownloadLocation);
    free(ann->pszPackageComment);
    memset(ann, 0, sizeof(*ann));
}

static void toml_free(PREUSETOML pt) {
    ULONG i;
    if (!pt) return;
    for (i = 0; i < pt->ulAnnotationCount; i++)
        ann_free(&pt->paAnnotations[i]);
    free(pt->paAnnotations);
    free(pt->pszSourceDir);
    free(pt);
}

/* ==================================================================
 * Common scalar-out helper
 * ================================================================== */

static APIRET copy_field_out(const char *pszValue,
                             PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    size_t n;
    if (!pszValue) return REUSE_ERROR_NOT_FOUND;
    n = strlen(pszValue);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return REUSE_NO_ERROR;
    }
    if (!pszBuf) return REUSE_ERROR_INVALID_PARAM;
    if (ulSize < n + 1) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return REUSE_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszValue, n);
    pszBuf[n] = '\0';
    if (pulUsed) *pulUsed = (ULONG)n;
    return REUSE_NO_ERROR;
}

/* ==================================================================
 * Handle helpers
 * ================================================================== */

static PREUSETOML as_toml(HREUSETOML h) { return (PREUSETOML)h; }
static PREUSEANN  as_ann(HREUSEANN h)    { return (PREUSEANN)h; }

/* ==================================================================
 * Public API
 * ================================================================== */

APIRET ReuseTomlOpen(PCSZ pszPath, HREUSETOML *phToml) {
    HTOMLDOC hDoc = NULLHANDLE;
    HTOMLNODE hVer = NULLHANDLE;
    HTOMLNODE hAnn = NULLHANDLE;
    PREUSETOML pt = NULL;
    APIRET rc;
    ULONG ulType = 0, ulCount = 0, i;
    LONGLONG llVersion = 0;

    if (!pszPath || !phToml) return REUSE_ERROR_INVALID_PARAM;
    *phToml = NULLHANDLE;

    rc = TomlOpen(pszPath, &hDoc);
    if (rc != TOML_NO_ERROR) return REUSE_ERROR_OPEN_FAILED;

    /* version (REUSE 3.3 §4.1.1) */
    rc = TomlQueryNode(hDoc, "version", &hVer);
    if (rc != TOML_NO_ERROR) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_MISSING;
    }
    if (TomlNodeGetType(hVer, &ulType) != TOML_NO_ERROR ||
        ulType != TOML_TYPE_INTEGER) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_MISSING;
    }
    TomlNodeGetInteger(hVer, &llVersion);
    if (llVersion != 1) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_UNSUP;
    }

    pt = (PREUSETOML)calloc(1, sizeof(REUSETOML));
    if (!pt) { TomlClose(hDoc); return REUSE_ERROR_OUT_OF_MEMORY; }
    pt->llVersion = llVersion;

    /* source_dir: filename without the last path component. */
    pt->pszSourceDir = dup_str(pszPath);
    if (pt->pszSourceDir) {
        char *slash = strrchr(pt->pszSourceDir, '/');
        char *backslash = strrchr(pt->pszSourceDir, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (slash) *slash = '\0';
    }

    /* [[annotations]] */
    rc = TomlQueryNode(hDoc, "annotations", &hAnn);
    if (rc == TOML_NO_ERROR) {
        if (TomlNodeGetType(hAnn, &ulType) == TOML_NO_ERROR &&
            ulType == TOML_TYPE_ARRAY) {
            TomlNodeGetArrayCount(hAnn, &ulCount);
            if (ulCount > 0) {
                pt->paAnnotations =
                    (REUSEANN*)calloc(ulCount, sizeof(REUSEANN));
                if (!pt->paAnnotations) {
                    toml_free(pt);
                    TomlClose(hDoc);
                    return REUSE_ERROR_OUT_OF_MEMORY;
                }
                pt->ulAnnotationCapacity = ulCount;
                for (i = 0; i < ulCount; i++) {
                    HTOMLNODE hItem = NULLHANDLE;
                    REUSEANN *ann = &pt->paAnnotations[pt->ulAnnotationCount];
                    rc = TomlNodeGetArrayElement(hAnn, i, &hItem);
                    if (rc != TOML_NO_ERROR) {
                        toml_free(pt);
                        TomlClose(hDoc);
                        return REUSE_ERROR_OPEN_FAILED;
                    }
                    if (parse_one_annotation(hItem, ann) != 0) {
                        toml_free(pt);
                        TomlClose(hDoc);
                        return REUSE_ERROR_ANNOTATION_NO_PATH;
                    }
                    ann->ulOrderInFile = pt->ulAnnotationCount;
                    pt->ulAnnotationCount++;
                }
            }
        }
    }

    TomlClose(hDoc);
    *phToml = (HREUSETOML)pt;
    return REUSE_NO_ERROR;
}

APIRET ReuseTomlClose(HREUSETOML hToml) {
    PREUSETOML pt;
    if (hToml == NULLHANDLE) return REUSE_NO_ERROR;
    pt = as_toml(hToml);
    if (!pt) return REUSE_ERROR_INVALID_HANDLE;
    toml_free(pt);
    return REUSE_NO_ERROR;
}

APIRET ReuseTomlGetVersion(HREUSETOML hToml, PLONGLONG pllValue) {
    PREUSETOML pt = as_toml(hToml);
    if (!pt || !pllValue) return REUSE_ERROR_INVALID_PARAM;
    *pllValue = pt->llVersion;
    return REUSE_NO_ERROR;
}

APIRET ReuseTomlGetSourceDir(HREUSETOML hToml, PSZ pszBuf,
                             ULONG ulSize, PULONG pulUsed) {
    PREUSETOML pt = as_toml(hToml);
    if (!pt) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pt->pszSourceDir, pszBuf, ulSize, pulUsed);
}

APIRET ReuseTomlGetAnnotationCount(HREUSETOML hToml, PULONG pulCount) {
    PREUSETOML pt = as_toml(hToml);
    if (!pt || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pt->ulAnnotationCount;
    return REUSE_NO_ERROR;
}

APIRET ReuseTomlGetAnnotation(HREUSETOML hToml, ULONG ulIndex,
                              HREUSEANN *phAnn) {
    PREUSETOML pt = as_toml(hToml);
    if (!pt || !phAnn) return REUSE_ERROR_INVALID_PARAM;
    *phAnn = NULLHANDLE;
    if (ulIndex >= pt->ulAnnotationCount) return REUSE_ERROR_INDEX_RANGE;
    *phAnn = (HREUSEANN)&pt->paAnnotations[ulIndex];
    return REUSE_NO_ERROR;
}

APIRET ReuseAnnGetPathCount(HREUSEANN hAnn, PULONG pulCount) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pa->ulPathCount;
    return REUSE_NO_ERROR;
}

APIRET ReuseAnnGetPath(HREUSEANN hAnn, ULONG ulIndex,
                       PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    if (ulIndex >= pa->ulPathCount) return REUSE_ERROR_INDEX_RANGE;
    return copy_field_out(pa->paPaths[ulIndex], pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetLicense(HREUSEANN hAnn,
                          PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszLicense, pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetCopyright(HREUSEANN hAnn,
                            PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszCopyright, pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetPackageName(HREUSEANN hAnn,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageName, pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetPackageSupplier(HREUSEANN hAnn,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageSupplier, pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetPackageDownloadLocation(HREUSEANN hAnn,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageDownloadLocation,
                          pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetPackageComment(HREUSEANN hAnn,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageComment, pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetContributorCount(HREUSEANN hAnn, PULONG pulCount) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pa->ulContributorCount;
    return REUSE_NO_ERROR;
}

APIRET ReuseAnnGetContributor(HREUSEANN hAnn, ULONG ulIndex,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    if (ulIndex >= pa->ulContributorCount) return REUSE_ERROR_INDEX_RANGE;
    return copy_field_out(pa->paContributors[ulIndex],
                          pszBuf, ulSize, pulUsed);
}

APIRET ReuseAnnGetPrecedence(HREUSEANN hAnn, PULONG pulPrecedence) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulPrecedence) return REUSE_ERROR_INVALID_PARAM;
    *pulPrecedence = pa->ulPrecedence;
    return REUSE_NO_ERROR;
}

APIRET ReuseAnnGetOrderInFile(HREUSEANN hAnn, PULONG pulOrder) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulOrder) return REUSE_ERROR_INVALID_PARAM;
    *pulOrder = pa->ulOrderInFile;
    return REUSE_NO_ERROR;
}
