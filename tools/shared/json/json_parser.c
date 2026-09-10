/* json_parser.c - минимальный JSON-парсер для C89 (OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json_parser.h"

#define INITIAL_CHILD_CAPACITY 4
#define MAX_DEPTH 100

static void skip_whitespace(const char **p) {
    while (**p && isspace((unsigned char)**p)) (*p)++;
}

static JsonNode *parse_value(const char **p, int depth);

static JsonNode *node_new(JsonType type) {
    JsonNode *node = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (node) {
        node->type = type;
        node->child_capacity = INITIAL_CHILD_CAPACITY;
        node->children = (JsonNode **)malloc(node->child_capacity * sizeof(JsonNode *));
        if (!node->children) {
            free(node);
            return NULL;
        }
        node->child_count = 0;
    }
    return node;
}

static void node_add_child(JsonNode *parent, JsonNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (JsonNode **)realloc(parent->children,
                                                parent->child_capacity * sizeof(JsonNode *));
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

/* Разбор строки JSON */
static char *parse_string(const char **p) {
    const char *start;
    char *result;
    size_t len;
    char *q;
    const char *s;
    int i;

    if (**p != '"') return NULL;
    (*p)++;
    start = *p;
    len = 0;
    while (**p && **p != '"') {
        if (**p == '\\') {
            (*p)++;
            if (**p == '\0') return NULL;
            switch (**p) {
                case 'n': case 't': case 'r': case 'b': case 'f':
                case '"': case '\\': case '/':
                    len += 1; (*p)++;
                    break;
                case 'u':
                    /* пропускаем 4 шестнадцатеричных цифры */
                    len += 1; (*p)++;
                    for (i = 0; i < 4; i++) {
                        if (!isxdigit((unsigned char)**p)) return NULL;
                        (*p)++;
                    }
                    break;
                default:
                    return NULL;
            }
        } else {
            len++;
            (*p)++;
        }
    }
    if (**p != '"') return NULL;
    (*p)++;

    result = (char *)malloc(len + 1);
    if (!result) return NULL;
    q = result;
    s = start;
    while (s < *p - 1) {
        if (*s == '\\') {
            s++;
            switch (*s) {
                case 'n': *q++ = '\n'; s++; break;
                case 't': *q++ = '\t'; s++; break;
                case 'r': *q++ = '\r'; s++; break;
                case 'b': *q++ = '\b'; s++; break;
                case 'f': *q++ = '\f'; s++; break;
                case '"': *q++ = '"'; s++; break;
                case '\\': *q++ = '\\'; s++; break;
                case '/': *q++ = '/'; s++; break;
                case 'u':
                    /* упрощение: записываем как '?' */
                    *q++ = '?';
                    s += 5;
                    break;
                default:
                    free(result);
                    return NULL;
            }
        } else {
            *q++ = *s++;
        }
    }
    *q = '\0';
    return result;
}

/* Разбор числа */
static JsonNode *parse_number(const char **p) {
    const char *start = *p;
    char *end;
    double val;
    JsonNode *node;

    if (**p == '-') (*p)++;
    while (**p && (isdigit((unsigned char)**p) || **p == '.' || **p == 'e' || **p == 'E' ||
           **p == '+' || **p == '-')) (*p)++;
    if (*p == start) return NULL;
    val = strtod(start, &end);
    if (end != *p) return NULL;

    node = node_new(JSON_NUMBER);
    if (node) node->number_value = val;
    return node;
}

/* Разбор массива */
static JsonNode *parse_array(const char **p, int depth) {
    JsonNode *array;
    JsonNode *child;

    array = node_new(JSON_ARRAY);
    if (!array) return NULL;

    if (**p != '[') {
        json_free(array);
        return NULL;
    }
    (*p)++;
    skip_whitespace(p);
    if (**p == ']') {
        (*p)++;
        return array;
    }

    while (1) {
        skip_whitespace(p);
        child = parse_value(p, depth);
        if (!child) {
            json_free(array);
            return NULL;
        }
        node_add_child(array, child);
        skip_whitespace(p);
        if (**p == ',') {
            (*p)++;
            continue;
        } else if (**p == ']') {
            (*p)++;
            break;
        } else {
            json_free(array);
            return NULL;
        }
    }
    return array;
}

