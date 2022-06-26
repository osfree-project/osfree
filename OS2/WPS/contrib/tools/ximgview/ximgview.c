
/*
 * ximgview.c:
 *      primitive image viewer. Uses MMPM/2 to understand all its
 *      image file formats.
 *
 *      This is also a simple example of how to multithread a PM
 *      application. Loading the bitmap is passed to a second
 *      transient thread.
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

#define INCL_WIN
#define INCL_WINWORKPLACE
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_GPI                // required for INCL_MMIO_CODEC
#define INCL_GPIBITMAPS         // required for INCL_MMIO_CODEC
#include <os2.h>

// multimedia includes
#define INCL_MCIOS2
#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

#include <stdio.h>
#include <setjmp.h>

#include "setup.h"

#include "helpers\cnrh.h"
#include "helpers\eah.h"
#include "helpers\dosh.h"
#include "helpers\except.h"
#include "helpers\gpih.h"
#include "helpers\mmpmh.h"
#include "helpers\stringh.h"
#include "helpers\winh.h"
#include "helpers\threads.h"

#include "bldlevel.h"
#include "dlgids.h"

#include "ximgview.h"

/* ******************************************************************
 *
 *   Global variables for all threads
 *
 ********************************************************************/

CHAR        G_szFilename[CCHMAXPATH] = "";

/* ******************************************************************
 *
 *   Image handling
 *
 ********************************************************************/

/*
 *@@ QueryErrorDescription:
 *
 */

PSZ QueryErrorDescription(ULONG ulError)
{
    switch (ulError)
    {
        // case MMHERR_UNKNOWN_ERROR:
        case MMHERR_NO_IOPROC:
            return "Image file format not supported.";

        case MMHERR_NOT_IMAGE_FILE:
            return "File is non-image multimedia file.";

        case MMHERR_OPEN_FAILED:
            return "Open failed.";

        case MMHERR_INCOMPATIBLE_HEADER:
            return "Incompatible bitmap header size.";

        case MMHERR_GPIGREATEPS_FAILED:
            return "Error creating memory presentation space (GpiCreatePS failed).";

        case MMHERR_GPICREATEBITMAP_FAILED:
            return "Error creating PM bitmap (GpiCreateBitmap failed).";

        case MMHERR_GPISETBITMAPBITS_FAILED:
            return "GpiSetBitmapBits failed.";

        case ERROR_NOT_ENOUGH_MEMORY:
            return "Out of memory.";

        case ERROR_PROTECTION_VIOLATION:
            return "Crashed reading bitmap.";

        case MMHERR_MMIOREAD_FAILED:
            return "Error reading bitmap data.";
    }

    return ("Unknown error.");
}

/*
 *@@ fntLoadBitmap:
 *      transient thread created by WMCommand_FileOpen
 *      to load the bitmap file.
 *
 *      Posts WM_DONELOADINGBMP to the client when done.
 */

void _Optlink fntLoadBitmap(PTHREADINFO pti)
{
    // create a memory PS
    HWND        hwndClient = (HWND)(pti->ulData);
    HBITMAP     hbmNew;
    APIRET      arc;

    SIZEL       szl = {100, 100};

    arc = mmhLoadImage(G_szFilename,
                       0,
                       NULL, // &szl,
                       &hbmNew);

    WinPostMsg(hwndClient,
               WM_DONELOADINGBMP,
               (MPARAM)arc,
               (MPARAM)hbmNew);
}

/* ******************************************************************
 *
 *   Global variables for main thread
 *
 ********************************************************************/

HAB         G_habMain = NULLHANDLE;
HMQ         G_hmqMain = NULLHANDLE;
HWND        G_hwndMain = NULLHANDLE;
HBITMAP     G_hbmLoaded = NULLHANDLE;

PFNWP       G_pfnwpFrameOrig = NULL;

BOOL        G_fStartingUp = FALSE;
BOOL        G_fUseLastOpenDir = FALSE;

// load-bitmap thread
BOOL        G_fLoadingBitmap = FALSE;
THREADINFO  G_tiLoadBitmap = {0};

// scroll bar data

SCROLLABLEWINDOW    G_scrwClient;

BITMAPINFOHEADER G_bmihLoadedBitmap;
// ULONG       G_ulVertScrollOfs,
//             G_ulHorzScrollOfs;

#define VERT_SCROLL_UNIT 10
#define HORZ_SCROLL_UNIT 10

/*
 *@@ G_GlobalSettings:
 *      global settings.
 */

