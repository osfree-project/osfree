/* dep5.c - Debian Copyright Format 1.0 (DEP5) parser, OS/2 API style
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

static char *dup_n(const char *s, size_t n) {
    char *r;
    r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

static char *dup_str(const char *s) {
    return dup_n(s, strlen(s));
}

/* Read the whole file into a malloc buffer. */
static int read_file_all(PCSZ pszPath, char **ppszText) {
    FILE *f;
    long sz;
    char *buf;
    if (!pszPath || !ppszText) return -1;
    f = fopen(pszPath, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return -1; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }
    buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return -1; }
    if (sz > 0 && fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf); fclose(f); return -1;
    }
    buf[sz] = '\0';
    fclose(f);
    *ppszText = buf;
    return 0;
}

/* ==================================================================
 * String buffer accumulator
 * ================================================================== */

typedef struct {
    char  *pbuf;
    size_t cap;
    size_t len;
} SBUF;

static int sbuf_init(SBUF *sb) {
    sb->cap = 64;
    sb->len = 0;
    sb->pbuf = (char*)malloc(sb->cap);
    if (!sb->pbuf) return -1;
    sb->pbuf[0] = '\0';
    return 0;
}

static void sbuf_free(SBUF *sb) {
    free(sb->pbuf);
    sb->pbuf = NULL;
    sb->cap = 0;
    sb->len = 0;
}

static int sbuf_put(SBUF *sb, const char *data, size_t n) {
    if (sb->len + n + 1 > sb->cap) {
        size_t ncap = sb->cap * 2 + n + 64;
        char *nb = (char*)realloc(sb->pbuf, ncap);
        if (!nb) return -1;
        sb->pbuf = nb;
        sb->cap = ncap;
    }
    memcpy(sb->pbuf + sb->len, data, n);
    sb->len += n;
    sb->pbuf[sb->len] = '\0';
    return 0;
}

static int sbuf_putc(SBUF *sb, char c) {
    return sbuf_put(sb, &c, 1);
}

static int sbuf_puts(SBUF *sb, const char *s) {
    return sbuf_put(sb, s, strlen(s));
}

/* ==================================================================
 * Field value normalization (Policy §5.6.13)
 *
 * Applied to every field value at parse time:
 *   - A line consisting of a single '.' is converted into a blank
 *     line (Policy §5.6.13, formatted text rule).
 *   - Trailing whitespace on any line is removed.
 * ================================================================== */

static int normalize_field_value(SBUF *sb) {
    char *src = sb->pbuf;
    size_t src_len = sb->len;
    SBUF out;
    size_t i = 0;

    if (src_len == 0) return 0;
    if (sbuf_init(&out) != 0) return -1;

    while (i < src_len) {
        size_t line_end = i;
        size_t line_trimmed;
        while (line_end < src_len && src[line_end] != '\n') line_end++;
        line_trimmed = line_end;
        while (line_trimmed > i &&
               (src[line_trimmed-1] == ' ' || src[line_trimmed-1] == '\t'))
            line_trimmed--;
        if (line_end - i == 1 && src[i] == '.') {
            /* Single dot -> blank line: skip content. */
        } else {
            if (line_trimmed > i) {
                if (sbuf_put(&out, src + i, line_trimmed - i) != 0) {
                    sbuf_free(&out); return -1;
                }
            }
        }
        if (line_end < src_len) {
            if (sbuf_putc(&out, '\n') != 0) {
                sbuf_free(&out); return -1;
            }
        }
        i = line_end + 1;
    }

    sbuf_free(sb);
    sb->pbuf = out.pbuf;
    sb->cap = out.cap;
    sb->len = out.len;
    return 0;
}

/* ==================================================================
 * Stanza builder
 * ================================================================== */

static int stanza_grow(PDEP5STANZA ps) {
    ULONG ncap;
    DEP5FIELD *na;
    if (ps->ulCount < ps->ulCapacity) return 0;
    ncap = ps->ulCapacity ? ps->ulCapacity * 2 : 8;
    na = (DEP5FIELD*)realloc(ps->paFields,
                             (size_t)ncap * sizeof(DEP5FIELD));
    if (!na) return -1;
    ps->paFields = na;
    ps->ulCapacity = ncap;
    return 0;
}

static PDEP5FIELD stanza_add_field(PDEP5STANZA ps) {
    PDEP5FIELD pf;
    if (stanza_grow(ps) != 0) return NULL;
    pf = &ps->paFields[ps->ulCount++];
    memset(pf, 0, sizeof(*pf));
    return pf;
}

static PDEP5FIELD stanza_find_field(PDEP5STANZA ps, PCSZ pszName) {
    ULONG i;
    if (!ps) return NULL;
    for (i = 0; i < ps->ulCount; i++) {
        if (strcasecmp(ps->paFields[i].pszName, pszName) == 0)
            return &ps->paFields[i];
    }
    return NULL;
}

static void stanza_free(PDEP5STANZA ps) {
    ULONG i;
    if (!ps) return;
    for (i = 0; i < ps->ulCount; i++) {
        free(ps->paFields[i].pszName);
        free(ps->paFields[i].pszValue);
    }
    free(ps->paFields);
    memset(ps, 0, sizeof(*ps));
}

