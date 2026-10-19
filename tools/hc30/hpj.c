/*!
 * @file hpj.c
 * @brief HPJ parser implementation.
 *
 * @par References
 *  [1] Borland Languages Help Compiler User's Guide, 1991, chapter 8.
 *  [2] Microsoft Help Workshop, "Help Project File Sections", 1992.
 *  [3] Microsoft Help Workshop, "Building a Help File with Build Tags".
 *  [4] OS/2 V2.0 Vol.4, IBM, 1993, section 12.2.
 */
#include "hpj.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ==================================================================
 * Internal record types
 * ================================================================== */

/*!
 * @struct HpjOptionRec
 * @brief One [OPTIONS] entry.
 */
typedef struct { char* pszName; char* pszValue; } HpjOptionRec;
/*!
 * @struct HpjFileRec
 * @brief One [FILES] entry.
 */
typedef struct { char* pszFile; }                   HpjFileRec;
/*!
 * @struct HpjAliasRec
 * @brief One [ALIAS] entry.
 */
typedef struct { char* pszName; char* pszContext; } HpjAliasRec;
/*!
 * @struct HpjMapRec
 * @brief One numeric [MAP] entry.
 */
typedef struct { char* pszContext; ULONG ulNumber; }HpjMapRec;
/*!
 * @struct HpjDefineRec
 * @brief One #define symbol from [MAP].
 */
typedef struct { char* pszName; ULONG ulValue; }    HpjDefineRec;
/*!
 * @struct HpjIncludeRec
 * @brief One #include file from [MAP].
 */
typedef struct { char* pszFile; }                   HpjIncludeRec;
/*!
 * @struct HpjConfigRec
 * @brief One [CONFIG] macro line.
 */
typedef struct { char* pszText; }                   HpjConfigRec;
/*!
 * @struct HpjBitmapRec
 * @brief One [BITMAPS] entry.
 */
typedef struct { char* pszFile; }                   HpjBitmapRec;
/*!
 * @struct HpjBaggageRec
 * @brief One [BAGGAGE] entry.
 */
typedef struct { char* pszFile; }                   HpjBaggageRec;
/*!
 * @struct HpjBuildTagRec
 * @brief One [BUILDTAGS] entry.
 */
typedef struct { char* pszTag; }                    HpjBuildTagRec;

/*!
 * @struct HpjDocRec
 * @brief HPJ document state.
 */
typedef struct {
    HVECTOR vOptions;
    HVECTOR vFiles;
    HVECTOR vAliases;
    HVECTOR vMaps;
    HVECTOR vDefines;
    HVECTOR vIncludes;
    HVECTOR vWindows;
    HVECTOR vConfigs;
    HVECTOR vBitmaps;
    HVECTOR vBaggage;
    HVECTOR vBuildTags;
    CHAR    szBaseDir[512];
} HpjDocRec;

/*!
 * @def DOC_FROM_HANDLE
 * @brief Convert an HHPJ handle to its HpjDocRec pointer.
 * @param[in] h Handle.
 */
#define DOC_FROM_HANDLE(h) ((HpjDocRec*)(h))
/*!
 * @def DOC_HANDLE_FROM
 * @brief Convert an HpjDocRec pointer to an HHPJ handle.
 * @param[in] d Record pointer.
 */
#define DOC_HANDLE_FROM(d) ((HANDLE)(d))

/* ==================================================================
 * String helpers
 * ================================================================== */

/*!
 * @brief Trim leading and trailing whitespace in place.
 *
 * @param[in,out] s String to trim. Not NULL.
 * @return The new beginning of the string within the same buffer.
 */
static char* hpj_trim(char* s)
{
    char* p = s;
    char* e;
    while (*p && isspace((unsigned char)*p)) p++;
    e = p + strlen(p);
    while (e > p && isspace((unsigned char)e[-1])) e--;
    *e = '\0';
    return p;
}

/*!
 * @brief Strip a trailing ';' comment from a line, respecting quotes.
 *
 * @param[in,out] s Line to strip. Not NULL.
 */
static void hpj_strip_comment(char* s)
{
    BOOL fInQuote = FALSE;
    char* p = s;
    while (*p) {
        if (*p == '"') fInQuote = !fInQuote;
        else if (*p == ';' && !fInQuote) { *p = '\0'; return; }
        p++;
    }
}

/*!
 * @brief Copy the substring [pBegin, pEnd) into a heap-allocated string.
 *
 * Needed because OpenWatcom does not provide strndup and strdup only
 * copies the whole string up to NUL.
 *
 * @param[in] pBegin Start of the substring.
 * @param[in] pEnd   End of the substring (exclusive).
 *
 * @retval NULL Invalid range or allocation failure.
 * @return Newly allocated NUL-terminated string.
 */
static char* hpj_substr(const char* pBegin, const char* pEnd)
{
    size_t n;
    char* p;
    if (!pBegin || !pEnd || pEnd < pBegin) return NULL;
    n = (size_t)(pEnd - pBegin);
    p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, pBegin, n);
    p[n] = '\0';
    return p;
}

/*!
 * @brief Split a "name=value" line, trimming both sides.
 *
 * @param[in]  pszLine   Line to split. Not NULL.
 * @param[out] ppszName  Receiver for the name. Not NULL.
 * @param[out] ppszValue Receiver for the value. Not NULL.
 *
 * @retval 0  No '=' present, or allocation failure.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int hpj_split_kv(const char* pszLine, char** ppszName, char** ppszValue)
{
    const char* pEq = strchr(pszLine, '=');
    char* pName;
    char* pValue;
    if (!pEq) return 0;
    pName = hpj_substr(pszLine, pEq);
    if (!pName) return 0;
    {
        char* pT = hpj_trim(pName);
        if (pT != pName) memmove(pName, pT, strlen(pT) + 1);
    }
    pValue = strdup(pEq + 1);
    if (!pValue) { free(pName); return 0; }
    {
        char* pT = hpj_trim(pValue);
        if (pT != pValue) memmove(pValue, pT, strlen(pT) + 1);
    }
    *ppszName = pName;
    *ppszValue = pValue;
    return 1;
}

/*!
 * @brief Extract the directory part of a path (up to and including the
 *        last '\\' or '/').
 *
 * @param[in]  pszPath Path to inspect. Not NULL.
 * @param[out] pszDir  Output buffer. Not NULL.
 * @param[in]  cbDir   Size of @a pszDir in bytes.
 */
