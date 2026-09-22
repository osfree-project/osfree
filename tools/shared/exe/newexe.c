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
};

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

    pNe = (struct _NE *)malloc(sizeof(*pNe));
    if (!pNe) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pNe->fp = fp;
    pNe->lNeOffset = lNeOffset;
    memcpy(&pNe->mz, &mz, sizeof(mz));
    memcpy(&pNe->ne, &ne, sizeof(ne));
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
