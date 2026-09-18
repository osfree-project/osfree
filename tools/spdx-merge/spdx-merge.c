/* spdx-merge.c - объединение SPDX JSON документов (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "json_parser.h"
#include "sha1.h"
#include "sha256_utils.h"
#include "spdx_db.h"
#include "spdx.h"
#include "ccl.h"

#define MAX_DOCS 100


static void print_json_string(const char *s) {
    char *esc = json_escape_string(s ? s : "");
    printf("\"%s\"", esc ? esc : "");
    free(esc);
}

/* ---------- Пути ---------- */

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

/* ---------- Processed / Rename ---------- */

typedef struct {
    char **paths;
    int count;
} ProcessedList;

typedef struct {
    char **old_ids;
    char **new_ids;
    int count;
} RenameMap;

static int is_processed(ProcessedList *l, const char *path) {
    int i;
    for (i = 0; i < l->count; i++)
        if (strcmp(l->paths[i], path) == 0) return 1;
    return 0;
}

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

static char *make_unique_id(const char *base, int counter) {
    char *r = (char*)malloc(strlen(base) + 24);
    sprintf(r, "%s-Duplicate%d", base, counter);
    return r;
}

static void add_rename(RenameMap *m, const char *old_id, const char *new_id) {
    m->old_ids = (char**)realloc(m->old_ids, (m->count + 1) * sizeof(char*));
    m->new_ids = (char**)realloc(m->new_ids, (m->count + 1) * sizeof(char*));
    m->old_ids[m->count] = strdup(old_id);
    m->new_ids[m->count] = strdup(new_id);
    m->count++;
}

static const char *get_renamed(const RenameMap *m, const char *old_id) {
    int i;
    for (i = 0; i < m->count; i++)
        if (strcmp(m->old_ids[i], old_id) == 0) return m->new_ids[i];
    return NULL;
}

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

/* ---------- Создание JSON-узлов ---------- */

static JsonNode *create_string_node(const char *key, const char *value) {
    JsonNode *n = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JSON_STRING;
    n->key = key ? strdup(key) : NULL;
    n->string_value = strdup(value ? value : "");
    return n;
}

static JsonNode *create_object_node(const char *key) {
    JsonNode *n = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JSON_OBJECT;
    n->key = key ? strdup(key) : NULL;
    n->child_capacity = 4;
    n->children = (JsonNode**)malloc(n->child_capacity * sizeof(JsonNode*));
    if (!n->children) { free(n->key); free(n); return NULL; }
    return n;
}

static JsonNode *create_array_node(const char *key) {
    JsonNode *n = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JSON_ARRAY;
    n->key = key ? strdup(key) : NULL;
    n->child_capacity = 4;
    n->children = (JsonNode**)malloc(n->child_capacity * sizeof(JsonNode*));
    if (!n->children) { free(n->key); free(n); return NULL; }
    return n;
}

static void add_child(JsonNode *parent, JsonNode *child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity ? parent->child_capacity * 2 : 4;
        JsonNode **new_children = (JsonNode**)realloc(parent->children,
            (size_t)new_cap * sizeof(JsonNode*));
        if (!new_children) {
            fprintf(stderr, "ERROR: out of memory\n");
            exit(EXIT_FAILURE);
        }
        parent->children = new_children;
        parent->child_capacity = new_cap;
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

static JsonNode *clone_node(JsonNode *node) {
    JsonNode *copy;
    int i;

    if (!node) return NULL;
    copy = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!copy) return NULL;
    copy->type = node->type;
    if (node->key) copy->key = strdup(node->key);
    if (node->string_value) copy->string_value = strdup(node->string_value);
    copy->number_value = node->number_value;
    copy->bool_value = node->bool_value;
    copy->child_count = node->child_count;
    copy->child_capacity = (node->child_count > 0) ? node->child_count : 1;
    copy->children = (JsonNode**)malloc(copy->child_capacity * sizeof(JsonNode*));
    if (!copy->children) {
        free(copy->key);
        free(copy->string_value);
        free(copy);
        return NULL;
    }
    for (i = 0; i < node->child_count; i++) {
        copy->children[i] = clone_node(node->children[i]);
        if (copy->children[i]) copy->children[i]->parent = copy;
    }
    return copy;
}

