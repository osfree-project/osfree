
/*
 *@@sourcefile mmpmh.c:
 *
 *      Usage: OS/2 PM programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  mmh*
 *
 *      This file is new with V1.0.1 and contains some
 *      code formerly in XWorkplace multimedia.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\mmpmh.h"
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

/*
 *      Copyright (C) 1997-2005 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_DOSPROFILE
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSERRORS

#define INCL_GPI                // required for INCL_MMIO_CODEC
#define INCL_GPIBITMAPS         // required for INCL_MMIO_CODEC
#include <os2.h>

#define INCL_MCIOS2
#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

#include <stdio.h>
#include <setjmp.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/dosh.h"
#include "helpers/except.h"
#include "helpers/gpih.h"
#include "helpers/mmpmh.h"
#include "helpers/standards.h"

#pragma hdrstop

/*
 *@@category: Helpers\Multimedia helpers
 *      See nls.c.
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

extern FNTD_MCISENDCOMMAND              *G_mciSendCommand = NULL;
extern FNTD_MCIGETERRORSTRING           *G_mciGetErrorString = NULL;

extern FNTD_MMIOINIFILECODEC            *G_mmioIniFileCODEC = NULL;
extern FNTD_MMIOQUERYCODECNAMELENGTH    *G_mmioQueryCODECNameLength = NULL;
extern FNTD_MMIOQUERYCODECNAME          *G_mmioQueryCODECName = NULL;
extern FNTD_MMIOINIFILEHANDLER          *G_mmioIniFileHandler = NULL;
extern FNTD_MMIOQUERYFORMATCOUNT        *G_mmioQueryFormatCount = NULL;
extern FNTD_MMIOGETFORMATS              *G_mmioGetFormats = NULL;
extern FNTD_MMIOGETFORMATNAME           *G_mmioGetFormatName = NULL;

extern FNTD_MMIOGETHEADER               *G_mmioGetHeader = NULL;
extern FNTD_MMIOREAD                    *G_mmioRead = NULL;
extern FNTD_MMIOIDENTIFYFILE            *G_mmioIdentifyFile = NULL;
extern FNTD_MMIOOPEN                    *G_mmioOpen = NULL;
extern FNTD_MMIOQUERYHEADERLENGTH       *G_mmioQueryHeaderLength = NULL;
extern FNTD_MMIOCLOSE                   *G_mmioClose = NULL;

BOOL G_fFuncsResolved = FALSE;

/*
 * G_aResolveFromMDM:
 *      functions imported from MDM.DLL.
 *      Used with doshResolveImports.
 */

static const RESOLVEFUNCTION G_aResolveFromMDM[] =
    {
        "mciSendCommand", (PFN*)&G_mciSendCommand,
        "mciGetErrorString", (PFN*)&G_mciGetErrorString
    };

/*
 * G_aResolveFromMMIO:
 *      functions resolved from MMIO.DLL.
 *      Used with doshResolveImports.
 */

static const RESOLVEFUNCTION G_aResolveFromMMIO[] =
    {
        "mmioIniFileCODEC", (PFN*)&G_mmioIniFileCODEC,
        "mmioQueryCODECNameLength", (PFN*)&G_mmioQueryCODECNameLength,
        "mmioQueryCODECName", (PFN*)&G_mmioQueryCODECName,
        "mmioIniFileHandler", (PFN*)&G_mmioIniFileHandler,
        "mmioQueryFormatCount", (PFN*)&G_mmioQueryFormatCount,
        "mmioGetFormats", (PFN*)&G_mmioGetFormats,
        "mmioGetFormatName", (PFN*)&G_mmioGetFormatName,
        "mmioGetHeader", (PFN*)&G_mmioGetHeader,
        "mmioRead", (PFN*)&G_mmioRead,
        "mmioIdentifyFile", (PFN*)&G_mmioIdentifyFile,
        "mmioOpen", (PFN*)&G_mmioOpen,
        "mmioQueryHeaderLength", (PFN*)&G_mmioQueryHeaderLength,
        "mmioClose", (PFN*)&G_mmioClose,
    };