struct
{
    BOOL    fResizeAfterLoad,
            fConstrain2Screen,
            fScale2WinSize;
} G_GlobalSettings;

/* ******************************************************************
 *
 *   Command handler
 *
 ********************************************************************/

VOID UpdateTitle(VOID)
{
    CHAR szTitle[500] = "ximgview";

    if (G_hbmLoaded)
    {
        CHAR szName[CCHMAXPATH] = "",
             szExt[CCHMAXPATH] = "";
        _splitpath(G_szFilename, NULL, NULL, szName, szExt);
        sprintf(szTitle + strlen(szTitle),
                " - %s%s (%dx%d, %d bpp)",
                szName, szExt,
                G_bmihLoadedBitmap.cx,
                G_bmihLoadedBitmap.cy,
                G_bmihLoadedBitmap.cBitCount);

    }

    WinSetWindowText(G_hwndMain, szTitle);
}

/*
 *@@ CalcViewportSize:
 *
 */

VOID CalcViewportSize(HWND hwndClient,
                      SIZEL *pszl)
{
    SWP swpClient;
    WinQueryWindowPos(hwndClient, &swpClient);
            // this already has the scroll bars subtracted;
            // apparently, these are frame controls...
    pszl->cx = swpClient.cx;
    pszl->cy = swpClient.cy;
}

/*
 *@@ UpdateScrollBars:
 *
 */

VOID UpdateScrollBars(HWND hwndClient)
{
    HWND hwndVScroll = WinWindowFromID(G_hwndMain, FID_VERTSCROLL);
    HWND hwndHScroll = WinWindowFromID(G_hwndMain, FID_HORZSCROLL);

    if (G_GlobalSettings.fScale2WinSize)
    {
        WinEnableWindow(hwndVScroll, FALSE);
        WinEnableWindow(hwndHScroll, FALSE);
    }
    else
    {
        SIZEL szlViewport;
        CalcViewportSize(hwndClient, &szlViewport);

        // vertical (height)
        winhUpdateScrollBar(hwndVScroll,
                            szlViewport.cy,
                            G_scrwClient.szlWorkarea.cy,
                            G_scrwClient.ptlScrollOfs.y,
                            FALSE);      // auto-hide

        // horizontal (width)
        winhUpdateScrollBar(hwndHScroll,
                            szlViewport.cx,
                            G_scrwClient.szlWorkarea.cx,
                            G_scrwClient.ptlScrollOfs.x,
                            FALSE);      // auto-hide
    }
}

/*
 *@@ UpdateMenuItems:
 *
 */

VOID UpdateMenuItems(VOID)
{
    HWND    hmenuMain = WinWindowFromID(G_hwndMain, FID_MENU);

    // file menu
    WinEnableMenuItem(hmenuMain, IDM_FILE,
                      // disable file menu if load-bitmap thread is running
                      !G_fLoadingBitmap);

    // view menu
    WinEnableMenuItem(hmenuMain, IDM_VIEW,
                      // disable view menu if load-bitmap thread is running
                      // or if we have no bitmap yet
                      (     (!G_fLoadingBitmap)
                        && (G_hbmLoaded != NULLHANDLE)
                      ));
    // options menu
    WinCheckMenuItem(hmenuMain, IDMI_OPT_RESIZEAFTERLOAD,
                     G_GlobalSettings.fResizeAfterLoad);
    WinCheckMenuItem(hmenuMain, IDMI_OPT_CONSTRAIN2SCREEEN,
                     G_GlobalSettings.fConstrain2Screen);
    WinCheckMenuItem(hmenuMain, IDMI_OPT_SCALE2WINSIZE,
                     G_GlobalSettings.fScale2WinSize);
}

/*
 *@@ SetWinSize2BmpSize:
 *
 */

