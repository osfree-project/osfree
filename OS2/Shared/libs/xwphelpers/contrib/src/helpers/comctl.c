
/*
 *@@sourcefile comctl.c:
 *      contains various window procedures for implementing
 *      common controls. The source file name has nothing to do
 *      with the Windoze DLL of the same name.
 *
 *      Usage: All PM programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  ctl*   common control helper functions
 *
 *      The functionality of this code is accessed with the use
 *      of a special helper function. Sometimes that function
 *      registers a new window class, sometimes a static control
 *      needs to be subclassed. See the respective functions for
 *      details.
 *
 *      In detail, we have:
 *
 *      --  a "menu button" control, which displays a menu when
 *          pressed (see ctlMakeMenuButton for details);
 *
 *      --  progress bar support (see ctlProgressBarFromStatic for
 *          details);
 *
 *      --  a "chart" control for displaying pie charts (all new
 *          with V0.9.0; see ctlChartFromStatic for details);
 *
 *      --  split windows support (all new with V0.9.0; see
 *          ctlCreateSplitWindow for details);
 *
 *      --  a subclassed static control for enhanced bitmap and
 *          and icon display (see ctl_fnwpBitmapStatic for details).
 *          This used to be in animate.c and has been enhanced
 *          with V0.9.0;
 *
 *      --  a "tooltip" control, which shows fly-over ("bubble")
 *          help over any window, including controls. This is
 *          largely API-compatible with the Win95 tooltip control.
 *          See ctl_fnwpTooltip for details;
 *
 *      --  a "checkbox container" control, which is a subclassed
 *          container which uses checkboxes as record icons.
 *          See ctlMakeCheckboxContainer for details.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 */

/*
 *      Copyright (C) 1997-2002 Ulrich M”ller.
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

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINPOINTERS
#define INCL_WINTRACKRECT
#define INCL_WINTIMER
#define INCL_WINSYS

#define INCL_WINRECTANGLES      /// xxx temporary

#define INCL_WINMENUS
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINSTDCNR
#define INCL_WINENTRYFIELDS
#define INCL_WINSHELLDATA

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPILCIDS
#define INCL_GPIPATHS
#define INCL_GPIREGIONS
#define INCL_GPIBITMAPS             // added V0.9.1 (2000-01-04) [umoeller]: needed for EMX headers
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers/cnrh.h"
#include "helpers/except.h"             // exception handling
#include "helpers/gpih.h"
#include "helpers/linklist.h"
#include "helpers/winh.h"
#include "helpers/standards.h"

#include "helpers/comctl.h"

#pragma hdrstop

/* ******************************************************************
 *
 *   Shared stuff
 *
 ********************************************************************/

/*
 *@@ ctlSendWmControl:
 *      little helper that post a WM_CONTROL message to
 *      a control's owner.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

MRESULT ctlSendWmControl(HWND hwndControl,     // in: control who's posting
                         USHORT usCode,        // in: code for SHORT2FROMMP(mp1)
                         MPARAM mp2)           // in: mp2 from WM_CONTROL
{
    HWND hwndOwner;

    if (hwndOwner = WinQueryWindow(hwndControl, QW_OWNER))
        return WinSendMsg(hwndOwner,
                          WM_CONTROL,
                          MPFROM2SHORT(WinQueryWindowUShort(hwndControl, QWS_ID),
                                       usCode),
                          mp2);

    return NULL;
}

/*
 *@@ ctlPostWmControl:
 *      little helper that post a WM_CONTROL message to
 *      a control's owner.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

BOOL ctlPostWmControl(HWND hwndControl,     // in: control who's posting
                      USHORT usCode,        // in: code for SHORT2FROMMP(mp1)
                      MPARAM mp2)           // in: mp2 from WM_CONTROL
{
    HWND hwndOwner;

    if (hwndOwner = WinQueryWindow(hwndControl, QW_OWNER))
        return WinPostMsg(hwndOwner,
                          WM_CONTROL,
                          MPFROM2SHORT(WinQueryWindowUShort(hwndControl, QWS_ID),
                                       usCode),
                          mp2);

    return FALSE;
}

/*
 *@@ ctlInitDWD:
 *      ininitializes the DEFWINDATA struct for the
 *      given window. This must be called in WM_CREATE
 *      of a window proc if it intends to use
 *      ctlDefWindowProc as its default window procedure
 *      for generic message handling.
 *
 *      Parameter remarks:
 *
 *      --  For hwnd and mp2, pass in what you get in WM_CREATE.
 *
 *      --  pdwd must point to a static DEFWINDATA which will
 *          maintain instance data for this control. It is
 *          recommended to have a DEFWINDATA struct in your
 *          window's instance data, which you must allocate
 *          in WM_CREATE anyway. So just pass the address of
 *          that struct within your instance buffer.
 *
 *      --  pDefWindowProc is called by ctlDefWindowProc
 *          after its own message processing. In most cases,
 *          pass in WinDefWindowProc, unless you're superclassing
 *          a standard PM control.
 *
 *      --  flCtl fine-tunes the behavior of ctlDefWindowProc:
 *
 *          --  If CCS_NOSENDCTLPTR, we do not send WM_CONTROLPOINTER
 *              on every mouse move, but force the system "arrow"
 *              pointer over the control.
 *
 *      --  paCtlColors must point to a CCTLCOLOR array. This
 *          enables automatic presparams and syscolor caching
 *          within ctlDefWindowProc so your window gets invalidated
 *          automatically. If you always use ctlQueryColor() in your
 *          WM_PAINT handler to determine the colors to be used for
 *          painting, you have full presparams support.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

VOID ctlInitDWD(HWND hwnd,
                MPARAM mp2,                     // in: mp2 of WM_CREATE
                PDEFWINDATA pdwd,
                PFNWP pDefWindowProc,           // in: parent window proc
                ULONG flCtl,                    // in: CTL_* flags
                const CCTLCOLOR *paCtlColors,
                ULONG cCtlColors)
{
    pdwd->hwnd = hwnd;
    pdwd->hab = WinQueryAnchorBlock(hwnd);
    pdwd->pDefWindowProc = pDefWindowProc;
    pdwd->flCtl = flCtl;
    pdwd->szlWin.cx = ((PCREATESTRUCT)mp2)->cx;
    pdwd->szlWin.cy = ((PCREATESTRUCT)mp2)->cy;
    pdwd->paCtlColors = paCtlColors;
    pdwd->cCtlColors = cCtlColors;

    pdwd->palColorValues = (PLONG)malloc(sizeof(LONG) * cCtlColors);

    ctlRefreshColors(pdwd);
}

/*
 *@@ ctlRefreshColors:
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

VOID ctlRefreshColors(PDEFWINDATA pdwd)
{
    ULONG   ul;
    for (ul = 0;
         ul < pdwd->cCtlColors;
         ++ul)
    {
        pdwd->palColorValues[ul] = winhQueryPresColor2(pdwd->hwnd,
                                                       pdwd->paCtlColors[ul].ulPP,
                                                       0,
                                                       pdwd->paCtlColors[ul].fInheritPP,
                                                       pdwd->paCtlColors[ul].ulSysColor);
    }
}

/*
 *@@ ctlQueryColor:
 *      returns the color value corresponding to the given
 *      color index. The color index is interpreted according
 *      to the CCTLCOLOR array that was given to ctlInitDWD
 *      and must not be larger than the number of colors in
 *      that array minus 1.
 *
 *@@added V1.0.1 (2003-01-22) [umoeller]
 */

