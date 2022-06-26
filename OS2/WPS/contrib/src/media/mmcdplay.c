
/*
 *@@sourcefile mmcdplay.c:
 *      implementation for the XMMCDPlayer class.
 *
 *      Note: Those G_mmio* and G_mci* identifiers are global
 *      variables containing MMPM/2 API entries. Those are
 *      resolved by xmmInit (mmthread.c) and must only be used
 *      after checking xmmQueryStatus.
 *
 *@@added V0.9.7 (2000-12-20) [umoeller]
 *@@header "media\media.h"
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
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
 *  7)  headers in filesys\ (as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINTRACKRECT
#define INCL_WINPOINTERS
#define INCL_WINRECTANGLES
#define INCL_WININPUT
#define INCL_WINTIMER
#define INCL_WINSYS
#define INCL_WINBUTTONS

#define INCL_GPI                // required for INCL_MMIO_CODEC
#define INCL_GPIBITMAPS         // required for INCL_MMIO_CODEC
#define INCL_GPIREGIONS
#include <os2.h>

// multimedia includes
#define INCL_MCIOS2
#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

// C library headers
#include <stdio.h>              // needed for except.h
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h
#include <io.h>                 // access etc.

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
#include "xmmcdplay.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file

#include "media\media.h"                // XWorkplace multimedia support

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

#define CDP_CICONS          6

#define BUTTON_BORDER       1       // border width
#define BUTTON_ICONSPACING  1       // room between icon and border

// button indices
#define CDBIDX_PREV         0
#define CDBIDX_PLAY         1
#define CDBIDX_PAUSE        2
#define CDBIDX_STOP         3
#define CDBIDX_NEXT         4
#define CDBIDX_EJECT        5

/*
 *@@ CDPLAYWINDATA:
 *
 */

typedef struct _CDPLAYWINDATA
{
    USHORT              cbSize;

    XMMCDPlayer         *somSelf;

    USEITEM             UseItem;            // use item; immediately followed by view item
    VIEWITEM            ViewItem;           // view item

    HAB                 hab;
    PFNWP               pfnwpFrameOrig;

    ULONG               cx,
                        cy;
    RECTL               rclClient;

    LONG                yTrackNTime;        // y offset of track and time (client coords)
    ULONG               cyTrackNTime;

    LONG                lcol3DDark,
                        lcol3DLight;

    HWND                ahwndButtons[CDP_CICONS];
    HPOINTER            ahptrButtons[CDP_CICONS];

    // cached data for display to avoid flickering
    ULONG               ulTrack;
    ULONG               ulSeconds;

    HWND                hwndContextMenu;

} CDPLAYWINDATA, *PCDPLAYWINDATA;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static BOOL     G_fCDPlayerClientRegistered = FALSE;

static const ULONG G_aulButtonIDs[] =
    {
        ID_ICON_CDPREV,
        ID_ICON_CDPLAY,
        ID_ICON_CDPAUSE,
        ID_ICON_CDSTOP,
        ID_ICON_CDNEXT,
        ID_ICON_CDEJECT
    };

/* ******************************************************************
 *
 *   Flat button window class
 *
 ********************************************************************/

typedef struct _FLATBUTTONDATA
{
    HWND            hwnd;

    ULONG           ulState;
            // 0: draw icon only (no frame).
            // 1: draw hilite (mouse-over frame).
            // 2: draw button depressed.
    BOOL            fCaptured;

    HPOINTER        hptr;
            // pointer to draw... this is not auto-freed!

    ULONG           cxMiniIcon;

    LONG            lcolBackground,
                    lcol3DDark,
                    lcol3DLight;

} FLATBUTTONDATA, *PFLATBUTTONDATA;

#define WC_FLATBUTTON   "XWPFlatButton"

/*
 *@@ ButtonCreate:
 *      implementation for WM_CREATE in fnwpFlatButton.
 */

STATIC MRESULT ButtonCreate(HWND hwnd)
{
    MRESULT mrc = (MPARAM)FALSE;

    PFLATBUTTONDATA pfbd = (PFLATBUTTONDATA)malloc(sizeof(FLATBUTTONDATA));
    if (pfbd)
    {
        memset(pfbd, 0, sizeof(*pfbd));
        pfbd->hwnd = hwnd;
        WinSetWindowPtr(hwnd, QWL_USER, pfbd);

        pfbd->cxMiniIcon = G_cxIcon; // WinQuerySysValue(HWND_DESKTOP, SV_CXICON) / 2;

        pfbd->lcolBackground
            = winhQueryPresColor(WinQueryWindow(hwnd, QW_OWNER),
                                 PP_BACKGROUNDCOLOR,
                                 FALSE,
                                 SYSCLR_BUTTONMIDDLE);
        pfbd->lcol3DDark = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0);
        pfbd->lcol3DLight = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);
    }
    else
        mrc = (MPARAM)TRUE;

    return mrc;
}

