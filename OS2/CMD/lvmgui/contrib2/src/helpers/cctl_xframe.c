
/*
 *@@sourcefile cctl_xframe.c:
 *      implementation for the "extended frame" common control.
 *      This is a drop-in replacement for WinCreateStdWindow;
 *      see ctlCreateStdWindow for an introduction.
 *
 *      See comctl.c for an overview of the common controls.
 *
 *      This is new with V1.0.1 (2002-11-30) [umoeller].
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\comctl.h"
 *@@added V1.0.1 (2002-11-30) [umoeller]
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

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSYS
#define INCL_WINSTATICS
#define INCL_WINSHELLDATA
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\standards.h"
#include "helpers\winh.h"

#include "helpers\comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Extended frame windows
 *      See comctl.c and ctlMakeMenuButton.
 */

/* ******************************************************************
 *
 *   Extended frame
 *
 ********************************************************************/

/*
 *@@ ctlFormatFrame:
 *      implementation for WM_FORMATFRAME in fnwpSubclExtFrame.
 *
 *      This can also be called externally if you have a frame
 *      window proc that implements tool or status bars.
 *      (XWorkplace status bars).
 *
 *@@changed V1.0.1 (2002-12-08) [umoeller]: didn't work if frame had no menu, fixed
 */

MRESULT ctlFormatExtFrame(HWND hwndFrame,
                          const XFRAMECONTROLS *pxfc,
                          MPARAM mp1,
                          MPARAM mp2)
{
    // query the number of standard frame controls
    ULONG   c = (ULONG)pxfc->pfnwpOrig(hwndFrame, WM_FORMATFRAME, mp1, mp2),
            cOrig = c;

    // increment the number of frame controls
    // to include our tool and status bar
    if (pxfc->hwndToolBar)
        ++c;
    if (pxfc->hwndStatusBar)
        ++c;

    if (c != cOrig)
    {
        // we have something:
        // format client, status, and tool bar
        ULONG       ul;
        PSWP        paswp = (PSWP)mp1,
                    pswpClient = NULL,
                    pswpMenu = NULL,
                    pswpTitleBar = NULL,
                    pswpToolBar = NULL,
                    pswpStatusBar = NULL,
                    // first of the new SWP entries:
                    pswpLast = paswp + cOrig;

        // PMREF says mp2 has a PRECTL to the client rectangle,
        // but we cannot rely on that since from my testing
        // that pointer is NULL if the frame has no client...
        // so calculate this manually
        POINTL      ptlBorderSizes;
        RECTL       rclFrame;
        WinSendMsg(hwndFrame,
                   WM_QUERYBORDERSIZE,
                   (MPARAM)&ptlBorderSizes,
                   0);
        WinQueryWindowRect(hwndFrame, &rclFrame);

        // find the client in the SWP array
        for (ul = 0; ul < cOrig; ul++)
        {
            switch (WinQueryWindowUShort(paswp[ul].hwnd, QWS_ID))
            {
                case FID_CLIENT:
                    pswpClient = &paswp[ul];
                break;

                case FID_MENU:
                    pswpMenu = &paswp[ul];
                break;

                case FID_TITLEBAR:
                    pswpTitleBar = &paswp[ul];
                break;
            }
        }

        // position tool bar, if present
        if (pxfc->hwndToolBar)
        {
            pswpToolBar = pswpLast++;

            pswpToolBar->fl = SWP_MOVE | SWP_SIZE; //  | SWP_NOADJUST;
            pswpToolBar->x  = ptlBorderSizes.x;

            if (pswpMenu)
                pswpToolBar->y = pswpMenu->y - pxfc->lToolBarHeight;
            // if we don't have a menu, we need to use the title bar instead
            // or the toolbar will overlap it V1.0.1 (2002-12-08) [umoeller]
            else if (pswpTitleBar)
                pswpToolBar->y = pswpTitleBar->y - pxfc->lToolBarHeight;
            else
                pswpToolBar->y  = rclFrame.yTop - ptlBorderSizes.y - pxfc->lToolBarHeight;

            pswpToolBar->cx = rclFrame.xRight - 2 * ptlBorderSizes.x;
            pswpToolBar->cy = pxfc->lToolBarHeight;
            pswpToolBar->hwndInsertBehind = HWND_TOP;
            pswpToolBar->hwnd = pxfc->hwndToolBar;
        }

        // position status bar, if present
        if (pxfc->hwndStatusBar)
        {
            pswpStatusBar = pswpLast++;

            pswpStatusBar->fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST;
            pswpStatusBar->x  = ptlBorderSizes.x;
            pswpStatusBar->y  = ptlBorderSizes.y;
            pswpStatusBar->cx = rclFrame.xRight - 2 * ptlBorderSizes.x;
            pswpStatusBar->cy = pxfc->lStatusBarHeight;
            pswpStatusBar->hwndInsertBehind = HWND_TOP;
            pswpStatusBar->hwnd = pxfc->hwndStatusBar;
        }

        // finally, reduce client
        if (pswpClient)
        {
            if (pswpStatusBar)
            {
                pswpClient->y  += pswpStatusBar->cy;
                pswpClient->cy -= pswpStatusBar->cy;
            }
            if (pswpToolBar)
            {
                pswpClient->cy -= pswpToolBar->cy;
            }
        }
    }

    return (MRESULT)c;
}