LONG ctlQueryColor(PDEFWINDATA pdwd, ULONG ulIndex)
{
    if (ulIndex < pdwd->cCtlColors)
        return pdwd->palColorValues[ulIndex];

    return 0;
}

/*
 *@@ ctlDefWindowProc:
 *      replacement default window procedure for controls that
 *      have a custom window class and do not inherit from
 *      standard OS/2 controls.
 *
 *      If a window proc wishes to use this, it must allocate
 *      its own private window data in WM_CREATE (preferrably in
 *      QWL_USER + 1) and have room for a DEFWINDATA struct in
 *      there. It must call ctlInitDWD in WM_CREATE also which
 *      initializes that structure. It can then safely pass
 *      messages to this function.
 *
 *@@added V1.0.1 (2002-11-30) [umoeller]
 */

MRESULT ctlDefWindowProc(PDEFWINDATA pdwd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    BOOL    fCallDefault = TRUE;
    HWND    hwndOwner;

    switch (msg)
    {
        case WM_MOUSEMOVE:
            if (    (!(pdwd->flCtl & CCS_NOSENDCTLPTR))
                 && (hwndOwner = WinQueryWindow(pdwd->hwnd, QW_OWNER))
               )
            {
                HPOINTER hptrArrow = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);
                WinSetPointer(HWND_DESKTOP,
                              (HPOINTER)WinSendMsg(hwndOwner,
                                                   WM_CONTROLPOINTER,
                                                   (MPARAM)WinQueryWindowUShort(pdwd->hwnd, QWS_ID),
                                                   (MPARAM)hptrArrow));
                fCallDefault = FALSE;
            }
        break;

        case WM_SYSCOLORCHANGE:
        case WM_PRESPARAMCHANGED:
            ctlRefreshColors(pdwd);
        break;

        case WM_ENABLE:
            WinInvalidateRect(pdwd->hwnd, NULL, TRUE);
        break;

        case WM_WINDOWPOSCHANGED:
            if (((PSWP)mp1)->fl & SWP_SIZE)
            {
                pdwd->szlWin.cx = ((PSWP)mp1)->cx;
                pdwd->szlWin.cy = ((PSWP)mp1)->cy;
            }
        break;

        case WM_DESTROY:
            FREE(pdwd->palColorValues);
        break;
    }

    if (fCallDefault)
        mrc = pdwd->pDefWindowProc(pdwd->hwnd, msg, mp1, mp2);

    return mrc;
}

/* ******************************************************************
 *
 *   "Separator line" control
 *
 ********************************************************************/

PFNWP   G_pfnwpSepStatic = NULL;

/*
 *@@ fnwpSeparatorLine:
 *      window proc for the subclassed static control that makes
 *      the "separator line" control.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added SEPS_VERTICAL
 *@@changed V1.0.1 (2002-11-30) [umoeller]: fixed default background color
 */

