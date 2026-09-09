/* spdx-merge.c - объединение SPDX JSON документов (C89, OpenWatcom)
 * Автоматически разрешает externalDocumentRefs, проверяет контрольные суммы (SHA1/SHA256),
 * предотвращает циклические ссылки и коллизии SPDXID.
 * Исправлено экранирование строк при выводе JSON.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "json_parser.h"
#include "sha1_utils.h"
#include "sha256_utils.h"

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

static char *read_file(const char *filename) {
    FILE *f;
    long size;
    char *buf;
    size_t read_size;

    f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) {
        fclose(f);
        return NULL;
    }
    buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    read_size = fread(buf, 1, size, f);
    fclose(f);
    if (read_size != (size_t)size) {
        free(buf);
        return NULL;
    }
    buf[size] = '\0';
    return buf;
}

static char *normalize_path(const char *path) {
    char *copy = strdup(path);
    char *out = malloc(strlen(path) + 3);
    char *p, *q;
    if (!copy || !out) {
        free(copy);
        free(out);
        return NULL;
    }
    p = copy;
    q = out;
#ifdef _WIN32
    if (isalpha((unsigned char)p[0]) && p[1] == ':') {
        *q++ = *p++;
        *q++ = *p++;
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
                    while (q > out && *(q-1) != '/' && *(q-1) != '\\') q--;
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
    char *last_sep = NULL;
    char *dir;
    size_t len;

    if (backslash && (!slash || backslash > slash))
        last_sep = backslash;
    else if (slash)
        last_sep = slash;

    if (!last_sep) {
        return strdup(".");
    }

    len = last_sep - filepath;
    if (len == 0) {
        return strdup("/");
    }

    dir = (char *)malloc(len + 1);
    if (!dir) return NULL;
    memcpy(dir, filepath, len);
    dir[len] = '\0';
    return dir;
}

static char *join_path(const char *dir, const char *rel) {
    char *result;
    size_t len1, len2;
    int need_sep;

    if (!dir || !rel) return NULL;
    len1 = strlen(dir);
    len2 = strlen(rel);
    need_sep = (len1 > 0 && dir[len1-1] != '/' && dir[len1-1] != '\\') ? 1 : 0;

    result = (char *)malloc(len1 + need_sep + len2 + 1);
    if (!result) return NULL;
    strcpy(result, dir);
    if (need_sep) strcat(result, "/");
    strcat(result, rel);
    return result;
}

static int is_processed(ProcessedList *list, const char *path) {
    int i;
    for (i = 0; i < list->count; i++) {
        if (strcmp(list->paths[i], path) == 0) return 1;
    }
    return 0;
}

static void add_processed(ProcessedList *list, const char *path) {
    if (list->count >= MAX_DOCS) {
        fprintf(stderr, "Too many documents\n");
        exit(EXIT_FAILURE);
    }
    list->paths[list->count] = strdup(path);
    list->count++;
}

static char *make_unique_id(const char *base_id, int counter) {
    char *new_id = malloc(strlen(base_id) + 20);
    sprintf(new_id, "%s-Duplicate%d", base_id, counter);
    return new_id;
}

static void add_rename(RenameMap *map, const char *old_id, const char *new_id) {
    map->old_ids = realloc(map->old_ids, (map->count + 1) * sizeof(char*));
    map->new_ids = realloc(map->new_ids, (map->count + 1) * sizeof(char*));
    map->old_ids[map->count] = strdup(old_id);
    map->new_ids[map->count] = strdup(new_id);
    map->count++;
}

static const char *get_renamed(const RenameMap *map, const char *old_id) {
    int i;
    for (i = 0; i < map->count; i++) {
        if (strcmp(map->old_ids[i], old_id) == 0) return map->new_ids[i];
    }
    return NULL;
}

static void free_rename_map(RenameMap *map) {
    int i;
    for (i = 0; i < map->count; i++) {
        free(map->old_ids[i]);
        free(map->new_ids[i]);
    }
    free(map->old_ids);
    free(map->new_ids);
    map->count = 0;
}

static JsonNode *create_string_node(const char *key, const char *value) {
    JsonNode *node = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!node) return NULL;
    node->type = JSON_STRING;
    node->key = key ? strdup(key) : NULL;
    node->string_value = strdup(value ? value : "");
    node->child_count = 0;
    node->children = NULL;
    return node;
}

static JsonNode *create_object_node(const char *key) {
    JsonNode *node = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!node) return NULL;
    node->type = JSON_OBJECT;
    node->key = key ? strdup(key) : NULL;
    node->child_capacity = 4;
    node->children = (JsonNode **)malloc(node->child_capacity * sizeof(JsonNode *));
    node->child_count = 0;
    return node;
}

static JsonNode *create_array_node(const char *key) {
    JsonNode *node = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!node) return NULL;
    node->type = JSON_ARRAY;
    node->key = key ? strdup(key) : NULL;
    node->child_capacity = 4;
    node->children = (JsonNode **)malloc(node->child_capacity * sizeof(JsonNode *));
    node->child_count = 0;
    return node;
}

static void add_child(JsonNode *parent, JsonNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (JsonNode **)realloc(parent->children,
                                                parent->child_capacity * sizeof(JsonNode *));
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

static JsonNode *clone_node(JsonNode *node) {
    JsonNode *copy;
    int i;

    copy = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!copy) return NULL;
    copy->type = node->type;
    if (node->key) copy->key = strdup(node->key);
    if (node->string_value) copy->string_value = strdup(node->string_value);
    copy->number_value = node->number_value;
    copy->bool_value = node->bool_value;
    copy->child_count = node->child_count;
    copy->child_capacity = (node->child_count > 0) ? node->child_count : 1;
    copy->children = (JsonNode **)malloc(copy->child_capacity * sizeof(JsonNode *));
    for (i = 0; i < node->child_count; i++) {
        copy->children[i] = clone_node(node->children[i]);
        if (copy->children[i]) copy->children[i]->parent = copy;
    }
    return copy;
}

static void replace_string_value(JsonNode *object, const char *key, const char *new_value) {
    JsonNode *child = json_find_child(object, key);
    if (child && child->type == JSON_STRING) {
        free(child->string_value);
        child->string_value = strdup(new_value);
    }
}

static void strip_document_ref(char *str) {
    char *colon;
    if (strncmp(str, "DocumentRef-", 12) == 0) {
        colon = strchr(str, ':');
        if (colon) {
            memmove(str, colon + 1, strlen(colon + 1) + 1);
        }
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
            strcmp(node->key, "relatedElement") == 0
        )) {
            strip_document_ref(node->string_value);
            new_id = get_renamed(map, node->string_value);
            if (new_id) {
                free(node->string_value);
                node->string_value = strdup(new_id);
            }
        }
    }
    for (i = 0; i < node->child_count; i++) {
        replace_ids_in_node(node->children[i], map);
    }
}

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
        fprintf(stderr, "Invalid checksum values\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(algo, "SHA1") == 0) {
        actual = sha1_file(filepath);
        if (!actual) {
            fprintf(stderr, "Cannot compute SHA1 for %s\n", filepath);
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(algo, "SHA256") == 0) {
        actual = sha256_file(filepath);
        if (!actual) {
            fprintf(stderr, "Cannot compute SHA256 for %s\n", filepath);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Unsupported checksum algorithm: %s\n", algo);
        exit(EXIT_FAILURE);
    }

    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "Checksum mismatch for %s: expected %s, got %s\n",
                filepath, expected, actual);
        free(actual);
        exit(EXIT_FAILURE);
    }
    free(actual);
}

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

    if (is_processed(processed, abs_path)) {
        free(abs_path);
        return;
    }
    add_processed(processed, abs_path);

    /* Обрабатываем externalDocumentRefs */
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

            if (strncmp(doc_uri, "http://", 7) == 0 || strncmp(doc_uri, "https://", 8) == 0) {
                fprintf(stderr, "Remote document URIs are not supported yet: %s\n", doc_uri);
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
                fprintf(stderr, "Cannot resolve document path: %s\n", doc_uri);
                exit(EXIT_FAILURE);
            }

            if (checksum_node && checksum_node->type == JSON_OBJECT) {
                verify_checksum(full_doc_path, checksum_node);
            } else {
                fprintf(stderr, "Missing checksum for external document %s\n", doc_uri);
                exit(EXIT_FAILURE);
            }

            doc_text = read_file(full_doc_path);
            if (!doc_text) {
                fprintf(stderr, "Cannot read external document: %s\n", full_doc_path);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }
            ext_root = json_parse(doc_text);
            free(doc_text);
            if (!ext_root) {
                fprintf(stderr, "Invalid JSON in external document: %s\n", full_doc_path);
                free(full_doc_path);
                exit(EXIT_FAILURE);
            }

            process_document(full_doc_path, ext_root, merged_root, processed, rename_map);
            json_free(ext_root);
            free(full_doc_path);
        }
    }

    /* Копируем packages */
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
                    JsonNode *existing_id = json_find_child(merged_packages->children[j], "SPDXID");
                    const char *ex_id = existing_id ? json_get_string(existing_id) : NULL;
                    if (ex_id && strcmp(ex_id, old_id) == 0) {
                        exists = 1;
                        break;
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

    /* Копируем files */
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
                    JsonNode *existing_id = json_find_child(merged_files->children[j], "SPDXID");
                    const char *ex_id = existing_id ? json_get_string(existing_id) : NULL;
                    if (ex_id && strcmp(ex_id, old_id) == 0) {
                        exists = 1;
                        break;
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

    /* Копируем relationships */
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

/* Экранирование строки для JSON */
static char *json_escape(const char *src) {
    size_t len, extra, i, j;
    char *dst;

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

/* Вывод JSON с отступами и экранированием строк */
static void print_json_indent(JsonNode *node, int indent) {
    int i, j;
    char *escaped;

    if (!node) return;

    for (j = 0; j < indent; j++) printf("  ");

    if (node->key) {
        escaped = json_escape(node->key);
        printf("\"%s\": ", escaped ? escaped : "");
        free(escaped);
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
            escaped = json_escape(node->string_value ? node->string_value : "");
            printf("\"%s\"", escaped ? escaped : "");
            free(escaped);
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

int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    int i;
    char *root_text;
    JsonNode *root;
    JsonNode *merged_root;
    ProcessedList processed;
    RenameMap rename_map;
    time_t now;
    struct tm *tm;
    char date[32];

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--input=", 8) == 0)
            input_file = argv[i] + 8;
        else if (strncmp(argv[i], "--output=", 9) == 0)
            output_file = argv[i] + 9;
    }

    if (!input_file) {
        fprintf(stderr, "Usage: spdx-merge --input=<root.spdx.json> [--output=merged.spdx.json]\n");
        return 1;
    }

    root_text = read_file(input_file);
    if (!root_text) {
        fprintf(stderr, "Cannot read input file: %s\n", input_file);
        return 1;
    }
    root = json_parse(root_text);
    free(root_text);
    if (!root) {
        fprintf(stderr, "Invalid JSON input\n");
        return 1;
    }

    processed.paths = malloc(MAX_DOCS * sizeof(char*));
    processed.count = 0;
    rename_map.old_ids = NULL;
    rename_map.new_ids = NULL;
    rename_map.count = 0;

    merged_root = create_object_node(NULL);
    if (!merged_root) {
        fprintf(stderr, "Memory allocation failed\n");
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
        add_child(creators, create_string_node(NULL, "Tool: osFree SPDX Merge Tool"));
        add_child(creation, create_string_node("created", date));
        add_child(creation, creators);
        add_child(merged_root, creation);
    }
    add_child(merged_root, create_string_node("dataLicense", "CC0-1.0"));
    {
        char ns[256];
        sprintf(ns, "https://osfree.org/spdxdocs/merged-%ld", (long)now);
        add_child(merged_root, create_string_node("documentNamespace", ns));
    }

    process_document(input_file, root, merged_root, &processed, &rename_map);

    if (output_file) {
        if (!freopen(output_file, "w", stdout)) {
            fprintf(stderr, "Cannot open output file: %s\n", output_file);
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

    return 0;
}
