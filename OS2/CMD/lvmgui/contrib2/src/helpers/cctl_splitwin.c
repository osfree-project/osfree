
/*
 *@@sourcefile cctl.splitwin.c:
 *      implementation for split windows.
 *      See comctl.c for an overview.
 *
 *      This has been extracted from comctl.c with V0.9.3 (2000-05-21) [umoeller].
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 *@@added V0.9.3 (2000-05-21) [umoeller].
 */

/*
 *      Copyright (C) 1997-2006 Ulrich M”ller.
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

#include "helpers\cnrh.h"
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\winh.h"

#include "helpers\comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Split windows
 *      See cctl_splitwin.c.
 */

/* ******************************************************************
 *
 *   Split windows
 *
 ********************************************************************/

/*
 *@@ PaintSplitWindow:
 *      implementation for WM_PAINT in ctl_fnwpSplitWindow.
 *
 *@@added V0.9.1 (2000-02-05) [umoeller]
 *@@changed V1.0.0 (2002-08-24) [umoeller]: added SBCF_3DEXPLORERSTYLE
 */

STATIC VOID PaintSplitWindow(HWND hwndSplit)
{
    HPS hps;

    if (hps = WinBeginPaint(hwndSplit, (HPS)0, NULL))
    {
        HWND    hwndSplitBar = WinWindowFromID(hwndSplit, ID_SPLITBAR);
        PSPLITBARDATA pData = (PSPLITBARDATA)WinQueryWindowULong(hwndSplitBar,
                                                                 QWL_USER);

        if ((pData->hwndLinked1) && (pData->hwndLinked2))
        {
            SWP     swp;

            // switch to RGB mode
            gpihSwitchToRGB(hps);

            // "3D-sunk" style?
            if (pData->sbcd.ulCreateFlags & SBCF_3DEXPLORERSTYLE)
            {
                // this style is new with V1.0.0 (2002-08-24) [umoeller];
                // simulate the Warp 4 entry field margins around the
                // right control only, but leave the left control flat.
                RECTL rcl;
                WinQueryWindowPos(pData->hwndLinked2, &swp);
                rcl.xLeft = swp.x - 2;
                rcl.yBottom = swp.y - 2;
                rcl.xRight = swp.x + swp.cx + 1;
                rcl.yTop = swp.y + swp.cy + 1;
                gpihDraw3DFrame2(hps,
                                 &rcl,
                                 1,
                                 RGBCOL_BLACK,
                                 pData->lcol3DLight);
                gpihDraw3DFrame2(hps,
                                 &rcl,
                                 1,
                                 RGBCOL_BLACK,
                                 pData->lcolInactiveBorder);
            }
            else if (pData->sbcd.ulCreateFlags & SBCF_3DSUNK)
            {
                // yes: draw sunk frame around split windows
                POINTL  ptl1;

                WinQueryWindowPos(pData->hwndLinked1, &swp);
                GpiSetColor(hps, pData->lcol3DDark);
                ptl1.x = swp.x - 1;
                ptl1.y = swp.y - 1;
                GpiMove(hps, &ptl1);
                ptl1.y = swp.y + swp.cy;
                GpiLine(hps, &ptl1);
                ptl1.x = swp.x + swp.cx;
                GpiLine(hps, &ptl1);
                GpiSetColor(hps, pData->lcol3DLight);
                ptl1.y = swp.y - 1;
                GpiLine(hps, &ptl1);
                ptl1.x = swp.x - 1;
                GpiLine(hps, &ptl1);

                WinQueryWindowPos(pData->hwndLinked2, &swp);
                GpiSetColor(hps, pData->lcol3DDark);
                ptl1.x = swp.x - 1;
                ptl1.y = swp.y - 1;
                GpiMove(hps, &ptl1);
                ptl1.y = swp.y + swp.cy;
                GpiLine(hps, &ptl1);
                ptl1.x = swp.x + swp.cx;
                GpiLine(hps, &ptl1);
                GpiSetColor(hps, pData->lcol3DLight);
                ptl1.y = swp.y - 1;
                GpiLine(hps, &ptl1);
                ptl1.x = swp.x - 1;
                GpiLine(hps, &ptl1);
            }
        }

        WinEndPaint(hps);
    }
}

