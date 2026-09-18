/* spdx_tag.c - SPDX tags and snippet parser (C89) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_tag.h"

/* Raised from 4096 to avoid truncating long copyright expressions. */
#define MAX_LINE 16384

/* Skip a UTF-8 BOM at the start of the buffer. */
static const char *skip_bom(const char *p) {
    if ((unsigned char)p[0] == 0xEF &&
        (unsigned char)p[1] == 0xBB &&
        (unsigned char)p[2] == 0xBF) {
        return p + 3;
    }
    return p;
}

/* Skip leading whitespace and a comment marker. */
static const char *skip_comment_prefix(const char *p) {
    while (*p == ' ' || *p == '\t') p++;
    if (p[0] == '/' && p[1] == '/') { p += 2; }
    else if (p[0] == '/' && p[1] == '*') { p += 2; }
    else if (p[0] == '#') { p++; }
    else if (p[0] == ';') { p++; }
    else if (p[0] == '%') { p++; }
    else if (p[0] == '-' && p[1] == '-') { p += 2; }
    else if (p[0] == '*') { p++; }
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* Check whether the meaningful part of a line starts with
 * REUSE-IgnoreStart or REUSE-IgnoreEnd. Returns 1, 2, or 0. */
static int line_ignore_marker(const char *line) {
    const char *p = skip_comment_prefix(line);
    if (strncmp(p, "REUSE-IgnoreStart", 17) == 0) {
        char c = p[17];
        if (c == '\0' || c == ' ' || c == '\t' ||
            c == '\n' || c == '\r')
            return 1;
    }
    if (strncmp(p, "REUSE-IgnoreEnd", 15) == 0) {
        char c = p[15];
        if (c == '\0' || c == ' ' || c == '\t' ||
            c == '\n' || c == '\r')
            return 2;
    }
    return 0;
}

/* Check whether the meaningful part of a line starts with the given
 * tag (tag may end with a colon). After the tag there must be a
 * space, a tab, a colon, a newline, or end of string. */
static int line_starts_with_tag(const char *line, const char *tag) {
    size_t tlen = strlen(tag);
    const char *p = skip_comment_prefix(line);
    if (strncmp(p, tag, tlen) != 0) return 0;
    if (p[tlen] == ':' && tag[tlen - 1] != ':') return 1;
    if (p[tlen] == '\0' || p[tlen] == ' ' || p[tlen] == '\t' ||
        p[tlen] == '\n' || p[tlen] == '\r')
        return 1;
    return 0;
}

/* Return a pointer to the value after the tag (skipping the colon and
 * surrounding whitespace). If the tag is not found or lacks a colon,
 * returns NULL. */
static const char *tag_value(const char *line, const char *tag_with_colon) {
    size_t tlen = strlen(tag_with_colon);
    const char *p = skip_comment_prefix(line);
    if (strncmp(p, tag_with_colon, tlen) != 0) return NULL;
    p += tlen;
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* Strip trailing whitespace, newlines and comment-closing markers. */
static void strip_trailing_markers(char *str) {
    size_t len = strlen(str);
    while (len > 0) {
        if (len >= 2 && str[len-2] == '*' && str[len-1] == '/') {
            len -= 2;
        } else if (len >= 2 && str[len-2] == '/' && str[len-1] == '/') {
            len -= 2;
        } else if (str[len-1] == ' ' || str[len-1] == '\t' ||
                   str[len-1] == '\n' || str[len-1] == '\r') {
            len -= 1;
        } else {
            break;
        }
    }
    str[len] = '\0';
}

/* ------------------------------------------------------------------ */
/* Snippet list                                                        */
/* ------------------------------------------------------------------ */

void APIENTRY SpdxSnippetListInit(SPDXSNIPPETLIST *pList) {
    pList->paItems = NULL;
    pList->nCount = 0;
    pList->nCapacity = 0;
}

void APIENTRY SpdxSnippetListFree(SPDXSNIPPETLIST *pList) {
    int i;
    if (!pList) return;
    for (i = 0; i < pList->nCount; i++) {
        free(pList->paItems[i].pszLicense);
        free(pList->paItems[i].pszCopyright);
    }
    free(pList->paItems);
    pList->paItems = NULL;
    pList->nCount = 0;
    pList->nCapacity = 0;
}

static PSPDXSNIPPET spdx_snippet_list_add(SPDXSNIPPETLIST *pList) {
    PSPDXSNIPPET pItem;
    if (pList->nCount >= pList->nCapacity) {
        int nNewCap = pList->nCapacity ? pList->nCapacity * 2 : 4;
        PSPDXSNIPPET paNew = (PSPDXSNIPPET)realloc(pList->paItems,
            (size_t)nNewCap * sizeof(SPDXSNIPPET));
        if (!paNew) return NULL;
        pList->paItems = paNew;
        pList->nCapacity = nNewCap;
    }
    pItem = &pList->paItems[pList->nCount++];
    memset(pItem, 0, sizeof(*pItem));
    return pItem;
}

/* ------------------------------------------------------------------ */
/* Public functions                                                    */
/* ------------------------------------------------------------------ */

APIRET APIENTRY SpdxFileHasTag(PCSZ pszFilename, PBOOL pfHasTag) {
    FILE *f;
    char line[MAX_LINE];
    int ignore = 0;
    int first_line = 1;

    if (!pszFilename || !pfHasTag) return SPDX_TAG_ERROR_INVALID_PARAM;
    *pfHasTag = FALSE_;

    f = fopen(pszFilename, "r");
    if (!f) return SPDX_TAG_ERROR_OPEN_FAILED;
    while (fgets(line, sizeof(line), f)) {
        char *work = line;
        int marker;

        if (first_line) {
            work = (char *)skip_bom(line);
            first_line = 0;
        }

        marker = line_ignore_marker(work);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        if (strstr(work, "SPDX-License-Identifier:")) {
            fclose(f);
            *pfHasTag = TRUE_;
            return SPDX_TAG_NO_ERROR;
        }
    }
    fclose(f);
    return SPDX_TAG_NO_ERROR;
}

APIRET APIENTRY SpdxFileGetLicense(PCSZ pszFilename, PSZ *ppszLicense) {
    FILE *f;
    char line[MAX_LINE];
    const char *needle = "SPDX-License-Identifier:";
    const size_t needle_len = strlen(needle);
    char *pos;
    char *result;
    char *start;
    size_t len;
    int ignore = 0;
    int first_line = 1;

    if (!pszFilename || !ppszLicense) return SPDX_TAG_ERROR_INVALID_PARAM;
    *ppszLicense = NULL;

    f = fopen(pszFilename, "r");
    if (!f) return SPDX_TAG_ERROR_OPEN_FAILED;

    while (fgets(line, sizeof(line), f)) {
        char *work = line;
        int marker;

        if (first_line) {
            work = (char *)skip_bom(line);
            first_line = 0;
        }

        marker = line_ignore_marker(work);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        pos = strstr(work, needle);
        if (pos) {
            start = (char *)skip_comment_prefix(pos + needle_len);
            strip_trailing_markers(start);
            len = strlen(start);
            if (len > 0) {
                result = (char *)malloc(len + 1);
                if (!result) {
                    fclose(f);
                    return SPDX_TAG_ERROR_OUT_OF_MEMORY;
                }
                memcpy(result, start, len);
                result[len] = '\0';
                fclose(f);
                *ppszLicense = result;
                return SPDX_TAG_NO_ERROR;
            }
        }
    }
    fclose(f);
    return SPDX_TAG_ERROR_NOT_FOUND;
}

APIRET APIENTRY SpdxFileGetCopyright(PCSZ pszFilename, PSZ *ppszCopyright) {
    FILE *f;
    char line[MAX_LINE];
    char *result = NULL;
    size_t result_len = 0;
    int ignore = 0;
    int first_line = 1;

    if (!pszFilename || !ppszCopyright) return SPDX_TAG_ERROR_INVALID_PARAM;
    *ppszCopyright = NULL;

    f = fopen(pszFilename, "r");
    if (!f) return SPDX_TAG_ERROR_OPEN_FAILED;

    while (fgets(line, sizeof(line), f)) {
        char *p;
        char *start;
        size_t len;
        int marker;

        if (first_line) {
            p = (char *)skip_bom(line);
            first_line = 0;
        } else {
            p = line;
        }

        marker = line_ignore_marker(p);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        p = (char *)skip_comment_prefix(p);

        if (strncmp(p, "SPDX-FileCopyrightText:", 23) == 0) {
            p += 23;
            while (*p == ' ' || *p == '\t') p++;
            start = p;
        } else if (strncmp(p, "Copyright", 9) == 0 &&
                   (p[9] == ' ' || p[9] == '\t')) {
            p += 9;
            while (*p == ' ' || *p == '\t') p++;
            start = p;
        } else if ((unsigned char)*p == 0xC2 &&
                   (unsigned char)*(p+1) == 0xA9) {
            p += 2;
            while (*p == ' ' || *p == '\t') p++;
            start = p;
        } else {
            continue;
        }

        strip_trailing_markers(start);
        len = strlen(start);
        if (len > 0) {
            char *new_result;
            size_t new_size = (result ? result_len + 1 + len + 1 : len + 1);
            new_result = (char *)realloc(result, new_size);
            if (!new_result) {
                free(result);
                fclose(f);
                return SPDX_TAG_ERROR_OUT_OF_MEMORY;
            }
            result = new_result;
            if (result_len > 0) {
                result[result_len++] = '\n';
            }
            memcpy(result + result_len, start, len);
            result_len += len;
            result[result_len] = '\0';
        }
    }
    fclose(f);

    if (!result) return SPDX_TAG_ERROR_NOT_FOUND;
    *ppszCopyright = result;
    return SPDX_TAG_NO_ERROR;
}

/* Append a line to an accumulator with a '\n' separator. */
static char *append_line(char *acc, size_t *acc_len, const char *line) {
    size_t llen = strlen(line);
    char *na;
    size_t new_size;

    if (llen == 0) return acc;

    new_size = (*acc_len ? *acc_len + 1 : 0) + llen + 1;
    na = (char*)realloc(acc, new_size);
    if (!na) return acc;

    if (*acc_len > 0) {
        na[(*acc_len)++] = '\n';
    }
    memcpy(na + *acc_len, line, llen);
    *acc_len += llen;
    na[*acc_len] = '\0';
    return na;
}

APIRET APIENTRY SpdxFileGetSnippets(PCSZ pszFilename,
                                    SPDXSNIPPETLIST *pOut) {
    FILE *f;
    char line[MAX_LINE];
    int lineno = 0;
    int first_line = 1;
    int in_snippet = 0;
    int snippet_start_line = 0;
    char *snippet_license = NULL;
    char *snippet_copyright = NULL;
    size_t snippet_copyright_len = 0;
    int ignore = 0;

    if (!pszFilename || !pOut) return SPDX_TAG_ERROR_INVALID_PARAM;

    SpdxSnippetListInit(pOut);

    f = fopen(pszFilename, "r");
    if (!f) return SPDX_TAG_ERROR_OPEN_FAILED;

    while (fgets(line, sizeof(line), f)) {
        char *work = line;
        int marker;

        lineno++;

        if (first_line) {
            work = (char *)skip_bom(line);
            first_line = 0;
        }

        marker = line_ignore_marker(work);
        if (marker == 1) { ignore = 1; continue; }
        if (marker == 2) { ignore = 0; continue; }
        if (ignore) continue;

        if (line_starts_with_tag(work, "SPDX-SnippetBegin")) {
            if (in_snippet) {
                fprintf(stderr,
                        "ERROR: %s:%d: nested SPDX-SnippetBegin.\n"
                        "       Fix one of:\n"
                        "         - remove the duplicate SPDX-SnippetBegin;\n"
                        "         - or add a matching SPDX-SnippetEnd "
                        "before the nested one.\n",
                        pszFilename, lineno);
                free(snippet_license);
                free(snippet_copyright);
                SpdxSnippetListFree(pOut);
                fclose(f);
                return SPDX_TAG_ERROR_SYNTAX;
            }
            in_snippet = 1;
            snippet_start_line = lineno;
            free(snippet_license); snippet_license = NULL;
            free(snippet_copyright); snippet_copyright = NULL;
            snippet_copyright_len = 0;
            continue;
        }

        if (line_starts_with_tag(work, "SPDX-SnippetEnd")) {
            PSPDXSNIPPET pS;
            if (!in_snippet) {
                fprintf(stderr,
                        "ERROR: %s:%d: SPDX-SnippetEnd without matching "
                        "SPDX-SnippetBegin.\n"
                        "       Fix one of:\n"
                        "         - remove this SPDX-SnippetEnd;\n"
                        "         - or add SPDX-SnippetBegin before the "
                        "snippet.\n",
                        pszFilename, lineno);
                SpdxSnippetListFree(pOut);
                fclose(f);
                return SPDX_TAG_ERROR_SYNTAX;
            }
            pS = spdx_snippet_list_add(pOut);
            if (!pS) {
                free(snippet_license);
                free(snippet_copyright);
                SpdxSnippetListFree(pOut);
                fclose(f);
                return SPDX_TAG_ERROR_OUT_OF_MEMORY;
            }
            pS->nLineStart  = snippet_start_line;
            pS->nLineEnd    = lineno;
            pS->pszLicense  = snippet_license;
            pS->pszCopyright = snippet_copyright;
            snippet_license = NULL;
            snippet_copyright = NULL;
            snippet_copyright_len = 0;
            in_snippet = 0;
            continue;
        }

        if (!in_snippet) continue;

        {
            const char *val;

            val = tag_value(work, "SPDX-SnippetCopyrightText:");
            if (val) {
                char tmp[MAX_LINE];
                size_t l = strlen(val);
                if (l >= sizeof(tmp)) l = sizeof(tmp) - 1;
                memcpy(tmp, val, l);
                tmp[l] = '\0';
                strip_trailing_markers(tmp);
                snippet_copyright = append_line(snippet_copyright,
                                                &snippet_copyright_len,
                                                tmp);
                continue;
            }

            val = tag_value(work, "SPDX-License-Identifier:");
            if (val) {
                char tmp[MAX_LINE];
                size_t l = strlen(val);
                if (l >= sizeof(tmp)) l = sizeof(tmp) - 1;
                memcpy(tmp, val, l);
                tmp[l] = '\0';
                strip_trailing_markers(tmp);
                free(snippet_license);
                snippet_license = (char*)malloc(strlen(tmp) + 1);
                if (snippet_license) strcpy(snippet_license, tmp);
                continue;
            }
        }
    }

    fclose(f);

    if (in_snippet) {
        fprintf(stderr,
                "ERROR: %s: unclosed SPDX-SnippetBegin at line %d.\n"
                "       Fix one of:\n"
                "         - add SPDX-SnippetEnd after the snippet;\n"
                "         - or remove SPDX-SnippetBegin if the code is not "
                "a snippet.\n",
                pszFilename, snippet_start_line);
        free(snippet_license);
        free(snippet_copyright);
        SpdxSnippetListFree(pOut);
        return SPDX_TAG_ERROR_SYNTAX;
    }

    return SPDX_TAG_NO_ERROR;
}
