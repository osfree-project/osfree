/* spdx-merge.c - объединение SPDX JSON документов (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "json_parser.h"
#include "sha1_utils.h"
#include "sha256_utils.h"
#include "spdx_db.h"
#include "spdx_utils.h"

#define MAX_DOCS 100

typedef struct {
    char **paths;
    int count;
} ProcessedList;

typedef struct {
    char **old_ids;
    char **new_ids;
    int count;
} RenameMap;

/* ---------- Экранирование строк для JSON ---------- */

static char *json_escape_string(const char *src) {
    size_t len, extra, i, j;
    char *dst;

    if (!src) src = "";
    len = strlen(src);
    extra = 0;
    for (i = 0; i < len; i++) {
        if (src[i] == '"' || src[i] == '\\' || src[i] == '\n' ||
            src[i] == '\r' || src[i] == '\t')
            extra++;
    }
    dst = (char*)malloc(len + extra + 1);
    if (!dst) return NULL;
    j = 0;
    for (i = 0; i < len; i++) {
        switch (src[i]) {
            case '"':  dst[j++] = '\\'; dst[j++] = '"'; break;
            case '\\': dst[j++] = '\\'; dst[j++] = '\\'; break;
            case '\n': dst[j++] = '\\'; dst[j++] = 'n'; break;
            case '\r': dst[j++] = '\\'; dst[j++] = 'r'; break;
            case '\t': dst[j++] = '\\'; dst[j++] = 't'; break;
            default:   dst[j++] = src[i]; break;
        }
    }
    dst[j] = '\0';
    return dst;
}

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

static int is_processed(ProcessedList *l, const char *path) {
    int i;
    for (i = 0; i < l->count; i++)
        if (strcmp(l->paths[i], path) == 0) return 1;
    return 0;
}

static void add_processed(ProcessedList *l, const char *path) {
    if (l->count >= MAX_DOCS) {
        fprintf(stderr, "Too many documents\n"); exit(EXIT_FAILURE);
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
    return n;
}

static JsonNode *create_array_node(const char *key) {
    JsonNode *n = (JsonNode*)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JSON_ARRAY;
    n->key = key ? strdup(key) : NULL;
    n->child_capacity = 4;
    n->children = (JsonNode**)malloc(n->child_capacity * sizeof(JsonNode*));
    return n;
}

static void add_child(JsonNode *parent, JsonNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (JsonNode**)realloc(parent->children,
            parent->child_capacity * sizeof(JsonNode*));
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

static JsonNode *clone_node(JsonNode *node) {
    JsonNode *copy;
    int i;

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
        fprintf(stderr, "Invalid checksum in externalDocumentRef\n");
        exit(EXIT_FAILURE);
    }
    algo = json_get_string(algo_node);
    expected = json_get_string(value_node);
    if (!algo || !expected) {
        fprintf(stderr, "Invalid checksum values\n"); exit(EXIT_FAILURE);
    }
    if (strcmp(algo, "SHA1") == 0)
        actual = sha1_file(filepath);
    else if (strcmp(algo, "SHA256") == 0)
        actual = sha256_file(filepath);
    else {
        fprintf(stderr, "Unsupported checksum algorithm: %s\n", algo);
        exit(EXIT_FAILURE);
    }
    if (!actual) {
        fprintf(stderr, "Cannot compute %s for %s\n", algo, filepath);
        exit(EXIT_FAILURE);
    }
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr,
                "Checksum mismatch for %s: expected %s, got %s\n",
                filepath, expected, actual);
        free(actual);
        exit(EXIT_FAILURE);
    }
    free(actual);
}

/* ---------- Валидация SPDX документа ---------- */

static void collect_local_spdxids(JsonNode *root, SpdxStrList *known) {
    JsonNode *arr;
    int i;

    if (!spdx_strlist_contains(known, "SPDXRef-DOCUMENT"))
        spdx_strlist_add(known, "SPDXRef-DOCUMENT");

    arr = json_find_child(root, "packages");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *id = json_find_child(arr->children[i], "SPDXID");
            const char *s = id ? json_get_string(id) : NULL;
            if (s) spdx_strlist_add_unique(known, s);
        }
    }
    arr = json_find_child(root, "files");
    if (arr && arr->type == JSON_ARRAY) {
        for (i = 0; i < arr->child_count; i++) {
            JsonNode *id = json_find_child(arr->children[i], "SPDXID");
            const char *s = id ? json_get_string(id) : NULL;
            if (s) spdx_strlist_add_unique(known, s);
        }
    }
}

