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

#define INCL_GRE_DCS
#include "pmddi.h"

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

        if (hdc == NULLHANDLE) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (GreOpenDC failed)");
            return 0;
        }

        pWnd->SetDC(hdc);

        F_DbgCtxExitf(ctx, "HDC=%lu (new)", (unsigned long)hdc);
        return (ULONG)hdc;
    }

    default:
        return 0;
    }
}
