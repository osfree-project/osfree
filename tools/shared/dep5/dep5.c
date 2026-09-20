/* dep5.c - Debian Copyright Format 1.0 (DEP5) parser
 * (C89 + Watcom extensions) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dep5.h"
#include "dep5_internal.h"

/**
 * @file dep5.c
 * @brief Implementation of the DEP5 parser.
 *
 * Conforms to:
 *   - https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
 */

/* ==================================================================
 * Small helpers
 * ================================================================== */

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

/* ==================================================================
 * String buffer accumulator
 * ================================================================== */

/**
 * @struct _SBUF
 * @brief Growable string buffer.
 */
typedef struct _SBUF {
    PSZ    pszBuf;  /**< Backing storage, or NULL. */
    size_t cbCap;   /**< Allocated bytes.          */
    size_t cbLen;   /**< Used bytes, excluding NUL.*/
} SBUF;

/**
 * @brief Initialize a string buffer with a small initial capacity.
 *
 * @param[out] pBuf  Buffer to initialize. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int sbuf_init(SBUF *pBuf) {
    pBuf->cbCap = 64;
    pBuf->cbLen = 0;
    pBuf->pszBuf = (PSZ)malloc(pBuf->cbCap);
    if (!pBuf->pszBuf) return -1;
    pBuf->pszBuf[0] = '\0';
    return 0;
}

/**
 * @brief Release the backing storage of a string buffer.
 *
 * The structure is reset to an empty state; the buffer can be
 * reinitialized later.
 *
 * @param[in,out] pBuf  Buffer to free. Not NULL.
 */
static void sbuf_free(SBUF *pBuf) {
    free(pBuf->pszBuf);
    pBuf->pszBuf = NULL;
    pBuf->cbCap = 0;
    pBuf->cbLen = 0;
}

/**
 * @brief Append a byte range to a string buffer.
 *
 * The buffer is grown as needed. A NUL terminator is always kept
 * after the appended data.
 *
 * @param[in,out] pBuf    Buffer. Not NULL.
 * @param[in]     pszData Bytes to append. Not NULL.
 * @param[in]     cbLen   Number of bytes.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int sbuf_put(SBUF *pBuf, PCSZ pszData, size_t cbLen) {
    if (pBuf->cbLen + cbLen + 1 > pBuf->cbCap) {
        size_t cbNewCap = pBuf->cbCap * 2 + cbLen + 64;
        PSZ pszNew = (PSZ)realloc(pBuf->pszBuf, cbNewCap);
        if (!pszNew) return -1;
        pBuf->pszBuf = pszNew;
        pBuf->cbCap = cbNewCap;
    }
    memcpy(pBuf->pszBuf + pBuf->cbLen, pszData, cbLen);
    pBuf->cbLen += cbLen;
    pBuf->pszBuf[pBuf->cbLen] = '\0';
    return 0;
}

/**
 * @brief Append one character to a string buffer.
 *
 * @param[in,out] pBuf  Buffer. Not NULL.
 * @param[in]     ch    Character.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int sbuf_putc(SBUF *pBuf, CHAR ch) {
    return sbuf_put(pBuf, &ch, 1);
}

/**
 * @brief Append a NUL-terminated string to a string buffer.
 *
 * @param[in,out] pBuf    Buffer. Not NULL.
 * @param[in]     pszStr  String. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int sbuf_puts(SBUF *pBuf, PCSZ pszStr) {
    return sbuf_put(pBuf, pszStr, strlen(pszStr));
}

/* ==================================================================
 * Field value normalization (Policy §5.6.13)
 *
 * Applied to every field value at parse time:
 *   - A line consisting of a single '.' is converted into a blank
 *     line (Policy §5.6.13, formatted text rule).
 *   - Trailing whitespace on any line is removed.
 * ================================================================== */

/**
 * @brief Normalize a field value in place.
 *
 * Replaces a line consisting of a single '.' with a blank line,
 * and removes trailing whitespace on every line. The result
 * replaces the contents of the buffer.
 *
 * @param[in,out] pBuf  Buffer to normalize. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int normalize_field_value(SBUF *pBuf) {
    PSZ pszSrc = pBuf->pszBuf;
    size_t cbSrcLen = pBuf->cbLen;
    SBUF out;
    size_t i = 0;

    if (cbSrcLen == 0) return 0;
    if (sbuf_init(&out) != 0) return -1;

    while (i < cbSrcLen) {
        size_t cbLineEnd = i;
        size_t cbLineTrimmed;
        while (cbLineEnd < cbSrcLen && pszSrc[cbLineEnd] != '\n')
            cbLineEnd++;
        cbLineTrimmed = cbLineEnd;
        while (cbLineTrimmed > i &&
               (pszSrc[cbLineTrimmed-1] == ' ' ||
                pszSrc[cbLineTrimmed-1] == '\t'))
            cbLineTrimmed--;
        if (cbLineEnd - i == 1 && pszSrc[i] == '.') {
            /* Single dot -> blank line: skip content. */
        } else {
            if (cbLineTrimmed > i) {
                if (sbuf_put(&out, pszSrc + i,
                             cbLineTrimmed - i) != 0) {
                    sbuf_free(&out); return -1;
                }
            }
        }
        if (cbLineEnd < cbSrcLen) {
            if (sbuf_putc(&out, '\n') != 0) {
                sbuf_free(&out); return -1;
            }
        }
        i = cbLineEnd + 1;
    }

    sbuf_free(pBuf);
    pBuf->pszBuf = out.pszBuf;
    pBuf->cbCap = out.cbCap;
    pBuf->cbLen = out.cbLen;
    return 0;
}

/* ==================================================================
 * Stanza builder
 * ================================================================== */

/**
 * @brief Grow the field array of a stanza if needed.
 *
 * @param[in,out] pStanza  Stanza. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int stanza_grow(PDEP5STANZA pStanza) {
    ULONG ulNewCap;
    PDEP5FIELD paNew;
    if (pStanza->ulCount < pStanza->ulCapacity) return 0;
    ulNewCap = pStanza->ulCapacity ? pStanza->ulCapacity * 2 : 8;
    paNew = (PDEP5FIELD)realloc(pStanza->paFields,
                                (size_t)ulNewCap * sizeof(DEP5FIELD));
    if (!paNew) return -1;
    pStanza->paFields = paNew;
    pStanza->ulCapacity = ulNewCap;
    return 0;
}

/**
 * @brief Append a zero-filled field slot to a stanza.
 *
 * @param[in,out] pStanza  Stanza. Not NULL.
 *
 * @return Pointer to the new field, or NULL on allocation failure.
 */
