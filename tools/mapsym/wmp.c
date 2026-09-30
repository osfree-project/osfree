/*!
 * @file  wmp.c
 * @brief Open Watcom WLINK map file reader implementation.
 *
 * Uses the ccl container library (HVECTOR). On host builds the OS/2
 * error codes are provided by os2err.h directly; INCL_DOSERRORS
 * must NOT be defined here.
 */
#include "wmp.h"
#include "ccl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*!
 * @def WMP_LINE_MAX
 * @brief Maximum length of a line read from a .wmp file.
 */
#define WMP_LINE_MAX 4096

/*!
 * @def WMP_MAGIC_OPEN
 * @brief Magic value marking an open WMP_HANDLE.
 */
#define WMP_MAGIC_OPEN   0x574D5031UL

/*!
 * @def WMP_MAGIC_CLOSED
 * @brief Magic value marking a closed WMP_HANDLE.
 */
#define WMP_MAGIC_CLOSED 0x574D5030UL

/*!
 * @def WMP_FIND_MAGIC
 * @brief Magic value marking a valid WMP_FIND cursor.
 */
#define WMP_FIND_MAGIC   0x57464E31UL

/*!
 * @brief One symbol entry within a segment.
 */
typedef struct WMP_SYMBOL {
    ULONG ulValue;  /**< @brief Symbol offset value.          */
    PSZ   pszName;  /**< @brief Owned copy of symbol name.    */
} WMP_SYMBOL;

/*!
 * @brief One segment of the WLINK map.
 */
typedef struct WMP_SEGMENT {
    ULONG   ulSegNum;   /**< @brief Segment number.        */
    ULONG   ulBaseOff;  /**< @brief Base offset.           */
    ULONG   ulSize;     /**< @brief Size in bytes.         */
    ULONG   ulFlags;    /**< @brief WMP_SEG_* flags.       */
    PSZ     pszName;    /**< @brief Owned segment name.    */
    PSZ     pszClass;   /**< @brief Owned segment class.   */
    PSZ     pszGroup;   /**< @brief Owned segment group.   */
    HVECTOR hSymbols;   /**< @brief Vector of WMP_SYMBOL*. */
} WMP_SEGMENT;

/*!
 * @brief Private state of an open WMP handle.
 */
typedef struct WMP_HANDLE {
    ULONG   ulMagic;        /**< @brief WMP_MAGIC_OPEN when valid.   */
    PSZ     pszModule;      /**< @brief Owned module name or NULL.   */
    ULONG   ulTotalSymbols; /**< @brief Total symbols.               */
    HVECTOR hSegments;      /**< @brief Vector of WMP_SEGMENT*.      */
} WMP_HANDLE;

/*!
 * @brief Iteration cursor over WMP symbols.
 */
typedef struct WMP_FIND {
    ULONG      ulMagic;   /**< @brief WMP_FIND_MAGIC when valid. */
    WMP_HANDLE *pWmp;     /**< @brief Owning map.                */
    ULONG      ulSegIdx;  /**< @brief Current segment index.     */
    ULONG      ulSymIdx;  /**< @brief Current symbol index.      */
} WMP_FIND;

/*!
 * @brief Validate a WMP handle and cast to the private struct.
 *
 * @param[in] hWmp  Public handle to validate.
 * @return Pointer to the private WMP_HANDLE.
 * @retval NULL  hWmp is NULL or has the wrong magic.
 */
static WMP_HANDLE *WmpGet(HWMP hWmp)
{
    WMP_HANDLE *pWmp = (WMP_HANDLE *)hWmp;
    if (pWmp == NULL || pWmp->ulMagic != WMP_MAGIC_OPEN) return NULL;
    return pWmp;
}

/*!
 * @brief Validate a find-handle and cast to the private struct.
 *
 * @param[in] hFind  Public cursor handle to validate.
 * @return Pointer to the private WMP_FIND.
 * @retval NULL  hFind is NULL or has the wrong magic.
 */
static WMP_FIND *WmpFindGet(HWMPFIND hFind)
{
    WMP_FIND *pFind = (WMP_FIND *)hFind;
    if (pFind == NULL || pFind->ulMagic != WMP_FIND_MAGIC) return NULL;
    return pFind;
}

/*!
 * @brief Fetch the segment pointer at a given index.
 *
 * @param[in]  pWmp   Validated map handle.
 * @param[in]  ulIdx  Zero-based segment index.
 * @param[out] ppSeg  Receives the pointer. Set to NULL on error.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_FILE_NOT_FOUND   ulIdx out of range.
 */
