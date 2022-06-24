
/*
 *@@sourcefile pg_control.c:
 *      XPager Desktop control window.
 *
 *      The XPager was originally derived from PageMage,
 *      a program written by Carlos Ugarte, which was
 *      eventually released as open source under the
 *      GPL. PageMage implemented its own system hook.
 *      while the PageMage window and the move handling
 *      were in a separate PM program.
 *
 *      Since XWorkplace implementation had very little
 *      in common with PageMage any more, it has been
 *      renamed to "XPager" with 0.9.18. With V0.9.19,
 *      the remaining parts have been rewritten, so this
 *      is no longer based on PageMage.
 *
 *      Basically, XPager consists of the following
 *      components:
 *
 *      --  The XPager control window (the pager).
 *          That's this file. The pager paints the
 *          representation of all virtual desktops in its
 *          client window.
 *
 *          Starting with V0.9.19, the pager is a client
 *          of the daemon window list. When the pager is
 *          started, it sends XDM_ADDWINLISTWATCH to
 *          the daemon object window (fnwpDaemonObject)
 *          to receive notifications when any desktop
 *          windows change. This allows us to track
 *          window creation, destruction, renames, and
 *          moves in the pager window.
 *
 *          The code for this is in pg_winlist.c and is
 *          now shared with the XCenter window list
 *          widget, which is another client of the
 *          daemon window list.
 *
 *      --  The XPager "move thread", which is a second
 *          thread which gets started when the pager is
 *          created. This is in pg_move.c.
 *
 *          This thread is reponsible for switching desktops.
 *          Switching desktops is actually done by moving all
 *          windows (except the sticky ones). So when the user
 *          switches one desktop to the right, all windows are
 *          actually moved to the left by the size of the PM screen.
 *
 *      --  As with the rest of the daemon, XPager receives
 *          notifications from XFLDR.DLL when its settings
 *          have been modified in the "Screen" settings notebook.
 */

/*
 *      Copyright (C) 2000-2008 Ulrich M”ller.
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

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSEXCEPTIONS
#define INCL_DOSMISC
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINSYS
#define INCL_WINSHELLDATA
#define INCL_WINTIMER
#define INCL_WINTRACKRECT
#define INCL_WINSWITCHLIST
#define INCL_WINPOINTERS
#define INCL_WINSTDCNR
#define INCL_WINRECTANGLES

#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#define INCL_GPILCIDS
#include <os2.h>

#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#define DONT_REPLACE_FOR_DBCS
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"                      // code generation and debugging options

#include "helpers\except.h"             // exception handling
#include "helpers\gpih.h"
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"
#include "helpers\threads.h"

#include "xwpapi.h"                     // public XWorkplace definitions
#include "shared\kernel.h"              // XWorkplace Kernel

#include "hook\xwphook.h"
#include "hook\hook_private.h"
#include "hook\xwpdaemn.h"              // XPager and daemon declarations

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

/*
 *@@ PAGERWINDATA:
 *      pager heap data stored in QWL_USER of
 *      the pager window. Created by pgrCreatePager.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

typedef struct _PAGERWINDATA
{
    USHORT      cb;                 // required for WM_CREATE

    PFNWP       pfnwpOrigFrame;

    SIZEL       szlClient;          // current client size

    PXBITMAP    pbmClient;          // client XBITMAP
    HBITMAP     hbmTemplate;        // "empty" bitmap with background and lines
                                    // used whenever client needs refresh for speed

    FATTRS      fattr;              // font attrs caught in presparamschanged
                                    // so we can quickly create a font for
                                    // the bitmap's memory PS

    BOOL        fNeedsRefresh;

} PAGERWINDATA, *PPAGERWINDATA;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static PCSZ         WC_PAGER = "XWPXPagerClient";

HMTX                G_hmtxSuppressNotify = NULLHANDLE;    // V0.9.14 (2001-08-25) [umoeller]

THREADINFO          G_tiMoveThread = {0};

BOOL                G_fActiveChangedPending = FALSE;      // V0.9.19 (2002-06-14) [lafaix]
BOOL                G_fDraggingCtrlWin = FALSE;  // V1.0.8 (2008-01-08) [pr]

extern HAB          G_habDaemon;        // xwpdaemn.c

/* ******************************************************************
 *
 *   XPager hook serialization
 *
 ********************************************************************/

/*
 *@@ pgrLockHook:
 *      increases cSuppressWinlistNotify in the global
 *      hook data to prevent the send-msg hook from
 *      intercepting window messages.
 *
 *      This is required whenever the daemon forces
 *      moving messages, e.g. because of screen
 *      switches, because otherwise we'd recurse
 *      into the hook.
 *
 *      Never modify the cSuppressWinlistNotify flag
 *      directly. The flag is protected by a mutex
 *      and supports recursive sets and clears now,
 *      so always use this function.
 *
 *      Call pgrUnlockHook to clear the flag
 *      again, which will release the mutex too.
 *
 *@@added V0.9.14 (2001-08-25) [umoeller]
 *@@changed V0.9.19 (2002-05-07) [umoeller]: now allowing recursive calls on the same thread
 */

BOOL pgrLockHook(PCSZ pcszFile, ULONG ulLine, PCSZ pcszFunction)
{
    if (!G_hmtxSuppressNotify)
    {
        // first call:
        DosCreateMutexSem(NULL, &G_hmtxSuppressNotify, 0, FALSE);
    }

    if (!DosRequestMutexSem(G_hmtxSuppressNotify, 4000))
    {
        ++(G_pHookData->cSuppressWinlistNotify);
                // V0.9.19 (2002-05-07) [umoeller]
        return TRUE;
    }

    DosBeep(100, 1000);
    return FALSE;

    /*
    if (    (    (G_hmtxSuppressNotify)
              && (!DosRequestMutexSem(G_hmtxSuppressNotify, 4000))
            )
         || (!DosCreateMutexSem(NULL,
                                &G_hmtxSuppressNotify,
                                0,
                                TRUE))
       )
    {
        ++(G_pHookData->cSuppressWinlistNotify);
                    // V0.9.19 (2002-05-07) [umoeller]
        return TRUE;
    }

    return FALSE;
    */
}

/*
 *@@ pgrUnlockHook:
 *
 *@@added V0.9.14 (2001-08-25) [umoeller]
 */

VOID pgrUnlockHook(VOID)
{
    if (G_pHookData->cSuppressWinlistNotify)
    {
        --(G_pHookData->cSuppressWinlistNotify);
        DosReleaseMutexSem(G_hmtxSuppressNotify);
    }
}

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 *@@ pgrCalcClientCY:
 *      calculates the new client height according to
 *      the client width, so that the frame can be
 *      sized proportionally.
 *
 *@@added V0.9.2 (2000-02-23) [umoeller]
 *@@changed V0.9.3 (2000-04-09) [umoeller]: now taking titlebar setting into account
 *@@changed V0.9.19 (2002-06-08) [umoeller]: rewritten
 */

LONG pgrCalcClientCY(LONG cx)
{
    return (   cx
             * G_pHookData->PagerConfig.cDesktopsY
             * G_pHookData->cyScreen
             / G_pHookData->PagerConfig.cDesktopsX
             / G_pHookData->cxScreen
          );
}

/*
 *@@ CheckFlashTimer:
 *      shortcut to start the flash timer if
 *      flashing is enabled.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 *@@changed V1.0.8 (2008-01-08) [pr]: keep pager window visible if mouse over it @@fixes 1038
 */

STATIC VOID CheckFlashTimer(VOID)
{
    BOOL   fMouseInWin = FALSE;
    POINTL ptlMouse;

    WinQueryPointerPos (HWND_DESKTOP, &ptlMouse);
    if (WinMapWindowPoints (HWND_DESKTOP, G_pHookData->hwndPagerFrame, &ptlMouse, 1))
    {
        RECTL rect;

        WinQueryWindowRect (G_pHookData->hwndPagerFrame, &rect);
        fMouseInWin = WinPtInRect (WinQueryAnchorBlock(HWND_DESKTOP), &rect, &ptlMouse);
    }

    if (   !fMouseInWin
        && !G_fDraggingCtrlWin
        && (G_pHookData->PagerConfig.flPager & PGRFL_FLASHTOTOP)
       )
        WinStartTimer(G_habDaemon,
                      G_pHookData->hwndPagerClient,
                      TIMERID_PGR_FLASH,
                      G_pHookData->PagerConfig.ulFlashDelay);
}

/*
 *@@ pgrIsShowing:
 *      returns TRUE if the specified window is at least
 *      partially visible on the current desktp.
 *
 *@@added V0.9.7 (2001-01-21) [umoeller]
 */

BOOL pgrIsShowing(PSWP pswp)
{
    // this was rewritten V0.9.7 (2001-01-18) [umoeller]
    LONG        bx = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
    LONG        by = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);

    return (     !(    // is right edge too wide to the left?
                       ((pswp->x + bx) >= G_pHookData->szlEachDesktopFaked.cx)
                    || ((pswp->x + pswp->cx - bx) <= 0)
                    || ((pswp->y + by) >= G_pHookData->szlEachDesktopFaked.cy)
                    || ((pswp->y + pswp->cy - by) <= 0)
                  )
            );
}