static PDEP5FIELD stanza_add_field(PDEP5STANZA pStanza) {
    PDEP5FIELD pField;
    if (stanza_grow(pStanza) != 0) return NULL;
    pField = &pStanza->paFields[pStanza->ulCount++];
    memset(pField, 0, sizeof(*pField));
    return pField;
}

/**
 * @brief Find a field by name (case-insensitive).
 *
 * @param[in] pStanza  Stanza. May be NULL.
 * @param[in] pszName  Field name. Not NULL.
 *
 * @return Pointer to the field, or NULL if not found.
 */
static PDEP5FIELD stanza_find_field(PDEP5STANZA pStanza, PCSZ pszName) {
    ULONG ulIdx;
    if (!pStanza) return NULL;
    for (ulIdx = 0; ulIdx < pStanza->ulCount; ulIdx++) {
        if (strcasecmp(pStanza->paFields[ulIdx].pszName, pszName) == 0)
            return &pStanza->paFields[ulIdx];
    }
    return NULL;
}

/**
 * @brief Release all memory owned by one stanza.
 *
 * @param[in,out] pStanza  Stanza. May be NULL.
 */
static void stanza_free(PDEP5STANZA pStanza) {
    ULONG ulIdx;
    if (!pStanza) return;
    for (ulIdx = 0; ulIdx < pStanza->ulCount; ulIdx++) {
        free(pStanza->paFields[ulIdx].pszName);
        free(pStanza->paFields[ulIdx].pszValue);
    }
    free(pStanza->paFields);
    memset(pStanza, 0, sizeof(*pStanza));
}

/* ==================================================================
 * Document builder
 * ================================================================== */

/**
 * @brief Grow the stanza array of a document if needed.
 *
 * @param[in,out] pDoc  Document. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int doc_grow(PDEP5DOC pDoc) {
    ULONG ulNewCap;
    PDEP5STANZA paNew;
    if (pDoc->ulCount < pDoc->ulCapacity) return 0;
    ulNewCap = pDoc->ulCapacity ? pDoc->ulCapacity * 2 : 8;
    paNew = (PDEP5STANZA)realloc(pDoc->paStanzas,
                                 (size_t)ulNewCap * sizeof(DEP5STANZA));
    if (!paNew) return -1;
    pDoc->paStanzas = paNew;
    pDoc->ulCapacity = ulNewCap;
    return 0;
}

/**
 * @brief Append a zero-filled stanza slot to a document.
 *
 * @param[in,out] pDoc  Document. Not NULL.
 *
 * @return Pointer to the new stanza, or NULL on allocation failure.
 */
static PDEP5STANZA doc_add_stanza(PDEP5DOC pDoc) {
    PDEP5STANZA pStanza;
    if (doc_grow(pDoc) != 0) return NULL;
    pStanza = &pDoc->paStanzas[pDoc->ulCount++];
    memset(pStanza, 0, sizeof(*pStanza));
    return pStanza;
}

/**
 * @brief Release a document and all owned stanzas.
 *
 * @param[in] pDoc  Document. May be NULL.
 */
static void doc_free(PDEP5DOC pDoc) {
    ULONG ulIdx;
    if (!pDoc) return;
    for (ulIdx = 0; ulIdx < pDoc->ulCount; ulIdx++)
        stanza_free(&pDoc->paStanzas[ulIdx]);
    free(pDoc->paStanzas);
    free(pDoc);
}

/* ==================================================================
 * Format validation (DEP5 §6.1)
 *
 * Both http and https URLs are valid and refer to the same spec.
 * Trailing slash is optional.
 * ================================================================== */

/**
 * @brief Query whether a Format field value is valid.
 *
 * Both http and https forms of the DEP5 URL are accepted, with or
 * without a trailing slash, and with any whitespace after.
 *
 * @param[in] pszValue  Field value. May be NULL.
 *
 * @return 1 if valid, 0 otherwise.
 */
static int format_is_valid(PCSZ pszValue) {
    static PCSZ apszPrefixes[] = {
        "https://www.debian.org/doc/packaging-manuals/copyright-format/1.0",
        "http://www.debian.org/doc/packaging-manuals/copyright-format/1.0",
        NULL
    };
    int i;
    if (!pszValue) return 0;
    for (i = 0; apszPrefixes[i]; i++) {
        size_t cbPrefixLen = strlen(apszPrefixes[i]);
        if (strncmp(pszValue, apszPrefixes[i], cbPrefixLen) == 0) {
            CHAR ch = pszValue[cbPrefixLen];
            if (ch == '\0' || ch == '/' || ch == ' ' || ch == '\t' ||
                ch == '\n' || ch == '\r')
                return 1;
        }
    }
    return 0;
}

/* ==================================================================
 * Files pattern parser (DEP5 §6.9)
 *
 * Patterns are whitespace-separated tokens. Each token may contain
 * escape sequences \* \? \\ — other escapes are errors. The space
 * character cannot be escaped.
 * ================================================================== */

/**
 * @brief Unescape one pattern token.
 *
 * Recognized escape sequences: \*, \?, \\. Any other escape is
 * treated as an error.
 *
 * @param[in]  pszToken  Token. Not NULL.
 * @param[in]  cbToken   Token length.
 * @param[out] ppszOut   Receiver for the malloc'd unescaped token.
 *                       Not NULL.
 *
 * @return 0 on success, -1 on invalid escape or allocation failure.
 */
static int unescape_pattern(PCSZ pszToken, size_t cbToken,
                            PSZ *ppszOut) {
    SBUF buf;
    size_t i;
    if (sbuf_init(&buf) != 0) return -1;
    for (i = 0; i < cbToken; i++) {
        CHAR ch = pszToken[i];
        if (ch == '\\') {
            if (i + 1 >= cbToken) goto fail;
            i++;
            switch (pszToken[i]) {
                case '*':  ch = '*';  break;
                case '?':  ch = '?';  break;
                case '\\': ch = '\\'; break;
                default: goto fail;
            }
        }
        if (sbuf_putc(&buf, ch) != 0) goto fail;
    }
    *ppszOut = buf.pszBuf;
    return 0;
fail:
    sbuf_free(&buf);
    return -1;
}

/**
 * @brief Parse a Files field into an array of unescaped patterns.
 *
 * @param[in]  pszValue     Files field value. Not NULL.
 * @param[out] ppapszOut    Receiver for the malloc'd array of
 *                          patterns. Not NULL.
 * @param[out] pulCount     Receiver for the pattern count. Not NULL.
 *
 * @return 0 on success, -1 on invalid escape or allocation failure.
 */
