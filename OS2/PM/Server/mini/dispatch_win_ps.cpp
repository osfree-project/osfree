/*!
 * @file dispatch_win_ps.cpp
 * @brief Win* PS / DC dispatcher.
 *
 * NWinOpenWindowDC associates an HDC with a window and returns it.
 * The association is 1:1: subsequent calls for the same HWND return
 * the existing HDC. The DC itself is owned by Fs_Wnd and opened by
 * Fs_Wnd::EnsureDC (GreOpenDC + GreSetupDC with the window origin
 * and full-window clip).
 *
 * Sync path in HandleWinCall: HWND travels in `data`, no payload, and
 * the reply is a bare 4-byte ulResult (the HDC).
 */
#include "Fs_server.hpp"
#include "F_debug.h"

#define INCL_GREALL
#include "pmddi.h"

/* Server-side PS helpers (GpiPsCreate / GpiPsFind / GpiPsDestroy). */
#define GPI_SERVER
#include "gpi_ps.h"

/*!
 * @brief Dispatch NWin* commands in the PS / DC domain (0x0060..0x007F).
 *
 * @param [in]  pRecvobj   Transport connection handle.
 * @param [in]  fnCode     NWin* opcode (with NWIN_SYNC_FLAG).
 * @param [in]  data       HWND for NWinOpenWindowDC.
 * @param [in]  pArgs      Payload for NWinBeginPaint / NWinEndPaint.
 * @param [in]  cbArgs     Payload size.
 * @param [out] pcbPayload sizeof(WinBeginPaint_Args) for NWinBeginPaint,
 *                         0 otherwise.
 * @return NWinOpenWindowDC: Associated HDC, or 0 (NULLHANDLE) on failure.
 * @retval 0    NULLHANDLE (no HAB, window not found, window not visible,
 *              GreOpenDC failed, or unknown fnCode).
 * @retval !0   Valid HDC on success.
 */
ULONG Fs_Server::DispatchWinPs(void *pRecvobj, ULONG fnCode, ULONG data,
                                PBYTE pArgs, ULONG cbArgs,
                                PULONG pcbPayload)
{
    Fs_Connection *pConn   = m_conns.FindByRecvobj(pRecvobj);
    Fs_Thread     *pThread = pConn ? pConn->GetThread() : NULL;
    Fs_Process    *pProc   = pThread ? pThread->GetProcess() : NULL;

    if (pcbPayload) *pcbPayload = 0;

    switch (fnCode) {

    case NWinOpenWindowDC: {
        HWND    hwnd = (HWND)data;
        Fs_Wnd *pWnd;
        HDC     hdc, hdcOld;
        int     ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinOpenWindowDC(recvobj=%p, "
                             "hwnd=%lu)", pRecvobj, (unsigned long)hwnd);

        if (!pProc || !pThread || !pThread->GetHab()) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (no HAB)");
            return 0;
        }

        pWnd = m_wnds.Find((ULONG)hwnd);
        if (!pWnd) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (window not found)");
            return 0;
        }
        if (!pWnd->IsVisible()) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (window not visible)");
            return 0;
        }

        /* HWND -> HDC association is 1:1; EnsureDC returns the existing
         * HDC or opens a fresh one and sets it up. */
        hdcOld = pWnd->GetDC();
        hdc = pWnd->EnsureDC();
        if (hdc == NULLHANDLE) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (GreOpenDC failed)");
            return 0;
        }

        F_DbgCtxExitf(ctx, "HDC=%lu (%s)", (unsigned long)hdc,
                      (hdcOld != NULLHANDLE) ? "existing" : "new");
        return (ULONG)hdc;
    }

    case NWinBeginPaint: {
        /* Async: pArgs = WinBeginPaint_Args (hwnd in payload). */
        PWinBeginPaint_Args a = (PWinBeginPaint_Args)pArgs;
        Fs_Wnd   *pWnd;
        HDC       hdc;
        PGPI_PS   ps;
        int       ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinBeginPaint(recvobj=%p, "
                             "cbArgs=%u)", pRecvobj, (unsigned)cbArgs);

        if (!a || cbArgs < sizeof(*a)) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (bad args)");
            return 0;
        }

        pWnd = m_wnds.Find((ULONG)a->hwnd);
        if (!pWnd) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (hwnd=%lu not found)",
                          (unsigned long)a->hwnd);
            return 0;
        }

        hdc = pWnd->EnsureDC();
        if (hdc == NULLHANDLE) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (no DC)");
            return 0;
        }

        /* DEVIATION FROM OS/2 PM: real PM returns a cached micro PS
         * (GPIT_MICRO) here — no segment store, no retained mode,
         * no SavePS/RestorePS. The minimal server has only Normal PS
         * (GPI_PS has no ulPSType field, see spec3 §A4), so we create
         * a fresh Normal PS and destroy it in WinEndPaint.
         *
         * Consequence: GpiOpenSegment / GpiSetDrawingMode(DM_RETAIN)
         * / GpiBeginElement succeed where they should return
         * GPI_ERROR for a micro PS. Accepted as a known deviation
         * until micro PS support is added.
         *
         * No update-region tracking yet: the whole window is treated
         * as invalid, and rcl = {0, 0, cx, cy}. */
        ps = GpiPsCreate(NULL, hdc, PU_PELS | GPIF_LONG | GPIA_ASSOC);
        if (!ps) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (GpiPsCreate failed)");
            return 0;
        }
        ps->m_hHab = pThread->GetIHab();

        a->hps          = (HPS)ps;
        a->rcl.xLeft    = 0;
        a->rcl.yBottom  = 0;
        a->rcl.xRight   = pWnd->GetCx();
        a->rcl.yTop     = pWnd->GetCy();

        if (pcbPayload) *pcbPayload = sizeof(*a);
        F_DbgCtxExitf(ctx, "hps=%p (hwnd=%lu, rcl={0,0,%ld,%ld})",
                      (void*)ps, (unsigned long)a->hwnd,
                      (long)a->rcl.xRight, (long)a->rcl.yTop);
        return (ULONG)ps;
    }

    case NWinEndPaint: {
        /* Async: pArgs = WinEndPaint_Args (hps in payload). */
        PWinEndPaint_Args a = (PWinEndPaint_Args)pArgs;
        PGPI_PS ps;
        int     ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinEndPaint(recvobj=%p, cbArgs=%u)",
                             pRecvobj, (unsigned)cbArgs);

        if (!a || cbArgs < sizeof(*a)) {
            F_DbgCtxExitf(ctx, "0 (bad args)");
            return 0;
        }

        ps = GpiPsFind((HPS)a->hps);
        if (!ps) {
            F_DbgCtxExitf(ctx, "0 (hps=%p not found)", (void*)a->hps);
            return 0;
        }

        /* DEVIATION FROM OS/2 PM: real PM releases the cached micro PS
         * back to PMWIN here; the minimal server destroys the Normal PS
         * it created in NWinBeginPaint. See the NWinBeginPaint comment. */
        GpiPsDestroy((HPS)a->hps);

        F_DbgCtxExitf(ctx, "1 (hps=%p)", (void*)a->hps);
        return 1;
    }

    default:
        return 0;
    }
}