/* Разбор объекта */
static JsonNode *parse_object(const char **p, int depth) {
    JsonNode *object;
    char *key;
    JsonNode *value;

    object = node_new(JSON_OBJECT);
    if (!object) return NULL;

    if (**p != '{') {
        json_free(object);
        return NULL;
    }
    (*p)++;
    skip_whitespace(p);
    if (**p == '}') {
        (*p)++;
        return object;
    }

    while (1) {
        skip_whitespace(p);
        if (**p != '"') {
            json_free(object);
            return NULL;
        }
        key = parse_string(p);
        if (!key) {
            json_free(object);
            return NULL;
        }
        skip_whitespace(p);
        if (**p != ':') {
            free(key);
            json_free(object);
            return NULL;
        }
        (*p)++;
        value = parse_value(p, depth);
        if (!value) {
            free(key);
            json_free(object);
            return NULL;
        }
        value->key = key;
        node_add_child(object, value);

        skip_whitespace(p);
        if (**p == ',') {
            (*p)++;
            continue;
        } else if (**p == '}') {
            (*p)++;
            break;
        } else {
            json_free(object);
            return NULL;
        }
    }
    return object;
}

/* Разбор значения */
static JsonNode *parse_value(const char **p, int depth) {
    JsonNode *node;

    if (depth > MAX_DEPTH) return NULL;
    skip_whitespace(p);
    switch (**p) {
        case '{': return parse_object(p, depth+1);
        case '[': return parse_array(p, depth+1);
        case '"': {
            char *str = parse_string(p);
            if (!str) return NULL;
            node = node_new(JSON_STRING);
            if (node) node->string_value = str;
            else free(str);
            return node;
        }
        case 't':
            if (strncmp(*p, "true", 4) == 0) {
                (*p) += 4;
                node = node_new(JSON_BOOLEAN);
                if (node) node->bool_value = 1;
                return node;
            }
            return NULL;
        case 'f':
            if (strncmp(*p, "false", 5) == 0) {
                (*p) += 5;
                node = node_new(JSON_BOOLEAN);
                if (node) node->bool_value = 0;
                return node;
            }
            return NULL;
        case 'n':
            if (strncmp(*p, "null", 4) == 0) {
                (*p) += 4;
                return node_new(JSON_NULL);
            }
            return NULL;
        default:
            if (**p == '-' || isdigit((unsigned char)**p)) {
                return parse_number(p);
            }
            return NULL;
    }
}

JsonNode *json_parse(const char *text) {
    const char *p = text;
    JsonNode *root = parse_value(&p, 0);
    if (root) {
        skip_whitespace(&p);
        if (*p != '\0') {
            json_free(root);
            return NULL;
        }
    }
    return root;
}

JsonNode *json_find_child(JsonNode *object, const char *key) {
    int i;
    if (!object || object->type != JSON_OBJECT) return NULL;
    for (i = 0; i < object->child_count; i++) {
        JsonNode *child = object->children[i];
        if (child->key && strcmp(child->key, key) == 0) return child;
    }
    return NULL;
}

JsonNode *json_find_path(JsonNode *root, const char *path) {
    char *copy;
    char *p;
    char *token;
    JsonNode *current = root;

    copy = strdup(path);
    if (!copy) return NULL;

    p = copy;
    token = copy;
    while (*p && current) {
        if (*p == '/') {
            *p = '\0';
            if (current->type == JSON_OBJECT) {
                current = json_find_child(current, token);
            } else if (current->type == JSON_ARRAY) {
                int index = atoi(token);
                if (index < 0 || index >= current->child_count) current = NULL;
                else current = current->children[index];
            } else {
                current = NULL;
            }
            p++;
            token = p;
        } else {
            p++;
        }
    }
    if (current && *token) {
        /* последний сегмент */
        if (current->type == JSON_OBJECT) {
            current = json_find_child(current, token);
        } else if (current->type == JSON_ARRAY) {
            int index = atoi(token);
            if (index < 0 || index >= current->child_count) current = NULL;
            else current = current->children[index];
        } else {
            current = NULL;
        }
    }

    free(copy);
    return current;
}

const char *json_get_string(JsonNode *node) {
    if (!node || node->type != JSON_STRING) return NULL;
    return node->string_value;
}

void json_free(JsonNode *node) {
    int i;
    if (!node) return;
    for (i = 0; i < node->child_count; i++) {
        json_free(node->children[i]);
    }
    free(node->children);
    free(node->key);
    free(node->string_value);
    free(node);
}
