
/*
 *@@sourcefile archives.h:
 *      header file for archives.c. See remarks there.
 *
 *      All funtions in this file have the arc* prefix.
 *
 *@@include #include <os2.h>
 *@@include #include "shared\notebook.h"  // for notebook callback prototypes
 *@@include #include "archives.h"
 */

/*
 *      Copyright (C) 1997-2005 Stefan Milcke,
 *                              Ulrich M”ller.
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

#ifndef ARCHIVES_HEADER_INCLUDED
    #define ARCHIVES_HEADER_INCLUDED

    /********************************************************************
     *
     *   "Archives" page replacement in WPDesktop
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY arcArchivesInitPage(PNOTEBOOKPAGE pnbp,
                                          ULONG flFlags);

        MRESULT XWPENTRY arcArchivesItemChanged(PNOTEBOOKPAGE pnbp,
                                       ULONG ulItemID,
                                       USHORT usNotifyCode,
                                       ULONG ulExtra);
    #endif

    /********************************************************************
     *
     *   Archiver settings
     *
     ********************************************************************/

    /*
     * Archiving settings flags:
     *      the following are stored in ARCHIVINGSETTINGS.ulArcFlags
     *      and correspond to the checkboxes on the "Archives"
     *      page replacement in the Desktop's settings notebook.
     */

    #define ARCF_ENABLED                0x0001
    #define ARCF_ALWAYS                 0x0002
    #define ARCF_NEXT                   0x0004
    #define ARCF_INI                    0x0008
    #define ARCF_DAYS                   0x0010

    /*
     *@@ ARCHIVINGSETTINGS:
     *      settings structure stored in OS2.INI.
     */

    typedef struct _ARCHIVINGSETTINGS
    {
        ULONG       ulArcFlags,               // ARCF_* flags (archives.h)
                    ulEveryDays;

        double      dIniFilesPercent;

        double      dAppsSizeLast,
                    dKeysSizeLast,
                    dDataSumLast;

        BOOL        fShowStatus;
        CHAR        cArchivesCount;
        BOOL        fShowRestore;
        USHORT      usRestoreTimeOut;
    } ARCHIVINGSETTINGS, *PARCHIVINGSETTINGS;

    VOID arcSetDefaultSettings(VOID);

    PARCHIVINGSETTINGS arcQuerySettings(VOID);

    BOOL arcSaveSettings(VOID);

    /********************************************************************
     *
     *   Archiving Enabling
     *
     ********************************************************************/

    APIRET arcQueryArchiveByte(UCHAR *pByte,
                               LONG offset);

    APIRET arcSetArchiveByte(UCHAR byte,
                             LONG offset);

    VOID arcForceNoArchiving(VOID);

    BOOL arcCheckIfBackupNeeded(HWND hwndNotify,
                                ULONG ulMsg);

    APIRET arcSwitchArchivingOn(BOOL fSwitchOn);

    BOOL arcSetNumArchives(PCHAR pcArchives,
                           BOOL fSet);

    BOOL arcCheckINIFiles(double* pdPercent,
                          PSZ pszIgnoreApp,
                          double* pdAppsSizeLast,
                          double* pdKeysSizeLast,
                          double* pdDataSumLast,
                          double* pdMaxDifferencePercent);
#endif

