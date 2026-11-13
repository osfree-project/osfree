/*!
 * @file dispatch_win_hab.cpp
 * @brief Win* HAB lifecycle dispatcher (Fs_Server method).
 *
 * HAB lookup follows OS/2 PM Reference (WinInitialize, Remarks):
 * "the operating system does not generally use the information
 *  supplied by the hab parameter; instead, it deduces it from the
 *  identity of the thread that is making the call."
 *
 * Navigation: recvobj -> Fs_Connection -> Fs_Thread -> Fs_Process.
 * HAB lives in Fs_Server::m_habs, keyed by iHab; Fs_Thread holds
 * a back-ref (m_pHab).
 *
 * Strict mode (m_cfg.fStrictHab == TRUE): additionally verifies that
 * the client-supplied `data` matches the thread's own HAB. Off by
 * default; used for testing / hardening.
 *
 * flOptions is not carried over the wire: in PM it is always 0.
 *
 * Sync path (fnCode 0x0001-0x000F): no cbArgs read, reply is a bare
 * 4-byte ulResult, matching Fc_TrnSendCommand's contract (Fc_trn.cpp).
 */

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"
#include "Fs_server.hpp"
#include "F_debug.h"

/*!
 * @brief Dispatch NWin* HAB lifecycle commands.
 *
 * Handles three sync commands with no payload:
 *   NWinInitialize    data = tid  -> return iHab (0 = rejected)
 *   NWinTerminate     data = hab  -> return BOOL (1 = destroyed)
 *   NWinGetLastError  data = hab  -> return PMERR_* (0 = no error)
 *
 * In non-strict mode the client-supplied `data` for Terminate /
 * GetLastError is ignored; the thread's own HAB is used. In strict
 * mode a mismatch rejects the command.
 *
 * @param [in]  pRecvobj   Transport connection handle (Fs_Connection key).
 * @param [in]  fnCode     NWin* opcode (NWinInitialize / NWinTerminate /
 *                         NWinGetLastError).
 * @param [in]  data       Primary argument: tid for Initialize, hab for
 *                         Terminate / GetLastError (informational in
 *                         non-strict mode).
 * @param [in]  pArgs      Unused (sync path has no payload).
 * @param [in]  cbArgs     Unused (sync path has no payload).
 * @param [out] pcbPayload Receives reply payload size (always 0).
 * @return NWinInitialize:   iHab (0 on failure).
 *         NWinTerminate:    1 on success, 0 on failure.
 *         NWinGetLastError: PMERR_* (0 if no error).
 * @retval 0  Failure: no process/thread, tid=0, or Create failed for
 *            NWinInitialize; no HAB, strict mismatch, or Destroy failed
 *            for NWinTerminate; no process/thread or no HAB for
 *            NWinGetLastError (the latter returns PMERR_INVALID_HAB
 *            instead of 0).
 * @retval 1  NWinTerminate succeeded.
 */
ULONG Fs_Server::DispatchWinHab(void *pRecvobj, ULONG fnCode, ULONG data,
                                 PBYTE pArgs, ULONG cbArgs,
                                 PULONG pcbPayload)
{
    Fs_Connection *pConn   = m_conns.FindByRecvobj(pRecvobj);
    Fs_Thread     *pThread = pConn ? pConn->GetThread() : NULL;
    Fs_Process    *pProc   = pThread ? pThread->GetProcess() : NULL;

    (void)pArgs; (void)cbArgs;
    if (pcbPayload) *pcbPayload = 0;

    switch (fnCode) {

    case NWinInitialize: {
        ULONG tid   = data;
        ULONG iHab  = 0;
        Fs_Hab *pHab;
        int    ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinInitialize(recvobj=%p, tid=%lu)",
                             pRecvobj, (unsigned long)tid);

        if (!pProc || !pThread) {
            F_DbgCtxExitf(ctx, "NULL (no process/thread)"); return 0;
        }
        if (tid == 0) {
            F_DbgCtxExitf(ctx, "NULL (tid=0)"); return 0;
        }
        if (pThread->GetHab() != NULL) {
            F_DbgCtxExitf(ctx, "NULL (already had HAB iHab=%lu)",
                          (unsigned long)pThread->GetIHab());
            return 0;
        }

        pHab = m_habs.Create(pProc->GetIdClient(), tid, &iHab);
        if (!pHab) {
            F_DbgCtxExitf(ctx, "NULL (Create failed)"); return 0;
        }
        pThread->SetHab(pHab);
        F_DbgCtxExitf(ctx, "iHab=%lu", (unsigned long)iHab);
        return iHab;
    }

    case NWinTerminate: {
        ULONG iHab;
        int   ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinTerminate(recvobj=%p, hab=%lu, "
                             "strict=%d)",
                             pRecvobj, (unsigned long)data,
                             (int)m_cfg.fStrictHab);

        if (!pProc || !pThread || !pThread->GetHab()) {
            F_DbgCtxExitf(ctx, "0 (no HAB)"); return 0;
        }
        iHab = pThread->GetIHab();
        if (m_cfg.fStrictHab && data != iHab) {
            F_DbgCtxExitf(ctx, "0 (strict: hab=%lu != iHab=%lu)",
                          (unsigned long)data, (unsigned long)iHab);
            return 0;
        }
        if (!m_habs.Destroy(pProc->GetIdClient(), pThread->GetTid(), iHab)) {
            F_DbgCtxExitf(ctx, "0 (Destroy failed)"); return 0;
        }
        pThread->ClearHab();
        F_DbgCtxExitf(ctx, "1 (iHab=%lu)", (unsigned long)iHab);
        return 1;
    }

    case NWinGetLastError: {
        ULONG  iHab;
        Fs_Hab *pHab;
        int    ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinGetLastError(recvobj=%p, hab=%lu, "
                             "strict=%d)",
                             pRecvobj, (unsigned long)data,
                             (int)m_cfg.fStrictHab);

        if (!pThread || !pThread->GetHab()) {
            F_DbgCtxExitf(ctx, "PMERR_INVALID_HAB (no HAB)");
            return (ULONG)PMERR_INVALID_HAB;
        }
        iHab = pThread->GetIHab();
        if (m_cfg.fStrictHab && data != iHab) {
            F_DbgCtxExitf(ctx, "PMERR_INVALID_HAB (strict: hab=%lu != iHab=%lu)",
                          (unsigned long)data, (unsigned long)iHab);
            return (ULONG)PMERR_INVALID_HAB;
        }
        pHab = pThread->GetHab();
        F_DbgCtxExitf(ctx, "%d", pHab->GetLastError());
        return (ULONG)pHab->GetLastError();
    }

    default:
        return 0;
    }
}
