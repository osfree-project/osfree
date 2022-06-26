
/*
 *@@sourcefile filesys.h:
 *      header file for filesys.c (extended file types implementation).
 *
 *      This file is ALL new with V0.9.0.
 *
 *@@include #include <os2.h>
 *@@include #include "shared\notebook.h"
 *@@include #include "filesys\filesys.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#ifndef FILESYS_HEADER_INCLUDED
    #define FILESYS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   File system information implementation
     *
     ********************************************************************/

    #ifdef SOM_WPFileSystem_h

        PSZ fsysQueryEASubject(WPFileSystem *somSelf);

        PSZ fsysQueryEAComments(WPFileSystem *somSelf);

        PSZ fsysQueryEAKeyphrases(WPFileSystem *somSelf);

        BOOL fsysSetEASubject(WPFileSystem *somSelf, PCSZ psz);

        BOOL fsysSetEAComments(WPFileSystem *somSelf, PCSZ psz);

        BOOL fsysSetEAKeyphrases(WPFileSystem *somSelf, PCSZ psz);

        #ifndef DIRTYBIT
            #define DIRTYBIT    0x80000000
        #endif
        #ifndef FOUNDBIT
            #define FOUNDBIT    0x40000000
        #endif

    #endif

    /* ******************************************************************
     *
     *   Populate / refresh
     *
     ********************************************************************/

    /*
     *@@ FDATETIME:
     *
     *@@added V0.9.16 (2001-10-28) [umoeller]
     */

    #pragma pack(2)
    typedef struct _FDATETIME
    {
        FDATE       Date;
        FTIME       Time;
    } FDATETIME, *PFDATETIME;
    #pragma pack()

    /*
     *@@ MAKEAWAKEFS:
     *      structure used with M_WPFileSystem::wpclsMakeAwake.
     *      Note that this is undocumented and may not work
     *      with every OS/2 version, although it works here
     *      with eCS.
     *
     *      This mostly has data from the FILEFINDBUF3L that
     *      we processed, although for some strange reason
     *      the fields have a different ordering here.
     *
     *      This also gets passed to WPFileSystem::wpRestoreState
     *      in the ulReserved parameter. ;-)
     *
     *      No idea where the high DWORD of the filesize is supposed
     *      to go now we have large file support V1.0.9 [pr]
     *
     *@@added V0.9.16 (2001-10-25) [umoeller]
     */

    typedef struct _MAKEAWAKEFS
    {
        PSZ         pszRealName;    // real name
        FDATETIME   Creation;
        FDATETIME   LastWrite;
        FDATETIME   LastAccess;
        ULONG       attrFile;
        ULONG       cbFile;         // file size
        ULONG       cbList;         // size of FEA2LIST
        PFEA2LIST   pFea2List;      // EAs
    } MAKEAWAKEFS, *PMAKEAWAKEFS;

    VOID fsysCreateStandardGEAList(VOID);

    // buffer size for DosFindFirst
    #define FINDBUFSIZE             0x10000     // 64K

    APIRET fsysCreateFindBuffer(PEAOP2 *pp);

    APIRET fsysFillFindBuffer(PCSZ pszFilename,
                              PFILEFINDBUF3L *ppfb3,
                              PEAOP2 *ppeaop);

    VOID fsysFreeFindBuffer(PEAOP2 *pp);

    #ifdef SOM_WPFolder_h
        BOOL fsysPopulateWithFSObjects(WPFolder *somSelf,
                                       HWND hwndReserved,
                                       PMINIRECORDCORE pMyRecord,
                                       PCSZ pcszFolderFullPath,
                                       BOOL fFoldersOnly,
                                       PCSZ pcszFileMask,
                                       PBOOL pfExit);
    #endif

    #ifdef SOM_WPFileSystem_h
        APIRET fsysRefresh(WPFileSystem *somSelf,
                           PVOID pvReserved);
    #endif

    /* ******************************************************************
     *
     *   "File" pages replacement in WPDataFile/WPFolder
     *
     ********************************************************************/

    #ifdef SOM_WPFileSystem_h
        ULONG fsysInsertFilePages(WPObject *somSelf,
                                  HWND hwndNotebook);
    #endif

#endif


