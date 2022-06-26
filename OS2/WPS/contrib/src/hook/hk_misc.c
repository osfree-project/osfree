
/*
 *@@sourcefile hk_misc.c:
 *
 *@@added V0.9.12 (2001-05-27) [umoeller]
 *@@header "hook\hook_private.h"
 */

/*
 *      Copyright (C) 1999-2003 Ulrich Mîller.
 *      Copyright (C) 1993-1999 Roman Stangl.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINMENUS
#define INCL_WINSCROLLBARS
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINHOOKS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>

// PMPRINTF in hooks is a tricky issue;
// avoid this unless this is really needed.
// If enabled, NEVER give the PMPRINTF window
// the focus, or your system will hang solidly...
#define DONTDEBUGATALL
#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"

#include "xwpapi.h"                     // public XWorkplace definitions

#include "hook\xwphook.h"
#include "hook\hook_private.h"          // private hook and daemon definitions
#include "hook\xwpdaemn.h"

#pragma hdrstop

/******************************************************************
 *
 *  Input hook -- miscellaneous processing
 *
 ******************************************************************/

/*
 *@@ WMButton_SystemMenuContext:
 *      this gets called from hookInputHook upon
 *      MB2 clicks to copy a window's system menu
 *      and display it as a popup menu on the
 *      title bar.
 *
 *      Based on code from WarpEnhancer, (C) Achim HasenmÅller.
 */

VOID WMButton_SystemMenuContext(HWND hwnd)     // of WM_BUTTON2CLICK
{
    POINTL      ptlMouse; // mouse coordinates
    HWND        hwndFrame; // handle of the frame window (parent)

    // get mouse coordinates (absolute coordinates)
    WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
    // query parent of title bar window (frame window)
    if (hwndFrame = WinQueryWindow(hwnd, QW_PARENT))
    {
        // query handle of system menu icon (action bar style)
        HWND hwndSysMenuIcon;
        if (hwndSysMenuIcon = WinWindowFromID(hwndFrame, FID_SYSMENU))
        {
            HWND        hNewMenu; // handle of our copied menu
            HWND        SysMenuHandle;

            // query item id of action bar system menu
            SHORT id = (SHORT)(USHORT)(ULONG)WinSendMsg(hwndSysMenuIcon,
                                                        MM_ITEMIDFROMPOSITION,
                                                        MPFROMSHORT(0), 0);
            // query item id of action bar system menu
            MENUITEM    mi = {0};
            CHAR        szItemText[100]; // buffer for menu text
            WinSendMsg(hwndSysMenuIcon, MM_QUERYITEM,
                       MPFROM2SHORT(id, FALSE),
                       MPFROMP(&mi));
            // submenu is our system menu
            SysMenuHandle = mi.hwndSubMenu;

            // create a new empty menu
            if (hNewMenu = WinCreateMenu(HWND_OBJECT, NULL))
            {
                // query how menu entries the original system menu has
                SHORT SysMenuItems = (SHORT)WinSendMsg(SysMenuHandle,
                                                       MM_QUERYITEMCOUNT,
                                                       0, 0);
                ULONG i;
                // loop through all entries in the original system menu
                for (i = 0; i < SysMenuItems; i++)
                {
                    id = (SHORT)(USHORT)(ULONG)WinSendMsg(SysMenuHandle,
                                                          MM_ITEMIDFROMPOSITION,
                                                          MPFROMSHORT(i),
                                                          0);
                    // get this menu item into mi buffer
                    WinSendMsg(SysMenuHandle,
                               MM_QUERYITEM,
                               MPFROM2SHORT(id, FALSE),
                               MPFROMP(&mi));
                    // query text of this menu entry into our buffer
                    WinSendMsg(SysMenuHandle,
                               MM_QUERYITEMTEXT,
                               MPFROM2SHORT(id, sizeof(szItemText)-1),
                               MPFROMP(szItemText));
                    // add this entry to our new menu
                    WinSendMsg(hNewMenu,
                               MM_INSERTITEM,
                               MPFROMP(&mi),
                               MPFROMP(szItemText));
                }

#ifndef __NOPAGER__
                // add the sticky entry if it's a top-level window
                // V1.0.0 (2002-09-14) [lafaix]
                if (    (WinQueryWindow(hwndFrame, QW_PARENT) == G_HookData.hwndPMDesktop)
                     && (G_HookData.PagerConfig.flPager & PGRFL_ADDSTICKYTOGGLE)
                     && (G_HookData.hwndPagerFrame)
                   )
                {
                    mi.iPosition = MIT_END;
                    mi.afStyle = MIS_SEPARATOR;
                    mi.afAttribute = 0;
                    mi.id = -1;
                    mi.hwndSubMenu = 0;
                    mi.hItem = 0;
                    WinSendMsg(hNewMenu,
                               MM_INSERTITEM,
                               MPFROMP(&mi),
                               NULL);
                    mi.afStyle = MIS_TEXT|MIS_SYSCOMMAND;
                    if (WinSendMsg(G_HookData.hwndDaemonObject,
                                   XDM_ISTRANSIENTSTICKY,
                                   MPFROMHWND(hwndFrame),
                                   0)
                       )
                        mi.afAttribute = MIA_CHECKED;
                    else
                        mi.afAttribute = 0;
                    mi.id = PGRIDM_TOGGLEITEM;
                    WinSendMsg(hNewMenu,
                               MM_INSERTITEM,
                               MPFROMP(&mi),
                               G_HookData.NLSData.apszNLSStrings[NLS_STICKYTOGGLE]);
                }
#endif

                // display popup menu
                WinPopupMenu(HWND_DESKTOP, hwndFrame, hNewMenu,
                             ptlMouse.x, ptlMouse.y, 0x8007, PU_HCONSTRAIN |
                             PU_VCONSTRAIN | PU_MOUSEBUTTON1 |
                             PU_MOUSEBUTTON2 | PU_KEYBOARD);
            }
        }
    }
}

/*
 *@@ WMChord_WinList:
 *      this displays the window list at the current
 *      mouse position when WM_CHORD comes in.
 *
 *      Based on code from WarpEnhancer, (C) Achim HasenmÅller.
 */

VOID WMChord_WinList(VOID)
{
    POINTL  ptlMouse;       // mouse coordinates
    SWP     WinListPos;     // position of window list window
    LONG WinListX, WinListY; // new ordinates of window list window
    // LONG DesktopCX, DesktopCY; // width and height of screen
    // get mouse coordinates (absolute coordinates)
    WinQueryPointerPos(HWND_DESKTOP, &ptlMouse);
    // get position of window list window
    WinQueryWindowPos(G_HookData.hwndSwitchList,
                      &WinListPos);
    // calculate window list position (mouse pointer is center)
    WinListX = ptlMouse.x - (WinListPos.cx / 2);
    if (WinListX < 0)
        WinListX = 0;
    WinListY = ptlMouse.y - (WinListPos.cy / 2);
    if (WinListY < 0)
        WinListY = 0;
    if (WinListX + WinListPos.cx > G_HookData.cxScreen)
        WinListX = G_HookData.cxScreen - WinListPos.cx;
    if (WinListY + WinListPos.cy > G_HookData.cyScreen)
        WinListY = G_HookData.cyScreen - WinListPos.cy;
    // set window list window to calculated position
    WinSetWindowPos(G_HookData.hwndSwitchList, HWND_TOP,
                    WinListX, WinListY, 0, 0,
                    SWP_MOVE | SWP_SHOW | SWP_ZORDER);
    // now make it the active window
    WinSetActiveWindow(HWND_DESKTOP,
                       G_HookData.hwndSwitchList);
}


