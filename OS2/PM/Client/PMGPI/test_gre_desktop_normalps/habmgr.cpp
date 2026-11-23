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
#include "pmclient.h"

#define DBG_TAG "PMMERGE"

HAB APIENTRY WinInitialize(ULONG flOptions)
{
    PTIB ptib;
    PPIB ppib;
    ULONG iHab;

    F_DbgEnterContext("flOptions=%lu)", flOptions);

    if (Fc_SesConnect(NULL) != 0) {
        F_DbgExitContext("NULLHANDLE (Fc_SesConnect failed)");
        return NULLHANDLE;
    }

    DosGetInfoBlocks(&ptib, &ppib);
    if (ptib->tib_ptib2->tib2_ultid == 0) {
        Fc_SesDisconnect();
        F_DbgExitContext("NULLHANDLE (tid=0)");
        return NULLHANDLE;
    }

    iHab = Fc_SesSendCommand(F_CMD_WIN(NWinInitialize), ptib->tib_ptib2->tib2_ultid);
    if (iHab == 0 || iHab == (ULONG)-1) {
        Fc_SesDisconnect();
        F_DbgExitContext("NULLHANDLE (NWinInitialize, tid=%lu, rc=0x%lx)",
                      ptib->tib_ptib2->tib2_ultid, (unsigned long)iHab);
        return NULLHANDLE;
    }

    F_DbgExitContext("HAB=%lu (tid=%lu)", (unsigned long)iHab, ptib->tib_ptib2->tib2_ultid);
    return (HAB)iHab;
}

BOOL APIENTRY WinTerminate(HAB hab)
{
    ULONG rc;

    F_DbgEnterContext("hab=%lu", (unsigned long)hab);

    rc = Fc_SesSendCommand(F_CMD_WIN(NWinTerminate), (ULONG)hab);
    Fc_SesDisconnect();

    F_DbgExitContext("%s (rc=0x%lx)", (rc == 1) ? "TRUE" : "FALSE", (unsigned long)rc);
    return (rc == 1) ? TRUE : FALSE;
}

/*!
 * @brief Open a device context for a window.
 * @param [in] hwnd  Window handle.
 * @return Valid HDC on success, NULLHANDLE on error (invalid hwnd
 *         or transport failure).
 */
HDC APIENTRY WinOpenWindowDC(HWND hwnd)
{
    HDC hdc;

    F_DbgEnterContext("hwnd=%lu", (unsigned long)hwnd);

    hdc = (HDC)Fc_SesSendCommand(F_CMD_WIN(NWinOpenWindowDC), (ULONG)hwnd);
    if (hdc == (HDC)-1)
        hdc = NULLHANDLE;

    F_DbgExitContext("HDC=%lu", (unsigned long)hdc);
    return hdc;
}
