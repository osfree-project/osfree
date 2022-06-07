
/*
 *@@sourcefile cctl_progbar.c:
 *      implementation for the progress bar common control.
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
 *      Copyright (C) 1997-2000 Ulrich M”ller.
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
 *@@category: Helpers\PM helpers\Window classes\Progress bars
 *      See cctl_progbar.c.
 */

/* ******************************************************************
 *
 *   Progress bars
 *
 ********************************************************************/

/*
 *@@ PaintProgress:
 *      this does the actual painting of the progress bar, called
 *      from ctl_fnwpProgressBar.
 *      It is called both upon WM_PAINT and WM_UPDATEPROGRESSBAR
 *      with different HPS's then.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.5 (2000-09-22) [umoeller]: fixed ypos of text
 */

STATIC VOID PaintProgress(PPROGRESSBARDATA pData,
                          HWND hwndBar,
                          PRECTL prclWin,      // in: window rectangle (WinQueryWindowRect)
                          HPS hps)
{
    POINTL  ptlText; // , aptlText[TXTBOX_COUNT];
    BOOL    fBackgroundPainted = FALSE;
    CHAR    szPercent[10] = "";
    RECTL   rclInnerButton;
    LONG    lcolScrollbar = WinQuerySysColor(HWND_DESKTOP,
                                             SYSCLR_SCROLLBAR,
                                             0);

    // switch to RGB mode
    gpihSwitchToRGB(hps);

    if (pData->ulPaintX <= pData->ulOldPaintX)
    {
        RECTL rclOuterFrame;        // inclusive
        rclOuterFrame.xLeft = 0;
        rclOuterFrame.yBottom = 0;
        rclOuterFrame.xRight = prclWin->xRight - 1;
        rclOuterFrame.yTop = prclWin->yTop - 1;

        gpihDraw3DFrame(hps,
                        &rclOuterFrame,     // inclusive
                        1,
                        WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0),
                        WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0));

        rclOuterFrame.xLeft++;
        rclOuterFrame.yBottom++;
        WinFillRect(hps,
                    &rclOuterFrame,     // exclusive, top right not drawn
                    lcolScrollbar);
        fBackgroundPainted = TRUE;
    }

    // now draw the actual progress;
    // rclInnerButton receives an _inclusive_ rectangle
    rclInnerButton.xLeft = 1;
    rclInnerButton.xRight = (pData->ulPaintX > (rclInnerButton.xLeft + 3))
                                 ? pData->ulPaintX
                                 : rclInnerButton.xLeft +
                                       ((pData->ulAttr & PBA_BUTTONSTYLE)
                                           ? 3 : 1);
    rclInnerButton.yBottom = 1;
    rclInnerButton.yTop = prclWin->yTop     // exclusive
                            - 2;            // 1 to make inclusive, 1 for outer frame

    if (pData->ulAttr & PBA_PERCENTFLAGS)
    {
        // percentage desired:

        POINTL  aptlText[TXTBOX_COUNT];
        LONG    lLineSpacing = 1;
        FONTMETRICS fm;
        if (GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm))
            lLineSpacing = fm.lEmHeight;

        sprintf(szPercent, "%lu %%", ((100 * pData->ulNow) / pData->ulMax) );
        // calculate string space
        GpiQueryTextBox(hps,
                        strlen(szPercent),
                        szPercent,
                        TXTBOX_COUNT,
                        (PPOINTL)&aptlText);

        ptlText.x =
                        (   (   (prclWin->xRight)     // cx
                              - (aptlText[TXTBOX_BOTTOMRIGHT].x - aptlText[TXTBOX_BOTTOMLEFT].x)
                            )
                        / 2);
        ptlText.y =
                        (   (   (prclWin->yTop)       // cy
                              - (lLineSpacing)
                            )
                        / 2) + 2;

        if (!fBackgroundPainted)
        {
            // if we haven't drawn the background already,
            // we'll need to do it now for the percentage area
            RECTL rcl2;
            rcl2.xLeft      = ptlText.x;
            rcl2.xRight     = ptlText.x + (aptlText[TXTBOX_BOTTOMRIGHT].x-aptlText[TXTBOX_BOTTOMLEFT].x);
            rcl2.yBottom    = ptlText.y;
            rcl2.yTop       = ptlText.y + lLineSpacing;
            WinFillRect(hps,
                        &rcl2,
                        lcolScrollbar);
        }
    }

    if (pData->ulAttr & PBA_BUTTONSTYLE)
    {
        // draw "raised" inner rect
        gpihDraw3DFrame(hps,
                        &rclInnerButton,
                        2,
                        WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0),
                        WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0));
    }

    rclInnerButton.xLeft += 2;
    rclInnerButton.yBottom += 2;
    rclInnerButton.yTop -= 2;
    rclInnerButton.xRight -= 2;

    if (rclInnerButton.xRight > rclInnerButton.xLeft)
    {
        POINTL ptl1;
        // draw interior of inner rect
        GpiSetColor(hps, WinQuerySysColor(HWND_DESKTOP,
                                          SYSCLR_BUTTONMIDDLE,
                                          0));
        ptl1.x = rclInnerButton.xLeft;
        ptl1.y = rclInnerButton.yBottom;
        GpiMove(hps, &ptl1);
        ptl1.x = rclInnerButton.xRight;
        ptl1.y = rclInnerButton.yTop;
        GpiBox(hps,
               DRO_FILL | DRO_OUTLINE,
               &ptl1,       // inclusive!
               0,
               0);
    }

    // now print the percentage
    if (pData->ulAttr & PBA_PERCENTFLAGS)
    {
        GpiMove(hps, &ptlText);
        GpiSetColor(hps, WinQuerySysColor(HWND_DESKTOP,
                                          SYSCLR_BUTTONDEFAULT,
                                          0));
        GpiCharString(hps, strlen(szPercent), szPercent);
    }

    // store current X position for next time
    pData->ulOldPaintX = pData->ulPaintX;
}

