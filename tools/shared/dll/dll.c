/*!
 * @file newexe.c
 *
 * @brief Implementation of the NE file access library.
 *
 * New Executable (NE) file access (C89). See newexe.h for the
 * public API.
 *
 * All public functions declared in newexe.h are implemented here.
 * The internal representation of HNE is defined in this translation
 * unit only; callers see it as an opaque HANDLE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "newexe.h"
#include "x86len.h"

/*!
 * @struct _NE_EXPORT_REC
 * @brief Cached record for one exported symbol.
 *
 * Populated lazily by NeParseExports. The name is filled in from
 * the resident and non-resident name tables; fNamed is FALSE when
 * the export has no name and is reachable only by ordinal.
 */
struct _NE_EXPORT_REC {
    char   achName[256];   /*!< Name, or empty when anonymous. */
    BOOL   fNamed;         /*!< TRUE if a real name was found. */
    USHORT usOrdinal;      /*!< Ordinal (1-based).               */
    ULONG  ulFlags;        /*!< Raw Entry Table flags byte.      */
    USHORT usSegment;      /*!< Segment number.                  */
    ULONG  ulOffset;       /*!< Offset within the segment.       */
};

/*!
 * @struct _NE
 * @brief Internal representation behind HNE.
 *
 * Not exposed to callers. newexe.h declares the handle as HANDLE,
 * so the layout of this structure may change freely.
 */
struct _NE {
    FILE          *fp;        /*!< Underlying file stream.      */
    long           lNeOffset; /*!< File offset of NE header.    */
    struct exe_hdr mz;        /*!< Cached MZ header.            */
    struct new_exe ne;        /*!< Cached NE header.            */

    struct _NE_EXPORT_REC *paExports;   /*!< Export cache. */
    ULONG          ulExportCount;       /*!< Used entries. */
    ULONG          ulExportCapacity;    /*!< Allocated.    */
    BOOL           fExportsParsed;      /*!< Lazy flag.    */
};

/*!
 * @struct _NE_ENUM
 * @brief Internal representation behind HNEENUM.
 *
 * A cursor is a thin index into the owning handle's export cache.
 */
struct _NE_ENUM {
    struct _NE *pOwner;    /*!< Owning handle.    */
    ULONG       ulIndex;   /*!< Current position. */
};

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a little-endian WORD from the current stream position.
 *
 * @param[in]  fp        Open stream. Not NULL.
 * @param[out] pusValue  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_READ_FAULT  Read failure.
 */
static APIRET NeReadU16(FILE *fp, PUSHORT pusValue)
{
    unsigned char ach[2];
    if (fread(ach, 1, 2, fp) != 2)
        return ERROR_READ_FAULT;
    *pusValue = (USHORT)((USHORT)ach[0] | ((USHORT)ach[1] << 8));
    return NO_ERROR;
}

/*!
 * @brief Append one record to the handle's export cache.
 *
 * Grows the array geometrically.
 *
 * @param[in] pNe   Handle. Not NULL.
 * @param[in] pRec  Record to append. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET NeAppendExport(struct _NE *pNe,
                             const struct _NE_EXPORT_REC *pRec)
{
    struct _NE_EXPORT_REC *paTmp;
    ULONG ulNew;

    if (pNe->ulExportCount == pNe->ulExportCapacity) {
        ulNew = (pNe->ulExportCapacity == 0)
              ? 64
              : pNe->ulExportCapacity * 2;
        paTmp = (struct _NE_EXPORT_REC *)realloc(pNe->paExports,
                    ulNew * sizeof(struct _NE_EXPORT_REC));
        if (paTmp == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        pNe->paExports = paTmp;
        pNe->ulExportCapacity = ulNew;
    }
    pNe->paExports[pNe->ulExportCount++] = *pRec;
    return NO_ERROR;
}

/*!
 * @brief Attach a name to the export with the given ordinal.
 *
 * If the record already has a name, the call has no effect: names
 * from the resident table take precedence over the non-resident
 * table because the resident one is parsed first.
 *
 * @param[in] pNe        Handle. Not NULL.
 * @param[in] pszName    NUL-terminated name. Not NULL.
 * @param[in] usOrdinal  Ordinal to match.
 */
static void NeAssignName(struct _NE *pNe, const char *pszName,
                         USHORT usOrdinal)
{
    ULONG i;

    for (i = 0; i < pNe->ulExportCount; i++) {
        if (pNe->paExports[i].usOrdinal == usOrdinal) {
            if (!pNe->paExports[i].fNamed) {
                strncpy(pNe->paExports[i].achName, pszName,
                        sizeof(pNe->paExports[i].achName) - 1);
                pNe->paExports[i].achName[
                    sizeof(pNe->paExports[i].achName) - 1] = '\0';
                pNe->paExports[i].fNamed = TRUE;
            }
            return;
        }
    }
}