static int parse_files_patterns(PCSZ pszValue, PSZ **ppapszOut,
                                PULONG pulCount) {
    PSZ *papszPatterns = NULL;
    ULONG ulCount = 0, ulCap = 4;
    PCSZ pszPos = pszValue;

    papszPatterns = (PSZ*)malloc(ulCap * sizeof(PSZ));
    if (!papszPatterns) return -1;

    while (*pszPos) {
        PCSZ pszStart;
        size_t cbTokLen;
        PSZ pszUnescaped;

        while (*pszPos && (*pszPos == ' ' || *pszPos == '\t' ||
                           *pszPos == '\n' || *pszPos == '\r'))
            pszPos++;
        if (!*pszPos) break;
        pszStart = pszPos;
        while (*pszPos && *pszPos != ' ' && *pszPos != '\t' &&
               *pszPos != '\n' && *pszPos != '\r')
            pszPos++;
        cbTokLen = (size_t)(pszPos - pszStart);

        if (unescape_pattern(pszStart, cbTokLen, &pszUnescaped) != 0) {
            ULONG ulK;
            for (ulK = 0; ulK < ulCount; ulK++) free(papszPatterns[ulK]);
            free(papszPatterns);
            return -1;
        }
        if (ulCount >= ulCap) {
            PSZ *papszNew;
            ulCap *= 2;
            papszNew = (PSZ*)realloc(papszPatterns, ulCap * sizeof(PSZ));
            if (!papszNew) { free(pszUnescaped); goto fail; }
            papszPatterns = papszNew;
        }
        papszPatterns[ulCount++] = pszUnescaped;
    }

    *ppapszOut = papszPatterns;
    *pulCount = ulCount;
    return 0;

fail:
    {
        ULONG ulK;
        for (ulK = 0; ulK < ulCount; ulK++) free(papszPatterns[ulK]);
        free(papszPatterns);
    }
    return -1;
}

/* ==================================================================
 * License field split
 *
 * Synopsis = first line (up to first '\n'). Body = rest.
 * Normalization of '.' -> blank line and trailing whitespace is
 * already done by normalize_field_value at parse time.
 * ================================================================== */

/**
 * @brief Split a License field value into synopsis and body.
 *
 * The synopsis is the first line, with trailing whitespace
 * removed. The body is everything after the first newline.
 *
 * @param[in]  pszValue      Field value. Not NULL.
 * @param[out] ppszShortName Receiver for the malloc'd synopsis.
 *                           Not NULL. Set to NULL on error.
 * @param[out] ppszBody      Receiver for the malloc'd body, or
 *                           NULL if there is no body. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int split_license(PSZ pszValue, PSZ *ppszShortName,
                         PSZ *ppszBody) {
    PCSZ pszPos = pszValue;
    PCSZ pszEol;

    *ppszShortName = NULL;
    *ppszBody = NULL;

    if (!pszPos) return -1;
    pszEol = strchr(pszPos, '\n');
    if (!pszEol) {
        *ppszShortName = strdup(pszPos);
        return (*ppszShortName) ? 0 : -1;
    }

    {
        size_t cbShortLen = (size_t)(pszEol - pszPos);
        while (cbShortLen > 0 && (pszPos[cbShortLen-1] == ' ' ||
                                  pszPos[cbShortLen-1] == '\t'))
            cbShortLen--;
        *ppszShortName = (PSZ)malloc(cbShortLen + 1);
        if (!*ppszShortName) return -1;
        memcpy(*ppszShortName, pszPos, cbShortLen);
        (*ppszShortName)[cbShortLen] = '\0';
    }
    if (pszEol[1] == '\0') {
        /* No body. */
        return 0;
    }
    *ppszBody = strdup(pszEol + 1);
    if (!*ppszBody) {
        free(*ppszShortName);
        *ppszShortName = NULL;
        return -1;
    }
    return 0;
}

/* ==================================================================
 * License short-name helpers
 * ================================================================== */

/**
 * @brief Query whether a License field's synopsis equals a name.
 *
 * Comparison is case-insensitive. Trailing whitespace on the
 * synopsis is ignored.
 *
 * @param[in] pField   License field. May be NULL.
 * @param[in] pszName  Name to compare. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int license_shortname_is(PDEP5FIELD pField, PCSZ pszName) {
    PCSZ pszValue;
    PCSZ pszEol;
    size_t cbSynLen;
    size_t cbNameLen;
    size_t i;
    if (!pField || !pField->pszValue) return 0;
    pszValue = pField->pszValue;
    pszEol = strchr(pszValue, '\n');
    cbSynLen = pszEol ? (size_t)(pszEol - pszValue) : strlen(pszValue);
    while (cbSynLen > 0 && (pszValue[cbSynLen-1] == ' ' ||
                            pszValue[cbSynLen-1] == '\t'))
        cbSynLen--;
    cbNameLen = strlen(pszName);
    if (cbNameLen != cbSynLen) return 0;
    for (i = 0; i < cbSynLen; i++) {
        if (tolower((UCHAR)pszValue[i]) !=
            tolower((UCHAR)pszName[i]))
            return 0;
    }
    return 1;
}

/**
 * @brief Query whether a License field has a body after the first
 *        newline.
 *
 * @param[in] pField  License field. May be NULL.
 *
 * @return 1 if a body is present, 0 otherwise.
 */
static int license_has_body(PDEP5FIELD pField) {
    PCSZ pszEol;
    if (!pField || !pField->pszValue) return 0;
    pszEol = strchr(pField->pszValue, '\n');
    if (!pszEol) return 0;
    return pszEol[1] != '\0';
}

/* ==================================================================
 * Stanza classification and validation
 * ================================================================== */

/**
 * @brief Classify a stanza by its fields.
 *
 * The first stanza in a file is always the header. Subsequent
 * stanzas are classified by the presence of a "Files" or "License"
 * field.
 *
 * @param[in] pStanza   Stanza. Not NULL.
 * @param[in] fIsFirst  TRUE if this is the first stanza.
 *
 * @return One of DEP5_STANZA_HEADER, DEP5_STANZA_FILES,
 *         DEP5_STANZA_LICENSE, or 0 if unclassified.
 */
static ULONG classify_stanza(PDEP5STANZA pStanza, int fIsFirst)
{
    if (fIsFirst) return DEP5_STANZA_HEADER;
    if (stanza_find_field(pStanza, "Files")) return DEP5_STANZA_FILES;
    if (stanza_find_field(pStanza, "License")) return DEP5_STANZA_LICENSE;
    return 0;
}

/**
 * @brief Validate the header stanza.
 *
 * Requires the mandatory "Format" field with a valid DEP5 URL.
 *
 * @param[in] pStanza  Stanza. Not NULL.
 *
 * @return 0 if valid, -1 otherwise.
 */
