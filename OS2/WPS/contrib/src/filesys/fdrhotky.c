
/*
 *@@sourcefile fdrhotky.c:
 *      implementation file for the XFolder folder hotkeys.
 *
 *      This code gets interfaced from method overrides in
 *      xfldr.c, from fnwpSubclWPFolderWindow, and from
 *      XFldWPS (xfwps.c).
 *
 *      This is for _folder_ hotkeys only, which are implemented
 *      thru folder subclassing (fnwpSubclWPFolderWindow),
 *      which in turn calls fdrProcessFldrHotkey.
 *      The global object hotkeys are instead implemented using
 *      the XWorkplace hook (xwphook.c).
 *
 *      This file is ALL new with V0.9.0. Most of this code
 *      used to be in common.c before V0.9.0.
 *
 *      Function prefix for this file:
 *      --  fdr*
 *
 *@@added V0.9.0 [umoeller]
 *@@header "filesys\folder.h"
 */

/*
 *      Copyright (C) 1997-2012 Ulrich M”ller.
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

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_WINSHELLDATA       // Prf* functions
#define INCL_WININPUT
#define INCL_WINWINDOWMGR
#define INCL_WINMENUS
#define INCL_WINDIALOGS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINSTDCNR
#include <os2.h>

// C library headers
#include <stdio.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\cnrh.h"               // container helper routines
#include "helpers\dialog.h"             // dialog helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\prfh.h"               // INI file helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\helppanels.h"          // all XWorkplace help panel IDs
#include "shared\notebook.h"            // generic XWorkplace notebook handling
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\fdrhotky.h"           // folder hotkey handling
#include "filesys\fdrmenus.h"           // shared folder menu logic

// other SOM headers
#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

/*
 *  The folder hotkeys are stored in a static array which
 *  is is FLDRHOTKEYCOUNT items in size. This is calculated
 *  at compile time according to the items which were
 *  declared in dlgids.h.
 *
 *  The following functions give the other XWorkplace components
 *  access to this data and/or manipulate it.
 */

// XFolder folder hotkeys static array
static XFLDHOTKEY     G_FolderHotkeys[FLDRHOTKEYCOUNT];

/* ******************************************************************
 *
 *   Folder hotkey functions
 *
 ********************************************************************/

/*
 *@@ fdrQueryFldrHotkeys:
 *      this returns the address of the static
 *      folder hotkeys array in common.c. The
 *      size of that array is FLDRHOTKEYSSIZE (common.h).
 *
 *@@changed V0.9.0 [umoeller]: moved this here from common.c
 */

PXFLDHOTKEY fdrQueryFldrHotkeys(VOID)
{
    return G_FolderHotkeys;
}

/*
 *@@ fdrLoadDefaultFldrHotkeys:
 *      this resets the folder hotkeys to the default
 *      values.
 *
 *@@changed V0.9.0 [umoeller]: moved this here from common.c
 */

void fdrLoadDefaultFldrHotkeys(VOID)
{
    // Ctrl+A: Select all
    G_FolderHotkeys[0].usKeyCode  = (USHORT)'a';
    G_FolderHotkeys[0].usFlags    = KC_CTRL;
    G_FolderHotkeys[0].usCommand  = WPMENUID_SELALL;

    // F5
    G_FolderHotkeys[1].usKeyCode  = VK_F5;
    G_FolderHotkeys[1].usFlags    = KC_VIRTUALKEY;
    G_FolderHotkeys[1].usCommand  = WPMENUID_REFRESH,

    // Backspace
    G_FolderHotkeys[2].usKeyCode  = VK_BACKSPACE;
    G_FolderHotkeys[2].usFlags    = KC_VIRTUALKEY;
    G_FolderHotkeys[2].usCommand  = ID_XFMI_OFS_OPENPARENT;

    // Ctrl+D: De-select all
    G_FolderHotkeys[3].usKeyCode  = (USHORT)'d';
    G_FolderHotkeys[3].usFlags    = KC_CTRL;
    G_FolderHotkeys[3].usCommand  = WPMENUID_DESELALL;

    // Ctrl+Shift+D: Details view
    G_FolderHotkeys[4].usKeyCode  = (USHORT)'D';
    G_FolderHotkeys[4].usFlags    = KC_CTRL+KC_SHIFT;
    G_FolderHotkeys[4].usCommand  = WPMENUID_DETAILS;

    // Ctrl+Shift+I: Icon  view
    G_FolderHotkeys[5].usKeyCode  = (USHORT)'I';
    G_FolderHotkeys[5].usFlags    = KC_CTRL+KC_SHIFT;
    G_FolderHotkeys[5].usCommand  = WPMENUID_ICON;

    // Ctrl+Shift+S: Open Settings
    G_FolderHotkeys[6].usKeyCode  = (USHORT)'S';
    G_FolderHotkeys[6].usFlags    = KC_CTRL+KC_SHIFT;
    G_FolderHotkeys[6].usCommand  = WPMENUID_PROPERTIES;

    // Ctrl+N: Sort by name
    G_FolderHotkeys[7].usKeyCode  = (USHORT)'n';
    G_FolderHotkeys[7].usFlags    = KC_CTRL;
    G_FolderHotkeys[7].usCommand  = ID_WPMI_SORTBYNAME;

    // Ctrl+Z: Sort by size
    G_FolderHotkeys[8].usKeyCode  = (USHORT)'z';
    G_FolderHotkeys[8].usFlags    = KC_CTRL;
    G_FolderHotkeys[8].usCommand  = ID_WPMI_SORTBYSIZE;

    // Ctrl+E: Sort by extension (NPS)
    G_FolderHotkeys[9].usKeyCode  = (USHORT)'e';
    G_FolderHotkeys[9].usFlags    = KC_CTRL;
    G_FolderHotkeys[9].usCommand  = ID_XFMI_OFS_SORTBYEXT;

    // Ctrl+W: Sort by write date
    G_FolderHotkeys[10].usKeyCode  = (USHORT)'w';
    G_FolderHotkeys[10].usFlags    = KC_CTRL;
    G_FolderHotkeys[10].usCommand  = ID_WPMI_SORTBYWRITEDATE;

    // Ctrl+Y: Sort by type
    G_FolderHotkeys[11].usKeyCode  = (USHORT)'y';
    G_FolderHotkeys[11].usFlags    = KC_CTRL;
    G_FolderHotkeys[11].usCommand  = ID_WPMI_SORTBYTYPE;

    // Shift+Backspace
    G_FolderHotkeys[12].usKeyCode  = VK_BACKSPACE;
    G_FolderHotkeys[12].usFlags    = KC_VIRTUALKEY+KC_SHIFT;
    G_FolderHotkeys[12].usCommand  = ID_XFMI_OFS_OPENPARENTANDCLOSE;

    // Ctrl+S: Select by name
    G_FolderHotkeys[13].usKeyCode  = (USHORT)'s';
    G_FolderHotkeys[13].usFlags    = KC_CTRL;
    G_FolderHotkeys[13].usCommand  = ID_XFMI_OFS_SELECTSOME;

    // Ctrl+insert: copy filename (w/out path)
    G_FolderHotkeys[14].usKeyCode  = VK_INSERT;
    G_FolderHotkeys[14].usFlags    = KC_VIRTUALKEY+KC_CTRL;
    G_FolderHotkeys[14].usCommand  = ID_XFMI_OFS_COPYFILENAME_SHORTSP;

    // list terminator
    G_FolderHotkeys[15].usCommand = 0;
}

/*
 *@@ fdrLoadFolderHotkeys:
 *      this initializes the folder hotkey array with
 *      the data which was previously stored in OS2.INI.
 *
 *@@changed V0.9.0 [umoeller]: moved this here from common.c
 */

void fdrLoadFolderHotkeys(VOID)
{
    ULONG ulCopied2 = sizeof(G_FolderHotkeys);
    if (!PrfQueryProfileData(HINI_USERPROFILE,
                             (PSZ)INIAPP_XWORKPLACE, (PSZ)INIKEY_ACCELERATORS,
                             &G_FolderHotkeys,
                             &ulCopied2))
        fdrLoadDefaultFldrHotkeys();
}

/*
 *@@ fdrStoreFldrHotkeys:
 *       this stores the folder hotkeys in OS2.INI.
 *
 *@@changed V0.9.0 [umoeller]: moved this here from common.c
 */

