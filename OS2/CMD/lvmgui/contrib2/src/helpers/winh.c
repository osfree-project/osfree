
/*
 *@@sourcefile winh.c:
 *      contains Presentation Manager helper functions that are
 *      independent of a single application, i.e. these can be
 *      used w/out the rest of the XWorkplace source in any PM
 *      program.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  winh*   Win (Presentation Manager) helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\winh.h"
 */

/*
 *      Copyright (C) 1997-2010 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSPROFILE
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSSESMGR
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WININPUT
#define INCL_WINDIALOGS
#define INCL_WINPOINTERS
#define INCL_WINRECTANGLES
#define INCL_WINSHELLDATA
#define INCL_WINTIMER
#define INCL_WINSYS
#define INCL_WINHELP
#define INCL_WINPROGRAMLIST
#define INCL_WINSWITCHLIST
#define INCL_WINBUTTONS
#define INCL_WINSTATICS
#define INCL_WINMENUS
#define INCL_WINENTRYFIELDS
#define INCL_WINSCROLLBARS
#define INCL_WINLISTBOXES
#define INCL_WINSTDSPIN
#define INCL_WINSTDSLIDER
#define INCL_WINCIRCULARSLIDER
#define INCL_WINSTDFILE
#define INCL_WINCLIPBOARD

#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS

#define INCL_GPIBITMAPS
#define INCL_GPIPRIMITIVES
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\winh.h"
#include "helpers\prfh.h"
#include "helpers\gpih.h"
#include "helpers\nls.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\xstring.h"            // extended string helpers

/*
 *@@category: Helpers\PM helpers\Wrappers
 */

/* ******************************************************************
 *
 *   Wrappers
 *
 ********************************************************************/

#ifdef WINH_STANDARDWRAPPERS

    /*
     *@@ winhSendMsg:
     *      wrapper for WinSendMsg.
     *
     *      If WINH_STANDARDWRAPPERS is #defined before
     *      including win.h, all WinSendMsg calls are
     *      redefined to use this wrapper instead. This
     *      reduces the amount of external fixups required
     *      for loading the module.
     *
     *@@added V0.9.12 (2001-05-18) [umoeller]
     */

    MRESULT winhSendMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
        // put the call in brackets so the macro won't apply here
        return (WinSendMsg)(hwnd, msg, mp1, mp2);
    }

    /*
     *@@ winhSendDlgItemMsg:
     *      wrapper for WinSendDlgItemMsg.
     *
     *      If WINH_STANDARDWRAPPERS is #defined before
     *      including win.h, all WinSendMsg calls are
     *      redefined to use this wrapper instead. This
     *      reduces the amount of external fixups required
     *      for loading the module.
     *
     *@@added V0.9.13 (2001-06-27) [umoeller]
     */

    MRESULT winhSendDlgItemMsg(HWND hwnd, ULONG id, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
        return (WinSendDlgItemMsg)(hwnd, id, msg, mp1, mp2);
    }

    /*
     *@@ winhPostMsg:
     *      wrapper for WinPostMsg.
     *
     *      If WINH_STANDARDWRAPPERS is #defined before
     *      including win.h, all WinSendMsg calls are
     *      redefined to use this wrapper instead. This
     *      reduces the amount of external fixups required
     *      for loading the module.
     *
     *@@added V0.9.12 (2001-05-18) [umoeller]
     */

    BOOL winhPostMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
    {
        // put the call in brackets so the macro won't apply here
        return (WinPostMsg)(hwnd, msg, mp1, mp2);
    }

    /*
     *@@ winhWindowFromID:
     *
     *@@added V0.9.12 (2001-05-18) [umoeller]
     */

    HWND winhWindowFromID(HWND hwnd, ULONG id)
    {
        // put the call in brackets so the macro won't apply here
        return (WinWindowFromID)(hwnd, id);
    }

    /*
     *@@ winhQueryWindow:
     *
     *@@added V0.9.12 (2001-05-18) [umoeller]
     */

    HWND winhQueryWindow(HWND hwnd, LONG lCode)
    {
        // put the call in brackets so the macro won't apply here
        return (WinQueryWindow)(hwnd, lCode);
    }

    /*
     *@@ winhQueryWindowPtr:
     *
     *@@added V0.9.13 (2001-06-21) [umoeller]
     */

    PVOID winhQueryWindowPtr(HWND hwnd, LONG index)
    {
        // put the call in brackets so the macro won't apply here
        return (WinQueryWindowPtr)(hwnd, index);
    }

    /*
     *@@ winhSetWindowText2:
     *
     *@@added V0.9.13 (2001-06-21) [umoeller]
     */

    BOOL winhSetWindowText2(HWND hwnd, const char *pcsz)
    {
        // put the call in brackets so the macro won't apply here
        return (WinSetWindowText)(hwnd, (PSZ)pcsz);
    }

    /*
     *@@ winhSetDlgItemText:
     *
     *@@added V0.9.13 (2001-06-21) [umoeller]
     */

    BOOL winhSetDlgItemText(HWND hwnd, ULONG id, const char *pcsz)
    {
        // put the call in brackets so the macro won't apply here
        return (WinSetDlgItemText)(hwnd, id, (PSZ)pcsz);
    }

    /*
     *@@ winhRequestMutexSem:
     *
     *@@added V0.9.16 (2002-01-26) [umoeller]
     */

    APIRET winhRequestMutexSem(HMTX hmtx, ULONG ulTimeout)
    {
        // put the call in brackets so the macro won't apply here
        return (WinRequestMutexSem)(hmtx, ulTimeout);
    }

#endif // WINH_STANDARDWRAPPERS

/*
 *@@category: Helpers\PM helpers\Rectangle helpers
 */

/* ******************************************************************
 *
 *   Rectangle helpers
 *
 ********************************************************************/

/*
 *@@ winhOffsetRect:
 *      like WinOffsetRect, but doesn't require
 *      an anchor block to be passed in. Why
 *      the original would need an anchor block
 *      for this awfully complicated task is
 *      a mystery to me anyway.
 *
 *@@added V0.9.9 (2001-03-13) [umoeller]
 */

VOID winhOffsetRect(PRECTL prcl,
                    LONG lx,
                    LONG ly)
{
    prcl->xLeft += lx;
    prcl->xRight += lx;
    prcl->yBottom += ly;
    prcl->yTop += ly;
}

/*
 *@@category: Helpers\PM helpers\Generics
 */

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

extern LONG G_cxScreen = 0,
            G_cyScreen = 0,
            G_cxIcon = 0,
            G_cyIcon = 0,
            G_lcol3DDark = 0,         // lo-3D color
            G_lcol3DLight = 0;        // hi-3D color

/* ******************************************************************
 *
 *   Generics
 *
 ********************************************************************/

/*
 *@@ winhInitGlobals:
 *      initializes a few global variables that are usually
 *      used all the time in many applications and also
 *      internally by many helper routines. You must call
 *      this at the start of your application if you want
 *      to use these.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

VOID winhInitGlobals(VOID)
{
    G_cxScreen = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    G_cyScreen = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
    G_cxIcon = WinQuerySysValue(HWND_DESKTOP, SV_CXICON);
    G_cyIcon = WinQuerySysValue(HWND_DESKTOP, SV_CYICON);
    G_lcol3DDark = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0);
    G_lcol3DLight = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);
}

/*
 *@@ winhQueryWindowStyle:
 *
 *@@added V0.9.13 (2001-07-02) [umoeller]
 */

ULONG winhQueryWindowStyle(HWND hwnd)
{
    return WinQueryWindowULong(hwnd, QWL_STYLE);
}

/*
 *@@ winhEnableDlgItem:
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

BOOL winhEnableDlgItem(HWND hwndDlg,
                       SHORT id,
                       BOOL fEnable)
{
    return WinEnableWindow(WinWindowFromID(hwndDlg, id), fEnable);
}

/*
 *@@ winhIsDlgItemEnabled:
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

BOOL winhIsDlgItemEnabled(HWND hwndDlg,
                          SHORT id)
{
    return WinIsWindowEnabled(WinWindowFromID(hwndDlg, id));
}

/*
 *@@ winhDestroyWindow:
 *      wrapper around WinDestroyWindow that also sets
 *      *phwnd to NULLHANDLE to avoid loose window
 *      handles lying around.
 *
 *@@added V1.0.1 (2002-12-11) [umoeller]
 */

BOOL winhDestroyWindow(HWND *phwnd)
{
    if (    (*phwnd)
         && (WinDestroyWindow(*phwnd))
       )
    {
        *phwnd = NULLHANDLE;

        return TRUE;
    }

    return FALSE;
}

/*
 *@@category: Helpers\PM helpers\Menu helpers
 */

/* ******************************************************************
 *
 *   Menu helpers
 *
 ********************************************************************/

/*
 *@@ winhQueryMenuItem:
 *      wrapper around MM_QUERYITEM.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

BOOL winhQueryMenuItem(HWND hwndMenu,
                       USHORT usItemID,
                       BOOL fSearchSubmenus,
                       PMENUITEM pmi)           // out: MENUITEM data
{
    return (BOOL)WinSendMsg(hwndMenu,
                            MM_QUERYITEM,
                            MPFROM2SHORT(usItemID, fSearchSubmenus),
                            (MPARAM)pmi);
}

/*
 *@@ winhQuerySubmenu:
 *      tests whether sID specifies a submenu in
 *      hMenu and returns the submenu window handle
 *      if so.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

HWND winhQuerySubmenu(HWND hMenu,
                      SHORT sID)
{
    MENUITEM mi = {0};
    if (    (WinSendMsg(hMenu,
                        MM_QUERYITEM,
                        MPFROM2SHORT(sID,
                                     FALSE),
                        (MPARAM)&mi))
         && (mi.afStyle & MIS_SUBMENU)
       )
        return mi.hwndSubMenu;

    return NULLHANDLE;
}

/*
 *@@ winhInsertMenuItem:
 *      this inserts one one menu item into a given menu.
 *
 *      Returns the return value of the MM_INSERTITEM msg:
 *      --  MIT_MEMERROR:    space allocation for menu item failed
 *      --  MIT_ERROR:       other error
 *      --  other:           zero-based index of new item in menu.
 */

SHORT winhInsertMenuItem(HWND hwndMenu,     // in:  menu to insert item into
                         SHORT iPosition,   // in:  zero-based index of where to
                                            //      insert or MIT_END
                         SHORT sItemId,     // in:  ID of new menu item
                         const char *pcszItemTitle,  // in:  title of new menu item
                         SHORT afStyle,
                            // in:  MIS_* style flags.
                            // Valid menu item styles are:
                            // --  MIS_SUBMENU
                            // --  MIS_SEPARATOR
                            // --  MIS_BITMAP: the display object is a bit map.
                            // --  MIS_TEXT: the display object is a text string.
                            // --  MIS_BUTTONSEPARATOR:
                            //          The item is a menu button. Any menu can have zero,
                            //          one, or two items of this type.  These are the last
                            //          items in a menu and are automatically displayed after
                            //          a separator bar. The user cannot move the cursor to
                            //          these items, but can select them with the pointing
                            //          device or with the appropriate key.
                            // --  MIS_BREAK: the item begins a new row or column.
                            // --  MIS_BREAKSEPARATOR:
                            //          Same as MIS_BREAK, except that it draws a separator
                            //          between rows or columns of a pull-down menu.
                            //          This style can only be used within a submenu.
                            // --  MIS_SYSCOMMAND:
                            //          menu posts a WM_SYSCOMMAND message rather than a
                            //          WM_COMMAND message.
                            // --  MIS_OWNERDRAW:
                            //          WM_DRAWITEM and WM_MEASUREITEM notification messages
                            //          are sent to the owner to draw the item or determine its size.
                            // --  MIS_HELP:
                            //          menu posts a WM_HELP message rather than a
                            //          WM_COMMAND message.
                            // --  MIS_STATIC
                            //          This type of item exists for information purposes only.
                            //          It cannot be selected with the pointing device or
                            //          keyboard.
                         SHORT afAttr)
                            // in:  MIA_* attribute flags
                            // Valid menu item attributes (afAttr) are:
                            // --  MIA_HILITED: if and only if, the item is selected.
                            // --  MIA_CHECKED: a check mark appears next to the item (submenu only).
                            // --  MIA_DISABLED: item is disabled and cannot be selected.
                            //         The item is drawn in a disabled state (gray).
                            // --  MIA_FRAMED: a frame is drawn around the item (top-level menu only).
                            // --  MIA_NODISMISS:
                            //          if the item is selected, the submenu remains down. A menu
                            //          with this attribute is not hidden until the  application
                            //          or user explicitly does so, for example by selecting either
                            //          another menu on the action bar or by pressing the escape key.
{
    MENUITEM mi;

    mi.iPosition = iPosition;
    mi.afStyle = afStyle;
    mi.afAttribute = afAttr;
    mi.id = sItemId;
    mi.hwndSubMenu = 0;
    mi.hItem = 0;

    return SHORT1FROMMR(WinSendMsg(hwndMenu,
                                   MM_INSERTITEM,
                                   (MPARAM)&mi,
                                   (MPARAM)pcszItemTitle));
}

/*
 *@@ winhInsertSubmenu:
 *      this inserts a submenu into a given menu and, if
 *      sItemId != 0, inserts one item into this new submenu also.
 *
 *      See winhInsertMenuItem for valid menu item styles and
 *      attributes.
 *
 *      Returns the HWND of the new submenu.
 */

HWND winhInsertSubmenu(HWND hwndMenu,       // in: menu to add submenu to
                       ULONG iPosition,     // in: index where to add submenu or MIT_END
                       SHORT sMenuId,       // in: menu ID of new submenu
                       const char *pcszSubmenuTitle, // in: title of new submenu
                       USHORT afMenuStyle,  // in: MIS* style flags for submenu;
                                            // MIS_SUBMENU will always be added
                       SHORT sItemId,       // in: ID of first item to add to submenu;
                                            // if 0, no first item is inserted
                       const char *pcszItemTitle,    // in: title of this item
                                            // (if sItemID != 0)
                       USHORT afItemStyle,  // in: style flags for this item, e.g. MIS_TEXT
                                            // (this is ignored if sItemID == 0)
                       USHORT afAttribute)  // in: attributes for this item, e.g. MIA_DISABLED
                                            // (this is ignored if sItemID == 0)
{
    MENUITEM mi;
    SHORT    src = MIT_ERROR;
    HWND     hwndNewMenu;

    // create new, empty menu
    hwndNewMenu = WinCreateMenu(hwndMenu,
                                NULL); // no menu template
    if (hwndNewMenu)
    {
        // add "submenu item" to this empty menu;
        // for some reason, PM always needs submenus
        // to be a menu item
        mi.iPosition = iPosition;
        mi.afStyle = afMenuStyle | MIS_SUBMENU;
        mi.afAttribute = 0;
        mi.id = sMenuId;
        mi.hwndSubMenu = hwndNewMenu;
        mi.hItem = 0;
        src = SHORT1FROMMR(WinSendMsg(hwndMenu, MM_INSERTITEM, (MPARAM)&mi, (MPARAM)pcszSubmenuTitle));
        if (    (src != MIT_MEMERROR)
            &&  (src != MIT_ERROR)
           )
        {
            // set the new menu's ID to the same as the
            // submenu item
            WinSetWindowUShort(hwndNewMenu, QWS_ID, sMenuId);

            if (sItemId)
            {
                // item id given: insert first menu item also
                mi.iPosition = 0;
                mi.afStyle = afItemStyle;
                mi.afAttribute = afAttribute;
                mi.id = sItemId;
                mi.hwndSubMenu = 0;
                mi.hItem = 0;
                WinSendMsg(hwndNewMenu,
                           MM_INSERTITEM,
                           (MPARAM)&mi,
                           (MPARAM)pcszItemTitle);
            }
        }
    }
    return hwndNewMenu;
}

/*
 *@@ winhSetMenuCondCascade:
 *      sets the "conditional cascade" style
 *      on the specified submenu.
 *
 *      This style must always be enabled manually
 *      because the resource compiler won't handle it.
 *
 *      Note: Pass in the _submenu_ window handle,
 *      not the one of the parent. With lDefaultItem,
 *      specify the item ID in the submenu which is
 *      to be checked as the default item.
 *
 *@@added V0.9.12 (2001-05-22) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: now supporting calling this more than once
 */

BOOL winhSetMenuCondCascade(HWND hwndMenu,          // in: submenu handle
                            LONG lDefaultItem)      // in: item ID of new default item
{
    BOOL    brc;
    ULONG   ulStyle = WinQueryWindowULong(hwndMenu, QWL_STYLE);
    LONG    lOldDefault = -1;

    if (ulStyle & MS_CONDITIONALCASCADE)
    {
        // menu is already conditional cascade:
        lOldDefault = (LONG)WinSendMsg(hwndMenu,
                                       MM_QUERYDEFAULTITEMID,
                                       0,
                                       0);
    }
    else
    {
        ulStyle |= MS_CONDITIONALCASCADE;
        WinSetWindowULong(hwndMenu, QWL_STYLE, ulStyle);
    }

    // make the first item in the subfolder
    // the default of cascading submenu
    brc = (BOOL)WinSendMsg(hwndMenu,
                           MM_SETDEFAULTITEMID,
                           (MPARAM)lDefaultItem,
                           0);

    if (    (lOldDefault != -1)
         && (lOldDefault != lDefaultItem)
       )
    {
        // unset the "checked" attribute of the old one
        // or we'll have two in the menu
        WinSendMsg(hwndMenu,
                   MM_SETITEMATTR,
                   MPFROM2SHORT(lOldDefault,
                                FALSE),
                   MPFROM2SHORT(MIA_CHECKED, 0));
    }

    return brc;
}

/*
 *@@ winhRemoveMenuItems:
 *      removes multiple menu items at once, as
 *      specified in the given array of menu item
 *      IDs.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

BOOL XWPENTRY winhRemoveMenuItems(HWND hwndMenu,            // in: menu to remove from
                                  const SHORT *asItemIDs,   // in: array of menu item IDs
                                  ULONG cItemIDs)           // in: array item count
{
    ULONG ul;
    for (ul = 0;
         ul < cItemIDs;
         ++ul)
    {
        SHORT s = asItemIDs[ul];
        winhRemoveMenuItem(hwndMenu,
                           s);
    }

    return TRUE;
}

/*
 *@@ winhInsertMenuSeparator:
 *      this inserts a separator into a given menu at
 *      the given position (which may be MIT_END);
 *      returns the position at which the item was
 *      inserted.
 */

SHORT winhInsertMenuSeparator(HWND hMenu,       // in: menu to add separator to
                              SHORT iPosition,  // in: index where to add separator or MIT_END
                              SHORT sId)        // in: separator menu ID (doesn't really matter)
{
    MENUITEM mi;
    mi.iPosition = iPosition;
    mi.afStyle = MIS_SEPARATOR;             // append separator
    mi.afAttribute = 0;
    mi.id = sId;
    mi.hwndSubMenu = 0;
    mi.hItem = 0;

    return SHORT1FROMMR(WinSendMsg(hMenu,
                                   MM_INSERTITEM,
                                   (MPARAM)&mi,
                                   (MPARAM)""));
}

/*
 *@@ winhCopyMenuItem2:
 *      copies a menu item from hmenuSource to hmenuTarget.
 *
 *      This creates a full duplicate. If usID specifies
 *      a submenu, the entire submenu is copied as well
 *      (this will then recurse).
 *
 *      fl can be any combination of:
 *
 *      --  COPYFL_STRIPTABS: strip off \t and everything
 *          that follows, if present.
 *
 *      NOTE: Copying submenus will work only if each item
 *      in the submenu has a unique menu ID. This is due
 *      to the dumb implementation of menus in PM where
 *      it is impossible to query menu items without
 *      knowing their ID.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: renamed, added fl
 */

BOOL winhCopyMenuItem2(HWND hmenuTarget,
                       HWND hmenuSource,
                       USHORT usID,
                       SHORT sTargetPosition,    // in: position to insert at or MIT_END
                       ULONG fl)                 // in: COPYFL_* flags
{
    BOOL brc = FALSE;
    MENUITEM mi = {0};
    if (WinSendMsg(hmenuSource,
                   MM_QUERYITEM,
                   MPFROM2SHORT(usID,
                                FALSE),
                   (MPARAM)&mi))
    {
        // found in source:
        // is it a separator?
        if (mi.afStyle & MIS_SEPARATOR)
            winhInsertMenuSeparator(hmenuTarget,
                                    sTargetPosition,
                                    usID);
        else
        {
            // no separator:
            // get item text
            PSZ pszSource;
            if (pszSource = winhQueryMenuItemText(hmenuSource,
                                                  usID))
            {
                PSZ p;
                // remove the hotkey description
                // V0.9.20 (2002-08-10) [umoeller]
                if (    (fl & COPYFL_STRIPTABS)
                     && (p = strchr(pszSource, '\t'))
                   )
                    *p = '\0';

                if (    (mi.afStyle & MIS_SUBMENU)
                     && (mi.hwndSubMenu)
                   )
                {
                    // this is the top of a submenu:
                    HWND hwndSubMenu;
                    if (hwndSubMenu = winhInsertSubmenu(hmenuTarget,
                                                        sTargetPosition,
                                                        mi.id,
                                                        pszSource,
                                                        mi.afStyle,
                                                        0,
                                                        NULL,
                                                        0,
                                                        0))
                    {
                        // now copy all the items in the submenu
                        SHORT cMenuItems = SHORT1FROMMR(WinSendMsg(mi.hwndSubMenu,
                                                                   MM_QUERYITEMCOUNT,
                                                                   0,
                                                                   0));
                        // loop through all entries in the original submenu
                        ULONG i;
                        for (i = 0;
                             i < cMenuItems;
                             i++)
                        {
                            SHORT id = SHORT1FROMMR(WinSendMsg(mi.hwndSubMenu,
                                                         MM_ITEMIDFROMPOSITION,
                                                         MPFROMSHORT(i),
                                                         0));
                            // recurse
                            winhCopyMenuItem2(hwndSubMenu,
                                             mi.hwndSubMenu,
                                             id,
                                             MIT_END,
                                             fl);
                        }

                        // now check... was the original submenu
                        // "conditional cascade"?
                        if (WinQueryWindowULong(mi.hwndSubMenu,
                                                QWL_STYLE)
                                & MS_CONDITIONALCASCADE)
                            // yes:
                        {
                            // get the original default item
                            SHORT sDefID = SHORT1FROMMR(WinSendMsg(mi.hwndSubMenu,
                                                                   MM_QUERYDEFAULTITEMID,
                                                                   0,
                                                                   0));
                            // set "conditional cascade style" on target too
                            winhSetMenuCondCascade(hwndSubMenu, sDefID);
                        }
                    } // end if (hwndSubmenu)
                } // end if (    (mi.afStyle & MIS_SUBMENU)
                else
                {
                    // no submenu:
                    // just copy that item
                    SHORT s;
                    mi.iPosition = sTargetPosition;
                    s = SHORT1FROMMR(WinSendMsg(hmenuTarget,
                                                MM_INSERTITEM,
                                                MPFROMP(&mi),
                                                MPFROMP(pszSource)));
                    if (    (s != MIT_MEMERROR)
                         && (s != MIT_ERROR)
                       )
                        brc = TRUE;
                }

                free(pszSource);

            } // end if (pszSource)
        } // end else if (mi.afStyle & MIS_SEPARATOR)
    } // end if (WinSendMsg(hmenuSource, MM_QUERYITEM,...

    return brc;
}

