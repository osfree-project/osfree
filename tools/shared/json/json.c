/*!
 *
 * @file json.c
 *
 * @brief Implementation of the JSON module.
 *
 * JSON parsing, building and serialization (C89). Conforms to:
 *   - RFC 8259, "The JavaScript Object Notation (JSON) Data
 *     Interchange Format".
 *   - ECMA-404, "The JSON Data Interchange Syntax".
 *
 * All nodes created during a document's lifetime are tracked in a
 * single list owned by the document, so that JsonClose can release
 * them regardless of whether they were attached to a parent.
 *
 * @todo Split this file into several translation units:
 *       json_parse (scanner), json_build (tree construction),
 *       json_read (accessors), json_out (serialization).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "json.h"

/* ==================================================================
 * Internal structures
 * ================================================================== */

/*!
 * @def JSON_MAX_DEPTH
 * @brief Maximum nesting depth accepted by the parser.
 */
#define JSON_MAX_DEPTH 100

/*!
 * @def JSON_CHILD_INIT
 * @brief Initial capacity of a node's child list.
 */
#define JSON_CHILD_INIT 4

/*!
 * @def JSON_NODE_INIT
 * @brief Initial capacity of a document's node list.
 */
#define JSON_NODE_INIT 16

/*!
 * @brief Forward declaration of a value node.
 */
typedef struct _JSONNODE JSONNODE, *PJSONNODE;

/*!
 * @brief Forward declaration of an open document.
 */
typedef struct _JSONDOC  JSONDOC,  *PJSONDOC;

/*!
 * @struct _JSONNODE
 * @brief One JSON value inside a document.
 *
 * A node may be detached (not attached to any parent) or attached
 * to an object or array. In an object, @c pszKey holds the field
 * name; in an array it is NULL.
 */
struct _JSONNODE {
    JSONTYPE   type;            /*!< Value type.             */
    PSZ        pszKey;          /*!< Field name, or NULL.    */
    PSZ        pszStringValue;  /*!< String value, or NULL.  */
    double     dblNumberValue;  /*!< Numeric value.          */
    BOOL       fBoolValue;      /*!< Boolean value.          */

    PJSONNODE *pChildren;       /*!< Children, or NULL.      */
    ULONG      ulChildCount;    /*!< Used children.          */
    ULONG      ulChildCapacity; /*!< Allocated child slots.  */
};

/*!
 * @struct _JSONDOC
 * @brief One open JSON document.
 *
 * All nodes created for this document are stored in @c pAllNodes
 * so that JsonClose can release them.
 */
struct _JSONDOC {
    PJSONNODE  pRoot;          /*!< Root node, or NULL.        */
    PJSONNODE *pAllNodes;      /*!< All nodes in the document. */
    ULONG      ulNodeCount;    /*!< Used node slots.           */
    ULONG      ulNodeCapacity; /*!< Allocated node slots.      */
};

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Register a node in its document.
 *
 * @param[in,out] pDoc   Document. Not NULL.
 * @param[in]     pNode  Node. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 *
 * @retval 0   Success.
 * @retval -1  Allocation failed.
 */
static int doc_track_node(PJSONDOC pDoc, PJSONNODE pNode) {
    if (pDoc->ulNodeCount >= pDoc->ulNodeCapacity) {
        ULONG ulNewCap = pDoc->ulNodeCapacity ? pDoc->ulNodeCapacity * 2
                                              : JSON_NODE_INIT;
        PJSONNODE *paNew = (PJSONNODE*)realloc(pDoc->pAllNodes,
                                (size_t)ulNewCap * sizeof(PJSONNODE));
        if (!paNew) return -1;
        pDoc->pAllNodes = paNew;
        pDoc->ulNodeCapacity = ulNewCap;
    }
    pDoc->pAllNodes[pDoc->ulNodeCount++] = pNode;
    return 0;
}

/*!
 * @brief Create and register a new node with a given type.
 *
 * @param[in] pDoc   Document. Not NULL.
 * @param[in] type   Value type.
 *
 * @return New node, or NULL on OOM.
 *
 * @retval NULL  Allocation failed.
 */
static PJSONNODE node_new(PJSONDOC pDoc, JSONTYPE type) {
    PJSONNODE pNode = (PJSONNODE)calloc(1, sizeof(JSONNODE));
    if (!pNode) return NULL;
    pNode->type = type;
    pNode->ulChildCapacity = 0;
    pNode->pChildren = NULL;
    pNode->ulChildCount = 0;
    if (doc_track_node(pDoc, pNode) != 0) {
        free(pNode);
        return NULL;
    }
    return pNode;
}

/*!
 * @brief Append a child node to a parent.
 *
 * @param[in,out] pParent  Parent node. Not NULL.
 * @param[in]     pChild   Child node. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 *
 * @retval 0   Success.
 * @retval -1  Allocation failed.
 */
static int node_add_child(PJSONNODE pParent, PJSONNODE pChild) {
    if (pParent->ulChildCount >= pParent->ulChildCapacity) {
        ULONG ulNewCap = pParent->ulChildCapacity
                         ? pParent->ulChildCapacity * 2
                         : JSON_CHILD_INIT;
        PJSONNODE *paNew = (PJSONNODE*)realloc(pParent->pChildren,
                                (size_t)ulNewCap * sizeof(PJSONNODE));
        if (!paNew) return -1;
        pParent->pChildren = paNew;
        pParent->ulChildCapacity = ulNewCap;
    }
    pParent->pChildren[pParent->ulChildCount++] = pChild;
    return 0;
}

/*!
 * @brief Release a document and every node in it.
 *
 * @param[in] pDoc  Document. May be NULL.
 */
static void doc_free(PJSONDOC pDoc) {
    ULONG ulIdx;
    if (!pDoc) return;
    for (ulIdx = 0; ulIdx < pDoc->ulNodeCount; ulIdx++) {
        PJSONNODE pNode = pDoc->pAllNodes[ulIdx];
        if (!pNode) continue;
        free(pNode->pszKey);
        free(pNode->pszStringValue);
        free(pNode->pChildren);
        free(pNode);
    }
    free(pDoc->pAllNodes);
    free(pDoc);
}

/* ==================================================================
 * Parser
 * ================================================================== */

/*!
 * @struct _PARSE
 * @brief Parser cursor.
 */
typedef struct _PARSE {
    PCSZ      pszPos;  /*!< Current position in the input text. */
    PJSONDOC  pDoc;    /*!< Owning document.                    */
} PARSE;

/*!
 * @brief Skip whitespace in the input.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 */