void fdrStoreFldrHotkeys(VOID)
{
    SHORT i2 = 0;

    // store only the accels that are actually used,
    // so count them first
    while (G_FolderHotkeys[i2].usCommand)
        i2++;

    PrfWriteProfileData(HINI_USERPROFILE,
                        (PSZ)INIAPP_XWORKPLACE, (PSZ)INIKEY_ACCELERATORS,
                        &G_FolderHotkeys,
                        (i2+1) * sizeof(XFLDHOTKEY));
}

/*
 *@@ fdrFindHotkey:
 *      searches the hotkeys list for whether
 *      a hotkey has been defined for the specified
 *      command (WM_COMMAND msg value).
 *
 *      If so, TRUE is returned and the hotkey
 *      definition is stored in the specified two
 *      USHORT's.
 *
 *      Otherwise, FALSE is returned.
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 */

BOOL fdrFindHotkey(USHORT usCommand,
                   PUSHORT pusFlags,
                   PUSHORT pusKeyCode)
{
    ULONG   i = 0;
    BOOL    brc = FALSE;
    // go thru all hotkeys
    while (G_FolderHotkeys[i].usCommand)
    {
        if (G_FolderHotkeys[i].usCommand == usCommand)
        {
            // found:
            *pusFlags = G_FolderHotkeys[i].usFlags;
            *pusKeyCode = G_FolderHotkeys[i].usKeyCode;
            brc = TRUE;
            break;
        }
        i++;
    }

    return brc;
}

/*
 *@@ fdrProcessFldrHotkey:
 *      this is called by fnwpSubclWPFolderWindow to
 *      check for whether a given WM_CHAR message matches
 *      one of the folder hotkeys.
 *
 *      The parameters are those of the WM_CHAR message. This
 *      returns TRUE if the pressed key was a hotkey; in that
 *      case, the corresponding WM_COMMAND message is
 *      automatically posted to the folder frame, which will
 *      cause the defined action to occur (that is, the WPS
 *      will call the proper wpMenuItemSelected method).
 *
 *      Note that you should pass in WM_CHAR even if KC_KEYUP
 *      is not set to be able to determine whether the key
 *      should be swallowed (V1.0.0). We check for the flag
 *      here and post the message only if the flag is clear,
 *      but return TRUE (for "swallow the message") in both
 *      cases.
 *
 *@@changed V0.9.0 [umoeller]: moved this here from common.c
 *@@changed V0.9.1 (2000-01-31) [umoeller]: changed prototype; this was using MPARAMS previously
 *@@changed V0.9.9 (2001-02-28) [pr]: allow multiple actions on same hotkey
 *@@changed V0.9.14 (2001-07-28) [umoeller]: now disabling sort and arrange hotkeys for desktop, if those menu items are disabled
 *@@changed V0.9.19 (2002-04-17) [umoeller]: adjusted for new menu handling
 *@@changed V1.0.0 (2002-08-24) [umoeller]: fixed key up/down processing
 */

BOOL fdrProcessFldrHotkey(WPFolder *somSelf,
                          HWND hwndFrame,   // in: folder frame
                          USHORT usFlags,
                          USHORT usch,
                          USHORT usvk)
{
    BOOL brc = FALSE;

    // now check if the key is relevant: filter out KEY UP
    // messages and check if either a virtual key (such as F5)
    // or Ctrl or Alt was pressed
    if (    // ((usFlags & KC_KEYUP) == 0)      nope, process both key down and up
            //                                  V1.0.0 (2002-08-24) [umoeller]
            (     ((usFlags & KC_VIRTUALKEY) != 0)
                  // Ctrl pressed?
               || ((usFlags & KC_CTRL) != 0)
                  // Alt pressed?
               || ((usFlags & KC_ALT) != 0)
                  // or one of the Win95 keys?
               || (   ((usFlags & KC_VIRTUALKEY) == 0)
                   && (     (usch == 0xEC00)
                        ||  (usch == 0xED00)
                        ||  (usch == 0xEE00)
                      )
                  )
            )
       )
    {
        USHORT  us = 0;
        USHORT  usKeyCode,
                usAllFlags;

        USHORT  usCommand;

        if (usFlags & KC_VIRTUALKEY)
            usKeyCode = usvk;
        else
            usKeyCode = usch;

        // filter out unwanted flags
        usAllFlags = usFlags;
        usFlags &= (KC_VIRTUALKEY | KC_CTRL | KC_ALT | KC_SHIFT);

        PMPF_KEYS(("hwndFrame: 0x%lX, usKeyCode: 0x%lX, usFlags: 0x%lX",
                    hwndFrame,
                    usKeyCode,
                    usFlags));

        // now go through the global accelerator list and check
        // if the pressed key was assigned an action to;
        // the array is terminated with a null usCommand
        while (usCommand = G_FolderHotkeys[us].usCommand)
        {
            if (      (G_FolderHotkeys[us].usFlags == usFlags)
                   && (G_FolderHotkeys[us].usKeyCode == usKeyCode)
               )
            {
                // OK: this is a hotkey...

                BOOL                fPost = TRUE;

                // find the corresponding
                // "command" (= menu ID) and post it to the frame
                // window, which will execute it

                // now, if sort or arrange are disabled for
                // the desktop and this is a sort or arrange
                // hotkey, swallow it
                // V0.9.14 (2001-07-28) [umoeller]
                if (cmnIsADesktop(somSelf))
                {
                    switch (usCommand)
                    {
                        case ID_WPMI_SORTBYNAME:
                        case ID_WPMI_SORTBYSIZE:
                        case ID_WPMI_SORTBYTYPE:
                        case ID_WPMI_SORTBYREALNAME:
                        case ID_WPMI_SORTBYWRITEDATE:
                        case ID_WPMI_SORTBYACCESSDATE:
                        case ID_WPMI_SORTBYCREATIONDATE:
                        case ID_XFMI_OFS_SORTBYEXT:
                        case ID_XFMI_OFS_SORTFOLDERSFIRST:
                        case ID_XFMI_OFS_SORTBYCLASS:
                            if (cmnQuerySetting(sflMenuDesktopWPS) & CTXT_SORT)
                                fPost = FALSE;
                        break;

                        case WPMENUID_ARRANGE:
                        case WPMENUID_ARRANGETOP: // ID_WPMI_ARRANGEFROMTOP:
                        case WPMENUID_ARRANGELEFT: // ID_WPMI_ARRANGEFROMLEFT:
                        case WPMENUID_ARRANGERIGHT: // ID_WPMI_ARRANGEFROMRIGHT:
                        case WPMENUID_ARRANGEBOTTOM: // ID_WPMI_ARRANGEFROMBOTTOM:
                        case WPMENUID_PERIMETER: // ID_WPMI_ARRANGEPERIMETER:
                        case WPMENUID_SELECTEDHORZ: // ID_WPMI_ARRANGEHORIZONTALLY:
                        case WPMENUID_SELECTEDVERT: // ID_WPMI_ARRANGEVERTICALLY:
                            if (cmnQuerySetting(sflMenuDesktopWPS) & CTXT_ARRANGE)
                                fPost = FALSE;
                        break;

                        // disallow close shortcut on desktop too
                        // because that would now open the shutdown
                        // window V0.9.16 (2002-01-04) [umoeller]
                        case ID_XFMI_OFS_CLOSE:
                            fPost = FALSE;
                    }
                }

                if (fPost)
                {
                    if (    (usCommand >= WPMENUID_USER)
                         && (usCommand < WPMENUID_USER + FIRST_VARIABLE)
                       )
                    {
                        // it's one of the "variable" menu items:
                        // add the global variable menu offset
                        usCommand += *G_pulVarMenuOfs;
                    }

                    // post only if this is a "key down" message
                    // V1.0.0 (2002-08-24) [umoeller]
                    if (!(usAllFlags & KC_KEYUP))
                    {
                        WinPostMsg(hwndFrame,
                                   WM_COMMAND,
                                   (MPARAM)usCommand,
                                   MPFROM2SHORT(CMDSRC_MENU,
                                                FALSE)  // results from keyboard operation
                                  );

                        PMPF_KEYS(("  posting command 0x%lX", usCommand));
                    }
                }

                // return TRUE even if we did NOT post;
                // otherwise the WM_CHAR will be processed
                // by parent winproc
                brc = TRUE;
            }

            us++;
        }
    }

    return brc;
}