/*
 *@@ winhCopyMenuItem:
 *      wrapper for winhCopyMenuItem2 because it was
 *      exported.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

BOOL winhCopyMenuItem(HWND hmenuTarget,
                      HWND hmenuSource,
                      USHORT usID,
                      SHORT sTargetPosition)    // in: position to insert at or MIT_END
{
    return winhCopyMenuItem2(hmenuTarget, hmenuSource, usID, sTargetPosition, 0);
}

/*
 *@@ winhMergeIntoSubMenu:
 *      creates a new submenu in hmenuTarget with the
 *      specified title at the specified position
 *      and copies the entire contents of hmenuSource
 *      into that.
 *
 *      Returns the window handle of the new submenu
 *      or NULLHANDLE on errors.
 *
 *      NOTE: Copying submenus will work only if each item
 *      in the submenu has a unique menu ID. This is due
 *      to the dumb implementation of menus in PM where
 *      it is impossible to query menu items without
 *      knowing their ID.
 *
 *@@added V0.9.9 (2001-03-09) [umoeller]
 */

HWND winhMergeIntoSubMenu(HWND hmenuTarget,         // in: menu where to create submenu
                          SHORT sTargetPosition,    // in: position to insert at or MIT_END
                          const char *pcszTitle,    // in: title of new submenu or NULL
                          SHORT sID,                // in: ID of new submenu
                          HWND hmenuSource)         // in: menu to merge
{
    HWND    hwndNewSubmenu;
    if (hwndNewSubmenu = WinCreateMenu(hmenuTarget, NULL))
    {
        MENUITEM    mi = {0};
        SHORT       src = 0;
        // SHORT s = 0;
        mi.iPosition = MIT_END;
        mi.afStyle = MIS_TEXT | MIS_SUBMENU;
        mi.id = 2000;
        mi.hwndSubMenu = hwndNewSubmenu;

        WinSetWindowUShort(hwndNewSubmenu, QWS_ID, sID);

        // insert new submenu into hmenuTarget
        src = SHORT1FROMMR(WinSendMsg(hmenuTarget,
                                      MM_INSERTITEM,
                                      (MPARAM)&mi,
                                      (MPARAM)pcszTitle));
        if (    (src != MIT_MEMERROR)
            &&  (src != MIT_ERROR)
           )
        {
            int i;
            SHORT cMenuItems = SHORT1FROMMR(WinSendMsg(hmenuSource,
                                                       MM_QUERYITEMCOUNT,
                                                       0, 0));

            // loop through all entries in the original menu
            for (i = 0; i < cMenuItems; i++)
            {
                SHORT id = SHORT1FROMMR(WinSendMsg(hmenuSource,
                                                   MM_ITEMIDFROMPOSITION,
                                                   MPFROMSHORT(i),
                                                   0));
                winhCopyMenuItem(hwndNewSubmenu,
                                 hmenuSource,
                                 id,
                                 MIT_END);
            }
        }
        else
        {
            // error:
            WinDestroyWindow(hwndNewSubmenu);
            hwndNewSubmenu = NULLHANDLE;
        }
    }

    return hwndNewSubmenu;
}

/*
 *@@ winhMergeIntoSubMenu:
 *      copies all items from hmenuSource into hmenuTarget,
 *      starting at the given position.
 *
 *      Returns the no. of items that were copied.
 *
 *      NOTE: Copying submenus will work only if each item
 *      in the submenu has a unique menu ID. This is due
 *      to the dumb implementation of menus in PM where
 *      it is impossible to query menu items without
 *      knowing their ID.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

ULONG winhMergeMenus(HWND hmenuTarget,         // in: menu to copy items to
                     SHORT sTargetPosition,    // in: position to insert at or MIT_END
                     HWND hmenuSource,         // in: menu to merge
                     ULONG fl)                 // in: COPYFL_* flags for winhCopyMenuItem2
{
    SHORT   sTarget = MIT_END;

    int i;
    SHORT cMenuItems = SHORT1FROMMR(WinSendMsg(hmenuSource,
                                               MM_QUERYITEMCOUNT,
                                               0, 0));

    // loop through all entries in the original menu
    for (i = 0; i < cMenuItems; i++)
    {
        SHORT id = SHORT1FROMMR(WinSendMsg(hmenuSource,
                                           MM_ITEMIDFROMPOSITION,
                                           MPFROM2SHORT(i, 0),
                                           0));
        winhCopyMenuItem2(hmenuTarget,
                          hmenuSource,
                          id,
                          MIT_END,
                          fl);
    }

    return i;
}

/*
 *@@ winhClearMenu:
 *      removes all menu items from the given (sub)menu.
 *      The menu itself is not destroyed, but is empty
 *      after calling this function.
 *
 *@@added V1.0.0 (2002-08-31) [umoeller]
 */

ULONG winhClearMenu(HWND hwndMenu)
{
    ULONG   cDeleted = 0;
    SHORT   sID;

    // what we do is getting the menu item count
    // and then delete the first item in the menu
    // x times because there is no "delete menu item
    // from position" message, and there might be
    // duplicate IDs in the menu... this should
    // work always

    SHORT   cMenuItems = SHORT1FROMMR(WinSendMsg(hwndMenu,
                                                 MM_QUERYITEMCOUNT,
                                                 0,
                                                 0));
    while (cMenuItems-- > 0)
    {
        sID = SHORT1FROMMR(WinSendMsg(hwndMenu,
                                      MM_ITEMIDFROMPOSITION,
                                      MPFROMSHORT(0),
                                      MPNULL));

        WinSendMsg(hwndMenu,
                   MM_DELETEITEM,
                   MPFROM2SHORT(sID, FALSE),
                   0);

        ++cDeleted;
    }

    return cDeleted;
}

/*
 *@@ winhQueryMenuItemText:
 *      this returns a menu item text as a PSZ
 *      to a newly allocated buffer or NULL if
 *      not found.
 *
 *      Returns NULL on error. Use free()
 *      to free the return value.
 *
 *      This uses MM_QUERYITEMTEXT internally.
 *      PMREF doesn't say anything about this,
 *      but from my testing this always recurses
 *      into submenus.
 *
 *      Use the WinSetMenuItemText macro to
 *      set the menu item text.
 */

PSZ winhQueryMenuItemText(HWND hwndMenu,
                          USHORT usItemID)  // in: menu item ID (not index)
{
    PSZ     prc = NULL;

    SHORT   sLength;
    if (sLength = SHORT1FROMMR(WinSendMsg(hwndMenu,
                                          MM_QUERYITEMTEXTLENGTH,
                                          (MPARAM)(ULONG)usItemID,
                                          (MPARAM)NULL)))
    {
        prc = (PSZ)malloc(sLength + 1);
        WinSendMsg(hwndMenu,
                   MM_QUERYITEMTEXT,
                   MPFROM2SHORT(usItemID, sLength + 1),
                   (MPARAM)prc);
    }

    return prc;
}

/*
 *@@ winhAppend2MenuItemText:
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 */

BOOL winhAppend2MenuItemText(HWND hwndMenu,
                             USHORT usItemID,  // in: menu item ID (not index)
                             const char *pcszAppend, // in: text to append
                             BOOL fTab)    // in: if TRUE, add \t before pcszAppend
{
    BOOL brc = FALSE;
    CHAR szItemText[400];
    if (WinSendMsg(hwndMenu,
                   MM_QUERYITEMTEXT,
                   MPFROM2SHORT(usItemID,
                                sizeof(szItemText)),
                   (MPARAM)szItemText))
    {
        // text copied:
        if (fTab)
        {
            if (strchr(szItemText, '\t'))
                // we already have a tab:
                strcat(szItemText, " ");
            else
                strcat(szItemText, "\t");
        }
        strcat(szItemText, pcszAppend);

        brc = (BOOL)WinSendMsg(hwndMenu,
                               MM_SETITEMTEXT,
                               MPFROMSHORT(usItemID),
                               (MPARAM)szItemText);
    }

    return brc;
}

/*
 *@@ winhMenuRemoveEllipse:
 *      removes a "..." substring from a menu item
 *      title, if found. This is useful if confirmations
 *      have been turned off for a certain menu item, which
 *      should be reflected in the menu.
 */

VOID winhMenuRemoveEllipse(HWND hwndMenu,
                           USHORT usItemId)    // in:  item to remove "..." from
{
    CHAR szBuf[255];
    CHAR *p;
    WinSendMsg(hwndMenu,
               MM_QUERYITEMTEXT,
               MPFROM2SHORT(usItemId, sizeof(szBuf)-1),
               (MPARAM)&szBuf);
    if ((p = strstr(szBuf, "...")))
        strcpy(p, p+3);
    WinSendMsg(hwndMenu,
               MM_SETITEMTEXT,
               MPFROMSHORT(usItemId),
               (MPARAM)&szBuf);
}

/*
 *@@ winhQueryItemUnderMouse:
 *      this queries the menu item which corresponds
 *      to the given mouse coordinates.
 *      Returns the ID of the menu item and stores its
 *      rectangle in *prtlItem; returns (-1) upon errors.
 */

SHORT winhQueryItemUnderMouse(HWND hwndMenu,      // in: menu handle
                              POINTL *pptlMouse,  // in: mouse coordinates
                              RECTL *prtlItem)    // out: rectangle of menu item
{
    SHORT   s, sItemId, sItemCount;
    HAB     habDesktop = WinQueryAnchorBlock(HWND_DESKTOP);

    sItemCount = SHORT1FROMMR(WinSendMsg(hwndMenu, MM_QUERYITEMCOUNT, MPNULL, MPNULL));

    for (s = 0;
         s <= sItemCount;
         s++)
    {
        sItemId = SHORT1FROMMR(WinSendMsg(hwndMenu,
                                          MM_ITEMIDFROMPOSITION,
                                          (MPARAM)(ULONG)s, MPNULL));
        WinSendMsg(hwndMenu,
                   MM_QUERYITEMRECT,
                   MPFROM2SHORT(sItemId, FALSE),
                   (MPARAM)prtlItem);
        if (WinPtInRect(habDesktop, prtlItem, pptlMouse))
            return sItemId;
    }
    /* sItemId = (SHORT)WinSendMsg(hwndMenu, MM_ITEMIDFROMPOSITION, (MPARAM)(sItemCount-1), MPNULL);
    return (sItemId); */

    return -1; // error: no valid menu item
}

/*
 *@@category: Helpers\PM helpers\Slider helpers
 */

/* ******************************************************************
 *
 *   Slider helpers
 *
 ********************************************************************/

/*
 *@@ winhReplaceWithLinearSlider:
 *      this destroys the control with the ID ulID in hwndDlg
 *      and creates a linear slider at the same position with the
 *      same ID (effectively replacing it).
 *
 *      This is needed because the IBM dialog editor (DLGEDIT.EXE)
 *      keeps crashing when creating sliders. So the way to do
 *      this easily is to create some other control with DLGEDIT
 *      where the slider should be later and call this function
 *      on that control when the dialog is initialized.
 *
 *      You need to specify _one_ of the following with ulSliderStyle:
 *      -- SLS_HORIZONTAL: horizontal slider (default)
 *      -- SLS_VERTICAL: vertical slider
 *
 *      plus _one_ additional common slider style for positioning:
 *      -- for horizontal sliders: SLS_BOTTOM, SLS_CENTER, or SLS_TOP
 *      -- for vertical sliders: SLS_LEFT, SLS_CENTER, or SLS_RIGHT
 *
 *      Additional common slider styles are:
 *      -- SLS_PRIMARYSCALE1: determines the location of the scale
 *                  on the slider shaft by using increment
 *                  and spacing specified for scale 1 as
 *                  the incremental value for positioning
 *                  the slider arm. Scale 1 is displayed
 *                  above the slider shaft of a horizontal
 *                  slider and to the right of the slider
 *                  shaft of a vertical slider. This is
 *                  the default for a slider.
 *      -- SLS_PRIMARYSCALE2: not supported by this function
 *      -- SLS_READONLY: creates a read-only slider, which
 *                  presents information to the user but
 *                  allows no interaction with the user.
 *      -- SLS_RIBBONSTRIP: fills, as the slider arm moves, the
 *                  slider shaft between the home position
 *                  and the slider arm with a color value
 *                  different from slider shaft color,
 *                  similar to mercury in a thermometer.
 *      -- SLS_OWNERDRAW: notifies the application whenever the
 *                  slider shaft, the ribbon strip, the
 *                  slider arm, and the slider background
 *                  are to be drawn.
 *      -- SLS_SNAPTOINCREMENT: causes the slider arm, when positioned
 *                  between two values, to be positioned
 *                  to the nearest value and redrawn at
 *                  that position.
 *
 *      Additionally, for horizontal sliders:
 *      -- SLS_BUTTONSLEFT: specifies that the optional slider
 *                  buttons are to be used and places them
 *                  to the left of the slider shaft. The
 *                  buttons move the slider arm by one
 *                  position, left or right, in the
 *                  direction selected.
 *      -- SLS_BUTTONSRIGHT: specifies that the optional slider
 *                  buttons are to be used and places them
 *                  to the right of the slider shaft. The
 *                  buttons move the slider arm by one
 *                  position, left or right, in the
 *                  direction selected.
 *      -- SLS_HOMELEFT: specifies the slider arm's home
 *                  position. The left edge is used as the
 *                  base value for incrementing (default).
 *      -- SLS_HOMERIGHT: specifies the slider arm's home
 *                  position. The right edge is used as
 *                  the base value for incrementing.
 *
 *      Instead, for vertical sliders:
 *      -- SLS_BUTTONSBOTTOM: specifies that the optional slider
 *                  buttons are to be used and places them
 *                  at the bottom of the slider shaft. The
 *                  buttons move the slider arm by one
 *                  position, up or down, in the direction
 *                  selected.
 *      -- SLS_BUTTONSTOP: specifies that the optional slider
 *                  buttons are to be used and places them
 *                  at the top of the slider shaft. The
 *                  buttons move the slider arm by one
 *                  position, up or down, in the direction
 *                  selected.
 *      -- SLS_HOMEBOTTOM: specifies the slider arm's home
 *                  position. The bottom of the slider is
 *                  used as the base value for
 *                  incrementing.
 *      -- SLS_HOMETOP: specifies the slider arm's home
 *                  position. The top of the slider is
 *                  used as the base value for
 *                  incrementing.
 *
 *      Notes: This function automatically adds WS_PARENTCLIP,
 *      WS_TABSTOP, and WS_SYNCPAINT to the specified styles.
 *      For the WS_TABSTOP style, hwndInsertAfter is important.
 *      If you specify HWND_TOP, your window will be the first
 *      in the tab stop list.
 *
 *      It also shows the slider after having done all the
 *      processing in here by calling WinShowWindow.
 *
 *      Also, we only provide support for scale 1 here, so
 *      do not specify SLS_PRIMARYSCALE2 with ulSliderStyle,
 *      and we have the slider calculate all the spacings.
 *
 *      This returns the HWND of the slider or NULLHANDLE upon
 *      errors.
 *
 *@@added V0.9.0 [umoeller]
 */

HWND winhReplaceWithLinearSlider(HWND hwndParent,   // in: parent of old control and slider
                                 HWND hwndOwner,          // in: owner of old control and slider
                                 HWND hwndInsertAfter,    // in: the control after which the slider should
                                                          // come up, or HWND_TOP, or HWND_BOTTOM
                                 ULONG ulID,              // in: ID of old control and slider
                                 ULONG ulSliderStyle,     // in: SLS_* styles
                                 ULONG ulTickCount)       // in: number of ticks (scale 1)
{
    HWND    hwndSlider = NULLHANDLE;
    HWND    hwndKill = WinWindowFromID(hwndParent, ulID);
    if (hwndKill)
    {
        SWP swpControl;
        if (WinQueryWindowPos(hwndKill, &swpControl))
        {
            SLDCDATA slcd;

            // destroy the old control
            WinDestroyWindow(hwndKill);

            // initialize slider control data
            slcd.cbSize = sizeof(SLDCDATA);
            slcd.usScale1Increments = ulTickCount;
            slcd.usScale1Spacing = 0;           // have slider calculate it
            slcd.usScale2Increments = 0;
            slcd.usScale2Spacing = 0;

            // create a slider with the same ID at the same
            // position
            hwndSlider = WinCreateWindow(hwndParent,
                                         WC_SLIDER,
                                         NULL,           // no window text
                                         ulSliderStyle
                                            | WS_PARENTCLIP
                                            | WS_SYNCPAINT
                                            | WS_TABSTOP,
                                         swpControl.x,
                                         swpControl.y,
                                         swpControl.cx,
                                         swpControl.cy,
                                         hwndOwner,
                                         hwndInsertAfter,
                                         ulID,           // same ID as destroyed control
                                         &slcd,          // slider control data
                                         NULL);          // presparams

            WinSendMsg(hwndSlider,
                       SLM_SETTICKSIZE,
                       MPFROM2SHORT(SMA_SETALLTICKS,
                                    6),     // 15 pixels high
                       NULL);

            WinShowWindow(hwndSlider, TRUE);
        }
    }

    return hwndSlider;
}

/*
 *@@ winhSetSliderTicks:
 *      this adds ticks to the given linear slider,
 *      which are ulPixels pixels high. A useful
 *      value for this is 4.
 *
 *      This queries the slider for the primary
 *      scale values. Only the primary scale is
 *      supported.
 *
 *      This function goes sets the ticks twice,
 *      once with mpEveryOther1 and ulPixels1,
 *      and then a second time with mpEveryOther2
 *      and ulPixels2. This allows you to quickly
 *      give, say, every tenth item a taller tick.
 *
 *      For every set, if mpEveryOther is 0, this sets
 *      all ticks on the primary slider scale.
 *
 *      If mpEveryOther is != 0, SHORT1FROMMP
 *      specifies the first tick to set, and
 *      SHORT2FROMMP specifies every other tick
 *      to set from there. For example:
 *
 +          MPFROM2SHORT(9, 10)
 *
 *      would set tick 9, 19, 29, and so forth.
 *
 *      If both mpEveryOther and ulPixels are -1,
 *      that set is skipped.
 *
 *      Example: Considering a slider with a
 *      primary scale from 0 to 30, using
 *
 +          winhSetSliderTicks(hwndSlider,
 +                             0,                       // every tick
 +                             3,                       //      to three pixels
 +                             MPFROM2SHORT(9, 10)      // then every tenth
 +                             6);                      //      to six pixels.
 *
 *      Returns FALSE upon errors.
 *
 *@@added V0.9.1 (99-12-04) [umoeller]
 *@@changed V0.9.7 (2001-01-18) [umoeller]: added second set
 */

BOOL winhSetSliderTicks(HWND hwndSlider,            // in: linear slider
                        MPARAM mpEveryOther1,       // in: set 1
                        ULONG ulPixels1,
                        MPARAM mpEveryOther2,       // in: set 2
                        ULONG ulPixels2)
{
    BOOL brc = FALSE;

    ULONG ulSet;
    MPARAM mpEveryOther = mpEveryOther1;
    ULONG ulPixels = ulPixels1;

    // do this twice
    for (ulSet = 0;
         ulSet < 2;
         ulSet++)
    {
        if (mpEveryOther == 0)
        {
            // set all ticks:
            brc = (BOOL)WinSendMsg(hwndSlider,
                                   SLM_SETTICKSIZE,
                                   MPFROM2SHORT(SMA_SETALLTICKS,
                                                ulPixels),
                                   NULL);
        }
        else if ( (mpEveryOther != (MPARAM)-1) && (ulPixels != -1) )
        {
            SLDCDATA  slcd;
            WNDPARAMS   wp;
            memset(&wp, 0, sizeof(WNDPARAMS));
            wp.fsStatus = WPM_CTLDATA;
            wp.cbCtlData = sizeof(slcd);
            wp.pCtlData = &slcd;
            // get primary scale data from the slider
            if (WinSendMsg(hwndSlider,
                           WM_QUERYWINDOWPARAMS,
                           (MPARAM)&wp,
                           0))
            {
                USHORT usStart = SHORT1FROMMP(mpEveryOther),
                       usEveryOther = SHORT2FROMMP(mpEveryOther);

                USHORT usScale1Max = slcd.usScale1Increments,
                       us;

                brc = TRUE;

                for (us = usStart; us < usScale1Max; us += usEveryOther)
                {
                    if (!(BOOL)WinSendMsg(hwndSlider,
                                          SLM_SETTICKSIZE,
                                          MPFROM2SHORT(us,
                                                       ulPixels),
                                          NULL))
                    {
                        brc = FALSE;
                        break;
                    }
                }
            }
        }

        // for the second loop, use second value set
        mpEveryOther = mpEveryOther2;
        ulPixels = ulPixels2;
                // we only loop twice
    } // end for (ulSet = 0; ulSet < 2;

    return brc;
}

