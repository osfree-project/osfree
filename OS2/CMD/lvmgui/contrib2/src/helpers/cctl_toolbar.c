
/*
 *@@sourcefile cctl_toolbar.c:
 *      implementation for the "tool bar" common control.
 *      See ctlCreateToolBar and ctl_fnwpToolbar.
 *
 *      See comctl.c for an overview of the common controls.
 *
 *      This file implements two window classes: a tool bar
 *      control (which is most conveniently used with
 *      ctlCreateStdWindow) and a tool button control as
 *      a replacement for the dull OS/2 pushbutton controls.
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
#define INCL_WINPOINTERS
#define INCL_WININPUT
#define INCL_WINBUTTONS

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#define INCL_GPIPATHS
#define INCL_GPIREGIONS
#define INCL_GPIBITMAPS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

#include "setup.h"                      // code generation and debugging options

#include "helpers/gpih.h"
#include "helpers/linklist.h"
#include "helpers/standards.h"
#include "helpers/stringh.h"
#include "helpers/winh.h"
#include "helpers/xstring.h"

#include "helpers/comctl.h"

#pragma hdrstop

/*
 *@@category: Helpers\PM helpers\Window classes\Toolbars
 *      See cctl_toolbar.c.
 */

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

/*
 *@@ TBBUTTONDATA:
 *
 */

typedef struct _TBBUTTONDATA
{
    XBUTTONDATA     bd;
    XBUTTONSTATE    bs;
} TBBUTTONDATA, *PTBBUTTONDATA;

/*
 *@@ TOOLBARDATA:
 *
 */

typedef struct _TOOLBARDATA
{
    DEFWINDATA  dwd;

    HWND        hwndControlsOwner,
                hwndToolTip;        // != NULLHANDLE if TBS_TOOLTIPS is set

    LONG        lSpacing,
                lMaxControlCY;

    LINKLIST    llControls;         // linked list of HWNDs for controls in toolbar

    ULONG       flReformat;
        #define RFFL_HEIGHT         0x0001      // height changed, needs complete resize

    XSTRING     strToolTipBuf;

} TOOLBARDATA, *PTOOLBARDATA;

#define TB_LEFT_SPACING     5
#define TB_BOTTOM_SPACING   5

#define TBB_BORDER          2

#define TBB_TEXTSPACING     2

/* ******************************************************************
 *
 *   "Toolbar button" control
 *
 ********************************************************************/

/*
 *@@ ctlInitXButtonData:
 *
 */

VOID ctlInitXButtonData(PXBUTTONDATA pbd,   // in: button data
                        ULONG fl)           // in: TBBS_* style flags
{
    if (fl & TBBS_BIGICON)
    {
        pbd->szlIconOrBitmap.cx = G_cxIcon;
        pbd->szlIconOrBitmap.cy = G_cyIcon;
    }
    else if (fl & TBBS_MINIICON)
    {
        pbd->szlIconOrBitmap.cx = G_cxIcon / 2;
        pbd->szlIconOrBitmap.cy = G_cyIcon / 2;
    }
    else if (    (fl & TBBS_BITMAP)
              && (pbd->hptr)
            )
    {
        BITMAPINFOHEADER2 bmih2;
        bmih2.cbFix = sizeof(BITMAPINFOHEADER2);
        GpiQueryBitmapInfoHeader(pbd->hptr,
                                 &bmih2);
        pbd->szlIconOrBitmap.cx = bmih2.cx;
        pbd->szlIconOrBitmap.cy = bmih2.cy;
    }
}

/*
 *@@ ctlPaintTBButton:
 *      paints a tool bar button control. Can be called externally
 *      for just painting a button even if this is not really
 *      a window.
 *
 *@@added V0.9.13 (2001-06-21) [umoeller]
 *@@changed V0.9.16 (2001-10-24) [umoeller]: fixed wrong hatch color and paint offset
 *@@changed V0.9.16 (2001-10-28) [umoeller]: added bitmap support, fixed bad clip rectangle
 *@@changed V0.9.20 (2002-08-04) [umoeller]: fixed button offset, depressed color
 *@@changed V1.0.1 (2002-11-30) [umoeller]: moved this here from comctl.c, renamed
 *@@changed V1.0.1 (2002-12-08) [umoeller]: added support for WS_DISABLED
 */

