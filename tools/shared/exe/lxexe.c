/*!
 * @file lxexe.c
 *
 * @brief Implementation of the LX file access library.
 *
 * Linear Executable (LX) file access (C89). See lxexe.h for the
 * public API.
 *
 * All public functions declared in lxexe.h are implemented here.
 * The internal representation of HLX is defined in this translation
 * unit only; callers see it as an opaque HANDLE.
 *
 * References:
 *   - IBM OS/2 Toolkit, "Linear Executable File Format".
 *   - OpenWatcom WLINK sources.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lxexe.h"
#include "x86len.h"

/*!
 * @struct _LX_EXPORT_REC
 * @brief Cached record for one exported symbol.
 *
 * Populated lazily by LxParseExports. Forwarder entries carry no
 * object or offset; for them only the flags and the name matter.
 */
struct _LX_EXPORT_REC {
    char   achName[256];  /*!< Name, or empty when anonymous.     */
    BOOL   fNamed;        /*!< TRUE if a real name was found.     */
    USHORT usOrdinal;     /*!< Ordinal (1-based).                 */
    ULONG  ulFlags;       /*!< Raw entry flags byte.              */
    ULONG  ulObject;      /*!< 1-based object number, 0 forwarder.*/
    ULONG  ulOffset;      /*!< Offset within the object.          */
    BOOL   fForwarder;    /*!< TRUE for a forwarder entry.        */
    BOOL   f32Bit;        /*!< TRUE if the entry is 32-bit code.  */
};

/*!
 * @struct _LX
 * @brief Internal representation behind HLX.
 *
 * Not exposed to callers. lxexe.h declares the handle as HANDLE,
 * so the layout of this structure may change freely.
 */
struct _LX {
    FILE          *fp;        /*!< Underlying file stream.    */
    struct exe_hdr mz;        /*!< Cached MZ header.          */
    struct lx_exe  lx;        /*!< Cached LX header.          */

    struct _LX_EXPORT_REC *paExports;  /*!< Export cache. */
    ULONG          ulExportCount;      /*!< Used entries. */
    ULONG          ulExportCapacity;   /*!< Allocated.    */
    BOOL           fExportsParsed;     /*!< Lazy flag.    */

    char           achModuleName[256]; /*!< Module name.  */
};

/*!
 * @struct _LX_ENUM
 * @brief Internal representation behind HLXENUM.
 *
 * A cursor is a thin index into the owning handle's export cache.
 */
struct _LX_ENUM {
    struct _LX *pOwner;    /*!< Owning handle.    */
    ULONG       ulIndex;   /*!< Current position. */
};

/* ------------------------------------------------------------------ */
/* Little-endian integer helpers                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a little-endian WORD from a file offset.
 *
 * @param[in]  fp        Open stream. Not NULL.
 * @param[in]  lOffset   Absolute file offset.
 * @param[out] pusValue  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_READ_FAULT  Seek or read failure.
 */
