
/*
 *@@sourcefile icons.h:
 *      header file for icons.c.
 *
 *      This file is ALL new with V0.9.16.
 *
 *@@include #include <os2.h>
 *@@include #include <wpobject.h>   // or any other WPS SOM header
 *@@include #include "shared\notebook.h"    // for notebook headers
 *@@include #include "filesys\icon.h"
 */

/*
 *      Copyright (C) 2001-2013 Ulrich M”ller.
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

#ifndef ICONS_HEADER_INCLUDED
    #define ICONS_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Icon data handling
     *
     ********************************************************************/

    APIRET icoBuildPtrHandle(PBYTE pbData,
                             HPOINTER *phptr);

    APIRET icoLoadICOFile(PCSZ pcszFilename,
                          HPOINTER *phptr,
                          PULONG pcbIconData,
                          PBYTE pbIconData);

    APIRET icoBuildPtrFromFEA2List(PFEA2LIST pFEA2List,
                                   HPOINTER *phptr,
                                   PULONG pcbIconData,
                                   PBYTE pbIconData);

    APIRET icoBuildPtrFromEAs(PCSZ pcszFilename,
                              HPOINTER *phptr,
                              PULONG pcbIconInfo,
                              PICONINFO pIconInfo);

    PSZ icoGetWinResourceTypeName(PSZ pszBuf,
                                   ULONG ulTypeThis);

    PCSZ icoGetOS2ResourceTypeName(ULONG ulResourceType);

    #ifdef EXEH_HEADER_INCLUDED
        APIRET icoLoadExeIcon(PEXECUTABLE pExec,
                              ULONG idResource,
                              HPOINTER *phptr,
                              PULONG pcbIconData,
                              PBYTE pbIconData);
    #endif

    /* ******************************************************************
     *
     *   Icon sharing
     *
     ********************************************************************/

    BOOL icomLockIconShares(VOID);

    VOID icomUnlockIconShares(VOID);

    #ifdef SOM_WPObject_h
        HPOINTER icomShareIcon(WPObject *somSelf,
                               WPObject *pobjClient,
                               BOOL fMakeGlobal);

        VOID icomUnShareIcon(WPObject *pobjServer,
                             WPObject *pobjClient);
    #endif

    /* ******************************************************************
     *
     *   Lazy icons
     *
     ********************************************************************/

    #ifdef SOM_WPDataFile_h
        BOOL icomQueueLazyIcon(WPDataFile *somSelf);
        VOID icomUnqueueLazyIcon(WPDataFile *somSelf);
    #endif

    /* ******************************************************************
     *
     *   Object icon management
     *
     ********************************************************************/

    BOOL icomRunReplacement(VOID);

    #ifdef SOM_WPObject_h
        HPOINTER icomQueryIconN(WPObject *pobj,
                                ULONG ulIndex);

        BOOL icomSetIconDataN(WPObject *pobj,
                              ULONG ulIndex,
                              PICONINFO pData);

        APIRET icomLoadIconData(WPObject *pobj,
                                ULONG ulIndex,
                                PICONINFO *ppIconInfo);

        APIRET icomCopyIconFromObject(WPObject *somSelf,
                                      WPObject *pobjSource,
                                      ULONG ulIndex);

        VOID icomResetIcon(WPObject *somSelf,
                           ULONG ulIndex);

        BOOL icomIsUsingDefaultIcon(WPObject *pobj,
                                    ULONG ulAnimationIndex);
    #endif

    /* ******************************************************************
     *
     *   Notebook callbacks (notebook.c) for new XFldObject "Icon" page
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED

        #define ICONFL_TITLE            0x0001
        #define ICONFL_ICON             0x0002
        #define ICONFL_TEMPLATE         0x0004
        #define ICONFL_LOCKEDINPLACE    0x0008
        #define ICONFL_HOTKEY           0x0010
        #define ICONFL_DETAILS          0x0020

        VOID icoFormatIconPage(PNOTEBOOKPAGE pnbp,
                               ULONG flFlags);

        VOID XWPENTRY icoIcon1InitPage(PNOTEBOOKPAGE pnbp,
                                       ULONG flFlags);

        MRESULT XWPENTRY icoIcon1ItemChanged(PNOTEBOOKPAGE pnbp,
                                             ULONG ulItemID, USHORT usNotifyCode,
                                             ULONG ulExtra);
    #endif

#endif