VOID ctlPaintTBButton(HPS hps,               // in: presentation space (RGB mode)
                      ULONG fl,              // in: TBBS_* flags
                      PXBUTTONDATA pbd,      // in: button data
                      PXBUTTONSTATE pbs)     // in: button state
{
    LONG    lBorder = 0,
            lOfs = 0;
    LONG    lLeft,
            lRight,
            lColorMiddle = ctlQueryColor(&pbd->dwd, CTLCOL_BGND); // pbd->dwd.lcolBackground;
    RECTL   rclWin,
            rclTemp;
    POINTL  ptl;

    rclWin.xLeft = 0;
    rclWin.yBottom = 0;
    rclWin.xRight = pbd->dwd.szlWin.cx;
    rclWin.yTop = pbd->dwd.szlWin.cy;
    // make backup for later
    memcpy(&rclTemp, &rclWin, sizeof(RECTL));

    if (!(fl & TBBS_FLAT))
        lBorder = TBB_BORDER;

    gpihSwitchToRGB(hps);

    if (pbs->fPaintButtonSunk)
    {
        // paint button "down":
        lLeft = G_lcol3DDark;
        lRight = G_lcol3DLight;
        // add offset for icon painting at the bottom
        lOfs += 1;
        if (!lBorder)
            lBorder = 1;
    }
    else
    {
        lLeft = G_lcol3DLight;
        lRight = G_lcol3DDark;

        if (    (!lBorder)
             && (pbs->fMouseOver)
             && (fl & TBBS_HILITE)
           )
            lBorder = 1;
    }

    if (lBorder)
    {
        // paint button border
        // make rcl inclusive
        rclWin.xRight--;
        rclWin.yTop--;
        gpihDraw3DFrame2(hps,
                         &rclWin,        // inclusive
                         lBorder,
                         lLeft,
                         lRight);
        rclWin.xRight++;
        rclWin.yTop++;
    }

    if (fl & TBBS_BACKGROUND)
    {
        // now paint button middle
        if (pbs->fPaintButtonSunk)
            // make the depressed color darker
            gpihManipulateRGB(&lColorMiddle,
                              .95);
        else if ((fl & TBBS_HILITE) && (pbs->fMouseOver))
            // make the mouse over color lighter
            gpihManipulateRGB(&lColorMiddle,
                              1.05);

        WinFillRect(hps,
                    &rclWin,        // exclusive
                    lColorMiddle);
    }

    // calc x and y so that icon is centered in rectangle
    ptl.x = ((pbd->dwd.szlWin.cx - pbd->szlIconOrBitmap.cx) / 2);
    // center vertically only if we have no text
    if (fl & TBBS_TEXT)
        ptl.y = pbd->dwd.szlWin.cy - pbd->szlIconOrBitmap.cy - 2;
    else
        ptl.y = ((pbd->dwd.szlWin.cy - pbd->szlIconOrBitmap.cy) / 2);

    if (fl & TBBS_INUSE)
    {
        // caller wants in-use (hatched) emphasis:
        // draw a box then
        POINTL ptl2;
        ptl2.x = ptl.x - 2;
        ptl2.y = ptl.y - 2;
        GpiMove(hps,
                &ptl2);     // &ptl
                            // duh, typo V0.9.16 (2001-10-24) [umoeller]
        GpiSetPattern(hps, PATSYM_DIAG1);
        GpiSetColor(hps, RGBCOL_BLACK);     // V0.9.16 (2001-10-24) [umoeller]
        ptl2.x = ptl.x + pbd->szlIconOrBitmap.cx + 1; // inclusive!
        ptl2.y = ptl.y + pbd->szlIconOrBitmap.cy + 1; // inclusive!
        GpiBox(hps,
               DRO_FILL,
               &ptl2,
               0,
               0);
    }

    // make rcl inclusive
    rclWin.xRight--;
    rclWin.yTop--;
    GpiIntersectClipRectangle(hps,
                              &rclWin);    // inclusive!

    if (    (pbd->hptr)
         && (fl & (TBBS_BIGICON | TBBS_MINIICON | TBBS_BITMAP))
       )
    {
        // RECTL rcl3;

        // now paint icon
        ptl.x += lOfs;
        ptl.y -= lOfs;

        /*
        rcl3.xLeft = ptl.x;
        rcl3.yBottom = ptl.y;
        rcl3.xRight = ptl.x + pbd->szlIconOrBitmap.cx;
        rcl3.yTop = ptl.y + pbd->szlIconOrBitmap.cy;
        WinFillRect(hps, &rcl3, RGBCOL_GREEN);
        */

        if (fl & TBBS_BITMAP)
            // V0.9.16 (2001-10-28) [umoeller]
            WinDrawBitmap(hps,
                          pbd->hptr,           // a bitmap really
                          NULL,                 // entire bitmap
                          &ptl,
                          0,
                          0,
                          (fl & WS_DISABLED)
                               ? DBM_HALFTONE
                               : DBM_NORMAL);
        else
        {
            ULONG   fl2 = DP_NORMAL;     // 0x0000

            if (!(fl & TBBS_BIGICON))
                fl2 = DP_MINI;
            if (fl & WS_DISABLED)       // V1.0.1 (2002-12-08) [umoeller]
                fl2 |= DP_HALFTONED;    // I love this... DBM_HALFTONE, but DP_HALFTONED!
                                        // PM is just so half-toned itself...
            WinDrawPointer(hps,
                           ptl.x,
                           ptl.y,
                           pbd->hptr,
                           fl2);
        }

        rclTemp.yTop -= pbd->szlIconOrBitmap.cy;
    }

    if (    (pbd->dwd.pszText)
         && (fl & TBBS_TEXT)
       )
    {
        GpiSetColor(hps,
                    ctlQueryColor(&pbd->dwd, CTLCOL_FGND)); // pbd->dwd.lcolForeground);
        rclTemp.yTop -= 2 * TBB_TEXTSPACING + lOfs;
        rclTemp.xRight += 2 * lOfs;     // twice the offset because we center horizontally
        winhDrawFormattedText(hps,
                              &rclTemp,
                              pbd->dwd.pszText,
                              DT_CENTER | DT_TOP | DT_MNEMONIC | DT_WORDBREAK);
    }
}

