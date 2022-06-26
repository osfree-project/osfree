
/*
 *@@sourcefile xsetup.h:
 *      header file for xsetup.c (XWPSetup implementation).
 *
 *@@include #include <os2.h>
 *@@include #include "shared\notebook.h"
 *@@include #include "shared\xsetup.h"
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

#ifndef XSETUP_HEADER_INCLUDED
    #define XSETUP_HEADER_INCLUDED

    BOOL setCreateStandardObject(HWND hwndOwner,
                                 USHORT usMenuID,
                                 BOOL fConfirm,
                                 BOOL fXWPObject);

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID setLogoInitPage(PNOTEBOOKPAGE pnbp,
                             ULONG flFlags);

        BOOL setLogoMessages(PNOTEBOOKPAGE pnbp,
                             ULONG msg, MPARAM mp1, MPARAM mp2,
                             MRESULT *pmrc);
    #endif

    #ifdef SOM_XWPSetup_h
        ULONG setInsertNotebookPages(XWPSetup *somSelf,
                                     HWND hwndDlg);
    #endif

#endif