static void hpj_split_dir(const char* pszPath, char* pszDir, size_t cbDir)
{
    const char* pLast = NULL;
    const char* p = pszPath;
    while (*p) {
        if (*p == '\\' || *p == '/') pLast = p;
        p++;
    }
    if (pLast) {
        size_t n = (size_t)(pLast - pszPath) + 1;
        if (n >= cbDir) n = cbDir - 1;
        memcpy(pszDir, pszPath, n);
        pszDir[n] = '\0';
    } else {
        pszDir[0] = '\0';
    }
}

/* ==================================================================
 * Section parsers
 * ================================================================== */

/*!
 * @brief Parse one [OPTIONS] line into a name/value pair.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_options(HpjDocRec* pDoc, const char* pszLine)
{
    char* pszName = NULL;
    char* pszValue = NULL;
    HpjOptionRec r;
    if (!hpj_split_kv(pszLine, &pszName, &pszValue)) return;
    r.pszName = pszName;
    r.pszValue = pszValue;
    VectorAdd(pDoc->vOptions, &r);
}

/*!
 * @brief Parse one [FILES] line.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_files(HpjDocRec* pDoc, const char* pszLine)
{
    HpjFileRec r;
    r.pszFile = strdup(pszLine);
    if (!r.pszFile) return;
    VectorAdd(pDoc->vFiles, &r);
}

/*!
 * @brief Parse one [ALIAS] line into a name/context pair.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_alias(HpjDocRec* pDoc, const char* pszLine)
{
    char* pszName = NULL;
    char* pszValue = NULL;
    HpjAliasRec r;
    if (!hpj_split_kv(pszLine, &pszName, &pszValue)) return;
    r.pszName = pszName;
    r.pszContext = pszValue;
    VectorAdd(pDoc->vAliases, &r);
}

/*!
 * @brief Look up a previously defined #define symbol.
 *
 * @param[in]  pDoc    Document. Not NULL.
 * @param[in]  pszName Symbol name. Not NULL.
 * @param[out] pfFound Receiver for the "found" flag. Not NULL.
 *
 * @retval 0  Symbol not found.
 * @return The symbol's numeric value, or 0 if not found.
 */
static ULONG hpj_find_define(HpjDocRec* pDoc, const char* pszName, BOOL* pfFound)
{
    ULONG n = 0, i;
    *pfFound = FALSE;
    VectorGetCount(pDoc->vDefines, &n);
    for (i = 0; i < n; i++) {
        HpjDefineRec r;
        VectorGetItem(pDoc->vDefines, i, &r, sizeof(r), NULL);
        if (r.pszName && stricmp(r.pszName, pszName) == 0) {
            *pfFound = TRUE;
            return r.ulValue;
        }
    }
    return 0;
}

/*!
 * @brief Parse one numeric [MAP] line into a context/number pair.
 *
 * The right-hand side may be decimal, 0x-prefixed hexadecimal, or a
 * previously defined #define symbol.
 *
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_map(HpjDocRec* pDoc, const char* pszLine)
{
    const char* p = pszLine;
    const char* pEnd;
    ULONG ulNum = 0;
    char* pszCtx;
    HpjMapRec r;

    pEnd = p;
    while (*pEnd && !isspace((unsigned char)*pEnd)) pEnd++;
    if (*pEnd == '\0') return;

    pszCtx = hpj_substr(p, pEnd);
    if (!pszCtx) return;

    p = pEnd;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') { free(pszCtx); return; }

    if (isdigit((unsigned char)*p)) {
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            p += 2;
            while (*p && isxdigit((unsigned char)*p)) {
                ULONG d;
                if (*p >= '0' && *p <= '9') d = (ULONG)(*p - '0');
                else if (*p >= 'a' && *p <= 'f') d = (ULONG)(*p - 'a' + 10);
                else d = (ULONG)(*p - 'A' + 10);
                ulNum = ulNum * 16 + d;
                p++;
            }
        } else {
            while (*p && isdigit((unsigned char)*p)) {
                ulNum = ulNum * 10 + (ULONG)(*p - '0');
                p++;
            }
        }
    } else if (isalpha((unsigned char)*p) || *p == '_') {
        const char* pName = p;
        BOOL fFound = FALSE;
        while (*p && (isalnum((unsigned char)*p) || *p == '_')) p++;
        {
            char* pszName = hpj_substr(pName, p);
            if (pszName) {
                ulNum = hpj_find_define(pDoc, pszName, &fFound);
                free(pszName);
            }
        }
        if (!fFound) ulNum = 0;
    }

    r.pszContext = pszCtx;
    r.ulNumber = ulNum;
    VectorAdd(pDoc->vMaps, &r);
}

/*!
 * @brief Parse one `#define NAME value` line from [MAP].
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text (starting with "#define"). Not NULL.
 */
static void hpj_parse_map_define(HpjDocRec* pDoc, const char* pszLine)
{
    const char* p = pszLine + 7;
    const char* pEnd;
    char* pszName;
    ULONG ulValue = 0;
    HpjDefineRec r;

    while (*p && isspace((unsigned char)*p)) p++;
    pEnd = p;
    while (*pEnd && !isspace((unsigned char)*pEnd)) pEnd++;
    if (*pEnd == '\0') return;

    pszName = hpj_substr(p, pEnd);
    if (!pszName) return;

    p = pEnd;
    while (*p && isspace((unsigned char)*p)) p++;

    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
        while (*p && isxdigit((unsigned char)*p)) {
            ULONG d;
            if (*p >= '0' && *p <= '9') d = (ULONG)(*p - '0');
            else if (*p >= 'a' && *p <= 'f') d = (ULONG)(*p - 'a' + 10);
            else d = (ULONG)(*p - 'A' + 10);
            ulValue = ulValue * 16 + d;
            p++;
        }
    } else {
        while (*p && isdigit((unsigned char)*p)) {
            ulValue = ulValue * 10 + (ULONG)(*p - '0');
            p++;
        }
    }
    r.pszName = pszName;
    r.ulValue = ulValue;
    VectorAdd(pDoc->vDefines, &r);
}