/*
 *@@ pgrRecoverWindows:
 *      recovers all windows (hopefully).
 *
 *      Gets called from several locations:
 *
 *      --  from dmnKillXPager because then we MUST
 *          recover, since the pager is dying and the
 *          user will be unable to retrieve windows back.
 *
 *      --  when XDM_RECOVERWINDOWS comes into
 *          fnwpDaemonObject. Most importantly, this
 *          gets posted from XShutdown before it starts
 *          closing windows so that window positions are
 *          not saved off-screen.
 *
 *          In the case of restart WPS, we now recover
 *          WPS windows only. V0.9.20 (2002-08-10) [umoeller]
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.19 (2002-06-02) [umoeller]: rewritten to use linklist
 *@@changed V0.9.19 (2002-06-08) [umoeller]: no longer recovering XCenters
 *@@changed V0.9.20 (2002-08-10) [umoeller]: added fWPSOnly for better restart wps support
 */

VOID pgrRecoverWindows(HAB hab,
                       BOOL fWPSOnly)
{
    LINKLIST    llSWPs;
    HENUM       henum;
    HWND        hwnd;

    ULONG       cWindows;
    PSWP        paswp;
    PID         pid;

    lstInit(&llSWPs,
            TRUE);      // auto-free

    // _PmpfF(("fWPSOnly is %d", fWPSOnly));

    henum = WinBeginEnumWindows(HWND_DESKTOP);
    while ((hwnd = WinGetNextWindow(henum)))
    {
        CHAR    szClassName[30];
        SWP     swp;
        if (    (hwnd != G_pHookData->hwndWPSDesktop)
                // if fWPSOnly is set, recover only windows
                // in the WPS process V0.9.20 (2002-08-10) [umoeller]
             && (    (!fWPSOnly)
                  || (    (WinQueryWindowProcess(hwnd, &pid, NULL))
                       && (pid == G_pHookData->pidWPS)
                     )
                )
             && (WinIsChild(hwnd, HWND_DESKTOP))
             && (!WinIsChild(hwnd, G_pHookData->hwndPagerFrame))
             && (WinQueryWindowPos(hwnd, &swp))
             && (!(swp.fl & (SWP_HIDE | SWP_MINIMIZE)))
             && (WinQueryClassName(hwnd, sizeof(szClassName), szClassName))
             && (strcmp(szClassName, "#32765"))
             // do not recover XCenter
             && (strcmp(szClassName, WC_XCENTER_FRAME))
             && (!pgrIsShowing(&swp))
           )
        {
            PSWP pswp;

            _Pmpf(("  restoring hwnd 0x%lX of pid %d", hwnd, pid));

            if (pswp = NEW(SWP))
            {
                memcpy(pswp, &swp, sizeof(SWP));
                lstAppendItem(&llSWPs, pswp);
            }
        }
    }
    WinEndEnumWindows(henum);

    if (    (cWindows = lstCountItems(&llSWPs))
         && (paswp = malloc(cWindows * sizeof(SWP)))
       )
    {
        LONG        cxEach = G_pHookData->szlEachDesktopFaked.cx,
                    cDesktopsX = G_pHookData->PagerConfig.cDesktopsX,
                    cyEach = G_pHookData->szlEachDesktopFaked.cy,
                    cDesktopsY = G_pHookData->PagerConfig.cDesktopsY;

        PSWP        pswpThis = paswp;
        PLISTNODE   pNode = lstQueryFirstNode(&llSWPs);

        while (pNode)
        {
            memcpy(pswpThis, pNode->pItemData, sizeof(SWP));

            pswpThis->fl = SWP_MOVE;
            if (!WinIsWindowVisible(pswpThis->hwnd))
                pswpThis->fl |= SWP_HIDE;

            pswpThis->x =   (   pswpThis->x
                              + (cDesktopsX * cxEach)
                            ) % cxEach;
            pswpThis->y =   (   pswpThis->y
                              + (cDesktopsY * cyEach)
                            ) % cyEach;

            if (pswpThis->x > cxEach - 5)
                pswpThis->x = pswpThis->x - cxEach;
            if (pswpThis->y > cyEach - 5)
                pswpThis->y = pswpThis->y - cyEach;

            pswpThis->hwndInsertBehind = HWND_TOP;

            pNode = pNode->pNext;
            pswpThis++;
        }

        WinSetMultWindowPos(hab,
                            paswp,
                            cWindows);
    }

    lstClear(&llSWPs);
}

/*
 *@@ GRID_X:
 *      returns the X coordinate of the grid line which
 *      is to the left of the given desktop no.
 *
 *      lDesktopX == 0 specifies the leftmost desktop.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

#define GRID_X(cx, lDesktopX) (cx * (lDesktopX) / G_pHookData->PagerConfig.cDesktopsX)

/*
 *@@ GRID_Y:
 *      returns the Y coordinate of the grid line which
 *      is to the bottom of the given desktop no.
 *
 *      lDesktopY == 0 specifies the bottommost desktop.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

#define GRID_Y(cy, lDesktopY) (cy * (lDesktopY) / G_pHookData->PagerConfig.cDesktopsY)

/*
 *@@ CreateTemplateBitmap:
 *      creates and returns the "template bitmap",
 *      which is the "empty" bitmap for the background
 *      of the pager. This includes the color fading,
 *      if enabled, and the desktop grid lines only.
 *
 *      We create the bitmap in RGB mode to profit
 *      from dithering if the display is not running
 *      in true color mode.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

STATIC HBITMAP CreateTemplateBitmap(HAB hab,
                                    PSIZEL pszl)
{
    HBITMAP hbmTemplate = NULLHANDLE;
    PXBITMAP pbmTemplate;
    if (pbmTemplate = gpihCreateXBitmap2(hab,
                                         pszl->cx,
                                         pszl->cy,
                                         1,
                                         24))
    {
        BKGNDINFO info;
        LONG l;
        HPS hpsMem = pbmTemplate->hpsMem;
        POINTL ptl;
        RECTL rclBkgnd;
        info.flPaintMode = G_pHookData->PagerConfig.flPaintMode;
        info.hbm = NULLHANDLE;
        info.lcol1 = G_pHookData->PagerConfig.lcolDesktop1;
        info.lcol2 = G_pHookData->PagerConfig.lcolDesktop2;
        rclBkgnd.xLeft = 0;
        rclBkgnd.yBottom = 0;
        rclBkgnd.xRight = pszl->cx - 1;
        rclBkgnd.yTop = pszl->cy - 1;
        gpihFillBackground(hpsMem,
                           &rclBkgnd,
                           &info);

        // draw the desktop grid on top of that
        GpiSetColor(hpsMem,
                    G_pHookData->PagerConfig.lcolGrid);

        // a) verticals (X separators)
        for (l = 1;
             l < G_pHookData->PagerConfig.cDesktopsX;
             ++l)
        {
            ptl.x = GRID_X(pszl->cx, l);
            ptl.y = 0;
            GpiMove(hpsMem, &ptl);
            ptl.y = pszl->cy - 1;
            GpiLine(hpsMem, &ptl);
        }

        // b) horizontals (Y separators)
        for (l = 1;
             l < G_pHookData->PagerConfig.cDesktopsY;
             ++l)
        {
            ptl.x = 0;
            ptl.y = GRID_Y(pszl->cy, l);
            GpiMove(hpsMem, &ptl);
            ptl.x = pszl->cx - 1;
            GpiLine(hpsMem, &ptl);
        }

        // detach this because otherwise we can't bitblt
        hbmTemplate = gpihDetachBitmap(pbmTemplate);
        gpihDestroyXBitmap(&pbmTemplate);
    }

    return hbmTemplate;
}

/*
 *@@ DestroyBitmaps:
 *      nukes the two member bitmaps to enforce
 *      a complete refresh on the next paint.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

STATIC VOID DestroyBitmaps(PPAGERWINDATA pWinData)
{
    if (pWinData->hbmTemplate)
    {
        GpiDeleteBitmap(pWinData->hbmTemplate);
        pWinData->hbmTemplate = NULLHANDLE;
    }

    gpihDestroyXBitmap(&pWinData->pbmClient);
}

/*
 *@@ MINIWINDOW:
 *      representation of a WININFO entry in the
 *      XPager client. This is only used for
 *      a temporary list in RefreshPagerBitmap.
 *
 *@@added V0.9.7 (2001-01-21) [umoeller]
 */

typedef struct _MINIWINDOW
{
    HWND            hwnd;           // window handle

    // calculated rectangle for mini window
    // in client coordinates (inclusive)
    POINTL          ptlLowerLeft,
                    ptlTopRight;
    PXWININFO       pWinInfo;       // ptr to wininfo this item
                                    // represents; always valid
} MINIWINDOW, *PMINIWINDOW;

/*
 *@@ DrawPointer:
 *
 *@@added V0.9.19 (2002-06-13) [umoeller]
 */