/*!
 * @brief Read a resident or non-resident name table.
 *
 * The table is a sequence of [len][name][ordinal:2] entries,
 * terminated by a length byte of zero. When @p fFirstIsModule is
 * TRUE, the first entry is skipped (it is the module name and is
 * not associated with any ordinal).
 *
 * The local name buffer is 256 bytes; a length-prefixed string can
 * hold at most 255 characters, so no truncation is possible and no
 * length check is required.
 *
 * @param[in] pNe            Handle. Not NULL.
 * @param[in] lOffset        Absolute file offset of the table.
 * @param[in] fFirstIsModule TRUE for the resident table.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_READ_FAULT  Read error.
 */
static APIRET NeReadNameTable(struct _NE *pNe, long lOffset,
                              BOOL fFirstIsModule)
{
    long lPos = lOffset;

    for (;;) {
        unsigned char uchLen;
        char          achName[256];
        USHORT        usOrdinal;

        if (fseek(pNe->fp, lPos, SEEK_SET) != 0)
            return ERROR_READ_FAULT;
        if (fread(&uchLen, 1, 1, pNe->fp) != 1)
            return ERROR_READ_FAULT;
        lPos++;

        if (uchLen == 0)
            break;

        if (fread(achName, 1, uchLen, pNe->fp) != uchLen)
            return ERROR_READ_FAULT;
        achName[uchLen] = '\0';
        lPos += uchLen;

        if (NeReadU16(pNe->fp, &usOrdinal) != NO_ERROR)
            return ERROR_READ_FAULT;
        lPos += 2;

        if (fFirstIsModule) {
            fFirstIsModule = FALSE;
            continue;
        }

        NeAssignName(pNe, achName, usOrdinal);
    }

    return NO_ERROR;
}

/*!
 * @brief Parse the Entry Table and both name tables on first use.
 *
 * Called from NeQueryExportFirst, NeQueryExportByName and
 * NeQueryExportByOrdinal. On success, the handle caches a
 * complete, name-resolved export list in increasing ordinal order.
 *
 * @param[in] pNe  Handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET NeParseExports(struct _NE *pNe)
{
    long   lPos;
    USHORT usOrdinal = 0;

    if (pNe->fExportsParsed)
        return NO_ERROR;

    lPos = pNe->lNeOffset + pNe->ne.ne_enttab;

    for (;;) {
        unsigned char uchCount;
        unsigned char uchType;
        USHORT        i;

        if (fseek(pNe->fp, lPos, SEEK_SET) != 0)
            return ERROR_READ_FAULT;
        if (fread(&uchCount, 1, 1, pNe->fp) != 1)
            return ERROR_READ_FAULT;
        lPos++;

        if (uchCount == 0)
            break;

        if (fread(&uchType, 1, 1, pNe->fp) != 1)
            return ERROR_READ_FAULT;
        lPos++;

        if (uchType == 0) {
            /* Empty bundle: skip this many ordinals. */
            usOrdinal += uchCount;
            continue;
        }

        if (uchType == 0xFF) {
            /* Movable segment entries: 6 bytes each. */
            for (i = 0; i < uchCount; i++) {
                unsigned char achEntry[6];
                struct _NE_EXPORT_REC stRec;
                unsigned char uchFlags;

                if (fread(achEntry, 1, 6, pNe->fp) != 6)
                    return ERROR_READ_FAULT;
                lPos += 6;
                usOrdinal++;

                uchFlags = achEntry[0];
                if ((uchFlags & NEENT_EXPORTED) == 0)
                    continue;

                memset(&stRec, 0, sizeof(stRec));
                stRec.usOrdinal = usOrdinal;
                stRec.ulFlags   = uchFlags;
                stRec.usSegment = achEntry[3];
                stRec.ulOffset  = (ULONG)achEntry[4] |
                                  ((ULONG)achEntry[5] << 8);

                if (NeAppendExport(pNe, &stRec) != NO_ERROR)
                    return ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            /* Fixed segment entries: 3 bytes each; the bundle type
               byte carries the segment number. */
            for (i = 0; i < uchCount; i++) {
                unsigned char achEntry[3];
                struct _NE_EXPORT_REC stRec;
                unsigned char uchFlags;

                if (fread(achEntry, 1, 3, pNe->fp) != 3)
                    return ERROR_READ_FAULT;
                lPos += 3;
                usOrdinal++;

                uchFlags = achEntry[0];
                if ((uchFlags & NEENT_EXPORTED) == 0)
                    continue;

                memset(&stRec, 0, sizeof(stRec));
                stRec.usOrdinal = usOrdinal;
                stRec.ulFlags   = uchFlags;
                stRec.usSegment = uchType;
                stRec.ulOffset  = (ULONG)achEntry[1] |
                                  ((ULONG)achEntry[2] << 8);

                if (NeAppendExport(pNe, &stRec) != NO_ERROR)
                    return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    /* Resident name table (relative to the NE header). */
    if (pNe->ne.ne_restab != 0) {
        APIRET rc = NeReadNameTable(pNe,
                        pNe->lNeOffset + pNe->ne.ne_restab, TRUE);
        if (rc != NO_ERROR)
            return rc;
    }

    /* Non-resident name table (absolute file offset). */
    if (pNe->ne.ne_nrestab != 0) {
        APIRET rc = NeReadNameTable(pNe,
                        (long)pNe->ne.ne_nrestab, FALSE);
        if (rc != NO_ERROR)
            return rc;
    }

    pNe->fExportsParsed = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Open an NE executable file.
 *
 * Reads and caches the MZ header and the NE header. The handle
 * returned in @p *phNe owns the underlying FILE stream and must be
 * released with NeClose.
 *
 * @param[in]  pszPath  Path to the NE file. Not NULL.
 * @param[out] phNe     Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on failure.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phNe is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error, bad MZ magic, bad
 *                                  NE magic, or invalid e_lfanew.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see NeClose
 */
APIRET APIENTRY NeOpen(PCSZ pszPath, HNE *phNe)
{
    FILE *fp;
    struct _NE *pNe;
    struct exe_hdr mz;
    struct new_exe ne;
    long lNeOffset;

    if (!pszPath || !phNe) return ERROR_INVALID_PARAMETER;
    *phNe = NULLHANDLE;

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
    lNeOffset = (long)E_LFANEW(mz);
    if (lNeOffset <= 0) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (fseek(fp, lNeOffset, SEEK_SET) != 0) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (fread(&ne, 1, sizeof(ne), fp) != sizeof(ne)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (NE_MAGIC(ne) != NEMAGIC) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }

    pNe = (struct _NE *)calloc(1, sizeof(*pNe));
    if (!pNe) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pNe->fp = fp;
    pNe->lNeOffset = lNeOffset;
    memcpy(&pNe->mz, &mz, sizeof(mz));
    memcpy(&pNe->ne, &ne, sizeof(ne));
    pNe->paExports = NULL;
    pNe->ulExportCount = 0;
    pNe->ulExportCapacity = 0;
    pNe->fExportsParsed = FALSE;
    *phNe = (HNE)pNe;
    return NO_ERROR;
}

/*!
 * @brief Close an NE executable.
 *
 * Flushes and closes the underlying stream and releases the handle.
 * Idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 * @param[in] hNe  Handle from NeOpen. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 *
 * @see NeOpen
 */
APIRET APIENTRY NeClose(HNE hNe)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE) return NO_ERROR;
    pNe = (struct _NE *)hNe;
    if (pNe->fp) fclose(pNe->fp);
    if (pNe->paExports) free(pNe->paExports);
    free(pNe);
    return NO_ERROR;
}

/*!
 * @brief Retrieve the cached MZ header.
 *
 * Copies the MZ header cached at open time into @p pMZ. No file I/O
 * is performed.
 *
 * @param[in]  hNe   Handle from NeOpen. Not NULLHANDLE.
 * @param[out] pMZ   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE or @p pMZ is
 *                                  NULL.
 *
 * @see NeQueryHeader
 */
APIRET APIENTRY NeQueryMZHeader(HNE hNe, struct exe_hdr *pMZ)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pMZ) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    memcpy(pMZ, &pNe->mz, sizeof(*pMZ));
    return NO_ERROR;
}