/*!
 * @brief Parse one `#include "file"` line from [MAP].
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text (starting with "#include"). Not NULL.
 */
static void hpj_parse_map_include(HpjDocRec* pDoc, const char* pszLine)
{
    const char* p = pszLine + 8;
    const char* pEnd;
    char* pszFile;
    HpjIncludeRec r;

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '"') {
        p++;
        pEnd = strchr(p, '"');
        if (!pEnd) return;
    } else {
        pEnd = p;
        while (*pEnd && !isspace((unsigned char)*pEnd)) pEnd++;
    }
    pszFile = hpj_substr(p, pEnd);
    if (!pszFile) return;
    r.pszFile = pszFile;
    VectorAdd(pDoc->vIncludes, &r);
}

/*!
 * @brief Parse one [WINDOWS] line into an HPJWINDOW structure.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_window(HpjDocRec* pDoc, const char* pszLine)
{
    HPJWINDOW win;
    const char* p;
    const char* pEq;
    const char* pQuote;
    const char* pQuote2;
    const char* pParen;
    const char* pParen2;

    memset(&win, 0, sizeof(win));
    pEq = strchr(pszLine, '=');
    if (!pEq) return;

    {
        size_t n = (size_t)(pEq - pszLine);
        if (n >= sizeof(win.szName)) n = sizeof(win.szName) - 1;
        memcpy(win.szName, pszLine, n);
        win.szName[n] = '\0';
        {
            char* pT = hpj_trim(win.szName);
            if (pT != win.szName) memmove(win.szName, pT, strlen(pT) + 1);
        }
    }
    p = pEq + 1;

    pQuote = strchr(p, '"');
    if (pQuote) {
        pQuote2 = strchr(pQuote + 1, '"');
        if (pQuote2) {
            size_t n = (size_t)(pQuote2 - pQuote - 1);
            if (n >= sizeof(win.szCaption)) n = sizeof(win.szCaption) - 1;
            memcpy(win.szCaption, pQuote + 1, n);
            win.szCaption[n] = '\0';
            p = pQuote2 + 1;
        }
    }

    pParen = strchr(p, '(');
    if (pParen) {
        pParen2 = strchr(pParen, ')');
        if (pParen2) {
            char szBuf[128];
            size_t n = (size_t)(pParen2 - pParen - 1);
            if (n >= sizeof(szBuf)) n = sizeof(szBuf) - 1;
            memcpy(szBuf, pParen + 1, n);
            szBuf[n] = '\0';
            sscanf(szBuf, "%d,%d,%d,%d", &win.x, &win.y, &win.w, &win.h);
            p = pParen2 + 1;
        }
    }

    if (*p == ',') p++;
    {
        const char* pComma = strchr(p, ',');
        if (pComma) {
            p = pComma + 1;
            if (*p == '(') {
                const char* pRgbEnd = strchr(p, ')');
                if (pRgbEnd) {
                    int r, g, b;
                    if (sscanf(p, "(%d,%d,%d)", &r, &g, &b) == 3) {
                        win.rgb[0] = (BYTE)r;
                        win.rgb[1] = (BYTE)g;
                        win.rgb[2] = (BYTE)b;
                    }
                    p = pRgbEnd + 1;
                    if (*p == ',') p++;
                    if (*p == '(') {
                        const char* pRgbEnd2 = strchr(p, ')');
                        if (pRgbEnd2) {
                            if (sscanf(p, "(%d,%d,%d)", &r, &g, &b) == 3) {
                                win.rgbNsr[0] = (BYTE)r;
                                win.rgbNsr[1] = (BYTE)g;
                                win.rgbNsr[2] = (BYTE)b;
                            }
                            p = pRgbEnd2 + 1;
                            if (*p == ',') p++;
                            win.top = atoi(p);
                            {
                                const char* pC = strchr(p, ',');
                                if (pC) win.scrollbars = atoi(pC + 1);
                            }
                        }
                    }
                }
            }
        }
    }
    VectorAdd(pDoc->vWindows, &win);
}

/*!
 * @brief Parse one [CONFIG] line.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_config(HpjDocRec* pDoc, const char* pszLine)
{
    HpjConfigRec r;
    r.pszText = strdup(pszLine);
    if (!r.pszText) return;
    VectorAdd(pDoc->vConfigs, &r);
}

/*!
 * @brief Parse one [BITMAPS] line.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_bitmap(HpjDocRec* pDoc, const char* pszLine)
{
    HpjBitmapRec r;
    r.pszFile = strdup(pszLine);
    if (!r.pszFile) return;
    VectorAdd(pDoc->vBitmaps, &r);
}

/*!
 * @brief Parse one [BAGGAGE] line.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_baggage(HpjDocRec* pDoc, const char* pszLine)
{
    HpjBaggageRec r;
    r.pszFile = strdup(pszLine);
    if (!r.pszFile) return;
    VectorAdd(pDoc->vBaggage, &r);
}

/*!
 * @brief Parse one [BUILDTAGS] line.
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszLine Line text. Not NULL.
 */
static void hpj_parse_buildtag(HpjDocRec* pDoc, const char* pszLine)
{
    HpjBuildTagRec r;
    r.pszTag = strdup(pszLine);
    if (!r.pszTag) return;
    VectorAdd(pDoc->vBuildTags, &r);
}