/* ******************************************************************
 *
 *   Initialization
 *
 ********************************************************************/

/*
 *@@ mmhInit:
 *      initializes the MMPM helpers by resolving
 *      some MMPM/2 functions dynamically from the
 *      MDM.DLL and MMIO.DLL libraries.
 */

APIRET mmhInit(VOID)
{
    APIRET arc;

    HMODULE hmodMDM = NULLHANDLE,
            hmodMMIO = NULLHANDLE;

    if (!(arc = doshResolveImports("MDM.DLL",
                                   &hmodMDM,
                                   G_aResolveFromMDM,
                                   ARRAYITEMCOUNT(G_aResolveFromMDM))))
        if (!(arc = doshResolveImports("MMIO.DLL",
                                       &hmodMMIO,
                                       G_aResolveFromMMIO,
                                       ARRAYITEMCOUNT(G_aResolveFromMMIO))))
            G_fFuncsResolved = TRUE;

    return arc;
}

/* ******************************************************************
 *
 *   IOProcs info
 *
 ********************************************************************/

/*
 *@@ mmhGetIOProcs:
 *      allocates and fills an array of MMFORMATINFO
 *      structures describing the IOProcs that are
 *      installed on the system.
 *
 *      On input, specify any combination of these
 *      flags to filter the type of IOProcs returned:
 *
 *      --  MMIO_MEDIATYPE_AUDIO            (0x00000002)
 *
 *      --  MMIO_MEDIATYPE_IMAGE            (0x00000001)
 *
 *      --  MMIO_MEDIATYPE_DIGITALVIDEO     (0x00000040)
 *
 *      --  MMIO_MEDIATYPE_MIDI             (0x00000004)
 *
 *      --  MMIO_MEDIATYPE_MOVIE            (0x00000100)
 *
 *      --  MMIO_MEDIATYPE_ANIMATION        (0x00000080)
 *
 *      --  MMIO_MEDIATYPE_COMPOUND         (0x00000008)
 *
 *      --  MMIO_MEDIATYPE_OTHER            (0x00000010)
 *
 *      --  MMIO_MEDIATYPE_UNKNOWN          (0x00000020)
 *
 *      If none of these are specified, all known formats
 *      are returned.
 *
 *      If NO_ERROR is returned, *ppaInfos has been
 *      set to a newly allocated array, which the caller
 *      must free().
 *
 *      Otherwise this returns one of the following
 *      error codes:
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  MMIOERR_INVALID_PARAMETER
 *
 *      --  MMIOERR_INTERNAL_SYSTEM
 *
 *      --  MMIO_ERROR: something else went wrong.
 */

APIRET mmhGetIOProcs(ULONG flMediaType,         // in: MMIO_MEDIATYPE_* flags
                     PMMFORMATINFO *ppaInfos,   // out: new array of formats
                     PLONG pcFormats)           // out: no. of items in array (not array size!)
{
    APIRET          arc;
    MMFORMATINFO    mmfi;
    LONG            cFormats;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

    memset(&mmfi, 0, sizeof(mmfi));     // zeroed struct means get all
    mmfi.ulMediaType = flMediaType;
    mmfi.ulStructLen = sizeof(mmfi);

    if (!(arc = G_mmioQueryFormatCount(&mmfi,
                                       &cFormats,
                                       0,     // "reserved"
                                       0)))    // "reserved"
    {
        PMMFORMATINFO   pammfi;
        if (!(pammfi = (PMMFORMATINFO)malloc(cFormats * sizeof(MMFORMATINFO))))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            LONG    lFormatsRead = 0;
            if (!(arc = G_mmioGetFormats(&mmfi,
                                         cFormats,
                                         pammfi,
                                         pcFormats,
                                         0,     // reserved
                                         0)))     // reserved
                *ppaInfos = pammfi;
            else
                free(pammfi);
        }
    }

    return arc;
}