static APIRET LxReadU16At(FILE *fp, long lOffset, PUSHORT pusValue)
{
    unsigned char ach[2];
    if (fseek(fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(ach, 1, 2, fp) != 2)
        return ERROR_READ_FAULT;
    *pusValue = (USHORT)((USHORT)ach[0] | ((USHORT)ach[1] << 8));
    return NO_ERROR;
}

/*!
 * @brief Read a little-endian DWORD from a file offset.
 *
 * @param[in]  fp        Open stream. Not NULL.
 * @param[in]  lOffset   Absolute file offset.
 * @param[out] pulValue  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_READ_FAULT  Seek or read failure.
 */
static APIRET LxReadU32At(FILE *fp, long lOffset, PULONG pulValue)
{
    unsigned char ach[4];
    if (fseek(fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(ach, 1, 4, fp) != 4)
        return ERROR_READ_FAULT;
    *pulValue = (ULONG)ach[0] | ((ULONG)ach[1] << 8) |
                ((ULONG)ach[2] << 16) | ((ULONG)ach[3] << 24);
    return NO_ERROR;
}

/*!
 * @brief Read a length-prefixed string at a file offset.
 *
 * The string on disk is stored as a byte count followed by that
 * many characters and no terminating NUL. This helper appends one.
 *
 * @param[in]  fp       Open stream. Not NULL.
 * @param[in]  lOffset  Absolute file offset of the length byte.
 * @param[out] pszName  Output buffer. Not NULL.
 * @param[in]  cbName   Size of @p pszName, including room for NUL.
 * @param[out] pcbUsed  Receives the number of bytes consumed.
 *                      Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_READ_FAULT         Read failure.
 * @retval ERROR_BUFFER_OVERFLOW    String does not fit.
 */
static APIRET LxReadNameAt(FILE *fp, long lOffset, PSZ pszName,
                           ULONG cbName, PULONG pcbUsed)
{
    unsigned char uchLen;

    if (fseek(fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(&uchLen, 1, 1, fp) != 1)
        return ERROR_READ_FAULT;

    if ((ULONG)uchLen + 1 > cbName)
        return ERROR_BUFFER_OVERFLOW;

    if (uchLen > 0) {
        if (fread(pszName, 1, uchLen, fp) != uchLen)
            return ERROR_READ_FAULT;
    }
    pszName[uchLen] = '\0';
    *pcbUsed = (ULONG)uchLen + 1;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Export cache helpers                                                */
/* ------------------------------------------------------------------ */

/*!
 * @brief Append one record to the handle's export cache.
 *
 * @param[in] pLx   Handle. Not NULL.
 * @param[in] pRec  Record to append. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET LxAppendExport(struct _LX *pLx,
                             const struct _LX_EXPORT_REC *pRec)
{
    struct _LX_EXPORT_REC *paTmp;
    ULONG ulNew;

    if (pLx->ulExportCount == pLx->ulExportCapacity) {
        ulNew = (pLx->ulExportCapacity == 0)
              ? 64
              : pLx->ulExportCapacity * 2;
        paTmp = (struct _LX_EXPORT_REC *)realloc(pLx->paExports,
                    ulNew * sizeof(struct _LX_EXPORT_REC));
        if (paTmp == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        pLx->paExports = paTmp;
        pLx->ulExportCapacity = ulNew;
    }
    pLx->paExports[pLx->ulExportCount++] = *pRec;
    return NO_ERROR;
}

/*!
 * @brief Attach a name to the export with the given ordinal.
 *
 * @param[in] pLx        Handle. Not NULL.
 * @param[in] pszName    NUL-terminated name. Not NULL.
 * @param[in] usOrdinal  Ordinal to match.
 */
static void LxAssignName(struct _LX *pLx, const char *pszName,
                         USHORT usOrdinal)
{
    ULONG i;

    for (i = 0; i < pLx->ulExportCount; i++) {
        if (pLx->paExports[i].usOrdinal == usOrdinal) {
            if (!pLx->paExports[i].fNamed) {
                strncpy(pLx->paExports[i].achName, pszName,
                        sizeof(pLx->paExports[i].achName) - 1);
                pLx->paExports[i].achName[
                    sizeof(pLx->paExports[i].achName) - 1] = '\0';
                pLx->paExports[i].fNamed = TRUE;
            }
            return;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Name table parser                                                   */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a resident or non-resident name table.
 *
 * The table is a sequence of [len][name][ordinal:2] entries,
 * terminated by a length byte of zero. When @p fFirstIsModule is
 * TRUE, the first entry is taken as the module name and skipped;
 * otherwise all entries are matched against the export cache.
 *
 * @param[in] pLx            Handle. Not NULL.
 * @param[in] lOffset        Absolute file offset of the table.
 * @param[in] lSize          Table size in bytes, or 0 if unknown.
 * @param[in] fFirstIsModule TRUE for the resident table.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_READ_FAULT  Read error.
 */
static APIRET LxReadNameTable(struct _LX *pLx, long lOffset,
                              long lSize, BOOL fFirstIsModule)
{
    long lPos = lOffset;
    long lEnd = (lSize > 0) ? lOffset + lSize : -1;

    for (;;) {
        unsigned char uchLen;
        char          achName[256];
        USHORT        usOrdinal;
        ULONG         cbUsed;

        if (lEnd > 0 && lPos >= lEnd)
            break;

        if (fseek(pLx->fp, lPos, SEEK_SET) != 0)
            return ERROR_READ_FAULT;
        if (fread(&uchLen, 1, 1, pLx->fp) != 1)
            return ERROR_READ_FAULT;

        if (uchLen == 0)
            break;

        {
            APIRET rc = LxReadNameAt(pLx->fp, lPos, achName,
                                     sizeof(achName), &cbUsed);
            if (rc != NO_ERROR)
                return rc;
            lPos += (long)cbUsed;
        }

        if (fFirstIsModule) {
            strncpy(pLx->achModuleName, achName,
                    sizeof(pLx->achModuleName) - 1);
            pLx->achModuleName[sizeof(pLx->achModuleName) - 1] = '\0';
            fFirstIsModule = FALSE;
            continue;
        }

        if (LxReadU16At(pLx->fp, lPos, &usOrdinal) != NO_ERROR)
            return ERROR_READ_FAULT;
        lPos += 2;

        LxAssignName(pLx, achName, usOrdinal);
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Entry table parser                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Parse the Entry Table and both name tables on first use.
 *
 * Called from LxExportFindFirst, LxExportFindByName and
 * LxExportFindByOrdinal. On success, the handle caches a complete,
 * name-resolved export list in increasing ordinal order.
 *
 * @param[in] pLx  Handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET LxParseExports(struct _LX *pLx)
{
    ULONG  ulPos;
    USHORT usOrdinal = 0;

    if (pLx->fExportsParsed)
        return NO_ERROR;

    ulPos = pLx->lx.lx_enttab;

    for (;;) {
        unsigned char uchCount;
        unsigned char uchType;
        unsigned char uchBase;
        ULONG         ulEntrySize;
        USHORT        i;

        if (fseek(pLx->fp, (long)ulPos, SEEK_SET) != 0)
            return ERROR_READ_FAULT;
        if (fread(&uchCount, 1, 1, pLx->fp) != 1)
            return ERROR_READ_FAULT;
        ulPos++;

        if (uchCount == 0)
            break;

        if (fread(&uchType, 1, 1, pLx->fp) != 1)
            return ERROR_READ_FAULT;
        ulPos++;

        uchBase = uchType & LXENT_TYPEMASK;

        switch (uchBase) {
        case LXENT_EMPTY:
            usOrdinal += uchCount;
            continue;
        case LXENT_16BIT:
            ulEntrySize = 6;
            break;
        case LXENT_286CALLGATE:
            ulEntrySize = 8;
            break;
        case LXENT_32BIT:
            ulEntrySize = 8;
            break;
        case LXENT_FORWARDER:
            ulEntrySize = 8;
            break;
        default:
            return ERROR_READ_FAULT;
        }

        for (i = 0; i < uchCount; i++) {
            unsigned char achEntry[8];
            struct _LX_EXPORT_REC stRec;
            unsigned char uchFlags;

            if (fread(achEntry, 1, ulEntrySize, pLx->fp)
                != ulEntrySize)
                return ERROR_READ_FAULT;
            ulPos += ulEntrySize;

            usOrdinal++;
            memset(&stRec, 0, sizeof(stRec));
            stRec.usOrdinal = usOrdinal;

            switch (uchBase) {
            case LXENT_16BIT:
                stRec.ulObject = (ULONG)achEntry[0] |
                                 ((ULONG)achEntry[1] << 8);
                uchFlags = achEntry[2];
                stRec.ulOffset = (ULONG)achEntry[3] |
                                 ((ULONG)achEntry[4] << 8);
                stRec.f32Bit = FALSE;
                break;
            case LXENT_286CALLGATE:
                stRec.ulObject = (ULONG)achEntry[0] |
                                 ((ULONG)achEntry[1] << 8);
                uchFlags = achEntry[2];
                stRec.ulOffset = (ULONG)achEntry[3] |
                                 ((ULONG)achEntry[4] << 8);
                stRec.f32Bit = FALSE;
                break;
            case LXENT_32BIT:
                stRec.ulObject = (ULONG)achEntry[0] |
                                 ((ULONG)achEntry[1] << 8);
                uchFlags = achEntry[2];
                stRec.ulOffset = (ULONG)achEntry[3] |
                                 ((ULONG)achEntry[4] << 8) |
                                 ((ULONG)achEntry[5] << 16) |
                                 ((ULONG)achEntry[6] << 24);
                stRec.f32Bit = TRUE;
                break;
            case LXENT_FORWARDER:
                uchFlags = achEntry[2];
                stRec.fForwarder = TRUE;
                stRec.f32Bit = TRUE;
                break;
            default:
                uchFlags = 0;
                break;
            }

            if ((uchFlags & LXENT_EXPORTED) == 0)
                continue;

            stRec.ulFlags = uchFlags;

            if (LxAppendExport(pLx, &stRec) != NO_ERROR)
                return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    /* Resident name table. */
    if (pLx->lx.lx_restab != 0) {
        APIRET rc = LxReadNameTable(pLx,
                        (long)pLx->lx.lx_restab, 0, TRUE);
        if (rc != NO_ERROR)
            return rc;
    }

    /* Non-resident name table. */
    if (pLx->lx.lx_nrestab != 0 && pLx->lx.lx_cbnrestab != 0) {
        APIRET rc = LxReadNameTable(pLx,
                        (long)pLx->lx.lx_nrestab,
                        (long)pLx->lx.lx_cbnrestab, FALSE);
        if (rc != NO_ERROR)
            return rc;
    }

    pLx->fExportsParsed = TRUE;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Object body access                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read up to @p ulLen bytes from an object.
 *
 * The object-relative offset is converted to a file offset through
 * the object page map. Bytes that fall into zero-filled pages
 * (page map entry 0) are returned as zeros, matching the way the
 * loader would materialise them.
 *
 * @param[in]  pLx      Handle. Not NULL.
 * @param[in]  ulObject 1-based object number.
 * @param[in]  ulOffset Offset within the object.
 * @param[out] puchBuf  Destination. Not NULL.
 * @param[in]  ulLen    Number of bytes requested.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Object number out of range.
 * @retval ERROR_READ_FAULT         Read error.
 */
static APIRET LxReadObjectBytes(struct _LX *pLx, ULONG ulObject,
                                ULONG ulOffset,
                                UCHAR *puchBuf, ULONG ulLen)
{
    struct lx_object stObj;
    ULONG  ulPageSize = pLx->lx.lx_pagesize;
    ULONG  ulRemaining = ulLen;
    ULONG  ulPos = ulOffset;
    APIRET rc;

    if (pLx->lx.lx_objcnt == 0 || ulPageSize == 0)
        return ERROR_READ_FAULT;
    if (ulObject < 1 || ulObject > pLx->lx.lx_objcnt)
        return ERROR_INVALID_PARAMETER;

    rc = LxReadU32At(pLx->fp,
                     (long)(pLx->lx.lx_objtab +
                            (ulObject - 1) * 24),
                     &stObj.o32_vsize);
    if (rc != NO_ERROR)
        return rc;
    rc = LxReadU32At(pLx->fp,
                     (long)(pLx->lx.lx_objtab +
                            (ulObject - 1) * 24 + 4),
                     &stObj.o32_vbase);
    if (rc != NO_ERROR)
        return rc;
    rc = LxReadU32At(pLx->fp,
                     (long)(pLx->lx.lx_objtab +
                            (ulObject - 1) * 24 + 8),
                     &stObj.o32_flags);
    if (rc != NO_ERROR)
        return rc;
    rc = LxReadU32At(pLx->fp,
                     (long)(pLx->lx.lx_objtab +
                            (ulObject - 1) * 24 + 12),
                     &stObj.o32_pagemap);
    if (rc != NO_ERROR)
        return rc;
    rc = LxReadU32At(pLx->fp,
                     (long)(pLx->lx.lx_objtab +
                            (ulObject - 1) * 24 + 16),
                     &stObj.o32_mapsize);
    if (rc != NO_ERROR)
        return rc;

    while (ulRemaining > 0) {
        ULONG ulPageNum = ulPos / ulPageSize;
        ULONG ulInPage  = ulPos % ulPageSize;
        ULONG ulChunk   = ulPageSize - ulInPage;
        ULONG ulFilePage;
        ULONG ulPmIdx;

        if (ulChunk > ulRemaining)
            ulChunk = ulRemaining;

        if (ulPageNum >= stObj.o32_mapsize)
            return ERROR_READ_FAULT;

        ulPmIdx = stObj.o32_pagemap + ulPageNum;
        rc = LxReadU32At(pLx->fp,
                         (long)(pLx->lx.lx_objmap + ulPmIdx * 4),
                         &ulFilePage);
        if (rc != NO_ERROR)
            return rc;

        if (ulFilePage == 0) {
            memset(puchBuf, 0, ulChunk);
        } else {
            long lFileOff = (long)((ulFilePage - 1) * ulPageSize
                                   + ulInPage);
            if (fseek(pLx->fp, lFileOff, SEEK_SET) != 0)
                return ERROR_READ_FAULT;
            if (fread(puchBuf, 1, ulChunk, pLx->fp) != ulChunk)
                return ERROR_READ_FAULT;
        }

        puchBuf += ulChunk;
        ulPos += ulChunk;
        ulRemaining -= ulChunk;
    }

    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Open / close                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Open an LX executable file.
 *
 * Reads and caches the MZ header and the LX header. The handle
 * returned in @p *phLx owns the underlying FILE stream and must be
 * released with LxClose.
 *
 * @param[in]  pszPath  Path to the LX file. Not NULL.
 * @param[out] phLx     Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on failure.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phLx is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error, bad MZ magic, bad
 *                                  LX magic, or invalid e_lfanew.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see LxClose
 */
APIRET APIENTRY LxOpen(PCSZ pszPath, HLX *phLx)
{
    FILE       *fp;
    struct _LX *pLx;
    struct exe_hdr mz;
    struct lx_exe  lx;
    long lLxOffset;

    if (!pszPath || !phLx) return ERROR_INVALID_PARAMETER;
    *phLx = NULLHANDLE;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    if (fread(&mz, 1, sizeof(mz), fp) != sizeof(mz)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (E_MAGIC(mz) != EMAGIC) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    lLxOffset = (long)E_LFANEW(mz);
    if (lLxOffset <= 0) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (fseek(fp, lLxOffset, SEEK_SET) != 0) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (fread(&lx, 1, sizeof(lx), fp) != sizeof(lx)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (lx.lx_magic != LXMAGIC) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }

    pLx = (struct _LX *)calloc(1, sizeof(*pLx));
    if (!pLx) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pLx->fp = fp;
    memcpy(&pLx->mz, &mz, sizeof(mz));
    memcpy(&pLx->lx, &lx, sizeof(lx));
    pLx->paExports = NULL;
    pLx->ulExportCount = 0;
    pLx->ulExportCapacity = 0;
    pLx->fExportsParsed = FALSE;
    pLx->achModuleName[0] = '\0';

    *phLx = (HLX)pLx;
    return NO_ERROR;
}

/*!
 * @brief Close an LX executable.
 *
 * Flushes and closes the underlying stream and releases the handle.
 * Idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 * @param[in] hLx  Handle from LxOpen. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 *
 * @see LxOpen
 */
APIRET APIENTRY LxClose(HLX hLx)
{
    struct _LX *pLx;

    if (hLx == NULLHANDLE) return NO_ERROR;
    pLx = (struct _LX *)hLx;
    if (pLx->fp) fclose(pLx->fp);
    if (pLx->paExports) free(pLx->paExports);
    free(pLx);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Header queries                                                      */
/* ------------------------------------------------------------------ */

/*!
 * @brief Retrieve the cached MZ header.
 *
 * @param[in]  hLx  Handle from LxOpen. Not NULLHANDLE.
 * @param[out] pMZ  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx is NULLHANDLE or @p pMZ
 *                                  is NULL.
 */
APIRET APIENTRY LxQueryMZHeader(HLX hLx, struct exe_hdr *pMZ)
{
    struct _LX *pLx;

    if (hLx == NULLHANDLE || !pMZ) return ERROR_INVALID_PARAMETER;
    pLx = (struct _LX *)hLx;
    memcpy(pMZ, &pLx->mz, sizeof(*pMZ));
    return NO_ERROR;
}

/*!
 * @brief Retrieve the cached LX header.
 *
 * @param[in]  hLx  Handle from LxOpen. Not NULLHANDLE.
 * @param[out] pLX  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx is NULLHANDLE or @p pLX
 *                                  is NULL.
 */
APIRET APIENTRY LxQueryHeader(HLX hLx, struct lx_exe *pLX)
{
    struct _LX *pLx;

    if (hLx == NULLHANDLE || !pLX) return ERROR_INVALID_PARAMETER;
    pLx = (struct _LX *)hLx;
    memcpy(pLX, &pLx->lx, sizeof(*pLX));
    return NO_ERROR;
}

/*!
 * @brief Return the number of objects.
 *
 * @param[in]  hLx       Handle from LxOpen. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx is NULLHANDLE or
 *                                  @p pulCount is NULL.
 */
APIRET APIENTRY LxQueryObjectCount(HLX hLx, PULONG pulCount)
{
    struct _LX *pLx;

    if (hLx == NULLHANDLE || !pulCount) return ERROR_INVALID_PARAMETER;
    pLx = (struct _LX *)hLx;
    *pulCount = pLx->lx.lx_objcnt;
    return NO_ERROR;
}

/*!
 * @brief Return one object table entry.
 *
 * @param[in]  hLx      Handle from LxOpen. Not NULLHANDLE.
 * @param[in]  ulIndex  1-based object index, in [1, lx_objcnt].
 * @param[out] pObj     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx is NULLHANDLE, @p pObj is
 *                                  NULL, or @p ulIndex is out of
 *                                  range.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxQueryObject(HLX hLx, ULONG ulIndex,
                              struct lx_object *pObj)
{
    struct _LX *pLx;
    long lOffset;
    APIRET rc;

    if (hLx == NULLHANDLE || !pObj) return ERROR_INVALID_PARAMETER;
    pLx = (struct _LX *)hLx;
    if (ulIndex < 1 || ulIndex > pLx->lx.lx_objcnt)
        return ERROR_INVALID_PARAMETER;

    lOffset = (long)(pLx->lx.lx_objtab + (ulIndex - 1) * 24);
    rc = LxReadU32At(pLx->fp, lOffset, &pObj->o32_vsize);
    if (rc != NO_ERROR) return rc;
    rc = LxReadU32At(pLx->fp, lOffset + 4, &pObj->o32_vbase);
    if (rc != NO_ERROR) return rc;
    rc = LxReadU32At(pLx->fp, lOffset + 8, &pObj->o32_flags);
    if (rc != NO_ERROR) return rc;
    rc = LxReadU32At(pLx->fp, lOffset + 12, &pObj->o32_pagemap);
    if (rc != NO_ERROR) return rc;
    rc = LxReadU32At(pLx->fp, lOffset + 16, &pObj->o32_mapsize);
    if (rc != NO_ERROR) return rc;
    rc = LxReadU32At(pLx->fp, lOffset + 20, &pObj->o32_reserved);
    if (rc != NO_ERROR) return rc;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Module name                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Return the module name of this LX image.
 *
 * The module name is the first entry of the Resident Name Table.
 * If the table has not been parsed yet, this function parses it.
 *
 * @param[in]  hLx      Handle from LxOpen. Not NULLHANDLE.
 * @param[out] pszName  Output buffer. Not NULL.
 * @param[in]  cbName   Size of @p pszName in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx is NULLHANDLE, @p pszName
 *                                  is NULL, or @p cbName is zero.
 * @retval ERROR_BUFFER_OVERFLOW    Name does not fit in @p pszName.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxQuerySelfModuleName(HLX hLx, PSZ pszName, ULONG cbName)
{
    struct _LX *pLx;
    size_t cb;

    if (hLx == NULLHANDLE || !pszName || cbName == 0)
        return ERROR_INVALID_PARAMETER;

    pLx = (struct _LX *)hLx;

    if (!pLx->fExportsParsed) {
        APIRET rc = LxParseExports(pLx);
        if (rc != NO_ERROR)
            return rc;
    }

    cb = strlen(pLx->achModuleName);
    if (cb + 1 > cbName)
        return ERROR_BUFFER_OVERFLOW;
    memcpy(pszName, pLx->achModuleName, cb + 1);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* LX export enumeration API                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Open a cursor on the first export of an LX module.
 *
 * @param[in]  hLx       Handle from LxOpen. Not NULLHANDLE.
 * @param[out] phEnum    Cursor receiver. Not NULL. Set to NULLHANDLE
 *                       on error or when the module has no exports.
 * @param[out] pulCount  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx or @p phEnum is NULL.
 * @retval ERROR_NO_MORE_ITEMS      No exports. *phEnum = NULLHANDLE.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxExportFindFirst(HLX hLx, HLXENUM *phEnum,
                                  PULONG pulCount)
{
    struct _LX      *pLx;
    struct _LX_ENUM *pEnum;
    APIRET           rc;

    if (hLx == NULLHANDLE || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pLx = (struct _LX *)hLx;
    *phEnum = NULLHANDLE;

    rc = LxParseExports(pLx);
    if (rc != NO_ERROR)
        return rc;

    if (pulCount != NULL)
        *pulCount = pLx->ulExportCount;

    if (pLx->ulExportCount == 0)
        return ERROR_NO_MORE_ITEMS;

    pEnum = (struct _LX_ENUM *)malloc(sizeof(*pEnum));
    if (pEnum == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->pOwner  = pLx;
    pEnum->ulIndex = 0;

    *phEnum = (HLXENUM)pEnum;
    return NO_ERROR;
}

/*!
 * @brief Advance an export cursor to the next export.
 *
 * @param[in] hEnum  Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     @p hEnum is NULLHANDLE.
 * @retval ERROR_NO_MORE_ITEMS      No more exports.
 */
APIRET APIENTRY LxExportFindNext(HLXENUM hEnum)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;

    if (pEnum == NULL)
        return ERROR_INVALID_HANDLE;
    if (pEnum->ulIndex + 1 >= pEnum->pOwner->ulExportCount)
        return ERROR_NO_MORE_ITEMS;
    pEnum->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Close an export cursor.
 *
 * @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 */
APIRET APIENTRY LxExportFindClose(HLXENUM hEnum)
{
    if (hEnum != NULLHANDLE)
        free(hEnum);
    return NO_ERROR;
}

/*!
 * @brief Position a new cursor on an export by name.
 *
 * @param[in]  hLx      Handle. Not NULLHANDLE.
 * @param[in]  pszName  Name to find. Not NULL.
 * @param[out] phEnum   Cursor receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Name not present.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxExportFindByName(HLX hLx, PCSZ pszName,
                                   HLXENUM *phEnum)
{
    struct _LX *pLx;
    ULONG       i;
    APIRET      rc;

    if (hLx == NULLHANDLE || pszName == NULL || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pLx = (struct _LX *)hLx;
    *phEnum = NULLHANDLE;

    rc = LxParseExports(pLx);
    if (rc != NO_ERROR)
        return rc;

    for (i = 0; i < pLx->ulExportCount; i++) {
        if (pLx->paExports[i].fNamed &&
            strcmp(pLx->paExports[i].achName, pszName) == 0) {
            struct _LX_ENUM *pEnum =
                (struct _LX_ENUM *)malloc(sizeof(*pEnum));
            if (pEnum == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            pEnum->pOwner  = pLx;
            pEnum->ulIndex = i;
            *phEnum = (HLXENUM)pEnum;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Position a new cursor on an export by ordinal.
 *
 * @param[in]  hLx        Handle. Not NULLHANDLE.
 * @param[in]  usOrdinal  Ordinal to find (1-based).
 * @param[out] phEnum     Cursor receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hLx or @p phEnum is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Ordinal is not exported.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY LxExportFindByOrdinal(HLX hLx, USHORT usOrdinal,
                                      HLXENUM *phEnum)
{
    struct _LX *pLx;
    ULONG       i;
    APIRET      rc;

    if (hLx == NULLHANDLE || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pLx = (struct _LX *)hLx;
    *phEnum = NULLHANDLE;

    rc = LxParseExports(pLx);
    if (rc != NO_ERROR)
        return rc;

    for (i = 0; i < pLx->ulExportCount; i++) {
        if (pLx->paExports[i].usOrdinal == usOrdinal) {
            struct _LX_ENUM *pEnum =
                (struct _LX_ENUM *)malloc(sizeof(*pEnum));
            if (pEnum == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            pEnum->pOwner  = pLx;
            pEnum->ulIndex = i;
            *phEnum = (HLXENUM)pEnum;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Retrieve the name of the current export.
 *
 * Size-query convention as described in lxexe.h. For anonymous
 * exports, a synthetic name "Ordinal<N>" is returned.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY LxExportGetName(HLXENUM hEnum,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;
    const struct _LX_EXPORT_REC *pRec;
    char   achSynth[32];
    PCSZ   pszSrc;
    size_t cb;

    if (pEnum == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;

    pRec = &pEnum->pOwner->paExports[pEnum->ulIndex];

    if (pRec->fNamed) {
        pszSrc = pRec->achName;
    } else {
        sprintf(achSynth, "Ordinal%u", (unsigned)pRec->usOrdinal);
        pszSrc = achSynth;
    }

    cb = strlen(pszSrc);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL)
            *pulUsed = (ULONG)(cb + 1);
        return NO_ERROR;
    }
    if (pszBuf == NULL)
        return ERROR_INVALID_PARAMETER;
    if (cb + 1 > ulSize) {
        if (pulUsed != NULL)
            *pulUsed = (ULONG)(cb + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszSrc, cb + 1);
    if (pulUsed != NULL)
        *pulUsed = (ULONG)cb;
    return NO_ERROR;
}

/*!
 * @brief Retrieve the ordinal of the current export.
 *
 * @param[in]  hEnum       Cursor. Not NULLHANDLE.
 * @param[out] pusOrdinal  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY LxExportGetOrdinal(HLXENUM hEnum, PUSHORT pusOrdinal)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;

    if (pEnum == NULL || pusOrdinal == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pusOrdinal = pEnum->pOwner->paExports[pEnum->ulIndex].usOrdinal;
    return NO_ERROR;
}

/*!
 * @brief Retrieve the raw entry flags of the current export.
 *
 * @param[in]  hEnum     Cursor. Not NULLHANDLE.
 * @param[out] pulFlags  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY LxExportGetFlags(HLXENUM hEnum, PULONG pulFlags)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;

    if (pEnum == NULL || pulFlags == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pulFlags = pEnum->pOwner->paExports[pEnum->ulIndex].ulFlags;
    return NO_ERROR;
}

/*!
 * @brief Query whether the current export has a real name.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
 * @param[out] pfNamed  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY LxExportIsNamed(HLXENUM hEnum, PBOOL pfNamed)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;

    if (pEnum == NULL || pfNamed == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pfNamed = pEnum->pOwner->paExports[pEnum->ulIndex].fNamed;
    return NO_ERROR;
}

/*!
 * @brief Query whether the current export is a variable.
 *
 * Tests the SINGLEDATA bit (LXENT_SINGLEDATA) in the raw flags.
 *
 * @param[in]  hEnum         Cursor. Not NULLHANDLE.
 * @param[out] pfGlobalData  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY LxExportIsGlobalData(HLXENUM hEnum, PBOOL pfGlobalData)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;

    if (pEnum == NULL || pfGlobalData == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pfGlobalData =
        (pEnum->pOwner->paExports[pEnum->ulIndex].ulFlags &
         LXENT_SINGLEDATA) != 0;
    return NO_ERROR;
}

/*!
 * @brief Query whether the current export is a forwarder.
 *
 * @param[in]  hEnum        Cursor. Not NULLHANDLE.
 * @param[out] pfForwarder  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY LxExportIsForwarder(HLXENUM hEnum, PBOOL pfForwarder)
{
    struct _LX_ENUM *pEnum = (struct _LX_ENUM *)hEnum;

    if (pEnum == NULL || pfForwarder == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pfForwarder = pEnum->pOwner->paExports[pEnum->ulIndex].fForwarder;
    return NO_ERROR;
}

/*!
 * @brief Determine the calling convention of the current export.
 *
 * Reads the first bytes of the function body from the object that
 * contains the entry point and walks them instruction by
 * instruction with X86Decode. The first ret-family opcode
 * encountered selects the convention:
 *
 *   - C3        ret        -> "_System"
 *   - C2 xx xx  ret imm16  -> "_stdcall"
 *   - CB        retf       -> "_System"
 *   - CA xx xx  retf imm16 -> "_stdcall"
 *
 * Forwarder entries always yield "_System". If the decoder hits an
 * unrecognized instruction, or if the scan reaches the end of the
 * available bytes without finding a ret, the default convention
 * "_System" is returned. LX does not store the calling convention
 * explicitly; the choice is a heuristic.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY LxExportGetConvention(HLXENUM hEnum,
                                      PSZ pszBuf, ULONG ulSize,
                                      PULONG pulUsed)
{
    struct _LX_ENUM *pEnum;
    const struct _LX_EXPORT_REC *pRec;
    UCHAR  achBuf[8192];
    ULONG  ulScan = 0;
    ULONG  pos;
    PCSZ   pszConv;
    size_t cb;
    APIRET rc;

    if (hEnum == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    pEnum = (struct _LX_ENUM *)hEnum;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;

    pRec = &pEnum->pOwner->paExports[pEnum->ulIndex];

    if (pRec->fForwarder) {
        pszConv = "_System";
    } else {
        pszConv = "_System";

        rc = LxReadObjectBytes(pEnum->pOwner, pRec->ulObject,
                               pRec->ulOffset, achBuf,
                               (ULONG)sizeof(achBuf));
        if (rc == NO_ERROR) {
            ulScan = (ULONG)sizeof(achBuf);

            pos = 0;
            while (pos < ulScan) {
                X86_INSN insn;

                if (X86Decode(achBuf + pos, ulScan - pos,
                              pRec->f32Bit, &insn) != NO_ERROR)
                    break;

                if (insn.ulFamily == X86_FAM_RET) {
                    pszConv = "_System";
                    break;
                }
                if (insn.ulFamily == X86_FAM_RETF) {
                    pszConv = "_System";
                    break;
                }
                if (insn.ulFamily == X86_FAM_RET_IMM) {
                    pszConv = insn.usImm ? "_stdcall"
                                         : "_System";
                    break;
                }
                if (insn.ulFamily == X86_FAM_RETF_IMM) {
                    pszConv = insn.usImm ? "_stdcall"
                                         : "_System";
                    break;
                }
                pos += insn.ulLength;
            }
        }
    }

    cb = strlen(pszConv);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL)
            *pulUsed = (ULONG)(cb + 1);
        return NO_ERROR;
    }
    if (pszBuf == NULL)
        return ERROR_INVALID_PARAMETER;
    if (cb + 1 > ulSize) {
        if (pulUsed != NULL)
            *pulUsed = (ULONG)(cb + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszConv, cb + 1);
    if (pulUsed != NULL)
        *pulUsed = (ULONG)cb;
    return NO_ERROR;
}