/*
 *@@ ButtonPaint2:
 *
 */

STATIC VOID ButtonPaint2(PFLATBUTTONDATA pfbd,
                         HPS hps)
{
    RECTL       rclButton,
                rclIcon;
    // LONG        lSaved = 0;

    WinQueryWindowRect(pfbd->hwnd, &rclButton);
            // exclusive, top right is the same as window cx/cy

    // center icon in button
    rclIcon.xLeft =   rclButton.xLeft
                      + (
                          ( rclButton.xRight - rclButton.xLeft )
                          - pfbd->cxMiniIcon
                        ) / 2,
    rclIcon.yBottom = rclButton.yBottom
                      + (
                          ( rclButton.yTop - rclButton.yBottom )
                          - pfbd->cxMiniIcon
                        ) / 2;

    if (pfbd->ulState)        // 1 or 2
    {
        // this needs an inclusive rectangle, so patch this
        LONG lLeft, lRight;

        if (pfbd->ulState == 1)
        {
            // mouse-over state:
            lLeft = pfbd->lcol3DLight;
            lRight = pfbd->lcol3DDark;
        }
        else
        {
            // depressed state:
            lLeft = pfbd->lcol3DDark;
            lRight = pfbd->lcol3DLight;
            rclIcon.xLeft += 2;
            rclIcon.yBottom -= 2;
        }

        // gpihDraw3DFrame wants an inclusive rectangle
        rclButton.xRight--;
        rclButton.yTop--;
        gpihDraw3DFrame2(hps,            // inclusive
                         &rclButton,
                         BUTTON_BORDER,
                         lLeft,
                         lRight);

        /*
        rclButton.xLeft += BUTTON_BORDER;
        rclButton.yBottom += BUTTON_BORDER;
        rclButton.xRight -= BUTTON_BORDER - 1;
        rclButton.yTop -= BUTTON_BORDER - 1;
        */

        rclButton.xRight++;
        rclButton.yTop++;

        WinFillRect(hps,                // exclusive
                    &rclButton,
                    RGBCOL_RED);
    }
    else
        // no frame:
        WinFillRect(hps,
                    &rclButton,
                    pfbd->lcolBackground);

    // calculate clip rectangle for painting the icon;
    // GpiIntersectClipRectangle wants an exclusive (!)
    // rectangle again... geeze
    rclIcon.xRight = rclButton.xRight - 1;
    rclIcon.yTop = rclButton.yTop - 1;

    GpiIntersectClipRectangle(hps, &rclIcon);       // exclusive
    WinDrawPointer(hps,
                   rclIcon.xLeft,
                   rclIcon.yBottom,
                   pfbd->hptr,
                   DP_MINI);
}

/*
 *@@ ButtonPaint:
 *      implementation for WM_PAINT in fnwpFlatButton.
 *
 *      ulState can be:
 *
 *      --  0: draw icon only (no frame).
 *      --  1: draw hilite (mouse-over frame).
 *      --  2: draw button depressed.
 */

STATIC VOID ButtonPaint(HWND hwnd)
{
    PFLATBUTTONDATA pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);

    HPS hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
    if ((pfbd) && (hps))
    {
        gpihSwitchToRGB(hps);
        ButtonPaint2(pfbd, hps);
    }
    WinReleasePS(hps);
}

/*
 *@@ ButtonSetIcon:
 *      implementation for WM_SETICON in fnwpFlatButton.
 */

STATIC VOID ButtonSetIcon(HWND hwnd, MPARAM mp1)
{
    PFLATBUTTONDATA pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    if (pfbd)
    {
        pfbd->hptr = (HPOINTER)mp1;
        WinInvalidateRect(hwnd, NULL, FALSE);
    }
}

/*
 *@@ ButtonPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 */