static int validate_header(PDEP5STANZA pStanza) {
    PDEP5FIELD pField = stanza_find_field(pStanza, "Format");
    if (!pField || !pField->pszValue || !pField->pszValue[0]) return -1;
    if (!format_is_valid(pField->pszValue)) return -1;
    return 0;
}

/**
 * @brief Validate a Files stanza.
 *
 * Requires Files, Copyright and License. A public-domain license
 * must have a body.
 *
 * @param[in] pStanza  Stanza. Not NULL.
 *
 * @return 0 if valid, -1 otherwise.
 */
static int validate_files(PDEP5STANZA pStanza) {
    PDEP5FIELD pField;
    pField = stanza_find_field(pStanza, "Files");
    if (!pField || !pField->pszValue || !pField->pszValue[0]) return -1;
    pField = stanza_find_field(pStanza, "Copyright");
    if (!pField || !pField->pszValue || !pField->pszValue[0]) return -1;
    pField = stanza_find_field(pStanza, "License");
    if (!pField || !pField->pszValue || !pField->pszValue[0]) return -1;
    /* DEP5 §7.1.1: public-domain requires body. */
    if (license_shortname_is(pField, "public-domain") &&
        !license_has_body(pField))
        return -1;
    return 0;
}

/**
 * @brief Validate a stand-alone License stanza.
 *
 * Requires the License field. A public-domain license must have a
 * body.
 *
 * @param[in] pStanza  Stanza. Not NULL.
 *
 * @return 0 if valid, -1 otherwise.
 */
static int validate_license(PDEP5STANZA pStanza) {
    PDEP5FIELD pField = stanza_find_field(pStanza, "License");
    if (!pField || !pField->pszValue || !pField->pszValue[0]) return -1;
    /* DEP5 §7.1.1: public-domain requires body. */
    if (license_shortname_is(pField, "public-domain") &&
        !license_has_body(pField))
        return -1;
    return 0;
}

/* ==================================================================
 * File splitting into stanzas and fields
 *
 * A stanza is a sequence of fields separated by blank lines.
 * A field is "Name: value" starting at column 0, followed by
 * continuation lines beginning with whitespace.
 * Lines whose first non-whitespace character is '#' are comments.
 *
 * Per Debian Policy §5.6.13: exactly one leading space of a
 * continuation line is stripped; the rest is content.
 * ================================================================== */

/**
 * @brief Parse a whole DEP5 text into a document structure.
 *
 * Splits the text into stanzas and fields, normalizes field
 * values, classifies each stanza, and validates the result.
 *
 * @param[in]  pszText    NUL-terminated file content. Not NULL.
 * @param[out] ppDoc      Receiver for the document. Not NULL.
 * @param[out] ppszError  Optional. May be NULL. On failure receives
 *                        a static error description.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Allocation failure.
 * @retval DEP5_ERROR_INVALID_SYNTAX    Syntax or structural error.
 */