static void replace_string_value(JsonNode *object, const char *key,
                                 const char *new_value) {
    JsonNode *c = json_find_child(object, key);
    if (c && c->type == JSON_STRING) {
        free(c->string_value);
        c->string_value = strdup(new_value);
    }
}

/* ---------- Работа с DocumentRef- ---------- */

static void strip_document_ref(char *str) {
    char *colon;
    if (strncmp(str, "DocumentRef-", 12) == 0) {
        colon = strchr(str, ':');
        if (colon) memmove(str, colon + 1, strlen(colon + 1) + 1);
    }
}

static void replace_ids_in_node(JsonNode *node, const RenameMap *map) {
    int i;
    const char *new_id;
    if (!node) return;
    if (node->type == JSON_STRING && node->string_value) {
        if (node->key && (
            strcmp(node->key, "SPDXID") == 0 ||
            strcmp(node->key, "spdxElementId") == 0 ||
            strcmp(node->key, "relatedSpdxElement") == 0 ||
            strcmp(node->key, "spdxId") == 0 ||
            strcmp(node->key, "element") == 0 ||
            strcmp(node->key, "relatedElement") == 0)) {
            strip_document_ref(node->string_value);
            new_id = get_renamed(map, node->string_value);
            if (new_id) {
                free(node->string_value);
                node->string_value = strdup(new_id);
            }
        }
    }
    for (i = 0; i < node->child_count; i++)
        replace_ids_in_node(node->children[i], map);
}

/* ---------- Проверка контрольной суммы ---------- */

static void verify_checksum(const char *filepath, JsonNode *checksum_node) {
    JsonNode *algo_node = json_find_child(checksum_node, "algorithm");
    JsonNode *value_node = json_find_child(checksum_node, "checksumValue");
    const char *algo, *expected;
    char *actual = NULL;

    if (!algo_node || !value_node) {
        fprintf(stderr,
                "ERROR: invalid checksum in externalDocumentRef.\n"
                "       Both 'algorithm' and 'checksumValue' fields are "
                "required.\n");
        exit(EXIT_FAILURE);
    }
    algo = json_get_string(algo_node);
    expected = json_get_string(value_node);
    if (!algo || !expected) {
        fprintf(stderr,
                "ERROR: invalid checksum values in externalDocumentRef.\n"
                "       'algorithm' and 'checksumValue' must be strings.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(algo, "SHA1") == 0) {
        if (Sha1File(filepath, &actual) != SHA1_NO_ERROR)
            actual = NULL;
    }
    else if (strcmp(algo, "SHA256") == 0)
        actual = sha256_file(filepath);
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

/* ---------- Валидация SPDX документа ---------- */

static void collect_local_spdxids(JsonNode *root, HSTRSET hKnown) {
    JsonNode *arr;
    int i;

    StrSetAdd(hKnown, "SPDXRef-DOCUMENT");

    arr = json_find_child(root, "packages");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *id = json_find_child(arr->children[i], "SPDXID");
            const char *s = id ? json_get_string(id) : NULL;
            if (s) StrSetAdd(hKnown, s);
        }
    }
    arr = json_find_child(root, "files");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *id = json_find_child(arr->children[i], "SPDXID");
            const char *s = id ? json_get_string(id) : NULL;
            if (s) StrSetAdd(hKnown, s);
        }
    }
    arr = json_find_child(root, "snippets");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *id = json_find_child(arr->children[i], "SPDXID");
            const char *s = id ? json_get_string(id) : NULL;
            if (s) StrSetAdd(hKnown, s);
        }
    }
}

static void collect_external_ids(JsonNode *root, HSTRSET hExternalIds) {
    JsonNode *arr = json_find_child(root, "externalDocumentRefs");
    int i;
    if (!arr || arr->type != JSON_ARRAY) return;
    for (i = 0; i < arr->child_count; i++) {
        JsonNode *id = json_find_child(arr->children[i], "externalDocumentId");
        const char *s = id ? json_get_string(id) : NULL;
        if (s) StrSetAdd(hExternalIds, s);
    }
}