/*!
 * @brief Retrieve the cached NE header.
 *
 * Copies the NE header cached at open time into @p pNE. No file I/O
 * is performed.
 *
 * @param[in]  hNe   Handle from NeOpen. Not NULLHANDLE.
 * @param[out] pNE   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE or @p pNE is
 *                                  NULL.
 *
 * @see NeQueryMZHeader
 */
APIRET APIENTRY NeQueryHeader(HNE hNe, struct new_exe *pNE)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pNE) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    memcpy(pNE, &pNe->ne, sizeof(*pNE));
    return NO_ERROR;
}

/*!
 * @brief Return the number of module references.
 *
 * @param[in]  hNe       Handle from NeOpen. Not NULLHANDLE.
 * @param[out] pusCount  Receiver for the module count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE or
 *                                  @p pusCount is NULL.
 *
 * @see NeQueryModuleName
 */
APIRET APIENTRY NeQueryModuleCount(HNE hNe, PUSHORT pusCount)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pusCount) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    *pusCount = pNe->ne.ne_cmod;
    return NO_ERROR;
}

/*!
 * @brief Return the name of a module reference.
 *
 * Reads one entry from the module reference table and resolves it
 * through the imported names table.
 *
 * @param[in]  hNe       Handle from NeOpen. Not NULLHANDLE.
 * @param[in]  usIndex   1-based module index, in [1, ne_cmod].
 * @param[out] pszName   Output buffer. Not NULL.
 * @param[in]  cbName    Size of @p pszName in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE, @p pszName
 *                                  is NULL, @p cbName is zero,
 *                                  @p usIndex is out of range, or
 *                                  the name does not fit in
 *                                  @p pszName.
 * @retval ERROR_READ_FAULT         Read error.
 *
 * @see NeQueryModuleCount
 */