STATIC MRESULT EXPENTRY fnwpSeparatorLine(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_PAINT:
        {
            RECTL rcl;
            HPS hps;
            WinQueryWindowRect(hwnd, &rcl);
            if (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
            {
                POINTL ptl;

                gpihSwitchToRGB(hps);

                WinFillRect(hps,
                            &rcl,
                            winhQueryPresColor2(hwnd,
                                                PP_BACKGROUNDCOLOR,
                                                PP_BACKGROUNDCOLORINDEX,
                                                TRUE,
                                                SYSCLR_DIALOGBACKGROUND)); // SYSCLR_WINDOW));
                                                        // fixed V1.0.1 (2002-11-30) [umoeller]

                if (WinQueryWindowULong(hwnd, QWL_STYLE) & SEPS_VERTICAL)
                {
                    GpiSetColor(hps, G_lcol3DDark);

                    ptl.x = (rcl.xRight - rcl.xLeft) / 2 - 1;
                    ptl.y = rcl.yBottom;
                    GpiMove(hps, &ptl);
                    ptl.y = rcl.yTop;
                    GpiLine(hps, &ptl);

                    GpiSetColor(hps, G_lcol3DLight);

                    ptl.y = rcl.yBottom;
                    ++ptl.x;
                    GpiMove(hps, &ptl);
                    ptl.y = rcl.yTop;
                    GpiLine(hps, &ptl);
                }
                else
                {
                    GpiSetColor(hps, G_lcol3DLight);

                    ptl.x = rcl.xLeft;
                    ptl.y = (rcl.yTop - rcl.yBottom) / 2 - 1;
                    GpiMove(hps, &ptl);
                    ptl.x = rcl.xRight;
                    GpiLine(hps, &ptl);

                    GpiSetColor(hps, G_lcol3DDark);

                    ptl.x = rcl.xLeft;
                    ++ptl.y;
                    GpiMove(hps, &ptl);
                    ptl.x = rcl.xRight;
                    GpiLine(hps, &ptl);
                }

                WinEndPaint(hps);
            }
        }
        break;

        default:
            mrc = G_pfnwpSepStatic(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ ctlRegisterSeparatorLine:
 *      registers the separator line control, which is a dull
 *      static displaying a 3D line for use as a separator
 *      in dialogs.
 *
 *      In addition to the standard WS_* styles, the control
 *      supports the SEPS_VERTICAL style bit. If set, the
 *      separator is vertical; if not, it is horizontal.
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

BOOL ctlRegisterSeparatorLine(HAB hab)
{
    CLASSINFO ciStatic;
    if (WinQueryClassInfo(hab,
                          WC_STATIC,
                          &ciStatic))
    {
        G_pfnwpSepStatic = ciStatic.pfnWindowProc;

        return WinRegisterClass(hab,
                                WC_CCTL_SEPARATOR,
                                fnwpSeparatorLine,
                                (ciStatic.flClassStyle & ~CS_PUBLIC),
                                ciStatic.cbWindowData);

    }

    return FALSE;
}

/*
 *@@category: Helpers\PM helpers\Window classes\Menu buttons
 *      See comctl.c and ctlMakeMenuButton.
 */

/* ******************************************************************
 *
 *   "Menu button" control
 *
 ********************************************************************/

/*
 *@@ MENUBUTTONDATA:
 *      internal data for "menu button"
 *
 *@@added V0.9.0 [umoeller]
 */

typedef struct _MENUBUTTONDATA
{
    PFNWP       pfnwpButtonOriginal;
    HMODULE     hmodMenu;
    ULONG       idMenu;
    BOOL        fMouseCaptured,         // TRUE if WinSetCapture
                fMouseButton1Down,      // TRUE in between WM_BUTTON1DOWN and WM_BUTTON1UP
                fButtonSunk;            // toggle state of the button
    HWND        hwndMenu;
} MENUBUTTONDATA, *PMENUBUTTONDATA;

/*
 *@@ ctlDisplayButtonMenu:
 *      displays the specified menu above the button.
 *
 *@@added V0.9.7 (2000-11-29) [umoeller]
 */

VOID ctlDisplayButtonMenu(HWND hwndButton,
                          HWND hwndMenu)
{
    SWP     swpButton;
    POINTL  ptlMenu;
    WinQueryWindowPos(hwndButton, &swpButton);
    ptlMenu.x = swpButton.x;
    ptlMenu.y = swpButton.y;

    // ptlMenu now has button coordinates
    // relative to the button's parent;
    // convert this to screen coordinates:
    WinMapWindowPoints(WinQueryWindow(hwndButton, QW_PARENT),
                       HWND_DESKTOP,
                       &ptlMenu,
                       1);

    // now show the menu on top of the button
    WinPopupMenu(HWND_DESKTOP,               // menu parent
                 hwndButton,                 // owner
                 hwndMenu,
                 (SHORT)(ptlMenu.x),
                 (SHORT)(ptlMenu.y + swpButton.cy - 1),
                 0,                          // ID
                 PU_NONE
                     | PU_MOUSEBUTTON1
                     | PU_KEYBOARD
                     | PU_HCONSTRAIN
                     | PU_VCONSTRAIN);
}

/*
 *@@ ctl_fnwpSubclassedMenuButton:
 *      subclassed window proc for "menu button".
 *      See ctlMakeMenuButton for details.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.2 (2000-02-28) [umoeller]: menu was displayed even if button was disabled; fixed
 *@@changed V0.9.14 (2001-07-31) [umoeller]: fixed WM_MENUEND submenu quirk
 */

MRESULT EXPENTRY ctl_fnwpSubclassedMenuButton(HWND hwndButton, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PMENUBUTTONDATA pmbd = (PMENUBUTTONDATA)WinQueryWindowULong(hwndButton, QWL_USER);

    switch (msg)
    {
        /*
         * WM_BUTTON1DOWN:
         * WM_BUTTON1UP:
         *      these show/hide the menu.
         *
         *      Showing the menu follows these steps:
         *          a)  first WM_BUTTON1DOWN hilites the button;
         *          b)  first WM_BUTTON1UP shows the menu.
         *
         *      When the button is pressed again, the open
         *      menu loses focus, which results in WM_MENUEND
         *      and destroys the window automatically.
         */

        case WM_BUTTON1DOWN:
        case WM_BUTTON1DBLCLK:
            // only do this if the button is enabled
            // V0.9.2 (2000-02-28) [umoeller]
            if (WinIsWindowEnabled(hwndButton))
            {
                // _Pmpf(("WM_BUTTON1DOWN"));
                // since we're not passing the message
                // to WinDefWndProc, we need to give
                // ourselves the focus
                WinSetFocus(HWND_DESKTOP, hwndButton);

                if (!pmbd->fMouseCaptured)
                {
                    // capture mouse events while the
                    // mouse button is down
                    WinSetCapture(HWND_DESKTOP, hwndButton);
                    pmbd->fMouseCaptured = TRUE;
                }

                pmbd->fMouseButton1Down = TRUE;

                if (!pmbd->fButtonSunk)
                {
                    // button not hilited yet (first click):
                    // do it now
                    // _Pmpf(("  Sinking menu WM_BUTTON1DOWN "));
                    pmbd->fButtonSunk = TRUE;
                    WinSendMsg(hwndButton,
                               BM_SETHILITE,
                               (MPARAM)TRUE,
                               (MPARAM)0);
                }

                // else: the menu has just been destroyed
                // (WM_MENUEND below)
            }

            mrc = (MPARAM)TRUE;     // message processed
        break;

        /*
         * WM_BUTTON1UP:
         *
         */

        case WM_BUTTON1UP:
            // only do this if the button is enabled
            // V0.9.2 (2000-02-28) [umoeller]
            if (WinIsWindowEnabled(hwndButton))
            {
                // _Pmpf(("WM_BUTTON1UP sunk %d hwndMenu 0x%lX", pmbd->fButtonSunk, pmbd->hwndMenu));

                // un-capture the mouse first
                if (pmbd->fMouseCaptured)
                {
                    WinSetCapture(HWND_DESKTOP, NULLHANDLE);
                    pmbd->fMouseCaptured = FALSE;
                }

                pmbd->fMouseButton1Down = FALSE;

                if (    (pmbd->fButtonSunk)      // set by WM_BUTTON1DOWN above
                     && (pmbd->hwndMenu)         // menu currently showing
                   )
                {
                    // button currently depressed:
                    // un-hilite button
                    // _Pmpf(("  Unsinking menu WM_BUTTON1UP 1"));
                    pmbd->fButtonSunk = FALSE;
                    WinSendMsg(hwndButton,
                               BM_SETHILITE,
                               (MPARAM)FALSE,
                               (MPARAM)0);
                }
                else
                {
                    // first button up:
                    // show menu

                    if (pmbd->idMenu)
                    {
                        // _Pmpf(("  Loading menu hmod %lX id %lX", pmbd->hmodMenu, pmbd->idMenu));
                        // menu specified: load from
                        // specified resources
                        pmbd->hwndMenu = WinLoadMenu(hwndButton,
                                                     pmbd->hmodMenu,
                                                     pmbd->idMenu);
                    }
                    else
                    {
                        HWND    hwndOwner = WinQueryWindow(hwndButton, QW_OWNER);
                        // _Pmpf(("  Querying menu WM_COMMAND from owner 0x%lX", hwndOwner));
                        // send WM_COMMAND to owner
                        pmbd->hwndMenu
                            = (HWND)WinSendMsg(hwndOwner,
                                               WM_COMMAND,
                                               (MPARAM)(ULONG)WinQueryWindowUShort(hwndButton,
                                                                                   QWS_ID),
                                               (MPARAM)0);
                    }

                    // _Pmpf(("  Loaded menu, hwnd: 0x%lX", pmbd->hwndMenu));

                    if (pmbd->hwndMenu)
                    {
                        // menu successfully loaded:
                        // find out where to put it
                        ctlDisplayButtonMenu(hwndButton,
                                             pmbd->hwndMenu);
                    } // end if (pmbd->hwndMenu)
                    else
                    {
                        // menu not loaded:
                        // _Pmpf(("  Unsinking menu WM_BUTTON1UP 2"));
                        pmbd->fButtonSunk = FALSE;
                        WinSendMsg(hwndButton,
                                   BM_SETHILITE,
                                   (MPARAM)FALSE,
                                   (MPARAM)0);
                    }
                }
            }

            mrc = (MPARAM)TRUE;     // message processed
        break;

        /*
         * WM_BUTTON1CLICK:
         *      swallow this
         */

        case WM_BUTTON1CLICK:
            mrc = (MPARAM)TRUE;
        break;

        /*
         * WM_SETFOCUS:
         *      swallow this, the button keeps painting
         *      itself otherwise
         */

        case WM_SETFOCUS:
        break;

        /*
         * WM_MENUEND:
         *      menu is destroyed; we get this
         *      because we're the owner
         */

        case WM_MENUEND:
            if ((HWND)mp2 == pmbd->hwndMenu) // V0.9.14 (2001-07-31) [umoeller]
            {
                BOOL fUnHilite = TRUE;
                // _Pmpf(("WM_MENUEND"));
                // At this point, the menu has been
                // destroyed already.
                // Since WM_BUTTON1UP handles the
                // default case that the user presses
                // the menu button a second time while
                // the menu is open, we only need
                // to handle the case that the user
                // c)   presses some key on the menu (ESC, menu selection) or
                // a)   selects a menu item (which
                //      dismisses the menu) or
                // b)   clicks anywhere else.

                // Case a) if mouse button 1 is not currently down
                if ((WinGetKeyState(HWND_DESKTOP, VK_BUTTON1) & 0x8000) == 0)
                    // button 1 _not_ down:
                    // must be keyboard
                    fUnHilite = TRUE;
                else
                    // button 1 _is_ down:
                    // query window under mouse pointer
                    ;

                if (fUnHilite)
                {
                    // _Pmpf(("  Unsinking menu WM_MENUEND"));
                    pmbd->fButtonSunk = FALSE;
                    WinSendMsg(hwndButton,
                               BM_SETHILITE,
                               (MPARAM)FALSE,
                               (MPARAM)0);
                }
                pmbd->hwndMenu = NULLHANDLE;
            } // end if ((HWND)mp1 == pmbd->pmbd->hwndMenu) // V0.9.14 (2001-07-31) [umoeller]
        break;

        /*
         * WM_COMMAND:
         *      this must be from the menu, so
         *      forward this to the button's owner
         */

        case WM_COMMAND:
            WinPostMsg(WinQueryWindow(hwndButton, QW_OWNER),
                       msg,
                       mp1,
                       mp2);
        break;

        /*
         * WM_DESTROY:
         *      clean up allocated data
         */

        case WM_DESTROY:
            mrc = pmbd->pfnwpButtonOriginal(hwndButton, msg, mp1, mp2);
            free(pmbd);
        break;

        default:
            mrc = pmbd->pfnwpButtonOriginal(hwndButton, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ ctlMakeMenuButton:
 *      this turns the specified button into a "menu button",
 *      which shows a popup menu when the button is depressed.
 *      This is done by subclassing the menu button with
 *      ctl_fnwpSubclassedMenuButton.
 *
 *      Simply call this function upon any button, and it'll
 *      turn in to a menu button.
 *
 *      When the user presses the button, the specified menu
 *      is loaded from the resources. The button will then
 *      be set as the owner, but it will forward all WM_COMMAND
 *      messages from the menu to its own owner (probably your
 *      dialog), so you can handle WM_COMMAND messages just as
 *      if the menu was owned by your dialog.
 *
 *      Alternatively, if you don't want to load a menu from
 *      the resources, you can specify idMenu == 0. In that case,
 *      when the menu button is pressed, it sends (!) a WM_COMMAND
 *      message to its owner with its ID in mp1 (as usual). The
 *      difference is that the return value from that message will
 *      be interpreted as a menu handle (HWND) to use for the button
 *      menu.
 *
 *      The subclassed button also handles menu destruction etc.
 *      by itself.
 *
 *@@added V0.9.0 [umoeller]
 */

BOOL ctlMakeMenuButton(HWND hwndButton,      // in: button to subclass
                       HMODULE hmodMenu,     // in: resource module (can be NULLHANDLE for
                                             // current EXE)
                       ULONG idMenu)         // in: resource menu ID (or 0)
{
    BOOL brc = FALSE;
    PMENUBUTTONDATA pmbd;
    if (pmbd = (PMENUBUTTONDATA)malloc(sizeof(MENUBUTTONDATA)))
    {
        memset(pmbd, 0, sizeof(MENUBUTTONDATA));
        if (pmbd->pfnwpButtonOriginal = WinSubclassWindow(hwndButton,
                                                          ctl_fnwpSubclassedMenuButton))
        {
            pmbd->hmodMenu = hmodMenu;
            pmbd->idMenu = idMenu;
            WinSetWindowULong(hwndButton, QWL_USER, (ULONG)pmbd);
            brc = TRUE;
        }
        else
            free(pmbd);
    }

    return brc;
}

/*
 *@@category: Helpers\PM helpers\Window classes\Static bitmaps
 *      See comctl.c and ctl_fnwpBitmapStatic.
 */

/* ******************************************************************
 *
 *   Subclassed Static Bitmap Control
 *
 ********************************************************************/

/*
 *@@ ctl_fnwpBitmapStatic:
 *      subclassed wnd proc for both static controls which
 *      should display icons and stretched bitmaps.
 *
 *      This is not a stand-alone window procedure, but must only
 *      be used with static controls subclassed by the functions
 *      listed below.
 *
 *      This is now (V0.9.0) used in two contexts:
 *      --  when subclassed from ctlPrepareStaticIcon,
 *          this displays transparent icons properly
 *          (for regular icons or icon animations);
 *      --  when subclassed from ctlPrepareStretchedBitmap,
 *          this displays a stretched bitmap.
 *
 *      The behavior depends on ANIMATIONDATA.ulFlags,
 *      which is set by both of these functions (either to
 *      ANF_ICON or ANF_BITMAP).
 *
 *      Only one msg is of interest to the "user" application
 *      of this control, which is SM_SETHANDLE,
 *      the normal message sent to a static icon control to change
 *      its icon or bitmap.
 *      The new handle (HPOINTER or HBITMAP) is
 *      in mp1; mp2 must be null.
 *
 *      Depending on ANIMATIONDATA.ulFlags, we do the following:
 *
 *      --  ANF_ICON:  Unfortunately, the
 *          standard static control paints garbage if
 *          the icons contain transparent areas, and the
 *          display often flickers too.
 *          We improve this by creating one bitmap from
 *          the icon that we were given which we can then
 *          simply copy to the screen in one step in
 *          WM_PAINT.
 *
 *      --  ANF_BITMAP: we create a bitmap which is
 *          stretched to the size of the static control,
 *          using gpihStretchBitmap.
 *
 *      In any case, a new bitmap is produced internally and
 *      stored so that it can easily be painted when WM_PAINT
 *      comes in. The source icon/bitmap can therefore
 *      be safely destroyed by the caller after sending
 *      SM_SETHANDLE. This bitmap is automatically deleted when
 *      the window is destroyed.
 *
 *@@changed V0.9.0 [umoeller]: function renamed
 *@@changed V0.9.0 [umoeller]: added support for stretched bitmaps
 *@@changed V0.9.0 [umoeller]: added halftoned display for WS_DISABLED
 *@@changed V0.9.0 [umoeller]: exported gpihIcon2Bitmap function to gpih.c
 *@@changed V0.9.0 [umoeller]: fixed paint errors when SM_SETHANDLE had NULL argument in mp1
 *@@changed V0.9.16 (2001-10-15) [umoeller]: now centering icon in static properly
 *@@changed V0.9.16 (2001-10-15) [umoeller]: this always used the presparam colors of the parent instead of its own ones
 */

MRESULT EXPENTRY ctl_fnwpBitmapStatic(HWND hwndStatic, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PANIMATIONDATA pa = (PANIMATIONDATA)WinQueryWindowULong(hwndStatic, QWL_USER);
                // animation data which was stored in window words

    PFNWP   OldStaticProc = NULL;
    MRESULT mrc = NULL;

    if (pa)
    {
        OldStaticProc = pa->OldStaticProc;

        switch(msg)
        {
            /*
             * WM_TIMER:
             *      this timer is started by the ctl* funcs
             *      below. Proceed to the next animation step.
             *
             *      Note: this timer is only used with
             *      ANF_ICON, and even then, it
             *      is not necessarily running.
             */

            case WM_TIMER:
                pa->usAniCurrent++;
                if (pa->usAniCurrent >= pa->usAniCount)
                    pa->usAniCurrent = 0;

                WinSendMsg(hwndStatic,
                           SM_SETHANDLE,
                           (MPARAM)pa->ahptrAniIcons[pa->usAniCurrent],
                           (MPARAM)NULL);
            break;

            /*
             * SM_SETHANDLE:
             *
             */

            case SM_SETHANDLE:
            {
                HDC hdcMem;
                HPS hpsMem;
                SIZEL szlPage;

                LONG lBkgndColor
                    /* = winhQueryPresColor(WinQueryWindow(hwndStatic, QW_PARENT),
                                         PP_BACKGROUNDCOLOR,
                                         FALSE,
                                         SYSCLR_DIALOGBACKGROUND); */
                    // fixed this... V0.9.16 (2001-10-15) [umoeller]
                    = winhQueryPresColor(hwndStatic,
                                         PP_BACKGROUNDCOLOR,
                                         TRUE,
                                         SYSCLR_DIALOGBACKGROUND);

                HPS hps = WinGetPS(hwndStatic);

                // if we have created bitmaps previously, delete them
                if (pa->hbm)
                {
                    GpiDeleteBitmap(pa->hbm);
                    pa->hbm = NULLHANDLE;
                }
                if (pa->hbmHalftoned)
                {
                    GpiDeleteBitmap(pa->hbmHalftoned);
                    pa->hbmHalftoned = NULLHANDLE;
                }

                // switch to RGB mode
                gpihSwitchToRGB(hps);

                // create a memory PS
                szlPage.cx = pa->rclIcon.xRight - pa->rclIcon.xLeft;
                szlPage.cy = pa->rclIcon.yTop - pa->rclIcon.yBottom;
                if (gpihCreateMemPS(pa->hab,
                                    &szlPage,
                                    &hdcMem,
                                    &hpsMem))
                {
                    // switch the memory PS to RGB mode too
                    gpihSwitchToRGB(hpsMem);

                    // create a suitable bitmap w/ the size of the
                    // static control
                    if (    ((pa->hbm = gpihCreateBitmap(hpsMem,
                                                         szlPage.cx,
                                                         szlPage.cy)))
                            // associate the bit map with the memory PS
                         && (GpiSetBitmap(hpsMem, pa->hbm) != HBM_ERROR)
                       )
                    {
                        // fill the bitmap with the current static
                        // background color
                        POINTL ptl = {0, 0};
                        GpiMove(hpsMem, &ptl);
                        ptl.x = pa->rclIcon.xRight;
                        ptl.y = pa->rclIcon.yTop;
                        GpiSetColor(hpsMem,
                                    lBkgndColor);
                        GpiBox(hpsMem,
                               DRO_FILL, // interior only
                               &ptl,
                               0, 0);    // no corner rounding

                        /*
                         * ANF_ICON:
                         *
                         */

                        if (pa->ulFlags & ANF_ICON)
                        {
                            // store new icon in our own structure
                            if (pa->hptr = (HPOINTER)mp1)
                            {
                                // center the icon in the bitmap
                                // V0.9.16 (2001-10-15) [umoeller]

                                // replaced call V0.9.19 (2002-06-18) [umoeller]
                                gpihDrawPointer(hpsMem,
                                                (   (pa->rclIcon.xRight - pa->rclIcon.xLeft)
                                                  - pa->szlIcon.cx
                                                ) / 2,
                                                (   (pa->rclIcon.yTop - pa->rclIcon.yBottom)
                                                  - pa->szlIcon.cy
                                                ) / 2,
                                                pa->hptr,
                                                &pa->szlIcon,
                                                NULL,       // no clipping
                                                0);         // no mini
                            }

                        } // end if (pa->ulFlags & ANF_ICON)

                        /*
                         * ANF_BITMAP:
                         *
                         */

                        else if (pa->ulFlags & ANF_BITMAP)
                        {
                            // store passed bitmap
                            HBITMAP     hbmSource;
                            if (hbmSource = (HBITMAP)mp1)
                                gpihStretchBitmap(hpsMem,       // target
                                                  hbmSource,    // source
                                                  NULL,     // use size of bitmap
                                                  &pa->rclIcon,
                                                  ((pa->ulFlags & ANF_PROPORTIONAL)
                                                        != 0));

                        } // end if (pa->ulFlags & ANF_BITMAP)

                    } // end if (GpiSetBitmap(...
                      // && (pa->hbm = gpihCreateBitmap(hpsMem, &(pa->rclIcon)))

                    // in any case, clean up now
                    GpiDestroyPS(hpsMem);
                    DevCloseDC(hdcMem);
                } // end if (gpihCreateMemPS(hab, &hdcMem, &hpsMem))

                WinReleasePS(hps);

                // enforce WM_PAINT
                WinInvalidateRect(hwndStatic, NULL, FALSE);
            }
            break;

            /*
             * WM_PAINT:
             *      "normal" paint; this only arrives here if the
             *      icon needs to be repainted. We simply paint
             *      the bitmap we created in WM_SETHANDLE.
             */

            case WM_PAINT:
            {
                RECTL rcl;
                HPS hps = WinBeginPaint(hwndStatic, NULLHANDLE, &rcl);
                POINTL ptl = {0, 0};

                if (pa->hbm)
                {
                    if (WinQueryWindowULong(hwndStatic, QWL_STYLE)
                            & WS_DISABLED)
                    {
                        // if the control is currently disabled,
                        // draw in half-toned (grayed)

                        LONG lBkgndColor = winhQueryPresColor(
                                        WinQueryWindow(hwndStatic, QW_PARENT),
                                        PP_BACKGROUNDCOLOR,
                                        FALSE,
                                        SYSCLR_DIALOGBACKGROUND);

                        // 1) check if we created the half-tone
                        // bitmap already (WinDrawBitmap doesn't
                        // work with half-toned color bitmaps, so
                        // here's yet another thing we have to do
                        // all alone)
                        if (pa->hbmHalftoned == NULLHANDLE)
                            pa->hbmHalftoned = gpihCreateHalftonedBitmap(pa->hab,
                                                                         pa->hbm,
                                                                         lBkgndColor);

                        if (pa->hbmHalftoned)
                            WinDrawBitmap(hps,
                                          pa->hbmHalftoned,
                                          NULL,     // whole bmp
                                          &ptl,     // lower left corner
                                          0, 0,     // no colors
                                          DBM_NORMAL);
                    }
                    else
                    {
                        // not disabled: draw regular bitmap
                        // we created previously
                        // draw the bitmap that we created previously
                        WinDrawBitmap(hps,
                                      pa->hbm,
                                      NULL,     // whole bmp
                                      &ptl,     // lower left corner
                                      0, 0,     // no colors
                                      DBM_NORMAL);
                    }
                }

                WinEndPaint(hps);
            }
            break;

            /*
             * WM_DESTROY:
             *      clean up.
             */

            case WM_DESTROY:
                // undo subclassing in case more WM_TIMERs come in
                WinSubclassWindow(hwndStatic, OldStaticProc);

                if (pa->hbm)
                    GpiDeleteBitmap(pa->hbm);
                if (pa->hbmHalftoned)
                    GpiDeleteBitmap(pa->hbm);

                // clean up ANIMATIONDATA struct
                WinSetWindowULong(hwndStatic, QWL_USER, (ULONG)NULL);
                free(pa);

                mrc = OldStaticProc(hwndStatic, msg, mp1, mp2);
            break;

            default:
                mrc = OldStaticProc(hwndStatic, msg, mp1, mp2);
       }
    }
    return mrc;
}

/* ******************************************************************
 *
 *   Icon animation
 *
 ********************************************************************/

/*
 *@@ CreateAnimationData:
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

STATIC PANIMATIONDATA CreateAnimationData(HAB hab,
                                          HWND hwndStatic,
                                          USHORT cAnimations)
{
    PANIMATIONDATA pa = NULL;

    if (cAnimations >= 1)
    {
        // create the ANIMATIONDATA structure,
        // initialize some fields,
        // and store it in QWL_USER of the static control
        ULONG   cbStruct =   sizeof(ANIMATIONDATA)
                           + ((cAnimations - 1) * sizeof(HPOINTER));

        if (pa = (PANIMATIONDATA)malloc(cbStruct))
        {
            memset(pa, 0, cbStruct);

            WinSetWindowULong(hwndStatic, QWL_USER, (ULONG)pa);

            pa->hab = hab;
            WinQueryWindowRect(hwndStatic, &pa->rclIcon);
            pa->OldStaticProc = WinSubclassWindow(hwndStatic, ctl_fnwpBitmapStatic);
            pa->szlIcon.cx = G_cxIcon;
            pa->szlIcon.cy = G_cyIcon;
        }
    }

    return pa;
}

/*
 *@@ ctlPrepareStaticIcon:
 *      turns a static control into one which properly
 *      displays transparent icons when given a
 *      SM_SETHANDLE msg.
 *      This is done by subclassing the static control
 *      with ctl_fnwpBitmapStatic.
 *
 *      This function gets called automatically by
 *      ctlPrepareAnimation, so you need not call it
 *      independently for animations. See the notes there
 *      how this can be done.
 *
 *      You can, however, call this function if you
 *      have some static control with transparent icons
 *      which is not animated but changes sometimes
 *      (because you have the same repaint problems there).
 *
 *      This func does _not_ start an animation yet.
 *
 *      To change the icon being displayed, send the control
 *      a SM_SETHANDLE msg with the icon handle in (HPOINTER)mp1.
 *
 *      Returns a PANIMATIONDATA if subclassing succeeded or
 *      NULL upon errors. If you only call this function, you
 *      do not need this structure; it is needed by
 *      ctlPrepareAnimation though.
 *
 *      The subclassed static icon func above automatically
 *      cleans up resources, so you don't have to worry about
 *      that either.
 *
 *@@changed V0.9.0 [umoeller]: adjusted for ctl_fnwpBitmapStatic changes
 */

PANIMATIONDATA ctlPrepareStaticIcon(HWND hwndStatic,
                                    USHORT usAnimCount) // needed for allocating extra memory,
                                                        // this must be at least 1
{
    PANIMATIONDATA pa;
    HAB     hab;
    if (    (hwndStatic)
         && (hab = WinQueryAnchorBlock(hwndStatic))
         && (WinIsWindow(hab, hwndStatic))
         && (pa = CreateAnimationData(hab,
                                      hwndStatic,
                                      usAnimCount))
       )
    {
        // switch static to icon mode
        pa->ulFlags = ANF_ICON;
        return pa;
    }

    return NULL;
}

/*
 *@@ ctlPrepareAnimation:
 *      this function turns a regular static icon
 *      control an animated one. This is the one-shot
 *      function for animating static icon controls.
 *
 *      It calls ctlPrepareStaticIcon first, then uses
 *      the passed parameters to prepare an animation:
 *
 *      pahptr must point to an array of HPOINTERs
 *      which must contain usAnimCount icon handles.
 *      If (fStartAnimation == TRUE), the animation
 *      is started already. Otherwise you'll have
 *      to call ctlStartAnimation yourself.
 *
 *      To create an animated icon, simply create a static
 *      control with the dialog editor (can be any static,
 *      e.g. a text control). Then do the following in your code:
 *
 *      1) load the dlg box template (using WinLoadDlg);
 *
 *      2) load all the icons for the animation in an array of
 *         HPOINTERs (check xsdLoadAnimation in shutdown.c for an
 *         example);
 *
 *      3) call ctlPrepareAnimation, e.g.:
 +             ctlPrepareAnimation(WinWindowFromID(hwndDlg, ID_ICON), // get icon hwnd
 +                             8,                   // no. of icons for the anim
 +                             &ahptr[0],           // ptr to first icon in the array
 +                             150,                 // delay
 +                             TRUE);               // start animation now
 *
 *      4) call WinProcessDlg(hwndDlg);
 *
 *      5) stop the animation, e.g.:
 +              ctlStopAnimation(WinWindowFromID(hwndDlg, ID_ICON));
 *
 *      6) destroy the dlg window;
 *
 *      7) free all the HPOINTERS loaded above (check xsdFreeAnimation
 *         in shutdown.c for an example).
 *
 *      When the icon control is destroyed, the
 *      subclassed window proc (ctl_fnwpBitmapStatic)
 *      automatically cleans up resources. However,
 *      the icons in *pahptr are not freed.
 */

BOOL ctlPrepareAnimation(HWND hwndStatic,   // icon hwnd
                         USHORT usAnimCount,         // no. of anim steps
                         HPOINTER *pahptr,           // array of HPOINTERs
                         ULONG ulDelay,              // delay per anim step (in ms)
                         BOOL fStartAnimation)       // TRUE: start animation now
{
    PANIMATIONDATA paNew;
    if (paNew = ctlPrepareStaticIcon(hwndStatic, usAnimCount))
    {
        paNew->ulDelay = ulDelay;
        // paNew->OldStaticProc already set
        paNew->hptr = NULLHANDLE;
        // paNew->rclIcon already set
        paNew->usAniCurrent = 0;
        paNew->usAniCount = usAnimCount;
        memcpy(&paNew->ahptrAniIcons,
               pahptr,
               (usAnimCount * sizeof(HPOINTER)));

        if (fStartAnimation)
        {
            WinStartTimer(paNew->hab,
                          hwndStatic,
                          1,
                          ulDelay);
            WinPostMsg(hwndStatic, WM_TIMER, (MPARAM)1, NULL);
        }
    }

    return (paNew != NULL);
}

/*
 *@@ ctlStartAnimation:
 *      starts an animation that not currently running. You
 *      must prepare the animation by calling ctlPrepareAnimation
 *      first.
 */

BOOL ctlStartAnimation(HWND hwndStatic)
{
    BOOL brc = FALSE;
    PANIMATIONDATA pa;
    if (    (pa = (PANIMATIONDATA)WinQueryWindowULong(hwndStatic, QWL_USER))
         && (WinStartTimer(pa->hab,
                           hwndStatic,
                           1,
                           pa->ulDelay))
       )
    {
        brc = TRUE;
        WinPostMsg(hwndStatic, WM_TIMER, (MPARAM)1, NULL);
    }

    return brc;
}

/*
 *@@ ctlStopAnimation:
 *      stops an animation that is currently running.
 *      This does not free any resources.
 */

BOOL ctlStopAnimation(HWND hwndStatic)
{
    return WinStopTimer(WinQueryAnchorBlock(hwndStatic), hwndStatic, 1);
}

/*
 *@@category: Helpers\PM helpers\Window classes\Stretched bitmaps
 *      See comctl.c and ctlPrepareStretchedBitmap.
 */

/* ******************************************************************
 *
 *   Bitmap functions
 *
 ********************************************************************/

/*
 *@@ ctlPrepareStretchedBitmap:
 *      turns a static control into a bitmap control
 *      which stretches the bitmap to the size of the
 *      control when given an SM_SETHANDLE msg.
 *
 *      If (fPreserveProportions == TRUE), the control
 *      will size the bitmap proportionally only. Otherwise,
 *      it will always stretch the bitmap to the whole
 *      size of the static control (possibly distorting
 *      the proportions). See gpihStretchBitmap for
 *      details.
 *
 *      This function subclasses the static control
 *      with ctl_fnwpBitmapStatic, setting the flags as
 *      necessary. However, this does not set the bitmap
 *      to display yet.
 *      To have a bitmap displayed, send the control
 *      a SM_SETHANDLE message with the bitmap to be
 *      displayed in (HBITMAP)mp1 after having used this
 *      function.
 *
 *      ctl_fnwpBitmapStatic will then create a private
 *      copy of that bitmap, stretched to its own size.
 *      You can therefore delete the "source" bitmap
 *      after sending SM_SETHANDLE.
 *
 *      You can also send SM_SETHANDLE several times.
 *      The control will then readjust itself and delete
 *      its old bitmap.
 *      But note that GpiWCBitBlt is invoked on that new
 *      bitmap with every new message, so this is not
 *      terribly fast.
 *
 *      Also note that you should not resize the static
 *      control after creation, because the bitmap will
 *      _not_ be adjusted.
 *
 *      This returns a PANIMATIONDATA if subclassing succeeded or
 *      NULL upon errors. You do not need to save this structure;
 *      it is cleaned up automatically when the control is destroyed.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.16 (2001-10-15) [umoeller]: some cleanup
 */

PANIMATIONDATA ctlPrepareStretchedBitmap(HWND hwndStatic,
                                         BOOL fPreserveProportions)
{
    PANIMATIONDATA pa;
    HAB hab;
    if (    (hwndStatic)
         && (hab = WinQueryAnchorBlock(hwndStatic))
         && (WinIsWindow(hab, hwndStatic))
         && (pa = CreateAnimationData(hab, hwndStatic, 1))
       )
    {
        // switch static to bitmap mode
        pa->ulFlags = ANF_BITMAP;
        if (fPreserveProportions)
            pa->ulFlags |= ANF_PROPORTIONAL;
        return pa;
    }

    return NULL;
}

/*
 *@@category: Helpers\PM helpers\Window classes\Hotkey entry field
 *      See comctl.c and ctl_fnwpObjectHotkeyEntryField.
 */

/* ******************************************************************
 *
 *   Hotkey entry field
 *
 ********************************************************************/

/*
 *@@ ctl_fnwpHotkeyEntryField:
 *      this is the window proc for a subclassed entry
 *      field which displays a description of a keyboard
 *      combination upon receiving WM_CHAR.
 *
 *      Use ctlMakeHotkeyEntryField to turn any existing
 *      entry field into such an "hotkey entry field".
 *
 *      The entry field is deleted on every WM_CHAR that
 *      comes in.
 *
 *      For this to work, this window proc needs the
 *      cooperation of its owner. Whenever a WM_CHAR
 *      message is received by the entry field which
 *      looks like a meaningful key or key combination
 *      (some filtering is done by this func), then
 *      the owner of the entry field receives a WM_CONTROL
 *      message with the new EN_HOTKEY notification code
 *      (see comctl.h for details).
 *
 *      It is then the responsibility of the owner to
 *      check whether the HOTKEYNOTIFY structure pointed
 *      to by WM_CONTROL's mp2 is a meaningful keyboard
 *      combination.
 *
 *      If not, the owner must return FALSE, and the
 *      entry field's contents are deleted.
 *
 *      If yes however, the owner must fill the szDescription
 *      field with a description of the keyboard combination
 *      (e.g. "Shift+Alt+C") and return TRUE. The entry field
 *      will then be set to that description.
 *
 *@@added V0.9.1 (99-12-19) [umoeller]
 *@@changed V0.9.16 (2001-12-08) [umoeller]: fixed empty entry field on tab key
 */

MRESULT EXPENTRY ctl_fnwpObjectHotkeyEntryField(HWND hwndEdit, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = (MPARAM)FALSE; // WM_CHAR not-processed flag

    // get object window data; this was stored in QWL_USER by
    // WM_INITDLG of obj_fnwpSettingsObjDetails
    // PXFOBJWINDATA   pWinData = (PXFOBJWINDATA)WinQueryWindowULong(hwndEdit, QWL_USER);
    PFNWP           pfnwpOrig = (PFNWP)WinQueryWindowULong(hwndEdit, QWL_USER);

    switch (msg)
    {
        case WM_CHAR:
        {
            /*
                  #define KC_CHAR                    0x0001
                  #define KC_VIRTUALKEY              0x0002
                  #define KC_SCANCODE                0x0004
                  #define KC_SHIFT                   0x0008
                  #define KC_CTRL                    0x0010
                  #define KC_ALT                     0x0020
                  #define KC_KEYUP                   0x0040
                  #define KC_PREVDOWN                0x0080
                  #define KC_LONEKEY                 0x0100
                  #define KC_DEADKEY                 0x0200
                  #define KC_COMPOSITE               0x0400
                  #define KC_INVALIDCOMP             0x0800
            */

            /*
                Examples:           usFlags  usKeyCode
                    F3                02       22
                    Ctrl+F4           12       23
                    Ctrl+Shift+F4     1a       23
                    Ctrl              12       0a
                    Alt               22       0b
                    Shift             0a       09
            */

            // USHORT usCommand;
            // USHORT usKeyCode;
            USHORT usFlags    = SHORT1FROMMP(mp1);
            // UCHAR  ucRepeat   = CHAR3FROMMP(mp1);
            UCHAR  ucScanCode = CHAR4FROMMP(mp1);
            USHORT usch       = SHORT1FROMMP(mp2);
            USHORT usvk       = SHORT2FROMMP(mp2);

            if (
                    // process only key-down messages
                    ((usFlags & KC_KEYUP) == 0)
                    // check flags:
                    // filter out those ugly composite key (accents etc.)
                 && ((usFlags & (KC_DEADKEY | KC_COMPOSITE | KC_INVALIDCOMP)) == 0)
               )
            {
                HOTKEYNOTIFY    hkn;
                ULONG           flReturned;
                HWND            hwndOwner = WinQueryWindow(hwndEdit, QW_OWNER);

                usFlags &= (KC_VIRTUALKEY | KC_CTRL | KC_ALT | KC_SHIFT);

                hkn.usFlags = usFlags;
                hkn.usvk = usvk;
                hkn.usch = usch;
                hkn.ucScanCode = ucScanCode;

                if (usFlags & KC_VIRTUALKEY)
                    hkn.usKeyCode = usvk;
                else
                    hkn.usKeyCode = usch;

                hkn.szDescription[0] = 0;

                flReturned = (ULONG)WinSendMsg(hwndOwner,
                                               WM_CONTROL,
                                               MPFROM2SHORT(WinQueryWindowUShort(hwndEdit,
                                                                                 QWS_ID),
                                                            EN_HOTKEY),  // new notification code
                                               (MPARAM)&hkn);
                if (flReturned & HEFL_SETTEXT)
                    WinSetWindowText(hwndEdit, hkn.szDescription);
                else if (flReturned & HEFL_FORWARD2OWNER)
                    WinPostMsg(hwndOwner,
                               WM_CHAR,
                               mp1, mp2);
                else
                    // fixed V0.9.16 (2001-12-06) [umoeller]:
                    // do not clear the entry field if we had HEFL_FORWARD2OWNER
                    WinSetWindowText(hwndEdit, "");

                mrc = (MPARAM)TRUE; // WM_CHAR processed flag;
            }
        }
        break;

        default:
            mrc = pfnwpOrig(hwndEdit, msg, mp1, mp2);
    }
    return mrc;
}

/*
 *@@ ctlMakeHotkeyEntryField:
 *      this turns any entry field into a "hotkey entry field"
 *      by subclassing it with ctl_fnwpObjectHotkeyEntryField.
 *      See remarks there for details.
 *
 *      Returns FALSE upon errors, TRUE otherwise.
 *
 *@@added V0.9.1 (99-12-19) [umoeller]
 */

BOOL ctlMakeHotkeyEntryField(HWND hwndHotkeyEntryField)
{
    PFNWP pfnwpOrig;
    if (pfnwpOrig = WinSubclassWindow(hwndHotkeyEntryField,
                                      ctl_fnwpObjectHotkeyEntryField))
    {
        WinSetWindowPtr(hwndHotkeyEntryField, QWL_USER, (PVOID)pfnwpOrig);
        return TRUE;
    }

    return FALSE;
}

/* ******************************************************************
 *
 *   Color rectangle
 *
 ********************************************************************/

STATIC PFNWP G_pfnwpColorRectOrigStatic = NULL;

/*
 *@@ ctl_fnwpSubclassedColorRect:
 *      window procedure for subclassed static frames representing
 *      a color.
 *
 *      The control simply paints itself as a rectangle with the
 *      color specified in its PP_BACKGROUNDCOLOR presentation
 *      parameter. If a window text is set for the control, it is
 *      painted with the PP_FOREGROUNDCOLOR color.
 *
 *      If the user drags a color onto the control, it notifies
 *      its owner with the WM_CONTROL message and the EN_CHANGE
 *      notification code, as with any entry field (since the
 *      static control knows no notifications, we use that code
 *      instead).
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 *@@changed V0.9.19 (2002-06-02) [umoeller]: moved this here from w_pulse.c
 */

STATIC MRESULT EXPENTRY ctl_fnwpSubclassedColorRect(HWND hwndStatic, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_PAINT:
        {
            LONG    lColor;
            RECTL   rclPaint;
            PSZ     pszText;

            HPS hps = WinBeginPaint(hwndStatic,
                                    NULLHANDLE, // HPS
                                    NULL); // PRECTL
            gpihSwitchToRGB(hps);
            WinQueryWindowRect(hwndStatic,
                               &rclPaint);      // exclusive
            lColor = winhQueryPresColor(hwndStatic,
                                        PP_BACKGROUNDCOLOR,
                                        FALSE,      // no inherit
                                        SYSCLR_DIALOGBACKGROUND);

            // make rect inclusive
            rclPaint.xRight--;
            rclPaint.yTop--;

            // draw interior
            GpiSetColor(hps, lColor);
            gpihBox(hps,
                    DRO_FILL,
                    &rclPaint);

            // draw frame
            GpiSetColor(hps, RGBCOL_BLACK);
            gpihBox(hps,
                    DRO_OUTLINE,
                    &rclPaint);

            if (pszText = winhQueryWindowText(hwndStatic))
            {
                GpiSetColor(hps,
                            winhQueryPresColor(hwndStatic,
                                               PP_FOREGROUNDCOLOR,
                                               FALSE,
                                               -1));
                WinDrawText(hps,
                            strlen(pszText),
                            pszText,
                            &rclPaint,
                            0,
                            0,
                            DT_CENTER | DT_VCENTER | DT_TEXTATTRS);

                free(pszText);
            }

            WinEndPaint(hps);
        }
        break;

        case WM_PRESPARAMCHANGED:
            switch ((ULONG)mp1)
            {
                case PP_BACKGROUNDCOLOR:
                    WinInvalidateRect(hwndStatic,
                                      NULL,
                                      FALSE);
                    // notify owner; since the static control
                    // doesn't send any notifications, we
                    // use EN_CHANGED
                    ctlSendWmControl(hwndStatic,
                                     EN_CHANGE,
                                     (MPARAM)hwndStatic);
                break;
            }
        break;

        default:
            mrc = G_pfnwpColorRectOrigStatic(hwndStatic, msg, mp1, mp2);
        break;
    }

    return mrc;
}

/*
 *@@ ctlMakeColorRect:
 *      turns a static rectangle into a color rectangle.
 *
 *@@added V0.9.19 (2002-06-02) [umoeller]
 */

BOOL ctlMakeColorRect(HWND hwndStatic)
{
    PFNWP pfnwp;

    if (pfnwp = WinSubclassWindow(hwndStatic,
                                  ctl_fnwpSubclassedColorRect))
    {
        G_pfnwpColorRectOrigStatic = pfnwp;
        return TRUE;
    }

    return FALSE;
}