/* Check whether a string is present in a set. */
static int set_has(HSTRSET hSet, const char *str) {
    BOOL found = FALSE_;
    if (hSet == NULLHANDLE) return 0;
    if (StrSetContains(hSet, str, &found) != NO_ERROR) return 0;
    return found ? 1 : 0;
}

static void validate_relationships(JsonNode *root, const char *filepath,
                                   HSTRSET hKnown, HSTRSET hExternalIds) {
    JsonNode *arr = json_find_child(root, "relationships");
    int i;

    if (!arr || arr->type != JSON_ARRAY) return;

    for (i = 0; i < arr->child_count; i++) {
        JsonNode *rel = arr->children[i];
        JsonNode *a = json_find_child(rel, "spdxElementId");
        JsonNode *b = json_find_child(rel, "relatedSpdxElement");
        const char *sa = a ? json_get_string(a) : NULL;
        const char *sb = b ? json_get_string(b) : NULL;

        if (!sa || !sb) {
            fprintf(stderr,
                    "ERROR: %s: relationship #%d missing "
                    "'spdxElementId' or 'relatedSpdxElement'.\n",
                    filepath, i);
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

static void validate_document(JsonNode *root, const char *filepath) {
    HSTRSET hKnown = NULLHANDLE;
    HSTRSET hExternalIds = NULLHANDLE;
    JsonNode *arr;
    int i;

    if (StrSetCreate(&hKnown) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }
    if (StrSetCreate(&hExternalIds) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        StrSetDestroy(hKnown);
        exit(EXIT_FAILURE);
    }

    collect_local_spdxids(root, hKnown);
    collect_external_ids(root, hExternalIds);

    arr = json_find_child(root, "packages");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *n;
            n = json_find_child(arr->children[i], "licenseConcluded");
            if (n) validate_license_field(filepath, "package.licenseConcluded",
                                          json_get_string(n));
            n = json_find_child(arr->children[i], "licenseDeclared");
            if (n) validate_license_field(filepath, "package.licenseDeclared",
                                          json_get_string(n));
        }
    }
    arr = json_find_child(root, "files");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *n;
            n = json_find_child(arr->children[i], "licenseConcluded");
            if (n) validate_license_field(filepath, "file.licenseConcluded",
                                          json_get_string(n));
            n = json_find_child(arr->children[i], "licenseInfoInFiles");
            if (n && n->type == JSON_ARRAY) {
                int j;
                for (j = 0; j < n->child_count; j++)
                    validate_license_field(filepath, "file.licenseInfoInFiles",
                                           json_get_string(n->children[j]));
            }
        }
    }

    validate_relationships(root, filepath, hKnown, hExternalIds);

    StrSetDestroy(hKnown);
    StrSetDestroy(hExternalIds);
}

/* ---------- Основная рекурсивная обработка ---------- */