STATIC VOID ButtonPresParamChanged(HWND hwnd, MPARAM mp1)
{
    PFLATBUTTONDATA pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    if (pfbd)
    {
        ULONG ulAttrChanged = (ULONG)mp1;
        BOOL fInvalidate = TRUE;
        // ULONG ul;

        switch (ulAttrChanged)
        {
            case 0:     // layout palette thing dropped
            case PP_BACKGROUNDCOLOR:
                pfbd->lcolBackground
                    = winhQueryPresColor(WinQueryWindow(hwnd, QW_OWNER),
                                         PP_BACKGROUNDCOLOR,
                                         FALSE,
                                         SYSCLR_BUTTONMIDDLE);
            break;

            default:
                fInvalidate = FALSE;
        }

        if (fInvalidate)
            WinInvalidateRect(hwnd, NULL, FALSE);
    }
}

/*
 *@@ ButtonMouseMove:
 *      implementation for WM_MOUSEMOVE in fnwpFlatButton.
 */

STATIC MRESULT ButtonMouseMove(HWND hwnd, MPARAM mp1)
{
    PFLATBUTTONDATA  pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);

    // only do this if
    // -- the button is not currently down
    // -- and the button hasn't been given "mouse-over" emphasis yet
    if (pfbd->ulState == 0)
    {
        HPS hps = WinGetPS(hwnd);
        gpihSwitchToRGB(hps);

        pfbd->ulState = 1;      // now has mouse-over emphasis

        ButtonPaint2(pfbd, hps);

        WinReleasePS(hps);
    }

    return (MRESULT)TRUE;
}

/*
 *@@ ButtonMouseLeave:
 *      implementation for WM_MOUSELEAVE in fnwpFlatButton.
 */

STATIC MRESULT ButtonMouseLeave(HWND hwnd)
{
    PFLATBUTTONDATA  pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);

    if (pfbd->ulState == 1)       // mouse-over emphasis, but not "down"
    {
        HPS hps = WinGetPS(hwnd);
        gpihSwitchToRGB(hps);

        pfbd->ulState = 0;

        // unpaint button
        ButtonPaint2(pfbd, hps);

        WinReleasePS(hps);
    }

    return (MRESULT)TRUE;
}

/*
 *@@ ButtonClick:
 *      implementation for BM_CLICK in fnwpFlatButton.
 */

STATIC MRESULT ButtonClick(HWND hwnd,
                           MPARAM mp1)     // if FALSE, downclick; if TRUE, upclick
{
    PFLATBUTTONDATA  pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    BOOL fUpclick = (BOOL)mp1;
    HPS hps = WinGetPS(hwnd);
    gpihSwitchToRGB(hps);

    if (!fUpclick)
    {
        // down-click:
        pfbd->fCaptured = WinSetCapture(HWND_DESKTOP, hwnd);
        pfbd->ulState = 2;      // button "down"
    }
    else
    {
        // up-click:
        pfbd->ulState = 1;      // button "mouse over"
        if (pfbd->fCaptured)
        {
            WinSetCapture(HWND_DESKTOP, NULLHANDLE);
            pfbd->fCaptured = FALSE;
        }

        WinPostMsg(WinQueryWindow(hwnd, QW_OWNER),
                   WM_COMMAND,
                   (MPARAM)WinQueryWindowUShort(hwnd, QWS_ID),
                   MPFROM2SHORT(CMDSRC_PUSHBUTTON,
                                TRUE));     // pointer device operation
    }

    // rrrrepaint
    ButtonPaint2(pfbd, hps);

    WinReleasePS(hps);

    return (MRESULT)TRUE;
}

/*
 *@@ ButtonDestroy:
 *      implementation for WM_DESTROY in fnwpFlatButton.
 */

