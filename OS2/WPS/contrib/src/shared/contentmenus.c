
/*
 *@@sourcefile contentmenus.c:
 *      this file contains shared functions for displaying
 *      folder contents in menus.
 *
 *      These "folder content" menus are subclassed PM menu
 *      controls which behave similar to the WarpCenter, i.e.
 *      they will auto-populate and display folder contents
 *      and open folders and objects in their default views
 *      when selected.
 *
 *      As opposed to the WarpCenter though, XWorkplace uses
 *      the PM menu control. This requires a lot of trickery
 *      though...
 *
 *      Folder content menus are neither exactly trivial to
 *      implement nor trivial to use because they require
 *      menu owner draw (for displaying icons in a menu)
 *      and subclassing popup menu controls (for intercepting
 *      mouse button 2). Even worse, while these menus are
 *      open, we need to maintain a list of objects that
 *      were inserted together with the respective menu item
 *      ID that was used. This is done with the help of a
 *      global linked list of VARMENULISTITEM structs.
 *
 *      You can only use folder content menus if you have
 *      access to messages sent or posted to the menus's
 *      owner. In other words, the functions in here need
 *      the cooperation of the menu owner to work.
 *
 *      For folder popup menus, this is the case because
 *      XFolder subclasses the folder window frame, which
 *      owns the menu. For XCenter object button widgets,
 *      the same is true, of course... they have their
 *      own widget window procedure.
 *
 *      To use folder content menus, perform the following
 *      steps:
 *
 *      1)  Before displaying a popup menu which contains
 *          folder content submenus, call cmnuInitItemCache.
 *
 *      2)  For each folder content submenu to insert, call
 *          cmnuPrepareContentSubmenu. This inserts a submenu
 *          with only a dull stub menu item at this point.
 *
 *      3)  Intercept WM_INITMENU in the menu owner's window
 *          procedure. This msg gets sent to a menu's owner
 *          window when a (sub)menu is about to display.
 *          If you get WM_INITMENU for one of the folder
 *          content submenus, call cmnuFillContentSubmenu,
 *          which will populate the corresponding folder
 *          and insert its contents into the menu.
 *
 *      4)  Call cmnuMeasureItem for each WM_MEASUREITEM
 *          which comes in.
 *
 *      5)  Call cmnuDrawItem for each WM_DRAWITEM which
 *          comes in.
 *
 *      6)  The window's owner will receive a WM_COMMAND
 *          if one of the objects is selected by the user.
 *          To get a VARMENULISTITEM structure which describes
 *          the object in the menu, call cmnuGetVarItem with
 *          the command's menu ID. You can then open the
 *          object, for example.
 *
 *      Function prefix for this file:
 *      --  cmnu*
 *
 *      This file is new with V0.9.7 (2000-11-29) [umoeller].
 *      The code in here used to be in src\filesys\fdrmenus.c
 *      and has been moved here so that it can be used in
 *      other code parts as well.
 *
 *@@header "shared\contentmenus.h"
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR        // SC_CLOSE etc.
#define INCL_WININPUT
#define INCL_WINDIALOGS
#define INCL_WINBUTTONS
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSTDCNR
#define INCL_WINMLE
#define INCL_WINCOUNTRY
#define INCL_WINCLIPBOARD
#define INCL_WINSYS
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h

#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS // [lafaix]
#define INCL_GPIPRIMITIVES
#define INCL_DEV
#include <os2.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\stringh.h"            // string helper routines
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
// #include "xfobj.ih"
#include "xfdisk.ih"                    // XFldDisk

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\contentmenus.h"        // shared menu logic
#include "shared\kernel.h"              // XWorkplace Kernel

#include "filesys\folder.h"             // XFolder implementation
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers
#pragma hdrstop                         // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Definitions
 *
 ********************************************************************/

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

// counts for providing unique menu id's (exported)
extern ULONG            G_ulVarItemCount = 0;    // number of inserted menu items
extern SHORT            G_sNextMenuId = 0;      // next menu item ID to use

// llContentMenuItems contains ONLY folder content menus
static LINKLIST         G_llContentMenuItems; // changed V0.9.0

// linked lists / counts for variable context menu items
// llVarMenuItems contains ALL variable items ever inserted
// (i.e. config folder items AND folder content items)
static LINKLIST         G_llVarMenuItems;     // changed V0.9.0

// icon for drawing the little triangle in
// folder content menus (subfolders)
// static HPOINTER         G_hMenuArrowIcon = NULLHANDLE;

// original wnd proc for folder content menus,
// which we must subclass
extern PFNWP            G_pfnwpFolderContentMenuOriginal = NULL;

static POINTL           G_ptlPositionBelow;         // in screen coords
static BOOL             G_fPositionBelow = FALSE;

// #define CX_ARROW 21

// global data for owner draw; we cache this for speed
static BOOL             G_bInitNeeded;          // if TRUE, data is refreshed

static ULONG            G_ulMiniIconSize = 0;
static RECTL            G_rtlMenuItem;
static LONG             G_lHiliteBackground,
                        G_lBackground,
                        G_lHiliteText,
                        G_lBorderLight,     // [lafaix]
                        G_lBorderDark,      // [lafaix]
                        G_lText;
static SIZEF            G_szfCharBox;       // [lafaix]
static LONG             G_lMaxDescender;    // [lafaix]

/* ******************************************************************
 *
 *   Functions
 *
 ********************************************************************/

/*
 *@@ fnwpSubclFolderContentMenu:
 *      this is the subclassed wnd proc for folder content menus.
 *
 *      We need to intercept mouse button 2 msgs to open a folder
 *      (WarpCenter behavior).
 *
 *      In addition, we hack WM_ADJUSTWINDOWPOS to be able to
 *      position popup menus anywhere we want, which is kinda
 *      difficult with regular menus.
 *
 *@@changed V0.9.0 [umoeller]: moved this func here from xfldr.c
 *@@changed V0.9.7 (2001-01-19) [umoeller]: reworked WM_ADJUSTWINDOWPOS for screen constrain
 *@@changed V0.9.7 (2001-01-19) [umoeller]: added support for cmnuSetPositionBelow
 *@@changed V0.9.19 (2002-05-02) [umoeller]: fixed bad object buttons with XCenter border spacing == 0
 */

