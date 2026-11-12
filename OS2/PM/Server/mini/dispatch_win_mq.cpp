/*!
 * @file dispatch_win_mq.cpp
 * @brief Win* message-queue dispatcher (HMQ lifecycle, post, get,
 *        dispatch).
 *
 * Handles the PMMQ wire domain (fnCode 0x0010..0x001F):
 *
 *   NWinCreateMsgQueue    sync   data=hab           -> hmq
 *   NWinDestroyMsgQueue   sync   data=hmq           -> BOOL
 *   NWinPostMsg           async  WinPostMsg_Args    -> BOOL
 *   NWinGetMsg            async  WinGetMsg_Args     -> BOOL + QMSG
 *   NWinDispatchMsg       async  WinDispatchMsg_Args-> MRESULT
 *
 * The queue itself lives on the HAB that called WinCreateMsgQueue
 * (Fs_Hab::SetQueue). Ownership and HMQ validation follow OS/2 PM:
 *   - WinCreateMsgQueue fails if the thread already has a queue
 *     (PMERR_MSG_QUEUE_ALREADY_EXISTS);
 *   - WinDestroyMsgQueue requires the HMQ to match the caller's own
 *     queue.
 *
 * NWinGetMsg blocks on Fs_Queue::Get until a message is available;
 * WM_QUIT terminates the loop (returns FALSE without delivering a
 * QMSG), matching EDM2 WinGetMsg. Message filtering (hwndFilter /
 * msgFirst / msgLast) is accepted but not applied yet.
 *
 * NWinDispatchMsg calls the server-side default window procedure
 * (Fs_WndList::DefWindowProc). Until per-class Server*WndProc
 * implementations appear, every message goes through the default
 * handler; that is the server-side analogue of PMWIN's
 * WinDefWindowProc.
 *
 * NWinPeekMsg is deferred (it needs the same wire layout as
 * NWinGetMsg plus the PM_REMOVE flag).
 */
#include "Fs_server.hpp"
#include "Fs_queue.hpp"
#include "F_debug.h"

/*!
 * @brief Dispatch NWin* commands in the message-queue domain
 *        (0x0010..0x001F).
 *
 * @param [in]  pRecvobj   Transport connection handle.
 * @param [in]  fnCode     NWin* opcode (with NWIN_SYNC_FLAG on sync
 *                         path).
 * @param [in]  data       HAB (CreateMsgQueue), HMQ (DestroyMsgQueue);
 *                         unused for PostMsg / GetMsg / DispatchMsg.
 * @param [in]  pArgs      Async payload (WinPostMsg_Args,
 *                         WinGetMsg_Args, WinDispatchMsg_Args).
 * @param [in]  cbArgs     Payload size.
 * @param [out] pcbPayload Reply payload size (sizeof(WinGetMsg_Args)
 *                         for NWinGetMsg, 0 otherwise).
 * @return Function-specific result.
 * @retval 0  Failure / FALSE / NULLHANDLE.
 * @retval >0 Success / TRUE / HMQ value / MRESULT.
 */
