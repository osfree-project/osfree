/* spdx-merge.c - объединение SPDX JSON файлов (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "json_parser.h"

static char *read_file(const char *filename) {
    FILE *f;
    long size;
    char *buf;

    f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
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

static void replace_string_value(JsonNode *object, const char *key, const char *new_value) {
    JsonNode *child = json_find_child(object, key);
    if (child && child->type == JSON_STRING) {
        free(child->string_value);
        child->string_value = strdup(new_value);
    }
}

/* Выводит строку с экранированием кавычек, backslash и управляющих символов */
static void print_escaped_string(const char *s) {
    putchar('"');
    while (*s) {
        switch (*s) {
            case '"':  printf("\\\""); break;
            case '\\': printf("\\\\"); break;
            case '\n': printf("\\n"); break;
            case '\r': printf("\\r"); break;
            case '\t': printf("\\t"); break;
            default:   putchar(*s);
        }
        s++;
    }
    putchar('"');
}

static void print_json_indent(JsonNode *node, int indent) {
    int i, j;
    if (!node) return;

    for (j = 0; j < indent; j++) printf("  ");

    if (node->key) {
        print_escaped_string(node->key);
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
            print_escaped_string(node->string_value ? node->string_value : "");
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
    const char *source_file = NULL;
    const char *binary_file = NULL;
    const char *output_file = NULL;
    int i;
    char *src_text = NULL;
    char *bin_text = NULL;
    JsonNode *src_root = NULL;
    JsonNode *bin_root = NULL;
    JsonNode *src_packages = NULL;
    JsonNode *bin_packages = NULL;
    JsonNode *src_files = NULL;
    JsonNode *bin_files = NULL;
    JsonNode *merged = NULL;
    JsonNode *merged_packages = NULL;
    JsonNode *merged_files = NULL;
    JsonNode *merged_relationships = NULL;
    JsonNode *node = NULL;
    JsonNode *src_pkg_clone = NULL;
    JsonNode *bin_pkg_clone = NULL;
    const char *src_spdx_id = NULL;
    const char *bin_spdx_id = NULL;
    time_t now;
    struct tm *tm;
    char date[32];

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--source=", 9) == 0)
            source_file = argv[i] + 9;
        else if (strncmp(argv[i], "--binary=", 9) == 0)
            binary_file = argv[i] + 9;
        else if (strncmp(argv[i], "--output=", 9) == 0)
            output_file = argv[i] + 9;
    }

    if (!source_file || !binary_file) {
        fprintf(stderr, "Usage: spdx-merge --source=<src.json> --binary=<bin.json> [--output=merged.json]\n");
        return 1;
    }

    src_text = read_file(source_file);
    bin_text = read_file(binary_file);
    if (!src_text || !bin_text) {
        fprintf(stderr, "Error reading input files\n");
        goto cleanup;
    }

    src_root = json_parse(src_text);
    bin_root = json_parse(bin_text);
    if (!src_root || !bin_root) {
        fprintf(stderr, "Invalid JSON input\n");
        goto cleanup;
    }

    src_packages = json_find_child(src_root, "packages");
    bin_packages = json_find_child(bin_root, "packages");
    if (!src_packages || !bin_packages ||
        src_packages->type != JSON_ARRAY || bin_packages->type != JSON_ARRAY) {
        fprintf(stderr, "Missing or invalid packages arrays\n");
        goto cleanup;
    }

    if (src_packages->child_count < 1 || bin_packages->child_count < 1) {
        fprintf(stderr, "Empty packages array in input files\n");
        goto cleanup;
    }

    src_files = json_find_child(src_root, "files");
    bin_files = json_find_child(bin_root, "files");

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    merged = create_object_node(NULL);
    if (!merged) goto cleanup;

    node = create_string_node("spdxVersion", "SPDX-2.3");
    add_child(merged, node);
    node = create_string_node("SPDXID", "SPDXRef-DOCUMENT");
    add_child(merged, node);
    node = create_string_node("name", "Merged SBOM");
    add_child(merged, node);

    {
        JsonNode *creation = create_object_node("creationInfo");
        JsonNode *created = create_string_node("created", date);
        JsonNode *creators = create_array_node("creators");
        JsonNode *tool = create_string_node(NULL, "Tool: osFree SPDX Merge Tool");
        add_child(creators, tool);
        add_child(creation, created);
        add_child(creation, creators);
        add_child(merged, creation);
    }

    node = create_string_node("dataLicense", "CC0-1.0");
    add_child(merged, node);
    {
        char ns[256];
        sprintf(ns, "https://osfree.org/spdxdocs/merged-%ld", (long)now);
        node = create_string_node("documentNamespace", ns);
        add_child(merged, node);
    }

    merged_packages = create_array_node("packages");
    src_pkg_clone = clone_node(src_packages->children[0]);
    if (!src_pkg_clone) goto cleanup;
    src_spdx_id = json_get_string(json_find_child(src_pkg_clone, "SPDXID"));
    add_child(merged_packages, src_pkg_clone);

    bin_pkg_clone = clone_node(bin_packages->children[0]);
    if (!bin_pkg_clone) goto cleanup;
    bin_spdx_id = json_get_string(json_find_child(bin_pkg_clone, "SPDXID"));
    add_child(merged_packages, bin_pkg_clone);

    add_child(merged, merged_packages);

    merged_files = create_array_node("files");
    if (src_files && src_files->type == JSON_ARRAY) {
        for (i = 0; i < src_files->child_count; i++) {
            JsonNode *clone = clone_node(src_files->children[i]);
            if (clone) {
                char new_id[256];
                const char *old_id = json_get_string(json_find_child(clone, "SPDXID"));
                if (old_id && strncmp(old_id, "SPDXRef-", 8) == 0) {
                    sprintf(new_id, "SPDXRef-SrcFile-%s", old_id + 8);
                    replace_string_value(clone, "SPDXID", new_id);
                }
                add_child(merged_files, clone);
            }
        }
    }
    if (bin_files && bin_files->type == JSON_ARRAY) {
        for (i = 0; i < bin_files->child_count; i++) {
            JsonNode *clone = clone_node(bin_files->children[i]);
            if (clone) {
                char new_id[256];
                const char *old_id = json_get_string(json_find_child(clone, "SPDXID"));
                if (old_id && strncmp(old_id, "SPDXRef-", 8) == 0) {
                    sprintf(new_id, "SPDXRef-BinFile-%s", old_id + 8);
                    replace_string_value(clone, "SPDXID", new_id);
                }
                add_child(merged_files, clone);
            }
        }
    }
    add_child(merged, merged_files);

    merged_relationships = create_array_node("relationships");
    if (src_spdx_id) {
        JsonNode *rel = create_object_node(NULL);
        add_child(rel, create_string_node("spdxElementId", "SPDXRef-DOCUMENT"));
        add_child(rel, create_string_node("relatedSpdxElement", src_spdx_id));
        add_child(rel, create_string_node("relationshipType", "DESCRIBES"));
        add_child(merged_relationships, rel);
    }
    if (bin_spdx_id) {
        JsonNode *rel = create_object_node(NULL);
        add_child(rel, create_string_node("spdxElementId", "SPDXRef-DOCUMENT"));
        add_child(rel, create_string_node("relatedSpdxElement", bin_spdx_id));
        add_child(rel, create_string_node("relationshipType", "DESCRIBES"));
        add_child(merged_relationships, rel);
    }
    if (src_spdx_id && bin_spdx_id) {
        JsonNode *rel = create_object_node(NULL);
        add_child(rel, create_string_node("spdxElementId", bin_spdx_id));
        add_child(rel, create_string_node("relatedSpdxElement", src_spdx_id));
        add_child(rel, create_string_node("relationshipType", "GENERATED_FROM"));
        add_child(merged_relationships, rel);
    }
    add_child(merged, merged_relationships);

    if (output_file) {
        if (!freopen(output_file, "w", stdout)) {
            fprintf(stderr, "Cannot open output file: %s\n", output_file);
            goto cleanup;
        }
    }
    print_json_indent(merged, 0);
    printf("\n");

cleanup:
    free(src_text);
    free(bin_text);
    json_free(src_root);
    json_free(bin_root);
    json_free(merged);
    return 0;
}