MRESULT EXPENTRY fnwpSubclFolderContentMenu(HWND hwndMenu, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT     mrc = 0;
    PFNWP       pfnwpOrig = 0;

    if (G_pfnwpFolderContentMenuOriginal)
        pfnwpOrig = G_pfnwpFolderContentMenuOriginal;
    else
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
                       "G_pfnwpFolderContentMenuOriginal is NULL");
        pfnwpOrig = WinDefWindowProc;
    }

    TRY_LOUD(excpt1)
    {
        USHORT  sSelected;
        POINTL  ptlMouse;
        RECTL   rtlItem;

        switch(msg)
        {
            case WM_ADJUSTWINDOWPOS:
            {
                // here comes the fun stuff...

                // This comes in several times and appears to represent
                // a terrible mess in the PM menu control.

                // 1) First call: x = y = cx = cy = 0; at this point, we
                //    still only have the "dummy" item.

                // 2) Then the menu sends WM_INITMENU. Our caller
                //    (e.g. OwgtInitMenu) calls cmnuInsertObjectsIntoMenu,
                //    which inserts all the variable items.
                //    So we get a MM_INSERTITEM for each.

                // 3) Since cmnuInsertObjectsIntoMenu sends an
                //    MM_QUERYITEMRECT to the menu, the menu will
                //    attempt to calculate its size. We get a WM_ADJUSTWINDOWPOS
                //    again, which causes a flurry of WM_MEASUREITEM messages
                //    on fnwpObjButtonWidget. On exit of that WM_ADJUSTWINDOWPOS,
                //    we have correct cx and cy sizes, but x and y are still 0.

                // 4) cmnuInsertObjectsIntoMenu sends MM_QUERYITEMCOUNT.

                // 5) cmnuInsertObjectsIntoMenu returns, OwgtInitMenu returns.

                // 6) For some reason, we then get another WM_ADJUSTWINDOWPOS
                //    with all fields == 0, which goes thru the WM_MEASUREITEM
                //    mess again.

                // 7) Then we get the first WM_ADJUSTWINDOWPOS with a valid
                //    x and y. APPARENTLY, this is from WinPopupMenu... but
                //    who knows. After that, we get the first WM_PAINT, so
                //    apparently the menu is now visible.

                // So what we do:
                // Every time WM_ADJUSTWINDOWPOS comes in, we check for whether
                // x and y are valid. If so, this is the right call... we hack it.
                PSWP pswp;
                if (pswp = (PSWP)mp1)
                {
                    PMPF_MENUS(("WM_ADJUSTWINDOWPOS %d, %d, %d, %d",
                                pswp->x, pswp->y, pswp->cx, pswp->cy));

                    // is this the message that really sets the window?
                    if (    /* pswp->x
                            X can be 0 if the user has an XCenter object button in the
                            very top left of the screen and set border spacing to 0
                            V0.9.19 (2002-05-02) [umoeller]
                         && */ pswp->y
                         && pswp->cx
                         && pswp->cy
                       )
                    {
                        // yes:
                        if (G_fPositionBelow)
                        {
                            // position menu below button:
                            pswp->x = G_ptlPositionBelow.x;
                            pswp->y = G_ptlPositionBelow.y - pswp->cy;
                            // only do this once
                            G_fPositionBelow = FALSE;
                        }

                        // always constrain the damn thing to the screen...
                        // PM makes large popup menus show up off the screen
                        // which is not terribly helpful.
                        if (pswp->x + pswp->cx > G_cxScreen)
                            // off screen:
                            if (pswp->cx < G_cxScreen)
                                pswp->x = G_cxScreen - pswp->cx;

                        if (pswp->y + pswp->cy > G_cyScreen)
                            // off screen:
                            if (pswp->cy < G_cyScreen)
                                pswp->y = G_cyScreen - pswp->cy;
                    }
                }
                mrc = pfnwpOrig(hwndMenu, msg, mp1, mp2);
            }
            break;

            #ifdef __DEBUG__
                case MM_SELECTITEM:
                    PMPF_MENUS(("MM_SELECTITEM: mp1 = %lX/%lX, mp2 = %lX",
                        SHORT1FROMMP(mp1),
                        SHORT2FROMMP(mp1),
                        mp2 ));
                    mrc = pfnwpOrig(hwndMenu, msg, mp1, mp2);
                break;
            #endif

            case WM_BUTTON2DOWN:
                PMPF_MENUS(("WM_BUTTON2DOWN"));

                ptlMouse.x = SHORT1FROMMP(mp1);
                ptlMouse.y = SHORT2FROMMP(mp1);
                WinSendMsg(hwndMenu, MM_SELECTITEM,
                           MPFROM2SHORT(MIT_NONE, FALSE),
                           MPFROM2SHORT(0, FALSE));
                sSelected = winhQueryItemUnderMouse(hwndMenu, &ptlMouse, &rtlItem);
                WinSendMsg(hwndMenu, MM_SETITEMATTR,
                           MPFROM2SHORT(sSelected,
                                        FALSE),
                           MPFROM2SHORT(MIA_HILITED,
                                        MIA_HILITED)
                    );
            break;

            /* case WM_BUTTON1DOWN:
                // let this be handled by the default proc
                PMPF_MENUS(("WM_BUTTON1DOWN"));

                // G_fFldrContentMenuButtonDown = TRUE;
                mrc = pfnwpOrig(hwndMenu, msg, mp1, mp2);
            break; */

            case WM_BUTTON1DBLCLK:
            case WM_BUTTON2UP:
            {
                // upon receiving these, we will open the object directly;
                // we need to cheat a little bit because sending
                // MM_SELECTITEM would open the submenu
                PMPF_MENUS(("WM_BUTTON2UP"));

                // G_fFldrContentMenuButtonDown = TRUE;
                ptlMouse.x = SHORT1FROMMP(mp1);
                ptlMouse.y = SHORT2FROMMP(mp1);
                WinSendMsg(hwndMenu, MM_SELECTITEM,
                           MPFROM2SHORT(MIT_NONE, FALSE),
                           MPFROM2SHORT(0, FALSE));
                sSelected = winhQueryItemUnderMouse(hwndMenu, &ptlMouse, &rtlItem);

#ifndef __NOXSYSTEMSOUNDS__
                cmnPlaySystemSound(MMSOUND_XFLD_CTXTSELECT);
#endif

                WinPostMsg(WinQueryWindow(hwndMenu, QW_OWNER),
                           WM_COMMAND,
                           (MPARAM)sSelected,
                           MPFROM2SHORT(CMDSRC_MENU, FALSE));
            }
            break;

            default:
                mrc = pfnwpOrig(hwndMenu, msg, mp1, mp2);
            break;
        } // end switch
    }
    CATCH(excpt1)
    {
        // exception occurred:
        mrc = 0;
    } END_CATCH();

    return mrc;
}

/* ******************************************************************
 *
 *   Functions
 *
 ********************************************************************/