static APIRET WmpGetSegment(WMP_HANDLE *pWmp, ULONG ulIdx,
                            WMP_SEGMENT **ppSeg)
{
    if (VectorGetItem(pWmp->hSegments, ulIdx, ppSeg,
                      sizeof(*ppSeg), NULL) != NO_ERROR) {
        *ppSeg = NULL;
        return ERROR_FILE_NOT_FOUND;
    }
    return NO_ERROR;
}

/*!
 * @brief Fetch the symbol pointer at (segment, symbol) index.
 *
 * @param[in]  pSeg      Validated segment.
 * @param[in]  ulIdx     Zero-based symbol index.
 * @param[out] ppSym     Receives the pointer. Set to NULL on error.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_NO_MORE_ITEMS    ulIdx out of range.
 */
static APIRET WmpGetSymbol(WMP_SEGMENT *pSeg, ULONG ulIdx,
                           WMP_SYMBOL **ppSym)
{
    if (VectorGetItem(pSeg->hSymbols, ulIdx, ppSym,
                      sizeof(*ppSym), NULL) != NO_ERROR) {
        *ppSym = NULL;
        return ERROR_NO_MORE_ITEMS;
    }
    return NO_ERROR;
}

/*!
 * @brief Common string-copy helper with size-query convention.
 *
 * @param[in]  pszSrc   Source string. Not NULL.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
static APIRET WmpStringOut(PCSZ pszSrc, PSZ pszBuf,
                           ULONG ulSize, PULONG pulUsed)
{
    ULONG ulLen = (ULONG)strlen(pszSrc) + 1;
    if (pulUsed != NULL) *pulUsed = ulLen;
    if (pszBuf == NULL && ulSize == 0) return NO_ERROR;
    if (pszBuf == NULL) return ERROR_INVALID_PARAMETER;
    if (ulSize < ulLen) return ERROR_BUFFER_OVERFLOW;
    strlcpy(pszBuf, pszSrc, ulSize);
    if (pulUsed != NULL) *pulUsed = ulLen - 1;
    return NO_ERROR;
}

/*!
 * @brief Parse up to 8 hex digits from a bounded string.
 *
 * @param[in] psz  String to parse.
 * @param[in] len  Maximum number of characters to consume.
 * @return Parsed value; stops at the first non-hex character.
 */
static ULONG WmpParseHex(PCSZ psz, size_t len)
{
    ULONG val = 0;
    size_t i;
    if (len > 8) len = 8;
    for (i = 0; i < len; i++) {
        int c = (unsigned char)psz[i];
        int v;
        if (c >= '0' && c <= '9') v = c - '0';
        else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
        else break;
        val = (val << 4) | (ULONG)v;
    }
    return val;
}

/*!
 * @brief Test whether a token is a valid hex number with optional colon.
 *
 * @param[in] psz  Token to test.
 * @return Integer flag.
 * @retval 0  Token is not a valid hex address.
 * @retval 1  Token is a valid hex address.
 */
static int WmpIsHexToken(PCSZ psz)
{
    int fHasDigit = 0, fHasColon = 0;
    while (*psz) {
        if (*psz == ':') {
            if (fHasColon) return 0;
            fHasColon = 1;
        } else if (isxdigit((unsigned char)*psz)) {
            fHasDigit = 1;
        } else return 0;
        psz++;
    }
    return fHasDigit;
}

/*!
 * @brief Test whether a line is the Watcom linker banner.
 *
 * @param[in] pszLine  Line to test.
 * @return Non-zero for a banner line.
 */
static int WmpIsBanner(PCSZ pszLine)
{
    return strncmp(pszLine, "Open Watcom Linker Version ", 27) == 0;
}

/*!
 * @brief Test whether a line starts the segments table.
 *
 * @param[in] pszLine  Line to test.
 * @return Non-zero for a segments header line.
 */
static int WmpIsSegHeader(PCSZ pszLine)
{
    return strncmp(pszLine, "Segment", 7) == 0 &&
           strstr(pszLine, "Class") &&
           strstr(pszLine, "Group") &&
           strstr(pszLine, "Address") &&
           strstr(pszLine, "Size");
}

/*!
 * @brief Test whether a line starts the symbol table.
 *
 * @param[in] pszLine  Line to test.
 * @return Non-zero for a symbol header line.
 */
static int WmpIsSymHeader(PCSZ pszLine)
{
    return strncmp(pszLine, "Address", 7) == 0 &&
           strstr(pszLine, "Symbol");
}

