/*!
 * @file  map.c
 * @brief Microsoft LINK map file reader implementation.
 *
 * Uses the ccl container library (HVECTOR) for dynamic segment and
 * symbol storage. On host builds the OS/2 error codes are provided
 * by os2err.h directly; INCL_DOSERRORS must NOT be defined here.
 */
#include "map.h"
#include "ccl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*!
 * @def MS_LINE_MAX
 * @brief Maximum length of a line read from a .map file.
 */
#define MS_LINE_MAX 4096

/*!
 * @def MS_MAGIC_OPEN
 * @brief Magic value marking an open MS_HANDLE.
 */
#define MS_MAGIC_OPEN   0x4D415031UL

/*!
 * @def MS_MAGIC_CLOSED
 * @brief Magic value marking a closed MS_HANDLE.
 */
#define MS_MAGIC_CLOSED 0x4D415030UL

/*!
 * @def MS_FIND_MAGIC
 * @brief Magic value marking a valid MS_FIND cursor.
 */
#define MS_FIND_MAGIC   0x4D464E31UL

/*!
 * @brief One symbol entry within a segment.
 */
typedef struct MS_SYMBOL {
    ULONG ulValue;   /**< @brief Symbol value (offset).     */
    PSZ   pszName;   /**< @brief Owned copy of symbol name. */
} MS_SYMBOL;

/*!
 * @brief One segment parsed from the map file.
 */
typedef struct MS_SEGMENT {
    ULONG   ulSegNum;   /**< @brief Segment number.             */
    ULONG   ulBaseOff;  /**< @brief Base offset of the segment. */
    ULONG   ulSize;     /**< @brief Size in bytes.              */
    ULONG   ulFlags;    /**< @brief MAP_SEG_* flags.            */
    PSZ     pszName;    /**< @brief Owned segment name.         */
    PSZ     pszClass;   /**< @brief Owned segment class.        */
    HVECTOR hSymbols;   /**< @brief Vector of MS_SYMBOL*.       */
} MS_SEGMENT;

/*!
 * @brief Private state of an open map handle.
 */
typedef struct MS_HANDLE {
    ULONG   ulMagic;        /**< @brief MS_MAGIC_OPEN when valid.      */
    PSZ     pszModule;      /**< @brief Owned module name or NULL.     */
    ULONG   ulTotalSymbols; /**< @brief Total symbols across segments. */
    HVECTOR hSegments;      /**< @brief Vector of MS_SEGMENT*.         */
} MS_HANDLE;

/*!
 * @brief Iteration cursor over all symbols.
 */
typedef struct MS_FIND {
    ULONG      ulMagic;   /**< @brief MS_FIND_MAGIC when valid. */
    MS_HANDLE *pMap;      /**< @brief Owning map.               */
    ULONG      ulSegIdx;  /**< @brief Current segment index.    */
    ULONG      ulSymIdx;  /**< @brief Current symbol index.     */
} MS_FIND;

/*!
 * @brief Validate a map handle and cast it to the private struct.
 *
 * @param[in] hMap Public handle to validate.
 * @return Pointer to the private MS_HANDLE.
 * @retval NULL  hMap is NULL or has the wrong magic value.
 */
static MS_HANDLE *MsGet(HMAP hMap)
{
    MS_HANDLE *pMap = (MS_HANDLE *)hMap;
    if (pMap == NULL || pMap->ulMagic != MS_MAGIC_OPEN)
        return NULL;
    return pMap;
}

/*!
 * @brief Validate a find-handle and cast it to the private struct.
 *
 * @param[in] hFind Public cursor handle to validate.
 * @return Pointer to the private MS_FIND.
 * @retval NULL  hFind is NULL or has the wrong magic value.
 */
static MS_FIND *MsFindGet(HMAPFIND hFind)
{
    MS_FIND *pFind = (MS_FIND *)hFind;
    if (pFind == NULL || pFind->ulMagic != MS_FIND_MAGIC)
        return NULL;
    return pFind;
}

