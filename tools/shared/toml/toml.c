/* toml.c - TOML v1.0.0 parser (C89 + Watcom extensions) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include "toml.h"
#include "toml_internal.h"

/**
 * @file toml.c
 * @brief Implementation of the TOML v1.0.0 parser.
 *
 * Conforms to:
 *   - https://toml.io/en/v1.0.0
 *   - https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
 */

/* ==================================================================
 * Parser state
 * ================================================================== */

/**
 * @struct _PARSE
 * @brief Parser cursor.
 */
typedef struct _PARSE {
    PCSZ pszPos;    /**< Current position.          */
    PCSZ pszEnd;    /**< End of input.              */
    PCSZ pszError;  /**< Static error description.  */
} PARSE;

/* ==================================================================
 * Small helpers
 * ================================================================== */

/**
 * @brief Duplicate a byte range with a terminating NUL.
 *
 * @param[in] pszSrc  Source bytes. Not NULL.
 * @param[in] cbLen   Number of bytes.
 *
 * @return malloc'd string, or NULL on OOM.
 */
static PSZ dup_n(PCSZ pszSrc, size_t cbLen) {
    PSZ pszOut;
    pszOut = (PSZ)malloc(cbLen + 1);
    if (!pszOut) return NULL;
    memcpy(pszOut, pszSrc, cbLen);
    pszOut[cbLen] = '\0';
    return pszOut;
}

/**
 * @brief Record a static error description.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[in]     pszMsg   Message. Not NULL.
 */
static void set_error(PARSE *pParser, PCSZ pszMsg) {
    if (pParser->pszError == NULL) pParser->pszError = pszMsg;
}

/**
 * @brief Skip spaces and tabs.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 */
static void skip_ws(PARSE *pParser) {
    while (pParser->pszPos < pParser->pszEnd &&
           (*pParser->pszPos == ' ' || *pParser->pszPos == '\t'))
        pParser->pszPos++;
}

/**
 * @brief Query whether a character is an ASCII digit.
 *
 * @param[in] c  Character.
 *
 * @return 1 if digit, 0 otherwise.
 */
static int is_digit_c(int c) {
    return c >= '0' && c <= '9';
}

/**
 * @brief Query whether a character may appear in a bare key.
 *
 * @param[in] c  Character.
 *
 * @return 1 if allowed, 0 otherwise.
 */
static int is_bare_key_char(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-';
}

/**
 * @brief Skip a comment starting at '#'.
 *
 * Comment body may contain any character except control characters
 * below 0x20 (other than tab) and 0x7F.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int skip_comment(PARSE *pParser) {
    if (pParser->pszPos >= pParser->pszEnd ||
        *pParser->pszPos != '#') return 0;
    pParser->pszPos++;
    while (pParser->pszPos < pParser->pszEnd) {
        UCHAR uch = (UCHAR)*pParser->pszPos;
        if (uch == '\n' || uch == '\r') return 0;
        if (uch < 0x20 && uch != '\t') return -1;
        if (uch == 0x7F) return -1;
        pParser->pszPos++;
    }
    return 0;
}

/**
 * @brief Consume one newline: '\n' or '\r\n'.
 *
 * A lone '\r' is an error.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int consume_newline(PARSE *pParser) {
    if (pParser->pszPos < pParser->pszEnd && *pParser->pszPos == '\r') {
        if (pParser->pszPos + 1 >= pParser->pszEnd ||
            pParser->pszPos[1] != '\n') return -1;
        pParser->pszPos += 2;
        return 0;
    }
    if (pParser->pszPos < pParser->pszEnd && *pParser->pszPos == '\n') {
        pParser->pszPos++;
        return 0;
    }
    return -1;
}

/**
 * @brief Skip whitespace, newlines and comments.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int skip_ws_nl_comments(PARSE *pParser) {
    for (;;) {
        skip_ws(pParser);
        if (pParser->pszPos < pParser->pszEnd &&
            *pParser->pszPos == '#') {
            if (skip_comment(pParser) != 0) return -1;
            continue;
        }
        if (pParser->pszPos < pParser->pszEnd &&
            (*pParser->pszPos == '\n' || *pParser->pszPos == '\r')) {
            if (consume_newline(pParser) != 0) return -1;
            continue;
        }
        break;
    }
    return 0;
}

/**
 * @brief Skip whitespace and an optional comment, then require a
 *        newline or end of input.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int skip_to_eol(PARSE *pParser) {
    skip_ws(pParser);
    if (pParser->pszPos < pParser->pszEnd &&
        *pParser->pszPos == '#') {
        if (skip_comment(pParser) != 0) return -1;
    }
    if (pParser->pszPos >= pParser->pszEnd) return 0;
    return consume_newline(pParser);
}

/* ==================================================================
 * IEEE 754 helpers
 * ================================================================== */

/**
 * @brief Return positive or negative infinity.
 *
 * @param[in] fNegative  TRUE_ for -inf, FALSE_ for +inf.
 *
 * @return The corresponding double value.
 */
static double toml_inf(int fNegative) {
    return fNegative ? -HUGE_VAL : HUGE_VAL;
}

/**
 * @brief Return a NaN value.
 *
 * @return A quiet NaN.
 */
static double toml_nan(void) {
    union {
        UCHAR b[sizeof(double)];
        double d;
    } u;
    size_t i;
    for (i = 0; i < sizeof(double); i++) u.b[i] = 0;
    if (sizeof(double) == 8) {
        u.b[7] = 0x7F; u.b[6] = 0xF8; u.b[0] = 0x01;
    } else if (sizeof(double) == 4) {
        u.b[3] = 0x7F; u.b[2] = 0xC0; u.b[0] = 0x01;
    }
    return u.d;
}

/* ==================================================================
 * UTF-8 validation (RFC 3629)
 * ================================================================== */

/**
 * @brief Validate a NUL-terminated UTF-8 string.
 *
 * @param[in] pszStr  String. Not NULL.
 *
 * @return 1 if valid, 0 otherwise.
 */
static int valid_utf8(PCSZ pszStr) {
    const UCHAR *puchPos;
    puchPos = (const UCHAR*)pszStr;
    while (*puchPos) {
        UCHAR uch = *puchPos++;
        ULONG ulCp;
        if (uch < 0x80) continue;
        if ((uch & 0xE0) == 0xC0) {
            if ((*puchPos & 0xC0) != 0x80) return 0;
            ulCp = ((ULONG)(uch & 0x1F) << 6) |
                   (ULONG)(*puchPos & 0x3F);
            if (ulCp < 0x80) return 0;
            puchPos++;
        } else if ((uch & 0xF0) == 0xE0) {
            if ((puchPos[0] & 0xC0) != 0x80) return 0;
            if ((puchPos[1] & 0xC0) != 0x80) return 0;
            ulCp = ((ULONG)(uch & 0x0F) << 12) |
                   ((ULONG)(puchPos[0] & 0x3F) << 6) |
                   (ULONG)(puchPos[1] & 0x3F);
            if (ulCp < 0x800) return 0;
            if (ulCp >= 0xD800 && ulCp <= 0xDFFF) return 0;
            puchPos += 2;
        } else if ((uch & 0xF8) == 0xF0) {
            if ((puchPos[0] & 0xC0) != 0x80) return 0;
            if ((puchPos[1] & 0xC0) != 0x80) return 0;
            if ((puchPos[2] & 0xC0) != 0x80) return 0;
            ulCp = ((ULONG)(uch & 0x07) << 18) |
                   ((ULONG)(puchPos[0] & 0x3F) << 12) |
                   ((ULONG)(puchPos[1] & 0x3F) << 6) |
                   (ULONG)(puchPos[2] & 0x3F);
            if (ulCp < 0x10000 || ulCp > 0x10FFFF) return 0;
            puchPos += 3;
        } else {
            return 0;
        }
    }
    return 1;
}

/* ==================================================================
 * Tree primitives
 * ================================================================== */

/**
 * @brief Create a new value node of the given type.
 *
 * @param[in] ulType  One of TOML_TYPE_*.
 *
 * @return New value, or NULL on OOM.
 */
static PTOMLVALUE value_new(ULONG ulType) {
    PTOMLVALUE pValue;
    pValue = (PTOMLVALUE)calloc(1, sizeof(TOMLVALUE));
    if (pValue) pValue->ulType = ulType;
    return pValue;
}

/**
 * @brief Create a new empty table.
 *
 * @return New table, or NULL on OOM.
 */
static PTOMLTABLE table_new(void) {
    PTOMLTABLE pTable;
    pTable = (PTOMLTABLE)calloc(1, sizeof(TOMLTABLE));
    if (!pTable) return NULL;
    if (VectorCreate((ULONG)sizeof(TOMLENTRY), &pTable->hEntries)
            != NO_ERROR) {
        free(pTable);
        return NULL;
    }
    return pTable;
}

/**
 * @brief Create a new empty array.
 *
 * @return New array, or NULL on OOM.
 */
static PTOMLARRAY array_new(void) {
    return (PTOMLARRAY)calloc(1, sizeof(TOMLARRAY));
}

static void value_free(PTOMLVALUE pValue);
static void table_free(PTOMLTABLE pTable);
static void array_free(PTOMLARRAY pArray);

/**
 * @brief Release an array and all its items.
 *
 * @param[in] pArray  Array. May be NULL.
 */
static void array_free(PTOMLARRAY pArray) {
    ULONG ulIdx;
    if (!pArray) return;
    for (ulIdx = 0; ulIdx < pArray->ulCount; ulIdx++)
        value_free(pArray->paItems[ulIdx]);
    free(pArray->paItems);
    free(pArray);
}

/**
 * @brief Release a table and all its entries.
 *
 * @param[in] pTable  Table. May be NULL.
 */
static void table_free(PTOMLTABLE pTable) {
    ULONG ulCount = 0;
    ULONG ulIdx;
    if (!pTable) return;
    if (pTable->hEntries != NULLHANDLE) {
        if (VectorGetCount(pTable->hEntries, &ulCount) == NO_ERROR) {
            for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
                TOMLENTRY entry;
                if (VectorGetItem(pTable->hEntries, ulIdx, &entry,
                                  (ULONG)sizeof(entry), NULL)
                        == NO_ERROR) {
                    free(entry.pszKey);
                    value_free(entry.pValue);
                }
            }
        }
        VectorDestroy(pTable->hEntries);
    }
    free(pTable);
}

/**
 * @brief Release a value and its owned resources.
 *
 * @param[in] pValue  Value. May be NULL.
 */
static void value_free(PTOMLVALUE pValue) {
    if (!pValue) return;
    switch (pValue->ulType) {
        case TOML_TYPE_STRING:
        case TOML_TYPE_DATETIME: free(pValue->u.pszString); break;
        case TOML_TYPE_ARRAY:    array_free(pValue->u.pArray); break;
        case TOML_TYPE_TABLE:    table_free(pValue->u.pTable); break;
        default: break;
    }
    free(pValue);
}

/**
 * @brief Release a tree built by TomlInternalParse.
 *
 * @param[in] pRoot  Root table. May be NULL.
 */
void TomlInternalFreeTree(PTOMLTABLE pRoot) {
    table_free(pRoot);
}

/**
 * @brief Grow an array's backing storage if needed.
 *
 * @param[in,out] pArray  Array. Not NULL.
 *
 * @return 0 on success, -1 on OOM.
 */
static int array_grow(PTOMLARRAY pArray) {
    ULONG ulNewCap;
    PTOMLVALUE *paNew;
    if (pArray->ulCount < pArray->ulCapacity) return 0;
    ulNewCap = pArray->ulCapacity ? pArray->ulCapacity * 2 : 8;
    paNew = (PTOMLVALUE*)realloc(pArray->paItems,
                                 (size_t)ulNewCap * sizeof(PTOMLVALUE));
    if (!paNew) return -1;
    pArray->paItems = paNew;
    pArray->ulCapacity = ulNewCap;
    return 0;
}

/**
 * @brief Find a table entry by key.
 *
 * @param[in]  pTable  Table. Not NULL.
 * @param[in]  pszKey  Key. Not NULL.
 * @param[out] pEntry  Optional. May be NULL. Receives a copy of the
 *                     entry on success.
 *
 * @return 1 if found, 0 otherwise.
 */
