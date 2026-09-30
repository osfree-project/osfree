/*!
 * @file  sym.c
 * @brief MAPSYM (.SYM) file reader and writer implementation.
 *
 * All on-disk structures are packed to 1-byte alignment because the
 * .SYM format has no padding. Uses ccl (HVECTOR) for storage.
 * On host builds the OS/2 error codes are provided by os2err.h
 * directly; INCL_DOSERRORS must NOT be defined here.
 */
#include "sym.h"
#include "ccl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#pragma pack(push, 1)

/*!
 * @brief On-disk MAPDEF header (16 bytes).
 */
typedef struct {
    USHORT ppNextMap;    /**< @brief Paragraph of the next map.      */
    UCHAR  bFlags;       /**< @brief Module flags.                   */
    UCHAR  bReserved1;   /**< @brief Reserved.                       */
    USHORT pSegEntry;    /**< @brief Entry point (segment).          */
    USHORT cConsts;      /**< @brief Number of constants.            */
    USHORT pConstDef;    /**< @brief Offset of constant table.       */
    USHORT cSegs;        /**< @brief Number of segments.             */
    USHORT ppSegDef;     /**< @brief Paragraph of first SEGDEF.      */
    UCHAR  cbMaxSym;     /**< @brief Longest symbol name length.     */
    UCHAR  cbModName;    /**< @brief Length of the module name.      */
} MAPDEF_DISK;

/*!
 * @brief On-disk SEGDEF record (21 bytes).
 */
typedef struct {
    USHORT ppNextSeg;    /**< @brief Paragraph of the next SEGDEF.   */
    USHORT cSymbols;     /**< @brief Number of symbols.              */
    USHORT pSymDef;      /**< @brief Offset of the symbol pointer.   */
    USHORT wReserved1;   /**< @brief Reserved.                       */
    USHORT wReserved2;   /**< @brief Reserved.                       */
    USHORT wReserved3;   /**< @brief Reserved.                       */
    USHORT wReserved4;   /**< @brief Reserved.                       */
    UCHAR  bFlags;       /**< @brief SYM_SEGDEF_* flags.             */
    UCHAR  bReserved1;   /**< @brief Reserved.                       */
    USHORT ppLineDef;    /**< @brief Offset of line-number data.     */
    UCHAR  bReserved2;   /**< @brief Reserved.                       */
    UCHAR  bReserved3;   /**< @brief Reserved.                       */
    UCHAR  cbSegName;    /**< @brief Length of the segment name.     */
} SEGDEF_DISK;

/*!
 * @brief On-disk 16-bit SYMDEF (3 bytes).
 */
typedef struct {
    USHORT wSymVal;      /**< @brief 16-bit symbol value.            */
    UCHAR  cbSymName;    /**< @brief Length of the symbol name.      */
} SYMDEF16_DISK;

/*!
 * @brief On-disk 32-bit SYMDEF (5 bytes).
 */
typedef struct {
    ULONG  dwSymVal;     /**< @brief 32-bit symbol value.            */
    UCHAR  cbSymName;    /**< @brief Length of the symbol name.      */
} SYMDEF32_DISK;

#pragma pack(pop)

/*!
 * @brief One in-memory symbol.
 */
typedef struct SYM_SYMBOL {
    ULONG ulValue;       /**< @brief Symbol value.                   */
    PSZ   pszName;       /**< @brief Owned copy of the symbol name.  */
} SYM_SYMBOL;

/*!
 * @brief One in-memory segment.
 */
typedef struct SYM_SEGMENT {
    PSZ     pszName;     /**< @brief Owned segment name.             */
    ULONG   ulFlags;     /**< @brief SYM_SEGDEF_* flags.             */
    HVECTOR hSymbols;    /**< @brief Vector of SYM_SYMBOL*.          */
} SYM_SEGMENT;

/*!
 * @brief Private state of an open .SYM handle.
 */
typedef struct {
    ULONG   ulMagic;     /**< @brief SYM_MAGIC_OPEN when valid.      */
    ULONG   flOpen;      /**< @brief SYM_OPEN_* flags.               */
    PSZ     pszPath;     /**< @brief Owned output path.              */
    PSZ     pszModule;   /**< @brief Owned module name or NULL.      */
    ULONG   ulModFlags;  /**< @brief Module flags.                   */
    HVECTOR hSegments;   /**< @brief Vector of SYM_SEGMENT*.         */
    int     fWritten;    /**< @brief True after SymFlush succeeded.  */
    FILE   *pFile;       /**< @brief Reserved; not currently used.   */
} SYM_HANDLE;

/*!
 * @brief Enumeration cursor over .SYM symbols.
 */
typedef struct {
    ULONG       ulMagic;   /**< @brief SYM_ENUM_MAGIC when valid.   */
    SYM_HANDLE *pSym;      /**< @brief Owning handle.               */
    ULONG       ulSegIdx;  /**< @brief Current segment index.       */
    ULONG       ulSymIdx;  /**< @brief Current symbol index.        */
} SYM_ENUM;

/*!
 * @def SYM_MAGIC_OPEN
 * @brief Magic value marking an open SYM_HANDLE.
 */
#define SYM_MAGIC_OPEN   0x53594D31UL

/*!
 * @def SYM_MAGIC_CLOSED
 * @brief Magic value marking a closed SYM_HANDLE.
 */
#define SYM_MAGIC_CLOSED 0x53594D30UL

/*!
 * @def SYM_ENUM_MAGIC
 * @brief Magic value marking a valid SYM_ENUM cursor.
 */
#define SYM_ENUM_MAGIC   0x53454E31UL

/*!
 * @brief Validate a .SYM handle and cast it to the private struct.
 *
 * @param[in] hSym  Public handle to validate.
 * @return Pointer to the private SYM_HANDLE.
 * @retval NULL  hSym is NULL or has the wrong magic.
 */
static SYM_HANDLE *SymGet(HSYMFILE hSym)
{
    SYM_HANDLE *pSym = (SYM_HANDLE *)hSym;
    if (pSym == NULL || pSym->ulMagic != SYM_MAGIC_OPEN)
        return NULL;
    return pSym;
}

/*!
 * @brief Validate an enumeration cursor and cast it.
 *
 * @param[in] hEnum  Public cursor to validate.
 * @return Pointer to the private SYM_ENUM.
 * @retval NULL  hEnum is NULL or has the wrong magic.
 */