/*
 *@@ ctl_fnwpProgressBar:
 *      this is the window procedure for the progress bar control.
 *
 *      This is not a stand-alone window procedure, but must only
 *      be used with static rectangle controls subclassed by
 *      ctlProgressBarFromStatic.
 *
 *      We need to capture WM_PAINT to draw the progress bar according
 *      to the current progress, and we also update the static text field
 *      (percentage) next to it.
 *
 *      This also evaluates the WM_UPDATEPROGRESSBAR message.
 *
 *@@changed V0.9.0 [umoeller]: moved this code here
 *@@changed V0.9.1 (99-12-06): fixed memory leak
 */

MRESULT EXPENTRY ctl_fnwpProgressBar(HWND hwndBar, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    HPS                 hps;
    PPROGRESSBARDATA    ppd = (PPROGRESSBARDATA)WinQueryWindowULong(hwndBar, QWL_USER);

    PFNWP   OldStaticProc = NULL;

    MRESULT mrc = NULL;

    if (ppd)
    {
        OldStaticProc = ppd->OldStaticProc;

        switch(msg)
        {

            /*
             *@@ WM_UPDATEPROGRESSBAR:
             *      post or send this message to a progress bar
             *      to have a new progress displayed.
             *      Parameters:
             *      --  ULONG mp1   current value
             *      --  ULONG mp2   max value
             *      Example: mp1 = 100, mp2 = 300 will result
             *      in a progress of 33%.
             */

            case WM_UPDATEPROGRESSBAR:
            {
                RECTL rclWin;
                WinQueryWindowRect(hwndBar, &rclWin);
                if (    (ppd->ulNow != (ULONG)mp1)
                     || (ppd->ulMax != (ULONG)mp2)
                   )
                {
                    ppd->ulNow = (ULONG)mp1;
                    ppd->ulMax = (ULONG)mp2;
                }
                else
                    // value not changed: do nothing
                    break;

                // check validity
                if (ppd->ulNow > ppd->ulMax)
                    ppd->ulNow = ppd->ulMax;
                // avoid division by zero
                if (ppd->ulMax == 0)
                {
                    ppd->ulMax = 1;
                    ppd->ulNow = 0;
                    // paint 0% then
                }

                // calculate new X position of the progress
                ppd->ulPaintX =
                    (ULONG)(
                              (    (ULONG)(rclWin.xRight - rclWin.xLeft - 2)
                                 * (ULONG)(ppd->ulNow)
                              )
                              /  (ULONG)ppd->ulMax
                           );
                if (ppd->ulPaintX != ppd->ulOldPaintX)
                {
                    // X position changed: redraw
                    // WinInvalidateRect(hwndBar, NULL, FALSE);
                    hps = WinGetPS(hwndBar);
                    PaintProgress(ppd, hwndBar, &rclWin, hps);
                    WinReleasePS(hps);
                }
            break; }

            case WM_PAINT:
            {
                RECTL rclWin;
                WinQueryWindowRect(hwndBar, &rclWin);
                hps = WinBeginPaint(hwndBar, NULLHANDLE, NULL);
                PaintProgress(ppd, hwndBar, &rclWin, hps);
                WinEndPaint(hps);
            break; }

            /*
             * WM_DESTROY:
             *      free PROGRESSBARDATA
             *      (added V0.9.1 (99-12-06))
             */

            case WM_DESTROY:
            {
                free(ppd);
                mrc = OldStaticProc(hwndBar, msg, mp1, mp2);
            break; }

            default:
                mrc = OldStaticProc(hwndBar, msg, mp1, mp2);
       }
    }
    return mrc;
}