/*
 *@@ winhReplaceWithCircularSlider:
 *      this destroys the control with the ID ulID in hwndDlg
 *      and creates a linear slider at the same position with the
 *      same ID (effectively replacing it).
 *
 *      This is needed because the IBM dialog editor (DLGEDIT.EXE)
 *      cannot create circular sliders. So the way to do this
 *      easily is to create some other control with DLGEDIT
 *      where the slider should be later and call this function
 *      on that control when the dialog is initialized.
 *
 *      You need to specify the following with ulSliderStyle:
 *      --  CSS_CIRCULARVALUE: draws a circular thumb, rather than a line,
 *                  for the value indicator.
 *      --  CSS_MIDPOINT: makes the mid-point tick mark larger.
 *      --  CSS_NOBUTTON: does not display value buttons. Per default, the
 *                  slider displays "-" and "+" buttons to the bottom left
 *                  and bottom right of the knob. (BTW, these bitmaps can be
 *                  changed using CSM_SETBITMAPDATA.)
 *      --  CSS_NONUMBER: does not display the value on the dial.
 *      --  CSS_NOTEXT: does not display title text under the dial.
 *                  Otherwise, the text in the pszTitle parameter
 *                  will be used.
 *      --  CSS_NOTICKS (only listed in pmstddlg.h, not in PMREF):
 *                  obviously, this prevents tick marks from being drawn.
 *      --  CSS_POINTSELECT: permits the values on the circular slider
 *                  to change immediately when dragged.
 *                  Direct manipulation is performed by using a mouse to
 *                  click on and drag the circular slider. There are two
 *                  modes of direct manipulation for the circular slider:
 *                  <BR><B>1)</B> The default direct manipulation mode is to scroll to
 *                  the value indicated by the position of the mouse.
 *                  This could be important if you used a circular slider
 *                  for a volume control, for example. Increasing the volume
 *                  from 0% to 100% too quickly could result in damage to
 *                  both the user's ears and the equipment.
 *                  <BR><B>2)</B>The other mode of direct manipulation permits
 *                  the value on the circular slider to change immediately when dragged.
 *                  This mode is enabled using the CSS_POINTSELECT style bit. When this
 *                  style is used, the value of the dial can be changed by tracking
 *                  the value with the mouse, which changes values quickly.
 *      --  CSS_PROPORTIONALTICKS: allow the length of the tick marks to be calculated
 *                  as a percentage of the radius (for small sliders).
 *      --  CSS_360: permits the scroll range to extend 360 degrees.
 *                  CSS_360 forces the CSS_NONUMBER style on. This is necessary
 *                  to keep the value indicator from corrupting the number value.
 *
 *      FYI: The most commonly known circular slider in OS/2, the one in the
 *      default "Sound" object, has a style of 0x9002018a, meaning
 *      CSS_NOTEXT | CSS_POINTSELECT | CSS_NOTICKS.
 *
 *      Notes: This function automatically adds WS_PARENTCLIP,
 *      WS_TABSTOP, and WS_SYNCPAINT to the specified styles.
 *      For the WS_TABSTOP style, hwndInsertAfter is important.
 *      If you specify HWND_TOP, your window will be the first
 *      in the tab stop list.
 *
 *      It also shows the slider after having done all the
 *      processing in here by calling WinShowWindow.
 *
 *      This returns the HWND of the slider or NULLHANDLE upon
 *      errors.
 *
 *@@added V0.9.0 [umoeller]
 */

HWND winhReplaceWithCircularSlider(HWND hwndParent,   // in: parent of old control and slider
                                   HWND hwndOwner,          // in: owner of old control and slider
                                   HWND hwndInsertAfter,    // in: the control after which the slider should
                                                            // come up, or HWND_TOP, or HWND_BOTTOM
                                   ULONG ulID,              // in: ID of old control and slider
                                   ULONG ulSliderStyle,     // in: SLS_* styles
                                   SHORT sMin,              // in: minimum value (e.g. 0)
                                   SHORT sMax,              // in: maximum value (e.g. 100)
                                   USHORT usIncrement,      // in: minimum increment (e.g. 1)
                                   USHORT usTicksEvery)     // in: ticks ever x values (e.g. 20)
{
    HWND    hwndSlider = NULLHANDLE;
    HWND    hwndKill = WinWindowFromID(hwndParent, ulID);
    if (hwndKill)
    {
        SWP swpControl;
        if (WinQueryWindowPos(hwndKill, &swpControl))
        {
            // destroy the old control
            WinDestroyWindow(hwndKill);

            // WinRegisterCircularSlider();

            // create a slider with the same ID at the same
            // position
            hwndSlider = WinCreateWindow(hwndParent,
                                         WC_CIRCULARSLIDER,
                                         "dummy",        // no window text
                                         ulSliderStyle
                                            // | WS_PARENTCLIP
                                            // | WS_SYNCPAINT
                                            | WS_TABSTOP,
                                         swpControl.x,
                                         swpControl.y,
                                         swpControl.cx,
                                         swpControl.cy,
                                         hwndOwner,
                                         hwndInsertAfter,
                                         ulID,           // same ID as destroyed control
                                         NULL,           // control data
                                         NULL);          // presparams

            if (hwndSlider)
            {
                // set slider range
                WinSendMsg(hwndSlider,
                           CSM_SETRANGE,
                           (MPARAM)(ULONG)sMin,
                           (MPARAM)(ULONG)sMax);

                // set slider increments
                WinSendMsg(hwndSlider,
                           CSM_SETINCREMENT,
                           (MPARAM)(ULONG)usIncrement,
                           (MPARAM)(ULONG)usTicksEvery);

                // set slider value
                WinSendMsg(hwndSlider,
                           CSM_SETVALUE,
                           (MPARAM)0,
                           (MPARAM)0);

                // for some reason, the slider always has
                // WS_CLIPSIBLINGS set, even though we don't
                // set this; we must unset this now, or
                // the slider won't draw itself (%&$&%"$&%!!!)
                WinSetWindowBits(hwndSlider,
                                 QWL_STYLE,
                                 0,         // unset bit
                                 WS_CLIPSIBLINGS);

                WinShowWindow(hwndSlider, TRUE);
            }
        }
    }

    return hwndSlider;
}

/*
 *@@category: Helpers\PM helpers\Spin button helpers
 */

/* ******************************************************************
 *
 *   Spin button helpers
 *
 ********************************************************************/

/*
 *@@ winhSetDlgItemSpinData:
 *      sets a spin button's limits and data within a dialog window.
 *      This only works for decimal spin buttons.
 */

VOID winhSetDlgItemSpinData(HWND hwndDlg,       // in: dlg window
                            ULONG idSpinButton,  // in: item ID of spin button
                            ULONG min,           // in: minimum allowed value
                            ULONG max,           // in: maximum allowed value
                            ULONG current)       // in: new current value
{
    HWND hwndSpinButton = WinWindowFromID(hwndDlg, idSpinButton);
    if (hwndSpinButton)
    {
        WinSendMsg(hwndSpinButton,
                   SPBM_SETLIMITS,          // Set limits message
                   (MPARAM)max,             // Spin Button maximum setting
                   (MPARAM)min);             // Spin Button minimum setting

        WinSendMsg(hwndSpinButton,
                   SPBM_SETCURRENTVALUE,    // Set current value message
                   (MPARAM)current,
                   (MPARAM)NULL);
    }
}

/*
 *@@ winhAdjustDlgItemSpinData:
 *      this can be called on a spin button control to
 *      have its current data snap to a grid. This only
 *      works for LONG integer values.
 *
 *      For example, if you specify 100 for the grid and call
 *      this func after you have received SPBN_UP/DOWNARROW,
 *      the spin button's value will always in/decrease
 *      so that the spin button's value is a multiple of 100.
 *
 *      By contrast, if (lGrid < 0), this will not really
 *      snap the value to a multiple of -lGrid, but instead
 *      in/decrease the value by -lGrid. The value will not
 *      necessarily be a multiple of the grid. (0.9.14)
 *
 *      This returns the "snapped" value to which the spin
 *      button was set.
 *
 *      If you specify lGrid == 0, this returns the spin
 *      button's value only without snapping (V0.9.0).
 *
 *@@changed V0.9.0 [umoeller]: added check for lGrid == 0 (caused division by zero previously)
 *@@changed V0.9.14 (2001-08-03) [umoeller]: added fixes for age-old problems with wrap around
 *@@changed V0.9.14 (2001-08-03) [umoeller]: added lGrid < 0 mode
 */

LONG winhAdjustDlgItemSpinData(HWND hwndDlg,     // in: dlg window
                               USHORT usItemID,  // in: item ID of spin button
                               LONG lGrid,       // in: grid
                               USHORT usNotifyCode) // in: SPBN_UP* or *DOWNARROW of WM_CONTROL message
{
    HWND hwndSpin = WinWindowFromID(hwndDlg, usItemID);
    LONG lBottom, lTop, lValue;

    // get value, which has already increased /
    // decreased by 1
    WinSendMsg(hwndSpin,
               SPBM_QUERYVALUE,
               (MPARAM)&lValue,
               MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));

    if ((lGrid)
        && (    (usNotifyCode == SPBN_UPARROW)
             || (usNotifyCode == SPBN_DOWNARROW)
           )
       )
    {
        // only if the up/down buttons were pressed,
        // snap to the nearest grid; if the user
        // manually enters something (SPBN_CHANGE),
        // we'll accept that value
        LONG lChanged = (usNotifyCode == SPBN_UPARROW)
                            // if the spin button went up, subtract 1
                            ? -1
                            : +1;
        LONG lPrev  = lValue + lChanged;

        // if grid is negative, it is assumed to
        // not be a "real" grid but jump in those
        // steps only
        if (lGrid < 0)
        {
            // add /subtract grid
            if (usNotifyCode == SPBN_UPARROW)
                lValue = lPrev - lGrid;
            else
                lValue = lPrev + lGrid;

            // lValue = (lValue / lGrid) * lGrid;
        }
        else
        {
            // add /subtract grid
            if (usNotifyCode == SPBN_UPARROW)
                lValue = lPrev + lGrid;
            else
                lValue = lPrev - lGrid;

            lValue = (lValue / lGrid) * lGrid;
        }

        // balance with spin button limits
        WinSendMsg(hwndSpin,
                   SPBM_QUERYLIMITS,
                   (MPARAM)&lTop,
                   (MPARAM)&lBottom);
        if (lValue < lBottom)
            lValue = lTop;
        else if (lValue > lTop)
            lValue = lBottom;

        WinSendMsg(hwndSpin,
                   SPBM_SETCURRENTVALUE,
                   (MPARAM)(lValue),
                   MPNULL);
    }
    return lValue;
}

/*
 *@@category: Helpers\PM helpers\List box helpers
 */

/* ******************************************************************
 *
 *   List box helpers
 *
 ********************************************************************/

/*
 *@@ winhQueryLboxItemText:
 *      returns the text of the specified
 *      list box item in a newly allocated
 *      buffer.
 *
 *      Returns NULL on error. Use fre()
 *      to free the return value.
 *
 *@@added V0.9.1 (99-12-14) [umoeller]
 */

PSZ winhQueryLboxItemText(HWND hwndListbox,
                          SHORT sIndex)
{
    PSZ   pszReturn = 0;
    SHORT cbText = SHORT1FROMMR(WinSendMsg(hwndListbox,
                                           LM_QUERYITEMTEXTLENGTH,
                                           (MPARAM)(ULONG)sIndex,
                                           0));
    if ((cbText) && (cbText != LIT_ERROR))
    {
        pszReturn = (PSZ)malloc(cbText + 1);        // add zero terminator
        WinSendMsg(hwndListbox,
                   LM_QUERYITEMTEXT,
                   MPFROM2SHORT(sIndex,
                                cbText + 1),
                   (MPARAM)pszReturn);
    }

    return pszReturn;
}

/*
 *@@ winhMoveLboxItem:
 *      this moves one list box item from one
 *      list box to another, including the
 *      item text and the item "handle"
 *      (see LM_QUERYITEMHANDLE).
 *
 *      sTargetIndex can either be a regular
 *      item index or one of the following
 *      (as in LM_INSERTITEM):
 *      -- LIT_END
 *      -- LIT_SORTASCENDING
 *      -- LIT_SORTDESCENDING
 *
 *      If (fSelectTarget == TRUE), the new
 *      item is also selected in the target
 *      list box.
 *
 *      Returns FALSE if moving failed. In
 *      that case, the list boxes are unchanged.
 *
 *@@added V0.9.1 (99-12-14) [umoeller]
 */

BOOL winhMoveLboxItem(HWND hwndSource,
                      SHORT sSourceIndex,
                      HWND hwndTarget,
                      SHORT sTargetIndex,
                      BOOL fSelectTarget)
{
    BOOL brc = FALSE;

    PSZ pszItemText = winhQueryLboxItemText(hwndSource, sSourceIndex);
    if (pszItemText)
    {
        ULONG   ulItemHandle = winhQueryLboxItemHandle(hwndSource,
                                                       sSourceIndex);
                    // probably 0, if not used
        LONG lTargetIndex = WinInsertLboxItem(hwndTarget,
                                              sTargetIndex,
                                              pszItemText);
        if (    (lTargetIndex != LIT_ERROR)
             && (lTargetIndex != LIT_MEMERROR)
           )
        {
            // successfully inserted:
            winhSetLboxItemHandle(hwndTarget, lTargetIndex, ulItemHandle);
            if (fSelectTarget)
                winhSetLboxSelectedItem(hwndTarget, lTargetIndex, TRUE);

            // remove source
            WinDeleteLboxItem(hwndSource,
                              sSourceIndex);

            brc = TRUE;
        }

        free(pszItemText);
    }

    return brc;
}

/*
 *@@ winhLboxSelectAll:
 *      this selects or deselects all items in the
 *      given list box, depending on fSelect.
 *
 *      Returns the number of items in the list box.
 */

ULONG winhLboxSelectAll(HWND hwndListBox,   // in: list box
                        BOOL fSelect)       // in: TRUE = select, FALSE = deselect
{
    LONG lItemCount = WinQueryLboxCount(hwndListBox);
    ULONG ul;

    for (ul = 0; ul < lItemCount; ul++)
    {
        WinSendMsg(hwndListBox,
                   LM_SELECTITEM,
                   (MPARAM)ul,      // index
                   (MPARAM)fSelect);
    }

    return lItemCount;
}

/*
 *@@ winhLboxFindItemFromHandle:
 *      finds the list box item with the specified
 *      handle.
 *
 *      Of course this only makes sense if each item
 *      has a unique handle indeed.
 *
 *      Returns the index of the item found or -1.
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 */

ULONG winhLboxFindItemFromHandle(HWND hwndListBox,
                                 ULONG ulHandle)
{
    LONG cItems;
    if (cItems = WinQueryLboxCount(hwndListBox))
    {
        ULONG ul;
        for (ul = 0;
             ul < cItems;
             ul++)
        {
            if (ulHandle == winhQueryLboxItemHandle(hwndListBox,
                                                    ul))
                return ul;
        }
    }

    return -1;
}

/*
 *@@category: Helpers\PM helpers\Scroll bar helpers
 */

/* ******************************************************************
 *
 *   Scroll bar helpers
 *
 ********************************************************************/

/*
 *@@ winhUpdateScrollBar:
 *      updates the given scroll bar according to the given
 *      values. This updates the scroll bar's thumb size,
 *      extension, and position, all in one shot.
 *
 *      This function usually gets called when the window is
 *      created and later when the window is resized.
 *
 *      This simplifies the typical functionality of a scroll
 *      bar in a client window which is to be scrolled. I am
 *      wondering why IBM never included such a function, since
 *      it is so damn basic and still writing it cost me a whole
 *      day.
 *
 *      Terminology:
 *
 *      --  "window": the actual window with scroll bars which displays
 *          a subrectangle of the available data. With a typical PM
 *          application, this will be your client window.
 *
 *          The width or height of this must be passed in ulWinPels.
 *
 *      --  "workarea": the entire data to be displayed, of which the
 *          "window" can only display a subrectangle, if the workarea
 *          is larger than the window.
 *
 *          The width or height of this must be passed in ulWorkareaPels.
 *          This can be smaller than ulWinPels (if the window is larger
 *          than the data) or the same or larger than ulWinPels
 *          (if the window is too small to show all the data).
 *
 *          This value is exclusive in the sense that the maximum
 *          window offset (below) can be the workarea minus one.
 *
 *      --  "window offset": the zero-based offset of the current
 *          window within the workarea, whose maximum value is
 *          the workarea minus one.
 *
 *          [pr]: I disagree with the above "workarea minus one" stuff.
 *                It is perfectly possible to have the workarea and the
 *                window area the same size and to display all the content.
 *                This also means you do NOT get a disabled scroll bar
 *                any more when they are the same size.
 *
 *          For horizontal scroll bars, this is the X coordinate,
 *          counting from the left of the window (0 means leftmost).
 *
 *          For vertical scroll bars, this is counted from the _top_
 *          of the workarea (0 means topmost, as opposed to OS/2
 *          window coordinates!). This is because for vertical scroll
 *          bars controls, higher values move the thumb _down_. Yes
 *          indeed, this conflicts with PM's coordinate system.
 *
 *          The window offset is therefore always positive.
 *
 *      The scroll bar gets disabled if the entire workarea is visible,
 *      that is, if ulWorkareaPels <= ulWinPels. In that case
 *      FALSE is returned. If (fAutoHide == TRUE), the scroll
 *      bar is not only disabled, but also hidden from the display.
 *      In that case, you will need to reformat your output because
 *      your workarea becomes larger without the scroll bar.
 *
 *      This function will set the range of the scroll bar to 0 up
 *      to a value depending on the workarea size. For vertical scroll
 *      bars, 0 means topmost (which is kinda sick with the OS/2
 *      coordinate system), for horizontal scroll bars, 0 means leftmost.
 *
 *      The maximum value of the scroll bar will be
 *
 +          (ulWorkareaPels - ulWinPels) / usScrollUnitPels
 *
 *      The thumb size of the scroll bar will also be adjusted
 *      based on the workarea and window size, as it should be.
 *
 *@@added V0.9.1 (2000-02-14) [umoeller]
 *@@changed V0.9.3 (2000-04-30) [umoeller]: fixed pels/unit confusion
 *@@changed V0.9.3 (2000-05-08) [umoeller]: now handling scroll units automatically
 *@@changed V1.0.1 (2003-01-25) [umoeller]: fixed max value which caused right/bottommost scroll button to never be disabled
 *@@changed V1.0.1 (2003-01-25) [umoeller]: fixed bad thumb position for large offsets
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: fix rounding errors @@fixes 1086
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: disable scroll bars when workarea = win area @@fixes 1086
 */

BOOL winhUpdateScrollBar(HWND hwndScrollBar,    // in: scroll bar (vertical or horizontal)
                         ULONG ulWinPels,       // in: vertical or horizontal dimension of
                                                // visible window part (in pixels),
                                                // excluding the scroll bar!
                         ULONG ulWorkareaPels,  // in: dimension of total data part, of
                                                // which ulWinPels is a sub-dimension
                                                // (in pixels);
                                                // if <= ulWinPels, the scrollbar will be
                                                // disabled
                         ULONG ulCurPelsOfs,    // in: current offset of visible part
                                                // (in pixels)
                         BOOL fAutoHide)        // in: hide scroll bar if disabled
{
    BOOL brc = FALSE;

    if (ulWorkareaPels > ulWinPels) // WarpIN V1.0.18
    {
        // for large workareas, adjust scroll bar units
        USHORT  usDivisor = 1, usRounder;
        USHORT  lMaxAllowedUnitOfs;

        if (ulWorkareaPels > 10000)
            usDivisor = 100;

        usRounder = usDivisor - 1;  // WarpIN V1.0.18
        // scrollbar needed:

        // workarea is larger than window:
        WinEnableWindow(hwndScrollBar, TRUE);
        if (fAutoHide)
            WinShowWindow(hwndScrollBar, TRUE);

        // calculate limit
        lMaxAllowedUnitOfs =   (ulWorkareaPels - ulWinPels + usRounder)
                             / usDivisor;

        // set thumb position and limit
        WinSendMsg(hwndScrollBar,
                   SBM_SETSCROLLBAR,
                   (MPARAM)((ulCurPelsOfs + usRounder) / usDivisor),   // position: 0 means top
                   MPFROM2SHORT(0,  // minimum
                                lMaxAllowedUnitOfs));    // maximum

        // set thumb size based on ulWinPels and
        // ulWorkareaPels
        WinSendMsg(hwndScrollBar,
                   SBM_SETTHUMBSIZE,
                   MPFROM2SHORT(    (ulWinPels + usRounder) / usDivisor,       // visible
                                    (ulWorkareaPels + usRounder) / usDivisor), // total
                   0);
        brc = TRUE;
    }
    else
    {
        // entire workarea is visible:
        WinEnableWindow(hwndScrollBar, FALSE);
        if (fAutoHide)
            WinShowWindow(hwndScrollBar, FALSE);
    }

    // _Pmpf(("End of winhUpdateScrollBar"));

    return brc;
}

/*
 *@@ winhHandleScrollMsg:
 *      this helper handles a WM_VSCROLL or WM_HSCROLL message
 *      posted to a client window when the user has worked on a
 *      client scroll bar. Calling this function is all you need to
 *      do to handle those two messages.
 *
 *      This is most useful in conjunction with winhUpdateScrollBar.
 *      See that function for the terminology also.
 *
 *      This function calculates the new scrollbar position (from
 *      the mp2 value, which can be line up/down, page up/down, or
 *      slider track) and calculates the scrolling offset
 *      accordingly, which is returned as a LONG.
 *
 *      This function assumes that the scrollbar operates
 *      on values starting from zero. The maximum value
 *      of the scroll bar is therefore
 *
 +          lWorkareaPels - (prcl2Scroll->yTop - prcl2Scroll->yBottom)
 *
 *      This function also automatically shrinks the scroll bar
 *      units, should you have a workarea size which doesn't fit
 *      into the SHORT's that the scroll bar uses internally. As
 *      a result, this function handles a the complete range of
 *      a LONG for the workarea. (The PM scroll bar implementation
 *      is really brain-dead in this respect... the workarea can
 *      easily be larger than 32768 pixels. That's what scroll bars
 *      are for in the first place, dammit.)
 *
 *      Replace "bottom" and "top" with "right" and "left" for
 *      horizontal scrollbars in the above formula.
 *
 *      Returns the amount of pixels to be passed to winhScrollWindow
 *      afterwards.
 *
 *@@added V0.9.1 (2000-02-13) [umoeller]
 *@@changed V0.9.3 (2000-04-30) [umoeller]: changed prototype, fixed pels/unit confusion
 *@@changed V0.9.3 (2000-05-08) [umoeller]: now handling scroll units automatically
 *@@changed V0.9.7 (2001-01-17) [umoeller]: changed PLONG to PULONG
 *@@changed V1.0.1 (2003-01-25) [umoeller]: changed prototype, no longer calling WinScrollWindow, fixed offset bugs
 *@@changed WarpIN V1.0.18 (2008-11-16) [pr]: fix rounding error @@fixes 1086
 */

