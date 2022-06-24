
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

#ifndef MENUS_HEADER_INCLUDED
    #define MENUS_HEADER_INCLUDED

    #ifndef INCL_WINMENUS
        #error "INCL_WINMENUS needs to be #define'd before including fdrmenus.h"
    #endif

    /* ******************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    #define CLIPBOARDKEY "%**C"        /* code in program object's parameter list for
                                          inserting clipboard data */

    // a bunch of decls only in the Warp 4 toolkit:

    #ifndef MENUS_SHORT
        #define MENUS_SHORT     0
        #define MENUS_LONG      1
        #define MENUS_DEFAULT   2

        #define MENUBAR_ON      1
        #define MENUBAR_OFF     0
        #define MENUBAR_DEFAULT 2
    #endif

    #ifndef WPMENUID_OPENPARENT
        #define WPMENUID_OPENPARENT         714
        #define WPMENUID_PASTE              715
        #define WPMENUID_CHANGETOICON       716
        #define WPMENUID_CHANGETOTREE       717
        #define WPMENUID_CHANGETODETAILS    718

        #define WPMENUID_VIEW               104

        // items in arrange menu
        #define WPMENUID_STANDARD           733
        #define WPMENUID_ARRANGETOP         734
        #define WPMENUID_ARRANGELEFT        735
        #define WPMENUID_ARRANGERIGHT       736
        #define WPMENUID_ARRANGEBOTTOM      737
        #define WPMENUID_PERIMETER          739
        #define WPMENUID_SELECTEDHORZ       740
        #define WPMENUID_SELECTEDVERT       741
        // end arrange

        #define WPMENUID_LOGOFF             738         // logoff network now

        #define WPMENUID_LOCKEDINPLACE      730
    #endif

    // non-standard declarations for folder hotkeys
    // to allow for specifying sort criteria
    #define ID_WPMI_SORTBYNAME             0x1770
    // #define ID_WPMI_SORTBYEXTENSION        0x9D87    was NPSWPS, removed V0.9.19 (2002-04-17) [umoeller]
    #define ID_WPMI_SORTBYTYPE             0x1771
    // the following match only if folder sort class is WPFileSystem
    #define ID_WPMI_SORTBYREALNAME         0x1777
    #define ID_WPMI_SORTBYSIZE             0x1778
    #define ID_WPMI_SORTBYWRITEDATE        0x1779
    #define ID_WPMI_SORTBYACCESSDATE       0x177B
    #define ID_WPMI_SORTBYCREATIONDATE     0x177D

    /*
        Warp 3 CTXT_* flags
        #define CTXT_CRANOTHER     0x0001
        #define CTXT_NEW           CTXT_CRANOTHER
        #define CTXT_OPEN          0x0002
        #define CTXT_WINDOW        0x0004
        #define CTXT_SWITCHTO      CTXT_WINDOW
        #define CTXT_CLOSE         0x0008
        #define CTXT_SETTINGS      0x0010
        #define CTXT_PRINT         0x0020
        #define CTXT_HELP          0x0040
        #define CTXT_DELETE        0x0080
        #define CTXT_COPY          0x0100
        #define CTXT_MOVE          0x0200
        #define CTXT_SHADOW        0x0400
        #define CTXT_LINK          CTXT_SHADOW
        #define CTXT_PROGRAM       0x0800
        #define CTXT_ICON        0x001000
        #define CTXT_TREE        0x002000
        #define CTXT_DETAILS     0x004000
        #define CTXT_FIND        0x008000
        #define CTXT_SELECT      0x010000
        #define CTXT_ARRANGE     0x020000
        #define CTXT_SORT        0x040000
        #define CTXT_SHUTDOWN    0x080000
        #define CTXT_LOCKUP      0x100000
        #define CTXT_PALETTE     0x200000
        #define CTXT_REFRESH     0x400000           // doesn't work on Warp 4
        #define CTXT_PICKUP      0x800000
        #define CTXT_PUTDOWN        0x1000000
        #define CTXT_PUTDOWN_CANCEL 0x2000000
    */

    // some more Warp 4 wpFilterPopupMenu flags
    #ifndef CTXT_PASTE
        #define CTXT_CHANGETOICON       0x04000000
        #define CTXT_CHANGETOTREE       0x08000000
        #define CTXT_CHANGETODETAILS    0x10000000
        #define CTXT_VIEW               0x20000000
        #define CTXT_PASTE              0x40000000
        // #define CTXT_UNDOARRANGE     0x80000000
                // must not be used because MENUITEMWITHID uses the highest
                // bit to tell CTXT_* flags from XWPCTXT_* flags
    #endif

    // additional XWP flags
    #define XWPCTXT_LOCKEDINPLACE       0x00000001
    #define XWPCTXT_LOGOFF              0x00000002      // logoff network now
    #define XWPCTXT_SYSTEMSETUP         0x00000004
    #define XWPCTXT_CHKDSK              0x00000008
    #define XWPCTXT_FORMAT              0x00000010
    #define XWPCTXT_COPYDSK             0x00000020
    #define XWPCTXT_LOCKDISK            0x00000040
    #define XWPCTXT_EJECTDISK           0x00000080
    #define XWPCTXT_UNLOCKDISK          0x00000100
    #define XWPCTXT_COPYFILENAME        0x00000200
    #define XWPCTXT_ATTRIBUTESMENU      0x00000400
    #define XWPCTXT_RESTARTWPS          0x00000800    // V1.0.0 (2002-10-08) [pr]
    #define XWPCTXT_SELALL              0x00001000
    #define XWPCTXT_DESELALL            0x00002000
    #define XWPCTXT_SELECTSOME          0x00004000
    #define XWPCTXT_LAYOUTITEMS         0x00008000
    #define XWPCTXT_FOLDERCONTENTS      0x00010000
    #define XWPCTXT_REFRESH_IN_VIEW     0x00020000