VOID DrawPointer(HPS hpsMem,
                 PMINIWINDOW pMiniThis,
                 LONG lcolCenter)
{
    HPOINTER hptr;

    if (    (hptr = pMiniThis->pWinInfo->hptrFrame)
            // do nothing if we've crashed on this icon before;
            // I had crashes with the x-file icon on the old
            // code... apparently this works now, but better
            // be safe than sorry
         && (!(pMiniThis->pWinInfo->flFlags & WLF_ICONCRASHED))
       )
    {
        TRY_LOUD(excpt1)
        {
            RECTL rclClip;
            rclClip.xLeft = pMiniThis->ptlLowerLeft.x + 1;
            rclClip.yBottom = pMiniThis->ptlLowerLeft.y + 1;
            rclClip.xRight = pMiniThis->ptlTopRight.x - 1;
            rclClip.yTop = pMiniThis->ptlTopRight.y - 1;

            gpihDrawPointer(hpsMem,
                            pMiniThis->ptlLowerLeft.x + 1,
                            pMiniThis->ptlLowerLeft.y + 1,
                            hptr,
                            &G_pHookData->szlIcon,
                            &rclClip,
                            DP_MINI);
        }
        CATCH(excpt1)
        {
            pMiniThis->pWinInfo->flFlags |= WLF_ICONCRASHED;
        } END_CATCH();
    }
}

/*
 *@@ RefreshPagerBitmap:
 *
 *      Calls CreateTemplateBitmap if necessary.
 *
 *      Preconditions:
 *
 *      --  The client bitmap must already have
 *          been created, but it is completely
 *          overpainted here.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

STATIC VOID RefreshPagerBitmap(HWND hwnd,
                               PPAGERWINDATA pWinData)
{
    HPS     hpsMem = pWinData->pbmClient->hpsMem;
    POINTL  ptl;

    LONG    cxEach = G_pHookData->szlEachDesktopFaked.cx,
            cyEach = G_pHookData->szlEachDesktopFaked.cy,
            xCurrent = G_pHookData->ptlCurrentDesktop.x,
            yCurrent = G_pHookData->ptlCurrentDesktop.y,
            cxClient = pWinData->szlClient.cx,
            cyClient = pWinData->szlClient.cy;

    ULONG   xDesktop = xCurrent / cxEach;
    ULONG   yDesktop = yCurrent / cyEach;

    ULONG   flPager = G_pHookData->PagerConfig.flPager;

    // check if we need a new template bitmap for background
    // (this gets destroyed on resize only)
    if (    (pWinData->hbmTemplate)
         || (pWinData->hbmTemplate = CreateTemplateBitmap(G_habDaemon,
                                                          &pWinData->szlClient))
       )
    {
        // a) bitblt the template into the client bitmap
        POINTL  aptl[4];
        memset(aptl, 0, sizeof(POINTL) * 4);
        aptl[1].x = cxClient - 1;
        aptl[1].y = cyClient - 1;
        aptl[3].x = cxClient;
        aptl[3].y = cyClient;
        GpiWCBitBlt(hpsMem,         // target HPS (bmp selected)
                    pWinData->hbmTemplate,        // source bmp
                    4L,             // must always be 4
                    &aptl[0],       // points array
                    ROP_SRCCOPY,
                    BBO_IGNORE);
    }

    // b) paint hatch for active desktop
    GpiSetColor(hpsMem, G_pHookData->PagerConfig.lcolActiveDesktop);
    GpiSetPattern(hpsMem, PATSYM_DIAG1);

    ptl.x = GRID_X(cxClient, xDesktop) + 1;
    ptl.y = GRID_Y(cyClient, yDesktop) + 1;

    GpiMove(hpsMem, &ptl);
    ptl.x = GRID_X(cxClient, xDesktop + 1) -1;
    ptl.y = GRID_Y(cyClient, yDesktop + 1) - 1;
    GpiBox(hpsMem,
           DRO_FILL,
           &ptl,
           0,
           0);

    GpiSetPattern(hpsMem, PATSYM_DEFAULT);

    // c) paint mini-windows
    if (flPager & PGRFL_MINIWINDOWS)
    {
        PMINIWINDOW     paMiniWindows = NULL;
        ULONG           cWinInfos = 0;

        // lock the window list all the while we're doing this
        // V0.9.7 (2001-01-21) [umoeller]
        if (pgrLockWinlist())
        {
            // allocate array of windows to be painted...
            // we fill this while enumerating Desktop windows,
            // which is needed for two reasons:
            // 1)   our internal list doesn't have the Z-order right;
            // 2)   WinBeginEnumWindows enumerates the windows from top
            //      to bottom, so we need to paint them in reverse order
            //      because the topmost window must be painted last

            // we allocate as many entries as are on the main
            // WININFO list to be on the safe side, but
            // not all will be used

            HENUM           henum = NULLHANDLE;

            if (    (cWinInfos = lstCountItems(&G_llWinInfos))
                 && (paMiniWindows = (PMINIWINDOW)malloc(cWinInfos * sizeof(MINIWINDOW)))
                 && (henum = WinBeginEnumWindows(HWND_DESKTOP))
               )
            {
                ULONG   cMiniWindowsUsed = 0;

                HWND    hwndThis;

                double  dScale_X =   (double)G_pHookData->PagerConfig.cDesktopsX
                                   * cxEach
                                   / cxClient,
                        dScale_Y =   (double)G_pHookData->PagerConfig.cDesktopsY
                                   * cyEach
                                   / cyClient;

                while (hwndThis = WinGetNextWindow(henum))
                {
                    PXWININFO pWinInfo;
                    // go thru local list and find the
                    // current enumeration window
                    if (    (WinIsWindowVisible(hwndThis))
                         && (pWinInfo = pgrFindWinInfo(hwndThis,
                                                       NULL))
                       )
                    {
                        BYTE bTypeThis;

                        // item is on list:
                        if (!pgrGetWinData(&pWinInfo->data, FALSE))
                            WinPostMsg(G_pHookData->hwndDaemonObject,
                                       XDM_WINDOWCHANGE,
                                       (MPARAM)hwndThis,
                                       (MPARAM)WM_DESTROY);
                        else
                        {
                             bTypeThis = pWinInfo->data.bWindowType;
                             if (    (bTypeThis == WINDOW_NORMAL)
                                  || (bTypeThis == WINDOW_MAXIMIZE)
                                  || (    (bTypeThis == WINDOW_STICKY)
                                       && (flPager & PGRFL_INCLUDESTICKY)
                                     )
                                  || (    (bTypeThis == WINDOW_NIL)
                                       && (flPager & PGRFL_INCLUDESECONDARY)
                                     )
                                )
                            {
                                // this window is to be painted:
                                // use a new item on the MINIWINDOW
                                // array then and calculate the
                                // mapping of the mini window

                                PMINIWINDOW pMiniThis
                                    = &paMiniWindows[cMiniWindowsUsed++];

                                LONG    xThis = pWinInfo->data.swp.x + xCurrent,
                                        yThis = pWinInfo->data.swp.y + yCurrent;

                                // store WININFO ptr; we hold the winlist
                                // locked all the time, so this is safe
                                pMiniThis->pWinInfo = pWinInfo;

                                pMiniThis->hwnd = hwndThis;

                                pMiniThis->ptlLowerLeft.x
                                    =   xThis / dScale_X;

                                pMiniThis->ptlLowerLeft.y
                                    =   yThis / dScale_Y + 1;

                                pMiniThis->ptlTopRight.x
                                    =   (xThis + pWinInfo->data.swp.cx) / dScale_X;

                                pMiniThis->ptlTopRight.y
                                    =   (yThis + pWinInfo->data.swp.cy) / dScale_Y + 1;

                            } // end if (    (bTypeThis == WINDOW_NORMAL) ...
                        }
                    } // end if (WinIsVisible)
                } // end while ((hwndThis = WinGetNextWindow(henum)) != NULLHANDLE)

                WinEndEnumWindows(henum);

                // now paint

                if (cMiniWindowsUsed)
                {
                    // we got something to paint:
                    // paint the mini windows in reverse order then
                    // (bottom to top)

                    HWND    hwndActive = WinQueryActiveWindow(HWND_DESKTOP);

                    // start with the last one
                    LONG l;
                    for (l = cMiniWindowsUsed - 1;
                         l >= 0;
                         --l)
                    {
                        PMINIWINDOW pMiniThis = &paMiniWindows[l];
                        LONG    lcolCenter,
                                lcolText;
                        PCSZ    pcszSwtitle;
                        ULONG   ulSwtitleLen;

                        // draw center
                        if (pMiniThis->hwnd == hwndActive)
                        {
                            // this is the active window:
                            lcolCenter = G_pHookData->PagerConfig.lcolActiveWindow;
                            lcolText = G_pHookData->PagerConfig.lcolActiveText;
                        }
                        else
                        {
                            lcolCenter = G_pHookData->PagerConfig.lcolInactiveWindow;
                            lcolText = G_pHookData->PagerConfig.lcolInactiveText;
                        }

                        GpiSetColor(hpsMem, lcolCenter);
                        GpiMove(hpsMem,
                                &pMiniThis->ptlLowerLeft);
                        GpiBox(hpsMem,
                               DRO_FILL,
                               &pMiniThis->ptlTopRight,
                               0,
                               0);

                        // draw border
                        GpiSetColor(hpsMem,
                                    G_pHookData->PagerConfig.lcolWindowFrame);
                        GpiBox(hpsMem,
                               DRO_OUTLINE,
                               &pMiniThis->ptlTopRight,
                               0,
                               0);

                        // draw window icon too?
                        // V0.9.19 (2002-06-13) [umoeller]
                        if (flPager & PGRFL_MINIWIN_ICONS)
                        {
                            // we can't use WinDrawPointer for many reasons,
                            // one of them being that it won't get the
                            // clipping right except with a major overhead
                            // and messing with the clip rectangle, so
                            // paint the icon manually
                            DrawPointer(hpsMem,
                                        pMiniThis,
                                        lcolCenter);
                        } // end if (flPager & PGRFL_MINIWIN_ICONS)

                        if (    (flPager & PGRFL_MINIWIN_TITLES)
                             && (pcszSwtitle = pMiniThis->pWinInfo->data.swctl.szSwtitle)
                             && (ulSwtitleLen = strlen(pcszSwtitle))
                           )
                        {
                            RECTL   rclText;

                            rclText.xLeft = pMiniThis->ptlLowerLeft.x + 1;
                            rclText.yBottom = pMiniThis->ptlLowerLeft.y + 1;
                            rclText.xRight = pMiniThis->ptlTopRight.x - 2;
                            rclText.yTop = pMiniThis->ptlTopRight.y - 2;

                            WinDrawText(hpsMem,
                                        ulSwtitleLen,
                                        (PSZ)pcszSwtitle,
                                        &rclText,
                                        lcolText,
                                        0,
                                        DT_LEFT | DT_TOP);

                        } // end if (    (flPager & PGRFL_MINIWIN_TITLES)
                    }
                } // end if (cMiniWindowsUsed)
            } // if (cWinInfos)

            pgrUnlockWinlist();
        }

        if (paMiniWindows)
            free(paMiniWindows);
    }
}

/*
 *@@ FindWindow:
 *      returns the window from the list which
 *      matches the given pager client coordinates.
 *      Returns NULLHANDLE if no window is found or
 *      if mini windows are not shown.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.18 (2002-02-19) [lafaix]: uses G_szlXPagerClient
 *@@changed V0.9.18 (2002-02-20) [lafaix]: reworked to correctly handle pager settings
 *@@changed V0.9.20 (2002-08-08) [umoeller]: fixed NIL windows
 */