static void collect_external_ids(JsonNode *root, SpdxStrList *external_ids) {
    JsonNode *arr = json_find_child(root, "externalDocumentRefs");
    int i;
    if (!arr || arr->type != JSON_ARRAY) return;
    for (i = 0; i < arr->child_count; i++) {
        JsonNode *id = json_find_child(arr->children[i], "externalDocumentId");
        const char *s = id ? json_get_string(id) : NULL;
        if (s) spdx_strlist_add(external_ids, s);
    }
}

static void validate_relationships(JsonNode *root, const char *filepath,
                                   SpdxStrList *known, SpdxStrList *external_ids) {
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
                    "Error: %s: relationship #%d missing spdxElementId/"
                    "relatedSpdxElement\n", filepath, i);
            exit(EXIT_FAILURE);
        }
        if (strncmp(sa, "DocumentRef-", 12) == 0) {
            char docref[256];
            const char *colon = strchr(sa, ':');
            if (!colon) {
                fprintf(stderr,
                        "Error: %s: malformed DocumentRef in spdxElementId: %s\n",
                        filepath, sa);
                exit(EXIT_FAILURE);
            }
            {
                size_t n = (size_t)(colon - sa);
                if (n >= sizeof(docref)) n = sizeof(docref) - 1;
                memcpy(docref, sa, n);
                docref[n] = '\0';
            }
            if (!spdx_strlist_contains(external_ids, docref)) {
                fprintf(stderr,
                        "Error: %s: unresolved externalDocumentRef '%s'\n",
                        filepath, docref);
                exit(EXIT_FAILURE);
            }
        } else if (!spdx_strlist_contains(known, sa)) {
            fprintf(stderr,
                    "Error: %s: relationship references unknown SPDXID '%s'\n",
                    filepath, sa);
            exit(EXIT_FAILURE);
        }

        if (strncmp(sb, "DocumentRef-", 12) == 0) {
            char docref[256];
            const char *colon = strchr(sb, ':');
            if (!colon) {
                fprintf(stderr,
                        "Error: %s: malformed DocumentRef in "
                        "relatedSpdxElement: %s\n", filepath, sb);
                exit(EXIT_FAILURE);
            }
            {
                size_t n = (size_t)(colon - sb);
                if (n >= sizeof(docref)) n = sizeof(docref) - 1;
                memcpy(docref, sb, n);
                docref[n] = '\0';
            }
            if (!spdx_strlist_contains(external_ids, docref)) {
                fprintf(stderr,
                        "Error: %s: unresolved externalDocumentRef '%s'\n",
                        filepath, docref);
                exit(EXIT_FAILURE);
            }
        } else if (!spdx_strlist_contains(known, sb)) {
            fprintf(stderr,
                    "Error: %s: relationship references unknown SPDXID '%s'\n",
                    filepath, sb);
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
                "Error: %s: invalid SPDX expression in %s: '%s'\n",
                filepath, field, value);
        exit(EXIT_FAILURE);
    }
    if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
        const char *p = bad;
        while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
        fprintf(stderr, "Error: %s: unknown SPDX identifier in %s: '",
                filepath, field);
        fwrite(bad, 1, (size_t)(p - bad), stderr);
        fprintf(stderr, "'\n");
        exit(EXIT_FAILURE);
    }
}

static void validate_document(JsonNode *root, const char *filepath) {
    SpdxStrList known, external_ids;
    JsonNode *arr;
    int i;

    spdx_strlist_init(&known);
    spdx_strlist_init(&external_ids);

    collect_local_spdxids(root, &known);
    collect_external_ids(root, &external_ids);

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

    validate_relationships(root, filepath, &known, &external_ids);

    spdx_strlist_free(&known);
    spdx_strlist_free(&external_ids);
}

/* ---------- Основная рекурсивная обработка ---------- */