#ifndef __NOMOVEREFRESHNOW__
    #define XWPCTXT_REFRESH_IN_MAIN     0x00040000
#endif
    #define XWPCTXT_BATCHRENAME         0x00080000
            // V0.9.19 (2002-06-18) [umoeller]

    #define XWPCTXT_HIGHBIT             0x80000000

    // wpDisplayMenu flags V1.0.0 (2002-08-28) [umoeller]
    #ifndef MENU_FOLDERPULLDOWN
        #define MENU_FOLDERPULLDOWN       0x00000005
        #define MENU_VIEWPULLDOWN         0x00000006
        #define MENU_HELPPULLDOWN         0x00000007
        #define MENU_EDITPULLDOWN         0x00000008
        #define MENU_SELECTEDPULLDOWN     0x00000009
        #define MENU_FOLDERMENUBAR        0x0000000A
        #define MENU_NODISPLAY            0x40000000
    #endif

    // OPEN_TREE and OPEN_DETAILS are only defined in wpfolder.h,
    // which makes it a hassle for us, so add the definitions
    // here too V1.0.0 (2002-08-31) [umoeller]
    #ifndef OPEN_TREE
        #define OPEN_TREE               101
    #endif
    #ifndef OPEN_DETAILS
        #define OPEN_DETAILS            102
    #endif
    #ifndef OPEN_UNKNOWN
        #define OPEN_UNKNOWN      -1
    #endif
    #ifndef OPEN_DEFAULT
        #define OPEN_DEFAULT       0
    #endif
    #ifndef OPEN_CONTENTS
        #define OPEN_CONTENTS      1
    #endif
    #ifndef OPEN_SETTINGS
        #define OPEN_SETTINGS      2
    #endif
    #ifndef OPEN_HELP
        #define OPEN_HELP          3
    #endif
    #ifndef OPEN_RUNNING
        #define OPEN_RUNNING       4
    #endif
    #ifndef OPEN_PROMPTDLG
        #define OPEN_PROMPTDLG     5
    #endif
    #ifndef OPEN_PALETTE
        #define OPEN_PALETTE       121
    #endif
    #ifndef CLOSED_ICON
        #define CLOSED_ICON        122
    #endif
    #ifndef OPEN_USER
        #define OPEN_USER          0x6500
    #endif

    /* ******************************************************************
     *
     *   Global variables
     *
     ********************************************************************/

    // original wnd proc for folder content menus,
    // which we must subclass (var sits in folder.c)
    extern PFNWP G_pfnwpFolderContentMenuOriginal;

    /* ******************************************************************
     *
     *   Global WPS menu settings
     *
     ********************************************************************/

    BOOL mnuQueryDefaultMenuBarVisibility(VOID);

    BOOL mnuSetDefaultMenuBarVisibility(BOOL fVisible);

    BOOL mnuQueryShortMenuStyle(VOID);

    BOOL mnuSetShortMenuStyle(BOOL fShort);

    /* ******************************************************************
     *
     *   Menu manipulation
     *
     ********************************************************************/

    #ifdef COMMON_HEADER_INCLUDED
        XWPSETTING mnuQueryMenuWPSSetting(WPObject *somSelf);

        XWPSETTING mnuQueryMenuXWPSetting(WPObject *somSelf);
    #endif

    VOID mnuRemoveMenuItems(WPObject *somSelf,
                            HWND hwndMenu,
                            const ULONG *aSuppressFlags,
                            ULONG cSuppressFlags);

    VOID mnuCheckDefaultSortItem(HWND hwndSortMenu,
                                 ULONG ulDefaultSort);

    BOOL mnuInsertFldrViewItems(WPFolder *somSelf,
                                HWND hwndViewSubmenu,
                                PCNRINFO pCnrInfo,
                                HWND hwndFrame,
                                ULONG ulView);

    VOID mnuInvalidateConfigCache(VOID);

    BOOL mnuModifyFolderMenu(WPFolder *somSelf,
                             HWND hwndMenu,
                             HWND hwndCnr,
                             ULONG ulMenuType,
                             ULONG ulView);

    BOOL mnuModifyDataFilePopupMenu(WPObject *somSelf,
                                    HWND hwndMenu,
                                    HWND hwndCnr);

    /* ******************************************************************
     *
     *   Notebook callbacks (notebook.c) for XFldWPS "Menu" pages
     *
     ********************************************************************/

    ULONG mnuAddWPSMenuPages(WPObject *somSelf,
                             HWND hwndDlg);

#endif