/* static const SYSCOLORSET G_scsToolbarButton =
    {
        TRUE,       // inherit presparams

        SYSCLR_BUTTONMIDDLE,
        SYSCLR_MENUTEXT
    };
*/

static const CCTLCOLOR G_scsToolbarButton[] =
    {
        TRUE, PP_BACKGROUNDCOLOR, SYSCLR_BUTTONMIDDLE,
        TRUE, PP_FOREGROUNDCOLOR, SYSCLR_MENUTEXT,
    };

/*
 *@@ BtnAutoSize:
 *
 */

STATIC VOID BtnAutoSize(ULONG flStyle,
                        PTBBUTTONDATA pData)
{
    if (flStyle & TBBS_AUTORESIZE)
    {
        HPS hps;
        if (hps = WinGetPS(pData->bd.dwd.hwnd))
        {
            ULONG   ulBorder = 2;
            if (!(flStyle & TBBS_FLAT))
                ulBorder += TBB_BORDER;

            pData->bd.dwd.szlWin.cx
            = pData->bd.dwd.szlWin.cy
            = 2 * ulBorder;

            if (flStyle & (TBBS_BIGICON | TBBS_MINIICON | TBBS_BITMAP))
            {
                pData->bd.dwd.szlWin.cx += pData->bd.szlIconOrBitmap.cx;
                pData->bd.dwd.szlWin.cy += pData->bd.szlIconOrBitmap.cy;

                if (flStyle & TBBS_TEXT)
                    pData->bd.dwd.szlWin.cx += 2 * TBB_TEXTSPACING;
            }

            if (flStyle & TBBS_TEXT)
            {
                RECTL   rcl = { 0, 0, 100, 100 };
                LONG    lMinX;
                winhDrawFormattedText(hps,
                                      &rcl,
                                      pData->bd.dwd.pszText,
                                      DT_TOP | DT_LEFT | DT_QUERYEXTENT);

                lMinX = (rcl.xRight - rcl.xLeft) + 2 * TBB_TEXTSPACING;
                STOREIFMAX(lMinX, pData->bd.dwd.szlWin.cx);
                // pData->bd.dwd.rcl.xRight += (rcl.xRight - rcl.xLeft) + 2 * TBB_TEXTSPACING;
                pData->bd.dwd.szlWin.cy += (rcl.yTop - rcl.yBottom) + 2 * TBB_TEXTSPACING;
            }

            WinSetWindowPos(pData->bd.dwd.hwnd,
                            0,
                            0,
                            0,
                            pData->bd.dwd.szlWin.cx,
                            pData->bd.dwd.szlWin.cy,
                            SWP_SIZE | SWP_NOADJUST);

            WinReleasePS(hps);
        }
    }
}

/*
 *@@ BtnCreate:
 *      implementation for WM_CREATE in ctl_fnwpToolbarButton.
 */

STATIC MRESULT BtnCreate(HWND hwndButton, MPARAM mp1, MPARAM mp2)
{
    PTBBUTTONDATA   pData;
    MRESULT         mrc = 0;

    if (!(pData = NEW(TBBUTTONDATA)))
        mrc = (MRESULT)TRUE;       // stop window creation
    else
    {
        PCSZ pcszText = ((PCREATESTRUCT)mp2)->pszText;
        PSZ p;

        WinSetWindowPtr(hwndButton, QWL_USER + 1, pData);
        ZERO(pData);

        // initialize DEFWINDOWDATA
        ctlInitDWD(hwndButton,
                   mp2,
                   &pData->bd.dwd,
                   WinDefWindowProc,
                   0,
                   G_scsToolbarButton,
                   ARRAYITEMCOUNT(G_scsToolbarButton));

        if (    (pcszText)
             && (*pcszText == '#')
           )
        {
            pData->bd.hptr = atoi(pcszText + 1);
            if (p = strchr(pcszText + 1, '#'))
                pcszText = p + 1;
            else
                pcszText = NULL;
        }

        ctlInitXButtonData(&pData->bd,
                           ((PCREATESTRUCT)mp2)->flStyle);

        if (pcszText)
        {
            pData->bd.dwd.pszText = strdup(pcszText);
            if (    (((PCREATESTRUCT)mp2)->flStyle & TBBS_DROPMNEMONIC)
                 && (p = strchr(pData->bd.dwd.pszText, '~'))
               )
                memmove(p, p + 1, strlen(p));
        }

        BtnAutoSize(((PCREATESTRUCT)mp2)->flStyle,
                    pData);

        if (    (((PCREATESTRUCT)mp2)->flStyle & TBBS_CHECKINITIAL)
             && (((PCREATESTRUCT)mp2)->flStyle & (TBBS_CHECK | TBBS_RADIO))
           )
            pData->bs.fPaintButtonSunk = TRUE;
    }

    return mrc;
}

/*
 *@@ BtnButton1Down:
 *      implementation for WM_BUTTON1DOWN in ctl_fnwpToolbarButton.
 */