static void process_document(const char *filepath, JsonNode *root,
                             JsonNode *merged_root, ProcessedList *processed,
                             RenameMap *rename_map) {
    JsonNode *ext_refs, *packages, *files, *relationships;
    JsonNode *merged_packages, *merged_files, *merged_relationships;
    char *abs_path;
    int i, j;

    abs_path = normalize_path(filepath);
    if (!abs_path) {
        fprintf(stderr, "Cannot normalize path: %s\n", filepath);
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
            char *doc_text;
            JsonNode *ext_root;

            if (!id_node || !doc_node) {
                fprintf(stderr, "Invalid externalDocumentRef in %s\n", filepath);
                exit(EXIT_FAILURE);
            }
            doc_uri = json_get_string(doc_node);
            if (!doc_uri) {
                fprintf(stderr, "spdxDocument must be a string\n");
                exit(EXIT_FAILURE);
            }
            if (strncmp(doc_uri, "http://", 7) == 0 ||
                strncmp(doc_uri, "https://", 8) == 0) {
                fprintf(stderr, "Remote URIs unsupported: %s\n", doc_uri);
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
                fprintf(stderr, "Cannot resolve: %s\n", doc_uri);
                exit(EXIT_FAILURE);
            }

            if (checksum_node && checksum_node->type == JSON_OBJECT)
                verify_checksum(full_doc_path, checksum_node);
            else {
                fprintf(stderr, "Missing checksum for %s\n", doc_uri);
                exit(EXIT_FAILURE);
            }

            doc_text = spdx_read_file_all(full_doc_path, NULL);
            if (!doc_text) {
                fprintf(stderr, "Cannot read: %s\n", full_doc_path);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }
            ext_root = json_parse(doc_text);
            free(doc_text);
            if (!ext_root) {
                fprintf(stderr, "Invalid JSON: %s\n", full_doc_path);
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
                fprintf(stderr, "Package without SPDXID in %s\n", filepath);
                exit(EXIT_FAILURE);
            }
            old_id = json_get_string(id_node);
            if (!old_id) {
                fprintf(stderr, "Invalid SPDXID in package\n");
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
                fprintf(stderr, "File without SPDXID in %s\n", filepath);
                exit(EXIT_FAILURE);
            }
            old_id = json_get_string(id_node);
            if (!old_id) {
                fprintf(stderr, "Invalid SPDXID in file\n");
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

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    const char *licenses_json = NULL;
    const char *exceptions_json = NULL;
    const char *details_dir = NULL;
    const char *exceptions_dir = NULL;
    const char *cache_file = NULL;
    int i;
    char *root_text;
    JsonNode *root, *merged_root;
    ProcessedList processed;
    RenameMap rename_map;
    time_t now;
    struct tm *tm;
    char date[32];
    int db_errs;

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--input=", 8) == 0)
            input_file = argv[i] + 8;
        else if (strncmp(argv[i], "--output=", 9) == 0)
            output_file = argv[i] + 9;
        else if (strncmp(argv[i], "--licenses-json=", 16) == 0)
            licenses_json = argv[i] + 16;
        else if (strncmp(argv[i], "--exceptions-json=", 18) == 0)
            exceptions_json = argv[i] + 18;
        else if (strncmp(argv[i], "--details-dir=", 14) == 0)
            details_dir = argv[i] + 14;
        else if (strncmp(argv[i], "--exceptions-dir=", 17) == 0)
            exceptions_dir = argv[i] + 17;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            cache_file = argv[i] + 8;
    }
    if (!input_file) {
        fprintf(stderr,
                "Usage: spdx-merge --input=<root.spdx.json> "
                "[--output=merged.spdx.json]\n"
                "       --licenses-json=<path> --exceptions-json=<path> "
                "--cache=<path>\n"
                "       [--details-dir=<path>] [--exceptions-dir=<path>]\n");
        return 1;
    }
    if (!licenses_json || !exceptions_json || !cache_file) {
        fprintf(stderr,
                "Error: --licenses-json, --exceptions-json, --cache required\n");
        return 1;
    }

    db_errs = spdx_db_init(licenses_json, exceptions_json,
                           details_dir, exceptions_dir, cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        fprintf(stderr, "Error: SPDX license database unavailable\n");
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        fprintf(stderr, "Error: SPDX exceptions database unavailable\n");
        spdx_db_free();
        return 1;
    }

    root_text = spdx_read_file_all(input_file, NULL);
    if (!root_text) {
        fprintf(stderr, "Cannot read input: %s\n", input_file);
        spdx_db_free();
        return 1;
    }
    root = json_parse(root_text);
    free(root_text);
    if (!root) {
        fprintf(stderr, "Invalid JSON input\n");
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
        fprintf(stderr, "OOM\n"); spdx_db_free(); return 1;
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
            fprintf(stderr, "Cannot open output: %s\n", output_file);
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
