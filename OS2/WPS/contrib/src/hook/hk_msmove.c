
/*
 *@@sourcefile hk_msmove.c:
 *
 *@@added V0.9.12 (2001-05-27) [umoeller]
 *@@header "hook\hook_private.h"
 */

/*
 *      Copyright (C) 1999-2003 Ulrich M”ller.
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

#include "hook\xwphook.h"
#include "hook\hook_private.h"          // private hook and daemon definitions

#pragma hdrstop

/******************************************************************
 *
 *  Input hook -- WM_MOUSEMOVE processing
 *
 ******************************************************************/

/*
 *@@ HiliteMenuItem:
 *      this un-hilites the currently hilited menu
 *      item in hwndMenu and optionally hilites the
 *      specified one instead.
 *
 *      Used during delayed sliding menu processing.
 *
 *@@added V0.9.2 (2000-02-26) [umoeller]
 *@@changed V0.9.6 (2000-10-27) [pr]: fixed some un-hilite problems; changed prototype
 *@@changed V0.9.11 (2001-04-25) [umoeller]: rewritten
 *@@changed V0.9.12 (2001-05-24) [lafaix]: changed prototype, now uses index instead of identity
 */

VOID HiliteMenuItem(HWND hwndMenu,
                    SHORT sItemIndex)      // in: item index to hilite
{
    // we now use an undocumented menu message which is used
    // by the menu control internally to properly select menu
    // items... if we change the menu item's attributes manually
    // (which is what we did before V0.9.11), the menu control
    // easily gets confused with managing the hilites.
    // Using this message works much better, apparently.
    #define MM_UNDOCSELECT1    0x019E       // l576 selects a menu item by its index
                                            // instead of the menu item ID

    WinSendMsg(hwndMenu,
               MM_UNDOCSELECT1,
               MPFROM2SHORT(sItemIndex,
                            0),         // apparently there are some flags in
                                        // here for opening submenus and stuff,
                                        // but we'll post MM_SELECTITEM later
                                        // anyway, so setting this to 0 will
                                        // work for our purposes
               0);                      // mp2 is always zero from my testing

    /* SHORT       sItemIndex2;
    // first, un-hilite the item which currently
    // has hilite state; otherwise, we'd successively
    // hilite _all_ items in the menu... doesn't look
    // too good.
    // We cannot use a global variable for storing
    // the last menu hilite, because there can be more
    // than one open (sub)menu, and we'd get confused
    // otherwise.
    // So go thru all menu items again and un-hilite
    // the first hilited menu item we find. This will
    // be the one either PM has hilited when something
    // was selected, or the one we might have hilited
    // here previously:
    for (sItemIndex2 = 0;
         sItemIndex2 < sItemCount;
         sItemIndex2++)
    {
        SHORT sCurrentItemIdentity2
            = (SHORT)WinSendMsg(hwndMenu,
                                MM_ITEMIDFROMPOSITION,
                                MPFROMSHORT(sItemIndex2),
                                NULL);
        ULONG ulAttr
            = (ULONG)WinSendMsg(hwndMenu,
                                MM_QUERYITEMATTR,
                                MPFROM2SHORT(sCurrentItemIdentity2,
                                             FALSE), // no search submenus
                                MPFROMSHORT(0xFFFF));
        if (ulAttr & MIA_HILITED)
        {
            WinSendMsg(hwndMenu,
                       MM_SETITEMATTR,
                       MPFROM2SHORT(sCurrentItemIdentity2,
                                    FALSE), // no search submenus
                       MPFROM2SHORT(MIA_HILITED,
                                    0));    // unset attribute
            // stop second loop
            // break;
        }
    }

    if (fHiLite)
        // now hilite the new item (the one under the mouse)
        WinSendMsg(hwndMenu,
                   MM_SETITEMATTR,
                   MPFROM2SHORT(sItemIndex,
                                FALSE), // no search submenus
                   MPFROM2SHORT(MIA_HILITED, MIA_HILITED));
    */
}

/*
 *@@ SelectMenuItem:
 *      this gets called to implement the "sliding menu" feature.
 *
 *@@added V0.9.2 (2000-02-26) [umoeller]
 *@@changed V0.9.6 (2000-10-29) [pr]: fixed submenu behavior
 *@@changed V0.9.12 (2001-05-25) [lafaix]: changed prototype, rewritten
 *@@changed V0.9.12 (2001-05-29) [lafaix]: fixed WPS folder action bars
 */