STATIC MRESULT BtnButton1Down(HWND hwndButton)
{
    PTBBUTTONDATA   pData;
    if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
    {
        ULONG   flStyle = winhQueryWindowStyle(hwndButton);

        if (flStyle & WS_DISABLED)
            WinAlarm(HWND_DESKTOP, WA_WARNING);
        else
        {
            WinSetFocus(HWND_DESKTOP, hwndButton);

            if (!pData->bs.fMouseCaptured)
            {
                // capture mouse events while the
                // mouse button is down
                WinSetCapture(HWND_DESKTOP, hwndButton);
                pData->bs.fMouseCaptured = TRUE;
            }

            if (!pData->bs.fPaintButtonSunk)
            {
                // toggle state is still UP (i.e. button pressed
                // for the first time): create menu
                pData->bs.fPaintButtonSunk = TRUE;
                WinInvalidateRect(hwndButton, NULL, FALSE);

                if (flStyle & TBBS_CHECK)
                    // ignore the next button 1 up
                    pData->bs.fIgnoreMB1Up = TRUE;

            } // end if (!pData->fButtonSunk)
        }
    }

    return (MRESULT)TRUE;        // processed
}

/*
 *@@ ClickNotifyOwner:
 *
 */

STATIC VOID ClickNotifyOwner(HWND hwndButton)
{
    HWND hwndOwner;

    if (hwndOwner = WinQueryWindow(hwndButton, QW_OWNER))
    {
        ULONG flStyle = winhQueryWindowStyle(hwndButton);

        if (flStyle & (TBBS_COMMAND | TBBS_SYSCOMMAND))
            WinPostMsg(hwndOwner,
                       (flStyle & TBBS_SYSCOMMAND)
                            ? WM_SYSCOMMAND
                            : WM_COMMAND,
                       (MPARAM)WinQueryWindowUShort(hwndButton, QWS_ID),
                       MPFROM2SHORT(CMDSRC_PUSHBUTTON,
                                    TRUE));     // pointer, not keyboard
        else
            ctlSendWmControl(hwndButton,
                             BN_CLICKED,
                             (MPARAM)hwndButton);
    }
}

/*
 *@@ BtnButton1Up:
 *      implementation for WM_BUTTON1UP in ctl_fnwpToolbarButton.
 */

STATIC MRESULT BtnButton1Up(HWND hwndButton)
{
    PTBBUTTONDATA   pData;
    if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
    {
        ULONG   flStyle = winhQueryWindowStyle(hwndButton);

        if (pData->bs.fMouseCaptured)
        {
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
            pData->bs.fMouseCaptured = FALSE;
        }

        if (!(flStyle & WS_DISABLED))
        {
            pData->bs.fMB1Pressed = FALSE;

            if (flStyle & TBBS_CHECK)
            {
                if (pData->bs.fIgnoreMB1Up)
                    pData->bs.fIgnoreMB1Up = FALSE;
                else
                    pData->bs.fPaintButtonSunk = FALSE;

                WinInvalidateRect(hwndButton, NULL, FALSE);

                ClickNotifyOwner(hwndButton);
            }
            else if (flStyle & TBBS_RADIO)
            {
                WinSendMsg(hwndButton,
                           TBBM_CHECK,
                           (MPARAM)1,
                           0);

                ClickNotifyOwner(hwndButton);
            }
            else
            {
                pData->bs.fPaintButtonSunk = FALSE;

                ClickNotifyOwner(hwndButton);

                WinInvalidateRect(hwndButton, NULL, FALSE);
            }
        }
    }

    return (MRESULT)TRUE;        // processed
}

#define IGNORE_CHECK_MAGIC 0x87678a1d

/*
 *@@ UncheckOthers:
 *      gets called twice from BtnCheck for radio buttons
 *      to uncheck the others in the group.
 */

STATIC VOID BtnUncheckOthers(HWND hwndButton,
                             ULONG ulQW)       // in: QW_PREV or QW_NEXT
{
    HWND    hwnd = hwndButton;
    CHAR    szClass[50];

    while (hwnd = WinQueryWindow(hwnd, ulQW))
    {
        if (    (!WinQueryClassName(hwnd, sizeof(szClass), szClass)
             || (strcmp(szClass, WC_CCTL_TBBUTTON))
             || (!(winhQueryWindowStyle(hwnd) & TBBS_RADIO)))
           )
            break;

        WinSendMsg(hwnd,
                   TBBM_CHECK,
                   (MPARAM)FALSE,
                   (MPARAM)IGNORE_CHECK_MAGIC);     // force uncheck without resending
    }
}

/*
 *@@ BtnCheck:
 *      implementation for TBBM_CHECK in ctl_fnwpToolbarButton.
 */

STATIC VOID BtnCheck(HWND hwndButton,
                     BOOL fCheck,
                     ULONG ulMagic)
{
    PTBBUTTONDATA   pData;

    if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
    {
        ULONG flStyle = winhQueryWindowStyle(hwndButton);

        if (    (flStyle & TBBS_CHECK)
             || (ulMagic == IGNORE_CHECK_MAGIC)
                    // magic code sent to radio tool bar buttons to
                    // force an uncheck without resending
           )
        {
            pData->bs.fPaintButtonSunk = fCheck;
        }
        else if (flStyle & TBBS_RADIO)
        {
            BtnUncheckOthers(hwndButton, QW_PREV);
            BtnUncheckOthers(hwndButton, QW_NEXT);

            pData->bs.fPaintButtonSunk = TRUE;
        }

        WinInvalidateRect(hwndButton, NULL, FALSE);
    }
}