/*
 *@@ ctlCalcExtFrameRect:
 *      implementation for WM_CALCFRAMERECT in fnwpSubclExtFrame.
 *
 *      This can also be called externally if you have a frame
 *      window proc that implements tool or status bars
 *      (XWorkplace status bars).
 *
 *@@added V1.0.0 (2002-08-28) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: moved here from winh.c, renamed
 */

MRESULT ctlCalcExtFrameRect(HWND hwndFrame,
                            const XFRAMECONTROLS *pxfc,
                            MPARAM mp1,
                            MPARAM mp2)
{
    PRECTL prclPassed = (PRECTL)mp1;

    MRESULT mrc = pxfc->pfnwpOrig(hwndFrame, WM_CALCFRAMERECT, mp1, mp2);

    if (mp2)
    {
        // mp2 == TRUE:  frame rectangle provided, calculate client
        //  call default window procedure to subtract child frame
        //  controls from the rectangle's height

        if (pxfc->hwndToolBar)
            prclPassed->yTop -= pxfc->lToolBarHeight;

        if (pxfc->hwndStatusBar)
            prclPassed->yBottom += pxfc->lStatusBarHeight;
    }
    else
    {
        // mp2 == FALSE: client area rectangle provided, calculate frame
        //  call default window procedure to subtract child frame
        //  controls from the rectangle's height;
        //  set the origin of the frame and increase it based upon the
        //  static text control's height

        if (pxfc->hwndToolBar)
            prclPassed->yTop += pxfc->lToolBarHeight;

        if (pxfc->hwndStatusBar)
            prclPassed->yBottom -= pxfc->lStatusBarHeight;
    }

    return mrc;
}

#define STATUS_BAR_HEIGHT       20

/*
 *@@ fnwpSubclExtFrame:
 *      subclassed frame window proc for the extended frame
 *      window (see ctlCreateStdWindow).
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: moved here from winh.c
 */

