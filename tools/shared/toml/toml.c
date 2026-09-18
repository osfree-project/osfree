/* toml.c - TOML v1.0.0 parser, OS/2 API style (C89 + Watcom extensions) */

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

typedef struct {
    const char *p;
    const char *end;
    PCSZ        pszError;
} PARSE;

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

static void set_error(PARSE *ps, const char *msg) {
    if (ps->pszError == NULL) ps->pszError = msg;
}

static void skip_ws(PARSE *ps) {
    while (ps->p < ps->end && (*ps->p == ' ' || *ps->p == '\t')) ps->p++;
}

static int is_digit_c(int c) {
    return c >= '0' && c <= '9';
}

static int is_bare_key_char(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_' || c == '-';
}

/* Comment: '#' followed by chars up to newline. Control characters
 * other than tab are not permitted. */
static int skip_comment(PARSE *ps) {
    if (ps->p >= ps->end || *ps->p != '#') return 0;
    ps->p++;
    while (ps->p < ps->end) {
        unsigned char c = (unsigned char)*ps->p;
        if (c == '\n' || c == '\r') return 0;
        if (c < 0x20 && c != '\t') return -1;
        if (c == 0x7F) return -1;
        ps->p++;
    }
    return 0;
}

/* Consume one newline: '\n' or '\r\n'. Lone '\r' is an error. */
static int consume_newline(PARSE *ps) {
    if (ps->p < ps->end && *ps->p == '\r') {
        if (ps->p + 1 >= ps->end || ps->p[1] != '\n') return -1;
        ps->p += 2;
        return 0;
    }
    if (ps->p < ps->end && *ps->p == '\n') {
        ps->p++;
        return 0;
    }
    return -1;
}

static int skip_ws_nl_comments(PARSE *ps) {
    for (;;) {
        skip_ws(ps);
        if (ps->p < ps->end && *ps->p == '#') {
            if (skip_comment(ps) != 0) return -1;
            continue;
        }
        if (ps->p < ps->end && (*ps->p == '\n' || *ps->p == '\r')) {
            if (consume_newline(ps) != 0) return -1;
            continue;
        }
        break;
    }
    return 0;
}

static int skip_to_eol(PARSE *ps) {
    skip_ws(ps);
    if (ps->p < ps->end && *ps->p == '#') {
        if (skip_comment(ps) != 0) return -1;
    }
    if (ps->p >= ps->end) return 0;
    return consume_newline(ps);
}

/* ==================================================================
 * IEEE 754 helpers
 * ================================================================== */

static double toml_inf(int negative) {
    return negative ? -HUGE_VAL : HUGE_VAL;
}

