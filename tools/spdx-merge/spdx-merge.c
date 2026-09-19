/* spdx-merge.c - merge SPDX JSON documents (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "json.h"
#include "sha1.h"
#include "sha256.h"
#include "spdx_db.h"
#include "spdx.h"
#include "ccl.h"

/**
 * @file spdx-merge.c
 * @brief Merge SPDX JSON documents.
 *
 * The merge walks externalDocumentRefs recursively, verifies each
 * checksum, resolves SPDXID collisions by renaming, and outputs the
 * merged document as SPDX 2.3 JSON.
 */

#define MAX_DOCS 100


/* ------------------------------------------------------------------ */
/* Path helpers                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Normalize a path to use '/' separators and drop '.' and
 *        '..' components.
 *
 * @param[in] path  Input path. Not NULL.
 *
 * @return malloc'd normalized path, or NULL on OOM.
 */
static char *normalize_path(const char *path) {
    char *copy = strdup(path);
    char *out = (char*)malloc(strlen(path) + 3);
    char *p, *q;
    if (!copy || !out) { free(copy); free(out); return NULL; }
    p = copy; q = out;
#ifdef _WIN32
    if (isalpha((unsigned char)p[0]) && p[1] == ':') {
        *q++ = *p++; *q++ = *p++;
    }
#endif
    while (*p) {
        if (*p == '/' || *p == '\\') {
            *q++ = '/';
            while (*p == '/' || *p == '\\') p++;
        } else if (*p == '.') {
            if (p[1] == '/' || p[1] == '\\' || p[1] == '\0') {
                p++;
                while (*p == '/' || *p == '\\') p++;
            } else if (p[1] == '.' && (p[2] == '/' || p[2] == '\\' || p[2] == '\0')) {
                p += 2;
                while (*p == '/' || *p == '\\') p++;
                if (q > out) {
                    q--;
                    while (q > out && *(q - 1) != '/' && *(q - 1) != '\\') q--;
                }
            } else {
                *q++ = *p++;
            }
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    free(copy);
    return out;
}

/**
 * @brief Return the directory part of a path.
 *
 * @param[in] filepath  Path. Not NULL.
 *
 * @return malloc'd directory, or NULL on OOM.
 */
static char *get_dirname(const char *filepath) {
    char *slash = strrchr(filepath, '/');
    char *backslash = strrchr(filepath, '\\');
    char *last = (backslash && (!slash || backslash > slash)) ? backslash : slash;
    char *dir;
    size_t len;

    if (!last) return strdup(".");
    len = (size_t)(last - filepath);
    if (len == 0) return strdup("/");
    dir = (char*)malloc(len + 1);
    if (!dir) return NULL;
    memcpy(dir, filepath, len);
    dir[len] = '\0';
    return dir;
}

/**
 * @brief Join a directory and a relative path.
 *
 * @param[in] dir  Directory. Not NULL.
 * @param[in] rel  Relative path. Not NULL.
 *
 * @return malloc'd path, or NULL on OOM.
 */
static char *join_path(const char *dir, const char *rel) {
    size_t len1 = strlen(dir), len2 = strlen(rel);
    int sep = (len1 > 0 && dir[len1 - 1] != '/' && dir[len1 - 1] != '\\') ? 1 : 0;
    char *r = (char*)malloc(len1 + sep + len2 + 1);
    if (!r) return NULL;
    strcpy(r, dir);
    if (sep) strcat(r, "/");
    strcat(r, rel);
    return r;
}

/* ------------------------------------------------------------------ */
/* Processed / Rename                                                  */
/* ------------------------------------------------------------------ */

typedef struct {
    char **paths;
    int count;
} ProcessedList;

typedef struct {
    char **old_ids;
    char **new_ids;
    int count;
} RenameMap;

/**
 * @brief Check whether a path has already been processed.
 */
static int is_processed(ProcessedList *l, const char *path) {
    int i;
    for (i = 0; i < l->count; i++)
        if (strcmp(l->paths[i], path) == 0) return 1;
    return 0;
}

/**
 * @brief Record a path as processed.
 */
static void add_processed(ProcessedList *l, const char *path) {
    if (l->count >= MAX_DOCS) {
        fprintf(stderr,
                "ERROR: too many documents to merge (limit: %d).\n"
                "       Reduce the number of externalDocumentRefs.\n",
                MAX_DOCS);
        exit(EXIT_FAILURE);
    }
    l->paths[l->count] = strdup(path);
    l->count++;
}

/**
 * @brief Generate a unique replacement for a colliding SPDXID.
 */
static char *make_unique_id(const char *base, int counter) {
    char *r = (char*)malloc(strlen(base) + 24);
    sprintf(r, "%s-Duplicate%d", base, counter);
    return r;
}

/**
 * @brief Record a rename from an old SPDXID to a new one.
 */
static void add_rename(RenameMap *m, const char *old_id, const char *new_id) {
    m->old_ids = (char**)realloc(m->old_ids, (m->count + 1) * sizeof(char*));
    m->new_ids = (char**)realloc(m->new_ids, (m->count + 1) * sizeof(char*));
    m->old_ids[m->count] = strdup(old_id);
    m->new_ids[m->count] = strdup(new_id);
    m->count++;
}

/**
 * @brief Return the new ID for an old one, or NULL.
 */
static const char *get_renamed(const RenameMap *m, const char *old_id) {
    int i;
    for (i = 0; i < m->count; i++)
        if (strcmp(m->old_ids[i], old_id) == 0) return m->new_ids[i];
    return NULL;
}

/**
 * @brief Release a rename map.
 */
static void free_rename_map(RenameMap *m) {
    int i;
    for (i = 0; i < m->count; i++) {
        free(m->old_ids[i]);
        free(m->new_ids[i]);
    }
    free(m->old_ids);
    free(m->new_ids);
    m->count = 0;
}

/* ------------------------------------------------------------------ */
/* JSON read helpers                                                   */
/* ------------------------------------------------------------------ */

/**
 * @brief Read a string field into a fixed-size buffer.
 *
 * @param[in]  hNode  Node handle, or NULLHANDLE.
 * @param[out] pszBuf Output buffer. Not NULL.
 * @param[in]  ulSize Size of pszBuf.
 *
 * @return 0 on success, -1 on error.
 */
static int json_read_string(HJSONNODE hNode, char *pszBuf, size_t ulSize) {
    if (hNode == NULLHANDLE) return -1;
    if (JsonNodeGetString(hNode, pszBuf, (ULONG)ulSize, NULL) != NO_ERROR)
        return -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* DocumentRef- handling                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Strip a leading "DocumentRef-<id>:" prefix in place.
 */
static void strip_document_ref(char *str) {
    char *colon;
    if (strncmp(str, "DocumentRef-", 12) == 0) {
        colon = strchr(str, ':');
        if (colon) memmove(str, colon + 1, strlen(colon + 1) + 1);
    }
}

/**
 * @brief Recursively replace renamed SPDXIDs inside a subtree.
 *
 * Only string nodes whose key is one of the ID-carrying fields are
 * modified.
 */
static void replace_ids_in_node(HJSONNODE hNode, const RenameMap *map) {
    ULONG type = 0;
    ULONG count = 0;
    ULONG i;

    if (hNode == NULLHANDLE) return;
    if (JsonNodeGetType(hNode, &type) != NO_ERROR) return;

    if (type == (ULONG)JSON_STRING) {
        char key[64];
        if (JsonNodeGetKey(hNode, key, sizeof(key), NULL) == NO_ERROR) {
            if (strcmp(key, "SPDXID") == 0 ||
                strcmp(key, "spdxElementId") == 0 ||
                strcmp(key, "relatedSpdxElement") == 0 ||
                strcmp(key, "spdxId") == 0 ||
                strcmp(key, "element") == 0 ||
                strcmp(key, "relatedElement") == 0) {
                char val[512];
                const char *new_id;
                if (json_read_string(hNode, val, sizeof(val)) == 0) {
                    strip_document_ref(val);
                    new_id = get_renamed(map, val);
                    if (new_id) {
                        JsonNodeSetValueString(hNode, new_id);
                    }
                }
            }
        }
        return;
    }

    if (type != (ULONG)JSON_OBJECT && type != (ULONG)JSON_ARRAY) return;

    if (JsonNodeGetCount(hNode, &count) != NO_ERROR) return;
    for (i = 0; i < count; i++) {
        HJSONNODE hChild = NULLHANDLE;
        if (type == (ULONG)JSON_OBJECT) {
            char ck[256];
            if (JsonNodeGetEntry(hNode, i, ck, sizeof(ck), NULL,
                                 &hChild) != NO_ERROR)
                continue;
        } else {
            if (JsonNodeGetElement(hNode, i, &hChild) != NO_ERROR)
                continue;
        }
        replace_ids_in_node(hChild, map);
    }
}

/* ------------------------------------------------------------------ */
/* Checksum verification                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Verify a checksum against the file content.
 *
 * @param[in] filepath       Path to the file. Not NULL.
 * @param[in] checksum_node  Object with 'algorithm' and
 *                           'checksumValue'. Not NULLHANDLE.
 */
static void verify_checksum(const char *filepath, HJSONNODE checksum_node) {
    HJSONNODE algo_node = NULLHANDLE;
    HJSONNODE value_node = NULLHANDLE;
    char algo[32];
    char expected[256];
    char *actual = NULL;

    if (JsonNodeGetChild(checksum_node, "algorithm", &algo_node) != NO_ERROR ||
        JsonNodeGetChild(checksum_node, "checksumValue", &value_node) != NO_ERROR) {
        fprintf(stderr,
                "ERROR: invalid checksum in externalDocumentRef.\n"
                "       Both 'algorithm' and 'checksumValue' fields are "
                "required.\n");
        exit(EXIT_FAILURE);
    }
    if (json_read_string(algo_node, algo, sizeof(algo)) != 0 ||
        json_read_string(value_node, expected, sizeof(expected)) != 0) {
        fprintf(stderr,
                "ERROR: invalid checksum values in externalDocumentRef.\n"
                "       'algorithm' and 'checksumValue' must be strings.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(algo, "SHA1") == 0) {
        char hex[41];
        if (Sha1File(filepath, hex, sizeof(hex), NULL) == NO_ERROR) {
            actual = strdup(hex);
        }
    }
    else if (strcmp(algo, "SHA256") == 0) {
        char hex[65];
        if (Sha256File(filepath, hex, sizeof(hex), NULL) == NO_ERROR) {
            actual = strdup(hex);
        }
    }
    else {
        fprintf(stderr,
                "ERROR: unsupported checksum algorithm: %s\n"
                "       Supported: SHA1, SHA256.\n", algo);
        exit(EXIT_FAILURE);
    }
    if (!actual) {
        fprintf(stderr,
                "ERROR: cannot compute %s for %s\n"
                "       Check that the file exists and is readable.\n",
                algo, filepath);
        exit(EXIT_FAILURE);
    }
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr,
                "ERROR: checksum mismatch for %s\n"
                "       Expected: %s\n"
                "       Actual:   %s\n"
                "       The external document has changed since the "
                "reference was recorded.\n",
                filepath, expected, actual);
        free(actual);
        exit(EXIT_FAILURE);
    }
    free(actual);
}

/* ------------------------------------------------------------------ */
/* Document validation                                                 */
/* ------------------------------------------------------------------ */

/**
 * @brief Collect all local SPDXIDs from packages, files and snippets.
 */
static void collect_local_spdxids(HJSONNODE hRoot, HSTRSET hKnown) {
    static const char *sections[] = { "packages", "files", "snippets", NULL };
    int k;

    StrSetAdd(hKnown, "SPDXRef-DOCUMENT");

    for (k = 0; sections[k]; k++) {
        HJSONNODE hArr = NULLHANDLE;
        ULONG count = 0, i;
        if (JsonNodeGetChild(hRoot, sections[k], &hArr) != NO_ERROR) continue;
        if (JsonNodeGetCount(hArr, &count) != NO_ERROR) continue;
        for (i = 0; i < count; i++) {
            HJSONNODE hItem = NULLHANDLE;
            HJSONNODE hId = NULLHANDLE;
            char id[512];
            if (JsonNodeGetElement(hArr, i, &hItem) != NO_ERROR) continue;
            if (JsonNodeGetChild(hItem, "SPDXID", &hId) != NO_ERROR) continue;
            if (json_read_string(hId, id, sizeof(id)) == 0)
                StrSetAdd(hKnown, id);
        }
    }
}

/**
 * @brief Collect all externalDocumentIds from a document.
 */
static void collect_external_ids(HJSONNODE hRoot, HSTRSET hExternalIds) {
    HJSONNODE hArr = NULLHANDLE;
    ULONG count = 0, i;

    if (JsonNodeGetChild(hRoot, "externalDocumentRefs", &hArr) != NO_ERROR)
        return;
    if (JsonNodeGetCount(hArr, &count) != NO_ERROR) return;
    for (i = 0; i < count; i++) {
        HJSONNODE hRef = NULLHANDLE;
        HJSONNODE hId = NULLHANDLE;
        char id[512];
        if (JsonNodeGetElement(hArr, i, &hRef) != NO_ERROR) continue;
        if (JsonNodeGetChild(hRef, "externalDocumentId", &hId) != NO_ERROR)
            continue;
        if (json_read_string(hId, id, sizeof(id)) == 0)
            StrSetAdd(hExternalIds, id);
    }
}

/**
 * @brief Check whether a string is present in a set.
 */
static int set_has(HSTRSET hSet, const char *str) {
    BOOL found = FALSE_;
    if (hSet == NULLHANDLE) return 0;
    if (StrSetContains(hSet, str, &found) != NO_ERROR) return 0;
    return found ? 1 : 0;
}

/**
 * @brief Validate relationships in a document.
 */
static void validate_relationships(HJSONNODE hRoot, const char *filepath,
                                   HSTRSET hKnown, HSTRSET hExternalIds) {
    HJSONNODE hArr = NULLHANDLE;
    ULONG count = 0, i;

    if (JsonNodeGetChild(hRoot, "relationships", &hArr) != NO_ERROR) return;
    if (JsonNodeGetCount(hArr, &count) != NO_ERROR) return;

    for (i = 0; i < count; i++) {
        HJSONNODE hRel = NULLHANDLE;
        HJSONNODE hA = NULLHANDLE;
        HJSONNODE hB = NULLHANDLE;
        char sa[512], sb[512];

        if (JsonNodeGetElement(hArr, i, &hRel) != NO_ERROR) continue;

        if (JsonNodeGetChild(hRel, "spdxElementId", &hA) != NO_ERROR ||
            JsonNodeGetChild(hRel, "relatedSpdxElement", &hB) != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: %s: relationship #%d missing "
                    "'spdxElementId' or 'relatedSpdxElement'.\n",
                    filepath, (int)i);
            exit(EXIT_FAILURE);
        }
        if (json_read_string(hA, sa, sizeof(sa)) != 0 ||
            json_read_string(hB, sb, sizeof(sb)) != 0) {
            fprintf(stderr,
                    "ERROR: %s: relationship #%d has non-string IDs.\n",
                    filepath, (int)i);
            exit(EXIT_FAILURE);
        }

        if (strncmp(sa, "DocumentRef-", 12) == 0) {
            char docref[256];
            const char *colon = strchr(sa, ':');
            if (!colon) {
                fprintf(stderr,
                        "ERROR: %s: malformed DocumentRef in "
                        "spdxElementId: %s\n", filepath, sa);
                exit(EXIT_FAILURE);
            }
            {
                size_t n = (size_t)(colon - sa);
                if (n >= sizeof(docref)) n = sizeof(docref) - 1;
                memcpy(docref, sa, n);
                docref[n] = '\0';
            }
            if (!set_has(hExternalIds, docref)) {
                fprintf(stderr,
                        "ERROR: %s: relationship references unresolved "
                        "externalDocumentRef '%s'.\n",
                        filepath, docref);
                exit(EXIT_FAILURE);
            }
        } else if (!set_has(hKnown, sa)) {
            fprintf(stderr,
                    "ERROR: %s: relationship references unknown "
                    "SPDXID '%s'.\n", filepath, sa);
            exit(EXIT_FAILURE);
        }

        if (strncmp(sb, "DocumentRef-", 12) == 0) {
            char docref[256];
            const char *colon = strchr(sb, ':');
            if (!colon) {
                fprintf(stderr,
                        "ERROR: %s: malformed DocumentRef in "
                        "relatedSpdxElement: %s\n", filepath, sb);
                exit(EXIT_FAILURE);
            }
            {
                size_t n = (size_t)(colon - sb);
                if (n >= sizeof(docref)) n = sizeof(docref) - 1;
                memcpy(docref, sb, n);
                docref[n] = '\0';
            }
            if (!set_has(hExternalIds, docref)) {
                fprintf(stderr,
                        "ERROR: %s: relationship references unresolved "
                        "externalDocumentRef '%s'.\n",
                        filepath, docref);
                exit(EXIT_FAILURE);
            }
        } else if (!set_has(hKnown, sb)) {
            fprintf(stderr,
                    "ERROR: %s: relationship references unknown "
                    "SPDXID '%s'.\n", filepath, sb);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Validate one SPDX license expression in a document field.
 */
static void validate_license_field(const char *filepath, const char *field,
                                   const char *value) {
    const char *bad = NULL;
    int rc;

    if (!value) return;
    rc = spdx_expression_validate(value, &bad);
    if (rc == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "ERROR: %s: invalid SPDX expression in %s: '%s'\n"
                "       See https://spdx.github.io/spdx-spec/v2.3/"
                "SPDX-license-expressions/ for the grammar.\n",
                filepath, field, value);
        exit(EXIT_FAILURE);
    }
    if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
        const char *p = bad;
        while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
        fprintf(stderr,
                "ERROR: %s: unknown SPDX identifier in %s: '",
                filepath, field);
        fwrite(bad, 1, (size_t)(p - bad), stderr);
        fprintf(stderr,
                "'\n"
                "       See https://spdx.org/licenses/ for the full list.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Validate one whole document.
 */
static void validate_document(HJSONNODE hRoot, const char *filepath) {
    HSTRSET hKnown = NULLHANDLE;
    HSTRSET hExternalIds = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    ULONG count = 0, i;

    if (StrSetCreate(&hKnown) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }
    if (StrSetCreate(&hExternalIds) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        StrSetDestroy(hKnown);
        exit(EXIT_FAILURE);
    }

    collect_local_spdxids(hRoot, hKnown);
    collect_external_ids(hRoot, hExternalIds);

    if (JsonNodeGetChild(hRoot, "packages", &hArr) == NO_ERROR &&
        JsonNodeGetCount(hArr, &count) == NO_ERROR) {
        for (i = 0; i < count; i++) {
            HJSONNODE hItem = NULLHANDLE;
            HJSONNODE hField = NULLHANDLE;
            char val[512];
            if (JsonNodeGetElement(hArr, i, &hItem) != NO_ERROR) continue;
            if (JsonNodeGetChild(hItem, "licenseConcluded", &hField) == NO_ERROR &&
                json_read_string(hField, val, sizeof(val)) == 0)
                validate_license_field(filepath, "package.licenseConcluded", val);
            if (JsonNodeGetChild(hItem, "licenseDeclared", &hField) == NO_ERROR &&
                json_read_string(hField, val, sizeof(val)) == 0)
                validate_license_field(filepath, "package.licenseDeclared", val);
        }
    }

    if (JsonNodeGetChild(hRoot, "files", &hArr) == NO_ERROR &&
        JsonNodeGetCount(hArr, &count) == NO_ERROR) {
        for (i = 0; i < count; i++) {
            HJSONNODE hItem = NULLHANDLE;
            HJSONNODE hField = NULLHANDLE;
            char val[512];
            if (JsonNodeGetElement(hArr, i, &hItem) != NO_ERROR) continue;
            if (JsonNodeGetChild(hItem, "licenseConcluded", &hField) == NO_ERROR &&
                json_read_string(hField, val, sizeof(val)) == 0)
                validate_license_field(filepath, "file.licenseConcluded", val);
            if (JsonNodeGetChild(hItem, "licenseInfoInFiles", &hField) == NO_ERROR) {
                ULONG lic_count = 0, j;
                if (JsonNodeGetCount(hField, &lic_count) == NO_ERROR) {
                    for (j = 0; j < lic_count; j++) {
                        HJSONNODE hElem = NULLHANDLE;
                        if (JsonNodeGetElement(hField, j, &hElem) != NO_ERROR)
                            continue;
                        if (json_read_string(hElem, val, sizeof(val)) == 0)
                            validate_license_field(filepath,
                                                   "file.licenseInfoInFiles",
                                                   val);
                    }
                }
            }
        }
    }

    validate_relationships(hRoot, filepath, hKnown, hExternalIds);

    StrSetDestroy(hKnown);
    StrSetDestroy(hExternalIds);
}

/* ------------------------------------------------------------------ */
/* Merge                                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Copy a top-level array from source into merged_root, if not
 *        already present.
 *
 * @param[in]  hDocMerged    Merged document. Not NULLHANDLE.
 * @param[in]  hMergedRoot   Merged root object. Not NULLHANDLE.
 * @param[in]  pszSection    Section name. Not NULL.
 * @param[out] phMergedArr   Receiver for the merged array. Not NULL.
 *
 * @return APIRET
 */
static APIRET ensure_merged_array(HJSONDOC hDocMerged, HJSONNODE hMergedRoot,
                                  PCSZ pszSection, HJSONNODE *phMergedArr) {
    APIRET rc;
    if (JsonNodeGetChild(hMergedRoot, pszSection, phMergedArr) == NO_ERROR)
        return NO_ERROR;
    rc = JsonNewArray(hDocMerged, phMergedArr);
    if (rc != NO_ERROR) return rc;
    return JsonObjectSet(hMergedRoot, pszSection, *phMergedArr);
}

/**
 * @brief Merge one source subtree array into the destination array.
 *
 * Each source element is checked for SPDXID collision against the
 * already-merged array. On collision, the element is cloned with a
 * new unique SPDXID and the rename is recorded.
 *
 * @param[in]  hDocMerged    Merged document. Not NULLHANDLE.
 * @param[in]  hSrcArr       Source array. Not NULLHANDLE.
 * @param[in]  hMergedArr    Destination array. Not NULLHANDLE.
 * @param[in]  rename_map    Rename map. Not NULL.
 * @param[in]  filepath      Source file path, for messages. Not NULL.
 *
 * @return APIRET
 */
static APIRET merge_section(HJSONDOC hDocMerged, HJSONNODE hSrcArr,
                            HJSONNODE hMergedArr, RenameMap *rename_map,
                            const char *filepath) {
    ULONG count = 0, i;
    if (JsonNodeGetCount(hSrcArr, &count) != NO_ERROR) return NO_ERROR;

    for (i = 0; i < count; i++) {
        HJSONNODE hItem = NULLHANDLE;
        HJSONNODE hId = NULLHANDLE;
        char old_id[512];
        const char *new_id_existing;

        if (JsonNodeGetElement(hSrcArr, i, &hItem) != NO_ERROR) continue;
        if (JsonNodeGetChild(hItem, "SPDXID", &hId) != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: %s: element without SPDXID.\n", filepath);
            return ERROR_INVALID_DATA;
        }
        if (json_read_string(hId, old_id, sizeof(old_id)) != 0) {
            fprintf(stderr,
                    "ERROR: %s: invalid SPDXID.\n", filepath);
            return ERROR_INVALID_DATA;
        }

        new_id_existing = get_renamed(rename_map, old_id);
        if (new_id_existing == NULL) {
            int exists = 0;
            ULONG j, mcount = 0;
            if (JsonNodeGetCount(hMergedArr, &mcount) == NO_ERROR) {
                for (j = 0; j < mcount; j++) {
                    HJSONNODE hEx = NULLHANDLE;
                    HJSONNODE hExId = NULLHANDLE;
                    char exs[512];
                    if (JsonNodeGetElement(hMergedArr, j, &hEx) != NO_ERROR) continue;
                    if (JsonNodeGetChild(hEx, "SPDXID", &hExId) != NO_ERROR) continue;
                    if (json_read_string(hExId, exs, sizeof(exs)) != 0) continue;
                    if (strcmp(exs, old_id) == 0) { exists = 1; break; }
                }
            }
            if (exists) {
                char *new_id = make_unique_id(old_id, (int)i);
                HJSONNODE hClone = NULLHANDLE;
                APIRET rc;
                add_rename(rename_map, old_id, new_id);
                rc = JsonCloneNode(hDocMerged, hItem, &hClone);
                if (rc != NO_ERROR) { free(new_id); return rc; }
                rc = JsonNodeSetString(hDocMerged, hClone, "SPDXID", new_id);
                if (rc != NO_ERROR) { free(new_id); return rc; }
                rc = JsonArrayAppend(hMergedArr, hClone);
                free(new_id);
                if (rc != NO_ERROR) return rc;
            } else {
                HJSONNODE hClone = NULLHANDLE;
                APIRET rc = JsonCloneNode(hDocMerged, hItem, &hClone);
                if (rc != NO_ERROR) return rc;
                rc = JsonArrayAppend(hMergedArr, hClone);
                if (rc != NO_ERROR) return rc;
            }
        } else {
            HJSONNODE hClone = NULLHANDLE;
            APIRET rc = JsonCloneNode(hDocMerged, hItem, &hClone);
            if (rc != NO_ERROR) return rc;
            rc = JsonNodeSetString(hDocMerged, hClone, "SPDXID", new_id_existing);
            if (rc != NO_ERROR) return rc;
            rc = JsonArrayAppend(hMergedArr, hClone);
            if (rc != NO_ERROR) return rc;
        }
    }
    return NO_ERROR;
}

/**
 * @brief Merge relationships from a source document.
 *
 * Relationships are cloned, their IDs are rewritten through the
 * rename map, and appended to the merged relationships array.
 */
static APIRET merge_relationships(HJSONDOC hDocMerged, HJSONNODE hSrcRoot,
                                  HJSONNODE hMergedArr,
                                  RenameMap *rename_map) {
    HJSONNODE hArr = NULLHANDLE;
    ULONG count = 0, i;

    if (JsonNodeGetChild(hSrcRoot, "relationships", &hArr) != NO_ERROR)
        return NO_ERROR;
    if (JsonNodeGetCount(hArr, &count) != NO_ERROR) return NO_ERROR;

    for (i = 0; i < count; i++) {
        HJSONNODE hRel = NULLHANDLE;
        HJSONNODE hClone = NULLHANDLE;
        APIRET rc;
        if (JsonNodeGetElement(hArr, i, &hRel) != NO_ERROR) continue;
        rc = JsonCloneNode(hDocMerged, hRel, &hClone);
        if (rc != NO_ERROR) return rc;
        replace_ids_in_node(hClone, rename_map);
        rc = JsonArrayAppend(hMergedArr, hClone);
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}

/**
 * @brief Recursively process one SPDX document.
 */
static void process_document(const char *filepath, HJSONNODE hRoot,
                             HJSONDOC hDocMerged, HJSONNODE hMergedRoot,
                             ProcessedList *processed, RenameMap *rename_map) {
    HJSONNODE hExtRefs = NULLHANDLE;
    HJSONNODE hPackages = NULLHANDLE;
    HJSONNODE hFiles = NULLHANDLE;
    HJSONNODE hSnippets = NULLHANDLE;
    HJSONNODE hMergedArr = NULLHANDLE;
    char *abs_path;
    ULONG count = 0, i;
    APIRET rc;

    abs_path = normalize_path(filepath);
    if (!abs_path) {
        fprintf(stderr, "ERROR: cannot normalize path: %s\n", filepath);
        exit(EXIT_FAILURE);
    }
    if (is_processed(processed, abs_path)) { free(abs_path); return; }
    add_processed(processed, abs_path);

    validate_document(hRoot, filepath);

    /* externalDocumentRefs */
    if (JsonNodeGetChild(hRoot, "externalDocumentRefs", &hExtRefs) == NO_ERROR &&
        JsonNodeGetCount(hExtRefs, &count) == NO_ERROR) {
        for (i = 0; i < count; i++) {
            HJSONNODE hRef = NULLHANDLE;
            HJSONNODE hIdNode = NULLHANDLE;
            HJSONNODE hDocNode = NULLHANDLE;
            HJSONNODE hChkNode = NULLHANDLE;
            char doc_uri[2048];
            char *full_doc_path;
            char *doc_text = NULL;
            HJSONDOC hExtDoc = NULLHANDLE;
            HJSONNODE hExtRoot = NULLHANDLE;

            if (JsonNodeGetElement(hExtRefs, i, &hRef) != NO_ERROR) continue;
            if (JsonNodeGetChild(hRef, "externalDocumentId", &hIdNode) != NO_ERROR ||
                JsonNodeGetChild(hRef, "spdxDocument", &hDocNode) != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: invalid externalDocumentRef in %s\n"
                        "       Both 'externalDocumentId' and "
                        "'spdxDocument' are required.\n", filepath);
                exit(EXIT_FAILURE);
            }
            if (json_read_string(hDocNode, doc_uri, sizeof(doc_uri)) != 0) {
                fprintf(stderr,
                        "ERROR: %s: 'spdxDocument' must be a string.\n",
                        filepath);
                exit(EXIT_FAILURE);
            }
            if (strncmp(doc_uri, "http://", 7) == 0 ||
                strncmp(doc_uri, "https://", 8) == 0) {
                fprintf(stderr,
                        "ERROR: %s: remote URIs are not supported: %s\n"
                        "       Use a local file path instead.\n",
                        filepath, doc_uri);
                exit(EXIT_FAILURE);
            }

            if (doc_uri[0] == '/' || doc_uri[0] == '\\' ||
                (isalpha((unsigned char)doc_uri[0]) && doc_uri[1] == ':')) {
                full_doc_path = strdup(doc_uri);
            } else {
                char *dir = get_dirname(filepath);
                full_doc_path = join_path(dir, doc_uri);
                free(dir);
            }
            if (!full_doc_path) {
                fprintf(stderr, "ERROR: %s: cannot resolve path: %s\n",
                        filepath, doc_uri);
                exit(EXIT_FAILURE);
            }

            if (JsonNodeGetChild(hRef, "checksum", &hChkNode) == NO_ERROR)
                verify_checksum(full_doc_path, hChkNode);
            else {
                fprintf(stderr,
                        "ERROR: %s: missing checksum for external "
                        "document: %s\n"
                        "       Each externalDocumentRef must include a "
                        "checksum.\n", filepath, doc_uri);
                exit(EXIT_FAILURE);
            }

            if (SpdxReadFileAll(full_doc_path, &doc_text, NULL) != NO_ERROR ||
                !doc_text) {
                fprintf(stderr,
                        "ERROR: cannot read external document: %s\n"
                        "       Check that the file exists and is "
                        "readable.\n", full_doc_path);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }
            if (JsonParse(doc_text, &hExtDoc) != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: invalid JSON in external document: %s\n",
                        full_doc_path);
                free(doc_text);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }
            free(doc_text);

            if (JsonRoot(hExtDoc, &hExtRoot) != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: cannot obtain root of: %s\n", full_doc_path);
                JsonClose(hExtDoc);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }

            process_document(full_doc_path, hExtRoot, hDocMerged,
                             hMergedRoot, processed, rename_map);
            JsonClose(hExtDoc);
            free(full_doc_path);
        }
    }

    /* packages */
    if (JsonNodeGetChild(hRoot, "packages", &hPackages) == NO_ERROR) {
        rc = ensure_merged_array(hDocMerged, hMergedRoot, "packages", &hMergedArr);
        if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
        rc = merge_section(hDocMerged, hPackages, hMergedArr, rename_map, filepath);
        if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
    }

    /* files */
    if (JsonNodeGetChild(hRoot, "files", &hFiles) == NO_ERROR) {
        rc = ensure_merged_array(hDocMerged, hMergedRoot, "files", &hMergedArr);
        if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
        rc = merge_section(hDocMerged, hFiles, hMergedArr, rename_map, filepath);
        if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
    }

    /* snippets */
    if (JsonNodeGetChild(hRoot, "snippets", &hSnippets) == NO_ERROR) {
        ULONG snip_count = 0;
        if (JsonNodeGetCount(hSnippets, &snip_count) == NO_ERROR &&
            snip_count > 0) {
            rc = ensure_merged_array(hDocMerged, hMergedRoot, "snippets",
                                     &hMergedArr);
            if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
            rc = merge_section(hDocMerged, hSnippets, hMergedArr,
                               rename_map, filepath);
            if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
        }
    }

    /* relationships */
    {
        rc = ensure_merged_array(hDocMerged, hMergedRoot, "relationships",
                                 &hMergedArr);
        if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
        rc = merge_relationships(hDocMerged, hRoot, hMergedArr, rename_map);
        if (rc != NO_ERROR) { free(abs_path); exit(EXIT_FAILURE); }
    }

    free(abs_path);
}

/* ------------------------------------------------------------------ */
/* CLI                                                                 */
/* ------------------------------------------------------------------ */

static void print_help(void) {
    printf("Usage: spdx-merge --input=<file> [--output=<file>] [options]\n"
           "\n"
           "Required:\n"
           "  --input=<file>             Root SPDX JSON document to merge\n"
           "  --spdx-db=<path>           SPDX database root "
           "(licenses.json,\n"
           "                             exceptions.json, details/, "
           "exceptions/)\n"
           "\n"
           "Optional:\n"
           "  --output=<file>            Write merged document to file "
           "(default: stdout)\n"
           "  --cache=<path>             SPDX database cache file\n"
           "  --help, -h                 Show this help\n"
           "\n"
           "The merge walks externalDocumentRefs recursively, verifies "
           "each\n"
           "checksum, and resolves SPDXID collisions by renaming.\n");
}

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    const char *spdx_db_root = NULL;
    const char *cache_file = NULL;
    int i;
    char *root_text = NULL;
    HJSONDOC hSrcDoc = NULLHANDLE;
    HJSONNODE hSrcRoot = NULLHANDLE;
    HJSONDOC hDocMerged = NULLHANDLE;
    HJSONNODE hMergedRoot = NULLHANDLE;
    HJSONNODE hCreation = NULLHANDLE;
    ProcessedList processed;
    RenameMap rename_map;
    time_t now;
    struct tm *tm;
    char date[32];
    int db_errs;
    APIRET rc;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        }
        else if (strncmp(argv[i], "--input=", 8) == 0)
            input_file = argv[i] + 8;
        else if (strncmp(argv[i], "--output=", 9) == 0)
            output_file = argv[i] + 9;
        else if (strncmp(argv[i], "--spdx-db=", 10) == 0)
            spdx_db_root = argv[i] + 10;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            cache_file = argv[i] + 8;
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s\n"
                    "       Run 'spdx-merge --help' for usage.\n",
                    argv[i]);
            return 1;
        }
    }

    if (!input_file) {
        fprintf(stderr,
                "ERROR: --input=<file> is required.\n"
                "       Run 'spdx-merge --help' for usage.\n");
        return 1;
    }
    if (!spdx_db_root) {
        fprintf(stderr,
                "ERROR: --spdx-db=<path> is required.\n"
                "       Run 'spdx-merge --help' for usage.\n");
        return 1;
    }

    db_errs = spdx_db_init(spdx_db_root, cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        fprintf(stderr,
                "ERROR: SPDX license database is unavailable "
                "(licenses.json not loaded).\n"
                "       Expected at <spdx-db>/licenses.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: SPDX exceptions database is unavailable "
                "(exceptions.json not loaded).\n"
                "       Expected at <spdx-db>/exceptions.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written.\n"
                "         Next run will re-parse JSON indexes.\n");

    if (SpdxReadFileAll(input_file, &root_text, NULL) != NO_ERROR ||
        !root_text) {
        fprintf(stderr,
                "ERROR: cannot read input file: %s\n"
                "       Check that the file exists and is readable.\n",
                input_file);
        spdx_db_free();
        return 1;
    }
    if (JsonParse(root_text, &hSrcDoc) != NO_ERROR) {
        fprintf(stderr,
                "ERROR: invalid JSON in input file: %s\n", input_file);
        free(root_text);
        spdx_db_free();
        return 1;
    }
    free(root_text);

    if (JsonRoot(hSrcDoc, &hSrcRoot) != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot obtain root of input file.\n");
        JsonClose(hSrcDoc);
        spdx_db_free();
        return 1;
    }

    processed.paths = (char**)malloc(MAX_DOCS * sizeof(char*));
    processed.count = 0;
    rename_map.old_ids = NULL;
    rename_map.new_ids = NULL;
    rename_map.count = 0;

    rc = JsonNewDoc(&hDocMerged);
    if (rc != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hSrcDoc);
        spdx_db_free();
        return 1;
    }

    rc = JsonNewObject(hDocMerged, &hMergedRoot);
    if (rc != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        spdx_db_free();
        return 1;
    }

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    if (JsonNodeSetString(hDocMerged, hMergedRoot, "spdxVersion", "SPDX-2.3") != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hMergedRoot, "SPDXID", "SPDXRef-DOCUMENT") != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hMergedRoot, "name", "Merged SPDX Document") != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hMergedRoot, "dataLicense", "CC0-1.0") != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        spdx_db_free();
        return 1;
    }

    {
        char ns[256];
        sprintf(ns, "https://osfree.org/spdxdocs/merged-%ld", (long)now);
        if (JsonNodeSetString(hDocMerged, hMergedRoot,
                              "documentNamespace", ns) != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            JsonClose(hDocMerged);
            JsonClose(hSrcDoc);
            spdx_db_free();
            return 1;
        }
    }

    if (JsonNewObject(hDocMerged, &hCreation) != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hCreation, "created", date) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        spdx_db_free();
        return 1;
    }
    {
        HJSONNODE hCreators = NULLHANDLE;
        HJSONNODE hToolName = NULLHANDLE;
        if (JsonNewArray(hDocMerged, &hCreators) != NO_ERROR ||
            JsonNewString(hDocMerged, "Tool: osFree SPDX Merge Tool",
                          &hToolName) != NO_ERROR ||
            JsonArrayAppend(hCreators, hToolName) != NO_ERROR ||
            JsonObjectSet(hCreation, "creators", hCreators) != NO_ERROR ||
            JsonObjectSet(hMergedRoot, "creationInfo", hCreation) != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            JsonClose(hDocMerged);
            JsonClose(hSrcDoc);
            spdx_db_free();
            return 1;
        }
    }

    process_document(input_file, hSrcRoot, hDocMerged, hMergedRoot,
                     &processed, &rename_map);

    if (output_file) {
        if (!freopen(output_file, "w", stdout)) {
            fprintf(stderr,
                    "ERROR: cannot open output file: %s\n"
                    "       Check directory permissions.\n",
                    output_file);
            JsonClose(hDocMerged);
            JsonClose(hSrcDoc);
            spdx_db_free();
            return 1;
        }
    }

    rc = JsonWriteFile(hMergedRoot, TRUE_, NULL);
    if (rc != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot serialize merged document.\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        spdx_db_free();
        return 1;
    }

    JsonClose(hDocMerged);
    JsonClose(hSrcDoc);
    free_rename_map(&rename_map);
    for (i = 0; i < processed.count; i++) free(processed.paths[i]);
    free(processed.paths);
    spdx_db_free();
    return 0;
}
