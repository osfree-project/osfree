
/*
 *@@sourcefile mmpmh.h:
 *      header file for mmpmh.c. See notes there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_GPI                // required for INCL_MMIO_CODEC
 *@@include #define INCL_GPIBITMAPS         // required for INCL_MMIO_CODEC
 *@@include #include <os2.h>
 *@@include // multimedia includes
 *@@include #define INCL_MCIOS2
 *@@include #define INCL_MMIOOS2
 *@@include #define INCL_MMIO_CODEC
 *@@include #include <os2me.h>
 *@@include #include "helpers\mmpmh.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
 *
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

#if __cplusplus
extern "C" {
#endif

#ifndef MMPMH_HEADER_INCLUDED
    #define MMPMH_HEADER_INCLUDED

    /********************************************************************
     *
     *   Errors
     *
     ********************************************************************/

    #define ERROR_MM_FIRST                      45000

    #define MMHERR_MMPM_NOT_INITIALIZED         (ERROR_MM_FIRST)
    #define MMHERR_NO_IOPROC                    (ERROR_MM_FIRST + 1)
    #define MMHERR_NOT_IMAGE_FILE               (ERROR_MM_FIRST + 2)
    #define MMHERR_OPEN_FAILED                  (ERROR_MM_FIRST + 3)
    #define MMHERR_INCOMPATIBLE_HEADER          (ERROR_MM_FIRST + 4)
    #define MMHERR_GPIGREATEPS_FAILED           (ERROR_MM_FIRST + 5)
    #define MMHERR_GPICREATEBITMAP_FAILED       (ERROR_MM_FIRST + 6)
    #define MMHERR_GPISETBITMAP_FAILED          (ERROR_MM_FIRST + 7)
    #define MMHERR_GPISETBITMAPBITS_FAILED      (ERROR_MM_FIRST + 8)
    #define MMHERR_MMIOREAD_FAILED              (ERROR_MM_FIRST + 9)

    #define ERROR_MM_LAST                       (ERROR_MM_FIRST + 9)

    /********************************************************************
     *
     *   MMPM/2 imports
     *
     ********************************************************************/

    // mciSendCommand
    typedef ULONG APIENTRY FNTD_MCISENDCOMMAND(USHORT,
                                               USHORT,
                                               ULONG,
                                               PVOID,
                                               USHORT);
    // mciGetErrorString
    typedef ULONG APIENTRY FNTD_MCIGETERRORSTRING(ULONG,
                                                  PSZ,
                                                  USHORT);

    // mmioIniFileCODEC
    typedef ULONG APIENTRY FNTD_MMIOINIFILECODEC(PCODECINIFILEINFO,
                                                 ULONG);
    typedef ULONG APIENTRY FNTD_MMIOQUERYCODECNAMELENGTH(PCODECINIFILEINFO,
                                                         PULONG);
    typedef ULONG APIENTRY FNTD_MMIOQUERYCODECNAME(PCODECINIFILEINFO,
                                                   PSZ,
                                                   PULONG);

    typedef ULONG APIENTRY FNTD_MMIOQUERYFORMATCOUNT(PMMFORMATINFO,
                                                     PLONG,
                                                     ULONG,
                                                     ULONG);

    typedef ULONG APIENTRY FNTD_MMIOGETFORMATS(PMMFORMATINFO,
                                               LONG,
                                               PVOID,
                                               PLONG,
                                               ULONG,
                                               ULONG);

    typedef ULONG APIENTRY FNTD_MMIOGETFORMATNAME(PMMFORMATINFO,
                                                  PSZ,
                                                  PLONG,
                                                  ULONG,
                                                  ULONG);

    typedef ULONG APIENTRY FNTD_MMIOIDENTIFYFILE(PCSZ pszFileName,
                                                 PMMIOINFO pmmioinfo,
                                                 PMMFORMATINFO pmmformatinfo,
                                                 PFOURCC pfccStorageSystem,
                                                 ULONG ulReserved,
                                                 ULONG ulFlags );

    typedef ULONG APIENTRY FNTD_MMIOQUERYHEADERLENGTH(HMMIO hmmio,
                                                      PLONG plHeaderLength,
                                                      ULONG ulReserved,
                                                      ULONG ulFlags );

    typedef ULONG APIENTRY FNTD_MMIOGETHEADER(HMMIO hmmio,
                                              PVOID pHeader,
                                              LONG lHeaderLength,
                                              PLONG plBytesRead,
                                              ULONG ulReserved,
                                              ULONG ulFlags);

    typedef ULONG APIENTRY FNTD_MMIOSETHEADER(HMMIO hmmio,
                                              PVOID pHeader,
                                              LONG lHeaderLength,
                                              PLONG plBytesWritten,
                                              ULONG ulReserved,
                                              ULONG ulFlags);

    typedef ULONG APIENTRY FNTD_MMIOINIFILEHANDLER(PMMINIFILEINFO pmminifileinfo,
                                                   ULONG ulFlags);

    typedef ULONG APIENTRY FNTD_MMIOIDENTIFYSTORAGESYSTEM(PSZ pszFileName,
                                                          PMMIOINFO pmmioinfo,
                                                          PFOURCC pfccStorageSystem);

    typedef ULONG APIENTRY FNTD_MMIODETERMINESSIOPROC(PSZ pszFileName,
                                                      PMMIOINFO pmmioinfo,
                                                      PFOURCC pfccStorageSystem,
                                                      PSZ pszParsedRemainder);

    typedef ULONG APIENTRY FNTD_MMIOQUERYIOPROCMODULEHANDLE(PMMIOPROC pIOProc,
                                                            PHMODULE phmodIOProc);

    typedef ULONG APIENTRY FNTD_MMIOCFCOMPACT(PSZ pszFileName, ULONG ulFlags);

    typedef HMMIO APIENTRY FNTD_MMIOOPEN(PCSZ pszFileName,
                                         PMMIOINFO pmmioinfo,
                                         ULONG ulOpenFlags);

    typedef LONG APIENTRY FNTD_MMIOREAD(HMMIO hmmio,
                                        PCHAR pchBuffer,
                                        LONG cBytes);

    // mmioClose
    typedef USHORT APIENTRY FNTD_MMIOCLOSE(HMMIO hmmio,
                                           USHORT usFlags);


    /* ******************************************************************
     *
     *   Global variables
     *
     ********************************************************************/

    extern FNTD_MCISENDCOMMAND          *G_mciSendCommand;
    extern FNTD_MCIGETERRORSTRING       *G_mciGetErrorString;

    extern FNTD_MMIOINIFILECODEC        *G_mmioIniFileCODEC;
    extern FNTD_MMIOQUERYCODECNAMELENGTH *G_mmioQueryCODECNameLength;
    extern FNTD_MMIOQUERYCODECNAME      *G_mmioQueryCODECName;
    extern FNTD_MMIOINIFILEHANDLER      *G_mmioIniFileHandler;
    extern FNTD_MMIOQUERYFORMATCOUNT    *G_mmioQueryFormatCount;
    extern FNTD_MMIOGETFORMATS          *G_mmioGetFormats;
    extern FNTD_MMIOGETFORMATNAME       *G_mmioGetFormatName;

    extern FNTD_MMIOCLOSE               *G_mmioClose;

    /* ******************************************************************
     *
     *   Public entry points
     *
     ********************************************************************/

    APIRET mmhInit(VOID);

    APIRET mmhGetIOProcs(ULONG flMediaType,
                         PMMFORMATINFO *ppaInfos,
                         PLONG pcFormats);

    APIRET mmhDescribeFCC(FOURCC fcc);

    APIRET mmhIdentifyFile(PCSZ pcszFilename,
                           PMMFORMATINFO pmmfi);

    APIRET mmhOpenImage(PCSZ pcszFilename,
                        FOURCC fccIOProc,
                        HMMIO *phmmio);

    APIRET mmhLoadImageHeader(HMMIO hmmio,
                              PMMIMAGEHEADER pmmih,
                              PULONG pcbBytesPerRow);

    APIRET mmhLoadImageBits(HMMIO hmmio,
                            PMMIMAGEHEADER pmmih,
                            ULONG cbBytesPerRow,
                            PBYTE *ppbBitmapBits);

    APIRET mmhCreateBitmapFromBits(PSIZEL pszlTarget,
                                   PBITMAPINFOHEADER2 pbmihSource,
                                   PBYTE pbBitmapBits,
                                   HBITMAP *phbmOut);

    APIRET mmhLoadImage(PCSZ pcszFilename,
                        FOURCC fccIOProc,
                        PSIZEL pszlBmp,
                        HBITMAP *phbmOut);

#endif

#if __cplusplus
}
#endif