/*!
 * @brief Dispatch one line to the parser for its current section.
 *
 * @param[in,out] pDoc        Document. Not NULL.
 * @param[in]     pszSection Current section name. Not NULL.
 * @param[in]     pszLine    Trimmed line text. Not NULL.
 */
static void hpj_dispatch(HpjDocRec* pDoc, const char* pszSection,
                         const char* pszLine)
{
    if (stricmp(pszSection, HPJ_SECTION_OPTIONS) == 0)
        hpj_parse_options(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_FILES) == 0)
        hpj_parse_files(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_ALIAS) == 0)
        hpj_parse_alias(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_MAP) == 0) {
        if (strnicmp(pszLine, "#define", 7) == 0)
            hpj_parse_map_define(pDoc, pszLine);
        else if (strnicmp(pszLine, "#include", 8) == 0)
            hpj_parse_map_include(pDoc, pszLine);
        else
            hpj_parse_map(pDoc, pszLine);
    }
    else if (stricmp(pszSection, HPJ_SECTION_WINDOWS) == 0)
        hpj_parse_window(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_CONFIG) == 0)
        hpj_parse_config(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_BITMAPS) == 0)
        hpj_parse_bitmap(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_BAGGAGE) == 0)
        hpj_parse_baggage(pDoc, pszLine);
    else if (stricmp(pszSection, HPJ_SECTION_BUILDTAGS) == 0)
        hpj_parse_buildtag(pDoc, pszLine);
}

/* ==================================================================
 * Buffer parser
 * ================================================================== */

/*!
 * @brief Forward declaration for the recursive file reader.
 *
 * @param[in,out] pDoc        Document. Not NULL.
 * @param[in]     pszFileName File to read. Not NULL.
 * @param[in]     depth       Current recursion depth.
 *
 * @return APIRET
 */
static APIRET hpj_read_one(HpjDocRec* pDoc, PCSZ pszFileName, int depth);

/*!
 * @brief Parse one HPJ buffer, dispatching every line to its section.
 *
 * @param[in,out] pDoc    Document. Not NULL.
 * @param[in]     pszBuf  Buffer. Not NULL.
 * @param[in]     lSize   Buffer length in bytes.
 * @param[in]     depth   Current recursion depth (for #include).
 *
 * @retval NO_ERROR  Success.
 * @return APIRET.
 */
static APIRET hpj_parse_buffer(HpjDocRec* pDoc, const char* pszBuf,
                               long lSize, int depth)
{
    const char* p = pszBuf;
    const char* pEnd = pszBuf + lSize;
    char szSection[64];
    BOOL fHaveSection = FALSE;

    szSection[0] = '\0';

    while (p < pEnd) {
        const char* pLineStart = p;
        const char* pLineEnd = p;
        char szLine[4096];
        long lLineLen;

        while (pLineEnd < pEnd && *pLineEnd != '\n' && *pLineEnd != '\r')
            pLineEnd++;
        lLineLen = (long)(pLineEnd - pLineStart);
        if (lLineLen >= (long)sizeof(szLine))
            lLineLen = (long)sizeof(szLine) - 1;
        memcpy(szLine, pLineStart, (size_t)lLineLen);
        szLine[lLineLen] = '\0';

        p = pLineEnd;
        while (p < pEnd && (*p == '\n' || *p == '\r')) p++;

        hpj_strip_comment(szLine);
        {
            char* pszTrimmed = hpj_trim(szLine);
            if (*pszTrimmed == '\0') continue;

            if (*pszTrimmed == '[') {
                char* pClose = strchr(pszTrimmed, ']');
                if (pClose) {
                    size_t n = (size_t)(pClose - pszTrimmed - 1);
                    if (n >= sizeof(szSection)) n = sizeof(szSection) - 1;
                    memcpy(szSection, pszTrimmed + 1, n);
                    szSection[n] = '\0';
                    {
                        char* pT = hpj_trim(szSection);
                        if (pT != szSection) memmove(szSection, pT, strlen(pT) + 1);
                    }
                    fHaveSection = TRUE;
                }
                continue;
            }

            /* #include directive: only valid in [MAP]. */
            if (fHaveSection &&
                stricmp(szSection, HPJ_SECTION_MAP) == 0 &&
                strnicmp(pszTrimmed, "#include", 8) == 0) {
                const char* q = pszTrimmed + 8;
                const char* qEnd;
                char szPath[512];
                while (*q && isspace((unsigned char)*q)) q++;
                if (*q == '"') {
                    q++;
                    qEnd = strchr(q, '"');
                    if (!qEnd) continue;
                } else {
                    qEnd = q;
                    while (*qEnd && !isspace((unsigned char)*qEnd)) qEnd++;
                }
                {
                    size_t n = (size_t)(qEnd - q);
                    if (n >= sizeof(szPath) - 1) n = sizeof(szPath) - 2;
                    memcpy(szPath, q, n);
                    szPath[n] = '\0';
                }
                {
                    char szFull[1024];
                    if (szPath[0] != '\\' && szPath[0] != '/' &&
                        pDoc->szBaseDir[0]) {
                        strcpy(szFull, pDoc->szBaseDir);
                        strcat(szFull, szPath);
                    } else {
                        strcpy(szFull, szPath);
                    }
                    if (depth < 16) {
                        char szSaveDir[512];
                        strcpy(szSaveDir, pDoc->szBaseDir);
                        hpj_split_dir(szFull, pDoc->szBaseDir,
                                      sizeof(pDoc->szBaseDir));
                        hpj_read_one(pDoc, szFull, depth + 1);
                        strcpy(pDoc->szBaseDir, szSaveDir);
                    }
                }
                continue;
            }

            if (fHaveSection) hpj_dispatch(pDoc, szSection, pszTrimmed);
        }
    }
    return NO_ERROR;
}

