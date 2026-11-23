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
#include <string.h>

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

/* ---- Window lifecycle proxies (PMMERGE) ---- */

/*! @brief 16-byte wire response header. */
struct FPM_RESP_HEADER {
    ULONG ulStatus;
    ULONG ulResult;
    ULONG cbPayload;
    ULONG ulLastError;
};

/*! @brief Send one async WIN command with payload; return server ulResult. */
static ULONG SendWinAsync(ULONG fnCode, ULONG data, const void *args, ULONG cbArgs)
{
    struct FPM_RESP_HEADER resp;
    LONG  received = 0;
    ULONG cb = cbArgs;

    if (Fc_SesPostCommand(F_CMD_WIN(fnCode), data) != 0) return 0;
    if (Fc_SesPostData(&cb, sizeof(cb)) != 0) return 0;
    if (cb > 0 && args && Fc_SesPostData((PVOID)args, cb) != 0) return 0;

    if (Fc_SesGetData(&resp, &received, sizeof(resp)) != 0) return 0;
    if (received < (LONG)sizeof(resp)) return 0;
    if (resp.ulStatus != 0) return 0;
    return resp.ulResult;
}

/*!
 * @brief Create a window.
 *
 * Wire: hwndParent in `data`, variable-length payload.
 * Class/name lengths are explicit (no NUL in payload). pCtlData and
 * pPresParams are marshalled as opaque blobs; their own header (first
 * 2 bytes USHORT for ctlData, first 4 bytes ULONG for presParams)
 * gives the size, per WinCreateWindow contract.
 */
HWND APIENTRY WinCreateWindow(HWND hwndParent, PCSZ pszClass, PCSZ pszName,
                              ULONG flStyle,
                              LONG x, LONG y, LONG cx, LONG cy,
                              HWND hwndOwner, HWND hwndInsertBehind,
                              ULONG id, PVOID pCtlData, PVOID pPresParams)
{
    BYTE   buf[1024];
    WinCreateWindow_Args *a = (WinCreateWindow_Args *)buf;
    ULONG  off, cbClass, cbName;
    ULONG  cbCtlData    = 0;
    ULONG  cbPresParams = 0;
    BYTE   flPresent    = 0;
    HWND   hwnd;

    F_DbgEnterContext("class=%s title=%s style=0x%lx",
                            pszClass ? pszClass : "(null)",
                            pszName  ? pszName  : "(null)",
                            (unsigned long)flStyle);

    cbClass = pszClass ? (ULONG)strlen(pszClass) : 0;
    cbName  = pszName  ? (ULONG)strlen(pszName)  : 0;
    if (cbClass > 255) cbClass = 255;
    if (cbName  > 255) cbName  = 255;

    if (pCtlData) {
        cbCtlData = (ULONG)(*(USHORT *)pCtlData);
        if (cbCtlData > 0) flPresent |= WCRF_HAS_CTLDATA;
    }
    if (pPresParams) {
        cbPresParams = *(ULONG *)pPresParams;
        if (cbPresParams > 0) flPresent |= WCRF_HAS_PRESPARAMS;
    }

    if (sizeof(WinCreateWindow_Args)
            + cbClass + cbName
            + ((flPresent & WCRF_HAS_CTLDATA)    ? 2 + cbCtlData    : 0)
            + ((flPresent & WCRF_HAS_PRESPARAMS) ? 4 + cbPresParams : 0)
        > sizeof(buf)) {
        F_DbgExitContext("NULLHANDLE (payload too large)");
        return NULLHANDLE;
    }

    a->cbClass          = cbClass;
    a->cbName           = cbName;
    a->flStyle          = flStyle;
    a->x = x; a->y = y; a->cx = cx; a->cy = cy;
    a->hwndOwner        = hwndOwner;
    a->hwndInsertBehind = hwndInsertBehind;
    a->id               = id;
    a->flPresent        = flPresent;

    off = sizeof(WinCreateWindow_Args);
    if (cbClass) { memcpy(buf + off, pszClass, cbClass); off += cbClass; }
    if (cbName)  { memcpy(buf + off, pszName,  cbName);  off += cbName;  }

    if (flPresent & WCRF_HAS_CTLDATA) {
        *(USHORT *)(buf + off) = (USHORT)cbCtlData;
        off += 2;
        memcpy(buf + off, pCtlData, cbCtlData);
        off += cbCtlData;
    }
    if (flPresent & WCRF_HAS_PRESPARAMS) {
        *(ULONG *)(buf + off) = cbPresParams;
        off += 4;
        memcpy(buf + off, pPresParams, cbPresParams);
        off += cbPresParams;
    }

    hwnd = (HWND)SendWinAsync(NWinCreateWindow, (ULONG)hwndParent, buf, off);

    F_DbgExitContext("hwnd=%lu", (unsigned long)hwnd);
    return hwnd;
}

BOOL APIENTRY WinShowWindow(HWND hwnd, BOOL fShow)
{
    WinShowWindow_Args a;
    ULONG rc;

    F_DbgEnterContext("hwnd=%lu fShow=%d",
                            (unsigned long)hwnd, (int)fShow);
    a.hwnd  = hwnd;
    a.fShow = fShow ? TRUE : FALSE;
    rc = SendWinAsync(NWinShowWindow, 0, &a, sizeof(a));
    F_DbgExitContext("%s", rc == 1 ? "TRUE" : "FALSE");
    return (rc == 1) ? TRUE : FALSE;
}

BOOL APIENTRY WinSetWindowPos(HWND hwnd, HWND hwndInsertBehind,
                              LONG x, LONG y, LONG cx, LONG cy, ULONG fl)
{
    WinSetWindowPos_Args a;
    ULONG rc;

    F_DbgEnterContext("hwnd=%lu fl=0x%lx",
                            (unsigned long)hwnd, (unsigned long)fl);
    a.hwnd             = hwnd;
    a.hwndInsertBehind = hwndInsertBehind;
    a.x = x; a.y = y; a.cx = cx; a.cy = cy;
    a.fl = fl;
    rc = SendWinAsync(NWinSetWindowPos, 0, &a, sizeof(a));
    F_DbgExitContext("%s", rc == 1 ? "TRUE" : "FALSE");
    return (rc == 1) ? TRUE : FALSE;
}

BOOL APIENTRY WinDestroyWindow(HWND hwnd)
{
    ULONG rc;

    F_DbgEnterContext("hwnd=%lu", (unsigned long)hwnd);
    rc = Fc_SesSendCommand(F_CMD_WIN(NWinDestroyWindow), (ULONG)hwnd);
    F_DbgExitContext("%s", rc == 1 ? "TRUE" : "FALSE");
    return (rc == 1) ? TRUE : FALSE;
}