/*!
 * @brief Split an address token into segment and offset parts.
 *
 * @param[in]  pszAddr    Address string. Not NULL.
 * @param[out] pulSegNum  Receives the segment number (0 if flat).
 * @param[out] pulOff     Receives the offset.
 * @return Integer flag.
 * @retval 0  Address was segmented ("seg:off").
 * @retval 1  Address was flat (no colon).
 */
static int WmpSplitAddress(PCSZ pszAddr, PULONG pulSegNum, PULONG pulOff)
{
    PCSZ colon = strchr(pszAddr, ':');
    if (colon != NULL) {
        *pulSegNum = WmpParseHex(pszAddr, (size_t)(colon - pszAddr));
        *pulOff    = WmpParseHex(colon + 1, strlen(colon + 1));
        return 0;
    }
    *pulSegNum = 0;
    *pulOff    = WmpParseHex(pszAddr, strlen(pszAddr));
    return 1;
}

/*!
 * @brief Release all segment and symbol storage of a map.
 *
 * @param[in,out] pWmp  Map whose storage should be freed.
 */
static void WmpFreeSegments(WMP_HANDLE *pWmp)
{
    ULONG i, count;
    if (pWmp->hSegments == NULLHANDLE) return;
    VectorGetCount(pWmp->hSegments, &count);
    for (i = 0; i < count; i++) {
        WMP_SEGMENT *pSeg = NULL;
        ULONG j, scount;
        if (VectorGetItem(pWmp->hSegments, i, &pSeg,
                          sizeof(pSeg), NULL) != NO_ERROR)
            continue;
        if (pSeg == NULL) continue;
        if (pSeg->pszName)  free(pSeg->pszName);
        if (pSeg->pszClass) free(pSeg->pszClass);
        if (pSeg->pszGroup) free(pSeg->pszGroup);
        if (pSeg->hSymbols != NULLHANDLE) {
            VectorGetCount(pSeg->hSymbols, &scount);
            for (j = 0; j < scount; j++) {
                WMP_SYMBOL *pSym = NULL;
                if (VectorGetItem(pSeg->hSymbols, j, &pSym,
                                  sizeof(pSym), NULL) != NO_ERROR)
                    continue;
                if (pSym) {
                    if (pSym->pszName) free(pSym->pszName);
                    free(pSym);
                }
            }
            VectorDestroy(pSeg->hSymbols);
        }
        free(pSeg);
    }
    VectorDestroy(pWmp->hSegments);
    pWmp->hSegments = NULLHANDLE;
    pWmp->ulTotalSymbols = 0;
}

/*!
 * @brief Free a private handle's storage and mark it closed.
 *
 * @param[in,out] pWmp  Handle to destroy. NULL is accepted.
 */
static void WmpCloseHandle(WMP_HANDLE *pWmp)
{
    if (pWmp == NULL) return;
    if (pWmp->pszModule) free(pWmp->pszModule);
    WmpFreeSegments(pWmp);
    pWmp->ulMagic = WMP_MAGIC_CLOSED;
    free(pWmp);
}

/*!
 * @brief Extract the module name from an "Executable Image:" line.
 *
 * @param[in,out] pWmp     Destination map.
 * @param[in]     pszLine  Header line. Not NULL.
 */
static void WmpSetModule(WMP_HANDLE *pWmp, PCSZ pszLine)
{
    static const char prefix[] = "Executable Image: ";
    PCSZ p, base, dot;
    size_t len;
    PSZ pszMod;

    if (strncmp(pszLine, prefix, sizeof(prefix) - 1) != 0) return;
    p = pszLine + sizeof(prefix) - 1;
    base = p;
    for (; *p; p++) if (*p == '/' || *p == '\\') base = p + 1;
    dot = strrchr(base, '.');
    len = (dot != NULL) ? (size_t)(dot - base) : strlen(base);
    if (len == 0) return;
    pszMod = (PSZ)malloc(len + 1);
    if (pszMod == NULL) return;
    memcpy(pszMod, base, len);
    pszMod[len] = '\0';
    if (pWmp->pszModule) free(pWmp->pszModule);
    pWmp->pszModule = pszMod;
}