/*
 *@@ cmnuInitItemCache:
 *      this initializes all global variables which hold
 *      object information while a folder context menu
 *      is open.
 *
 *      This must be called each time before a menu is
 *      displayed which contains one or several folder
 *      content submenus (inserted with cmnuPrepareContentSubmenu),
 *      or otherwise we'll quickly run out of menu item IDs.
 *
 *      For folder context menus, this gets called from
 *      mnuModifyFolderMenu.
 *
 *@@added V0.9.7 (2000-11-29) [umoeller]
 */

VOID cmnuInitItemCache(VOID)
{
    G_sNextMenuId = *G_pulVarMenuOfs + ID_XFMI_OFS_VARIABLE;
    G_ulVarItemCount = 0;         // reset the number of variable items to 0

    if (G_llContentMenuItems.ulMagic != LINKLISTMAGIC)
    {
        // first call: initialize lists
        lstInit(&G_llContentMenuItems, TRUE);
        lstInit(&G_llVarMenuItems, TRUE);
    }
    else
    {
        // subsequent calls: clear lists
        // (this might take a while)
        HPOINTER    hptrOld = winhSetWaitPointer();
        lstClear(&G_llContentMenuItems);
        lstClear(&G_llVarMenuItems);
        WinSetPointer(HWND_DESKTOP, hptrOld);
    }
}

/*
 *@@ cmnuAppendMi2List:
 *      this stores a variable XFolder menu item in the
 *      respective global linked list (llVarMenuItems)
 *      and increases sNextMenuId for the next item.
 *      Returns FALSE if too many items have already been
 *      used and menus should be closed.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.0 [umoeller]: fixed bug with filenames > 100 chars
 *@@changed V0.9.0 [umoeller]: lowered menu item limit
 *@@changed V0.9.19 (2002-06-18) [umoeller]: optimized memory management
 */

BOOL cmnuAppendMi2List(WPObject *pObject,
                       ULONG ulObjType)     // in: OC_* flag
{
    PVARMENULISTITEM    pNewItem;

    CHAR                szTitle[CCHMAXPATH];
    PCSZ                pcszTitle;
    ULONG               ulTitleLen = 0,
                        cbPtlArray;

    // allocate VARMENULISTITEM dynamically
    // based on the object title's length
    // V0.9.19 (2002-06-18) [umoeller]
    if (pcszTitle = _wpQueryTitle(pObject))
    {
        // copy to temp buffer first so we can know
        // the size of the beautified string
        ulTitleLen = strhBeautifyTitle2(szTitle,
                                        pcszTitle);
    }

    cbPtlArray = sizeof(POINTL) * (ulTitleLen + 1);

    if (pNewItem = (PVARMENULISTITEM)malloc(   sizeof(VARMENULISTITEM)
                                             + cbPtlArray
                                                        // for pptlPositions
                                             + ulTitleLen + 1
                                                        // for pszTitle
                                           ))
    {
        pNewItem->pObject = pObject;
        pNewItem->ulObjType = ulObjType;

        if (!ulTitleLen)
            pNewItem->pszTitle = NULL;
        else
        {
            // point pszTitle to after POINTL array
            pNewItem->pszTitle =   (PBYTE)pNewItem->aptlPositions
                                 + cbPtlArray;
            memcpy(pNewItem->pszTitle,
                   szTitle,             // beautified copy
                   ulTitleLen + 1);     // null-terminate
        }

        pNewItem->ulTitleLen = ulTitleLen;

        lstAppendItem(&G_llVarMenuItems, pNewItem);

        if (G_sNextMenuId < 0x7800)       // lowered V0.9.0
        {
            G_sNextMenuId++;
            G_ulVarItemCount++;
            return TRUE;
        }

        krnPostThread1ObjectMsg(T1M_LIMITREACHED, MPNULL, MPNULL);
    }

    return FALSE;
}

/*
 *@@ cmnuAppendFldr2ContentList:
 *      this stores a folder / menu item id in a global linked
 *      list so that the subclassed window procedure can find
 *      it later for populating the folder and displaying its
 *      contents in the menu.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 */

VOID cmnuAppendFldr2ContentList(WPFolder *pFolder,
                                SHORT sMenuId)
{
    PCONTENTMENULISTITEM pNewItem = (PCONTENTMENULISTITEM)malloc(sizeof(CONTENTMENULISTITEM));
    pNewItem->pFolder = pFolder;
    pNewItem->sMenuId = sMenuId;
    lstAppendItem(&G_llContentMenuItems, pNewItem);
}

/*
 *@@ cmnuInsertOneObjectMenuItem:
 *      this sub-subroutine is called by mnuFillContentSubmenu
 *      whenever a single menu item is to be inserted
 *      (all objects except folders); returns the menu
 *      item id.
 */

ULONG cmnuInsertOneObjectMenuItem(HWND hAddToMenu,   // hwnd of menu to add to
                                  USHORT iPosition,
                                  PCSZ pcszNewItemString,          // title of new item
                                  USHORT afStyle,
                                  WPObject *pObject,                  // pointer to corresponding object
                                  ULONG ulObjType)
{
    ULONG               rc = G_sNextMenuId;

    winhInsertMenuItem(hAddToMenu,
                       iPosition,
                       G_sNextMenuId,
                       pcszNewItemString,
                       afStyle,
                       0);

    if (cmnuAppendMi2List(pObject, ulObjType))
        // give signal for calling function that we found something
        return rc;

    return 0; // report error
}

/*
 *@@ cmnuSetPositionBelow:
 *      sets the position _below_ which the next menu
 *      filled with cmnuInsertObjectsIntoMenu should
 *      appear.
 *
 *      This has been added for the XCenter object button
 *      widgets. Unfortunately it is impossible to position
 *      a menu _below_ something with WinPopupMenu, which
 *      will always put the menu to the top right of the
 *      given coordinates.
 *
 *      Since we're subclassing the menus anyway... we
 *      can use the coordinates given here to reposition
 *      the menu anywhere we want.
 *
 *@@added V0.9.7 (2001-01-19) [umoeller]
 */

VOID cmnuSetPositionBelow(PPOINTL pptlBelow)
{
    if (pptlBelow)
    {
        G_ptlPositionBelow.x = pptlBelow->x;
        G_ptlPositionBelow.y = pptlBelow->y;
        G_fPositionBelow = TRUE;
    }
}

/*
 *@@ cmnuPrepareContentSubmenu:
 *      prepares a folder content submenu by inserting a
 *      submenu named pszTitle into hwndMenu.
 *
 *      This submenu then only contains the "empty" menu item,
 *      however, this menu can be filled with objects if the
 *      user opens it.
 *
 *      To support this, you need to intercept the WM_INITMENU
 *      message and then call cmnuFillContentSubmenu.
 *
 *      This way, we can fill the folder content submenus only when
 *      this is needed, because we can impossibly populate all
 *      folder content menus when the context menu is initially
 *      opened.
 *
 *      This function returns the new submenu's menu item ID.
 */