STATIC VOID ButtonDestroy(HWND hwnd)
{
    PFLATBUTTONDATA pfbd = (PFLATBUTTONDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    if (pfbd)
    {
        free(pfbd);
        WinSetWindowPtr(hwnd, QWL_USER, 0);
    }
}

/*
 *@@ fnwpFlatButton:
 *      window proc for the "flat button" window class.
 */

STATIC MRESULT EXPENTRY fnwpFlatButton(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CREATE:
            mrc = ButtonCreate(hwnd);
        break;

        case WM_PAINT:
            ButtonPaint(hwnd);
        break;

        case WM_SETICON:
            ButtonSetIcon(hwnd, mp1);
        break;

        case WM_PRESPARAMCHANGED:
            ButtonPresParamChanged(hwnd, mp1);
        break;

        case WM_MOUSEMOVE:
            mrc = ButtonMouseMove(hwnd, mp1);
        break;

#ifndef WM_MOUSEENTER
    #define WM_MOUSEENTER   0x041E
#endif
#ifndef WM_MOUSELEAVE
    #define WM_MOUSELEAVE   0x041F
#endif

        case WM_MOUSELEAVE:
            mrc = ButtonMouseLeave(hwnd);
        break;

        case WM_BUTTON1DOWN:
            WinSendMsg(hwnd,
                       BM_CLICK,
                       (MPARAM)FALSE,       // downclick
                       0);
            mrc = (MPARAM)TRUE;
        break;

        case WM_BUTTON1UP:
            mrc = (MPARAM)TRUE;
        break;

        case WM_BUTTON1CLICK:
        case WM_BUTTON1DBLCLK:
            WinSendMsg(hwnd,
                       BM_CLICK,
                       (MPARAM)TRUE,       // upclick
                       0);
            mrc = (MPARAM)TRUE;
        break;

        case BM_CLICK:
            ButtonClick(hwnd, mp1);
        break;

        case WM_DESTROY:
            ButtonDestroy(hwnd);
        break;

        default:
            mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ RegisterFlatButtonClass:
 *
 */

STATIC BOOL RegisterFlatButtonClass(HAB hab)
{
    return WinRegisterClass(hab,
                            WC_FLATBUTTON,
                            fnwpFlatButton,
                            CS_SIZEREDRAW | CS_SYNCPAINT, //  | CS_PARENTCLIP,
                            sizeof(PFLATBUTTONDATA));
}

/* ******************************************************************
 *
 *   CD player client window proc
 *
 ********************************************************************/

/*
 *@@ CDCreate:
 *      implementation for WM_CREATE.
 */

STATIC MRESULT CDCreate(HWND hwnd,
                        MPARAM mp1)
{
    MRESULT         mrc = (MRESULT)FALSE;
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)mp1;
    HMODULE         hmod = cmnQueryMainResModuleHandle();
    ULONG           ul = 0,
                    cxMiniIcon = G_cxIcon /* WinQuerySysValue(HWND_DESKTOP, SV_CXICON) */ / 2,
                    cxButton = 2 * BUTTON_BORDER
                             + 2 * BUTTON_ICONSPACING
                             + cxMiniIcon;
    LONG            x = 0;

    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);

    WinSetWindowPtr(hwnd, QWL_USER, mp1);

    pWinData->hab = WinQueryAnchorBlock(hwnd);

    pWinData->lcol3DDark = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0);
    pWinData->lcol3DLight = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0);

    x = 5;

    _Pmpf(("cxButton: %d", cxButton));

    for (ul = 0;
         ul < CDP_CICONS;
         ul++)
    {
        pWinData->ahptrButtons[ul] = WinLoadPointer(HWND_DESKTOP,
                                                    hmod,
                                                    G_aulButtonIDs[ul]);

        pWinData->ahwndButtons[ul]
            = WinCreateWindow(hwnd,         // parent
                              WC_FLATBUTTON,
                              "",
                              WS_VISIBLE,
                              x,
                              5,
                              cxButton,
                              cxButton,
                              hwnd,         // owner
                              HWND_TOP,
                              ul + 1000,    // id
                              NULL,
                              NULL);
        if (pWinData->ahwndButtons[ul])
            WinSendMsg(pWinData->ahwndButtons[ul],
                       WM_SETICON,
                       (MPARAM)pWinData->ahptrButtons[ul],
                       0);
        x += cxMiniIcon + 5;
    }

    pWinData->yTrackNTime = 5 + cxButton + 5;
    pWinData->cyTrackNTime = 20;        // overridden when presparam comes in
    pWinData->cx = x + 5;
    pWinData->cy = pWinData->yTrackNTime + pWinData->cyTrackNTime + 5;

    if (_pvPlayer)
    {
        // we already have a device:
        // enable position advise,
        // get current data
        PXMMCDPLAYER pPlayer = _pvPlayer;
        xmmCDPositionAdvise(pPlayer,
                            hwnd,           // client
                            CDM_POSITIONUPDATE);        // msg to post
        pWinData->ulTrack = pPlayer->ulTrack;
        pWinData->ulSeconds = pPlayer->ulSecondsInTrack;
    }

    return mrc;
}

/*
 *@@ CDAdjustWindowPos:
 *      implementation for WM_ADJUSTWINDOWPOS.
 */