static APIRET parse_document(PCSZ pszText, PDEP5DOC *ppDoc,
                             PCSZ *ppszError) {
    PDEP5DOC pDoc;
    PDEP5STANZA pCurrent = NULL;
    PDEP5FIELD pCurrentField = NULL;
    SBUF value_buf;
    int fHaveField = 0;
    PCSZ pszPos = pszText;
    APIRET rc = NO_ERROR;

    pDoc = (PDEP5DOC)calloc(1, sizeof(DEP5DOC));
    if (!pDoc) return ERROR_NOT_ENOUGH_MEMORY;
    pDoc->ulHeaderIndex = DEP5_NO_HEADER;

    if (sbuf_init(&value_buf) != 0) {
        free(pDoc);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Skip UTF-8 BOM. */
    if ((UCHAR)pszPos[0] == 0xEF &&
        (UCHAR)pszPos[1] == 0xBB &&
        (UCHAR)pszPos[2] == 0xBF) pszPos += 3;

    for (;;) {
        PCSZ pszLineStart = pszPos;
        PCSZ pszLineEnd;
        size_t cbLineLen;
        int fBlank, fComment, fContinuation;

        /* Find end of the current line. */
        while (*pszPos && *pszPos != '\n') pszPos++;
        pszLineEnd = pszPos;
        cbLineLen = (size_t)(pszLineEnd - pszLineStart);

        /* Strip trailing CR (CRLF handling). */
        if (cbLineLen > 0 && pszLineStart[cbLineLen-1] == '\r')
            cbLineLen--;

        /* Classify the line. */
        fBlank = 1;
        {
            size_t i;
            for (i = 0; i < cbLineLen; i++) {
                if (pszLineStart[i] != ' ' && pszLineStart[i] != '\t') {
                    fBlank = 0;
                    break;
                }
            }
        }
        fComment = (!fBlank && pszLineStart[0] == '#');
        fContinuation = (!fBlank && !fComment &&
                         (pszLineStart[0] == ' ' ||
                          pszLineStart[0] == '\t'));

        if (fBlank) {
            /* End of current field and stanza. */
            if (fHaveField) {
                if (pCurrentField && pCurrent) {
                    if (normalize_field_value(&value_buf) != 0) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto fail;
                    }
                    pCurrentField->pszValue = strdup(value_buf.pszBuf);
                    if (!pCurrentField->pszValue) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto fail;
                    }
                }
                fHaveField = 0;
                pCurrentField = NULL;
                value_buf.cbLen = 0;
                if (value_buf.pszBuf) value_buf.pszBuf[0] = '\0';
            }
            if (pCurrent) {
                pCurrent = NULL;
            }
        } else if (fComment) {
            /* Ignore. */
        } else if (fContinuation && pCurrentField) {
            /* Continuation: strip exactly one leading space. */
            PCSZ pszContent = pszLineStart;
            size_t cbContentLen = cbLineLen;
            if (cbContentLen > 0 && (pszContent[0] == ' ' ||
                                     pszContent[0] == '\t')) {
                pszContent++;
                cbContentLen--;
            }
            if (value_buf.cbLen > 0) {
                if (sbuf_putc(&value_buf, '\n') != 0) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
            }
            if (cbContentLen > 0) {
                if (sbuf_put(&value_buf, pszContent,
                             cbContentLen) != 0) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
            }
        } else if (fContinuation && !pCurrentField) {
            /* Continuation without a preceding field: syntax error. */
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "continuation line without a field";
            goto fail;
        } else {
            /* Start of a new field. */
            PCSZ pszColon = NULL;
            size_t i;
            PDEP5FIELD pField;

            for (i = 0; i < cbLineLen; i++) {
                if (pszLineStart[i] == ':') {
                    pszColon = pszLineStart + i;
                    break;
                }
            }
            if (!pszColon) {
                rc = DEP5_ERROR_INVALID_SYNTAX;
                if (ppszError) *ppszError = "field line without colon";
                goto fail;
            }

            /* Close previous field. */
            if (fHaveField && pCurrentField && pCurrent) {
                if (normalize_field_value(&value_buf) != 0) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
                pCurrentField->pszValue = strdup(value_buf.pszBuf);
                if (!pCurrentField->pszValue) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
            }
            value_buf.cbLen = 0;
            if (value_buf.pszBuf) value_buf.pszBuf[0] = '\0';

            /* Start a new stanza if needed. */
            if (!pCurrent) {
                pCurrent = doc_add_stanza(pDoc);
                if (!pCurrent) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
            }

            /* Extract and validate the field name. */
            {
                size_t cbNameLen = (size_t)(pszColon - pszLineStart);
                ULONG ulK;
                int fDup = 0;
                while (cbNameLen > 0 &&
                       (pszLineStart[cbNameLen-1] == ' ' ||
                        pszLineStart[cbNameLen-1] == '\t'))
                    cbNameLen--;
                if (cbNameLen == 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError) *ppszError = "empty field name";
                    goto fail;
                }
                /* Duplicate check (case-insensitive). */
                for (ulK = 0; ulK < pCurrent->ulCount; ulK++) {
                    if (strlen(pCurrent->paFields[ulK].pszName) == cbNameLen &&
                        strncasecmp(pCurrent->paFields[ulK].pszName,
                                    pszLineStart, cbNameLen) == 0) {
                        fDup = 1; break;
                    }
                }
                if (fDup) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "duplicate field name in stanza";
                    goto fail;
                }
                pField = stanza_add_field(pCurrent);
                if (!pField) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
                pField->pszName = (PSZ)malloc(cbNameLen + 1);
                if (!pField->pszName) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
                memcpy(pField->pszName, pszLineStart, cbNameLen);
                pField->pszName[cbNameLen] = '\0';
            }

            /* Value: everything after colon, with leading whitespace
             * of the first line stripped. */
            {
                PCSZ pszValStart = pszColon + 1;
                size_t cbValLen;
                while (*pszValStart == ' ' || *pszValStart == '\t')
                    pszValStart++;
                cbValLen = (size_t)((pszLineStart + cbLineLen) - pszValStart);
                if (cbValLen > 0) {
                    if (sbuf_put(&value_buf, pszValStart, cbValLen) != 0) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto fail;
                    }
                }
            }

            pCurrentField = pField;
            fHaveField = 1;
        }

        if (*pszPos != '\n') break;
        pszPos++;
    }

    /* Close trailing field. */
    if (fHaveField && pCurrentField && pCurrent) {
        if (normalize_field_value(&value_buf) != 0) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto fail;
        }
        pCurrentField->pszValue = strdup(value_buf.pszBuf);
        if (!pCurrentField->pszValue) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto fail;
        }
    }

    /* Classify and validate stanzas. */
    {
        ULONG ulIdx;
        if (pDoc->ulCount == 0) {
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "file has no stanzas";
            goto fail;
        }
        for (ulIdx = 0; ulIdx < pDoc->ulCount; ulIdx++) {
            PDEP5STANZA pStanza = &pDoc->paStanzas[ulIdx];
            ULONG ulKind = classify_stanza(pStanza, ulIdx == 0);
            if (ulKind == 0) {
                rc = DEP5_ERROR_INVALID_SYNTAX;
                if (ppszError) *ppszError = "unrecognized stanza";
                goto fail;
            }
            pStanza->ulKind = ulKind;
            if (ulKind == DEP5_STANZA_HEADER) {
                if (pDoc->ulHeaderIndex != DEP5_NO_HEADER) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError) *ppszError = "multiple header stanzas";
                    goto fail;
                }
                if (validate_header(pStanza) != 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "invalid header (Format missing or bad)";
                    goto fail;
                }
                pDoc->ulHeaderIndex = ulIdx;
            } else if (ulKind == DEP5_STANZA_FILES) {
                if (validate_files(pStanza) != 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "Files stanza missing "
                                     "Files/Copyright/License "
                                     "or public-domain without body";
                    goto fail;
                }
                pDoc->ulFilesCount++;
            } else if (ulKind == DEP5_STANZA_LICENSE) {
                if (validate_license(pStanza) != 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "License stanza missing License "
                                     "or public-domain without body";
                    goto fail;
                }
                pDoc->ulLicenseCount++;
            }
        }
        if (pDoc->ulHeaderIndex == DEP5_NO_HEADER) {
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "missing header stanza";
            goto fail;
        }
        if (pDoc->ulFilesCount == 0) {
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "no Files stanzas";
            goto fail;
        }
    }

    sbuf_free(&value_buf);
    *ppDoc = pDoc;
    return NO_ERROR;

fail:
    sbuf_free(&value_buf);
    doc_free(pDoc);
    return rc;
}

/* ==================================================================
 * Handle helpers
 * ================================================================== */

/**
 * @brief Translate a public document handle into the internal
 *        pointer.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 */
PDEP5DOC Dep5InternalGetDoc(HDEP5DOC hDoc) { return (PDEP5DOC)hDoc; }

/**
 * @brief Translate a public cursor handle into the internal pointer.
 *
 * @param[in] hFind  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 */
PDEP5FIND Dep5InternalGetFind(HDEP5FIND hFind) { return (PDEP5FIND)hFind; }

/**
 * @brief Find the index of the first stanza of a given kind.
 *
 * @param[in] pDoc   Document. Not NULL.
 * @param[in] ulKind One of DEP5_STANZA_*.
 *
 * @return Stanza index, or DEP5_NO_HEADER if not found.
 */
static ULONG doc_find_first_index(PDEP5DOC pDoc, ULONG ulKind) {
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < pDoc->ulCount; ulIdx++) {
        if (pDoc->paStanzas[ulIdx].ulKind == ulKind) return ulIdx;
    }
    return DEP5_NO_HEADER;
}

/**
 * @brief Find the index of the next stanza of a given kind.
 *
 * @param[in] pDoc   Document. Not NULL.
 * @param[in] ulKind One of DEP5_STANZA_*.
 * @param[in] ulFrom Starting index (exclusive).
 *
 * @return Stanza index, or DEP5_NO_HEADER if not found.
 */