SHORT cmnuPrepareContentSubmenu(WPFolder *somSelf, // in: folder whose content is to be displayed
                                HWND hwndMenu,    // in: menu to insert submenu into
                                PCSZ pcszTitle,     // in: submenu item title
                                USHORT iPosition, // in: position to insert at (or MIT_END)
                                BOOL fOwnerDraw)  // in: owner-draw style flag for submenu (ie. display icons)
{
    HWND    hwndNewMenu;
    SHORT   sId = G_sNextMenuId;
    if (hwndNewMenu = winhInsertSubmenu(hwndMenu,
                                        iPosition,
                                        sId,
                                        pcszTitle,
                                        (fOwnerDraw
                                            ? MIS_OWNERDRAW
                                            : 0),
                                        *G_pulVarMenuOfs + ID_XFMI_OFS_DUMMY,
                                        cmnGetString(ID_XSSI_FLDREMPTY),
                                            // (cmnQueryNLSStrings())->pszFldrEmpty,
                                        MIS_TEXT,
                                        MIA_DISABLED))
    {
        cmnuAppendFldr2ContentList(somSelf, sId);
        cmnuAppendMi2List(somSelf, OC_CONTENTFOLDER);
        return(sId);
    }

    return 0;
}

/*
 *@@ MENULISTITEM:
 *
 */

typedef struct _MENULISTITEM
{
    WPObject    *pObject;
    CHAR        szItemString[256];
} MENULISTITEM, *PMENULISTITEM;

/*
 *@@ fncbSortContentMenuItems:
 *      callback sort func for the sort functions in linklist.c.
 *
 *      This sorts the folder content menu items alphabetically.
 */

SHORT XWPENTRY fncbSortContentMenuItems(PVOID pItem1, PVOID pItem2, PVOID hab)
{
    switch (WinCompareStrings((HAB)hab, 0, 0,
                              ((PMENULISTITEM)pItem1)->szItemString,
                              ((PMENULISTITEM)pItem2)->szItemString, 0))
    {
        case WCS_LT:    return -1;
        case WCS_GT:    return 1;
    }

    return 0;
}

/*
 *@@ cmnuInsertObjectsIntoMenu:
 *      this does the real work for cmnuFillContentSubmenu:
 *      collecting the folder's contents, sorting that into
 *      folders and objects and reformatting the submenu in
 *      columns.
 *
 *      You can call this function directly to have folder
 *      contents inserted into a certain menu, if you don't
 *      want to use a submenu. You still need to process
 *      messages in the menu's owner, as described at the
 *      top of contentmenus.c.
 *
 *      This calls another cmnuPrepareContentSubmenu for
 *      each folder or cmnuInsertOneObjectMenuItem for each
 *      non-folder item to be inserted.
 *
 *      The menu is separated into two sections (for
 *      folders and other objects), each of which
 *      is sorted alphabetically.
 *
 *      This has been extracted from mnuFillContentSubmenu
 *      for clarity and also because the folder mutex is now
 *      requested while the contents are retrieved.
 *
 *      Preconditions:
 *
 *      --  We assume that pFolder is fully populated. This does
 *          not populate pFolder any more.
 *
 *      --  Since this function can take a while and the PM msg
 *          queue is blocked while inserting objects, you should
 *          display a "Wait" pointer while calling this.
 *
 *@@added V0.9.1 (2000-02-01) [umoeller]
 *@@changed V0.9.3 (2000-04-28) [umoeller]: now pre-resolving wpQueryContent for speed
 *@@changed V0.9.7 (2001-01-21) [lafaix]: using MIS_BREAKSEPARATOR instead of MIS_BREAK
 *@@changed V0.9.16 (2002-01-05) [umoeller]: now applying folder "include" criteria
 *@@changed V0.9.19 (2002-04-14) [umoeller]: finally sorting folder templates under non-folders
 *@@changed V0.9.20 (2002-07-09) [pr]: made "... objects dropped" entry clickable to open folder
 */

