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
 * @brief Duplicate a NUL-terminated string.
 *
 * @param[in] pszSrc  Source string. Not NULL.
 *
 * @return malloc'd copy, or NULL on OOM.
 */
static PSZ dup_str(PCSZ pszSrc) {
    return dup_n(pszSrc, strlen(pszSrc));
}

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

static int sbuf_init(SBUF *pBuf) {
    pBuf->cbCap = 64;
    pBuf->cbLen = 0;
    pBuf->pszBuf = (PSZ)malloc(pBuf->cbCap);
    if (!pBuf->pszBuf) return -1;
    pBuf->pszBuf[0] = '\0';
    return 0;
}

static void sbuf_free(SBUF *pBuf) {
    free(pBuf->pszBuf);
    pBuf->pszBuf = NULL;
    pBuf->cbCap = 0;
    pBuf->cbLen = 0;
}

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

static int sbuf_putc(SBUF *pBuf, CHAR ch) {
    return sbuf_put(pBuf, &ch, 1);
}

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

static PDEP5FIELD stanza_add_field(PDEP5STANZA pStanza) {
    PDEP5FIELD pField;
    if (stanza_grow(pStanza) != 0) return NULL;
    pField = &pStanza->paFields[pStanza->ulCount++];
    memset(pField, 0, sizeof(*pField));
    return pField;
}

static PDEP5FIELD stanza_find_field(PDEP5STANZA pStanza, PCSZ pszName) {
    ULONG ulIdx;
    if (!pStanza) return NULL;
    for (ulIdx = 0; ulIdx < pStanza->ulCount; ulIdx++) {
        if (strcasecmp(pStanza->paFields[ulIdx].pszName, pszName) == 0)
            return &pStanza->paFields[ulIdx];
    }
    return NULL;
}

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

static PDEP5STANZA doc_add_stanza(PDEP5DOC pDoc) {
    PDEP5STANZA pStanza;
    if (doc_grow(pDoc) != 0) return NULL;
    pStanza = &pDoc->paStanzas[pDoc->ulCount++];
    memset(pStanza, 0, sizeof(*pStanza));
    return pStanza;
}

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

static int split_license(PSZ pszValue, PSZ *ppszShortName,
                         PSZ *ppszBody) {
    PCSZ pszPos = pszValue;
    PCSZ pszEol;

    *ppszShortName = NULL;
    *ppszBody = NULL;

    if (!pszPos) return -1;
    pszEol = strchr(pszPos, '\n');
    if (!pszEol) {
        *ppszShortName = dup_str(pszPos);
        return (*ppszShortName) ? 0 : -1;
    }

    {
        size_t cbShortLen = (size_t)(pszEol - pszPos);
        while (cbShortLen > 0 && (pszPos[cbShortLen-1] == ' ' ||
                                  pszPos[cbShortLen-1] == '\t'))
            cbShortLen--;
        *ppszShortName = dup_n(pszPos, cbShortLen);
        if (!*ppszShortName) return -1;
    }
    if (pszEol[1] == '\0') {
        /* No body. */
        return 0;
    }
    *ppszBody = dup_str(pszEol + 1);
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

/* Return 1 if the License field's synopsis (first line) matches
 * pszName case-insensitively. */
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

/* Return 1 if the License field has a body (any content after the
 * first newline). */
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

static ULONG classify_stanza(PDEP5STANZA pStanza, int fIsFirst)
{
    if (fIsFirst) return DEP5_STANZA_HEADER;
    if (stanza_find_field(pStanza, "Files")) return DEP5_STANZA_FILES;
    if (stanza_find_field(pStanza, "License")) return DEP5_STANZA_LICENSE;
    return 0;
}

static int validate_header(PDEP5STANZA pStanza) {
    PDEP5FIELD pField = stanza_find_field(pStanza, "Format");
    if (!pField || !pField->pszValue || !pField->pszValue[0]) return -1;
    if (!format_is_valid(pField->pszValue)) return -1;
    return 0;
}

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
                    pCurrentField->pszValue = dup_str(value_buf.pszBuf);
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
                pCurrentField->pszValue = dup_str(value_buf.pszBuf);
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
                pField->pszName = dup_n(pszLineStart, cbNameLen);
                if (!pField->pszName) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto fail;
                }
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
        pCurrentField->pszValue = dup_str(value_buf.pszBuf);
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

PDEP5DOC Dep5InternalGetDoc(HDEP5DOC hDoc) { return (PDEP5DOC)hDoc; }
PDEP5FIND Dep5InternalGetFind(HDEP5FIND hFind) { return (PDEP5FIND)hFind; }

static ULONG doc_find_first_index(PDEP5DOC pDoc, ULONG ulKind) {
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < pDoc->ulCount; ulIdx++) {
        if (pDoc->paStanzas[ulIdx].ulKind == ulKind) return ulIdx;
    }
    return DEP5_NO_HEADER;
}

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
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL.
 *
 * @return APIRET
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
 * @param[in] hDoc  Document handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
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
 * @param[in]  hDoc       Handle. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
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

void Dep5InternalFindFreeCache(PDEP5FIND pFind) { find_free_cache(pFind); }

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
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[out] phFind    Cursor receiver. Not NULL.
 * @param[out] pulCount  Optional. May be NULL.
 *
 * @return APIRET
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
 * @param[in] hFind  Cursor. NULLHANDLE is accepted as a no-op.
 *
 * @return APIRET
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
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
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
 * @brief Number of patterns in the Files field of the current stanza.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
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
 * @param[in]  ulIndex    Pattern index.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
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
 * @param[out] phFind    Cursor receiver. Not NULL.
 * @param[out] pulCount  Optional. May be NULL.
 *
 * @return APIRET
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
 * @param[in] hFind  Cursor from Dep5LicenseFindFirst. Not NULLHANDLE.
 *
 * @return APIRET
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
 * @param[in] hFind  Cursor. NULLHANDLE is accepted as a no-op.
 *
 * @return APIRET
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
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
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
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
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