static void process_document(const char *filepath, JsonNode *root,
                             JsonNode *merged_root, ProcessedList *processed,
                             RenameMap *rename_map) {
    JsonNode *ext_refs, *packages, *files, *snippets, *relationships;
    JsonNode *merged_packages, *merged_files, *merged_snippets;
    JsonNode *merged_relationships;
    char *abs_path;
    int i, j;

    abs_path = normalize_path(filepath);
    if (!abs_path) {
        fprintf(stderr,
                "ERROR: cannot normalize path: %s\n", filepath);
        exit(EXIT_FAILURE);
    }
    if (is_processed(processed, abs_path)) { free(abs_path); return; }
    add_processed(processed, abs_path);

    validate_document(root, filepath);

    ext_refs = json_find_child(root, "externalDocumentRefs");
    if (ext_refs && ext_refs->type == JSON_ARRAY) {
        for (i = 0; i < ext_refs->child_count; i++) {
            JsonNode *ref = ext_refs->children[i];
            JsonNode *id_node = json_find_child(ref, "externalDocumentId");
            JsonNode *doc_node = json_find_child(ref, "spdxDocument");
            JsonNode *checksum_node = json_find_child(ref, "checksum");
            const char *doc_uri;
            char *full_doc_path;
            char *doc_text = NULL;
            JsonNode *ext_root;

            if (!id_node || !doc_node) {
                fprintf(stderr,
                        "ERROR: invalid externalDocumentRef in %s\n"
                        "       Both 'externalDocumentId' and "
                        "'spdxDocument' are required.\n", filepath);
                exit(EXIT_FAILURE);
            }
            doc_uri = json_get_string(doc_node);
            if (!doc_uri) {
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
                fprintf(stderr,
                        "ERROR: %s: cannot resolve path: %s\n",
                        filepath, doc_uri);
                exit(EXIT_FAILURE);
            }

            if (checksum_node && checksum_node->type == JSON_OBJECT)
                verify_checksum(full_doc_path, checksum_node);
            else {
                fprintf(stderr,
                        "ERROR: %s: missing checksum for external "
                        "document: %s\n"
                        "       Each externalDocumentRef must include a "
                        "checksum.\n", filepath, doc_uri);
                exit(EXIT_FAILURE);
            }

            if (SpdxReadFileAll(full_doc_path, &doc_text, NULL) != NO_ERROR
                || !doc_text) {
                fprintf(stderr,
                        "ERROR: cannot read external document: %s\n"
                        "       Check that the file exists and is "
                        "readable.\n", full_doc_path);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }
            ext_root = json_parse(doc_text);
            free(doc_text);
            if (!ext_root) {
                fprintf(stderr,
                        "ERROR: invalid JSON in external document: %s\n",
                        full_doc_path);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }

            process_document(full_doc_path, ext_root, merged_root,
                             processed, rename_map);
            json_free(ext_root);
            free(full_doc_path);
        }
    }

    packages = json_find_child(root, "packages");
    merged_packages = json_find_child(merged_root, "packages");
    if (!merged_packages) {
        merged_packages = create_array_node("packages");
        add_child(merged_root, merged_packages);
    }
    if (packages && packages->type == JSON_ARRAY) {
        for (i = 0; i < packages->child_count; i++) {
            JsonNode *pkg = packages->children[i];
            JsonNode *id_node = json_find_child(pkg, "SPDXID");
            const char *old_id;
            if (!id_node) {
                fprintf(stderr,
                        "ERROR: %s: package without SPDXID.\n", filepath);
                exit(EXIT_FAILURE);
            }
            old_id = json_get_string(id_node);
            if (!old_id) {
                fprintf(stderr,
                        "ERROR: %s: invalid package SPDXID.\n", filepath);
                exit(EXIT_FAILURE);
            }
            if (get_renamed(rename_map, old_id) == NULL) {
                int exists = 0;
                for (j = 0; j < merged_packages->child_count; j++) {
                    JsonNode *ex = json_find_child(merged_packages->children[j],
                                                   "SPDXID");
                    const char *exs = ex ? json_get_string(ex) : NULL;
                    if (exs && strcmp(exs, old_id) == 0) {
                        exists = 1; break;
                    }
                }
                if (exists) {
                    char *new_id = make_unique_id(old_id, i);
                    JsonNode *clone = clone_node(pkg);
                    add_rename(rename_map, old_id, new_id);
                    replace_string_value(clone, "SPDXID", new_id);
                    add_child(merged_packages, clone);
                    free(new_id);
                } else {
                    add_child(merged_packages, clone_node(pkg));
                }
            } else {
                JsonNode *clone = clone_node(pkg);
                const char *new_id = get_renamed(rename_map, old_id);
                replace_string_value(clone, "SPDXID", new_id);
                add_child(merged_packages, clone);
            }
        }
    }

    files = json_find_child(root, "files");
    merged_files = json_find_child(merged_root, "files");
    if (!merged_files) {
        merged_files = create_array_node("files");
        add_child(merged_root, merged_files);
    }
    if (files && files->type == JSON_ARRAY) {
        for (i = 0; i < files->child_count; i++) {
            JsonNode *file = files->children[i];
            JsonNode *id_node = json_find_child(file, "SPDXID");
            const char *old_id;
            if (!id_node) {
                fprintf(stderr,
                        "ERROR: %s: file without SPDXID.\n", filepath);
                exit(EXIT_FAILURE);
            }
            old_id = json_get_string(id_node);
            if (!old_id) {
                fprintf(stderr,
                        "ERROR: %s: invalid file SPDXID.\n", filepath);
                exit(EXIT_FAILURE);
            }
            if (get_renamed(rename_map, old_id) == NULL) {
                int exists = 0;
                for (j = 0; j < merged_files->child_count; j++) {
                    JsonNode *ex = json_find_child(merged_files->children[j],
                                                   "SPDXID");
                    const char *exs = ex ? json_get_string(ex) : NULL;
                    if (exs && strcmp(exs, old_id) == 0) {
                        exists = 1; break;
                    }
                }
                if (exists) {
                    char *new_id = make_unique_id(old_id, i);
                    JsonNode *clone = clone_node(file);
                    add_rename(rename_map, old_id, new_id);
                    replace_string_value(clone, "SPDXID", new_id);
                    add_child(merged_files, clone);
                    free(new_id);
                } else {
                    add_child(merged_files, clone_node(file));
                }
            } else {
                JsonNode *clone = clone_node(file);
                const char *new_id = get_renamed(rename_map, old_id);
                replace_string_value(clone, "SPDXID", new_id);
                add_child(merged_files, clone);
            }
        }
    }

    snippets = json_find_child(root, "snippets");
    merged_snippets = json_find_child(merged_root, "snippets");
    if (snippets && snippets->type == JSON_ARRAY && snippets->child_count > 0) {
        if (!merged_snippets) {
            merged_snippets = create_array_node("snippets");
            add_child(merged_root, merged_snippets);
        }
        for (i = 0; i < snippets->child_count; i++) {
            JsonNode *sn = snippets->children[i];
            JsonNode *id_node = json_find_child(sn, "SPDXID");
            const char *old_id;
            if (!id_node) {
                fprintf(stderr,
                        "ERROR: %s: snippet without SPDXID.\n", filepath);
                exit(EXIT_FAILURE);
            }
            old_id = json_get_string(id_node);
            if (!old_id) {
                fprintf(stderr,
                        "ERROR: %s: invalid snippet SPDXID.\n", filepath);
                exit(EXIT_FAILURE);
            }
            if (get_renamed(rename_map, old_id) == NULL) {
                int exists = 0;
                for (j = 0; j < merged_snippets->child_count; j++) {
                    JsonNode *ex = json_find_child(merged_snippets->children[j],
                                                   "SPDXID");
                    const char *exs = ex ? json_get_string(ex) : NULL;
                    if (exs && strcmp(exs, old_id) == 0) {
                        exists = 1; break;
                    }
                }
                if (exists) {
                    char *new_id = make_unique_id(old_id, i);
                    JsonNode *clone = clone_node(sn);
                    add_rename(rename_map, old_id, new_id);
                    replace_string_value(clone, "SPDXID", new_id);
                    add_child(merged_snippets, clone);
                    free(new_id);
                } else {
                    add_child(merged_snippets, clone_node(sn));
                }
            } else {
                JsonNode *clone = clone_node(sn);
                const char *new_id = get_renamed(rename_map, old_id);
                replace_string_value(clone, "SPDXID", new_id);
                add_child(merged_snippets, clone);
            }
        }
    }

    relationships = json_find_child(root, "relationships");
    merged_relationships = json_find_child(merged_root, "relationships");
    if (!merged_relationships) {
        merged_relationships = create_array_node("relationships");
        add_child(merged_root, merged_relationships);
    }
    if (relationships && relationships->type == JSON_ARRAY) {
        for (i = 0; i < relationships->child_count; i++) {
            JsonNode *rel = relationships->children[i];
            JsonNode *clone = clone_node(rel);
            replace_ids_in_node(clone, rename_map);
            add_child(merged_relationships, clone);
        }
    }

    free(abs_path);
}