VOID cmnuInsertObjectsIntoMenu(WPFolder *pFolder,   // in: folder whose contents
                                                    // are to be inserted
                               HWND hwndMenu)       // in: submenu to append items to
{
    HAB             habDesktop = WinQueryAnchorBlock(HWND_DESKTOP);
    BOOL            fFolderLocked = FALSE;
    PLISTNODE       pNode = NULL;

    // We will first create two lists in memory
    // for all folders and non-folders; we will
    // then sort these lists alphabetically and
    // finally insert their content into the menu
    PLINKLIST       pllFolders = lstCreate(TRUE),   // items are freeable
                    pllNonFolders = lstCreate(TRUE);

    MENUITEM        mi;

    SHORT           sItemId;
    SHORT           sItemSize, sItemsPerColumn, sItemCount,
                    sColumns,
                    s;

    RECTL           rtlItem;

    ULONG           ulObjectsLeftOut = 0;
                // counts items which were left out because
                // too many are in the folder to be displayed

    TRY_LOUD(excpt1)
    {
        WPObject        *pObject, *pObject2;

        // subclass menu window to allow MB2 clicks
        G_pfnwpFolderContentMenuOriginal
            = WinSubclassWindow(hwndMenu,
                                fnwpSubclFolderContentMenu);

        // remove "empty" item (if it exists)
        winhDeleteMenuItem(hwndMenu,
                           *G_pulVarMenuOfs + ID_XFMI_OFS_DUMMY);

        // start collecting stuff; lock the folder contents,
        // do this in a protected block (exception handler,
        // must-complete section)
        if (fFolderLocked = !_wpRequestFolderMutexSem(pFolder, 5000))
        {
            ULONG   ulTotalObjectsAdded = 0;
            // now collect all objects in folder
            // V0.9.20 (2002-07-31) [umoeller]: now using get_pobjNext SOM attribute
            for (pObject = _wpQueryContent(pFolder, NULL, QC_FIRST);
                 pObject;
                 pObject = *__get_pobjNext(pObject))
            {
                // apply folder's "include" criteria
                // V0.9.16 (2002-01-05) [umoeller]
                if (!fdrIsObjectFiltered(pFolder,
                                         pObject))
                {
                    // object not filtered:
                    // dereference shadows, if necessary
                    if (pObject2 = _xwpResolveIfLink(pObject))
                    {
                        BOOL    fIsFolder;

                        if (fIsFolder = (    (_somIsA(pObject2, _WPFolder))
                                          || (_somIsA(pObject2, _WPDisk))
                                        ))
                            // treat folder templates as non-folders
                            // V0.9.19 (2002-04-14) [umoeller]
                            fIsFolder = (!(_wpQueryStyle(pObject2) & OBJSTYLE_TEMPLATE));

                        // append this always if it's a folder
                        // or, if it's no folder, we haven't
                        // exceeded 100 objects yet
                        if (    (fIsFolder)
                             || (ulTotalObjectsAdded < 100)
                           )
                        {
                            PMENULISTITEM pmliNew = malloc(sizeof(MENULISTITEM));
                            pmliNew->pObject = pObject2;
                            strlcpy(pmliNew->szItemString,
                                    _wpQueryTitle(pObject2),
                                    sizeof(pmliNew->szItemString));

                            // remove line breaks
                            strhBeautifyTitle(pmliNew->szItemString);

                            if (fIsFolder)
                                // folder/disk: append to folder list
                                lstAppendItem(pllFolders,
                                              pmliNew);
                            else
                                // other (non-folder):
                                // append to objects list
                                lstAppendItem(pllNonFolders,
                                              pmliNew);

                            // raise object count (to avoid too many)
                            ulTotalObjectsAdded++;
                        }
                        else
                            // item left out: count those separately
                            // to for informational menu item later
                            ulObjectsLeftOut++;
                    }
                }
            } // end for pObject
        } // end if (fFolderLocked)
    }
    CATCH(excpt1) { } END_CATCH();

    if (fFolderLocked)
        _wpReleaseFolderMutexSem(pFolder);

    // now sort the lists alphabetically
    lstQuickSort(pllFolders,
                 fncbSortContentMenuItems,
                 (PVOID)habDesktop);
    lstQuickSort(pllNonFolders,
                 fncbSortContentMenuItems,
                 (PVOID)habDesktop);

    // insert folders into menu
    pNode = lstQueryFirstNode(pllFolders);
    while (pNode)
    {
        PMENULISTITEM pmli = pNode->pItemData;
        // folder items
        sItemId = cmnuPrepareContentSubmenu(pmli->pObject,
                                            hwndMenu,
                                            pmli->szItemString,
                                            MIT_END,
#ifndef __NOFOLDERCONTENTS__
                                            cmnQuerySetting(sfFolderContentShowIcons));
#else
                                            TRUE);
#endif
                                                     // OwnerDraw flag

        // next folder
        pNode = pNode->pNext;
    }

    // if we have both objects and folders:
    // insert separator between them
    if (    (pllFolders->ulCount)
         && (pllNonFolders->ulCount)
       )
       cmnInsertSeparator(hwndMenu, MIT_END);

    // insert non-folder objects into menu
    pNode = lstQueryFirstNode(pllNonFolders);
    while (pNode)
    {
        PMENULISTITEM pmli = pNode->pItemData;
        sItemId = cmnuInsertOneObjectMenuItem(hwndMenu,
                                              MIT_END,
                                              pmli->szItemString,
#ifndef __NOFOLDERCONTENTS__
                                              (cmnQuerySetting(sfFolderContentShowIcons))
                                                 ? MIS_OWNERDRAW
                                                 : MIS_TEXT,
#else
                                              MIS_OWNERDRAW,
#endif
                                              pmli->pObject,
                                              OC_CONTENT);
        if (sItemId)
            pNode = pNode->pNext;
        else
            pNode = NULL;

        /* {
            SWP swpMenu;
            WinQueryWindowPos(hwndMenu, &swpMenu);
            _Pmpf(("current window pos: %d, %d, %d, %d",
                    swpMenu.x, swpMenu.y, swpMenu.cx, swpMenu.cy));
        } */
    }

    // did we leave out any objects?
    if (ulObjectsLeftOut)
    {
        // yes: add a message saying so
        CHAR    szMsgItem[300];
        // PNLSSTRINGS pNLSStrings = cmnQueryNLSStrings();
        cmnInsertSeparator(hwndMenu, MIT_END);

        sprintf(szMsgItem,
                cmnGetString(ID_XSSI_DROPPED1),  // "... %d objects dropped,"
                ulObjectsLeftOut);

        winhInsertMenuItem(hwndMenu,
                           MIT_END,
                           *G_pulVarMenuOfs + ID_XFMI_OFS_DUMMY,
                           szMsgItem,
                           MIS_TEXT,
                           MIA_DISABLED);
        /*
        winhInsertMenuItem(hwndMenu,
                           MIT_END,
                           *G_pulVarMenuOfs + ID_XFMI_OFS_DUMMY,
                           cmnGetString(ID_XSSI_DROPPED2),  // "open folder to see them", // pszDropped2
                           MIS_TEXT,
                           MIA_DISABLED);
        */

        // V0.9.20 (2002-07-09) [pr]
        cmnuInsertOneObjectMenuItem(hwndMenu,
                                    MIT_END,
                                    cmnGetString(ID_XSSI_DROPPED2),  // "open folder to see them", // pszDropped2
                                    MIS_TEXT,
                                    pFolder,
                                    OC_CONTENT);
    }

    // calculate maximum number of items per column by looking
    // at the screen and item sizes
    WinSendMsg(hwndMenu,
               MM_QUERYITEMRECT,
               MPFROM2SHORT(sItemId, FALSE),
               (MPARAM)&rtlItem);
    sItemSize = (rtlItem.yTop - rtlItem.yBottom);
    if (sItemSize == 0)
        sItemSize = 20;
    sItemsPerColumn = (USHORT)( (  WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN)
                                   - 30
                                )
                                / sItemSize );
    if (sItemsPerColumn == 0)
        sItemsPerColumn = 20;

    // sItemsPerColumn now contains the items which will
    // beautifully fit into one column; we now reduce this
    // number if the last column would contain white space
    sItemCount = (USHORT)WinSendMsg(hwndMenu,
                                    MM_QUERYITEMCOUNT,
                                    0, 0);
    // calculate number of resulting columns
    sColumns = (sItemCount / sItemsPerColumn) + 1;
    // distribute remainder in last column to all columns;
    // if you don't get this, don't worry, I got no clue either
    sItemsPerColumn -= (    (    sItemsPerColumn
                               - (sItemCount % sItemsPerColumn)
                            )
                         / sColumns
                       );

    // now, for through every (sItemsPerColumn)'th menu item,
    // set MIS_BREAK style to force a new column
    for (s = sItemsPerColumn;
         s < sItemCount;
         s += sItemsPerColumn)
    {
        sItemId = (USHORT)WinSendMsg(hwndMenu,
                                     MM_ITEMIDFROMPOSITION,
                                     (MPARAM)s, MPNULL);
        WinSendMsg(hwndMenu,
                   MM_QUERYITEM,
                   MPFROM2SHORT(sItemId, FALSE),
                   &mi);
        mi.afStyle |= MIS_BREAKSEPARATOR;
                        // [lafaix] mi.afStyle |= MIS_BREAK;
        WinSendMsg(hwndMenu,
                   MM_SETITEM,
                   MPFROM2SHORT(sItemId, FALSE),
                   &mi);
    }

    // clean up
    lstFree(&pllFolders);
    lstFree(&pllNonFolders);
}