/* ==================================================================
 * Document builder
 * ================================================================== */

static int doc_grow(PDEP5DOC pd) {
    ULONG ncap;
    DEP5STANZA *na;
    if (pd->ulCount < pd->ulCapacity) return 0;
    ncap = pd->ulCapacity ? pd->ulCapacity * 2 : 8;
    na = (DEP5STANZA*)realloc(pd->paStanzas,
                              (size_t)ncap * sizeof(DEP5STANZA));
    if (!na) return -1;
    pd->paStanzas = na;
    pd->ulCapacity = ncap;
    return 0;
}

static PDEP5STANZA doc_add_stanza(PDEP5DOC pd) {
    PDEP5STANZA ps;
    if (doc_grow(pd) != 0) return NULL;
    ps = &pd->paStanzas[pd->ulCount++];
    memset(ps, 0, sizeof(*ps));
    return ps;
}

static void doc_free(PDEP5DOC pd) {
    ULONG i;
    if (!pd) return;
    for (i = 0; i < pd->ulCount; i++) stanza_free(&pd->paStanzas[i]);
    free(pd->paStanzas);
    free(pd);
}

/* ==================================================================
 * Format validation (DEP5 §6.1)
 *
 * Both http and https URLs are valid and refer to the same spec.
 * Trailing slash is optional.
 * ================================================================== */