VOID SelectMenuItem(HWND hwndMenu,
                    SHORT sItemIndex,   // item index to select (NOT identity!)
                    BOOL bOpen)         // open submenu (if any) ?
{
    MENUITEM    menuitemCurrent;
    // USHORT      usSelItem;

    #ifndef MM_QUERYITEMBYPOS
        #define MM_QUERYITEMBYPOS 0x01f3
        // this undocumented message is sent to retrieve the definition
        // of a menu item by its position.  This message will only query
        // an item in the specified menu.
        //
        // Parameters:
        //     SHORT1FROMMP(mp1) = position of item in the menu
        //     SHORT2FROMMP(mp1) = reserved, set to 0
        //     mp2 = (16 bit pointer) points to a MENUITEM structure
        //           to be filled in.
    #endif
    #ifndef MAKE_16BIT_POINTER
        #define MAKE_16BIT_POINTER(p) \
                ((PVOID)MAKEULONG(LOUSHORT(p),(HIUSHORT(p) << 3) | 7))
        // converts a flat 32bit pointer to its 16bit offset/selector form
    #endif

    // Selecting a menu item is a 2-step operation: (1) we select the item
    // and force the submenu to be displayed (if appropriate):

    WinSendMsg(hwndMenu,
               MM_UNDOCSELECT1,
               MPFROM2SHORT(sItemIndex, bOpen),
               0);

    // (2) Then, if the expansion is requested, we check if the item
    // is a submenu.  If so, we select its first entry:

    if (bOpen)
    {
        // V0.9.12 (2001-05-29) [lafaix]: we must query the
        // menu item structure here, after selecting it, because
        // some apps change the submenu dynamically (WPS action
        // bars being an example).
        if (WinSendMsg(hwndMenu,
                       MM_QUERYITEMBYPOS,
                       MPFROMSHORT(sItemIndex),
                       MAKE_16BIT_POINTER(&menuitemCurrent)))
        {
            if (menuitemCurrent.afStyle & MIS_SUBMENU)
                WinSendMsg(menuitemCurrent.hwndSubMenu,
                           MM_UNDOCSELECT1,
                           0,
                           0);
        }
    }
}

#ifndef __NOSLIDINGFOCUS__

/*
 *@@ WMMouseMove_SlidingFocus:
 *      this gets called when hookInputHook intercepts
 *      WM_MOUSEMOVE to do the "sliding focus" processing.
 *
 *      This function evaluates hwnd to find out whether
 *      the mouse has moved over a new frame window.
 *      If so, XDM_SLIDINGFOCUS is posted to the daemon's
 *      object window, which will then do the actual focus
 *      and active window processing (starting a timer, if
 *      delayed focus is active).
 *
 *      Processing is fairly complicated because special
 *      case checks are needed for certain window classes
 *      (combo boxes, Win-OS/2 windows). Basically, this
 *      function finds the frame window to which hwnd
 *      (the window under the mouse, from the hook) belongs
 *      and passes that frame to the daemon. If another
 *      frame is found between the main frame and hwnd,
 *      that frame is passed to the daemon also.
 *
 *      Frame activation is then not done from the hook, but
 *      from the daemon instead.
 *
 *      Inspired by code from ProgramCommander (W) Roman Stangl.
 *      Fixed a few problems with MDI frames.
 *
 *@@changed V0.9.3 (2000-05-22) [umoeller]: fixed combobox problems
 *@@changed V0.9.3 (2000-05-22) [umoeller]: fixed MDI frames problems
 *@@changed V0.9.4 (2000-06-12) [umoeller]: fixed Win-OS/2 menu problems
 *@@changed V0.9.9 (2001-03-14) [lafaix]: disabling sliding when processing mouse switch
 *@@changed V0.9.19 (2002-06-02) [umoeller]: fixed idiocy with Mozilla menus
 */