/*!
 * @brief Parse one segment line and append a new WMP_SEGMENT.
 *
 * @param[in,out] pWmp     Destination map.
 * @param[in]     pszLine  One trimmed segment row.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Malformed row.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET WmpParseSegLine(WMP_HANDLE *pWmp, PCSZ pszLine)
{
    char szName[256], szClass[64], szGroup[64], szAddr[64], szSize[64];
    ULONG ulSegNum, ulOff, ulSize, ulFlags;
    int fFlat, n;
    WMP_SEGMENT *pSeg;

    n = sscanf(pszLine, "%255s %63s %63s %63s %63s",
               szName, szClass, szGroup, szAddr, szSize);
    if (n != 5) return ERROR_INVALID_DATA;
    if (szName[0] == '\0' || szAddr[0] == '\0' || szSize[0] == '\0')
        return ERROR_INVALID_DATA;
    if (!WmpIsHexToken(szAddr) || !WmpIsHexToken(szSize))
        return ERROR_INVALID_DATA;

    fFlat = WmpSplitAddress(szAddr, &ulSegNum, &ulOff);
    ulSize = WmpParseHex(szSize, strlen(szSize));
    ulFlags = 0;
    if (fFlat) ulFlags |= WMP_SEG_32BIT;
    else {
        PCSZ colon = strchr(szAddr, ':');
        if (colon && strlen(colon + 1) >= 8) ulFlags |= WMP_SEG_32BIT;
    }

    pSeg = (WMP_SEGMENT *)calloc(1, sizeof(*pSeg));
    if (pSeg == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pSeg->pszName  = strdup(szName);
    pSeg->pszClass = strdup(szClass);
    pSeg->pszGroup = strdup(szGroup);
    if (!pSeg->pszName || !pSeg->pszClass || !pSeg->pszGroup) {
        free(pSeg->pszName); free(pSeg->pszClass); free(pSeg->pszGroup);
        free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pSeg->ulSegNum  = ulSegNum;
    pSeg->ulBaseOff = ulOff;
    pSeg->ulSize    = ulSize;
    pSeg->ulFlags   = ulFlags;
    if (VectorCreate(sizeof(WMP_SYMBOL *), &pSeg->hSymbols) != NO_ERROR) {
        free(pSeg->pszName); free(pSeg->pszClass); free(pSeg->pszGroup);
        free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (VectorAdd(pWmp->hSegments, &pSeg) != NO_ERROR) {
        VectorDestroy(pSeg->hSymbols);
        free(pSeg->pszName); free(pSeg->pszClass); free(pSeg->pszGroup);
        free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/*!
 * @brief Locate the segment covering an address.
 *
 * @param[in]  pWmp      Validated map handle.
 * @param[in]  ulSegNum  Segment number (ignored when fFlat is set).
 * @param[in]  ulOff     Address offset.
 * @param[in]  fFlat     Non-zero for flat (no colon) addressing.
 * @param[out] pulIdx    Receives the segment index on success.
 * @return Integer flag.
 * @retval 0  No segment matched.
 * @retval 1  Segment found; *pulIdx holds its index.
 */