/*
 *@@ cmnuFillContentSubmenu:
 *      this fills a folder content submenu stub (which was created
 *      with cmnuPrepareContentSubmenu) with the contents of the
 *      corresponding folder.
 *
 *      This checks for the "empty" menu item inserted by
 *      cmnuPrepareContentSubmenu and removes that before inserting
 *      the folder contents. As a result, this only works with menu
 *      stubs inserted by cmnuPrepareContentSubmenu. (To directly
 *      fill a menu, call cmnuInsertObjectsIntoMenu instead, which
 *      otherwise gets called by this function.)
 *
 *      This must then be called from the menu's owner window proc
 *      when WM_INITMENU is received for a folder content menu item.
 *      For example, fnwpSubclWPFolderWindow calls this for
 *      folder content menus.
 *
 *      This way, we can fill the folder content submenus only when
 *      this is needed, because we can impossibly populate all
 *      folder content menus when the context menu is initially
 *      opened.
 *
 *      This func only needs the sMenuId returned by that function
 *      and the hwndMenu of the submenu which will be filled.
 *      It automatically finds the folder contents from the
 *      somSelf which was specified with cmnuPrepareContentSubmenu.
 *      The submenu will be subclassed in order to allow opening
 *      objects with MB2.
 *
 *      Note that at the time that WM_INITMENU comes in, the (sub)menu
 *      is still invisible.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.1 (2000-02-01) [umoeller]: extracted mnuInsertObjectsIntoMenu for mutex protection
 */

VOID cmnuFillContentSubmenu(SHORT sMenuId, // in: menu ID of selected folder content submenu
                            HWND hwndMenu) // in: that submenu's window handle
{
    PLISTNODE       pNode = lstQueryFirstNode(&G_llContentMenuItems);
    PCONTENTMENULISTITEM pcmli = NULL;

    WPFolder        *pFolder = NULL;

    // first check if the menu contains the "[empty]" item;
    // this means that it's one of the folder content menus
    // and hasn't been filled yet (it could have been re-clicked,
    // and we don't want double menu items);
    if ((ULONG)WinSendMsg(hwndMenu,
                          MM_ITEMIDFROMPOSITION,
                          0, 0)
                == *G_pulVarMenuOfs + ID_XFMI_OFS_DUMMY
       )
    {
        // _Pmpf(("    first item is DUMMY"));

        // get folder to be populated from the linked list
        // (llContentMenuItems)
        while (pNode)
        {
            pcmli = pNode->pItemData;
            if (pcmli->sMenuId == sMenuId)
            {
                pFolder = pcmli->pFolder;
                break;
            }

            pNode = pNode->pNext;
            pcmli = 0;
        }

        // _Pmpf(("    folder is %s", (pFolder) ? _wpQueryTitle(pFolder) : "NULL"));

        // pFolder now contains the folder,
        // pcmli has the CONTENTMENULISTITEM
        if (pFolder)
        {   // folder found: populate

            // show "Wait" pointer
            HPOINTER    hptrOld = winhSetWaitPointer();

            // if pFolder is a disk object: get root folder
            if (_somIsA(pFolder, _WPDisk))
                pFolder = _XFldDisk    // V1.0.5 (2006-06-10) [pr]: fix crash
                          ? _xwpSafeQueryRootFolder(pFolder, FALSE, NULL)
                          : _wpQueryRootFolder(pFolder);

            if (pFolder)
            {
                // populate
                fdrCheckIfPopulated(pFolder,
                                    FALSE);    // full populate

                if (_wpQueryContent(pFolder, NULL, QC_FIRST))
                {
                    // folder does contain objects: go!
                    // insert all objects (this takes a long time)...
                    cmnuInsertObjectsIntoMenu(pFolder,
                                              hwndMenu);

                    // fix menu position...
                }
            }

            // reset the mouse pointer
            WinSetPointer(HWND_DESKTOP, hptrOld);
        } // end if (pFolder)
    } // end if MM_ITEMIDFROMPOSITION == ID_XFMI_OFS_DUMMY
}

/*
 *@@ cmnuGetVarItem:
 *
 *@@added V0.9.7 (2000-11-29) [umoeller]
 */

PVARMENULISTITEM cmnuGetVarItem(ULONG ulOfs)
{
    return (PVARMENULISTITEM)lstItemFromIndex(&G_llVarMenuItems,
                                              ulOfs);
}

/* ******************************************************************
 *
 *   Functions for folder content menu ownerdraw
 *
 ********************************************************************/

#ifndef RGB_DARKGRAY // [lafaix]
  #define RGB_DARKGRAY 0x00808080L
#endif

/*
 *@@ cmnuPrepareOwnerDraw:
 *      this is called from the subclassed folder frame procedure
 *      (fnwpSubclWPFolderWindow in xfldr.c) when it receives
 *      WM_INITMENU for a folder content submenu. We can now
 *      do a few queries to get important data which we need for
 *      owner-drawing later.
 *
 *@@changed V0.9.0 [umoeller]: changed colors
 *@@changed V0.9.7 (2001-01-21) [lafaix]: fixed nonrecursive queries
 *@@changed V0.9.7 (2001-01-21) [lafaix]: added border colors and CharBox
 *@@changed V0.9.7 (2001-01-26) [lafaix]: fixed nonrecursive queries
 */