STATIC HWND FindWindow(PPAGERWINDATA pWinData,
                       PPOINTL ptlClient,       // in: client coords
                       BOOL fAllowStickes)      // in: if TRUE, allow returning sticky windows
{
    HWND    hwndResult = NULLHANDLE;
    ULONG   flPager = G_pHookData->PagerConfig.flPager;

    if (flPager & PGRFL_MINIWINDOWS)
    {
        // mini windows are shown: check if the point denotes one of them
        // V0.9.18 (2002-02-20) [lafaix]
        POINTL      ptlCalc;
        HENUM       henumPoint;
        HWND        hwndPoint;
        SWP         swpPoint;

        // scale the client coordinate to the real desktop
        // coordinate (in terms of the current desktop)
        ptlCalc.x = (   ptlClient->x
                      * G_pHookData->szlEachDesktopFaked.cx
                      / (pWinData->szlClient.cx / G_pHookData->PagerConfig.cDesktopsX)
                    ) - G_pHookData->ptlCurrentDesktop.x;
        ptlCalc.y = (   ptlClient->y
                      * G_pHookData->szlEachDesktopFaked.cy
                      / (pWinData->szlClient.cy / G_pHookData->PagerConfig.cDesktopsY)
                    ) - G_pHookData->ptlCurrentDesktop.y;

        // enum all desktop windows again because our
        // internal winlist does not have the z-order right
        henumPoint = WinBeginEnumWindows(HWND_DESKTOP);
        while (hwndPoint = WinGetNextWindow(henumPoint))
        {
            if (hwndPoint == G_pHookData->hwndPagerFrame)
                // ignore XPager frame
                continue;

            if (    (WinIsWindowVisible(hwndPoint))
                 && (WinQueryWindowPos(hwndPoint, &swpPoint))
                 && (ptlCalc.x >= swpPoint.x)
                 && (ptlCalc.x <= swpPoint.x + swpPoint.cx)
                 && (ptlCalc.y >= swpPoint.y)
                 && (ptlCalc.y <= swpPoint.y + swpPoint.cy)
               )
            {
                // this window matches the coordinates:
                // check if it's visible in the client window
                // V0.9.18 (2002-02-20) [lafaix]
                if (pgrLockWinlist())
                {
                    PXWININFO pWinInfo = pgrFindWinInfo(hwndPoint, NULL);

                    if (    (pWinInfo)
                         && (    (pWinInfo->data.bWindowType == WINDOW_NORMAL)
                              || (pWinInfo->data.bWindowType == WINDOW_MAXIMIZE)
                                 // fixed NIL windows V0.9.20 (2002-08-08) [umoeller]
                              || (    (pWinInfo->data.bWindowType == WINDOW_NIL)
                                   && (flPager & PGRFL_INCLUDESECONDARY)
                                 )
                              || (    (fAllowStickes)
                                   && (pWinInfo->data.bWindowType == WINDOW_STICKY)
                                 )
                            )
                       )
                    {
                        hwndResult = hwndPoint;
                    }

                    pgrUnlockWinlist();

                    if (hwndResult)
                        break;
                }
            }
        }
        WinEndEnumWindows(henumPoint);
    }

    return hwndResult;
}

/* ******************************************************************
 *
 *   Pager client
 *
 ********************************************************************/

/*
 *@@ PagerPaint:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

STATIC VOID PagerPaint(HWND hwnd)
{
    PPAGERWINDATA pWinData;
    HPS hps;
    if (    (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
         && (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))
       )
    {
        RECTL rclClient;
        static POINTL ptlNull = {0, 0};
        WinQueryWindowRect(hwnd, &rclClient);

        gpihSwitchToRGB(hps);

        if (!pWinData->pbmClient)
        {
            // we don't have a client bitmap yet,
            // or the window has been resized:
            // create one
            if (pWinData->pbmClient = gpihCreateXBitmap(G_habDaemon,
                                                        rclClient.xRight,
                                                        rclClient.yTop))
            {
                // create logical font for the memory PS
                // (this data has been set in presparamschanged)

                HPS hpsMem = pWinData->pbmClient->hpsMem;
                GpiCreateLogFont(hpsMem,
                                 NULL,
                                 LCID_PAGER_FONT,
                                 &pWinData->fattr);
                GpiSetCharSet(hpsMem, LCID_PAGER_FONT);

                pWinData->fNeedsRefresh = TRUE;
            }
        }

        if (pWinData->pbmClient)
        {
            if (pWinData->fNeedsRefresh)
            {
                // client bitmap needs refresh:
                RefreshPagerBitmap(hwnd,
                                   pWinData);
                pWinData->fNeedsRefresh = FALSE;
            }

            WinDrawBitmap(hps,
                          pWinData->pbmClient->hbm,
                          NULL,     // whole bitmap
                          &ptlNull,
                          0,
                          0,
                          DBM_NORMAL);

            #ifdef __DEBUG__
            {
                CHAR sz[300];
                sprintf(sz,
                        "%d/%d",
                        G_pHookData->ptlCurrentDesktop.x,
                        G_pHookData->ptlCurrentDesktop.y);
                GpiSetColor(hps, RGBCOL_WHITE);
                WinDrawText(hps,
                            strlen(sz),
                            sz,
                            &rclClient,
                            0,
                            0,
                            DT_LEFT | DT_BOTTOM | DT_TEXTATTRS);
            }
            #endif
        }

        WinEndPaint(hps);
    }
}

/*
 *@@ PagerPresParamChanged:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

STATIC VOID PagerPresParamChanged(HWND hwnd, MPARAM mp1)
{
    PPAGERWINDATA pWinData;
    if (    (LONGFROMMP(mp1) == PP_FONTNAMESIZE)
         && (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
       )
    {
        // get the font metrics of the current window
        // font; if we get a cached micro PS, PM will
        // automatically have the font set, so we can
        // easily get its data and create our own
        // logical font for the bitmap work
        FONTMETRICS fm;
        HPS         hps = WinGetPS(hwnd);
        GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), &fm);

        pWinData->fattr.usRecordLength = sizeof(FATTRS);
        pWinData->fattr.fsSelection = fm.fsSelection;
        pWinData->fattr.lMatch = fm.lMatch;
        strcpy(pWinData->fattr.szFacename, fm.szFacename);
        pWinData->fattr.idRegistry = fm.idRegistry;
        pWinData->fattr.usCodePage = fm.usCodePage;
        pWinData->fattr.lMaxBaselineExt = fm.lMaxBaselineExt;
        pWinData->fattr.lAveCharWidth = fm.lAveCharWidth;

        WinReleasePS(hps);

        WinPostMsg(hwnd,
                   PGRM_REFRESHCLIENT,
                   (MPARAM)FALSE,
                   0);
    }
}

/*
 *@@ PagerPositionFrame:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 *@@changed V1.0.1 (2003-02-02) [umoeller]: adjusted bottom CY to avoid overlap with bottom XCenter
 */