/* ******************************************************************
 *
 *   Notebook callbacks (notebook.c) for XFldWPS "Hotkeys" page
 *
 ********************************************************************/

#pragma pack(1)

/*
 *@@ FLDRHOTKEYDESC:
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 */

typedef struct _FLDRHOTKEYDESC
{
    USHORT      ulStringID;         // string ID for listbox
    USHORT      usPostCommand,      // command to post / store with hotkey
                usMenuCommand;      // menu item to append hotkey to
    BYTE        bWarp4;             // TRUE if item requires Warp 4
} FLDRHOTKEYDESC, *PFLDRHOTKEYDESC;

#pragma pack()

/*
 *@@ G_szLBEntries:
 *
 *
 */

CHAR  G_szLBEntries[FLDRHOTKEYCOUNT][MAXLBENTRYLENGTH];

/*
 *@@ G_aDescriptions:
 *      array of FLDRHOTKEYDESC items.
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 */

FLDRHOTKEYDESC G_aDescriptions[FLDRHOTKEYCOUNT] =
    {
         ID_XSSI_LB_REFRESHNOW, WPMENUID_REFRESH, WPMENUID_REFRESH, FALSE,
         ID_XSSI_LB_SNAPTOGRID, ID_XFMI_OFS_SNAPTOGRID, ID_XFMI_OFS_SNAPTOGRID, FALSE,
         ID_XSSI_LB_SELECTALL, WPMENUID_SELALL, WPMENUID_SELALL, FALSE,
         ID_XSSI_LB_OPENPARENTFOLDER, ID_XFMI_OFS_OPENPARENT, 0x2CA, FALSE,

         ID_XSSI_LB_OPENSETTINGSNOTEBOOK, WPMENUID_PROPERTIES, WPMENUID_PROPERTIES, FALSE,

         ID_XSSI_LB_OPENNEWDETAILSVIEW, WPMENUID_DETAILS, WPMENUID_DETAILS, FALSE,
         ID_XSSI_LB_OPENNEWICONVIEW, WPMENUID_ICON, WPMENUID_ICON, FALSE,

         ID_XSSI_LB_DESELECTALL, WPMENUID_DESELALL, WPMENUID_DESELALL, FALSE,

         ID_XSSI_LB_OPENNEWTREEVIEW, WPMENUID_TREE, WPMENUID_TREE, FALSE,

         ID_XSSI_LB_FIND, WPMENUID_FIND, WPMENUID_FIND, FALSE,

         ID_XSSI_LB_PICKUP, WPMENUID_PICKUP, WPMENUID_PICKUP, TRUE, // was FALSE V1.0.0 (2002-09-05) [lafaix]
         ID_XSSI_LB_PICKUPCANCELDRAG, WPMENUID_PUTDOWN_CANCEL, WPMENUID_PUTDOWN_CANCEL, TRUE, // was FALSE V1.0.0 (2002-09-05) [lafaix]

         ID_XSSI_LB_SORTBYNAME, ID_WPMI_SORTBYNAME, ID_WPMI_SORTBYNAME, FALSE,
         ID_XSSI_LB_SORTBYSIZE, ID_WPMI_SORTBYSIZE, ID_WPMI_SORTBYSIZE, FALSE,
         ID_XSSI_LB_SORTBYTYPE, ID_WPMI_SORTBYTYPE, ID_WPMI_SORTBYTYPE, FALSE,
         ID_XSSI_LB_SORTBYREALNAME, ID_WPMI_SORTBYREALNAME, ID_WPMI_SORTBYREALNAME, FALSE,
         ID_XSSI_LB_SORTBYWRITEDATE, ID_WPMI_SORTBYWRITEDATE, ID_WPMI_SORTBYWRITEDATE, FALSE,
         ID_XSSI_LB_SORTBYACCESSDATE, ID_WPMI_SORTBYACCESSDATE, ID_WPMI_SORTBYACCESSDATE, FALSE,
         ID_XSSI_LB_SORTBYCREATIONDATE, ID_WPMI_SORTBYCREATIONDATE, ID_WPMI_SORTBYCREATIONDATE, FALSE,

         ID_XSSI_LB_SWITCHTOICONVIEW, WPMENUID_CHANGETOICON, WPMENUID_CHANGETOICON, // ID_WPMI_SHOWICONVIEW, ID_WPMI_SHOWICONVIEW,
            TRUE,
         ID_XSSI_LB_SWITCHTODETAILSVIEW, WPMENUID_CHANGETODETAILS, WPMENUID_CHANGETODETAILS, // ID_WPMI_SHOWDETAILSVIEW, ID_WPMI_SHOWDETAILSVIEW,
            TRUE,
         ID_XSSI_LB_SWITCHTOTREEVIEW, WPMENUID_CHANGETOTREE, WPMENUID_CHANGETOTREE, // ID_WPMI_SHOWTREEVIEW, ID_WPMI_SHOWTREEVIEW,
            TRUE,

         ID_XSSI_LB_ARRANGEDEFAULT, WPMENUID_ARRANGE, WPMENUID_ARRANGE, FALSE,
         ID_XSSI_LB_ARRANGEFROMTOP, WPMENUID_ARRANGETOP, WPMENUID_ARRANGETOP, // ID_WPMI_ARRANGEFROMTOP, ID_WPMI_ARRANGEFROMTOP,
            TRUE,
         ID_XSSI_LB_ARRANGEFROMLEFT, WPMENUID_ARRANGELEFT, WPMENUID_ARRANGELEFT, // ID_WPMI_ARRANGEFROMLEFT, ID_WPMI_ARRANGEFROMLEFT,
            TRUE,
         ID_XSSI_LB_ARRANGEFROMRIGHT, WPMENUID_ARRANGERIGHT, WPMENUID_ARRANGERIGHT, // ID_WPMI_ARRANGEFROMRIGHT, ID_WPMI_ARRANGEFROMRIGHT,
            TRUE,
         ID_XSSI_LB_ARRANGEFROMBOTTOM, WPMENUID_ARRANGEBOTTOM, WPMENUID_ARRANGEBOTTOM, // ID_WPMI_ARRANGEFROMBOTTOM, ID_WPMI_ARRANGEFROMBOTTOM,
            TRUE,
         ID_XSSI_LB_ARRANGEPERIMETER, WPMENUID_PERIMETER, WPMENUID_PERIMETER, // ID_WPMI_ARRANGEPERIMETER, ID_WPMI_ARRANGEPERIMETER,
            TRUE,
         ID_XSSI_LB_ARRANGEHORIZONTALLY, WPMENUID_SELECTEDHORZ, WPMENUID_SELECTEDHORZ, // ID_WPMI_ARRANGEHORIZONTALLY, ID_WPMI_ARRANGEHORIZONTALLY,
            TRUE,
         ID_XSSI_LB_ARRANGEVERTICALLY, WPMENUID_SELECTEDVERT, WPMENUID_SELECTEDVERT, // ID_WPMI_ARRANGEVERTICALLY, ID_WPMI_ARRANGEVERTICALLY,
            TRUE,

         ID_XSSI_LB_INSERT, WPMENUID_PASTE, WPMENUID_PASTE, // ID_WPMI_PASTE, ID_WPMI_PASTE,
            TRUE,

         ID_XSSI_LB_SORTBYEXTENSION, ID_XFMI_OFS_SORTBYEXT, ID_XFMI_OFS_SORTBYEXT, FALSE,
         ID_XSSI_LB_OPENPARENTFOLDERANDCLOSE, ID_XFMI_OFS_OPENPARENTANDCLOSE, ID_XFMI_OFS_OPENPARENTANDCLOSE, FALSE,

         ID_XSSI_LB_CLOSEWINDOW, ID_XFMI_OFS_CLOSE, ID_XFMI_OFS_CLOSE, FALSE,
         ID_XSSI_LB_SELECTSOME, ID_XFMI_OFS_SELECTSOME, ID_XFMI_OFS_SELECTSOME, FALSE,
         ID_XSSI_LB_SORTFOLDERSFIRST, ID_XFMI_OFS_SORTFOLDERSFIRST, ID_XFMI_OFS_SORTFOLDERSFIRST, FALSE,
         ID_XSSI_LB_SORTBYCLASS, ID_XFMI_OFS_SORTBYCLASS, ID_XFMI_OFS_SORTBYCLASS, FALSE,

         ID_XSSI_LB_CONTEXTMENU, ID_XFMI_OFS_CONTEXTMENU, ID_XFMI_OFS_CONTEXTMENU, FALSE,
         ID_XSSI_LB_TASKLIST, 0x8011, 0, FALSE,

         ID_XSSI_LB_COPYFILENAME_SHORT, ID_XFMI_OFS_COPYFILENAME_SHORTSP, 0, FALSE,
         ID_XSSI_LB_COPYFILENAME_FULL, ID_XFMI_OFS_COPYFILENAME_FULLSP, 0, FALSE,

         // V1.0.0 (2002-09-05) [lafaix]
         ID_XSSI_LB_PICKUPDROPCOPY, WPMENUID_PUTDOWN_COPY, WPMENUID_PUTDOWN_COPY, TRUE,
         ID_XSSI_LB_PICKUPDROPMOVE, WPMENUID_PUTDOWN_MOVE, WPMENUID_PUTDOWN_MOVE, TRUE,
         ID_XSSI_LB_PICKUPDROPLINK, WPMENUID_PUTDOWN_LINK, WPMENUID_PUTDOWN_LINK, TRUE
    };

