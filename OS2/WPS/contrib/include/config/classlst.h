
/*
 *@@sourcefile classlst.h:
 *      header file for classlst.c, which contains the logic
 *      for displaying the WPS class tree in a container.
 *
 *      Most of this file was called "objdlgs.h" in V0.80.
 *
 *      In order to use this, you need to do the following:
 *
 *      1) #include classlst.h, but make sure you
 *         have #include'd os2.h before
 *
 *      2) set up the SELECTCLASSDATA structure below
 *         and pass this to WinDlgBox. Example:
 *
 *             SELECTCLASSDATA scd;
 *             strcpy(scd.szDlgTitle, "Select Class");
 *             strcpy(scd.szIntroText, "Select the class for which you want to change "
 *                                   "the status bar info.");
 *             strcpy(scd.szRootClass, "WPObject");
 *             strcpy(scd.szClassSelected, "WPObject");
 *             scd.ulHelpPanel = 0;
 *
 *             WinDlgBox(HWND_DESKTOP, hwndDlg,
 *                     fnwpSelectStatusBarClass,
 *                     NLS_MODULE, ID_XSD_SELECTCLASS,  // resources
 *                     &scd);
 *
 *      This also contains the logic for the "Internals"
 *      page in each object's settings notebook.
 *
 *@@include #define INCL_WINSTDCNR
 *@@include #define INCL_WINWORKPLACE
 *@@include #include <os2.h>
 *@@include #include <wpobject.h>
 *@@include #include "helpers\linklist.h"
 *@@include #include "shared\notebook.h"   // for notebook callback prototypes
 *@@include #include "setup\classlst.h"
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

#ifndef CLASSLST_HEADER_INCLUDED
    #define CLASSLST_HEADER_INCLUDED

    /********************************************************************
     *
     *   "Register new class" dlg
     *
     ********************************************************************/

    /*
     *@@ REGISTERCLASSDATA:
     *
     */

    typedef struct _REGISTERCLASSDATA
    {
        CHAR    szModName[CCHMAXPATH];
        CHAR    szClassName[256];
        const char* pszHelpLibrary;         // help library
        ULONG   ulHelpPanel;            // help panel; if 0, the "Help"
                                        // button is disabled
    } REGISTERCLASSDATA, *PREGISTERCLASSDATA;

    MRESULT EXPENTRY fnwpRegisterClass(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2);

    /********************************************************************
     *
     *   XWPClassList implementation interface
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY cllClassListInitPage(PNOTEBOOKPAGE pnbp,
                                           ULONG flFlags);

        MRESULT XWPENTRY cllClassListItemChanged(PNOTEBOOKPAGE pnbp,
                                         ULONG ulItemID,
                                         USHORT usNotifyCode,
                                         ULONG ulExtra);
    #endif

    BOOL cllModifyPopupMenu(XWPClassList *somSelf,
                            HWND hwndMenu,
                            HWND hwndCnr,
                            ULONG iPosition);

    BOOL cllMenuItemSelected(XWPClassList *somSelf,
                             HWND hwndFrame,
                             ULONG ulMenuId);

    HWND cllCreateClassListView(WPObject *somSelf,
                                HWND hwndCnr,
                                ULONG ulView);

#endif