LONG winhHandleScrollMsg(HWND hwndScrollBar,        // in: vertical or horizontal scroll bar window
                         PLONG plCurPelsOfs,        // in/out: current workarea offset (in window coordinates)
                         LONG lWindowPels,          // in: window cx or cy (in window coordinates)
                         LONG lWorkareaPels,        // in: total workarea dimension,
                                                    // into which *plCurPelsOfs is an offset
                         USHORT usLineStepPels,     // in: pixels to scroll line-wise
                                                    // (scroll bar buttons pressed)
                         ULONG msg,                 // in: either WM_VSCROLL or WM_HSCROLL
                         MPARAM mp2)                // in: complete mp2 of WM_VSCROLL/WM_HSCROLL;
                                                    // this has two SHORT's (usPos and usCmd),
                                                    // see PMREF for details
{
    LONG    lOldPelsOfs = *plCurPelsOfs;
    USHORT  usPosUnits = SHORT1FROMMP(mp2), // in scroll units
            usCmd = SHORT2FROMMP(mp2);
    LONG    lLimitPels;

    // for large workareas, adjust scroll bar units
    LONG    lScrollUnitPels = 1;
    if (lWorkareaPels > 10000)
        lScrollUnitPels = 100;

    switch (usCmd)
    {
        case SB_LINEUP:
            *plCurPelsOfs -= usLineStepPels;
        break;

        case SB_LINEDOWN:
            *plCurPelsOfs += usLineStepPels;
        break;

        case SB_PAGEUP:
            *plCurPelsOfs -= lWindowPels;
        break;

        case SB_PAGEDOWN:
            *plCurPelsOfs += lWindowPels;
        break;

        case SB_SLIDERTRACK:
            *plCurPelsOfs = (LONG)usPosUnits * lScrollUnitPels;
        break;

        case SB_SLIDERPOSITION:
            *plCurPelsOfs = (LONG)usPosUnits * lScrollUnitPels;
        break;
    }

    // calc max scroll offset:
    // if we have a viewport of 200 and the window size is 199,
    // we can have scroll values of 0 or 1
    lLimitPels = lWorkareaPels - lWindowPels;

    // now delimit
    if (*plCurPelsOfs < 0)
        *plCurPelsOfs = 0;
    else if (*plCurPelsOfs > lLimitPels)
        *plCurPelsOfs = lLimitPels;

    if (    (*plCurPelsOfs != lOldPelsOfs)
         || (*plCurPelsOfs == 0)
         || (*plCurPelsOfs == lLimitPels)
       )
    {
        // changed:
        WinSendMsg(hwndScrollBar,
                   SBM_SETPOS,
                   (MPARAM)((*plCurPelsOfs + (lScrollUnitPels / 2)) / lScrollUnitPels),  // WarpIN V1.0.18
                   0);

        if (msg == WM_VSCROLL)
            return (*plCurPelsOfs - lOldPelsOfs);

        return -(*plCurPelsOfs - lOldPelsOfs);
    }

    return 0;
}

/*
 *@@ winhScrollWindow:
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

BOOL winhScrollWindow(HWND hwnd2Scroll,
                      PRECTL prclClip,          // clipping rectangle or NULL
                      PPOINTL pptlScroll)
{
    return !!WinScrollWindow(hwnd2Scroll,
                             pptlScroll->x,
                             pptlScroll->y,
                             prclClip,
                             prclClip,
                             NULLHANDLE,     // no region
                             NULL,           // no rect
                             SW_INVALIDATERGN);
}

/*
 *@@ winhProcessScrollChars2:
 *      helper for processing WM_CHAR messages for
 *      client windows with scroll bars.
 *
 *      If your window has scroll bars, you normally
 *      need to process a number of keystrokes to be
 *      able to scroll the window contents. This is
 *      tiresome to code, so here is a helper.
 *
 *      When receiving WM_CHAR, call this function.
 *      If this returns TRUE, the keystroke has been
 *      a scroll keystroke, and the window has been
 *      updated (by sending WM_VSCROLL or WM_HSCROLL
 *      to hwndClient). Otherwise, you should process
 *      the keystroke as usual because it's not a
 *      scroll keystroke.
 *
 *      The following keystrokes are processed here:
 *
 *      --  "cursor up, down, right, left": scroll one
 *          line in the proper direction.
 *      --  "page up, down": scroll one page up or down.
 *      --  "Home": scroll leftmost.
 *      --  "Ctrl+ Home": scroll topmost.
 *      --  "End": scroll rightmost.
 *      --  "Ctrl+ End": scroll bottommost.
 *      --  "Ctrl + page up, down": scroll one screen left or right.
 *
 *      This is CUA behavior, if anyone still cares about that.
 *
 *      Returns TRUE if the message has been processed.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 *@@changed V0.9.9 (2001-02-01) [lafaix]: Ctrl+PgUp/Dn now do one screen left/right
 */

BOOL winhProcessScrollChars2(HWND hwndClient,    // in: client window
                             const SCROLLABLEWINDOW *pscrw,   // in: scroller data
                             MPARAM mp1,         // in: WM_CHAR mp1
                             MPARAM mp2)         // in: WM_CHAR mp2
{
    BOOL    fProcessed = FALSE;
    USHORT usFlags    = SHORT1FROMMP(mp1);
    USHORT usvk       = SHORT2FROMMP(mp2);

    if (usFlags & KC_VIRTUALKEY)
    {
        ULONG   ulMsg = 0;
        SHORT   sPos = 0;
        SHORT   usCmd = 0;
        fProcessed = TRUE;

        switch (usvk)
        {
            case VK_UP:
                ulMsg = WM_VSCROLL;
                usCmd = SB_LINEUP;
            break;

            case VK_DOWN:
                ulMsg = WM_VSCROLL;
                usCmd = SB_LINEDOWN;
            break;

            case VK_RIGHT:
                ulMsg = WM_HSCROLL;
                usCmd = SB_LINERIGHT;
            break;

            case VK_LEFT:
                ulMsg = WM_HSCROLL;
                usCmd = SB_LINELEFT;
            break;

            case VK_PAGEUP:
                if (usFlags & KC_CTRL)
                    ulMsg = WM_HSCROLL;
                 else
                    ulMsg = WM_VSCROLL;
                usCmd = SB_PAGEUP;
            break;

            case VK_PAGEDOWN:
                if (usFlags & KC_CTRL)
                    ulMsg = WM_HSCROLL;
                else
                    ulMsg = WM_VSCROLL;
                usCmd = SB_PAGEDOWN;
            break;

            case VK_HOME:
                if (usFlags & KC_CTRL)
                    // vertical:
                    ulMsg = WM_VSCROLL;
                else
                    ulMsg = WM_HSCROLL;

                sPos = 0;
                usCmd = SB_SLIDERPOSITION;
            break;

            case VK_END:
                if (usFlags & KC_CTRL)
                {
                    // vertical:
                    ulMsg = WM_VSCROLL;
                    sPos = pscrw->szlWorkarea.cy;
                }
                else
                {
                    ulMsg = WM_HSCROLL;
                    sPos = pscrw->szlWorkarea.cx;
                }

                usCmd = SB_SLIDERPOSITION;
            break;

            default:
                // other:
                fProcessed = FALSE;
        }

        if (    ((usFlags & KC_KEYUP) == 0)
             && (ulMsg)
           )
        {
            HWND   hwndScrollBar = ((ulMsg == WM_VSCROLL)
                                        ? pscrw->hwndVScroll
                                        : pscrw->hwndHScroll);
            if (WinIsWindowEnabled(hwndScrollBar))
            {
                USHORT usID = WinQueryWindowUShort(hwndScrollBar,
                                                   QWS_ID);
                WinSendMsg(hwndClient,
                           ulMsg,
                           MPFROMSHORT(usID),
                           MPFROM2SHORT(sPos,
                                        usCmd));
            }
        }
    }

    return fProcessed;
}

/*
 *@@ winhProcessScrollChars:
 *      wrapper around winhProcessScrollChars2 for prototype
 *      compatibility.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

BOOL winhProcessScrollChars(HWND hwndClient,    // in: client window
                            HWND hwndVScroll,   // in: vertical scroll bar
                            HWND hwndHScroll,   // in: horizontal scroll bar
                            MPARAM mp1,         // in: WM_CHAR mp1
                            MPARAM mp2,         // in: WM_CHAR mp2
                            ULONG ulVertMax,    // in: maximum workarea cy
                            ULONG ulHorzMax)    // in: maximum workarea cx
{
    SCROLLABLEWINDOW scrw;

    scrw.hwndVScroll = hwndVScroll;
    scrw.hwndHScroll = hwndHScroll;
    scrw.szlWorkarea.cx = ulHorzMax;
    scrw.szlWorkarea.cy = ulVertMax;

    return winhProcessScrollChars2(hwndClient,
                                   &scrw,
                                   mp1,
                                   mp2);
}

/*
 *@@ winhCreateScrollBars:
 *      creates two scroll bars with an arbitrary
 *      position for later use with winhUpdateScrollBar.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

BOOL XWPENTRY winhCreateScroller(HWND hwndParent,           // in: parent and owner of scroll bars
                                 PSCROLLABLEWINDOW pscrw,   // out: scroller data
                                 ULONG idVScroll,           // in: window ID of vertical scroll bar
                                 ULONG idHScroll)           // in: window ID of horizontal scroll bar
{
    SBCDATA     sbcd;
    sbcd.cb = sizeof(SBCDATA);
    sbcd.sHilite = 0;
    sbcd.posFirst = 0;
    sbcd.posLast = 100;
    sbcd.posThumb = 30;
    sbcd.cVisible = 50;
    sbcd.cTotal = 50;

    pscrw->cxScrollBar = WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);
    pscrw->cyScrollBar = WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL);

    pscrw->idVScroll = idVScroll;
    pscrw->idHScroll = idHScroll;

    pscrw->szlWorkarea.cx
    = pscrw->szlWorkarea.cy
    = pscrw->ptlScrollOfs.x
    = pscrw->ptlScrollOfs.y
    = 0;

    return (    (pscrw->hwndVScroll = WinCreateWindow(hwndParent,
                                                      WC_SCROLLBAR,
                                                      "",
                                                      SBS_VERT | SBS_THUMBSIZE | WS_VISIBLE,
                                                      10, 10,
                                                      20, 100,
                                                      hwndParent,     // owner
                                                      HWND_TOP,
                                                      idVScroll,
                                                      &sbcd,
                                                      0))
             && (pscrw->hwndHScroll = WinCreateWindow(hwndParent,
                                                      WC_SCROLLBAR,
                                                      "",
                                                      SBS_THUMBSIZE | WS_VISIBLE,
                                                      10, 10,
                                                      20, 100,
                                                      hwndParent,     // owner
                                                      HWND_TOP,
                                                      idHScroll,
                                                      &sbcd,
                                                      0))
           );
}

/*
 *@@ winhHandleScrollerMsgs:
 *      unified function that you can simply call for all of
 *      the WM_HSCROLL, WM_VSCROLL, and WM_CHAR messages to
 *      get full automatic scrolling support for window
 *      messages.
 *
 *      This calls winhHandleScrollMsg, winhScrollWindow,
 *      and winhProcessScrollChars in turn as appropriate.
 *
 *      However, you still need to call winhUpdateScrollBar
 *      for each scroll bar whenever either the workarea or
 *      window size changes.
 *
 *      See winhUpdateScrollBar for the terminology.
 *
 *      The size of the workarea must be passed in with
 *      pscrw, as well as the window and scrollbar handles.
 *      By contrast, the current window size must be passed
 *      in via the pszlWin parameter. This allows for cooperation
 *      with the ctlDefWindowProc funcs, which have their own
 *      fields for this in DEFWINDOWDATA so we won't duplicate.
 *
 *      Preconditions:
 *
 *      --  hwnd2Scroll (the "client") and the scroll bar windows
 *          must be siblings presently. In other words, scrolling
 *          won't work correctly if the scroll bars are children
 *          of the client because we do not presently handle
 *          passing a clipping rectangle to WinScrollWindow here.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

MRESULT winhHandleScrollerMsgs(HWND hwnd2Scroll,        // in: "client" window
                               PSCROLLABLEWINDOW pscrw, // in: scroller data
                               PSIZEL pszlWin,
                               ULONG msg,
                               MPARAM mp1,
                               MPARAM mp2)
{
    MRESULT mrc = 0;
    POINTL  ptlScroll = {0, 0};

    switch (msg)
    {
        case WM_VSCROLL:
            if (ptlScroll.y = winhHandleScrollMsg(pscrw->hwndVScroll,
                                                  &pscrw->ptlScrollOfs.y,
                                                  pszlWin->cy,
                                                  pscrw->szlWorkarea.cy,
                                                  10,
                                                  msg,
                                                  mp2))
                winhScrollWindow(hwnd2Scroll,
                                 NULL,
                                 &ptlScroll);
        break;

        case WM_HSCROLL:
            if (ptlScroll.x = winhHandleScrollMsg(pscrw->hwndHScroll,
                                                  &pscrw->ptlScrollOfs.x,
                                                  pszlWin->cx,
                                                  pscrw->szlWorkarea.cx,
                                                  10,
                                                  msg,
                                                  mp2))
                winhScrollWindow(hwnd2Scroll,
                                 NULL,
                                 &ptlScroll);
        break;

        case WM_CHAR:
            mrc = (MRESULT)winhProcessScrollChars(hwnd2Scroll,
                                                  pscrw->hwndVScroll,
                                                  pscrw->hwndHScroll,
                                                  mp1,
                                                  mp2,
                                                  pscrw->szlWorkarea.cy,
                                                  pscrw->szlWorkarea.cx);
        break;
    }

    return mrc;
}

/*
 *@@category: Helpers\PM helpers\Window positioning
 */

/* ******************************************************************
 *
 *   Window positioning helpers
 *
 ********************************************************************/

/*
 *@@ winhSaveWindowPos:
 *      saves the position of a certain window. As opposed
 *      to the barely documented WinStoreWindowPos API, this
 *      one only saves one regular SWP structure for the given
 *      window, as returned by WinQueryWindowPos for hwnd.
 *
 *      If the window is currently maximized or minimized,
 *      we won't store the current window size and position
 *      (which wouldn't make much sense), but retrieve the
 *      "restored" window position from the window words
 *      instead.
 *
 *      The window should still be visible on the screen
 *      when calling this function. Do not call it in WM_DESTROY,
 *      because then the SWP data is no longer valid.
 *
 *      This returns TRUE if saving was successful.
 *
 *@@changed V0.9.1 (99-12-19) [umoeller]: added minimize/maximize support
 */

BOOL winhSaveWindowPos(HWND hwnd,   // in: window to save
                       HINI hIni,   // in: INI file (or HINI_USER/SYSTEM)
                       const char *pcszApp,  // in: INI application name
                       const char *pcszKey)  // in: INI key name
{
    BOOL brc = FALSE;
    SWP swp;
    if (WinQueryWindowPos(hwnd, &swp))
    {
        if (swp.fl & (SWP_MAXIMIZE | SWP_MINIMIZE))
        {
            // window currently maximized or minimized:
            // retrieve "restore" position from window words
            swp.x = WinQueryWindowUShort(hwnd, QWS_XRESTORE);
            swp.y = WinQueryWindowUShort(hwnd, QWS_YRESTORE);
            swp.cx = WinQueryWindowUShort(hwnd, QWS_CXRESTORE);
            swp.cy = WinQueryWindowUShort(hwnd, QWS_CYRESTORE);
        }

        brc = PrfWriteProfileData(hIni, (PSZ)pcszApp, (PSZ)pcszKey, &swp, sizeof(swp));
    }
    return brc;
}

/*
 *@@ winhRestoreWindowPos:
 *      this will retrieve a window position which was
 *      previously stored using winhSaveWindowPos.
 *
 *      The window should not be visible to avoid flickering.
 *      "fl" must contain the SWP_flags as in WinSetWindowPos.
 *
 *      Note that only the following may be used:
 *      --  SWP_MOVE        reposition the window
 *      --  SWP_SIZE        also resize the window to
 *                          the stored position; this might
 *                          lead to problems with different
 *                          video resolutions, so be careful.
 *      --  SWP_SHOW        make window visible too
 *      --  SWP_NOREDRAW    changes are not redrawn
 *      --  SWP_NOADJUST    do not send a WM_ADJUSTWINDOWPOS message
 *                          before moving or sizing
 *      --  SWP_ACTIVATE    activate window (make topmost)
 *      --  SWP_DEACTIVATE  deactivate window (make bottommost)
 *
 *      Do not specify any other SWP_* flags.
 *
 *      If SWP_SIZE is not set, the window will be moved only.
 *
 *      This returns TRUE if INI data was found.
 *
 *      This function automatically checks for whether the
 *      window would be positioned outside the visible screen
 *      area and will adjust coordinates accordingly. This can
 *      happen when changing video resolutions.
 *
 *@@changed V0.9.7 (2000-12-20) [umoeller]: fixed invalid params if INI key not found
 */

BOOL winhRestoreWindowPos(HWND hwnd,   // in: window to restore
                          HINI hIni,   // in: INI file (or HINI_USER/SYSTEM)
                          const char *pcszApp,  // in: INI application name
                          const char *pcszKey,  // in: INI key name
                          ULONG fl)    // in: "fl" parameter for WinSetWindowPos
{
    BOOL    brc = FALSE;
    SWP     swp;
    ULONG   cbswp = sizeof(swp);
    ULONG   fl2 = (fl & ~SWP_ZORDER);

    if (PrfQueryProfileData(hIni, (PSZ)pcszApp, (PSZ)pcszKey, &swp, &cbswp))
    {
        ULONG ulScreenCX = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
        ULONG ulScreenCY = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

        brc = TRUE;

        if ((fl & SWP_SIZE) == 0)
        {
            // if no resize, we need to get the current position
            SWP swpNow;
            brc = WinQueryWindowPos(hwnd, &swpNow);
            swp.cx = swpNow.cx;
            swp.cy = swpNow.cy;
        }

        if (brc)
        {
            // check for full visibility
            if ( (swp.x + swp.cx) > ulScreenCX)
                swp.x = ulScreenCX - swp.cx;
            if ( (swp.y + swp.cy) > ulScreenCY)
                swp.y = ulScreenCY - swp.cy;
        }

        brc = TRUE;

    }
    else
    {
        // window pos not found in INI: unset SWP_MOVE etc.
        WinQueryWindowPos(hwnd, &swp);
        fl2 &= ~(SWP_MOVE | SWP_SIZE);
    }

    WinSetWindowPos(hwnd,
                    NULLHANDLE,       // insert-behind window
                    swp.x,
                    swp.y,
                    swp.cx,
                    swp.cy,
                    fl2);        // SWP_* flags

    return brc;
}

/*
 *@@ winhStoreWindowPos:
 *      saves the position of a certain window in the same format
 *      as the barely documented WinStoreWindowPos API.
 *      This uses the completely undocumented calls
 *      WinGetFrameTreePPSize and WinGetFrameTreePPs imported
 *      from PMWIN.DLL ordinals 972 and 973.
 *
 *      The window should still be visible on the screen
 *      when calling this function. Do not call it in WM_DESTROY,
 *      because then the SWP data is no longer valid.
 *
 *      This returns TRUE if saving was successful.
 *
 *@@added XWP V1.0.6 (2006-10-31) [pr]: @@fixes 458
 *@@changed XWP V1.0.7 (2006-12-16) [pr]: detect screen height/width @@fixes 903
 */

BOOL winhStoreWindowPos(HWND hwnd,   // in: window to save
                        HINI hIni,   // in: INI file (or HINI_USER/SYSTEM)
                        const char *pcszApp,  // in: INI application name
                        const char *pcszKey)  // in: INI key name
{
    BOOL brc = FALSE;
    SWP swp;

    if (WinQueryWindowPos(hwnd, &swp))
    {
        ULONG ulSizePP = WinGetFrameTreePPSize(hwnd);
        ULONG ulSize = sizeof(STOREPOS) + ulSizePP;
        PSTOREPOS pStorePos;

        if ((pStorePos = malloc(ulSize)))
        {
            // This first bit is guesswork as I don't know what it all means,
            // but it always seems to be the same everywhere I've looked.
            pStorePos->usMagic = 0x7B6A;
            pStorePos->ulRes1 = 1;
            pStorePos->ulRes2 = 1;
            pStorePos->ulRes3 = 0xFFFFFFFF;
            pStorePos->ulRes4 = 0xFFFFFFFF;

            pStorePos->ulFlags = swp.fl;
            pStorePos->usXPos = pStorePos->usRestoreXPos = swp.x;
            pStorePos->usYPos = pStorePos->usRestoreYPos = swp.y;
            pStorePos->usWidth = pStorePos->usRestoreWidth = swp.cx;
            pStorePos->usHeight = pStorePos->usRestoreHeight = swp.cy;
            if (swp.fl & (SWP_MAXIMIZE | SWP_MINIMIZE))
            {
                pStorePos->usRestoreXPos = WinQueryWindowUShort(hwnd, QWS_XRESTORE);
                pStorePos->usRestoreYPos = WinQueryWindowUShort(hwnd, QWS_YRESTORE);
                pStorePos->usRestoreWidth = WinQueryWindowUShort(hwnd, QWS_CXRESTORE);
                pStorePos->usRestoreHeight = WinQueryWindowUShort(hwnd, QWS_CYRESTORE);
            }

            pStorePos->usMinXPos = WinQueryWindowUShort(hwnd, QWS_XMINIMIZE);
            pStorePos->usMinYPos = WinQueryWindowUShort(hwnd, QWS_YMINIMIZE);
            pStorePos->ulScreenWidth = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);;
            pStorePos->ulScreenHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);;
            pStorePos->ulPPLen = WinGetFrameTreePPs(hwnd, ulSizePP, (PSZ)(pStorePos + 1));
            ulSize = pStorePos->ulPPLen + sizeof(STOREPOS);
            brc = PrfWriteProfileData(hIni, (PSZ)pcszApp, (PSZ)pcszKey, pStorePos, ulSize);
            free(pStorePos);
        }
    }
    return brc;
}