/*
 *@@ AddHotkeyToMenuItem:
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 *@@changed V0.9.19 (2002-04-17) [umoeller]: adjusted for new menu
 *@@changed V1.0.9 (2012-02-27) [pr]: support hotkey description @@fixes 249
 */

STATIC VOID AddHotkeyToMenuItem(HWND hwndMenu,
                                USHORT usPostCommand2Find,
                                USHORT usMenuCommand)
{
    USHORT  usFlags, usKeyCode;
    CHAR    szDescription[100];

    if (fdrFindHotkey(usPostCommand2Find,
                      &usFlags,
                      &usKeyCode))
    {
        if (    (usMenuCommand >= WPMENUID_USER)
             && (usMenuCommand < WPMENUID_USER + FIRST_VARIABLE)
           )
            // it's one of the "variable" menu items:
            // add the global variable menu offset
            usMenuCommand += *G_pulVarMenuOfs;

        cmnDescribeKey(szDescription,
                       usFlags,
                       usKeyCode,
                       NULL, // V1.0.9
                       sizeof(szDescription));

        winhAppend2MenuItemText(hwndMenu,
                                usMenuCommand,
                                szDescription,
                                TRUE);
    }
}

/*
 *@@ fdrAddHotkeysToPulldown:
 *      gets called from our menu hacks for WM_INITMENU
 *      to add hotkeys to the "Edit" pulldown properly
 *      as well.
 *
 *@@added V0.9.20 (2002-08-08) [umoeller]
 */

VOID fdrAddHotkeysToPulldown(HWND hwndPulldown,     // in: submenu handle
                             const ULONG *paulMenuIDs,    // in: array of menu IDs to test
                             ULONG cMenuIDs)        // in: array item count
{
    if (
#ifndef __ALWAYSFDRHOTKEYS__
            (cmnQuerySetting(sfFolderHotkeys))
         &&
#endif
            (cmnQuerySetting(sfShowHotkeysInMenus))
        )
    {
        ULONG ul;
        for (ul = 0;
             ul < cMenuIDs;
             ++ul)
        {
            AddHotkeyToMenuItem(hwndPulldown,
                                paulMenuIDs[ul],
                                paulMenuIDs[ul]);
        }
    }
}

/*
 *@@ fdrAddHotkeysToMenu:
 *      gets called by XFldObject::wpModifyPopupMenu to add
 *      hotkey descriptions to the popup menu.
 *
 *      Note that this adds the generic hotkeys for WPObject
 *      only.
 *
 *@@added V0.9.2 (2000-03-06) [umoeller]
 *@@changed V0.9.4 (2000-06-11) [umoeller]: hotkeys showed up even if hotkeys were globally disabled; fixed
 *@@changed V0.9.19 (2002-04-17) [umoeller]: adjusted for new menu handling
 *@@changed V1.0.0 (2002-08-31) [umoeller]: changed prototype for Warp 4 method overrides
 *@@changed V1.0.9 (2012-02-27) [pr]: support hotkey description @@fixes 249
 */

VOID fdrAddHotkeysToMenu(WPObject *somSelf,
                         HWND hwndCnr,
                         HWND hwndMenu,     // in: menu created by wpDisplayMenu
                         ULONG ulMenuType)  // in: menu type from wpModifyMenu
{
    if (
#ifndef __ALWAYSFDRHOTKEYS__
            (cmnQuerySetting(sfFolderHotkeys)) // V0.9.4 (2000-06-11) [umoeller]
         &&
#endif
            (cmnQuerySetting(sfShowHotkeysInMenus))
       )
    {
        CHAR    szDescription[100];
        ULONG   flMenuXWP = cmnQuerySetting(mnuQueryMenuXWPSetting(somSelf));

        PMPF_MENUS(("hwndMenu 0x%lX, id 0x%lX",
                    hwndMenu,
                    WinQueryWindowUShort(hwndMenu, QWS_ID)));

        switch (ulMenuType)
        {
            case MENU_OBJECTPOPUP:
            case MENU_SELECTEDPULLDOWN:
                // delete
                winhAppend2MenuItemText(hwndMenu,
                                        WPMENUID_DELETE,
                                        cmnGetString(ID_XSSI_KEY_DELETE),  // pszDelete
                                        TRUE);

                // open settings
                cmnDescribeKey(szDescription,
                               KC_ALT | KC_VIRTUALKEY,
                               VK_ENTER,
                               NULL, // V1.0.9
                               sizeof(szDescription));
                winhAppend2MenuItemText(hwndMenu,
                                        WPMENUID_PROPERTIES,
                                        szDescription,
                                        TRUE);

                // default help
                cmnDescribeKey(szDescription,
                               KC_VIRTUALKEY,
                               VK_F1,
                               NULL, // V1.0.9
                               sizeof(szDescription));
                winhAppend2MenuItemText(hwndMenu,
                                        WPMENUID_EXTENDEDHELP,
                                        szDescription,
                                        TRUE);

                // copy filename
                if (!(flMenuXWP & XWPCTXT_COPYFILENAME))
                {
                    AddHotkeyToMenuItem(hwndMenu,
                                        ID_XFMI_OFS_COPYFILENAME_SHORTSP,
                                        ID_XFMI_OFS_COPYFILENAME_SHORTSP); // ID_XFMI_OFS_COPYFILENAME_MENU);
                    AddHotkeyToMenuItem(hwndMenu,
                                        ID_XFMI_OFS_COPYFILENAME_FULLSP,
                                        ID_XFMI_OFS_COPYFILENAME_FULLSP); // ID_XFMI_OFS_COPYFILENAME_MENU); // same menu item!
                }
            break;

            case MENU_OPENVIEWPOPUP:
            case MENU_FOLDERPULLDOWN:
            {
                // menu on cnr whitespace:
                ULONG       ul;

                for (ul = 0;
                     ul < ARRAYITEMCOUNT(G_aDescriptions);
                     ++ul)
                {
                    // menu modification allowed for this command?
                    if (G_aDescriptions[ul].usMenuCommand)
                    {
                        AddHotkeyToMenuItem(hwndMenu,
                                            G_aDescriptions[ul].usPostCommand, // usPostCommand2Find
                                            G_aDescriptions[ul].usMenuCommand); // usMenuCommand
                    }
                }

#ifndef __NOMOVEREFRESHNOW__
                if (!(flMenuXWP & XWPCTXT_REFRESH_IN_MAIN))
                    AddHotkeyToMenuItem(hwndMenu,
                                        WPMENUID_REFRESH,
                                        ID_XFMI_OFS_REFRESH);
#endif
            }
            break;
        }
    } // end if (cmnQuerySetting(sfShowHotkeysInMenus))
}

/* ******************************************************************
 *
 *   Folder hotkeys page notebook functions (notebook.c)
 *
 ********************************************************************/