static int format_is_valid(PCSZ pszValue) {
    static const char *prefixes[] = {
        "https://www.debian.org/doc/packaging-manuals/copyright-format/1.0",
        "http://www.debian.org/doc/packaging-manuals/copyright-format/1.0",
        NULL
    };
    int i;
    if (!pszValue) return 0;
    for (i = 0; prefixes[i]; i++) {
        size_t plen = strlen(prefixes[i]);
        if (strncmp(pszValue, prefixes[i], plen) == 0) {
            char c = pszValue[plen];
            if (c == '\0' || c == '/' || c == ' ' || c == '\t' ||
                c == '\n' || c == '\r')
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

static int unescape_pattern(PCSZ pszToken, size_t nToken,
                            char **ppszOut) {
    SBUF sb;
    size_t i;
    if (sbuf_init(&sb) != 0) return -1;
    for (i = 0; i < nToken; i++) {
        char c = pszToken[i];
        if (c == '\\') {
            if (i + 1 >= nToken) goto fail;
            i++;
            switch (pszToken[i]) {
                case '*':  c = '*';  break;
                case '?':  c = '?';  break;
                case '\\': c = '\\'; break;
                default: goto fail;
            }
        }
        if (sbuf_putc(&sb, c) != 0) goto fail;
    }
    *ppszOut = sb.pbuf;
    return 0;
fail:
    sbuf_free(&sb);
    return -1;
}

static int parse_files_patterns(PCSZ pszValue, char ***ppaOut,
                                ULONG *pulCount) {
    char **paPatterns = NULL;
    ULONG n = 0, cap = 4;
    const char *p = pszValue;

    paPatterns = (char**)malloc(cap * sizeof(char*));
    if (!paPatterns) return -1;

    while (*p) {
        const char *start;
        size_t tok_len;
        char *unescaped;

        while (*p && (*p == ' ' || *p == '\t' ||
                      *p == '\n' || *p == '\r'))
            p++;
        if (!*p) break;
        start = p;
        while (*p && *p != ' ' && *p != '\t' &&
               *p != '\n' && *p != '\r')
            p++;
        tok_len = (size_t)(p - start);

        if (unescape_pattern(start, tok_len, &unescaped) != 0) {
            ULONG k;
            for (k = 0; k < n; k++) free(paPatterns[k]);
            free(paPatterns);
            return -1;
        }
        if (n >= cap) {
            char **na;
            cap *= 2;
            na = (char**)realloc(paPatterns, cap * sizeof(char*));
            if (!na) { free(unescaped); goto fail; }
            paPatterns = na;
        }
        paPatterns[n++] = unescaped;
    }

    *ppaOut = paPatterns;
    *pulCount = n;
    return 0;

fail:
    {
        ULONG k;
        for (k = 0; k < n; k++) free(paPatterns[k]);
        free(paPatterns);
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
    const char *p = pszValue;
    const char *eol;

    *ppszShortName = NULL;
    *ppszBody = NULL;

    if (!p) return -1;
    eol = strchr(p, '\n');
    if (!eol) {
        *ppszShortName = dup_str(p);
        return (*ppszShortName) ? 0 : -1;
    }

    {
        size_t slen = (size_t)(eol - p);
        while (slen > 0 && (p[slen-1] == ' ' || p[slen-1] == '\t'))
            slen--;
        *ppszShortName = dup_n(p, slen);
        if (!*ppszShortName) return -1;
    }
    if (eol[1] == '\0') {
        /* No body. */
        return 0;
    }
    *ppszBody = dup_str(eol + 1);
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
static int license_shortname_is(PDEP5FIELD pf, const char *pszName) {
    const char *value;
    const char *eol;
    size_t syn_len;
    size_t name_len;
    size_t i;
    if (!pf || !pf->pszValue) return 0;
    value = pf->pszValue;
    eol = strchr(value, '\n');
    syn_len = eol ? (size_t)(eol - value) : strlen(value);
    while (syn_len > 0 && (value[syn_len-1] == ' ' || value[syn_len-1] == '\t'))
        syn_len--;
    name_len = strlen(pszName);
    if (name_len != syn_len) return 0;
    for (i = 0; i < syn_len; i++) {
        if (tolower((unsigned char)value[i]) !=
            tolower((unsigned char)pszName[i]))
            return 0;
    }
    return 1;
}

/* Return 1 if the License field has a body (any content after the
 * first newline). */
static int license_has_body(PDEP5FIELD pf) {
    const char *eol;
    if (!pf || !pf->pszValue) return 0;
    eol = strchr(pf->pszValue, '\n');
    if (!eol) return 0;
    return eol[1] != '\0';
}

/* ==================================================================
 * Stanza classification and validation
 * ================================================================== */

static ULONG classify_stanza(PDEP5STANZA ps, int is_first)
{
    if (is_first) return DEP5_STANZA_HEADER;
    if (stanza_find_field(ps, "Files")) return DEP5_STANZA_FILES;
    if (stanza_find_field(ps, "License")) return DEP5_STANZA_LICENSE;
    return 0;
}

static int validate_header(PDEP5STANZA ps) {
    PDEP5FIELD pf = stanza_find_field(ps, "Format");
    if (!pf || !pf->pszValue || !pf->pszValue[0]) return -1;
    if (!format_is_valid(pf->pszValue)) return -1;
    return 0;
}

static int validate_files(PDEP5STANZA ps) {
    PDEP5FIELD pf;
    pf = stanza_find_field(ps, "Files");
    if (!pf || !pf->pszValue || !pf->pszValue[0]) return -1;
    pf = stanza_find_field(ps, "Copyright");
    if (!pf || !pf->pszValue || !pf->pszValue[0]) return -1;
    pf = stanza_find_field(ps, "License");
    if (!pf || !pf->pszValue || !pf->pszValue[0]) return -1;
    /* DEP5 §7.1.1: public-domain requires body. */
    if (license_shortname_is(pf, "public-domain") &&
        !license_has_body(pf))
        return -1;
    return 0;
}

static int validate_license(PDEP5STANZA ps) {
    PDEP5FIELD pf = stanza_find_field(ps, "License");
    if (!pf || !pf->pszValue || !pf->pszValue[0]) return -1;
    /* DEP5 §7.1.1: public-domain requires body. */
    if (license_shortname_is(pf, "public-domain") &&
        !license_has_body(pf))
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

static int parse_document(PCSZ pszText, PDEP5DOC *ppDoc, PCSZ *ppszError) {
    PDEP5DOC pd;
    PDEP5STANZA current = NULL;
    PDEP5FIELD current_field = NULL;
    SBUF value_buf;
    int have_field = 0;
    const char *p = pszText;
    int rc = DEP5_NO_ERROR;

    pd = (PDEP5DOC)calloc(1, sizeof(DEP5DOC));
    if (!pd) return DEP5_ERROR_OUT_OF_MEMORY;
    pd->ulHeaderIndex = DEP5_NO_HEADER;

    if (sbuf_init(&value_buf) != 0) {
        free(pd);
        return DEP5_ERROR_OUT_OF_MEMORY;
    }

    /* Skip UTF-8 BOM. */
    if ((unsigned char)p[0] == 0xEF &&
        (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) p += 3;

    for (;;) {
        const char *line_start = p;
        const char *line_end;
        size_t line_len;
        int blank, comment, continuation;

        /* Find end of the current line. */
        while (*p && *p != '\n') p++;
        line_end = p;
        line_len = (size_t)(line_end - line_start);

        /* Strip trailing CR (CRLF handling). */
        if (line_len > 0 && line_start[line_len-1] == '\r')
            line_len--;

        /* Classify the line. */
        blank = 1;
        {
            size_t i;
            for (i = 0; i < line_len; i++) {
                if (line_start[i] != ' ' && line_start[i] != '\t') {
                    blank = 0;
                    break;
                }
            }
        }
        comment = (!blank && line_start[0] == '#');
        continuation = (!blank && !comment &&
                        (line_start[0] == ' ' || line_start[0] == '\t'));

        if (blank) {
            /* End of current field and stanza. */
            if (have_field) {
                if (current_field && current) {
                    if (normalize_field_value(&value_buf) != 0) {
                        rc = DEP5_ERROR_OUT_OF_MEMORY;
                        goto fail;
                    }
                    current_field->pszValue = dup_str(value_buf.pbuf);
                    if (!current_field->pszValue) {
                        rc = DEP5_ERROR_OUT_OF_MEMORY;
                        goto fail;
                    }
                }
                have_field = 0;
                current_field = NULL;
                value_buf.len = 0;
                if (value_buf.pbuf) value_buf.pbuf[0] = '\0';
            }
            if (current) {
                current = NULL;
            }
        } else if (comment) {
            /* Ignore. */
        } else if (continuation && current_field) {
            /* Continuation: strip exactly one leading space. */
            const char *content = line_start;
            size_t content_len = line_len;
            if (content_len > 0 && (content[0] == ' ' || content[0] == '\t')) {
                content++;
                content_len--;
            }
            if (value_buf.len > 0) {
                if (sbuf_putc(&value_buf, '\n') != 0) {
                    rc = DEP5_ERROR_OUT_OF_MEMORY;
                    goto fail;
                }
            }
            if (content_len > 0) {
                if (sbuf_put(&value_buf, content, content_len) != 0) {
                    rc = DEP5_ERROR_OUT_OF_MEMORY;
                    goto fail;
                }
            }
        } else if (continuation && !current_field) {
            /* Continuation without a preceding field: syntax error. */
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "continuation line without a field";
            goto fail;
        } else {
            /* Start of a new field. */
            const char *colon = NULL;
            size_t i;
            PDEP5FIELD pf;

            for (i = 0; i < line_len; i++) {
                if (line_start[i] == ':') { colon = line_start + i; break; }
            }
            if (!colon) {
                rc = DEP5_ERROR_INVALID_SYNTAX;
                if (ppszError) *ppszError = "field line without colon";
                goto fail;
            }

            /* Close previous field. */
            if (have_field && current_field && current) {
                if (normalize_field_value(&value_buf) != 0) {
                    rc = DEP5_ERROR_OUT_OF_MEMORY;
                    goto fail;
                }
                current_field->pszValue = dup_str(value_buf.pbuf);
                if (!current_field->pszValue) {
                    rc = DEP5_ERROR_OUT_OF_MEMORY;
                    goto fail;
                }
            }
            value_buf.len = 0;
            if (value_buf.pbuf) value_buf.pbuf[0] = '\0';

            /* Start a new stanza if needed. */
            if (!current) {
                current = doc_add_stanza(pd);
                if (!current) {
                    rc = DEP5_ERROR_OUT_OF_MEMORY;
                    goto fail;
                }
            }

            /* Extract and validate the field name. */
            {
                size_t name_len = (size_t)(colon - line_start);
                ULONG k;
                int dup = 0;
                while (name_len > 0 &&
                       (line_start[name_len-1] == ' ' ||
                        line_start[name_len-1] == '\t'))
                    name_len--;
                if (name_len == 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError) *ppszError = "empty field name";
                    goto fail;
                }
                /* Duplicate check (case-insensitive). */
                for (k = 0; k < current->ulCount; k++) {
                    if (strlen(current->paFields[k].pszName) == name_len &&
                        strncasecmp(current->paFields[k].pszName,
                                    line_start, name_len) == 0) {
                        dup = 1; break;
                    }
                }
                if (dup) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "duplicate field name in stanza";
                    goto fail;
                }
                pf = stanza_add_field(current);
                if (!pf) { rc = DEP5_ERROR_OUT_OF_MEMORY; goto fail; }
                pf->pszName = dup_n(line_start, name_len);
                if (!pf->pszName) { rc = DEP5_ERROR_OUT_OF_MEMORY; goto fail; }
            }

            /* Value: everything after colon, with leading whitespace
             * of the first line stripped. */
            {
                const char *val_start = colon + 1;
                size_t val_len;
                while (*val_start == ' ' || *val_start == '\t') val_start++;
                val_len = (size_t)((line_start + line_len) - val_start);
                if (val_len > 0) {
                    if (sbuf_put(&value_buf, val_start, val_len) != 0) {
                        rc = DEP5_ERROR_OUT_OF_MEMORY;
                        goto fail;
                    }
                }
            }

            current_field = pf;
            have_field = 1;
        }

        if (*p != '\n') break;
        p++;
    }

    /* Close trailing field. */
    if (have_field && current_field && current) {
        if (normalize_field_value(&value_buf) != 0) {
            rc = DEP5_ERROR_OUT_OF_MEMORY;
            goto fail;
        }
        current_field->pszValue = dup_str(value_buf.pbuf);
        if (!current_field->pszValue) {
            rc = DEP5_ERROR_OUT_OF_MEMORY;
            goto fail;
        }
    }

    /* Classify and validate stanzas. */
    {
        ULONG i;
        if (pd->ulCount == 0) {
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "file has no stanzas";
            goto fail;
        }
        for (i = 0; i < pd->ulCount; i++) {
            PDEP5STANZA ps = &pd->paStanzas[i];
            ULONG kind = classify_stanza(ps, i == 0);
            if (kind == 0) {
                rc = DEP5_ERROR_INVALID_SYNTAX;
                if (ppszError) *ppszError = "unrecognized stanza";
                goto fail;
            }
            ps->ulKind = kind;
            if (kind == DEP5_STANZA_HEADER) {
                if (pd->ulHeaderIndex != DEP5_NO_HEADER) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError) *ppszError = "multiple header stanzas";
                    goto fail;
                }
                if (validate_header(ps) != 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "invalid header (Format missing or bad)";
                    goto fail;
                }
                pd->ulHeaderIndex = i;
            } else if (kind == DEP5_STANZA_FILES) {
                if (validate_files(ps) != 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "Files stanza missing "
                                     "Files/Copyright/License "
                                     "or public-domain without body";
                    goto fail;
                }
                pd->ulFilesCount++;
            } else if (kind == DEP5_STANZA_LICENSE) {
                if (validate_license(ps) != 0) {
                    rc = DEP5_ERROR_INVALID_SYNTAX;
                    if (ppszError)
                        *ppszError = "License stanza missing License "
                                     "or public-domain without body";
                    goto fail;
                }
                pd->ulLicenseCount++;
            }
        }
        if (pd->ulHeaderIndex == DEP5_NO_HEADER) {
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "missing header stanza";
            goto fail;
        }
        if (pd->ulFilesCount == 0) {
            rc = DEP5_ERROR_INVALID_SYNTAX;
            if (ppszError) *ppszError = "no Files stanzas";
            goto fail;
        }
    }

    sbuf_free(&value_buf);
    *ppDoc = pd;
    return DEP5_NO_ERROR;

fail:
    sbuf_free(&value_buf);
    doc_free(pd);
    return rc;
}

/* ==================================================================
 * Handle helpers
 * ================================================================== */

PDEP5DOC Dep5InternalGetDoc(HDEP5DOC hDoc) { return (PDEP5DOC)hDoc; }
PDEP5FIND Dep5InternalGetFind(HDEP5FIND hFind) { return (PDEP5FIND)hFind; }

static ULONG doc_find_first_index(PDEP5DOC pd, ULONG ulKind) {
    ULONG i;
    for (i = 0; i < pd->ulCount; i++) {
        if (pd->paStanzas[i].ulKind == ulKind) return i;
    }
    return DEP5_NO_HEADER;
}

static ULONG doc_find_next_index(PDEP5DOC pd, ULONG ulKind, ULONG ulFrom) {
    ULONG i;
    for (i = ulFrom + 1; i < pd->ulCount; i++) {
        if (pd->paStanzas[i].ulKind == ulKind) return i;
    }
    return DEP5_NO_HEADER;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Open and parse a debian/copyright file.
 *
 * Reads the file, splits it into stanzas, classifies each stanza, and
 * returns a document handle. All internal buffers are owned by the
 * module and released by Dep5Close.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR             Success.
 * @retval DEP5_ERROR_INVALID_PARAM  pszPath or phDoc is NULL.
 * @retval DEP5_ERROR_OPEN_FAILED    File cannot be opened.
 * @retval DEP5_ERROR_READ_FAILED    Read error.
 * @retval DEP5_ERROR_INVALID_SYNTAX Stanza structure is invalid
 *                                   (missing header, missing required
 *                                   fields, unknown stanza kind).
 * @retval DEP5_ERROR_OUT_OF_MEMORY  Memory allocation failure.
 *
 * @note Ownership of the handle transfers to the caller. It must be
 *       released with Dep5Close.
 * @see Dep5Close
 */
APIRET APIENTRY Dep5Open(PCSZ pszPath, HDEP5DOC *phDoc) {
    char *text = NULL;
    PDEP5DOC pd = NULL;
    PCSZ pszError = NULL;
    APIRET rc;

    if (!pszPath || !phDoc) return DEP5_ERROR_INVALID_PARAM;
    *phDoc = NULLHANDLE;

    if (read_file_all(pszPath, &text) != 0)
        return DEP5_ERROR_OPEN_FAILED;

    rc = parse_document(text, &pd, &pszError);
    free(text);
    if (rc != DEP5_NO_ERROR) return rc;

    *phDoc = (HDEP5DOC)pd;
    return DEP5_NO_ERROR;
}

/**
 * @brief Close a document.
 *
 * Releases all internal buffers, including any active Files and
 * License cursors created from this document. After return the handle
 * is invalid.
 *
 * @param[in] hDoc  Document handle. NULLHANDLE is accepted and treated
 *                  as a no-op.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR             Success. Also returned for
 *                                   NULLHANDLE.
 * @retval DEP5_ERROR_INVALID_HANDLE Handle is not recognized.
 *
 * @warning Calling Dep5Close twice with the same handle is undefined.
 *          The caller should set the handle to NULLHANDLE after close.
 * @see Dep5Open
 */
APIRET APIENTRY Dep5Close(HDEP5DOC hDoc) {
    PDEP5DOC pd;
    if (hDoc == NULLHANDLE) return DEP5_NO_ERROR;
    pd = Dep5InternalGetDoc(hDoc);
    if (!pd) return DEP5_ERROR_INVALID_HANDLE;
    doc_free(pd);
    return DEP5_NO_ERROR;
}

/* ------------------------------------------------------------------
 * Header field access
 * ------------------------------------------------------------------ */

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
 * Whitespace-separated-list and line-based-list fields are returned
 * verbatim (list splitting is the caller's responsibility).
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hDoc       Handle. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   Any parameter is NULL or field
 *                                    name empty.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NOT_FOUND       Field not present in the header.
 * @retval DEP5_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY Dep5HeaderGetField(HDEP5DOC hDoc, PCSZ pszField,
                                   PSZ pszBuffer, ULONG ulBufSize,
                                   PULONG pulSize) {
    PDEP5DOC pd = Dep5InternalGetDoc(hDoc);
    PDEP5STANZA ps;
    PDEP5FIELD pf;
    size_t n;

    if (!pd || !pszField || !pszField[0]) return DEP5_ERROR_INVALID_PARAM;
    if (pd->ulHeaderIndex == DEP5_NO_HEADER) return DEP5_ERROR_NOT_FOUND;
    ps = &pd->paStanzas[pd->ulHeaderIndex];
    pf = stanza_find_field(ps, pszField);
    if (!pf || !pf->pszValue) return DEP5_ERROR_NOT_FOUND;

    n = strlen(pf->pszValue);
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_NO_ERROR;
    }
    if (!pszBuffer) return DEP5_ERROR_INVALID_PARAM;
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pf->pszValue, n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return DEP5_NO_ERROR;
}

/* ------------------------------------------------------------------
 * Files stanza enumeration
 * ------------------------------------------------------------------ */

static void find_free_cache(PDEP5FIND pf) {
    ULONG i;
    if (!pf) return;
    for (i = 0; i < pf->ulPatternCount; i++) free(pf->paPatterns[i]);
    free(pf->paPatterns);      pf->paPatterns = NULL;
    pf->ulPatternCount = 0;
    free(pf->pszShortName);    pf->pszShortName = NULL;
    free(pf->pszLicenseText);  pf->pszLicenseText = NULL;
}

void Dep5InternalFindFreeCache(PDEP5FIND pf) { find_free_cache(pf); }

static int find_load_cache(PDEP5FIND pf) {
    PDEP5STANZA ps = &pf->pDoc->paStanzas[pf->ulCurrent];
    PDEP5FIELD pf_files, pf_license;

    find_free_cache(pf);

    if (pf->ulKind == DEP5_STANZA_FILES) {
        pf_files = stanza_find_field(ps, "Files");
        if (!pf_files || !pf_files->pszValue) return -1;
        if (parse_files_patterns(pf_files->pszValue, &pf->paPatterns,
                                 &pf->ulPatternCount) != 0)
            return -1;
    }

    pf_license = stanza_find_field(ps, "License");
    if (pf_license && pf_license->pszValue) {
        if (split_license(pf_license->pszValue,
                          &pf->pszShortName,
                          &pf->pszLicenseText) != 0)
            return -1;
    }
    return 0;
}

/**
 * @brief Start enumerating Files stanzas.
 *
 * Creates a cursor and positions it on the first Files stanza. The
 * total number of Files stanzas is returned in *pulCount.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] phFind   Cursor receiver. Not NULL. Set to NULLHANDLE on
 *                      error or if the document has no Files stanzas.
 * @param[out] pulCount Optional. May be NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   hDoc or phFind is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NO_MORE_ENTRIES Document has no Files stanzas.
 * @retval DEP5_ERROR_OUT_OF_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY Dep5FilesFindFirst(HDEP5DOC hDoc, HDEP5FIND *phFind,
                                   PULONG pulCount) {
    PDEP5DOC pd = Dep5InternalGetDoc(hDoc);
    PDEP5FIND pf;
    ULONG idx;

    if (!pd || !phFind) return DEP5_ERROR_INVALID_PARAM;
    *phFind = NULLHANDLE;

    idx = doc_find_first_index(pd, DEP5_STANZA_FILES);
    if (idx == DEP5_NO_HEADER) return DEP5_ERROR_NO_MORE_ENTRIES;

    pf = (PDEP5FIND)calloc(1, sizeof(DEP5FIND));
    if (!pf) return DEP5_ERROR_OUT_OF_MEMORY;
    pf->pDoc = pd;
    pf->ulKind = DEP5_STANZA_FILES;
    pf->ulCurrent = idx;
    pf->ulSeen = 1;

    if (find_load_cache(pf) != 0) {
        find_free_cache(pf);
        free(pf);
        return DEP5_ERROR_OUT_OF_MEMORY;
    }

    if (pulCount) *pulCount = pd->ulFilesCount;
    *phFind = (HDEP5FIND)pf;
    return DEP5_NO_ERROR;
}

/**
 * @brief Advance the cursor to the next Files stanza.
 *
 * @param[in] hFind  Cursor from Dep5FilesFindFirst. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NO_MORE_ENTRIES No more Files stanzas.
 * @retval DEP5_ERROR_OUT_OF_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY Dep5FilesFindNext(HDEP5FIND hFind) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    ULONG idx;
    if (!pf) return DEP5_ERROR_INVALID_HANDLE;
    if (pf->ulKind != DEP5_STANZA_FILES) return DEP5_ERROR_INVALID_HANDLE;

    idx = doc_find_next_index(pf->pDoc, DEP5_STANZA_FILES, pf->ulCurrent);
    if (idx == DEP5_NO_HEADER) return DEP5_ERROR_NO_MORE_ENTRIES;
    pf->ulCurrent = idx;
    pf->ulSeen++;
    if (find_load_cache(pf) != 0)
        return DEP5_ERROR_OUT_OF_MEMORY;
    return DEP5_NO_ERROR;
}

/**
 * @brief Close a Files enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR             Success. Also returned for
 *                                   NULLHANDLE.
 * @retval DEP5_ERROR_INVALID_HANDLE Handle is not recognized.
 */
APIRET APIENTRY Dep5FilesFindClose(HDEP5FIND hFind) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    if (!pf) return DEP5_NO_ERROR;
    if (pf->ulKind != DEP5_STANZA_FILES) return DEP5_ERROR_INVALID_HANDLE;
    find_free_cache(pf);
    free(pf);
    return DEP5_NO_ERROR;
}

/* ------------------------------------------------------------------
 * Files stanza field access
 * ------------------------------------------------------------------ */

/**
 * @brief Retrieve a field value from the current Files stanza.
 *
 * Supported fields (case-insensitive):
 *   "Files", "Copyright", "License", "Comment".
 * Extra fields present in the source are also accessible by their
 * exact name.
 *
 * If the field is multi-line, its lines are joined with '\n'.
 *
 * For the "License" field, only the synopsis (first line) is
 * returned. To fetch the full license text, look it up through the
 * stand-alone License stanzas.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[in]  pszField   Field name. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NOT_FOUND       Field not present in this stanza.
 * @retval DEP5_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY Dep5FilesGetField(HDEP5FIND hFind, PCSZ pszField,
                                  PSZ pszBuffer, ULONG ulBufSize,
                                  PULONG pulSize) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    PDEP5STANZA ps;
    PDEP5FIELD f;
    size_t n;

    if (!pf || !pszField || !pszField[0]) return DEP5_ERROR_INVALID_PARAM;
    if (pf->ulKind != DEP5_STANZA_FILES) return DEP5_ERROR_INVALID_HANDLE;
    ps = &pf->pDoc->paStanzas[pf->ulCurrent];

    /* Special case: License — synopsis only. */
    if (strcasecmp(pszField, "License") == 0) {
        if (!pf->pszShortName) return DEP5_ERROR_NOT_FOUND;
        n = strlen(pf->pszShortName);
        if (pszBuffer == NULL && ulBufSize == 0) {
            if (pulSize) *pulSize = (ULONG)(n + 1);
            return DEP5_NO_ERROR;
        }
        if (!pszBuffer) return DEP5_ERROR_INVALID_PARAM;
        if (ulBufSize < n + 1) {
            if (pulSize) *pulSize = (ULONG)(n + 1);
            return DEP5_ERROR_BUFFER_OVERFLOW;
        }
        memcpy(pszBuffer, pf->pszShortName, n);
        pszBuffer[n] = '\0';
        if (pulSize) *pulSize = (ULONG)n;
        return DEP5_NO_ERROR;
    }

    f = stanza_find_field(ps, pszField);
    if (!f || !f->pszValue) return DEP5_ERROR_NOT_FOUND;
    n = strlen(f->pszValue);
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_NO_ERROR;
    }
    if (!pszBuffer) return DEP5_ERROR_INVALID_PARAM;
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, f->pszValue, n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return DEP5_NO_ERROR;
}

/**
 * @brief Number of patterns in the Files field of the current stanza.
 *
 * @param[in]  hFind    Cursor. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR             Success.
 * @retval DEP5_ERROR_INVALID_PARAM  hFind or pulCount is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE Handle is not recognized.
 */
APIRET APIENTRY Dep5FilesGetPatternCount(HDEP5FIND hFind, PULONG pulCount) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    if (!pf || !pulCount) return DEP5_ERROR_INVALID_PARAM;
    if (pf->ulKind != DEP5_STANZA_FILES) return DEP5_ERROR_INVALID_HANDLE;
    *pulCount = pf->ulPatternCount;
    return DEP5_NO_ERROR;
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
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_INDEX_RANGE     Index out of range.
 * @retval DEP5_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY Dep5FilesGetPattern(HDEP5FIND hFind, ULONG ulIndex,
                                    PSZ pszBuffer, ULONG ulBufSize,
                                    PULONG pulSize) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    const char *pszPattern;
    size_t n;

    if (!pf || !pszBuffer) return DEP5_ERROR_INVALID_PARAM;
    if (pf->ulKind != DEP5_STANZA_FILES) return DEP5_ERROR_INVALID_HANDLE;
    if (ulIndex >= pf->ulPatternCount) return DEP5_ERROR_NOT_FOUND;
    pszPattern = pf->paPatterns[ulIndex];
    n = strlen(pszPattern);
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pszPattern, n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return DEP5_NO_ERROR;
}

