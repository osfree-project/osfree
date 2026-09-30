/*!
 * @file  mapsym.c
 * @brief MAPSYM replacement built on the Map, Wmp and Sym libraries.
 *
 * Reads a linker map file (Microsoft LINK or Open Watcom WLINK),
 * then writes a .SYM symbol file.
 *
 * On host builds the OS/2 error codes are provided by os2err.h
 * directly; INCL_DOSERRORS must NOT be defined here.
 */
#include "map.h"
#include "wmp.h"
#include "sym.h"
#include "ccl.h"
#include "os2types.h"
#include "os2err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*!
 * @def MAPSYM_VERSION
 * @brief Version string printed by Usage() and the banner.
 */
#define MAPSYM_VERSION "1.0"

/*!
 * @brief Strip a trailing " (comment)" from a symbol name in place.
 *
 * @param[in,out] psz  Symbol name to clean. Not NULL.
 */
static void StripParenComment(PSZ psz)
{
    PSZ p = strrchr(psz, '(');
    if (p != NULL) {
        PSZ q = p;
        while (q > psz && isspace((unsigned char)q[-1])) q--;
        if (q > psz && *q == ' ') { *q = '\0'; return; }
    }
}

/*!
 * @brief Remove a trailing balanced-parenthesis argument list.
 *
 * @param[in,out] psz  Symbol name to trim. Not NULL.
 */
static void StripArgList(PSZ psz)
{
    size_t len = strlen(psz);
    if (len > 0 && psz[len - 1] == ')') {
        int depth = 0;
        size_t i;
        for (i = len; i > 0; i--) {
            if (psz[i - 1] == ')') depth++;
            else if (psz[i - 1] == '(') {
                depth--;
                if (depth == 0) { psz[i - 1] = '\0'; return; }
            }
        }
    }
}

/*!
 * @brief Drop common C keywords from a demangled identifier.
 *
 * The word-boundary check treats '_' as an identifier character, so
 * identifiers like "_long" keep the keyword "long".
 *
 * @param[in,out] psz  Identifier to clean. Not NULL.
 */
static void DropKeywords(PSZ psz)
{
    static const char *kw[] = {
        "near", "int", "short", "unsigned", "long",
        "void", "char", "const", "wchar_t", NULL
    };
    PSZ r = psz, w = psz;
    while (*r) {
        int matched = 0;
        int start = (r == psz) ||
                    (!isalnum((unsigned char)r[-1]) && r[-1] != '_');
        if (start) {
            const char **pp;
            for (pp = kw; *pp; pp++) {
                size_t cb = strlen(*pp);
                if (strncmp(r, *pp, cb) == 0 &&
                    !isalnum((unsigned char)r[cb]) && r[cb] != '_') {
                    r += cb;
                    while (isspace((unsigned char)*r)) r++;
                    if (*r == '*' || *r == '&') r++;
                    while (isspace((unsigned char)*r)) r++;
                    matched = 1;
                    break;
                }
            }
        }
        if (!matched) *w++ = *r++;
    }
    *w = '\0';
}

/*!
 * @brief Replace "std::" with "std".
 *
 * @param[in,out] psz  Identifier to rewrite. Not NULL.
 */
static void ReplaceStd(PSZ psz)
{
    PSZ r = psz, w = psz;
    while (*r) {
        if (strncmp(r, "std::", 5) == 0) {
            memcpy(w, "std", 3); w += 3; r += 5;
            while (isspace((unsigned char)*r)) r++;
        } else *w++ = *r++;
    }
    *w = '\0';
}

/*!
 * @brief Replace "::~ " with "__x".
 *
 * @param[in,out] psz  Identifier to rewrite. Not NULL.
 */
static void ReplaceDtor(PSZ psz)
{
    PSZ r = psz, w = psz;
    while (*r) {
        if (r[0] == ':' && r[1] == ':' && r[2] == '~' && r[3] == ' ') {
            memcpy(w, "__x", 3); w += 3; r += 4;
        } else *w++ = *r++;
    }
    *w = '\0';
}

/*!
 * @brief Replace "::" with "__".
 *
 * @param[in,out] psz  Identifier to rewrite. Not NULL.
 */