static void skip_whitespace(PARSE *pParser) {
    while (*pParser->pszPos &&
           isspace((unsigned char)*pParser->pszPos))
        pParser->pszPos++;
}

/*!
 * @brief Validate a NUL-terminated UTF-8 string (RFC 3629).
 *
 * Rejects overlong encodings, surrogates and codepoints above
 * U+10FFFF.
 *
 * @param[in] pszStr  String. Not NULL.
 *
 * @return TRUE if valid, FALSE otherwise.
 *
 * @retval TRUE   Valid UTF-8.
 * @retval FALSE  Invalid UTF-8.
 */
static BOOL validate_utf8(PCSZ pszStr) {
    const UCHAR *puchPos = (const UCHAR*)pszStr;
    while (*puchPos) {
        UCHAR uch = *puchPos++;
        if (uch < 0x80) continue;
        if ((uch & 0xE0) == 0xC0) {
            ULONG ulCp;
            if ((*puchPos & 0xC0) != 0x80) return FALSE;
            ulCp = ((ULONG)(uch & 0x1F) << 6) |
                   (ULONG)(*puchPos & 0x3F);
            if (ulCp < 0x80) return FALSE;
            puchPos++;
        } else if ((uch & 0xF0) == 0xE0) {
            ULONG ulCp;
            if ((puchPos[0] & 0xC0) != 0x80) return FALSE;
            if ((puchPos[1] & 0xC0) != 0x80) return FALSE;
            ulCp = ((ULONG)(uch & 0x0F) << 12) |
                   ((ULONG)(puchPos[0] & 0x3F) << 6) |
                   (ULONG)(puchPos[1] & 0x3F);
            if (ulCp < 0x800) return FALSE;
            if (ulCp >= 0xD800 && ulCp <= 0xDFFF) return FALSE;
            puchPos += 2;
        } else if ((uch & 0xF8) == 0xF0) {
            ULONG ulCp;
            if ((puchPos[0] & 0xC0) != 0x80) return FALSE;
            if ((puchPos[1] & 0xC0) != 0x80) return FALSE;
            if ((puchPos[2] & 0xC0) != 0x80) return FALSE;
            ulCp = ((ULONG)(uch & 0x07) << 18) |
                   ((ULONG)(puchPos[0] & 0x3F) << 12) |
                   ((ULONG)(puchPos[1] & 0x3F) << 6) |
                   (ULONG)(puchPos[2] & 0x3F);
            if (ulCp < 0x10000 || ulCp > 0x10FFFF) return FALSE;
            puchPos += 3;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

/*!
 * @brief Encode a Unicode scalar value as UTF-8.
 *
 * @param[in]  ulCp   Codepoint. Must be <= U+10FFFF and not a
 *                    surrogate.
 * @param[out] pszOut Output buffer. Must have room for up to 4
 *                    bytes.
 *
 * @return Number of bytes written (1..4).
 *
 * @retval 1  One-byte sequence written.
 * @retval 2  Two-byte sequence written.
 * @retval 3  Three-byte sequence written.
 * @retval 4  Four-byte sequence written.
 */
static int utf8_encode(ULONG ulCp, PSZ pszOut) {
    if (ulCp < 0x80) {
        pszOut[0] = (CHAR)ulCp;
        return 1;
    }
    if (ulCp < 0x800) {
        pszOut[0] = (CHAR)(0xC0 | (ulCp >> 6));
        pszOut[1] = (CHAR)(0x80 | (ulCp & 0x3F));
        return 2;
    }
    if (ulCp < 0x10000) {
        pszOut[0] = (CHAR)(0xE0 | (ulCp >> 12));
        pszOut[1] = (CHAR)(0x80 | ((ulCp >> 6) & 0x3F));
        pszOut[2] = (CHAR)(0x80 | (ulCp & 0x3F));
        return 3;
    }
    pszOut[0] = (CHAR)(0xF0 | (ulCp >> 18));
    pszOut[1] = (CHAR)(0x80 | ((ulCp >> 12) & 0x3F));
    pszOut[2] = (CHAR)(0x80 | ((ulCp >> 6) & 0x3F));
    pszOut[3] = (CHAR)(0x80 | (ulCp & 0x3F));
    return 4;
}

/*!
 * @brief Number of bytes utf8_encode will write for @p ulCp.
 *
 * @param[in] ulCp  Codepoint.
 *
 * @return 1, 2, 3 or 4.
 *
 * @retval 1  One-byte sequence.
 * @retval 2  Two-byte sequence.
 * @retval 3  Three-byte sequence.
 * @retval 4  Four-byte sequence.
 */
static int utf8_encoded_len(ULONG ulCp) {
    if (ulCp < 0x80) return 1;
    if (ulCp < 0x800) return 2;
    if (ulCp < 0x10000) return 3;
    return 4;
}

/*!
 * @brief Read 4 hex digits.
 *
 * @param[in]  pszPos  Pointer to 4 hex characters. Not NULL.
 * @param[out] pulOut  Receiver. Not NULL.
 *
 * @return TRUE on success, FALSE on malformed input.
 *
 * @retval TRUE   All four characters were hex digits.
 * @retval FALSE  Malformed input.
 */
static BOOL hex4(PCSZ pszPos, PULONG pulOut) {
    int i;
    ULONG ulVal = 0;
    for (i = 0; i < 4; i++) {
        CHAR ch = pszPos[i];
        int d;
        if (ch >= '0' && ch <= '9') d = ch - '0';
        else if (ch >= 'a' && ch <= 'f') d = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') d = ch - 'A' + 10;
        else return FALSE;
        ulVal = (ulVal << 4) | (ULONG)d;
    }
    *pulOut = ulVal;
    return TRUE;
}

/*!
 * @brief Parse a JSON string.
 *
 * @param[in,out] ppszPos  Cursor pointing at the opening '"'. On
 *                         success, advanced past the closing '"'.
 *                         Not NULL.
 *
 * @return malloc'd NUL-terminated string, or NULL on error.
 *
 * @retval NULL  Syntax error or allocation failure.
 */
static PSZ parse_string(PCSZ *ppszPos) {
    PCSZ pszStart;
    PCSZ pszEnd;
    PSZ pszResult;
    PSZ pszQ;
    PCSZ pszS;
    size_t cbLen;

    if (**ppszPos != '"') return NULL;
    (*ppszPos)++;
    pszStart = *ppszPos;
    cbLen = 0;

    while (**ppszPos && **ppszPos != '"') {
        if (**ppszPos == '\\') {
            (*ppszPos)++;
            if (**ppszPos == '\0') return NULL;
            switch (**ppszPos) {
                case 'n': case 't': case 'r': case 'b': case 'f':
                case '"': case '\\': case '/':
                    cbLen += 1;
                    (*ppszPos)++;
                    break;
                case 'u': {
                    ULONG ulCp;
                    (*ppszPos)++;
                    if (!hex4(*ppszPos, &ulCp)) return NULL;
                    (*ppszPos) += 4;
                    if (ulCp >= 0xD800 && ulCp <= 0xDBFF) {
                        ULONG ulLo;
                        if ((*ppszPos)[0] != '\\' ||
                            (*ppszPos)[1] != 'u') return NULL;
                        if (!hex4(*ppszPos + 2, &ulLo)) return NULL;
                        if (ulLo < 0xDC00 || ulLo > 0xDFFF) return NULL;
                        ulCp = 0x10000 + ((ulCp - 0xD800) << 10) +
                               (ulLo - 0xDC00);
                        (*ppszPos) += 6;
                    } else if (ulCp >= 0xDC00 && ulCp <= 0xDFFF) {
                        return NULL;
                    }
                    cbLen += (size_t)utf8_encoded_len(ulCp);
                    break;
                }
                default:
                    return NULL;
            }
        } else {
            cbLen++;
            (*ppszPos)++;
        }
    }
    if (**ppszPos != '"') return NULL;
    (*ppszPos)++;
    pszEnd = *ppszPos - 1;

    pszResult = (PSZ)malloc(cbLen + 1);
    if (!pszResult) return NULL;

    pszQ = pszResult;
    pszS = pszStart;
    while (pszS < pszEnd) {
        if (*pszS == '\\') {
            pszS++;
            switch (*pszS) {
                case 'n': *pszQ++ = '\n'; pszS++; break;
                case 't': *pszQ++ = '\t'; pszS++; break;
                case 'r': *pszQ++ = '\r'; pszS++; break;
                case 'b': *pszQ++ = '\b'; pszS++; break;
                case 'f': *pszQ++ = '\f'; pszS++; break;
                case '"': *pszQ++ = '"'; pszS++; break;
                case '\\': *pszQ++ = '\\'; pszS++; break;
                case '/': *pszQ++ = '/'; pszS++; break;
                case 'u': {
                    ULONG ulCp, ulLo;
                    pszS++;
                    hex4(pszS, &ulCp);
                    pszS += 4;
                    if (ulCp >= 0xD800 && ulCp <= 0xDBFF) {
                        hex4(pszS + 2, &ulLo);
                        ulCp = 0x10000 + ((ulCp - 0xD800) << 10) +
                               (ulLo - 0xDC00);
                        pszS += 6;
                    }
                    pszQ += utf8_encode(ulCp, pszQ);
                    break;
                }
                default:
                    free(pszResult);
                    return NULL;
            }
        } else {
            *pszQ++ = *pszS++;
        }
    }
    *pszQ = '\0';
    return pszResult;
}

/*!
 * @brief Return 10 raised to integer power @p nExp.
 *
 * @param[in] nExp  Exponent.
 *
 * @return 10 ** @p nExp.
 */
static double pow10_int(int nExp) {
    double dResult = 1.0;
    int i;
    if (nExp >= 0) {
        for (i = 0; i < nExp; i++) dResult *= 10.0;
    } else {
        for (i = 0; i > nExp; i--) dResult *= 0.1;
    }
    return dResult;
}

/*!
 * @brief Parse a JSON number (RFC 8259 §6).
 *
 * @param[in,out] ppszPos  Cursor. On success, advanced past the
 *                         number. Not NULL.
 * @param[out]    pdOut    Receiver. Not NULL.
 *
 * @return TRUE on success, FALSE on malformed input.
 *
 * @retval TRUE   Success.
 * @retval FALSE  Malformed number.
 */
static BOOL parse_number(PCSZ *ppszPos, double *pdOut) {
    PCSZ pszPos = *ppszPos;
    PCSZ pszStart = pszPos;
    double dblSign = 1.0;
    double dblVal = 0.0;

    if (*pszPos == '-') { dblSign = -1.0; pszPos++; }

    if (*pszPos == '0') {
        pszPos++;
        if (isdigit((unsigned char)*pszPos)) return FALSE;
    } else if (*pszPos >= '1' && *pszPos <= '9') {
        while (isdigit((unsigned char)*pszPos)) {
            dblVal = dblVal * 10.0 + (double)(*pszPos - '0');
            pszPos++;
        }
    } else {
        return FALSE;
    }

    if (*pszPos == '.') {
        double dblScale = 0.1;
        pszPos++;
        if (!isdigit((unsigned char)*pszPos)) return FALSE;
        while (isdigit((unsigned char)*pszPos)) {
            dblVal += (double)(*pszPos - '0') * dblScale;
            dblScale *= 0.1;
            pszPos++;
        }
    }

    if (*pszPos == 'e' || *pszPos == 'E') {
        int nExpSign = 1;
        int nExpVal = 0;
        pszPos++;
        if (*pszPos == '+') pszPos++;
        else if (*pszPos == '-') { nExpSign = -1; pszPos++; }
        if (!isdigit((unsigned char)*pszPos)) return FALSE;
        while (isdigit((unsigned char)*pszPos)) {
            if (nExpVal < 100000)
                nExpVal = nExpVal * 10 + (*pszPos - '0');
            pszPos++;
        }
        dblVal *= pow10_int(nExpSign * nExpVal);
    }

    if (pszPos == pszStart) return FALSE;
    *ppszPos = pszPos;
    *pdOut = dblSign * dblVal;
    return TRUE;
}

/*!
 * @brief Forward declaration of the value parser.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[in]     nDepth   Current nesting depth.
 * @param[out]    ppOut    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 *
 * @retval 0   Success.
 * @retval -1  Syntax error or allocation failure.
 */
static int parse_value(PARSE *pParser, int nDepth, PJSONNODE *ppOut);

/*!
 * @brief Parse a JSON array.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[in]     nDepth   Current nesting depth.
 * @param[out]    ppOut    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 *
 * @retval 0   Success.
 * @retval -1  Syntax error or allocation failure.
 */
static int parse_array(PARSE *pParser, int nDepth, PJSONNODE *ppOut) {
    PJSONNODE pArr;

    pArr = node_new(pParser->pDoc, JSON_ARRAY);
    if (!pArr) return -1;

    if (*pParser->pszPos != '[') return -1;
    pParser->pszPos++;
    skip_whitespace(pParser);
    if (*pParser->pszPos == ']') {
        pParser->pszPos++;
        *ppOut = pArr;
        return 0;
    }

    for (;;) {
        PJSONNODE pChild = NULL;
        skip_whitespace(pParser);
        if (parse_value(pParser, nDepth, &pChild) != 0) return -1;
        if (node_add_child(pArr, pChild) != 0) return -1;
        skip_whitespace(pParser);
        if (*pParser->pszPos == ',') {
            pParser->pszPos++;
            continue;
        } else if (*pParser->pszPos == ']') {
            pParser->pszPos++;
            break;
        } else {
            return -1;
        }
    }
    *ppOut = pArr;
    return 0;
}

/*!
 * @brief Parse a JSON object.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[in]     nDepth   Current nesting depth.
 * @param[out]    ppOut    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 *
 * @retval 0   Success.
 * @retval -1  Syntax error or allocation failure.
 */
static int parse_object(PARSE *pParser, int nDepth, PJSONNODE *ppOut) {
    PJSONNODE pObj;

    pObj = node_new(pParser->pDoc, JSON_OBJECT);
    if (!pObj) return -1;

    if (*pParser->pszPos != '{') return -1;
    pParser->pszPos++;
    skip_whitespace(pParser);
    if (*pParser->pszPos == '}') {
        pParser->pszPos++;
        *ppOut = pObj;
        return 0;
    }

    for (;;) {
        PSZ pszKey;
        PJSONNODE pValue = NULL;

        skip_whitespace(pParser);
        if (*pParser->pszPos != '"') return -1;
        pszKey = parse_string(&pParser->pszPos);
        if (!pszKey) return -1;
        skip_whitespace(pParser);
        if (*pParser->pszPos != ':') {
            free(pszKey);
            return -1;
        }
        pParser->pszPos++;
        if (parse_value(pParser, nDepth, &pValue) != 0) {
            free(pszKey);
            return -1;
        }
        pValue->pszKey = pszKey;
        if (node_add_child(pObj, pValue) != 0) return -1;

        skip_whitespace(pParser);
        if (*pParser->pszPos == ',') {
            pParser->pszPos++;
            continue;
        } else if (*pParser->pszPos == '}') {
            pParser->pszPos++;
            break;
        } else {
            return -1;
        }
    }
    *ppOut = pObj;
    return 0;
}

/*!
 * @brief Parse one JSON value.
 *
 * Dispatches on the current character and recurses into the parser
 * for objects and arrays.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[in]     nDepth   Current nesting depth.
 * @param[out]    ppOut    Receiver. Not NULL.
 *
 * @return 0 on success, -1 on syntax or OOM error.
 *
 * @retval 0   Success.
 * @retval -1  Syntax error or allocation failure.
 */
static int parse_value(PARSE *pParser, int nDepth, PJSONNODE *ppOut) {
    PJSONNODE pNode;

    if (nDepth > JSON_MAX_DEPTH) return -1;
    skip_whitespace(pParser);

    switch (*pParser->pszPos) {
        case '{': return parse_object(pParser, nDepth + 1, ppOut);
        case '[': return parse_array(pParser, nDepth + 1, ppOut);
        case '"': {
            PSZ pszStr = parse_string(&pParser->pszPos);
            if (!pszStr) return -1;
            pNode = node_new(pParser->pDoc, JSON_STRING);
            if (!pNode) { free(pszStr); return -1; }
            pNode->pszStringValue = pszStr;
            *ppOut = pNode;
            return 0;
        }
        case 't':
            if (strncmp(pParser->pszPos, "true", 4) == 0) {
                pParser->pszPos += 4;
                pNode = node_new(pParser->pDoc, JSON_BOOLEAN);
                if (!pNode) return -1;
                pNode->fBoolValue = TRUE;
                *ppOut = pNode;
                return 0;
            }
            return -1;
        case 'f':
            if (strncmp(pParser->pszPos, "false", 5) == 0) {
                pParser->pszPos += 5;
                pNode = node_new(pParser->pDoc, JSON_BOOLEAN);
                if (!pNode) return -1;
                pNode->fBoolValue = FALSE;
                *ppOut = pNode;
                return 0;
            }
            return -1;
        case 'n':
            if (strncmp(pParser->pszPos, "null", 4) == 0) {
                pParser->pszPos += 4;
                pNode = node_new(pParser->pDoc, JSON_NULL);
                if (!pNode) return -1;
                *ppOut = pNode;
                return 0;
            }
            return -1;
        default:
            if (*pParser->pszPos == '-' ||
                isdigit((unsigned char)*pParser->pszPos)) {
                double dblVal;
                if (!parse_number(&pParser->pszPos, &dblVal)) return -1;
                pNode = node_new(pParser->pDoc, JSON_NUMBER);
                if (!pNode) return -1;
                pNode->dblNumberValue = dblVal;
                *ppOut = pNode;
                return 0;
            }
            return -1;
    }
}

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/*!
 * @brief Parse a NUL-terminated JSON text into a document.
 *
 * On success, @p *phDoc receives a document handle. The input
 * buffer is only read; it is not retained. The caller owns the
 * buffer.
 *
 * @param[in]  pszText  JSON text. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszText or phDoc is NULL.
 * @retval ERROR_INVALID_DATA       Malformed JSON, invalid UTF-8,
 *                                  or trailing bytes after the value.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 */
APIRET APIENTRY JsonParse(PCSZ pszText, HJSONDOC *phDoc) {
    PCSZ pszPos = pszText;
    PARSE parser;
    PJSONDOC pDoc;
    PJSONNODE pRoot = NULL;

    if (!pszText || !phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;

    /* Skip UTF-8 BOM (RFC 8259 §8.1: implementations MAY ignore) */
    if ((UCHAR)pszPos[0] == 0xEF &&
        (UCHAR)pszPos[1] == 0xBB &&
        (UCHAR)pszPos[2] == 0xBF) {
        pszPos += 3;
    }

    if (!validate_utf8(pszPos)) return ERROR_INVALID_DATA;

    pDoc = (PJSONDOC)calloc(1, sizeof(JSONDOC));
    if (!pDoc) return ERROR_NOT_ENOUGH_MEMORY;

    parser.pszPos = pszPos;
    parser.pDoc = pDoc;

    if (parse_value(&parser, 0, &pRoot) != 0) {
        doc_free(pDoc);
        return ERROR_INVALID_DATA;
    }

    skip_whitespace(&parser);
    if (*parser.pszPos != '\0') {
        doc_free(pDoc);
        return ERROR_INVALID_DATA;
    }

    pDoc->pRoot = pRoot;
    *phDoc = (HJSONDOC)pDoc;
    return NO_ERROR;
}

/*!
 * @brief Release a document and all associated nodes.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
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

/*!
 * @brief Create an empty document for building a tree.
 *
 * Nodes created with JsonNew* belong to the document and are
 * released by JsonClose. Before serialization the caller must
 * designate a root node with JsonSetRoot.
 *
 * @param[out] phDoc  Handle receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewDoc(HJSONDOC *phDoc) {
    PJSONDOC pDoc;

    if (!phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;

    pDoc = (PJSONDOC)calloc(1, sizeof(JSONDOC));
    if (!pDoc) return ERROR_NOT_ENOUGH_MEMORY;

    *phDoc = (HJSONDOC)pDoc;
    return NO_ERROR;
}

/*!
 * @brief Designate the root node of a document.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hNode  Root node. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or hNode is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY JsonSetRoot(HJSONDOC hDoc, HJSONNODE hNode) {
    PJSONDOC pDoc;
    if (hDoc == NULLHANDLE || hNode == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    pDoc = (PJSONDOC)hDoc;
    pDoc->pRoot = (PJSONNODE)hNode;
    return NO_ERROR;
}

/*!
 * @brief Create a new object node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewObject(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONNODE pNode;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pNode = node_new((PJSONDOC)hDoc, JSON_OBJECT);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;
    *phNode = (HJSONNODE)pNode;
    return NO_ERROR;
}

/*!
 * @brief Create a new array node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewArray(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONNODE pNode;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pNode = node_new((PJSONDOC)hDoc, JSON_ARRAY);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;
    *phNode = (HJSONNODE)pNode;
    return NO_ERROR;
}

/*!
 * @brief Create a new string node.
 *
 * The value is copied into the node.
 *
 * @param[in]  hDoc      Document handle. Not NULLHANDLE.
 * @param[in]  pszValue  String value. Not NULL.
 * @param[out] phNode    Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE, pszValue or
 *                                  phNode is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewString(HJSONDOC hDoc, PCSZ pszValue,
                              HJSONNODE *phNode) {
    PJSONNODE pNode;
    PSZ pszCopy;
    if (hDoc == NULLHANDLE || !pszValue || !phNode)
        return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pszCopy = strdup(pszValue);
    if (!pszCopy) return ERROR_NOT_ENOUGH_MEMORY;
    pNode = node_new((PJSONDOC)hDoc, JSON_STRING);
    if (!pNode) { free(pszCopy); return ERROR_NOT_ENOUGH_MEMORY; }
    pNode->pszStringValue = pszCopy;
    *phNode = (HJSONNODE)pNode;
    return NO_ERROR;
}

/*!
 * @brief Create a new number node.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  dValue  Numeric value.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewNumber(HJSONDOC hDoc, double dValue,
                              HJSONNODE *phNode) {
    PJSONNODE pNode;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pNode = node_new((PJSONDOC)hDoc, JSON_NUMBER);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;
    pNode->dblNumberValue = dValue;
    *phNode = (HJSONNODE)pNode;
    return NO_ERROR;
}

/*!
 * @brief Create a new boolean node.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  fValue  TRUE or FALSE.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewBoolean(HJSONDOC hDoc, BOOL fValue,
                               HJSONNODE *phNode) {
    PJSONNODE pNode;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pNode = node_new((PJSONDOC)hDoc, JSON_BOOLEAN);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;
    pNode->fBoolValue = fValue ? TRUE : FALSE;
    *phNode = (HJSONNODE)pNode;
    return NO_ERROR;
}

/*!
 * @brief Create a new null node.
 *
 * @param[in]  hDoc   Document handle. Not NULLHANDLE.
 * @param[out] phNode Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc is NULLHANDLE or phNode is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNewNull(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONNODE pNode;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    pNode = node_new((PJSONDOC)hDoc, JSON_NULL);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;
    *phNode = (HJSONNODE)pNode;
    return NO_ERROR;
}

/*!
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
 *
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
    PSZ pszCopy;

    if (hObj == NULLHANDLE || !pszKey || hValue == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    pObj = (PJSONNODE)hObj;
    pVal = (PJSONNODE)hValue;
    if (pObj->type != JSON_OBJECT) return ERROR_INVALID_DATA;
    pszCopy = strdup(pszKey);
    if (!pszCopy) return ERROR_NOT_ENOUGH_MEMORY;
    pVal->pszKey = pszCopy;
    if (node_add_child(pObj, pVal) != 0) {
        free(pszCopy);
        pVal->pszKey = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/*!
 * @brief Append a value to an array.
 *
 * @param[in] hArr    Array handle. Not NULLHANDLE.
 * @param[in] hValue  Value node. Not NULLHANDLE.
 *
 * @return APIRET
 *
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

/*!
 * @brief Obtain the root node of a parsed document.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[out] phNode  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phNode is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Document has no root.
 */
APIRET APIENTRY JsonRoot(HJSONDOC hDoc, HJSONNODE *phNode) {
    PJSONDOC pDoc;
    if (hDoc == NULLHANDLE || !phNode) return ERROR_INVALID_PARAMETER;
    pDoc = (PJSONDOC)hDoc;
    if (!pDoc->pRoot) return ERROR_FILE_NOT_FOUND;
    *phNode = (HJSONNODE)pDoc->pRoot;
    return NO_ERROR;
}

/*!
 * @brief Query the type of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of a JSONTYPE value. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode or pulType is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY JsonNodeGetType(HJSONNODE hNode, PULONG pulType) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pulType) return ERROR_INVALID_PARAMETER;
    *pulType = (ULONG)pNode->type;
    return NO_ERROR;
}

/*!
 * @brief Obtain a child of an object node by key.
 *
 * @param[in]  hNode    Node handle (object). Not NULLHANDLE.
 * @param[in]  pszKey   Key. Not NULL.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an object.
 * @retval ERROR_FILE_NOT_FOUND     Key not present.
 */
APIRET APIENTRY JsonNodeGetChild(HJSONNODE hNode, PCSZ pszKey,
                                 HJSONNODE *phChild) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    ULONG ulIdx;

    if (hNode == NULLHANDLE || !pszKey || !phChild)
        return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pNode->type != JSON_OBJECT) return ERROR_INVALID_DATA;

    for (ulIdx = 0; ulIdx < pNode->ulChildCount; ulIdx++) {
        if (pNode->pChildren[ulIdx]->pszKey &&
            strcmp(pNode->pChildren[ulIdx]->pszKey, pszKey) == 0) {
            *phChild = (HJSONNODE)pNode->pChildren[ulIdx];
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Query the number of elements in an array node or entries
 *        in an object node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is neither array nor object.
 */
APIRET APIENTRY JsonNodeGetCount(HJSONNODE hNode, PULONG pulCount) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    if (pNode->type != JSON_ARRAY && pNode->type != JSON_OBJECT)
        return ERROR_INVALID_DATA;
    *pulCount = pNode->ulChildCount;
    return NO_ERROR;
}

/*!
 * @brief Obtain an element of an array node by index.
 *
 * @param[in]  hNode    Node handle (array). Not NULLHANDLE.
 * @param[in]  ulIndex  Element index.
 * @param[out] phChild  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not an array.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY JsonNodeGetElement(HJSONNODE hNode, ULONG ulIndex,
                                   HJSONNODE *phChild) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !phChild) return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pNode->type != JSON_ARRAY) return ERROR_INVALID_DATA;
    if (ulIndex >= pNode->ulChildCount) return ERROR_NO_MORE_ITEMS;
    *phChild = (HJSONNODE)pNode->pChildren[ulIndex];
    return NO_ERROR;
}

/*!
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
 *
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
    PJSONNODE pNode = (PJSONNODE)hNode;
    PCSZ pszKey;
    size_t cbKeyLen;

    if (hNode == NULLHANDLE || !pszKeyBuf || !phChild)
        return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pNode->type != JSON_OBJECT) return ERROR_INVALID_DATA;
    if (ulIndex >= pNode->ulChildCount) return ERROR_NO_MORE_ITEMS;

    pszKey = pNode->pChildren[ulIndex]->pszKey;
    if (!pszKey) pszKey = "";
    cbKeyLen = strlen(pszKey);

    if (ulKeySize < cbKeyLen + 1) {
        if (pulKeyUsed) *pulKeyUsed = (ULONG)cbKeyLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszKeyBuf, pszKey, cbKeyLen);
    pszKeyBuf[cbKeyLen] = '\0';
    if (pulKeyUsed) *pulKeyUsed = (ULONG)cbKeyLen;
    *phChild = (HJSONNODE)pNode->pChildren[ulIndex];
    return NO_ERROR;
}

/*!
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
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonNodeGetString(HJSONNODE hNode,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    PCSZ pszStr;
    size_t cbLen;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (pNode->type != JSON_STRING) return ERROR_INVALID_DATA;

    pszStr = pNode->pszStringValue ? pNode->pszStringValue : "";
    cbLen = strlen(pszStr);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszStr, cbLen);
    pszBuf[cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/*!
 * @brief Read a boolean value of a node.
 *
 * @param[in]  hNode    Node handle (boolean). Not NULLHANDLE.
 * @param[out] pfValue  Receiver TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a boolean.
 */
APIRET APIENTRY JsonNodeGetBoolean(HJSONNODE hNode, PBOOL pfValue) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pfValue) return ERROR_INVALID_PARAMETER;
    if (pNode->type != JSON_BOOLEAN) return ERROR_INVALID_DATA;
    *pfValue = pNode->fBoolValue;
    return NO_ERROR;
}

/*!
 * @brief Read a number value of a node.
 *
 * @param[in]  hNode    Node handle (number). Not NULLHANDLE.
 * @param[out] pdValue  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a number.
 */
APIRET APIENTRY JsonNodeGetNumber(HJSONNODE hNode, double *pdValue) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    if (hNode == NULLHANDLE || !pdValue) return ERROR_INVALID_PARAMETER;
    if (pNode->type != JSON_NUMBER) return ERROR_INVALID_DATA;
    *pdValue = pNode->dblNumberValue;
    return NO_ERROR;
}

/* ==================================================================
 * Node modification and cloning
 * ================================================================== */

/*!
 * @brief Retrieve the key of a node.
 *
 * For a child of an object, this is the field name. For a child of
 * an array, or for the root node, the key is empty.
 *
 * Size-query convention as for JsonNodeGetString.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY JsonNodeGetKey(HJSONNODE hNode,
                               PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    PCSZ pszKey;
    size_t cbLen;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    pszKey = pNode->pszKey ? pNode->pszKey : "";
    cbLen = strlen(pszKey);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszKey, cbLen);
    pszBuf[cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/*!
 * @brief Set the string value of an existing string node.
 *
 * The node must be of type JSON_STRING. The new value is copied
 * into the node.
 *
 * @param[in] hNode    Node handle (string). Not NULLHANDLE.
 * @param[in] pszVal   New value. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE, or pszVal is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonNodeSetValueString(HJSONNODE hNode, PCSZ pszVal) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    PSZ pszCopy;

    if (hNode == NULLHANDLE || !pszVal) return ERROR_INVALID_PARAMETER;
    if (pNode->type != JSON_STRING) return ERROR_INVALID_DATA;

    pszCopy = strdup(pszVal);
    if (!pszCopy) return ERROR_NOT_ENOUGH_MEMORY;

    free(pNode->pszStringValue);
    pNode->pszStringValue = pszCopy;
    return NO_ERROR;
}

/*!
 * @brief Set or replace a string field on an object.
 *
 * If the object already has a child with the given key and that
 * child is a string, its value is replaced. Otherwise a new string
 * child is appended.
 *
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] pszVal  Field value. Not NULL.
 *
 * @return APIRET
 *
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
    ULONG ulIdx;

    if (hDoc == NULLHANDLE || hObj == NULLHANDLE ||
        !pszKey || !pszVal)
        return ERROR_INVALID_PARAMETER;
    if (pObj->type != JSON_OBJECT) return ERROR_INVALID_DATA;

    for (ulIdx = 0; ulIdx < pObj->ulChildCount; ulIdx++) {
        PJSONNODE pChild = pObj->pChildren[ulIdx];
        if (pChild->pszKey && strcmp(pChild->pszKey, pszKey) == 0) {
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

/*!
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
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonCloneNode(HJSONDOC hDst, HJSONNODE hSrc,
                              HJSONNODE *phDst) {
    PJSONDOC  pDocDst = (PJSONDOC)hDst;
    PJSONNODE pNodeSrc  = (PJSONNODE)hSrc;
    PJSONNODE pNodeDst;
    ULONG ulIdx;

    if (hDst == NULLHANDLE || hSrc == NULLHANDLE || !phDst)
        return ERROR_INVALID_PARAMETER;
    *phDst = NULLHANDLE;

    pNodeDst = node_new(pDocDst, pNodeSrc->type);
    if (!pNodeDst) return ERROR_NOT_ENOUGH_MEMORY;

    if (pNodeSrc->pszKey) {
        pNodeDst->pszKey = strdup(pNodeSrc->pszKey);
        if (!pNodeDst->pszKey) return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (pNodeSrc->pszStringValue) {
        pNodeDst->pszStringValue = strdup(pNodeSrc->pszStringValue);
        if (!pNodeDst->pszStringValue) return ERROR_NOT_ENOUGH_MEMORY;
    }
    pNodeDst->dblNumberValue = pNodeSrc->dblNumberValue;
    pNodeDst->fBoolValue = pNodeSrc->fBoolValue;

    for (ulIdx = 0; ulIdx < pNodeSrc->ulChildCount; ulIdx++) {
        HJSONNODE hChildClone = NULLHANDLE;
        APIRET rc = JsonCloneNode(hDst, (HJSONNODE)pNodeSrc->pChildren[ulIdx],
                                  &hChildClone);
        if (rc != NO_ERROR) return rc;
        if (node_add_child(pNodeDst, (PJSONNODE)hChildClone) != 0)
            return ERROR_NOT_ENOUGH_MEMORY;
    }

    *phDst = (HJSONNODE)pNodeDst;
    return NO_ERROR;
}

/* ==================================================================
 * Serialization
 * ================================================================== */

/*!
 * @struct _SBUF
 * @brief Growable string buffer used during serialization.
 */
typedef struct _SBUF {
    PSZ    pszBuf;  /*!< Backing storage, or NULL.      */
    size_t cbCap;   /*!< Allocated bytes.               */
    size_t cbLen;   /*!< Used bytes, excluding NUL.     */
    int    nOom;    /*!< Non-zero once an OOM occurred. */
} SBUF;

/*!
 * @brief Initialize a string buffer.
 *
 * @param[out] pBuf  Buffer. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 *
 * @retval 0   Success.
 * @retval -1  Allocation failed.
 */
static int sbuf_init(SBUF *pBuf) {
    pBuf->cbCap = 64;
    pBuf->cbLen = 0;
    pBuf->nOom = 0;
    pBuf->pszBuf = (PSZ)malloc(pBuf->cbCap);
    if (!pBuf->pszBuf) { pBuf->nOom = 1; return -1; }
    pBuf->pszBuf[0] = '\0';
    return 0;
}

/*!
 * @brief Release a string buffer.
 *
 * @param[in,out] pBuf  Buffer. Not NULL.
 */
static void sbuf_free(SBUF *pBuf) {
    free(pBuf->pszBuf);
    pBuf->pszBuf = NULL;
    pBuf->cbCap = 0;
    pBuf->cbLen = 0;
}

/*!
 * @brief Append bytes to a string buffer.
 *
 * @param[in,out] pBuf     Buffer. Not NULL.
 * @param[in]     pszData  Bytes to append. Not NULL.
 * @param[in]     cbLen    Number of bytes.
 */
static void sbuf_put(SBUF *pBuf, PCSZ pszData, size_t cbLen) {
    if (pBuf->nOom) return;
    if (pBuf->cbLen + cbLen + 1 > pBuf->cbCap) {
        size_t cbNewCap = pBuf->cbCap * 2 + cbLen + 64;
        PSZ pszNew = (PSZ)realloc(pBuf->pszBuf, cbNewCap);
        if (!pszNew) { pBuf->nOom = 1; return; }
        pBuf->pszBuf = pszNew;
        pBuf->cbCap = cbNewCap;
    }
    memcpy(pBuf->pszBuf + pBuf->cbLen, pszData, cbLen);
    pBuf->cbLen += cbLen;
    pBuf->pszBuf[pBuf->cbLen] = '\0';
}

/*!
 * @brief Append one character to a string buffer.
 *
 * @param[in,out] pBuf  Buffer. Not NULL.
 * @param[in]     ch    Character.
 */
static void sbuf_putc(SBUF *pBuf, CHAR ch) {
    sbuf_put(pBuf, &ch, 1);
}

/*!
 * @brief Append a NUL-terminated string to a string buffer.
 *
 * @param[in,out] pBuf    Buffer. Not NULL.
 * @param[in]     pszStr  String. Not NULL.
 */
static void sbuf_puts(SBUF *pBuf, PCSZ pszStr) {
    sbuf_put(pBuf, pszStr, strlen(pszStr));
}

/*!
 * @brief Append a JSON-quoted and escaped string.
 *
 * @param[in,out] pBuf    Buffer. Not NULL.
 * @param[in]     pszStr  String value. May be NULL (treated as "").
 */
static void sbuf_put_escaped(SBUF *pBuf, PCSZ pszStr) {
    static PCSZ pszHex = "0123456789ABCDEF";
    if (!pszStr) pszStr = "";
    sbuf_putc(pBuf, '"');
    while (*pszStr) {
        UCHAR uch = (UCHAR)*pszStr++;
        switch (uch) {
            case '"':  sbuf_puts(pBuf, "\\\""); break;
            case '\\': sbuf_puts(pBuf, "\\\\"); break;
            case '\b': sbuf_puts(pBuf, "\\b"); break;
            case '\f': sbuf_puts(pBuf, "\\f"); break;
            case '\n': sbuf_puts(pBuf, "\\n"); break;
            case '\r': sbuf_puts(pBuf, "\\r"); break;
            case '\t': sbuf_puts(pBuf, "\\t"); break;
            default:
                if (uch < 0x20) {
                    CHAR achTmp[7];
                    achTmp[0] = '\\';
                    achTmp[1] = 'u';
                    achTmp[2] = '0';
                    achTmp[3] = '0';
                    achTmp[4] = pszHex[(uch >> 4) & 0x0F];
                    achTmp[5] = pszHex[uch & 0x0F];
                    achTmp[6] = '\0';
                    sbuf_puts(pBuf, achTmp);
                } else {
                    sbuf_putc(pBuf, (CHAR)uch);
                }
                break;
        }
    }
    sbuf_putc(pBuf, '"');
}

/*!
 * @brief Append a JSON number to a string buffer.
 *
 * @param[in,out] pBuf    Buffer. Not NULL.
 * @param[in]     dblVal  Numeric value.
 */
static void sbuf_put_number(SBUF *pBuf, double dblVal) {
    CHAR achTmp[64];
    sprintf(achTmp, "%g", dblVal);
    sbuf_puts(pBuf, achTmp);
}

/*!
 * @brief Append a newline and indentation to a string buffer.
 *
 * @param[in,out] pBuf    Buffer. Not NULL.
 * @param[in]     nIndent Nesting depth.
 */
static void sbuf_indent(SBUF *pBuf, int nIndent) {
    int i;
    sbuf_putc(pBuf, '\n');
    for (i = 0; i < nIndent * 2; i++) sbuf_putc(pBuf, ' ');
}

/*!
 * @brief Serialize a node into a string buffer.
 *
 * @param[in,out] pBuf     Buffer. Not NULL.
 * @param[in]     pNode    Node. Not NULL.
 * @param[in]     fIndent  Non-zero for pretty-printed output.
 * @param[in]     nDepth   Current nesting depth.
 */
static void sbuf_write_node(SBUF *pBuf, PJSONNODE pNode,
                            int fIndent, int nDepth) {
    ULONG ulIdx;
    if (pBuf->nOom) return;

    switch (pNode->type) {
        case JSON_NULL:
            sbuf_puts(pBuf, "null");
            break;
        case JSON_BOOLEAN:
            sbuf_puts(pBuf, pNode->fBoolValue ? "true" : "false");
            break;
        case JSON_NUMBER:
            sbuf_put_number(pBuf, pNode->dblNumberValue);
            break;
        case JSON_STRING:
            sbuf_put_escaped(pBuf, pNode->pszStringValue);
            break;
        case JSON_ARRAY:
            sbuf_putc(pBuf, '[');
            for (ulIdx = 0; ulIdx < pNode->ulChildCount; ulIdx++) {
                if (ulIdx > 0) sbuf_putc(pBuf, ',');
                if (fIndent) sbuf_indent(pBuf, nDepth + 1);
                sbuf_write_node(pBuf, pNode->pChildren[ulIdx],
                                fIndent, nDepth + 1);
            }
            if (fIndent && pNode->ulChildCount > 0)
                sbuf_indent(pBuf, nDepth);
            sbuf_putc(pBuf, ']');
            break;
        case JSON_OBJECT:
            sbuf_putc(pBuf, '{');
            for (ulIdx = 0; ulIdx < pNode->ulChildCount; ulIdx++) {
                if (ulIdx > 0) sbuf_putc(pBuf, ',');
                if (fIndent) sbuf_indent(pBuf, nDepth + 1);
                sbuf_put_escaped(pBuf, pNode->pChildren[ulIdx]->pszKey);
                sbuf_putc(pBuf, ':');
                if (fIndent) sbuf_putc(pBuf, ' ');
                sbuf_write_node(pBuf, pNode->pChildren[ulIdx],
                                fIndent, nDepth + 1);
            }
            if (fIndent && pNode->ulChildCount > 0)
                sbuf_indent(pBuf, nDepth);
            sbuf_putc(pBuf, '}');
            break;
    }
}

/*!
 * @brief Serialize a node into a caller-supplied buffer.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: text copied and NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[in]  fIndent   TRUE for pretty-printed output with
 *                       two-space indentation; FALSE for compact
 *                       output.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonFormat(HJSONNODE hNode, BOOL fIndent,
                           PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    SBUF buf;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    if (sbuf_init(&buf) != 0) return ERROR_NOT_ENOUGH_MEMORY;

    sbuf_write_node(&buf, pNode, fIndent ? 1 : 0, 0);
    if (buf.nOom) {
        sbuf_free(&buf);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)buf.cbLen + 1;
        sbuf_free(&buf);
        return NO_ERROR;
    }
    if (ulSize < buf.cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)buf.cbLen + 1;
        sbuf_free(&buf);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, buf.pszBuf, buf.cbLen);
    pszBuf[buf.cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)buf.cbLen;

    sbuf_free(&buf);
    return NO_ERROR;
}

/*!
 * @brief Serialize a node into a file or to stdout.
 *
 * When @p pszPath is NULL, the output is written to stdout.
 *
 * @param[in] hNode    Node handle. Not NULLHANDLE.
 * @param[in] fIndent  TRUE for pretty-printed output; FALSE for
 *                     compact output.
 * @param[in] pszPath  Output file path, or NULL for stdout.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_OPEN_FAILED        Cannot open output file.
 * @retval ERROR_READ_FAULT         Write error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonWriteFile(HJSONNODE hNode, BOOL fIndent,
                              PCSZ pszPath) {
    PJSONNODE pNode = (PJSONNODE)hNode;
    SBUF buf;
    FILE *fp;

    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    if (sbuf_init(&buf) != 0) return ERROR_NOT_ENOUGH_MEMORY;
    sbuf_write_node(&buf, pNode, fIndent ? 1 : 0, 0);
    if (buf.nOom) {
        sbuf_free(&buf);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pszPath) {
        fp = fopen(pszPath, "wb");
        if (!fp) { sbuf_free(&buf); return ERROR_OPEN_FAILED; }
    } else {
        fp = stdout;
    }

    if (fwrite(buf.pszBuf, 1, buf.cbLen, fp) != buf.cbLen) {
        if (fp != stdout) fclose(fp);
        sbuf_free(&buf);
        return ERROR_READ_FAULT;
    }
    if (fp != stdout) fclose(fp);

    sbuf_free(&buf);
    return NO_ERROR;
}

/*!
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
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY JsonEscapeString(PCSZ pszSrc,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    SBUF buf;

    if (!pszSrc) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    if (sbuf_init(&buf) != 0) return ERROR_NOT_ENOUGH_MEMORY;

    /* Escape without surrounding quotes. */
    {
        static PCSZ pszHex = "0123456789ABCDEF";
        PCSZ pszPos = pszSrc;
        while (*pszPos) {
            UCHAR uch = (UCHAR)*pszPos++;
            switch (uch) {
                case '"':  sbuf_puts(&buf, "\\\""); break;
                case '\\': sbuf_puts(&buf, "\\\\"); break;
                case '\b': sbuf_puts(&buf, "\\b"); break;
                case '\f': sbuf_puts(&buf, "\\f"); break;
                case '\n': sbuf_puts(&buf, "\\n"); break;
                case '\r': sbuf_puts(&buf, "\\r"); break;
                case '\t': sbuf_puts(&buf, "\\t"); break;
                default:
                    if (uch < 0x20) {
                        CHAR achTmp[7];
                        achTmp[0] = '\\';
                        achTmp[1] = 'u';
                        achTmp[2] = '0';
                        achTmp[3] = '0';
                        achTmp[4] = pszHex[(uch >> 4) & 0x0F];
                        achTmp[5] = pszHex[uch & 0x0F];
                        achTmp[6] = '\0';
                        sbuf_puts(&buf, achTmp);
                    } else {
                        sbuf_putc(&buf, (CHAR)uch);
                    }
                    break;
            }
        }
    }

    if (buf.nOom) {
        sbuf_free(&buf);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)buf.cbLen + 1;
        sbuf_free(&buf);
        return NO_ERROR;
    }
    if (ulSize < buf.cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)buf.cbLen + 1;
        sbuf_free(&buf);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, buf.pszBuf, buf.cbLen);
    pszBuf[buf.cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)buf.cbLen;

    sbuf_free(&buf);
    return NO_ERROR;
}
