
/*
 *@@sourcefile level.h:
 *      header file for level.c. See remarks there.
 *
 *@@include #include "helpers\level.h"
 */

/*
 *      Copyright (C) 2000 Ulrich M”ller.
 *      Copyright (C) 1994 Martin Lafaix (EDM/2 2-05).
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

#ifndef LEVEL_HEADER_INCLUDED
    #define LEVEL_HEADER_INCLUDED

    APIRET lvlOpenLevelFile(PSZ pszName, PHFILE phFile,
                            ULONG ulOpenMode, PSZ pszCID);
    APIRET lvlQueryLevelFile(PSZ pszName, PSZ pszCID,
                             PVOID pBuffer, ULONG ulBufSize);
    APIRET lvlQueryLevelFileData(HFILE hFile, ULONG ulWhat,
                                 PVOID pBuffer, ULONG ulBufSize, PULONG pulSize);
    APIRET lvlWriteLevelFileData(HFILE hFile, ULONG ulWhat,
                                 PVOID pBuffer, ULONG ulBufSize, PULONG pulSize);

    #define lvlCloseLevelFile DosClose

    /***************************************************************************/
    /* ulOpenMode values                                                       */
    /***************************************************************************/
    #define OLF_OPEN          0x00000001L              /* scan from current dir*/
    #define OLF_SCANDISKS     0x00000002L              /* scan all disks       */
    #define OLF_CHECKID       0x00000004L              /* check component ID   */
    #define OLF_CREATE        0x00000008L              /* create syslevel file */

    /***************************************************************************/
    /* ulWhat values                                                           */
    /***************************************************************************/
    #define QLD_MAJORVERSION  1
    #define QLD_MINORVERSION  2
    #define QLD_REVISION      3
    #define QLD_KIND          4
    #define QLD_CURRENTCSD    5
    #define QLD_PREVIOUSCSD   6
    #define QLD_TITLE         7
    #define QLD_ID            8
    #define QLD_TYPE          9

    /***************************************************************************/
    /* kind values                                                             */
    /***************************************************************************/
    #define SLK_BASE          0                        /* 'base' component     */
    #define SLK_EXTENSION     2                        /* system extension     */
    #define SLK_STANDARD      15                       /* 'normal' component   */

    #ifdef SYSLEVEL_PRIVATE
        /***************************************************************************/
        /* privates structures                                                     */
        /***************************************************************************/
        #pragma pack(1)

        typedef struct _SYSLEVELHEADER {                   /* syslevel file header */
           unsigned char h_magic[2];                       /* magic cookie (0xFFFF)*/
           unsigned char h_name[9];                        /* SYSLEVEL string      */
           unsigned char h_reserved1[4];                   /* unknown - set to zero*/
           unsigned char h_updated;                        /* unknown, too         */
           unsigned char h_reserved2[17];                  /* unknown, again       */
           ULONG         h_data;                           /* data structure offset*/
        } SYSLEVELHEADER, *PSYSLEVELHEADER;

        typedef struct _SYSLEVELDATA {                     /* syslevel file data   */
           unsigned char d_reserved1[2];                   /* unknown - set to zero*/
           unsigned char d_kind[1];                        /* component kind       */
           unsigned char d_version[2];                     /* component version    */
           unsigned char d_reserved2[2];                   /* unknown              */
           unsigned char d_clevel[7];                      /* current CSD level    */
           unsigned char d_reserved3;                      /* unknown - 0x5F       */
           unsigned char d_plevel[7];                      /* previous CSD level   */
           unsigned char d_reserved4;                      /* unknown - 0x5F       */
           unsigned char d_title[80];                      /* component title      */
           unsigned char d_cid[9];                         /* component ID         */
           unsigned char d_revision[1];                    /* component revision   */
           unsigned char d_type[1+5];                      /* component type       */
        } SYSLEVELDATA, *PSYSLEVELDATA;

        #pragma pack()

        /***************************************************************************/
        /* local functions prototypes                                              */
        /***************************************************************************/
        static APIRET _locatesysleveldata(HFILE hFile);
        static APIRET _readsysleveldata(HFILE hFile, PSYSLEVELDATA psld);
        static APIRET _writesysleveldata(HFILE hFile, PSYSLEVELDATA psld);
        static APIRET _writesyslevelheader(HFILE hFile, PSYSLEVELHEADER pslh);
        static PSYSLEVELDATA _allocsysleveldata(HFILE hFile);
        static VOID _freesysleveldata(PSYSLEVELDATA psld);
        static APIRET _findsyslevelfile(PSZ pszName, PSZ pszCID, PVOID pBuffer);
        static APIRET _searchsubdirs(PSZ pszName, PSZ pszCID, PVOID pBuffer);
        static ULONG _firstdrive(ULONG ulDriveMap);
        static ULONG _nextdrive(ULONG ulDriveMap, ULONG ulCurrent);
        static APIRET _getfullname(PVOID pBuffer, PSZ pszFileName);
        static APIRET _findallsyslevelfiles(PSZ achFileName, PSZ pszCID,
                                            PPVOID ppBuffer, PULONG pulBufSize);

        /***************************************************************************/
        /* useful macros                                                           */
        /***************************************************************************/
        // #define min(x, y)       ((x) > (y) ? (y) : (x))
        #define _setsize(s)     if(pulSize) *pulSize = s
        #define _getfield(f)    if(ulBufSize >= sizeof(psld->f))\
                                  memcpy(pBuffer, psld->f, sizeof(psld->f));\
                                else\
                                  rc = 122; /* ERROR_INSUFFICIENT_BUFFER */\
                                _setsize(sizeof(psld->f))
        #define _setfield(f)    if(ulBufSize >= sizeof(psld->f))\
                                  memcpy(psld->f, pBuffer, sizeof(psld->f));\
                                else\
                                  rc = 122; /* ERROR_INSUFFICIENT_BUFFER */\
                                _setsize(sizeof(psld->f));
        #define _achBuffer      ((CHAR *)pBuffer)

        #define ERROR_INSUFFICIENT_BUFFER   122
        #define ERROR_INVALID_PARAMETER     87

        #define trace printf("rc = %d\n", rc);

    #endif // SYSLEVEL_PRIVATE

#endif

#if __cplusplus
}
#endif

