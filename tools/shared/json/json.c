/* json.c - JSON parsing, building and serialization (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json.h"

/**
 * @file json.c
 * @brief Implementation of the JSON module.
 *
 * Conforms to:
 *   - RFC 8259, "The JavaScript Object Notation (JSON) Data
 *     Interchange Format".
 *   - ECMA-404, "The JSON Data Interchange Syntax".
 *
 * All nodes created during a document's lifetime are tracked in a
 * single list owned by the document, so that JsonClose can release
 * them regardless of whether they were attached to a parent.
 */

/* ==================================================================
 * Internal structures
 * ================================================================== */

/** @brief Maximum nesting depth accepted by the parser. */
#define JSON_MAX_DEPTH 100

/** @brief Initial capacity of a node's child list. */
#define JSON_CHILD_INIT 4

/** @brief Initial capacity of a document's node list. */
#define JSON_NODE_INIT 16

typedef struct _JSONNODE JSONNODE, *PJSONNODE;
typedef struct _JSONDOC  JSONDOC,  *PJSONDOC;

/**
 * @struct _JSONNODE
 * @brief One JSON value inside a document.
 *
 * A node may be detached (not attached to any parent) or attached to
 * an object or array. In an object, @c key holds the field name; in
 * an array it is NULL.
 */
struct _JSONNODE {
    JsonType          type;          /**< Value type.                  */
    char             *key;           /**< Field name, or NULL.         */
    char             *string_value;  /**< String value, or NULL.       */
    double            number_value;  /**< Numeric value.               */
    BOOL              bool_value;    /**< Boolean value.               */

    PJSONNODE        *children;      /**< Children, or NULL.           */
    ULONG             child_count;   /**< Used children.               */
    ULONG             child_capacity;/**< Allocated child slots.       */
};

/**
 * @struct _JSONDOC
 * @brief One open JSON document.
 *
 * All nodes created for this document are stored in @c all_nodes so
 * that JsonClose can release them.
 */
struct _JSONDOC {
    PJSONNODE  root;                 /**< Root node, or NULL.          */
    PJSONNODE *all_nodes;            /**< All nodes in the document.   */
    ULONG      node_count;           /**< Used node slots.             */
    ULONG      node_capacity;        /**< Allocated node slots.        */
};

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/**
 * @brief Duplicate a NUL-terminated string.
 *
 * @param[in] s  Source string, or NULL.
 *
 * @return malloc'd copy, or NULL on OOM.
 */
static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

/**
 * @brief Duplicate a byte range with a terminating NUL.
 *
 * @param[in] s  Source bytes. Not NULL.
 * @param[in] n  Number of bytes.
 *
 * @return malloc'd string, or NULL on OOM.
 */
