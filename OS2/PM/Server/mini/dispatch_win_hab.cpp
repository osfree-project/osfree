/*!
 * @file dispatch_win_hab.cpp
 * @brief Win* HAB lifecycle dispatcher (Phase 2).
 *
 * Implements NWinInitialize, NWinTerminate, NWinGetLastError.
 * Return value goes through ulResult; pArgs/pcbPayload are unused.
 *
 * NWinInitialize:   data = tid  (in), ulResult = iHab   (out)
 * NWinTerminate:    data = iHab (in), ulResult = BOOL   (out)
 * NWinGetLastError: data = iHab (in), ulResult = PMERR_* (out)
 *
 * Sync path: HandleWinCall in Fs_server.cpp routes 0x0001-0x000F
 * directly here without reading a cbArgs header; reply is a 4-byte
 * ulResult, matching Fc_TrnSendCommand's contract (Fc_trn.cpp).
 * flOptions is not carried over the wire.
 */
#include "dispatch_win.hpp"
#include "F_debug.h"

ULONG DispatchWinHab(Fs_ProcessList *pProcs, void *pRecvobj,
                     ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload)
{
    (void)pArgs; (void)cbArgs;
    if (pcbPayload) *pcbPayload = 0;
    if (!pProcs) return 0;

    switch (fnCode) {

    case NWinInitialize: {
        Fs_Process          *pProc;
        Fs_ClientConnection *pThread;
        ULONG                iHab = 0;
        int                  ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinInitialize(recvobj=%p, tid=%lu)",
                             pRecvobj, (unsigned long)data);

        pProc = pProcs->FindByConn(pRecvobj);
        if (!pProc) { F_DbgCtxExitf(ctx, "NULL (no process)"); return 0; }
        pThread = pProc->FindThreadByConn(pRecvobj);
        if (!pThread) { F_DbgCtxExitf(ctx, "NULL (no thread)"); return 0; }

        if (!pProc->AttachHab(pThread, data, &iHab)) {
            F_DbgCtxExitf(ctx, "NULL (AttachHab failed)");
            return 0;
        }
        F_DbgCtxExitf(ctx, "iHab=%lu", (unsigned long)iHab);
        return iHab;
    }

    case NWinTerminate: {
        Fs_Process *pProc;
        int ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinTerminate(iHab=%lu)",
                             (unsigned long)data);

        pProc = pProcs->FindByConn(pRecvobj);
        if (!pProc) { F_DbgCtxExitf(ctx, "GPI_ERROR (no process)"); return 0; }
        if (pProc->DetachHab(data)) {
            F_DbgCtxExitf(ctx, "1");
            return 1;
        }
        F_DbgCtxExitf(ctx, "GPI_ERROR (iHab not found)");
        return 0;
    }

    case NWinGetLastError: {
        Fs_Hab *pHab;
        pHab = pProcs->FindHabByIHab(data);
        if (!pHab) return (ULONG)PMERR_INVALID_HAB;
        return (ULONG)pHab->GetLastError();
    }

    default:
        return 0;
    }
}