STATIC MRESULT EXPENTRY fnwpSubclExtFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT         mrc = 0;

    PEXTFRAMEDATA   pData = (PEXTFRAMEDATA)WinQueryWindowPtr(hwndFrame, QWL_USER);
    ULONG           c;

    switch (msg)
    {
        case WM_QUERYFRAMECTLCOUNT:
            // query the standard frame controls count
            c = (ULONG)pData->xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);

            // increment the number of frame controls
            // to include our tool and status bar
            if (pData->xfc.hwndToolBar)
                ++c;
            if (pData->xfc.hwndStatusBar)
                ++c;

            mrc = (MPARAM)c;
        break;

        case WM_FORMATFRAME:
            mrc = ctlFormatExtFrame(hwndFrame,
                                    &pData->xfc,
                                    mp1,
                                    mp2);
        break;

        case WM_CALCFRAMERECT:
            // we have a status bar: calculate its rectangle
            mrc = ctlCalcExtFrameRect(hwndFrame,
                                      &pData->xfc,
                                      mp1,
                                      mp2);
        break;

        /*
        case WM_WINDOWPOSCHANGED:
            WinSendMsg(hwndFrame, WM_UPDATEFRAME, 0, 0);
        break; */

        case WM_SYSCOMMAND:
            if (    (SHORT1FROMMP(mp1) == SC_CLOSE)
                 && (pData->CData.hiniSaveWinPos)
               )
            {
                SWP swp;
                WinQueryWindowPos(hwndFrame, &swp);
                PrfWriteProfileData(pData->CData.hiniSaveWinPos,
                                    (PSZ)pData->CData.pcszIniApp,
                                    (PSZ)pData->CData.pcszIniKey,
                                    &swp,
                                    sizeof(swp));
            }

            mrc = pData->xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
        break;

        case WM_DESTROY:
            WinSetWindowPtr(hwndFrame, QWL_USER, NULL);
            WinSubclassWindow(hwndFrame, pData->xfc.pfnwpOrig);
            free(pData);

            mrc = pData->xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
        break;

        default:
            mrc = pData->xfc.pfnwpOrig(hwndFrame, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ ctlCreateStatusBar:
 *      creates a status bar for a frame window.
 *
 *      Normally there's no need to call this manually,
 *      this gets called by ctlCreateStdWindow
 *      automatically.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: moved here from winh.c, renamed
 */

HWND ctlCreateStatusBar(HWND hwndFrame,
                        HWND hwndOwner,
                        const char *pcszText,      // in: initial status bar text
                        const char *pcszFont,      // in: font to use for status bar
                        LONG lColor)               // in: foreground color for status bar
{
    // create status bar
    HWND        hwndReturn = NULLHANDLE;

    PPRESPARAMS ppp = winhCreateDefaultPresparams();

    hwndReturn = WinCreateWindow(hwndFrame,
                                 WC_STATIC,
                                 (PSZ)pcszText,
                                 SS_TEXT | DT_VCENTER | WS_VISIBLE,
                                 0, 0, 0, 0,
                                 hwndOwner,
                                 HWND_TOP,
                                 FID_STATUSBAR,
                                 NULL,
                                 ppp);
    free(ppp);

    return hwndReturn;
}

/*
 *@@ ctlCreateStdWindow:
 *      creates an extended frame window. Use this
 *      instead of WinCreateStdWindow if you want a
 *      quick way to add an enhanced frame window with
 *      status or tool bars.
 *
 *      pData must point to an EXTFRAMECDATA structure
 *      which contains a copy of the parameters to be
 *      passed to winhCreateStdWindow. In addition,
 *      this contains the flExtFlags field, which allows
 *      you to automatically create a status and tool bar
 *      for the frame.
 *
 *      Note that we subclass the frame here and require
 *      QWL_USER for that. The frame's QWL_USER points
 *      to an EXTFRAMEDATA structure whose pUser parameter
 *      you may use for additional data, if you want to
 *      do further subclassing.
 *
 *      For tool bars to work, you must call ctlRegisterToolbar
 *      beforehand.
 *
 *      This has the following extra features:
 *
 *      --  To have a status bar created automatically,
 *          pass in XFCF_STATUSBAR with EXTFRAMECDATA.flExtFrame.
 *          The status bar is a frame control and can be
 *          queried with WinWindowFromID(hwndFrame, FID_STATUSBAR).
 *          It is a subclassed static and thus supports plain
 *          WinSetWindowText and the like.
 *
 *      --  To have a tool bar created automatically,
 *          pass in XFCF_TOOLBAR with EXTFRAMECDATA.flExtFrame
 *          and an array of TOOLBARCONTROL structures for the
 *          list of controls to be created in the tool bar.
 *          See cctl_toolbar.c for details. The tool bar can be
 *          queried with WinWindowFromID(hwndFrame, FID_TOOLBAR).
 *
 *      --  You can specify pswpFrame for the initial window
 *          position. If it is NULL, the extended frame will
 *          resize itself to fill most of the screen with about
 *          20 pixels border around it.
 *
 *      --  If you specify hiniSaveWinPos, pcszIniApp, and
 *          pcszIniKey with EXTFRAMECDATA, the frame supports
 *          saving its window position automatically. Only if
 *          no window position is found in that key, pswpFrame
 *          is respected.
 *
 *      --  The frame will only be visible and activated when this
 *          function returns if you specify WS_VISIBLE with flStyleFrame.
 *          The SWP_SHOW, SWP_HIDE, and SWP_ACTIVATE flags are
 *          always filtered out from the given pswpFrame. If
 *          WS_VISIBLE is set, the frame will be activated also.
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 *@@changed V1.0.1 (2002-11-30) [umoeller]: moved here from winh.c, renamed
 *@@changed V1.0.1 (2002-11-30) [umoeller]: added support for saving window pos automatically
 */

HWND ctlCreateStdWindow(PEXTFRAMECDATA pData,        // in: extended frame data
                        PHWND phwndClient)          // out: created client wnd
{
    HWND hwndFrame;

    SWP     swpFrame;
    ULONG   cb = sizeof(swpFrame);

    if (    (!pData->hiniSaveWinPos)
         || (!PrfQueryProfileData(pData->hiniSaveWinPos,
                                  (PSZ)pData->pcszIniApp,
                                  (PSZ)pData->pcszIniKey,
                                  &swpFrame,
                                  &cb))
         || (cb != sizeof(swpFrame))
       )
    {
        // window pos not loaded:
        // try to use the specified one
        if (pData->pswpFrame)
            memcpy(&swpFrame, pData->pswpFrame, sizeof(SWP));
        else
        {
            swpFrame.hwndInsertBehind = HWND_TOP;
            swpFrame.x = 20;
            swpFrame.y = 20;
            swpFrame.cx = G_cxScreen - 2 * 20;
            swpFrame.cy = G_cyScreen - 2 * 20;
            swpFrame.fl = SWP_MOVE | SWP_SIZE;
        }
    }

    swpFrame.fl &= ~ SWP_SHOW | SWP_HIDE | SWP_ACTIVATE;

    if (hwndFrame = winhCreateStdWindow(HWND_DESKTOP,
                                        &swpFrame,
                                        pData->flFrame | WS_CLIPCHILDREN,
                                        pData->flStyleFrame & ~ WS_VISIBLE,
                                            // do not show before we have formatted
                                            // the frame correctly
                                        pData->pcszFrameTitle,
                                        pData->ulResourcesID,
                                        pData->pcszClassClient,
                                        pData->flStyleClient,
                                        pData->ulID,
                                        pData->pClientCtlData,
                                        phwndClient))
    {
        PEXTFRAMEDATA pFrameData;

        // subclass frame for supporting status bar and msgs
        if (pFrameData = NEW(EXTFRAMEDATA))
        {
            ZERO(pFrameData),
            memcpy(&pFrameData->CData, pData, sizeof(pFrameData->CData));

            if (pFrameData->xfc.pfnwpOrig = WinSubclassWindow(hwndFrame,
                                                              fnwpSubclExtFrame))
            {
                WinSetWindowPtr(hwndFrame, QWL_USER, pFrameData);

                if (pData->flExtFrame & XFCF_TOOLBAR)
                {
                    HWND hwndControlsOwner;

                    // determine owner for the tool bar controls:
                    // client, if present and XFCF_FORCETBOWNER is
                    // not set
                    if (    (!(hwndControlsOwner = *phwndClient))
                         || (pData->flExtFrame & XFCF_FORCETBOWNER)
                       )
                        hwndControlsOwner = hwndFrame;

                    // create tool bar as frame control
                    if (pFrameData->xfc.hwndToolBar = ctlCreateToolBar(hwndFrame,
                                                                       hwndFrame,
                                                                       WS_VISIBLE
                                                                            | TBS_TOOLTIPS
                                                                            | TBS_AUTORESIZE,
                                                                       // owner for controls:
                                                                       // client, if it exists
                                                                       hwndControlsOwner,
                                                                       pData->cTBControls,
                                                                       pData->paTBControls))
                    {
                        WinQueryWindowPos(pFrameData->xfc.hwndToolBar,
                                          &swpFrame);
                        pFrameData->xfc.lToolBarHeight = swpFrame.cy;
                    }
                }

                if (pData->flExtFrame & XFCF_STATUSBAR)
                {
                    // create status bar as frame control
                    pFrameData->xfc.hwndStatusBar = ctlCreateStatusBar(hwndFrame,
                                                                       hwndFrame,
                                                                       "",
                                                                       "9.WarpSans",
                                                                       CLR_BLACK);

                    pFrameData->xfc.lStatusBarHeight = STATUS_BAR_HEIGHT;
                }

                WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPNULL, MPNULL);

                if (pData->flStyleFrame & WS_VISIBLE)
                    WinSetWindowPos(hwndFrame,
                                    HWND_TOP,
                                    0,
                                    0,
                                    0,
                                    0,
                                    SWP_SHOW | SWP_ZORDER | SWP_ACTIVATE);
            }
            else
            {
                free(pFrameData);
                WinDestroyWindow(hwndFrame);
                hwndFrame = NULLHANDLE;
            }
        }
    }

    return hwndFrame;
}

