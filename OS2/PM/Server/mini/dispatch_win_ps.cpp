/*!
 * @file dispatch_win_ps.cpp
 * @brief Win* PS / DC dispatcher.
 *
 * NWinOpenWindowDC associates an HDC with a window and returns it.
 * The association is 1:1: subsequent calls for the same HWND return
 * the existing HDC. The DC is created via GreOpenDC.
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
 * @brief Ensure the window has a DC, creating and setting it up if not.
 *
 * Shared by NWinOpenWindowDC and NWinBeginPaint: both need a valid
 * HDC on the window before any PS or drawing can happen. Idempotent:
 * returns the existing HDC if one is already attached.
 *
 * Mirrors the DC-open path in NWinOpenWindowDC (GreOpenDC with
 * {"DISPLAY","GRE2SDL"}, SetDC, GreSetupDC with the window origin and
 * full-window clip). Kept as a private helper so the two commands do
 * not drift apart.
 *
 * @param [in] pWnd  Window slot.
 * @return HDC on success, NULLHANDLE on failure.
 * @retval NULLHANDLE  pWnd is NULL, GreOpenDC failed, or driver
 *                     rejected the DC.
 * @retval !=NULLHANDLE Valid HDC (existing or freshly opened).
 */
static HDC SrvEnsureWindowDC(Fs_Wnd *pWnd)
{
    HDC hdc;

    if (!pWnd) return NULLHANDLE;

    hdc = pWnd->GetDC();
    if (hdc != NULLHANDLE) return hdc;

    {
        PSZ pdop[2] = { (PSZ)"DISPLAY", (PSZ)"GRE2SDL" };
        hdc = GreOpenDC(NULLHANDLE, OD_DIRECT, (PSZ)"*", 2,
                        (PDEVOPENDATA)pdop);
    }
    if (hdc == NULLHANDLE || (ULONG)hdc == (ULONG)-1)
        return NULLHANDLE;

    pWnd->SetDC(hdc);

    {
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
    return hdc;
}

/*!
 * @brief Dispatch NWin* commands in the PS / DC domain (0x0060..0x007F).
 *
 * @param [in]  pRecvobj   Transport connection handle.
 * @param [in]  fnCode     NWin* opcode (with NWIN_SYNC_FLAG).
 * @param [in]  data       HWND for NWinOpenWindowDC.
 * @param [in]  pArgs      Unused (sync path).
 * @param [in]  cbArgs     Unused (sync path).
 * @param [out] pcbPayload Always 0.
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

    (void)pArgs; (void)cbArgs;
    if (pcbPayload) *pcbPayload = 0;

    switch (fnCode) {

    case NWinOpenWindowDC: {
        HWND    hwnd = (HWND)data;
        Fs_Wnd *pWnd;
        HDC     hdc;
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

        /* HWND -> HDC association is 1:1; return the existing HDC. */
        hdc = pWnd->GetDC();
        if (hdc != NULLHANDLE) {
            F_DbgCtxExitf(ctx, "HDC=%lu (existing)", (unsigned long)hdc);
            return (ULONG)hdc;
        }

        {
            PSZ pdop[2] = { (PSZ)"DISPLAY", (PSZ)"GRE2SDL" };
            hdc = GreOpenDC(NULLHANDLE, OD_DIRECT, (PSZ)"*", 2,
                            (PDEVOPENDATA)pdop);
        }

        /* GreOpenDC returns (HDC)(ULONG)-1 on failure (driver rejected
         * by GRE, NewDc failed).  Both 0 and 0xFFFFFFFF are invalid
         * HDC values.  Report NULLHANDLE, otherwise the client would
         * treat 0xFFFFFFFF as a transport error (sync contract of
         * Fc_SesSendCommand uses -1 for transport error). */
        if (hdc == NULLHANDLE || (ULONG)hdc == (ULONG)-1) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (GreOpenDC failed, hdc=0x%lx)",
                          (unsigned long)hdc);
            return 0;
        }

        pWnd->SetDC(hdc);

        /* PMWIN sets up the DC when the window is created.  In real PM
         * this is done by GreSetupDC; GRE implements it by simulating
         * the mandatory driver hooks (GreDeviceSetDCOrigin and
         * GreNotifyClipChange). */
        {
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

        F_DbgCtxExitf(ctx, "HDC=%lu (new)", (unsigned long)hdc);
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

        hdc = SrvEnsureWindowDC(pWnd);
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