VOID SetWinSize2BmpSize(VOID)
{
    SWP swpOld;
    RECTL rcl;

    LONG   lNewCX,
           lNewCY,
           lNewY;

    // calculate new frame size needed:
    // WinCalcFrameRect really wants screen
    // coordinates, but we don't care here
    rcl.xLeft = 0;
    rcl.xRight = rcl.xLeft + G_bmihLoadedBitmap.cx;
    rcl.yBottom = 0;
    rcl.yTop = rcl.yBottom + G_bmihLoadedBitmap.cy;
    WinCalcFrameRect(G_hwndMain,
                     &rcl,
                     FALSE);    // calc frame from client

    // new frame window size:
    WinQueryWindowPos(G_hwndMain,
                      &swpOld);
    lNewCX = rcl.xRight - rcl.xLeft;
    lNewCY = rcl.yTop - rcl.yBottom;

    lNewY = swpOld.y
                  // calculate difference between old
                  // and new cy:
                  // if the new bitmap is larger, we
                  // need to move y DOWN
                  + swpOld.cy
                  - lNewCY;

    // limit window size if off screen
    if (G_GlobalSettings.fConstrain2Screen)
    {
        if ((swpOld.x + lNewCX) > G_cxScreen)
            lNewCX = G_cxScreen - swpOld.x;
        if (lNewY < 0)
        {
            lNewY = 0;
            lNewCY = swpOld.y + swpOld.cy;
        }
    }

    // now move/resize window:
    // make the top left corner constant
    WinSetWindowPos(G_hwndMain,
                    HWND_TOP,
                    // pos
                    swpOld.x,         // don't change
                    lNewY,
                    // width
                    lNewCX,
                    lNewCY,
                    SWP_MOVE | SWP_SIZE
                        // in case the bitmap is loaded
                        // as a startup parameter:
                        | SWP_SHOW | SWP_ACTIVATE);
}

/*
 *@@ WMCommand_FileOpen:
 *
 */

BOOL WMCommand_FileOpen(HWND hwndClient)
{
    BOOL        brc = FALSE;

    CHAR        szFile[CCHMAXPATH] = "*";

    ULONG       flFlags = WINH_FOD_INISAVEDIR;

    if (G_fUseLastOpenDir)
        // this is FALSE if we were loaded with a
        // image file on the cmd line; in that case,
        // use the startup directory
        flFlags |= WINH_FOD_INILOADDIR;

    if (winhFileDlg(hwndClient,
                    szFile,       // in: file mask; out: fully q'd filename
                    flFlags,
                    HINI_USER,
                    INIAPP,
                    INIKEY_LASTDIR))
    {
        strcpy(G_szFilename, szFile);
        thrCreate(&G_tiLoadBitmap,
                  fntLoadBitmap,
                  &G_fLoadingBitmap,        // running flag
                  "LoadBitmap",
                  THRF_PMMSGQUEUE | THRF_WAIT,
                  hwndClient);      // user param
        UpdateMenuItems();

        // for subsequent opens, load from INI
        G_fUseLastOpenDir = TRUE;
    }

    return (brc);
}

/*
 *@@ WMDoneLoadingBitmap:
 *      handler for WM_DONELOADINGBITMAP when
 *      load-bitmap thread is done. ulError
 *      has the error code (0 if none).
 */

VOID WMDoneLoadingBitmap(HWND hwndClient,
                         ULONG ulError,
                         HBITMAP hbmNew)
{
    if (ulError)
    {
        CHAR szMsg[1000];
        sprintf(szMsg,
                "Error %d loading \"%s\": %s",
                ulError,
                G_szFilename,
                QueryErrorDescription(ulError));

        if (G_fStartingUp)
            // first call:
            WinSetWindowPos(G_hwndMain, 0, 0,0,0,0, SWP_SHOW | SWP_ACTIVATE);
        winhDebugBox(hwndClient, "Error", szMsg);
    }
    else
    {
        // free old bitmap
        if (G_hbmLoaded)
            GpiDeleteBitmap(G_hbmLoaded);

        G_hbmLoaded = hbmNew;

        GpiQueryBitmapParameters(G_hbmLoaded,
                                 &G_bmihLoadedBitmap);

        // reset scroller
        G_scrwClient.szlWorkarea.cx = G_bmihLoadedBitmap.cx;
        G_scrwClient.szlWorkarea.cy = G_bmihLoadedBitmap.cy;
        G_scrwClient.ptlScrollOfs.x = 0;
        G_scrwClient.ptlScrollOfs.y = 0;

        if (G_GlobalSettings.fResizeAfterLoad)
            SetWinSize2BmpSize();

        UpdateScrollBars(hwndClient);
        WinInvalidateRect(hwndClient, NULL, FALSE);
    }

    // wait till load-bitmap thread has really exited
    thrWait(&G_tiLoadBitmap);

    UpdateTitle();
    UpdateMenuItems();

    G_fStartingUp = FALSE;
}

/*
 *@@ WMClose_CanClose:
 *      if this returns TRUE, the main window is
 *      closed and the application is terminated.
 *      Prompt for saving data here.
 */