/*
 *@@ ctl_fnwpSplitWindow:
 *      this is the window procedure of the "invisible"
 *      split window. One of these windows is created
 *      for each split view and has exactly three children:
 *
 *      1)  the split bar (ctl_fnwpSplitBar);
 *
 *      2)  the left or bottom window linked to the split bar;
 *
 *      3)  the right or top window linked to the split bar.
 *
 *      See ctlCreateSplitWindow for more info and a detailed
 *      window hierarchy.
 *
 *      This stand-alone window procedure must be registered.
 *      ctlCreateSplitWindow does this for you.
 *
 *@@added V0.9.0 [umoeller]
 */

MRESULT EXPENTRY ctl_fnwpSplitWindow(HWND hwndSplit, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = (MRESULT)0;

    switch(msg)
    {
        /*
         * WM_WINDOWPOSCHANGED:
         *
         */

        case WM_WINDOWPOSCHANGED:
            // this msg is passed two SWP structs:
            // one for the old, one for the new data
            // (from PM docs)
            if (((PSWP)mp1)->fl & SWP_SIZE)
                // _Pmpf(("ctl_fnwpSplitWindow, WM_WINDOWPOSCHANGED"));
                ctlUpdateSplitWindow(hwndSplit);

            mrc = WinDefWindowProc(hwndSplit, msg, mp1, mp2);
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
            PaintSplitWindow(hwndSplit);
        break;

        /*
         *@@ SPLM_SETLINKS:
         *      this specifies the windows which the
         *      split window will link. This updates
         *      the internal SPLITBARDATA and changes
         *      the parents of the two subwindows to
         *      the split window.
         *
         *      Parameters:
         *      -- HWND mp1:   left or bottom subwindow
         *      -- HWND mp2:   right or top  subwindow
         */

        case SPLM_SETLINKS:
        {
            HWND    hwndSplitBar = WinWindowFromID(hwndSplit, ID_SPLITBAR);
            PSPLITBARDATA pData;
            if (pData = (PSPLITBARDATA)WinQueryWindowULong(hwndSplitBar,
                                                           QWL_USER))
            {
                pData->hwndLinked1 = (HWND)mp1;
                pData->hwndLinked2 = (HWND)mp2;
                // change parents of the windows to link
                WinSetParent(pData->hwndLinked1, hwndSplit,
                             FALSE);        // no redraw
                WinSetParent(pData->hwndLinked2, hwndSplit,
                             FALSE);        // no redraw
            }
        }
        break;

        default:
            mrc = WinDefWindowProc(hwndSplit, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ TrackSplitBar:
 *      implementation for WM_BUTTON1DOWN/WM_BUTTON2DOWN in ctl_fnwpSplitBar.
 *
 *@@added V0.9.1 (2000-02-05) [umoeller]
 *@@changed XWP V1.0.5 (2006-04-13) [pr]: Fix horrid rounding errors
 */

STATIC VOID TrackSplitBar(HWND hwndBar,
                          PSPLITBARDATA pData)
{
    TRACKINFO   track;
    RECTL       rclBar;

    track.cxBorder = 2;
    track.cyBorder = 2;
    track.cxGrid   = 1;
    track.cyGrid   = 1;
    track.cxKeyboard = 8;
    track.cyKeyboard = 8;

    WinQueryWindowRect(hwndBar, &rclBar);
    WinMapWindowPoints(hwndBar,
                       HWND_DESKTOP,        // map to screen coords.
                       (PPOINTL)&rclBar,
                       2);          // 2 points == rectangle

    // limit tracking space to parent window;
    // this is either the client or another split window
    WinQueryWindowRect(pData->sbcd.hwndParentAndOwner,
                       &track.rclBoundary);
    WinMapWindowPoints(pData->sbcd.hwndParentAndOwner,
                       HWND_DESKTOP,        // map to screen coords.
                       (PPOINTL)&track.rclBoundary,
                       2);      // 2 points, since we have a RECTL

    // decrease tracking limits by what was
    // specified by the caller
    if (pData->sbcd.ulCreateFlags & SBCF_HORIZONTAL)
    {
        // horizontal split bar
        track.rclBoundary.yBottom += pData->sbcd.ulLeftOrBottomLimit;
        track.rclBoundary.yTop -= pData->sbcd.ulRightOrTopLimit;
        track.rclBoundary.xLeft = rclBar.xLeft;
        track.rclBoundary.xRight = rclBar.xRight;
    }
    else
    {
        // vertical split bar
        track.rclBoundary.xLeft += pData->sbcd.ulLeftOrBottomLimit;
        track.rclBoundary.xRight -= pData->sbcd.ulRightOrTopLimit;
        track.rclBoundary.yBottom = rclBar.yBottom;
        track.rclBoundary.yTop = rclBar.yTop;
    }

    // initial tracking rectangle = split bar
    memcpy(&track.rclTrack, &rclBar, sizeof(rclBar));

    track.ptlMinTrackSize.x =
    track.ptlMaxTrackSize.x = track.rclTrack.xRight
                            - track.rclTrack.xLeft;     // width of status bar
    track.ptlMinTrackSize.y =
    track.ptlMaxTrackSize.y = track.rclTrack.yTop
                            - track.rclTrack.yBottom;   // height of status bar

    track.fs = TF_MOVE | TF_ALLINBOUNDARY;

    // now do the tracking: this is a modal
    // operation and returns only after the
    // mouse has been released
    if (WinTrackRect(HWND_DESKTOP, 0, &track))
    {
        // OK, successfully moved: reposition the
        // windows which are linked to the split bar

        if (pData->sbcd.ulCreateFlags & SBCF_HORIZONTAL)
        {
            // horizontal split bar
            ULONG ulNewYPos = track.rclTrack.yBottom
                            - track.rclBoundary.yBottom;
            // add the limit specified by the caller
            ulNewYPos += pData->sbcd.ulLeftOrBottomLimit;

            if (pData->sbcd.ulCreateFlags & SBCF_PERCENTAGE)
            {
                // status bar pos is determined by
                // a percentage:
                // well, we'll need a new percentage then
                RECTL rclClient;
                WinQueryWindowRect(pData->sbcd.hwndParentAndOwner,
                                   &rclClient);
                // XWP V1.0.5 (2006-04-13) [pr]: Fix horrid rounding errors
                pData->sbcd.lPos = (ulNewYPos * 200 + (rclClient.yTop - rclClient.yBottom))
                                   / ((rclClient.yTop - rclClient.yBottom) * 2);
            }
            else
                // absolute split bar positioning:
                if (pData->sbcd.lPos > 0)
                    // from bottom: easy
                    pData->sbcd.lPos = ulNewYPos;
                else
                {
                    // negative -> from top:
                    RECTL rclClient;
                    WinQueryWindowRect(pData->sbcd.hwndParentAndOwner,
                                       &rclClient);
                    // calc new negative
                    pData->sbcd.lPos = ulNewYPos - rclClient.yTop;
                }
        }
        else
        {
            // vertical split bar:
            ULONG ulNewXPos = track.rclTrack.xLeft
                            - track.rclBoundary.xLeft;
            // add the limit specified by the caller
            ulNewXPos += pData->sbcd.ulLeftOrBottomLimit;

            if (pData->sbcd.ulCreateFlags & SBCF_PERCENTAGE)
            {
                // status bar pos is determined by
                // a percentage:
                // well, we'll need a new percentage then
                RECTL rclClient;
                WinQueryWindowRect(pData->sbcd.hwndParentAndOwner,
                                   &rclClient);
                // XWP V1.0.5 (2006-04-13) [pr]: Fix horrid rounding errors
                pData->sbcd.lPos = (ulNewXPos * 200 + (rclClient.xRight - rclClient.xLeft))
                                   / ((rclClient.xRight - rclClient.xLeft) * 2);
            }
            else
                // absolute split bar positioning:
                if (pData->sbcd.lPos > 0)
                    // from left: easy
                    pData->sbcd.lPos = ulNewXPos;
                else
                {
                    // negative -> from right:
                    RECTL rclClient;
                    WinQueryWindowRect(pData->sbcd.hwndParentAndOwner,
                                       &rclClient);
                    // calc new negative
                    pData->sbcd.lPos = ulNewXPos - rclClient.xRight;
                }
        }

        // update parent (== "split window")
        ctlUpdateSplitWindow(WinQueryWindow(hwndBar, QW_PARENT));
    }
}

/*
 *@@ PaintSplitBar:
 *      implementation for WM_PAINT in ctl_fnwpSplitBar.
 *
 *@@added V0.9.1 (2000-02-05) [umoeller]
 *@@changed V0.9.1 (2000-02-05) [umoeller]: fixed paint errors with sunken 3D style
 */

STATIC VOID PaintSplitBar(HWND hwndBar,
                          PSPLITBARDATA pData)
{
    HPS     hps;
    RECTL   rcl,
            rclBar;
    POINTL  ptl1;
    if (hps = WinBeginPaint(hwndBar, NULLHANDLE, &rcl))
    {
        WinQueryWindowRect(hwndBar, &rclBar);
        // switch to RGB mode
        GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);

        WinFillRect(hps,
                    &rclBar,
                    pData->lcolInactiveBorder);

        if (!(pData->sbcd.ulCreateFlags & (SBCF_3DSUNK | SBCF_3DEXPLORERSTYLE)))
                        // V1.0.0 (2002-08-31) [umoeller]
        {
            GpiSetColor(hps, pData->lcol3DLight);
            // draw left border (bottom to up)
            ptl1.x = 0;
            ptl1.y = 0;
            GpiMove(hps, &ptl1);
            ptl1.y = (rclBar.yTop - rclBar.yBottom) - 1;
            if (pData->sbcd.ulCreateFlags & SBCF_VERTICAL)
                // vertical:
                GpiLine(hps, &ptl1);
            else
                GpiMove(hps, &ptl1);

            // draw top border (to right)
            ptl1.x = (rclBar.xRight - rclBar.xLeft) - 1;
            if (pData->sbcd.ulCreateFlags & SBCF_VERTICAL)
                // vertical:
                GpiMove(hps, &ptl1);
            else
                GpiLine(hps, &ptl1);

            GpiSetColor(hps, pData->lcol3DDark);
            // draw right border (to bottom)
            ptl1.y = 0;
            if (pData->sbcd.ulCreateFlags & SBCF_VERTICAL)
                // vertical:
                GpiLine(hps, &ptl1);
            else
                GpiMove(hps, &ptl1);

            if (!(pData->sbcd.ulCreateFlags & SBCF_VERTICAL))
            {
                // horizontal:
                // draw bottom border
                ptl1.x = 0;
                GpiLine(hps, &ptl1);
            }
        }

        WinEndPaint(hps);
    }
}

/*
 *@@ ctl_fnwpSplitBar:
 *      window procedure for all split bars, horizontal and vertical.
 *      This paints the split bar and handles dragging the split bar
 *      and repositioning the "linked" windows afterwards.
 *
 *      This is not a stand-alone window procedure, but must only
 *      be used with ctlCreateSplitWindow, which creates and subclasses
 *      a static control as necessary.
 *
 *      See ctlCreateSplitWindow for more info and a detailed
 *      window hierarchy.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (99-12-07): fixed memory leak
 *@@changed V0.9.9 (2001-02-01) [lafaix]: added MB2 drag
 *@@changed V0.9.14 (2001-08-21) [umoeller]: SBCF_MOVEABLE was always ignored, fixed
 */

MRESULT EXPENTRY ctl_fnwpSplitBar(HWND hwndBar, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    PSPLITBARDATA pData = (PSPLITBARDATA)WinQueryWindowULong(hwndBar, QWL_USER);

    PFNWP   OldStaticProc = NULL;

    MRESULT mrc = NULL;

    if (pData)
    {
        OldStaticProc = pData->OldStaticProc;

        switch(msg)
        {
            /*
             * WM_MOUSEMOVE:
             *      change mouse pointer when over split bar.
             */

            case WM_MOUSEMOVE:
                if (pData->sbcd.ulCreateFlags & SBCF_MOVEABLE)
                    // V0.9.14 (2001-08-21) [umoeller]
                    // split bar is moveable:
                    pData->hptrOld = WinSetPointer(HWND_DESKTOP, pData->hptrMove);
                else
                    mrc = OldStaticProc(hwndBar, msg, mp1, mp2);
            break;

            /*
             * WM_BUTTON1DOWN:
             *      this will start moving the split bar.
             *      We use WinTrackRect for this.
             */

            case WM_BUTTON1DOWN:
            case WM_BUTTON2DOWN:
                if (pData->sbcd.ulCreateFlags & SBCF_MOVEABLE)
                    // V0.9.14 (2001-08-21) [umoeller]
                    // split bar is moveable:
                    TrackSplitBar(hwndBar, pData);
                else
                    mrc = OldStaticProc(hwndBar, msg, mp1, mp2);
            break;

            /*
             * WM_PAINT:
             *      paint the split bar
             */

            case WM_PAINT:
                PaintSplitBar(hwndBar, pData);
            break;

            case WM_DESTROY:
                free (pData);       // added V0.9.1 (99-12-07)
                mrc = OldStaticProc(hwndBar, msg, mp1, mp2);
            break;

            default:
                mrc = OldStaticProc(hwndBar, msg, mp1, mp2);
        }
    }
    return mrc;
}

/*
 *@@ ctlCreateSplitWindow:
 *      this creates a new split window view according to
 *      the given SPLITBARCDATA structure. A split view
 *      links two existing windows together with a split
 *      bar in between, which can be moved with the mouse.
 *
 *      Split windows are used for example in XWorkplace
 *      for the WPS class list object and in WarpIN for
 *      the database.
 *
 *      After the split view has been created using this
 *      function, all window resizing/positioning is
 *      automatically performed, that is:
 *
 *      a) if the parent window of the split view (e.g.
 *         the frame) is resized, the subwindows are
 *         adjusted;
 *
 *      b) if the split bar is moved, the linked windows
 *         are adjusted also.
 *
 *      <B>Creating a split view</B>
 *
 *      To create a split view, you need to perform the
 *      following steps:
 *
 *      1)  Create two windows that shall be separated by
 *          a split bar. These can be _any_ PM windows:
 *          a simple control window (e.g. a container),
 *          a frame you have created, or even a dialog
 *          which has been loaded using WinLoadDlg.
 *
 *          Note: With frame windows, make sure they have the
 *          FCF_NOBYTEALIGN flag set, or they'll look funny.
 *
 *      2)  Fill a SPLITBARCDATA structure with the necessary
 *          data for the split view (see comctl.h for details).
 *
 *      3)  Call this function with that structure, which creates
 *          and returns the "split window", the invisible parent
 *          of the windows to be split.
 *          See the window hierarchy below.
 *
 *      4)  Send a SPLM_SETLINKS message to the split window
 *          (returned by this function), with the two windows
 *          to be linked in mp1 and mp2. This has been implemented
 *          using a message so that you can re-link windows later
 *          at any time. (This is how the XWorkplace "WPS Class List"
 *          window does it when the settings notebook is changed.)
 *
 *          Sending this message will change parentship (not ownership)
 *          of those two windows to the invisible split window.
 *
 *      5)  Position the split window (which is returned from this
 *          function) within your existing windows using
 *          WinSetWindowPos(..., SWP_SIZE), which will automatically
 *          reposition the two linked subwindows of the split window.
 *          This works because the window procedure of the split
 *          window (ctl_fnwpSplitWindow) handles WM_WINDOWPOSCHANGED to
 *          recalculate the positions of the child windows.
 *
 *      This function returns the HWND of the "split window".
 *      The handle of the split _bar_, if you absolutely need it,
 *      can be found using
 +          WinWindowFromID(hwndWhatThisReturns, ID_SPLITBAR).
 *
 *      After SPLM_SETLINKS, the following window hierarchy
 *      is established (parentship):
 *
 +      SPLITBARCDATA.hwndClient (whatever you have specified;
 +         |                      e.g. FID_CLIENT of your frame window.
 +         |                      You must intercept WM_SIZE and
 +         |                      call WinSetWindowPos on the "split window".)
 +         |
 +         +--  hwndReturn ("split window" created and returned by this
 +                 |       function; uses ctl_fnwpSplitWindow)
 +                 |
 +                 +-- hwndLink1 with SPLM_SETLINKS (parent changed)
 +                 |      |
 +                 |      +-- ... (your child windows remain untouched)
 +                 |
 +                 +-- ID_SPLITBAR (uses ctl_fnwpSplitBar)
 +                 |
 +                 +-- hwndLink2 with SPLM_SETLINKS (parent changed)
 +                        |
 +                        +-- ... (your child windows remain untouched)
 *
 *      Note that only parentship of hwndSplit1 and hwndSplit2
 *      is changed. Ownership remains untouched. So you can specify
 *      any window as the owner of hwndLink1/2 so that messages
 *      can be forwarded properly.
 *
 *      After the above hierarchy has been established, there are
 *      two situations where the "linked" windows will be repositioned
 *      and/or resized:
 *
 *      1)  ctl_fnwpSplitBar will automatically resize and reposition
 *          the left and right "linked" windows if the user moves the
 *          split bar.
 *
 *      2)  If the "split window" itself receives WM_WINDOWPOSCHANGED,
 *          e.g. because WinSetWindowPos has been invoked on the
 *          split window (which you should do when WM_SIZE is received
 *          by your parent window), the "linked" windows will
 *          automatically be repositioned and resized. This is done
 *          in ctl_fnwpSplitWindow.
 *
 *      As a result, you must implement the following in your window
 *      procedures for the windows passed to this function:
 *
 *      1)  The window procedure of SPLITBARCDATA.hwndClient (the
 *          parent of the split window) must intercept WM_SIZE and
 *          do a WinSetWindowPos on the split window. This will
 *          readjust the split bar and SPLITBARCDATA.hwndLink1/2.
 *
 *      2)  The two linked windows should in turn handle WM_SIZE
 *          correctly because whenever the split window is resized,
 *          it invokes a WinSetWindowPos on SPLITBARCDATA.hwndLink1/2
 *          in turn.
 *
 *          If you're using a single standard control as your subwindow
 *          (e.g. a container), this is no problem. However, if you
 *          specify frame windows, you might want to reposition
 *          the controls in those windows in turn.
 *
 *          Note that dialog windows do not receive WM_SIZE;
 *          you'll need to handle WM_WINDOWPOSCHANGED instead.
 *
 *      3)  Your window procedure should forward WM_SYSCOMMAND to
 *          the owner (top) window so that Alt-F4 etc. hotkeys
 *          still work.
 *
 *      If you wish to <B>nest split windows,</B> you can do so by
 *      specifying the "split window" (the HWND which is returned
 *      by this function) as the "hwndLink1/2" to another call
 *      of this function. This way you can create a complex
 *      split window hierarchy (similar to what Netscape does
 *      with the FRAMESET tag). So to create a split view like
 *      this:
 *
 +          +---------+------------+
 +          |         |            |
 +          |         |  2         |
 +          |         |            |
 +          |  1      +------------+
 +          |         |            |
 +          |         |  3         |
 +          |         |            |
 +          +---------+------------+
 *
 *      First invoke ctlCreateSplitWindow to link "2" and "3"
 *      together (creating a horizontal split bar), which returns
 *      a "split window"; then link "1" and that split window
 *      together again.
 *
 *@@added V0.9.0 [umoeller]
 */

HWND ctlCreateSplitWindow(HAB hab,
                          PSPLITBARCDATA psbcd) // in: split window control data
{
    HWND    hwndSplit = NULLHANDLE,
            hwndBar = NULLHANDLE;
    static  s_Registered = FALSE;

    if (psbcd)
    {
        // register "split window" class
        if (!s_Registered)
        {
            WinRegisterClass(hab,
                             WC_SPLITWINDOW,
                             ctl_fnwpSplitWindow,
                             CS_SIZEREDRAW | CS_SYNCPAINT,
                             0);        // additional bytes to reserve
            s_Registered = TRUE;
        }

        if (    (hwndSplit = WinCreateWindow(psbcd->hwndParentAndOwner,  // parent
                                             WC_SPLITWINDOW,
                                             "",
                                             WS_VISIBLE,
                                             0, 0, 10, 10,
                                             psbcd->hwndParentAndOwner,  // owner
                                             HWND_TOP,
                                             psbcd->ulSplitWindowID,
                                             NULL,
                                             NULL))
             && (hwndBar = WinCreateWindow(psbcd->hwndParentAndOwner,  // parent
                                           WC_STATIC,
                                           "",
                                           WS_VISIBLE            // wnd style
                                             | SS_TEXT,
                                           0, 0, 10, 10,
                                           psbcd->hwndParentAndOwner,  // owner
                                           HWND_TOP,
                                           ID_SPLITBAR,          // win ID
                                           NULL,                 // cdata
                                           NULL))                // presparams
           )
        {
            // create SPLITBARDATA to store in split bar's QWL_USER
            PSPLITBARDATA pData;
            if (pData = (PSPLITBARDATA)malloc(sizeof(SPLITBARDATA)))
            {
                // set parent for split bar
                WinSetParent(hwndBar, hwndSplit, FALSE);

                memset(pData, 0, sizeof(SPLITBARDATA));

                // copy control data
                memcpy(&(pData->sbcd), psbcd, sizeof(SPLITBARCDATA));

                // subclass static control to make it a split bar
                pData->OldStaticProc = WinSubclassWindow(hwndBar, ctl_fnwpSplitBar);
                pData->hptrOld = NULLHANDLE;
                pData->hptrMove = WinQuerySysPointer(HWND_DESKTOP,
                            (psbcd->ulCreateFlags & SBCF_HORIZONTAL)
                                ? SPTR_SIZENS
                                : SPTR_SIZEWE,
                            FALSE);     // don't make copy
                pData->fCaptured = FALSE;
                pData->hwndLinked1 =
                pData->hwndLinked2 = NULLHANDLE;

                // caching these colors now V1.0.0 (2002-08-21) [umoeller]
                pData->lcol3DDark = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0);
                pData->lcol3DLight = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);
                pData->lcolInactiveBorder = WinQuerySysColor(HWND_DESKTOP, SYSCLR_INACTIVEBORDER, 0);

                WinSetWindowULong(hwndBar, QWL_USER, (ULONG)pData);
            }
        }
    }

    return hwndSplit;
}