/*!
 * @brief Read one HPJ file, then parse it into the document.
 *
 * The current base directory is saved, set to the file's own
 * directory, and restored afterwards so that `#include` paths in
 * nested files resolve relative to the file that contains them.
 *
 * @param[in,out] pDoc        Document. Not NULL.
 * @param[in]     pszFileName File to read. Not NULL.
 * @param[in]     depth       Current recursion depth.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     File could not be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 * @retval ERROR_READ_FAULT         Read error.
 * @return APIRET.
 */
static APIRET hpj_read_one(HpjDocRec* pDoc, PCSZ pszFileName, int depth)
{
    FILE* f;
    char* pszBuf;
    long lSize;
    char szSaveDir[512];

    f = fopen(pszFileName, "rb");
    if (!f) return ERROR_FILE_NOT_FOUND;
    fseek(f, 0, SEEK_END);
    lSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    pszBuf = (char*)malloc(lSize + 1);
    if (!pszBuf) { fclose(f); return ERROR_NOT_ENOUGH_MEMORY; }
    if (fread(pszBuf, 1, lSize, f) != (size_t)lSize) {
        free(pszBuf); fclose(f);
        return ERROR_READ_FAULT;
    }
    pszBuf[lSize] = '\0';
    fclose(f);

    strcpy(szSaveDir, pDoc->szBaseDir);
    hpj_split_dir(pszFileName, pDoc->szBaseDir, sizeof(pDoc->szBaseDir));

    hpj_parse_buffer(pDoc, pszBuf, lSize, depth);

    strcpy(pDoc->szBaseDir, szSaveDir);
    free(pszBuf);
    return NO_ERROR;
}
/* ==================================================================
 * Public API: document lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty HPJ document.
 *
 * @param[out] phDoc Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HpjCreateDoc(PHHPJ phDoc)
{
    HpjDocRec* pDoc;
    APIRET rc;

    if (!phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;
    pDoc = (HpjDocRec*)malloc(sizeof(HpjDocRec));
    if (!pDoc) return ERROR_NOT_ENOUGH_MEMORY;
    memset(pDoc, 0, sizeof(*pDoc));

    if ((rc = VectorCreate(sizeof(HpjOptionRec),   0, NULL, &pDoc->vOptions))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjFileRec),     0, NULL, &pDoc->vFiles))     != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjAliasRec),    0, NULL, &pDoc->vAliases))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjMapRec),      0, NULL, &pDoc->vMaps))      != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjDefineRec),   0, NULL, &pDoc->vDefines))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjIncludeRec),  0, NULL, &pDoc->vIncludes))  != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HPJWINDOW),      0, NULL, &pDoc->vWindows))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjConfigRec),   0, NULL, &pDoc->vConfigs))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjBitmapRec),   0, NULL, &pDoc->vBitmaps))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjBaggageRec),  0, NULL, &pDoc->vBaggage))   != NO_ERROR) goto fail;
    if ((rc = VectorCreate(sizeof(HpjBuildTagRec), 0, NULL, &pDoc->vBuildTags)) != NO_ERROR) goto fail;

    *phDoc = DOC_HANDLE_FROM(pDoc);
    return NO_ERROR;
fail:
    if (pDoc->vOptions) VectorDestroy(pDoc->vOptions);
    if (pDoc->vFiles) VectorDestroy(pDoc->vFiles);
    if (pDoc->vAliases) VectorDestroy(pDoc->vAliases);
    if (pDoc->vMaps) VectorDestroy(pDoc->vMaps);
    if (pDoc->vDefines) VectorDestroy(pDoc->vDefines);
    if (pDoc->vIncludes) VectorDestroy(pDoc->vIncludes);
    if (pDoc->vWindows) VectorDestroy(pDoc->vWindows);
    if (pDoc->vConfigs) VectorDestroy(pDoc->vConfigs);
    if (pDoc->vBitmaps) VectorDestroy(pDoc->vBitmaps);
    if (pDoc->vBaggage) VectorDestroy(pDoc->vBaggage);
    if (pDoc->vBuildTags) VectorDestroy(pDoc->vBuildTags);
    free(pDoc);
    return rc;
}

/*!
 * @brief Destroy an HPJ document and release all its data.
 *
 * @param[in] hDoc Document handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULLHANDLE.
 */