typedef struct _SUBCLHOTKEYEF
{
    PFNWP       pfnwpOrig;

    HWND        hwndSet,
                hwndClear;

    USHORT      usFlags;        // in: as in WM_CHAR
    USHORT      usKeyCode;      // in: if KC_VIRTUAL is set, this has usKeyCode;
                                //     otherwise usCharCode

} SUBCLHOTKEYEF, *PSUBCLHOTKEYEF;

/*
 *@@ fnwpFolderHotkeyEntryField:
 *      this is the window proc for the subclassed entry
 *      field on the "Folder Hotkeys" notebook page. We will
 *      intercept all WM_CHAR messages and set the entry field
 *      display to the key description instead of the character.
 *      Moreover, we will update the global folder hotkey array
 *      according to the currently selected listbox item on that
 *      page.
 *
 *@@changed V0.9.0 [umoeller]: renamed from fnwpHotkeyEntryField
 *@@changed V0.9.9 (2001-04-04) [umoeller]: added "set" support
 *@@changed V1.0.0 (2002-09-10) [lafaix]: no longer freeing pshef
 *@@changed V1.0.9 (2012-02-27) [pr]: support hotkey description @@fixes 249
 */

STATIC MRESULT EXPENTRY fnwpFolderHotkeyEntryField(HWND hwndEdit, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    // get original wnd proc; this was stored in the
    // window words in xfwps.c
    PSUBCLHOTKEYEF pshef = (PSUBCLHOTKEYEF)WinQueryWindowPtr(hwndEdit, QWL_USER);

    PFNWP OldEditProc = pshef->pfnwpOrig;

    MRESULT mrc = (MPARAM)FALSE; // WM_CHAR not-processed flag

    switch (msg)
    {
        case WM_CHAR:
        {
            // USHORT usCommand;
            USHORT usKeyCode;
            USHORT usFlags    = SHORT1FROMMP(mp1);
            USHORT usch       = SHORT1FROMMP(mp2);
            USHORT usvk       = SHORT2FROMMP(mp2);

            if ((usFlags & KC_KEYUP) == 0)
            {
                if (    (    (usFlags & KC_VIRTUALKEY)
                          && (    (usvk == VK_TAB)
                               || (usvk == VK_BACKTAB)
                             )
                        )
                     || (    ((usFlags & (KC_CTRL | KC_SHIFT | KC_ALT)) == KC_ALT)
                          && (   (WinSendMsg(pshef->hwndSet,
                                             WM_MATCHMNEMONIC,
                                             (MPARAM)usch,
                                             0))
                              || (WinSendMsg(pshef->hwndClear,
                                             WM_MATCHMNEMONIC,
                                             (MPARAM)usch,
                                             0))
                             )
                        )
                   )
                {
                    // pass those to owner
                    WinPostMsg(WinQueryWindow(hwndEdit, QW_OWNER),
                               msg,
                               mp1,
                               mp2);
                }
                /*
                else if (     ((usFlags & KC_VIRTUALKEY) != 0)
                           || ((usFlags & KC_CTRL) != 0)
                           || ((usFlags & KC_ALT) != 0)
                           || (   ((usFlags & KC_VIRTUALKEY) == 0)
                               && (     (usch == 0xEC00)
                                    ||  (usch == 0xED00)
                                    ||  (usch == 0xEE00)
                                  )
                              )
                        )
                */
                else
                {
                    usFlags &= (KC_VIRTUALKEY | KC_CTRL | KC_ALT | KC_SHIFT);
                    if (usFlags & KC_VIRTUALKEY)
                        usKeyCode = usvk;
                    else
                        usKeyCode = usch;

                    if (cmnIsValidHotkey(usFlags,
                                         usKeyCode))
                    {
                        // looks like a valid hotkey:
                        CHAR    szKeyName[100];

                        pshef->usFlags = usFlags;
                        pshef->usKeyCode = usKeyCode;

                        cmnDescribeKey(szKeyName,
                                       usFlags,
                                       usKeyCode,
                                       NULL, // V1.0.9
                                       sizeof(szKeyName));
                        WinSetWindowText(hwndEdit, szKeyName);

                        WinEnableWindow(pshef->hwndSet, TRUE);
                    }
                    else
                        WinEnableWindow(pshef->hwndSet, FALSE);

                    WinEnableWindow(pshef->hwndClear, TRUE);
                }
            }

            mrc = (MPARAM)TRUE;     // processed
        }
        break;

        default:
            mrc = OldEditProc(hwndEdit, msg, mp1, mp2);
    }

    return mrc;
}

static const XWPSETTING G_HotkeysBackup[] =
    {
        sfFolderHotkeysDefault,
        sfShowHotkeysInMenus
    };

#define HOTKEYWIDTH       70
#define ACTIONWIDTH       70
#define PARAMWIDTH        100

static const CONTROLDEF
    DefinitionGroup = LOADDEF_GROUP(ID_XSDI_HOTKEY_GROUP, SZL_AUTOSIZE),
    HotkeyTxt = CONTROLDEF_TEXT_CENTER(
                            LOAD_STRING,
                            ID_XSDI_HOTKEY_HOTKEY,
                            HOTKEYWIDTH,
                            -1),
    Hotkey = CONTROLDEF_ENTRYFIELD(
                            LOAD_STRING,
                            ID_XSDI_HOTKEY_HOTKEY_EF,
                            HOTKEYWIDTH,
                            -1),
    ActionTxt = CONTROLDEF_TEXT_CENTER(
                            LOAD_STRING,
                            ID_XSDI_HOTKEY_ACTION,
                            ACTIONWIDTH,
                            -1),
#undef ACTIONDROP
#ifdef ACTIONDROP
    ActionList = CONTROLDEF_DROPDOWNLIST(
                            ID_XSDI_HOTKEY_ACTION_DROP,
                            ACTIONWIDTH,
                            70),
#endif
    ParamTxt = CONTROLDEF_TEXT_CENTER(
                            LOAD_STRING,
                            ID_XSDI_HOTKEY_PARAM,
                            PARAMWIDTH,
                            -1),
    ParamList = CONTROLDEF_DROPDOWNLIST(
                            ID_XSDI_HOTKEY_PARAM_DROP,
                            PARAMWIDTH,
                            100);

static const DLGHITEM dlgAddHotkey[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                START_GROUP_TABLE(&DefinitionGroup),
                    START_ROW(0),
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&HotkeyTxt),
                            START_ROW(0),
                                CONTROL_DEF(&Hotkey),
                        END_TABLE,
#ifdef ACTIONDROP
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&ActionTxt),
                            START_ROW(0),
                                CONTROL_DEF(&ActionList),
                        END_TABLE,
#endif
                        START_TABLE,
                            START_ROW(0),
                                CONTROL_DEF(&ParamTxt),
                            START_ROW(0),
                                CONTROL_DEF(&ParamList),
                        END_TABLE,
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),           // common.c
                CONTROL_DEF(&G_CancelButton),       // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

/*
 *@@ fnwpHotkeyRecord:
 *
 *@@added V1.0.0 (2002-09-09) [lafaix]
 */

