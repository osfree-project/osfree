/*!
 * @file dispatch_win_ps.cpp
 * @brief Win* PS / DC dispatcher.
 *
 * NWinOpenWindowDC returns a GRE device context for a window. The HDC
 * is obtained through the existing GPI path: DispatchGpi is invoked
 * with NDevOpenDC and a marshalled GpiDevOpenDC_Args payload
 * (OD_DIRECT, token "*", pdop {"DISPLAY","GRE2SDL"}), matching the
 * client-side DevOpenDC wire layout (pmserver.log: ArgsSize=34).
 *
 * Sync path in HandleWinCall: HWND travels in `data`, no payload, and
 * the reply is a bare 4-byte ulResult (the HDC).
 */
#include "Fs_server.hpp"
#include "F_debug.h"
#include "F_cmd.h"

#include <string.h>

/*!
 * @brief Dispatch NWin* commands in the PS / DC domain (0x0060..0x007F).
 *
 * @param [in]  pRecvobj   Transport connection handle.
 * @param [in]  fnCode     NWin* opcode (with NWIN_SYNC_FLAG).
 * @param [in]  data       HWND for NWinOpenWindowDC.
 * @param [in]  pArgs      Unused (sync path).
 * @param [in]  cbArgs     Unused (sync path).
 * @param [out] pcbPayload Always 0.
 * @return NWinOpenWindowDC: HDC, or 0 (NULLHANDLE) on failure.
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
        HWND   hwnd = (HWND)data;
        ULONG  iHab;
        BYTE   buf[64];
        ULONG  off;
        ULONG  cbOdcPayload = 0;
        ULONG  hdc;
        int    ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinOpenWindowDC(recvobj=%p, "
                             "hwnd=%lu)", pRecvobj, (unsigned long)hwnd);

        if (!pProc || !pThread || !pThread->GetHab()) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (no HAB)");
            return 0;
        }

        /* Only the desktop window exists in the registry today. Any
           other HWND -> NULLHANDLE (PM Reference: PMERR_INVALID_HWND). */
        if (hwnd == 0 ||
            GetHwndDesktop() == 0 ||
            (ULONG)hwnd != GetHwndDesktop()) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (hwnd=%lu, desktop=%lu)",
                          (unsigned long)hwnd,
                          (unsigned long)GetHwndDesktop());
            return 0;
        }

        iHab = pThread->GetIHab();

        /* Marshal NDevOpenDC. Layout matches the client DevOpenDC path
           (pmserver.log, ArgsSize=34):
             GpiDevOpenDC_Args { hab, OD_DIRECT, 2, 0 }   16 bytes
             "*\0"                                         2 bytes
             "DISPLAY\0"                                   8 bytes
             "GRE2SDL\0"                                   8 bytes
        */
        memset(buf, 0, sizeof(buf));
        ((GpiDevOpenDC_Args *)buf)->hab     = (HAB)iHab;
        ((GpiDevOpenDC_Args *)buf)->lType   = OD_DIRECT;
        ((GpiDevOpenDC_Args *)buf)->lCount  = 2;
        ((GpiDevOpenDC_Args *)buf)->hdcComp = 0;
        off = sizeof(GpiDevOpenDC_Args);

        buf[off++] = '*'; buf[off++] = '\0';
        memcpy(buf + off, "DISPLAY", 7); off += 7; buf[off++] = '\0';
        memcpy(buf + off, "GRE2SDL", 7); off += 7; buf[off++] = '\0';

        hdc = DispatchGpi(pRecvobj, NDevOpenDC, 0,
                          buf, off, &cbOdcPayload);

        if (cbOdcPayload != 0) {
            F_DbgPrint(F_DBG_SEC_DISPATCH, F_DBG_LVL_WARNING,
                       "[PMSERVER] NWinOpenWindowDC: NDevOpenDC returned "
                       "unexpected payload=%lu (ignored)",
                       (unsigned long)cbOdcPayload);
        }

        F_DbgCtxExitf(ctx, "HDC=%lu", (unsigned long)hdc);
        return hdc;
    }

    default:
        return 0;
    }
}
