
/*
 *@@sourcefile fdrmenus.h:
 *      header file for fdrmenus.c.
 *
 *      This file is new with V0.81. The function prototypes in
 *      this file used to be in common.h and have now been exported
 *      to make their context more lucid.
 *
 *      Some declarations for fdrmenus.c are still in common.h however.
 *
 *@@include #define INCL_WINMENUS
 *@@include #include <os2.h>
 *@@include #include <wpfolder.h> // WPFolder
 *@@include #include <wppgm.h> // WPProgram, for some funcs only
 *@@include #include <wpobject.h> // only if other WPS headers are not included
 *@@include #include "shared\common.h"
 *@@include #include "shared\notebook.h" // for menu notebook callback prototypes
 *@@include #include "filesys\fdrmenus.h"
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

#ifndef FDRCOMMAND_HEADER_INCLUDED
    #define FDRCOMMAND_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   "Selecting menu items" reaction
     *
     ********************************************************************/

    BOOL fcmdSelectingFsysMenuItem(WPObject *somSelf,
                                   USHORT usItem,
                                   BOOL fPostCommand,
                                   HWND hwndMenu,
                                   HWND hwndCnr,
                                   ULONG ulSelection,
                                   BOOL *pfDismiss);

    BOOL fcmdSelectingFdrMenuItem(WPFolder *somSelf,
                                  USHORT usItem,
                                  BOOL fPostCommand,
                                  HWND hwndMenu,
                                  HWND hwndCnr,
                                  ULONG ulSelection,
                                  BOOL *pfDismiss);

    /* ******************************************************************
     *
     *   "Menu item selected" reaction
     *
     ********************************************************************/

    BOOL fcmdProcessViewCommand(WPFolder *somSelf,
                                USHORT usCommand,
                                HWND hwndCnr,
                                WPObject* pFirstObject,
                                ULONG ulSelectionFlags);

    BOOL fcmdMenuItemSelected(WPFolder *somSelf,
                              HWND hwndFrame,
                              ULONG ulMenuId);

    BOOL fcmdMenuItemHelpSelected(WPObject *somSelf,
                                 ULONG MenuId);

#endif