STATIC MRESULT EXPENTRY fnwpEditHotkeyRecord(HWND hwndDlg,
                                             ULONG msg,
                                             MPARAM mp1,
                                             MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_COMMAND:
        {
            BOOL fDismiss = TRUE;

            if (SHORT1FROMMP(mp1) == DID_OK)
            {
                // before allowing OK, check if the key is valid
                HWND hwnd = WinWindowFromID(hwndDlg, ID_XSDI_HOTKEY_HOTKEY_EF);
                PSUBCLHOTKEYEF pshef = (PSUBCLHOTKEYEF)WinQueryWindowPtr(hwnd, QWL_USER);

                if (    (pshef->usFlags == 0)
                     && (pshef->usKeyCode == 0)
                   )
                {
                    cmnErrorMsgBox(hwndDlg,
                                   0,
                                   253,
                                   MB_OK,
                                   TRUE);
                    fDismiss = FALSE;

                    WinSetFocus(HWND_DESKTOP, hwnd);
                }
                else
                {
                    hwnd = WinWindowFromID(hwndDlg, ID_XSDI_HOTKEY_PARAM_DROP);

                    if (WinQueryWindowTextLength(hwnd) == 0)
                    {
                        cmnErrorMsgBox(hwndDlg,
                                       0,
                                       254,
                                       MB_OK,
                                       TRUE);
                        fDismiss = FALSE;

                        WinSetFocus(HWND_DESKTOP, hwnd);
                    }
                }
            }

            if (fDismiss)
                WinDismissDlg(hwndDlg, SHORT1FROMMP(mp1));
        }
        break;

        case WM_HELP:
            cmnDisplayHelp(NULL,
                           ID_XSH_SETTINGS_FDRHOTKEYS_DLG); // V1.0.0 (2002-09-12) [umoeller]
        break;

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ HOTKEYRECORD:
 *      extended record core for "Hotkeys" container.
 *
 *@@added V1.0.0 (2002-09-05) [lafaix]
 */

typedef struct _HOTKEYRECORD
{
    RECORDCORE  recc;
    CHAR        szHotkeyName[200];
    PSZ         pcszKeyName;
    PSZ         pcszPlugin;
    PSZ         pcszParameters;
    USHORT      usKey;
    USHORT      usFlags;
    ULONG       ulCommand;
} HOTKEYRECORD, *PHOTKEYRECORD;

/*
 *@@ fdrSaveFldrHotkeys:
 *       this saves the folder hotkeys contained in the
 *       container to OS2.INI.
 *
 *@@added V1.0.0 (2002-09-11) [lafaix]
 */

void fdrSaveFldrHotkeys(HWND hwndCnr)
{
    PHOTKEYRECORD   pRec2 = NULL;
    USHORT          usCmd = CMA_FIRST;
    BOOL            fCont = TRUE;
    USHORT          usHotkeyIndex = 0;      // raised with each iteration

    // go through all records
    do
    {
        pRec2 = (PHOTKEYRECORD)WinSendMsg(hwndCnr,
                                          CM_QUERYRECORD,
                                          pRec2, // ignored on first call
                                          MPFROM2SHORT(usCmd,     // CMA_FIRST or CMA_NEXT
                                                       CMA_ITEMORDER));
        usCmd = CMA_NEXT;

        if ((pRec2) && ((ULONG)pRec2 != -1))
        {
            G_FolderHotkeys[usHotkeyIndex].usKeyCode = pRec2->usKey;
            G_FolderHotkeys[usHotkeyIndex].usFlags = pRec2->usFlags;
            G_FolderHotkeys[usHotkeyIndex].usCommand = pRec2->ulCommand;
            usHotkeyIndex++;
        }
        else
            fCont = FALSE;

    } while (fCont);

    // mark the end of the hotkeys array
    if (usHotkeyIndex < FLDRHOTKEYCOUNT)
        G_FolderHotkeys[usHotkeyIndex].usCommand = 0;

    // saving to disk
    fdrStoreFldrHotkeys();
}

/*
 *@@ AdjustStickyRecord:
 *      adjusts the pcsz* values in the HOTKEYRECORD.
 *
 *@@added V1.0.0 (2002-09-15) [lafaix]
 *@@changed V1.0.9 (2012-02-27) [pr]: support hotkey description @@fixes 249
 */

STATIC VOID AdjustHotkeyRecord(PHOTKEYRECORD pRec)
{
    USHORT i = 0;

    pRec->pcszPlugin = "Command";

    cmnDescribeKey(pRec->szHotkeyName,
                   pRec->usFlags,
                   pRec->usKey,
                   NULL, // V1.0.9
                   sizeof(pRec->szHotkeyName));

    pRec->pcszKeyName = pRec->szHotkeyName;
    pRec->pcszParameters = "n/a";

    while (i < FLDRHOTKEYCOUNT)
    {
        if (pRec->ulCommand == G_aDescriptions[i].usPostCommand)
        {
            pRec->pcszParameters = G_szLBEntries[i];
            break;
        }
        i++;
    }
}

/*
 *@@ AddHotkeyRecord:
 *      creates and inserts a HOTKEYRECORD for the given
 *      container with the specified title.
 *
 *@@added V1.0.0 (2002-09-05) [lafaix]
 */

STATIC VOID AddHotkeyRecord(HWND hwndCnr,
                            USHORT usKey,          // in: key scancode
                            USHORT usFlags,         // in: key modifiers (VK_*)
                            ULONG ulCommand,       // in: command
                            BOOL fInvalidate)      // in: if TRUE, invalidate records
{
    PHOTKEYRECORD pRec;

    if (pRec = (PHOTKEYRECORD)cnrhAllocRecords(hwndCnr,
                                               sizeof(HOTKEYRECORD),
                                               1))
    {
        pRec->usKey = usKey;
        pRec->usFlags = usFlags;
        pRec->ulCommand = ulCommand;

        AdjustHotkeyRecord(pRec);

        cnrhInsertRecords(hwndCnr,
                          NULL, // parent
                          (PRECORDCORE)pRec,
                          fInvalidate,
                          pRec->szHotkeyName,
                          CRA_RECORDREADONLY,
                          1);   // count
    }
}

/*
 *@@ EditHotkeyRecord:
 *      edit and possibly inserts a HOTKEYRECORD.
 *
 *@@added V1.0.0 (2002-09-08) [lafaix]
 */

STATIC VOID EditHotkeyRecord(PHOTKEYRECORD pRec,
                             PNOTEBOOKPAGE pnbp,
                             HWND hwndCnr,
                             BOOL fInsert)
{
    HWND        hwndDlg;

    if (!dlghCreateDlg(&hwndDlg,
                       pnbp->hwndDlgPage,
                       FCF_FIXED_DLG,
                       fnwpEditHotkeyRecord,
                       cmnGetString(fInsert
                                        ? ID_XSDI_HOTKEY_ADDTITLE
                                        : ID_XSDI_HOTKEY_EDITTITLE),
                       dlgAddHotkey,
                       ARRAYITEMCOUNT(dlgAddHotkey),
                       NULL,
                       cmnQueryDefaultFont()))
    {
        ULONG ul;
        HWND  hwndItem = WinWindowFromID(hwndDlg, ID_XSDI_HOTKEY_HOTKEY_EF);
        SUBCLHOTKEYEF shef;
        USHORT usOldFlags, usOldKey;

        winhCenterWindow(hwndDlg);

        // filling in the key field
        memset(&shef, 0, sizeof(shef));
        WinSetWindowPtr(hwndItem, QWL_USER, &shef);
        shef.pfnwpOrig = WinSubclassWindow(hwndItem, fnwpFolderHotkeyEntryField);

        shef.hwndSet = WinWindowFromID(pnbp->hwndDlgPage, DID_OK);
        shef.hwndClear = WinWindowFromID(pnbp->hwndDlgPage, DID_CANCEL);
        usOldFlags = shef.usFlags = pRec->usFlags;
        usOldKey = shef.usKeyCode = pRec->usKey;

        WinSetWindowText(hwndItem, pRec->pcszKeyName);

#ifdef ACTIONDROP
        // filling the possible actions (just Command currently)
        hwndItem = WinWindowFromID(hwndDlg, ID_XSDI_HOTKEY_ACTION_DROP);
        WinInsertLboxItem(hwndItem,
                          0,
                          "Command"); // @@todo localize
        WinSendMsg(hwndItem, LM_SELECTITEM, MPFROMSHORT(0), MPFROMSHORT(TRUE));
#endif

        hwndItem = WinWindowFromID(hwndDlg, ID_XSDI_HOTKEY_PARAM_DROP);
        for (ul = 0;
             ul < FLDRHOTKEYCOUNT;
             ul++)
        {
            if (    G_szLBEntries[ul]
                 && (    (G_fIsWarp4)
                      || (!G_aDescriptions[ul].bWarp4)
                    )
               )
            {
                WinInsertLboxItem(hwndItem,
                                  LIT_SORTASCENDING,
                                  G_szLBEntries[ul]);
            }
        }
        WinSetWindowText(hwndItem, pRec->pcszParameters);

        if (WinProcessDlg(hwndDlg) == DID_OK)
        {
            CHAR szCommand[MAXLBENTRYLENGTH];
            BOOL fAbort = FALSE;

            // OK pressed:
            pRec->usFlags = shef.usFlags;
            pRec->usKey = shef.usKeyCode;
            WinQueryWindowText(hwndItem, sizeof(szCommand), szCommand);

            // getting the parameter value
            for (ul = 0;
                 ul < FLDRHOTKEYCOUNT;
                 ul++)
            {
                if (!strcmp(szCommand, G_szLBEntries[ul]))
                {
                    pRec->ulCommand = G_aDescriptions[ul].usPostCommand;
                    break;
                }
            }

            // checking for duplicate key code
            for (ul = 0;
                 ul < FLDRHOTKEYCOUNT;
                 ul++)
            {
                if (G_FolderHotkeys[ul].usCommand == 0)
                    break;
                if (    (G_FolderHotkeys[ul].usKeyCode == pRec->usKey)
                     && (G_FolderHotkeys[ul].usFlags == pRec->usFlags)
                     && (    (fInsert)
                          || (pRec->usKey != usOldKey)
                          || (pRec->usFlags != usOldFlags)
                              // ignore same hotkey if editing
                        )
                   )
                {
                    // found a duplicate hotkey
                    if (cmnErrorMsgBox(pnbp->hwndDlgPage,
                                       0,
                                       255,
                                       MB_YESNO|MB_DEFBUTTON2,
                                       TRUE) == MBID_NO)
                    {
                        // abort
                        fAbort = TRUE;
                    }
                    else
                    {
                        PHOTKEYRECORD pRec2 = NULL;
                        USHORT        usCmd = CMA_FIRST;
                        BOOL          fCont = TRUE;
                        USHORT        usCurrent = 0;

                        // remove existing entry
                        do
                        {
                            pRec2 = (PHOTKEYRECORD)WinSendMsg(hwndCnr,
                                                              CM_QUERYRECORD,
                                                              pRec2, // ignored on first call
                                                              MPFROM2SHORT(usCmd,     // CMA_FIRST or CMA_NEXT
                                                                           CMA_ITEMORDER));
                            usCmd = CMA_NEXT;

                            if ((pRec2) && ((ULONG)pRec2 != -1))
                            {
                                if (usCurrent == ul)
                                {
                                    WinSendMsg(hwndCnr,
                                               CM_REMOVERECORD,
                                               (MPARAM)&pRec2,
                                               MPFROM2SHORT(1,
                                                            CMA_FREE | CMA_INVALIDATE));
                                    fCont = FALSE;
                                }
                            }
                            else
                                fCont = FALSE;

                            usCurrent++;
                        } while (fCont);
                    }
                    break;
                }
            }

            if (!fAbort)
            {
                if (fInsert)
                    AddHotkeyRecord(hwndCnr,
                                    shef.usKeyCode,
                                    shef.usFlags,
                                    pRec->ulCommand,
                                    TRUE);          // invalidate
                else
                {
                    // pRec is already in container
                    AdjustHotkeyRecord(pRec);

                    // invalidate container to refresh view
                    WinSendMsg(hwndCnr,
                               CM_INVALIDATERECORD,
                               (MPARAM)&pRec,
                               MPFROM2SHORT(1,
                                            CMA_TEXTCHANGED));
                }

                fdrSaveFldrHotkeys(hwndCnr);
            }
        }

        winhDestroyWindow(&hwndDlg);
    }
}

static const CONTROLDEF
    HotkeysEnabled = LOADDEF_AUTOCHECKBOX(ID_XSDI_ACCELERATORS),
    HotkeysInMenus = LOADDEF_AUTOCHECKBOX(ID_XSDI_SHOWINMENUS),
    HotkeysGroup = LOADDEF_GROUP(ID_XSDI_HOTKEYS_GROUP, SZL_AUTOSIZE),
    HotkeysCnr = CONTROLDEF_CONTAINER(
                            ID_XSDI_CNR,
                            200,        // for now, will be resized
                            100);       // for now, will be resized

static const DLGHITEM G_dlgHotkeys[] =
    {
        START_TABLE,            // root table, required
            START_ROW(0),
                CONTROL_DEF(&HotkeysEnabled),
            START_ROW(0),
                CONTROL_DEF(&HotkeysInMenus),
            START_ROW(0),
                START_GROUP_TABLE(&HotkeysGroup),
                    START_ROW(0),
                        CONTROL_DEF(&HotkeysCnr),
                    START_ROW(0),
                        CONTROL_DEF(&G_AddButton),
                        CONTROL_DEF(&G_EditButton),
                        CONTROL_DEF(&G_RemoveButton),
                END_TABLE,
            START_ROW(0),       // notebook buttons (will be moved)
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };

MPARAM G_ampHotkeys[] =
    {
        MPFROM2SHORT(ID_XSDI_ACCELERATORS, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_SHOWINMENUS, XAC_MOVEY),
        MPFROM2SHORT(ID_XSDI_CNR, XAC_SIZEX | XAC_SIZEY),
        MPFROM2SHORT(ID_XSDI_HOTKEYS_GROUP, XAC_SIZEX | XAC_SIZEY),
    };

/*
 *@@ fdrHotkeysInitPage:
 *      notebook callback function (notebook.c) for the
 *      "Folder hotkeys" page in the "Workplace Shell" object.
 *      Sets the controls on the page according to the
 *      Global Settings.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.19 (2002-04-17) [umoeller]: finally skipping Warp 4 specific entries for Warp 3
 *@@changed V1.0.0 (2002-09-05) [lafaix]: reworked, uses the  dialog formatter too
 */

VOID fdrHotkeysInitPage(PNOTEBOOKPAGE pnbp,   // notebook info struct
                        ULONG flFlags)        // CBI_* flags (notebook.h)
{
    SHORT i;

    if (flFlags & CBI_INIT)
    {
        HWND        hwndCnr;
        HAB     hab = WinQueryAnchorBlock(pnbp->hwndDlgPage);
        HMODULE hmod = cmnQueryNLSModuleHandle(FALSE);
        XFIELDINFO  xfi[4];

        // first call: backup Global Settings for "Undo" button;
        // this memory will be freed automatically by the
        // common notebook window function (notebook.c) when
        // the notebook page is destroyed
        pnbp->pUser = cmnBackupSettings(G_HotkeysBackup,
                                         ARRAYITEMCOUNT(G_HotkeysBackup));
        // and also backup the Folder Hotkeys array in the
        // second pointer
        if (pnbp->pUser2 = malloc(FLDRHOTKEYSSIZE))
            memcpy(pnbp->pUser2,
                   fdrQueryFldrHotkeys(),
                   FLDRHOTKEYSSIZE);

        // insert the controls using the dialog formatter
        // V1.0.0 (2002-09-05) [lafaix]
        ntbFormatPage(pnbp->hwndDlgPage,
                      G_dlgHotkeys,
                      ARRAYITEMCOUNT(G_dlgHotkeys));

        for (i = 0; i < FLDRHOTKEYCOUNT; i++)
        {
            if (WinLoadString(hab,
                              hmod,
                              G_aDescriptions[i].ulStringID,
                              sizeof(G_szLBEntries[i]),
                              G_szLBEntries[i])
                     == 0)
            {
                cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "Unable to load strings.");
                break;
            }
        }

        hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                  ID_XSDI_CNR);

        i = 0;

        // set up cnr details view
        xfi[i].ulFieldOffset = FIELDOFFSET(HOTKEYRECORD, pcszKeyName);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSDI_HOTKEY_HOTKEY);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;