VOID cmnuPrepareOwnerDraw(// SHORT sMenuIDMsg, // from WM_INITMENU: SHORT mp1 submenu id
                          HWND hwndMenuMsg) // from WM_INITMENU: HWND  mp2 menu window handle
{
    // query bounding rectangle of "[empty]" item, according to
    // which we will format our own items
    WinSendMsg(hwndMenuMsg,
               MM_QUERYITEMRECT,
               MPFROM2SHORT(WinSendMsg(hwndMenuMsg,
                                       MM_ITEMIDFROMPOSITION,
                                       0, 0),
                            FALSE),
               &G_rtlMenuItem);

    // query presentation parameters (colors and font) for menu (changed V0.9.0)
    G_lBackground     = winhQueryPresColor(hwndMenuMsg,
                                           PP_MENUBACKGROUNDCOLOR,
                                           TRUE, // [lafaix] FALSE,       // no inherit
                                           SYSCLR_MENU);
    G_lText           = winhQueryPresColor(hwndMenuMsg,
                                           PP_MENUFOREGROUNDCOLOR,
                                           TRUE /* FALSE */,
                                           SYSCLR_MENUTEXT);
    G_lHiliteBackground = winhQueryPresColor(hwndMenuMsg,
                                             PP_MENUHILITEBGNDCOLOR,
                                             TRUE /* FALSE */,
                                             SYSCLR_MENUHILITEBGND);
    G_lHiliteText     = winhQueryPresColor(hwndMenuMsg,
                                           PP_MENUHILITEFGNDCOLOR,
                                           TRUE /* FALSE */,
                                           SYSCLR_MENUHILITE);
    // [lafaix] begin

    #ifndef PP_BORDERLIGHTCOLOR
        #define PP_BORDERLIGHTCOLOR   51L
    #endif

    G_lBorderLight = winhQueryPresColor(hwndMenuMsg,
                                        PP_BORDERLIGHTCOLOR, TRUE, -1);
    if (G_lBorderLight == -1)
        G_lBorderLight = RGB_WHITE;

    #ifndef PP_BORDERDARKCOLOR
        #define PP_BORDERDARKCOLOR   52L
    #endif

    G_lBorderDark = winhQueryPresColor(hwndMenuMsg,
                                       PP_BORDERDARKCOLOR, TRUE, -1);
    if (G_lBorderDark == -1)
        G_lBorderDark = RGB_DARKGRAY;

    // query font CharBox and MaxDescender, to be done in cmnuMeasureItem
    // as we don't have the presentation space here
    G_bInitNeeded = TRUE;

    /* G_szfCharBox.cx = 0; G_szfCharBox.cy = 0;

    G_lMaxDescender = 0; */
    // [lafaix] end
}

/*
 *@@ cmnuMeasureItem:
 *      this is called from the subclassed folder frame procedure
 *      (fnwpSubclWPFolderWindow in xfldr.c) when it receives
 *      WM_MEASUREITEM for each owner-draw folder content menu item.
 *      We will use the data queried above to calculate the dimensions
 *      of the items we're going to draw later.
 *      We must return the MRESULT for WM_MEASUREITEM here, about
 *      which different documentation exists... I have chosen to return
 *      the height of the menu bar only.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.7 (2001-01-26) [lafaix]: added CharBox and MaxDescender queries
 *@@changed V0.9.16 (2001-10-31) [umoeller]: now using at least system mini-icon height
 *@@changed V0.9.19 (2002-06-18) [umoeller]: now truncating long titles with ellipses
 */

MRESULT cmnuMeasureItem(POWNERITEM poi)     // owner-draw info structure
{
    PVARMENULISTITEM pItem;

    if (G_ulMiniIconSize == 0)
        // not queried yet?
        G_ulMiniIconSize = WinQuerySysValue(HWND_DESKTOP, SV_CYICON) / 2;

    if (G_bInitNeeded)
    {
        FONTMETRICS fm;

        // the presentation space we get in cmnuDrawItem may differ from
        // the one we have here, it that the font size reverts to the one
        // of the default font, so we save it
        GpiQueryCharBox(poi->hps, &G_szfCharBox);

        // get max descender which is needed to position the item
        // correctly in its rectangle
        GpiQueryFontMetrics(poi->hps, sizeof(FONTMETRICS), &fm);
        G_lMaxDescender = fm.lMaxDescender;

        G_bInitNeeded = FALSE;
    }

    // get the item from the linked list of variable menu items
    // which corresponds to the menu item whose size is being queried
    if (pItem = (PVARMENULISTITEM)lstItemFromIndex(&G_llVarMenuItems,
                                                   (   poi->idItem
                                                     - (  *G_pulVarMenuOfs
                                                        + ID_XFMI_OFS_VARIABLE))))
    {
        // find out the space required for drawing this item with
        // the current font and fill the owner draw structure (mp2)
        // accordingly

            /* POINTL aptlText[TXTBOX_COUNT];
            GpiQueryTextBox(poi->hps,
                            pItem->ulTitleLen,
                            pItem->pszTitle,
                            TXTBOX_COUNT,
                            (PPOINTL)&aptlText);
            poi->rclItem.xRight = aptlText[TXTBOX_TOPRIGHT].x
                                    + G_ulMiniIconSize
                                    + 6;     // lafaix
            */

        // V0.9.19 (2002-06-18) [umoeller]: rewritten to insert ellipse
        // if the string to be painted is terribly long

        LONG    cxTitle = 0;

        // it would be more accurate if I moved this call
        // into the loop so that the array is updated with
        // every ellipse that replaces one character, but
        // why add that much expense for a couple of pixels...
        // this can get called 200 times for each menu item
        // in the worst case
        GpiQueryCharStringPos(poi->hps,
                              0,        // no options
                              pItem->ulTitleLen,
                              pItem->pszTitle,
                              NULL,
                              pItem->aptlPositions);

        while (TRUE)
        {
            cxTitle = pItem->aptlPositions[pItem->ulTitleLen].x;

            if (    (cxTitle > G_cxScreen / 4)
                 && (pItem->ulTitleLen > 10)
               )
            {
                // "abcdefghijkl"
                //  ^ pItem->pszTitle
                //              ^ pItem->pszTitle[pItem->ulTitleLen]
                //          ^ pItem->pszTitle[pItem->ulTitleLen - 4]
                memcpy(pItem->pszTitle + pItem->ulTitleLen - 4,
                       "...",
                       4);      // null-terminate
                --(pItem->ulTitleLen);
            }
            else
                break;
        }

        poi->rclItem.xRight =   cxTitle
                              + G_ulMiniIconSize
                              + 6;     // lafaix

        poi->rclItem.yTop = G_rtlMenuItem.yTop - G_rtlMenuItem.yBottom;

        // make sure the item has at least the height of
        // the system mini-icon size
        // V0.9.16 (2001-10-31) [umoeller]
        if (poi->rclItem.yTop < G_ulMiniIconSize + 2)
            poi->rclItem.yTop = G_ulMiniIconSize + 2;
    }

    return MRFROMSHORT(poi->rclItem.yTop);
}