/*
 *@@ winhAdjustControls:
 *      helper function for dynamically adjusting a window's
 *      controls when the window is resized.
 *
 *      This is most useful with dialogs loaded from resources
 *      which should be sizeable. Normally, when the dialog
 *      frame is resized, the controls stick to their positions,
 *      and for dialogs with many controls, programming the
 *      changes can be tiresome.
 *
 *      Enter this function. ;-) Basically, this takes a
 *      static array of MPARAM's as input (plus one dynamic
 *      storage area for the window positions).
 *
 *      This function must get called in three contexts:
 *      during WM_INITDLG, during WM_WINDOWPOSCHANGED, and
 *      during WM_DESTROY, with varying parameters.
 *
 *      In detail, there are four things you need to do to make
 *      this work:
 *
 *      1)  Set up a static array (as a global variable) of
 *          MPARAM's, one for each control in your array.
 *          Each MPARAM will have the control's ID and the
 *          XAC_* flags (winh.h) how the control shall be moved.
 *          Use MPFROM2SHORT to easily create this. Example:
 *
 +          MPARAM ampControlFlags[] =
 +              {  MPFROM2SHORT(ID_CONTROL_1, XAC_MOVEX),
 +                 MPFROM2SHORT(ID_CONTROL_2, XAC_SIZEY),
 +                  ...
 +              }
 *
 *          This can safely be declared as a global variable
 *          because this data will only be read and never
 *          changed by this function.
 *
 *      2)  In WM_INITDLG of your dialog function, set up
 *          an XADJUSTCTRLS structure, preferrably in your
 *          window words (QWL_USER).
 *
 *          ZERO THAT STRUCTURE (memset(&xac, 0, sizeof(XADJUSTCTRLS),
 *          or this func will not work (because it will intialize
 *          things on the first WM_WINDOWPOSCHANGED).
 *
 *      3)  Intercept WM_WINDOWPOSCHANGED:
 *
 +          case WM_WINDOWPOSCHANGED:
 +          {
 +              // this msg is passed two SWP structs:
 +              // one for the old, one for the new data
 +              // (from PM docs)
 +              PSWP pswpNew = PVOIDFROMMP(mp1);
 +              PSWP pswpOld = pswpNew + 1;
 +
 +              // resizing?
 +              if (pswpNew->fl & SWP_SIZE)
 +              {
 +                  PXADJUSTCTRLS pxac = ... // get it from your window words
 +
 +                  winhAdjustControls(hwndDlg,             // dialog
 +                                     ampControlFlags,     // MPARAMs array
 +                                     sizeof(ampControlFlags) / sizeof(MPARAM),
 +                                                          // items count
 +                                     pswpNew,             // mp1
 +                                     pxac);               // storage area
 +              }
 +              mrc = WinDefDlgProc(hwnd, msg, mp1, mp2); ...
 *
 *      4)  In WM_DESTROY, call this function again with pmpFlags,
 *          pswpNew, and pswpNew set to NULL. This will clean up the
 *          data which has been allocated internally (pointed to from
 *          the XADJUSTCTRLS structure).
 *          Don't forget to free your storage for XADJUSTCTLRS
 *          _itself_, that's the job of the caller.
 *
 *      This might sound complicated, but it's a lot easier than
 *      having to write dozens of WinSetWindowPos calls oneself.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.19 (2002-04-13) [umoeller]: added correlation for entry field repositioning, this was always off
 */

BOOL winhAdjustControls(HWND hwndDlg,           // in: dialog (req.)
                        const MPARAM *pmpFlags, // in: init flags or NULL for cleanup
                        ULONG ulCount,          // in: item count (req.)
                        PSWP pswpNew,           // in: pswpNew from WM_WINDOWPOSCHANGED or NULL for cleanup
                        PXADJUSTCTRLS pxac)     // in: adjust-controls storage area (req.)
{
    BOOL    brc = FALSE;
    ULONG   ul = 0;

    if ((pmpFlags) && (pxac))
    {
        PSWP    pswpThis;
        const MPARAM  *pmpThis;
        LONG    ldcx, ldcy;
        ULONG   cWindows = 0;

        // V0.9.19 (2002-04-13) [umoeller]
        LONG cxMarginEF = 3 * WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
        LONG cyMarginEF = 3 * WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);

        // setup mode:
        if (pxac->fInitialized == FALSE)
        {
            // first call: get all the SWP's
            WinQueryWindowPos(hwndDlg, &pxac->swpMain);
            // _Pmpf(("winhAdjustControls: queried main cx = %d, cy = %d",
               //      pxac->swpMain.cx, pxac->swpMain.cy));

            pxac->paswp = (PSWP)malloc(sizeof(SWP) * ulCount);

            pswpThis = pxac->paswp;
            pmpThis = pmpFlags;

            for (ul = 0;
                 ul < ulCount;
                 ul++)
            {
                HWND hwndThis;
                CHAR szClass[10];
                if (hwndThis = WinWindowFromID(hwndDlg, SHORT1FROMMP(*pmpThis)))
                {
                    WinQueryWindowPos(hwndThis, pswpThis);

                    // correlate the stupid repositioning of entry fields
                    // V0.9.19 (2002-04-13) [umoeller]
                    if (    (WinQueryClassName(hwndThis, sizeof(szClass), szClass)
                         && (!strcmp(szClass, "#6"))
                         && (WinQueryWindowULong(hwndThis, QWL_STYLE) & ES_MARGIN))
                       )
                    {
                        pswpThis->x += cxMarginEF;
                        pswpThis->y += cyMarginEF;
                        pswpThis->cx -= 2 * cxMarginEF;
                        pswpThis->cy -= 2 * cyMarginEF;
                    }

                    cWindows++;
                }

                pswpThis++;
                pmpThis++;
            }

            pxac->fInitialized = TRUE;
            // _Pmpf(("winhAdjustControls: queried %d controls", cWindows));
        }

        if (pswpNew)
        {
            // compute width and height delta
            ldcx = (pswpNew->cx - pxac->swpMain.cx);
            ldcy = (pswpNew->cy - pxac->swpMain.cy);

            // _Pmpf(("winhAdjustControls: new cx = %d, cy = %d",
              //       pswpNew->cx, pswpNew->cy));

            // now adjust the controls
            cWindows = 0;
            pswpThis = pxac->paswp;
            pmpThis = pmpFlags;
            for (ul = 0;
                 ul < ulCount;
                 ul++)
            {
                HWND hwndThis;
                if (hwndThis = WinWindowFromID(hwndDlg, SHORT1FROMMP(*pmpThis)))
                {
                    LONG    x = pswpThis->x,
                            y = pswpThis->y,
                            cx = pswpThis->cx,
                            cy = pswpThis->cy;

                    ULONG   ulSwpFlags = 0;
                    // get flags for this control
                    USHORT  usFlags = SHORT2FROMMP(*pmpThis);

                    if (usFlags & XAC_MOVEX)
                    {
                        x += ldcx;
                        ulSwpFlags |= SWP_MOVE;
                    }
                    if (usFlags & XAC_MOVEY)
                    {
                        y += ldcy;
                        ulSwpFlags |= SWP_MOVE;
                    }
                    if (usFlags & XAC_SIZEX)
                    {
                        cx += ldcx;
                        ulSwpFlags |= SWP_SIZE;
                    }
                    if (usFlags & XAC_SIZEY)
                    {
                        cy += ldcy;
                        ulSwpFlags |= SWP_SIZE;
                    }

                    if (ulSwpFlags)
                    {
                        WinSetWindowPos(hwndThis,
                                        NULLHANDLE, // hwndInsertBehind
                                        x, y, cx, cy,
                                        ulSwpFlags);
                        cWindows++;
                        brc = TRUE;
                    }
                }

                pswpThis++;
                pmpThis++;
            }

            // _Pmpf(("winhAdjustControls: set %d windows", cWindows));
        }
    }
    else
    {
        // pxac == NULL:
        // cleanup mode
        if (pxac->paswp)
            free(pxac->paswp);
    }

    return brc;
}

/*
 *@@ winhCenterWindow:
 *      centers a window within its parent window. If that's
 *      the PM desktop, it will be centered according to the
 *      whole screen.
 *      For dialog boxes, use WinCenteredDlgBox as a one-shot
 *      function.
 *
 *      Note: When calling this function, the window should
 *      not be visible to avoid flickering.
 *      This func does not show the window either, so call
 *      WinShowWindow afterwards.
 */

void winhCenterWindow(HWND hwnd)
{
   RECTL rclParent;
   RECTL rclWindow;

   WinQueryWindowRect(hwnd, &rclWindow);
   WinQueryWindowRect(WinQueryWindow(hwnd, QW_PARENT), &rclParent);

   rclWindow.xLeft   = (rclParent.xRight - rclWindow.xRight) / 2;
   rclWindow.yBottom = (rclParent.yTop   - rclWindow.yTop  ) / 2;

   WinSetWindowPos(hwnd, NULLHANDLE, rclWindow.xLeft, rclWindow.yBottom,
                    0, 0, SWP_MOVE);
}

/*
 *@@ winhCenteredDlgBox:
 *      just like WinDlgBox, but the dlg box is centered on the screen;
 *      you should mark the dlg template as not visible in the dlg
 *      editor, or display will flicker.
 *      As opposed to winhCenterWindow, this _does_ show the window.
 */

ULONG winhCenteredDlgBox(HWND hwndParent,
                         HWND hwndOwner,
                         PFNWP pfnDlgProc,
                         HMODULE hmod,
                         ULONG idDlg,
                         PVOID pCreateParams)
{
    ULONG   ulReply;
    HWND    hwndDlg = WinLoadDlg(hwndParent,
                                 hwndOwner,
                                 pfnDlgProc,
                                 hmod,
                                 idDlg,
                                 pCreateParams);
    winhCenterWindow(hwndDlg);
    ulReply = WinProcessDlg(hwndDlg);
    WinDestroyWindow(hwndDlg);
    return ulReply;
}

/*
 *@@ winhPlaceBesides:
 *      attempts to place hwnd somewhere besides
 *      hwndRelative.
 *
 *      fl is presently ignored, but should be
 *      PLF_SMART for future extensions.
 *
 *      Works only if hwnd is a desktop child.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 *@@changed V1.0.0 (2002-08-26) [umoeller]: fixed cx and cy confusion
 */

BOOL winhPlaceBesides(HWND hwnd,
                      HWND hwndRelative,
                      ULONG fl)
{
    SWP     swpRel,
            swpThis;
    LONG    xNew, yNew;

    if (    (WinQueryWindowPos(hwndRelative, &swpRel))
         && (WinQueryWindowPos(hwnd, &swpThis))
       )
    {
        HWND    hwndRelParent,
                hwndThisParent;
        POINTL  ptlRel;
		ptlRel.x=swpRel.x;
		ptlRel.y=swpRel.y;
        if (    (hwndRelParent = WinQueryWindow(hwndRelative, QW_PARENT))
             && (hwndThisParent = WinQueryWindow(hwnd, QW_PARENT))
             && (hwndRelParent != hwndThisParent)
           )
        {
            WinMapWindowPoints(hwndRelParent,
                               hwndThisParent,
                               &ptlRel,
                               1);
        }

        // place right first
        xNew = ptlRel.x + swpRel.cx;
        // center vertically
        yNew = ptlRel.y  + ((swpRel.cy - swpThis.cy) / 2);

        // if (xNew + swpThis.cy > WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN))
                // not cy, but cx V1.0.0 (2002-08-26) [umoeller]
        if (xNew + swpThis.cx > G_cxScreen)
        {
            // place left then
            xNew = ptlRel.x - swpThis.cx;

            if (xNew < 0)
            {
                // center then
                winhCenterWindow(hwnd);
                return TRUE;
            }
        }

        return WinSetWindowPos(hwnd,
                               0,
                               xNew,
                               yNew,
                               0,
                               0,
                               SWP_MOVE);
    }

    return FALSE;
}

/*
 *@@ winhFindWindowBelow:
 *      finds the window with the same parent
 *      which sits right below hwndFind in the
 *      window Z-order.
 *
 *@@added V0.9.7 (2000-12-04) [umoeller]
 */

HWND winhFindWindowBelow(HWND hwndFind)
{
    HWND hwnd = NULLHANDLE,
         hwndParent = WinQueryWindow(hwndFind, QW_PARENT);

    if (hwndParent)
    {
        HENUM   henum = WinBeginEnumWindows(hwndParent);
        HWND    hwndThis;
        while (hwndThis = WinGetNextWindow(henum))
        {
            SWP swp;
            WinQueryWindowPos(hwndThis, &swp);
            if (swp.hwndInsertBehind == hwndFind)
            {
                hwnd = hwndThis;
                break;
            }
        }
        WinEndEnumWindows(henum);
    }

    return hwnd;
}

/*
 *@@category: Helpers\PM helpers\Presentation parameters
 */

/* ******************************************************************
 *
 *   Presparams helpers
 *
 ********************************************************************/

/*
 *@@ winhQueryWindowFont:
 *      returns the window font presentation parameter
 *      in a newly allocated buffer.
 *
 *      Returns NULL on error. Use free()
 *      to free the return value.
 *
 *@@added V0.9.1 (2000-02-14) [umoeller]
 */

PSZ winhQueryWindowFont(HWND hwnd)
{
    CHAR  szNewFont[100] = "";
    WinQueryPresParam(hwnd,
                      PP_FONTNAMESIZE,
                      0,
                      NULL,
                      (ULONG)sizeof(szNewFont),
                      (PVOID)&szNewFont,
                      QPF_NOINHERIT);
    if (szNewFont[0] != 0)
        return strdup(szNewFont);

    return NULL;
}

/*
 *@@ winhQueryDefaultFont:
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 *@@changed V1.0.4 (2005-09-02) [bvl]: Return 'Combined' fonts on DBCS systems to show DBCS characters properly @@fixes 655
 */

PCSZ winhQueryDefaultFont(VOID)
{
    if (doshIsWarp4())
        if(nlsDBCS())
            return "9.WarpSans Combined";
        else
            return "9.WarpSans";
    else
        if(nlsDBCS())
            return "8.Helv Combined";
        else
            return "8.Helv";
}

/*
 *@@ winhQueryMenuSysFont:
 *      returns the system menu font in a new buffer
 *      to be free()'d by caller.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

PSZ winhQueryMenuSysFont(VOID)
{
    PSZ pszStdMenuFont;
    if (!(pszStdMenuFont = prfhQueryProfileData(HINI_USER,
                                                PMINIAPP_SYSTEMFONTS, // "PM_SystemFonts",
                                                PMINIKEY_MENUSFONT, // "Menus",
                                                NULL)))
        pszStdMenuFont = prfhQueryProfileData(HINI_USER,
                                              PMINIAPP_SYSTEMFONTS, // "PM_SystemFonts",
                                              PMINIKEY_DEFAULTFONT, // "DefaultFont",
                                              NULL);

    return pszStdMenuFont;
}

/*
 *@@ winhSetWindowFont:
 *      this sets a window's font by invoking
 *      WinSetPresParam on it.
 *
 *      If (pszFont == NULL), a default font will be set
 *      (on Warp 4, "9.WarpSans", on Warp 3, "8.Helv").
 *
 *      winh.h also defines the winhSetDlgItemFont macro.
 *
 *      Returns TRUE if successful or FALSE otherwise.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: optimized
 */

BOOL winhSetWindowFont(HWND hwnd,
                       const char *pcszFont)
{
    if (!pcszFont)
        pcszFont = winhQueryDefaultFont();

    return WinSetPresParam(hwnd,
                           PP_FONTNAMESIZE,
                           strlen(pcszFont) + 1,
                           (PSZ)pcszFont);
}

/*
 *@@ winhSetControlsFont:
 *      this sets the font for all the controls of hwndDlg
 *      which have a control ID in the range of usIDMin to
 *      usIDMax. "Unused" IDs (i.e. -1) will also be set.
 *
 *      If (pszFont == NULL), a default font will be set
 *      (on Warp 4, "9.WarpSans", on Warp 3, "8.Helv").
 *
 *      Returns the no. of controls set.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.4 (2005-09-02) [bvl]: Return 'Combined' fonts on DBCS systems to show DBCS characters properly @@fixes 655
 */

ULONG winhSetControlsFont(HWND hwndDlg,      // in: dlg to set
                          SHORT usIDMin,     // in: minimum control ID to be set (inclusive)
                          SHORT usIDMax,     // in: maximum control ID to be set (inclusive)
                          const char *pcszFont)  // in: font to use (e.g. "9.WarpSans") or NULL
{
    ULONG   ulrc = 0;
    HENUM   henum;
    HWND    hwndItem;
    ULONG   cbFont;

    if (!pcszFont)
        pcszFont = winhQueryDefaultFont();

    cbFont = strlen(pcszFont) + 1;
    // set font for all the dialog controls
    henum = WinBeginEnumWindows(hwndDlg);
    while ((hwndItem = WinGetNextWindow(henum)))
    {
        SHORT sID = WinQueryWindowUShort(hwndItem, QWS_ID);
        if (    (sID == -1)
             || ((sID >= usIDMin) && (sID <= usIDMax))
           )
            if (WinSetPresParam(hwndItem,
                                PP_FONTNAMESIZE,
                                cbFont,
                                (PSZ)pcszFont))
                // successful:
                ulrc++;
    }
    WinEndEnumWindows(henum);

    return ulrc;
}

/*
 *@@ winhStorePresParam:
 *      this appends a new presentation parameter to an
 *      array of presentation parameters which can be
 *      passed to WinCreateWindow. This is preferred
 *      over setting the presparams using WinSetPresParams,
 *      because that call will cause a lot of messages.
 *
 *      On the first call, pppp _must_ be NULL. This
 *      will allocate memory for storing the given
 *      data as necessary and modify *pppp to point
 *      to the new array.
 *
 *      On subsequent calls with the same pppp, memory
 *      will be reallocated, the old data will be copied,
 *      and the new given data will be appended.
 *
 *      Use free() on your PPRESPARAMS pointer (whose
 *      address was passed) after WinCreateWindow.
 *
 *      See winhQueryPresColor for typical presparams
 *      used in OS/2.
 *
 *      Example:
 *
 +          PPRESPARAMS ppp = NULL;
 +          CHAR szFont[] = "9.WarpSans";
 +          LONG lColor = CLR_WHITE;
 +          winhStorePresParam(&ppp, PP_FONTNAMESIZE, sizeof(szFont), szFont);
 +          winhStorePresParam(&ppp, PP_BACKGROUNDCOLOR, sizeof(lColor), &lColor);
 +          WinCreateWindow(...., ppp);
 +          free(ppp);
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL winhStorePresParam(PPRESPARAMS *pppp,      // in: data pointer (modified)
                        ULONG ulAttrType,       // in: PP_* index
                        ULONG cbData,           // in: sizeof(*pData), e.g. sizeof(LONG)
                        PVOID pData)            // in: presparam data (e.g. a PLONG to a color)
{
    BOOL        brc = FALSE;
    if (pppp)
    {
        ULONG       cbOld = 0,
                    cbNew;
        PBYTE       pbTemp = 0;
        PPRESPARAMS pppTemp = 0;
        PPARAM      pppCopyTo = 0;

        if (*pppp != NULL)
            // subsequent calls:
            cbOld = (**pppp).cb;

        cbNew = sizeof(ULONG)       // PRESPARAMS.cb
                + cbOld             // old count, which does not include PRESPARAMS.cb
                + sizeof(ULONG)     // PRESPARAMS.aparam[0].id
                + sizeof(ULONG)     // PRESPARAMS.aparam[0].cb
                + cbData;           // PRESPARAMS.aparam[0].ab[]

        pbTemp = (PBYTE)malloc(cbNew);
        if (pbTemp)
        {
            pppTemp = (PPRESPARAMS)pbTemp;

            if (*pppp != NULL)
            {
                // copy old data
                memcpy(pbTemp, *pppp, cbOld + sizeof(ULONG)); // including PRESPARAMS.cb
                pppCopyTo = (PPARAM)(pbTemp             // new buffer
                                     + sizeof(ULONG)    // skipping PRESPARAMS.cb
                                     + cbOld);          // old PARAM array
            }
            else
                // first call:
                pppCopyTo = pppTemp->aparam;

            pppTemp->cb = cbNew - sizeof(ULONG);     // excluding PRESPARAMS.cb
            pppCopyTo->id = ulAttrType;
            pppCopyTo->cb = cbData;       // byte count of PARAM.ab[]
            memcpy(pppCopyTo->ab, pData, cbData);

            free(*pppp);
            *pppp = pppTemp;

            brc = TRUE;
        }
    }

    return brc;
}

/*
 *@@ winhCreateDefaultPresparams:
 *
 *      Caller must free() the return value.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

PPRESPARAMS winhCreateDefaultPresparams(VOID)
{
    PPRESPARAMS ppp = NULL;

    PCSZ    pcszFont = winhQueryDefaultFont();
    LONG    lColor;

    winhStorePresParam(&ppp,
                       PP_FONTNAMESIZE,
                       strlen(pcszFont) + 1,
                       (PVOID)pcszFont);

    lColor = WinQuerySysColor(HWND_DESKTOP,
                              SYSCLR_DIALOGBACKGROUND,
                              0);
    winhStorePresParam(&ppp,
                       PP_BACKGROUNDCOLOR,
                       sizeof(lColor),
                       &lColor);

    lColor = RGBCOL_BLACK;
    winhStorePresParam(&ppp,
                       PP_FOREGROUNDCOLOR,
                       sizeof(lColor),
                       &lColor);

    return ppp;
}

/*
 *@@ winhQueryPresColor2:
 *      returns the specified color. This is queried in the
 *      following order:
 *
 *      1)  hwnd's pres params are searched for ulPP
 *          (which should be a PP_* index);
 *      2)  if (fInherit == TRUE), the parent windows
 *          are searched also;
 *      3)  if this fails or (fInherit == FALSE), WinQuerySysColor
 *          is called to get lSysColor (which should be a SYSCLR_*
 *          index), if lSysColor != -1;
 *      4)  if (lSysColor == -1), -1 is returned.
 *
 *      The return value is always an RGB LONG, _not_ a color index.
 *      This is even true for the returned system colors, which are
 *      converted to RGB.
 *
 *      If you do any painting with this value, you should switch
 *      the HPS you're using to RGB mode (use gpihSwitchToRGB for that).
 *
 *      Some useful ulPP / lSysColor pairs
 *      (default values as in PMREF):
 *
 +          --  PP_FOREGROUNDCOLOR          SYSCLR_WINDOWTEXT (for most controls also)
 +                                          SYSCLR_WINDOWSTATICTEXT (for static controls)
 +                 Foreground color (default: black)
 +          --  PP_BACKGROUNDCOLOR          SYSCLR_BACKGROUND
 +                                          SYSCLR_DIALOGBACKGROUND
 +                                          SYSCLR_FIELDBACKGROUND (for disabled scrollbars)
 +                                          SYSCLR_WINDOW (application surface -- empty clients)
 +                 Background color (default: light gray)
 +          --  PP_ACTIVETEXTFGNDCOLOR
 +          --  PP_HILITEFOREGROUNDCOLOR    SYSCLR_HILITEFOREGROUND
 +                 Highlighted foreground color, for example for selected menu
 +                 (def.: white)
 +          --  PP_ACTIVETEXTBGNDCOLOR
 +          --  PP_HILITEBACKGROUNDCOLOR    SYSCLR_HILITEBACKGROUND
 +                 Highlighted background color (def.: dark gray)
 +          --  PP_INACTIVETEXTFGNDCOLOR
 +          --  PP_DISABLEDFOREGROUNDCOLOR  SYSCLR_MENUDISABLEDTEXT
 +                 Disabled foreground color (dark gray)
 +          --  PP_INACTIVETEXTBGNDCOLOR
 +          --  PP_DISABLEDBACKGROUNDCOLOR
 +                 Disabled background color
 +          --  PP_BORDERCOLOR              SYSCLR_WINDOWFRAME
 +                                          SYSCLR_INACTIVEBORDER
 +                 Border color (around pushbuttons, in addition to
 +                 the 3D colors)
 +          --  PP_ACTIVECOLOR              SYSCLR_ACTIVETITLE
 +                 Active color
 +          --  PP_INACTIVECOLOR            SYSCLR_INACTIVETITLE
 +                 Inactive color
 *
 *      For menus:
 +          --  PP_MENUBACKGROUNDCOLOR      SYSCLR_MENU
 +          --  PP_MENUFOREGROUNDCOLOR      SYSCLR_MENUTEXT
 +          --  PP_MENUHILITEBGNDCOLOR      SYSCLR_MENUHILITEBGND
 +          --  PP_MENUHILITEFGNDCOLOR      SYSCLR_MENUHILITE
 +          --  ??                          SYSCLR_MENUDISABLEDTEXT
 +
 *      For containers (according to the API ref. at EDM/2):
 +          --  PP_FOREGROUNDCOLOR          SYSCLR_WINDOWTEXT
 +          --  PP_BACKGROUNDCOLOR          SYSCLR_WINDOW
 +          --  PP_HILITEFOREGROUNDCOLOR    SYSCLR_HILITEFOREGROUND
 +          --  PP_HILITEBACKGROUNDCOLOR    SYSCLR_HILITEBACKGROUND
 +          --  PP_BORDERCOLOR
 +                  (used for separator lines, eg. in Details view)
 +          --  PP_ICONTEXTBACKGROUNDCOLOR
 +                  (column titles in Details view?!?)
 +
 *      For listboxes / entryfields / MLE's:
 +          --  PP_BACKGROUNDCOLOR          SYSCLR_ENTRYFIELD
 *
 *  PMREF has more of these.
 *
 *@@changed V0.9.0 [umoeller]: removed INI key query, using SYSCLR_* instead; function prototype changed
 *@@changed V0.9.0 [umoeller]: added fInherit parameter
 *@@changed V0.9.7 (2000-12-02) [umoeller]: added lSysColor == -1 support
 *@@changed V0.9.20 (2002-08-04) [umoeller]: added ulPPIndex, renamed func
 */