BOOL WMClose_CanClose(HWND hwndClient)
{
    return (TRUE);
}

/* ******************************************************************
 *
 *   Main window
 *
 ********************************************************************/

/*
 *@@ PaintClient:
 *
 */

VOID PaintClient(HWND hwndClient,
                 HPS hps,
                 PRECTL prclPaint)
{
    BOOL    fFillBackground = TRUE;
    SIZEL   szlViewport;

    CalcViewportSize(hwndClient, &szlViewport);

    if (G_hbmLoaded)
    {
        POINTL ptlDest = {0, 0};
        RECTL rclDest;

        if (G_GlobalSettings.fScale2WinSize)
        {
            // "scale-to-window" mode: that's easy
            RECTL rclClient;
            WinQueryWindowRect(hwndClient, &rclClient);
            WinDrawBitmap(hps,
                          G_hbmLoaded,
                          NULL,     // all
                          (PPOINTL)&rclClient, // in stretch mode, this is
                                               // used for the rectangle
                          CLR_BLACK,    // in case we have a b/w bmp
                          CLR_WHITE,    // in case we have a b/w bmp
                          DBM_NORMAL | DBM_STRETCH);
            fFillBackground = FALSE;
        }
        else
        {
            // original size mode:
            if (G_scrwClient.szlWorkarea.cy < szlViewport.cy)
                // center vertically:
                ptlDest.y = (szlViewport.cy - G_scrwClient.szlWorkarea.cy) / 2;
            else
            {
                // use scroller offset:
                // calc leftover space
                ULONG ulClipped = G_scrwClient.szlWorkarea.cy - szlViewport.cy;
                // this is a positive value if the scroller is
                // down from the top or 0 if its at the top.
                // So if it's zero, we must use -ulClipped.
                ptlDest.y -= ulClipped;
                // if it's above zero, we must add to y.
                ptlDest.y += G_scrwClient.ptlScrollOfs.y;
            }

            if (G_scrwClient.szlWorkarea.cx < szlViewport.cx)
                // center horizontally:
                ptlDest.x = (szlViewport.cx - G_scrwClient.szlWorkarea.cx) / 2;
            else
            {
                // use scroller offset:
                // calc leftover space
                ULONG ulClipped = G_scrwClient.szlWorkarea.cx - szlViewport.cx;
                // this is a positive value if the scroller is
                // right from the left or 0 if its at the left.
                // So if it's zero, we must use -ulClipped.
                // ptlDest.x -= ulClipped;
                // if it's above zero, we must further subtract from x.
                ptlDest.x -= G_scrwClient.ptlScrollOfs.x;
            }

            WinDrawBitmap(hps,
                          G_hbmLoaded,
                          NULL,        // subrectangle to be drawn
                          &ptlDest,
                          0, 0,
                          DBM_NORMAL);

            {
                RECTL   rcl;
                CHAR    szTemp[400],
                        szNLS1[20],
                        szNLS2[20],
                        szNLS3[20],
                        szNLS4[20],
                        szNLS5[20],
                        szNLS6[20],
                        szNLS7[20],
                        szNLS8[20];
                FONTMETRICS fm;

                SIZEL szlRange;
                szlRange.cx = SHORT2FROMMR(WinSendMsg(G_scrwClient.hwndHScroll,
                                                      SBM_QUERYRANGE,
                                                      0,
                                                      0));
                szlRange.cy = SHORT2FROMMR(WinSendMsg(G_scrwClient.hwndVScroll,
                                                      SBM_QUERYRANGE,
                                                      0,
                                                      0));

                sprintf(szTemp,
                        "Workarea: (%s/%s), viewport: (%s/%s)\nscrOfs (%s/%s), scrLimit (%s/%s)",
                        nlsThousandsULong(szNLS1,
                                          G_scrwClient.szlWorkarea.cx,
                                          '.'),
                        nlsThousandsULong(szNLS2,
                                          G_scrwClient.szlWorkarea.cy,
                                          '.'),
                        nlsThousandsULong(szNLS3,
                                          szlViewport.cx,
                                          '.'),
                        nlsThousandsULong(szNLS4,
                                          szlViewport.cy,
                                          '.'),
                        nlsThousandsULong(szNLS5,
                                          G_scrwClient.ptlScrollOfs.x,
                                          '.'),
                        nlsThousandsULong(szNLS6,
                                          G_scrwClient.ptlScrollOfs.y,
                                          '.'),
                        nlsThousandsULong(szNLS7,
                                          szlRange.cx,
                                          '.'),
                        nlsThousandsULong(szNLS8,
                                          szlRange.cy,
                                          '.'));

                rcl.xLeft = 10;
                rcl.xRight = 1000;
                rcl.yBottom = 10;
                rcl.yTop = 1000;
                GpiQueryFontMetrics(hps, sizeof(fm), &fm);
                GpiSetColor(hps, CLR_WHITE);
                gpihDrawString(hps,
                               szTemp,
                               &rcl,
                               DT_LEFT | DT_BOTTOM,
                               &fm);
            }

            // cut out the rectangle of the bitmap we just
            // painted from the clipping region so that
            // WinFillRect below cannot overwrite it
            rclDest.xLeft = ptlDest.x;
            rclDest.yBottom = ptlDest.y;
            rclDest.xRight = ptlDest.x + G_scrwClient.szlWorkarea.cx;
            rclDest.yTop = ptlDest.y + G_scrwClient.szlWorkarea.cy;
            GpiExcludeClipRectangle(hps,
                                    &rclDest);
        }
    }

    if (fFillBackground)
        // fill the remainder white
        WinFillRect(hps, prclPaint, CLR_WHITE);
}