/* ------------------------------------------------------------------
 * Stand-alone License stanza enumeration
 * ------------------------------------------------------------------ */

/**
 * @brief Start enumerating stand-alone License stanzas.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] phFind   Cursor receiver. Not NULL. Set to NULLHANDLE on
 *                      error or if the document has no License
 *                      stanzas.
 * @param[out] pulCount Optional. May be NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   hDoc or phFind is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NO_MORE_ENTRIES Document has no License stanzas.
 * @retval DEP5_ERROR_OUT_OF_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY Dep5LicenseFindFirst(HDEP5DOC hDoc, HDEP5FIND *phFind,
                                     PULONG pulCount) {
    PDEP5DOC pd = Dep5InternalGetDoc(hDoc);
    PDEP5FIND pf;
    ULONG idx;

    if (!pd || !phFind) return DEP5_ERROR_INVALID_PARAM;
    *phFind = NULLHANDLE;

    idx = doc_find_first_index(pd, DEP5_STANZA_LICENSE);
    if (idx == DEP5_NO_HEADER) return DEP5_ERROR_NO_MORE_ENTRIES;

    pf = (PDEP5FIND)calloc(1, sizeof(DEP5FIND));
    if (!pf) return DEP5_ERROR_OUT_OF_MEMORY;
    pf->pDoc = pd;
    pf->ulKind = DEP5_STANZA_LICENSE;
    pf->ulCurrent = idx;
    pf->ulSeen = 1;

    if (find_load_cache(pf) != 0) {
        find_free_cache(pf);
        free(pf);
        return DEP5_ERROR_OUT_OF_MEMORY;
    }

    if (pulCount) *pulCount = pd->ulLicenseCount;
    *phFind = (HDEP5FIND)pf;
    return DEP5_NO_ERROR;
}

/**
 * @brief Advance the cursor to the next License stanza.
 *
 * @param[in] hFind  Cursor from Dep5LicenseFindFirst. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NO_MORE_ENTRIES No more License stanzas.
 * @retval DEP5_ERROR_OUT_OF_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY Dep5LicenseFindNext(HDEP5FIND hFind) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    ULONG idx;
    if (!pf) return DEP5_ERROR_INVALID_HANDLE;
    if (pf->ulKind != DEP5_STANZA_LICENSE) return DEP5_ERROR_INVALID_HANDLE;

    idx = doc_find_next_index(pf->pDoc, DEP5_STANZA_LICENSE, pf->ulCurrent);
    if (idx == DEP5_NO_HEADER) return DEP5_ERROR_NO_MORE_ENTRIES;
    pf->ulCurrent = idx;
    pf->ulSeen++;
    if (find_load_cache(pf) != 0)
        return DEP5_ERROR_OUT_OF_MEMORY;
    return DEP5_NO_ERROR;
}

/**
 * @brief Close a License enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR             Success. Also returned for
 *                                   NULLHANDLE.
 * @retval DEP5_ERROR_INVALID_HANDLE Handle is not recognized.
 */
