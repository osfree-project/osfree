/*! @file habmgr.cpp
    @brief Client-side WinInitialize / WinTerminate.

    Two PM-API entry points. Both delegate to PMCLIENT.DLL (Fc_Ses*)
    and to the WIN wire class (win_wire.h). No local HAB table: the
    server owns anchor blocks, the client only keeps the HAB value and
    passes it back on WinTerminate.

    Wire contract (sync path 0x0001-0x000F in HandleWinCall):

        NWinInitialize    data = client tid
                          reply = 4 bytes, iHab (0 = rejected)
        NWinTerminate     data = iHab
                          reply = 4 bytes, BOOL (1 = destroyed)
        NWinGetLastError  data = iHab
                          reply = 4 bytes, PMERR_* (0 = no error)

    Transport errors surface as (APIRET)-1 from Fc_SesSendCommand.

    Per-thread model: Fc_SesConnect() opens the calling thread's slot.
    A thread may call WinInitialize at most once between WinTerminate
    calls; the second call returns NULLHANDLE because Fc_SesConnect
    rejects an already-active slot.

    flOptions is accepted for API compatibility but not sent over the
    wire: in PM it is always 0, and the server does not read it.
*/

#include "F_platform.h"
#include "F_cmd.h"
#include "F_debug.h"
#include "win_wire.h"
#include "pmclient.h"

/*! @brief TID of the calling thread. */
static ULONG QueryClientTid(void)
{
    PTIB ptib;
    PPIB ppib;
    DosGetInfoBlocks(&ptib, &ppib);
    return (ULONG)ptib->tib_ptib2->tib2_ultid;
}

/*! @brief Send one sync WIN command; return its ulResult.
    @return Server ulResult, or (APIRET)-1 on transport error. */
static ULONG SendWinSync(ULONG fnCode, ULONG data)
{
    return Fc_SesSendCommand(F_CMD_WIN(fnCode), data);
}

HAB APIENTRY WinInitialize(ULONG flOptions)
{
    ULONG tid;
    ULONG iHab;
    int   ctx;

    (void)flOptions;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: WinInitialize");
    if (Fc_SesConnect(NULL) != 0) {
        F_DbgCtxExitf(ctx, "NULLHANDLE (Fc_SesConnect failed)");
        return NULLHANDLE;
    }

    tid = QueryClientTid();
    if (tid == 0) {
        Fc_SesDisconnect();
        F_DbgCtxExitf(ctx, "NULLHANDLE (tid=0)");
        return NULLHANDLE;
    }

    iHab = SendWinSync(NWinInitialize, tid);
    if (iHab == 0 || iHab == (ULONG)-1) {
        Fc_SesDisconnect();
        F_DbgCtxExitf(ctx, "NULLHANDLE (NWinInitialize, tid=%lu, rc=0x%lx)",
                      (unsigned long)tid, (unsigned long)iHab);
        return NULLHANDLE;
    }

    F_DbgCtxExitf(ctx, "HAB=%lu (tid=%lu)",
                  (unsigned long)iHab, (unsigned long)tid);
    return (HAB)iHab;
}

BOOL APIENTRY WinTerminate(HAB hab)
{
    ULONG rc;
    int   ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMCLIENT, F_DBG_LVL_VERBOSE,
                         "PMC: WinTerminate(hab=%lu)",
                         (unsigned long)hab);

    rc = SendWinSync(NWinTerminate, (ULONG)hab);
    Fc_SesDisconnect();

    F_DbgCtxExitf(ctx, "%d (rc=0x%lx)",
                  (rc == 1) ? 1 : 0, (unsigned long)rc);
    return (rc == 1) ? TRUE : FALSE;
}