static void ReplaceDColon(PSZ psz)
{
    PSZ r = psz, w = psz;
    while (*r) {
        if (r[0] == ':' && r[1] == ':') {
            memcpy(w, "__", 2); w += 2; r += 2;
        } else *w++ = *r++;
    }
    *w = '\0';
}

/*!
 * @brief Replace punctuation with spaces.
 *
 * @param[in,out] psz  Identifier to rewrite. Not NULL.
 */
static void ReplacePunct(PSZ psz)
{
    PSZ p;
    for (p = psz; *p; p++)
        if (strchr("[]()<>,", *p)) *p = ' ';
}

/*!
 * @brief Collapse whitespace runs into single '_' characters.
 *
 * @param[in,out] psz  Identifier to rewrite. Not NULL.
 */
static void CollapseWhitespace(PSZ psz)
{
    PSZ r = psz, w = psz;
    while (*r) {
        if (isspace((unsigned char)*r)) {
            *w++ = '_';
            while (isspace((unsigned char)*r)) r++;
        } else *w++ = *r++;
    }
    *w = '\0';
}

/*!
 * @brief Collapse runs of '_' into a single '_'.
 *
 * @param[in,out] psz  Identifier to rewrite. Not NULL.
 */
static void CollapseUnderscores(PSZ psz)
{
    PSZ r = psz, w = psz;
    while (*r) {
        if (*r == '_') { *w++ = '_'; while (*r == '_') r++; }
        else *w++ = *r++;
    }
    *w = '\0';
}

/*!
 * @brief Remove a leading '_' if present.
 *
 * @param[in,out] psz  Identifier to trim. Not NULL.
 */
static void TrimLeadingUnderscore(PSZ psz)
{
    if (psz[0] == '_') memmove(psz, psz + 1, strlen(psz));
}

/*!
 * @brief Remove a trailing '_' if present.
 *
 * @param[in,out] psz  Identifier to trim. Not NULL.
 */
static void TrimTrailingUnderscore(PSZ psz)
{
    size_t len = strlen(psz);
    if (len > 0 && psz[len - 1] == '_') psz[len - 1] = '\0';
}

/*!
 * @brief Apply all name-mangling steps in order.
 *
 * @param[in,out] psz  Identifier to mangle. Not NULL.
 */
static void MangleSymbol(PSZ psz)
{
    StripParenComment(psz);
    StripArgList(psz);
    DropKeywords(psz);
    ReplaceStd(psz);
    ReplaceDtor(psz);
    ReplaceDColon(psz);
    ReplacePunct(psz);
    CollapseWhitespace(psz);
    CollapseUnderscores(psz);
    TrimLeadingUnderscore(psz);
    TrimTrailingUnderscore(psz);
    if (strlen(psz) > SYM_MAX_SYM_NAME) psz[SYM_MAX_SYM_NAME] = '\0';
}

/*!
 * @brief Type of the source-specific first-symbol iterator.
 *
 * @param[in]  hSrc        Source map handle.
 * @param[out] phFind      Receives the cursor.
 * @param[out] pulSegIdx   Receives the first segment index.
 * @param[out] pulSymIdx   Receives the first symbol index.
 * @param[out] pszName     Output buffer for the name.
 * @param[in]  ulNameSize  Size of pszName.
 * @param[out] pulNameUsed Optional. Receives used length.
 * @param[out] pulValue    Receives the first value.
 * @return APIRET.
 */
typedef APIRET (*PFN_FINDFIRST)(HANDLE hSrc, HANDLE *phFind,
                                PULONG pulSegIdx, PULONG pulSymIdx,
                                PSZ pszName, ULONG ulNameSize,
                                PULONG pulNameUsed, PULONG pulValue);

/*!
 * @brief Type of the source-specific next-symbol iterator.
 *
 * @param[in]  hFind       Cursor.
 * @param[out] pulSegIdx   Receives the segment index.
 * @param[out] pulSymIdx   Receives the symbol index.
 * @param[out] pszName     Output buffer for the name.
 * @param[in]  ulNameSize  Size of pszName.
 * @param[out] pulNameUsed Optional. Receives used length.
 * @param[out] pulValue    Receives the value.
 * @return APIRET.
 */
typedef APIRET (*PFN_FINDNEXT)(HANDLE hFind,
                               PULONG pulSegIdx, PULONG pulSymIdx,
                               PSZ pszName, ULONG ulNameSize,
                               PULONG pulNameUsed, PULONG pulValue);