VOID WMMouseMove_SlidingFocus(HWND hwnd,        // in: wnd under mouse, from hookInputHook
                              BOOL fMouseMoved, // in: TRUE if mouse has been moved since previous WM_MOUSEMOVE
                                                // (determined by hookInputHook)
                              PSZ pszClassUnderMouse) // in: window class of hwnd (determined
                                                      // by hookInputHook)
{
    BOOL    fStopTimers = FALSE;    // setting this to TRUE will stop timers

#ifndef __NOPAGER__
    if (G_HookData.fProcessingWraparound)
        return;
#endif

    do      // just a do for breaking, no loop
    {
        // get currently active window; this can only
        // be a frame window (WC_FRAME)
        HWND    hwndActiveNow;

        // check 1: check if the active window is still the
        //          the one which was activated by ourselves
        //          previously (either by the hook during WM_BUTTON1DOWN
        //          or by the daemon in sliding focus processing):
        if (hwndActiveNow = WinQueryActiveWindow(HWND_DESKTOP))
        {
            if (hwndActiveNow != G_HookData.hwndActivatedByUs)
            {
                // active window is not the one we set active:
                // this probably means that some new
                // window popped up which we haven't noticed
                // and was explicitly made active either by the
                // shell or by an application, so we use this
                // for the below checks. Otherwise, sliding focus
                // would be disabled after a new window has popped
                // up until the mouse was moved over a new frame window.
                G_hwndLastFrameUnderMouse = hwndActiveNow;
                G_hwndLastSubframeUnderMouse = NULLHANDLE;
                G_HookData.hwndActivatedByUs = hwndActiveNow;

                fStopTimers = TRUE;
                        // this will be overridden if we start a new
                        // timer below; but just in case an old timer
                        // might still be running V0.9.4 (2000-08-03) [umoeller]
            }
        }

        if (fMouseMoved)            // has mouse really moved?
        {
            // OK:
            HWND    hwndDesktopChild = hwnd,
                    hwndTempParent = NULLHANDLE,
                    hwndFrameInBetween = NULLHANDLE;
            HWND    hwndFocusNow = WinQueryFocus(HWND_DESKTOP);
            CHAR    szClassName[30],
                    szWindowText[30];

            // check 2: make sure mouse is not captured
            if (WinQueryCapture(HWND_DESKTOP) != NULLHANDLE)
            {
                // stop timers and quit
                fStopTimers = TRUE;
                break;
            }

            // check 3: quit if menu has the focus
            if (hwndFocusNow)
            {
                CHAR    szFocusClass[MAXNAMEL+4] = "";
                WinQueryClassName(hwndFocusNow,
                                  sizeof(szFocusClass),
                                  szFocusClass);

                if (!strcmp(szFocusClass, "#4"))
                {
                    // menu:
                    // stop timers and quit
                    fStopTimers = TRUE;
                    break;
                }
            }

            // now climb up the parent window hierarchy of the
            // window under the mouse (first: hwndDesktopChild)
            // until we reach the Desktop; the last window we
            // had is then in hwndDesktopChild
            hwndTempParent = WinQueryWindow(hwndDesktopChild, QW_PARENT);
            while (     (hwndTempParent != G_HookData.hwndPMDesktop)
                     && (hwndTempParent != NULLHANDLE)
                  )
            {
                WinQueryClassName(hwndDesktopChild,
                                  sizeof(szClassName), szClassName);
                if (!strcmp(szClassName, "#1"))
                    // it's a frame:
                    hwndFrameInBetween = hwndDesktopChild;
                hwndDesktopChild = hwndTempParent;
                hwndTempParent = WinQueryWindow(hwndDesktopChild, QW_PARENT);
            }

            if (hwndFrameInBetween == hwndDesktopChild)
                hwndFrameInBetween = NULLHANDLE;

            // hwndDesktopChild now has the window which we need to activate
            // (the topmost parent under the desktop of the window under the mouse)

            WinQueryClassName(hwndDesktopChild,
                              sizeof(szClassName), szClassName);

            // check 4: skip certain window classes

            if (    (!strcmp(szClassName, "#4"))
                            // menu
                 || (!strcmp(szClassName, "#7"))
                         // listbox: as a desktop child, this must be a
                         // a drop-down box which is currently open
                 || (!strcmp(szClassName, "MozillaWindowClass"))
                         // this fixes mozilla popup menus; those
                         // are a MozillaWindowClass as child of
                         // HWND_DESKTOP (this does not break the
                         // main Mozilla windows because those have
                         // a WC_FRAME)
                         // V0.9.19 (2002-06-02) [umoeller]
               )
            {
                // stop timers and quit
                fStopTimers = TRUE;
                break;
            }

            WinQueryWindowText(hwndDesktopChild, sizeof(szWindowText), szWindowText);
            if (strstr(szWindowText, "Seamless"))
                // ignore seamless Win-OS/2 menus; these are separate windows!
            {
                // stop timers and quit
                fStopTimers = TRUE;
                break;
            }

            // OK, enough checks.
            // Now let's do the sliding focus if
            // 1) the desktop window (hwndDesktopChild, highest parent) changed or
            // 2) if hwndDesktopChild has several subframes and the subframe changed:

            if (hwndDesktopChild)
                if (    (hwndDesktopChild != G_hwndLastFrameUnderMouse)
                     || (   (hwndFrameInBetween != NULLHANDLE)
                         && (hwndFrameInBetween != G_hwndLastSubframeUnderMouse)
                        )
                   )
                {
                    // OK, mouse moved to a new desktop window:
                    // store that for next time
                    G_hwndLastFrameUnderMouse = hwndDesktopChild;
                    G_hwndLastSubframeUnderMouse = hwndFrameInBetween;

                    // notify daemon of the change;
                    // it is the daemon which does the rest
                    // (timer handling, window activation etc.)
                    WinPostMsg(G_HookData.hwndDaemonObject,
                               XDM_SLIDINGFOCUS,
                               (MPARAM)hwndFrameInBetween,  // can be NULLHANDLE
                               (MPARAM)hwndDesktopChild);
                    fStopTimers = FALSE;
                }
        }
    } while (FALSE); // end do

    if (fStopTimers)
        WinPostMsg(G_HookData.hwndDaemonObject,
                   XDM_SLIDINGFOCUS,
                   (MPARAM)NULLHANDLE,
                   (MPARAM)NULLHANDLE);     // stop timers
}

#endif

/*
 *@@ WMMouseMove_SlidingMenus:
 *      this gets called when hookInputHook intercepts
 *      WM_MOUSEMOVE and the window under the mouse is
 *      a menu control to do automatic menu selection.
 *
 *      This implementation is slightly tricky. ;-)
 *      We cannot query the MENUITEM data of a menu
 *      which does not belong to the calling process
 *      because this message needs access to the caller's
 *      storage. I have tried this, this causes every
 *      application to crash as soon as a menu item
 *      gets selected.
 *
 *      Fortunately, Roman Stangl has found a cool way
 *      of doing this, so I could build on that.
 *
 *      So now we start a timer in the daemon which will
 *      post a special WM_MOUSEMOVE message back to us
 *      which can only be received by us
 *      (with the same HWND and MP1 as the original
 *      WM_MOUSEMOVE, but a special flag in MP2). When
 *      that special message comes in, we can select
 *      the menu item.
 *
 *      In addition to Roman's code, we always change the
 *      MIA_HILITE attribute of the current menu item
 *      under the mouse, so the menu hilite follows the
 *      mouse immediately, even though submenus may be
 *      opened delayed.
 *
 *      We return TRUE if the msg is to be swallowed.
 *
 *      Inspired by code from ProgramCommander (W) Roman Stangl.
 *
 *@@added V0.9.2 (2000-02-26) [umoeller]
 *@@changed V0.9.6 (2000-10-27) [pr]: fixed un-hilite problems
 *@@changed V0.9.6 (2000-10-27) [umoeller]: added optional NPSWPS-like submenu behavior
 *@@changed V0.9.9 (2001-03-10) [umoeller]: fixed cc sensitivity and various selection issues
 *@@changed V0.9.12 (2001-05-21) [lafaix]: fixed incorrect vertical pos check
 *@@changed V0.9.12 (2001-05-22) [lafaix]: removed delay for menu bar entries
 *@@changed V0.9.12 (2001-05-25) [lafaix]: reworked logic to fix various selection issues
 *@@changed V0.9.12 (2001-05-29) [lafaix]: added extra check for non-root menus with no selection
 */