LONG winhQueryPresColor2(HWND hwnd,          // in: window to query
                         ULONG ulppRGB,      // in: PP_* index for RGB color
                         ULONG ulppIndex,    // in: PP_* index for color _index_ (can be null)
                         BOOL fInherit,      // in: search parent windows too?
                         LONG lSysColor)     // in: SYSCLR_* index or -1
{
    ULONG   ul,
            attrFound;
    LONG    lColorFound;

    if (ulppRGB != (ULONG)-1)
    {
        ULONG fl = 0;
        if (!fInherit)
            fl = QPF_NOINHERIT;
        if (ulppIndex)
            fl |= QPF_ID2COLORINDEX;            // convert indexed color 2 to RGB V0.9.20 (2002-08-04) [umoeller]

        if (ul = WinQueryPresParam(hwnd,
                                   ulppRGB,
                                   ulppIndex,
                                   &attrFound,
                                   sizeof(lColorFound),
                                   &lColorFound,
                                   fl))
            return lColorFound;
    }

    // not found: get system color
    if (lSysColor != -1)
        return WinQuerySysColor(HWND_DESKTOP, lSysColor, 0);

    return -1;
}

/*
 *@@ winhQueryPresColor:
 *      compatibility function because this one was
 *      exported.
 *
 *@@added V0.9.20 (2002-08-04) [umoeller]
 */

LONG XWPENTRY winhQueryPresColor(HWND hwnd,
                                 ULONG ulPP,
                                 BOOL fInherit,
                                 LONG lSysColor)
{
    return winhQueryPresColor2(hwnd,
                               ulPP,
                               0,
                               fInherit,
                               lSysColor);
}

/*
 *@@ winhSetPresColor:
 *      sets a color presparam. ulIndex specifies
 *      the presparam to be set and would normally
 *      be either PP_BACKGROUNDCOLOR or PP_FOREGROUNDCOLOR.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

BOOL winhSetPresColor(HWND hwnd,
                      ULONG ulIndex,
                      LONG lColor)
{
    return WinSetPresParam(hwnd,
                           ulIndex,
                           sizeof(LONG),
                           &lColor);
}

/*
 *@@category: Helpers\PM helpers\Help (IPF)
 */

/* ******************************************************************
 *
 *   Help instance helpers
 *
 ********************************************************************/

/*
 *@@ winhCreateHelp:
 *      creates a help instance and connects it with the
 *      given frame window.
 *
 *      If (pszFileName == NULL), we'll retrieve the
 *      executable's fully qualified file name and
 *      replace the extension with .HLP simply. This
 *      avoids the typical "Help not found" errors if
 *      the program isn't started in its own directory.
 *
 *      If you have created a help table in memory, specify it
 *      with pHelpTable. To load a help table from the resources,
 *      specify hmod (or NULLHANDLE) and set pHelpTable to the
 *      following:
 +
 +          (PHELPTABLE)MAKELONG(usTableID, 0xffff)
 *
 *      Returns the help window handle or NULLHANDLE on errors.
 *
 *      Based on an EDM/2 code snippet.
 *
 *@@added V0.9.4 (2000-07-03) [umoeller]
 */

HWND winhCreateHelp(HWND hwndFrame,      // in: app's frame window handle; can be NULLHANDLE
                    const char *pcszFileName,    // in: help file name or NULL
                    HMODULE hmod,           // in: module with help table or NULLHANDLE (current)
                    PHELPTABLE pHelpTable,  // in: help table or resource ID
                    const char *pcszWindowTitle) // in: help window title or NULL
{
    HELPINIT hi;
    PSZ      pszExt;
    CHAR     szName[CCHMAXPATH] = "";
    HWND     hwndHelp;

    if (pcszFileName == NULL)
    {
        PPIB     ppib;
        PTIB     ptib;
        DosGetInfoBlocks(&ptib, &ppib);
        DosQueryModuleName(ppib->pib_hmte, sizeof(szName), szName);

        pszExt = strrchr(szName, '.');
        if (pszExt)
            strcpy(pszExt, ".hlp");
        else
            strcat(szName, ".hlp");

        pcszFileName = szName;
    }

    hi.cb                       = sizeof(HELPINIT);
    hi.ulReturnCode             = 0;
    hi.pszTutorialName          = NULL;
    hi.phtHelpTable             = pHelpTable;
    hi.hmodHelpTableModule      = hmod;
    hi.hmodAccelActionBarModule = NULLHANDLE;
    hi.idAccelTable             = 0;
    hi.idActionBar              = 0;
    hi.pszHelpWindowTitle       = (PSZ)pcszWindowTitle;
    hi.fShowPanelId             = CMIC_HIDE_PANEL_ID;
    hi.pszHelpLibraryName       = (PSZ)pcszFileName;

    hwndHelp = WinCreateHelpInstance(WinQueryAnchorBlock(hwndFrame),
                                     &hi);
    if ((hwndFrame) && (hwndHelp))
    {
        WinAssociateHelpInstance(hwndHelp, hwndFrame);
    }

    return hwndHelp;
}

/*
 *@@ winhDisplayHelpPanel:
 *      displays the specified help panel ID.
 *
 *      If (ulHelpPanel == 0), this displays the
 *      standard OS/2 "Using help" panel.
 *
 *      Returns zero on success or one of the
 *      help manager error codes on failure.
 *      See HM_ERROR for those.
 *
 *@@added V0.9.7 (2001-01-21) [umoeller]
 */

ULONG winhDisplayHelpPanel(HWND hwndHelpInstance,   // in: from winhCreateHelp
                           ULONG ulHelpPanel)       // in: help panel ID
{
    return (ULONG)WinSendMsg(hwndHelpInstance,
                             HM_DISPLAY_HELP,
                             (MPARAM)ulHelpPanel,
                             (MPARAM)(    (ulHelpPanel != 0)
                                          ? HM_RESOURCEID
                                          : 0));
}

/*
 *@@ winhDestroyHelp:
 *      destroys the help instance created by winhCreateHelp.
 *
 *      Based on an EDM/2 code snippet.
 *
 *@@added V0.9.4 (2000-07-03) [umoeller]
 */

void winhDestroyHelp(HWND hwndHelp,
                     HWND hwndFrame)    // can be NULLHANDLE if not used with winhCreateHelp
{
    if (hwndHelp)
    {
        if (hwndFrame)
            WinAssociateHelpInstance(NULLHANDLE, hwndFrame);
        WinDestroyHelpInstance(hwndHelp);
    }
}

/*
 *@@category: Helpers\PM helpers\Application control
 */

/* ******************************************************************
 *
 *   Application control
 *
 ********************************************************************/

/*
 *@@ winhAnotherInstance:
 *      this tests whether another instance of the same
 *      application is already running.
 *
 *      To identify instances of the same application, the
 *      application must call this function during startup
 *      with the unique name of an OS/2 semaphore. As with
 *      all OS/2 semaphores, the semaphore name must begin
 *      with "\\SEM32\\". The semaphore isn't really used
 *      except for testing for its existence, since that
 *      name is unique among all processes.
 *
 *      If another instance is found, TRUE is returned. If
 *      (fSwitch == TRUE), that instance is switched to,
 *      using the tasklist.
 *
 *      If no other instance is found, FALSE is returned only.
 *
 *      Based on an EDM/2 code snippet.
 *
 *@@added V0.9.0 (99-10-22) [umoeller]
 */

BOOL winhAnotherInstance(const char *pcszSemName,    // in: semaphore ID
                         BOOL fSwitch)      // in: if TRUE, switch to first instance if running
{
    HMTX hmtx;

    if (DosCreateMutexSem((PSZ)pcszSemName,
                          &hmtx,
                          DC_SEM_SHARED,
                          TRUE)
              == NO_ERROR)
        // semapore created: this doesn't happen if the semaphore
        // exists already, so no other instance is running
        return FALSE;

    // else: instance running
    hmtx = NULLHANDLE;

    // switch to other instance?
    if (fSwitch)
    {
        // yes: query mutex creator
        if (DosOpenMutexSem((PSZ)pcszSemName,
                            &hmtx)
                    == NO_ERROR)
        {
            PID     pid = 0;
            TID     tid = 0;        // unused
            ULONG   ulCount;        // unused

            if (DosQueryMutexSem(hmtx, &pid, &tid, &ulCount) == NO_ERROR)
            {
                HSWITCH hswitch = WinQuerySwitchHandle(NULLHANDLE, pid);
                if (hswitch != NULLHANDLE)
                    WinSwitchToProgram(hswitch);
            }

            DosCloseMutexSem(hmtx);
        }
    }

    return TRUE;      // another instance exists
}

/*
 *@@ winhAddToTasklist:
 *      this adds the specified window to the tasklist
 *      with hIcon as its program icon (which is also
 *      set for the main window). This is useful for
 *      the old "dialog as main window" trick.
 *
 *      Returns the HSWITCH of the added entry.
 */

HSWITCH winhAddToTasklist(HWND hwnd,       // in: window to add
                          HPOINTER hIcon)  // in: icon for main window
{
    SWCNTRL     swctl;
    HSWITCH hswitch = 0;
    swctl.hwnd = hwnd;                     // window handle
    swctl.hwndIcon = hIcon;                // icon handle
    swctl.hprog = NULLHANDLE;              // program handle (use default)
    WinQueryWindowProcess(hwnd, &(swctl.idProcess), NULL);
                                           // process identifier
    swctl.idSession = 0;                   // session identifier ?
    swctl.uchVisibility = SWL_VISIBLE;     // visibility
    swctl.fbJump = SWL_JUMPABLE;           // jump indicator
    // get window title from window titlebar
    if (hwnd)
        WinQueryWindowText(hwnd, sizeof(swctl.szSwtitle), swctl.szSwtitle);
    swctl.bProgType = PROG_DEFAULT;        // program type
    hswitch = WinAddSwitchEntry(&swctl);

    // give the main window the icon
    if ((hwnd) && (hIcon))
        WinSendMsg(hwnd,
                   WM_SETICON,
                   (MPARAM)hIcon,
                   NULL);

    return hswitch;
}

/*
 *@@ winhUpdateTasklist:
 *      refreshes the task list entry for the given
 *      window with a new title text.
 *
 *@@added V1.0.1 (2003-01-25) [umoeller]
 */

BOOL winhUpdateTasklist(HWND hwnd,
                        PCSZ pcszNewTitle)
{
    HSWITCH hsw;
    if (hsw = WinQuerySwitchHandle(hwnd, 0))
    {
        SWCNTRL swc;
        WinQuerySwitchEntry(hsw, &swc);
        strhncpy0(swc.szSwtitle,
                  pcszNewTitle,
                  sizeof(swc.szSwtitle));
        return !WinChangeSwitchEntry(hsw, &swc);
    }

    return FALSE;
}

/*
 *@@category: Helpers\PM helpers\Miscellaneous
 */

/* ******************************************************************
 *
 *   Miscellaneous
 *
 ********************************************************************/

/*
 *@@ winhFree:
 *      frees a block of memory allocated by the
 *      winh* functions.
 *
 *      Since the winh* functions use malloc(),
 *      you can also use free() directly on such
 *      blocks. However, you must use winhFree
 *      if the winh* functions are in a module
 *      with a different C runtime.
 *
 *@@added V0.9.7 (2000-12-06) [umoeller]
 */

VOID winhFree(PVOID p)
{
    if (p)
        free(p);
}

/*
 *@@ winhSleep:
 *      sleeps at least the specified amount of time,
 *      without blocking the message queue.
 *
 *      NOTE: This function is a bit expensive because
 *      it creates a temporary object window. If you
 *      need to sleep several times, you should rather
 *      use a private timer.
 *
 *@@added V0.9.4 (2000-07-11) [umoeller]
 *@@changed V0.9.9 (2001-03-11) [umoeller]: rewritten
 */

VOID winhSleep(ULONG ulSleep)    // in: sleep time in milliseconds
{
    HWND    hwnd;

    if (hwnd = winhCreateObjectWindow(WC_STATIC, NULL))
    {
        QMSG    qmsg;
        HAB     hab;

        if (    (hab = WinQueryAnchorBlock(hwnd))
             && (WinStartTimer(hab,
                               hwnd,
                               1,
                               ulSleep))
           )
        {
            while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
            {
                if (    (qmsg.hwnd == hwnd)
                     && (qmsg.msg == WM_TIMER)
                     && (qmsg.mp1 == (MPARAM)1)     // timer ID
                   )
                    break;

                WinDispatchMsg(hab, &qmsg);
            }
            WinStopTimer(hab,
                         hwnd,
                         1);
        }
        else
            // timer creation failed:
            DosSleep(ulSleep);

        WinDestroyWindow(hwnd);
    }
    else
        DosSleep(ulSleep);
}

/*
 *@@ winhFileDlg:
 *      one-short function for opening an "Open" file
 *      dialog.
 *
 *      On input, pszFile specifies the directory and
 *      file specification (e.g. "F:\*.txt").
 *
 *      Returns TRUE if the user pressed OK. In that
 *      case, the fully qualified filename is written
 *      into pszFile again.
 *
 *      Returns FALSE if the user pressed Cancel.
 *
 *      Notes about flFlags:
 *
 *      -- WINH_FOD_SAVEDLG: display a "Save As" dialog.
 *         Otherwise an "Open" dialog is displayed.
 *
 *      -- WINH_FOD_INILOADDIR: load a directory from the
 *         specified INI key and switch the dlg to it.
 *         In that case, on input, pszFile must only
 *         contain the file filter without any path
 *         specification, because that is loaded from
 *         the INI key. If the INI key does not exist,
 *         the current process directory will be used.
 *
 *      -- WINH_FOD_INISAVEDIR: if the user presses OK,
 *         the directory of the selected file is written
 *         to the specified INI key so that it can be
 *         reused later. This flag is independent of
 *         WINH_FOD_INISAVEDIR: you can specify none,
 *         one, or both of them.
 *
 *@@added V0.9.3 (2000-04-29) [umoeller]
 *@@changed V0.9.12 (2001-05-21) [umoeller]: this failed if INI data had root dir, fixed
 */

BOOL winhFileDlg(HWND hwndOwner,    // in: owner for file dlg
                 PSZ pszFile,       // in: file mask; out: fully q'd filename
                                    //    (should be CCHMAXPATH in size)
                 ULONG flFlags,     // in: any combination of the following:
                                    // -- WINH_FOD_SAVEDLG: save dlg; else open dlg
                                    // -- WINH_FOD_INILOADDIR: load FOD path from INI
                                    // -- WINH_FOD_INISAVEDIR: store FOD path to INI on OK
                 HINI hini,         // in: INI file to load/store last path from (can be HINI_USER)
                 const char *pcszApplication, // in: INI application to load/store last path from
                 const char *pcszKey)        // in: INI key to load/store last path from
{
    FILEDLG fd;
    FILESTATUS3 fs3;

    memset(&fd, 0, sizeof(FILEDLG));
    fd.cbSize = sizeof(FILEDLG);
    fd.fl = FDS_CENTER;

    if (flFlags & WINH_FOD_SAVEDLG)
        fd.fl |= FDS_SAVEAS_DIALOG;
    else
        fd.fl |= FDS_OPEN_DIALOG;

    if (    (hini)
         && (flFlags & WINH_FOD_INILOADDIR)
         && (PrfQueryProfileString(hini,
                                   (PSZ)pcszApplication,
                                   (PSZ)pcszKey,
                                   "",      // default string V0.9.9 (2001-02-10) [umoeller]
                                   fd.szFullFile,
                                   sizeof(fd.szFullFile)-10)
                     > 2)
         // added these checks V0.9.12 (2001-05-21) [umoeller]
         && (!DosQueryPathInfo(fd.szFullFile,
                               FIL_STANDARD,
                               &fs3,
                               sizeof(fs3)))
         && (fs3.attrFile & FILE_DIRECTORY)
       )
    {
        // found: append "\*"
        strcat(fd.szFullFile, "\\");
        strcat(fd.szFullFile, pszFile);
    }
    else
        // default: copy pszFile
        strcpy(fd.szFullFile, pszFile);
        // fixed V0.9.12 (2001-05-21) [umoeller]

    if (    WinFileDlg(HWND_DESKTOP,    // parent
                       hwndOwner, // owner
                       &fd)
        && (fd.lReturn == DID_OK)
       )
    {
        // save path back?
        if (    (hini)
             && (flFlags & WINH_FOD_INISAVEDIR)
           )
        {
            // get the directory that was used
            PSZ p = strrchr(fd.szFullFile, '\\');
            if (p)
            {
                // contains directory:
                // copy to OS2.INI
                PSZ pszDir = strhSubstr(fd.szFullFile, p);
                if (pszDir)
                {
                    PrfWriteProfileString(hini,
                                          (PSZ)pcszApplication,
                                          (PSZ)pcszKey,
                                          pszDir);
                    free(pszDir);
                }
            }
        }

        strcpy(pszFile, fd.szFullFile);

        return TRUE;
    }

    return FALSE;
}

/*
 *@@ winhQueryWaitPointer:
 *      shortcut for getting the system "wait" pointer.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

HPOINTER winhQueryWaitPointer(VOID)
{
    return WinQuerySysPointer(HWND_DESKTOP,
                              SPTR_WAIT,
                              FALSE);   // no copy
}

/*
 *@@ winhSetWaitPointer:
 *      this sets the mouse pointer to "Wait".
 *      Returns the previous pointer (HPOINTER),
 *      which should be stored somewhere to be
 *      restored later. Example:
 +          HPOINTER hptrOld = winhSetWaitPointer();
 +          ...
 +          WinSetPointer(HWND_DESKTOP, hptrOld);
 */

HPOINTER winhSetWaitPointer(VOID)
{
    HPOINTER hptr = WinQueryPointer(HWND_DESKTOP);
    WinSetPointer(HWND_DESKTOP,
                  winhQueryWaitPointer());
    return hptr;
}

/*
 *@@ winhQueryWindowText2:
 *      this returns the window text of the specified
 *      HWND in a newly allocated buffer.
 *
 *      If pulExtra is specified, *pulExtra bytes will
 *      be allocated in addition to the window text
 *      length. Useful if you plan to append something
 *      to the string. On output, *pulExtra receives
 *      the string length excluding the extra bytes
 *      and the terminating null byte.
 *
 *      Returns NULL on error. Use free()
 *      to free the return value.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

PSZ winhQueryWindowText2(HWND hwnd,         // in: window whose text to query
                         PULONG pulExtra)   // in: extra bytes to allocate or NULL,
                                            // out: size of allocated buffer (including null byte)
{
    PSZ     pszText = NULL;
    ULONG   cbText;
    if (cbText = WinQueryWindowTextLength(hwnd))
    {
        ULONG cbExtra = 1;      // additional null character
        if (pulExtra)
            cbExtra += *pulExtra;

        if (pszText = (PSZ)malloc(cbText + cbExtra))
        {
            WinQueryWindowText(hwnd,
                               cbText + 1,
                               pszText);
            if (pulExtra)
                *pulExtra = cbText;
        }
    }

    return pszText;
}

/*
 *@@ winhQueryWindowText:
 *      this returns the window text of the specified
 *      HWND in a newly allocated buffer.
 *
 *      Returns NULL on error. Use free()
 *      to free the return value.
 *
 *@@changed V1.0.1 (2003-01-05) [umoeller]: now using winhQueryWindowText2
 */

PSZ winhQueryWindowText(HWND hwnd)
{
    return winhQueryWindowText2(hwnd, NULL);        // V1.0.1 (2003-01-05) [umoeller]
}