/* ---------- Вывод JSON ---------- */

static void print_json_indent(JsonNode *node, int indent) {
    int i, j;
    if (!node) return;

    for (j = 0; j < indent; j++) printf("  ");

    if (node->key) {
        print_json_string(node->key);
        printf(": ");
    }

    switch (node->type) {
        case JSON_NULL:
            printf("null");
            break;
        case JSON_BOOLEAN:
            printf(node->bool_value ? "true" : "false");
            break;
        case JSON_NUMBER:
            printf("%g", node->number_value);
            break;
        case JSON_STRING:
            print_json_string(node->string_value ? node->string_value : "");
            break;
        case JSON_ARRAY:
            printf("[\n");
            for (i = 0; i < node->child_count; i++) {
                if (i > 0) printf(",\n");
                print_json_indent(node->children[i], indent + 1);
            }
            printf("\n");
            for (j = 0; j < indent; j++) printf("  ");
            printf("]");
            break;
        case JSON_OBJECT:
            printf("{\n");
            for (i = 0; i < node->child_count; i++) {
                if (i > 0) printf(",\n");
                print_json_indent(node->children[i], indent + 1);
            }
            printf("\n");
            for (j = 0; j < indent; j++) printf("  ");
            printf("}");
            break;
    }
}

/* ---------- CLI ---------- */

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
           "  --details-dir=<path>       SPDX details directory "
           "(if not under spdx-db)\n"
           "  --exceptions-dir=<path>    SPDX exceptions directory "
           "(if not under spdx-db)\n"
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
    JsonNode *root, *merged_root;
    ProcessedList processed;
    RenameMap rename_map;
    time_t now;
    struct tm *tm;
    char date[32];
    int db_errs;

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
    root = json_parse(root_text);
    free(root_text);
    if (!root) {
        fprintf(stderr,
                "ERROR: invalid JSON in input file: %s\n",
                input_file);
        spdx_db_free();
        return 1;
    }

    processed.paths = (char**)malloc(MAX_DOCS * sizeof(char*));
    processed.count = 0;
    rename_map.old_ids = NULL;
    rename_map.new_ids = NULL;
    rename_map.count = 0;

    merged_root = create_object_node(NULL);
    if (!merged_root) {
        fprintf(stderr, "ERROR: out of memory\n");
        json_free(root);
        spdx_db_free();
        return 1;
    }

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    add_child(merged_root, create_string_node("spdxVersion", "SPDX-2.3"));
    add_child(merged_root, create_string_node("SPDXID", "SPDXRef-DOCUMENT"));
    add_child(merged_root, create_string_node("name", "Merged SPDX Document"));
    {
        JsonNode *creation = create_object_node("creationInfo");
        JsonNode *creators = create_array_node("creators");
        char ns[256];
        add_child(creators, create_string_node(NULL,
            "Tool: osFree SPDX Merge Tool"));
        add_child(creation, create_string_node("created", date));
        add_child(creation, creators);
        add_child(merged_root, creation);
        sprintf(ns, "https://osfree.org/spdxdocs/merged-%ld", (long)now);
        add_child(merged_root, create_string_node("documentNamespace", ns));
    }
    add_child(merged_root, create_string_node("dataLicense", "CC0-1.0"));

    process_document(input_file, root, merged_root, &processed, &rename_map);

    if (output_file) {
        if (!freopen(output_file, "w", stdout)) {
            fprintf(stderr,
                    "ERROR: cannot open output file: %s\n"
                    "       Check directory permissions.\n",
                    output_file);
            json_free(root);
            json_free(merged_root);
            spdx_db_free();
            return 1;
        }
    }
    print_json_indent(merged_root, 0);
    printf("\n");

    json_free(root);
    json_free(merged_root);
    free_rename_map(&rename_map);
    for (i = 0; i < processed.count; i++) free(processed.paths[i]);
    free(processed.paths);
    spdx_db_free();
    return 0;
}