static ULONG doc_find_next_index(PDEP5DOC pDoc, ULONG ulKind,
                                 ULONG ulFrom) {
    ULONG ulIdx;
    for (ulIdx = ulFrom + 1; ulIdx < pDoc->ulCount; ulIdx++) {
        if (pDoc->paStanzas[ulIdx].ulKind == ulKind) return ulIdx;
    }
    return DEP5_NO_HEADER;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Open and parse a debian/copyright file.
 *
 * Reads the file, splits it into stanzas, classifies each stanza,
 * and returns a document handle. All internal buffers are owned by
 * the module and released by Dep5Close.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_PARAMETER      pszPath or phDoc is NULL.
 * @retval ERROR_OPEN_FAILED            File cannot be opened.
 * @retval ERROR_READ_FAULT             Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Memory allocation failure.
 * @retval DEP5_ERROR_INVALID_SYNTAX    Stanza structure is invalid
 *                                      (missing header, missing
 *                                      required fields, unknown
 *                                      stanza kind).
 *
 * @note Ownership of the handle transfers to the caller. It must be
 *       released with Dep5Close.
 * @see Dep5Close
 */
APIRET APIENTRY Dep5Open(PCSZ pszPath, HDEP5DOC *phDoc) {
    PSZ pszText = NULL;
    PDEP5DOC pDoc = NULL;
    PCSZ pszError = NULL;
    APIRET rc;

    if (!pszPath || !phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;

    if (read_file_all(pszPath, &pszText) != 0)
        return ERROR_OPEN_FAILED;

    rc = parse_document(pszText, &pDoc, &pszError);
    free(pszText);
    if (rc != NO_ERROR) return rc;

    *phDoc = (HDEP5DOC)pDoc;
    return NO_ERROR;
}

/**
 * @brief Close a document.
 *
 * Releases all internal buffers, including any active Files and
 * License cursors created from this document. After return the
 * handle is invalid.
 *
 * @param[in] hDoc  Document handle. NULLHANDLE is accepted and
 *                  treated as a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @warning Calling Dep5Close twice with the same handle is
 *          undefined. The caller should set the handle to
 *          NULLHANDLE after close.
 * @see Dep5Open
 */
APIRET APIENTRY Dep5Close(HDEP5DOC hDoc) {
    PDEP5DOC pDoc;
    if (hDoc == NULLHANDLE) return NO_ERROR;
    pDoc = Dep5InternalGetDoc(hDoc);
    if (!pDoc) return ERROR_INVALID_HANDLE;
    doc_free(pDoc);
    return NO_ERROR;
}

/**
 * @brief Retrieve a field value from the header stanza.
 *
 * Supported fields (case-insensitive):
 *   "Format", "Upstream-Name", "Upstream-Contact", "Source",
 *   "Disclaimer", "Comment", "License", "Copyright".
 * Extra fields present in the source are also accessible by their
 * exact name.
 *
 * If the field is multi-line, its lines are joined with '\n'.
 * Whitespace-separated-list and line-based-list fields are
 * returned verbatim (list splitting is the caller's
 * responsibility).
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query
 * only and returns the required size (including NUL) in *pulSize.
 *
 * @param[in]  hDoc       Handle. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL. On success — string
 *                        length without NUL. On BUFFER_OVERFLOW —
 *                        required size including NUL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or field
 *                                  name empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Field not present in the header.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5HeaderGetField(HDEP5DOC hDoc, PCSZ pszField,
                                   PSZ pszBuffer, ULONG ulBufSize,
                                   PULONG pulSize) {
    PDEP5DOC pDoc = Dep5InternalGetDoc(hDoc);
    PDEP5STANZA pStanza;
    PDEP5FIELD pField;
    size_t cbLen;

    if (!pDoc || !pszField || !pszField[0]) return ERROR_INVALID_PARAMETER;
    if (pDoc->ulHeaderIndex == DEP5_NO_HEADER) return ERROR_FILE_NOT_FOUND;
    pStanza = &pDoc->paStanzas[pDoc->ulHeaderIndex];
    pField = stanza_find_field(pStanza, pszField);
    if (!pField || !pField->pszValue) return ERROR_FILE_NOT_FOUND;

    cbLen = strlen(pField->pszValue);
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuffer) return ERROR_INVALID_PARAMETER;
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pField->pszValue, cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/* ------------------------------------------------------------------
 * Files stanza enumeration
 * ------------------------------------------------------------------ */

/**
 * @brief Release per-stanza derived data cached in a cursor.
 *
 * @param[in,out] pFind  Cursor. May be NULL.
 */
static void find_free_cache(PDEP5FIND pFind) {
    ULONG ulIdx;
    if (!pFind) return;
    for (ulIdx = 0; ulIdx < pFind->ulPatternCount; ulIdx++)
        free(pFind->paPatterns[ulIdx]);
    free(pFind->paPatterns);      pFind->paPatterns = NULL;
    pFind->ulPatternCount = 0;
    free(pFind->pszShortName);    pFind->pszShortName = NULL;
    free(pFind->pszLicenseText);  pFind->pszLicenseText = NULL;
}

/**
 * @brief Release the cache of a cursor (internal helper for dep5.c
 *        and dep5_internal.h consumers).
 *
 * @param[in,out] pFind  Cursor. May be NULL.
 */
void Dep5InternalFindFreeCache(PDEP5FIND pFind) { find_free_cache(pFind); }

/**
 * @brief Load and cache derived data for the current stanza.
 *
 * Patterns for Files stanzas, and license synopsis and body for
 * any stanza with a License field.
 *
 * @param[in,out] pFind  Cursor. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int find_load_cache(PDEP5FIND pFind) {
    PDEP5STANZA pStanza = &pFind->pDoc->paStanzas[pFind->ulCurrent];
    PDEP5FIELD pFieldFiles, pFieldLicense;

    find_free_cache(pFind);

    if (pFind->ulKind == DEP5_STANZA_FILES) {
        pFieldFiles = stanza_find_field(pStanza, "Files");
        if (!pFieldFiles || !pFieldFiles->pszValue) return -1;
        if (parse_files_patterns(pFieldFiles->pszValue, &pFind->paPatterns,
                                 &pFind->ulPatternCount) != 0)
            return -1;
    }

    pFieldLicense = stanza_find_field(pStanza, "License");
    if (pFieldLicense && pFieldLicense->pszValue) {
        if (split_license(pFieldLicense->pszValue,
                          &pFind->pszShortName,
                          &pFind->pszLicenseText) != 0)
            return -1;
    }
    return 0;
}

/**
 * @brief Start enumerating Files stanzas.
 *
 * Creates a cursor and positions it on the first Files stanza. The
 * total number of Files stanzas is returned in *pulCount, allowing
 * the caller to pre-allocate memory.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[out] phFind    Cursor receiver. Not NULL. Set to
 *                       NULLHANDLE on error or if the document has
 *                       no Files stanzas.
 * @param[out] pulCount  Optional. May be NULL. On success receives
 *                       the total number of Files stanzas.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phFind is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval ERROR_NO_MORE_ITEMS      Document has no Files stanzas.
 *                                  *phFind = NULLHANDLE.
 *
 * @note Release the cursor with Dep5FilesFindClose (or Dep5Close if
 *       the caller forgot).
 * @see Dep5FilesFindNext, Dep5FilesFindClose
 */
APIRET APIENTRY Dep5FilesFindFirst(HDEP5DOC hDoc, HDEP5FIND *phFind,
                                   PULONG pulCount) {
    PDEP5DOC pDoc = Dep5InternalGetDoc(hDoc);
    PDEP5FIND pFind;
    ULONG ulIdx;

    if (!pDoc || !phFind) return ERROR_INVALID_PARAMETER;
    *phFind = NULLHANDLE;

    ulIdx = doc_find_first_index(pDoc, DEP5_STANZA_FILES);
    if (ulIdx == DEP5_NO_HEADER) return ERROR_NO_MORE_ITEMS;

    pFind = (PDEP5FIND)calloc(1, sizeof(DEP5FIND));
    if (!pFind) return ERROR_NOT_ENOUGH_MEMORY;
    pFind->pDoc = pDoc;
    pFind->ulKind = DEP5_STANZA_FILES;
    pFind->ulCurrent = ulIdx;
    pFind->ulSeen = 1;

    if (find_load_cache(pFind) != 0) {
        find_free_cache(pFind);
        free(pFind);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pulCount) *pulCount = pDoc->ulFilesCount;
    *phFind = (HDEP5FIND)pFind;
    return NO_ERROR;
}

/**
 * @brief Advance the cursor to the next Files stanza.
 *
 * @param[in] hFind  Cursor from Dep5FilesFindFirst. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more Files stanzas.
 *
 * @see Dep5FilesFindFirst, Dep5FilesFindClose
 */
APIRET APIENTRY Dep5FilesFindNext(HDEP5FIND hFind) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    ULONG ulIdx;
    if (!pFind) return ERROR_INVALID_HANDLE;
    if (pFind->ulKind != DEP5_STANZA_FILES) return ERROR_INVALID_HANDLE;

    ulIdx = doc_find_next_index(pFind->pDoc, DEP5_STANZA_FILES,
                                pFind->ulCurrent);
    if (ulIdx == DEP5_NO_HEADER) return ERROR_NO_MORE_ITEMS;
    pFind->ulCurrent = ulIdx;
    pFind->ulSeen++;
    if (find_load_cache(pFind) != 0)
        return ERROR_NOT_ENOUGH_MEMORY;
    return NO_ERROR;
}

/**
 * @brief Close a Files enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @note If Dep5FilesFindClose is not called, Dep5Close releases all
 *       remaining cursors.
 * @see Dep5FilesFindFirst
 */
APIRET APIENTRY Dep5FilesFindClose(HDEP5FIND hFind) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    if (!pFind) return NO_ERROR;
    if (pFind->ulKind != DEP5_STANZA_FILES) return ERROR_INVALID_HANDLE;
    find_free_cache(pFind);
    free(pFind);
    return NO_ERROR;
}