/*
 *@@ ctl_fnwpToolbarButton:
 *      window proc for the tool bar button control.
 *
 *      This control is not at all based on the ugly OS/2 button
 *      control, but a complete rewrite. This supports a large
 *      variety of TBBS_* style flags which are useful in the
 *      context of a tool bar.
 *
 *      The following styles are supported:
 *
 *      --  WS_DISABLED @@todo halftone the display
 *
 *      --  optionally one of TBBS_BIGICON, TBBS_MINIICON, or
 *          TBBS_BITMAP to paint a picture in the control
 *
 *      --  optionally TBBS_TEXT; you can use this alone or
 *          together with one of the picture styles
 *
 *      --  TBBS_CHECK: if set, button toggles between pressed
 *          and released on every click ("checkbox" style,
 *          even though it still looks as a button).
 *
 *      --  TBBS_RADIO: if set, the button assumes it is part of
 *          a group and behaves like a radio button, that is, it
 *          automatically unchecks its sibling buttons which have
 *          this style too.
 *
 *      --  TBBS_AUTORESIZE: if set, the button automatically
 *          resizes itself to the space it needs when its style
 *          or text changes.
 *
 *      --  TBBS_HILITE: if set, the button hilites when mouse
 *          moves over it.
 *
 *      --  TBBS_FLAT: if set, the button paints a border only
 *          if the mouse is moving over it; if not set, it
 *          always has a (thicker) border.
 *
 *      --  TBBS_COMMAND, TBBS_SYSCOMMAND: if none of these are
 *          set, the button _sends_ WM_CONTROL with the standard
 *          button code BN_CLICKED to its owner when it is
 *          pressed. If TBBS_COMMAND is set, the button _posts_
 *          WM_COMMAND instead; if TBBS_SYSCOMMAND is set, the
 *          button posts WM_SYSCOMMAND instead.
 *
 *          Note that this is different from the standard button
 *          behavior: even a tool bar button that does not have
 *          the TBBS_CHECK or TBBS_RADIO styles will only post
 *          WM_COMMAND if the TBBS_COMMAND style is set.
 *
 *      There are two ways to set the icon or bitmap to be
 *      displayed with the control:
 *
 *      --  Pass it with the window title on tool bar creation in
 *          the form "#handle#text", where "handle" is the decimal
 *          HPOINTER or HBITMAP and "text" is the actual button
 *          text. Note that this only works on creation, not with
 *          WinSetWindowText after creation.
 *
 *      --  Send a XBBM_SETHANDLE message after button creation.
 */