static int WmpFindSegment(WMP_HANDLE *pWmp, ULONG ulSegNum, ULONG ulOff,
                          int fFlat, ULONG *pulIdx)
{
    ULONG i, count;
    VectorGetCount(pWmp->hSegments, &count);
    for (i = 0; i < count; i++) {
        WMP_SEGMENT *pSeg = NULL;
        VectorGetItem(pWmp->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        if (fFlat) {
            if (ulOff >= pSeg->ulBaseOff &&
                ulOff < pSeg->ulBaseOff + pSeg->ulSize) {
                *pulIdx = i; return 1;
            }
        } else {
            if (pSeg->ulSegNum == ulSegNum &&
                ulOff >= pSeg->ulBaseOff &&
                ulOff < pSeg->ulBaseOff + pSeg->ulSize) {
                *pulIdx = i; return 1;
            }
        }
    }
    return 0;
}

/*!
 * @brief Parse one symbol row and attach the symbol to its segment.
 *
 * @param[in,out] pWmp     Destination map.
 * @param[in]     pszLine  One trimmed symbol row.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Malformed row.
 * @retval ERROR_FILE_NOT_FOUND     No matching segment.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET WmpParseSymLine(WMP_HANDLE *pWmp, PCSZ pszLine)
{
    char szAddr[64], szSym[512];
    PCSZ p, end;
    size_t n;
    ULONG ulSegNum, ulOff, ulIdx;
    int fFlat;
    WMP_SEGMENT *pSeg;
    WMP_SYMBOL *pSym;

    p = pszLine;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') return ERROR_INVALID_DATA;
    end = p;
    while (*end != '\0' && *end != ' ' && *end != '\t') end++;
    n = (size_t)(end - p);
    if (n == 0 || n >= sizeof(szAddr)) return ERROR_INVALID_DATA;
    memcpy(szAddr, p, n);
    szAddr[n] = '\0';
    if (n > 0 && (szAddr[n - 1] == '+' || szAddr[n - 1] == '*'))
        szAddr[n - 1] = '\0';
    if (!WmpIsHexToken(szAddr)) return ERROR_INVALID_DATA;

    p = end;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') return ERROR_INVALID_DATA;
    n = strlen(p);
    while (n > 0 && isspace((unsigned char)p[n - 1])) n--;
    if (n == 0 || n >= sizeof(szSym)) return ERROR_INVALID_DATA;
    memcpy(szSym, p, n);
    szSym[n] = '\0';

    fFlat = WmpSplitAddress(szAddr, &ulSegNum, &ulOff);
    if (!WmpFindSegment(pWmp, ulSegNum, ulOff, fFlat, &ulIdx))
        return ERROR_FILE_NOT_FOUND;

    VectorGetItem(pWmp->hSegments, ulIdx, &pSeg, sizeof(pSeg), NULL);
    if (pSeg == NULL) return ERROR_FILE_NOT_FOUND;

    pSym = (WMP_SYMBOL *)malloc(sizeof(*pSym));
    if (pSym == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pSym->ulValue = fFlat ? (ulOff - pSeg->ulBaseOff) : ulOff;
    pSym->pszName = strdup(szSym);
    if (pSym->pszName == NULL) { free(pSym); return ERROR_NOT_ENOUGH_MEMORY; }
    if (VectorAdd(pSeg->hSymbols, &pSym) != NO_ERROR) {
        free(pSym->pszName); free(pSym);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pWmp->ulTotalSymbols++;
    return NO_ERROR;
}

/*!
 * @brief Open a Watcom WLINK map file.
 *
 * @param[in]  pszPath  Path to the .wmp file. Not NULL.
 * @param[out] phWmp    Receives the new handle. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or phWmp is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_INVALID_DATA       File has no linker banner or no segments.
 */
APIRET APIENTRY WmpOpen(PCSZ pszPath, HWMP *phWmp)
{
    FILE *pFile;
    char szLine[WMP_LINE_MAX];
    int state = 0;
    WMP_HANDLE *pWmp;

    if (pszPath == NULL || phWmp == NULL) return ERROR_INVALID_PARAMETER;
    *phWmp = NULLHANDLE;

    pWmp = (WMP_HANDLE *)calloc(1, sizeof(WMP_HANDLE));
    if (pWmp == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pWmp->ulMagic = WMP_MAGIC_OPEN;
    if (VectorCreate(sizeof(WMP_SEGMENT *), &pWmp->hSegments) != NO_ERROR) {
        WmpCloseHandle(pWmp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pFile = fopen(pszPath, "r");
    if (pFile == NULL) {
        WmpCloseHandle(pWmp);
        return ERROR_OPEN_FAILED;
    }
    if (fgets(szLine, sizeof(szLine), pFile) == NULL ||
        !WmpIsBanner(szLine)) {
        fclose(pFile);
        WmpCloseHandle(pWmp);
        return ERROR_INVALID_DATA;
    }

    while (fgets(szLine, sizeof(szLine), pFile) != NULL) {
        size_t len = strlen(szLine);
        while (len > 0 && (szLine[len - 1] == '\n' ||
                           szLine[len - 1] == '\r'))
            szLine[--len] = '\0';

        if (strncmp(szLine, "Executable Image: ", 18) == 0) {
            WmpSetModule(pWmp, szLine); continue;
        }
        if (WmpIsSegHeader(szLine)) { state = 1; continue; }
        if (WmpIsSymHeader(szLine)) { state = 2; continue; }
        if (szLine[0] == '+' || szLine[0] == '|' || szLine[0] == '=' ||
            szLine[0] == '*' || szLine[0] == '\0') continue;
        if (strncmp(szLine, "Module:", 7) == 0) continue;

        if (state == 1) WmpParseSegLine(pWmp, szLine);
        else if (state == 2) WmpParseSymLine(pWmp, szLine);
    }
    fclose(pFile);

    {
        ULONG ulCount = 0;
        VectorGetCount(pWmp->hSegments, &ulCount);
        if (ulCount == 0) {
            WmpCloseHandle(pWmp);
            return ERROR_INVALID_DATA;
        }
    }
    *phWmp = (HWMP)pWmp;
    return NO_ERROR;
}

/*!
 * @brief Close a WMP handle.
 *
 * @param[in] hWmp  Handle. NULLHANDLE is a no-op.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY WmpClose(HWMP hWmp)
{
    WMP_HANDLE *pWmp;
    if (hWmp == NULLHANDLE) return NO_ERROR;
    pWmp = WmpGet(hWmp);
    if (pWmp == NULL) return ERROR_INVALID_HANDLE;
    WmpCloseHandle(pWmp);
    return NO_ERROR;
}

/*!
 * @brief Query the module name.
 *
 * @param[in]  hWmp     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Module name not set.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpQueryModule(HWMP hWmp, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    if (pWmp == NULL) return ERROR_INVALID_HANDLE;
    if (pWmp->pszModule == NULL) return ERROR_FILE_NOT_FOUND;
    return WmpStringOut(pWmp->pszModule, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the total number of symbols across all segments.
 *
 * @param[in]  hWmp      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY WmpQuerySymbolCount(HWMP hWmp, PULONG pulCount)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    if (pWmp == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    *pulCount = pWmp->ulTotalSymbols;
    return NO_ERROR;
}

/*!
 * @brief Query the number of segments.
 *
 * @param[in]  hWmp      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY WmpQuerySegmentCount(HWMP hWmp, PULONG pulCount)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    if (pWmp == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(pWmp->hSegments, pulCount);
}

/*!
 * @brief Query the name of a segment.
 *
 * @param[in]  hWmp     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpQuerySegmentName(HWMP hWmp, ULONG ulSegIdx,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL) return ERROR_INVALID_HANDLE;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    return WmpStringOut(pSeg->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the class of a segment.
 *
 * @param[in]  hWmp     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpQuerySegmentClass(HWMP hWmp, ULONG ulSegIdx,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL) return ERROR_INVALID_HANDLE;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (pSeg->pszClass == NULL) return ERROR_FILE_NOT_FOUND;
    return WmpStringOut(pSeg->pszClass, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the group of a segment.
 *
 * @param[in]  hWmp     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpQuerySegmentGroup(HWMP hWmp, ULONG ulSegIdx,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL) return ERROR_INVALID_HANDLE;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (pSeg->pszGroup == NULL) return ERROR_FILE_NOT_FOUND;
    return WmpStringOut(pSeg->pszGroup, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the flags of a segment (WMP_SEG_*).
 *
 * @param[in]  hWmp      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[out] pulFlags  Receives the flags. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulFlags is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY WmpQuerySegmentFlags(HWMP hWmp, ULONG ulSegIdx, PULONG pulFlags)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL || pulFlags == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulFlags = pSeg->ulFlags;
    return NO_ERROR;
}

/*!
 * @brief Query the size of a segment in bytes.
 *
 * @param[in]  hWmp     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[out] pulSize  Receives the size. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSize is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY WmpQuerySegmentSize(HWMP hWmp, ULONG ulSegIdx, PULONG pulSize)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL || pulSize == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulSize = pSeg->ulSize;
    return NO_ERROR;
}

/*!
 * @brief Query the base offset of a segment.
 *
 * @param[in]  hWmp        Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx    Zero-based segment index.
 * @param[out] pulBaseOff  Receives the base offset. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulBaseOff is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY WmpQuerySegmentBaseOff(HWMP hWmp, ULONG ulSegIdx, PULONG pulBaseOff)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL || pulBaseOff == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulBaseOff = pSeg->ulBaseOff;
    return NO_ERROR;
}

/*!
 * @brief Query the segment number of a segment.
 *
 * @param[in]  hWmp       Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx   Zero-based segment index.
 * @param[out] pulSegNum  Receives the segment number. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSegNum is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY WmpQuerySegmentSegNum(HWMP hWmp, ULONG ulSegIdx, PULONG pulSegNum)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL || pulSegNum == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulSegNum = pSeg->ulSegNum;
    return NO_ERROR;
}

/*!
 * @brief Query the number of symbols in one segment.
 *
 * @param[in]  hWmp      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY WmpQuerySegmentSymbolCount(HWMP hWmp, ULONG ulSegIdx, PULONG pulCount)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    if (pWmp == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    return VectorGetCount(pSeg->hSymbols, pulCount);
}

/*!
 * @brief Query the value of a symbol.
 *
 * @param[in]  hWmp      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[in]  ulSymIdx  Zero-based symbol index.
 * @param[out] pulValue  Receives the value. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulValue is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 * @retval ERROR_NO_MORE_ITEMS      ulSymIdx out of range.
 */
APIRET APIENTRY WmpQuerySymbolValue(HWMP hWmp, ULONG ulSegIdx,
                                    ULONG ulSymIdx, PULONG pulValue)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    WMP_SYMBOL *pSym;
    if (pWmp == NULL || pulValue == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (WmpGetSymbol(pSeg, ulSymIdx, &pSym) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    *pulValue = pSym->ulValue;
    return NO_ERROR;
}

/*!
 * @brief Query the name of a symbol.
 *
 * @param[in]  hWmp     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[in]  ulSymIdx Zero-based symbol index.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 * @retval ERROR_NO_MORE_ITEMS      ulSymIdx out of range.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpQuerySymbolName(HWMP hWmp, ULONG ulSegIdx,
                                   ULONG ulSymIdx, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    WMP_SYMBOL *pSym;
    if (pWmp == NULL) return ERROR_INVALID_HANDLE;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (WmpGetSymbol(pSeg, ulSymIdx, &pSym) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    return WmpStringOut(pSym->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Find a symbol by its exact value.
 *
 * @param[in]  hWmp       Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx   Zero-based segment index.
 * @param[in]  ulValue    Value to look for.
 * @param[out] pulSymIdx  Receives the symbol index. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSymIdx is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Not found or ulSegIdx out of range.
 */
APIRET APIENTRY WmpFindSymbolByValue(HWMP hWmp, ULONG ulSegIdx,
                                     ULONG ulValue, PULONG pulSymIdx)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_SEGMENT *pSeg;
    ULONG i, count;
    if (pWmp == NULL || pulSymIdx == NULL) return ERROR_INVALID_PARAMETER;
    if (WmpGetSegment(pWmp, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    VectorGetCount(pSeg->hSymbols, &count);
    for (i = 0; i < count; i++) {
        WMP_SYMBOL *pSym = NULL;
        VectorGetItem(pSeg->hSymbols, i, &pSym, sizeof(pSym), NULL);
        if (pSym != NULL && pSym->ulValue == ulValue) {
            *pulSymIdx = i; return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Find a symbol by name across all segments.
 *
 * @param[in]  hWmp       Handle. Not NULLHANDLE.
 * @param[in]  pszName    Name to look for. Not NULL.
 * @param[out] pulSegIdx  Optional. Receives the segment index.
 * @param[out] pulSymIdx  Optional. Receives the symbol index.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pszName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Not found.
 */
APIRET APIENTRY WmpFindSymbolByName(HWMP hWmp, PCSZ pszName,
                                    PULONG pulSegIdx, PULONG pulSymIdx)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    ULONG i, j, segCount, symCount;
    if (pWmp == NULL || pszName == NULL) return ERROR_INVALID_PARAMETER;
    VectorGetCount(pWmp->hSegments, &segCount);
    for (i = 0; i < segCount; i++) {
        WMP_SEGMENT *pSeg = NULL;
        VectorGetItem(pWmp->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        VectorGetCount(pSeg->hSymbols, &symCount);
        for (j = 0; j < symCount; j++) {
            WMP_SYMBOL *pSym = NULL;
            VectorGetItem(pSeg->hSymbols, j, &pSym, sizeof(pSym), NULL);
            if (pSym != NULL && strcmp(pSym->pszName, pszName) == 0) {
                if (pulSegIdx) *pulSegIdx = i;
                if (pulSymIdx) *pulSymIdx = j;
                return NO_ERROR;
            }
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Start iterating over all symbols.
 *
 * @param[in]  hWmp         Handle. Not NULLHANDLE.
 * @param[out] phFind       Receives the cursor. Not NULL.
 * @param[out] pulSegIdx    Receives segment index. Not NULL.
 * @param[out] pulSymIdx    Receives symbol index. Not NULL.
 * @param[out] pszName      Output buffer. Not NULL.
 * @param[in]  ulNameSize   Size of pszName in bytes.
 * @param[out] pulNameUsed  Optional. Receives required size.
 * @param[out] pulValue     Receives symbol value. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  Any required pointer is NULL.
 * @retval ERROR_NO_MORE_ITEMS      There are no symbols.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpFindFirstSymbol(HWMP hWmp, HWMPFIND *phFind,
                                   PULONG pulSegIdx, PULONG pulSymIdx,
                                   PSZ pszName, ULONG ulNameSize,
                                   PULONG pulNameUsed, PULONG pulValue)
{
    WMP_HANDLE *pWmp = WmpGet(hWmp);
    WMP_FIND *pFind;
    ULONG i, segCount;
    APIRET rc;

    if (pWmp == NULL || phFind == NULL || pulSegIdx == NULL ||
        pulSymIdx == NULL || pszName == NULL || pulValue == NULL)
        return ERROR_INVALID_PARAMETER;
    *phFind = NULLHANDLE;

    VectorGetCount(pWmp->hSegments, &segCount);
    for (i = 0; i < segCount; i++) {
        WMP_SEGMENT *pSeg = NULL;
        ULONG cnt = 0;
        VectorGetItem(pWmp->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &cnt);
        if (cnt > 0) break;
    }
    if (i >= segCount) return ERROR_NO_MORE_ITEMS;

    {
        WMP_SEGMENT *pSeg = NULL;
        WMP_SYMBOL *pSym = NULL;
        VectorGetItem(pWmp->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        VectorGetItem(pSeg->hSymbols, 0, &pSym, sizeof(pSym), NULL);
        if (pSym == NULL) return ERROR_NO_MORE_ITEMS;

        pFind = (WMP_FIND *)calloc(1, sizeof(WMP_FIND));
        if (pFind == NULL) return ERROR_NOT_ENOUGH_MEMORY;
        pFind->ulMagic  = WMP_FIND_MAGIC;
        pFind->pWmp     = pWmp;
        pFind->ulSegIdx = i;
        pFind->ulSymIdx = 0;
        *phFind = (HWMPFIND)pFind;

        rc = WmpStringOut(pSym->pszName, pszName, ulNameSize, pulNameUsed);
        if (rc != NO_ERROR) {
            free(pFind);
            *phFind = NULLHANDLE;
            return rc;
        }
        *pulSegIdx = i;
        *pulSymIdx = 0;
        *pulValue  = pSym->ulValue;
    }
    return NO_ERROR;
}

/*!
 * @brief Advance the iteration to the next symbol.
 *
 * @param[in]  hFind        Cursor. Not NULLHANDLE.
 * @param[out] pulSegIdx    Receives segment index. Not NULL.
 * @param[out] pulSymIdx    Receives symbol index. Not NULL.
 * @param[out] pszName      Output buffer. Not NULL.
 * @param[in]  ulNameSize   Size of pszName in bytes.
 * @param[out] pulNameUsed  Optional. Receives required size.
 * @param[out] pulValue     Receives symbol value. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Cursor is not recognized.
 * @retval ERROR_INVALID_PARAMETER  Any required pointer is NULL.
 * @retval ERROR_NO_MORE_ITEMS      End of iteration.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY WmpFindNextSymbol(HWMPFIND hFind,
                                  PULONG pulSegIdx, PULONG pulSymIdx,
                                  PSZ pszName, ULONG ulNameSize,
                                  PULONG pulNameUsed, PULONG pulValue)
{
    WMP_FIND *pFind = WmpFindGet(hFind);
    WMP_HANDLE *pWmp;
    ULONG segCount;
    APIRET rc;

    if (pFind == NULL || pulSegIdx == NULL || pulSymIdx == NULL ||
        pszName == NULL || pulValue == NULL)
        return ERROR_INVALID_PARAMETER;
    pWmp = pFind->pWmp;
    VectorGetCount(pWmp->hSegments, &segCount);

    pFind->ulSymIdx++;
    while (pFind->ulSegIdx < segCount) {
        WMP_SEGMENT *pSeg = NULL;
        ULONG cnt = 0;
        VectorGetItem(pWmp->hSegments, pFind->ulSegIdx, &pSeg,
                      sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &cnt);
        if (pFind->ulSymIdx < cnt) break;
        pFind->ulSegIdx++;
        pFind->ulSymIdx = 0;
    }
    if (pFind->ulSegIdx >= segCount) return ERROR_NO_MORE_ITEMS;

    {
        WMP_SEGMENT *pSeg = NULL;
        WMP_SYMBOL *pSym = NULL;
        VectorGetItem(pWmp->hSegments, pFind->ulSegIdx, &pSeg,
                      sizeof(pSeg), NULL);
        VectorGetItem(pSeg->hSymbols, pFind->ulSymIdx, &pSym,
                      sizeof(pSym), NULL);
        if (pSym == NULL) return ERROR_NO_MORE_ITEMS;
        rc = WmpStringOut(pSym->pszName, pszName, ulNameSize, pulNameUsed);
        if (rc != NO_ERROR) return rc;
        *pulSegIdx = pFind->ulSegIdx;
        *pulSymIdx = pFind->ulSymIdx;
        *pulValue  = pSym->ulValue;
    }
    return NO_ERROR;
}

/*!
 * @brief Close an iteration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is a no-op.
 * @return APIRET.
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Cursor is not recognized.
 */
APIRET APIENTRY WmpFindClose(HWMPFIND hFind)
{
    WMP_FIND *pFind = WmpFindGet(hFind);
    if (pFind == NULL) {
        if (hFind == NULLHANDLE) return NO_ERROR;
        return ERROR_INVALID_HANDLE;
    }
    pFind->ulMagic = 0;
    free(pFind);
    return NO_ERROR;
}
