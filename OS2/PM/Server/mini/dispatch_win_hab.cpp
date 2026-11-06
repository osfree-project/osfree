/*!
 * @file dispatch_win_hab.cpp
 * @brief Win* HAB lifecycle dispatcher (Phase 2).
 *
 * Implements NWinInitialize, NWinTerminate, NWinGetLastError.
 * Return value goes through ulResult; pArgs/pcbPayload are unused.
 *
 * NWinInitialize:  data = flOptions (in), ulResult = iHab (out)
 * NWinTerminate:   data = iHab       (in), ulResult = BOOL  (out)
 * NWinGetLastError: data = iHab      (in), ulResult = PMERR_* (out)
 */
#include "dispatch_win.hpp"
#include "F_debug.h"

ULONG DispatchWinHab(Fs_HabList *pHabs, ULONG idClient,
                     ULONG fnCode, ULONG data,
                     PBYTE pArgs, ULONG cbArgs, PULONG pcbPayload)
{
    (void)pArgs; (void)cbArgs;
    if (pcbPayload) *pcbPayload = 0;
    if (!pHabs) return 0;

    switch (fnCode) {

    case NWinInitialize: {
        ULONG iHab = 0;
        int ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinInitialize(idClient=%lu, flOptions=0x%lx)",
                             (unsigned long)idClient, (unsigned long)data);

        if (!pHabs->Create(idClient, &iHab)) {
            F_DbgCtxExitf(ctx, "NULL (table full)");
            return 0;
        }
        F_DbgCtxExitf(ctx, "iHab=%lu", (unsigned long)iHab);
        return iHab;
    }

    case NWinTerminate: {
        int ctx;

        ctx = F_DbgCtxEnterf(F_DBG_SEC_DISPATCH, F_DBG_LVL_VERBOSE,
                             "[PMSERVER] NWinTerminate(iHab=%lu)",
                             (unsigned long)data);
        if (pHabs->Destroy(data)) {
            F_DbgCtxExitf(ctx, "1");
            return 1;
        }
        F_DbgCtxExitf(ctx, "GPI_ERROR (iHab not found)");
        return 0;
    }

    case NWinGetLastError: {
        Fs_Hab *pHab;

        pHab = pHabs->FindByIHab(data);
        if (!pHab) return (ULONG)PMERR_INVALID_HAB;
        return (ULONG)pHab->GetLastError();
    }

    default:
        /* NWinQueryAnchorBlock, NWinQueryVersion — Phase 4 */
        return 0;
    }
}
