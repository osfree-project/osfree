
/*
 *@@sourcefile fdrhotky.h:
 *      header file for fdrhotky.c.
 *
 *      This file was separated from folder.h with
 *      V1.0.0 (2002-08-31) [umoeller].
 *
 *@@include #include <os2.h>
 *@@include #include <wpfolder.h>
 *@@include #include "shared\notebook.h"    // for notebook callback prototypes
 *@@include #include "filesys\fdrhotky.h"
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
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

#ifndef FDRHOTKY_HEADER_INCLUDED
    #define FDRHOTKY_HEADER_INCLUDED

    /********************************************************************
     *
     *   Folder hotkey functions (fdrhotky.c)
     *
     ********************************************************************/

    // maximum no. of folder hotkeys
    #define FLDRHOTKEYCOUNT (ID_XSSI_LB_LAST-ID_XSSI_LB_FIRST+1)

    // maximum length of folder hotkey descriptions
    #define MAXLBENTRYLENGTH 50

    /*
     *@@ XFLDHOTKEY:
     *      XFolder folder hotkey definition.
     *      A static array of these exists in folder.c.
     */

    typedef struct _XFLDHOTKEY
    {
        USHORT  usFlags;     //  Keyboard control codes
        USHORT  usKeyCode;   //  Hardware scan code
        USHORT  usCommand;   //  corresponding menu item id to send to container
    } XFLDHOTKEY, *PXFLDHOTKEY;

    #define FLDRHOTKEYSSIZE sizeof(XFLDHOTKEY)*FLDRHOTKEYCOUNT

    PXFLDHOTKEY fdrQueryFldrHotkeys(VOID);

    void fdrLoadDefaultFldrHotkeys(VOID);

    void fdrLoadFolderHotkeys(VOID);

    void fdrStoreFldrHotkeys(VOID);

    BOOL fdrFindHotkey(USHORT usCommand,
                       PUSHORT pusFlags,
                       PUSHORT pusKeyCode);

    BOOL fdrProcessFldrHotkey(WPFolder *somSelf,
                              HWND hwndFrame,
                              USHORT usFlags,
                              USHORT usch,
                              USHORT usvk);

    VOID fdrAddHotkeysToPulldown(HWND hwndPulldown,
                                 const ULONG *paulMenuIDs,
                                 ULONG cMenuIDs);

    VOID fdrAddHotkeysToMenu(WPObject *somSelf,
                             HWND hwndCnr,
                             HWND hwndMenu,
                             ULONG ulMenuType);

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY fdrHotkeysInitPage(PNOTEBOOKPAGE pnbp,
                                         ULONG flFlags);

        MRESULT XWPENTRY fdrHotkeysItemChanged(PNOTEBOOKPAGE pnbp,
                                      ULONG ulItemID,
                                      USHORT usNotifyCode,
                                      ULONG ulExtra);
    #endif

#endif