static double toml_nan(void) {
    union {
        unsigned char b[sizeof(double)];
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

static int valid_utf8(const char *s) {
    const unsigned char *p;
    p = (const unsigned char*)s;
    while (*p) {
        unsigned char c = *p++;
        unsigned long cp;
        if (c < 0x80) continue;
        if ((c & 0xE0) == 0xC0) {
            if ((*p & 0xC0) != 0x80) return 0;
            cp = ((unsigned long)(c & 0x1F) << 6) |
                 (unsigned long)(*p & 0x3F);
            if (cp < 0x80) return 0;
            p++;
        } else if ((c & 0xF0) == 0xE0) {
            if ((p[0] & 0xC0) != 0x80) return 0;
            if ((p[1] & 0xC0) != 0x80) return 0;
            cp = ((unsigned long)(c & 0x0F) << 12) |
                 ((unsigned long)(p[0] & 0x3F) << 6) |
                 (unsigned long)(p[1] & 0x3F);
            if (cp < 0x800) return 0;
            if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
            p += 2;
        } else if ((c & 0xF8) == 0xF0) {
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

/* ==================================================================
 * Tree primitives
 * ================================================================== */

static PTOMLVALUE value_new(ULONG t) {
    PTOMLVALUE pv;
    pv = (PTOMLVALUE)calloc(1, sizeof(TOMLVALUE));
    if (pv) pv->ulType = t;
    return pv;
}

static PTOMLTABLE table_new(void) {
    PTOMLTABLE pt;
    pt = (PTOMLTABLE)calloc(1, sizeof(TOMLTABLE));
    if (!pt) return NULL;
    if (VectorCreate((ULONG)sizeof(TOMLENTRY), &pt->hEntries) != NO_ERROR) {
        free(pt);
        return NULL;
    }
    return pt;
}

static PTOMLARRAY array_new(void) {
    return (PTOMLARRAY)calloc(1, sizeof(TOMLARRAY));
}

static void value_free(PTOMLVALUE pv);
static void table_free(PTOMLTABLE pt);
static void array_free(PTOMLARRAY pa);

static void array_free(PTOMLARRAY pa) {
    ULONG i;
    if (!pa) return;
    for (i = 0; i < pa->ulCount; i++) value_free(pa->paItems[i]);
    free(pa->paItems);
    free(pa);
}

static void table_free(PTOMLTABLE pt) {
    ULONG ulCount = 0;
    ULONG i;
    if (!pt) return;
    if (pt->hEntries != NULLHANDLE) {
        if (VectorGetCount(pt->hEntries, &ulCount) == NO_ERROR) {
            for (i = 0; i < ulCount; i++) {
                TOMLENTRY entry;
                if (VectorGetItem(pt->hEntries, i, &entry,
                                  (ULONG)sizeof(entry), NULL) == NO_ERROR) {
                    free(entry.pszKey);
                    value_free(entry.pValue);
                }
            }
        }
        VectorDestroy(pt->hEntries);
    }
    free(pt);
}

static void value_free(PTOMLVALUE pv) {
    if (!pv) return;
    switch (pv->ulType) {
        case TOML_TYPE_STRING:
        case TOML_TYPE_DATETIME: free(pv->u.pszString); break;
        case TOML_TYPE_ARRAY:    array_free(pv->u.pArray); break;
        case TOML_TYPE_TABLE:    table_free(pv->u.pTable); break;
        default: break;
    }
    free(pv);
}

void TomlInternalFreeTree(PTOMLTABLE pRoot) {
    table_free(pRoot);
}

static int array_grow(PTOMLARRAY pa) {
    ULONG ncap;
    PTOMLVALUE *na;
    if (pa->ulCount < pa->ulCapacity) return 0;
    ncap = pa->ulCapacity ? pa->ulCapacity * 2 : 8;
    na = (PTOMLVALUE*)realloc(pa->paItems,
                              (size_t)ncap * sizeof(PTOMLVALUE));
    if (!na) return -1;
    pa->paItems = na;
    pa->ulCapacity = ncap;
    return 0;
}

/* Find a table entry by key. The entry is copied into *pEntry if
 * pEntry is not NULL. Returns 1 on success, 0 if not found. */
static int table_find_copy(PTOMLTABLE pt, PCSZ pszKey,
                           PTOMLENTRY pEntry) {
    ULONG ulCount = 0;
    ULONG i;
    if (!pt || pt->hEntries == NULLHANDLE) return 0;
    if (VectorGetCount(pt->hEntries, &ulCount) != NO_ERROR) return 0;
    for (i = 0; i < ulCount; i++) {
        TOMLENTRY entry;
        if (VectorGetItem(pt->hEntries, i, &entry,
                          (ULONG)sizeof(entry), NULL) == NO_ERROR) {
            if (strcmp(entry.pszKey, pszKey) == 0) {
                if (pEntry) *pEntry = entry;
                return 1;
            }
        }
    }
    return 0;
}

/* Return pointer to entry's value if found, or NULL. Caller gets
 * a copy of the value pointer; the value itself is owned by the
 * table. */
static PTOMLVALUE table_find_value(PTOMLTABLE pt, PCSZ pszKey) {
    TOMLENTRY entry;
    if (table_find_copy(pt, pszKey, &entry)) return entry.pValue;
    return NULL;
}

/* Return 1 if the key exists, 0 otherwise. */
static int table_has_key(PTOMLTABLE pt, PCSZ pszKey) {
    return table_find_copy(pt, pszKey, NULL);
}

static int table_add(PTOMLTABLE pt, PSZ pszKey, PTOMLVALUE pValue) {
    TOMLENTRY entry;
    entry.pszKey = pszKey;
    entry.pValue = pValue;
    if (VectorAdd(pt->hEntries, &entry) != NO_ERROR) return -1;
    return 0;
}

static int array_add(PTOMLARRAY pa, PTOMLVALUE pv) {
    if (array_grow(pa) != 0) return -1;
    pa->paItems[pa->ulCount++] = pv;
    return 0;
}

static PTOMLVALUE value_table(PTOMLTABLE pt) {
    PTOMLVALUE pv = value_new(TOML_TYPE_TABLE);
    if (!pv) return NULL;
    pv->u.pTable = pt;
    return pv;
}

static PTOMLVALUE value_array(PTOMLARRAY pa) {
    PTOMLVALUE pv = value_new(TOML_TYPE_ARRAY);
    if (!pv) return NULL;
    pv->u.pArray = pa;
    return pv;
}

/* ==================================================================
 * String parsing
 * ================================================================== */

static int hex_digit(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Emit UTF-8 for a Unicode scalar value. */
static int utf8_emit(unsigned long cp, char *out) {
    if (cp < 0x80) { out[0] = (char)cp; return 1; }
    if (cp < 0x800) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}

static int sbuf_put(char **pbuf, size_t *pcap, size_t *plen,
                    const char *data, size_t n) {
    if (*plen + n + 1 > *pcap) {
        size_t ncap = *pcap * 2 + n + 64;
        char *nb = (char*)realloc(*pbuf, ncap);
        if (!nb) return -1;
        *pbuf = nb;
        *pcap = ncap;
    }
    memcpy(*pbuf + *plen, data, n);
    *plen += n;
    (*pbuf)[*plen] = '\0';
    return 0;
}

static int sbuf_putc(char **pbuf, size_t *pcap, size_t *plen, char c) {
    return sbuf_put(pbuf, pcap, plen, &c, 1);
}

static int parse_hex_escape(PARSE *ps, char *ubuf, int *pnb) {
    int nhex = (*ps->p == 'u') ? 4 : 8;
    unsigned long cp = 0;
    int i;
    ps->p++;
    for (i = 0; i < nhex; i++) {
        int d;
        if (ps->p >= ps->end) return -1;
        d = hex_digit((unsigned char)*ps->p);
        if (d < 0) return -1;
        cp = (cp << 4) | (unsigned long)d;
        ps->p++;
    }
    *pnb = utf8_emit(cp, ubuf);
    return (*pnb == 0) ? -1 : 0;
}

/* Basic string: "..." */
static char *parse_basic_string(PARSE *ps) {
    size_t cap = 64, len = 0;
    char *buf = (char*)malloc(cap);
    if (!buf) return NULL;
    buf[0] = '\0';
    ps->p++;

    while (ps->p < ps->end && *ps->p != '"') {
        char c = *ps->p;
        if (c == '\\') {
            ps->p++;
            if (ps->p >= ps->end) goto fail;
            switch (*ps->p) {
                case 'b': c = '\b'; ps->p++; break;
                case 't': c = '\t'; ps->p++; break;
                case 'n': c = '\n'; ps->p++; break;
                case 'f': c = '\f'; ps->p++; break;
                case 'r': c = '\r'; ps->p++; break;
                case '"': c = '"';  ps->p++; break;
                case '\\': c = '\\'; ps->p++; break;
                case 'u': case 'U': {
                    char ubuf[4]; int nb;
                    if (parse_hex_escape(ps, ubuf, &nb) != 0) goto fail;
                    if (sbuf_put(&buf, &cap, &len, ubuf, (size_t)nb) != 0)
                        goto fail;
                    continue;
                }
                default: goto fail;
            }
        } else if ((unsigned char)c < 0x20 && c != '\t') {
            goto fail;
        } else if ((unsigned char)c == 0x7F) {
            goto fail;
        } else {
            ps->p++;
        }
        if (sbuf_putc(&buf, &cap, &len, c) != 0) goto fail;
    }
    if (ps->p >= ps->end || *ps->p != '"') goto fail;
    ps->p++;
    return buf;
fail:
    free(buf);
    return NULL;
}

/* Multi-line basic string: """...""" */
static char *parse_multiline_basic_string(PARSE *ps) {
    size_t cap = 64, len = 0;
    char *buf = (char*)malloc(cap);
    int done = 0;
    if (!buf) return NULL;
    buf[0] = '\0';
    ps->p += 3;
    /* Trim leading newline */
    if (ps->p < ps->end && *ps->p == '\r' &&
        ps->p + 1 < ps->end && ps->p[1] == '\n') {
        ps->p += 2;
    } else if (ps->p < ps->end && *ps->p == '\n') {
        ps->p++;
    }

    while (!done && ps->p < ps->end) {
        char c = *ps->p;

        if (c == '"') {
            const char *run_start = ps->p;
            int n = 0;
            int k;
            while (ps->p < ps->end && *ps->p == '"') { n++; ps->p++; }
            if (n >= 3 && n <= 5) {
                for (k = 0; k < n - 3; k++) {
                    if (sbuf_putc(&buf, &cap, &len, '"') != 0) goto fail;
                }
                done = 1;
                continue;
            }
            if (n > 5) {
                (void)run_start;
                goto fail;
            }
            for (k = 0; k < n; k++) {
                if (sbuf_putc(&buf, &cap, &len, '"') != 0) goto fail;
            }
            continue;
        }

        if (c == '\\') {
            /* Check for line continuation: \ [ws] newline */
            const char *q = ps->p + 1;
            while (q < ps->end && (*q == ' ' || *q == '\t')) q++;
            if (q < ps->end && (*q == '\n' || *q == '\r')) {
                ps->p = q;
                if (consume_newline(ps) != 0) goto fail;
                while (ps->p < ps->end) {
                    char w = *ps->p;
                    if (w == ' ' || w == '\t') { ps->p++; continue; }
                    if (w == '\r' || w == '\n') {
                        if (consume_newline(ps) != 0) goto fail;
                        continue;
                    }
                    break;
                }
                continue;
            }
            /* Regular escape */
            ps->p++;
            if (ps->p >= ps->end) goto fail;
            switch (*ps->p) {
                case 'b': c = '\b'; ps->p++; break;
                case 't': c = '\t'; ps->p++; break;
                case 'n': c = '\n'; ps->p++; break;
                case 'f': c = '\f'; ps->p++; break;
                case 'r': c = '\r'; ps->p++; break;
                case '"': c = '"';  ps->p++; break;
                case '\\': c = '\\'; ps->p++; break;
                case 'u': case 'U': {
                    char ubuf[4]; int nb;
                    if (parse_hex_escape(ps, ubuf, &nb) != 0) goto fail;
                    if (sbuf_put(&buf, &cap, &len, ubuf, (size_t)nb) != 0)
                        goto fail;
                    continue;
                }
                default: goto fail;
            }
        } else if ((unsigned char)c < 0x20 &&
                   c != '\t' && c != '\n' && c != '\r') {
            goto fail;
        } else if ((unsigned char)c == 0x7F) {
            goto fail;
        } else {
            ps->p++;
        }
        if (sbuf_putc(&buf, &cap, &len, c) != 0) goto fail;
    }
    if (!done) goto fail;
    return buf;
fail:
    free(buf);
    return NULL;
}

/* Literal string: '...' */
static char *parse_literal_string(PARSE *ps) {
    const char *start;
    ps->p++;
    start = ps->p;
    while (ps->p < ps->end && *ps->p != '\'' && *ps->p != '\n' &&
           *ps->p != '\r') {
        unsigned char c = (unsigned char)*ps->p;
        if (c < 0x20 && c != '\t') return NULL;
        if (c == 0x7F) return NULL;
        ps->p++;
    }
    if (ps->p >= ps->end || *ps->p != '\'') return NULL;
    {
        char *r = dup_n(start, (size_t)(ps->p - start));
        ps->p++;
        return r;
    }
}

/* Multi-line literal string: '''...''' */
static char *parse_multiline_literal_string(PARSE *ps) {
    const char *start;
    const char *content_end = NULL;
    ps->p += 3;
    if (ps->p < ps->end && *ps->p == '\r' &&
        ps->p + 1 < ps->end && ps->p[1] == '\n') {
        ps->p += 2;
    } else if (ps->p < ps->end && *ps->p == '\n') {
        ps->p++;
    }
    start = ps->p;

    while (ps->p < ps->end) {
        unsigned char c = (unsigned char)*ps->p;
        if (c == '\'') {
            const char *run_start = ps->p;
            int n = 0;
            while (ps->p < ps->end && *ps->p == '\'') { n++; ps->p++; }
            if (n >= 3 && n <= 5) {
                content_end = run_start + (n - 3);
                break;
            }
            if (n > 5) return NULL;
            /* n < 3: all content, continue */
            continue;
        }
        if (c < 0x20 && c != '\t' && c != '\n' && c != '\r') return NULL;
        if (c == 0x7F) return NULL;
        ps->p++;
    }
    if (!content_end) return NULL;
    /* Validate the range for control chars */
    {
        const char *q;
        for (q = start; q < content_end; q++) {
            unsigned char cc = (unsigned char)*q;
            if (cc < 0x20 && cc != '\t' && cc != '\n' && cc != '\r')
                return NULL;
            if (cc == 0x7F) return NULL;
        }
    }
    return dup_n(start, (size_t)(content_end - start));
}

/* ==================================================================
 * Date/time validation (TOML v1.0.0 grammar)
 * ================================================================== */

/* Parse two decimal digits into an integer value. */
static int parse_2digit(const char *s) {
    return (s[0] - '0') * 10 + (s[1] - '0');
}

/* Days in a month, taking leap years into account. month is 1-based. */
static int days_in_month(int year, int month) {
    static const int dim[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) return 0;
    if (month == 2) {
        int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        return leap ? 29 : 28;
    }
    return dim[month];
}

/* Validate the full date/time token against the TOML v1.0.0 grammar
 * (RFC 3339 subset): year 0000-9999, month 01-12, day according to
 * month and leap year, hour 00-23, minute 00-59, second 00-60 (leap
 * second allowed), fractional seconds one or more digits, offset hour
 * 00-23 and minute 00-59. */
static int validate_datetime(const char *s, size_t len) {
    size_t pos = 0;
    int have_date = 0, have_time = 0;
    int year = 0, month = 0, day = 0;

    /* date-fullyear "-" date-month "-" date-mday (RFC 3339) */
    if (len >= 10 &&
        is_digit_c(s[0]) && is_digit_c(s[1]) &&
        is_digit_c(s[2]) && is_digit_c(s[3]) && s[4] == '-' &&
        is_digit_c(s[5]) && is_digit_c(s[6]) && s[7] == '-' &&
        is_digit_c(s[8]) && is_digit_c(s[9])) {
        year = (s[0]-'0')*1000 + (s[1]-'0')*100 +
               (s[2]-'0')*10 + (s[3]-'0');
        month = parse_2digit(s + 5);
        day = parse_2digit(s + 8);
        if (month < 1 || month > 12) return 0;
        if (day < 1 || day > days_in_month(year, month)) return 0;
        have_date = 1;
        pos = 10;
    }

    /* time-hour ":" time-minute ":" time-second [ time-secfrac ] */
    if (len - pos >= 8 &&
        is_digit_c(s[pos+0]) && is_digit_c(s[pos+1]) && s[pos+2] == ':' &&
        is_digit_c(s[pos+3]) && is_digit_c(s[pos+4]) && s[pos+5] == ':' &&
        is_digit_c(s[pos+6]) && is_digit_c(s[pos+7])) {
        int hour, minute, second;
        if (have_date) {
            if (s[pos] != 'T' && s[pos] != 't' && s[pos] != ' ') return 0;
            pos++;
        }
        hour   = parse_2digit(s + pos);
        minute = parse_2digit(s + pos + 3);
        second = parse_2digit(s + pos + 6);
        if (hour > 23 || minute > 59 || second > 60) return 0;
        have_time = 1;
        pos += 8;
        if (pos < len && s[pos] == '.') {
            pos++;
            if (pos >= len || !is_digit_c(s[pos])) return 0;
            while (pos < len && is_digit_c(s[pos])) pos++;
        }
        if (pos < len) {
            if (s[pos] == 'Z' || s[pos] == 'z') {
                pos++;
            } else if (s[pos] == '+' || s[pos] == '-') {
                if (len - pos < 6) return 0;
                if (!is_digit_c(s[pos+1]) || !is_digit_c(s[pos+2]) ||
                    s[pos+3] != ':' ||
                    !is_digit_c(s[pos+4]) || !is_digit_c(s[pos+5]))
                    return 0;
                {
                    int oh = parse_2digit(s + pos + 1);
                    int om = parse_2digit(s + pos + 4);
                    if (oh > 23 || om > 59) return 0;
                }
                pos += 6;
            }
        }
    }
    return pos == len && (have_date || have_time);
}

/* Quick syntactic check: does the token look like a date (YYYY-) or a
 * time (HH:)? Used to decide whether to enter the datetime parser. */
static int looks_like_datetime(const char *p, const char *end) {
    if (p + 5 <= end &&
        is_digit_c(p[0]) && is_digit_c(p[1]) &&
        is_digit_c(p[2]) && is_digit_c(p[3]) && p[4] == '-') return 1;
    if (p + 3 <= end &&
        is_digit_c(p[0]) && is_digit_c(p[1]) && p[2] == ':') return 1;
    return 0;
}

/* ==================================================================
 * Number parsing
 * ================================================================== */

static int digit_value(int c, int base) {
    if (c >= '0' && c <= '9') {
        int d = c - '0';
        if (d < base) return d;
        return -1;
    }
    if (base == 16 && c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (base == 16 && c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_integer_token(const char *start, const char *end,
                               LONGLONG *out) {
    int base = 10;
    unsigned __int64 v = 0;
    int sign = 1;
    int had_sign = 0;
    const char *p = start;
    int any = 0;
    int prev_was_digit = 0;
    unsigned __int64 limit;
    int is_radix = 0;

    if (p < end && (*p == '+' || *p == '-')) {
        if (*p == '-') sign = -1;
        had_sign = 1;
        p++;
    }

    /* Radix prefixes are strictly lowercase (ABNF %x30.78, %x30.6F,
     * %x30.62). Only one prefix form is recognized; uppercase 0X/0O/0B
     * is a syntax error. */
    if (p + 1 < end && *p == '0') {
        char n = p[1];
        if (n == 'x')      { base = 16; is_radix = 1; p += 2; }
        else if (n == 'o') { base = 8;  is_radix = 1; p += 2; }
        else if (n == 'b') { base = 2;  is_radix = 1; p += 2; }
    }
    if (is_radix && had_sign) return 0;

    /* Leading zero check for decimal integers */
    if (!is_radix) {
        const char *q = p;
        int first = -1;
        int count = 0;
        while (q < end) {
            if (*q == '_') { q++; continue; }
            if (*q >= '0' && *q <= '9') {
                if (first < 0) first = *q - '0';
                count++;
            }
            q++;
        }
        if (first == 0 && count > 1) return 0;
    }

    if (sign < 0) {
        limit = ((unsigned __int64)1) << 63;
    } else {
        limit = (((unsigned __int64)1) << 63) - 1;
    }

    while (p < end) {
        int d;
        if (*p == '_') {
            if (!prev_was_digit) return 0;
            if (p + 1 >= end) return 0;
            if (digit_value((unsigned char)p[1], base) < 0) return 0;
            prev_was_digit = 0;
            p++;
            continue;
        }
        d = digit_value((unsigned char)*p, base);
        if (d < 0) return 0;
        if (v > (limit - (unsigned __int64)d) / (unsigned __int64)base)
            return 0;
        v = v * base + (unsigned __int64)d;
        any = 1;
        prev_was_digit = 1;
        p++;
    }
    if (!any) return 0;
    if (!prev_was_digit) return 0;

    if (sign < 0) {
        *out = (LONGLONG)(0 - v);
    } else {
        *out = (LONGLONG)v;
    }
    return 1;
}

static int is_number_token_start(const char *p, const char *end) {
    if (p >= end) return 0;
    if (*p == '+' || *p == '-') return 1;
    if (is_digit_c(*p)) return 1;
    if (p + 3 <= end) {
        if (strncmp(p, "inf", 3) == 0) return 1;
        if (strncmp(p, "nan", 3) == 0) return 1;
    }
    return 0;
}

/* Validate a float token (underscores already stripped). */
static int validate_float_clean(const char *s) {
    size_t i = 0, n = strlen(s);
    int saw_dot = 0, saw_exp = 0;

    if (i < n && (s[i] == '+' || s[i] == '-')) i++;
    if (i >= n) return 0;

    if (s[i] == '0') {
        i++;
        if (i < n && s[i] >= '0' && s[i] <= '9') return 0;
    } else if (s[i] >= '1' && s[i] <= '9') {
        i++;
        while (i < n && s[i] >= '0' && s[i] <= '9') i++;
    } else {
        return 0;
    }

    if (i < n && s[i] == '.') {
        i++;
        if (i >= n || !(s[i] >= '0' && s[i] <= '9')) return 0;
        i++;
        while (i < n && s[i] >= '0' && s[i] <= '9') i++;
        saw_dot = 1;
    }

    if (i < n && (s[i] == 'e' || s[i] == 'E')) {
        i++;
        if (i < n && (s[i] == '+' || s[i] == '-')) i++;
        if (i >= n || !(s[i] >= '0' && s[i] <= '9')) return 0;
        i++;
        while (i < n && s[i] >= '0' && s[i] <= '9') i++;
        saw_exp = 1;
    }

    if (i != n) return 0;
    if (!saw_dot && !saw_exp) return 0;
    return 1;
}

static int parse_number_or_datetime(PARSE *ps, PTOMLVALUE *ppv) {
    const char *start = ps->p;
    int negative = 0;

    /* Special float values: inf, nan (lowercase only) */
    {
        const char *p = start;
        if (p < ps->end && (*p == '+' || *p == '-')) {
            negative = (*p == '-');
            p++;
        }
        if (p + 3 <= ps->end) {
            if (strncmp(p, "inf", 3) == 0 &&
                (p + 3 == ps->end || !is_bare_key_char(p[3]))) {
                PTOMLVALUE pv = value_new(TOML_TYPE_FLOAT);
                if (!pv) return -1;
                pv->u.dblFloat = toml_inf(negative);
                ps->p = p + 3;
                *ppv = pv;
                return 0;
            }
            if (strncmp(p, "nan", 3) == 0 &&
                (p + 3 == ps->end || !is_bare_key_char(p[3]))) {
                PTOMLVALUE pv = value_new(TOML_TYPE_FLOAT);
                if (!pv) return -1;
                pv->u.dblFloat = toml_nan();
                ps->p = p + 3;
                *ppv = pv;
                return 0;
            }
        }
    }

    /* Date/time */
    if (looks_like_datetime(ps->p, ps->end)) {
        const char *tok_start = ps->p;
        char *tok;
        PTOMLVALUE pv;
        size_t tok_len;

        if (ps->p + 10 <= ps->end &&
            is_digit_c(ps->p[0]) && is_digit_c(ps->p[1]) &&
            is_digit_c(ps->p[2]) && is_digit_c(ps->p[3]) &&
            ps->p[4] == '-') {
            ps->p += 10;
            /* Optional time part */
            if (ps->p < ps->end &&
                (*ps->p == 'T' || *ps->p == 't' || *ps->p == ' ')) {
                const char *sep = ps->p;
                ps->p++;
                if (ps->p + 8 <= ps->end &&
                    is_digit_c(ps->p[0]) && is_digit_c(ps->p[1]) &&
                    ps->p[2] == ':' &&
                    is_digit_c(ps->p[3]) && is_digit_c(ps->p[4]) &&
                    ps->p[5] == ':' &&
                    is_digit_c(ps->p[6]) && is_digit_c(ps->p[7])) {
                    ps->p += 8;
                    if (ps->p < ps->end && *ps->p == '.') {
                        ps->p++;
                        while (ps->p < ps->end && is_digit_c(*ps->p))
                            ps->p++;
                    }
                    if (ps->p < ps->end &&
                        (*ps->p == 'Z' || *ps->p == 'z')) {
                        ps->p++;
                    } else if (ps->p < ps->end &&
                               (*ps->p == '+' || *ps->p == '-')) {
                        if (ps->p + 6 <= ps->end &&
                            is_digit_c(ps->p[1]) && is_digit_c(ps->p[2]) &&
                            ps->p[3] == ':' &&
                            is_digit_c(ps->p[4]) && is_digit_c(ps->p[5])) {
                            ps->p += 6;
                        }
                    }
                } else {
                    ps->p = sep;
                }
            }
        } else {
            /* Local time HH:MM:SS */
            ps->p += 8;
            if (ps->p < ps->end && *ps->p == '.') {
                ps->p++;
                while (ps->p < ps->end && is_digit_c(*ps->p)) ps->p++;
            }
        }
        tok_len = (size_t)(ps->p - tok_start);
        tok = dup_n(tok_start, tok_len);
        if (!tok) return -1;
        if (!validate_datetime(tok, tok_len)) { free(tok); return -1; }
        pv = value_new(TOML_TYPE_DATETIME);
        if (!pv) { free(tok); return -1; }
        pv->u.pszString = tok;
        *ppv = pv;
        return 0;
    }

    /* Radix integer (0x, 0o, 0b) — prefixes are strictly lowercase. */
    {
        const char *p = start;
        if (*p == '+' || *p == '-') p++;
        if (p + 1 < ps->end && *p == '0' &&
            (p[1] == 'x' || p[1] == 'o' || p[1] == 'b')) {
            const char *q = start;
            LONGLONG ll;
            PTOMLVALUE pv;
            while (q < ps->end) {
                char c = *q;
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
                    c == ',' || c == ']' || c == '}' || c == '#') break;
                q++;
            }
            ps->p = q;
            if (!parse_integer_token(start, q, &ll)) return -1;
            pv = value_new(TOML_TYPE_INTEGER);
            if (!pv) return -1;
            pv->u.llInteger = ll;
            *ppv = pv;
            return 0;
        }
    }

    /* Decimal number (integer or float) */
    {
        const char *q = start;
        int dot_or_exp = 0;
        size_t tok_len;
        char *clean;
        size_t clen, i;
        PTOMLVALUE pv;

        while (q < ps->end) {
            char c = *q;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
                c == ',' || c == ']' || c == '}' || c == '#') break;
            if (c == '.' || c == 'e' || c == 'E') dot_or_exp = 1;
            q++;
        }
        tok_len = (size_t)(q - start);
        ps->p = q;

        if (dot_or_exp) {
            clean = (char*)malloc(tok_len + 1);
            if (!clean) return -1;
            clen = 0;
            for (i = 0; i < tok_len; i++) {
                if (start[i] == '_') {
                    if (i == 0 || i == tok_len - 1 ||
                        !is_digit_c((unsigned char)start[i-1]) ||
                        !is_digit_c((unsigned char)start[i+1])) {
                        free(clean);
                        return -1;
                    }
                    continue;
                }
                clean[clen++] = start[i];
            }
            clean[clen] = '\0';
            if (!validate_float_clean(clean)) {
                free(clean);
                return -1;
            }
            {
                double d = strtod(clean, NULL);
                free(clean);
                pv = value_new(TOML_TYPE_FLOAT);
                if (!pv) return -1;
                pv->u.dblFloat = d;
                *ppv = pv;
                return 0;
            }
        } else {
            LONGLONG ll;
            if (!parse_integer_token(start, q, &ll)) return -1;
            pv = value_new(TOML_TYPE_INTEGER);
            if (!pv) return -1;
            pv->u.llInteger = ll;
            *ppv = pv;
            return 0;
        }
    }
}

/* ==================================================================
 * Boolean
 * ================================================================== */

static int parse_boolean(PARSE *ps, PTOMLVALUE *ppv) {
    if (ps->p + 4 <= ps->end && strncmp(ps->p, "true", 4) == 0 &&
        (ps->p + 4 == ps->end || !is_bare_key_char(ps->p[4]))) {
        PTOMLVALUE pv = value_new(TOML_TYPE_BOOLEAN);
        if (!pv) return -1;
        pv->u.fBoolean = TRUE_;
        ps->p += 4;
        *ppv = pv;
        return 0;
    }
    if (ps->p + 5 <= ps->end && strncmp(ps->p, "false", 5) == 0 &&
        (ps->p + 5 == ps->end || !is_bare_key_char(ps->p[5]))) {
        PTOMLVALUE pv = value_new(TOML_TYPE_BOOLEAN);
        if (!pv) return -1;
        pv->u.fBoolean = FALSE_;
        ps->p += 5;
        *ppv = pv;
        return 0;
    }
    return -1;
}

/* ==================================================================
 * Arrays and inline tables
 * ================================================================== */

static int parse_value(PARSE *ps, PTOMLVALUE *ppv);
static int parse_key_path(PARSE *ps, char ***ppaKeys, ULONG *pulCount);

static void free_key_path(char **aKeys, ULONG n) {
    ULONG i;
    if (!aKeys) return;
    for (i = 0; i < n; i++) free(aKeys[i]);
    free(aKeys);
}

static int parse_array(PARSE *ps, PTOMLVALUE *ppv) {
    PTOMLARRAY pa = array_new();
    PTOMLVALUE pv;
    if (!pa) return -1;
    ps->p++;

    for (;;) {
        PTOMLVALUE item = NULL;
        if (skip_ws_nl_comments(ps) != 0) { array_free(pa); return -1; }
        if (ps->p >= ps->end) { array_free(pa); return -1; }
        if (*ps->p == ']') { ps->p++; break; }
        if (parse_value(ps, &item) != 0) { array_free(pa); return -1; }
        if (array_add(pa, item) != 0) {
            value_free(item); array_free(pa); return -1;
        }
        if (skip_ws_nl_comments(ps) != 0) { array_free(pa); return -1; }
        if (ps->p >= ps->end) { array_free(pa); return -1; }
        if (*ps->p == ',') { ps->p++; continue; }
        if (*ps->p == ']') { ps->p++; break; }
        array_free(pa); return -1;
    }
    pv = value_array(pa);
    if (!pv) { array_free(pa); return -1; }
    *ppv = pv;
    return 0;
}

static int inline_assign(PTOMLTABLE pt, char **aKeys, ULONG nKeys,
                         PTOMLVALUE pValue) {
    PTOMLTABLE cur = pt;
    ULONG i;
    for (i = 0; i + 1 < nKeys; i++) {
        PTOMLVALUE subv = table_find_value(cur, aKeys[i]);
        if (!subv) {
            PTOMLTABLE sub = table_new();
            PTOMLVALUE newsubv;
            if (!sub) return -1;
            sub->flFlags |= TOML_TABLE_INLINE;
            newsubv = value_table(sub);
            if (!newsubv) { table_free(sub); return -1; }
            if (table_add(cur, strdup(aKeys[i]), newsubv) != 0) {
                value_free(newsubv); return -1;
            }
            cur = sub;
        } else if (subv->ulType == TOML_TYPE_TABLE) {
            if (!(subv->u.pTable->flFlags & TOML_TABLE_INLINE))
                return -1;
            cur = subv->u.pTable;
        } else {
            return -1;
        }
    }
    if (nKeys == 0) return -1;
    if (table_has_key(cur, aKeys[nKeys - 1])) return -1;
    if (table_add(cur, strdup(aKeys[nKeys - 1]), pValue) != 0) return -1;
    return 0;
}

static int parse_inline_table(PARSE *ps, PTOMLVALUE *ppv) {
    PTOMLTABLE pt = table_new();
    PTOMLVALUE pvt;
    if (!pt) return -1;
    pt->flFlags |= TOML_TABLE_INLINE;
    ps->p++;

    skip_ws(ps);
    if (ps->p < ps->end && *ps->p == '}') {
        ps->p++;
        pvt = value_table(pt);
        if (!pvt) { table_free(pt); return -1; }
        *ppv = pvt;
        return 0;
    }
    for (;;) {
        char **aKeys = NULL;
        ULONG nKeys = 0;
        PTOMLVALUE val = NULL;
        int ok;

        skip_ws(ps);
        if (parse_key_path(ps, &aKeys, &nKeys) != 0) {
            table_free(pt); return -1;
        }
        skip_ws(ps);
        if (ps->p >= ps->end || *ps->p != '=') {
            free_key_path(aKeys, nKeys); table_free(pt); return -1;
        }
        ps->p++;
        skip_ws(ps);
        if (parse_value(ps, &val) != 0) {
            free_key_path(aKeys, nKeys); table_free(pt); return -1;
        }
        ok = inline_assign(pt, aKeys, nKeys, val);
        free_key_path(aKeys, nKeys);
        if (ok != 0) {
            value_free(val); table_free(pt); return -1;
        }
        skip_ws(ps);
        if (ps->p >= ps->end) { table_free(pt); return -1; }
        if (*ps->p == ',') { ps->p++; continue; }
        if (*ps->p == '}') { ps->p++; break; }
        table_free(pt); return -1;
    }
    pvt = value_table(pt);
    if (!pvt) { table_free(pt); return -1; }
    *ppv = pvt;
    return 0;
}

static int parse_value(PARSE *ps, PTOMLVALUE *ppv) {
    skip_ws(ps);
    if (ps->p >= ps->end) return -1;

    if (*ps->p == '"') {
        char *s;
        PTOMLVALUE pv;
        if (ps->p + 2 < ps->end && ps->p[1] == '"' && ps->p[2] == '"')
            s = parse_multiline_basic_string(ps);
        else
            s = parse_basic_string(ps);
        if (!s) return -1;
        pv = value_new(TOML_TYPE_STRING);
        if (!pv) { free(s); return -1; }
        pv->u.pszString = s;
        *ppv = pv;
        return 0;
    }
    if (*ps->p == '\'') {
        char *s;
        PTOMLVALUE pv;
        if (ps->p + 2 < ps->end && ps->p[1] == '\'' && ps->p[2] == '\'')
            s = parse_multiline_literal_string(ps);
        else
            s = parse_literal_string(ps);
        if (!s) return -1;
        pv = value_new(TOML_TYPE_STRING);
        if (!pv) { free(s); return -1; }
        pv->u.pszString = s;
        *ppv = pv;
        return 0;
    }
    if (*ps->p == '[') return parse_array(ps, ppv);
    if (*ps->p == '{') return parse_inline_table(ps, ppv);
    if (*ps->p == 't' || *ps->p == 'f') return parse_boolean(ps, ppv);
    if (is_number_token_start(ps->p, ps->end))
        return parse_number_or_datetime(ps, ppv);
    return -1;
}

/* ==================================================================
 * Keys
 * ================================================================== */

static char *parse_key_part(PARSE *ps) {
    skip_ws(ps);
    if (ps->p >= ps->end) return NULL;
    if (*ps->p == '"') return parse_basic_string(ps);
    if (*ps->p == '\'') return parse_literal_string(ps);
    {
        const char *start = ps->p;
        while (ps->p < ps->end && is_bare_key_char((unsigned char)*ps->p))
            ps->p++;
        if (ps->p == start) return NULL;
        return dup_n(start, (size_t)(ps->p - start));
    }
}

static int parse_key_path(PARSE *ps, char ***ppaKeys, ULONG *pulCount) {
    char **aKeys;
    ULONG n = 0, cap = 4;

    aKeys = (char**)malloc(cap * sizeof(char*));
    if (!aKeys) return -1;

    for (;;) {
        char *part = parse_key_part(ps);
        if (!part) goto fail;
        if (n >= cap) {
            char **na;
            cap *= 2;
            na = (char**)realloc(aKeys, cap * sizeof(char*));
            if (!na) { free(part); goto fail; }
            aKeys = na;
        }
        aKeys[n++] = part;
        skip_ws(ps);
        if (ps->p < ps->end && *ps->p == '.') { ps->p++; continue; }
        break;
    }
    *ppaKeys = aKeys;
    *pulCount = n;
    return 0;
fail:
    {
        ULONG i;
        for (i = 0; i < n; i++) free(aKeys[i]);
        free(aKeys);
    }
    return -1;
}

/* ==================================================================
 * Table navigation
 *
 * intermediate_flags: flags applied to newly created intermediate
 *                     tables.
 * final_flags:        flags applied to a newly created final table.
 *
 * Existing final tables are checked for redefinition when
 * final_flags & TOML_TABLE_EXPLICIT.
 * ================================================================== */

static PTOMLTABLE descend(PTOMLTABLE pRoot, char **aKeys, ULONG ulCount,
                          ULONG intermediate_flags,
                          ULONG final_flags) {
    PTOMLTABLE cur = pRoot;
    ULONG i;
    for (i = 0; i < ulCount; i++) {
        PTOMLVALUE existing = table_find_value(cur, aKeys[i]);
        int is_last = (i + 1 == ulCount);
        ULONG want_flags = is_last ? final_flags : intermediate_flags;

        if (existing) {
            if (existing->ulType == TOML_TYPE_TABLE) {
                PTOMLTABLE sub = existing->u.pTable;
                if (sub->flFlags & TOML_TABLE_INLINE) return NULL;
                if (is_last && (want_flags & TOML_TABLE_EXPLICIT)) {
                    if (sub->flFlags & TOML_TABLE_EXPLICIT) return NULL;
                    if (sub->flFlags & TOML_TABLE_FROM_DOTTED) return NULL;
                    sub->flFlags |= TOML_TABLE_EXPLICIT;
                }
                cur = sub;
            } else if (existing->ulType == TOML_TYPE_ARRAY) {
                PTOMLARRAY arr = existing->u.pArray;
                PTOMLVALUE last;
                if (!(arr->flFlags & TOML_ARRAY_OF_TABLES)) return NULL;
                if (arr->ulCount == 0) return NULL;
                last = arr->paItems[arr->ulCount - 1];
                if (last->ulType != TOML_TYPE_TABLE) return NULL;
                cur = last->u.pTable;
            } else {
                return NULL;
            }
        } else {
            PTOMLTABLE sub = table_new();
            PTOMLVALUE subv;
            ULONG fl;
            if (!sub) return NULL;
            fl = want_flags & ~TOML_TABLE_ARRAY_ELEM;
            sub->flFlags |= fl;
            subv = value_table(sub);
            if (!subv) { table_free(sub); return NULL; }
            if (table_add(cur, strdup(aKeys[i]), subv) != 0) {
                value_free(subv); return NULL;
            }
            cur = sub;
        }
    }
    return cur;
}

/* ==================================================================
 * Main parse
 * ================================================================== */

APIRET TomlInternalParse(PCSZ pszText, PTOMLTABLE *ppRoot,
                         PCSZ *ppszError) {
    PARSE ps;
    PTOMLTABLE root;
    PTOMLTABLE current;
    const char *p;
    char **aKeys = NULL;
    ULONG nKeys = 0;
    PTOMLVALUE val = NULL;
    APIRET rc = TOML_NO_ERROR;

    if (!pszText || !ppRoot) return TOML_ERROR_INVALID_PARAM;

    p = pszText;
    if ((unsigned char)p[0] == 0xEF &&
        (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) p += 3;

    if (!valid_utf8(p)) {
        if (ppszError) *ppszError = "Invalid UTF-8 input";
        return TOML_ERROR_INVALID_UTF8;
    }

    root = table_new();
    if (!root) {
        if (ppszError) *ppszError = "Out of memory";
        return TOML_ERROR_OUT_OF_MEMORY;
    }

    ps.p = p;
    ps.end = p + strlen(p);
    ps.pszError = NULL;
    current = root;

    while (ps.p < ps.end) {
        if (skip_ws_nl_comments(&ps) != 0) {
            rc = TOML_ERROR_INVALID_SYNTAX;
            goto done;
        }
        if (ps.p >= ps.end) break;

        if (*ps.p == '[') {
            int is_array = 0;
            ps.p++;
            if (ps.p < ps.end && *ps.p == '[') { is_array = 1; ps.p++; }
            if (parse_key_path(&ps, &aKeys, &nKeys) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            skip_ws(&ps);
            if (ps.p >= ps.end || *ps.p != ']') {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            ps.p++;
            if (is_array) {
                if (ps.p >= ps.end || *ps.p != ']') {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
                ps.p++;
            }
            if (skip_to_eol(&ps) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            if (nKeys == 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }

            if (is_array) {
                PTOMLTABLE parent;
                PTOMLVALUE existing;
                PTOMLTABLE new_tbl;

                parent = descend(root, aKeys, nKeys - 1, 0, 0);
                if (!parent) {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
                existing = table_find_value(parent, aKeys[nKeys - 1]);
                new_tbl = table_new();
                if (!new_tbl) {
                    rc = TOML_ERROR_OUT_OF_MEMORY;
                    goto done;
                }
                new_tbl->flFlags |= TOML_TABLE_ARRAY_ELEM;

                if (existing) {
                    PTOMLARRAY arr;
                    PTOMLVALUE arrv;
                    if (existing->ulType != TOML_TYPE_ARRAY ||
                        !(existing->u.pArray->flFlags &
                          TOML_ARRAY_OF_TABLES)) {
                        table_free(new_tbl);
                        rc = TOML_ERROR_INVALID_SYNTAX;
                        goto done;
                    }
                    arr = existing->u.pArray;
                    arrv = value_table(new_tbl);
                    if (!arrv) {
                        table_free(new_tbl);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                    if (array_add(arr, arrv) != 0) {
                        value_free(arrv);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                } else {
                    PTOMLARRAY arr = array_new();
                    PTOMLVALUE arrv, av;
                    if (!arr) {
                        table_free(new_tbl);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                    arr->flFlags |= TOML_ARRAY_OF_TABLES;
                    arrv = value_table(new_tbl);
                    if (!arrv) {
                        array_free(arr);
                        table_free(new_tbl);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                    if (array_add(arr, arrv) != 0) {
                        array_free(arr);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                    av = value_array(arr);
                    if (!av) {
                        array_free(arr);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                    if (table_add(parent, strdup(aKeys[nKeys - 1]),
                                  av) != 0) {
                        value_free(av);
                        rc = TOML_ERROR_OUT_OF_MEMORY;
                        goto done;
                    }
                }
                current = new_tbl;
            } else {
                PTOMLTABLE t = descend(root, aKeys, nKeys,
                                       0, TOML_TABLE_EXPLICIT);
                if (!t) {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
                current = t;
            }
            free_key_path(aKeys, nKeys);
            aKeys = NULL;
            nKeys = 0;
        } else {
            PTOMLTABLE target;

            if (parse_key_path(&ps, &aKeys, &nKeys) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            skip_ws(&ps);
            if (ps.p >= ps.end || *ps.p != '=') {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            ps.p++;
            if (parse_value(&ps, &val) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            if (skip_to_eol(&ps) != 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }
            if (nKeys == 0) {
                rc = TOML_ERROR_INVALID_SYNTAX;
                goto done;
            }

            if (nKeys == 1) {
                target = current;
            } else {
                target = descend(current, aKeys, nKeys - 1,
                                 TOML_TABLE_FROM_DOTTED,
                                 TOML_TABLE_FROM_DOTTED);
                if (!target) {
                    rc = TOML_ERROR_INVALID_SYNTAX;
                    goto done;
                }
            }
            if (table_has_key(target, aKeys[nKeys - 1])) {
                rc = TOML_ERROR_DUPLICATE_KEY;
                goto done;
            }
            if (table_add(target, strdup(aKeys[nKeys - 1]), val) != 0) {
                rc = TOML_ERROR_OUT_OF_MEMORY;
                goto done;
            }
            val = NULL;
            free_key_path(aKeys, nKeys);
            aKeys = NULL;
            nKeys = 0;
        }
    }

    *ppRoot = root;
    root = NULL;
    rc = TOML_NO_ERROR;

done:
    free_key_path(aKeys, nKeys);
    value_free(val);
    if (root) table_free(root);
    if (rc != TOML_NO_ERROR && ppszError) {
        *ppszError = ps.pszError ? ps.pszError : "TOML syntax error";
    }
    return rc;
}

/* ==================================================================
 * Handles and file helpers
 * ================================================================== */

static PTOMLDOC as_doc(HTOMLDOC h) { return (PTOMLDOC)h; }
static PTOMLFIND as_find(HTOMLFIND h) { return (PTOMLFIND)h; }

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

/**
 * @brief Open a TOML document from a file.
 *
 * Reads the file, parses TOML v1.0.0, allocates all internal buffers
 * and returns a document handle.
 *
 * @param[in]  pszPath  Path to the file. Must not be NULL.
 * @param[out] phToml   Handle receiver. Must not be NULL. Set to
 *                      NULLHANDLE on error.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  pszPath or phToml is NULL.
 * @retval TOML_ERROR_OPEN_FAILED    File cannot be opened.
 * @retval TOML_ERROR_READ_FAILED    Read error.
 * @retval TOML_ERROR_INVALID_UTF8   File content is not valid UTF-8.
 * @retval TOML_ERROR_INVALID_SYNTAX TOML syntax error.
 * @retval TOML_ERROR_DUPLICATE_KEY  Duplicate key.
 * @retval TOML_ERROR_OUT_OF_MEMORY  Memory allocation failure.
 *
 * @note Ownership of the handle transfers to the caller. It must be
 *       released with TomlClose.
 * @see TomlClose
 */
APIRET APIENTRY TomlOpen(PCSZ pszPath, HTOMLDOC *phToml) {
    char *text = NULL;
    PTOMLTABLE root = NULL;
    PTOMLDOC doc;
    PCSZ err = NULL;
    APIRET rc;

    if (!pszPath || !phToml) return TOML_ERROR_INVALID_PARAM;
    *phToml = NULLHANDLE;

    if (read_file_all(pszPath, &text) != 0) return TOML_ERROR_OPEN_FAILED;
    rc = TomlInternalParse(text, &root, &err);
    free(text);
    if (rc != TOML_NO_ERROR) return rc;

    doc = (PTOMLDOC)calloc(1, sizeof(TOMLDOC));
    if (!doc) { table_free(root); return TOML_ERROR_OUT_OF_MEMORY; }
    doc->pRoot = root;
    doc->pFirstFind = NULL;
    doc->pRootNode = (PTOMLVALUE)calloc(1, sizeof(TOMLVALUE));
    if (!doc->pRootNode) {
        table_free(root);
        free(doc);
        return TOML_ERROR_OUT_OF_MEMORY;
    }
    doc->pRootNode->ulType = TOML_TYPE_TABLE;
    doc->pRootNode->u.pTable = root;
    *phToml = (HTOMLDOC)doc;
    return TOML_NO_ERROR;
}

/**
 * @brief Close a document.
 *
 * Releases all internal buffers, including any active Find cursors and
 * node handles associated with the document.
 *
 * @param[in] hToml  Document handle. NULLHANDLE is accepted and treated
 *                   as a no-op.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success. Also returned for
 *                                   NULLHANDLE.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 *
 * @warning Calling TomlClose twice with the same handle is undefined.
 * @see TomlOpen
 */
APIRET APIENTRY TomlClose(HTOMLDOC hToml) {
    PTOMLDOC doc;
    if (hToml == NULLHANDLE) return TOML_NO_ERROR;
    doc = as_doc(hToml);
    if (!doc) return TOML_ERROR_INVALID_HANDLE;
    {
        PTOMLFIND f = doc->pFirstFind;
        while (f) {
            PTOMLFIND nxt = f->pNext;
            free(f->pszPattern);
            free(f);
            f = nxt;
        }
    }
    /* Free only the wrapper value; the table itself is released below
     * by table_free(doc->pRoot). */
    free(doc->pRootNode);
    table_free(doc->pRoot);
    free(doc);
    return TOML_NO_ERROR;
}

/* ==================================================================
 * Path navigation
 * ================================================================== */

static PTOMLVALUE find_path(PTOMLTABLE pRoot, PCSZ pszPath) {
    char *copy, *tok, *p;
    PTOMLTABLE t;
    PTOMLVALUE cur = NULL;

    if (!pRoot || !pszPath || !*pszPath) return NULL;
    copy = strdup(pszPath);
    if (!copy) return NULL;

    t = pRoot;
    p = copy;
    while (*p) {
        PTOMLVALUE next;
        tok = p;
        while (*p && *p != '.') p++;
        if (*p == '.') { *p = '\0'; p++; }
        next = table_find_value(t, tok);
        if (!next) { free(copy); return NULL; }
        cur = next;
        if (*p) {
            if (cur->ulType != TOML_TYPE_TABLE) { free(copy); return NULL; }
            t = cur->u.pTable;
        }
    }
    free(copy);
    return cur;
}

/* ==================================================================
 * Query API
 * ================================================================== */

/**
 * @brief Query the type of a value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path "a.b.c". Not NULL, not empty.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL or path empty.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 */
APIRET APIENTRY TomlQueryType(HTOMLDOC hToml, PCSZ pszPath, PULONG pulType) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !pulType) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    *pulType = pv->ulType;
    return TOML_NO_ERROR;
}

static APIRET copy_string_out(PCSZ src, PSZ dst, ULONG ulSize,
                              PULONG pulSize) {
    size_t n = src ? strlen(src) : 0;
    if (pulSize) *pulSize = (ULONG)n;
    if (ulSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(dst, src ? src : "", n);
    dst[n] = '\0';
    return TOML_NO_ERROR;
}

/**
 * @brief Query a string value by dotted path.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND       Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH   Value is neither STRING nor
 *                                    DATETIME.
 * @retval TOML_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY TomlQueryString(HTOMLDOC hToml, PCSZ pszPath,
                                PSZ pszBuffer, ULONG ulBufSize,
                                PULONG pulSize) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    PCSZ s;
    size_t n;

    if (!doc || !pszPath) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_STRING && pv->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;

    s = pv->u.pszString;
    n = s ? strlen(s) : 0;

    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_NO_ERROR;
    }
    if (!pszBuffer) return TOML_ERROR_INVALID_PARAM;
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, s ? s : "", n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return TOML_NO_ERROR;
}

/**
 * @brief Query an integer value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL, not empty.
 * @param[out] pllValue Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Value is not INTEGER.
 *                                   *pllValue = 0.
 */
APIRET APIENTRY TomlQueryInteger(HTOMLDOC hToml, PCSZ pszPath,
                                 PLONGLONG pllValue) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !pllValue) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_INTEGER) {
        *pllValue = 0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pllValue = pv->u.llInteger;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a floating-point value by dotted path.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[out] pdblValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Value is not FLOAT.
 *                                   *pdblValue = 0.0.
 */
APIRET APIENTRY TomlQueryFloat(HTOMLDOC hToml, PCSZ pszPath,
                               double *pdblValue) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !pdblValue) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_FLOAT) {
        *pdblValue = 0.0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pdblValue = pv->u.dblFloat;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a boolean value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL, not empty.
 * @param[out] pfValue  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Value is not BOOLEAN.
 *                                   *pfValue = FALSE_.
 */
APIRET APIENTRY TomlQueryBoolean(HTOMLDOC hToml, PCSZ pszPath,
                                 PBOOL pfValue) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !pfValue) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_BOOLEAN) {
        *pfValue = FALSE_;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pfValue = pv->u.fBoolean;
    return TOML_NO_ERROR;
}

/**
 * @brief Query the number of elements in an array by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL, not empty.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Value is not ARRAY.
 */
APIRET APIENTRY TomlQueryArrayCount(HTOMLDOC hToml, PCSZ pszPath,
                                    PULONG pulCount) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !pulCount) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    *pulCount = pv->u.pArray->ulCount;
    return TOML_NO_ERROR;
}

/**
 * @brief Query the type of one array element by index.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL, not empty.
 * @param[in]  ulIndex  Element index.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Path does not refer to an array.
 * @retval TOML_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET APIENTRY TomlQueryArrayType(HTOMLDOC hToml, PCSZ pszPath,
                                   ULONG ulIndex, PULONG pulType) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !pulType) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pv->u.pArray->ulCount) return TOML_ERROR_INDEX_RANGE;
    *pulType = pv->u.pArray->paItems[ulIndex]->ulType;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a string element of an array by index.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[in]  ulIndex    Element index.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND       Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH   Element is neither STRING nor
 *                                    DATETIME.
 * @retval TOML_ERROR_INDEX_RANGE     Index out of range.
 * @retval TOML_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY TomlQueryArrayString(HTOMLDOC hToml, PCSZ pszPath,
                                     ULONG ulIndex, PSZ pszBuffer,
                                     ULONG ulBufSize, PULONG pulSize) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv, item;
    PCSZ s;
    size_t n;

    if (!doc || !pszPath) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pv->u.pArray->ulCount) return TOML_ERROR_INDEX_RANGE;
    item = pv->u.pArray->paItems[ulIndex];
    if (item->ulType != TOML_TYPE_STRING &&
        item->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;

    s = item->u.pszString;
    n = s ? strlen(s) : 0;
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_NO_ERROR;
    }
    if (!pszBuffer) return TOML_ERROR_INVALID_PARAM;
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, s ? s : "", n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return TOML_NO_ERROR;
}

/**
 * @brief Query an integer element of an array by index.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path. Not NULL, not empty.
 * @param[in]  ulIndex   Element index.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Element is not INTEGER.
 * @retval TOML_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET APIENTRY TomlQueryArrayInteger(HTOMLDOC hToml, PCSZ pszPath,
                                      ULONG ulIndex, PLONGLONG pllValue) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv, item;
    if (!doc || !pszPath || !pllValue) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pv->u.pArray->ulCount) return TOML_ERROR_INDEX_RANGE;
    item = pv->u.pArray->paItems[ulIndex];
    if (item->ulType != TOML_TYPE_INTEGER) return TOML_ERROR_TYPE_MISMATCH;
    *pllValue = item->u.llInteger;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a floating-point element of an array by index.
 *
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path. Not NULL, not empty.
 * @param[in]  ulIndex    Element index.
 * @param[out] pdblValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Element is not FLOAT.
 * @retval TOML_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET APIENTRY TomlQueryArrayFloat(HTOMLDOC hToml, PCSZ pszPath,
                                    ULONG ulIndex, double *pdblValue) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv, item;
    if (!doc || !pszPath || !pdblValue) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pv->u.pArray->ulCount) return TOML_ERROR_INDEX_RANGE;
    item = pv->u.pArray->paItems[ulIndex];
    if (item->ulType != TOML_TYPE_FLOAT) return TOML_ERROR_TYPE_MISMATCH;
    *pdblValue = item->u.dblFloat;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a boolean element of an array by index.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path. Not NULL, not empty.
 * @param[in]  ulIndex  Element index.
 * @param[out] pfValue  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH  Element is not BOOLEAN.
 * @retval TOML_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET APIENTRY TomlQueryArrayBoolean(HTOMLDOC hToml, PCSZ pszPath,
                                      ULONG ulIndex, PBOOL pfValue) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv, item;
    if (!doc || !pszPath || !pfValue) return TOML_ERROR_INVALID_PARAM;
    pv = find_path(doc->pRoot, pszPath);
    if (!pv) return TOML_ERROR_NOT_FOUND;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pv->u.pArray->ulCount) return TOML_ERROR_INDEX_RANGE;
    item = pv->u.pArray->paItems[ulIndex];
    if (item->ulType != TOML_TYPE_BOOLEAN) return TOML_ERROR_TYPE_MISMATCH;
    *pfValue = item->u.fBoolean;
    return TOML_NO_ERROR;
}

/* ==================================================================
 * DOM-style traversal
 * ================================================================== */

static PTOMLVALUE as_node(HTOMLNODE h) { return (PTOMLVALUE)h; }

/**
 * @brief Obtain a node handle for a value by dotted path.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path "a.b.c". Not NULL. "" for root.
 * @param[out] phNode   Node receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND      Path not found.
 */
APIRET APIENTRY TomlQueryNode(HTOMLDOC hToml, PCSZ pszPath,
                              HTOMLNODE *phNode) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    if (!doc || !pszPath || !phNode) return TOML_ERROR_INVALID_PARAM;
    *phNode = NULLHANDLE;
    if (!*pszPath) {
        pv = doc->pRootNode;
    } else {
        pv = find_path(doc->pRoot, pszPath);
    }
    if (!pv) return TOML_ERROR_NOT_FOUND;
    *phNode = (HTOMLNODE)pv;
    return TOML_NO_ERROR;
}

/**
 * @brief Obtain a node handle for the root table.
 *
 * @param[in]  hToml   Handle. Not NULLHANDLE.
 * @param[out] phNode  Node receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 */
APIRET APIENTRY TomlQueryRootNode(HTOMLDOC hToml, HTOMLNODE *phNode) {
    PTOMLDOC doc = as_doc(hToml);
    if (!doc || !phNode) return TOML_ERROR_INVALID_PARAM;
    *phNode = (HTOMLNODE)doc->pRootNode;
    return TOML_NO_ERROR;
}

/**
 * @brief Query the type of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of TOML_TYPE_*. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 */
APIRET APIENTRY TomlNodeGetType(HTOMLNODE hNode, PULONG pulType) {
    PTOMLVALUE pv = as_node(hNode);
    if (!pv || !pulType) return TOML_ERROR_INVALID_PARAM;
    *pulType = pv->ulType;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a string value of a node.
 *
 * If pszBuffer is NULL and ulBufSize is 0, performs a size query only
 * and returns the required size (including NUL) in *pulSize.
 *
 * @param[in]  hNode      Node handle. Not NULLHANDLE.
 * @param[out] pszBuffer  Output buffer. Not NULL unless size-query.
 * @param[in]  ulBufSize  Size of pszBuffer in bytes.
 * @param[out] pulSize    Optional. May be NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH   Node is neither STRING nor DATETIME.
 * @retval TOML_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY TomlNodeGetString(HTOMLNODE hNode, PSZ pszBuffer,
                                  ULONG ulBufSize, PULONG pulSize) {
    PTOMLVALUE pv = as_node(hNode);
    PCSZ s;
    size_t n;
    if (!pv) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_STRING && pv->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;
    s = pv->u.pszString;
    n = s ? strlen(s) : 0;
    if (pszBuffer == NULL && ulBufSize == 0) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_NO_ERROR;
    }
    if (!pszBuffer) return TOML_ERROR_INVALID_PARAM;
    if (ulBufSize < n + 1) {
        if (pulSize) *pulSize = (ULONG)(n + 1);
        return TOML_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuffer, s ? s : "", n);
    pszBuffer[n] = '\0';
    if (pulSize) *pulSize = (ULONG)n;
    return TOML_NO_ERROR;
}

/**
 * @brief Query an integer value of a node.
 *
 * @param[in]  hNode     Node handle. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not INTEGER.
 */
APIRET APIENTRY TomlNodeGetInteger(HTOMLNODE hNode, PLONGLONG pllValue) {
    PTOMLVALUE pv = as_node(hNode);
    if (!pv || !pllValue) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_INTEGER) {
        *pllValue = 0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pllValue = pv->u.llInteger;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a floating-point value of a node.
 *
 * @param[in]  hNode      Node handle. Not NULLHANDLE.
 * @param[out] pdblValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not FLOAT.
 */
APIRET APIENTRY TomlNodeGetFloat(HTOMLNODE hNode, double *pdblValue) {
    PTOMLVALUE pv = as_node(hNode);
    if (!pv || !pdblValue) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_FLOAT) {
        *pdblValue = 0.0;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pdblValue = pv->u.dblFloat;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a boolean value of a node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pfValue  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not BOOLEAN.
 */
APIRET APIENTRY TomlNodeGetBoolean(HTOMLNODE hNode, PBOOL pfValue) {
    PTOMLVALUE pv = as_node(hNode);
    if (!pv || !pfValue) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_BOOLEAN) {
        *pfValue = FALSE_;
        return TOML_ERROR_TYPE_MISMATCH;
    }
    *pfValue = pv->u.fBoolean;
    return TOML_NO_ERROR;
}

/**
 * @brief Query the number of elements in an array node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not ARRAY.
 */
APIRET APIENTRY TomlNodeGetArrayCount(HTOMLNODE hNode, PULONG pulCount) {
    PTOMLVALUE pv = as_node(hNode);
    if (!pv || !pulCount) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    *pulCount = pv->u.pArray->ulCount;
    return TOML_NO_ERROR;
}

/**
 * @brief Query an array element by index.
 *
 * @param[in]  hNode    Node handle (array). Not NULLHANDLE.
 * @param[in]  ulIndex  Element index.
 * @param[out] phChild  Receiver of the element's node handle. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not ARRAY.
 * @retval TOML_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET APIENTRY TomlNodeGetArrayElement(HTOMLNODE hNode, ULONG ulIndex,
                                        HTOMLNODE *phChild) {
    PTOMLVALUE pv = as_node(hNode);
    if (!pv || !phChild) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_ARRAY) return TOML_ERROR_TYPE_MISMATCH;
    if (ulIndex >= pv->u.pArray->ulCount) {
        *phChild = NULLHANDLE;
        return TOML_ERROR_INDEX_RANGE;
    }
    *phChild = (HTOMLNODE)pv->u.pArray->paItems[ulIndex];
    return TOML_NO_ERROR;
}

/**
 * @brief Query the number of entries in a table node.
 *
 * @param[in]  hNode    Node handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not TABLE.
 */
APIRET APIENTRY TomlNodeGetTableCount(HTOMLNODE hNode, PULONG pulCount) {
    PTOMLVALUE pv = as_node(hNode);
    ULONG ulCount = 0;
    if (!pv || !pulCount) return TOML_ERROR_INVALID_PARAM;
    if (pv->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
    if (VectorGetCount(pv->u.pTable->hEntries, &ulCount) != NO_ERROR)
        return TOML_ERROR_INVALID_HANDLE;
    *pulCount = ulCount;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a table entry by key.
 *
 * @param[in]  hNode    Node handle (table). Not NULLHANDLE.
 * @param[in]  pszKey   Key. Not NULL.
 * @param[out] phChild  Receiver of the value's node handle. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH  Node is not TABLE.
 * @retval TOML_ERROR_NOT_FOUND      Key not found.
 */
APIRET APIENTRY TomlNodeGetTableEntryByKey(HTOMLNODE hNode, PCSZ pszKey,
                                           HTOMLNODE *phChild) {
    PTOMLVALUE pv = as_node(hNode);
    TOMLENTRY entry;
    if (!pv || !pszKey || !phChild) return TOML_ERROR_INVALID_PARAM;
    *phChild = NULLHANDLE;
    if (pv->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
    if (!table_find_copy(pv->u.pTable, pszKey, &entry))
        return TOML_ERROR_NOT_FOUND;
    *phChild = (HTOMLNODE)entry.pValue;
    return TOML_NO_ERROR;
}

/**
 * @brief Query a table entry by index.
 *
 * @param[in]  hNode         Node handle (table). Not NULLHANDLE.
 * @param[in]  ulIndex       Entry index.
 * @param[out] pszKeyBuffer  Key output buffer. Not NULL.
 * @param[in]  ulKeyBufSize  Size of pszKeyBuffer in bytes.
 * @param[out] pulKeyUsed    Optional. May be NULL.
 * @param[out] phChild       Receiver of the value's node handle. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_TYPE_MISMATCH   Node is not TABLE.
 * @retval TOML_ERROR_INDEX_RANGE     Index out of range.
 * @retval TOML_ERROR_BUFFER_OVERFLOW Key buffer too small.
 */
APIRET APIENTRY TomlNodeGetTableEntryByIndex(HTOMLNODE hNode, ULONG ulIndex,
                                             PSZ pszKeyBuffer,
                                             ULONG ulKeyBufSize,
                                             PULONG pulKeyUsed,
                                             HTOMLNODE *phChild) {
    PTOMLVALUE pv = as_node(hNode);
    TOMLENTRY entry;
    size_t klen;
    ULONG ulCount = 0;

    if (!pv || !pszKeyBuffer || !phChild) return TOML_ERROR_INVALID_PARAM;
    *phChild = NULLHANDLE;
    if (pv->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
    if (VectorGetCount(pv->u.pTable->hEntries, &ulCount) != NO_ERROR)
        return TOML_ERROR_INVALID_HANDLE;
    if (ulIndex >= ulCount) {
        return TOML_ERROR_INDEX_RANGE;
    }
    if (VectorGetItem(pv->u.pTable->hEntries, ulIndex, &entry,
                      (ULONG)sizeof(entry), NULL) != NO_ERROR)
        return TOML_ERROR_INDEX_RANGE;

    klen = strlen(entry.pszKey);
    if (ulKeyBufSize < klen + 1) {
        if (pulKeyUsed) *pulKeyUsed = (ULONG)(klen + 1);
        return TOML_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszKeyBuffer, entry.pszKey, klen);
    pszKeyBuffer[klen] = '\0';
    if (pulKeyUsed) *pulKeyUsed = (ULONG)klen;
    *phChild = (HTOMLNODE)entry.pValue;
    return TOML_NO_ERROR;
}

/* ==================================================================
 * Find API
 * ================================================================== */

static int glob_match(PCSZ pat, PCSZ txt) {
    while (*pat) {
        if (*pat == '*') {
            pat++;
            if (!*pat) return 1;
            while (*txt) {
                if (glob_match(pat, txt)) return 1;
                txt++;
            }
            return glob_match(pat, txt);
        }
        if (*pat == '?') {
            if (!*txt) return 0;
            pat++; txt++;
            continue;
        }
        if (*pat != *txt) return 0;
        pat++; txt++;
    }
    return *txt == '\0';
}

static int find_next_match(PTOMLTABLE pt, ULONG ulStart,
                           PCSZ pszPattern,
                           ULONG *pulNext,
                           PCSZ *ppszKey, PTOMLVALUE *ppValue) {
    ULONG i;
    ULONG ulCount = 0;
    if (VectorGetCount(pt->hEntries, &ulCount) != NO_ERROR) return -1;
    for (i = ulStart; i < ulCount; i++) {
        TOMLENTRY entry;
        if (VectorGetItem(pt->hEntries, i, &entry,
                          (ULONG)sizeof(entry), NULL) != NO_ERROR) return -1;
        if (glob_match(pszPattern, entry.pszKey)) {
            *pulNext = i + 1;
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
 * @param[in]  hToml      Handle. Not NULLHANDLE.
 * @param[in]  pszPath    Path to the table. Not NULL. Use "" for root.
 * @param[in]  pszPattern Pattern. Not NULL.
 * @param[out] phFind     Cursor receiver. Not NULL.
 * @param[out] pulType    Optional. May be NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval TOML_ERROR_NOT_FOUND       Path not found.
 * @retval TOML_ERROR_TYPE_MISMATCH   Path does not refer to a table.
 * @retval TOML_ERROR_NO_MORE_ENTRIES No entry matches the pattern.
 * @retval TOML_ERROR_OUT_OF_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY TomlFindFirst(HTOMLDOC hToml, PCSZ pszPath,
                              PCSZ pszPattern, HTOMLFIND *phFind,
                              PULONG pulType) {
    PTOMLDOC doc = as_doc(hToml);
    PTOMLVALUE pv;
    PTOMLTABLE t;
    PTOMLFIND f;
    PCSZ pszKey = NULL;
    PTOMLVALUE pVal = NULL;
    ULONG next = 0;

    if (!doc || !pszPattern || !phFind) return TOML_ERROR_INVALID_PARAM;
    *phFind = NULLHANDLE;

    if (pszPath && *pszPath) {
        pv = find_path(doc->pRoot, pszPath);
        if (!pv) return TOML_ERROR_NOT_FOUND;
        if (pv->ulType != TOML_TYPE_TABLE) return TOML_ERROR_TYPE_MISMATCH;
        t = pv->u.pTable;
    } else {
        t = doc->pRoot;
    }
    if (find_next_match(t, 0, pszPattern, &next, &pszKey, &pVal) != 0)
        return TOML_ERROR_NO_MORE_ENTRIES;

    f = (PTOMLFIND)calloc(1, sizeof(TOMLFIND));
    if (!f) return TOML_ERROR_OUT_OF_MEMORY;
    f->pDoc = doc;
    f->pTable = t;
    f->ulCurrent = next;
    f->pszPattern = strdup(pszPattern);
    if (!f->pszPattern) { free(f); return TOML_ERROR_OUT_OF_MEMORY; }
    f->pNext = doc->pFirstFind;
    f->pszCurrentKey = pszKey;
    f->pCurrentValue = pVal;
    doc->pFirstFind = f;

    *phFind = (HTOMLFIND)f;
    if (pulType) *pulType = pVal->ulType;
    return TOML_NO_ERROR;
}

/**
 * @brief Advance the cursor to the next matching entry.
 *
 * @param[in]  hFind    Cursor from TomlFindFirst. Not NULLHANDLE.
 * @param[out] pulType  Optional. May be NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval TOML_ERROR_NO_MORE_ENTRIES No more matching entries.
 */
APIRET APIENTRY TomlFindNext(HTOMLFIND hFind, PULONG pulType) {
    PTOMLFIND f = as_find(hFind);
    PCSZ pszKey = NULL;
    PTOMLVALUE pVal = NULL;
    ULONG next = 0;
    if (!f) return TOML_ERROR_INVALID_HANDLE;
    if (find_next_match(f->pTable, f->ulCurrent, f->pszPattern,
                        &next, &pszKey, &pVal) != 0)
        return TOML_ERROR_NO_MORE_ENTRIES;
    f->ulCurrent = next;
    f->pszCurrentKey = pszKey;
    f->pCurrentValue = pVal;
    if (pulType) *pulType = pVal->ulType;
    return TOML_NO_ERROR;
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
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval TOML_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY TomlFindKey(HTOMLFIND hFind,
                            PSZ pszBuffer, ULONG ulBufSize,
                            PULONG pulSize) {
    PTOMLFIND f = as_find(hFind);
    if (!f || !pszBuffer) return TOML_ERROR_INVALID_PARAM;
    if (!f->pszCurrentKey) return TOML_ERROR_NOT_FOUND;
    return copy_string_out(f->pszCurrentKey, pszBuffer, ulBufSize, pulSize);
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
 * @retval TOML_NO_ERROR              Success.
 * @retval TOML_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE  Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH   Current entry is neither STRING
 *                                    nor DATETIME.
 * @retval TOML_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET APIENTRY TomlFindString(HTOMLFIND hFind,
                               PSZ pszBuffer, ULONG ulBufSize,
                               PULONG pulSize) {
    PTOMLFIND f = as_find(hFind);
    if (!f || !pszBuffer) return TOML_ERROR_INVALID_PARAM;
    if (!f->pCurrentValue) return TOML_ERROR_NOT_FOUND;
    if (f->pCurrentValue->ulType != TOML_TYPE_STRING &&
        f->pCurrentValue->ulType != TOML_TYPE_DATETIME)
        return TOML_ERROR_TYPE_MISMATCH;
    return copy_string_out(f->pCurrentValue->u.pszString,
                           pszBuffer, ulBufSize, pulSize);
}

/**
 * @brief Retrieve the integer value of the current entry.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH  Current entry is not INTEGER.
 */
APIRET APIENTRY TomlFindInteger(HTOMLFIND hFind, PLONGLONG pllValue) {
    PTOMLFIND f = as_find(hFind);
    if (!f || !pllValue) return TOML_ERROR_INVALID_PARAM;
    if (!f->pCurrentValue) return TOML_ERROR_NOT_FOUND;
    if (f->pCurrentValue->ulType != TOML_TYPE_INTEGER)
        return TOML_ERROR_TYPE_MISMATCH;
    *pllValue = f->pCurrentValue->u.llInteger;
    return TOML_NO_ERROR;
}

/**
 * @brief Retrieve the floating-point value of the current entry.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pdblValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH  Current entry is not FLOAT.
 */
APIRET APIENTRY TomlFindFloat(HTOMLFIND hFind, double *pdblValue) {
    PTOMLFIND f = as_find(hFind);
    if (!f || !pdblValue) return TOML_ERROR_INVALID_PARAM;
    if (!f->pCurrentValue) return TOML_ERROR_NOT_FOUND;
    if (f->pCurrentValue->ulType != TOML_TYPE_FLOAT)
        return TOML_ERROR_TYPE_MISMATCH;
    *pdblValue = f->pCurrentValue->u.dblFloat;
    return TOML_NO_ERROR;
}

/**
 * @brief Retrieve the boolean value of the current entry.
 *
 * @param[in]  hFind    Cursor. Not NULLHANDLE.
 * @param[out] pfValue  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success.
 * @retval TOML_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 * @retval TOML_ERROR_TYPE_MISMATCH  Current entry is not BOOLEAN.
 */
APIRET APIENTRY TomlFindBoolean(HTOMLFIND hFind, PBOOL pfValue) {
    PTOMLFIND f = as_find(hFind);
    if (!f || !pfValue) return TOML_ERROR_INVALID_PARAM;
    if (!f->pCurrentValue) return TOML_ERROR_NOT_FOUND;
    if (f->pCurrentValue->ulType != TOML_TYPE_BOOLEAN)
        return TOML_ERROR_TYPE_MISMATCH;
    *pfValue = f->pCurrentValue->u.fBoolean;
    return TOML_NO_ERROR;
}

/**
 * @brief Close an enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is accepted and treated as a
 *                   no-op.
 *
 * @return APIRET
 * @retval TOML_NO_ERROR             Success. Also returned for
 *                                   NULLHANDLE.
 * @retval TOML_ERROR_INVALID_HANDLE Handle is not recognized.
 */
APIRET APIENTRY TomlFindClose(HTOMLFIND hFind) {
    PTOMLFIND f = as_find(hFind);
    PTOMLFIND *pp;
    if (!f) return TOML_NO_ERROR;
    pp = &f->pDoc->pFirstFind;
    while (*pp) {
        if (*pp == f) { *pp = f->pNext; break; }
        pp = &(*pp)->pNext;
    }
    free(f->pszPattern);
    free(f);
    return TOML_NO_ERROR;
}