/*
 *@@ ctlUpdateSplitWindow:
 *      this function takes a split window as input and
 *      repositions all its children (the split bars
 *      and the other subwindows which are separated by the
 *      split bars) according to the parent.
 *
 *      This function gets called from ctl_fnwpSplitWindow
 *      when WM_SIZE is received. Normally, you won't have
 *      to call this function independently; you should do
 *      a WinSetWindowPos on the split window instead.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V1.0.0 (2002-08-24) [umoeller]: added support for SBCF_3DEXPLORERSTYLE
 *@@changed XWP V1.0.5 (2006-04-13) [pr]: Fix rounding errors and repaint problems @@fixes 228
 */

BOOL ctlUpdateSplitWindow(HWND hwndSplit)
{
    BOOL    brc = FALSE;
    HWND    hwndSplitBar;
    PSPLITBARDATA psbd;

    if (    (hwndSplitBar = WinWindowFromID(hwndSplit, ID_SPLITBAR))
         && (psbd = (PSPLITBARDATA)WinQueryWindowPtr(hwndSplitBar, QWL_USER))
       )
    {
        PSPLITBARCDATA  psbcd = &(psbd->sbcd);
        RECTL           rclSplit,
                        rclBar;
        LONG            l3DOfsLink1 = 0,
                        l3DOfsLink2 = 0;

        // _Pmpf(("Entering ctlUpdateSplitWindow for hwndSplit 0x%lX", hwndSplit));

        // query the rectangle of the split window's parent;
        // this is either the client or another split window
        WinQueryWindowRect(hwndSplit, &rclSplit);

        /* _Pmpf(("  rect: %d, %d, %d, %d",
                        rclSplit.xLeft,
                        rclSplit.yBottom,
                        rclSplit.xRight - rclSplit.xLeft,
                        rclSplit.yTop - rclSplit.yBottom)); */

        // update split bar
        if (psbcd->ulCreateFlags & SBCF_HORIZONTAL)
        {
            // _Pmpf(("  Calc horizontal"));
            // horizontal split bar:
            if (psbcd->ulCreateFlags & SBCF_PERCENTAGE)
                // take height of client and apply percentage
                // XWP V1.0.5 (2006-04-13) [pr]: Fix horrid rounding errors
                rclBar.yBottom = ((rclSplit.yTop - rclSplit.yBottom)
                                * psbcd->lPos + 50)
                                / 100;
            else
                if (psbcd->lPos > 0)
                    // offset from bottom:
                    rclBar.yBottom = psbcd->lPos;
                else
                    // offset from right:
                    rclBar.yBottom = (rclSplit.yTop - rclSplit.yBottom)
                                     + psbcd->lPos;  // which is negative

            rclBar.yTop = rclBar.yBottom + WinQuerySysValue(HWND_DESKTOP,
                                                            SV_CXSIZEBORDER);
            rclBar.xLeft = 0;
            // take width of client
            rclBar.xRight = (rclSplit.xRight - rclSplit.xLeft);
        }
        else
        {
            // _Pmpf(("  Calc vertical"));
            // vertical split bar:
            if (psbcd->ulCreateFlags & SBCF_PERCENTAGE)
                // take width of client and apply percentage
                // XWP V1.0.5 (2006-04-13) [pr]: Fix horrid rounding errors
                rclBar.xLeft = ((rclSplit.xRight - rclSplit.xLeft)
                                * psbcd->lPos + 50)
                                / 100;
            else
                if (psbcd->lPos > 0)
                    // offset from left:
                    rclBar.xLeft = psbcd->lPos;
                else
                    // offset from right:
                    rclBar.xLeft = (rclSplit.xRight - rclSplit.xLeft)
                                   + psbcd->lPos;  // which is negative

            rclBar.xRight = rclBar.xLeft + WinQuerySysValue(HWND_DESKTOP,
                                                            SV_CXSIZEBORDER);
            rclBar.yBottom = 0;
            // take height of client
            rclBar.yTop = (rclSplit.yTop - rclSplit.yBottom);
        }

        // reposition split bar
        brc = WinSetWindowPos(hwndSplitBar,
                              HWND_TOP,
                              rclBar.xLeft,
                              rclBar.yBottom,
                              rclBar.xRight - rclBar.xLeft,
                              rclBar.yTop - rclBar.yBottom,
                              SWP_MOVE | SWP_SIZE);

        /* _Pmpf(("  Set splitbar hwnd %lX to %d, %d, %d, %d; rc: %d",
                        hwndSplitBar,
                        rclBar.xLeft,
                        rclBar.yBottom,
                        rclBar.xRight - rclBar.xLeft,
                        rclBar.yTop - rclBar.yBottom,
                        brc)); */

        // reposition left/bottom window of split bar
        if (psbcd->ulCreateFlags & SBCF_3DEXPLORERSTYLE)
        {
            l3DOfsLink2 = 2;
        }
        else if (psbcd->ulCreateFlags & SBCF_3DSUNK)
        {
            l3DOfsLink1 = 1;
            l3DOfsLink2 = 1;
        }
        // else 0

        // now reposition the linked windows
        if (psbcd->ulCreateFlags & SBCF_HORIZONTAL)
        {
            // horizontal:
            // reposition bottom window of split bar
            WinSetWindowPos(psbd->hwndLinked1,
                            HWND_TOP,
                            l3DOfsLink1,
                            l3DOfsLink1,
                            rclBar.xRight - rclBar.xLeft - 2 * l3DOfsLink1,
                            rclBar.yBottom - 2 * l3DOfsLink1,
                                    // the window rect is non-inclusive
                            SWP_MOVE | SWP_SIZE);

            // reposition top window of split bar
            WinSetWindowPos(psbd->hwndLinked2,
                            HWND_TOP,
                            l3DOfsLink2,
                            rclBar.yTop + l3DOfsLink2,
                                    // the window rect is non-inclusive
                            rclBar.xRight - rclBar.xLeft - 2 * l3DOfsLink2,
                            rclSplit.yTop - rclBar.yTop - 2 * l3DOfsLink2,
                            SWP_MOVE | SWP_SIZE);
        }
        else
        {
            // vertical:
            // reposition left window of split bar
            WinSetWindowPos(psbd->hwndLinked1,
                            HWND_TOP,
                            l3DOfsLink1,
                            l3DOfsLink1,
                            rclBar.xLeft - 2 * l3DOfsLink1,
                                    // the window rect is non-inclusive
                            rclBar.yTop - rclBar.yBottom - 2 * l3DOfsLink1,
                            SWP_MOVE | SWP_SIZE);

            // reposition right window of split bar
            WinSetWindowPos(psbd->hwndLinked2,
                            HWND_TOP,
                            rclBar.xRight + l3DOfsLink2,
                                    // the window rect is non-inclusive
                            l3DOfsLink2,
                            rclSplit.xRight - rclBar.xRight - 2 * l3DOfsLink2,
                            rclBar.yTop - rclBar.yBottom - 2 * l3DOfsLink2,
                            SWP_MOVE | SWP_SIZE);
        }

        // repaint split window (3D frame)
        WinInvalidateRect(hwndSplit,
                          NULL,         // all
                          FALSE);       // don't repaint children
        // XWP V1.0.5 (2006-04-13) [pr]: repaint right/bottom frame @@fixes 228
        WinInvalidateRect(psbd->hwndLinked2,
                          NULL,
                          TRUE);
    }

    return brc;
}

/*
 *@@ ctlQuerySplitPos:
 *      this returns the position of the split bar
 *      (the child control of hwndSplit; see ctlCreateSplitWindow
 *      for the window hierarchy).
 *
 *      The meaning of the return value depends on what you
 *      specified with ulCreateFlags in the SPLITBARCDATA
 *      structure passed to ctlCreateSplitWindow, that is,
 *      it can be a percentage or an offset from the left
 *      or from the right of the split window.
 *
 *      This returns NULL upon errors.
 *
 *@@added V0.9.0 [umoeller]
 */

LONG ctlQuerySplitPos(HWND hwndSplit)
{
    // the split bar data is stored in QWL_USER of the
    // split bar (not the split window)
    HWND    hwndSplitBar;
    PSPLITBARDATA psbd;
    if (    (hwndSplitBar = WinWindowFromID(hwndSplit, ID_SPLITBAR))
         && (psbd = (PSPLITBARDATA)WinQueryWindowULong(hwndSplitBar, QWL_USER))
       )
        return psbd->sbcd.lPos;

    return 0;
}


