
/*
 *@@sourcefile mapfile.h:
 *      header file for map_vac.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\linklist.h"
 *@@include #include "helpers\mapfile.h"
 */

/*      Copyright (C) 2002 Ulrich M”ller.
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

#ifndef MAPFILE_HEADER_INCLUDED
    #define MAPFILE_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Errors
     *
     ********************************************************************/

    #define MAPERROR_FIRST                      10000
    #define MAPERROR_NO_MODULE_NAME             (MAPERROR_FIRST)
    #define MAPERROR_SYNTAX_SEGMENTS_START      (MAPERROR_FIRST + 1)
                // cannot handle initial line in segments block;
                // line must have format
                // " 0001:00000000 000001630H FREQ_CODE1             CODE 32-bit"
    #define MAPERROR_SYNTAX_SEGMENTS_NOSEGNAME  (MAPERROR_FIRST + 2)
                // cannot find segment name
    #define MAPERROR_SYNTAX_SEGMENTS_SEGTYPE    (MAPERROR_FIRST + 3)
                // unknown segment type
    #define MAPERROR_SYNTAX_SEGMENTS_OTHER      (MAPERROR_FIRST + 4)

    #define MAPERROR_SYNTAX_EXPORTS_OBJOFS      (MAPERROR_FIRST + 5)
    #define MAPERROR_SYNTAX_EXPORTS_NAME1       (MAPERROR_FIRST + 6)
    #define MAPERROR_SYNTAX_EXPORTS_OTHER       (MAPERROR_FIRST + 7)

    #define MAPERROR_SYNTAX_PUBLICS_OBJOFS      (MAPERROR_FIRST + 8)
    #define MAPERROR_SYNTAX_PUBLICS_OTHER       (MAPERROR_FIRST + 9)

    /* ******************************************************************
     *
     *   Map definitions
     *
     ********************************************************************/

    /*
     *@@ SEGMENT:
     *
     */

    typedef struct _SEGMENT
    {
        ULONG   ulObjNo;
        ULONG   ulObjOfs;
        ULONG   cbSeg;
        CHAR    szSegName[30];
        ULONG   ulType;
                    #define SEGTYPE_CODE32      1   // CODE 32-bit
                    #define SEGTYPE_CODE16      2   // CODE 16-bit
                    #define SEGTYPE_CONST32     3   // CONST 32-bit
                    #define SEGTYPE_DATA16      4   // DATA16 16-bit
                    #define SEGTYPE_DATA32      5   // DATA 32-bit
                    #define SEGTYPE_EDC_CLASS32 6   // EDC_CLASS 32-bit
                    #define SEGTYPE_BSS32       7   // BSS 32-bit
                    #define SEGTYPE_STACK32     8   // STACK 32-bit

    } SEGMENT, *PSEGMENT;

    /*
     *@@ EXPORT:
     *
     */

    typedef struct _EXPORT
    {
        ULONG   ulObjNo;
        ULONG   ulObjOfs;

        ULONG   ulName1Len,
                ulName2Len;
        CHAR    szNames[1];         // ulName1Len chars, followed by \0,
                                    // followed by ulName2Len chars, followed by \0
    } EXPORT, *PEXPORT;

    /*
     *@@ PUBLIC:
     *
     */

    typedef struct _PUBLIC
    {
        ULONG   ulObjNo;
        ULONG   ulObjOfs;

        ULONG   ulNameLen,
                ulImportLen;

        ULONG   ulImportIndex;      // if ulImportLen != 0, the index from where
                                    // the thing was imported

        CHAR    szName[1];          // ulNameLen chars, followed by \0,
                                    // and if ulImportLen != 0, followed
                                    // by ulImportLen chars, followed by \0

    } PUBLIC, *PPUBLIC;

    /*
     *@@ MAPFILE:
     *
     */

    typedef struct _MAPFILE
    {
        CHAR        szModule[CCHMAXPATH];

        LINKLIST    llSegments;
        LINKLIST    llExports;
        LINKLIST    llPublics;

    } MAPFILE, *PMAPFILE;

    /* ******************************************************************
     *
     *   Prototypes
     *
     ********************************************************************/

    APIRET mapvRead(PCSZ pcszMapFilename,
                    PMAPFILE *ppMapFile,
                    PSZ pszLineBuf,
                    ULONG cbLineBuf,
                    PULONG pulLineError);

    APIRET mapvClose(PMAPFILE *ppMapFile);

#endif

#if __cplusplus
}
#endif

