/* spdx.c - shared SPDX utility functions.
 * Part of the reuse support layer. */

/**
 * @file spdx.c
 * @brief Implementation of the shared SPDX utility functions.
 *
 * Copyright (c) osFree Project 2026, <http://www.osFree.org>
 *   for licence see licence.txt in root directory, or project website
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx.h"

/* ==================================================================
 * File helpers
 * ================================================================== */

APIRET APIENTRY SpdxReadFileAll(PCSZ pszPath, PSZ *ppszText, PLONG pcbSize) {
    FILE *f;
    long sz;
    char *buf;
    size_t rd;

    if (!pszPath || !ppszText) return ERROR_INVALID_PARAMETER;
    *ppszText = NULL;
    if (pcbSize) *pcbSize = 0;

    f = fopen(pszPath, "rb");
    if (!f) return ERROR_OPEN_FAILED;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return ERROR_READ_FAULT; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return ERROR_READ_FAULT; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return ERROR_READ_FAULT; }

    buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return ERROR_NOT_ENOUGH_MEMORY; }

    rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) { free(buf); return ERROR_READ_FAULT; }

    buf[sz] = '\0';
    if (pcbSize) *pcbSize = (LONG)sz;
    *ppszText = buf;
    return NO_ERROR;
}

PCSZ APIENTRY SpdxGetFileName(PCSZ pszPath) {
    PCSZ pSlash;
    PCSZ pBackslash;

    if (!pszPath) return NULL;
    pSlash = strrchr(pszPath, '/');
    pBackslash = strrchr(pszPath, '\\');
    if (pBackslash && (!pSlash || pBackslash > pSlash))
        return pBackslash + 1;
    if (pSlash)
        return pSlash + 1;
    return pszPath;
}

/* ==================================================================
 * SPDX expression helpers
 * ================================================================== */

APIRET APIENTRY SpdxExpressionCollectIds(PCSZ pszExpr, HSTRSET hOut) {
    const char *delims = " \t()";
    char *copy;
    char *tok;
    APIRET rc;

    if (!pszExpr) return ERROR_INVALID_PARAMETER;
    if (hOut == NULLHANDLE) return ERROR_INVALID_HANDLE;

    copy = strdup(pszExpr);
    if (!copy) return ERROR_NOT_ENOUGH_MEMORY;

    tok = strtok(copy, delims);
    while (tok) {
        if (strcmp(tok, "AND") != 0 &&
            strcmp(tok, "OR")  != 0 &&
            strcmp(tok, "WITH") != 0 &&
            tok[0] != '\0') {
            rc = StrSetAdd(hOut, tok);
            if (rc != NO_ERROR) {
                free(copy);
                return rc;
            }
        }
        tok = strtok(NULL, delims);
    }
    free(copy);
    return NO_ERROR;
}

/* ==================================================================
 * Text helpers
 * ================================================================== */

APIRET APIENTRY SpdxNormalizeText(PCSZ pszSrc, PSZ *ppszOut) {
    size_t n, i, j;
    size_t out_len;
    char *buf;
    char *p, *w;

    if (!pszSrc || !ppszOut) return ERROR_INVALID_PARAMETER;
    *ppszOut = NULL;

    n = strlen(pszSrc);
    buf = (char*)malloc(n + 1);
    if (!buf) return ERROR_NOT_ENOUGH_MEMORY;

    i = 0;
    if (n >= 3 && (unsigned char)pszSrc[0] == 0xEF &&
                  (unsigned char)pszSrc[1] == 0xBB &&
                  (unsigned char)pszSrc[2] == 0xBF) {
        i = 3;
    }

    /* Normalize newlines: CRLF and lone CR become LF. */
    j = 0;
    while (i < n) {
        if (pszSrc[i] == '\r') {
            buf[j++] = '\n';
            if (i + 1 < n && pszSrc[i+1] == '\n') i++;
            i++;
        } else {
            buf[j++] = pszSrc[i++];
        }
    }
    buf[j] = '\0';

    /* Strip trailing whitespace per line; drop trailing empty lines. */
    p = buf;
    w = buf;
    while (*p) {
        char *line_start = p;
        char *line_end;
        while (*p && *p != '\n') p++;
        line_end = p;
        while (line_end > line_start &&
               (line_end[-1] == ' ' || line_end[-1] == '\t')) {
            line_end--;
        }
        memmove(w, line_start, (size_t)(line_end - line_start));
        w += (line_end - line_start);
        if (*p == '\n') {
            *w++ = '\n';
            p++;
        }
    }
    *w = '\0';
    out_len = (size_t)(w - buf);
    while (out_len > 0 && buf[out_len-1] == '\n') out_len--;
    buf[out_len] = '\0';

    *ppszOut = buf;
    return NO_ERROR;
}