BOOL WMMouseMove_SlidingMenus(HWND hwndCurrentMenu,  // in: menu wnd under mouse, from hookInputHook
                              MPARAM mp1,
                              MPARAM mp2)
{
    static BOOL     G_bDelayProcessed = FALSE,
                    G_bOpen = FALSE;

    BOOL            brc = FALSE;        // per default, don't swallow msg

    // check for special message which was posted from
    // the daemon when the "delayed sliding menu" timer
    // elapsed:
    if (    // current mp1 the same as timer mp1?
            (mp1 == G_HookData.mpDelayedSlidingMenuMp1)
            // special code from daemon in mp2?
         && (SHORT1FROMMP(mp2) == HT_DELAYEDSLIDINGMENU)
       )
    {
        // yes, special message:

        // get current window under mouse pointer; we
        // must ignore the timer msg if the user has
        // moved the mouse away from the menu since
        // the timer was started
        HWND    hwndUnderMouse = WinWindowFromPoint(HWND_DESKTOP,
                                                    &G_ptlMousePosDesktop,
                                                    TRUE);  // enum desktop children
        // start V0.9.12 (2001-05-25) [lafaix]:
        if (    (hwndCurrentMenu == G_HookData.hwndMenuUnderMouse)
             && (hwndCurrentMenu == hwndUnderMouse)
             && (!G_bDelayProcessed)
           )
        {
            // select menu item under the mouse
            SelectMenuItem(hwndCurrentMenu,
                           G_HookData.sMenuItemIndexUnderMouse,
                           G_bOpen);
                           // stored from last run
                           // when timer was started...
            G_bDelayProcessed = TRUE;
        }
        else
            // do nothing, but reset location cache
            G_HookData.hwndMenuUnderMouse = NULLHANDLE;

        // V0.9.12 (2001-05-25) [lafaix]:
        // the above is strictly equivalent to the following code, except
        // that it's simpler and faster.
        /*
        CHAR    szClassUnderMouse[100];
        if (WinQueryClassName(hwndUnderMouse, sizeof(szClassUnderMouse), szClassUnderMouse))
        {
            if (!strcmp(szClassUnderMouse, "#4"))
            {
                if (    // timer menu hwnd same as last menu under mouse?
                        (hwndCurrentMenu == G_HookData.hwndMenuUnderMouse)
                        // last WM_MOUSEMOVE hwnd same as last menu under mouse?
                     && (G_hwndUnderMouse == G_HookData.hwndMenuUnderMouse)
                        // timer menu hwnd same as current window under mouse?
                     && (hwndUnderMouse == G_HookData.hwndMenuUnderMouse)
                    )
                {
                    // check if the menu still exists
                    // if (WinIsWindow(hwndCurrentMenu))
                    // and if it's visible; the menu
                    // might have been hidden by now because
                    // the user has already proceeded to
                    // another submenu
                    if (WinIsWindowVisible(hwndCurrentMenu))
                    {
                        // OK:
                        brc = TRUE;
                    }
                }
            }
        }

        if (brc)
        {
            if (!G_bDelayProcessed)
            {
                // select menu item under the mouse
                SelectMenuItem(hwndCurrentMenu,
                               G_HookData.sMenuItemIndexUnderMouse,
                               G_bOpen);
                               // stored from last run
                               // when timer was started...
                G_bDelayProcessed = TRUE;
            }
        }
        */ // @@@

        // V0.9.11 (2001-04-25) [umoeller]:
        // the following is no longer needed with the new hilite code above
        /* else
        {
            if (G_HookData.hwndMenuUnderMouse)
            {
                // remove previously hilited but not selected entry,
                // as it is unknown to PM
                SHORT   sItemCount
                    = (SHORT)WinSendMsg(G_HookData.hwndMenuUnderMouse,
                                        MM_QUERYITEMCOUNT,
                                        0, 0);
                HiliteMenuItem(G_HookData.hwndMenuUnderMouse,
                               sItemCount,
                               // G_HookData.sMenuItemUnderMouse,
                               FALSE);
                G_HookData.hwndMenuUnderMouse = 0;
                sLastHiliteID = MIT_NONE;
            }
        } */ // @@@
    }
    else
    {
        // not from daemon, but regular WM_MOUSEMOVE:

        // check if anything is currently selected in the menu
        // (this rules out main menu bars if user hasn't clicked
        // into them yet)
        ULONG   ulMenuStyle = WinQueryWindowULong(hwndCurrentMenu, QWL_STYLE);
        USHORT  usItemCount = (USHORT)WinSendMsg(hwndCurrentMenu,
                                                 MM_QUERYITEMCOUNT,
                                                 0, 0),
                usItemIndex = 0,
                usOldItemIndex = 0xFFFF,
                usNewItemIndex = 0xFFFF;

        BOOL    bActive = FALSE,
                bFound = FALSE;
        LONG    lCurrentXRight;

        // loop through all items in the current menu
        // and query each item's rectangle
        for (usItemIndex = 0;
             usItemIndex < usItemCount;
             usItemIndex++)
        {
            RECTL       rectlItem;

            USHORT usCurrentItemIdentity = (USHORT)WinSendMsg(hwndCurrentMenu,
                                                              MM_ITEMIDFROMPOSITION,
                                                              MPFROMSHORT(usItemIndex),
                                                              NULL);
            #ifndef MM_QUERYITEMATTRBYPOS
                #define MM_QUERYITEMATTRBYPOS 0x01f5
                // this undocumented message is sent to retrieve the current
                // attributes of a menu item specified by its position in the
                // menu.  The attributes returned are the current state values
                // of the menu item ANDed with the attribute mask specified in
                // mp2.  It does not change any of the attributes of the menu
                // item.  This message will only query an item in the menu
                // specified.
                //
                // Paramters:
                //     SHORT1FROMMP(mp1) = position of item to find
                //     SHORT2FROMMP(mp1) = reserved, set to zero
                //     mp2 = requested atributes mask
            #endif

            USHORT usAttr = (USHORT)WinSendMsg(hwndCurrentMenu,
                                               MM_QUERYITEMATTRBYPOS,
                                               MPFROMSHORT(usItemIndex),
                                               MPFROMSHORT(MIA_HILITED));

            if (usAttr == MIA_HILITED)
            {
                // this menu contained a selected entry, so it was active
                bActive = TRUE;
                usOldItemIndex = usItemIndex;
            }

            // get the menuentry's rectangle to test if it covers the
            // current mouse pointer position
            WinSendMsg(hwndCurrentMenu,
                       MM_QUERYITEMRECT,
                       MPFROM2SHORT(usCurrentItemIdentity,
                                    FALSE),
                       MPFROMP(&rectlItem));

            // V0.9.9 (2001-03-10) [umoeller]: moved "conditional casc." stuff down

            if (    (G_ptsMousePosWin.x > rectlItem.xLeft)
                 && (G_ptsMousePosWin.x <= rectlItem.xRight)
                 && (G_ptsMousePosWin.y >= rectlItem.yBottom) // V0.9.12 (2001-05-21) [lafaix]
                 && (G_ptsMousePosWin.y < rectlItem.yTop)     // V0.9.12 (2001-05-21) [lafaix]
               )
            {
                lCurrentXRight = rectlItem.xRight;
                usNewItemIndex = usItemIndex;
                bFound = TRUE;
            }

            // we've found what we were looking for ..
            if (bActive && bFound)
                break;

        } // end for (sItemIndex = 0; ...

        #ifndef MS_ROOT
            #define MS_ROOT 0x00000008L
            // menu is the root of the menu hierarchy
        #endif

        if (    (bFound)
             && (    (bActive)
                  || ((ulMenuStyle & MS_ROOT) != MS_ROOT) // V0.9.12 (2001-05-29) [lafaix]
                )
           )
        {
            BOOL    fSelect = TRUE;

            // do extra checks if "cc sensitivity" on;
            // moved this here V0.9.9 (2001-03-10) [umoeller]
            // so that we can always hilite, but we do not always
            // select
            if (G_HookData.HookConfig.fConditionalCascadeSensitive)
            {
                // check if the pointer position is within the item's
                // rectangle or just the right hand half if it is a popup
                // menu and has conditional cascade submenus
                MENUITEM    menuitemCurrent;
                if (WinSendMsg(hwndCurrentMenu,
                               MM_QUERYITEMBYPOS,
                               MPFROMSHORT(usNewItemIndex),
                               MAKE_16BIT_POINTER(&menuitemCurrent)))
                {
                    if (menuitemCurrent.afStyle & MIS_SUBMENU)
                    {
                        ULONG ulStyle =
                            WinQueryWindowULong(menuitemCurrent.hwndSubMenu,
                                                QWL_STYLE);
                        if (ulStyle & MS_CONDITIONALCASCADE)
                            // is "cc" menu:
                            // select only if mouse is on the right
                            // (approximately above the cc button)
                            if (G_ptsMousePosWin.x < lCurrentXRight - 20)
                                // not over button: do not select then
                                fSelect = FALSE;
                    }
                }
            }

            // do we have a submenu delay?
            if (    (G_HookData.HookConfig.ulSubmenuDelay)
                 && ((ulMenuStyle & MS_ACTIONBAR) != MS_ACTIONBAR) // V0.9.12 (2001-05-22) [lafaix]
               )
            {
                // delayed:
                // this is a three-step process:
                // 1)  If we used MM_SELECTITEM on the item, this
                //     would immediately open the subwindow (if the
                //     item represents a submenu).
                //     So instead, we first need to manually change
                //     the hilite attribute of the menu item under
                //     the mouse so that the item under the mouse is
                //     always immediately hilited (without being
                //     "selected"; PM doesn't know what we're doing here!)

                // V0.9.9 (2001-03-10) [umoeller]
                // Note that we even hilite the menu item for
                // conditional cascade submenus if "cc sensitivity"
                // is on. We only differentiate for _selection_ below now.

                // has item changed since last time?
                if (    (usNewItemIndex != usOldItemIndex)
                     && (G_HookData.HookConfig.fMenuImmediateHilite)
                   )
                    HiliteMenuItem(hwndCurrentMenu,
                                   usNewItemIndex);

                // 2)  We then post the daemon a message to start
                //     a timer. Before that, we store the menu item
                //     data in HOOKDATA so we can re-use it when
                //     the timer elapses.

                // V0.9.12 (2001-05-25) [lafaix]: we only post the
                // message if something has changed
                if (    (usNewItemIndex != usOldItemIndex)
                     || (G_HookData.hwndMenuUnderMouse != hwndCurrentMenu)
                     || (G_HookData.sMenuItemIndexUnderMouse != usNewItemIndex)
                     || (fSelect && (G_bOpen == FALSE))
                   )
                {
                    // prepare data for delayed selection:
                    // when the special WM_MOUSEMOVE comes in,
                    // we check against all these.
                    // a) store mp1 for comparison later
                    G_HookData.mpDelayedSlidingMenuMp1 = mp1;
                    // b) store menu
                    G_HookData.hwndMenuUnderMouse = hwndCurrentMenu;
                    // c) store menu item
                    G_HookData.sMenuItemIndexUnderMouse = usNewItemIndex;
                    // d) enable submenu delay processing
                    G_bDelayProcessed = FALSE;
                    // e) record fSelect state
                    G_bOpen = fSelect;
                    // f) notify daemon of the change, which
                    // will start the timer and post WM_MOUSEMOVE
                    // back to us
                    WinPostMsg(G_HookData.hwndDaemonObject,
                               XDM_SLIDINGMENU,
                               mp1,
                               0);

                    // 3)  When the timer elapses, the daemon posts a special
                    //     WM_MOUSEMOVE to the same menu control for which
                    //     the timer was started. See the "special message"
                    //     processing on top. We then immediately select
                    //     the menu item.
                }
            } // end if (HookData.HookConfig.ulSubmenuDelay)
            else
            {
                // V0.9.12 (2001-05-25) [lafaix]: disable timer now, and update
                // hookdata so that we know the mouse has moved (we must do that,
                // cause we may be over an action bar)
                G_bDelayProcessed = TRUE;
                G_HookData.hwndMenuUnderMouse = hwndCurrentMenu;

                // no delay, but immediately:
                if (    (usOldItemIndex != usNewItemIndex)
                     && (fSelect)
                   )
                    SelectMenuItem(hwndCurrentMenu,
                                   usNewItemIndex,
                                   fSelect);
            }
        }
    }

    return brc;
}