APIRET APIENTRY HpjDestroyDoc(HHPJ hDoc)
{
    HpjDocRec* pDoc;
    ULONG n, i;

    if (hDoc == NULLHANDLE) return NO_ERROR;
    pDoc = DOC_FROM_HANDLE(hDoc);

    if (pDoc->vOptions) {
        VectorGetCount(pDoc->vOptions, &n);
        for (i = 0; i < n; i++) {
            HpjOptionRec r;
            VectorGetItem(pDoc->vOptions, i, &r, sizeof(r), NULL);
            if (r.pszName) free(r.pszName);
            if (r.pszValue) free(r.pszValue);
        }
        VectorDestroy(pDoc->vOptions);
    }
    if (pDoc->vFiles) {
        VectorGetCount(pDoc->vFiles, &n);
        for (i = 0; i < n; i++) {
            HpjFileRec r;
            VectorGetItem(pDoc->vFiles, i, &r, sizeof(r), NULL);
            if (r.pszFile) free(r.pszFile);
        }
        VectorDestroy(pDoc->vFiles);
    }
    if (pDoc->vAliases) {
        VectorGetCount(pDoc->vAliases, &n);
        for (i = 0; i < n; i++) {
            HpjAliasRec r;
            VectorGetItem(pDoc->vAliases, i, &r, sizeof(r), NULL);
            if (r.pszName) free(r.pszName);
            if (r.pszContext) free(r.pszContext);
        }
        VectorDestroy(pDoc->vAliases);
    }
    if (pDoc->vMaps) {
        VectorGetCount(pDoc->vMaps, &n);
        for (i = 0; i < n; i++) {
            HpjMapRec r;
            VectorGetItem(pDoc->vMaps, i, &r, sizeof(r), NULL);
            if (r.pszContext) free(r.pszContext);
        }
        VectorDestroy(pDoc->vMaps);
    }
    if (pDoc->vDefines) {
        VectorGetCount(pDoc->vDefines, &n);
        for (i = 0; i < n; i++) {
            HpjDefineRec r;
            VectorGetItem(pDoc->vDefines, i, &r, sizeof(r), NULL);
            if (r.pszName) free(r.pszName);
        }
        VectorDestroy(pDoc->vDefines);
    }
    if (pDoc->vIncludes) {
        VectorGetCount(pDoc->vIncludes, &n);
        for (i = 0; i < n; i++) {
            HpjIncludeRec r;
            VectorGetItem(pDoc->vIncludes, i, &r, sizeof(r), NULL);
            if (r.pszFile) free(r.pszFile);
        }
        VectorDestroy(pDoc->vIncludes);
    }
    if (pDoc->vWindows) VectorDestroy(pDoc->vWindows);
    if (pDoc->vConfigs) {
        VectorGetCount(pDoc->vConfigs, &n);
        for (i = 0; i < n; i++) {
            HpjConfigRec r;
            VectorGetItem(pDoc->vConfigs, i, &r, sizeof(r), NULL);
            if (r.pszText) free(r.pszText);
        }
        VectorDestroy(pDoc->vConfigs);
    }
    if (pDoc->vBitmaps) {
        VectorGetCount(pDoc->vBitmaps, &n);
        for (i = 0; i < n; i++) {
            HpjBitmapRec r;
            VectorGetItem(pDoc->vBitmaps, i, &r, sizeof(r), NULL);
            if (r.pszFile) free(r.pszFile);
        }
        VectorDestroy(pDoc->vBitmaps);
    }
    if (pDoc->vBaggage) {
        VectorGetCount(pDoc->vBaggage, &n);
        for (i = 0; i < n; i++) {
            HpjBaggageRec r;
            VectorGetItem(pDoc->vBaggage, i, &r, sizeof(r), NULL);
            if (r.pszFile) free(r.pszFile);
        }
        VectorDestroy(pDoc->vBaggage);
    }
    if (pDoc->vBuildTags) {
        VectorGetCount(pDoc->vBuildTags, &n);
        for (i = 0; i < n; i++) {
            HpjBuildTagRec r;
            VectorGetItem(pDoc->vBuildTags, i, &r, sizeof(r), NULL);
            if (r.pszTag) free(r.pszTag);
        }
        VectorDestroy(pDoc->vBuildTags);
    }
    free(pDoc);
    return NO_ERROR;
}

/* ==================================================================
 * Public API: parsing
 * ================================================================== */

/*!
 * @brief Read one HPJ file and populate the document.
 *
 * @param[in] hDoc        Document handle. Not NULLHANDLE.
 * @param[in] pszFileName NUL-terminated file name. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszFileName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     File could not be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HpjReadFile(HHPJ hDoc, PCSZ pszFileName)
{
    HpjDocRec* pDoc;
    if (hDoc == NULLHANDLE || !pszFileName) return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    return hpj_read_one(pDoc, pszFileName, 0);
}

/*!
 * @brief Read a list of HPJ files and populate the document.
 *
 * @param[in] hDoc      Document handle. Not NULLHANDLE.
 * @param[in] cFiles    Number of files. Must be > 0.
 * @param[in] apszFiles Array of NUL-terminated file names. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or @a cFiles <= 0.
 * @retval ERROR_FILE_NOT_FOUND     A file could not be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HpjReadFiles(HHPJ hDoc, int cFiles, PCSZ* apszFiles)
{
    HpjDocRec* pDoc;
    int i;
    APIRET rc;
    if (hDoc == NULLHANDLE || cFiles <= 0 || !apszFiles)
        return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    for (i = 0; i < cFiles; i++) {
        rc = hpj_read_one(pDoc, apszFiles[i], 0);
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Generic size-query string return
 * ================================================================== */

/*!
 * @brief Return a string using the size-query convention.
 *
 * @param[in]  psz     Source string, or NULL for an empty string.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR               Success.
 * @retval ERROR_BUFFER_OVERFLOW  Buffer too small.
 */
