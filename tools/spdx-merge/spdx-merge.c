/* spdx-merge.c - объединение SPDX JSON файлов (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "json_parser.h"

/* Чтение файла в строку */
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

/* Клонирование узла JSON */
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

/* Создание узла-строки с ключом */
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

/* Создание пустого объекта с ключом */
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

/* Создание пустого массива с ключом */
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

/* Добавление дочернего элемента в объект/массив */
static void add_child(JsonNode *parent, JsonNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (JsonNode **)realloc(parent->children,
                                                parent->child_capacity * sizeof(JsonNode *));
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

/* Поиск и замена строкового значения у дочернего элемента с указанным ключом */
static void replace_string_value(JsonNode *object, const char *key, const char *new_value) {
    JsonNode *child = json_find_child(object, key);
    if (child && child->type == JSON_STRING) {
        free(child->string_value);
        child->string_value = strdup(new_value);
    }
}

/* Вывод JSON с отступами */
static void print_json_indent(JsonNode *node, int indent) {
    int i, j;

    if (!node) return;

    for (j = 0; j < indent; j++) printf("  ");

    if (node->key) {
        printf("\"%s\": ", node->key);
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
            printf("\"%s\"", node->string_value ? node->string_value : "");
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

    src_files = json_find_child(src_root, "files");
    bin_files = json_find_child(bin_root, "files");

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    /* Создаём корневой объект объединённого документа */
    merged = create_object_node(NULL);
    if (!merged) goto cleanup;

    /* spdxVersion */
    node = create_string_node("spdxVersion", "SPDX-2.3");
    add_child(merged, node);
    /* SPDXID */
    node = create_string_node("SPDXID", "SPDXRef-DOCUMENT");
    add_child(merged, node);
    /* name */
    node = create_string_node("name", "Merged SBOM");
    add_child(merged, node);
    /* creationInfo */
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
    /* dataLicense */
    node = create_string_node("dataLicense", "CC0-1.0");
    add_child(merged, node);
    /* documentNamespace */
    {
        char ns[256];
        sprintf(ns, "https://osfree.org/spdxdocs/merged-%ld", (long)now);
        node = create_string_node("documentNamespace", ns);
        add_child(merged, node);
    }

    /* packages */
    merged_packages = create_array_node("packages");
    /* Клонируем исходные пакеты с префиксом SrcPkg */
    for (i = 0; i < src_packages->child_count; i++) {
        JsonNode *clone = clone_node(src_packages->children[i]);
        if (clone) {
            replace_string_value(clone, "SPDXID", "SPDXRef-SrcPkg-Package");
            add_child(merged_packages, clone);
        }
    }
    /* Клонируем бинарные пакеты с префиксом BinPkg */
    for (i = 0; i < bin_packages->child_count; i++) {
        JsonNode *clone = clone_node(bin_packages->children[i]);
        if (clone) {
            replace_string_value(clone, "SPDXID", "SPDXRef-BinPkg-Package");
            add_child(merged_packages, clone);
        }
    }
    add_child(merged, merged_packages);

    /* files */
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

    /* relationships */
    merged_relationships = create_array_node("relationships");
    /* DESCRIBES для исходного пакета */
    {
        JsonNode *rel = create_object_node(NULL);
        add_child(rel, create_string_node("spdxElementId", "SPDXRef-DOCUMENT"));
        add_child(rel, create_string_node("relatedSpdxElement", "SPDXRef-SrcPkg-Package"));
        add_child(rel, create_string_node("relationshipType", "DESCRIBES"));
        add_child(merged_relationships, rel);
    }
    /* DESCRIBES для бинарного пакета */
    {
        JsonNode *rel = create_object_node(NULL);
        add_child(rel, create_string_node("spdxElementId", "SPDXRef-DOCUMENT"));
        add_child(rel, create_string_node("relatedSpdxElement", "SPDXRef-BinPkg-Package"));
        add_child(rel, create_string_node("relationshipType", "DESCRIBES"));
        add_child(merged_relationships, rel);
    }
    /* GENERATED_FROM: бинарный пакет сгенерирован из исходного */
    {
        JsonNode *rel = create_object_node(NULL);
        add_child(rel, create_string_node("spdxElementId", "SPDXRef-BinPkg-Package"));
        add_child(rel, create_string_node("relatedSpdxElement", "SPDXRef-SrcPkg-Package"));
        add_child(rel, create_string_node("relationshipType", "GENERATED_FROM"));
        add_child(merged_relationships, rel);
    }
    add_child(merged, merged_relationships);

    /* Вывод результата */
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
