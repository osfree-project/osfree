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
 * @brief Send one async WIN command; copy the reply payload back
 *        into `args` in place.
 *
 * The counterpart of SendWinAsync for commands whose server-side
 * reply includes an updated copy of the input argument structure
 * (currently NWinGetMsg, whose out QMSG is written back into
 * WinGetMsg_Args.qmsg). Uses cbArgs on the way out and
 * FPM_RESP_HEADER.cbPayload on the way back, matching HandleWinCall's
 * "in-place payload" contract.
 *
 * @param [in]     fnCode  NWin* opcode (async, no NWIN_SYNC_FLAG).
 * @param [in]     data    Primary handle for Fc_SesPostCommand.
 * @param [in,out] args    Input payload; receives the reply payload
 *                         when cbPayload > 0.
 * @param [in]     cbArgs  Size of `args` in bytes.
 * @return Server ulResult; 0 on transport error or cbPayload too large.
 * @retval 0  Transport error, status != 0, or reply larger than cbArgs.
 * @retval >0 Server ulResult (BOOL, handle, or MRESULT).
 */
static ULONG SendWinAsyncData(ULONG fnCode, ULONG data,
                               void *args, ULONG cbArgs)
{
    struct FPM_RESP_HEADER resp;
    LONG  received = 0;
    LONG  len      = 0;
    ULONG cb       = cbArgs;

    if (Fc_SesPostCommand(F_CMD_WIN(fnCode), data) != 0) return 0;
    if (Fc_SesPostData(&cb, sizeof(cb)) != 0) return 0;
    if (cb > 0 && args && Fc_SesPostData(args, cb) != 0) return 0;

    if (Fc_SesGetData(&resp, &received, sizeof(resp)) != 0) return 0;
    if (received < (LONG)sizeof(resp)) return 0;
    if (resp.ulStatus != 0) return 0;

    if (resp.cbPayload > 0) {
        if (!args || resp.cbPayload > cbArgs) return 0;
        if (Fc_SesGetData(args, &len, (LONG)resp.cbPayload) != 0) return 0;
    }
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

/*!
 * @brief Retrieve the next message from the calling thread's queue.
 *
 * Blocks until a message is available (the server side blocks on
 * Fs_Queue::Get via an event semaphore). Returns FALSE if WM_QUIT was
 * received; per EDM2 WinGetMsg, WM_QUIT terminates the loop without
 * being delivered to the caller.
 *
 * Message filtering (hwndFilter / msgFirst / msgLast) is accepted for
 * API compatibility but not applied yet.
 *
 * @param [in]  hab        Anchor block of the calling thread.
 * @param [out] pqmsg      Receives the extracted message.
 * @param [in]  hwndFilter Window filter (0 = any window).
 * @param [in]  msgFirst   First message id (0 = WM_FIRST).
 * @param [in]  msgLast    Last message id (0 = WM_LAST).
 * @return TRUE if a message was retrieved, FALSE otherwise.
 * @retval TRUE   Message delivered into *pqmsg.
 * @retval FALSE  pqmsg == NULL, no queue on the calling HAB, transport
 *                error, or WM_QUIT was received (no message delivered).
 */
BOOL APIENTRY WinGetMsg(HAB hab, PQMSG pqmsg, HWND hwndFilter,
                        ULONG msgFirst, ULONG msgLast)
{
    WinGetMsg_Args a;
    ULONG rc;

    F_DbgEnterContext("hab=%lu filter=%lu first=0x%lx last=0x%lx",
                      (unsigned long)hab, (unsigned long)hwndFilter,
                      (unsigned long)msgFirst, (unsigned long)msgLast);

    if (!pqmsg) {
        F_DbgExitContext("FALSE (pqmsg NULL)");
        return FALSE;
    }

    a.iHab       = (ULONG)hab;
    a.hwndFilter = hwndFilter;
    a.msgFirst   = msgFirst;
    a.msgLast    = msgLast;
    /* a.qmsg is out; zero it so a failed Get is observably "no message". */
    a.qmsg.hwnd       = NULLHANDLE;
    a.qmsg.msg        = 0;
    a.qmsg.mp1        = 0;
    a.qmsg.mp2        = 0;
    a.qmsg.time       = 0;
    a.qmsg.ptl.x      = 0;
    a.qmsg.ptl.y      = 0;

    rc = SendWinAsyncData(NWinGetMsg, 0, &a, sizeof(a));
    if (rc != 1) {
        F_DbgExitContext("FALSE (rc=0x%lx)", (unsigned long)rc);
        return FALSE;
    }

    *pqmsg = a.qmsg;
    F_DbgExitContext("TRUE (hwnd=%lu, msg=0x%lx)",
                     (unsigned long)a.qmsg.hwnd,
                     (unsigned long)a.qmsg.msg);
    return TRUE;
}

/*!
 * @brief Dispatch a message to its target window's procedure.
 *
 * Sends the QMSG to the server, which calls the server-side default
 * window procedure (Fs_WndList::DefWindowProc) and returns the
 * message result. Until per-class Server*WndProc implementations
 * appear (Phase 5 / D3), every message goes through the default
 * handler. The server-side default is the analogue of PMWIN's
 * WinDefWindowProc.
 *
 * @param [in] hab    Anchor block of the calling thread.
 * @param [in] pqmsg  Message to dispatch.
 * @return Message result (MRESULT); 0 on transport error.
 * @retval 0  Transport error, or handled with a zero result.
 * @retval !=0 Message result returned by the window procedure.
 */
MRESULT APIENTRY WinDispatchMsg(HAB hab, PQMSG pqmsg)
{
    WinDispatchMsg_Args a;
    ULONG rc;

    F_DbgEnterContext("hab=%lu", (unsigned long)hab);

    if (!pqmsg) {
        F_DbgExitContext("0 (pqmsg NULL)");
        return 0;
    }

    a.iHab = (ULONG)hab;
    a.qmsg = *pqmsg;

    rc = SendWinAsync(NWinDispatchMsg, 0, &a, sizeof(a));
    F_DbgExitContext("0x%lx (hwnd=%lu, msg=0x%lx)",
                     (unsigned long)rc,
                     (unsigned long)pqmsg->hwnd,
                     (unsigned long)pqmsg->msg);
    return (MRESULT)rc;
}
