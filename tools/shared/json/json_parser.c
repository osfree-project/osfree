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

/* --- UTF-8 encoding of \uXXXX codepoints (RFC 8259 §7) --- */

static int utf8_encoded_len(unsigned long cp) {
    if (cp < 0x80) return 1;
    if (cp < 0x800) return 2;
    if (cp < 0x10000) return 3;
    return 4;
}

static int utf8_encode(unsigned long cp, char *out) {
    if (cp < 0x80) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = (char)(0xF0 | (cp >> 18));
    out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    out[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

static int hex4(const char *p, unsigned long *out) {
    int i;
    unsigned long v = 0;
    for (i = 0; i < 4; i++) {
        char c = p[i];
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else return 0;
        v = (v << 4) | (unsigned long)d;
    }
    *out = v;
    return 1;
}

static char *parse_string(const char **p) {
    const char *start;
    const char *end;
    char *result;
    char *q;
    const char *s;
    size_t len;

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
                    len += 1;
                    (*p)++;
                    break;
                case 'u': {
                    unsigned long cp;
                    (*p)++;
                    if (!hex4(*p, &cp)) return NULL;
                    (*p) += 4;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        unsigned long lo;
                        if ((*p)[0] != '\\' || (*p)[1] != 'u') return NULL;
                        if (!hex4(*p + 2, &lo)) return NULL;
                        if (lo < 0xDC00 || lo > 0xDFFF) return NULL;
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        (*p) += 6;
                    } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                        return NULL;
                    }
                    len += (size_t)utf8_encoded_len(cp);
                    break;
                }
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
    end = *p - 1;

    result = (char *)malloc(len + 1);
    if (!result) return NULL;

    q = result;
    s = start;
    while (s < end) {
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
                case 'u': {
                    unsigned long cp, lo;
                    s++;
                    hex4(s, &cp);
                    s += 4;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        hex4(s + 2, &lo);
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        s += 6;
                    }
                    q += utf8_encode(cp, q);
                    break;
                }
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