APIRET APIENTRY Dep5LicenseFindClose(HDEP5FIND hFind) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    if (!pf) return DEP5_NO_ERROR;
    if (pf->ulKind != DEP5_STANZA_LICENSE) return DEP5_ERROR_INVALID_HANDLE;
    find_free_cache(pf);
    free(pf);
    return DEP5_NO_ERROR;
}

/**
 * @brief Retrieve the short name from the current License stanza.
 *
 * The short name is the first line of the License field, up to the
 * first whitespace.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY Dep5LicenseGetShortName(HDEP5FIND hFind,
                                        PSZ pszBuffer, ULONG ulBufSize,
                                        PULONG pulSize) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    size_t n;
    if (!pf || !pszBuffer) return DEP5_ERROR_INVALID_PARAM;
    if (pf->ulKind != DEP5_STANZA_LICENSE) return DEP5_ERROR_INVALID_HANDLE;
    if (!pf->pszShortName) return DEP5_ERROR_NOT_FOUND;
    n = strlen(pf->pszShortName);
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pf->pszShortName, n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return DEP5_NO_ERROR;
}

/**
 * @brief Retrieve the license text from the current License stanza.
 *
 * The license text is the body of the License field (all lines after
 * the first), with leading whitespace of one level removed, lines
 * joined by '\n'.
 *
 * If the stanza has no body (only the synopsis), DEP5_ERROR_NOT_FOUND
 * is returned.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval DEP5_NO_ERROR              Success.
 * @retval DEP5_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval DEP5_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval DEP5_ERROR_NOT_FOUND       Stanza has no license text.
 * @retval DEP5_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY Dep5LicenseGetText(HDEP5FIND hFind,
                                   PSZ pszBuffer, ULONG ulBufSize,
                                   PULONG pulSize) {
    PDEP5FIND pf = Dep5InternalGetFind(hFind);
    size_t n;
    if (!pf || !pszBuffer) return DEP5_ERROR_INVALID_PARAM;
    if (pf->ulKind != DEP5_STANZA_LICENSE) return DEP5_ERROR_INVALID_HANDLE;
    if (!pf->pszLicenseText || !pf->pszLicenseText[0])
        return DEP5_ERROR_NOT_FOUND;
    n = strlen(pf->pszLicenseText);
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return DEP5_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, pf->pszLicenseText, n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return DEP5_NO_ERROR;
}
