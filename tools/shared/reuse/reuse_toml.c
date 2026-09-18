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
 *
 * Return conventions for read_* helpers:
 *    0  - success (value stored, possibly NULL if the key is absent)
 *   -1  - key present but has an unsupported type
 *   -2  - out of memory
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

/* Read "path": string or array of strings. Each element becomes one
 * entry in ann->paPaths.
 *
 * Returns:
 *    0  - ok
 *   -1  - key absent (REUSE_ERROR_ANNOT_NO_PATH)
 *   -2  - key present but wrong type (REUSE_ERROR_ANNOT_BAD_PATH)
 *   -3  - out of memory
 */
static int read_paths_into(HTOMLNODE hTable, REUSEANN *ann) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, i;

    if (TomlNodeGetTableEntryByKey(hTable, "path", &hChild) != TOML_NO_ERROR)
        return -1;
    if (TomlNodeGetType(hChild, &ulType) != TOML_NO_ERROR)
        return -2;

    if (ulType == TOML_TYPE_STRING) {
        char *s = node_to_str(hChild);
        if (!s) return -3;
        if (add_path(ann, s) != 0) { free(s); return -3; }
        free(s);
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -2;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (i = 0; i < ulCount; i++) {
        HTOMLNODE hElem = NULLHANDLE;
        char *s;
        if (TomlNodeGetArrayElement(hChild, i, &hElem) != TOML_NO_ERROR)
            return -2;
        if (TomlNodeGetType(hElem, &ulType) != TOML_NO_ERROR)
            return -2;
        if (ulType != TOML_TYPE_STRING) return -2;
        s = node_to_str(hElem);
        if (!s) return -3;
        if (add_path(ann, s) != 0) { free(s); return -3; }
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
        if (!s) return -2;
        na = (char**)realloc(ann->paContributors,
                             (size_t)(ann->ulContributorCount + 1) *
                             sizeof(char*));
        if (!na) { free(s); return -2; }
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
        if (!s) return -2;
        na = (char**)realloc(ann->paContributors,
                             (size_t)(ann->ulContributorCount + 1) *
                             sizeof(char*));
        if (!na) { free(s); return -2; }
        ann->paContributors = na;
        ann->paContributors[ann->ulContributorCount++] = s;
    }
    return 0;
}

static int read_precedence(HTOMLNODE hTable, REUSEANN *ann) {
    char *s = read_scalar_string(hTable, "precedence");
    if (!s) return 0;
    if (strcmp(s, "closest") == 0)
        ann->ulPrecedence = REUSE_PRECEDENCE_CLOSEST;
    else if (strcmp(s, "aggregate") == 0)
        ann->ulPrecedence = REUSE_PRECEDENCE_AGGREGATE;
    else if (strcmp(s, "override") == 0)
        ann->ulPrecedence = REUSE_PRECEDENCE_OVERRIDE;
    free(s);
    return 0;
}

/* Parse one [[annotations]] entry. Returns 0 on success, APIRET-coded
 * error otherwise. */