/*
 *@@ winhQueryDlgItemText2:
 *      shortcut around winhQueryWindowText2 to allow for
 *      specifying a dialog item ID instead.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

PSZ winhQueryDlgItemText2(HWND hwnd,
                          USHORT usItemID,
                          PULONG pulExtra)
{
    return winhQueryWindowText2(WinWindowFromID(hwnd, usItemID),
                                pulExtra);
}

/*
 *@@ winhSetWindowText:
 *      like WinSetWindowText, but this one accepts
 *      printf-like arguments.
 *
 *      Note that the total string is limited to
 *      1000 characters.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 */

BOOL winhSetWindowText(HWND hwnd,
                       const char *pcszFormat,
                       ...)
{
    CHAR szBuf[1000];
    va_list     args;
    int         i;
    va_start(args, pcszFormat);
    i = vsprintf(szBuf, pcszFormat, args);
    va_end(args);

    return WinSetWindowText(hwnd,
                            szBuf);
}

/*
 *@@ winhAppendWindowEllipseText:
 *      appends three dots ("...") to the title
 *      of the given window.
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

BOOL winhAppendWindowEllipseText(HWND hwnd)
{
    ULONG cbExtra = 3;
    PSZ psz;
    BOOL brc = FALSE;
    if (psz = winhQueryWindowText2(hwnd, &cbExtra))
    {
        memcpy(psz + cbExtra, "...", 4);
        brc = WinSetWindowText(hwnd, psz);
        free(psz);
    }

    return brc;
}

/*
 *@@ winhAppendDlgItemEllipseText:
 *
 *@@added V1.0.1 (2003-01-05) [umoeller]
 */

BOOL winhAppendDlgItemEllipseText(HWND hwnd,
                                  USHORT usItemID)
{
    return winhAppendWindowEllipseText(WinWindowFromID(hwnd, usItemID));
}

/*
 *@@ winhReplaceWindowText:
 *      this is a combination of winhQueryWindowText
 *      and strhFindReplace to replace substrings in a window.
 *
 *      This is useful for filling in placeholders
 *      a la "%1" in control windows, e.g. static
 *      texts.
 *
 *      This replaces only the first occurence of
 *      pszSearch.
 *
 *      Returns TRUE only if the window exists and
 *      the search string was replaced.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL winhReplaceWindowText(HWND hwnd,           // in: window whose text is to be modified
                           const char *pcszSearch,       // in: search string (e.g. "%1")
                           const char *pcszReplaceWith)  // in: replacement string for pszSearch
{
    BOOL    brc = FALSE;
    PSZ     pszText;

    if (pszText = winhQueryWindowText(hwnd))
    {
        ULONG ulOfs = 0;
        if (strhFindReplace(&pszText, &ulOfs, pcszSearch, pcszReplaceWith) > 0)
        {
            WinSetWindowText(hwnd, pszText);
            brc = TRUE;
        }

        free(pszText);
    }

    return brc;
}

/*
 *@@ winhEnableDlgItems:
 *      this enables/disables a whole range of controls
 *      in a window by enumerating the child windows
 *      until usIDFirst is found. If so, that subwindow
 *      is enabled/disabled and all the following windows
 *      in the enumeration also, until usIDLast is found.
 *
 *      Note that this affects _all_ controls following
 *      the usIDFirst window, no matter what ID they have
 *      (even if "-1"), until usIDLast is found.
 *
 *      Returns the no. of controls which were enabled/disabled
 *      (null if none).
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-20) [umoeller]: renamed from winhEnableDlgItems
 */

ULONG winhEnableControls(HWND hwndDlg,                  // in: dialog window
                         USHORT usIDFirst,              // in: first affected control ID
                         USHORT usIDLast,               // in: last affected  control ID (inclusive)
                         BOOL fEnable)                  // in: enable or disable?
{
    HENUM   henum1 = NULLHANDLE;
    HWND    hwndThis = NULLHANDLE;
    ULONG   ulCount = 0;

    henum1 = WinBeginEnumWindows(hwndDlg);
    while ((hwndThis = WinGetNextWindow(henum1)) != NULLHANDLE)
    {
        USHORT usIDCheckFirst = WinQueryWindowUShort(hwndThis, QWS_ID),
               usIDCheckLast;
        if (usIDCheckFirst == usIDFirst)
        {
            WinEnableWindow(hwndThis, fEnable);
            ulCount++;

            while ((hwndThis = WinGetNextWindow(henum1)) != NULLHANDLE)
            {
                WinEnableWindow(hwndThis, fEnable);
                ulCount++;
                usIDCheckLast = WinQueryWindowUShort(hwndThis, QWS_ID);
                if (usIDCheckLast == usIDLast)
                    break;
            }

            break;  // outer loop
        }
    }
    WinEndEnumWindows(henum1);

    return ulCount;
}

/*
 *@@ winhEnableControls2:
 *      like winhEnableControls, but instead this
 *      takes an array of ULONGs as input, which
 *      is assumed to contain the dialog IDs of
 *      the controls to be enabled/disabled.
 *
 *@@added V0.9.19 (2002-05-28) [umoeller]
 */

ULONG winhEnableControls2(HWND hwndDlg,             // in: dialog window
                          const ULONG *paulIDs,     // in: array of dialog IDs
                          ULONG cIDs,               // in: array item count (NOT array size)
                          BOOL fEnable)             // in: enable or disable?
{
    ULONG   ul,
            ulrc = 0;
    for (ul = 0;
         ul < cIDs;
         ++ul)
    {
        if (WinEnableControl(hwndDlg, paulIDs[ul], fEnable))
            ++ulrc;
    }

    return ulrc;
}

/*
 *@@ winhCreateStdWindow:
 *      much like WinCreateStdWindow, but this one
 *      allows you to have the standard window
 *      positioned automatically, using a given
 *      SWP structure (*pswpFrame).
 *
 *      The frame is created with the specified parent
 *      (usually HWND_DESKTOP), but no owner.
 *
 *      The client window is created with the frame as
 *      its parent and owner and gets an ID of FID_CLIENT.
 *
 *      Alternatively, you can set pswpFrame to NULL
 *      and specify FCF_SHELLPOSITION with flFrameCreateFlags.
 *      If you want the window to be shown, specify
 *      SWP_SHOW (and maybe SWP_ACTIVATE) in *pswpFrame.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.5 (2000-08-13) [umoeller]: flStyleClient never worked, fixed
 *@@changed V0.9.7 (2000-12-08) [umoeller]: fixed client calc for invisible window
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added support for NULL pcszClassClient
 */

HWND winhCreateStdWindow(HWND hwndFrameParent,      // in: normally HWND_DESKTOP
                         PSWP pswpFrame,            // in: frame wnd pos (ptr can be NULL)
                         ULONG flFrameCreateFlags,  // in: FCF_* flags
                         ULONG ulFrameStyle,        // in: WS_* flags (e.g. WS_VISIBLE, WS_ANIMATE)
                         const char *pcszFrameTitle, // in: frame title (title bar)
                         ULONG ulResourcesID,       // in: according to FCF_* flags
                         const char *pcszClassClient, // in: client class name (can be NULL for no client)
                         ULONG flStyleClient,       // in: client style
                         ULONG ulID,                // in: frame window ID
                         PVOID pClientCtlData,      // in: pCtlData structure pointer for client
                         PHWND phwndClient)         // out: created client wnd (required)
{
    FRAMECDATA  fcdata;
    HWND        hwndFrame;
    RECTL       rclClient;

    fcdata.cb            = sizeof(FRAMECDATA);
    fcdata.flCreateFlags = flFrameCreateFlags;
    fcdata.hmodResources = (HMODULE)NULL;
    fcdata.idResources   = ulResourcesID;

    // create the frame and client windows
    if (hwndFrame = WinCreateWindow(hwndFrameParent,
                                    WC_FRAME,
                                    (PSZ)pcszFrameTitle,
                                    ulFrameStyle,
                                    0,0,0,0,
                                    NULLHANDLE,
                                    HWND_TOP,
                                    ulID,
                                    &fcdata,
                                    NULL))
    {
        if (    (!pcszClassClient)          // V1.0.1 (2002-11-30) [umoeller]
             || (*phwndClient = WinCreateWindow(hwndFrame,      // parent
                                                (PSZ)pcszClassClient, // class
                                                NULL,           // no title
                                                flStyleClient,  // style
                                                0,0,0,0,        // size and position = 0
                                                hwndFrame,      // owner
                                                HWND_BOTTOM,    // bottom z-order
                                                FID_CLIENT,     // frame window ID
                                                pClientCtlData, // class data
                                                NULL))          // no presparams
           )
        {
            if (pswpFrame)
            {
                // position frame
                WinSetWindowPos(hwndFrame,
                                pswpFrame->hwndInsertBehind,
                                pswpFrame->x,
                                pswpFrame->y,
                                pswpFrame->cx,
                                pswpFrame->cy,
                                pswpFrame->fl);

                if (!pcszClassClient)
                    *phwndClient = NULLHANDLE;
                else
                {
                    // position client
                    // WinQueryWindowRect(hwndFrame, &rclClient);
                    // doesn't work because it might be invisible V0.9.7 (2000-12-08) [umoeller]
                    rclClient.xLeft = 0;
                    rclClient.yBottom = 0;
                    rclClient.xRight = pswpFrame->cx;
                    rclClient.yTop = pswpFrame->cy;
                    WinCalcFrameRect(hwndFrame,
                                     &rclClient,
                                     TRUE);     // calc client from frame
                    WinSetWindowPos(*phwndClient,
                                    HWND_TOP,
                                    rclClient.xLeft,
                                    rclClient.yBottom,
                                    rclClient.xRight - rclClient.xLeft,
                                    rclClient.yTop - rclClient.yBottom,
                                    SWP_MOVE | SWP_SIZE | SWP_SHOW);
                }
            }
        }
    }

    return hwndFrame;
}

/*
 *@@ winhCreateObjectWindow:
 *      creates an object window of the specified
 *      window class, which you should have registered
 *      before calling this. pvCreateParam will be
 *      given to the window on WM_CREATE.
 *
 *      Returns the HWND of the object window or
 *      NULLHANDLE on errors.
 *
 *@@added V0.9.3 (2000-04-17) [umoeller]
 *@@changed V0.9.7 (2001-01-17) [umoeller]: made this a function from a macro
 */

HWND winhCreateObjectWindow(const char *pcszWindowClass,    // in: PM window class name
                            PVOID pvCreateParam)            // in: create param
{
    return WinCreateWindow(HWND_OBJECT,
                           (PSZ)pcszWindowClass,
                           (PSZ)"",
                           0,
                           0,0,0,0,
                           0,
                           HWND_BOTTOM,
                           0,
                           pvCreateParam,
                           NULL);
}

/*
 *@@ winhCreateControl:
 *      creates a control with a size and position of 0.
 *
 *@@added V0.9.9 (2001-03-13) [umoeller]
 *@@changed V1.0.0 (2002-08-26) [umoeller]: added separate hwndOwner
 */

HWND winhCreateControl(HWND hwndParent,             // in: parent window
                       HWND hwndOwner,              // in: owner window
                       const char *pcszClass,       // in: window class (e.g. WC_BUTTON)
                       const char *pcszText,        // in: window title
                       ULONG ulStyle,               // in: control style
                       ULONG ulID)                  // in: control ID
{
    return WinCreateWindow(hwndParent,
                           (PSZ)pcszClass,
                           (PSZ)pcszText,
                           ulStyle,
                           0, 0, 0, 0,
                           hwndOwner,
                           HWND_TOP,
                           ulID,
                           NULL,
                           NULL);
}

/*
 *@@ winhSetParentAndOwner:
 *      switches owner _and_ parent of the given window.
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 */

BOOL winhSetParentAndOwner(HWND hwnd,           // in: window whose parent and owner to change
                           HWND hwndNewParent,  // in: new parent and owner
                           BOOL fRedraw)
{
    return (    WinSetParent(hwnd, hwndNewParent, fRedraw)
             && WinSetOwner(hwnd, hwndNewParent)
           );
}

/*
 *@@ winhRepaintWindows:
 *      this repaints all children of hwndParent.
 *      If this is passed as HWND_DESKTOP, the
 *      whole screen is repainted.
 *
 *@@changed V0.9.7 (2000-12-13) [umoeller]: hwndParent was never respected, fixed
 */

VOID winhRepaintWindows(HWND hwndParent)
{
    HWND    hwndTop;
    HENUM   henum = WinBeginEnumWindows(hwndParent);
    while ((hwndTop = WinGetNextWindow(henum)))
        if (WinIsWindowShowing(hwndTop))
            WinInvalidateRect(hwndTop, NULL, TRUE);
    WinEndEnumWindows(henum);
}

/*
 *@@ winhFindMsgQueue:
 *      returns the message queue which matches
 *      the given process and thread IDs. Since,
 *      per IBM definition, every thread may only
 *      have one MQ, this should be unique.
 *
 *@@added V0.9.2 (2000-03-08) [umoeller]
 */

HMQ winhFindMsgQueue(PID pid,           // in: process ID
                     TID tid,           // in: thread ID
                     HAB* phab)         // out: anchor block
{
    HWND    hwndThis = 0,
            rc = 0;
    HENUM   henum = WinBeginEnumWindows(HWND_OBJECT);
    while ((hwndThis = WinGetNextWindow(henum)))
    {
        CHAR    szClass[200];
        if (WinQueryClassName(hwndThis, sizeof(szClass), szClass))
        {
            if (!strcmp(szClass, "#32767"))
            {
                // message queue window:
                PID pidWin = 0;
                TID tidWin = 0;
                WinQueryWindowProcess(hwndThis,
                                      &pidWin,
                                      &tidWin);
                if (    (pidWin == pid)
                     && (tidWin == tid)
                   )
                {
                    // get HMQ from window words
                    if (rc = WinQueryWindowULong(hwndThis, QWL_HMQ))
                        if (phab)
                            *phab = WinQueryAnchorBlock(hwndThis);

                    break;
                }
            }
        }
    }
    WinEndEnumWindows(henum);

    return rc;
}

/*
 *@@ winhFindHardErrorWindow:
 *      this searches all children of HWND_OBJECT
 *      for the PM hard error windows, which are
 *      invisible most of the time. When a hard
 *      error occurs, that window is made a child
 *      of HWND_DESKTOP instead.
 *
 *      Stolen from ProgramCommander/2 (C) Roman Stangl.
 *
 *@@added V0.9.3 (2000-04-27) [umoeller]
 */

VOID winhFindPMErrorWindows(HWND *phwndHardError,  // out: hard error window
                            HWND *phwndSysError)   // out: system error window
{
    PID     pidObject;  // HWND_OBJECT's process and thread id
    TID     tidObject;
    PID     pidObjectChild;     // HWND_OBJECT's child window process and thread id
    TID     tidObjectChild;
    HENUM   henumObject;  // HWND_OBJECT enumeration handle
    HWND    hwndObjectChild;   // Window handle of current HWND_OBJECT child
    UCHAR   ucClassName[32];  // Window class e.g. #1 for WC_FRAME
    CLASSINFO classinfoWindow;  // Class info of current HWND_OBJECT child

    *phwndHardError = NULLHANDLE;
    *phwndSysError = NULLHANDLE;

    // query HWND_OBJECT's window process
    WinQueryWindowProcess(WinQueryObjectWindow(HWND_DESKTOP), &pidObject, &tidObject);
    // enumerate all child windows of HWND_OBJECT
    henumObject = WinBeginEnumWindows(HWND_OBJECT);
    while ((hwndObjectChild = WinGetNextWindow(henumObject)))
    {
        // see if the current HWND_OBJECT child window runs in the
        // process of HWND_OBJECT (PM)
        WinQueryWindowProcess(hwndObjectChild, &pidObjectChild, &tidObjectChild);
        if (pidObject == pidObjectChild)
        {
            // get the child window's data
            WinQueryClassName(hwndObjectChild,
                              sizeof(ucClassName),
                              (PCH)ucClassName);
            WinQueryClassInfo(WinQueryAnchorBlock(hwndObjectChild),
                              (PSZ)ucClassName,
                              &classinfoWindow);
            if (    (!strcmp((PSZ)ucClassName, "#1")
                 || (classinfoWindow.flClassStyle & CS_FRAME))
               )
            {
                // if the child window is a frame window and running in
                // HWND_OBJECT's (PM's) window process, it must be the
                // PM Hard Error or System Error window
                WinQueryClassName(WinWindowFromID(hwndObjectChild,
                                                  FID_CLIENT),
                                  sizeof(ucClassName),
                                  (PSZ)ucClassName);
                if (!strcmp((PSZ)ucClassName, "PM Hard Error"))
                {
                    *phwndHardError = hwndObjectChild;
                    if (*phwndSysError)
                        // we found the other one already:
                        // stop searching, we got both
                        break;
                }
                else
                {
                    printf("Utility: Found System Error %08X\n", (int)hwndObjectChild);
                    *phwndSysError = hwndObjectChild;
                    if (*phwndHardError)
                        // we found the other one already:
                        // stop searching, we got both
                        break;
                }
            }
        } // end if (pidObject == pidObjectChild)
    } // end while ((hwndObjectChild = WinGetNextWindow(henumObject)) != NULLHANDLE)
    WinEndEnumWindows(henumObject);
}

/*
 *@@ winhCreateFakeDesktop:
 *      this routine creates and displays a frameless window over
 *      the whole screen in the color of PM's Desktop to fool the
 *      user that all windows have been closed (which in fact might
 *      not be the case).
 *
 *      This window's background color is set to the Desktop's
 *      (PM's one, not the WPS's one).
 *
 *      Returns the HWND of this window.
 */

HWND winhCreateFakeDesktop(HWND hwndSibling)
{
    // presparam for background
    typedef struct _BACKGROUND
    {
        ULONG   cb;     // length of the aparam parameter, in bytes
        ULONG   id;     // attribute type identity
        ULONG   cb2;    // byte count of the ab parameter
        RGB     rgb;    // attribute value
    } BACKGROUND;

    BACKGROUND  background;
    LONG        lDesktopColor;

    // create fake desktop window = empty window with
    // the size of full screen
    lDesktopColor = WinQuerySysColor(HWND_DESKTOP,
                                     SYSCLR_BACKGROUND,
                                     0);
    background.cb = sizeof(background.id)
                  + sizeof(background.cb)
                  + sizeof(background.rgb);
    background.id = PP_BACKGROUNDCOLOR;
    background.cb2 = sizeof(RGB);
    background.rgb.bBlue = (CHAR1FROMMP(lDesktopColor));
    background.rgb.bGreen= (CHAR2FROMMP(lDesktopColor));
    background.rgb.bRed  = (CHAR3FROMMP(lDesktopColor));

    return WinCreateWindow(HWND_DESKTOP,  // parent window
                           WC_FRAME,      // class name
                           "",            // window text
                           WS_VISIBLE,    // window style
                           0, 0,          // position and size
                           G_cxScreen,
                           G_cyScreen,
                           NULLHANDLE,    // owner window
                           hwndSibling,   // sibling window
                           1,             // window id
                           NULL,          // control data
                           &background); // presentation parms
}

/*
 *@@ winhAssertWarp4Notebook:
 *      this takes hwndDlg as a notebook dialog page and
 *      goes thru all its controls. If a control with an
 *      ID <= udIdThreshold is found, this is assumed to
 *      be a button which is to be given the BS_NOTEBOOKBUTTON
 *      style. You should therefore give all your button
 *      controls which should be moved such an ID.
 *
 *      Note that this function will now automatically
 *      find out the lowest y coordinate that was used
 *      for a non-notebook button and move all controls
 *      down accordingly. As a result, ulDownUnit must
 *      no longer be specified (V0.9.19).
 *
 *      This function is useful if you wish to create
 *      notebook pages using dlgedit.exe which are compatible
 *      with both Warp 3 and Warp 4. This should be executed
 *      in WM_INITDLG of the notebook dlg function if the app
 *      has determined that it is running on Warp 4.
 *
 *@@changed V0.9.16 (2002-02-02) [umoeller]: fixed entry fields
 *@@changed V0.9.19 (2002-04-24) [umoeller]: removed ulDownUnits
 *@@changed V0.9.19 (2002-05-02) [umoeller]: fix for combobox
 */

BOOL winhAssertWarp4Notebook(HWND hwndDlg,
                             USHORT usIdThreshold)  // in: ID threshold
{
    BOOL brc = FALSE;

    if (doshIsWarp4())
    {
        LONG    yLowest = 10000;
        HWND    hwndItem;
        HENUM   henum = 0;
        PSWP    paswp,
                pswpThis;
        ULONG   cWindows = 0,
                ul;

        BOOL    fIsVisible;

        if (fIsVisible = WinIsWindowVisible(hwndDlg))
            // avoid flicker
            WinEnableWindowUpdate(hwndDlg, FALSE);

        if (paswp = (PSWP)malloc(sizeof(SWP) * 100))
        {
            pswpThis = paswp;

            // loop 1: set notebook buttons, find lowest y used
            henum = WinBeginEnumWindows(hwndDlg);
            while ((hwndItem = WinGetNextWindow(henum)))
            {
                USHORT usId = WinQueryWindowUShort(hwndItem, QWS_ID);
                // _Pmpf(("hwndItem: 0x%lX, ID: 0x%lX", hwndItem, usId));
                if (usId <= usIdThreshold)
                {
                    // pushbutton to change:
                    WinSetWindowBits(hwndItem,
                                     QWL_STYLE,
                                     BS_NOTEBOOKBUTTON, BS_NOTEBOOKBUTTON);
                    brc = TRUE;
                }
                else
                {
                    // no pushbutton to change:
                    CHAR szClass[10];

                    WinQueryWindowPos(hwndItem, pswpThis);

                    // special handling for entry fields
                    // V0.9.16 (2002-02-02) [umoeller]
                    WinQueryClassName(hwndItem, sizeof(szClass), szClass);
                    if (!strcmp(szClass, "#6"))
                    {
                        pswpThis->x += 3 * WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER);
                        pswpThis->y += 3 * WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);
                    }

                    // check lowest y
                    if (    (pswpThis->y < yLowest)
                         // ignore combobox, this will distort everything
                         // AGAIN ... sigh V0.9.19 (2002-05-02) [umoeller]
                         && (strcmp(szClass, "#2"))
                       )
                        yLowest = pswpThis->y ;

                    ++pswpThis;
                    if (++cWindows == 100)
                        break;
                }
            } // end while ((hwndItem = WinGetNextWindow(henum)))
            WinEndEnumWindows(henum);

            // now adjust window positions
            pswpThis = paswp;
            for (ul = 0;
                 ul < cWindows;
                 ++ul, ++pswpThis)
            {
                pswpThis->y -= (yLowest - 8);
                            // 8 is magic to match the lower border of the
                            // standard WPS notebook pages V0.9.19 (2002-04-24) [umoeller]
                pswpThis->fl = SWP_MOVE;
            }

            WinSetMultWindowPos(WinQueryAnchorBlock(hwndDlg),
                                paswp,
                                cWindows);

            free(paswp);
        }

        if (fIsVisible)
            WinShowWindow(hwndDlg, TRUE);
    }

    return brc;
}