/* ******************************************************************
 *
 *   File identification
 *
 ********************************************************************/

/*
 *@@ mmhIdentifyFile:
 *      attempts to find an IOProc that can read the specified
 *      file.
 *
 *      If NO_ERROR is returned, this fills the given MMFORMATINFO
 *      with data about the given file.
 *
 *      Otherwise this returns:
 *
 *      --  MMHERR_MMPM_NOT_INITIALIZED: mmhInit not called or
 *          it failed.
 *
 *      --  MMHERR_NO_IOPROC: no IOProc is installed
 *          to handle this format.
 */

APIRET mmhIdentifyFile(PCSZ pcszFilename,       // in: file to check
                       PMMFORMATINFO pmmfi)     // out: file format info
{
    APIRET  arc;
    FOURCC  fccStorageSystem;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

    if (!(arc = G_mmioIdentifyFile(pcszFilename,
                                   NULL,    // needed for RIFF only
                                   pmmfi,   // out: format info (FOURCC)
                                   &fccStorageSystem, // out: FOURCC of storage IOProc
                                   0L,         // reserved
                                   0L)))       // can be:
                                    /*  MMIO_FORCE_IDENTIFY_SS
                                            Forces the identification of a storage
                                            system by ignoring the file
                                            name and actually checking the MMIO Manager's
                                            I/O procedure list.
                                        MMIO_FORCE_IDENTIFY_FF
                                            Forces the identification  of a file
                                            format by ignoring the file name
                                            and actually checking the MMIO Manager's
                                            I/O procedure list. */
    {
        // if mmioIdentifyFile did not find a custom-written IO proc which
        // can understand the image file, then it will return the DOS IO Proc
        // info because the image file IS a DOS file.
        if (    (pmmfi->fccIOProc == FOURCC_DOS)
             || (!(pmmfi->ulFlags & MMIO_CANREADTRANSLATED))
           )
            arc = MMHERR_NO_IOPROC;
    }

    return arc;
}

/* ******************************************************************
 *
 *   Image loading
 *
 ********************************************************************/

/*
 *@@ mmhOpenImage:
 *
 */

