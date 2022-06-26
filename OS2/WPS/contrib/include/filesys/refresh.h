
/*
 *@@sourcefile refresh.h:
 *      header file for refresh.c (folder auto-refresh).
 *
 *      This file is ALL new with V0.9.9 (2001-01-31) [umoeller]
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\threads.h"
 *@@include #include "filesys\refresh.h"
 */

/*
 *      Copyright (C) 2001-2003 Ulrich M”ller.
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

#ifndef REFRESH_HEADER_INCLUDED
    #define REFRESH_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Dos*ChangeNotify Declarations
     *
     ********************************************************************/

    // Found these in the DDK headers.

    #pragma pack(1)
    typedef struct _CNPATH {      /* CHANGENOTIFYPATH */
        ULONG   oNextEntryOffset;
        ULONG   wFlags;
        USHORT  cbName;
        CHAR    szName[1];
    } CNPATH;
    typedef CNPATH *PCNPATH;

    typedef struct _CNINFO {      /* CHANGENOTIFYINFO */
        ULONG   oNextEntryOffset;
        CHAR    bAction;
        USHORT  cbName;
        CHAR    szName[1];
    } CNINFO;
    typedef CNINFO *PCNINFO;
    #pragma pack()

    #define  RCNF_FILE_ADDED        0x0001
    #define  RCNF_FILE_DELETED      0x0002
    #define  RCNF_DIR_ADDED         0x0003
    #define  RCNF_DIR_DELETED       0x0004
    #define  RCNF_MOVED_IN          0x0005
    #define  RCNF_MOVED_OUT         0x0006
    #define  RCNF_CHANGED           0x0007
    #define  RCNF_OLDNAME           0x0008
    #define  RCNF_NEWNAME           0x0009
    #define  RCNF_DEVICE_ATTACHED   0x000A
    #define  RCNF_DEVICE_DETACHED   0x000B

    #define  RCNF_XWP_FULLREFRESH   0x00F0
            // extra code only used in XWPNOTIFY to force a full refresh
            // on the folder; in that case, only XWPNOTIFY.pFolder is
            // set and wants a full refresh

    APIRET  APIENTRY DosOpenChangeNotify(PCNPATH PathBuf,
                                         ULONG LogSize,
                                         PHDIR hdir,
                                         ULONG ulReserved);

    APIRET  APIENTRY DosResetChangeNotify(PCNINFO LogBuf,
                                          ULONG BufferSize,
                                          PULONG LogCount,
                                          HDIR hdir);

    APIRET  APIENTRY DosCloseChangeNotify(HDIR hdir);

    /* ******************************************************************
     *
     *   Additional XWP declarations
     *
     ********************************************************************/

    /*
     *@@ XWPNOTIFY:
     *      one file-system notification for our refresh
     *      replacements.
     *
     *      These things are variable in size, depending
     *      on the length of the short file name.
     *
     *@@added V0.9.9 (2001-01-29) [umoeller]
     */

    typedef struct _XWPNOTIFY
    {
        ULONG           ulMS;           // for ager; milliseconds counter value
                                        // when notification was received

        WPFolder        *pFolder;       // folder where change occurred

        PSZ             pShortName;     // ptr into CNInfo.szName to where short name starts

        CNINFO          CNInfo;         // original CNINFO from DosResetChangeNotify
                                        // (find-notify thread)

    } XWPNOTIFY, *PXWPNOTIFY;

    #define     NM_NOTIFICATION     WM_USER

    #define     NM_OVERFLOW         (WM_USER + 1)

    VOID refrClearFolderNotifications(WPFolder *pFolder);

    #ifdef THREADS_HEADER_INCLUDED
        VOID _Optlink refr_fntSentinel(PTHREADINFO ptiMyself);
    #endif

#endif