static SYM_ENUM *SymEnumGet(HSYMFIND hEnum)
{
    SYM_ENUM *pEnum = (SYM_ENUM *)hEnum;
    if (pEnum == NULL || pEnum->ulMagic != SYM_ENUM_MAGIC)
        return NULL;
    return pEnum;
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
static APIRET SymStringOut(PCSZ pszSrc, PSZ pszBuf,
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
 * @brief Read a fixed-length string from a .SYM file.
 *
 * @param[in] pFile  File to read from. Not NULL.
 * @param[in] ulLen  Length of the string, excluding the NUL.
 * @return Owned NUL-terminated copy.
 * @retval NULL   ulLen was zero or read/alloc failed.
 */
static PSZ SymReadString(FILE *pFile, ULONG ulLen)
{
    PSZ psz;
    if (ulLen == 0) return NULL;
    psz = (PSZ)malloc(ulLen + 1);
    if (psz == NULL) return NULL;
    if (fread(psz, 1, ulLen, pFile) != ulLen) {
        free(psz);
        return NULL;
    }
    psz[ulLen] = '\0';
    return psz;
}

/*!
 * @brief Fetch the segment pointer at a given index.
 *
 * @param[in]  pSym   Validated .SYM handle.
 * @param[in]  ulIdx  Zero-based segment index.
 * @param[out] ppSeg  Receives the pointer. Set to NULL on error.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulIdx out of range.
 */
static APIRET SymGetSegment(SYM_HANDLE *pSym, ULONG ulIdx,
                            SYM_SEGMENT **ppSeg)
{
    if (VectorGetItem(pSym->hSegments, ulIdx, ppSeg,
                      sizeof(*ppSeg), NULL) != NO_ERROR) {
        *ppSeg = NULL;
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    }
    return NO_ERROR;
}

/*!
 * @brief Fetch the symbol pointer at (segment, symbol) index.
 *
 * @param[in]  pSeg    Validated segment.
 * @param[in]  ulIdx   Zero-based symbol index.
 * @param[out] ppSym   Receives the pointer. Set to NULL on error.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_NO_MORE_ITEMS    ulIdx out of range.
 */
static APIRET SymGetSymbol(SYM_SEGMENT *pSeg, ULONG ulIdx,
                           SYM_SYMBOL **ppSym)
{
    if (VectorGetItem(pSeg->hSymbols, ulIdx, ppSym,
                      sizeof(*ppSym), NULL) != NO_ERROR) {
        *ppSym = NULL;
        return ERROR_NO_MORE_ITEMS;
    }
    return NO_ERROR;
}

/*!
 * @brief Release all segment and symbol storage of a handle.
 *
 * @param[in,out] pSym  Handle whose storage should be freed.
 */
static void SymFreeSegments(SYM_HANDLE *pSym)
{
    ULONG i, count;
    if (pSym->hSegments == NULLHANDLE) return;
    VectorGetCount(pSym->hSegments, &count);
    for (i = 0; i < count; i++) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG j, scount;
        if (VectorGetItem(pSym->hSegments, i, &pSeg,
                          sizeof(pSeg), NULL) != NO_ERROR)
            continue;
        if (pSeg == NULL) continue;
        if (pSeg->pszName) free(pSeg->pszName);
        if (pSeg->hSymbols != NULLHANDLE) {
            VectorGetCount(pSeg->hSymbols, &scount);
            for (j = 0; j < scount; j++) {
                SYM_SYMBOL *pS = NULL;
                if (VectorGetItem(pSeg->hSymbols, j, &pS,
                                  sizeof(pS), NULL) != NO_ERROR)
                    continue;
                if (pS) {
                    if (pS->pszName) free(pS->pszName);
                    free(pS);
                }
            }
            VectorDestroy(pSeg->hSymbols);
        }
        free(pSeg);
    }
    VectorDestroy(pSym->hSegments);
    pSym->hSegments = NULLHANDLE;
}

/*!
 * @brief Free a private handle's storage and mark it closed.
 *
 * @param[in,out] pSym  Handle to destroy. NULL is accepted.
 */
static void SymCloseHandle(SYM_HANDLE *pSym)
{
    if (pSym == NULL) return;
    if (pSym->pFile) fclose(pSym->pFile);
    if (pSym->pszModule) free(pSym->pszModule);
    if (pSym->pszPath) free(pSym->pszPath);
    SymFreeSegments(pSym);
    pSym->ulMagic = SYM_MAGIC_CLOSED;
    free(pSym);
}

/*!
 * @brief Read and validate a MAPDEF header plus module name.
 *
 * @param[in]  pFile        Open .SYM file.
 * @param[out] pMapDef      Receives the raw MAPDEF.
 * @param[out] ppszModule   Receives the owned module name.
 * @param[out] pulModFlags  Receives the module flags.
 * @return APIRET.
 * @retval NO_ERROR                    Success.
 * @retval SYM_ERROR_INVALID_SYNTAX    Malformed header.
 * @retval ERROR_NOT_ENOUGH_MEMORY     Allocation failed.
 */
static APIRET SymReadMapDef(FILE *pFile, MAPDEF_DISK *pMapDef,
                            PSZ *ppszModule, ULONG *pulModFlags)
{
    ULONG i;

    if (fread(pMapDef, sizeof(MAPDEF_DISK), 1, pFile) != 1)
        return SYM_ERROR_INVALID_SYNTAX;
    if (pMapDef->cSegs == 0 ||
        pMapDef->cSegs > SYM_MAX_SEGMENTS)
        return SYM_ERROR_INVALID_SYNTAX;
    if (pMapDef->ppSegDef == 0 || pMapDef->cbModName == 0)
        return SYM_ERROR_INVALID_SYNTAX;
    if (pMapDef->cbModName > SYM_MAX_MOD_NAME)
        return SYM_ERROR_INVALID_SYNTAX;

    *ppszModule = SymReadString(pFile, pMapDef->cbModName);
    if (*ppszModule == NULL)
        return SYM_ERROR_INVALID_SYNTAX;
    for (i = 0; i < pMapDef->cbModName; i++) {
        UCHAR ch = (UCHAR)(*ppszModule)[i];
        if (ch < 0x20 || ch >= 0x7F) {
            free(*ppszModule);
            *ppszModule = NULL;
            return SYM_ERROR_INVALID_SYNTAX;
        }
    }
    *pulModFlags = pMapDef->bFlags;
    return NO_ERROR;
}

/*!
 * @brief Read the symbol pointer array and each SYMDEF.
 *
 * @param[in]     pFile         Open .SYM file.
 * @param[in,out] pSeg          Segment being populated.
 * @param[in]     ulSegOffset   File offset of the SEGDEF record.
 * @param[in]     pSymDef       Offset of the pointer array.
 * @param[in]     cSymbols      Number of symbols.
 * @param[in]     f32Bit        Non-zero for 32-bit SYMDEFs.
 * @return APIRET.
 * @retval NO_ERROR                    Success.
 * @retval SYM_ERROR_INVALID_SYNTAX    Malformed data.
 * @retval SYM_ERROR_TOO_MANY_SYMBOLS  cSymbols above format limit.
 * @retval ERROR_READ_FAULT            Seek failure.
 * @retval ERROR_NOT_ENOUGH_MEMORY     Allocation failed.
 */
static APIRET SymReadSymbols(FILE *pFile, SYM_SEGMENT *pSeg,
                             ULONG ulSegOffset, USHORT pSymDef,
                             USHORT cSymbols, int f32Bit)
{
    ULONG i;
    USHORT *paSymPtr;

    if (cSymbols == 0) return NO_ERROR;
    if (cSymbols > SYM_MAX_SYMBOLS_PER_SEG)
        return SYM_ERROR_TOO_MANY_SYMBOLS;

    if (fseek(pFile, (long)ulSegOffset + (long)pSymDef, SEEK_SET) != 0)
        return ERROR_READ_FAULT;

    paSymPtr = (USHORT *)malloc(cSymbols * sizeof(USHORT));
    if (paSymPtr == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    if (fread(paSymPtr, sizeof(USHORT), cSymbols, pFile) != cSymbols) {
        free(paSymPtr);
        return SYM_ERROR_INVALID_SYNTAX;
    }

    for (i = 0; i < cSymbols; i++) {
        ULONG ulSymOffset = ulSegOffset + paSymPtr[i];
        SYM_SYMBOL *pSym;

        pSym = (SYM_SYMBOL *)calloc(1, sizeof(*pSym));
        if (pSym == NULL) { free(paSymPtr); return ERROR_NOT_ENOUGH_MEMORY; }

        if (fseek(pFile, (long)ulSymOffset, SEEK_SET) != 0) {
            free(pSym); free(paSymPtr);
            return ERROR_READ_FAULT;
        }
        if (f32Bit) {
            SYMDEF32_DISK s32;
            if (fread(&s32, sizeof(SYMDEF32_DISK), 1, pFile) != 1) {
                free(pSym); free(paSymPtr);
                return SYM_ERROR_INVALID_SYNTAX;
            }
            pSym->ulValue = s32.dwSymVal;
            pSym->pszName = SymReadString(pFile, s32.cbSymName);
        } else {
            SYMDEF16_DISK s16;
            if (fread(&s16, sizeof(SYMDEF16_DISK), 1, pFile) != 1) {
                free(pSym); free(paSymPtr);
                return SYM_ERROR_INVALID_SYNTAX;
            }
            pSym->ulValue = (ULONG)s16.wSymVal;
            pSym->pszName = SymReadString(pFile, s16.cbSymName);
        }
        if (pSym->pszName == NULL) {
            free(pSym); free(paSymPtr);
            return SYM_ERROR_INVALID_SYNTAX;
        }
        if (VectorAdd(pSeg->hSymbols, &pSym) != NO_ERROR) {
            free(pSym->pszName); free(pSym); free(paSymPtr);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    free(paSymPtr);
    return NO_ERROR;
}

/*!
 * @brief Load a complete .SYM file into the given handle.
 *
 * @param[in,out] pSym   Handle being populated.
 * @param[in]     pFile  Open file, positioned at offset 0.
 * @return APIRET.
 * @retval NO_ERROR                    Success.
 * @retval SYM_ERROR_INVALID_SYNTAX    Malformed file.
 * @retval ERROR_READ_FAULT            Seek failure.
 * @retval ERROR_NOT_ENOUGH_MEMORY     Allocation failed.
 */
static APIRET SymOpenReader(SYM_HANDLE *pSym, FILE *pFile)
{
    MAPDEF_DISK mapDef;
    APIRET rc = NO_ERROR;
    ULONG ulSegPara, i;
    long lFileSize;

    if (fseek(pFile, 0, SEEK_END) != 0) return ERROR_READ_FAULT;
    lFileSize = ftell(pFile);
    if (lFileSize < 0) return ERROR_READ_FAULT;
    if (fseek(pFile, 0, SEEK_SET) != 0) return ERROR_READ_FAULT;

    rc = SymReadMapDef(pFile, &mapDef, &pSym->pszModule,
                       &pSym->ulModFlags);
    if (rc != NO_ERROR) return rc;

    ulSegPara = mapDef.ppSegDef;

    for (i = 0; i < mapDef.cSegs; i++) {
        long lSegOffset = (long)ulSegPara * 16L;
        SYM_SEGMENT *pSeg;
        SEGDEF_DISK segDef;
        ULONG ulNextPara;

        if (lSegOffset >= lFileSize) { rc = SYM_ERROR_INVALID_SYNTAX; break; }
        if (fseek(pFile, lSegOffset, SEEK_SET) != 0) {
            rc = ERROR_READ_FAULT; break;
        }
        if (fread(&segDef, sizeof(SEGDEF_DISK), 1, pFile) != 1) {
            rc = SYM_ERROR_INVALID_SYNTAX; break;
        }

        pSeg = (SYM_SEGMENT *)calloc(1, sizeof(*pSeg));
        if (pSeg == NULL) { rc = ERROR_NOT_ENOUGH_MEMORY; break; }
        pSeg->ulFlags = segDef.bFlags;
        if (VectorCreate(sizeof(SYM_SYMBOL *), &pSeg->hSymbols) != NO_ERROR) {
            free(pSeg); rc = ERROR_NOT_ENOUGH_MEMORY; break;
        }
        /* NOTE: cbSegName is UCHAR (max 255) and SYM_MAX_SEG_NAME is
         * 255, so an explicit upper-bound check would be dead code. */
        pSeg->pszName = SymReadString(pFile, segDef.cbSegName);
        if (pSeg->pszName == NULL && segDef.cbSegName != 0) {
            VectorDestroy(pSeg->hSymbols); free(pSeg);
            rc = SYM_ERROR_INVALID_SYNTAX; break;
        }
        ulNextPara = segDef.ppNextSeg;

        if (segDef.cSymbols > 0) {
            int f32Bit = (segDef.bFlags & SYM_SEGDEF_32BIT) != 0;
            rc = SymReadSymbols(pFile, pSeg, (ULONG)lSegOffset,
                                segDef.pSymDef, segDef.cSymbols, f32Bit);
            if (rc != NO_ERROR) {
                if (pSeg->pszName) free(pSeg->pszName);
                VectorDestroy(pSeg->hSymbols);
                free(pSeg);
                break;
            }
        }
        if (VectorAdd(pSym->hSegments, &pSeg) != NO_ERROR) {
            if (pSeg->pszName) free(pSeg->pszName);
            VectorDestroy(pSeg->hSymbols);
            free(pSeg);
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        ulSegPara = ulNextPara;
    }
    return rc;
}

/*!
 * @brief Serialise the in-memory model into a .SYM file.
 *
 * @param[in] pSym  Handle to write. Must have pszModule set.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Module name is missing or bad.
 * @retval ERROR_OPEN_FAILED        Output file cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_WRITE_FAULT        fwrite failed.
 */
static APIRET SymWriteFile(SYM_HANDLE *pSym)
{
    FILE *pFile;
    ULONG i, j;
    ULONG ulTotalSize;
    USHORT *pSegPara, *pSegNextPara;
    ULONG *pSegSymPtrOff;
    ULONG ulMaxSymLen, ulMaxSegNameLen, ulModNameLen;
    UCHAR *pbImage;
    ULONG ulSegCount;

    if (pSym->pszModule == NULL) return ERROR_INVALID_PARAMETER;
    ulModNameLen = (ULONG)strlen(pSym->pszModule);
    if (ulModNameLen == 0 || ulModNameLen > SYM_MAX_MOD_NAME)
        return ERROR_INVALID_PARAMETER;

    VectorGetCount(pSym->hSegments, &ulSegCount);
    ulMaxSymLen = 0;
    ulMaxSegNameLen = 0;
    for (i = 0; i < ulSegCount; i++) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG symCount = 0;
        VectorGetItem(pSym->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        {
            ULONG l = (ULONG)strlen(pSeg->pszName);
            if (l > ulMaxSegNameLen) ulMaxSegNameLen = l;
        }
        VectorGetCount(pSeg->hSymbols, &symCount);
        for (j = 0; j < symCount; j++) {
            SYM_SYMBOL *pS = NULL;
            ULONG l2;
            VectorGetItem(pSeg->hSymbols, j, &pS, sizeof(pS), NULL);
            if (pS == NULL) continue;
            l2 = (ULONG)strlen(pS->pszName);
            if (l2 > ulMaxSymLen) ulMaxSymLen = l2;
        }
    }

    ulTotalSize = (sizeof(MAPDEF_DISK) + ulModNameLen + 15) & ~15UL;

    pSegPara = (USHORT *)calloc(ulSegCount, sizeof(USHORT));
    pSegNextPara = (USHORT *)calloc(ulSegCount, sizeof(USHORT));
    pSegSymPtrOff = (ULONG *)calloc(ulSegCount, sizeof(ULONG));
    if (!pSegPara || !pSegNextPara || !pSegSymPtrOff) {
        free(pSegPara); free(pSegNextPara); free(pSegSymPtrOff);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for (i = 0; i < ulSegCount; i++) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG ulSegNameLen, ulSymPtrArraySize, ulSymDataSize, ulSegSize;
        ULONG symCount = 0;
        VectorGetItem(pSym->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        ulSegNameLen = (ULONG)strlen(pSeg->pszName);
        VectorGetCount(pSeg->hSymbols, &symCount);
        ulSymPtrArraySize = symCount * sizeof(USHORT);
        ulSymDataSize = 0;
        for (j = 0; j < symCount; j++) {
            SYM_SYMBOL *pS = NULL;
            ULONG l;
            VectorGetItem(pSeg->hSymbols, j, &pS, sizeof(pS), NULL);
            if (pS == NULL) continue;
            l = (ULONG)strlen(pS->pszName);
            if (pSeg->ulFlags & SYM_SEGDEF_32BIT)
                ulSymDataSize += sizeof(SYMDEF32_DISK) + l;
            else
                ulSymDataSize += sizeof(SYMDEF16_DISK) + l;
        }
        ulSegSize = (sizeof(SEGDEF_DISK) + ulSegNameLen + 1) & ~1UL;
        pSegSymPtrOff[i] = ulSegSize;
        ulSegSize += ulSymPtrArraySize + ulSymDataSize;
        ulSegSize = (ulSegSize + 15) & ~15UL;

        pSegPara[i] = (USHORT)(ulTotalSize / 16);
        ulTotalSize += ulSegSize;
    }

    for (i = 0; i < ulSegCount; i++) {
        ULONG n = (i + 1) % ulSegCount;
        pSegNextPara[i] = pSegPara[n];
    }
    ulTotalSize += 8;

    pbImage = (UCHAR *)calloc(1, ulTotalSize);
    if (pbImage == NULL) {
        free(pSegPara); free(pSegNextPara); free(pSegSymPtrOff);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    {
        MAPDEF_DISK *pMapDef = (MAPDEF_DISK *)pbImage;
        pMapDef->ppNextMap = (USHORT)(ulTotalSize / 16);
        pMapDef->bFlags = (UCHAR)pSym->ulModFlags;
        pMapDef->cSegs = (USHORT)ulSegCount;
        pMapDef->ppSegDef = ulSegCount ? pSegPara[0] : 0;
        pMapDef->cbMaxSym = (UCHAR)ulMaxSymLen;
        pMapDef->cbModName = (UCHAR)ulModNameLen;
        memcpy(pbImage + sizeof(MAPDEF_DISK), pSym->pszModule,
               ulModNameLen);
    }

    for (i = 0; i < ulSegCount; i++) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG ulSegOffset, ulSegNameLen, ulPos;
        UCHAR *pbSeg, *pbSymPtrArray;
        SEGDEF_DISK *pSegDef;
        ULONG symCount = 0;

        VectorGetItem(pSym->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        ulSegOffset = (ULONG)pSegPara[i] * 16;
        pbSeg = pbImage + ulSegOffset;
        pSegDef = (SEGDEF_DISK *)pbSeg;
        ulSegNameLen = (ULONG)strlen(pSeg->pszName);

        VectorGetCount(pSeg->hSymbols, &symCount);
        pSegDef->ppNextSeg = pSegNextPara[i];
        pSegDef->cSymbols = (USHORT)symCount;
        pSegDef->pSymDef = (USHORT)pSegSymPtrOff[i];
        pSegDef->wReserved1 = (USHORT)(i + 1);
        pSegDef->bFlags = (UCHAR)pSeg->ulFlags;
        pSegDef->cbSegName = (UCHAR)ulSegNameLen;
        memcpy(pbSeg + sizeof(SEGDEF_DISK), pSeg->pszName, ulSegNameLen);

        pbSymPtrArray = pbSeg + pSegSymPtrOff[i];
        ulPos = pSegSymPtrOff[i] + symCount * sizeof(USHORT);
        for (j = 0; j < symCount; j++) {
            SYM_SYMBOL *pS = NULL;
            ULONG l;
            UCHAR *pbSymData;
            USHORT uOff;
            VectorGetItem(pSeg->hSymbols, j, &pS, sizeof(pS), NULL);
            if (pS == NULL) continue;
            l = (ULONG)strlen(pS->pszName);
            pbSymData = pbSeg + ulPos;
            uOff = (USHORT)ulPos;
            memcpy(pbSymPtrArray + j * sizeof(USHORT), &uOff,
                   sizeof(USHORT));
            if (pSeg->ulFlags & SYM_SEGDEF_32BIT) {
                SYMDEF32_DISK d;
                d.dwSymVal = pS->ulValue;
                d.cbSymName = (UCHAR)l;
                memcpy(pbSymData, &d, sizeof(d));
                memcpy(pbSymData + sizeof(d), pS->pszName, l);
                ulPos += (ULONG)sizeof(d) + l;
            } else {
                SYMDEF16_DISK d;
                d.wSymVal = (USHORT)pS->ulValue;
                d.cbSymName = (UCHAR)l;
                memcpy(pbSymData, &d, sizeof(d));
                memcpy(pbSymData + sizeof(d), pS->pszName, l);
                ulPos += (ULONG)sizeof(d) + l;
            }
        }
    }

    {
        UCHAR *pbLast = pbImage + ulTotalSize - 8;
        pbLast[0] = 0;
        pbLast[1] = 0;
        pbLast[2] = 1;
        pbLast[3] = 5;
    }

    pFile = fopen(pSym->pszPath, "wb");
    if (pFile == NULL) {
        free(pbImage); free(pSegPara); free(pSegNextPara);
        free(pSegSymPtrOff);
        return ERROR_OPEN_FAILED;
    }
    if (fwrite(pbImage, 1, ulTotalSize, pFile) != ulTotalSize) {
        fclose(pFile);
        free(pbImage); free(pSegPara); free(pSegNextPara);
        free(pSegSymPtrOff);
        return ERROR_WRITE_FAULT;
    }
    fclose(pFile);

    free(pbImage); free(pSegPara); free(pSegNextPara);
    free(pSegSymPtrOff);
    return NO_ERROR;
}

/*!
 * @brief Open a .SYM file for reading or writing.
 *
 * @param[in]  pszPath  File path. Not NULL.
 * @param[out] phSym    Receives the handle. Not NULL.
 * @param[in]  flOpen   SYM_OPEN_* combination.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments or flags.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SYM_ERROR_INVALID_SYNTAX Malformed file.
 */
APIRET APIENTRY SymOpen(PCSZ pszPath, HSYMFILE *phSym, ULONG flOpen)
{
    SYM_HANDLE *pSym;
    FILE *pFile;
    APIRET rc;
    ULONG flAccess;

    if (pszPath == NULL || phSym == NULL) return ERROR_INVALID_PARAMETER;
    *phSym = NULLHANDLE;

    flAccess = flOpen & (SYM_OPEN_READ | SYM_OPEN_WRITE);
    if (flAccess != SYM_OPEN_READ && flAccess != SYM_OPEN_WRITE)
        return ERROR_INVALID_PARAMETER;

    pSym = (SYM_HANDLE *)calloc(1, sizeof(SYM_HANDLE));
    if (pSym == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pSym->ulMagic = SYM_MAGIC_OPEN;
    pSym->flOpen = flOpen;
    pSym->pszPath = (PSZ)malloc(strlen(pszPath) + 1);
    if (pSym->pszPath == NULL) {
        SymCloseHandle(pSym);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    strcpy(pSym->pszPath, pszPath);

    if (VectorCreate(sizeof(SYM_SEGMENT *), &pSym->hSegments) != NO_ERROR) {
        SymCloseHandle(pSym);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (flAccess == SYM_OPEN_READ) {
        pFile = fopen(pszPath, "rb");
        if (pFile == NULL) {
            SymCloseHandle(pSym);
            return ERROR_OPEN_FAILED;
        }
        rc = SymOpenReader(pSym, pFile);
        fclose(pFile);
        if (rc != NO_ERROR) {
            SymCloseHandle(pSym);
            return rc;
        }
    } else {
        if ((flOpen & (SYM_OPEN_CREATE | SYM_OPEN_TRUNCATE)) == 0) {
            SymCloseHandle(pSym);
            return ERROR_INVALID_PARAMETER;
        }
    }

    *phSym = (HSYMFILE)pSym;
    return NO_ERROR;
}

/*!
 * @brief Close a .SYM handle.
 *
 * @param[in] hSym  Handle. NULLHANDLE is a no-op.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY SymClose(HSYMFILE hSym)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    APIRET rc = NO_ERROR;
    ULONG ulSegCount = 0;

    if (pSym == NULL) {
        if (hSym == NULLHANDLE) return NO_ERROR;
        return ERROR_INVALID_HANDLE;
    }
    VectorGetCount(pSym->hSegments, &ulSegCount);
    if ((pSym->flOpen & SYM_OPEN_WRITE) != 0 &&
        pSym->pszModule != NULL && ulSegCount > 0 && !pSym->fWritten) {
        rc = SymWriteFile(pSym);
    }
    SymCloseHandle(pSym);
    return rc;
}

/*!
 * @brief Query the module name.
 *
 * @param[in]  hSym     Handle. Not NULLHANDLE.
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
APIRET APIENTRY SymQueryModule(HSYMFILE hSym, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    if (pSym == NULL) return ERROR_INVALID_HANDLE;
    if (pSym->pszModule == NULL) return ERROR_FILE_NOT_FOUND;
    return SymStringOut(pSym->pszModule, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the number of segments.
 *
 * @param[in]  hSym      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY SymQuerySegmentCount(HSYMFILE hSym, PULONG pulCount)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    if (pSym == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    return VectorGetCount(pSym->hSegments, pulCount);
}

/*!
 * @brief Query the total number of symbols across all segments.
 *
 * @param[in]  hSym      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY SymQuerySymbolCount(HSYMFILE hSym, PULONG pulCount)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    ULONG i, count, total = 0;
    if (pSym == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    VectorGetCount(pSym->hSegments, &count);
    for (i = 0; i < count; i++) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG s = 0;
        VectorGetItem(pSym->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &s);
        total += s;
    }
    *pulCount = total;
    return NO_ERROR;
}

/*!
 * @brief Query the name of a segment.
 *
 * @param[in]  hSym     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 * @retval ERROR_INVALID_PARAMETER      pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW        Buffer too small.
 */
APIRET APIENTRY SymQuerySegmentName(HSYMFILE hSym, ULONG ulSegIdx,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    if (pSym == NULL) return ERROR_INVALID_HANDLE;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    return SymStringOut(pSeg->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the flags of a segment (SYM_SEGDEF_*).
 *
 * @param[in]  hSym      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[out] pulFlags  Receives the flags. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER      pulFlags is NULL.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 */
APIRET APIENTRY SymQuerySegmentFlags(HSYMFILE hSym, ULONG ulSegIdx,
                                     PULONG pulFlags)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    if (pSym == NULL || pulFlags == NULL) return ERROR_INVALID_PARAMETER;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    *pulFlags = pSeg->ulFlags;
    return NO_ERROR;
}

/*!
 * @brief Query the number of symbols in one segment.
 *
 * @param[in]  hSym      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[out] pulCount  Receives the count. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER      pulCount is NULL.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 */
APIRET APIENTRY SymQuerySegmentSymbolCount(HSYMFILE hSym, ULONG ulSegIdx,
                                           PULONG pulCount)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    if (pSym == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    return VectorGetCount(pSeg->hSymbols, pulCount);
}

/*!
 * @brief Query the value of a symbol.
 *
 * @param[in]  hSym      Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx  Zero-based segment index.
 * @param[in]  ulSymIdx  Zero-based symbol index.
 * @param[out] pulValue  Receives the value. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER      pulValue is NULL.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 * @retval ERROR_NO_MORE_ITEMS          ulSymIdx out of range.
 */
APIRET APIENTRY SymQuerySymbolValue(HSYMFILE hSym, ULONG ulSegIdx,
                                    ULONG ulSymIdx, PULONG pulValue)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    SYM_SYMBOL *pS;
    if (pSym == NULL || pulValue == NULL) return ERROR_INVALID_PARAMETER;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    if (SymGetSymbol(pSeg, ulSymIdx, &pS) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    *pulValue = pS->ulValue;
    return NO_ERROR;
}

/*!
 * @brief Query the name of a symbol.
 *
 * @param[in]  hSym     Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx Zero-based segment index.
 * @param[in]  ulSymIdx Zero-based symbol index.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 * @retval ERROR_NO_MORE_ITEMS          ulSymIdx out of range.
 * @retval ERROR_INVALID_PARAMETER      pszBuf NULL without size query.
 * @retval ERROR_BUFFER_OVERFLOW        Buffer too small.
 */
APIRET APIENTRY SymQuerySymbolName(HSYMFILE hSym, ULONG ulSegIdx,
                                   ULONG ulSymIdx, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    SYM_SYMBOL *pS;
    if (pSym == NULL) return ERROR_INVALID_HANDLE;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    if (SymGetSymbol(pSeg, ulSymIdx, &pS) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    return SymStringOut(pS->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Find a symbol by its exact value.
 *
 * @param[in]  hSym       Handle. Not NULLHANDLE.
 * @param[in]  ulSegIdx   Zero-based segment index.
 * @param[in]  ulValue    Value to look for.
 * @param[out] pulSymIdx  Receives the symbol index. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER      pulSymIdx is NULL.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 * @retval ERROR_FILE_NOT_FOUND         No such value.
 */
APIRET APIENTRY SymFindSymbolByValue(HSYMFILE hSym, ULONG ulSegIdx,
                                     ULONG ulValue, PULONG pulSymIdx)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    ULONG i, count;
    if (pSym == NULL || pulSymIdx == NULL) return ERROR_INVALID_PARAMETER;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;
    VectorGetCount(pSeg->hSymbols, &count);
    for (i = 0; i < count; i++) {
        SYM_SYMBOL *pS = NULL;
        VectorGetItem(pSeg->hSymbols, i, &pS, sizeof(pS), NULL);
        if (pS != NULL && pS->ulValue == ulValue) {
            *pulSymIdx = i;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Find a symbol by name across all segments.
 *
 * @param[in]  hSym       Handle. Not NULLHANDLE.
 * @param[in]  pszName    Name to look for. Not NULL.
 * @param[out] pulSegIdx  Optional. Receives the segment index.
 * @param[out] pulSymIdx  Optional. Receives the symbol index.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pszName is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No such symbol.
 */
APIRET APIENTRY SymFindSymbolByName(HSYMFILE hSym, PCSZ pszName,
                                    PULONG pulSegIdx, PULONG pulSymIdx)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    ULONG i, j, segCount, symCount;
    if (pSym == NULL || pszName == NULL) return ERROR_INVALID_PARAMETER;
    VectorGetCount(pSym->hSegments, &segCount);
    for (i = 0; i < segCount; i++) {
        SYM_SEGMENT *pSeg = NULL;
        VectorGetItem(pSym->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg == NULL) continue;
        VectorGetCount(pSeg->hSymbols, &symCount);
        for (j = 0; j < symCount; j++) {
            SYM_SYMBOL *pS = NULL;
            VectorGetItem(pSeg->hSymbols, j, &pS, sizeof(pS), NULL);
            if (pS != NULL && strcmp(pS->pszName, pszName) == 0) {
                if (pulSegIdx) *pulSegIdx = i;
                if (pulSymIdx) *pulSymIdx = j;
                return NO_ERROR;
            }
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Open an enumeration cursor over the symbols of a handle.
 *
 * @param[in]  hSym    Handle. Not NULLHANDLE.
 * @param[out] phEnum  Receives the cursor. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  phEnum is NULL.
 * @retval ERROR_NO_MORE_ITEMS      There are no symbols.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SymEnumFirst(HSYMFILE hSym, HSYMFIND *phEnum)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_ENUM *pEnum;
    ULONG i, segCount;
    if (pSym == NULL || phEnum == NULL) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;
    VectorGetCount(pSym->hSegments, &segCount);
    for (i = 0; i < segCount; i++) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG cnt = 0;
        VectorGetItem(pSym->hSegments, i, &pSeg, sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &cnt);
        if (cnt > 0) break;
    }
    if (i >= segCount) return ERROR_NO_MORE_ITEMS;
    pEnum = (SYM_ENUM *)calloc(1, sizeof(SYM_ENUM));
    if (pEnum == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->ulMagic = SYM_ENUM_MAGIC;
    pEnum->pSym = pSym;
    pEnum->ulSegIdx = i;
    pEnum->ulSymIdx = 0;
    *phEnum = (HSYMFIND)pEnum;
    return NO_ERROR;
}

/*!
 * @brief Advance the enumeration cursor to the next symbol.
 *
 * @param[in] hFind  Cursor. Not NULLHANDLE.
 * @return APIRET.
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Cursor is not recognized.
 * @retval ERROR_NO_MORE_ITEMS    Cursor already on the last symbol.
 */
APIRET APIENTRY SymEnumNext(HSYMFIND hFind)
{
    SYM_ENUM *pEnum = SymEnumGet(hFind);
    SYM_HANDLE *pSym;
    ULONG segCount;
    if (pEnum == NULL) return ERROR_INVALID_HANDLE;
    pSym = pEnum->pSym;
    VectorGetCount(pSym->hSegments, &segCount);
    pEnum->ulSymIdx++;
    while (pEnum->ulSegIdx < segCount) {
        SYM_SEGMENT *pSeg = NULL;
        ULONG cnt = 0;
        VectorGetItem(pSym->hSegments, pEnum->ulSegIdx, &pSeg,
                      sizeof(pSeg), NULL);
        if (pSeg != NULL) VectorGetCount(pSeg->hSymbols, &cnt);
        if (pEnum->ulSymIdx < cnt) return NO_ERROR;
        pEnum->ulSegIdx++;
        pEnum->ulSymIdx = 0;
    }
    return ERROR_NO_MORE_ITEMS;
}

/*!
 * @brief Retrieve the segment index at the cursor.
 *
 * @param[in]  hFind      Cursor. Not NULLHANDLE.
 * @param[out] pulSegIdx  Receives the segment index. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Cursor is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulSegIdx is NULL.
 */
APIRET APIENTRY SymEnumGetSegIdx(HSYMFIND hFind, PULONG pulSegIdx)
{
    SYM_ENUM *pEnum = SymEnumGet(hFind);
    if (pEnum == NULL || pulSegIdx == NULL) return ERROR_INVALID_PARAMETER;
    *pulSegIdx = pEnum->ulSegIdx;
    return NO_ERROR;
}

/*!
 * @brief Retrieve the symbol value at the cursor.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pulValue  Receives the value. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Cursor is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pulValue is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Segment disappeared.
 * @retval ERROR_NO_MORE_ITEMS      Symbol index out of range.
 */
APIRET APIENTRY SymEnumGetValue(HSYMFIND hFind, PULONG pulValue)
{
    SYM_ENUM *pEnum = SymEnumGet(hFind);
    SYM_SEGMENT *pSeg;
    SYM_SYMBOL *pS;
    if (pEnum == NULL || pulValue == NULL) return ERROR_INVALID_PARAMETER;
    VectorGetItem(pEnum->pSym->hSegments, pEnum->ulSegIdx, &pSeg,
                  sizeof(pSeg), NULL);
    if (pSeg == NULL) return ERROR_FILE_NOT_FOUND;
    if (SymGetSymbol(pSeg, pEnum->ulSymIdx, &pS) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    *pulValue = pS->ulValue;
    return NO_ERROR;
}

/*!
 * @brief Retrieve the symbol name at the cursor.
 *
 * @param[in]  hFind     Cursor. Not NULLHANDLE.
 * @param[out] pszBuf    Output buffer, or NULL for size query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. Receives required size.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Cursor is not recognized.
 * @retval ERROR_INVALID_PARAMETER  pszBuf NULL without size query.
 * @retval ERROR_FILE_NOT_FOUND     Segment disappeared.
 * @retval ERROR_NO_MORE_ITEMS      Symbol index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY SymEnumGetName(HSYMFIND hFind, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed)
{
    SYM_ENUM *pEnum = SymEnumGet(hFind);
    SYM_SEGMENT *pSeg;
    SYM_SYMBOL *pS;
    if (pEnum == NULL) return ERROR_INVALID_HANDLE;
    VectorGetItem(pEnum->pSym->hSegments, pEnum->ulSegIdx, &pSeg,
                  sizeof(pSeg), NULL);
    if (pSeg == NULL) return ERROR_FILE_NOT_FOUND;
    if (SymGetSymbol(pSeg, pEnum->ulSymIdx, &pS) != NO_ERROR)
        return ERROR_NO_MORE_ITEMS;
    return SymStringOut(pS->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Close an enumeration cursor.
 *
 * @param[in] hFind  Cursor. NULLHANDLE is a no-op.
 * @return APIRET.
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Cursor is not recognized.
 */
APIRET APIENTRY SymEnumClose(HSYMFIND hFind)
{
    SYM_ENUM *pEnum = SymEnumGet(hFind);
    if (pEnum == NULL) {
        if (hFind == NULLHANDLE) return NO_ERROR;
        return ERROR_INVALID_HANDLE;
    }
    pEnum->ulMagic = 0;
    free(pEnum);
    return NO_ERROR;
}

/*!
 * @brief Set the module name of a writable handle.
 *
 * @param[in] hSym       Handle opened for writing. Not NULLHANDLE.
 * @param[in] pszModule  Module name. Not NULL, 1..SYM_MAX_MOD_NAME.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle not opened for writing.
 * @retval ERROR_INVALID_PARAMETER  Bad module name.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SymSetModule(HSYMFILE hSym, PCSZ pszModule)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    ULONG ulLen;

    if (pSym == NULL || pszModule == NULL) return ERROR_INVALID_PARAMETER;
    if ((pSym->flOpen & SYM_OPEN_WRITE) == 0)
        return ERROR_INVALID_HANDLE;

    ulLen = (ULONG)strlen(pszModule);
    if (ulLen == 0 || ulLen > SYM_MAX_MOD_NAME)
        return ERROR_INVALID_PARAMETER;

    if (pSym->pszModule) free(pSym->pszModule);
    pSym->pszModule = (PSZ)malloc(ulLen + 1);
    if (pSym->pszModule == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    strcpy(pSym->pszModule, pszModule);
    pSym->fWritten = 0;
    return NO_ERROR;
}

/*!
 * @brief Append a new empty segment to a writable handle.
 *
 * @param[in]  hSym       Handle opened for writing. Not NULLHANDLE.
 * @param[in]  pszName    Segment name. Not NULL.
 * @param[in]  ulFlags    SYM_SEGDEF_* flags.
 * @param[out] pulSegIdx  Optional. Receives the new segment index.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle not opened for writing.
 * @retval ERROR_INVALID_PARAMETER      Bad name or no module set.
 * @retval SYM_ERROR_TOO_MANY_SEGMENTS  Segment limit reached.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Allocation failure.
 */
APIRET APIENTRY SymAddSegment(HSYMFILE hSym, PCSZ pszName,
                              ULONG ulFlags, PULONG pulSegIdx)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    ULONG ulLen, count;

    if (pSym == NULL || pszName == NULL) return ERROR_INVALID_PARAMETER;
    if ((pSym->flOpen & SYM_OPEN_WRITE) == 0)
        return ERROR_INVALID_HANDLE;
    if (pSym->pszModule == NULL) return ERROR_INVALID_PARAMETER;

    ulLen = (ULONG)strlen(pszName);
    if (ulLen == 0 || ulLen > SYM_MAX_SEG_NAME)
        return ERROR_INVALID_PARAMETER;

    VectorGetCount(pSym->hSegments, &count);
    if (count >= SYM_MAX_SEGMENTS)
        return SYM_ERROR_TOO_MANY_SEGMENTS;

    pSeg = (SYM_SEGMENT *)calloc(1, sizeof(*pSeg));
    if (pSeg == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pSeg->pszName = (PSZ)malloc(ulLen + 1);
    if (pSeg->pszName == NULL) { free(pSeg); return ERROR_NOT_ENOUGH_MEMORY; }
    strcpy(pSeg->pszName, pszName);
    pSeg->ulFlags = ulFlags;
    if (VectorCreate(sizeof(SYM_SYMBOL *), &pSeg->hSymbols) != NO_ERROR) {
        free(pSeg->pszName); free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (VectorAdd(pSym->hSegments, &pSeg) != NO_ERROR) {
        VectorDestroy(pSeg->hSymbols);
        free(pSeg->pszName); free(pSeg);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (pulSegIdx) *pulSegIdx = count;
    pSym->fWritten = 0;
    return NO_ERROR;
}

/*!
 * @brief Append a symbol to an existing segment.
 *
 * @param[in] hSym      Handle opened for writing. Not NULLHANDLE.
 * @param[in] ulSegIdx  Zero-based segment index.
 * @param[in] ulValue   Symbol value.
 * @param[in] pszName   Symbol name. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                     Success.
 * @retval ERROR_INVALID_HANDLE         Handle not opened for writing.
 * @retval ERROR_INVALID_PARAMETER      Bad name or 16-bit overflow.
 * @retval SYM_ERROR_SEGMENT_NOT_FOUND  ulSegIdx out of range.
 * @retval SYM_ERROR_TOO_MANY_SYMBOLS   Symbol limit reached.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Allocation failure.
 */
APIRET APIENTRY SymAddSymbol(HSYMFILE hSym, ULONG ulSegIdx,
                             ULONG ulValue, PCSZ pszName)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    SYM_SEGMENT *pSeg;
    SYM_SYMBOL *pS;
    ULONG ulLen, count;

    if (pSym == NULL || pszName == NULL) return ERROR_INVALID_PARAMETER;
    if ((pSym->flOpen & SYM_OPEN_WRITE) == 0)
        return ERROR_INVALID_HANDLE;
    if (SymGetSegment(pSym, ulSegIdx, &pSeg) != NO_ERROR)
        return SYM_ERROR_SEGMENT_NOT_FOUND;

    ulLen = (ULONG)strlen(pszName);
    if (ulLen == 0 || ulLen > SYM_MAX_SYM_NAME)
        return ERROR_INVALID_PARAMETER;
    if ((pSeg->ulFlags & SYM_SEGDEF_32BIT) == 0 && ulValue > 0xFFFFUL)
        return ERROR_INVALID_PARAMETER;

    VectorGetCount(pSeg->hSymbols, &count);
    if (count >= SYM_MAX_SYMBOLS_PER_SEG)
        return SYM_ERROR_TOO_MANY_SYMBOLS;

    pS = (SYM_SYMBOL *)malloc(sizeof(*pS));
    if (pS == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pS->ulValue = ulValue;
    pS->pszName = (PSZ)malloc(ulLen + 1);
    if (pS->pszName == NULL) { free(pS); return ERROR_NOT_ENOUGH_MEMORY; }
    strcpy(pS->pszName, pszName);
    if (VectorAdd(pSeg->hSymbols, &pS) != NO_ERROR) {
        free(pS->pszName); free(pS);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pSym->fWritten = 0;
    return NO_ERROR;
}

/*!
 * @brief Force the handle to be written to disk.
 *
 * @param[in] hSym  Handle opened for writing. Not NULLHANDLE.
 * @return APIRET.
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle not opened for writing.
 * @retval ERROR_INVALID_PARAMETER  Module name not set.
 * @retval ERROR_OPEN_FAILED        Output file cannot be opened.
 * @retval ERROR_WRITE_FAULT        fwrite failed.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SymFlush(HSYMFILE hSym)
{
    SYM_HANDLE *pSym = SymGet(hSym);
    APIRET rc;
    if (pSym == NULL) return ERROR_INVALID_HANDLE;
    if ((pSym->flOpen & SYM_OPEN_WRITE) == 0)
        return ERROR_INVALID_HANDLE;
    if (pSym->pszModule == NULL) return ERROR_INVALID_PARAMETER;
    if (pSym->fWritten) return NO_ERROR;
    rc = SymWriteFile(pSym);
    if (rc == NO_ERROR) pSym->fWritten = 1;
    return rc;
}