APIRET APIENTRY NeQueryModuleName(HNE hNe, USHORT usIndex,
                                  PSZ pszName, ULONG cbName)
{
    struct _NE *pNe;
    WORD  usNameOffset;
    BYTE  uchLen;
    long  lOffset;

    if (hNe == NULLHANDLE || !pszName || cbName == 0)
        return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    if (usIndex == 0 || usIndex > pNe->ne.ne_cmod)
        return ERROR_INVALID_PARAMETER;

    /* Module table: WORD offsets into the imported names table. */
    lOffset = pNe->lNeOffset + pNe->ne.ne_modtab
            + (long)(usIndex - 1) * 2;
    if (fseek(pNe->fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(&usNameOffset, 1, sizeof(usNameOffset), pNe->fp)
        != sizeof(usNameOffset))
        return ERROR_READ_FAULT;

    /* Imported names table: length-prefixed string. */
    lOffset = pNe->lNeOffset + pNe->ne.ne_imptab + usNameOffset;
    if (fseek(pNe->fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(&uchLen, 1, 1, pNe->fp) != 1)
        return ERROR_READ_FAULT;

    if ((ULONG)uchLen + 1 > cbName)
        return ERROR_INVALID_PARAMETER;

    if (fread(pszName, 1, uchLen, pNe->fp) != uchLen)
        return ERROR_READ_FAULT;
    pszName[uchLen] = '\0';
    return NO_ERROR;
}

/*!
 * @brief Return the number of segments.
 *
 * @param[in]  hNe       Handle from NeOpen. Not NULLHANDLE.
 * @param[out] pusCount  Receiver for the segment count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE or
 *                                  @p pusCount is NULL.
 *
 * @see NeQuerySegment
 */
APIRET APIENTRY NeQuerySegmentCount(HNE hNe, PUSHORT pusCount)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pusCount) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    *pusCount = pNe->ne.ne_cseg;
    return NO_ERROR;
}

/*!
 * @brief Return one segment table entry.
 *
 * @param[in]  hNe      Handle from NeOpen. Not NULLHANDLE.
 * @param[in]  usIndex  1-based segment index, in [1, ne_cseg].
 * @param[out] pSeg     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE, @p pSeg is
 *                                  NULL, or @p usIndex is out of
 *                                  range.
 * @retval ERROR_READ_FAULT         Read error.
 *
 * @see NeQuerySegmentCount
 */
APIRET APIENTRY NeQuerySegment(HNE hNe, USHORT usIndex,
                               struct new_seg *pSeg)
{
    struct _NE *pNe;
    long lOffset;

    if (hNe == NULLHANDLE || !pSeg) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    if (usIndex == 0 || usIndex > pNe->ne.ne_cseg)
        return ERROR_INVALID_PARAMETER;