#ifndef __NOMOVEMENT2FEATURES__

/*
 *@@ WMMouseMove_AutoHideMouse:
 *      this gets called when hookInputHook intercepts
 *      WM_MOUSEMOVE to automatically hide the mouse
 *      pointer.
 *
 *      We start a timer here which posts WM_TIMER
 *      to fnwpDaemonObject in the daemon. So it's the
 *      daemon which actually hides the mouse.
 *
 *      Based on code from WarpEnhancer, (C) Achim Hasenmller.
 *
 *@@added V0.9.1 (99-12-03) [umoeller]
 *@@changed V0.9.5 (2000-09-20) [pr]: fixed auto-hide bug
 *@@changed V0.9.14 (2001-08-01) [lafaix]: allows auto-hide disabling while in menu mode
 *@@changed V1.0.0 (2002-09-05) [lafaix]: do not hide if a mouse button is down
 */

VOID WMMouseMove_AutoHideMouse(VOID)
{
    // is the timer currently running?
    if (G_HookData.idAutoHideTimer != NULLHANDLE)
    {
        // stop the running async timer
        WinStopTimer(G_HookData.habDaemonObject,
                     G_HookData.hwndDaemonObject,
                     G_HookData.idAutoHideTimer);
        G_HookData.idAutoHideTimer = NULLHANDLE;
    }

    // show the mouse pointer now
    if (G_HookData.fMousePointerHidden)
    {
        WinShowPointer(HWND_DESKTOP, TRUE);
        G_HookData.fMousePointerHidden = FALSE;
    }

    // (re)start timer
    if (    (G_HookData.HookConfig.__fAutoHideMouse) // V0.9.5 (2000-09-20) [pr] fix auto-hide mouse bug
         && (G_hwndRootMenu == NULLHANDLE) // V0.9.14 (2001-08-01) [lafaix]
         && ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON1) & 0x8000) == 0) // V1.0.0 (2002-09-05) [lafaix]
         && ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON2) & 0x8000) == 0)
         && ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON3) & 0x8000) == 0)
       )
        G_HookData.idAutoHideTimer =
            WinStartTimer(G_HookData.habDaemonObject,
                          G_HookData.hwndDaemonObject,
                          TIMERID_AUTOHIDEMOUSE,
                          (G_HookData.HookConfig.__ulAutoHideDelay + 1) * 1000);
}