static APIRET parse_one_annotation(HTOMLNODE hItem, REUSEANN *ann) {
    APIRET rc;

    memset(ann, 0, sizeof(*ann));
    ann->ulPrecedence = REUSE_PRECEDENCE_CLOSEST;

    rc = read_paths_into(hItem, ann);
    if (rc == -1) return REUSE_ERROR_ANNOT_NO_PATH;
    if (rc == -2) return REUSE_ERROR_ANNOT_BAD_PATH;
    if (rc == -3) return REUSE_ERROR_OUT_OF_MEMORY;
    if (ann->ulPathCount == 0) return REUSE_ERROR_ANNOT_NO_PATH;

    ann->pszLicense   = read_string_or_join(hItem,
                          "SPDX-License-Identifier", " AND ");
    ann->pszCopyright = read_string_or_join(hItem,
                          "SPDX-FileCopyrightText", "\n");

    rc = read_contributors_into(hItem, ann);
    if (rc == -1) return REUSE_ERROR_ANNOT_BAD_FIELD;
    if (rc == -2) return REUSE_ERROR_OUT_OF_MEMORY;

    read_precedence(hItem, ann);

    ann->pszPackageName = read_scalar_string(hItem, "SPDX-PackageName");
    ann->pszPackageSupplier =
        read_scalar_string(hItem, "SPDX-PackageSupplier");
    ann->pszPackageDownloadLocation =
        read_scalar_string(hItem, "SPDX-PackageDownloadLocation");
    ann->pszPackageComment =
        read_scalar_string(hItem, "SPDX-PackageComment");

    return REUSE_NO_ERROR;
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

static void doc_free(PREUSETOMLDOC pd) {
    ULONG i;
    if (!pd) return;
    for (i = 0; i < pd->ulAnnotationCount; i++)
        ann_free(&pd->paAnnotations[i]);
    free(pd->paAnnotations);
    free(pd->pszSourceDir);
    free(pd);
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

static PREUSETOMLDOC as_doc(HREUSETOML h) { return (PREUSETOMLDOC)h; }
static PREUSEANN     as_ann(HREUSEANN h)   { return (PREUSEANN)h; }

/* ==================================================================
 * Public API
 * ================================================================== */

APIRET APIENTRY ReuseOpen(PCSZ pszPath, HREUSETOML *phToml) {
    HTOMLDOC hDoc = NULLHANDLE;
    HTOMLNODE hVer = NULLHANDLE;
    HTOMLNODE hAnn = NULLHANDLE;
    PREUSETOMLDOC pd = NULL;
    APIRET rc;
    ULONG ulType = 0, ulCount = 0, i;
    LONGLONG llVersion = 0;

    if (!pszPath || !phToml) return REUSE_ERROR_INVALID_PARAM;
    *phToml = NULLHANDLE;

    rc = TomlOpen(pszPath, &hDoc);
    if (rc == TOML_ERROR_INVALID_PARAM)
        return REUSE_ERROR_INVALID_PARAM;
    if (rc == TOML_ERROR_OPEN_FAILED)
        return REUSE_ERROR_OPEN_FAILED;
    if (rc == TOML_ERROR_READ_FAILED)
        return REUSE_ERROR_READ_FAILED;
    if (rc == TOML_ERROR_OUT_OF_MEMORY)
        return REUSE_ERROR_OUT_OF_MEMORY;
    if (rc != TOML_NO_ERROR)
        return REUSE_ERROR_SYNTAX;

    /* version (REUSE 3.3 §4.1.1) */
    rc = TomlQueryNode(hDoc, "version", &hVer);
    if (rc != TOML_NO_ERROR) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_MISSING;
    }
    if (TomlNodeGetType(hVer, &ulType) != TOML_NO_ERROR) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_MISSING;
    }
    if (ulType != TOML_TYPE_INTEGER) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_NOT_INT;
    }
    TomlNodeGetInteger(hVer, &llVersion);
    if (llVersion != 1) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_UNSUP;
    }

    pd = (PREUSETOMLDOC)calloc(1, sizeof(REUSETOMLDOC));
    if (!pd) { TomlClose(hDoc); return REUSE_ERROR_OUT_OF_MEMORY; }
    pd->llVersion = llVersion;

    /* source_dir: filename without the last path component. */
    pd->pszSourceDir = dup_str(pszPath);
    if (pd->pszSourceDir) {
        char *slash = strrchr(pd->pszSourceDir, '/');
        char *backslash = strrchr(pd->pszSourceDir, '\\');
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
                pd->paAnnotations =
                    (REUSEANN*)calloc(ulCount, sizeof(REUSEANN));
                if (!pd->paAnnotations) {
                    doc_free(pd);
                    TomlClose(hDoc);
                    return REUSE_ERROR_OUT_OF_MEMORY;
                }
                pd->ulAnnotationCapacity = ulCount;
                for (i = 0; i < ulCount; i++) {
                    HTOMLNODE hItem = NULLHANDLE;
                    REUSEANN *ann = &pd->paAnnotations[pd->ulAnnotationCount];
                    APIRET arc;
                    rc = TomlNodeGetArrayElement(hAnn, i, &hItem);
                    if (rc != TOML_NO_ERROR) {
                        doc_free(pd);
                        TomlClose(hDoc);
                        return REUSE_ERROR_SYNTAX;
                    }
                    arc = parse_one_annotation(hItem, ann);
                    if (arc != REUSE_NO_ERROR) {
                        doc_free(pd);
                        TomlClose(hDoc);
                        return arc;
                    }
                    ann->ulOrderInFile = pd->ulAnnotationCount;
                    pd->ulAnnotationCount++;
                }
            }
        }
    }

    TomlClose(hDoc);
    *phToml = (HREUSETOML)pd;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseClose(HREUSETOML hToml) {
    PREUSETOMLDOC pd;
    if (hToml == NULLHANDLE) return REUSE_NO_ERROR;
    pd = as_doc(hToml);
    if (!pd) return REUSE_ERROR_INVALID_HANDLE;
    doc_free(pd);
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseGetVersion(HREUSETOML hToml, PLONGLONG pllValue) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd || !pllValue) return REUSE_ERROR_INVALID_PARAM;
    *pllValue = pd->llVersion;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseGetSourceDir(HREUSETOML hToml, PSZ pszBuf,
                                  ULONG ulSize, PULONG pulUsed) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pd->pszSourceDir, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseGetAnnotationCount(HREUSETOML hToml, PULONG pulCount) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pd->ulAnnotationCount;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseGetAnnotation(HREUSETOML hToml, ULONG ulIndex,
                                   HREUSEANN *phAnn) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd || !phAnn) return REUSE_ERROR_INVALID_PARAM;
    *phAnn = NULLHANDLE;
    if (ulIndex >= pd->ulAnnotationCount) return REUSE_ERROR_INDEX_RANGE;
    *phAnn = (HREUSEANN)&pd->paAnnotations[ulIndex];
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseAnnGetPathCount(HREUSEANN hAnn, PULONG pulCount) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pa->ulPathCount;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseAnnGetPath(HREUSEANN hAnn, ULONG ulIndex,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    if (ulIndex >= pa->ulPathCount) return REUSE_ERROR_INDEX_RANGE;
    return copy_field_out(pa->paPaths[ulIndex], pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetLicense(HREUSEANN hAnn,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszLicense, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetCopyright(HREUSEANN hAnn,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszCopyright, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetPackageName(HREUSEANN hAnn,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageName, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetPackageSupplier(HREUSEANN hAnn,
                                           PSZ pszBuf, ULONG ulSize,
                                           PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageSupplier, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetPackageDownloadLocation(HREUSEANN hAnn,
                                                   PSZ pszBuf, ULONG ulSize,
                                                   PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageDownloadLocation,
                          pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetPackageComment(HREUSEANN hAnn,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    return copy_field_out(pa->pszPackageComment, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetContributorCount(HREUSEANN hAnn, PULONG pulCount) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pa->ulContributorCount;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseAnnGetContributor(HREUSEANN hAnn, ULONG ulIndex,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return REUSE_ERROR_INVALID_PARAM;
    if (ulIndex >= pa->ulContributorCount) return REUSE_ERROR_INDEX_RANGE;
    return copy_field_out(pa->paContributors[ulIndex],
                          pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseAnnGetPrecedence(HREUSEANN hAnn, PULONG pulPrecedence) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulPrecedence) return REUSE_ERROR_INVALID_PARAM;
    *pulPrecedence = pa->ulPrecedence;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseAnnGetOrderInFile(HREUSEANN hAnn, PULONG pulOrder) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulOrder) return REUSE_ERROR_INVALID_PARAM;
    *pulOrder = pa->ulOrderInFile;
    return REUSE_NO_ERROR;
}