/**
 * @brief Retrieve a field value from the current Files stanza.
 *
 * Supported fields (case-insensitive):
 *   "Files", "Copyright", "License", "Comment".
 * Extra fields present in the source are also accessible by their
 * exact name.
 *
 * The returned value is the field's full text, with lines joined
 * by '\n'. Whitespace-separated lists (Files) and line-based lists
 * (Copyright) are returned verbatim; use Dep5FilesGetPattern and
 * Dep5FilesGetPatternCount for parsed Files patterns.
 *
 * The License field's indented body (after the first line) is not
 * included. Only the synopsis (short name or names, first line) is
 * returned. To fetch the license text, look it up through the
 * stand-alone License stanzas (Dep5LicenseFindFirst /
 * Dep5LicenseGetText).
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query
 * only.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Field not present in this stanza.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5FilesGetField(HDEP5FIND hFind, PCSZ pszField,
                                  PSZ pszBuffer, ULONG ulBufSize,
                                  PULONG pulSize) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    PDEP5STANZA pStanza;
    PDEP5FIELD pField;
    size_t cbLen;

    if (!pFind || !pszField || !pszField[0]) return ERROR_INVALID_PARAMETER;
    if (pFind->ulKind != DEP5_STANZA_FILES) return ERROR_INVALID_HANDLE;
    pStanza = &pFind->pDoc->paStanzas[pFind->ulCurrent];

    /* Special case: License — synopsis only. */
    if (strcasecmp(pszField, "License") == 0) {
        if (!pFind->pszShortName) return ERROR_FILE_NOT_FOUND;
        cbLen = strlen(pFind->pszShortName);
        if (pszBuffer == NULL && ulBufSize == 0) {
            if (pulSize) *pulSize = (ULONG)cbLen + 1;
            return NO_ERROR;
        }
        if (!pszBuffer) return ERROR_INVALID_PARAMETER;
        if (ulBufSize < cbLen + 1) {
            if (pulSize) *pulSize = (ULONG)cbLen + 1;
            return ERROR_BUFFER_OVERFLOW;
        }
        memcpy(pszBuffer, pFind->pszShortName, cbLen);
        pszBuffer[cbLen] = '\0';
        if (pulSize) *pulSize = (ULONG)cbLen;
        return NO_ERROR;
    }

    pField = stanza_find_field(pStanza, pszField);
    if (!pField || !pField->pszValue) return ERROR_FILE_NOT_FOUND;
    cbLen = strlen(pField->pszValue);
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuffer) return ERROR_INVALID_PARAMETER;
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pField->pszValue, cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/**
 * @brief Number of patterns in the Files field of the current
 *        stanza.
 *
 * Patterns are the tokens of the whitespace-separated Files list.
 * Each token has been unescaped: @c \* is @c * , @c \? is @c ? ,
 * and @c \\ is @c \ . The space character cannot appear inside a
 * pattern.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hFind or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY Dep5FilesGetPatternCount(HDEP5FIND hFind,
                                         PULONG pulCount) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    if (!pFind || !pulCount) return ERROR_INVALID_PARAMETER;
    if (pFind->ulKind != DEP5_STANZA_FILES) return ERROR_INVALID_HANDLE;
    *pulCount = pFind->ulPatternCount;
    return NO_ERROR;
}

/**
 * @brief Retrieve one pattern from the Files field by index.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[in]  ulIndex    Pattern index. Range [0, count).
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5FilesGetPattern(HDEP5FIND hFind, ULONG ulIndex,
                                    PSZ pszBuffer, ULONG ulBufSize,
                                    PULONG pulSize) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    PCSZ pszPattern;
    size_t cbLen;

    if (!pFind || !pszBuffer) return ERROR_INVALID_PARAMETER;
    if (pFind->ulKind != DEP5_STANZA_FILES) return ERROR_INVALID_HANDLE;
    if (ulIndex >= pFind->ulPatternCount) return ERROR_FILE_NOT_FOUND;
    pszPattern = pFind->paPatterns[ulIndex];
    cbLen = strlen(pszPattern);
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pszPattern, cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/* ------------------------------------------------------------------
 * Stand-alone License stanza enumeration
 * ------------------------------------------------------------------ */