/*!
 * @brief Type of the source-specific cursor close routine.
 *
 * @param[in] hFind  Cursor.
 * @return APIRET.
 */
typedef APIRET (*PFN_FINDCLOSE)(HANDLE hFind);

/*!
 * @brief Copy all source symbols into a SYM file with name mangling.
 *
 * @note Both the source (Map/Wmp) and the target (Sym) enumerate
 *       segments in the same order, and SymAddSegment assigns
 *       sequential indices starting at 0, so the source segIdx can
 *       be reused as the target segIdx.
 *
 * @param[in]  hSym      Target .SYM handle.
 * @param[in]  hSrc      Source map handle (Map or Wmp).
 * @param[in]  pfnFirst  First-symbol iterator for the source type.
 * @param[in]  pfnNext   Next-symbol iterator for the source type.
 * @param[in]  pfnClose  Cursor close for the source type.
 * @param[out] pulAdded  Receives the number of symbols added.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval other                    First failing operation's code.
 */
static APIRET EmitSymFromSource(HSYMFILE hSym,
                                HANDLE hSrc,
                                PFN_FINDFIRST pfnFirst,
                                PFN_FINDNEXT pfnNext,
                                PFN_FINDCLOSE pfnClose,
                                ULONG *pulAdded)
{
    HANDLE hFind = NULLHANDLE;
    ULONG ulSegIdx = 0, ulSymIdx = 0, ulValue = 0;
    char szName[SYM_MAX_SYM_NAME + 1];
    APIRET rc;

    rc = pfnFirst(hSrc, &hFind, &ulSegIdx, &ulSymIdx,
                  szName, sizeof(szName), NULL, &ulValue);
    while (rc == NO_ERROR) {
        PSZ pszMangled = (PSZ)malloc(strlen(szName) + 1);
        if (pszMangled == NULL) {
            pfnClose(hFind);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        strcpy(pszMangled, szName);
        MangleSymbol(pszMangled);
        if (pszMangled[0] != '\0') {
            rc = SymAddSymbol(hSym, ulSegIdx, ulValue, pszMangled);
            if (rc != NO_ERROR) {
                fprintf(stderr,
                        "mapsym: cannot add symbol %s (error %lu)\n",
                        pszMangled, (unsigned long)rc);
                free(pszMangled);
                pfnClose(hFind);
                return rc;
            }
            (*pulAdded)++;
        }
        free(pszMangled);
        rc = pfnNext(hFind, &ulSegIdx, &ulSymIdx,
                     szName, sizeof(szName), NULL, &ulValue);
    }
    pfnClose(hFind);
    return (rc == ERROR_NO_MORE_ITEMS) ? NO_ERROR : rc;
}

/*!
 * @brief Print the command-line usage banner.
 */
static void Usage(void)
{
    printf("MAPSYM - create .SYM symbol file from linker .MAP file\n"
           "Version " MAPSYM_VERSION "\n"
           "\n"
           "Usage: mapsym [-1] mapfile\n"
           "\n"
           "  -1        Display group information (accepted, ignored)\n"
           "  mapfile   Map file to process; .MAP extension assumed\n"
           "\n"
           "The output .SYM file has the same base name as the input\n"
           "map file.\n");
}

/*!
 * @brief Read the first line of a file.
 *
 * @param[in]  pszPath  File path. Not NULL.
 * @param[out] pszBuf   Destination buffer. Not NULL.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @return Integer flag.
 * @retval 0  File could not be opened or read.
 * @retval 1  First line was read successfully.
 */
static int ReadFirstLine(PCSZ pszPath, PSZ pszBuf, ULONG ulSize)
{
    FILE *pFile = fopen(pszPath, "r");
    int fOk = 0;
    if (pFile == NULL) return 0;
    if (fgets(pszBuf, (int)ulSize, pFile) != NULL) fOk = 1;
    fclose(pFile);
    return fOk;
}

/*!
 * @brief Program entry point.
 *
 * @param[in] argc  Number of arguments.
 * @param[in] argv  Argument vector. argv[0] is the program name.
 * @return Process exit code.
 * @retval 0  Success.
 * @retval 1  Failure.
 */
int main(int argc, char *argv[])
{
    PCSZ pszMapPath = NULL;
    PCSZ pszSymPath = NULL;
    char *pszMapBuf = NULL;
    char *pszSymBuf = NULL;
    HSYMFILE hSym = NULLHANDLE;
    APIRET rc;
    int i;
    size_t len;
    char szLine[512];
    int fWatcom;
    ULONG ulSegCount = 0, ulAdded = 0;
    char szModule[SYM_MAX_MOD_NAME + 1];

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-1") == 0 || strcmp(argv[i], "/1") == 0)
            continue;
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            Usage();
            return 1;
        }
        if (pszMapPath == NULL) pszMapPath = argv[i];
        else { Usage(); return 1; }
    }
    if (pszMapPath == NULL) { Usage(); return 1; }

    len = strlen(pszMapPath);
    if (strrchr(pszMapPath, '.') == NULL) {
        pszMapBuf = (char *)malloc(len + 5);
        if (pszMapBuf == NULL) {
            fprintf(stderr, "mapsym: out of memory\n");
            return 1;
        }
        strcpy(pszMapBuf, pszMapPath);
        strcat(pszMapBuf, ".MAP");
        pszMapPath = pszMapBuf;
    }

    pszSymBuf = (char *)malloc(strlen(pszMapPath) + 1);
    if (pszSymBuf == NULL) {
        fprintf(stderr, "mapsym: out of memory\n");
        free(pszMapBuf);
        return 1;
    }
    strcpy(pszSymBuf, pszMapPath);
    {
        char *dot = strrchr(pszSymBuf, '.');
        if (dot != NULL) *dot = '\0';
    }
    {
        size_t symLen = strlen(pszSymBuf);
        char *tmp = (char *)realloc(pszSymBuf, symLen + 5);
        if (tmp == NULL) {
            fprintf(stderr, "mapsym: out of memory\n");
            free(pszMapBuf); free(pszSymBuf);
            return 1;
        }
        pszSymBuf = tmp;
        strcat(pszSymBuf, ".SYM");
    }
    pszSymPath = pszSymBuf;

    if (!ReadFirstLine(pszMapPath, szLine, sizeof(szLine))) {
        fprintf(stderr, "mapsym: cannot read %s\n", pszMapPath);
        free(pszMapBuf); free(pszSymBuf);
        return 1;
    }
    fWatcom = (strstr(szLine, "Open Watcom Linker Version") != NULL);

    rc = SymOpen(pszSymPath, &hSym,
                 SYM_OPEN_WRITE | SYM_OPEN_CREATE | SYM_OPEN_TRUNCATE);
    if (rc != NO_ERROR) {
        fprintf(stderr, "mapsym: cannot create %s (error %lu)\n",
                pszSymPath, (unsigned long)rc);
        free(pszMapBuf); free(pszSymBuf);
        return 1;
    }

    if (fWatcom) {
        HWMP hWmp = NULLHANDLE;
        ULONG j;

        rc = WmpOpen(pszMapPath, &hWmp);
        if (rc != NO_ERROR) {
            fprintf(stderr, "mapsym: cannot parse %s (error %lu)\n",
                    pszMapPath, (unsigned long)rc);
            SymClose(hSym);
            free(pszMapBuf); free(pszSymBuf);
            return 1;
        }

        szModule[0] = '\0';
        WmpQueryModule(hWmp, szModule, sizeof(szModule), NULL);
        if (szModule[0] == '\0') {
            PCSZ p = strrchr(pszMapPath, '\\');
            PCSZ q;
            size_t modLen;
            if (p == NULL) p = strrchr(pszMapPath, '/');
            q = p ? p + 1 : pszMapPath;
            modLen = strlen(q);
            {
                PCSZ dot = strrchr(q, '.');
                if (dot) modLen = (size_t)(dot - q);
            }
            if (modLen > SYM_MAX_MOD_NAME) modLen = SYM_MAX_MOD_NAME;
            memcpy(szModule, q, modLen);
            szModule[modLen] = '\0';
        }
        SymSetModule(hSym, szModule);

        WmpQuerySegmentCount(hWmp, &ulSegCount);
        for (j = 0; j < ulSegCount; j++) {
            char szSeg[SYM_MAX_SEG_NAME + 1];
            ULONG ulFlags = 0, ulSymFlags = 0, ulSegIdx = 0;
            WmpQuerySegmentName(hWmp, j, szSeg, sizeof(szSeg), NULL);
            WmpQuerySegmentFlags(hWmp, j, &ulFlags);
            if (ulFlags & WMP_SEG_32BIT) ulSymFlags |= SYM_SEGDEF_32BIT;
            if (ulFlags & WMP_SEG_ALPHA) ulSymFlags |= SYM_SEGDEF_ALPHA;
            rc = SymAddSegment(hSym, szSeg, ulSymFlags, &ulSegIdx);
            if (rc != NO_ERROR) {
                fprintf(stderr,
                        "mapsym: cannot add segment %s (error %lu)\n",
                        szSeg, (unsigned long)rc);
                WmpClose(hWmp); SymClose(hSym);
                free(pszMapBuf); free(pszSymBuf);
                return 1;
            }
        }

        EmitSymFromSource(hSym, (HANDLE)hWmp,
                          (PFN_FINDFIRST)WmpFindFirstSymbol,
                          (PFN_FINDNEXT)WmpFindNextSymbol,
                          (PFN_FINDCLOSE)WmpFindClose,
                          &ulAdded);
        WmpClose(hWmp);
    } else {
        HMAP hMap = NULLHANDLE;
        ULONG j;

        rc = MapOpen(pszMapPath, &hMap);
        if (rc != NO_ERROR) {
            fprintf(stderr, "mapsym: cannot parse %s (error %lu)\n",
                    pszMapPath, (unsigned long)rc);
            SymClose(hSym);
            free(pszMapBuf); free(pszSymBuf);
            return 1;
        }

        szModule[0] = '\0';
        MapQueryModule(hMap, szModule, sizeof(szModule), NULL);
        if (szModule[0] == '\0') {
            PCSZ p = strrchr(pszMapPath, '\\');
            PCSZ q;
            size_t modLen;
            if (p == NULL) p = strrchr(pszMapPath, '/');
            q = p ? p + 1 : pszMapPath;
            modLen = strlen(q);
            {
                PCSZ dot = strrchr(q, '.');
                if (dot) modLen = (size_t)(dot - q);
            }
            if (modLen > SYM_MAX_MOD_NAME) modLen = SYM_MAX_MOD_NAME;
            memcpy(szModule, q, modLen);
            szModule[modLen] = '\0';
        }
        SymSetModule(hSym, szModule);

        MapQuerySegmentCount(hMap, &ulSegCount);
        for (j = 0; j < ulSegCount; j++) {
            char szSeg[SYM_MAX_SEG_NAME + 1];
            ULONG ulFlags = 0, ulSymFlags = 0, ulSegIdx = 0;
            MapQuerySegmentName(hMap, j, szSeg, sizeof(szSeg), NULL);
            MapQuerySegmentFlags(hMap, j, &ulFlags);
            if (ulFlags & MAP_SEG_32BIT) ulSymFlags |= SYM_SEGDEF_32BIT;
            if (ulFlags & MAP_SEG_ALPHA) ulSymFlags |= SYM_SEGDEF_ALPHA;
            rc = SymAddSegment(hSym, szSeg, ulSymFlags, &ulSegIdx);
            if (rc != NO_ERROR) {
                fprintf(stderr,
                        "mapsym: cannot add segment %s (error %lu)\n",
                        szSeg, (unsigned long)rc);
                MapClose(hMap); SymClose(hSym);
                free(pszMapBuf); free(pszSymBuf);
                return 1;
            }
        }

        EmitSymFromSource(hSym, (HANDLE)hMap,
                          (PFN_FINDFIRST)MapFindFirstSymbol,
                          (PFN_FINDNEXT)MapFindNextSymbol,
                          (PFN_FINDCLOSE)MapFindClose,
                          &ulAdded);
        MapClose(hMap);
    }

    rc = SymFlush(hSym);
    if (rc != NO_ERROR) {
        fprintf(stderr, "mapsym: cannot write %s (error %lu)\n",
                pszSymPath, (unsigned long)rc);
    } else {
        printf("mapsym: processed %lu segments and %lu symbols for %s\n",
               (unsigned long)ulSegCount, (unsigned long)ulAdded,
               szModule);
    }
    SymClose(hSym);
    free(pszMapBuf);
    free(pszSymBuf);
    return rc == NO_ERROR ? 0 : 1;
}

/* end of mapsym.c */