/*
 *@@ winhDrawFormattedText:
 *      this func takes a rectangle and draws pszText into
 *      it, breaking the words as neccessary. The line spacing
 *      is determined from the font currently selected in hps.
 *
 *      As opposed to WinDrawText, this can draw several lines
 *      at once, and format the _complete_ text according to the
 *      flCmd parameter, which is like with WinDrawText.
 *
 *      After this function returns, *prcl is modified like this:
 *
 *      -- yTop and yBottom contain the upper and lower boundaries
 *         which were needed to draw the text. This depends on
 *         whether DT_TOP etc. were specified.
 *         To get the height of the rectangle used, calculate the
 *         delta between yTop and yBottom.
 *
 *      -- xLeft and xRight are modified to contain the outmost
 *         left and right coordinates which were needed to draw
 *         the text. This will be set to the longest line which
 *         was encountered.
 *
 *      The following DT_* flags are supported:
 *
 *      --  DT_LEFT, DT_CENTER, DT_RIGHT all work.
 *
 *      --  Vertically however only DT_TOP is supported.
 *
 *      --  You can specify DT_QUERYEXTENT to only have
 *          these text boundaries calculated without actually
 *          drawing.
 *
 *      Note that DT_TEXTATTRS will always be added, so you
 *      will want to call GpiSetColor before this.
 *
 *      This returns the number of lines drawn.
 *
 *@@changed V0.9.0 [umoeller]: prcl.xLeft and xRight are now updated too upon return
 */

ULONG winhDrawFormattedText(HPS hps,     // in: presentation space; its settings
                                         // are used, but not altered
                            PRECTL prcl, // in/out: rectangle to use for drawing
                                         // (modified)
                            const char *pcszText, // in: text to draw (zero-terminated)
                            ULONG flCmd) // in: DT_* flags like in WinDrawText; see remarks
{
    PSZ     p = (PSZ)pcszText;
    LONG    lDrawn = 1,
            lTotalDrawn = 0,
            lLineCount = 0,
            lOrigYTop = prcl->yTop;
    ULONG   ulTextLen = strlen(pcszText),
            ulCharHeight,
            flCmd2,
            xLeftmost = prcl->xRight,
            xRightmost = prcl->xLeft;
    RECTL   rcl2;

    flCmd2 = flCmd | DT_WORDBREAK | DT_TEXTATTRS;

    ulCharHeight = gpihQueryLineSpacing(hps);

    while (    (lDrawn)
            && (lTotalDrawn < ulTextLen)
          )
    {
        memcpy(&rcl2, prcl, sizeof(rcl2));
        lDrawn = WinDrawText(hps,
                             ulTextLen - lTotalDrawn,
                             p,
                             &rcl2,
                             0,
                             0,                       // colors
                             flCmd2);

        // update char counters
        p += lDrawn;
        lTotalDrawn += lDrawn;

        // update x extents
        if (rcl2.xLeft < xLeftmost)
            xLeftmost = rcl2.xLeft;
        if (rcl2.xRight > xRightmost)
            xRightmost = rcl2.xRight;

        // update y for next line
        prcl->yTop -= ulCharHeight;

        // increase line count
        lLineCount++;
    }

    prcl->xLeft = xLeftmost;
    prcl->xRight = xRightmost;
    prcl->yBottom = prcl->yTop;
    prcl->yTop = lOrigYTop;

    return lLineCount;
}

/*
 *@@ winhQuerySwitchList:
 *      returns the switch list in a newly
 *      allocated buffer. This does the
 *      regular double WinQuerySwitchList
 *      call to first get the no. of items
 *      and then get the items.
 *
 *      The no. of items can be found in
 *      the returned SWBLOCK.cwsentry.
 *
 *      Returns NULL on errors. Use
 *      free() to free the return value.
 *
 *@@added V0.9.7 (2000-12-06) [umoeller]
 */

PSWBLOCK winhQuerySwitchList(HAB hab)
{
    ULONG       cItems = WinQuerySwitchList(hab, NULL, 0);
    ULONG       ulBufSize = (cItems * sizeof(SWENTRY)) + sizeof(HSWITCH);
    PSWBLOCK    pSwBlock;
    if (pSwBlock = (PSWBLOCK)malloc(ulBufSize))
    {
        if (!(cItems = WinQuerySwitchList(hab, pSwBlock, ulBufSize)))
        {
            free(pSwBlock);
            pSwBlock = NULL;
        }
    }

    return pSwBlock;
}

typedef HSWITCH APIENTRY WINHSWITCHFROMHAPP(HAPP happ);

/*
 *@@ winhHSWITCHfromHAPP:
 *      resolves and calls the undocumented
 *      WinHSWITCHfromHAPP API.
 *
 *@@added V0.9.19 (2002-04-17) [umoeller]
 */

HSWITCH winhHSWITCHfromHAPP(HAPP happ)
{
    static WINHSWITCHFROMHAPP *pWinHSWITCHfromHAPP = NULL;

    if (!pWinHSWITCHfromHAPP)
        // first call: import WinHSWITCHfromHAPP
        // WinHSWITCHfromHAPP PMMERGE.5199
        doshQueryProcAddr("PMMERGE",
                          5199,
                          (PFN*)&pWinHSWITCHfromHAPP);

    if (pWinHSWITCHfromHAPP)
        return pWinHSWITCHfromHAPP(happ);

    return NULLHANDLE;
}

/*
 *@@ winhQueryTasklistWindow:
 *      returns the window handle of the PM task list.
 *
 *@@added V0.9.7 (2000-12-07) [umoeller]
 */

HWND winhQueryTasklistWindow(VOID)
{
    SWBLOCK  swblock;

    // the tasklist has entry #0 in the SWBLOCK
    WinQuerySwitchList(NULLHANDLE, &swblock, sizeof(SWBLOCK));

    return swblock.aswentry[0].swctl.hwnd;
}

/*
 *@@ winhKillTasklist:
 *      this will destroy the Tasklist (window list) window.
 *      Note: you will only be able to get it back after a
 *      reboot, not a WPS restart. Only for use at shutdown and such.
 *      This trick by Uri J. Stern at
 *      http://zebra.asta.fh-weingarten.de/os2/Snippets/Howt8881.HTML
 */

VOID winhKillTasklist(VOID)
{
    HWND     hwndTasklist = winhQueryTasklistWindow();
    WinPostMsg(hwndTasklist,
               0x0454,     // undocumented msg for killing tasklist
               NULL, NULL);
}

// the following must be added for EMX (99-10-22) [umoeller]
#ifndef NERR_BufTooSmall
      #define NERR_BASE       2100
      #define NERR_BufTooSmall        (NERR_BASE+23)
            // the API return buffer is too small
#endif

/*
 *@@ winhQueryPendingSpoolJobs:
 *      returns the number of pending print jobs in the spooler
 *      or 0 if none. Useful for testing before shutdown.
 */

ULONG winhQueryPendingSpoolJobs(VOID)
{
    // BOOL    rcPending = FALSE;
    ULONG       ulTotalJobCount = 0;

    SPLERR      splerr;
    USHORT      jobCount;
    ULONG       cbBuf;
    ULONG       cTotal;
    ULONG       cReturned;
    ULONG       cbNeeded;
    ULONG       ulLevel;
    ULONG       i,j;
    PSZ         pszComputerName;
    PBYTE       pBuf = NULL;
    PPRQINFO3   prq;
    PPRJINFO2   prj2;

    ulLevel = 4L;
    pszComputerName = (PSZ)NULL;
    splerr = SplEnumQueue(pszComputerName, ulLevel, pBuf, 0L, // cbBuf
                          &cReturned, &cTotal,
                          &cbNeeded, NULL);
    if (    (splerr == ERROR_MORE_DATA)
         || (splerr == NERR_BufTooSmall)
       )
    {
        if (!DosAllocMem((PPVOID)&pBuf,
                         cbNeeded,
                         PAG_READ | PAG_WRITE | PAG_COMMIT))
        {
            cbBuf = cbNeeded;
            splerr = SplEnumQueue(pszComputerName, ulLevel, pBuf, cbBuf,
                                  &cReturned, &cTotal,
                                  &cbNeeded, NULL);
            if (splerr == NO_ERROR)
            {
                // set pointer to point to the beginning of the buffer
                prq = (PPRQINFO3)pBuf;

                // cReturned has the count of the number of PRQINFO3 structures
                for (i = 0;
                     i < cReturned;
                     i++)
                {
                    // save the count of jobs; there are this many PRJINFO2
                    // structures following the PRQINFO3 structure
                    jobCount = prq->cJobs;
                    // _Pmpf(( "Job count in this queue is %d",jobCount ));

                    // increment the pointer past the PRQINFO3 structure
                    prq++;

                    // set a pointer to point to the first PRJINFO2 structure
                    prj2=(PPRJINFO2)prq;
                    for (j = 0;
                         j < jobCount;
                         j++)
                    {
                        // increment the pointer to point to the next structure
                        prj2++;
                        // increase the job count, which we'll return
                        ulTotalJobCount++;

                    } // endfor jobCount

                    // after doing all the job structures, prj2 points to the next
                    // queue structure; set the pointer for a PRQINFO3 structure
                    prq = (PPRQINFO3)prj2;
                } //endfor cReturned
            } // endif NO_ERROR
            DosFreeMem(pBuf);
        }
    } // end if Q level given

    return ulTotalJobCount;
}

/*
 *@@ winhSetNumLock:
 *      this sets the NumLock key on or off, depending
 *      on fState.
 *
 *      Based on code from WarpEnhancer, (C) Achim Hasenmller.
 *
 *@@added V0.9.1 (99-12-18) [umoeller]
 *@@changed V1.0.6 (2006-09-30) [pr]: Set Keyboard LEDs to match @@fixes 831
 */

VOID winhSetNumLock(BOOL fState)
{
    // BOOL  fRestoreKBD = FALSE;  //  Assume we're not going to close Kbd
    BYTE KeyStateTable[256];
    ULONG ulActionTaken; //  Used by DosOpen
    HFILE hKbd;

    // read keyboard state table
    if (WinSetKeyboardStateTable(HWND_DESKTOP, &KeyStateTable[0],
                                 FALSE))
    {
        // first set the PM state
        if (fState)
            KeyStateTable[VK_NUMLOCK] |= 0x01; //  Turn numlock on
        else
            KeyStateTable[VK_NUMLOCK] &= 0xFE; //  Turn numlock off

        // set keyboard state table with new state values
        WinSetKeyboardStateTable(HWND_DESKTOP, &KeyStateTable[0], TRUE);
    }

    // now set the OS/2 keyboard state

    // try to open OS/2 keyboard driver
    if (!DosOpen("KBD$",
                 &hKbd, &ulActionTaken,
                 0,     // cbFile
                 FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
                 NULL))
    {
        SHIFTSTATE ShiftState;
        USHORT usLEDState;
        ULONG DataLen = sizeof(SHIFTSTATE);

        memset(&ShiftState, '\0', DataLen);
        DosDevIOCtl(hKbd, IOCTL_KEYBOARD, KBD_GETSHIFTSTATE,
                    NULL, 0L, NULL,
                    &ShiftState, DataLen, &DataLen);

        if (fState)
            ShiftState.fsState |= 0x0020; // turn NumLock on
        else
            ShiftState.fsState &= 0xFFDF; // turn NumLock off

        DosDevIOCtl(hKbd, IOCTL_KEYBOARD, KBD_SETSHIFTSTATE,
                    &ShiftState, DataLen, &DataLen,
                    NULL, 0L, NULL);

        // XWP V1.0.6 (2006-09-30) [pr]: Set Keyboard LEDs to match @@fixes 831
        usLEDState = (ShiftState.fsState & (SCROLLLOCK_ON | NUMLOCK_ON | CAPSLOCK_ON)) >> 4;
        DataLen = sizeof(usLEDState);
        DosDevIOCtl(hKbd, IOCTL_KEYBOARD, KBD_ALTERKBDLED,
                    &usLEDState, DataLen, &DataLen,
                    NULL, 0L, NULL);

        // now close OS/2 keyboard driver
        DosClose(hKbd);
    }
}

/*
 *@@ winhSetClipboardText:
 *      sets the clipboard data to the given text,
 *      replacing the current clipboard contents.
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 */

BOOL winhSetClipboardText(HAB hab,
                          PCSZ pcsz,
                          ULONG cbSize)     // in: size of buffer INCLUDING null byte
{
    BOOL    fSuccess = FALSE;

    if (WinOpenClipbrd(hab))
    {
        PSZ pszDest;
        if (!DosAllocSharedMem((PVOID*)&pszDest,
                               NULL,
                               cbSize,
                               PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE))
        {
            memcpy(pszDest,
                   pcsz,
                   cbSize);

            WinEmptyClipbrd(hab);

            fSuccess = WinSetClipbrdData(hab,       // anchor-block handle
                                         (ULONG)pszDest, // pointer to text data
                                         CF_TEXT,        // data is in text format
                                         CFI_POINTER);   // passing a pointer

            // PMREF says (implicitly) it is not necessary to call
            // DosFreeMem. I hope that is correct.
            // V0.9.19 (2002-06-02) [umoeller]
        }

        WinCloseClipbrd(hab);
    }

    return fSuccess;
}

/*
 *@@category: Helpers\PM helpers\Workplace Shell\WPS class list
 */

/* ******************************************************************
 *
 *   WPS Class List helpers
 *
 ********************************************************************/

/*
 *@@ winhQueryWPSClassList:
 *      this returns the WPS class list in a newly
 *      allocated buffer. This is just a shortcut to
 *      the usual double WinEnumObjectClasses call.
 *
 *      The return value is actually of the POBJCLASS type,
 *      so you better cast this manually. We declare this
 *      this as PBYTE though because POBJCLASS requires
 *      INCL_WINWORKPLACE.
 *      See WinEnumObjectClasses() for details.
 *
 *      Returns NULL on error. Use free()
 *      to free the return value.
 *
 *@@added V0.9.0 [umoeller]
 */

PBYTE winhQueryWPSClassList(VOID)
{
    ULONG       ulSize;
    POBJCLASS   pObjClass = 0;

    // get WPS class list size
    if (WinEnumObjectClasses(NULL, &ulSize))
    {
        // allocate buffer
        pObjClass = (POBJCLASS)malloc(ulSize + 1);
        // and load the classes into it
        WinEnumObjectClasses(pObjClass, &ulSize);
    }

    return (PBYTE)pObjClass;
}

/*
 *@@ winhQueryWPSClass:
 *      this returns the POBJCLASS item if pszClass is registered
 *      with the WPS or NULL if the class could not be found.
 *
 *      The return value is actually of the POBJCLASS type,
 *      so you better cast this manually. We declare this
 *      this as PBYTE though because POBJCLASS requires
 *      INCL_WINWORKPLACE.
 *
 *      This takes as input the return value of winhQueryWPSClassList,
 *      which you must call first.
 *
 *      <B>Usage:</B>
 +          PBYTE   pClassList = winhQueryWPSClassList(),
 +                  pWPFolder;
 +          if (pClassList)
 +          {
 +              if (pWPFolder = winhQueryWPSClass(pClassList, "WPFolder"))
 +                  ...
 +              free(pClassList);
 +          }
 *
 *@@added V0.9.0 [umoeller]
 */

PBYTE winhQueryWPSClass(PBYTE pObjClass,  // in: buffer returned by
                                          // winhQueryWPSClassList
                        const char *pszClass)     // in: class name to query
{
    PBYTE   pbReturn = 0;

    POBJCLASS pocThis = (POBJCLASS)pObjClass;
    // now go thru the WPS class list
    while (pocThis)
    {
        if (!strcmp(pocThis->pszClassName, pszClass))
        {
            pbReturn = (PBYTE)pocThis;
            break;
        }

        // next class
        pocThis = pocThis->pNext;
    } // end while (pocThis)

    return pbReturn;
}

/*
 *@@ winhRegisterClass:
 *      this works just like WinRegisterObjectClass,
 *      except that it returns a more meaningful
 *      error code than just FALSE in case registering
 *      fails.
 *
 *      This returns NO_ERROR if the class was successfully
 *      registered (WinRegisterObjectClass returned TRUE).
 *
 *      Otherwise, we do a DosLoadModule if maybe the DLL
 *      couldn't be loaded in the first place. If DosLoadModule
 *      did not return NO_ERROR, this function returns that
 *      return code, which can be:
 *
 *      --  2   ERROR_FILE_NOT_FOUND: pcszModule does not exist
 *      --  2   ERROR_FILE_NOT_FOUND
 *      --  3   ERROR_PATH_NOT_FOUND
 *      --  4   ERROR_TOO_MANY_OPEN_FILES
 *      --  5   ERROR_ACCESS_DENIED
 *      --  8   ERROR_NOT_ENOUGH_MEMORY
 *      --  11  ERROR_BAD_FORMAT
 *      --  26  ERROR_NOT_DOS_DISK (unknown media type)
 *      --  32  ERROR_SHARING_VIOLATION
 *      --  33  ERROR_LOCK_VIOLATION
 *      --  36  ERROR_SHARING_BUFFER_EXCEEDED
 *      --  95  ERROR_INTERRUPT (interrupted system call)
 *      --  108 ERROR_DRIVE_LOCKED (by another process)
 *      --  123 ERROR_INVALID_NAME (illegal character or FS name not valid)
 *      --  127 ERROR_PROC_NOT_FOUND (DosQueryProcAddr error)
 *      --  180 ERROR_INVALID_SEGMENT_NUMBER
 *      --  182 ERROR_INVALID_ORDINAL
 *      --  190 ERROR_INVALID_MODULETYPE (probably an application)
 *      --  191 ERROR_INVALID_EXE_SIGNATURE (probably not LX DLL)
 *      --  192 ERROR_EXE_MARKED_INVALID (by linker)
 *      --  194 ERROR_ITERATED_DATA_EXCEEDS_64K (in a DLL segment)
 *      --  195 ERROR_INVALID_MINALLOCSIZE
 *      --  196 ERROR_DYNLINK_FROM_INVALID_RING
 *      --  198 ERROR_INVALID_SEGDPL
 *      --  199 ERROR_AUTODATASEG_EXCEEDS_64K
 *      --  201 ERROR_RELOCSRC_CHAIN_EXCEEDS_SEGLIMIT
 *      --  206 ERROR_FILENAME_EXCED_RANGE (not matching 8+3 spec)
 *      --  295 ERROR_INIT_ROUTINE_FAILED (DLL init routine failed)
 *
 *      In all these cases, pszBuf may contain a meaningful
 *      error message from DosLoadModule, especially if an import
 *      could not be resolved.
 *
 *      Still worse, if DosLoadModule returned NO_ERROR, we
 *      probably have some SOM internal error. A probable
 *      reason is that the parent class of pcszClassName
 *      is not installed, but that's WPS/SOM internal
 *      and cannot be queried from outside the WPS context.
 *
 *      In that case, ERROR_OPEN_FAILED (110) is returned.
 *      That one sounded good to me. ;-)
 */

APIRET winhRegisterClass(const char* pcszClassName, // in: e.g. "XFolder"
                         const char* pcszModule,    // in: e.g. "C:\XFOLDER\XFLDR.DLL"
                         PSZ pszBuf,                // out: error message from DosLoadModule
                         ULONG cbBuf)               // in: sizeof(*pszBuf), passed to DosLoadModule
{
    APIRET arc = NO_ERROR;

    if (!WinRegisterObjectClass((PSZ)pcszClassName, (PSZ)pcszModule))
    {
        // failed: do more error checking then, try DosLoadModule
        HMODULE hmod = NULLHANDLE;
        if (!(arc = DosLoadModule(pszBuf, cbBuf,
                                  (PSZ)pcszModule,
                                  &hmod)))
        {
            // DosLoadModule succeeded:
            // some SOM error then
            DosFreeModule(hmod);
            arc = ERROR_OPEN_FAILED;
        }
    }
    // else: ulrc still 0 (== no error)

    return arc;
}

/*
 *@@ winhIsClassRegistered:
 *      quick one-shot function which checks if
 *      a class is currently registered. Calls
 *      winhQueryWPSClassList and winhQueryWPSClass
 *      in turn.
 *
 *@@added V0.9.2 (2000-02-26) [umoeller]
 */

BOOL winhIsClassRegistered(const char *pcszClass)
{
    BOOL    brc = FALSE;
    PBYTE   pClassList;

    if (pClassList = winhQueryWPSClassList())
    {
        if (winhQueryWPSClass(pClassList, pcszClass))
            brc = TRUE;

        free(pClassList);
    }

    return brc;
}

/*
 *@@category: Helpers\PM helpers\Workplace Shell
 */

/*
 *@@ winhResetWPS:
 *      restarts the WPS using PrfReset. Returns
 *      one of the following:
 *
 *      -- 0: no error.
 *      -- 1: PrfReset failed.
 *      -- 2 or 4: PrfQueryProfile failed.
 *      -- 3: malloc() failed.
 *
 *@@added V0.9.4 (2000-07-01) [umoeller]
 *@@changed V1.0.5 (2005-02-17) [pr]: replaced this with something less brutal
 */

ULONG winhResetWPS(HAB hab)
{
    ULONG ulrc = 0;

#if 1
    WinRestartWorkplace();
#else
    // find out current profile names
    PRFPROFILE Profiles;
    Profiles.cchUserName = Profiles.cchSysName = 0;
    // first query their file name lengths
    if (PrfQueryProfile(hab, &Profiles))
    {
        // allocate memory for filenames
        Profiles.pszUserName  = (PSZ)malloc(Profiles.cchUserName);
        Profiles.pszSysName  = (PSZ)malloc(Profiles.cchSysName);

        if (Profiles.pszSysName)
        {
            // get filenames
            if (PrfQueryProfile(hab, &Profiles))
            {

                // "change" INIs to these filenames:
                // THIS WILL RESET THE WPS
                if (PrfReset(hab, &Profiles) == FALSE)
                    ulrc = 1;
                free(Profiles.pszSysName);
                free(Profiles.pszUserName);
            }
            else
                ulrc = 2;
        }
        else
            ulrc = 3;
    }
    else
        ulrc = 4;
#endif

    return ulrc;
}