STATIC VOID CDAdjustWindowPos(HWND hwnd,
                              PSWP pswp)
{
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    if (pWinData)
    {
        pswp->cx = pWinData->cx;
    }
}

/*
 *@@ PaintTrack:
 *
 */

STATIC VOID PaintTrack(PCDPLAYWINDATA pWinData,
                       HPS hps,
                       BOOL fDrawFrames)
{
    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
    RECTL   rcl;
    CHAR    szTrack[10];
    ULONG   flText = DT_CENTER | DT_VCENTER;

    ULONG   cx = pWinData->rclClient.xRight - pWinData->rclClient.xLeft;
       //      cy = pWinData->rclClient.yTop - pWinData->rclClient.yBottom;

    rcl.xLeft = pWinData->rclClient.xLeft;
    rcl.yBottom = pWinData->yTrackNTime;
    rcl.xRight = rcl.xLeft + (cx / 3) - 1;
    rcl.yTop = rcl.yBottom + pWinData->cyTrackNTime - 1;

    if (fDrawFrames)
        gpihDraw3DFrame(hps,
                        &rcl,
                        1,
                        pWinData->lcol3DDark,
                        pWinData->lcol3DLight);
    else
        flText |= DT_ERASERECT;

    rcl.xLeft++;
    rcl.yBottom++;
    sprintf(szTrack, "%d", pWinData->ulTrack);
    WinDrawText(hps,
                strlen(szTrack),
                szTrack,
                &rcl,
                _lcolForeground,
                _lcolBackground,
                flText);
}

/*
 *@@ PaintTime:
 *
 */

STATIC VOID PaintTime(PCDPLAYWINDATA pWinData,
                      HPS hps,
                      BOOL fDrawFrames)
{
    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
    RECTL   rcl;
    CHAR    szTime[10];
    ULONG   flText = DT_CENTER | DT_VCENTER;

    ULONG   cx = pWinData->rclClient.xRight - pWinData->rclClient.xLeft;
//             cy = pWinData->rclClient.yTop - pWinData->rclClient.yBottom;

    rcl.xLeft = pWinData->rclClient.xLeft + (cx / 3) + 5;
    rcl.yBottom = pWinData->yTrackNTime;
    rcl.xRight = pWinData->rclClient.xRight - 1;
    rcl.yTop = rcl.yBottom + pWinData->cyTrackNTime - 1;

    if (fDrawFrames)
        gpihDraw3DFrame(hps,
                        &rcl,
                        1,
                        pWinData->lcol3DDark,
                        pWinData->lcol3DLight);
    else
        flText |= DT_ERASERECT;

    rcl.xLeft++;
    rcl.yBottom++;
    sprintf(szTime, "%d:%02d",
            pWinData->ulSeconds / 60,
            pWinData->ulSeconds % 60);
    WinDrawText(hps,
                strlen(szTime),
                szTime,
                &rcl,
                _lcolForeground,
                _lcolBackground,
                flText);
}

/*
 *@@ CDPaint:
 *      implementation for WM_PAINT.
 */

STATIC VOID CDPaint(HWND hwnd)
{
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
    HPS             hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);

    if (hps)
    {
        RECTL   rclWin;
        // ULONG   ul = 0;

        gpihSwitchToRGB(hps);

        WinQueryWindowRect(hwnd,
                           &rclWin);        // exclusive

        // draw outer CD player frame
        rclWin.xRight--;
        rclWin.yTop--;
        gpihDraw3DFrame(hps,
                        &rclWin,
                        1,
                        pWinData->lcol3DLight,
                        pWinData->lcol3DDark);
        rclWin.xLeft++;
        rclWin.yBottom++;
        WinFillRect(hps,
                    &rclWin,
                    _lcolBackground);

        pWinData->rclClient.xLeft = rclWin.xLeft + 5;
        pWinData->rclClient.yBottom = rclWin.yBottom + 5;
        pWinData->rclClient.xRight = rclWin.xRight - 5;
        pWinData->rclClient.yTop = rclWin.yTop + 5;

        PaintTrack(pWinData,
                   hps,
                   TRUE);        // draw frames

        PaintTime(pWinData,
                  hps,
                  TRUE);        // draw frames

        WinEndPaint(hps);
    }
}

/*
 *@@ CDContextMenu:
 *      implementation for WM_CONTEXTMENU.
 */