#ifdef ACTIONDROP
        xfi[i].ulFieldOffset = FIELDOFFSET(HOTKEYRECORD, pcszPlugin);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSDI_HOTKEY_ACTION);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;
#endif

        xfi[i].ulFieldOffset = FIELDOFFSET(HOTKEYRECORD, pcszParameters);
        xfi[i].pszColumnTitle = cmnGetString(ID_XSDI_HOTKEY_PARAM);
        xfi[i].ulDataType = CFA_STRING | CFA_HORZSEPARATOR;
        xfi[i++].ulOrientation = CFA_LEFT;

        cnrhSetFieldInfos(hwndCnr,
                          xfi,
                          i,             // array item count
                          FALSE,         // don't draw lines
                          1);            // return first column

        BEGIN_CNRINFO()
        {
            cnrhSetView(CV_DETAIL | CA_DETAILSVIEWTITLES);
        } END_CNRINFO(hwndCnr);

    }

    if (flFlags & CBI_SET)
    {
        HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                       ID_XSDI_CNR);

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_ACCELERATORS,
                              cmnQuerySetting(sfFolderHotkeysDefault));

        winhSetDlgItemChecked(pnbp->hwndDlgPage, ID_XSDI_SHOWINMENUS,
                              cmnQuerySetting(sfShowHotkeysInMenus));

        cnrhRemoveAll(hwndCnr);

        i = 0;
        while (G_FolderHotkeys[i].usCommand)
        {
            AddHotkeyRecord(hwndCnr,
                            G_FolderHotkeys[i].usKeyCode,
                            G_FolderHotkeys[i].usFlags,
                            G_FolderHotkeys[i].usCommand,
                            FALSE);
            cnrhInvalidateAll(hwndCnr);
            i++;
        }
    }

    if (flFlags & CBI_ENABLE)
    {
        WinEnableControl(pnbp->hwndDlgPage,
                         DID_EDIT,
                         G_FolderHotkeys[0].usCommand != 0);
        WinEnableControl(pnbp->hwndDlgPage,
                         DID_REMOVE,
                         G_FolderHotkeys[0].usCommand != 0);
    }
}