/*
 *@@ ctlProgressBarFromStatic:
 *      this function turns an existing static rectangle control
 *      into a progress bar by subclassing its window procedure
 *      with ctl_fnwpProgressBar.
 *
 *      This way you can easily create a progress bar as a static
 *      control in any Dialog Editor; after loading the dlg template,
 *      simply call this function with the hwnd of the static control
 *      to make it a status bar.
 *
 *      This is used for all the progress bars in XWorkplace and
 *      WarpIN.
 *
 *      In order to _update_ the progress bar, simply post or send
 *      WM_UPDATEPROGRESSBAR to the static (= progress bar) window;
 *      this message is equal to WM_USER and needs the following
 *      parameters:
 *      --  mp1     ULONG ulNow: the current progress
 *      --  mp2     ULONG ulMax: the maximally possible progress
 *                               (= 100%)
 *
 *      The progress bar automatically calculates the current progress
 *      display. For example, if ulNow = 4096 and ulMax = 8192,
 *      a progress of 50% will be shown. It is possible to change
 *      ulMax after the progress bar has started display. If ulMax
 *      is 0, a progress of 0% will be shown (to avoid division
 *      by zero traps).
 *
 *      ulAttr accepts of the following:
 *      --  PBA_NOPERCENTAGE:    do not display percentage
 *      --  PBA_ALIGNLEFT:       left-align percentage
 *      --  PBA_ALIGNRIGHT:      right-align percentage
 *      --  PBA_ALIGNCENTER:     center percentage
 *      --  PBA_BUTTONSTYLE:     no "flat", but button-like look
 *
 *@@changed V0.9.0 [umoeller]: moved this code here
 */

BOOL ctlProgressBarFromStatic(HWND hwndChart, ULONG ulAttr)
{
    PFNWP OldStaticProc;
    if (OldStaticProc = WinSubclassWindow(hwndChart, ctl_fnwpProgressBar))
    {
        PPROGRESSBARDATA pData = (PPROGRESSBARDATA)malloc(sizeof(PROGRESSBARDATA));
        pData->ulMax = 1;
        pData->ulNow = 0;
        pData->ulPaintX = 0;
        pData->ulAttr = ulAttr;
        pData->OldStaticProc = OldStaticProc;

        WinSetWindowULong(hwndChart, QWL_USER, (ULONG)pData);
        return TRUE;
    }

    return FALSE;
}