STATIC MRESULT CDContextMenu(HWND hwnd, MPARAM mp1)
{
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
    POINTL          ptlPopup;
    // HPS             hps;

    ptlPopup.x = SHORT1FROMMP(mp1);
    ptlPopup.y = SHORT2FROMMP(mp1);

    // set flag for wpModifyPopupMenu so that it will
    // add the "close" menu item; there's no other way
    // to get this to work since we don't have a container
    // as our client... sigh
    _fShowingOpenViewMenu = TRUE;
            // this will be unset in wpModifyPopupMenu

    pWinData->hwndContextMenu
            = _wpDisplayMenu(pWinData->somSelf,
                             // owner: the frame
                             WinQueryWindow(hwnd, QW_OWNER),
                             // client: our client
                             hwnd,
                             &ptlPopup,
                             MENU_OPENVIEWPOPUP, //  | MENU_NODISPLAY,
                             0);

    return (MPARAM)TRUE;      // message processed
}

/*
 *@@ CDCommand:
 *      implementation for WM_COMMAND.
 */

STATIC VOID CDCommand(HWND hwnd, MPARAM mp1)
{
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);

    USHORT usCmd = (USHORT)mp1;

    switch (usCmd - 1000)
    {
        case CDBIDX_PREV:
            _xwpCDPrevTrack(pWinData->somSelf);
        break;

        case CDBIDX_PLAY:
            _xwpCDPlay(pWinData->somSelf);
        break;

        case CDBIDX_PAUSE:
            _xwpCDPause(pWinData->somSelf);
        break;

        case CDBIDX_STOP:
            _xwpCDStop(pWinData->somSelf);
        break;

        case CDBIDX_NEXT:
            _xwpCDNextTrack(pWinData->somSelf);
        break;

        case CDBIDX_EJECT:
            _xwpCDEject(pWinData->somSelf);
        break;

        default:
            if (usCmd == WPMENUID_CLOSE)
                // that's our own "close" menu item
                // from wpModifyPopupMenu
                WinPostMsg(hwnd, WM_CLOSE, 0, 0);
    }
}

/*
 *@@ CDPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 */

STATIC VOID CDPresParamChanged(HWND hwnd, MPARAM mp1)
{
    PCDPLAYWINDATA pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
    ULONG ulAttrChanged = (ULONG)mp1;
    BOOL fInvalidate = TRUE;
    // ULONG ul;

    switch (ulAttrChanged)
    {
        case 0:     // layout palette thing dropped
        case PP_BACKGROUNDCOLOR:
        case PP_FOREGROUNDCOLOR:
            _lcolBackground
                = winhQueryPresColor(hwnd,
                                     PP_BACKGROUNDCOLOR,
                                     FALSE,
                                     SYSCLR_DIALOGBACKGROUND);
            _lcolForeground
                = winhQueryPresColor(hwnd,
                                     PP_FOREGROUNDCOLOR,
                                     FALSE,
                                     SYSCLR_WINDOWSTATICTEXT);
        break;

        case PP_FONTNAMESIZE:
        {
            HPS hps;

            if (_pszFont)
            {
                free(_pszFont);
                _pszFont = NULL;
            }

            _pszFont = winhQueryWindowFont(hwnd);

            hps = WinGetPS(hwnd);
            pWinData->cyTrackNTime = gpihQueryLineSpacing(hps)
                                     + 2 * 1; // bordewr
            pWinData->cy = pWinData->yTrackNTime + pWinData->cyTrackNTime + 5;
            WinReleasePS(hwnd);

            // adjust size
            WinSetWindowPos(WinQueryWindow(hwnd, QW_PARENT),
                            NULLHANDLE,
                            0,
                            0,
                            pWinData->cx,
                            pWinData->cy,
                            SWP_SIZE);
        }
        break;

        default:
            fInvalidate = FALSE;
    }

    if (fInvalidate)
    {
        WinInvalidateRect(hwnd, NULL, FALSE);
        _wpSaveDeferred(pWinData->somSelf);
    }
}

/*
 *@@ CDPositionUpdate:
 *      implementation for MM_MCIPOSITIONCHANGE.
 */

STATIC VOID CDPositionUpdate(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);

    ULONG ulTrack = (ULONG)mp1;
    ULONG ulSeconds = (ULONG)mp2;

    HPS hps = WinGetPS(hwnd);
    gpihSwitchToRGB(hps);

    if (ulTrack != -1)
    {
        pWinData->ulTrack = ulTrack;
        PaintTrack(pWinData,
                   hps,
                   FALSE);
    }

    if (ulSeconds != -1)
    {
        pWinData->ulSeconds = ulSeconds;
        PaintTime(pWinData,
                  hps,
                  FALSE);
    }

    WinReleasePS(hps);
}