/*
 *@@ fdrHotkeysItemChanged:
 *      notebook callback function (notebook.c) for the
 *      "Folder hotkeys" page in the "Workplace Shell" object.
 *      Reacts to changes of any of the dialog controls.
 *
 *@@changed V0.9.0 [umoeller]: adjusted function prototype
 *@@changed V0.9.9 (2001-04-04) [umoeller]: added "Set" button
 *@@changed V1.0.0 (2002-09-08) [lafaix]: reworked to use a container
 */

MRESULT fdrHotkeysItemChanged(PNOTEBOOKPAGE pnbp,
                              ULONG ulItemID,
                              USHORT usNotifyCode,
                              ULONG ulExtra)      // for checkboxes: contains new state
{
    MRESULT mrc = (MRESULT)0;

    HWND hwndCnr = WinWindowFromID(pnbp->hwndDlgPage,
                                   ID_XSDI_CNR);

    switch (ulItemID)
    {
        case ID_XSDI_CNR:
            switch (usNotifyCode)
            {
                /*
                 * CN_CONTEXTMENU:
                 *
                 */

                case CN_CONTEXTMENU:
                {
                    HWND    hPopupMenu = NULLHANDLE;

                    // in the CREATENOTEBOOKPAGE structure
                    // so that the notebook.c function can
                    // remove source emphasis later automatically
                    pnbp->hwndSourceCnr = pnbp->hwndControl;
                    if (pnbp->preccSource = (PRECORDCORE)ulExtra)
                    {
                        // popup menu on container recc:
                        // disabling "add" item if hotkey array full
                        if (hPopupMenu = cmnLoadMenu(pnbp->hwndDlgPage, // V1.0.1 (2003-01-05) [umoeller]
                                                     cmnQueryNLSModuleHandle(FALSE),
                                                     ID_XFM_CNRITEM_SEL))
                            WinEnableMenuItem(hPopupMenu,
                                              ID_XFMI_CNRITEM_NEW,
                                              TRUE); // @todo
                    }
                    else
                    {
                        // popup menu on cnr whitespace
                        // disabling "add" item if hotkey array full
                        if (hPopupMenu = cmnLoadMenu(pnbp->hwndDlgPage, // V1.0.1 (2003-01-05) [umoeller]
                                                     cmnQueryNLSModuleHandle(FALSE),
                                                     ID_XFM_CNRITEM_NOSEL))
                            WinEnableMenuItem(hPopupMenu,
                                              ID_XFMI_CNRITEM_NEW,
                                              TRUE); // @todo
                    }

                    if (hPopupMenu)
                        cnrhShowContextMenu(pnbp->hwndControl,  // cnr
                                            (PRECORDCORE)pnbp->preccSource,
                                            hPopupMenu,
                                            pnbp->hwndDlgPage);    // owner
                }
                break;

                /*
                 * CN_ENTER:
                 *      ulExtra has the record that was clicked on.
                 */

                case CN_ENTER:
                    // this crashed if the user double-clicked on
                    // cnr whitespace since the record was then NULL
                    if (ulExtra)        // V1.0.0 (2002-08-28) [umoeller]
                        EditHotkeyRecord((PHOTKEYRECORD)ulExtra,
                                         pnbp,
                                         hwndCnr,
                                         FALSE); // do not create a new record
                break;
            }
        break;

        case ID_XSDI_ACCELERATORS:
            cmnSetSetting(sfFolderHotkeysDefault, ulExtra);
        break;

        case ID_XSDI_SHOWINMENUS:
            cmnSetSetting(sfShowHotkeysInMenus, ulExtra);
        break;

        case DID_ADD:
        case ID_XFMI_CNRITEM_NEW:
        {
            HOTKEYRECORD rec;
            memset(&rec, 0, sizeof(rec));
            EditHotkeyRecord(&rec,
                             pnbp,
                             hwndCnr,
                             TRUE); // create a new record if needed
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        }
        break;

        /*
         * ID_XFMI_CNRITEM_EDIT:
         *      show "Edit sticky window entry" dialog and edit
         *      the entry from that dialog
         *      (menu item command).
         */

        case ID_XFMI_CNRITEM_EDIT:
            EditHotkeyRecord((PHOTKEYRECORD)pnbp->preccSource,
                             pnbp,
                             hwndCnr,
                             FALSE); // do not create a new record
        break;

        /*
         * DID_EDIT
         *      show "Edit sticky window entry" dialog and edit
         *      the currently selected entry from that dialog
         *      (button command).
         */

        case DID_EDIT:
        {
            // get current selected record
            PHOTKEYRECORD pRec;
            if (    (pRec = (PHOTKEYRECORD)WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORDEMPHASIS,
                                                      (MPARAM)CMA_FIRST,
                                                      (MPARAM)CRA_SELECTED))
                 && ((LONG)pRec != -1L)
               )
                EditHotkeyRecord(pRec,
                                 pnbp,
                                 hwndCnr,
                                 FALSE); // do not create a new record
        }
        break;

        /*
         * ID_XFMI_CNRITEM_DELETE:
         *      remove sticky window record
         *      (menu item command).
         */

        case ID_XFMI_CNRITEM_DELETE:
            WinSendMsg(hwndCnr,
                       CM_REMOVERECORD,
                       &(pnbp->preccSource), // double pointer...
                       MPFROM2SHORT(1, CMA_FREE | CMA_INVALIDATE));
            fdrSaveFldrHotkeys(hwndCnr);
            pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
        break;

        case DID_REMOVE:
        {
            // get current selected record
            PHOTKEYRECORD pRec;
            if (    (pRec = (PHOTKEYRECORD)WinSendMsg(hwndCnr,
                                                      CM_QUERYRECORDEMPHASIS,
                                                      (MPARAM)CMA_FIRST,
                                                      (MPARAM)CRA_SELECTED))
                 && ((LONG)pRec != -1L)
               )
            {
                WinSendMsg(hwndCnr,
                           CM_REMOVERECORD,
                           &pRec, // double pointer...
                           MPFROM2SHORT(1, CMA_FREE | CMA_INVALIDATE));
                // @todo: really remove the definition
                fdrSaveFldrHotkeys(hwndCnr);
                pnbp->inbp.pfncbInitPage(pnbp, CBI_ENABLE);
            }
        }
        break;

        case DID_UNDO:
            // "Undo" button: restore the settings for this page
            cmnRestoreSettings(pnbp->pUser,
                               ARRAYITEMCOUNT(G_HotkeysBackup));

            // here, also restore the backed-up FolderHotkeys array
            // second pointer
            memcpy(fdrQueryFldrHotkeys(), pnbp->pUser2, FLDRHOTKEYSSIZE);

            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            fdrStoreFldrHotkeys();
        break;

        case DID_DEFAULT:
            // set the default settings for this settings page
            // (this is in common.c because it's also used at
            // Desktop startup)
            cmnSetDefaultSettings(pnbp->inbp.ulPageID);
            fdrLoadDefaultFldrHotkeys();
            // update the display by calling the INIT callback
            pnbp->inbp.pfncbInitPage(pnbp, CBI_SET | CBI_ENABLE);
            // cmnStoreGlobalSettings();
            fdrStoreFldrHotkeys();
        break;
    }

    return mrc;
}