STATIC VOID PagerPositionFrame(VOID)
{
    // disable message processing in the hook
    if (pgrLockHook(__FILE__, __LINE__, __FUNCTION__))
    {
        ULONG   flOptions;
        SWP     swpPager;

        ULONG   cb = sizeof(swpPager);

        if (    (!PrfQueryProfileData(HINI_USER,
                                      INIAPP_XWPHOOK,
                                      INIKEY_HOOK_PAGERWINPOS,
                                      &swpPager,
                                      &cb))
             || (cb != sizeof(swpPager))
           )
        {
            swpPager.x = 10;
            swpPager.y = 20 + WinQuerySysValue(HWND_DESKTOP,
                                               SV_CYICON) / 2;  // V1.0.1 (2003-02-02) [umoeller]
            swpPager.cx = G_pHookData->cxScreen * 18 / 100;
            swpPager.cy = pgrCalcClientCY(swpPager.cx);

            swpPager.cx += 2 * WinQuerySysValue(HWND_DESKTOP,
                                                SV_CXSIZEBORDER);
            swpPager.cy += 2 * WinQuerySysValue(HWND_DESKTOP,
                                                SV_CYSIZEBORDER);
        }

        WinSetWindowPos(G_pHookData->hwndPagerFrame,
                        NULLHANDLE,
                        swpPager.x,
                        swpPager.y,
                        swpPager.cx,
                        swpPager.cy,
                        SWP_MOVE | SWP_SIZE | SWP_SHOW);

        CheckFlashTimer();

        pgrUnlockHook();
    }
}

/*
 *@@ PagerButtonClick:
 *      implementation for WM_BUTTON1CLICK and WM_BUTTON2CLICK
 *      in fnwpPager.
 *
 *@@added V0.9.7 (2001-01-18) [umoeller]
 *@@changed V0.9.20 (2002-08-08) [umoeller]: added shift mb2 click for hiding pager
 *@@changed V1.0.0 (2002-09-13) [umoeller]: fixed pager window flickering with desktop switch
 */