/*
 *@@ CDClose:
 *      implementation for WM_CLOSE.
 */

STATIC VOID CDClose(HWND hwnd)
{
    PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
    CHAR szIniKey[100];
    sprintf(szIniKey,
            "%s%lX",
            INIKEY_WNDPOSXMMCDPLAY,
            _wpQueryHandle(pWinData->somSelf));
    winhSaveWindowPos(WinQueryWindow(hwnd, QW_OWNER),   // the frame
                      HINI_USER,
                      INIAPP_XWORKPLACE,
                      szIniKey);
    // destroy the frame, which in turn destroys ourselves
    WinDestroyWindow(WinQueryWindow(hwnd, QW_PARENT));
}

/*
 *@@ fnwpCDPlayerClient:
 *      window proc for the CD player client.
 */

STATIC MRESULT EXPENTRY fnwpCDPlayerClient(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    TRY_LOUD(excpt1)
    {
        switch (msg)
        {
            case WM_CREATE:
                CDCreate(hwnd,
                         mp1);
            break;

            case WM_PAINT:
                CDPaint(hwnd);
            break;

            case WM_BEGINDRAG:
                WinSendMsg(WinQueryWindow(hwnd, QW_OWNER),  // the frame
                           WM_TRACKFRAME,
                           (MPARAM)TF_MOVE,
                           0);
            break;

            case WM_CONTEXTMENU:
                CDContextMenu(hwnd, mp1);
            break;

            case WM_COMMAND:
                CDCommand(hwnd, mp1);
            break;

            case WM_PRESPARAMCHANGED:
                CDPresParamChanged(hwnd, mp1);
            break;

            case CDM_POSITIONUPDATE:
                CDPositionUpdate(hwnd, mp1, mp2);
            break;

            case WM_CLOSE:
                CDClose(hwnd);
            break;

            case WM_DESTROY:
            {
                PCDPLAYWINDATA pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
                XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
                ULONG ul;

                if (_pvPlayer)
                    xmmCDPositionAdvise(_pvPlayer,
                                        NULLHANDLE,         // disable
                                        0);        // msg to post

                _hwndOpenView = NULLHANDLE;

                for (ul = 0;
                     ul < CDP_CICONS;
                     ul++)
                {
                    WinDestroyPointer(pWinData->ahptrButtons[ul]);
                }

                // remove this window from the object's use list
                _wpDeleteFromObjUseList(pWinData->somSelf,
                                        &pWinData->UseItem);
                _wpFreeMem(pWinData->somSelf, (PBYTE)pWinData);
                WinSetWindowPtr(hwnd, QWL_USER, 0);

                mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
            }
            break;

            default:
                mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
        }
    }
    CATCH(excpt1) {} END_CATCH();

    return mrc;
}

/* ******************************************************************
 *
 *   CD player frame window proc
 *
 ********************************************************************/

/*
 *@@ fnwpCDPlayerFrame:
 *      frame window proc. xmmCreateCDPlayerView subclasses
 *      the frame with this.
 */

STATIC MRESULT EXPENTRY fnwpCDPlayerFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_SYSCOMMAND:
         *      we must intercept this; since we don't have
         *      a system menu, closing from the task list
         *      won't work otherwise
         */

        case WM_SYSCOMMAND:
            switch ((USHORT)mp1)
            {
                case SC_CLOSE:
                    WinPostMsg(WinWindowFromID(hwnd, FID_CLIENT),
                               WM_CLOSE,
                               0, 0);
                break;
            }
        break;

        default:
        {
            PCDPLAYWINDATA pWinData = (PCDPLAYWINDATA)WinQueryWindowPtr(hwnd, QWL_USER);
            mrc = pWinData->pfnwpFrameOrig(hwnd, msg, mp1, mp2);
        }
    }

    return mrc;
}

/* ******************************************************************
 *
 *   CD player interface
 *
 ********************************************************************/

/*
 *@@ xmmCreateCDPlayerView:
 *      this gets called from XWPCDPlayer::wpOpen to
 *      create a new "CD player" view. The parameters
 *      are just passed on from wpOpen.
 *
 *      The volume window is a regular standard PM
 *      frame with a circular slider inside.
 */