    lOffset = pNe->lNeOffset + pNe->ne.ne_segtab
            + (long)(usIndex - 1) * (long)sizeof(*pSeg);
    if (fseek(pNe->fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(pSeg, 1, sizeof(*pSeg), pNe->fp) != sizeof(*pSeg))
        return ERROR_READ_FAULT;
    return NO_ERROR;
}

/*!
 * @brief Return the number of relocations for a segment.
 *
 * The relocation table follows the segment data at the offset
 * computed from the segment's sector and length fields.
 *
 * @param[in]  hNe        Handle from NeOpen. Not NULLHANDLE.
 * @param[in]  usSegment  1-based segment index.
 * @param[out] pusCount   Receiver for the relocation count. Not
 *                        NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE,
 *                                  @p pusCount is NULL, or
 *                                  @p usSegment is out of range.
 * @retval ERROR_READ_FAULT         Read error.
 *
 * @see NeQueryReloc
 */
APIRET APIENTRY NeQueryRelocCount(HNE hNe, USHORT usSegment,
                                  PUSHORT pusCount)
{
    struct _NE *pNe;
    struct new_seg seg;
    long lOffset;
    WORD usCount;
    APIRET rc;

    if (hNe == NULLHANDLE || !pusCount) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;

    rc = NeQuerySegment(hNe, usSegment, &seg);
    if (rc != NO_ERROR) return rc;

    /* Relocation table immediately follows the segment data. */
    lOffset = ((long)seg.ns_sector << pNe->ne.ne_align)
            + (seg.ns_cbseg ? seg.ns_cbseg : 0x10000L);
    if (fseek(pNe->fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(&usCount, 1, sizeof(usCount), pNe->fp)
        != sizeof(usCount))
        return ERROR_READ_FAULT;

    *pusCount = usCount;
    return NO_ERROR;
}

/*!
 * @brief Return one relocation table entry.
 *
 * @param[in]  hNe        Handle from NeOpen. Not NULLHANDLE.
 * @param[in]  usSegment  1-based segment index.
 * @param[in]  usIndex    0-based relocation index.
 * @param[out] pRlc       Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE, @p pRlc is
 *                                  NULL, or @p usSegment is out of
 *                                  range.
 * @retval ERROR_READ_FAULT         Read error.
 *
 * @see NeQueryRelocCount
 */
APIRET APIENTRY NeQueryReloc(HNE hNe, USHORT usSegment,
                             USHORT usIndex,
                             struct new_rlc *pRlc)
{
    struct _NE *pNe;
    struct new_seg seg;
    long lOffset;
    APIRET rc;

    if (hNe == NULLHANDLE || !pRlc) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;

    rc = NeQuerySegment(hNe, usSegment, &seg);
    if (rc != NO_ERROR) return rc;

    lOffset = ((long)seg.ns_sector << pNe->ne.ne_align)
            + (seg.ns_cbseg ? seg.ns_cbseg : 0x10000L)
            + 2L
            + (long)usIndex * (long)sizeof(*pRlc);
    if (fseek(pNe->fp, lOffset, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(pRlc, 1, sizeof(*pRlc), pNe->fp) != sizeof(*pRlc))
        return ERROR_READ_FAULT;
    return NO_ERROR;
}

/*!
 * @brief Return the module name of this NE image.
 *
 * The module name is the first entry of the Resident Name Table,
 * stored as a length-prefixed string without a terminating NUL.
 * This function copies it into @p pszName as a NUL-terminated
 * string.
 *
 * The local name buffer is 256 bytes; a length-prefixed string can
 * hold at most 255 characters, so no truncation is possible.
 *
 * @param[in]  hNe      Handle from NeOpen. Not NULLHANDLE.
 * @param[out] pszName  Output buffer. Not NULL.
 * @param[in]  cbName   Size of @p pszName in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe is NULLHANDLE, @p pszName
 *                                  is NULL, or @p cbName is zero.
 * @retval ERROR_BUFFER_OVERFLOW    Name does not fit in @p pszName.
 * @retval ERROR_READ_FAULT         Read error.
 *
 * @see NeQueryModuleName
 */
APIRET APIENTRY NeQuerySelfModuleName(HNE hNe, PSZ pszName, ULONG cbName)
{
    struct _NE *pNe;
    long  lPos;
    BYTE  uchLen;
    char  achName[256];

    if (hNe == NULLHANDLE || !pszName || cbName == 0)
        return ERROR_INVALID_PARAMETER;

    pNe = (struct _NE *)hNe;
    lPos = pNe->lNeOffset + pNe->ne.ne_restab;

    if (fseek(pNe->fp, lPos, SEEK_SET) != 0)
        return ERROR_READ_FAULT;
    if (fread(&uchLen, 1, 1, pNe->fp) != 1)
        return ERROR_READ_FAULT;

    if (uchLen == 0) {
        pszName[0] = '\0';
        return NO_ERROR;
    }
    if ((ULONG)uchLen + 1 > cbName)
        return ERROR_BUFFER_OVERFLOW;
    if (fread(achName, 1, uchLen, pNe->fp) != uchLen)
        return ERROR_READ_FAULT;
    achName[uchLen] = '\0';
    memcpy(pszName, achName, (size_t)uchLen + 1);
    return NO_ERROR;
}

/*!
 * @brief Bind a DOS stub and an NE image into one file.
 *
 * Copies @p pszStub into @p pszOutput, pads the output to the NE
 * segment alignment, copies the NE image from @p pszInput, and
 * patches the e_lfanew field of the MZ header, the segment sector
 * offsets, and the non-resident name table offset.
 *
 * @param[in] pszStub    Path to the DOS stub file. Not NULL.
 * @param[in] pszInput   Path to the input NE file. Not NULL.
 * @param[in] pszOutput  Path to the output file. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_OPEN_FAILED        A file cannot be opened.
 * @retval ERROR_READ_FAULT         Read error in stub or input, or
 *                                  invalid e_lfanew in the input.
 * @retval ERROR_WRITE_FAULT        Write error in the output.
 */
APIRET APIENTRY NeBind(PCSZ pszStub, PCSZ pszInput,
                       PCSZ pszOutput)
{
    FILE          *fstub = NULL;
    FILE          *fin   = NULL;
    FILE          *fout  = NULL;
    struct exe_hdr mz;
    struct new_exe ne;
    long           lNeOffset;
    long           lPos;
    long           lEnd;
    long           lDelta;
    char           buffer[1024];
    ULONG          cb;
    USHORT         usSegCount;
    USHORT         i;
    struct new_seg seg;
    APIRET         rc = NO_ERROR;

    if (!pszStub || !pszInput || !pszOutput)
        return ERROR_INVALID_PARAMETER;

    fout = fopen(pszOutput, "wb");
    if (!fout) return ERROR_OPEN_FAILED;

    fstub = fopen(pszStub, "rb");
    if (!fstub) { rc = ERROR_OPEN_FAILED; goto cleanup; }

    while ((cb = (ULONG)fread(buffer, 1, sizeof(buffer), fstub)) > 0) {
        if (fwrite(buffer, 1, cb, fout) != cb) {
            rc = ERROR_WRITE_FAULT; goto cleanup;
        }
    }
    fclose(fstub);
    fstub = NULL;

    fin = fopen(pszInput, "rb");
    if (!fin) { rc = ERROR_OPEN_FAILED; goto cleanup; }

    if (fread(&mz, 1, sizeof(mz), fin) != sizeof(mz)) {
        rc = ERROR_READ_FAULT; goto cleanup;
    }
    lNeOffset = (long)E_LFANEW(mz);
    if (lNeOffset <= 0) { rc = ERROR_READ_FAULT; goto cleanup; }
    if (fseek(fin, lNeOffset, SEEK_SET) != 0) {
        rc = ERROR_READ_FAULT; goto cleanup;
    }
    if (fread(&ne, 1, sizeof(ne), fin) != sizeof(ne)) {
        rc = ERROR_READ_FAULT; goto cleanup;
    }

    /* Pad output to the NE segment alignment. */
    lPos = ftell(fout);
    if (lPos < 0) { rc = ERROR_WRITE_FAULT; goto cleanup; }
    cb = (ULONG)((((lPos >> ne.ne_align) + 1)
                        << ne.ne_align) - lPos);
    if (cb > 0) {
        memset(buffer, 0, sizeof(buffer));
        while (cb > 0) {
            ULONG cbChunk = (cb < sizeof(buffer))
                          ? cb : (ULONG)sizeof(buffer);
            if (fwrite(buffer, 1, cbChunk, fout) != cbChunk) {
                rc = ERROR_WRITE_FAULT; goto cleanup;
            }
            cb -= cbChunk;
        }
    }

    lEnd = ftell(fout);
    if (lEnd < 0) { rc = ERROR_WRITE_FAULT; goto cleanup; }

    /* Patch e_lfanew in the output MZ header. */
    if (fseek(fout, 0x3cL, SEEK_SET) != 0) {
        rc = ERROR_WRITE_FAULT; goto cleanup;
    }
    {
        DWORD dwEnd = (DWORD)lEnd;
        if (fwrite(&dwEnd, 1, 4, fout) != 4) {
            rc = ERROR_WRITE_FAULT; goto cleanup;
        }
    }
    if (fseek(fout, lEnd, SEEK_SET) != 0) {
        rc = ERROR_WRITE_FAULT; goto cleanup;
    }

    /* Sector shift between the old and new NE positions. */
    lDelta = (lNeOffset >> ne.ne_align) - (lEnd >> ne.ne_align);

    /* Copy the NE image verbatim. */
    if (fseek(fin, lNeOffset, SEEK_SET) != 0) {
        rc = ERROR_READ_FAULT; goto cleanup;
    }
    while ((cb = (ULONG)fread(buffer, 1, sizeof(buffer), fin)) > 0) {
        if (fwrite(buffer, 1, cb, fout) != cb) {
            rc = ERROR_WRITE_FAULT; goto cleanup;
        }
    }

    /* Patch the segment table in the output image. */
    usSegCount = ne.ne_cseg;
    for (i = 1; i <= usSegCount; i++) {
        long lInOff  = lNeOffset + ne.ne_segtab
                     + (long)(i - 1) * (long)sizeof(seg);
        long lOutOff = lEnd + ne.ne_segtab
                     + (long)(i - 1) * (long)sizeof(seg);

        if (fseek(fin, lInOff, SEEK_SET) != 0) {
            rc = ERROR_READ_FAULT; goto cleanup;
        }
        if (fread(&seg, 1, sizeof(seg), fin) != sizeof(seg)) {
            rc = ERROR_READ_FAULT; goto cleanup;
        }
        if (seg.ns_sector) {
            seg.ns_sector = (WORD)(seg.ns_sector - lDelta);
            if (fseek(fout, lOutOff, SEEK_SET) != 0) {
                rc = ERROR_WRITE_FAULT; goto cleanup;
            }
            if (fwrite(&seg, 1, sizeof(seg), fout) != sizeof(seg)) {
                rc = ERROR_WRITE_FAULT; goto cleanup;
            }
        }
    }

    /* Patch the non-resident name table offset. */
    if (fseek(fout, lEnd + 0x2cL, SEEK_SET) != 0) {
        rc = ERROR_WRITE_FAULT; goto cleanup;
    }
    {
        DWORD dwNr = (DWORD)(ne.ne_nrestab
                             - (DWORD)(lNeOffset - lEnd));
        if (fwrite(&dwNr, 1, 4, fout) != 4) {
            rc = ERROR_WRITE_FAULT; goto cleanup;
        }
    }

cleanup:
    if (fstub) fclose(fstub);
    if (fin)   fclose(fin);
    if (fout)  fclose(fout);
    return rc;
}

/* ------------------------------------------------------------------ */
/* NE export enumeration API                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Open a cursor on the first export of an NE module.
 *
 * The Entry Table and both name tables are parsed on first use and
 * cached in the handle. Subsequent accessors read from the cache
 * without touching the disk.
 *
 * @param[in]  hNe       Handle from NeOpen. Not NULLHANDLE.
 * @param[out] phEnum    Cursor receiver. Not NULL. Set to NULLHANDLE
 *                       on error or when the module has no exports.
 * @param[out] pulCount  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe or @p phEnum is NULL.
 * @retval ERROR_NO_MORE_ITEMS      No exports. *phEnum = NULLHANDLE.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 *
 * @see NeQueryExportNext
 * @see NeQueryExportClose
 */
APIRET APIENTRY NeQueryExportFirst(HNE hNe, HNEENUM *phEnum,
                                   PULONG pulCount)
{
    struct _NE      *pNe;
    struct _NE_ENUM *pEnum;
    APIRET           rc;

    if (hNe == NULLHANDLE || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pNe = (struct _NE *)hNe;
    *phEnum = NULLHANDLE;

    rc = NeParseExports(pNe);
    if (rc != NO_ERROR)
        return rc;

    if (pulCount != NULL)
        *pulCount = pNe->ulExportCount;

    if (pNe->ulExportCount == 0)
        return ERROR_NO_MORE_ITEMS;

    pEnum = (struct _NE_ENUM *)malloc(sizeof(*pEnum));
    if (pEnum == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->pOwner  = pNe;
    pEnum->ulIndex = 0;

    *phEnum = (HNEENUM)pEnum;
    return NO_ERROR;
}

/*!
 * @brief Advance an export cursor to the next export.
 *
 * @param[in] hEnum  Cursor from NeQueryExportFirst or one of the
 *                   find-by functions. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     @p hEnum is NULLHANDLE.
 * @retval ERROR_NO_MORE_ITEMS      No more exports.
 */
APIRET APIENTRY NeQueryExportNext(HNEENUM hEnum)
{
    struct _NE_ENUM *pEnum = (struct _NE_ENUM *)hEnum;

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
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 */
APIRET APIENTRY NeQueryExportClose(HNEENUM hEnum)
{
    if (hEnum != NULLHANDLE)
        free(hEnum);
    return NO_ERROR;
}

/*!
 * @brief Position a new cursor on an export by name.
 *
 * @param[in]  hNe      Handle. Not NULLHANDLE.
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
APIRET APIENTRY NeQueryExportByName(HNE hNe, PCSZ pszName,
                                    HNEENUM *phEnum)
{
    struct _NE *pNe;
    ULONG       i;
    APIRET      rc;

    if (hNe == NULLHANDLE || pszName == NULL || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pNe = (struct _NE *)hNe;
    *phEnum = NULLHANDLE;

    rc = NeParseExports(pNe);
    if (rc != NO_ERROR)
        return rc;

    for (i = 0; i < pNe->ulExportCount; i++) {
        if (pNe->paExports[i].fNamed &&
            strcmp(pNe->paExports[i].achName, pszName) == 0) {
            struct _NE_ENUM *pEnum =
                (struct _NE_ENUM *)malloc(sizeof(*pEnum));
            if (pEnum == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            pEnum->pOwner  = pNe;
            pEnum->ulIndex = i;
            *phEnum = (HNEENUM)pEnum;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Position a new cursor on an export by ordinal.
 *
 * @param[in]  hNe        Handle. Not NULLHANDLE.
 * @param[in]  usOrdinal  Ordinal to find (1-based).
 * @param[out] phEnum     Cursor receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hNe or @p phEnum is NULL.
 * @retval ERROR_FILE_NOT_FOUND     Ordinal is not exported.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY NeQueryExportByOrdinal(HNE hNe, USHORT usOrdinal,
                                       HNEENUM *phEnum)
{
    struct _NE *pNe;
    ULONG       i;
    APIRET      rc;

    if (hNe == NULLHANDLE || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pNe = (struct _NE *)hNe;
    *phEnum = NULLHANDLE;

    rc = NeParseExports(pNe);
    if (rc != NO_ERROR)
        return rc;

    for (i = 0; i < pNe->ulExportCount; i++) {
        if (pNe->paExports[i].usOrdinal == usOrdinal) {
            struct _NE_ENUM *pEnum =
                (struct _NE_ENUM *)malloc(sizeof(*pEnum));
            if (pEnum == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            pEnum->pOwner  = pNe;
            pEnum->ulIndex = i;
            *phEnum = (HNEENUM)pEnum;
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Retrieve the name of the current export.
 *
 * Size-query convention as described in newexe.h. For anonymous
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
APIRET APIENTRY NeQueryExportName(HNEENUM hEnum,
                                  PSZ pszBuf, ULONG ulSize,
                                  PULONG pulUsed)
{
    struct _NE_ENUM *pEnum = (struct _NE_ENUM *)hEnum;
    const struct _NE_EXPORT_REC *pRec;
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
APIRET APIENTRY NeQueryExportOrdinal(HNEENUM hEnum, PUSHORT pusOrdinal)
{
    struct _NE_ENUM *pEnum = (struct _NE_ENUM *)hEnum;

    if (pEnum == NULL || pusOrdinal == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pusOrdinal = pEnum->pOwner->paExports[pEnum->ulIndex].usOrdinal;
    return NO_ERROR;
}

/*!
 * @brief Retrieve the raw Entry Table flags of the current export.
 *
 * @param[in]  hEnum     Cursor. Not NULLHANDLE.
 * @param[out] pulFlags  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY NeQueryExportFlags(HNEENUM hEnum, PULONG pulFlags)
{
    struct _NE_ENUM *pEnum = (struct _NE_ENUM *)hEnum;

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
APIRET APIENTRY NeQueryExportNamed(HNEENUM hEnum, PBOOL pfNamed)
{
    struct _NE_ENUM *pEnum = (struct _NE_ENUM *)hEnum;

    if (pEnum == NULL || pfNamed == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pfNamed = pEnum->pOwner->paExports[pEnum->ulIndex].fNamed;
    return NO_ERROR;
}

/*!
 * @brief Query whether the current export is a forwarder.
 *
 * Always FALSE for NE; present for symmetry with the LX reader.
 *
 * @param[in]  hEnum        Cursor. Not NULLHANDLE.
 * @param[out] pfForwarder  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 */
APIRET APIENTRY NeQueryExportForwarder(HNEENUM hEnum,
                                       PBOOL pfForwarder)
{
    struct _NE_ENUM *pEnum = (struct _NE_ENUM *)hEnum;

    if (pEnum == NULL || pfForwarder == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;
    *pfForwarder = FALSE;
    return NO_ERROR;
}

/*!
 * @brief Determine the calling convention of the current export.
 *
 * Reads the first bytes of the function body from the segment that
 * contains the entry point and walks them instruction by instruction
 * with X86Decode. The first ret-family opcode encountered selects
 * the convention:
 *
 *   - C3        ret        -> "_System"
 *   - C2 xx xx  ret imm16  -> "_Pascal"
 *   - CB        retf       -> "_System"
 *   - CA xx xx  retf imm16 -> "_Far16 _Pascal"
 *
 * If the decoder hits an unrecognized instruction, or if the scan
 * reaches the end of the available bytes without finding a ret, the
 * default convention "_System" is returned. NE does not store the
 * calling convention explicitly; the choice is a heuristic.
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
APIRET APIENTRY NeQueryExportConvention(HNEENUM hEnum,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed)
{
    struct _NE_ENUM *pEnum;
    const struct _NE_EXPORT_REC *pRec;
    struct new_seg seg;
    long           lSegStart;
    ULONG          ulSegLen;
    ULONG          ulScan;
    ULONG          ulRead;
    UCHAR         *puchBody;
    BOOL           f32;
    ULONG          pos;
    PCSZ           pszConv;
    size_t         cb;
    APIRET         rc;

    if (hEnum == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    pEnum = (struct _NE_ENUM *)hEnum;
    if (pEnum->ulIndex >= pEnum->pOwner->ulExportCount)
        return ERROR_INVALID_PARAMETER;

    pRec = &pEnum->pOwner->paExports[pEnum->ulIndex];
    pszConv = "_System";
    puchBody = NULL;

    rc = NeQuerySegment((HNE)pEnum->pOwner, pRec->usSegment, &seg);
    if (rc == NO_ERROR) {
        lSegStart = ((long)seg.ns_sector)
                    << pEnum->pOwner->ne.ne_align;
        ulSegLen = seg.ns_cbseg ? seg.ns_cbseg : 0x10000UL;

        if (pRec->ulOffset < ulSegLen) {
            ulScan = ulSegLen - pRec->ulOffset;
            if (ulScan > 8192)
                ulScan = 8192;

            puchBody = (UCHAR *)malloc(ulScan);
            if (puchBody == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;

            if (fseek(pEnum->pOwner->fp,
                      lSegStart + (long)pRec->ulOffset,
                      SEEK_SET) == 0) {
                ulRead = (ULONG)fread(puchBody, 1, ulScan,
                                      pEnum->pOwner->fp);
                f32 = (seg.ns_flags & NS32BIT) ? TRUE : FALSE;

                pos = 0;
                while (pos < ulRead) {
                    X86_INSN insn;

                    if (X86Decode(puchBody + pos, ulRead - pos,
                                  f32, &insn) != NO_ERROR)
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
                        pszConv = insn.usImm ? "_Pascal"
                                             : "_System";
                        break;
                    }
                    if (insn.ulFamily == X86_FAM_RETF_IMM) {
                        pszConv = insn.usImm ? "_Far16 _Pascal"
                                             : "_System";
                        break;
                    }
                    pos += insn.ulLength;
                }
            }
            free(puchBody);
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
