/*! newexe.c - New Executable (NE) file access (C89)
 *
 *  See newexe.h for the public API.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "newexe.h"

/*! @file newexe.c
 *  @brief Implementation of the NE file access library.
 *
 *  All public functions declared in newexe.h are implemented here.
 *  The internal representation of HNE is defined in this translation
 *  unit only; callers see it as an opaque HANDLE.
 */

/*! @brief Internal representation behind HNE.
 *
 *  Not exposed to callers. newexe.h declares the handle as HANDLE,
 *  so the layout of this structure may change freely.
 */
struct _NE {
    FILE          *fp;        /*!< Underlying file stream. */
    long           lNeOffset; /*!< File offset of the NE header. */
    struct exe_hdr mz;        /*!< Cached MZ header. */
    struct new_exe ne;        /*!< Cached NE header. */
};

/*! @brief Open an NE executable file.
 *
 *  @copydetails NeOpen
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

/*! @brief Close an NE executable.
 *
 *  @copydetails NeClose
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

/*! @brief Retrieve the cached MZ header.
 *
 *  @copydetails NeQueryMZHeader
 */
APIRET APIENTRY NeQueryMZHeader(HNE hNe, struct exe_hdr *pMZ)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pMZ) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    memcpy(pMZ, &pNe->mz, sizeof(*pMZ));
    return NO_ERROR;
}

/*! @brief Retrieve the cached NE header.
 *
 *  @copydetails NeQueryHeader
 */
APIRET APIENTRY NeQueryHeader(HNE hNe, struct new_exe *pNE)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pNE) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    memcpy(pNE, &pNe->ne, sizeof(*pNE));
    return NO_ERROR;
}

/*! @brief Return the number of module references.
 *
 *  @copydetails NeQueryModuleCount
 */
APIRET APIENTRY NeQueryModuleCount(HNE hNe, PUSHORT pusCount)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pusCount) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    *pusCount = pNe->ne.ne_cmod;
    return NO_ERROR;
}

/*! @brief Return the name of a module reference.
 *
 *  @copydetails NeQueryModuleName
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

/*! @brief Return the number of segments.
 *
 *  @copydetails NeQuerySegmentCount
 */
APIRET APIENTRY NeQuerySegmentCount(HNE hNe, PUSHORT pusCount)
{
    struct _NE *pNe;

    if (hNe == NULLHANDLE || !pusCount) return ERROR_INVALID_PARAMETER;
    pNe = (struct _NE *)hNe;
    *pusCount = pNe->ne.ne_cseg;
    return NO_ERROR;
}

/*! @brief Return one segment table entry.
 *
 *  @copydetails NeQuerySegment
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

/*! @brief Return the number of relocations for a segment.
 *
 *  @copydetails NeQueryRelocCount
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

/*! @brief Return one relocation table entry.
 *
 *  @copydetails NeQueryReloc
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

/*! @brief Bind a DOS stub and an NE image into one file.
 *
 *  @copydetails NeBind
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
