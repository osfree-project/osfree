
/*
 *@@sourcefile desktop.h:
 *      header file for desktop.c (XFldDesktop implementation).
 *
 *      This file is ALL new with V0.9.0.
 *
 *@@include #include <os2.h>
 *@@include #include "shared\notebook.h"
 *@@include #include "filesys\desktop.h"
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

#ifndef SETPAGES_HEADER_INCLUDED
    #define SETPAGES_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   Setup strings
     *
     ********************************************************************/

    BOOL dtpQuerySetup(WPDesktop *somSelf,
                       PVOID pstrSetup);

    BOOL dtpSetup(WPDesktop *somSelf,
                  PCSZ pcszSetupString);

    /* ******************************************************************
     *
     *   Desktop menus
     *
     ********************************************************************/

    #define DEBUG_MENUID_LISTHEAP       WPMENUID_USER
    #define DEBUG_MENUID_RELEASEFREED   WPMENUID_USER + 1
    #define DEBUG_MENUID_CRASH_MENU     WPMENUID_USER + 2
    #define DEBUG_MENUID_CRASH_THR1     WPMENUID_USER + 3
    #define DEBUG_MENUID_CRASH_WORKER   WPMENUID_USER + 4
    #define DEBUG_MENUID_CRASH_QUICK    WPMENUID_USER + 5
    #define DEBUG_MENUID_CRASH_FILE     WPMENUID_USER + 6
    #define DEBUG_MENUID_DUMPWINLIST    WPMENUID_USER + 7

    VOID dtpModifyPopupMenu(WPDesktop *somSelf,
                            HWND hwndMenu);

    BOOL dtpMenuItemSelected(XFldDesktop *somSelf,
                             HWND hwndFrame,
                             PULONG pulMenuId);

    /* ******************************************************************
     *
     *   XFldDesktop notebook settings pages callbacks (notebook.c)
     *
     ********************************************************************/

    #ifdef NOTEBOOK_HEADER_INCLUDED
        VOID XWPENTRY dtpStartupInitPage(PNOTEBOOKPAGE pnbp,
                                         ULONG flFlags);

        MRESULT XWPENTRY dtpStartupItemChanged(PNOTEBOOKPAGE pnbp,
                                      ULONG ulItemID,
                                      USHORT usNotifyCode,
                                      ULONG ulExtra);
    #else
        #error "shared\notebook.h needs to be included before including desktop.h".
    #endif
#endif