static APIRET hpj_return_string(const char* psz, PSZ pszBuf,
                                ULONG ulSize, PULONG pulUsed)
{
    ULONG len = psz ? (ULONG)strlen(psz) : 0;
    if (!pszBuf && ulSize == 0) {
        if (pulUsed) *pulUsed = len;
        return NO_ERROR;
    }
    if (ulSize < len + 1) {
        if (pulUsed) *pulUsed = len + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    if (psz) memcpy(pszBuf, psz, len + 1);
    else pszBuf[0] = '\0';
    if (pulUsed) *pulUsed = len;
    return NO_ERROR;
}

/* ==================================================================
 * Public API: options
 * ================================================================== */

/*!
 * @brief Query the number of [OPTIONS] entries.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryOptionCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vOptions, pulCount);
}

/*!
 * @brief Query the option name at the given index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryOptionName(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjOptionRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vOptions, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query an option value by name.
 *
 * The name is compared case-insensitively.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  pszName Option name. Not NULL.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Option not present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryOptionValue(HHPJ hDoc, PCSZ pszName,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjDocRec* pDoc;
    ULONG n = 0, i;
    if (hDoc == NULLHANDLE || !pszName) return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    VectorGetCount(pDoc->vOptions, &n);
    for (i = 0; i < n; i++) {
        HpjOptionRec r;
        VectorGetItem(pDoc->vOptions, i, &r, sizeof(r), NULL);
        if (r.pszName && stricmp(r.pszName, pszName) == 0)
            return hpj_return_string(r.pszValue, pszBuf, ulSize, pulUsed);
    }
    return ERROR_FILE_NOT_FOUND;
}

/* ==================================================================
 * Public API: files
 * ================================================================== */

/*!
 * @brief Query the number of files listed in [FILES].
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryFileCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vFiles, pulCount);
}

/*!
 * @brief Query a file name from [FILES] by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryFileName(HHPJ hDoc, ULONG ulIndex,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjFileRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vFiles, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszFile, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Public API: aliases
 * ================================================================== */

/*!
 * @brief Query the number of alias entries.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryAliasCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vAliases, pulCount);
}

/*!
 * @brief Query an alias name by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryAliasName(HHPJ hDoc, ULONG ulIndex,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjAliasRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vAliases, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the context string that an alias points to.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryAliasContext(HHPJ hDoc, ULONG ulIndex,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjAliasRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vAliases, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszContext, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Resolve an alias name to its context string.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  pszName Alias name. Not NULL.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc or @a pszName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Alias not present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryAliasResolve(HHPJ hDoc, PCSZ pszName,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjDocRec* pDoc;
    ULONG n = 0, i;
    if (hDoc == NULLHANDLE || !pszName) return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    VectorGetCount(pDoc->vAliases, &n);
    for (i = 0; i < n; i++) {
        HpjAliasRec r;
        VectorGetItem(pDoc->vAliases, i, &r, sizeof(r), NULL);
        if (r.pszName && stricmp(r.pszName, pszName) == 0)
            return hpj_return_string(r.pszContext, pszBuf, ulSize, pulUsed);
    }
    return ERROR_FILE_NOT_FOUND;
}

/* ==================================================================
 * Public API: maps
 * ================================================================== */

/*!
 * @brief Query the number of numeric context mappings.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryMapCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vMaps, pulCount);
}

/*!
 * @brief Query a context string from [MAP] by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryMapContext(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjMapRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vMaps, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszContext, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the numeric identifier from [MAP] by index.
 *
 * @param[in]  hDoc      Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex   Zero-based index.
 * @param[out] pulNumber Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY HpjQueryMapNumber(HHPJ hDoc, ULONG ulIndex, PULONG pulNumber)
{
    HpjMapRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE || !pulNumber) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vMaps, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    *pulNumber = r.ulNumber;
    return NO_ERROR;
}

/*!
 * @brief Resolve a context string to its numeric identifier.
 *
 * @param[in]  hDoc       Document handle. Not NULLHANDLE.
 * @param[in]  pszContext Context string. Not NULL.
 * @param[out] pulNumber  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Context not present.
 */
APIRET APIENTRY HpjQueryMapResolve(HHPJ hDoc, PCSZ pszContext, PULONG pulNumber)
{
    HpjDocRec* pDoc;
    ULONG n = 0, i;
    if (hDoc == NULLHANDLE || !pszContext || !pulNumber)
        return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    VectorGetCount(pDoc->vMaps, &n);
    for (i = 0; i < n; i++) {
        HpjMapRec r;
        VectorGetItem(pDoc->vMaps, i, &r, sizeof(r), NULL);
        if (r.pszContext && stricmp(r.pszContext, pszContext) == 0) {
            *pulNumber = r.ulNumber;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/* ==================================================================
 * Public API: defines
 * ================================================================== */

/*!
 * @brief Query the number of #define symbols found in [MAP].
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryDefineCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vDefines, pulCount);
}

/*!
 * @brief Query a #define symbol name by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryDefineName(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjDefineRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vDefines, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query a #define symbol value by index.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pulValue Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY HpjQueryDefineValue(HHPJ hDoc, ULONG ulIndex, PULONG pulValue)
{
    HpjDefineRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE || !pulValue) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vDefines, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    *pulValue = r.ulValue;
    return NO_ERROR;
}

/* ==================================================================
 * Public API: includes
 * ================================================================== */

/*!
 * @brief Query the number of #include files found in [MAP].
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryIncludeCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vIncludes, pulCount);
}

/*!
 * @brief Query an #include file name by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryIncludeName(HHPJ hDoc, ULONG ulIndex,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjIncludeRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vIncludes, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszFile, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Public API: windows
 * ================================================================== */

/*!
 * @brief Query the number of window definitions.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryWindowCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vWindows, pulCount);
}

/*!
 * @brief Query a window definition by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pWin    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 */
APIRET APIENTRY HpjQueryWindow(HHPJ hDoc, ULONG ulIndex, HPJWINDOW* pWin)
{
    if (hDoc == NULLHANDLE || !pWin) return ERROR_INVALID_PARAMETER;
    return VectorGetItem(DOC_FROM_HANDLE(hDoc)->vWindows, ulIndex,
                         pWin, sizeof(HPJWINDOW), NULL);
}

/*!
 * @brief Query a window definition by name.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  pszName Window name. Not NULL.
 * @param[out] pWin    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Window not present.
 */
APIRET APIENTRY HpjQueryWindowByName(HHPJ hDoc, PCSZ pszName, HPJWINDOW* pWin)
{
    HpjDocRec* pDoc;
    ULONG n = 0, i;
    if (hDoc == NULLHANDLE || !pszName || !pWin) return ERROR_INVALID_PARAMETER;
    pDoc = DOC_FROM_HANDLE(hDoc);
    VectorGetCount(pDoc->vWindows, &n);
    for (i = 0; i < n; i++) {
        HPJWINDOW w;
        VectorGetItem(pDoc->vWindows, i, &w, sizeof(w), NULL);
        if (stricmp(w.szName, pszName) == 0) { *pWin = w; return NO_ERROR; }
    }
    return ERROR_FILE_NOT_FOUND;
}

/* ==================================================================
 * Public API: config
 * ================================================================== */

/*!
 * @brief Query the number of [CONFIG] macro lines.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryConfigCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vConfigs, pulCount);
}

/*!
 * @brief Query a [CONFIG] macro line by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryConfig(HHPJ hDoc, ULONG ulIndex,
                               PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjConfigRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vConfigs, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszText, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Public API: bitmaps
 * ================================================================== */

/*!
 * @brief Query the number of bitmap file names.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryBitmapCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vBitmaps, pulCount);
}

/*!
 * @brief Query a bitmap file name by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryBitmapName(HHPJ hDoc, ULONG ulIndex,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjBitmapRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vBitmaps, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszFile, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Public API: baggage
 * ================================================================== */

/*!
 * @brief Query the number of baggage file names.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryBaggageCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vBaggage, pulCount);
}

/*!
 * @brief Query a baggage file name by index.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryBaggageName(HHPJ hDoc, ULONG ulIndex,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjBaggageRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vBaggage, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszFile, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Public API: build tags
 * ================================================================== */

/*!
 * @brief Query the number of build tags.
 *
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY HpjQueryBuildTagCount(HHPJ hDoc, PULONG pulCount)
{
    if (hDoc == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(DOC_FROM_HANDLE(hDoc)->vBuildTags, pulCount);
}

/*!
 * @brief Query a build tag by index.
 *
 * The returned string is the tag name with any leading '+' or '-'
 * prefix preserved.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hDoc is NULL.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HpjQueryBuildTagName(HHPJ hDoc, ULONG ulIndex,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    HpjBuildTagRec r;
    APIRET rc;
    if (hDoc == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(DOC_FROM_HANDLE(hDoc)->vBuildTags, ulIndex,
                       &r, sizeof(r), NULL);
    if (rc != NO_ERROR) return rc;
    return hpj_return_string(r.pszTag, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Testing
 * ================================================================== */

/*!
 * @brief Compare two HPJ documents for equality.
 *
 * Intended for unit tests. Returns TRUE if both documents have the
 * same sections and entries in the same order.
 *
 * @param[in] hDocA First document. Not NULLHANDLE.
 * @param[in] hDocB Second document. Not NULLHANDLE.
 *
 * @retval TRUE   Documents are equal.
 * @retval FALSE  Documents differ, or a handle is NULL.
 * @return TRUE if equal, FALSE otherwise.
 */
BOOL APIENTRY HpjEquals(HHPJ hDocA, HHPJ hDocB)
{
    HpjDocRec* pA = DOC_FROM_HANDLE(hDocA);
    HpjDocRec* pB = DOC_FROM_HANDLE(hDocB);
    ULONG ca = 0, cb = 0, i;

    if (!pA || !pB) return FALSE;

    VectorGetCount(pA->vOptions, &ca); VectorGetCount(pB->vOptions, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjOptionRec ra, rb;
        VectorGetItem(pA->vOptions, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vOptions, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszName ? ra.pszName : "",
                   rb.pszName ? rb.pszName : "") != 0) return FALSE;
        if (strcmp(ra.pszValue ? ra.pszValue : "",
                   rb.pszValue ? rb.pszValue : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vFiles, &ca); VectorGetCount(pB->vFiles, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjFileRec ra, rb;
        VectorGetItem(pA->vFiles, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vFiles, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszFile ? ra.pszFile : "",
                   rb.pszFile ? rb.pszFile : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vAliases, &ca); VectorGetCount(pB->vAliases, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjAliasRec ra, rb;
        VectorGetItem(pA->vAliases, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vAliases, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszName ? ra.pszName : "",
                   rb.pszName ? rb.pszName : "") != 0) return FALSE;
        if (strcmp(ra.pszContext ? ra.pszContext : "",
                   rb.pszContext ? rb.pszContext : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vMaps, &ca); VectorGetCount(pB->vMaps, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjMapRec ra, rb;
        VectorGetItem(pA->vMaps, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vMaps, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszContext ? ra.pszContext : "",
                   rb.pszContext ? rb.pszContext : "") != 0) return FALSE;
        if (ra.ulNumber != rb.ulNumber) return FALSE;
    }

    VectorGetCount(pA->vDefines, &ca); VectorGetCount(pB->vDefines, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjDefineRec ra, rb;
        VectorGetItem(pA->vDefines, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vDefines, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszName ? ra.pszName : "",
                   rb.pszName ? rb.pszName : "") != 0) return FALSE;
        if (ra.ulValue != rb.ulValue) return FALSE;
    }

    VectorGetCount(pA->vIncludes, &ca); VectorGetCount(pB->vIncludes, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjIncludeRec ra, rb;
        VectorGetItem(pA->vIncludes, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vIncludes, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszFile ? ra.pszFile : "",
                   rb.pszFile ? rb.pszFile : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vWindows, &ca); VectorGetCount(pB->vWindows, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HPJWINDOW wa, wb;
        VectorGetItem(pA->vWindows, i, &wa, sizeof(wa), NULL);
        VectorGetItem(pB->vWindows, i, &wb, sizeof(wb), NULL);
        if (memcmp(&wa, &wb, sizeof(HPJWINDOW)) != 0) return FALSE;
    }

    VectorGetCount(pA->vConfigs, &ca); VectorGetCount(pB->vConfigs, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjConfigRec ra, rb;
        VectorGetItem(pA->vConfigs, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vConfigs, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszText ? ra.pszText : "",
                   rb.pszText ? rb.pszText : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vBitmaps, &ca); VectorGetCount(pB->vBitmaps, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjBitmapRec ra, rb;
        VectorGetItem(pA->vBitmaps, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vBitmaps, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszFile ? ra.pszFile : "",
                   rb.pszFile ? rb.pszFile : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vBaggage, &ca); VectorGetCount(pB->vBaggage, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjBaggageRec ra, rb;
        VectorGetItem(pA->vBaggage, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vBaggage, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszFile ? ra.pszFile : "",
                   rb.pszFile ? rb.pszFile : "") != 0) return FALSE;
    }

    VectorGetCount(pA->vBuildTags, &ca); VectorGetCount(pB->vBuildTags, &cb);
    if (ca != cb) return FALSE;
    for (i = 0; i < ca; i++) {
        HpjBuildTagRec ra, rb;
        VectorGetItem(pA->vBuildTags, i, &ra, sizeof(ra), NULL);
        VectorGetItem(pB->vBuildTags, i, &rb, sizeof(rb), NULL);
        if (strcmp(ra.pszTag ? ra.pszTag : "",
                   rb.pszTag ? rb.pszTag : "") != 0) return FALSE;
    }

    return TRUE;
}