/*
 *@@ fnwpMainClient:
 *
 */

MRESULT EXPENTRY fnwpMainClient(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
        {
            USHORT usCmd = SHORT1FROMMP(mp1);
            switch (usCmd)
            {
                case IDMI_FILE_OPEN:
                    WMCommand_FileOpen(hwndClient);
                break;

                case IDMI_FILE_EXIT:
                    WinPostMsg(G_hwndMain, WM_CLOSE, 0, 0);
                break;

                case IDMI_VIEW_SIZEORIG:
                    SetWinSize2BmpSize();
                break;

                case IDMI_OPT_RESIZEAFTERLOAD:
                    G_GlobalSettings.fResizeAfterLoad = !G_GlobalSettings.fResizeAfterLoad;
                    UpdateMenuItems();
                break;

                case IDMI_OPT_CONSTRAIN2SCREEEN:
                    G_GlobalSettings.fConstrain2Screen = !G_GlobalSettings.fConstrain2Screen;
                    UpdateMenuItems();
                break;

                case IDMI_OPT_SCALE2WINSIZE:
                    G_GlobalSettings.fScale2WinSize = !G_GlobalSettings.fScale2WinSize;
                    UpdateMenuItems();
                    UpdateScrollBars(hwndClient);
                    WinInvalidateRect(hwndClient, NULL, FALSE);
                break;
            }
        break; }

        /*
         *@@ WM_DONELOADINGBMP:
         *      load-bitmap thread is done.
         *      Parameters:
         *      -- ULONG mp1: error code.
         *      -- HBITMAP mp2: new bitmap handle.
         */

        case WM_DONELOADINGBMP:
            WMDoneLoadingBitmap(hwndClient,
                                (ULONG)mp1,
                                (HBITMAP)mp2);
        break;

        /*
         * WM_SIZE:
         *
         */

        case WM_SIZE:
            UpdateScrollBars(hwndClient);
            WinInvalidateRect(hwndClient, NULL, FALSE);
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
        {
            RECTL rclPaint;
            HPS hps = WinBeginPaint(hwndClient,
                                    NULLHANDLE,
                                    &rclPaint);
            PaintClient(hwndClient,
                        hps,
                        &rclPaint);
            WinEndPaint(hps);
        break; }

        case WM_VSCROLL:
        case WM_HSCROLL:
        case WM_CHAR:
        {
            SIZEL szlViewport;
            CalcViewportSize(hwndClient, &szlViewport);
            winhHandleScrollerMsgs(hwndClient,
                                   &G_scrwClient,
                                   &szlViewport,
                                   msg,
                                   mp1,
                                   mp2);
        }
        break;

        case WM_CLOSE:
            if (WMClose_CanClose(hwndClient))
                winhSaveWindowPos(G_hwndMain,
                                  HINI_USER,
                                  INIAPP,
                                  INIKEY_MAINWINPOS);
            WinPostMsg(hwndClient, WM_QUIT, 0, 0);
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }
    return (mrc);
}

/*
 *@@ fnwpMainFrame:
 *
 */