APIRET mmhOpenImage(PCSZ pcszFilename,  // in: filename to load
                    FOURCC fccIOProc,   // in: FOURCC of ioproc to use; if 0, we call mmhIdentifyFile here
                    HMMIO *phmmio)      // out: mmio handle
{
    APIRET  arc = NO_ERROR;

    if (    (!pcszFilename)
         || (!phmmio)
       )
        return ERROR_INVALID_PARAMETER;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

#ifdef __DEBUG__
    TRY_LOUD(excpt1)
#else
    TRY_QUIET(excpt1)
#endif
    {
        if (!fccIOProc)
        {
            MMFORMATINFO mmfi;
            if (!(arc = mmhIdentifyFile(pcszFilename, &mmfi)))
                // ensure this is an IMAGE IOproc
                if (mmfi.ulMediaType != MMIO_MEDIATYPE_IMAGE)
                    arc = MMHERR_NOT_IMAGE_FILE;
                else
                    fccIOProc = mmfi.fccIOProc;
        }

        if (!arc)
        {
            MMIOINFO      mmioinfo;

            memset(&mmioinfo, 0, sizeof(MMIOINFO));
            mmioinfo.fccIOProc = fccIOProc;
            mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;

            if (!(*phmmio = G_mmioOpen(pcszFilename,
                                       &mmioinfo,
                                       MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY)))
                arc = MMHERR_OPEN_FAILED;
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    return arc;
}

/*
 *@@ mmhLoadImageHeader:
 *
 */

APIRET mmhLoadImageHeader(HMMIO hmmio,              // in: mmio handle from mmhOpenImage
                          PMMIMAGEHEADER pmmih,     // out: mmio image header
                          PULONG pcbBytesPerRow)    // out: padded bytes for each bitmap scan line
{
    APIRET  arc = NO_ERROR;

    if (    (!hmmio)
         || (!pmmih)
         || (!pcbBytesPerRow)
       )
        return ERROR_INVALID_PARAMETER;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

#ifdef __DEBUG__
    TRY_LOUD(excpt1)
#else
    TRY_QUIET(excpt1)
#endif
    {
        ULONG           cbHeader;
        LONG            lBytesRead;
        if (    (G_mmioQueryHeaderLength(hmmio,
                                         (PLONG)&cbHeader,
                                         0L,
                                         0L))
             || (cbHeader != sizeof (MMIMAGEHEADER))
             || (G_mmioGetHeader(hmmio,
                                 pmmih,
                                 sizeof(MMIMAGEHEADER),
                                 &lBytesRead,
                                 0,
                                 0))
           )
            arc = MMHERR_INCOMPATIBLE_HEADER;
        else
        {
            /*
             *  Determine the number of bytes required, per row.
             *      PLANES MUST ALWAYS BE = 1
             */

            ULONG   cRowBits;
            // ULONG   cbBytesPerRow;

            ULONG   cbPadding;

            // szlSourceBmp.cx = mmih.mmXDIBHeader.BMPInfoHeader2.cx;
            // szlSourceBmp.cy = mmih.mmXDIBHeader.BMPInfoHeader2.cy;

            // sBitCount = mmih.mmXDIBHeader.BMPInfoHeader2.cBitCount;

            /*
             *  Account for odd bits used in 1bpp or 4bpp images that are
             *  NOT on byte boundaries.
             */

            cRowBits =   pmmih->mmXDIBHeader.BMPInfoHeader2.cx
                       * pmmih->mmXDIBHeader.BMPInfoHeader2.cBitCount;
            *pcbBytesPerRow = cRowBits >> 3;

            if (cRowBits % 8)
                ++(*pcbBytesPerRow);

            /*
             *  Ensure the row length in bytes accounts for byte padding.
             *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
             *  The data FROM an IOProc should always appear in this form.
             */

            if (cbPadding = ((*pcbBytesPerRow) % 4))
                *pcbBytesPerRow += 4 - cbPadding;
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    return arc;
}

/*
 *@@ mmhLoadImageBits:
 *
 */

APIRET mmhLoadImageBits(HMMIO hmmio,            // in: mmio handle from mmhOpenImage
                        PMMIMAGEHEADER pmmih,   // in: image header from mmhLoadImageHeader
                        ULONG cbBytesPerRow,    // in: padded bytes per scanline from mmhLoadImageHeader
                        PBYTE *ppbBitmapBits)   // out: bitmap data allocated via DosAllocMem
{
    APIRET  arc = NO_ERROR;

    if (    (!ppbBitmapBits)
         || (!cbBytesPerRow)
         || (!pmmih)
         || (!hmmio)
       )
        return ERROR_INVALID_PARAMETER;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

    *ppbBitmapBits = NULL;

#ifdef __DEBUG__
    TRY_LOUD(excpt1)
#else
    TRY_QUIET(excpt1)
#endif
    {
        ULONG cScanLines = pmmih->mmXDIBHeader.BMPInfoHeader2.cy;

        if (!(arc = DosAllocMem((PPVOID)ppbBitmapBits,
                                cbBytesPerRow * cScanLines,
                                PAG_COMMIT | PAG_READ | PAG_WRITE)))
        {
            ULONG   ulRowThis;
            PBYTE   pbCurrent = *ppbBitmapBits;

            // load the bitmap from the file,
            // one line at a time, starting from the BOTTOM
            for (ulRowThis = 0;
                 ulRowThis < cScanLines;
                 ulRowThis++)
            {
                ULONG   cbRead;

                if (!(cbRead = G_mmioRead(hmmio,
                                          pbCurrent,
                                          cbBytesPerRow)))
                    // 0 means "done" here:
                    break;
                else if (cbRead == MMIO_ERROR)
                {
                    arc = MMHERR_MMIOREAD_FAILED;
                    break;
                }

                pbCurrent += cbBytesPerRow;
            }
        } // end if (DosAllocMem((PPVOID)&pRowBuffer,
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (    (arc)
         && (*ppbBitmapBits)
       )
    {
        DosFreeMem(*ppbBitmapBits);
        *ppbBitmapBits = NULL;
    }

    return arc;
}

/*
 *@@ mmhCreateBitmapFromBits:
 *
 *@@added V1.0.1 (2003-01-29) [umoeller]
 */

APIRET mmhCreateBitmapFromBits(PSIZEL pszlTarget,   // in: desired size of new bitmap or NULL for no resize
                               PBITMAPINFOHEADER2 pbmihSource,  // in: source bitmap format
                               PBYTE pbBitmapBits,  // in: source bitmap data
                               HBITMAP *phbmOut)   // out: newly created bitmap
{
    APIRET  arc = NO_ERROR;

    volatile HDC     hdcMem = NULLHANDLE; // XWP V1.0.4 (2005-10-09) [pr]
    volatile HPS     hpsMem = NULLHANDLE;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

    if (    (!pbmihSource)
         || (!pbBitmapBits)
         || (!phbmOut)
       )
        return ERROR_INVALID_PARAMETER;

    *phbmOut = NULLHANDLE;

#ifdef __DEBUG__
    TRY_LOUD(excpt1)
#else
    TRY_QUIET(excpt1)
#endif
    {
        SIZEL   szlPS;

        if (pszlTarget)
        {
            // caller wants to scale:
            szlPS.cx = pszlTarget->cx;
            szlPS.cy = pszlTarget->cy;
        }
        else
        {
            szlPS.cx = pbmihSource->cx;
            szlPS.cy = pbmihSource->cy;
        }

        if (!gpihCreateMemPS(WinQueryAnchorBlock(HWND_DESKTOP),
                             &szlPS,
                             (HDC *) &hdcMem,
                             (HPS *) &hpsMem))
            arc = MMHERR_GPIGREATEPS_FAILED;
        else
        {
            BITMAPINFOHEADER2 bmihTarget;
            memcpy(&bmihTarget,
                   pbmihSource,
                   sizeof(bmihTarget));

            bmihTarget.cx = szlPS.cx;
            bmihTarget.cy = szlPS.cy;

            // I'd love to pass the bits directly to
            // GpiCreateBitmap, but scaling doesn't
            // work with CBI_INIT... so we have to do
            // GpiDrawBits separately.
            if (!(*phbmOut = GpiCreateBitmap(hpsMem,
                                             &bmihTarget,
                                             0,
                                             0,
                                             0)))
                arc = MMHERR_GPICREATEBITMAP_FAILED;
            else
            {
                // bitmap created:

                if (HBM_ERROR == GpiSetBitmap(hpsMem,
                                              *phbmOut))
                    arc = MMHERR_GPISETBITMAP_FAILED;
                else
                {
                    POINTL aptl[4];
                    // target bottom left (world coordinates)
                    aptl[0].x = 0;
                    aptl[0].y = 0;
                    // target top right (world coordinates)
                    aptl[1].x = bmihTarget.cx - 1;
                    aptl[1].y = bmihTarget.cy - 1;
                    // source bottom left (device coordinates)
                    aptl[2].x = 0;
                    aptl[2].y = 0;
                    // source top right (device coordinates)
                    aptl[3].x = pbmihSource->cx;
                    aptl[3].y = pbmihSource->cy;

                    if (GPI_ERROR == GpiDrawBits(hpsMem,
                                                 pbBitmapBits,
                                                 (PBITMAPINFO2)pbmihSource,
                                                 4,
                                                 aptl,
                                                 ROP_SRCCOPY,
                                                 BBO_IGNORE))
                        arc = MMHERR_GPISETBITMAPBITS_FAILED;

                    /*
                    if (GPI_ALTERROR == GpiSetBitmapBits(hpsMem,
                                                         (LONG)0,
                                                         (LONG)szlSourceBmp.cy,
                                                         (PBYTE)pRowBuffer,
                                                         (PBITMAPINFO2)&bmihSource))
                        arc = MMHERR_GPISETBITMAPBITS_FAILED;
                    */
                } // end if (GpiSetBitmap(hpsMem
            } // if (!(*phbmOut = GpiCreateBitmap(hpsMem,
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (hpsMem)
    {
        // unset bitmap from mem PS
        GpiSetBitmap(hpsMem,
                     NULLHANDLE);

        if (arc && *phbmOut)
        {
            // error:
            GpiDeleteBitmap(*phbmOut);
            *phbmOut = NULLHANDLE;
        }

        GpiDestroyPS(hpsMem);
    }

    if (hdcMem)
        DevCloseDC(hdcMem);

    return arc;
}

/*
 *@@ mmhLoadImage:
 *      one-shot function for loading an image from a file.
 *      Understands any image file format supported by MMPM/2.
 *
 *      If fccIOProc is specified, it _must_ represent an
 *      IOProc for an image format. We do not verify this
 *      here.
 *
 *      Otherwise, if fccIOProc is NULLHANDLE, we automatically
 *      run mmhIdentifyFile and may return MMHERR_NOT_IMAGE_FILE.
 *
 *      Returns either NO_ERROR or one of the following
 *      error codes:
 *
 *      --  MMHERR_MMPM_NOT_INITIALIZED: mmhInit not called or
 *          it failed.
 *
 *      --  MMHERR_NO_IOPROC: file format not understood
 *          by MMPM/2.
 *
 *      --  MMHERR_NOT_IMAGE_FILE: file format understood, but
 *          is not image file or cannot be translated.
 *
 *      --  MMHERR_OPEN_FAILED: mmioOpen failed.
 *
 *      --  MMHERR_INCOMPATIBLE_HEADER: ioproc returned invalid
 *          header.
 *
 *      --  ERROR_PROTECTION_VIOLATION: probably crash in IOProc.
 *
 *      plus the error codes from CreateBitmapFromFile().
 */

APIRET mmhLoadImage(PCSZ pcszFilename,  // in: filename to load
                    FOURCC fccIOProc,   // in: FOURCC of ioproc to use; if 0, we call mmhIdentifyFile here
                    PSIZEL pszlBmp,     // in: desired size of new bitmap or NULL for no resize
                    HBITMAP *phbmOut)   // out: newly created bitmap
{
    APIRET  arc = NO_ERROR;

    volatile PBYTE   pbBitmapBits = NULL; // XWP V1.0.4 (2005-10-09) [pr]
    volatile HMMIO   hmmio = NULLHANDLE;

    if (    (!pcszFilename)
         || (!phbmOut)
       )
        return ERROR_INVALID_PARAMETER;

    if (!G_fFuncsResolved)
        return MMHERR_MMPM_NOT_INITIALIZED;

#ifdef __DEBUG__
    TRY_LOUD(excpt1)
#else
    TRY_QUIET(excpt1)
#endif
    {
        if (!(arc = mmhOpenImage(pcszFilename,
                                 fccIOProc,
                                 (HMMIO *) &hmmio)))
        {
            MMIMAGEHEADER   mmihSource;
            ULONG           cbBytesPerRow;

            if (!(arc = mmhLoadImageHeader(hmmio,
                                           &mmihSource,
                                           &cbBytesPerRow)))
            {
                if (!(arc = mmhLoadImageBits(hmmio,
                                             &mmihSource,
                                             cbBytesPerRow,
                                             (PBYTE *) &pbBitmapBits)))
                {
                    arc = mmhCreateBitmapFromBits(pszlBmp,
                                                  &mmihSource.mmXDIBHeader.BMPInfoHeader2,
                                                  pbBitmapBits,
                                                  phbmOut);
                } // if (!(arc = mmhLoadImageBits(pcszFilename,
            } // if (!(arc = mmhLoadImageHeader(pcszFilename,
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (pbBitmapBits)
        DosFreeMem(pbBitmapBits);

    if (hmmio)
        G_mmioClose(hmmio, 0);

    return arc;
}