MRESULT EXPENTRY ctl_fnwpToolbarButton(HWND hwndButton, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT         mrc = 0;
    PTBBUTTONDATA   pData;

    switch (msg)
    {
        case WM_CREATE:
            mrc = BtnCreate(hwndButton, mp1, mp2);
        break;

        case WM_BUTTON1DOWN:
            mrc = BtnButton1Down(hwndButton);
        break;

        case WM_BUTTON1UP:
            mrc = BtnButton1Up(hwndButton);
        break;

        case WM_MOUSEENTER:
        case WM_MOUSELEAVE:
            if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
            {
                BOOL    fMouseOver = (msg == WM_MOUSEENTER);
                if (fMouseOver != pData->bs.fMouseOver)
                {
                    pData->bs.fMouseOver = fMouseOver;

                    if (winhQueryWindowStyle(hwndButton) & TBBS_HILITE)
                        WinInvalidateRect(hwndButton, NULL, FALSE);
                }
            }
        break;

        case WM_PAINT:
        {
            HPS hps;
            RECTL rcl;
            POINTL ptl;
            if (hps = WinBeginPaint(hwndButton, NULLHANDLE, &rcl))
            {
                if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
                {
                    gpihSwitchToRGB(hps);
                    ctlPaintTBButton(hps,
                                     winhQueryWindowStyle(hwndButton)
                                        | TBBS_BACKGROUND,
                                     &pData->bd,
                                     &pData->bs);
                }

                WinEndPaint(hps);
            }
        }
        break;

        /*
         *@@ TBBM_CHECK:
         *      checks the given button. Effect depends on the
         *      button style:
         *
         *      --  With TBBS_CHECK, this sets the button check
         *          state to (BOOL)mp1.
         *
         *      --  With TBBS_CHECKGROUP, this sets the current
         *          button check state and unchecks neighboring
         *          buttons (siblings) that have the same style.
         *          mp1 is ignored.
         *
         *      As opposed to a check in response to a mouse
         *      event, this does _not_ send out the BN_CLICKED
         *      notification.
         */

        case TBBM_CHECK:
            BtnCheck(hwndButton, (BOOL)mp1, (ULONG)mp2);
        break;

        /*
         *@@ TBBM_QUERYCHECK:
         *      returns the current check status of a button
         *      with TBBS_CHECK or TBBS_CHECKGROUP style
         *      as TRUE or FALSE.
         *
         *      No parameters.
         */

        case TBBM_QUERYCHECK:
            if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
                mrc = (MRESULT)pData->bs.fPaintButtonSunk;
        break;

        case WM_DESTROY:
            if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
            {
                FREE(pData->bd.dwd.pszText);
                free(pData);
            }
        break;

        default:
            if (pData = (PTBBUTTONDATA)WinQueryWindowPtr(hwndButton, QWL_USER + 1))
                mrc = ctlDefWindowProc(&pData->bd.dwd, msg, mp1, mp2);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   "Toolbar" control
 *
 ********************************************************************/

static const CCTLCOLOR G_scsToolbar[] =
    {
        TRUE, PP_BACKGROUNDCOLOR, SYSCLR_MENU,
        TRUE, PP_FOREGROUNDCOLOR, SYSCLR_MENUTEXT
    };

/*
 *@@ CreateToolbarControl:
 *
 */

STATIC HWND CreateToolbarControl(PTOOLBARDATA pData,
                                 PTOOLBARCONTROL pControl,
                                 PLONG px,
                                 PPRESPARAMS ppp)
{
    HWND hwndControl;

    if (hwndControl = WinCreateWindow(pData->dwd.hwnd,
                                      (PSZ)pControl->pcszClass,
                                      (PSZ)pControl->pcszTitle,
                                      pControl->flStyle,
                                      *px,
                                      TB_BOTTOM_SPACING,
                                      pControl->cx,
                                      pControl->cy,
                                      pData->hwndControlsOwner,
                                      HWND_TOP,
                                      pControl->id,
                                      NULL,
                                      ppp))
    {
        *px += pControl->cx + pData->lSpacing;

        if (pData->hwndToolTip)
        {
            TOOLINFO    ti = {0};
            ti.ulFlags = TTF_CENTER_X_ON_TOOL | TTF_POS_Y_BELOW_TOOL | TTF_SUBCLASS;
            ti.hwndToolOwner = pData->dwd.hwnd;
            ti.pszText = PSZ_TEXTCALLBACK;
            ti.hwndTool = hwndControl;
            WinSendMsg(pData->hwndToolTip,
                       TTM_ADDTOOL,
                       (MPARAM)0,
                       &ti);
        }
    }

    return hwndControl;
}

/*
 *@@ ReformatControls:
 *
 */

STATIC VOID ReformatControls(HWND hwndToolBar)
{
    PTOOLBARDATA pData;
    if (pData = (PTOOLBARDATA)WinQueryWindowPtr(hwndToolBar, QWL_USER + 1))
    {
        LONG        x = TB_LEFT_SPACING;
        PLISTNODE   pNode;
        LONG        cControls;
        PSWP        paswp,
                    pswpThis;

        if (    (cControls = lstCountItems(&pData->llControls))
             && (paswp = (PSWP)malloc(sizeof(SWP) * cControls))
           )
        {
            BOOL rc;

            pswpThis = paswp;

            FOR_ALL_NODES(&pData->llControls, pNode)
            {
                HWND    hwnd = (HWND)pNode->pItemData;
                SWP     swp;
                CHAR    szClass[50];

                WinQueryWindowPos(hwnd, &swp);

                if (    (WinQueryClassName(hwnd, sizeof(szClass), szClass))
                     && (!strcmp(szClass, WC_CCTL_SEPARATOR))
                   )
                {
                    pswpThis->cy = pData->lMaxControlCY;
                    pswpThis->fl = SWP_MOVE | SWP_SIZE;
                }
                else
                {
                    pswpThis->cy = swp.cy;
                    pswpThis->fl = SWP_MOVE;
                }

                pswpThis->cx = swp.cx;
                pswpThis->y = TB_BOTTOM_SPACING;
                pswpThis->x = x;
                pswpThis->hwndInsertBehind = HWND_BOTTOM;
                pswpThis->hwnd = hwnd;

                x += swp.cx + pData->lSpacing;

                if (swp.cy > pData->lMaxControlCY)
                {
                    pData->lMaxControlCY = swp.cy;
                    pData->flReformat = RFFL_HEIGHT;
                }

                pswpThis++;
            }

            rc = WinSetMultWindowPos(pData->dwd.hab,
                                     paswp,
                                     cControls);

            free(paswp);
        }
    }
}

/*
 *@@ TbAddControls:
 *
 */

STATIC ULONG TbAddControls(PTOOLBARDATA pData,
                           ULONG cControls,
                           PTOOLBARCONTROL paControls,
                           LONG lIndex)                   // in: index before which to add entries; -1 means rightmost
{
    ULONG   ul,
            cCreated = 0;

    LONG    x = 0;
    HWND    hwndBefore;
    SWP     swp;

    if (!lIndex)
        x = TB_LEFT_SPACING;
    else
    {
        if (    (lIndex > 0)
             && (hwndBefore = (HWND)lstItemFromIndex(&pData->llControls,
                                                     lIndex))
             && (WinQueryWindowPos(hwndBefore, &swp))
           )
        {
            x = swp.x + swp.cx + pData->lSpacing;
        }
    }

    if (x)
    {
        PPRESPARAMS ppp = NULL;

        PCSZ    pcszFont = winhQueryDefaultFont();
        LONG    lColor;

        winhStorePresParam(&ppp,
                           PP_FONTNAMESIZE,
                           strlen(pcszFont) + 1,
                           (PVOID)pcszFont);

        lColor = ctlQueryColor(&pData->dwd, CTLCOL_BGND);
        winhStorePresParam(&ppp,
                           PP_BACKGROUNDCOLOR,
                           sizeof(lColor),
                           &lColor);

        lColor = ctlQueryColor(&pData->dwd, CTLCOL_FGND);
        winhStorePresParam(&ppp,
                           PP_FOREGROUNDCOLOR,
                           sizeof(lColor),
                           &lColor);

        // create controls
        for (ul = 0;
             ul < cControls;
             ++ul)
        {
            HWND    hwndControl;

            if (hwndControl = CreateToolbarControl(pData,
                                                   &paControls[ul],
                                                   &x,
                                                   ppp))
            {
                lstInsertItemBefore(&pData->llControls,
                                    (PVOID)hwndControl,
                                    lIndex++);
                ++cCreated;
            }
            else
                break;
        }

        if (ppp)
            free(ppp);
    }

    pData->lMaxControlCY = 0;
    ReformatControls(pData->dwd.hwnd);

    if (pData->flReformat & RFFL_HEIGHT)
    {
        if (WinQueryWindowULong(pData->dwd.hwnd, QWL_STYLE) & TBS_AUTORESIZE)
        {
            WinQueryWindowPos(pData->dwd.hwnd, &swp);
            WinSetWindowPos(pData->dwd.hwnd,
                            0,
                            0,
                            0,
                            swp.cx,
                            pData->lMaxControlCY + 2 * TB_BOTTOM_SPACING,
                            SWP_SIZE);

            ReformatControls(pData->dwd.hwnd);

            ctlPostWmControl(pData->dwd.hwnd,
                             TBN_RESIZED,
                             0);

            pData->flReformat &= ~RFFL_HEIGHT;
        }
    }

    return cCreated;
}

/*
 *@@ TbCreate:
 *
 */

STATIC MRESULT TbCreate(HWND hwndToolBar, MPARAM mp1, MPARAM mp2)
{
    PTOOLBARDATA pData;
    PTOOLBARCDATA ptbcd = (PTOOLBARCDATA)mp1;

    if (!(pData = NEW(TOOLBARDATA)))
        return (MRESULT)TRUE;       // stop window creation

    WinSetWindowPtr(hwndToolBar, QWL_USER + 1, pData);
    ZERO(pData);

    // initialize DEFWINDOWDATA
    ctlInitDWD(hwndToolBar,
               mp2,
               &pData->dwd,
               WinDefWindowProc,
               0,
               G_scsToolbar,
               ARRAYITEMCOUNT(G_scsToolbar));

    pData->hwndControlsOwner = ptbcd->hwndControlsOwner;
    pData->lSpacing = 5;
    lstInit(&pData->llControls, FALSE);

    xstrInit(&pData->strToolTipBuf, 0);

    if (((PCREATESTRUCT)mp2)->flStyle & TBS_TOOLTIPS)
    {
        pData->hwndToolTip = WinCreateWindow(HWND_DESKTOP,
                                             WC_CCTL_TOOLTIP,
                                             NULL,
                                             TTS_ALWAYSTIP,
                                             0, 0, 0, 0,    // window pos and size, ignored
                                             hwndToolBar,
                                             NULLHANDLE,    // hwndInsertBehind, ignored
                                             0,
                                             NULL,          // control data
                                             NULL);         // presparams
    }

    if (    (ptbcd->cControls)
         && (ptbcd->patbc)
       )
    {
        TbAddControls(pData,
                      ptbcd->cControls,
                      ptbcd->patbc,
                      0);
    }

    return (MRESULT)FALSE;
}

/*
 *@@ TbDestroy:
 *
 */

STATIC VOID TbDestroy(HWND hwndToolBar)
{
    PTOOLBARDATA pData;
    if (pData = (PTOOLBARDATA)WinQueryWindowPtr(hwndToolBar, QWL_USER + 1))
    {
        PLISTNODE pNode;

        if (pData->hwndToolTip)
            WinDestroyWindow(pData->hwndToolTip);

        FOR_ALL_NODES(&pData->llControls, pNode)
        {
            WinDestroyWindow((HWND)pNode->pItemData);
        }
        lstClear(&pData->llControls);

        xstrClear(&pData->strToolTipBuf);

        free(pData);
    }
}

/*
 *@@ ctl_fnwpToolbar:
 *      window proc for the tool bar class.
 *
 *      The tool bar understands the following messages:
 *
 *      --  TBM_ADDCONTROLS
 */

MRESULT EXPENTRY ctl_fnwpToolbar(HWND hwndToolBar, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;
    PTOOLBARDATA pData;

    switch (msg)
    {
        case WM_CREATE:
            TbCreate(hwndToolBar, mp1, mp2);
        break;

        /*
         *@@ TBM_ADDCONTROLS:
         *      adds new controls to the tool bar.
         *
         *      Parameters:
         *
         *      --  PTOOLBARCONTROL mp1: array of TOOLBARCONTROL structs
         *          which specify the windows to add to the tool bar.
         *
         *      --  SHORT1FROMMP(mp2): number of items in that array
         *          (not array size).
         *
         *      --  SHORT2FROMMP(mp2): index where to add the new controls.
         *          0 means leftmost, 1 before the second item, and so on.
         *          -1 means add rightmost.
         *
         *      The tool bar will automatically repaint itself. If it
         *      also has the TBS_AUTORESIZE window style, it will even
         *      automatically resize itself and post its owner a
         *      WM_CONTROL with TBN_RESIZED message so it can adjust
         *      itself.
         */

        case TBM_ADDCONTROLS:
            if (pData = (PTOOLBARDATA)WinQueryWindowPtr(hwndToolBar, QWL_USER + 1))
            {
                TbAddControls(pData,
                              SHORT1FROMMP(mp2),
                              (PTOOLBARCONTROL)mp1,
                              SHORT2FROMMP(mp2));
            }
        break;

        case WM_CONTROL:
            if (    (pData = (PTOOLBARDATA)WinQueryWindowPtr(hwndToolBar, QWL_USER + 1))
                 && (pData->hwndToolTip)
                 && (SHORT2FROMMP(mp1) == TTN_NEEDTEXT)
               )
            {
                PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
                PSZ psz;
                xstrClear(&pData->strToolTipBuf);

                if (psz = winhQueryWindowText(pttt->hwndTool))
                    xstrset(&pData->strToolTipBuf, psz);

                pttt->ulFormat = TTFMT_PSZ;
                pttt->pszText = pData->strToolTipBuf.psz;
            }
        break;

        case WM_PAINT:
        {
            HPS hps;
            RECTL rcl;
            POINTL ptl;
            if (hps = WinBeginPaint(hwndToolBar, NULLHANDLE, &rcl))
            {
                gpihSwitchToRGB(hps);
                rcl.yBottom += 2;
                WinFillRect(hps,
                            &rcl,
                            winhQueryPresColor2(hwndToolBar,
                                                PP_BACKGROUNDCOLOR,
                                                PP_BACKGROUNDCOLORINDEX,
                                                FALSE,
                                                SYSCLR_MENU));
                ptl.x = 0;
                ptl.y = 0;
                GpiSetColor(hps, WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0));
                GpiMove(hps, &ptl);
                ptl.x = rcl.xRight;
                GpiLine(hps, &ptl);
                ptl.x = 0;
                ptl.y = 1;
                GpiSetColor(hps, WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0));
                GpiMove(hps, &ptl);
                ptl.x = rcl.xRight;
                GpiLine(hps, &ptl);

                WinEndPaint(hps);
            }
        }
        break;

        case WM_DESTROY:
            TbDestroy(hwndToolBar);
        break;

        default:
            if (pData = (PTOOLBARDATA)WinQueryWindowPtr(hwndToolBar, QWL_USER + 1))
                mrc = ctlDefWindowProc(&pData->dwd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ ctlRegisterToolbar:
 *      this registers the tool bar window class (ctl_fnwpToolbar)
 *      _and_ the tool bar button control (ctl_fnwpToolbarButton)
 *      for an application. This is required before the tool bar
 *      control can be used.
 */

BOOL ctlRegisterToolbar(HAB hab)
{
    return (    WinRegisterClass(hab,
                                 WC_CCTL_TOOLBAR,
                                 ctl_fnwpToolbar,
                                 CS_SYNCPAINT | /* CS_CLIPSIBLINGS | */ CS_CLIPCHILDREN,
                                 sizeof(PVOID) * 2)   // addt'l bytes to reserve:
                                         // one pointer for QWL_USER,
                                         // one more for instance data
             && WinRegisterClass(hab,
                                 WC_CCTL_TBBUTTON,
                                 ctl_fnwpToolbarButton,
                                 CS_SYNCPAINT | CS_CLIPSIBLINGS | CS_CLIPCHILDREN,
                                 sizeof(PVOID) * 2)   // addt'l bytes to reserve:
                                         // one pointer for QWL_USER,
                                         // one more for instance data
           );
}

/*
 *@@ ctlCreateToolBar:
 *      type-safe wrapper around WinCreateWindow to create a tool bar.
 *
 *      The easiest way to create a tool bar completely with the tools
 *      is to pass them as an array of TOOLBARCONTROL structs here,
 *      which simply specify the window classes to create. In most
 *      cases, you will want to add tools of the WC_CCTL_TBBUTTON
 *      and WC_SEPARATORLINE classes.
 *
 *      Keep in mind to call ctlRegisterToolbar and ctlRegisterSeparatorLine
 *      first, or window creation will fail.
 */

HWND ctlCreateToolBar(HWND hwndParent,      // in: parent of tool bar (e.g. frame)
                      HWND hwndOwner,       // in: owner of tool bar itself (e.g. frame)
                      ULONG flStyle,        // in: window style (WS_VISIBLE | TBS_* flags)
                      HWND hwndControlsOwner, // in: owner for tool bar controls (e.g. frame client)
                      ULONG cControls,
                      PTOOLBARCONTROL patbc)
{
    TOOLBARCDATA    tbcd;
    memset(&tbcd, 0, sizeof(tbcd));
    tbcd.cb = sizeof(tbcd);
    tbcd.hwndControlsOwner = hwndControlsOwner;
    tbcd.cControls = cControls;
    tbcd.patbc = patbc;

    return WinCreateWindow(hwndParent,
                           WC_CCTL_TOOLBAR,
                           NULL,
                           flStyle,
                           0,
                           0,
                           0,
                           0,
                           hwndOwner,
                           HWND_BOTTOM,
                           FID_TOOLBAR,
                           &tbcd,
                           NULL);
}