MRESULT EXPENTRY fnwpMainFrame(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_QUERYTRACKINFO:
        {
            PTRACKINFO pti = (PTRACKINFO)mp2;

            G_pfnwpFrameOrig(hwndFrame, msg, mp1, mp2);

            if (pti->ptlMinTrackSize.x < 100)
                pti->ptlMinTrackSize.x = 100;
            if (pti->ptlMinTrackSize.y < 100)
                pti->ptlMinTrackSize.y = 100;
            mrc = (MRESULT)TRUE;
        break; }

        default:
            mrc = G_pfnwpFrameOrig(hwndFrame, msg, mp1, mp2);
    }

    return (mrc);
}

/*
 * main:
 *      program entry point.
 */

int main(int argc,
         char *argv[])
{
    QMSG        qmsg;
    HWND        hwndClient;
    ULONG       cbGlobalSettings = sizeof(G_GlobalSettings);
    ULONG       flSwpFlags;

    G_fStartingUp = TRUE;

    if (!(G_habMain = WinInitialize(0)))
        return FALSE;

    if (!(G_hmqMain = WinCreateMsgQueue(G_habMain, 0)))
        return FALSE;

    winhInitGlobals();      // V1.0.1 (2002-11-30) [umoeller]

    mmhInit();              // V1.0.1 (2003-01-25) [umoeller]

    // initialize global settings
    memset(&G_GlobalSettings, 0, sizeof(G_GlobalSettings));
    PrfQueryProfileData(HINI_USER,
                        INIAPP,
                        INIKEY_SETTINGS,
                        &G_GlobalSettings,
                        &cbGlobalSettings);

    WinRegisterClass(G_habMain,
                     WC_MY_CLIENT_CLASS,
                     fnwpMainClient,
                     CS_SIZEREDRAW,
                     sizeof(ULONG));

    G_hwndMain = winhCreateStdWindow(HWND_DESKTOP,
                                     0,
                                     FCF_TITLEBAR
                                        | FCF_SYSMENU
                                        | FCF_MINMAX
                                        | FCF_VERTSCROLL
                                        | FCF_HORZSCROLL
                                        | FCF_SIZEBORDER
                                        | FCF_ICON
                                        | FCF_MENU
                                        | FCF_TASKLIST,
                                     0, // WS_VISIBLE,
                                     "Title",
                                     1,     // icon resource
                                     WC_MY_CLIENT_CLASS,
                                     WS_VISIBLE,
                                     0,
                                     NULL,
                                     &hwndClient);

    G_scrwClient.hwndVScroll = WinWindowFromID(G_hwndMain, FID_VERTSCROLL);
    G_scrwClient.hwndHScroll = WinWindowFromID(G_hwndMain, FID_HORZSCROLL);

    G_pfnwpFrameOrig = WinSubclassWindow(G_hwndMain,
                                         fnwpMainFrame);

    // standard SWP flags
    flSwpFlags = SWP_MOVE | SWP_SIZE;

    // parse args
    if (argc > 1)
    {
        HDC hdcMem;
        HBITMAP hbmNew;
        HWND hwndShape;
        ULONG ulrc;
        strcpy(G_szFilename, argv[1]);
        thrCreate(&G_tiLoadBitmap,
                  fntLoadBitmap,
                  &G_fLoadingBitmap,        // running flag
                  "LoadBitmap",
                  THRF_PMMSGQUEUE | THRF_WAIT,
                  hwndClient);      // user param == client window
    }
    else
    {
        // no parameter:
        // show window initially
        flSwpFlags |= SWP_SHOW | SWP_ACTIVATE;
        // and retrieve last open dir from OS2.INI
        G_fUseLastOpenDir = TRUE;
    }

    UpdateTitle();
    UpdateMenuItems();

    if (!winhRestoreWindowPos(G_hwndMain,
                              HINI_USER,
                              INIAPP,
                              INIKEY_MAINWINPOS,
                              flSwpFlags))
        // standard pos
        WinSetWindowPos(G_hwndMain,
                        HWND_TOP,
                        10, 10, 500, 500,
                        flSwpFlags);

    //  standard PM message loop
    while (WinGetMsg(G_habMain, &qmsg, NULLHANDLE, 0, 0))
    {
        WinDispatchMsg(G_habMain, &qmsg);
    }

    // initialize global settings
    PrfWriteProfileData(HINI_USER,
                        INIAPP,
                        INIKEY_SETTINGS,
                        &G_GlobalSettings,
                        sizeof(G_GlobalSettings));

    // clean up on the way out
    WinDestroyMsgQueue(G_hmqMain);
    WinTerminate(G_habMain);

    return TRUE;
}