#endif

/*
 *@@ WMMouseMove:
 *
 *@@added V0.9.3 (2000-04-30) [umoeller]
 *@@changed V0.9.3 (2000-04-30) [umoeller]: pointer was hidden while MB3-dragging; fixed
 *@@changed V0.9.4 (2000-08-03) [umoeller]: fixed sliding menus without mouse moving
 *@@changed V0.9.5 (2000-08-22) [umoeller]: sliding focus was always on, working half way; fixed
 *@@changed V0.9.5 (2000-09-20) [pr]: fixed auto-hide bug
 *@@changed V0.9.9 (2001-03-15) [lafaix]: now uses corner sensitivity
 *@@changed V0.9.9 (2001-03-15) [lafaix]: added AutoScroll support
 *@@changed V0.9.14 (2001-08-21) [umoeller]: added fixes for while system is locked up
 *@@changed V0.9.18 (2002-02-12) [pr]: mods. for screen wrap
 *@@changed V1.0.0 (2002-09-05) [lafaix]: refined auto-hide behavior
 */

BOOL WMMouseMove(PQMSG pqmsg,
                 PBOOL pfRestartAutoHide)      // out: set to TRUE if auto-hide must be processed
{
    BOOL    brc = FALSE;        // swallow?
    BOOL    fGlobalMouseMoved = FALSE,
            fWinChanged = FALSE;

    do // allow break's
    {
        HWND    hwndCaptured = WinQueryCapture(HWND_DESKTOP);

        // store mouse pos in win coords
        G_ptsMousePosWin.x = SHORT1FROMMP(pqmsg->mp1);
        G_ptsMousePosWin.y = SHORT2FROMMP(pqmsg->mp1);

        // has position changed since last WM_MOUSEMOVE?
        // PM keeps posting WM_MOUSEMOVE even if the
        // mouse hasn't moved, so we can drop unnecessary
        // processing...
        if (G_ptlMousePosDesktop.x != pqmsg->ptl.x)
        {
            // store x mouse pos in Desktop coords
            G_ptlMousePosDesktop.x = pqmsg->ptl.x;
            fGlobalMouseMoved = TRUE;
        }
        if (G_ptlMousePosDesktop.y != pqmsg->ptl.y)
        {
            // store y mouse pos in Desktop coords
            G_ptlMousePosDesktop.y = pqmsg->ptl.y;
            fGlobalMouseMoved = TRUE;
        }
        if (pqmsg->hwnd != G_hwndUnderMouse)
        {
            // mouse has moved to a different window:
            // this can happen even if the coordinates
            // are the same, because PM posts WM_MOUSEMOVE
            // with changing window positions also to
            // allow an application to change pointers
            G_hwndUnderMouse = pqmsg->hwnd;
            fWinChanged = TRUE;
        }

        if (    (fGlobalMouseMoved)
             || (fWinChanged)
           )
        {

            /*
             * MB3 scroll
             *
             */

            // are we currently in scrolling mode
            if (    (    (G_HookData.HookConfig.fMB3Scroll)
                      || (G_HookData.bAutoScroll)
                    )
                 && (G_HookData.hwndCurrentlyScrolling)
                 && (!G_HookData.hwndLockupFrame)    // system not locked up V0.9.14
               )
            {
                // simulate mouse capturing by passing the scrolling
                // window, no matter what hwnd came in with WM_MOUSEMOVE
                brc = WMMouseMove_MB3Scroll(G_HookData.hwndCurrentlyScrolling);
                break;  // skip all the rest
            }

            // make sure that the mouse is not currently captured
            if (hwndCaptured == NULLHANDLE)
            {
                CHAR    szClassUnderMouse[200];

                WinQueryClassName(pqmsg->hwnd,
                                  sizeof(szClassUnderMouse),
                                  szClassUnderMouse);

                /*
                 * sliding focus:
                 *
                 */

#ifndef __NOSLIDINGFOCUS__
                if (    (G_HookData.HookConfig.__fSlidingFocus)
                     && (!G_HookData.hwndLockupFrame)    // system not locked up V0.9.14
                   )
                {
                    // sliding focus enabled?
                    // V0.9.5 (2000-08-22) [umoeller]
                    WMMouseMove_SlidingFocus(pqmsg->hwnd,
                                             fGlobalMouseMoved,
                                             szClassUnderMouse);
                }
#endif

                if (fGlobalMouseMoved)
                {
                    // only if mouse has moved, not
                    // on window change:

                    ULONG   bHotCorner = -1;

                    /*
                     * hot corners:
                     *
                     *changed V0.9.9 (2001-03-15) [lafaix]: now uses corner sensitivity
                     */

                    if (!G_HookData.hwndLockupFrame)    // system not locked up V0.9.14
                    {
                        // check if mouse is in one of the screen
                        // corners
                        if (G_ptlMousePosDesktop.x == 0)
                        {
                            if (G_ptlMousePosDesktop.y == 0)
                                bHotCorner = SCREENCORNER_BOTTOMLEFT;
                            else if (G_ptlMousePosDesktop.y == G_HookData.cyScreen - 1)
                                bHotCorner = SCREENCORNER_TOPLEFT;
                            // or maybe left screen border:
                            // make sure mouse y is in the middle third of the screen
                            else if (    (G_ptlMousePosDesktop.y >= G_HookData.cyScreen * G_HookData.HookConfig.ulCornerSensitivity / 100)
                                      && (G_ptlMousePosDesktop.y <= G_HookData.cyScreen * (100 - G_HookData.HookConfig.ulCornerSensitivity) / 100)
                                    )
                                bHotCorner = SCREENCORNER_LEFT;
                        }
                        else if (G_ptlMousePosDesktop.x == G_HookData.cxScreen - 1)
                        {
                            if (G_ptlMousePosDesktop.y == 0)
                                bHotCorner = SCREENCORNER_BOTTOMRIGHT;
                            else if (G_ptlMousePosDesktop.y == G_HookData.cyScreen - 1)
                                bHotCorner = SCREENCORNER_TOPRIGHT;
                            // or maybe right screen border:
                            // make sure mouse y is in the middle third of the screen
                            else if (    (G_ptlMousePosDesktop.y >= G_HookData.cyScreen * G_HookData.HookConfig.ulCornerSensitivity / 100)
                                      && (G_ptlMousePosDesktop.y <= G_HookData.cyScreen * (100 - G_HookData.HookConfig.ulCornerSensitivity) / 100)
                                    )
                                bHotCorner = SCREENCORNER_RIGHT;
                        }
                        else
                            // more checks for top and bottom screen border:
                            if (    (G_ptlMousePosDesktop.y == 0)   // bottom
                                 || (G_ptlMousePosDesktop.y == G_HookData.cyScreen - 1) // top
                               )
                            {
                                if (    (G_ptlMousePosDesktop.x >= G_HookData.cxScreen * G_HookData.HookConfig.ulCornerSensitivity / 100)
                                     && (G_ptlMousePosDesktop.x <= G_HookData.cxScreen * (100 - G_HookData.HookConfig.ulCornerSensitivity) / 100)
                                   )
                                    if (G_ptlMousePosDesktop.y == 0)
                                        bHotCorner = SCREENCORNER_BOTTOM;
                                    else
                                        bHotCorner = SCREENCORNER_TOP;
                            }

                        // is mouse in a screen corner?
                        if (bHotCorner != -1)
                            // yes:
                            // notify thread-1 object window, which
                            // will start the user-configured action
                            // (if any)
                            WinPostMsg(G_HookData.hwndDaemonObject,
                                       XDM_HOTCORNER,
                                       (MPARAM)bHotCorner,
                                       (MPARAM)NULL);

                    } // end if (!G_HookData.hwndLockupFrame)    // system not locked up V0.9.14

                    /*
                     * sliding menus:
                     *    only if mouse has moved globally
                     *    V0.9.4 (2000-08-03) [umoeller]
                     */

                    if (    (G_HookData.HookConfig.fSlidingMenus)
                         && (!strcmp(szClassUnderMouse, "#4"))
                       )
                        // window under mouse is a menu:
                        WMMouseMove_SlidingMenus(pqmsg->hwnd,
                                                 pqmsg->mp1,
                                                 pqmsg->mp2);

#ifndef __NOMOVEMENT2FEATURES__

                    /*
                     * auto hide disabled over buttons
                     */

                    if (    (G_HookData.HookConfig.__fAutoHideMouse)
                         && (G_HookData.HookConfig.__ulAutoHideFlags & AHF_IGNOREBUTTONS)
                         && (!strcmp(szClassUnderMouse, "#3"))
                       )
                    {
                        // window under mouse is a button, do not restart autohide
                        // is the timer currently running?
                        if (G_HookData.idAutoHideTimer != NULLHANDLE)
                        {
                            // stop the running async timer
                            WinStopTimer(G_HookData.habDaemonObject,
                                         G_HookData.hwndDaemonObject,
                                         G_HookData.idAutoHideTimer);

                            G_HookData.idAutoHideTimer = NULLHANDLE;
                        }

                        // show the mouse pointer now
                        if (G_HookData.fMousePointerHidden)
                        {
                            WinShowPointer(HWND_DESKTOP, TRUE);
                            G_HookData.fMousePointerHidden = FALSE;
                        }

                        // do not restart autohide (we undefine fGlobalMouseMoved
                        // so that the ending test does not hide the pointer again)
                        fGlobalMouseMoved = FALSE;
                        // undefining fWinChanged too because the ending test has
                        // changed.  V1.0.0 (2002-09-05) [lafaix]
                        fWinChanged = FALSE;
                    }
#endif

                } // end if (fMouseMoved)

            } // if (WinQueryCapture(HWND_DESKTOP) == NULLHANDLE)
        } // end if (fMouseMoved)
    } while (FALSE);

    if (    (fGlobalMouseMoved)
         || (fWinChanged)
                // we must restart autohide if just the window under
                // the pointer has changed, if for example the button
                // we were over is no more.   V1.0.0 (2002-09-05) [lafaix]
       )
    {
        /*
         * auto-hide pointer:
         *
         */

        // V0.9.9 (2001-01-29) [umoeller]
        *pfRestartAutoHide = TRUE;
    }

    return brc;
}