static char *dup_n(const char *s, size_t n) {
    char *p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

/**
 * @brief Register a node in its document.
 *
 * @param[in,out] pd  Document. Not NULL.
 * @param[in]     pn  Node. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 */
static int doc_track_node(PJSONDOC pd, PJSONNODE pn) {
    if (pd->node_count >= pd->node_capacity) {
        ULONG ncap = pd->node_capacity ? pd->node_capacity * 2
                                       : JSON_NODE_INIT;
        PJSONNODE *na = (PJSONNODE*)realloc(pd->all_nodes,
                                            (size_t)ncap * sizeof(PJSONNODE));
        if (!na) return -1;
        pd->all_nodes = na;
        pd->node_capacity = ncap;
    }
    pd->all_nodes[pd->node_count++] = pn;
    return 0;
}

/**
 * @brief Create and register a new node with a given type.
 *
 * @param[in] pd  Document. Not NULL.
 * @param[in] t   Value type.
 *
 * @return New node, or NULL on OOM.
 */
static PJSONNODE node_new(PJSONDOC pd, JsonType t) {
    PJSONNODE pn = (PJSONNODE)calloc(1, sizeof(JSONNODE));
    if (!pn) return NULL;
    pn->type = t;
    pn->child_capacity = 0;
    pn->children = NULL;
    pn->child_count = 0;
    if (doc_track_node(pd, pn) != 0) {
        free(pn);
        return NULL;
    }
    return pn;
}

/**
 * @brief Append a child node to a parent.
 *
 * @param[in,out] parent  Parent node. Not NULL.
 * @param[in]     child   Child node. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 */
static int node_add_child(PJSONNODE parent, PJSONNODE child) {
    if (parent->child_count >= parent->child_capacity) {
        ULONG ncap = parent->child_capacity ? parent->child_capacity * 2
                                             : JSON_CHILD_INIT;
        PJSONNODE *na = (PJSONNODE*)realloc(parent->children,
                                            (size_t)ncap * sizeof(PJSONNODE));
        if (!na) return -1;
        parent->children = na;
        parent->child_capacity = ncap;
    }
    parent->children[parent->child_count++] = child;
    return 0;
}

/**
 * @brief Release a document and every node in it.
 *
 * @param[in] pd  Document. May be NULL.
 */
static void doc_free(PJSONDOC pd) {
    ULONG i;
    if (!pd) return;
    for (i = 0; i < pd->node_count; i++) {
        PJSONNODE pn = pd->all_nodes[i];
        if (!pn) continue;
        free(pn->key);
        free(pn->string_value);
        free(pn->children);
        free(pn);
    }
    free(pd->all_nodes);
    free(pd);
}

/* ==================================================================
 * Parser
 * ================================================================== */

/**
 * @struct _PARSE
 * @brief Parser cursor.
 */
typedef struct {
    const char *p;      /**< Current position in the input text.  */
    PJSONDOC    pd;     /**< Owning document.                     */
} PARSE;

/**
 * @brief Skip whitespace in the input.
 *
 * @param[in,out] ps  Parser. Not NULL.
 */
static void skip_whitespace(PARSE *ps) {
    while (*ps->p && isspace((unsigned char)*ps->p)) ps->p++;
}

/**
 * @brief Validate a NUL-terminated UTF-8 string (RFC 3629).
 *
 * Rejects overlong encodings, surrogates and codepoints above
 * U+10FFFF.
 *
 * @param[in] s  String. Not NULL.
 *
 * @return 1 if valid, 0 otherwise.
 */
static int validate_utf8(const char *s) {
    const unsigned char *p = (const unsigned char*)s;
    while (*p) {
        unsigned char c = *p++;
        if (c < 0x80) continue;
        if ((c & 0xE0) == 0xC0) {
            unsigned long cp;
            if ((*p & 0xC0) != 0x80) return 0;
            cp = ((unsigned long)(c & 0x1F) << 6) |
                 (unsigned long)(*p & 0x3F);
            if (cp < 0x80) return 0;
            p++;
        } else if ((c & 0xF0) == 0xE0) {
            unsigned long cp;
            if ((p[0] & 0xC0) != 0x80) return 0;
            if ((p[1] & 0xC0) != 0x80) return 0;
            cp = ((unsigned long)(c & 0x0F) << 12) |
                 ((unsigned long)(p[0] & 0x3F) << 6) |
                 (unsigned long)(p[1] & 0x3F);
            if (cp < 0x800) return 0;
            if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
            p += 2;
        } else if ((c & 0xF8) == 0xF0) {
            unsigned long cp;
            if ((p[0] & 0xC0) != 0x80) return 0;
            if ((p[1] & 0xC0) != 0x80) return 0;
            if ((p[2] & 0xC0) != 0x80) return 0;
            cp = ((unsigned long)(c & 0x07) << 18) |
                 ((unsigned long)(p[0] & 0x3F) << 12) |
                 ((unsigned long)(p[1] & 0x3F) << 6) |
                 (unsigned long)(p[2] & 0x3F);
            if (cp < 0x10000 || cp > 0x10FFFF) return 0;
            p += 3;
        } else {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Encode a Unicode scalar value as UTF-8.
 *
 * @param[in]  cp   Codepoint. Must be <= U+10FFFF and not a
 *                  surrogate.
 * @param[out] out  Output buffer. Must have room for up to 4 bytes.
 *
 * @return Number of bytes written (1..4).
 */
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

/**
 * @brief Number of bytes utf8_encode will write for @p cp.
 *
 * @param[in] cp  Codepoint.
 *
 * @return 1, 2, 3 or 4.
 */
static int utf8_encoded_len(unsigned long cp) {
    if (cp < 0x80) return 1;
    if (cp < 0x800) return 2;
    if (cp < 0x10000) return 3;
    return 4;
}

/**
 * @brief Read 4 hex digits.
 *
 * @param[in]  p    Pointer to 4 hex characters. Not NULL.
 * @param[out] out  Receiver.
 *
 * @return 1 on success, 0 on malformed input.
 */
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

/**
 * @brief Parse a JSON string.
 *
 * @param[in,out] p  Cursor pointing at the opening '"'. On success,
 *                   advanced past the closing '"'.
 *
 * @return malloc'd NUL-terminated string, or NULL on error.
 */
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

    result = (char*)malloc(len + 1);
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

/**
 * @brief Return 10 raised to integer power @p exp.
 *
 * @param[in] exp  Exponent.
 *
 * @return 10 ** @p exp.
 */
static double pow10_int(int exp) {
    double r = 1.0;
    int i;
    if (exp >= 0) {
        for (i = 0; i < exp; i++) r *= 10.0;
    } else {
        for (i = 0; i > exp; i--) r *= 0.1;
    }
    return r;
}

/**
 * @brief Parse a JSON number (RFC 8259 §6).
 *
 * @param[in,out] p    Cursor. On success, advanced past the number.
 * @param[out]    out  Receiver.
 *
 * @return 1 on success, 0 on malformed input.
 */
static int parse_number(const char **p, double *out) {
    const char *s = *p;
    const char *start = s;
    double sign = 1.0;
    double val = 0.0;

    if (*s == '-') { sign = -1.0; s++; }

    if (*s == '0') {
        s++;
        if (isdigit((unsigned char)*s)) return 0;
    } else if (*s >= '1' && *s <= '9') {
        while (isdigit((unsigned char)*s)) {
            val = val * 10.0 + (double)(*s - '0');
            s++;
        }
    } else {
        return 0;
    }

    if (*s == '.') {
        double scale = 0.1;
        s++;
        if (!isdigit((unsigned char)*s)) return 0;
        while (isdigit((unsigned char)*s)) {
            val += (double)(*s - '0') * scale;
            scale *= 0.1;
            s++;
        }
    }

    if (*s == 'e' || *s == 'E') {
        int exp_sign = 1;
        int exp_val = 0;
        s++;
        if (*s == '+') s++;
        else if (*s == '-') { exp_sign = -1; s++; }
        if (!isdigit((unsigned char)*s)) return 0;
        while (isdigit((unsigned char)*s)) {
            if (exp_val < 100000)
                exp_val = exp_val * 10 + (*s - '0');
            s++;
        }
        val *= pow10_int(exp_sign * exp_val);
    }

    if (s == start) return 0;
    *p = s;
    *out = sign * val;
    return 1;
}

static int parse_value(PARSE *ps, int depth, PJSONNODE *out);

/**
 * @brief Parse a JSON array.
 *
 * @param[in,out] ps     Parser. Not NULL.
 * @param[in]     depth  Current nesting depth.
 * @param[out]    out    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 */
static int parse_array(PARSE *ps, int depth, PJSONNODE *out) {
    PJSONNODE arr;

    arr = node_new(ps->pd, JSON_ARRAY);
    if (!arr) return -1;

    if (*ps->p != '[') return -1;
    ps->p++;
    skip_whitespace(ps);
    if (*ps->p == ']') {
        ps->p++;
        *out = arr;
        return 0;
    }

    for (;;) {
        PJSONNODE child = NULL;
        skip_whitespace(ps);
        if (parse_value(ps, depth, &child) != 0) return -1;
        if (node_add_child(arr, child) != 0) return -1;
        skip_whitespace(ps);
        if (*ps->p == ',') {
            ps->p++;
            continue;
        } else if (*ps->p == ']') {
            ps->p++;
            break;
        } else {
            return -1;
        }
    }
    *out = arr;
    return 0;
}

/**
 * @brief Parse a JSON object.
 *
 * @param[in,out] ps     Parser. Not NULL.
 * @param[in]     depth  Current nesting depth.
 * @param[out]    out    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 */
static int parse_object(PARSE *ps, int depth, PJSONNODE *out) {
    PJSONNODE obj;

    obj = node_new(ps->pd, JSON_OBJECT);
    if (!obj) return -1;

    if (*ps->p != '{') return -1;
    ps->p++;
    skip_whitespace(ps);
    if (*ps->p == '}') {
        ps->p++;
        *out = obj;
        return 0;
    }

    for (;;) {
        char *key;
        PJSONNODE value = NULL;

        skip_whitespace(ps);
        if (*ps->p != '"') return -1;
        key = parse_string(&ps->p);
        if (!key) return -1;
        skip_whitespace(ps);
        if (*ps->p != ':') {
            free(key);
            return -1;
        }
        ps->p++;
        if (parse_value(ps, depth, &value) != 0) {
            free(key);
            return -1;
        }
        value->key = key;
        if (node_add_child(obj, value) != 0) return -1;

        skip_whitespace(ps);
        if (*ps->p == ',') {
            ps->p++;
            continue;
        } else if (*ps->p == '}') {
            ps->p++;
            break;
        } else {
            return -1;
        }
    }
    *out = obj;
    return 0;
}

/**
 * @brief Parse one JSON value.
 *
 * Dispatches on the current character and recurses into the parser
 * for objects and arrays.
 *
 * @param[in,out] ps     Parser. Not NULL.
 * @param[in]     depth  Current nesting depth.
 * @param[out]    out    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 */
static int parse_value(PARSE *ps, int depth, PJSONNODE *out) {
    PJSONNODE node;

    if (depth > JSON_MAX_DEPTH) return -1;
    skip_whitespace(ps);

    switch (*ps->p) {
        case '{': return parse_object(ps, depth + 1, out);
        case '[': return parse_array(ps, depth + 1, out);
        case '"': {
            char *str = parse_string(&ps->p);
            if (!str) return -1;
            node = node_new(ps->pd, JSON_STRING);
            if (!node) { free(str); return -1; }
            node->string_value = str;
            *out = node;
            return 0;
        }
        case 't':
            if (strncmp(ps->p, "true", 4) == 0) {
                ps->p += 4;
                node = node_new(ps->pd, JSON_BOOLEAN);
                if (!node) return -1;
                node->bool_value = TRUE_;
                *out = node;
                return 0;
            }
            return -1;
        case 'f':
            if (strncmp(ps->p, "false", 5) == 0) {
                ps->p += 5;
                node = node_new(ps->pd, JSON_BOOLEAN);
                if (!node) return -1;
                node->bool_value = FALSE_;
                *out = node;
                return 0;
            }
            return -1;
        case 'n':
            if (strncmp(ps->p, "null", 4) == 0) {
                ps->p += 4;
                node = node_new(ps->pd, JSON_NULL);
                if (!node) return -1;
                *out = node;
                return 0;
            }
            return -1;
        default:
            if (*ps->p == '-' || isdigit((unsigned char)*ps->p)) {
                double d;
                if (!parse_number(&ps->p, &d)) return -1;
                node = node_new(ps->pd, JSON_NUMBER);
                if (!node) return -1;
                node->number_value = d;
                *out = node;
                return 0;
            }
            return -1;
    }
}

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/**
 * @brief Parse a NUL-terminated JSON text into a document.
 *
 * On success, @p *phDoc receives a document handle. The input buffer
 * is only read; it is not retained. The caller owns the buffer.
 *
 * @param[in]  pszText  JSON text. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszText or phDoc is NULL.
 * @retval ERROR_INVALID_DATA       Malformed JSON, invalid UTF-8,
 *                                  or trailing bytes after the value.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonParse(PCSZ pszText, HJSONDOC *phDoc) {
    const char *p = pszText;
    PARSE ps;
    PJSONDOC pd;
    PJSONNODE root = NULL;

    if (!pszText || !phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;

    /* Skip UTF-8 BOM (RFC 8259 §8.1: implementations MAY ignore) */
    if ((unsigned char)p[0] == 0xEF &&
        (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    if (!validate_utf8(p)) return ERROR_INVALID_DATA;

    pd = (PJSONDOC)calloc(1, sizeof(JSONDOC));
    if (!pd) return ERROR_NOT_ENOUGH_MEMORY;

    ps.p = p;
    ps.pd = pd;

    if (parse_value(&ps, 0, &root) != 0) {
        doc_free(pd);
        return ERROR_INVALID_DATA;
    }

    skip_whitespace(&ps);
    if (*ps.p != '\0') {
        doc_free(pd);
        return ERROR_INVALID_DATA;
    }

    pd->root = root;
    *phDoc = (HJSONDOC)pd;
    return NO_ERROR;
}

/**
 * @brief Release a document and all associated nodes.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY JsonClose(HJSONDOC hDoc) {
    if (hDoc == NULLHANDLE) return NO_ERROR;
    doc_free((PJSONDOC)hDoc);
    return NO_ERROR;
}

/* ==================================================================
 * Building
 * ================================================================== */

/**
 * @brief Create an empty document for building a tree.
 *
 * Nodes created with JsonNew* belong to the document and are
 * released by JsonClose. Before serialization the caller must
 * designate a root node with JsonSetRoot.
 *
 * @param[out] phDoc  Handle receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewDoc(HJSONDOC *phDoc) {
    PJSONDOC pd;

    if (!phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;

    pd = (PJSONDOC)calloc(1, sizeof(JSONDOC));
    if (!pd) return ERROR_NOT_ENOUGH_MEMORY;

    *phDoc = (HJSONDOC)pd;
    return NO_ERROR;
}

/**
 * @brief Designate the root node of a document.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hNode  Root node. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or hNode is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY JsonSetRoot(HJSONDOC hDoc, HJSONNODE hNode) {
    PJSONDOC pd;
    if (hDoc == NULLHANDLE || hNode == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    pd = (PJSONDOC)hDoc;
    pd->root = (PJSONNODE)hNode;
    return NO_ERROR;
}

/**
 * @brief Create a new object node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewObject(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONNODE pn;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pn = node_new((PJSONDOC)hDoc, JSON_OBJECT);
    if (!pn) return ERROR_NOT_ENOUGH_MEMORY;
    *phNode = (HJSONNODE)pn;
    return NO_ERROR;
}

/**
 * @brief Create a new array node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewArray(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONNODE pn;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pn = node_new((PJSONDOC)hDoc, JSON_ARRAY);
    if (!pn) return ERROR_NOT_ENOUGH_MEMORY;
    *phNode = (HJSONNODE)pn;
    return NO_ERROR;
}

/**
 * @brief Create a new string node.
 *
 * The value is copied into the node.
 *
 * @param[in]  hDoc      Document handle. Not NULLHANDLE.
 * @param[in]  pszValue  String value. Not NULL.
 * @param[out] phNode    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE, pszValue or
 *                                  phNode is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewString(HJSONDOC hDoc, PCSZ pszValue,
                              HJSONNODE *phNode) {
    PJSONNODE pn;
    char *copy;
    if (hDoc == NULLHANDLE || !pszValue || !phNode)
        return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    copy = dup_str(pszValue);
    if (!copy) return ERROR_NOT_ENOUGH_MEMORY;
    pn = node_new((PJSONDOC)hDoc, JSON_STRING);
    if (!pn) { free(copy); return ERROR_NOT_ENOUGH_MEMORY; }
    pn->string_value = copy;
    *phNode = (HJSONNODE)pn;
    return NO_ERROR;
}

/**
 * @brief Create a new number node.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  dValue  Numeric value.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewNumber(HJSONDOC hDoc, double dValue,
                              HJSONNODE *phNode) {
    PJSONNODE pn;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pn = node_new((PJSONDOC)hDoc, JSON_NUMBER);
    if (!pn) return ERROR_NOT_ENOUGH_MEMORY;
    pn->number_value = dValue;
    *phNode = (HJSONNODE)pn;
    return NO_ERROR;
}

/**
 * @brief Create a new boolean node.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  fValue  TRUE_ or FALSE_.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewBoolean(HJSONDOC hDoc, BOOL fValue,
                               HJSONNODE *phNode) {
    PJSONNODE pn;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pn = node_new((PJSONDOC)hDoc, JSON_BOOLEAN);
    if (!pn) return ERROR_NOT_ENOUGH_MEMORY;
    pn->bool_value = fValue ? TRUE_ : FALSE_;
    *phNode = (HJSONNODE)pn;
    return NO_ERROR;
}

/**
 * @brief Create a new null node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewNull(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONNODE pn;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pn = node_new((PJSONDOC)hDoc, JSON_NULL);
    if (!pn) return ERROR_NOT_ENOUGH_MEMORY;
    *phNode = (HJSONNODE)pn;
    return NO_ERROR;
}

/**
 * @brief Attach a value to an object under a key.
 *
 * The key is copied into the object. The value node must belong to
 * the same document as the object.
 *
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] hValue  Value node. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       hObj is not an object.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonObjectSet(HJSONNODE hObj, PCSZ pszKey,
                              HJSONNODE hValue) {
    PJSONNODE pObj;
    PJSONNODE pVal;
    char *copy;

    if (hObj == NULLHANDLE || !pszKey || hValue == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    pObj = (PJSONNODE)hObj;
    pVal = (PJSONNODE)hValue;
    if (pObj->type != JSON_OBJECT) return ERROR_INVALID_DATA;
    copy = dup_str(pszKey);
    if (!copy) return ERROR_NOT_ENOUGH_MEMORY;
    pVal->key = copy;
    if (node_add_child(pObj, pVal) != 0) {
        free(copy);
        pVal->key = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/**
 * @brief Append a value to an array.
 *
 * @param[in] hArr    Array handle. Not NULLHANDLE.
 * @param[in] hValue  Value node. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hArr or hValue is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       hArr is not an array.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonArrayAppend(HJSONNODE hArr, HJSONNODE hValue) {
    PJSONNODE pArr;
    PJSONNODE pVal;

    if (hArr == NULLHANDLE || hValue == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    pArr = (PJSONNODE)hArr;
    pVal = (PJSONNODE)hValue;
    if (pArr->type != JSON_ARRAY) return ERROR_INVALID_DATA;
    if (node_add_child(pArr, pVal) != 0) return ERROR_NOT_ENOUGH_MEMORY;
    return NO_ERROR;
}

/* ==================================================================
 * Reading values
 * ================================================================== */

/**
 * @brief Obtain the root node of a parsed document.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phNode is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Document has no root.
 */
APIRET APIENTRY JsonRoot(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONDOC pd;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    pd = (PJSONDOC)hDoc;
    if (!pd->root) return ERROR_FILE_NOT_FOUND;
    *phNode = (HJSONNODE)pd->root;
    return NO_ERROR;
}

/**
 * @brief Query the type of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of a JsonType value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode or pulType is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY JsonNodeGetType(HJSONNODE hNode, PULONG pulType) {
    PJSONNODE pn = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pulType) return ERROR_INVALID_PARAMETER;
    *pulType = (ULONG)pn->type;
    return NO_ERROR;
}

/**
 * @brief Obtain a child of an object node by key.
 *
 * @param[in]  hNode    Node handle (object). Not NULLHANDLE.
 * @param[in]  pszKey   Key. Not NULL.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an object.
 * @retval ERROR_FILE_NOT_FOUND     Key not present.
 */
APIRET APIENTRY JsonNodeGetChild(HJSONNODE hNode, PCSZ pszKey,
                                 HJSONNODE *phChild) {
    PJSONNODE pn = (PJSONNODE)hNode;
    ULONG i;

    if (hNode == NULLHANDLE || !pszKey || !phChild)
        return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pn->type != JSON_OBJECT) return ERROR_INVALID_DATA;

    for (i = 0; i < pn->child_count; i++) {
        if (pn->children[i]->key &&
            strcmp(pn->children[i]->key, pszKey) == 0) {
            *phChild = (HJSONNODE)pn->children[i];
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/**
 * @brief Number of elements in an array node or entries in an
 *        object node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is neither array nor object.
 */
APIRET APIENTRY JsonNodeGetCount(HJSONNODE hNode, PULONG pulCount) {
    PJSONNODE pn = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    if (pn->type != JSON_ARRAY && pn->type != JSON_OBJECT)
        return ERROR_INVALID_DATA;
    *pulCount = pn->child_count;
    return NO_ERROR;
}

/**
 * @brief Obtain an element of an array node by index.
 *
 * @param[in]  hNode    Node handle (array). Not NULLHANDLE.
 * @param[in]  ulIndex  Element index.
 * @param[out] phChild  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an array.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY JsonNodeGetElement(HJSONNODE hNode, ULONG ulIndex,
                                   HJSONNODE *phChild) {
    PJSONNODE pn = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !phChild) return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pn->type != JSON_ARRAY) return ERROR_INVALID_DATA;
    if (ulIndex >= pn->child_count) return ERROR_NO_MORE_ITEMS;
    *phChild = (HJSONNODE)pn->children[ulIndex];
    return NO_ERROR;
}

/**
 * @brief Obtain an entry of an object node by index.
 *
 * @param[in]  hNode       Node handle (object). Not NULLHANDLE.
 * @param[in]  ulIndex     Entry index.
 * @param[out] pszKeyBuf   Key buffer. Not NULL.
 * @param[in]  ulKeySize   Size of pszKeyBuf.
 * @param[out] pulKeyUsed  Optional. May be NULL.
 * @param[out] phChild     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any required parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an object.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Key buffer too small.
 */
APIRET APIENTRY JsonNodeGetEntry(HJSONNODE hNode, ULONG ulIndex,
                                 PSZ pszKeyBuf, ULONG ulKeySize,
                                 PULONG pulKeyUsed, HJSONNODE *phChild) {
    PJSONNODE pn = (PJSONNODE)hNode;
    const char *key;
    size_t klen;

    if (hNode == NULLHANDLE || !pszKeyBuf || !phChild)
        return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pn->type != JSON_OBJECT) return ERROR_INVALID_DATA;
    if (ulIndex >= pn->child_count) return ERROR_NO_MORE_ITEMS;

    key = pn->children[ulIndex]->key;
    if (!key) key = "";
    klen = strlen(key);

    if (ulKeySize < klen + 1) {
        if (pulKeyUsed) *pulKeyUsed = (ULONG)(klen + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszKeyBuf, key, klen);
    pszKeyBuf[klen] = '\0';
    if (pulKeyUsed) *pulKeyUsed = (ULONG)klen;
    *phChild = (HJSONNODE)pn->children[ulIndex];
    return NO_ERROR;
}

/**
 * @brief Read a string value of a node.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied and NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hNode    Node handle (string). Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonNodeGetString(HJSONNODE hNode,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PJSONNODE pn = (PJSONNODE)hNode;
    const char *s;
    size_t n;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (pn->type != JSON_STRING) return ERROR_INVALID_DATA;

    s = pn->string_value ? pn->string_value : "";
    n = strlen(s);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < n + 1) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, s, n);
    pszBuf[n] = '\0';
    if (pulUsed) *pulUsed = (ULONG)n;
    return NO_ERROR;
}

/**
 * @brief Read a boolean value of a node.
 *
 * @param[in]  hNode    Node handle (boolean). Not NULLHANDLE.
 * @param[out] pfValue  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a boolean.
 */
APIRET APIENTRY JsonNodeGetBoolean(HJSONNODE hNode, PBOOL pfValue) {
    PJSONNODE pn = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pfValue) return ERROR_INVALID_PARAMETER;
    if (pn->type != JSON_BOOLEAN) return ERROR_INVALID_DATA;
    *pfValue = pn->bool_value;
    return NO_ERROR;
}

/**
 * @brief Read a number value of a node.
 *
 * @param[in]  hNode     Node handle (number). Not NULLHANDLE.
 * @param[out] pdValue   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a number.
 */
APIRET APIENTRY JsonNodeGetNumber(HJSONNODE hNode, double *pdValue) {
    PJSONNODE pn = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pdValue) return ERROR_INVALID_PARAMETER;
    if (pn->type != JSON_NUMBER) return ERROR_INVALID_DATA;
    *pdValue = pn->number_value;
    return NO_ERROR;
}

/* ==================================================================
 * Node modification and cloning
 * ================================================================== */

/**
 * @brief Retrieve the key of a node.
 *
 * For a child of an object, this is the field name. For a child of an
 * array, or for the root node, the key is empty.
 *
 * Size-query convention as for JsonNodeGetString.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonNodeGetKey(HJSONNODE hNode,
                               PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PJSONNODE pn = (PJSONNODE)hNode;
    const char *key;
    size_t n;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    key = pn->key ? pn->key : "";
    n = strlen(key);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < n + 1) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, key, n);
    pszBuf[n] = '\0';
    if (pulUsed) *pulUsed = (ULONG)n;
    return NO_ERROR;
}

/**
 * @brief Set the string value of an existing string node.
 *
 * The node must be of type JSON_STRING. The new value is copied into
 * the node.
 *
 * @param[in] hNode    Node handle (string). Not NULLHANDLE.
 * @param[in] pszVal   New value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE, or pszVal is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNodeSetValueString(HJSONNODE hNode, PCSZ pszVal) {
    PJSONNODE pn = (PJSONNODE)hNode;
    char *copy;

    if (hNode == NULLHANDLE || !pszVal) return ERROR_INVALID_PARAMETER;
    if (pn->type != JSON_STRING) return ERROR_INVALID_DATA;

    copy = dup_str(pszVal);
    if (!copy) return ERROR_NOT_ENOUGH_MEMORY;

    free(pn->string_value);
    pn->string_value = copy;
    return NO_ERROR;
}

/**
 * @brief Set or replace a string field on an object.
 *
 * If the object already has a child with the given key and that child
 * is a string, its value is replaced. Otherwise a new string child
 * is appended.
 *
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] pszVal  Field value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       hObj is not an object, or the
 *                                  existing child is not a string.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNodeSetString(HJSONDOC hDoc, HJSONNODE hObj,
                                  PCSZ pszKey, PCSZ pszVal) {
    PJSONNODE pObj = (PJSONNODE)hObj;
    ULONG i;

    if (hDoc == NULLHANDLE || hObj == NULLHANDLE ||
        !pszKey || !pszVal)
        return ERROR_INVALID_PARAMETER;
    if (pObj->type != JSON_OBJECT) return ERROR_INVALID_DATA;

    for (i = 0; i < pObj->child_count; i++) {
        PJSONNODE pChild = pObj->children[i];
        if (pChild->key && strcmp(pChild->key, pszKey) == 0) {
            if (pChild->type != JSON_STRING) return ERROR_INVALID_DATA;
            return JsonNodeSetValueString((HJSONNODE)pChild, pszVal);
        }
    }

    {
        HJSONNODE hNew = NULLHANDLE;
        APIRET rc = JsonNewString(hDoc, pszVal, &hNew);
        if (rc != NO_ERROR) return rc;
        return JsonObjectSet(hObj, pszKey, hNew);
    }
}

/**
 * @brief Deep-clone a subtree into a document.
 *
 * The source node must belong to some document; the destination
 * document receives a full copy of the subtree, with all keys and
 * values. Returns a handle for the new root of the copy.
 *
 * @param[in]  hDst   Destination document. Not NULLHANDLE.
 * @param[in]  hSrc   Source node. Not NULLHANDLE.
 * @param[out] phDst  Receiver for the new node. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonCloneNode(HJSONDOC hDst, HJSONNODE hSrc,
                              HJSONNODE *phDst) {
    PJSONDOC  pdDst = (PJSONDOC)hDst;
    PJSONNODE pSrc  = (PJSONNODE)hSrc;
    PJSONNODE pDst;
    ULONG i;

    if (hDst == NULLHANDLE || hSrc == NULLHANDLE || !phDst)
        return ERROR_INVALID_PARAMETER;
    *phDst = NULLHANDLE;

    pDst = node_new(pdDst, pSrc->type);
    if (!pDst) return ERROR_NOT_ENOUGH_MEMORY;

    if (pSrc->key) {
        pDst->key = dup_str(pSrc->key);
        if (!pDst->key) return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (pSrc->string_value) {
        pDst->string_value = dup_str(pSrc->string_value);
        if (!pDst->string_value) return ERROR_NOT_ENOUGH_MEMORY;
    }
    pDst->number_value = pSrc->number_value;
    pDst->bool_value = pSrc->bool_value;

    for (i = 0; i < pSrc->child_count; i++) {
        HJSONNODE hChildClone = NULLHANDLE;
        APIRET rc = JsonCloneNode(hDst, (HJSONNODE)pSrc->children[i],
                                  &hChildClone);
        if (rc != NO_ERROR) return rc;
        if (node_add_child(pDst, (PJSONNODE)hChildClone) != 0)
            return ERROR_NOT_ENOUGH_MEMORY;
    }

    *phDst = (HJSONNODE)pDst;
    return NO_ERROR;
}

/* ==================================================================
 * Serialization
 * ================================================================== */

/**
 * @struct _SBUF
 * @brief Growable string buffer used during serialization.
 */
typedef struct {
    char  *buf;   /**< Backing storage, or NULL.       */
    size_t cap;   /**< Allocated bytes.                */
    size_t len;   /**< Used bytes, excluding NUL.      */
    int    oom;   /**< Non-zero once an OOM occurred.  */
} SBUF;

/**
 * @brief Initialize a string buffer.
 *
 * @param[out] sb  Buffer. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 */
static int sbuf_init(SBUF *sb) {
    sb->cap = 64;
    sb->len = 0;
    sb->oom = 0;
    sb->buf = (char*)malloc(sb->cap);
    if (!sb->buf) { sb->oom = 1; return -1; }
    sb->buf[0] = '\0';
    return 0;
}

/**
 * @brief Release a string buffer.
 *
 * @param[in,out] sb  Buffer. Not NULL.
 */
static void sbuf_free(SBUF *sb) {
    free(sb->buf);
    sb->buf = NULL;
    sb->cap = 0;
    sb->len = 0;
}

/**
 * @brief Append bytes to a string buffer.
 *
 * @param[in,out] sb    Buffer. Not NULL.
 * @param[in]     data  Bytes to append. Not NULL.
 * @param[in]     n     Number of bytes.
 */
static void sbuf_put(SBUF *sb, const char *data, size_t n) {
    if (sb->oom) return;
    if (sb->len + n + 1 > sb->cap) {
        size_t ncap = sb->cap * 2 + n + 64;
        char *nb = (char*)realloc(sb->buf, ncap);
        if (!nb) { sb->oom = 1; return; }
        sb->buf = nb;
        sb->cap = ncap;
    }
    memcpy(sb->buf + sb->len, data, n);
    sb->len += n;
    sb->buf[sb->len] = '\0';
}

/**
 * @brief Append one character to a string buffer.
 *
 * @param[in,out] sb  Buffer. Not NULL.
 * @param[in]     c   Character.
 */
static void sbuf_putc(SBUF *sb, char c) {
    sbuf_put(sb, &c, 1);
}

/**
 * @brief Append a NUL-terminated string to a string buffer.
 *
 * @param[in,out] sb  Buffer. Not NULL.
 * @param[in]     s   String. Not NULL.
 */
static void sbuf_puts(SBUF *sb, const char *s) {
    sbuf_put(sb, s, strlen(s));
}

/**
 * @brief Append a JSON-quoted and escaped string.
 *
 * @param[in,out] sb  Buffer. Not NULL.
 * @param[in]     s   String value. May be NULL (treated as "").
 */
static void sbuf_put_escaped(SBUF *sb, const char *s) {
    static const char hex[] = "0123456789ABCDEF";
    if (!s) s = "";
    sbuf_putc(sb, '"');
    while (*s) {
        unsigned char c = (unsigned char)*s++;
        switch (c) {
            case '"':  sbuf_puts(sb, "\\\""); break;
            case '\\': sbuf_puts(sb, "\\\\"); break;
            case '\b': sbuf_puts(sb, "\\b"); break;
            case '\f': sbuf_puts(sb, "\\f"); break;
            case '\n': sbuf_puts(sb, "\\n"); break;
            case '\r': sbuf_puts(sb, "\\r"); break;
            case '\t': sbuf_puts(sb, "\\t"); break;
            default:
                if (c < 0x20) {
                    char tmp[7];
                    tmp[0] = '\\';
                    tmp[1] = 'u';
                    tmp[2] = '0';
                    tmp[3] = '0';
                    tmp[4] = hex[(c >> 4) & 0x0F];
                    tmp[5] = hex[c & 0x0F];
                    tmp[6] = '\0';
                    sbuf_puts(sb, tmp);
                } else {
                    sbuf_putc(sb, (char)c);
                }
                break;
        }
    }
    sbuf_putc(sb, '"');
}

/**
 * @brief Append a JSON number to a string buffer.
 *
 * @param[in,out] sb  Buffer. Not NULL.
 * @param[in]     d   Numeric value.
 */
static void sbuf_put_number(SBUF *sb, double d) {
    char tmp[64];
    sprintf(tmp, "%g", d);
    sbuf_puts(sb, tmp);
}

/**
 * @brief Append a newline and indentation to a string buffer.
 *
 * @param[in,out] sb      Buffer. Not NULL.
 * @param[in]     indent  Nesting depth.
 */
static void sbuf_indent(SBUF *sb, int indent) {
    int i;
    sbuf_putc(sb, '\n');
    for (i = 0; i < indent * 2; i++) sbuf_putc(sb, ' ');
}

/**
 * @brief Serialize a node into a string buffer.
 *
 * @param[in,out] sb       Buffer. Not NULL.
 * @param[in]     pn       Node. Not NULL.
 * @param[in]     fIndent  Non-zero for pretty-printed output.
 * @param[in]     depth    Current nesting depth.
 */
static void sbuf_write_node(SBUF *sb, PJSONNODE pn, int fIndent, int depth) {
    ULONG i;
    if (sb->oom) return;

    switch (pn->type) {
        case JSON_NULL:
            sbuf_puts(sb, "null");
            break;
        case JSON_BOOLEAN:
            sbuf_puts(sb, pn->bool_value ? "true" : "false");
            break;
        case JSON_NUMBER:
            sbuf_put_number(sb, pn->number_value);
            break;
        case JSON_STRING:
            sbuf_put_escaped(sb, pn->string_value);
            break;
        case JSON_ARRAY:
            sbuf_putc(sb, '[');
            for (i = 0; i < pn->child_count; i++) {
                if (i > 0) sbuf_putc(sb, ',');
                if (fIndent) sbuf_indent(sb, depth + 1);
                sbuf_write_node(sb, pn->children[i], fIndent, depth + 1);
            }
            if (fIndent && pn->child_count > 0) sbuf_indent(sb, depth);
            sbuf_putc(sb, ']');
            break;
        case JSON_OBJECT:
            sbuf_putc(sb, '{');
            for (i = 0; i < pn->child_count; i++) {
                if (i > 0) sbuf_putc(sb, ',');
                if (fIndent) sbuf_indent(sb, depth + 1);
                sbuf_put_escaped(sb, pn->children[i]->key);
                sbuf_putc(sb, ':');
                if (fIndent) sbuf_putc(sb, ' ');
                sbuf_write_node(sb, pn->children[i], fIndent, depth + 1);
            }
            if (fIndent && pn->child_count > 0) sbuf_indent(sb, depth);
            sbuf_putc(sb, '}');
            break;
    }
}

/**
 * @brief Serialize a node into a caller-supplied buffer.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: text copied and NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[in]  fIndent   TRUE_ for pretty-printed output with two-space
 *                       indentation; FALSE_ for compact output.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonFormat(HJSONNODE hNode, BOOL fIndent,
                           PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PJSONNODE pn = (PJSONNODE)hNode;
    SBUF sb;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    if (sbuf_init(&sb) != 0) return ERROR_NOT_ENOUGH_MEMORY;

    sbuf_write_node(&sb, pn, fIndent ? 1 : 0, 0);
    if (sb.oom) {
        sbuf_free(&sb);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(sb.len + 1);
        sbuf_free(&sb);
        return NO_ERROR;
    }
    if (ulSize < sb.len + 1) {
        if (pulUsed) *pulUsed = (ULONG)(sb.len + 1);
        sbuf_free(&sb);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, sb.buf, sb.len);
    pszBuf[sb.len] = '\0';
    if (pulUsed) *pulUsed = (ULONG)sb.len;

    sbuf_free(&sb);
    return NO_ERROR;
}

/**
 * @brief Serialize a node into a file or to stdout.
 *
 * When @p pszPath is NULL, the output is written to stdout.
 *
 * @param[in] hNode    Node handle. Not NULLHANDLE.
 * @param[in] fIndent  TRUE_ for pretty-printed output; FALSE_ for
 *                     compact output.
 * @param[in] pszPath  Output file path, or NULL for stdout.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_OPEN_FAILED        Cannot open output file.
 * @retval ERROR_READ_FAULT         Write error.
 */
APIRET APIENTRY JsonWriteFile(HJSONNODE hNode, BOOL fIndent,
                              PCSZ pszPath) {
    PJSONNODE pn = (PJSONNODE)hNode;
    SBUF sb;
    FILE *f;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    if (sbuf_init(&sb) != 0) return ERROR_NOT_ENOUGH_MEMORY;
    sbuf_write_node(&sb, pn, fIndent ? 1 : 0, 0);
    if (sb.oom) {
        sbuf_free(&sb);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pszPath) {
        f = fopen(pszPath, "wb");
        if (!f) { sbuf_free(&sb); return ERROR_OPEN_FAILED; }
    } else {
        f = stdout;
    }

    if (fwrite(sb.buf, 1, sb.len, f) != sb.len) {
        if (f != stdout) fclose(f);
        sbuf_free(&sb);
        return ERROR_READ_FAULT;
    }
    if (f != stdout) fclose(f);

    sbuf_free(&sb);
    return NO_ERROR;
}

/**
 * @brief Escape a string for insertion into a JSON value.
 *
 * Applies the escape rules of RFC 8259 §7: quotation mark, reverse
 * solidus and control characters below 0x20 are escaped; the short
 * forms \b, \f, \n, \r, \t are used where available; remaining
 * control characters use \u00XX.
 *
 * Size-query convention as for JsonFormat.
 *
 * @param[in]  pszSrc   Source string. Not NULL. May be empty.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonEscapeString(PCSZ pszSrc,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    SBUF sb;

    if (!pszSrc) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    if (sbuf_init(&sb) != 0) return ERROR_NOT_ENOUGH_MEMORY;

    /* Escape without surrounding quotes. */
    {
        static const char hex[] = "0123456789ABCDEF";
        const char *s = pszSrc;
        while (*s) {
            unsigned char c = (unsigned char)*s++;
            switch (c) {
                case '"':  sbuf_puts(&sb, "\\\""); break;
                case '\\': sbuf_puts(&sb, "\\\\"); break;
                case '\b': sbuf_puts(&sb, "\\b"); break;
                case '\f': sbuf_puts(&sb, "\\f"); break;
                case '\n': sbuf_puts(&sb, "\\n"); break;
                case '\r': sbuf_puts(&sb, "\\r"); break;
                case '\t': sbuf_puts(&sb, "\\t"); break;
                default:
                    if (c < 0x20) {
                        char tmp[7];
                        tmp[0] = '\\';
                        tmp[1] = 'u';
                        tmp[2] = '0';
                        tmp[3] = '0';
                        tmp[4] = hex[(c >> 4) & 0x0F];
                        tmp[5] = hex[c & 0x0F];
                        tmp[6] = '\0';
                        sbuf_puts(&sb, tmp);
                    } else {
                        sbuf_putc(&sb, (char)c);
                    }
                    break;
            }
        }
    }

    if (sb.oom) {
        sbuf_free(&sb);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(sb.len + 1);
        sbuf_free(&sb);
        return NO_ERROR;
    }
    if (ulSize < sb.len + 1) {
        if (pulUsed) *pulUsed = (ULONG)(sb.len + 1);
        sbuf_free(&sb);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, sb.buf, sb.len);
    pszBuf[sb.len] = '\0';
    if (pulUsed) *pulUsed = (ULONG)sb.len;

    sbuf_free(&sb);
    return NO_ERROR;
}