/*
 *@@ cmnuDrawItem:
 *      this is called from the subclassed folder frame procedure
 *      (fnwpSubclWPFolderWindow in xfldr.c) when it receives
 *      WM_DRAWITEM for each owner-draw folder content menu item.
 *      We will draw one menu item including the icons with each
 *      call of this function.
 *      This must return TRUE if the item was drawn.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for new linklist functions
 *@@changed V0.9.7 (2001-01-21) [lafaix]: reworked submenu arrow handling
 *@@changed V0.9.7 (2001-01-21) [lafaix]: reworked painting stuff
 *@@changed V0.9.16 (2001-10-31) [umoeller]: now using at least system mini-icon height
 *@@changed V0.9.19 (2002-04-14) [lafaix]: fixed text vertical position
 *@@changed V0.9.19 (2002-06-18) [umoeller]: added in-use emphasis
 */

BOOL cmnuDrawItem(MPARAM mp1,     // from WM_DRAWITEM: USHORT menu item id
                  MPARAM mp2)     // from WM_DRAWITEM: POWNERITEM structure
{
    BOOL brc = FALSE;
    RECTL      rcl;
    LONG       lColor;
    POWNERITEM poi = (POWNERITEM)mp2;
    POINTL     ptl;

    // get the item from the linked list of variable menu items
    // which corresponds to the menu item being drawn
    PVARMENULISTITEM pItem;
    HPOINTER hptrIcon;

    if (pItem = (PVARMENULISTITEM)lstItemFromIndex(&G_llVarMenuItems,
                                                   (poi->idItem
                                                      - (   *G_pulVarMenuOfs
                                                          + ID_XFMI_OFS_VARIABLE))))
    {
        if (    (poi->fsAttribute != poi->fsAttributeOld)
             && (pItem->ulObjType == OC_CONTENTFOLDER)
           )
        {
            // if the attribute of a folder has changed, i.e. item's
            // hilite state has been altered: we then need
            // to handle the little "submenu" arrow, because
            // this will be overpainted by the WinFillRect
            // below. We do this by sending a MM_SETITEMATTR
            // to the menu.
            // This will call cmnuDrawItem again to paint the item.
            WinSendMsg(poi->hwnd,
                       MM_SETITEMATTR,
                       MPFROM2SHORT(poi->idItem,
                                    FALSE), // no search submenus
                       MPFROM2SHORT(MIA_HILITED, (poi->fsAttribute & MIA_HILITED)));
        }
        else
        {
            LONG    xIcon,
                    yIcon;

            // get the item's (object's) icon;
            // this call can take a while if the folder
            // was just queried
            hptrIcon = _wpQueryIcon(pItem->pObject);

            // switch to RGB mode
            GpiCreateLogColorTable(poi->hps, 0, LCOLF_RGB, 0, 0, NULL);

            // find out the background color, which depends
            // on whether the item is highlighted (= selected);
            // these colors have been initialized by WM_INITMENU
            // above
            if (poi->fsAttribute & MIA_HILITED)
                lColor = G_lHiliteBackground;
            else
                lColor = G_lBackground;

            // draw rectangle in lColor, size of whole item
            rcl = poi->rclItem;
            rcl.xLeft += 4; // [lafaix]
            rcl.xRight -= 2; // [lafaix]
            WinFillRect(poi->hps, &rcl, lColor);

            // let's fix the vertical border that has been overwritten by PM
            GpiSetColor(poi->hps,
                        G_lBorderLight);
            ptl.x = poi->rclItem.xLeft;
            ptl.y = poi->rclItem.yTop;
            GpiMove(poi->hps, &ptl);
            ptl.y = poi->rclItem.yBottom;
            GpiLine(poi->hps, &ptl);
            // if we are close to the bottom, let's fix that border too;
            // the "+4" offset makes us redraw the bottom twice in most
            // cases but is necessary as some menus with three or more
            // columns may have an odd number of elements
            if (rcl.yBottom < (rcl.yTop - rcl.yBottom + 4))
            {
                ptl.y = 1;
                GpiLine(poi->hps, &ptl);
                ptl.x++;
                GpiSetColor(poi->hps,
                            G_lBorderDark);
                GpiMove(poi->hps, &ptl);
                ptl.x = poi->rclItem.xRight;
                GpiLine(poi->hps, &ptl);
            }

            // calculate the base for both the object and the icon text
            ptl.x =    poi->rclItem.xLeft
                     + G_ulMiniIconSize
                     + 10;

            ptl.y = poi->rclItem.yBottom + G_lMaxDescender + 1;

            // centering the text vertically if smaller than the mini icon
            // V0.9.18 (2002-03-11) [lafaix]
            if ((G_rtlMenuItem.yTop - G_rtlMenuItem.yBottom) < (G_ulMiniIconSize + 2))
                ptl.y += (G_ulMiniIconSize + 2 - (G_rtlMenuItem.yTop - G_rtlMenuItem.yBottom)) / 2;

            // print the item's text
            GpiMove(poi->hps, &ptl);
            GpiSetColor(poi->hps,
                        (poi->fsAttribute & MIA_HILITED)
                            ? G_lHiliteText
                            : G_lText);
            GpiSetCharBox(poi->hps, &G_szfCharBox);
            GpiCharString(poi->hps,
                          pItem->ulTitleLen,
                          pItem->pszTitle);

            // calc icon position
            xIcon = poi->rclItem.xLeft + 6;
                    // center vertically:
                    // V0.9.16 (2001-10-31) [umoeller]
            yIcon = poi->rclItem.yBottom
                    + (    (   poi->rclItem.yTop
                             - poi->rclItem.yBottom
                             - G_ulMiniIconSize
                           ) / 2
                      );

            // draw the icon hatched if we have an open view
            // V0.9.19 (2002-06-18) [umoeller]
            if (_wpFindViewItem(pItem->pObject, VIEW_ANY, NULL))
            {
                POINTL ptl2;
                ptl2.x = xIcon - 2;
                ptl2.y = rcl.yBottom;
                GpiMove(poi->hps,
                        &ptl2);
                GpiSetPattern(poi->hps, PATSYM_DIAG1);
                GpiSetColor(poi->hps, 0);       // RGB black
                ptl2.x += G_ulMiniIconSize + 2 - 1; // inclusive!
                ptl2.y = rcl.yTop - 1; // inclusive!
                GpiBox(poi->hps,
                       DRO_FILL,
                       &ptl2,
                       0,
                       0);
            }

            // draw the item's icon
            WinDrawPointer(poi->hps,
                           xIcon,
                           yIcon,
                           hptrIcon,
                           DP_MINI);
        }

        // now, this is funny: we need to ALWAYS delete the
        // MIA_HILITED flag in both the old and new attributes,
        // or PM will invert the item again for some reason;
        // this must be code from the stone ages (i.e. Microsoft)
        poi->fsAttributeOld = (poi->fsAttribute &= ~MIA_HILITED);

        brc = TRUE;
    }

    return brc;
}