static int table_find_copy(PTOMLTABLE pTable, PCSZ pszKey,
                           PTOMLENTRY pEntry) {
    ULONG ulCount = 0;
    ULONG ulIdx;
    if (!pTable || pTable->hEntries == NULLHANDLE) return 0;
    if (VectorGetCount(pTable->hEntries, &ulCount) != NO_ERROR) return 0;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        TOMLENTRY entry;
        if (VectorGetItem(pTable->hEntries, ulIdx, &entry,
                          (ULONG)sizeof(entry), NULL) == NO_ERROR) {
            if (strcmp(entry.pszKey, pszKey) == 0) {
                if (pEntry) *pEntry = entry;
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Find a value in a table by key.
 *
 * @param[in] pTable  Table. Not NULL.
 * @param[in] pszKey  Key. Not NULL.
 *
 * @return Pointer to the value, or NULL if not found.
 */
static PTOMLVALUE table_find_value(PTOMLTABLE pTable, PCSZ pszKey) {
    TOMLENTRY entry;
    if (table_find_copy(pTable, pszKey, &entry)) return entry.pValue;
    return NULL;
}

/**
 * @brief Query whether a table contains a key.
 *
 * @param[in] pTable  Table. Not NULL.
 * @param[in] pszKey  Key. Not NULL.
 *
 * @return 1 if present, 0 otherwise.
 */
static int table_has_key(PTOMLTABLE pTable, PCSZ pszKey) {
    return table_find_copy(pTable, pszKey, NULL);
}

/**
 * @brief Append a key/value pair to a table.
 *
 * @param[in] pTable  Table. Not NULL.
 * @param[in] pszKey  Key. Ownership transfers to the table.
 * @param[in] pValue  Value. Ownership transfers to the table.
 *
 * @return 0 on success, -1 on OOM.
 */
static int table_add(PTOMLTABLE pTable, PSZ pszKey, PTOMLVALUE pValue) {
    TOMLENTRY entry;
    entry.pszKey = pszKey;
    entry.pValue = pValue;
    if (VectorAdd(pTable->hEntries, &entry) != NO_ERROR) return -1;
    return 0;
}

/**
 * @brief Append a value to an array.
 *
 * @param[in] pArray  Array. Not NULL.
 * @param[in] pValue  Value. Ownership transfers to the array.
 *
 * @return 0 on success, -1 on OOM.
 */
static int array_add(PTOMLARRAY pArray, PTOMLVALUE pValue) {
    if (array_grow(pArray) != 0) return -1;
    pArray->paItems[pArray->ulCount++] = pValue;
    return 0;
}

/**
 * @brief Wrap a table in a value node.
 *
 * @param[in] pTable  Table. Not NULL.
 *
 * @return New value, or NULL on OOM.
 */
static PTOMLVALUE value_table(PTOMLTABLE pTable) {
    PTOMLVALUE pValue = value_new(TOML_TYPE_TABLE);
    if (!pValue) return NULL;
    pValue->u.pTable = pTable;
    return pValue;
}

/**
 * @brief Wrap an array in a value node.
 *
 * @param[in] pArray  Array. Not NULL.
 *
 * @return New value, or NULL on OOM.
 */
static PTOMLVALUE value_array(PTOMLARRAY pArray) {
    PTOMLVALUE pValue = value_new(TOML_TYPE_ARRAY);
    if (!pValue) return NULL;
    pValue->u.pArray = pArray;
    return pValue;
}

/* ==================================================================
 * String parsing
 * ================================================================== */

/**
 * @brief Return the value of a single hex digit.
 *
 * @param[in] c  Character.
 *
 * @return Value in [0, 15], or -1 if not a hex digit.
 */
static int hex_digit(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/**
 * @brief Encode a Unicode scalar value as UTF-8.
 *
 * @param[in]  ulCp    Codepoint.
 * @param[out] pszOut  Output buffer. Must have room for up to 4 bytes.
 *
 * @return Number of bytes written (1..4), or 0 on invalid input.
 */
static int utf8_emit(ULONG ulCp, PSZ pszOut) {
    if (ulCp < 0x80) { pszOut[0] = (CHAR)ulCp; return 1; }
    if (ulCp < 0x800) {
        pszOut[0] = (CHAR)(0xC0 | (ulCp >> 6));
        pszOut[1] = (CHAR)(0x80 | (ulCp & 0x3F));
        return 2;
    }
    if (ulCp < 0x10000) {
        if (ulCp >= 0xD800 && ulCp <= 0xDFFF) return 0;
        pszOut[0] = (CHAR)(0xE0 | (ulCp >> 12));
        pszOut[1] = (CHAR)(0x80 | ((ulCp >> 6) & 0x3F));
        pszOut[2] = (CHAR)(0x80 | (ulCp & 0x3F));
        return 3;
    }
    if (ulCp <= 0x10FFFF) {
        pszOut[0] = (CHAR)(0xF0 | (ulCp >> 18));
        pszOut[1] = (CHAR)(0x80 | ((ulCp >> 12) & 0x3F));
        pszOut[2] = (CHAR)(0x80 | ((ulCp >> 6) & 0x3F));
        pszOut[3] = (CHAR)(0x80 | (ulCp & 0x3F));
        return 4;
    }
    return 0;
}

/**
 * @brief Append bytes to a growable buffer.
 *
 * @param[in,out] ppszBuf  Pointer to the buffer pointer. Not NULL.
 * @param[in,out] pcbCap   Pointer to the capacity. Not NULL.
 * @param[in,out] pcbLen   Pointer to the used length. Not NULL.
 * @param[in]     pszData  Bytes to append. Not NULL.
 * @param[in]     cbLen    Number of bytes.
 *
 * @return 0 on success, -1 on OOM.
 */
static int sbuf_put(PSZ *ppszBuf, size_t *pcbCap, size_t *pcbLen,
                    PCSZ pszData, size_t cbLen) {
    if (*pcbLen + cbLen + 1 > *pcbCap) {
        size_t cbNewCap = *pcbCap * 2 + cbLen + 64;
        PSZ pszNew = (PSZ)realloc(*ppszBuf, cbNewCap);
        if (!pszNew) return -1;
        *ppszBuf = pszNew;
        *pcbCap = cbNewCap;
    }
    memcpy(*ppszBuf + *pcbLen, pszData, cbLen);
    *pcbLen += cbLen;
    (*ppszBuf)[*pcbLen] = '\0';
    return 0;
}

/**
 * @brief Append one byte to a growable buffer.
 *
 * @param[in,out] ppszBuf  Pointer to the buffer pointer. Not NULL.
 * @param[in,out] pcbCap   Pointer to the capacity. Not NULL.
 * @param[in,out] pcbLen   Pointer to the used length. Not NULL.
 * @param[in]     ch       Byte.
 *
 * @return 0 on success, -1 on OOM.
 */
static int sbuf_putc(PSZ *ppszBuf, size_t *pcbCap, size_t *pcbLen,
                     CHAR ch) {
    return sbuf_put(ppszBuf, pcbCap, pcbLen, &ch, 1);
}

/**
 * @brief Parse a \uXXXX or \UXXXXXXXX escape.
 *
 * @param[in,out] pParser   Parser at the 'u' or 'U'. Not NULL.
 * @param[out]    pszUbuf   Output buffer. Must have room for 4 bytes.
 * @param[out]    pnBytes   Number of bytes written.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_hex_escape(PARSE *pParser, PSZ pszUbuf, int *pnBytes) {
    int nHex = (*pParser->pszPos == 'u') ? 4 : 8;
    ULONG ulCp = 0;
    int i;
    pParser->pszPos++;
    for (i = 0; i < nHex; i++) {
        int d;
        if (pParser->pszPos >= pParser->pszEnd) return -1;
        d = hex_digit((UCHAR)*pParser->pszPos);
        if (d < 0) return -1;
        ulCp = (ulCp << 4) | (ULONG)d;
        pParser->pszPos++;
    }
    *pnBytes = utf8_emit(ulCp, pszUbuf);
    return (*pnBytes == 0) ? -1 : 0;
}

/**
 * @brief Parse a basic string: "..." .
 *
 * @param[in,out] pParser  Parser at the opening '"'. Not NULL.
 *
 * @return malloc'd string, or NULL on error.
 */
static PSZ parse_basic_string(PARSE *pParser) {
    size_t cbCap = 64, cbLen = 0;
    PSZ pszBuf = (PSZ)malloc(cbCap);
    if (!pszBuf) return NULL;
    pszBuf[0] = '\0';
    pParser->pszPos++;

    while (pParser->pszPos < pParser->pszEnd &&
           *pParser->pszPos != '"') {
        CHAR ch = *pParser->pszPos;
        if (ch == '\\') {
            pParser->pszPos++;
            if (pParser->pszPos >= pParser->pszEnd) goto fail;
            switch (*pParser->pszPos) {
                case 'b': ch = '\b'; pParser->pszPos++; break;
                case 't': ch = '\t'; pParser->pszPos++; break;
                case 'n': ch = '\n'; pParser->pszPos++; break;
                case 'f': ch = '\f'; pParser->pszPos++; break;
                case 'r': ch = '\r'; pParser->pszPos++; break;
                case '"': ch = '"';  pParser->pszPos++; break;
                case '\\': ch = '\\'; pParser->pszPos++; break;
                case 'u': case 'U': {
                    CHAR achUbuf[4]; int nBytes;
                    if (parse_hex_escape(pParser, achUbuf,
                                         &nBytes) != 0) goto fail;
                    if (sbuf_put(&pszBuf, &cbCap, &cbLen, achUbuf,
                                 (size_t)nBytes) != 0)
                        goto fail;
                    continue;
                }
                default: goto fail;
            }
        } else if ((UCHAR)ch < 0x20 && ch != '\t') {
            goto fail;
        } else if ((UCHAR)ch == 0x7F) {
            goto fail;
        } else {
            pParser->pszPos++;
        }
        if (sbuf_putc(&pszBuf, &cbCap, &cbLen, ch) != 0) goto fail;
    }
    if (pParser->pszPos >= pParser->pszEnd ||
        *pParser->pszPos != '"') goto fail;
    pParser->pszPos++;
    return pszBuf;
fail:
    free(pszBuf);
    return NULL;
}

/**
 * @brief Parse a multi-line basic string: """...""" .
 *
 * @param[in,out] pParser  Parser at the opening '"""'. Not NULL.
 *
 * @return malloc'd string, or NULL on error.
 */
static PSZ parse_multiline_basic_string(PARSE *pParser) {
    size_t cbCap = 64, cbLen = 0;
    PSZ pszBuf = (PSZ)malloc(cbCap);
    int fDone = 0;
    if (!pszBuf) return NULL;
    pszBuf[0] = '\0';
    pParser->pszPos += 3;
    if (pParser->pszPos < pParser->pszEnd &&
        *pParser->pszPos == '\r' &&
        pParser->pszPos + 1 < pParser->pszEnd &&
        pParser->pszPos[1] == '\n') {
        pParser->pszPos += 2;
    } else if (pParser->pszPos < pParser->pszEnd &&
               *pParser->pszPos == '\n') {
        pParser->pszPos++;
    }

    while (!fDone && pParser->pszPos < pParser->pszEnd) {
        CHAR ch = *pParser->pszPos;

        if (ch == '"') {
            PCSZ pszRunStart = pParser->pszPos;
            int n = 0;
            int k;
            while (pParser->pszPos < pParser->pszEnd &&
                   *pParser->pszPos == '"') { n++; pParser->pszPos++; }
            if (n >= 3 && n <= 5) {
                for (k = 0; k < n - 3; k++) {
                    if (sbuf_putc(&pszBuf, &cbCap, &cbLen, '"') != 0)
                        goto fail;
                }
                fDone = 1;
                continue;
            }
            if (n > 5) {
                (void)pszRunStart;
                goto fail;
            }
            for (k = 0; k < n; k++) {
                if (sbuf_putc(&pszBuf, &cbCap, &cbLen, '"') != 0)
                    goto fail;
            }
            continue;
        }

        if (ch == '\\') {
            PCSZ pszQ = pParser->pszPos + 1;
            while (pszQ < pParser->pszEnd && (*pszQ == ' ' ||
                                              *pszQ == '\t')) pszQ++;
            if (pszQ < pParser->pszEnd &&
                (*pszQ == '\n' || *pszQ == '\r')) {
                pParser->pszPos = pszQ;
                if (consume_newline(pParser) != 0) goto fail;
                while (pParser->pszPos < pParser->pszEnd) {
                    CHAR chW = *pParser->pszPos;
                    if (chW == ' ' || chW == '\t') {
                        pParser->pszPos++; continue;
                    }
                    if (chW == '\r' || chW == '\n') {
                        if (consume_newline(pParser) != 0) goto fail;
                        continue;
                    }
                    break;
                }
                continue;
            }
            pParser->pszPos++;
            if (pParser->pszPos >= pParser->pszEnd) goto fail;
            switch (*pParser->pszPos) {
                case 'b': ch = '\b'; pParser->pszPos++; break;
                case 't': ch = '\t'; pParser->pszPos++; break;
                case 'n': ch = '\n'; pParser->pszPos++; break;
                case 'f': ch = '\f'; pParser->pszPos++; break;
                case 'r': ch = '\r'; pParser->pszPos++; break;
                case '"': ch = '"';  pParser->pszPos++; break;
                case '\\': ch = '\\'; pParser->pszPos++; break;
                case 'u': case 'U': {
                    CHAR achUbuf[4]; int nBytes;
                    if (parse_hex_escape(pParser, achUbuf,
                                         &nBytes) != 0) goto fail;
                    if (sbuf_put(&pszBuf, &cbCap, &cbLen, achUbuf,
                                 (size_t)nBytes) != 0)
                        goto fail;
                    continue;
                }
                default: goto fail;
            }
        } else if ((UCHAR)ch < 0x20 &&
                   ch != '\t' && ch != '\n' && ch != '\r') {
            goto fail;
        } else if ((UCHAR)ch == 0x7F) {
            goto fail;
        } else {
            pParser->pszPos++;
        }
        if (sbuf_putc(&pszBuf, &cbCap, &cbLen, ch) != 0) goto fail;
    }
    if (!fDone) goto fail;
    return pszBuf;
fail:
    free(pszBuf);
    return NULL;
}

/**
 * @brief Parse a literal string: '...' .
 *
 * @param[in,out] pParser  Parser at the opening '\''. Not NULL.
 *
 * @return malloc'd string, or NULL on error.
 */
static PSZ parse_literal_string(PARSE *pParser) {
    PCSZ pszStart;
    pParser->pszPos++;
    pszStart = pParser->pszPos;
    while (pParser->pszPos < pParser->pszEnd &&
           *pParser->pszPos != '\'' &&
           *pParser->pszPos != '\n' &&
           *pParser->pszPos != '\r') {
        UCHAR uch = (UCHAR)*pParser->pszPos;
        if (uch < 0x20 && uch != '\t') return NULL;
        if (uch == 0x7F) return NULL;
        pParser->pszPos++;
    }
    if (pParser->pszPos >= pParser->pszEnd ||
        *pParser->pszPos != '\'') return NULL;
    {
        PSZ pszResult = dup_n(pszStart,
                              (size_t)(pParser->pszPos - pszStart));
        pParser->pszPos++;
        return pszResult;
    }
}

/**
 * @brief Parse a multi-line literal string: '''...''' .
 *
 * @param[in,out] pParser  Parser at the opening "'''". Not NULL.
 *
 * @return malloc'd string, or NULL on error.
 */
static PSZ parse_multiline_literal_string(PARSE *pParser) {
    PCSZ pszStart;
    PCSZ pszContentEnd = NULL;
    pParser->pszPos += 3;
    if (pParser->pszPos < pParser->pszEnd &&
        *pParser->pszPos == '\r' &&
        pParser->pszPos + 1 < pParser->pszEnd &&
        pParser->pszPos[1] == '\n') {
        pParser->pszPos += 2;
    } else if (pParser->pszPos < pParser->pszEnd &&
               *pParser->pszPos == '\n') {
        pParser->pszPos++;
    }
    pszStart = pParser->pszPos;

    while (pParser->pszPos < pParser->pszEnd) {
        UCHAR uch = (UCHAR)*pParser->pszPos;
        if (uch == '\'') {
            PCSZ pszRunStart = pParser->pszPos;
            int n = 0;
            while (pParser->pszPos < pParser->pszEnd &&
                   *pParser->pszPos == '\'') { n++; pParser->pszPos++; }
            if (n >= 3 && n <= 5) {
                pszContentEnd = pszRunStart + (n - 3);
                break;
            }
            if (n > 5) return NULL;
            continue;
        }
        if (uch < 0x20 && uch != '\t' && uch != '\n' && uch != '\r')
            return NULL;
        if (uch == 0x7F) return NULL;
        pParser->pszPos++;
    }
    if (!pszContentEnd) return NULL;
    {
        PCSZ pszQ;
        for (pszQ = pszStart; pszQ < pszContentEnd; pszQ++) {
            UCHAR uchCc = (UCHAR)*pszQ;
            if (uchCc < 0x20 && uchCc != '\t' &&
                uchCc != '\n' && uchCc != '\r')
                return NULL;
            if (uchCc == 0x7F) return NULL;
        }
    }
    return dup_n(pszStart, (size_t)(pszContentEnd - pszStart));
}

/* ==================================================================
 * Date/time validation (TOML v1.0.0 grammar)
 * ================================================================== */

/**
 * @brief Parse two decimal digits into an integer value.
 *
 * @param[in] pszPos  Pointer to two digits. Not NULL.
 *
 * @return Value in [0, 99].
 */
static int parse_2digit(PCSZ pszPos) {
    return (pszPos[0] - '0') * 10 + (pszPos[1] - '0');
}

/**
 * @brief Days in a month, taking leap years into account.
 *
 * @param[in] nYear   Full year.
 * @param[in] nMonth  Month, 1-based.
 *
 * @return Number of days in the month, or 0 on invalid month.
 */
static int days_in_month(int nYear, int nMonth) {
    static const int anDim[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (nMonth < 1 || nMonth > 12) return 0;
    if (nMonth == 2) {
        int fLeap = (nYear % 4 == 0 &&
                     (nYear % 100 != 0 || nYear % 400 == 0));
        return fLeap ? 29 : 28;
    }
    return anDim[nMonth];
}

/**
 * @brief Validate a date/time token against the TOML v1.0.0 grammar
 *        (RFC 3339 subset).
 *
 * @param[in] pszStr  Token. Not NULL.
 * @param[in] cbLen   Token length.
 *
 * @return 1 if valid, 0 otherwise.
 */
static int validate_datetime(PCSZ pszStr, size_t cbLen) {
    size_t cbPos = 0;
    int fHaveDate = 0, fHaveTime = 0;
    int nYear = 0, nMonth = 0, nDay = 0;

    if (cbLen >= 10 &&
        is_digit_c(pszStr[0]) && is_digit_c(pszStr[1]) &&
        is_digit_c(pszStr[2]) && is_digit_c(pszStr[3]) &&
        pszStr[4] == '-' &&
        is_digit_c(pszStr[5]) && is_digit_c(pszStr[6]) &&
        pszStr[7] == '-' &&
        is_digit_c(pszStr[8]) && is_digit_c(pszStr[9])) {
        nYear = (pszStr[0]-'0')*1000 + (pszStr[1]-'0')*100 +
                (pszStr[2]-'0')*10 + (pszStr[3]-'0');
        nMonth = parse_2digit(pszStr + 5);
        nDay = parse_2digit(pszStr + 8);
        if (nMonth < 1 || nMonth > 12) return 0;
        if (nDay < 1 || nDay > days_in_month(nYear, nMonth)) return 0;
        fHaveDate = 1;
        cbPos = 10;
    }

    if (cbLen - cbPos >= 8 &&
        is_digit_c(pszStr[cbPos+0]) && is_digit_c(pszStr[cbPos+1]) &&
        pszStr[cbPos+2] == ':' &&
        is_digit_c(pszStr[cbPos+3]) && is_digit_c(pszStr[cbPos+4]) &&
        pszStr[cbPos+5] == ':' &&
        is_digit_c(pszStr[cbPos+6]) && is_digit_c(pszStr[cbPos+7])) {
        int nHour, nMinute, nSecond;
        if (fHaveDate) {
            if (pszStr[cbPos] != 'T' && pszStr[cbPos] != 't' &&
                pszStr[cbPos] != ' ') return 0;
            cbPos++;
        }
        nHour   = parse_2digit(pszStr + cbPos);
        nMinute = parse_2digit(pszStr + cbPos + 3);
        nSecond = parse_2digit(pszStr + cbPos + 6);
        if (nHour > 23 || nMinute > 59 || nSecond > 60) return 0;
        fHaveTime = 1;
        cbPos += 8;
        if (cbPos < cbLen && pszStr[cbPos] == '.') {
            cbPos++;
            if (cbPos >= cbLen || !is_digit_c(pszStr[cbPos])) return 0;
            while (cbPos < cbLen && is_digit_c(pszStr[cbPos])) cbPos++;
        }
        if (cbPos < cbLen) {
            if (pszStr[cbPos] == 'Z' || pszStr[cbPos] == 'z') {
                cbPos++;
            } else if (pszStr[cbPos] == '+' || pszStr[cbPos] == '-') {
                if (cbLen - cbPos < 6) return 0;
                if (!is_digit_c(pszStr[cbPos+1]) ||
                    !is_digit_c(pszStr[cbPos+2]) ||
                    pszStr[cbPos+3] != ':' ||
                    !is_digit_c(pszStr[cbPos+4]) ||
                    !is_digit_c(pszStr[cbPos+5]))
                    return 0;
                {
                    int nOh = parse_2digit(pszStr + cbPos + 1);
                    int nOm = parse_2digit(pszStr + cbPos + 4);
                    if (nOh > 23 || nOm > 59) return 0;
                }
                cbPos += 6;
            }
        }
    }
    return cbPos == cbLen && (fHaveDate || fHaveTime);
}

/**
 * @brief Quick syntactic check: does the token look like a date or a
 *        time?
 *
 * @param[in] pszPos  Start of the token. Not NULL.
 * @param[in] pszEnd  End of the input. Not NULL.
 *
 * @return 1 if it looks like a date or time, 0 otherwise.
 */
static int looks_like_datetime(PCSZ pszPos, PCSZ pszEnd) {
    if (pszPos + 5 <= pszEnd &&
        is_digit_c(pszPos[0]) && is_digit_c(pszPos[1]) &&
        is_digit_c(pszPos[2]) && is_digit_c(pszPos[3]) &&
        pszPos[4] == '-') return 1;
    if (pszPos + 3 <= pszEnd &&
        is_digit_c(pszPos[0]) && is_digit_c(pszPos[1]) &&
        pszPos[2] == ':') return 1;
    return 0;
}

/* ==================================================================
 * Number parsing
 * ================================================================== */

/**
 * @brief Return the value of a digit in the given base.
 *
 * @param[in] c      Character.
 * @param[in] nBase  Base (2, 8, 10 or 16).
 *
 * @return Value, or -1 if not a valid digit in that base.
 */
static int digit_value(int c, int nBase) {
    if (c >= '0' && c <= '9') {
        int d = c - '0';
        if (d < nBase) return d;
        return -1;
    }
    if (nBase == 16 && c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (nBase == 16 && c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/**
 * @brief Parse an integer token.
 *
 * @param[in]  pszStart  Start of the token. Not NULL.
 * @param[in]  pszEnd    End of the token. Not NULL.
 * @param[out] pllOut    Receiver. Not NULL.
 *
 * @return 1 on success, 0 on syntax error.
 */
static int parse_integer_token(PCSZ pszStart, PCSZ pszEnd,
                               PLONGLONG pllOut) {
    int nBase = 10;
    unsigned __int64 ullVal = 0;
    int nSign = 1;
    int fHadSign = 0;
    PCSZ pszPos = pszStart;
    int fAny = 0;
    int fPrevWasDigit = 0;
    unsigned __int64 ullLimit;
    int fIsRadix = 0;

    if (pszPos < pszEnd && (*pszPos == '+' || *pszPos == '-')) {
        if (*pszPos == '-') nSign = -1;
        fHadSign = 1;
        pszPos++;
    }

    if (pszPos + 1 < pszEnd && *pszPos == '0') {
        CHAR ch = pszPos[1];
        if (ch == 'x')      { nBase = 16; fIsRadix = 1; pszPos += 2; }
        else if (ch == 'o') { nBase = 8;  fIsRadix = 1; pszPos += 2; }
        else if (ch == 'b') { nBase = 2;  fIsRadix = 1; pszPos += 2; }
    }
    if (fIsRadix && fHadSign) return 0;

    if (!fIsRadix) {
        PCSZ pszQ = pszPos;
        int nFirst = -1;
        int nCount = 0;
        while (pszQ < pszEnd) {
            if (*pszQ == '_') { pszQ++; continue; }
            if (*pszQ >= '0' && *pszQ <= '9') {
                if (nFirst < 0) nFirst = *pszQ - '0';
                nCount++;
            }
            pszQ++;
        }
        if (nFirst == 0 && nCount > 1) return 0;
    }

    if (nSign < 0) {
        ullLimit = ((unsigned __int64)1) << 63;
    } else {
        ullLimit = (((unsigned __int64)1) << 63) - 1;
    }

    while (pszPos < pszEnd) {
        int d;
        if (*pszPos == '_') {
            if (!fPrevWasDigit) return 0;
            if (pszPos + 1 >= pszEnd) return 0;
            if (digit_value((UCHAR)pszPos[1], nBase) < 0) return 0;
            fPrevWasDigit = 0;
            pszPos++;
            continue;
        }
        d = digit_value((UCHAR)*pszPos, nBase);
        if (d < 0) return 0;
        if (ullVal > (ullLimit - (unsigned __int64)d) /
                     (unsigned __int64)nBase)
            return 0;
        ullVal = ullVal * nBase + (unsigned __int64)d;
        fAny = 1;
        fPrevWasDigit = 1;
        pszPos++;
    }
    if (!fAny) return 0;
    if (!fPrevWasDigit) return 0;

    if (nSign < 0) {
        *pllOut = (LONGLONG)(0 - ullVal);
    } else {
        *pllOut = (LONGLONG)ullVal;
    }
    return 1;
}

/**
 * @brief Query whether the cursor starts a number token.
 *
 * @param[in] pszPos  Start. Not NULL.
 * @param[in] pszEnd  End of the input. Not NULL.
 *
 * @return 1 if so, 0 otherwise.
 */
static int is_number_token_start(PCSZ pszPos, PCSZ pszEnd) {
    if (pszPos >= pszEnd) return 0;
    if (*pszPos == '+' || *pszPos == '-') return 1;
    if (is_digit_c(*pszPos)) return 1;
    if (pszPos + 3 <= pszEnd) {
        if (strncmp(pszPos, "inf", 3) == 0) return 1;
        if (strncmp(pszPos, "nan", 3) == 0) return 1;
    }
    return 0;
}

/**
 * @brief Validate a float token (underscores already stripped).
 *
 * @param[in] pszStr  Token. Not NULL.
 *
 * @return 1 if valid, 0 otherwise.
 */
static int validate_float_clean(PCSZ pszStr) {
    size_t i = 0, cbLen = strlen(pszStr);
    int fSawDot = 0, fSawExp = 0;

    if (i < cbLen && (pszStr[i] == '+' || pszStr[i] == '-')) i++;
    if (i >= cbLen) return 0;

    if (pszStr[i] == '0') {
        i++;
        if (i < cbLen && pszStr[i] >= '0' && pszStr[i] <= '9') return 0;
    } else if (pszStr[i] >= '1' && pszStr[i] <= '9') {
        i++;
        while (i < cbLen && pszStr[i] >= '0' && pszStr[i] <= '9') i++;
    } else {
        return 0;
    }

    if (i < cbLen && pszStr[i] == '.') {
        i++;
        if (i >= cbLen || !(pszStr[i] >= '0' && pszStr[i] <= '9')) return 0;
        i++;
        while (i < cbLen && pszStr[i] >= '0' && pszStr[i] <= '9') i++;
        fSawDot = 1;
    }

    if (i < cbLen && (pszStr[i] == 'e' || pszStr[i] == 'E')) {
        i++;
        if (i < cbLen && (pszStr[i] == '+' || pszStr[i] == '-')) i++;
        if (i >= cbLen || !(pszStr[i] >= '0' && pszStr[i] <= '9')) return 0;
        i++;
        while (i < cbLen && pszStr[i] >= '0' && pszStr[i] <= '9') i++;
        fSawExp = 1;
    }

    if (i != cbLen) return 0;
    if (!fSawDot && !fSawExp) return 0;
    return 1;
}

/**
 * @brief Parse a number or date/time token.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[out]    ppValue  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_number_or_datetime(PARSE *pParser, PTOMLVALUE *ppValue) {
    PCSZ pszStart = pParser->pszPos;
    int fNegative = 0;

    {
        PCSZ pszPos = pszStart;
        if (pszPos < pParser->pszEnd && (*pszPos == '+' || *pszPos == '-')) {
            fNegative = (*pszPos == '-');
            pszPos++;
        }
        if (pszPos + 3 <= pParser->pszEnd) {
            if (strncmp(pszPos, "inf", 3) == 0 &&
                (pszPos + 3 == pParser->pszEnd ||
                 !is_bare_key_char(pszPos[3]))) {
                PTOMLVALUE pValue = value_new(TOML_TYPE_FLOAT);
                if (!pValue) return -1;
                pValue->u.dblFloat = toml_inf(fNegative);
                pParser->pszPos = pszPos + 3;
                *ppValue = pValue;
                return 0;
            }
            if (strncmp(pszPos, "nan", 3) == 0 &&
                (pszPos + 3 == pParser->pszEnd ||
                 !is_bare_key_char(pszPos[3]))) {
                PTOMLVALUE pValue = value_new(TOML_TYPE_FLOAT);
                if (!pValue) return -1;
                pValue->u.dblFloat = toml_nan();
                pParser->pszPos = pszPos + 3;
                *ppValue = pValue;
                return 0;
            }
        }
    }

    if (looks_like_datetime(pParser->pszPos, pParser->pszEnd)) {
        PCSZ pszTokStart = pParser->pszPos;
        PSZ pszTok;
        PTOMLVALUE pValue;
        size_t cbTokLen;

        if (pParser->pszPos + 10 <= pParser->pszEnd &&
            is_digit_c(pParser->pszPos[0]) &&
            is_digit_c(pParser->pszPos[1]) &&
            is_digit_c(pParser->pszPos[2]) &&
            is_digit_c(pParser->pszPos[3]) &&
            pParser->pszPos[4] == '-') {
            pParser->pszPos += 10;
            if (pParser->pszPos < pParser->pszEnd &&
                (*pParser->pszPos == 'T' ||
                 *pParser->pszPos == 't' ||
                 *pParser->pszPos == ' ')) {
                PCSZ pszSep = pParser->pszPos;
                pParser->pszPos++;
                if (pParser->pszPos + 8 <= pParser->pszEnd &&
                    is_digit_c(pParser->pszPos[0]) &&
                    is_digit_c(pParser->pszPos[1]) &&
                    pParser->pszPos[2] == ':' &&
                    is_digit_c(pParser->pszPos[3]) &&
                    is_digit_c(pParser->pszPos[4]) &&
                    pParser->pszPos[5] == ':' &&
                    is_digit_c(pParser->pszPos[6]) &&
                    is_digit_c(pParser->pszPos[7])) {
                    pParser->pszPos += 8;
                    if (pParser->pszPos < pParser->pszEnd &&
                        *pParser->pszPos == '.') {
                        pParser->pszPos++;
                        while (pParser->pszPos < pParser->pszEnd &&
                               is_digit_c(*pParser->pszPos))
                            pParser->pszPos++;
                    }
                    if (pParser->pszPos < pParser->pszEnd &&
                        (*pParser->pszPos == 'Z' ||
                         *pParser->pszPos == 'z')) {
                        pParser->pszPos++;
                    } else if (pParser->pszPos < pParser->pszEnd &&
                               (*pParser->pszPos == '+' ||
                                *pParser->pszPos == '-')) {
                        if (pParser->pszPos + 6 <= pParser->pszEnd &&
                            is_digit_c(pParser->pszPos[1]) &&
                            is_digit_c(pParser->pszPos[2]) &&
                            pParser->pszPos[3] == ':' &&
                            is_digit_c(pParser->pszPos[4]) &&
                            is_digit_c(pParser->pszPos[5])) {
                            pParser->pszPos += 6;
                        }
                    }
                } else {
                    pParser->pszPos = pszSep;
                }
            }
        } else {
            pParser->pszPos += 8;
            if (pParser->pszPos < pParser->pszEnd &&
                *pParser->pszPos == '.') {
                pParser->pszPos++;
                while (pParser->pszPos < pParser->pszEnd &&
                       is_digit_c(*pParser->pszPos))
                    pParser->pszPos++;
            }
        }
        cbTokLen = (size_t)(pParser->pszPos - pszTokStart);
        pszTok = dup_n(pszTokStart, cbTokLen);
        if (!pszTok) return -1;
        if (!validate_datetime(pszTok, cbTokLen)) {
            free(pszTok);
            return -1;
        }
        pValue = value_new(TOML_TYPE_DATETIME);
        if (!pValue) { free(pszTok); return -1; }
        pValue->u.pszString = pszTok;
        *ppValue = pValue;
        return 0;
    }

    {
        PCSZ pszPos = pszStart;
        if (*pszPos == '+' || *pszPos == '-') pszPos++;
        if (pszPos + 1 < pParser->pszEnd && *pszPos == '0' &&
            (pszPos[1] == 'x' || pszPos[1] == 'o' || pszPos[1] == 'b')) {
            PCSZ pszQ = pszStart;
            LONGLONG llVal;
            PTOMLVALUE pValue;
            while (pszQ < pParser->pszEnd) {
                CHAR ch = *pszQ;
                if (ch == ' ' || ch == '\t' || ch == '\n' ||
                    ch == '\r' || ch == ',' || ch == ']' ||
                    ch == '}' || ch == '#')
                    break;
                pszQ++;
            }
            pParser->pszPos = pszQ;
            if (!parse_integer_token(pszStart, pszQ, &llVal)) return -1;
            pValue = value_new(TOML_TYPE_INTEGER);
            if (!pValue) return -1;
            pValue->u.llInteger = llVal;
            *ppValue = pValue;
            return 0;
        }
    }

    {
        PCSZ pszQ = pszStart;
        int fDotOrExp = 0;
        size_t cbTokLen;
        PSZ pszClean;
        size_t cbCleanLen, i;
        PTOMLVALUE pValue;

        while (pszQ < pParser->pszEnd) {
            CHAR ch = *pszQ;
            if (ch == ' ' || ch == '\t' || ch == '\n' ||
                ch == '\r' || ch == ',' || ch == ']' ||
                ch == '}' || ch == '#')
                break;
            if (ch == '.' || ch == 'e' || ch == 'E') fDotOrExp = 1;
            pszQ++;
        }
        cbTokLen = (size_t)(pszQ - pszStart);
        pParser->pszPos = pszQ;

        if (fDotOrExp) {
            pszClean = (PSZ)malloc(cbTokLen + 1);
            if (!pszClean) return -1;
            cbCleanLen = 0;
            for (i = 0; i < cbTokLen; i++) {
                if (pszStart[i] == '_') {
                    if (i == 0 || i == cbTokLen - 1 ||
                        !is_digit_c((UCHAR)pszStart[i-1]) ||
                        !is_digit_c((UCHAR)pszStart[i+1])) {
                        free(pszClean);
                        return -1;
                    }
                    continue;
                }
                pszClean[cbCleanLen++] = pszStart[i];
            }
            pszClean[cbCleanLen] = '\0';
            if (!validate_float_clean(pszClean)) {
                free(pszClean);
                return -1;
            }
            {
                double dblVal = strtod(pszClean, NULL);
                free(pszClean);
                pValue = value_new(TOML_TYPE_FLOAT);
                if (!pValue) return -1;
                pValue->u.dblFloat = dblVal;
                *ppValue = pValue;
                return 0;
            }
        } else {
            LONGLONG llVal;
            if (!parse_integer_token(pszStart, pszQ, &llVal)) return -1;
            pValue = value_new(TOML_TYPE_INTEGER);
            if (!pValue) return -1;
            pValue->u.llInteger = llVal;
            *ppValue = pValue;
            return 0;
        }
    }
}

/* ==================================================================
 * Boolean
 * ================================================================== */

/**
 * @brief Parse a boolean token (true / false).
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[out]    ppValue  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_boolean(PARSE *pParser, PTOMLVALUE *ppValue) {
    if (pParser->pszPos + 4 <= pParser->pszEnd &&
        strncmp(pParser->pszPos, "true", 4) == 0 &&
        (pParser->pszPos + 4 == pParser->pszEnd ||
         !is_bare_key_char(pParser->pszPos[4]))) {
        PTOMLVALUE pValue = value_new(TOML_TYPE_BOOLEAN);
        if (!pValue) return -1;
        pValue->u.fBoolean = TRUE_;
        pParser->pszPos += 4;
        *ppValue = pValue;
        return 0;
    }
    if (pParser->pszPos + 5 <= pParser->pszEnd &&
        strncmp(pParser->pszPos, "false", 5) == 0 &&
        (pParser->pszPos + 5 == pParser->pszEnd ||
         !is_bare_key_char(pParser->pszPos[5]))) {
        PTOMLVALUE pValue = value_new(TOML_TYPE_BOOLEAN);
        if (!pValue) return -1;
        pValue->u.fBoolean = FALSE_;
        pParser->pszPos += 5;
        *ppValue = pValue;
        return 0;
    }
    return -1;
}

/* ==================================================================
 * Arrays and inline tables
 * ================================================================== */

static int parse_value(PARSE *pParser, PTOMLVALUE *ppValue);
static int parse_key_path(PARSE *pParser, PSZ **ppapszKeys,
                          PULONG pulCount);

/**
 * @brief Release a parsed key path.
 *
 * @param[in] papszKeys  Array of keys. May be NULL.
 * @param[in] ulCount    Number of entries.
 */
static void free_key_path(PSZ *papszKeys, ULONG ulCount) {
    ULONG ulIdx;
    if (!papszKeys) return;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) free(papszKeys[ulIdx]);
    free(papszKeys);
}

/**
 * @brief Parse a TOML array.
 *
 * @param[in,out] pParser  Parser at the opening '['. Not NULL.
 * @param[out]    ppValue  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_array(PARSE *pParser, PTOMLVALUE *ppValue) {
    PTOMLARRAY pArray = array_new();
    PTOMLVALUE pValue;
    if (!pArray) return -1;
    pParser->pszPos++;

    for (;;) {
        PTOMLVALUE pItem = NULL;
        if (skip_ws_nl_comments(pParser) != 0) {
            array_free(pArray); return -1;
        }
        if (pParser->pszPos >= pParser->pszEnd) {
            array_free(pArray); return -1;
        }
        if (*pParser->pszPos == ']') { pParser->pszPos++; break; }
        if (parse_value(pParser, &pItem) != 0) {
            array_free(pArray); return -1;
        }
        if (array_add(pArray, pItem) != 0) {
            value_free(pItem); array_free(pArray); return -1;
        }
        if (skip_ws_nl_comments(pParser) != 0) {
            array_free(pArray); return -1;
        }
        if (pParser->pszPos >= pParser->pszEnd) {
            array_free(pArray); return -1;
        }
        if (*pParser->pszPos == ',') { pParser->pszPos++; continue; }
        if (*pParser->pszPos == ']') { pParser->pszPos++; break; }
        array_free(pArray); return -1;
    }
    pValue = value_array(pArray);
    if (!pValue) { array_free(pArray); return -1; }
    *ppValue = pValue;
    return 0;
}

/**
 * @brief Assign a value to a dotted key path inside an inline table.
 *
 * @param[in] pTable    Table. Not NULL.
 * @param[in] papszKeys Key path. Not NULL.
 * @param[in] ulNKeys   Number of keys.
 * @param[in] pValue    Value. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int inline_assign(PTOMLTABLE pTable, PSZ *papszKeys,
                         ULONG ulNKeys, PTOMLVALUE pValue) {
    PTOMLTABLE pCur = pTable;
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx + 1 < ulNKeys; ulIdx++) {
        PTOMLVALUE pSubValue = table_find_value(pCur, papszKeys[ulIdx]);
        if (!pSubValue) {
            PTOMLTABLE pSub = table_new();
            PTOMLVALUE pNewSubValue;
            if (!pSub) return -1;
            pSub->flFlags |= TOML_TABLE_INLINE;
            pNewSubValue = value_table(pSub);
            if (!pNewSubValue) { table_free(pSub); return -1; }
            if (table_add(pCur, strdup(papszKeys[ulIdx]),
                          pNewSubValue) != 0) {
                value_free(pNewSubValue); return -1;
            }
            pCur = pSub;
        } else if (pSubValue->ulType == TOML_TYPE_TABLE) {
            if (!(pSubValue->u.pTable->flFlags & TOML_TABLE_INLINE))
                return -1;
            pCur = pSubValue->u.pTable;
        } else {
            return -1;
        }
    }
    if (ulNKeys == 0) return -1;
    if (table_has_key(pCur, papszKeys[ulNKeys - 1])) return -1;
    if (table_add(pCur, strdup(papszKeys[ulNKeys - 1]),
                  pValue) != 0) return -1;
    return 0;
}

/**
 * @brief Parse an inline table.
 *
 * @param[in,out] pParser  Parser at the opening '{'. Not NULL.
 * @param[out]    ppValue  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_inline_table(PARSE *pParser, PTOMLVALUE *ppValue) {
    PTOMLTABLE pTable = table_new();
    PTOMLVALUE pValueTable;
    if (!pTable) return -1;
    pTable->flFlags |= TOML_TABLE_INLINE;
    pParser->pszPos++;

    skip_ws(pParser);
    if (pParser->pszPos < pParser->pszEnd &&
        *pParser->pszPos == '}') {
        pParser->pszPos++;
        pValueTable = value_table(pTable);
        if (!pValueTable) { table_free(pTable); return -1; }
        *ppValue = pValueTable;
        return 0;
    }
    for (;;) {
        PSZ *papszKeys = NULL;
        ULONG ulNKeys = 0;
        PTOMLVALUE pValue = NULL;
        int nOk;

        skip_ws(pParser);
        if (parse_key_path(pParser, &papszKeys, &ulNKeys) != 0) {
            table_free(pTable); return -1;
        }
        skip_ws(pParser);
        if (pParser->pszPos >= pParser->pszEnd ||
            *pParser->pszPos != '=') {
            free_key_path(papszKeys, ulNKeys);
            table_free(pTable); return -1;
        }
        pParser->pszPos++;
        skip_ws(pParser);
        if (parse_value(pParser, &pValue) != 0) {
            free_key_path(papszKeys, ulNKeys);
            table_free(pTable); return -1;
        }
        nOk = inline_assign(pTable, papszKeys, ulNKeys, pValue);
        free_key_path(papszKeys, ulNKeys);
        if (nOk != 0) {
            value_free(pValue); table_free(pTable); return -1;
        }
        skip_ws(pParser);
        if (pParser->pszPos >= pParser->pszEnd) {
            table_free(pTable); return -1;
        }
        if (*pParser->pszPos == ',') { pParser->pszPos++; continue; }
        if (*pParser->pszPos == '}') { pParser->pszPos++; break; }
        table_free(pTable); return -1;
    }
    pValueTable = value_table(pTable);
    if (!pValueTable) { table_free(pTable); return -1; }
    *ppValue = pValueTable;
    return 0;
}

/**
 * @brief Parse one TOML value.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 * @param[out]    ppValue  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_value(PARSE *pParser, PTOMLVALUE *ppValue) {
    skip_ws(pParser);
    if (pParser->pszPos >= pParser->pszEnd) return -1;

    if (*pParser->pszPos == '"') {
        PSZ pszStr;
        PTOMLVALUE pValue;
        if (pParser->pszPos + 2 < pParser->pszEnd &&
            pParser->pszPos[1] == '"' &&
            pParser->pszPos[2] == '"')
            pszStr = parse_multiline_basic_string(pParser);
        else
            pszStr = parse_basic_string(pParser);
        if (!pszStr) return -1;
        pValue = value_new(TOML_TYPE_STRING);
        if (!pValue) { free(pszStr); return -1; }
        pValue->u.pszString = pszStr;
        *ppValue = pValue;
        return 0;
    }
    if (*pParser->pszPos == '\'') {
        PSZ pszStr;
        PTOMLVALUE pValue;
        if (pParser->pszPos + 2 < pParser->pszEnd &&
            pParser->pszPos[1] == '\'' &&
            pParser->pszPos[2] == '\'')
            pszStr = parse_multiline_literal_string(pParser);
        else
            pszStr = parse_literal_string(pParser);
        if (!pszStr) return -1;
        pValue = value_new(TOML_TYPE_STRING);
        if (!pValue) { free(pszStr); return -1; }
        pValue->u.pszString = pszStr;
        *ppValue = pValue;
        return 0;
    }
    if (*pParser->pszPos == '[') return parse_array(pParser, ppValue);
    if (*pParser->pszPos == '{') return parse_inline_table(pParser, ppValue);
    if (*pParser->pszPos == 't' || *pParser->pszPos == 'f')
        return parse_boolean(pParser, ppValue);
    if (is_number_token_start(pParser->pszPos, pParser->pszEnd))
        return parse_number_or_datetime(pParser, ppValue);
    return -1;
}

/* ==================================================================
 * Keys
 * ================================================================== */

/**
 * @brief Parse one key part (bare, basic or literal string).
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return malloc'd key, or NULL on error.
 */
static PSZ parse_key_part(PARSE *pParser) {
    skip_ws(pParser);
    if (pParser->pszPos >= pParser->pszEnd) return NULL;
    if (*pParser->pszPos == '"') return parse_basic_string(pParser);
    if (*pParser->pszPos == '\'') return parse_literal_string(pParser);
    {
        PCSZ pszStart = pParser->pszPos;
        while (pParser->pszPos < pParser->pszEnd &&
               is_bare_key_char((UCHAR)*pParser->pszPos))
            pParser->pszPos++;
        if (pParser->pszPos == pszStart) return NULL;
        return dup_n(pszStart, (size_t)(pParser->pszPos - pszStart));
    }
}

/**
 * @brief Parse a dotted key path.
 *
 * @param[in,out] pParser     Parser. Not NULL.
 * @param[out]    ppapszKeys  Receiver for the key array. Not NULL.
 * @param[out]    pulCount    Receiver for the count. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int parse_key_path(PARSE *pParser, PSZ **ppapszKeys,
                          PULONG pulCount) {
    PSZ *papszKeys;
    ULONG ulN = 0, ulCap = 4;

    papszKeys = (PSZ*)malloc(ulCap * sizeof(PSZ));
    if (!papszKeys) return -1;

    for (;;) {
        PSZ pszPart = parse_key_part(pParser);
        if (!pszPart) goto fail;
        if (ulN >= ulCap) {
            PSZ *papszNew;
            ulCap *= 2;
            papszNew = (PSZ*)realloc(papszKeys, ulCap * sizeof(PSZ));
            if (!papszNew) { free(pszPart); goto fail; }
            papszKeys = papszNew;
        }
        papszKeys[ulN++] = pszPart;
        skip_ws(pParser);
        if (pParser->pszPos < pParser->pszEnd &&
            *pParser->pszPos == '.') { pParser->pszPos++; continue; }
        break;
    }
    *ppapszKeys = papszKeys;
    *pulCount = ulN;
    return 0;
fail:
    {
        ULONG ulIdx;
        for (ulIdx = 0; ulIdx < ulN; ulIdx++) free(papszKeys[ulIdx]);
        free(papszKeys);
    }
    return -1;
}

/* ==================================================================
 * Table navigation
 * ================================================================== */

/**
 * @brief Descend into a table by a key path, creating tables as
 *        needed.
 *
 * @param[in] pRoot                Root table. Not NULL.
 * @param[in] papszKeys            Key path. Not NULL.
 * @param[in] ulCount              Number of keys.
 * @param[in] ulIntermediateFlags  Flags for intermediate tables.
 * @param[in] ulFinalFlags         Flags for the final table.
 *
 * @return Final table, or NULL on error.
 */
static PTOMLTABLE descend(PTOMLTABLE pRoot, PSZ *papszKeys,
                          ULONG ulCount, ULONG ulIntermediateFlags,
                          ULONG ulFinalFlags) {
    PTOMLTABLE pCur = pRoot;
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        PTOMLVALUE pExisting = table_find_value(pCur, papszKeys[ulIdx]);
        int fIsLast = (ulIdx + 1 == ulCount);
        ULONG ulWantFlags = fIsLast ? ulFinalFlags : ulIntermediateFlags;

        if (pExisting) {
            if (pExisting->ulType == TOML_TYPE_TABLE) {
                PTOMLTABLE pSub = pExisting->u.pTable;
                if (pSub->flFlags & TOML_TABLE_INLINE) return NULL;
                if (fIsLast && (ulWantFlags & TOML_TABLE_EXPLICIT)) {
                    if (pSub->flFlags & TOML_TABLE_EXPLICIT) return NULL;
                    if (pSub->flFlags & TOML_TABLE_FROM_DOTTED) return NULL;
                    pSub->flFlags |= TOML_TABLE_EXPLICIT;
                }
                pCur = pSub;
            } else if (pExisting->ulType == TOML_TYPE_ARRAY) {
                PTOMLARRAY pArray = pExisting->u.pArray;
                PTOMLVALUE pLast;
                if (!(pArray->flFlags & TOML_ARRAY_OF_TABLES)) return NULL;
                if (pArray->ulCount == 0) return NULL;
                pLast = pArray->paItems[pArray->ulCount - 1];
                if (pLast->ulType != TOML_TYPE_TABLE) return NULL;
                pCur = pLast->u.pTable;
            } else {
                return NULL;
            }
        } else {
            PTOMLTABLE pSub = table_new();
            PTOMLVALUE pSubValue;
            ULONG ulFlags;
            if (!pSub) return NULL;
            ulFlags = ulWantFlags & ~TOML_TABLE_ARRAY_ELEM;
            pSub->flFlags |= ulFlags;
            pSubValue = value_table(pSub);
            if (!pSubValue) { table_free(pSub); return NULL; }
            if (table_add(pCur, strdup(papszKeys[ulIdx]),
                          pSubValue) != 0) {
                value_free(pSubValue); return NULL;
            }
            pCur = pSub;
        }
    }
    return pCur;
}

/* ==================================================================
 * Main parse
 * ================================================================== */

/**
 * @brief Parse TOML text and build the tree.
 *
 * @param[in]  pszText    NUL-terminated UTF-8 TOML text. Not NULL.
 * @param[out] ppRoot     Receiver for the root table. Not NULL.
 * @param[out] ppszError  Optional. May be NULL. Receives a static
 *                        error description on failure.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      pszText or ppRoot is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval TOML_ERROR_INVALID_UTF8      File content is not valid
 *                                      UTF-8.
 * @retval TOML_ERROR_INVALID_SYNTAX    TOML syntax error.
 * @retval TOML_ERROR_DUPLICATE_KEY     Duplicate key.
 */
APIRET TomlInternalParse(PCSZ pszText, PTOMLTABLE *ppRoot,
                         PCSZ *ppszError) {
    PARSE parser;
    PTOMLTABLE pRoot;
    PTOMLTABLE pCurrent;
    PCSZ pszPos;
    PSZ *papszKeys = NULL;
    ULONG ulNKeys = 0;
    PTOMLVALUE pValue = NULL;
    APIRET rc = NO_ERROR;

    if (!pszText || !ppRoot) return ERROR_INVALID_PARAMETER;

    pszPos = pszText;
    if ((UCHAR)pszPos[0] == 0xEF &&
        (UCHAR)pszPos[1] == 0xBB &&
        (UCHAR)pszPos[2] == 0xBF) pszPos += 3;

    if (!valid_utf8(pszPos)) {
        if (ppszError) *ppszError = "Invalid UTF-8 input";
        return TOML_ERROR_INVALID_UTF8;
    }

    pRoot = table_new();
    if (!pRoot) {
        if (ppszError) *ppszError = "Out of memory";
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    parser.pszPos = pszPos;
    parser.pszEnd = pszPos + strlen(pszPos);
    parser.pszError = NULL;
    pCurrent = pRoot;

    while (parser.pszPos < parser.pszEnd) {
        if (skip_ws_nl_comments(&parser) != 0) {
            rc = TOML_ERROR_INVALID_SYNTAX;
            goto done;
        }
        if (parser.pszPos >= parser.pszEnd) break;

        if (*parser.pszPos == '[') {
            int fArray = 0;
            parser.pszPos++;
            if (parser.pszPos < parser.pszEnd &&
                *parser.pszPos == '[') { fArray = 1; parser.pszPos++; }
            if (parse_key_path(&parser, &papszKeys, &ulNKeys) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            skip_ws(&parser);
            if (parser.pszPos >= parser.pszEnd ||
                *parser.pszPos != ']') {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            parser.pszPos++;
            if (fArray) {
                if (parser.pszPos >= parser.pszEnd ||
                    *parser.pszPos != ']') {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
                parser.pszPos++;
            }
            if (skip_to_eol(&parser) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            if (ulNKeys == 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }

            if (fArray) {
                PTOMLTABLE pParent;
                PTOMLVALUE pExisting;
                PTOMLTABLE pNewTable;

                pParent = descend(pRoot, papszKeys, ulNKeys - 1, 0, 0);
                if (!pParent) {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
                pExisting = table_find_value(pParent,
                                             papszKeys[ulNKeys - 1]);
                pNewTable = table_new();
                if (!pNewTable) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto done;
                }
                pNewTable->flFlags |= TOML_TABLE_ARRAY_ELEM;

                if (pExisting) {
                    PTOMLARRAY pArray;
                    PTOMLVALUE pArrValue;
                    if (pExisting->ulType != TOML_TYPE_ARRAY ||
                        !(pExisting->u.pArray->flFlags &
                          TOML_ARRAY_OF_TABLES)) {
                        table_free(pNewTable);
                        rc = TOML_ERROR_INVALID_SYNTAX;
                        goto done;
                    }
                    pArray = pExisting->u.pArray;
                    pArrValue = value_table(pNewTable);
                    if (!pArrValue) {
                        table_free(pNewTable);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                    if (array_add(pArray, pArrValue) != 0) {
                        value_free(pArrValue);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                } else {
                    PTOMLARRAY pArray = array_new();
                    PTOMLVALUE pArrValue, pAv;
                    if (!pArray) {
                        table_free(pNewTable);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                    pArray->flFlags |= TOML_ARRAY_OF_TABLES;
                    pArrValue = value_table(pNewTable);
                    if (!pArrValue) {
                        array_free(pArray);
                        table_free(pNewTable);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                    if (array_add(pArray, pArrValue) != 0) {
                        array_free(pArray);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                    pAv = value_array(pArray);
                    if (!pAv) {
                        array_free(pArray);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                    if (table_add(pParent,
                                  strdup(papszKeys[ulNKeys - 1]),
                                  pAv) != 0) {
                        value_free(pAv);
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto done;
                    }
                }
                pCurrent = pNewTable;
            } else {
                PTOMLTABLE pTable = descend(pRoot, papszKeys, ulNKeys,
                                            0, TOML_TABLE_EXPLICIT);
                if (!pTable) {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
                pCurrent = pTable;
            }
            free_key_path(papszKeys, ulNKeys);
            papszKeys = NULL;
            ulNKeys = 0;
        } else {
            PTOMLTABLE pTarget;

            if (parse_key_path(&parser, &papszKeys, &ulNKeys) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            skip_ws(&parser);
            if (parser.pszPos >= parser.pszEnd ||
                *parser.pszPos != '=') {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            parser.pszPos++;
            if (parse_value(&parser, &pValue) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            if (skip_to_eol(&parser) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            if (ulNKeys == 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }

            if (ulNKeys == 1) {
                pTarget = pCurrent;
            } else {
                pTarget = descend(pCurrent, papszKeys, ulNKeys - 1,
                                  TOML_TABLE_FROM_DOTTED,
                                  TOML_TABLE_FROM_DOTTED);
                if (!pTarget) {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
            }
            if (table_has_key(pTarget, papszKeys[ulNKeys - 1])) {
                rc = TOML_ERROR_DUPLICATE_KEY;
                goto done;
            }
            if (table_add(pTarget, strdup(papszKeys[ulNKeys - 1]),
                          pValue) != 0) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }
            pValue = NULL;
            free_key_path(papszKeys, ulNKeys);
            papszKeys = NULL;
            ulNKeys = 0;
        }
    }

    *ppRoot = pRoot;
    pRoot = NULL;
    rc = NO_ERROR;

done:
    free_key_path(papszKeys, ulNKeys);
    value_free(pValue);
    if (pRoot) table_free(pRoot);
    if (rc != NO_ERROR && ppszError) {
        *ppszError = parser.pszError ? parser.pszError : "TOML syntax error";
    }
    return rc;
}

/* ==================================================================
 * Handles and file helpers
 * ================================================================== */

/**
 * @brief Translate a public document handle into the internal pointer.
 *
 * @param[in] h  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if h is NULLHANDLE.
 */
static PTOMLDOC as_doc(HTOMLDOC h) { return (PTOMLDOC)h; }

/**
 * @brief Translate a public find handle into the internal pointer.
 *
 * @param[in] h  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if h is NULLHANDLE.
 */
static PTOMLFIND as_find(HTOMLFIND h) { return (PTOMLFIND)h; }

/**
 * @brief Read the whole file into a malloc buffer.
 *
 * @param[in]  pszPath   Path to the file. Not NULL.
 * @param[out] ppszText  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int read_file_all(PCSZ pszPath, PSZ *ppszText) {
    FILE *fp;
    long lSize;
    PSZ pszBuf;
    if (!pszPath || !ppszText) return -1;
    fp = fopen(pszPath, "rb");
    if (!fp) return -1;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return -1; }
    lSize = ftell(fp);
    if (lSize < 0) { fclose(fp); return -1; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return -1; }
    pszBuf = (PSZ)malloc((size_t)lSize + 1);
    if (!pszBuf) { fclose(fp); return -1; }
    if (lSize > 0 && fread(pszBuf, 1, (size_t)lSize, fp) != (size_t)lSize) {
        free(pszBuf); fclose(fp); return -1;
    }
    pszBuf[lSize] = '\0';
    fclose(fp);
    *ppszText = pszBuf;
    return 0;
}

/**
 * @brief Open a TOML document from a file.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phToml   Handle receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      pszPath or phToml is NULL.
 * @retval ERROR_OPEN_FAILED            File cannot be opened.
 * @retval ERROR_READ_FAULT             Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval TOML_ERROR_INVALID_UTF8      File content is not valid
 *                                      UTF-8.
 * @retval TOML_ERROR_INVALID_SYNTAX    TOML syntax error.
 * @retval TOML_ERROR_DUPLICATE_KEY     Duplicate key.
 */
APIRET APIENTRY TomlOpen(PCSZ pszPath, HTOMLDOC *phToml) {
    PSZ pszText = NULL;
    PTOMLTABLE pRoot = NULL;
    PTOMLDOC pDoc;
    PCSZ pszErr = NULL;
    APIRET rc;

    if (!pszPath || !phToml) return ERROR_INVALID_PARAMETER;
    *phToml = NULLHANDLE;

    if (read_file_all(pszPath, &pszText) != 0) return ERROR_OPEN_FAILED;
    rc = TomlInternalParse(pszText, &pRoot, &pszErr);
    free(pszText);
    if (rc != NO_ERROR) return rc;

    pDoc = (PTOMLDOC)calloc(1, sizeof(TOMLDOC));
    if (!pDoc) { table_free(pRoot); return ERROR_NOT_ENOUGH_MEMORY; }
    pDoc->pRoot = pRoot;
    pDoc->pFirstFind = NULL;
    pDoc->pRootNode = (PTOMLVALUE)calloc(1, sizeof(TOMLVALUE));
    if (!pDoc->pRootNode) {
        table_free(pRoot);
        free(pDoc);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pDoc->pRootNode->ulType = TOML_TYPE_TABLE;
    pDoc->pRootNode->u.pTable = pRoot;
    *phToml = (HTOMLDOC)pDoc;
    return NO_ERROR;
}

/**
 * @brief Close a document.
 *
 * @param[in] hToml  Document handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 */
APIRET APIENTRY TomlClose(HTOMLDOC hToml) {
    PTOMLDOC pDoc;
    if (hToml == NULLHANDLE) return NO_ERROR;
    pDoc = as_doc(hToml);
    if (!pDoc) return ERROR_INVALID_HANDLE;
    {
        PTOMLFIND pFind = pDoc->pFirstFind;
        while (pFind) {
            PTOMLFIND pNext = pFind->pNext;
            free(pFind->pszPattern);
            free(pFind);
            pFind = pNext;
        }
    }
    free(pDoc->pRootNode);
    table_free(pDoc->pRoot);
    free(pDoc);
    return NO_ERROR;
}

/* ==================================================================
 * Path navigation
 * ================================================================== */

/**
 * @brief Find a value by dotted path.
 *
 * @param[in] pRoot    Root table. Not NULL.
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return Pointer to the value, or NULL if not found.
 */
static PTOMLVALUE find_path(PTOMLTABLE pRoot, PCSZ pszPath) {
    PSZ pszCopy, pszTok, pszPos;
    PTOMLTABLE pTable;
    PTOMLVALUE pCur = NULL;

    if (!pRoot || !pszPath || !*pszPath) return NULL;
    pszCopy = strdup(pszPath);
    if (!pszCopy) return NULL;

    pTable = pRoot;
    pszPos = pszCopy;
    while (*pszPos) {
        PTOMLVALUE pNext;
        pszTok = pszPos;
        while (*pszPos && *pszPos != '.') pszPos++;
        if (*pszPos == '.') { *pszPos = '\0'; pszPos++; }
        pNext = table_find_value(pTable, pszTok);
        if (!pNext) { free(pszCopy); return NULL; }
        pCur = pNext;
        if (*pszPos) {
            if (pCur->ulType != TOML_TYPE_TABLE) {
                free(pszCopy); return NULL;
            }
            pTable = pCur->u.pTable;
        }
    }
    free(pszCopy);
    return pCur;
}

/* ==================================================================
 * Query API
 * ================================================================== */

/**
 * @brief Query the type of a value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryType(HTOMLDOC hToml, PCSZ pszPath,
                              PULONG pulType) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !pulType) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    *pulType = pValue->ulType;
    return NO_ERROR;
}

/**
 * @brief Copy a string into a caller-supplied buffer following the
 *        size-query convention.
 *
 * @param[in]  pszSrc   Source string, or NULL.
 * @param[out] pszDst   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszDst.
 * @param[out] pulSize  Optional. May be NULL.
 *
 * @return APIRET
 */
static APIRET copy_string_out(PCSZ pszSrc, PSZ pszDst, ULONG ulSize,
                              PULONG pulSize) {
    size_t cbLen = pszSrc ? strlen(pszSrc) : 0;
    if (pulSize) *pulSize = (ULONG)cbLen;
    if (ulSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszDst, pszSrc ? pszSrc : "", cbLen);
    pszDst[cbLen] = '\0';
    return NO_ERROR;
}

/**
 * @brief Query a string value by dotted path.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryString(HTOMLDOC hToml, PCSZ pszPath,
                                PSZ pszBuffer, ULONG ulBufSize,
                                PULONG pulSize) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    PCSZ pszStr;
    size_t cbLen;

    if (!pDoc || !pszPath) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_STRING &&
        pValue->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;

    pszStr = pValue->u.pszString;
    cbLen = pszStr ? strlen(pszStr) : 0;

    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuffer) return ERROR_INVALID_PARAMETER;
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pszStr ? pszStr : "", cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/**
 * @brief Query an integer value by dotted path.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryInteger(HTOMLDOC hToml, PCSZ pszPath,
                                 PLONGLONG pllValue) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !pllValue) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_INTEGER) {
        *pllValue = 0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pllValue = pValue->u.llInteger;
    return NO_ERROR;
}

/**
 * @brief Query a floating-point value by dotted path.
 *
 * @param[in]  hToml       Handle. Not NULLHANDLE.
 * @param[in]  pszPath     Path. Not NULL.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryFloat(HTOMLDOC hToml, PCSZ pszPath,
                               double *pdblValue) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !pdblValue) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_FLOAT) {
        *pdblValue = 0.0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pdblValue = pValue->u.dblFloat;
    return NO_ERROR;
}

/**
 * @brief Query a boolean value by dotted path.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryBoolean(HTOMLDOC hToml, PCSZ pszPath,
                                 PBOOL pfValue) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !pfValue) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_BOOLEAN) {
        *pfValue = FALSE_;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pfValue = pValue->u.fBoolean;
    return NO_ERROR;
}

/**
 * @brief Query the number of elements in an array by dotted path.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryArrayCount(HTOMLDOC hToml, PCSZ pszPath,
                                    PULONG pulCount) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !pulCount) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    *pulCount = pValue->u.pArray->ulCount;
    return NO_ERROR;
}

/**
 * @brief Query the type of one array element by index.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL.
 * @param[in]  ulIndex   Element index.
 * @param[out] pulType   Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryArrayType(HTOMLDOC hToml, PCSZ pszPath,
                                   ULONG ulIndex, PULONG pulType) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !pulType) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pValue->u.pArray->ulCount) return ERROR_NO_MORE_ITEMS;
    *pulType = pValue->u.pArray->paItems[ulIndex]->ulType;
    return NO_ERROR;
}

/**
 * @brief Query a string element of an array by index.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL.
 * @param[in]  ulIndex    Element index.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryArrayString(HTOMLDOC hToml, PCSZ pszPath,
                                     ULONG ulIndex, PSZ pszBuffer,
                                     ULONG ulBufSize, PULONG pulSize) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue, pItem;
    PCSZ pszStr;
    size_t cbLen;

    if (!pDoc || !pszPath) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pValue->u.pArray->ulCount) return ERROR_NO_MORE_ITEMS;
    pItem = pValue->u.pArray->paItems[ulIndex];
    if (pItem->ulType != TOML_TYPE_STRING &&
        pItem->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;

    pszStr = pItem->u.pszString;
    cbLen = pszStr ? strlen(pszStr) : 0;
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuffer) return ERROR_INVALID_PARAMETER;
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pszStr ? pszStr : "", cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/**
 * @brief Query an integer element of an array by index.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL.
 * @param[in]  ulIndex    Element index.
 * @param[out] pllValue   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryArrayInteger(HTOMLDOC hToml, PCSZ pszPath,
                                      ULONG ulIndex, PLONGLONG pllValue) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue, pItem;
    if (!pDoc || !pszPath || !pllValue) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pValue->u.pArray->ulCount) return ERROR_NO_MORE_ITEMS;
    pItem = pValue->u.pArray->paItems[ulIndex];
    if (pItem->ulType != TOML_TYPE_INTEGER) return TOML_ERROR_TYPE_MISMATCH;
    *pllValue = pItem->u.llInteger;
    return NO_ERROR;
}

/**
 * @brief Query a floating-point element of an array by index.
 *
 * @param[in]  hToml       Handle. Not NULLHANDLE.
 * @param[in]  pszPath     Path. Not NULL.
 * @param[in]  ulIndex     Element index.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryArrayFloat(HTOMLDOC hToml, PCSZ pszPath,
                                    ULONG ulIndex, double *pdblValue) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue, pItem;
    if (!pDoc || !pszPath || !pdblValue) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pValue->u.pArray->ulCount) return ERROR_NO_MORE_ITEMS;
    pItem = pValue->u.pArray->paItems[ulIndex];
    if (pItem->ulType != TOML_TYPE_FLOAT) return TOML_ERROR_TYPE_MISMATCH;
    *pdblValue = pItem->u.dblFloat;
    return NO_ERROR;
}

/**
 * @brief Query a boolean element of an array by index.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL.
 * @param[in]  ulIndex   Element index.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryArrayBoolean(HTOMLDOC hToml, PCSZ pszPath,
                                      ULONG ulIndex, PBOOL pfValue) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue, pItem;
    if (!pDoc || !pszPath || !pfValue) return ERROR_INVALID_PARAMETER;
    pValue = find_path(pDoc->pRoot, pszPath);
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pValue->u.pArray->ulCount) return ERROR_NO_MORE_ITEMS;
    pItem = pValue->u.pArray->paItems[ulIndex];
    if (pItem->ulType != TOML_TYPE_BOOLEAN) return TOML_ERROR_TYPE_MISMATCH;
    *pfValue = pItem->u.fBoolean;
    return NO_ERROR;
}

/* ==================================================================
 * DOM-style traversal
 * ================================================================== */

/**
 * @brief Translate a public node handle into the internal pointer.
 *
 * @param[in] h  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if h is NULLHANDLE.
 */
static PTOMLVALUE as_node(HTOMLNODE h) { return (PTOMLVALUE)h; }

/**
 * @brief Obtain a node handle for a value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL.
 * @param[out] phNode   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryNode(HTOMLDOC hToml, PCSZ pszPath,
                              HTOMLNODE *phNode) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    if (!pDoc || !pszPath || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = NULLHANDLE;
    if (!*pszPath) {
        pValue = pDoc->pRootNode;
    } else {
        pValue = find_path(pDoc->pRoot, pszPath);
    }
    if (!pValue) return ERROR_FILE_NOT_FOUND;
    *phNode = (HTOMLNODE)pValue;
    return NO_ERROR;
}

/**
 * @brief Obtain a node handle for the root table.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[out] phNode   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlQueryRootNode(HTOMLDOC hToml, HTOMLNODE *phNode) {
    PTOMLDOC pDoc = as_doc(hToml);
    if (!pDoc || !phNode) return ERROR_INVALID_PARAMETER;
    *phNode = (HTOMLNODE)pDoc->pRootNode;
    return NO_ERROR;
}

/**
 * @brief Query the type of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetType(HTOMLNODE hNode, PULONG pulType) {
    PTOMLVALUE pValue = as_node(hNode);
    if (!pValue || !pulType) return ERROR_INVALID_PARAMETER;
    *pulType = pValue->ulType;
    return NO_ERROR;
}

/**
 * @brief Query a string value of a node.
 *
 * @param[in]  hNode      Node handle. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetString(HTOMLNODE hNode, PSZ pszBuffer,
                                  ULONG ulBufSize, PULONG pulSize) {
    PTOMLVALUE pValue = as_node(hNode);
    PCSZ pszStr;
    size_t cbLen;
    if (!pValue) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_STRING &&
        pValue->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;
    pszStr = pValue->u.pszString;
    cbLen = pszStr ? strlen(pszStr) : 0;
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuffer) return ERROR_INVALID_PARAMETER;
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pszStr ? pszStr : "", cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/**
 * @brief Query an integer value of a node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetInteger(HTOMLNODE hNode, PLONGLONG pllValue) {
    PTOMLVALUE pValue = as_node(hNode);
    if (!pValue || !pllValue) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_INTEGER) {
        *pllValue = 0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pllValue = pValue->u.llInteger;
    return NO_ERROR;
}

/**
 * @brief Query a floating-point value of a node.
 *
 * @param[in]  hNode       Node handle. Not NULLHANDLE.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetFloat(HTOMLNODE hNode, double *pdblValue) {
    PTOMLVALUE pValue = as_node(hNode);
    if (!pValue || !pdblValue) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_FLOAT) {
        *pdblValue = 0.0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pdblValue = pValue->u.dblFloat;
    return NO_ERROR;
}

/**
 * @brief Query a boolean value of a node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetBoolean(HTOMLNODE hNode, PBOOL pfValue) {
    PTOMLVALUE pValue = as_node(hNode);
    if (!pValue || !pfValue) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_BOOLEAN) {
        *pfValue = FALSE_;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pfValue = pValue->u.fBoolean;
    return NO_ERROR;
}

/**
 * @brief Query the number of elements in an array node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetArrayCount(HTOMLNODE hNode, PULONG pulCount) {
    PTOMLVALUE pValue = as_node(hNode);
    if (!pValue || !pulCount) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    *pulCount = pValue->u.pArray->ulCount;
    return NO_ERROR;
}

/**
 * @brief Query an array element by index.
 *
 * @param[in]  hNode     Node handle (array). Not NULLHANDLE.
 * @param[in]  ulIndex   Element index.
 * @param[out] phChild   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetArrayElement(HTOMLNODE hNode, ULONG ulIndex,
                                        HTOMLNODE *phChild) {
    PTOMLVALUE pValue = as_node(hNode);
    if (!pValue || !phChild) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pValue->u.pArray->ulCount) {
        *phChild = NULLHANDLE;
        return ERROR_NO_MORE_ITEMS;
    }
    *phChild = (HTOMLNODE)pValue->u.pArray->paItems[ulIndex];
    return NO_ERROR;
}

/**
 * @brief Query the number of entries in a table node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetTableCount(HTOMLNODE hNode, PULONG pulCount) {
    PTOMLVALUE pValue = as_node(hNode);
    ULONG ulCount = 0;
    if (!pValue || !pulCount) return ERROR_INVALID_PARAMETER;
    if (pValue->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
    if (VectorGetCount(pValue->u.pTable->hEntries, &ulCount) != NO_ERROR)
        return ERROR_INVALID_HANDLE;
    *pulCount = ulCount;
    return NO_ERROR;
}

/**
 * @brief Query a table entry by key.
 *
 * @param[in]  hNode     Node handle (table). Not NULLHANDLE.
 * @param[in]  pszKey    Key. Not NULL.
 * @param[out] phChild   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetTableEntryByKey(HTOMLNODE hNode, PCSZ pszKey,
                                           HTOMLNODE *phChild) {
    PTOMLVALUE pValue = as_node(hNode);
    TOMLENTRY entry;
    if (!pValue || !pszKey || !phChild) return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pValue->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
    if (!table_find_copy(pValue->u.pTable, pszKey, &entry))
        return ERROR_FILE_NOT_FOUND;
    *phChild = (HTOMLNODE)entry.pValue;
    return NO_ERROR;
}

/**
 * @brief Query a table entry by index.
 *
 * @param[in]  hNode          Node handle (table). Not NULLHANDLE.
 * @param[in]  ulIndex        Entry index.
 * @param[out] pszKeyBuffer   Key output buffer. Not NULL.
 * @param[in]  ulKeyBufSize   Size of pszKeyBuffer in bytes.
 * @param[out] pulKeyUsed     Optional. May be NULL.
 * @param[out] phChild        Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlNodeGetTableEntryByIndex(HTOMLNODE hNode,
                                             ULONG ulIndex,
                                             PSZ pszKeyBuffer,
                                             ULONG ulKeyBufSize,
                                             PULONG pulKeyUsed,
                                             HTOMLNODE *phChild) {
    PTOMLVALUE pValue = as_node(hNode);
    TOMLENTRY entry;
    size_t cbKeyLen;
    ULONG ulCount = 0;

    if (!pValue || !pszKeyBuffer || !phChild) return ERROR_INVALID_PARAMETER;
    *phChild = NULLHANDLE;
    if (pValue->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
    if (VectorGetCount(pValue->u.pTable->hEntries, &ulCount) != NO_ERROR)
        return ERROR_INVALID_HANDLE;
    if (ulIndex >= ulCount) return ERROR_NO_MORE_ITEMS;
    if (VectorGetItem(pValue->u.pTable->hEntries, ulIndex, &entry,
                      (ULONG)sizeof(entry), NULL) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;

    cbKeyLen = strlen(entry.pszKey);
    if (ulKeyBufSize < cbKeyLen + 1) {
        if (pulKeyUsed) *pulKeyUsed = (ULONG)cbKeyLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszKeyBuffer, entry.pszKey, cbKeyLen);
    pszKeyBuffer[cbKeyLen] = '\0';
    if (pulKeyUsed) *pulKeyUsed = (ULONG)cbKeyLen;
    *phChild = (HTOMLNODE)entry.pValue;
    return NO_ERROR;
}

/* ==================================================================
 * Find API
 * ================================================================== */

/**
 * @brief Wildcard matcher for Find patterns.
 *
 * @param[in] pszPat  Pattern. Not NULL.
 * @param[in] pszTxt  Text. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int glob_match(PCSZ pszPat, PCSZ pszTxt) {
    while (*pszPat) {
        if (*pszPat == '*') {
            pszPat++;
            if (!*pszPat) return 1;
            while (*pszTxt) {
                if (glob_match(pszPat, pszTxt)) return 1;
                pszTxt++;
            }
            return glob_match(pszPat, pszTxt);
        }
        if (*pszPat == '?') {
            if (!*pszTxt) return 0;
            pszPat++; pszTxt++;
            continue;
        }
        if (*pszPat != *pszTxt) return 0;
        pszPat++; pszTxt++;
    }
    return *pszTxt == '\0';
}

/**
 * @brief Find the next entry in a table matching a pattern.
 *
 * @param[in]  pTable      Table. Not NULL.
 * @param[in]  ulStart     First index to try.
 * @param[in]  pszPattern  Pattern. Not NULL.
 * @param[out] pulNext     Next index. Not NULL.
 * @param[out] ppszKey     Receiver for the key. Not NULL.
 * @param[out] ppValue     Receiver for the value. Not NULL.
 *
 * @return 0 on success, -1 if no match.
 */
static int find_next_match(PTOMLTABLE pTable, ULONG ulStart,
                           PCSZ pszPattern, PULONG pulNext,
                           PCSZ *ppszKey, PTOMLVALUE *ppValue) {
    ULONG ulIdx;
    ULONG ulCount = 0;
    if (VectorGetCount(pTable->hEntries, &ulCount) != NO_ERROR) return -1;
    for (ulIdx = ulStart; ulIdx < ulCount; ulIdx++) {
        TOMLENTRY entry;
        if (VectorGetItem(pTable->hEntries, ulIdx, &entry,
                          (ULONG)sizeof(entry), NULL) != NO_ERROR)
            return -1;
        if (glob_match(pszPattern, entry.pszKey)) {
            *pulNext = ulIdx + 1;
            *ppszKey = entry.pszKey;
            *ppValue = entry.pValue;
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Start enumerating entries in a table.
 *
 * @param[in]  hToml       Handle. Not NULLHANDLE.
 * @param[in]  pszPath     Path to the table. Not NULL.
 * @param[in]  pszPattern  Pattern. Not NULL.
 * @param[out] phFind      Cursor receiver. Not NULL.
 * @param[out] pulType     Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindFirst(HTOMLDOC hToml, PCSZ pszPath,
                              PCSZ pszPattern, HTOMLFIND *phFind,
                              PULONG pulType) {
    PTOMLDOC pDoc = as_doc(hToml);
    PTOMLVALUE pValue;
    PTOMLTABLE pTable;
    PTOMLFIND pFind;
    PCSZ pszKey = NULL;
    PTOMLVALUE pVal = NULL;
    ULONG ulNext = 0;

    if (!pDoc || !pszPattern || !phFind) return ERROR_INVALID_PARAMETER;
    *phFind = NULLHANDLE;

    if (pszPath && *pszPath) {
        pValue = find_path(pDoc->pRoot, pszPath);
        if (!pValue) return ERROR_FILE_NOT_FOUND;
        if (pValue->ulType != TOML_TYPE_TABLE)
            return TOML_ERROR_TYPE_MISMATCH;
        pTable = pValue->u.pTable;
    } else {
        pTable = pDoc->pRoot;
    }
    if (find_next_match(pTable, 0, pszPattern, &ulNext,
                        &pszKey, &pVal) != 0)
        return ERROR_NO_MORE_ITEMS;

    pFind = (PTOMLFIND)calloc(1, sizeof(TOMLFIND));
    if (!pFind) return ERROR_NOT_ENOUGH_MEMORY;
    pFind->pDoc = pDoc;
    pFind->pTable = pTable;
    pFind->ulCurrent = ulNext;
    pFind->pszPattern = strdup(pszPattern);
    if (!pFind->pszPattern) { free(pFind); return ERROR_NOT_ENOUGH_MEMORY; }
    pFind->pNext = pDoc->pFirstFind;
    pFind->pszCurrentKey = pszKey;
    pFind->pCurrentValue = pVal;
    pDoc->pFirstFind = pFind;

    *phFind = (HTOMLFIND)pFind;
    if (pulType) *pulType = pVal->ulType;
    return NO_ERROR;
}

/**
 * @brief Advance the cursor to the next matching entry.
 *
 * @param[in]  hFind    Cursor. Not NULLHANDLE.
 * @param[out] pulType  Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindNext(HTOMLFIND hFind, PULONG pulType) {
    PTOMLFIND pFind = as_find(hFind);
    PCSZ pszKey = NULL;
    PTOMLVALUE pVal = NULL;
    ULONG ulNext = 0;
    if (!pFind) return ERROR_INVALID_HANDLE;
    if (find_next_match(pFind->pTable, pFind->ulCurrent, pFind->pszPattern,
                        &ulNext, &pszKey, &pVal) != 0)
        return ERROR_NO_MORE_ITEMS;
    pFind->ulCurrent = ulNext;
    pFind->pszCurrentKey = pszKey;
    pFind->pCurrentValue = pVal;
    if (pulType) *pulType = pVal->ulType;
    return NO_ERROR;
}

/**
 * @brief Retrieve the key of the current entry.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindKey(HTOMLFIND hFind,
                            PSZ pszBuffer, ULONG ulBufSize,
                            PULONG pulSize) {
    PTOMLFIND pFind = as_find(hFind);
    if (!pFind || !pszBuffer) return ERROR_INVALID_PARAMETER;
    if (!pFind->pszCurrentKey) return ERROR_FILE_NOT_FOUND;
    return copy_string_out(pFind->pszCurrentKey, pszBuffer, ulBufSize,
                           pulSize);
}

/**
 * @brief Retrieve the string value of the current entry.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindString(HTOMLFIND hFind,
                               PSZ pszBuffer, ULONG ulBufSize,
                               PULONG pulSize) {
    PTOMLFIND pFind = as_find(hFind);
    if (!pFind || !pszBuffer) return ERROR_INVALID_PARAMETER;
    if (!pFind->pCurrentValue) return ERROR_FILE_NOT_FOUND;
    if (pFind->pCurrentValue->ulType != TOML_TYPE_STRING &&
        pFind->pCurrentValue->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;
    return copy_string_out(pFind->pCurrentValue->u.pszString,
                           pszBuffer, ulBufSize, pulSize);
}

/**
 * @brief Retrieve the integer value of the current entry.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindInteger(HTOMLFIND hFind, PLONGLONG pllValue) {
    PTOMLFIND pFind = as_find(hFind);
    if (!pFind || !pllValue) return ERROR_INVALID_PARAMETER;
    if (!pFind->pCurrentValue) return ERROR_FILE_NOT_FOUND;
    if (pFind->pCurrentValue->ulType != TOML_TYPE_INTEGER)
        return TOML_ERROR_TYPE_MISMATCH;
    *pllValue = pFind->pCurrentValue->u.llInteger;
    return NO_ERROR;
}

/**
 * @brief Retrieve the floating-point value of the current entry.
 *
 * @param[in]  hFind       Cursor. Not NULLHANDLE.
 * @param[out] pdblValue   Receiver. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindFloat(HTOMLFIND hFind, double *pdblValue) {
    PTOMLFIND pFind = as_find(hFind);
    if (!pFind || !pdblValue) return ERROR_INVALID_PARAMETER;
    if (!pFind->pCurrentValue) return ERROR_FILE_NOT_FOUND;
    if (pFind->pCurrentValue->ulType != TOML_TYPE_FLOAT)
        return TOML_ERROR_TYPE_MISMATCH;
    *pdblValue = pFind->pCurrentValue->u.dblFloat;
    return NO_ERROR;
}

/**
 * @brief Retrieve the boolean value of the current entry.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pfValue   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindBoolean(HTOMLFIND hFind, PBOOL pfValue) {
    PTOMLFIND pFind = as_find(hFind);
    if (!pFind || !pfValue) return ERROR_INVALID_PARAMETER;
    if (!pFind->pCurrentValue) return ERROR_FILE_NOT_FOUND;
    if (pFind->pCurrentValue->ulType != TOML_TYPE_BOOLEAN)
        return TOML_ERROR_TYPE_MISMATCH;
    *pfValue = pFind->pCurrentValue->u.fBoolean;
    return NO_ERROR;
}

/**
 * @brief Close an enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is a no-op.
 *
 * @return APIRET
 */
APIRET APIENTRY TomlFindClose(HTOMLFIND hFind) {
    PTOMLFIND pFind = as_find(hFind);
    PTOMLFIND *pp;
    if (!pFind) return NO_ERROR;
    pp = &pFind->pDoc->pFirstFind;
    while (*pp) {
        if (*pp == pFind) { *pp = pFind->pNext; break; }
        pp = &(*pp)->pNext;
    }
    free(pFind->pszPattern);
    free(pFind);
    return NO_ERROR;
}