STATIC MRESULT PagerButtonClick(HWND hwnd,
                                ULONG msg,
                                MPARAM mp1,
                                MPARAM mp2)
{
    POINTL  ptlMouse =
        {
            SHORT1FROMMP(mp1),
            SHORT2FROMMP(mp1)
        };

    HWND    hwndClicked = NULLHANDLE;
    PPAGERWINDATA pWinData;

    if (    (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
         && (G_pHookData->PagerConfig.flPager & PGRFL_MINIWIN_MOUSE)
       )
    {
        if (    (hwndClicked = FindWindow(pWinData,
                                          &ptlMouse,
                                          TRUE))        // allow clicking on stickies
             && (WinIsWindowEnabled(hwndClicked))
           )
        {
            if (msg == WM_BUTTON1CLICK)
            {
                // we first force a desktop switch if follow focus
                // is disabled
                // V0.9.19 (2002-06-14) [lafaix]
                if (G_pHookData->PagerConfig.flPager & PGRFL_NOFOLLOWFOCUS)
                {
                    pgrSwitchToDesktop(hwndClicked,
                                       TRUE,   // do move
                                       FALSE); // do not flash to top
                }

                // mb1: activate window
                WinSetActiveWindow(HWND_DESKTOP, hwndClicked);

                // even though the hook should monitor
                // this change and post PGRM_ACTIVECHANGED
                // to us, this does not work if the user
                // clicked on the currently active window
                // because then there's no change...
                // so post PGRM_ACTIVECHANGED anyway
                WinPostMsg(G_pHookData->hwndDaemonObject,
                           XDM_WINDOWCHANGE,
                           0,
                           (MPARAM)WM_ACTIVATE);

                // and refresh client right away because
                // we have a delay with activechanged
                #if 0       // no, this flickers V1.0.0 (2002-09-13) [umoeller]
                WinPostMsg(hwnd,
                           PGRM_REFRESHCLIENT,
                           (MPARAM)FALSE,
                           0);
                #endif
            }
            else
            {
                // mb2: lower window
                if (pgrLockHook(__FILE__, __LINE__, __FUNCTION__))
                {
                     WinSetWindowPos(hwndClicked,
                                     HWND_BOTTOM,
                                     0, 0, 0, 0,
                                     SWP_ZORDER);

                     WinPostMsg(hwnd,
                                PGRM_REFRESHCLIENT,
                                (MPARAM)FALSE,
                                0);

                    pgrUnlockHook();
                }
            }
        }
    }

    if (!hwndClicked)
    {
        // click on whitespace (not on mini-window):

        switch (msg)
        {
            case WM_BUTTON2CLICK:
                // mouse button 2 on empty area:

                // if shift is pressed, hide the pager
                // V0.9.20 (2002-08-08) [umoeller]
                if (    (G_pHookData->PagerConfig.flPager & PGRFL_FLASHTOTOP)
                     && (SHORT2FROMMP(mp2) & KC_SHIFT)
                   )
                    WinShowWindow(G_pHookData->hwndPagerFrame, FALSE);
                else
                {
                    // tell XFLDR.DLL that pager context menu
                    // was requested, together with desktop
                    // coordinates of where the user clicked
                    // so the XFLDR.DLL can display the context
                    // menu.... we don't want the NLS stuff in
                    // the daemon! V0.9.11 (2001-04-25) [umoeller]
                    WinMapWindowPoints(G_pHookData->hwndPagerClient,
                                       HWND_DESKTOP,
                                       &ptlMouse,
                                       1);
                    WinPostMsg(G_pXwpGlobalShared->hwndThread1Object,
                               T1M_PAGERCTXTMENU,
                               MPFROM2SHORT(ptlMouse.x,
                                            ptlMouse.y),
                               0);
                }
            break;

            case WM_BUTTON1CLICK:
                // click on desktop background:
                // switch to that desktop then

                WinPostMsg(G_pHookData->hwndPagerMoveThread,
                           PGRM_MOVEBYDELTA,
                           (MPARAM)(   G_pHookData->ptlCurrentDesktop.x
                                     - (   ptlMouse.x
                                         * G_pHookData->PagerConfig.cDesktopsX
                                         / pWinData->szlClient.cx
                                         * G_pHookData->szlEachDesktopFaked.cx
                                       )
                                   ),
                           (MPARAM)(   G_pHookData->ptlCurrentDesktop.y
                                     - (   ptlMouse.y
                                         * G_pHookData->PagerConfig.cDesktopsY
                                         / pWinData->szlClient.cy
                                         * G_pHookData->szlEachDesktopFaked.cy
                                       )
                                   ));
            break;
        }
    }

    CheckFlashTimer();

    return (MPARAM)TRUE;      // processed
}

/*
 *@@ PagerDrag:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 *@@changed V1.0.6 (2006-08-17) [pr]: Inverted Shift drag function at user request @@fixes 733
 */

STATIC VOID PagerDrag(HWND hwnd, MPARAM mp1)
{
    PPAGERWINDATA pWinData;
    if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
    {
        HWND        hwndTracked;
        SWP         swpTracked;

        POINTL      ptlMouse =
            {
                SHORT1FROMMP(mp1),
                SHORT2FROMMP(mp1)
            };

        // we must have the focus or the cursor and escape keys
        // won't work while dragging
        HWND            hwndOldFocus = WinQueryFocus(HWND_DESKTOP);
        WinSetFocus(HWND_DESKTOP, hwnd);

        if (G_pHookData->PagerConfig.flPager & PGRFL_FLASHTOTOP)
            WinStopTimer(G_habDaemon,
                         hwnd,
                         TIMERID_PGR_FLASH);

        if (    (!(hwndTracked = FindWindow(pWinData,
                                            &ptlMouse,
                                            FALSE)))        // do not track stickies
             || (hwndTracked == G_pHookData->hwndWPSDesktop)
           )
        {
            // user has started dragging on a non-mini window
            // (XPager client background): drag the entire
            // window then
            WinSendMsg(G_pHookData->hwndPagerFrame,
                       WM_TRACKFRAME,
                       (MPARAM)TF_MOVE,
                       0);
        }
        else
        {
            // user has started dragging a mini window: track
            // that one then
            TRACKINFO   ti;
            POINTL      ptlInitial;
            LONG        cxEach = G_pHookData->szlEachDesktopFaked.cx,
                        cyEach = G_pHookData->szlEachDesktopFaked.cy,
                        xCurrent = G_pHookData->ptlCurrentDesktop.x,
                        yCurrent = G_pHookData->ptlCurrentDesktop.y,
                        cxClient = pWinData->szlClient.cx,
                        cyClient = pWinData->szlClient.cy;
            double      dScaleX =   (double)G_pHookData->PagerConfig.cDesktopsX
                                  * cxEach
                                  / cxClient,
                        dScaleY =   (double)G_pHookData->PagerConfig.cDesktopsY
                                  * cyEach
                                  / cyClient;

            WinQueryWindowPos(hwndTracked, &swpTracked);
            ti.cxBorder = 1;
            ti.cyBorder = 1;

            ti.fs = TF_STANDARD | TF_MOVE | TF_SETPOINTERPOS | TF_PARTINBOUNDARY;

            // V1.0.6 (2006-08-17) [pr]: Inverted this function at user request @@fixes 733
            if (WinGetKeyState(HWND_DESKTOP, VK_SHIFT) & 0x8000)
            {
                // shift pressed: allow any position
                ti.cxGrid = 1;
                ti.cyGrid = 1;
            }
            else
            {
                // shift not pressed: set grid to each desktop
                ti.cxGrid = cxClient / G_pHookData->PagerConfig.cDesktopsX;
                ti.cyGrid = cyClient / G_pHookData->PagerConfig.cDesktopsY;
                ti.fs |= TF_GRID;
            }

            ti.cxKeyboard = 1;
            ti.cyKeyboard = 1;

            // for consistency, the track rect must have the size of the mini window
            // it represents
            ti.rclTrack.xLeft   =   (swpTracked.x + xCurrent)
                                  / dScaleX;
            ti.rclTrack.yBottom =   (swpTracked.y + yCurrent)
                                  / dScaleY + 1;
            ti.rclTrack.xRight  =   (swpTracked.x + swpTracked.cx + xCurrent)
                                  / dScaleX
                                  + 1;
            ti.rclTrack.yTop    =   (swpTracked.y + swpTracked.cy + yCurrent)
                                  / dScaleY
                                  + 2;

            ptlInitial.x = ti.rclTrack.xLeft;
            ptlInitial.y = ti.rclTrack.yBottom;

            // add a 1px boundary to the client area so that the moved
            // window remains accessible
            ti.rclBoundary.xLeft = 1;
            ti.rclBoundary.yBottom = 1;
            ti.rclBoundary.xRight = cxClient - 1;
            ti.rclBoundary.yTop = cyClient - 1;

            ti.ptlMinTrackSize.x = 2;
            ti.ptlMinTrackSize.y = 2;
            ti.ptlMaxTrackSize.x = cxClient;
            ti.ptlMaxTrackSize.y = cyClient;

            if (WinTrackRect(hwnd,
                             NULLHANDLE,        // hps
                             &ti))
            {
                POINTL ptlTracked;

                if (ti.fs & TF_GRID)
                {
                    // if in grid mode, we better keep the exact position we had
                    // before, instead of the approximation the track returned
                    // V0.9.18 (2002-02-14) [lafaix]
                    ptlTracked.x =   swpTracked.x
                                   +   ((ti.rclTrack.xLeft - ptlInitial.x) / ti.cxGrid)
                                     * cxEach;
                    ptlTracked.y =   swpTracked.y
                                   +   ((ti.rclTrack.yBottom - ptlInitial.y) / ti.cyGrid)
                                     * cyEach;
                }
                else
                {
                    // if the x position hasn't changed, keep the exact x position
                    // we had before V0.9.18 (2002-02-14) [lafaix]
                    if (ti.rclTrack.xLeft != ptlInitial.x)
                        ptlTracked.x =   (ti.rclTrack.xLeft * dScaleX)
                                       - xCurrent;
                    else
                        ptlTracked.x = swpTracked.x;

                    // if the y position hasn't changed, keep the exact y position
                    // we had before V0.9.18 (2002-02-14) [lafaix]
                    if (ti.rclTrack.yBottom != ptlInitial.y)
                        ptlTracked.y =   (ti.rclTrack.yBottom * dScaleY)
                                       - yCurrent;
                    else
                        ptlTracked.y = swpTracked.y;
                }

                // disable message processing in the hook
                if (pgrLockHook(__FILE__, __LINE__, __FUNCTION__))
                {
                    WinSetWindowPos(hwndTracked,
                                    HWND_TOP,
                                    ptlTracked.x,
                                    ptlTracked.y,
                                    0,
                                    0,
                                    SWP_MOVE | SWP_NOADJUST);

                    pgrUnlockHook();
                }
            }
        }

        // restore the old focus if we had one
        // (WinSetFocus(NULLHANDLE) brings up the window list...)
        if (hwndOldFocus)
            WinSetFocus(HWND_DESKTOP, hwndOldFocus);
                // Note: this also has the side effect that if the
                // user drags the window that is currently active
                // to another desktop, we switch desktops because
                // the hook will detect that a different window
                // got activated. I think that's useful... it will
                // not happen if a non-active window gets dragged.

        WinPostMsg(hwnd,
                   PGRM_REFRESHCLIENT,
                   (MPARAM)FALSE,
                   0);
    }
}

/*
 *@@ pgrSwitchToDesktop:
 *      switch to the desktop containing the center of hwnd (if
 *      possible).  Do the move if fMove is TRUE.  Flash the
 *      pager if fFlashToTop is TRUE when the move is possible.
 *
 *      Returns TRUE if the move was possible.
 *
 *@@added V0.9.20 (2002-07-26) [lafaix]
 */

BOOL pgrSwitchToDesktop(HWND hwnd,
                        BOOL fMove,
                        BOOL fFlashToTop)
{
    BOOL brc = FALSE;

    if (hwnd)
    {
        SWP swp;

        if (WinQueryWindowPos(hwnd, &swp))
        {
            // calculate the absolute coordinate of the center
            // of the window relative to the bottom
            // left desktop:
            LONG    cx = G_pHookData->szlEachDesktopFaked.cx,
                    cy = G_pHookData->szlEachDesktopFaked.cy,
                    xCurrent = G_pHookData->ptlCurrentDesktop.x,
                    yCurrent = G_pHookData->ptlCurrentDesktop.y,
                    x =      (    swp.x
                                + (swp.cx / 2)
                                + xCurrent
                             ) / cx
                               * cx,
                    y =      (    swp.y
                                + (swp.cy / 2)
                                + yCurrent
                             ) / cy
                               * cy;

            // bump boundaries
            if (    (x >= 0)
                 && (x <= (G_pHookData->PagerConfig.cDesktopsX * cx))
                 && (y >= 0)
                 && (y <= (G_pHookData->PagerConfig.cDesktopsY * cy))
               )
            {
                if (fMove)
                    WinPostMsg(G_pHookData->hwndPagerMoveThread,
                               PGRM_MOVEBYDELTA,
                               (MPARAM)(xCurrent - x),
                               (MPARAM)(yCurrent - y));

                if (fFlashToTop)
                {
                    WinSetWindowPos(G_pHookData->hwndPagerFrame,
                                    HWND_TOP,
                                    0, 0, 0, 0,
                                    SWP_SHOW | SWP_ZORDER);
                    CheckFlashTimer();
                }

                brc = TRUE;
            }
        }
    }

    return brc;
}

/*
 *@@ PagerActiveChanged:
 *      implementation for PGRM_ACTIVECHANGED in
 *      fnwpPager.
 *
 *      This switches the current desktop if a
 *      window becomes active that is on a different
 *      desktop.
 *
 *      Note that this is deferred through a PM timer
 *      and actually gets called when WM_TIMER for
 *      TIMERID_PGR_ACTIVECHANGED comes in.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 *@@changed V0.9.19 (2002-06-02) [umoeller]: made this configurable
 *@@changed V1.0.0 (2002-09-13) [umoeller]: fixed pager window flickering with desktop switch
 */

STATIC VOID PagerActiveChanged(HWND hwnd)
{
    // we only do this if we are not currently processing
    // a pager wraparound
    if (!G_pHookData->fProcessingWraparound)
    {
        HWND        hwndActive;
        BOOL        fRefresh = TRUE;       // V1.0.0 (2002-09-13) [umoeller]

        if (hwndActive = WinQueryActiveWindow(HWND_DESKTOP))
        {
            // test if this is a sticky window;
            // if so, never switch desktops
            HSWITCH hsw;
            SWP     swpActive;
            if (hsw = WinQuerySwitchHandle(hwndActive, 0))
            {
                SWCNTRL swc;
                if (!WinQuerySwitchEntry(hsw, &swc))
                        // for some reason, this returns 0 on success!!
                {
                    if (pgrIsSticky(hwndActive, swc.szSwtitle))
                        // it's sticky: get outta here
                        hwndActive = NULLHANDLE;
                }
                else
                    // no switch entry available: do not switch
                    hwndActive = NULLHANDLE;
            }

            if (    (hwndActive)
                 && (WinQueryWindowPos(hwndActive, &swpActive))
               )
            {
                // do not switch to hidden or minimized windows
                if (    (0 == (swpActive.fl & (SWP_HIDE | SWP_MINIMIZE)))
                        // only move if window is not visible
                     && (!pgrIsShowing(&swpActive))
                   )
                {
                    pgrSwitchToDesktop(hwndActive,
                                       // only actually post the msg if follow focus
                                       // is enabled V0.9.19 (2002-06-02) [umoeller]
                                       !(G_pHookData->PagerConfig.flPager & PGRFL_NOFOLLOWFOCUS),
                                       // even if follow focus is disabled we should still bring
                                       // the pager window back to top in flash mode
                                       // V0.9.19 (2002-06-02) [umoeller]
                                       // but only if in flash mode
                                       // V0.9.19 (2002-06-13) [lafaix]
                                       (G_pHookData->PagerConfig.flPager & PGRFL_FLASHTOTOP));

                    // move thread takes care of refresh, so do NOT
                    // post refresh below, or we'll flicker
                    fRefresh = FALSE;
                }
            }
        } // end if (hwndActive)

        // refresh client
        if (fRefresh)       // V1.0.0 (2002-09-13) [umoeller]
            WinPostMsg(hwnd,
                       PGRM_REFRESHCLIENT,
                       (MPARAM)FALSE,
                       0);

    } // end if (!G_pHookData->fProcessingWraparound)

    G_fActiveChangedPending = FALSE;
}

/*
 *@@ PagerHotkey:
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 *@@changed V1.0.4 (2005-08-29) [pr]: Restart flash timer when hotkeys are used
 */

STATIC VOID PagerHotkey(MPARAM mp1)
{
    LONG    dx = 0,
            dy = 0;

    switch ((ULONG)mp1)
    {
        case 0x63:                              // left
            dx = G_pHookData->szlEachDesktopFaked.cx;
        break;

        case 0x64:                              // right
            dx = -G_pHookData->szlEachDesktopFaked.cx;
        break;

        case 0x66:                              // down
            dy = G_pHookData->szlEachDesktopFaked.cy;
        break;

        case 0x61:                              // up
            dy = -G_pHookData->szlEachDesktopFaked.cy;
        break;
    }

    WinPostMsg(G_pHookData->hwndPagerMoveThread,
               PGRM_MOVEBYDELTA,
               (MPARAM)dx,
               (MPARAM)dy);
    CheckFlashTimer(); // V1.0.4 (2005-08-29) [pr]
}

/*
 *@@ fnwpPager:
 *      window proc for the pager client, which shows the
 *      mini windows.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 *@@changed V0.9.20 (2002-07-03) [umoeller]: reduced activate delay from 200 to 50 ms
 *@@changed V1.0.8 (2008-01-08) [pr]: keep pager window visible if mouse over it @@fixes 1038
 */

STATIC MRESULT EXPENTRY fnwpPager(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    TRY_LOUD(excpt1)
    {
        switch (msg)
        {
            /*
             * WM_CREATE:
             *      we receive the PAGERWINDATA as the create param.
             */

            case WM_CREATE:
                WinSetWindowPtr(hwnd, QWL_USER, mp1);

                // make the pager a client of the switchlist thread
                WinSendMsg(G_pHookData->hwndDaemonObject,
                           XDM_ADDWINLISTWATCH,
                           (MPARAM)hwnd,        // window to be notified
                           (MPARAM)PGRM_WINDOWCHANGE);      // msg to post

                // set offsets for current desktop
                G_pHookData->ptlCurrentDesktop.x
                    =   (G_pHookData->PagerConfig.bStartX - 1)
                      * G_pHookData->szlEachDesktopFaked.cx;
                G_pHookData->ptlCurrentDesktop.y
                    =   (G_pHookData->PagerConfig.bStartY - 1)
                      * G_pHookData->szlEachDesktopFaked.cy;
            break;

            case WM_DESTROY:
                // un-make the pager a client of the switchlist thread
                WinSendMsg(G_pHookData->hwndDaemonObject,
                           XDM_REMOVEWINLISTWATCH,
                           (MPARAM)hwnd,        // window to be notified
                           NULL);
            break;

            case WM_PAINT:
                PagerPaint(hwnd);
            break;

            case WM_WINDOWPOSCHANGED:
            {
                PPAGERWINDATA  pWinData;

                if (    (((PSWP)mp1)->fl & SWP_SIZE)
                     && (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
                     // fixed the following, we had a AND here which caused
                     // the repaint to fail when the desktop dimensions were
                     // changed only V0.9.19 (2002-07-01) [umoeller]
                     && (    (pWinData->szlClient.cx != ((PSWP)mp1)->cx)
                          || (pWinData->szlClient.cy != ((PSWP)mp1)->cy)
                        )
                   )
                {
                    // size changed:
                    // then the bitmaps need a resize,
                    // destroy both the template and the client bitmaps
                    DestroyBitmaps(pWinData);

                    pWinData->szlClient.cx = ((PSWP)mp1)->cx;
                    pWinData->szlClient.cy = ((PSWP)mp1)->cy;

                    // we have CS_SIZEREDRAW, so we get repainted now
                }
            }
            break;

            case WM_PRESPARAMCHANGED:
                PagerPresParamChanged(hwnd, mp1);
            break;

            /*
             *@@ PGRM_POSITIONFRAME:
             *      causes the pager to position itself
             *      based on the current settings.
             *      Sent on startup.
             *
             *@@added V0.9.19 (2002-05-07) [umoeller]
             */

            case PGRM_POSITIONFRAME:
                PagerPositionFrame();
            break;

            /*
             *@@ PGRM_WINDOWCHANGE:
             *      implementation for the pager being
             *      a "window list notify" client (see
             *      XDM_ADDWINLISTWATCH).
             *
             *@@added V0.9.19 (2002-05-28) [umoeller]
             */

            case PGRM_WINDOWCHANGE:
                if ((ULONG)mp2 == WM_ACTIVATE)
                {
                    G_fActiveChangedPending = TRUE;

                    // at this point, start a timer only so we can
                    // defer processing a bit to avoid flicker
                    WinStartTimer(G_habDaemon,
                                  hwnd,
                                  TIMERID_PGR_ACTIVECHANGED,
                                  50);     // reduced from 200 ms
                                           // V0.9.20 (2002-07-03) [umoeller]
                }
                else if (!G_fActiveChangedPending)
                    // refresh the client bitmap
                    WinPostMsg(hwnd,
                               PGRM_REFRESHCLIENT,
                               (MPARAM)FALSE,
                               0);
            break;

            case PGRM_ICONCHANGE:
                // refresh the client bitmap
                WinPostMsg(hwnd,
                           PGRM_REFRESHCLIENT,
                           (MPARAM)FALSE,
                           0);
            break;

            /*
             *@@ PGRM_REFRESHCLIENT:
             *      posted from various places if the
             *      pager window needs not only be repainted,
             *      but completely refreshed (i.e. we need
             *      to rebuild the client bitmap because
             *      window positions have changed etc.).
             *
             *      In order not to cause excessive rebuilds
             *      of the bitmap, we only post WM_SEM2 here.
             *      According to PMREF, WM_SEM2 is lower in
             *      priority than msgs posted by WinPostMsg
             *      and from the input queue, but still higher
             *      than WM_PAINT. As a result, if there are
             *      several window updates pending, the client
             *      window will not get rebuilt until all
             *      are processed.
             *
             *      Parameters:
             *
             *      --  BOOL mp1: if TRUE, we delete the cached
             *          background bitmap in order to have the
             *          complete display refreshed. That is
             *          expensive and only needed if color
             *          settings have changed.
             *
             *      --  mp2: not used.
             */

            case PGRM_REFRESHCLIENT:
            {
                PPAGERWINDATA pWinData;
                if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
                {
                    pWinData->fNeedsRefresh = TRUE;
                    WinPostMsg(hwnd,
                               WM_SEM2,
                               (MPARAM)1,       // semaphore bits
                               0);
                    if (mp1)
                    {
                        DestroyBitmaps(pWinData);
                        CheckFlashTimer();
                    }
                }
            }
            break;

            /*
             * WM_SEM2:
             *      PM semaphore message with priority
             *      higher only than WM_PAINT; set by
             *      PGRM_REFRESHCLIENT to refresh the client.
             */

            case WM_SEM2:
                // ignore the refresh request if an ACTIVECHANGED timer
                // is pending; this prevent unnecessary repainting
                // V0.9.19 (2002-06-14) [lafaix]
                if (!G_fActiveChangedPending)
                    WinInvalidateRect(hwnd, NULL, FALSE);
            break;

            /*
             *@@ PGRM_WRAPAROUND:
             *      sent by the daemon when the mouse has hit
             *      a screen border and a wraparound was configured.
             *
             *      If a mouse switch request is pending, we must
             *      discard incoming requests, so as to prevent
             *      erratic desktop movement.  Otherwise, we post
             *      ourself a message to process the request at
             *      a later time.
             *
             *      This message must be sent, not posted.
             */

            case PGRM_WRAPAROUND:
                // avoid excessive switching, and disable
                // sliding focus too
                if (!G_pHookData->fProcessingWraparound)
                {
                    G_pHookData->fProcessingWraparound = TRUE;

                    // delay the switch
                    WinPostMsg(hwnd,
                               PGRM_PAGERHOTKEY,
                               mp1,
                               (MPARAM)TRUE);
                }
            break;

            /*
             *@@ PGRM_PAGERHOTKEY:
             *      posted by hookPreAccelHook when
             *      one of the arrow hotkeys was
             *      detected.
             *
             *      (UCHAR)mp1 has the scan code of the
             *      key which was pressed.
             *
             *      (BOOL)mp2 is true if this has been
             *      posted due to a mouse wraparound request.
             *
             *@@changed V0.9.9 (2001-03-14) [lafaix]: mp2 defined.
             */

            case PGRM_PAGERHOTKEY:
                PagerHotkey(mp1);
            break;

            case WM_TIMER:
            {
                BOOL fStop = TRUE;
                switch ((USHORT)mp1)
                {
                    case TIMERID_PGR_ACTIVECHANGED:
                        PagerActiveChanged(hwnd);
                    break;

                    case TIMERID_PGR_FLASH:
                        WinShowWindow(G_pHookData->hwndPagerFrame, FALSE);
                    break;

                    default:
                        fStop = FALSE;
                        mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
                }

                if (fStop)
                    WinStopTimer(G_habDaemon,
                                 hwnd,
                                 (USHORT)mp1);
            }
            break;

            /*
             * WM_BUTTON1CLICK:
             *      activate a specific window.
             */

            case WM_BUTTON1CLICK:
            case WM_BUTTON2CLICK:
                mrc = PagerButtonClick(hwnd, msg, mp1, mp2);
            break;

            /*
             * WM_BUTTON2MOTIONSTART:
             *      move the windows within the pager.
             */

            // V1.0.8 (2008-01-08) [pr]
            case WM_BEGINDRAG:
                G_fDraggingCtrlWin = TRUE;
                PagerDrag(hwnd, mp1);
            break;

            case WM_ENDDRAG:
                G_fDraggingCtrlWin = FALSE;
                CheckFlashTimer();
            break;

#ifndef WM_MOUSEENTER
    #define WM_MOUSEENTER   0x041E
    #define WM_MOUSELEAVE   0x041F
#endif

            case WM_MOUSEENTER:
                // stop timer so that it is only triggered when leaving the window
                if (G_pHookData->PagerConfig.flPager & PGRFL_FLASHTOTOP)
                    WinStopTimer(G_habDaemon,
                                 hwnd,
                                 TIMERID_PGR_FLASH);
            break;

            case WM_MOUSELEAVE:
                CheckFlashTimer();
            break;

            default:
                mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
            break;
        }
    }
    CATCH(excpt1)
    {
        mrc = 0;
    } END_CATCH();

    return mrc;
}

/* ******************************************************************
 *
 *   Pager frame
 *
 ********************************************************************/

/*
 *@@ fnwpSubclPagerFrame:
 *      window proc for the subclassed pager frame to
 *      hack ourselves into the resizing and stuff.
 *
 *@@added V0.9.19 (2002-05-07) [umoeller]
 */

STATIC MRESULT EXPENTRY fnwpSubclPagerFrame(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT     mrc = 0;

    PPAGERWINDATA pWinData;

    switch (msg)
    {
        case WM_ADJUSTWINDOWPOS:
            if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                PSWP pswp = (PSWP)mp1;

                if (pswp->cx < 20)
                    pswp->cx = 20;
                if (pswp->cy < 20)
                    pswp->cy = 20;

                if (G_pHookData->PagerConfig.flPager & PGRFL_PRESERVEPROPS)
                {
                    LONG cxFrame = WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER),
                         cyFrame = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);

                    LONG  cx =   pswp->cx
                               - 2 * cxFrame;
                    pswp->cy =   pgrCalcClientCY(cx)
                               + 2 * cyFrame;
                }

                // we never want to become active:
                if (pswp->fl & SWP_ACTIVATE)
                {
                    pswp->fl &= ~SWP_ACTIVATE;
                    pswp->fl |= SWP_DEACTIVATE;
                }

                mrc = pWinData->pfnwpOrigFrame(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_FOCUSCHANGE:
         *      the standard frame window proc sends out a
         *      flurry of WM_SETFOCUS and WM_ACTIVATE
         *      messages. We never want to get the focus,
         *      no matter what.
         */

        case WM_FOCUSCHANGE:
        break;

        /*
         * WM_TRACKFRAME:
         *      comes in when the user attempts to resize
         *      the XPager window. We need to stop the
         *      flash timer while this is going on.
         */

        case WM_TRACKFRAME:
            if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                // stop flash timer, if running
                if (G_pHookData->PagerConfig.flPager & PGRFL_FLASHTOTOP)
                    WinStopTimer(G_habDaemon,
                                 G_pHookData->hwndPagerClient,
                                 TIMERID_PGR_FLASH);

                // now track window (WC_FRAME runs a modal message loop)
                mrc = pWinData->pfnwpOrigFrame(hwnd, msg, mp1, mp2);

                CheckFlashTimer();
            }
        break;

        /*
         * WM_QUERYTRACKINFO:
         *      the frame control generates this when
         *      WM_TRACKFRAME comes in.
         */

        case WM_QUERYTRACKINFO:
            if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                PTRACKINFO  ptr;
                pWinData->pfnwpOrigFrame(hwnd, msg, mp1, mp2);
                ptr = (PTRACKINFO) PVOIDFROMMP(mp2);
                ptr->ptlMinTrackSize.x = 20;
                ptr->ptlMinTrackSize.y = 20;

                mrc = MRFROMSHORT(TRUE);
            }
        break;

        /*
         * WM_WINDOWPOSCHANGED:
         *      store new window pos in OS2.INI.
         */

        case WM_WINDOWPOSCHANGED:
            if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                if (((PSWP)mp1)->fl & (SWP_SIZE | SWP_MOVE))
                    PrfWriteProfileData(HINI_USER,
                                        INIAPP_XWPHOOK,
                                        INIKEY_HOOK_PAGERWINPOS,
                                        mp1,
                                        sizeof(SWP));
                if (((PSWP)mp1)->fl & SWP_SHOW)
                    CheckFlashTimer();

                mrc = pWinData->pfnwpOrigFrame(hwnd, msg, mp1, mp2);
            }
        break;

        /*
         * WM_DESTROY:
         *      children receive this before the frame,
         *      so free the win data here and not in
         *      the client.
         */

        case WM_DESTROY:
            if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
            {
                mrc = pWinData->pfnwpOrigFrame(hwnd, msg, mp1, mp2);
                DestroyBitmaps(pWinData);
                G_pHookData->hwndPagerFrame = NULLHANDLE;
                G_pHookData->hwndPagerClient = NULLHANDLE;
                free(pWinData);
            }
        break;

        default:
            if (pWinData = (PPAGERWINDATA)WinQueryWindowPtr(hwnd, QWL_USER))
                mrc = pWinData->pfnwpOrigFrame(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/* ******************************************************************
 *
 *   Pager creation
 *
 ********************************************************************/

/*
 *@@ pgrCreatePager:
 *      creates the XPager control window (frame
 *      and client). This gets called by dmnStartXPager
 *      on thread 1 when XPager has been enabled.
 *
 *      This registers the XPager client class
 *      using fnwpPager, which does most of
 *      the work for XPager, and subclasses the
 *      frame with fnwpSubclPagerFrame.
 *
 *@@added V0.9.2 (2000-02-21) [umoeller]
 *@@changed V0.9.3 (2000-05-22) [umoeller]: the hook was interfering, fixed
 */

BOOL pgrCreatePager(VOID)
{
    PPAGERWINDATA pWinData;
    BOOL    brc = TRUE;

    if (G_pHookData->hwndPagerFrame)
        // pager is already running:
        return TRUE;

    WinRegisterClass(G_habDaemon,
                     (PSZ)WC_PAGER,
                     (PFNWP)fnwpPager,
                     CS_SIZEREDRAW,
                     sizeof(PVOID));

    // allocate pager win data
    if (pWinData = NEW(PAGERWINDATA))
    {
        ZERO(pWinData);
        pWinData->cb = sizeof(PAGERWINDATA);

        // disable message processing in the hook
        if (pgrLockHook(__FILE__, __LINE__, __FUNCTION__))
        {
            FRAMECDATA  fcdata;
            HWND        hwndFrame;
            RECTL       rclClient;

            fcdata.cb            = sizeof(FRAMECDATA);
            fcdata.flCreateFlags = FCF_NOBYTEALIGN | FCF_SIZEBORDER;
            fcdata.hmodResources = (HMODULE)NULL;
            fcdata.idResources   = 0;

            // create frame and client
            if (    (G_pHookData->hwndPagerFrame = WinCreateWindow(HWND_DESKTOP,
                                                                   WC_FRAME,
                                                                   "Pager",
                                                                   0,
                                                                   0, 0, 0, 0,
                                                                   NULLHANDLE,       // owner
                                                                   HWND_TOP,
                                                                   0,                // id
                                                                   &fcdata,         // frame class data
                                                                   NULL))           // no presparams
                 && (G_pHookData->hwndPagerClient = WinCreateWindow(G_pHookData->hwndPagerFrame,
                                                                    (PSZ)WC_PAGER,
                                                                    "",
                                                                    WS_VISIBLE,
                                                                    0, 0, 0, 0,
                                                                    G_pHookData->hwndPagerFrame, // owner
                                                                    HWND_TOP,
                                                                    FID_CLIENT,
                                                                    pWinData,
                                                                    NULL))
               )
            {
                // give frame access to win data too
                WinSetWindowPtr(G_pHookData->hwndPagerFrame,
                                QWL_USER,
                                pWinData);

                // set frame icon
                WinSendMsg(G_pHookData->hwndPagerFrame,
                           WM_SETICON,
                           (MPARAM)G_hptrDaemon,
                           NULL);

                // set font (client catches this)
                #define DEFAULTFONT "2.System VIO"
                WinSetPresParam(G_pHookData->hwndPagerClient,
                                PP_FONTNAMESIZE,
                                sizeof(DEFAULTFONT),
                                DEFAULTFONT);

                thrCreate(&G_tiMoveThread,
                          fntMoveThread,
                          NULL, // running flag
                          "PgmgMove",
                          THRF_WAIT | THRF_PMMSGQUEUE,    // PM msgq
                          0);
                    // this creates the XPager object window

                // subclass frame
                pWinData->pfnwpOrigFrame = WinSubclassWindow(G_pHookData->hwndPagerFrame,
                                                             fnwpSubclPagerFrame);

                // have pgr position itself
                WinSendMsg(G_pHookData->hwndPagerClient,
                           PGRM_POSITIONFRAME,
                           0,
                           0);

                brc = TRUE;
            }

            pgrUnlockHook();
        }
    }

    return brc;
}