HWND xmmCreateCDPlayerView(WPObject *somSelf,
                           HWND hwndCnr,
                           ULONG ulView)
{
    HWND            hwndFrame = 0;

    TRY_LOUD(excpt1)
    {
        HAB         hab = WinQueryAnchorBlock(hwndCnr);

        if (!G_fCDPlayerClientRegistered)
        {
            // first call: register client window class
            G_fCDPlayerClientRegistered
                = (    (WinRegisterClass(hab,
                                         WC_CDPLAYER_CLIENT,
                                         fnwpCDPlayerClient,
                                         CS_SIZEREDRAW
                                            | CS_CLIPCHILDREN
                                            | CS_PARENTCLIP,
                                         sizeof(PCDPLAYWINDATA)))
                    && (RegisterFlatButtonClass(hab))
                   );
        }

        if (G_fCDPlayerClientRegistered)
        {
            // allocate window data (use item and such)
            PCDPLAYWINDATA  pWinData = (PCDPLAYWINDATA)_wpAllocMem(somSelf,
                                                                   sizeof(CDPLAYWINDATA),
                                                                   0);

            if (pWinData)
            {
                HWND        hwndClient = NULLHANDLE;
                ULONG       flCreate;                      // Window creation flags
                SWP         swpFrame;

                memset(pWinData, 0, sizeof(*pWinData));

                pWinData->cbSize = sizeof(*pWinData);
                pWinData->somSelf = somSelf;

                // create the frame window
                flCreate = // FCF_SYSMENU
                            // | FCF_SIZEBORDER
                            // | FCF_TITLEBAR
                            // | FCF_MINMAX
                            // | FCF_TASKLIST
                            FCF_NOBYTEALIGN;

                swpFrame.x = 100;
                swpFrame.y = 100;
                swpFrame.cx = 500;
                swpFrame.cy = 200;
                swpFrame.hwndInsertBehind = HWND_TOP;
                swpFrame.fl = SWP_MOVE | SWP_SIZE | SWP_ZORDER;

                hwndFrame = winhCreateStdWindow(HWND_DESKTOP,           // frame parent
                                                &swpFrame,
                                                flCreate,
                                                WS_ANIMATE,
                                                _wpQueryTitle(somSelf), // title bar
                                                0,                      // res IDs
                                                WC_CDPLAYER_CLIENT,     // client class
                                                WS_VISIBLE,             // client wnd style:
                                                0,                      // frame ID
                                                pWinData,               // client control data
                                                &hwndClient);

                if ((hwndFrame) && (hwndClient))
                {
                    // frame and client successfully created:
                    XMMCDPlayerData *somThis = XMMCDPlayerGetData(pWinData->somSelf);
                    // PNLSSTRINGS     pNLSStrings = cmnQueryNLSStrings();
                    // view title: we remove "~" later
                    CHAR            szIniKey[100];

                    // add the use list item to the object's use list
                    cmnRegisterView(somSelf,
                                    &pWinData->UseItem,
                                    ulView,
                                    hwndFrame,
                                    cmnGetString(ID_XSSI_CDPLAYERVIEW));

                    _hwndOpenView = hwndClient;

                    // subclass frame
                    WinSetWindowPtr(hwndFrame, QWL_USER, pWinData);
                    pWinData->pfnwpFrameOrig = WinSubclassWindow(hwndFrame,
                                                                 fnwpCDPlayerFrame);

                    winhSetWindowFont(hwndClient,
                                      (_pszFont)
                                        ? _pszFont
                                        : cmnQueryDefaultFont());
                                // WM_PRESPARAMCHANGED resized the window

                    // try to set saved position for the frame;
                    // this does not move if we don't have a pos saved yet,
                    // but still shows the window
                    sprintf(szIniKey,
                            "%s%lX",
                            INIKEY_WNDPOSXMMCDPLAY,
                            _wpQueryHandle(somSelf));
                    winhRestoreWindowPos(hwndFrame,
                                         HINI_USER,
                                         INIAPP_XWORKPLACE,
                                         szIniKey,
                                         SWP_MOVE | SWP_SHOW | SWP_ACTIVATE);
                }
            }
        } // end if (G_fCDPlayerClientRegistered)
    }
    CATCH(excpt1)
    {
        hwndFrame = NULLHANDLE;
    } END_CATCH();

    return hwndFrame;
}