ULONG Fs_Server::DispatchWinMq(void *pRecvobj, ULONG fnCode, ULONG data,
                                PBYTE pArgs, ULONG cbArgs,
                                PULONG pcbPayload)
{
    Fs_Connection *pConn   = m_conns.FindByRecvobj(pRecvobj);
    Fs_Thread     *pThread = pConn ? pConn->GetThread() : NULL;

    if (pcbPayload) *pcbPayload = 0;
    if (!pThread || !pThread->GetHab()) return 0;

    switch (fnCode) {

    case NWinCreateMsgQueue: {
        Fs_Hab   *pHab = pThread->GetHab();
        Fs_Queue *pQ;
        ULONG     hmq;
        int       ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinCreateMsgQueue(recvobj=%p, "
                             "iHab=%lu)",
                             pRecvobj, (unsigned long)pHab->GetIHab());

        if (pHab->GetQueue() != NULL) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (PMERR_MSG_QUEUE_ALREADY_EXISTS)");
            return 0;
        }

        hmq = AllocHmq();
        pQ  = new Fs_Queue(pHab->GetIHab(), hmq);
        if (!pQ) {
            F_DbgCtxExitf(ctx, "NULLHANDLE (Fs_Queue allocation failed)");
            return 0;
        }
        pHab->SetQueue(pQ);
        F_DbgCtxExitf(ctx, "hmq=%lu", (unsigned long)hmq);
        return hmq;
    }

    case NWinDestroyMsgQueue: {
        Fs_Hab   *pHab = pThread->GetHab();
        Fs_Queue *pQ   = pHab->GetQueue();
        int       ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinDestroyMsgQueue(recvobj=%p, "
                             "hmq=%lu)", pRecvobj, (unsigned long)data);

        if (!pQ) {
            F_DbgCtxExitf(ctx, "0 (no queue)");
            return 0;
        }
        if ((ULONG)data != pQ->GetHmq()) {
            F_DbgCtxExitf(ctx, "0 (hmq mismatch: expected %lu)",
                          (unsigned long)pQ->GetHmq());
            return 0;
        }
        delete pQ;
        pHab->ClearQueue();
        F_DbgCtxExitf(ctx, "1");
        return 1;
    }

    case NWinPostMsg: {
        /* Async: pArgs = WinPostMsg_Args. */
        PWinPostMsg_Args a = (PWinPostMsg_Args)pArgs;
        Fs_Wnd    *pWnd;
        Fs_Hab    *pHab;
        Fs_Queue  *pQ;
        QMSG       qmsg;
        BOOL       ok;
        int        ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinPostMsg(recvobj=%p, cbArgs=%u)",
                             pRecvobj, (unsigned)cbArgs);

        if (!a || cbArgs < sizeof(*a)) {
            F_DbgCtxExitf(ctx, "0 (bad args)");
            return 0;
        }

        if (a->hwnd == NULLHANDLE) {
            /* EDM2 WinPostMsg: "If hwnd is NULLHANDLE, the message is
             * placed on the message queue of the calling thread." No
             * window lookup is performed; the calling HAB owns the
             * queue. This is the canonical path for WM_QUIT. */
            pHab = pThread->GetHab();
            if (!pHab || !pHab->GetQueue()) {
                F_DbgCtxExitf(ctx, "0 (no queue on calling HAB)");
                return 0;
            }
            pQ = pHab->GetQueue();
        } else {
            pWnd = m_wnds.Find((ULONG)a->hwnd);
            if (!pWnd) {
                F_DbgCtxExitf(ctx, "0 (hwnd=%lu not found)",
                              (unsigned long)a->hwnd);
                return 0;
            }
            pHab = m_habs.FindByIHab(pWnd->GetHab());
            if (!pHab || !pHab->GetQueue()) {
                F_DbgCtxExitf(ctx, "0 (no queue for hwnd owner)");
                return 0;
            }
            pQ = pHab->GetQueue();
        }

        /* Build QMSG from wire args. time and ptl are not tracked by
         * the minimal server yet. */
        qmsg.hwnd       = a->hwnd;
        qmsg.msg        = a->msg;
        qmsg.mp1        = a->mp1;
        qmsg.mp2        = a->mp2;
        qmsg.time       = 0;
        qmsg.ptl.x      = 0;
        qmsg.ptl.y      = 0;

        ok = pQ->Add(&qmsg);
        F_DbgCtxExitf(ctx, "%d (hwnd=%lu, msg=0x%lx, iHab=%lu)",
                      ok ? 1 : 0, (unsigned long)a->hwnd,
                      (unsigned long)a->msg,
                      (unsigned long)pHab->GetIHab());
        return ok ? 1 : 0;
    }

    case NWinGetMsg: {
        /* Async: pArgs = WinGetMsg_Args. Blocks until a message is
         * available on the caller's HAB queue. */
        PWinGetMsg_Args a = (PWinGetMsg_Args)pArgs;
        Fs_Hab   *pHab = pThread->GetHab();
        Fs_Queue *pQ   = pHab->GetQueue();
        QMSG      qmsg;
        int       ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinGetMsg(recvobj=%p, cbArgs=%u)",
                             pRecvobj, (unsigned)cbArgs);

        if (!a || cbArgs < sizeof(*a)) {
            F_DbgCtxExitf(ctx, "0 (bad args)");
            return 0;
        }
        if (!pQ) {
            F_DbgCtxExitf(ctx, "0 (no queue)");
            return 0;
        }

        if (!pQ->Get(&qmsg)) {
            F_DbgCtxExitf(ctx, "0 (Get failed)");
            return 0;
        }

        /* WM_QUIT terminates the loop without being delivered to the
         * caller (EDM2 WinGetMsg: returns FALSE if WM_QUIT is
         * received). Filtering (hwndFilter / msgFirst / msgLast) is
         * not implemented yet; the values are accepted and ignored. */
        if (qmsg.msg == WM_QUIT) {
            F_DbgCtxExitf(ctx, "0 (WM_QUIT)");
            return 0;
        }

        a->qmsg = qmsg;
        if (pcbPayload) *pcbPayload = sizeof(*a);
        F_DbgCtxExitf(ctx, "1 (hwnd=%lu, msg=0x%lx)",
                      (unsigned long)qmsg.hwnd, (unsigned long)qmsg.msg);
        return 1;
    }

    case NWinDispatchMsg: {
        /* Async: pArgs = WinDispatchMsg_Args. Calls the server-side
         * default window procedure; result travels in ulResult. */
        PWinDispatchMsg_Args a = (PWinDispatchMsg_Args)pArgs;
        MRESULT   mres;
        int       ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinDispatchMsg(recvobj=%p, "
                             "cbArgs=%u)", pRecvobj, (unsigned)cbArgs);

        if (!a || cbArgs < sizeof(*a)) {
            F_DbgCtxExitf(ctx, "0 (bad args)");
            return 0;
        }

        mres = m_wnds.DefWindowProc(a->qmsg.hwnd, a->qmsg.msg,
                                     a->qmsg.mp1, a->qmsg.mp2);
        F_DbgCtxExitf(ctx, "0x%lx (hwnd=%lu, msg=0x%lx)",
                      (unsigned long)mres,
                      (unsigned long)a->qmsg.hwnd,
                      (unsigned long)a->qmsg.msg);
        return (ULONG)mres;
    }

    default:
        return 0;
    }
}