/*!
 * @brief Fetch the segment pointer stored at a given index.
 *
 * @param[in]  pMap   Validated map handle.
 * @param[in]  ulIdx  Zero-based segment index.
 * @param[out] ppSeg  Receives the pointer. Set to NULL on error.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     ulIdx is out of range.
 */
static APIRET MsGetSegment(MS_HANDLE *pMap, ULONG ulIdx,
                           MS_SEGMENT **ppSeg)
{
    if (VectorGetItem(pMap->hSegments, ulIdx, ppSeg,
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
 * @param[in]  ulSymIdx  Zero-based symbol index.
 * @param[out] ppSym     Receives the pointer. Set to NULL on error.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NO_MORE_ITEMS      ulSymIdx is out of range.
 */
static APIRET MsGetSymbol(MS_SEGMENT *pSeg, ULONG ulSymIdx,
                          MS_SYMBOL **ppSym)
{
    if (VectorGetItem(pSeg->hSymbols, ulSymIdx, ppSym,
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
 * @param[out] pulUsed  Optional. Receives the required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
static APIRET MsStringOut(PCSZ pszSrc, PSZ pszBuf,
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
static ULONG MsParseHex(PCSZ psz, size_t len)
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
 * @brief Split "seg:off" in place.
 *
 * @param[in,out] pszAddr    Buffer containing the address; the colon
 *                           is replaced by a NUL on success.
 * @param[out]    pulSegNum  Receives the segment part.
 * @param[out]    pulOff     Receives the offset part.
 * @return Status.
 * @retval 0  Address had no colon; nothing was written.
 * @retval 1  Address was successfully split.
 */
static int MsSplitAddress(PSZ pszAddr, PULONG pulSegNum, PULONG pulOff)
{
    PSZ colon = strchr(pszAddr, ':');
    if (colon == NULL) return 0;
    *colon = '\0';
    *pulSegNum = MsParseHex(pszAddr, strlen(pszAddr));
    *pulOff    = MsParseHex(colon + 1, strlen(colon + 1));
    return 1;
}

/*!
 * @brief Strip a trailing " (comment)" from a symbol name in place.
 *
 * @param[in,out] psz  Symbol name to clean. Not NULL.
 */
static void MsStripParenComment(PSZ psz)
{
    PSZ p = strrchr(psz, '(');
    if (p != NULL) {
        PSZ q = p;
        while (q > psz && isspace((unsigned char)q[-1])) q--;
        if (q > psz && *q == ' ') { *q = '\0'; return; }
    }
}

/*!
 * @brief Test whether a line is the segments section header.
 *
 * @param[in] pszLine  Line to test. Not NULL.
 * @return Non-zero when the line starts the segments table.
 */
static int MsIsSegHeader(PCSZ pszLine)
{
    return strncmp(pszLine, "Start", 5) == 0 &&
           strstr(pszLine, "Length") != NULL &&
           strstr(pszLine, "Name") != NULL;
}

/*!
 * @brief Test whether a line is the "Publics by Value" header.
 *
 * @param[in] pszLine  Line to test. Not NULL.
 * @return Non-zero when the line starts the symbol table.
 */
static int MsIsSymHeader(PCSZ pszLine)
{
    return strstr(pszLine, "Publics by Value") != NULL;
}

/*!
 * @brief Release all segment and symbol storage of a map.
 *
 * @param[in,out] pMap  Map whose storage should be freed.
 */
static void MsFreeSegments(MS_HANDLE *pMap)
{
    ULONG i, count;
    if (pMap->hSegments == NULLHANDLE) return;
    VectorGetCount(pMap->hSegments, &count);
    for (i = 0; i < count; i++) {
        MS_SEGMENT *pSeg = NULL;
        ULONG j, scount;
        if (VectorGetItem(pMap->hSegments, i, &pSeg,
                          sizeof(pSeg), NULL) != NO_ERROR)
            continue;
        if (pSeg == NULL) continue;
        if (pSeg->pszName) free(pSeg->pszName);
        if (pSeg->pszClass) free(pSeg->pszClass);
        if (pSeg->hSymbols != NULLHANDLE) {
            VectorGetCount(pSeg->hSymbols, &scount);
            for (j = 0; j < scount; j++) {
                MS_SYMBOL *pSym = NULL;
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
    VectorDestroy(pMap->hSegments);
    pMap->hSegments = NULLHANDLE;
    pMap->ulTotalSymbols = 0;
}

/*!
 * @brief Free a private handle's storage and mark it closed.
 *
 * @param[in,out] pMap  Handle to destroy. NULL is accepted.
 */
static void MsCloseHandle(MS_HANDLE *pMap)
{
    if (pMap == NULL) return;
    if (pMap->pszModule) free(pMap->pszModule);
    MsFreeSegments(pMap);
    pMap->ulMagic = MS_MAGIC_CLOSED;
    free(pMap);
}

/*!
 * @brief Parse one segment line and append a new MS_SEGMENT.
 *
 * @param[in,out] pMap     Destination map.
 * @param[in]     pszLine  One trimmed segment row.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Malformed row.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET MsParseSegLine(MS_HANDLE *pMap, PCSZ pszLine)
{
    char szAddr[64], szLen[64], szName[256], szClass[64], szExtra[64];
    ULONG ulSegNum, ulBaseOff, ulSize, ulFlags;
    int n;
    MS_SEGMENT *pSeg;

    szExtra[0] = '\0';
    n = sscanf(pszLine, "%63s %63s %255s %63s %63s",
               szAddr, szLen, szName, szClass, szExtra);
    if (n < 4) return ERROR_INVALID_DATA;
    if (!MsSplitAddress(szAddr, &ulSegNum, &ulBaseOff))
        return ERROR_INVALID_DATA;
    if (szName[0] == '\0') return ERROR_INVALID_DATA;

    ulSize = MsParseHex(szLen, strlen(szLen));
    ulFlags = 0;
    if (strstr(szClass, "32-bit") || strstr(szExtra, "32-bit"))
        ulFlags |= MAP_SEG_32BIT;

    pSeg = (MS_SEGMENT *)calloc(1, sizeof(*pSeg));
    if (pSeg == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pSeg->pszName  = strdup(szName);
    pSeg->pszClass = strdup(szClass);
    if (pSeg->pszName == NULL || pSeg->pszClass == NULL) {
        free(pSeg->pszName); free(pSeg->pszClass); free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pSeg->ulSegNum  = ulSegNum;
    pSeg->ulBaseOff = ulBaseOff;
    pSeg->ulSize    = ulSize;
    pSeg->ulFlags   = ulFlags;
    if (VectorCreate(sizeof(MS_SYMBOL *), &pSeg->hSymbols) != NO_ERROR) {
        free(pSeg->pszName); free(pSeg->pszClass); free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (VectorAdd(pMap->hSegments, &pSeg) != NO_ERROR) {
        VectorDestroy(pSeg->hSymbols);
        free(pSeg->pszName); free(pSeg->pszClass); free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    return NO_ERROR;
}

/*!
 * @brief Parse one "Publics by Value" line and attach the symbol.
 *
 * @param[in,out] pMap     Destination map.
 * @param[in]     pszLine  One trimmed symbol row.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Malformed row.
 * @retval ERROR_FILE_NOT_FOUND     Segment of the symbol is unknown.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET MsParseSymLine(MS_HANDLE *pMap, PCSZ pszLine)
{
    char szAddr[64], szSym[512];
    PCSZ p;
    size_t addrLen, n;
    ULONG ulSegNum, ulOff, ulIdx, ulCount;
    MS_SEGMENT *pSeg = NULL;
    MS_SYMBOL *pSym;

    p = pszLine;
    while (isspace((unsigned char)*p)) p++;

    addrLen = 0;
    while (p[addrLen] != '\0' && !isspace((unsigned char)p[addrLen]))
        addrLen++;
    if (addrLen == 0 || addrLen >= sizeof(szAddr))
        return ERROR_INVALID_DATA;
    memcpy(szAddr, p, addrLen);
    szAddr[addrLen] = '\0';
    p += addrLen;
    while (isspace((unsigned char)*p)) p++;

    if (strncmp(p, "Imp", 3) == 0 &&
        (p[3] == '\0' || isspace((unsigned char)p[3]))) {
        p += 3;
        while (isspace((unsigned char)*p)) p++;
    }
    if (*p == '\0') return ERROR_INVALID_DATA;

    n = strlen(p);
    while (n > 0 && isspace((unsigned char)p[n - 1])) n--;
    if (n == 0 || n >= sizeof(szSym)) return ERROR_INVALID_DATA;
    memcpy(szSym, p, n);
    szSym[n] = '\0';
    MsStripParenComment(szSym);
    if (szSym[0] == '\0') return ERROR_INVALID_DATA;

    if (!MsSplitAddress(szAddr, &ulSegNum, &ulOff))
        return ERROR_INVALID_DATA;

    VectorGetCount(pMap->hSegments, &ulCount);
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        MS_SEGMENT *pCand = NULL;
        VectorGetItem(pMap->hSegments, ulIdx, &pCand,
                      sizeof(pCand), NULL);
        if (pCand != NULL && pCand->ulSegNum == ulSegNum) {
            pSeg = pCand;
            break;
        }
    }
    if (pSeg == NULL) return ERROR_FILE_NOT_FOUND;

    pSym = (MS_SYMBOL *)malloc(sizeof(*pSym));
    if (pSym == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pSym->ulValue  = ulOff;
    pSym->pszName  = strdup(szSym);
    if (pSym->pszName == NULL) { free(pSym); return ERROR_NOT_ENOUGH_MEMORY; }
    if (VectorAdd(pSeg->hSymbols, &pSym) != NO_ERROR) {
        free(pSym->pszName); free(pSym);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pMap->ulTotalSymbols++;
    return NO_ERROR;
}

/*!
 * @brief Open a Microsoft LINK map file.
 *
 * @param[in]  pszPath  Path to the map file. Not NULL.
 * @param[out] phMap    Receives the new handle. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or phMap is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_INVALID_DATA       File has no segment table.
 */
APIRET APIENTRY MapOpen(PCSZ pszPath, HMAP *phMap)
{
    FILE *pFile;
    char szLine[MS_LINE_MAX];
    int fInSegments = 0, fInAddresses = 0;
    MS_HANDLE *pMap;

    if (pszPath == NULL || phMap == NULL)
        return ERROR_INVALID_PARAMETER;
    *phMap = NULLHANDLE;

    pMap = (MS_HANDLE *)calloc(1, sizeof(MS_HANDLE));
    if (pMap == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pMap->ulMagic = MS_MAGIC_OPEN;
    if (VectorCreate(sizeof(MS_SEGMENT *), &pMap->hSegments) != NO_ERROR) {
        MsCloseHandle(pMap);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pFile = fopen(pszPath, "r");
    if (pFile == NULL) {
        MsCloseHandle(pMap);
        return ERROR_OPEN_FAILED;
    }

    while (fgets(szLine, sizeof(szLine), pFile) != NULL) {
        size_t len = strlen(szLine);
        while (len > 0 && (szLine[len - 1] == '\n' ||
                           szLine[len - 1] == '\r'))
            szLine[--len] = '\0';

        if (MsIsSegHeader(szLine)) {
            fInSegments = 1;
            fInAddresses = 0;
            continue;
        }
        if (MsIsSymHeader(szLine)) {
            fInAddresses = 1;
            fInSegments = 0;
            continue;
        }
        if (szLine[0] == '=' || szLine[0] == '\0')
            continue;

        if (fInSegments) {
            MsParseSegLine(pMap, szLine);
        } else if (fInAddresses) {
            MsParseSymLine(pMap, szLine);
        }
    }
    fclose(pFile);

    {
        ULONG ulCount = 0;
        VectorGetCount(pMap->hSegments, &ulCount);
        if (ulCount == 0) {
            MsCloseHandle(pMap);
            return ERROR_INVALID_DATA;
        }
    }
    *phMap = (HMAP)pMap;
    return NO_ERROR;
}

/*!
 * @brief Close a map handle.
 *
 * @param[in] hMap  Handle. NULLHANDLE is a no-op.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY MapClose(HMAP hMap)
{
    MS_HANDLE *pMap;
    if (hMap == NULLHANDLE) return NO_ERROR;
    pMap = MsGet(hMap);
    if (pMap == NULL) return ERROR_INVALID_HANDLE;
    MsCloseHandle(pMap);
    return NO_ERROR;
}

/*!
 * @brief Set the module name.
 *
 * @param[in] hMap       Handle. Not NULLHANDLE.
 * @param[in] pszModule  Module name. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pszModule is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY MapSetModule(HMAP hMap, PCSZ pszModule)
{
    MS_HANDLE *pMap = MsGet(hMap);
    PSZ pszNew;
    if (pMap == NULL) return ERROR_INVALID_HANDLE;
    if (pszModule == NULL) return ERROR_INVALID_PARAMETER;
    pszNew = strdup(pszModule);
    if (pszNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    if (pMap->pszModule) free(pMap->pszModule);
    pMap->pszModule = pszNew;
    return NO_ERROR;
}

/*!
 * @brief Query the module name.
 *
 * @param[in]  hMap     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No module name has been set.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY MapQueryModule(HMAP hMap, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed)
{
    MS_HANDLE *pMap = MsGet(hMap);
    if (pMap == NULL) return ERROR_INVALID_HANDLE;
    if (pMap->pszModule == NULL) return ERROR_FILE_NOT_FOUND;
    return MsStringOut(pMap->pszModule, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the total number of symbols across all segments.
 *
 * @param[in]  hMap      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY MapQuerySymbolCount(HMAP hMap, PULONG pulCount)
{
    MS_HANDLE *pMap = MsGet(hMap);
    if (pMap == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    *pulCount = pMap->ulTotalSymbols;
    return NO_ERROR;
}

/*!
 * @brief Query the number of segments.
 *
 * @param[in]  hMap      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY MapQuerySegmentCount(HMAP hMap, PULONG pulCount)
{
    MS_HANDLE *pMap = MsGet(hMap);
    if (pMap == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(pMap->hSegments, pulCount);
}

/*!
 * @brief Query the name of a segment.
 *
 * @param[in]  hMap     Handle. Not NULLHANDLE.
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
APIRET APIENTRY MapQuerySegmentName(HMAP hMap, ULONG ulSegIdx,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL) return ERROR_INVALID_HANDLE;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    return MsStringOut(pSeg->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the class of a segment (Microsoft-specific).
 *
 * @param[in]  hMap     Handle. Not NULLHANDLE.
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
APIRET APIENTRY MapQuerySegmentClass(HMAP hMap, ULONG ulSegIdx,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL) return ERROR_INVALID_HANDLE;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (pSeg->pszClass == NULL) return ERROR_FILE_NOT_FOUND;
    return MsStringOut(pSeg->pszClass, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the flags of a segment (MAP_SEG_*).
 *
 * @param[in]  hMap      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[out] pulFlags  Receives the flags. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulFlags is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY MapQuerySegmentFlags(HMAP hMap, ULONG ulSegIdx,
                                     PULONG pulFlags)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL || pulFlags == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulFlags = pSeg->ulFlags;
    return NO_ERROR;
}

/*!
 * @brief Query the size of a segment in bytes.
 *
 * @param[in]  hMap     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[out] pulSize  Receives the size. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSize is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY MapQuerySegmentSize(HMAP hMap, ULONG ulSegIdx,
                                    PULONG pulSize)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL || pulSize == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulSize = pSeg->ulSize;
    return NO_ERROR;
}

/*!
 * @brief Query the base offset of a segment.
 *
 * @param[in]  hMap        Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx    Zero-based segment index.
 * @param[out] pulBaseOff  Receives the base offset. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulBaseOff is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY MapQuerySegmentBaseOff(HMAP hMap, ULONG ulSegIdx,
                                       PULONG pulBaseOff)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL || pulBaseOff == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulBaseOff = pSeg->ulBaseOff;
    return NO_ERROR;
}

/*!
 * @brief Query the segment number of a segment.
 *
 * @param[in]  hMap       Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx   Zero-based segment index.
 * @param[out] pulSegNum  Receives the segment number. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSegNum is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY MapQuerySegmentSegNum(HMAP hMap, ULONG ulSegIdx,
                                      PULONG pulSegNum)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL || pulSegNum == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    *pulSegNum = pSeg->ulSegNum;
    return NO_ERROR;
}

/*!
 * @brief Query the number of symbols in one segment.
 *
 * @param[in]  hMap      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 * @retval ERROR_FILE_NOT_FOUND     ulSegIdx out of range.
 */
APIRET APIENTRY MapQuerySegmentSymbolCount(HMAP hMap, ULONG ulSegIdx,
                                           PULONG pulCount)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    if (pMap == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    return VectorGetCount(pSeg->hSymbols, pulCount);
}

/*!
 * @brief Query the value of a symbol.
 *
 * @param[in]  hMap      Handle. Not NULLHANDLE.
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
APIRET APIENTRY MapQuerySymbolValue(HMAP hMap, ULONG ulSegIdx,
                                    ULONG ulSymIdx, PULONG pulValue)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    MS_SYMBOL *pSym;
    if (pMap == NULL || pulValue == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (MsGetSymbol(pSeg, ulSymIdx, &pSym) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    *pulValue = pSym->ulValue;
    return NO_ERROR;
}

/*!
 * @brief Query the name of a symbol.
 *
 * @param[in]  hMap     Handle. Not NULLHANDLE.
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
APIRET APIENTRY MapQuerySymbolName(HMAP hMap, ULONG ulSegIdx,
                                   ULONG ulSymIdx, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    MS_SYMBOL *pSym;
    if (pMap == NULL) return ERROR_INVALID_HANDLE;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    if (MsGetSymbol(pSeg, ulSymIdx, &pSym) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    return MsStringOut(pSym->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Find a symbol by its exact value.
 *
 * @param[in]  hMap       Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx   Zero-based segment index.
 * @param[in]  ulValue    Value to look for.
 * @param[out] pulSymIdx  Receives the symbol index. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSymIdx is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Not found or ulSegIdx out of range.
 */
APIRET APIENTRY MapFindSymbolByValue(HMAP hMap, ULONG ulSegIdx,
                                     ULONG ulValue, PULONG pulSymIdx)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_SEGMENT *pSeg;
    ULONG i, count;
    if (pMap == NULL || pulSymIdx == NULL) return ERROR_INVALID_PARAMETER;
    if (MsGetSegment(pMap, ulSegIdx, &pSeg) != NO_ERROR)
        return ERROR_FILE_NOT_FOUND;
    VectorGetCount(pSeg->hSymbols, &count);
    for (i = 0; i < count; i++) {
        MS_SYMBOL *pSym = NULL;
        VectorGetItem(pSeg->hSymbols, i, &pSym, sizeof(pSym), NULL);
        if (pSym != NULL && pSym->ulValue == ulValue) {
            *pulSymIdx = i;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Find a symbol by name across all segments.
 *
 * @param[in]  hMap       Handle. Not NULLHANDLE.
 * @param[in]  pszName    Name to look for. Not NULL.
 * @param[out] pulSegIdx  Optional. Receives the segment index.
 * @param[out] pulSymIdx  Optional. Receives the symbol index.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pszName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Not found.
 */
APIRET APIENTRY MapFindSymbolByName(HMAP hMap, PCSZ pszName,
                                    PULONG pulSegIdx, PULONG pulSymIdx)
{
    MS_HANDLE *pMap = MsGet(hMap);
    ULONG i, j, segCount, symCount;
    if (pMap == NULL || pszName == NULL) return ERROR_INVALID_PARAMETER;
    VectorGetCount(pMap->hSegments, &segCount);
    for (i = 0; i < segCount; i++) {
        MS_SEGMENT *pSeg = NULL;
        VectorGetItem(pMap->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        VectorGetCount(pSeg->hSymbols, &symCount);
        for (j = 0; j < symCount; j++) {
            MS_SYMBOL *pSym = NULL;
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
 * @param[in]  hMap         Handle. Not NULLHANDLE.
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
APIRET APIENTRY MapFindFirstSymbol(HMAP hMap, HMAPFIND *phFind,
                                   PULONG pulSegIdx, PULONG pulSymIdx,
                                   PSZ pszName, ULONG ulNameSize,
                                   PULONG pulNameUsed, PULONG pulValue)
{
    MS_HANDLE *pMap = MsGet(hMap);
    MS_FIND *pFind;
    ULONG i, segCount;
    APIRET rc;

    if (pMap == NULL || phFind == NULL || pulSegIdx == NULL ||
        pulSymIdx == NULL || pszName == NULL || pulValue == NULL)
        return ERROR_INVALID_PARAMETER;
    *phFind = NULLHANDLE;

    VectorGetCount(pMap->hSegments, &segCount);
    for (i = 0; i < segCount; i++) {
        MS_SEGMENT *pSeg = NULL;
        ULONG cnt = 0;
        VectorGetItem(pMap->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &cnt);
        if (cnt > 0) break;
    }
    if (i >= segCount) return ERROR_NO_MORE_ITEMS;

    {
        MS_SEGMENT *pSeg = NULL;
        MS_SYMBOL *pSym = NULL;
        VectorGetItem(pMap->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        VectorGetItem(pSeg->hSymbols, 0, &pSym, sizeof(pSym), NULL);
        if (pSym == NULL) return ERROR_NO_MORE_ITEMS;

        pFind = (MS_FIND *)calloc(1, sizeof(MS_FIND));
        if (pFind == NULL) return ERROR_NOT_ENOUGH_MEMORY;
        pFind->ulMagic  = MS_FIND_MAGIC;
        pFind->pMap     = pMap;
        pFind->ulSegIdx = i;
        pFind->ulSymIdx = 0;
        *phFind = (HMAPFIND)pFind;

        rc = MsStringOut(pSym->pszName, pszName, ulNameSize, pulNameUsed);
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
APIRET APIENTRY MapFindNextSymbol(HMAPFIND hFind,
                                  PULONG pulSegIdx, PULONG pulSymIdx,
                                  PSZ pszName, ULONG ulNameSize,
                                  PULONG pulNameUsed, PULONG pulValue)
{
    MS_FIND *pFind = MsFindGet(hFind);
    MS_HANDLE *pMap;
    ULONG segCount;
    APIRET rc;

    if (pFind == NULL || pulSegIdx == NULL || pulSymIdx == NULL ||
        pszName == NULL || pulValue == NULL)
        return ERROR_INVALID_PARAMETER;
    pMap = pFind->pMap;
    VectorGetCount(pMap->hSegments, &segCount);

    pFind->ulSymIdx++;
    while (pFind->ulSegIdx < segCount) {
        MS_SEGMENT *pSeg = NULL;
        ULONG cnt = 0;
        VectorGetItem(pMap->hSegments, pFind->ulSegIdx, &pSeg,
                      sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &cnt);
        if (pFind->ulSymIdx < cnt) break;
        pFind->ulSegIdx++;
        pFind->ulSymIdx = 0;
    }
    if (pFind->ulSegIdx >= segCount) return ERROR_NO_MORE_ITEMS;

    {
        MS_SEGMENT *pSeg = NULL;
        MS_SYMBOL *pSym = NULL;
        VectorGetItem(pMap->hSegments, pFind->ulSegIdx, &pSeg,
                      sizeof(pSeg), NULL);
        VectorGetItem(pSeg->hSymbols, pFind->ulSymIdx, &pSym,
                      sizeof(pSym), NULL);
        if (pSym == NULL) return ERROR_NO_MORE_ITEMS;
        rc = MsStringOut(pSym->pszName, pszName, ulNameSize, pulNameUsed);
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
APIRET APIENTRY MapFindClose(HMAPFIND hFind)
{
    MS_FIND *pFind = MsFindGet(hFind);
    if (pFind == NULL) {
        if (hFind == NULLHANDLE) return NO_ERROR;
        return ERROR_INVALID_HANDLE;
    }
    pFind->ulMagic = 0;
    free(pFind);
    return NO_ERROR;
}
