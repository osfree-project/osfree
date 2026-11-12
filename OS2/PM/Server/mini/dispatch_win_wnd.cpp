/*!
 * @file dispatch_win_wnd.cpp
 * @brief Win* window-lifecycle dispatcher.
 *
 * Handles NWinCreateWindow, NWinShowWindow, NWinSetWindowPos,
 * NWinDestroyWindow. The server stores geometry, visibility, style,
 * title, ctlData and presParams. No rendering: the client draws via
 * Gpi* on the PS created for this window's DC.
 */
#include "Fs_server.hpp"
#include "F_debug.h"

#define INCL_GREALL
#include "pmddi.h"

#include <string.h>

/*! @brief Read `cb` bytes from payload and duplicate into a fresh blob. */
static PBYTE CopyBlob(PBYTE p, ULONG cb)
{
    PVOID pv = NULL;
    if (cb == 0) return NULL;
    if (DosAllocMem(&pv, cb, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
        return NULL;
    memcpy(pv, p, cb);
    return (PBYTE)pv;
}

/*!
 * @brief Fill the current window rectangle with black.
 *
 * Simulation of the desktop repaint that in real PM redraws the area
 * uncovered by hiding or moving a window.  Saves the current line
 * colour, paints the DC's current window rectangle in black, and
 * restores the colour.
 *
 * Must be called while the DC origin still reflects the window's
 * current position (i.e., before a move updates the origin).
 *
 * The colour is passed as a raw 0x00BBGGRR value (0x00000000 = black),
 * not as an OS/2 CLR_* logical index: the presentation driver currently
 * interprets lColor as RGB (see Os2ColorToRgb in gre2sdl.cpp), so CLR_*
 * constants would produce meaningless results.
 *
 * @param [in] hdc  Device context of the window.
 * @param [in] pWnd Window whose rectangle should be painted.
 */
static void SrvEraseWindowRect(HDC hdc, Fs_Wnd *pWnd)
{
    LINEBUNDLE savedLb, lb;
    BOXPARAMS  box;
    POINTL     ptl00;

    memset(&savedLb, 0, sizeof(savedLb));
    memset(&lb,      0, sizeof(lb));

    GreDeviceGetAttributes(hdc, PRIM_LINE, LBB_COLOR, &savedLb);

    lb.lColor = 0x00000000L;
    GreDeviceSetAttributes(hdc, PRIM_LINE, LBB_COLOR, 0L, &lb);

    ptl00.x = 0;
    ptl00.y = 0;
    GreSetCurrentPosition(hdc, &ptl00);

    box.ptl.x   = pWnd->GetCx();
    box.ptl.y   = pWnd->GetCy();
    box.sizl.cx = 0;
    box.sizl.cy = 0;
    GreBoxInterior(hdc, &box);

    GreDeviceSetAttributes(hdc, PRIM_LINE, LBB_COLOR, 0L, &savedLb);
}

/*!
 * @brief Dispatch NWin* commands in the window lifecycle domain (0x0020..0x005F).
 *
 * @param [in]  pRecvobj   Transport connection handle.
 * @param [in]  fnCode     NWin* opcode (with NWIN_SYNC_FLAG for destroy).
 * @param [in]  data       hwndParent for CreateWindow, hwnd for DestroyWindow.
 * @param [in]  pArgs      Payload for async commands.
 * @param [in]  cbArgs     Payload size.
 * @param [out] pcbPayload Always 0.
 * @return CreateWindow: HWND; ShowWindow / SetWindowPos / DestroyWindow: BOOL.
 */
ULONG Fs_Server::DispatchWinWnd(void *pRecvobj, ULONG fnCode, ULONG data,
                                PBYTE pArgs, ULONG cbArgs,
                                PULONG pcbPayload)
{
    Fs_Connection *pConn   = m_conns.FindByRecvobj(pRecvobj);
    Fs_Thread     *pThread = pConn ? pConn->GetThread() : NULL;
    Fs_Process    *pProc   = pThread ? pThread->GetProcess() : NULL;

    if (pcbPayload) *pcbPayload = 0;
    if (!pProc || !pThread || !pThread->GetHab()) return 0;

    switch (fnCode) {

    case NWinCreateWindow: {
        PWinCreateWindow_Args a;
        ULONG  off, cbClass, cbName;
        PBYTE  pbCtlData = NULL, pbPresParams = NULL;
        ULONG  cbCtlData = 0, cbPresParams = 0;
        char   szClass[256], szTitle[64];
        Fs_Wnd *pWnd;
        ULONG   hwnd = 0;
        int     ctx;

        if (!pArgs || cbArgs < sizeof(WinCreateWindow_Args)) return 0;
        a = (PWinCreateWindow_Args)pArgs;

        cbClass = a->cbClass;
        cbName  = a->cbName;
        off     = sizeof(WinCreateWindow_Args);

        if (cbClass > 255 || cbName > 255) return 0;
        if (off + cbClass + cbName > cbArgs) return 0;

        memcpy(szClass, pArgs + off, cbClass);
        szClass[cbClass] = '\0';
        off += cbClass;

        {
            ULONG nCopy = cbName < (sizeof(szTitle) - 1)
                        ? cbName : (ULONG)(sizeof(szTitle) - 1);
            if (nCopy) memcpy(szTitle, pArgs + off, nCopy);
            szTitle[nCopy] = '\0';
            off += cbName;
        }

        if (a->flPresent & WCRF_HAS_CTLDATA) {
            USHORT us;
            if (off + 2 > cbArgs) return 0;
            us = *(USHORT *)(pArgs + off);
            off += 2;
            if (off + us > cbArgs) return 0;
            pbCtlData = pArgs + off;
            cbCtlData = us;
            off += us;
        }

        if (a->flPresent & WCRF_HAS_PRESPARAMS) {
            ULONG cb;
            if (off + 4 > cbArgs) return 0;
            cb = *(ULONG *)(pArgs + off);
            off += 4;
            if (off + cb > cbArgs) return 0;
            pbPresParams = pArgs + off;
            cbPresParams = cb;
            off += cb;
        }

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinCreateWindow(parent=%lu, "
                             "class=%s, title=%s, %ld,%ld %ldx%ld, "
                             "cbCtl=%lu, cbPres=%lu)",
                             (unsigned long)data, szClass, szTitle,
                             (long)a->x, (long)a->y, (long)a->cx, (long)a->cy,
                             (unsigned long)cbCtlData,
                             (unsigned long)cbPresParams);

        pWnd = m_wnds.Create(pThread->GetIHab(), &hwnd);
        if (!pWnd) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (m_wnds.Create failed)");
            return 0;
        }
        pWnd->SetRect(a->x, a->y, a->cx, a->cy);
        pWnd->SetStyle(a->flStyle);
        pWnd->SetVisible((a->flStyle & WS_VISIBLE) ? TRUE : FALSE);
        pWnd->SetTitle(szTitle);

        if (cbCtlData) {
            PBYTE p = CopyBlob(pbCtlData, cbCtlData);
            if (p) pWnd->SetCtlData(p, cbCtlData);
        }
        if (cbPresParams) {
            PBYTE p = CopyBlob(pbPresParams, cbPresParams);
            if (p) pWnd->SetPresParams(p, cbPresParams);
        }

        F_DbgCtxExitf(ctx, "hwnd=%lu", (unsigned long)hwnd);
        return hwnd;
    }

    case NWinShowWindow: {
        PWinShowWindow_Args a;
        Fs_Wnd *pWnd;
        HDC     hdc;
        BOOL    bWasVisible, bNewVisible;

        if (!pArgs || cbArgs < sizeof(WinShowWindow_Args)) return 0;
        a = (PWinShowWindow_Args)pArgs;
        pWnd = m_wnds.Find((ULONG)a->hwnd);
        if (!pWnd) return 0;

        hdc = pWnd->GetDC();
        bWasVisible = pWnd->IsVisible();
        bNewVisible = a->fShow ? TRUE : FALSE;

        if (hdc != NULLHANDLE && bWasVisible && !bNewVisible) {
            /* Hide: erase the window rectangle (simulated desktop
             * repaint) and clear the visible region. */
            RECTL rclEmpty;
            SrvEraseWindowRect(hdc, pWnd);
            rclEmpty.xLeft   = 0;
            rclEmpty.yBottom = 0;
            rclEmpty.xRight  = 0;
            rclEmpty.yTop    = 0;
            GreSetupDC(hdc, 0,
                       (LONG)pWnd->GetX(), (LONG)pWnd->GetY(),
                       &rclEmpty,
                       SETUPDC_VISRGN);
        } else if (hdc != NULLHANDLE && !bWasVisible && bNewVisible) {
            /* Show: restore the visible region to the full window rect.
             * The canvas content was erased on hide; the client is
             * expected to redraw. */
            RECTL rcl;
            rcl.xLeft   = 0;
            rcl.yBottom = 0;
            rcl.xRight  = pWnd->GetCx();
            rcl.yTop    = pWnd->GetCy();
            GreSetupDC(hdc, 0,
                       (LONG)pWnd->GetX(), (LONG)pWnd->GetY(),
                       &rcl,
                       SETUPDC_VISRGN | SETUPDC_ORIGIN);
        }

        pWnd->SetVisible(bNewVisible);
        return 1;
    }

    case NWinSetWindowPos: {
        PWinSetWindowPos_Args a;
        Fs_Wnd *pWnd;
        HDC     hdc;
        BOOL    bWasVisible, bNewVisible;

        if (!pArgs || cbArgs < sizeof(WinSetWindowPos_Args)) return 0;
        a = (PWinSetWindowPos_Args)pArgs;
        pWnd = m_wnds.Find((ULONG)a->hwnd);
        if (!pWnd) return 0;

        hdc = pWnd->GetDC();
        bWasVisible = pWnd->IsVisible();
        bNewVisible = bWasVisible;
        if (a->fl & SWP_SHOW) bNewVisible = TRUE;
        if (a->fl & SWP_HIDE) bNewVisible = FALSE;

        if (hdc != NULLHANDLE && bWasVisible) {
            /* Erase the old rectangle while the DC origin still
             * reflects the old position (simulated desktop repaint). */
            SrvEraseWindowRect(hdc, pWnd);
        }

        if (a->fl & SWP_MOVE) pWnd->SetPos(a->x, a->y);
        if (a->fl & SWP_SIZE) pWnd->SetSize(a->cx, a->cy);
        pWnd->SetVisible(bNewVisible);

        if (hdc != NULLHANDLE) {
            if (bNewVisible) {
                /* Re-setup DC with the new origin and clip. */
                RECTL rcl;
                rcl.xLeft   = 0;
                rcl.yBottom = 0;
                rcl.xRight  = pWnd->GetCx();
                rcl.yTop    = pWnd->GetCy();
                GreSetupDC(hdc, 0,
                           (LONG)pWnd->GetX(), (LONG)pWnd->GetY(),
                           &rcl,
                           SETUPDC_VISRGN | SETUPDC_ORIGIN);
            } else {
                RECTL rclEmpty;
                rclEmpty.xLeft   = 0;
                rclEmpty.yBottom = 0;
                rclEmpty.xRight  = 0;
                rclEmpty.yTop    = 0;
                GreSetupDC(hdc, 0,
                           (LONG)pWnd->GetX(), (LONG)pWnd->GetY(),
                           &rclEmpty,
                           SETUPDC_VISRGN);
            }
        }

        return 1;
    }

    case NWinDestroyWindow: {
        Fs_Wnd *pWnd;
        Fs_Hab *pHab;
        ULONG   iHab;

        pWnd = m_wnds.Find((ULONG)data);
        if (!pWnd) return 0;

        /* EDM2 WinDestroyWindow: "The window to be destroyed must
         * have been created by the thread that is issuing this call."
         * Verified via the window's HAB back-ref: HWND -> iHab ->
         * Fs_Hab -> tid. HWND_DESKTOP is server-owned (iHab == 0),
         * so no client thread can match it. */
        iHab = pWnd->GetHab();
        if (iHab == 0) return 0;

        pHab = m_habs.FindByIHab(iHab);
        if (!pHab || pHab->GetTid() != pThread->GetTid()) return 0;

        m_wnds.Destroy((ULONG)data);
        return 1;
    }

    default:
        return 0;
    }
}
