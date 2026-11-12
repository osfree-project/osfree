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

        if (!pArgs || cbArgs < sizeof(WinShowWindow_Args)) return 0;
        a = (PWinShowWindow_Args)pArgs;
        pWnd = m_wnds.Find((ULONG)a->hwnd);
        if (!pWnd) return 0;
        pWnd->SetVisible(a->fShow ? TRUE : FALSE);
        return 1;
    }

    case NWinSetWindowPos: {
        PWinSetWindowPos_Args a;
        Fs_Wnd *pWnd;

        if (!pArgs || cbArgs < sizeof(WinSetWindowPos_Args)) return 0;
        a = (PWinSetWindowPos_Args)pArgs;
        pWnd = m_wnds.Find((ULONG)a->hwnd);
        if (!pWnd) return 0;

        if (a->fl & SWP_MOVE) pWnd->SetPos(a->x, a->y);
        if (a->fl & SWP_SIZE) pWnd->SetSize(a->cx, a->cy);
        if (a->fl & SWP_SHOW) pWnd->SetVisible(TRUE);
        if (a->fl & SWP_HIDE) pWnd->SetVisible(FALSE);
        return 1;
    }

    case NWinDestroyWindow: {
        if (!m_wnds.Find((ULONG)data)) return 0;
        m_wnds.Destroy((ULONG)data);
        return 1;
    }

    default:
        return 0;
    }
}