/**
 * @brief Start enumerating stand-alone License stanzas.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[out] phFind    Cursor receiver. Not NULL. Set to
 *                       NULLHANDLE on error or if the document has
 *                       no License stanzas.
 * @param[out] pulCount  Optional. May be NULL. On success receives
 *                       the total number of License stanzas.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc or phFind is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval ERROR_NO_MORE_ITEMS      Document has no License stanzas.
 *                                  *phFind = NULLHANDLE.
 *
 * @note Release the cursor with Dep5LicenseFindClose (or Dep5Close
 *       if the caller forgot).
 * @see Dep5LicenseFindNext, Dep5LicenseFindClose
 */
APIRET APIENTRY Dep5LicenseFindFirst(HDEP5DOC hDoc, HDEP5FIND *phFind,
                                     PULONG pulCount) {
    PDEP5DOC pDoc = Dep5InternalGetDoc(hDoc);
    PDEP5FIND pFind;
    ULONG ulIdx;

    if (!pDoc || !phFind) return ERROR_INVALID_PARAMETER;
    *phFind = NULLHANDLE;

    ulIdx = doc_find_first_index(pDoc, DEP5_STANZA_LICENSE);
    if (ulIdx == DEP5_NO_HEADER) return ERROR_NO_MORE_ITEMS;

    pFind = (PDEP5FIND)calloc(1, sizeof(DEP5FIND));
    if (!pFind) return ERROR_NOT_ENOUGH_MEMORY;
    pFind->pDoc = pDoc;
    pFind->ulKind = DEP5_STANZA_LICENSE;
    pFind->ulCurrent = ulIdx;
    pFind->ulSeen = 1;

    if (find_load_cache(pFind) != 0) {
        find_free_cache(pFind);
        free(pFind);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (pulCount) *pulCount = pDoc->ulLicenseCount;
    *phFind = (HDEP5FIND)pFind;
    return NO_ERROR;
}

/**
 * @brief Advance the cursor to the next License stanza.
 *
 * @param[in] hFind  Cursor from Dep5LicenseFindFirst. Not
 *                   NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more License stanzas.
 *
 * @see Dep5LicenseFindFirst, Dep5LicenseFindClose
 */
APIRET APIENTRY Dep5LicenseFindNext(HDEP5FIND hFind) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    ULONG ulIdx;
    if (!pFind) return ERROR_INVALID_HANDLE;
    if (pFind->ulKind != DEP5_STANZA_LICENSE) return ERROR_INVALID_HANDLE;

    ulIdx = doc_find_next_index(pFind->pDoc, DEP5_STANZA_LICENSE,
                                pFind->ulCurrent);
    if (ulIdx == DEP5_NO_HEADER) return ERROR_NO_MORE_ITEMS;
    pFind->ulCurrent = ulIdx;
    pFind->ulSeen++;
    if (find_load_cache(pFind) != 0)
        return ERROR_NOT_ENOUGH_MEMORY;
    return NO_ERROR;
}

/**
 * @brief Close a License enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @see Dep5LicenseFindFirst
 */
APIRET APIENTRY Dep5LicenseFindClose(HDEP5FIND hFind) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    if (!pFind) return NO_ERROR;
    if (pFind->ulKind != DEP5_STANZA_LICENSE) return ERROR_INVALID_HANDLE;
    find_free_cache(pFind);
    free(pFind);
    return NO_ERROR;
}

/**
 * @brief Retrieve the short name from the current License stanza.
 *
 * The short name is the first line of the License field, up to the
 * first whitespace. For example, for a stanza beginning with
 * "License: GPL-2+ with OpenSSL exception", the short name is
 * "GPL-2+ with OpenSSL exception".
 *
 * If the stanza is missing the License field (which the parser
 * does not allow), ERROR_FILE_NOT_FOUND is returned.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Stanza has no License field.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5LicenseGetShortName(HDEP5FIND hFind,
                                        PSZ pszBuffer, ULONG ulBufSize,
                                        PULONG pulSize) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    size_t cbLen;
    if (!pFind || !pszBuffer) return ERROR_INVALID_PARAMETER;
    if (pFind->ulKind != DEP5_STANZA_LICENSE) return ERROR_INVALID_HANDLE;
    if (!pFind->pszShortName) return ERROR_FILE_NOT_FOUND;
    cbLen = strlen(pFind->pszShortName);
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pFind->pszShortName, cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}

/**
 * @brief Retrieve the license text from the current License stanza.
 *
 * The license text is the body of the License field (all lines
 * after the first), with leading whitespace of one level removed,
 * lines joined by '\n'. A single dot on a line (".") denotes a
 * blank line per DEP5 §4.4 and Debian Policy §5.6.13.
 *
 * If the stanza has no body (only the synopsis),
 * ERROR_FILE_NOT_FOUND is returned.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Stanza has no license text.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY Dep5LicenseGetText(HDEP5FIND hFind,
                                   PSZ pszBuffer, ULONG ulBufSize,
                                   PULONG pulSize) {
    PDEP5FIND pFind = Dep5InternalGetFind(hFind);
    size_t cbLen;
    if (!pFind || !pszBuffer) return ERROR_INVALID_PARAMETER;
    if (pFind->ulKind != DEP5_STANZA_LICENSE) return ERROR_INVALID_HANDLE;
    if (!pFind->pszLicenseText || !pFind->pszLicenseText[0])
        return ERROR_FILE_NOT_FOUND;
    cbLen = strlen(pFind->pszLicenseText);
    if (ulBufSize < cbLen + 1) {
        if (pulSize) *pulSize = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pFind->pszLicenseText, cbLen);
    pszBuffer[cbLen] = '\0';
    if (pulSize) *pulSize = (ULONG)cbLen;
    return NO_ERROR;
}
