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

#define INCL_PM
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

HMQ APIENTRY WinCreateMsgQueue(HAB hab, LONG cmsg)
{
    (void)cmsg;  /* PM v3+: lQueuesize is ignored */
    return (HMQ)Fc_SesSendCommand(F_CMD_WIN(NWinCreateMsgQueue), (ULONG)hab);
}

BOOL APIENTRY WinDestroyMsgQueue(HMQ hmq)
{
    return (BOOL)Fc_SesSendCommand(F_CMD_WIN(NWinDestroyMsgQueue), (ULONG)hmq);
}

/*!
 * @brief Post a message to a window's message queue.
 */
BOOL WinPostMsg(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    WinPostMsg_Args a;
    ULONG rc;

    a.hwnd = hwnd;
    a.msg  = msg;
    a.mp1  = mp1;
    a.mp2  = mp2;

    rc = SendWinAsync(NWinPostMsg, (ULONG)hwnd, &a, sizeof(a));
    return (rc == 1) ? TRUE : FALSE;
}

/*!
 * @brief Retrieve the next message from the calling thread's queue.
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

/*!
 * @brief Begin a paint cycle for a window.
 */
HPS APIENTRY WinBeginPaint(HWND hwnd, HPS hps, PRECTL prcl)
{
    WinBeginPaint_Args a;
    ULONG rc;

    F_DbgEnterContext("hwnd=%lu", (unsigned long)hwnd);

    a.hwnd = hwnd;
    a.hps  = hps;
    a.rcl.xLeft   = 0;
    a.rcl.yBottom = 0;
    a.rcl.xRight  = 0;
    a.rcl.yTop    = 0;

    rc = SendWinAsyncData(NWinBeginPaint, (ULONG)hwnd, &a, sizeof(a));
    if (rc == 0) {
        F_DbgExitContext("NULLHANDLE (rc=0)");
        return NULLHANDLE;
    }

    if (prcl) *prcl = a.rcl;

    F_DbgExitContext("hps=%p (rcl={%ld,%ld,%ld,%ld})",
                     (void*)a.hps,
                     (long)a.rcl.xLeft, (long)a.rcl.yBottom,
                     (long)a.rcl.xRight, (long)a.rcl.yTop);
    return a.hps;
}

/*!
 * @brief End a paint cycle; destroys the PS returned by WinBeginPaint.
 */
BOOL APIENTRY WinEndPaint(HPS hps)
{
    WinEndPaint_Args a;
    ULONG rc;

    F_DbgEnterContext("hps=%p", (void*)hps);

    a.hps = hps;

    rc = SendWinAsync(NWinEndPaint, (ULONG)hps, &a, sizeof(a));
    F_DbgExitContext("%s", rc == 1 ? "TRUE" : "FALSE");
    return (rc == 1) ? TRUE : FALSE;
}

/*!
 * @brief Create a standard frame window (frame + titlebar + client).
 *
 * Mirrors OS/2 PM WinCreateStdWindow:
 *   - flFrameStyle is flStyle (WS_* only),
 *   - flCreateFlags is in/out (FCF_*; on success *pflCreateFlags is
 *     updated by the server to what was actually honoured),
 *   - pszClientClass == NULL creates no client window,
 *   - phwndClient receives the client HWND (NULLHANDLE if none).
 *
 * @param [in]     hwndParent       Parent window (HWND_DESKTOP for main).
 * @param [in]     flStyle          Frame WS_* style.
 * @param [in,out] pflCreateFlags   FCF_* creation flags (in/out).
 * @param [in]     pszClientClass   Client window class, or NULL.
 * @param [in]     pszTitle         Frame title.
 * @param [in]     flClientStyle    Client WS_* style.
 * @param [in]     hmod             Resource module (unused).
 * @param [in]     idResources      Resource id (unused).
 * @param [out]    phwndClient      Receives the client HWND.
 * @return Frame HWND, or NULLHANDLE on failure.
 * @retval NULLHANDLE  Bad arguments, unsupported FCF_*, or transport
 *                     error.
 * @retval !=NULLHANDLE Frame HWND on success.
 */
HWND APIENTRY WinCreateStdWindow(HWND hwndParent, ULONG flStyle,
                                 PULONG pflCreateFlags,
                                 PCSZ pszClientClass, PCSZ pszTitle,
                                 ULONG flClientStyle, HMODULE hmod,
                                 ULONG idResources, HWND *phwndClient)
{
    BYTE   buf[1024];
    WinCreateStdWindow_Args *a = (WinCreateStdWindow_Args *)buf;
    ULONG  off, cbClass, cbTitle;
    ULONG  rc;

    F_DbgEnterContext("parent=%lu style=0x%lx flags=0x%lx class=%s",
                      (unsigned long)hwndParent,
                      (unsigned long)flStyle,
                      (unsigned long)(pflCreateFlags ? *pflCreateFlags : 0),
                      pszClientClass ? pszClientClass : "(null)");

    if (phwndClient) *phwndClient = NULLHANDLE;

    cbClass = pszClientClass ? (ULONG)strlen(pszClientClass) : 0;
    cbTitle = pszTitle       ? (ULONG)strlen(pszTitle)       : 0;
    if (cbClass > 255) cbClass = 255;
    if (cbTitle > 255) cbTitle = 255;

    if (sizeof(WinCreateStdWindow_Args) + cbClass + cbTitle > sizeof(buf)) {
        F_DbgExitContext("NULLHANDLE (payload too large)");
        return NULLHANDLE;
    }

    a->hwndParent    = hwndParent;
    a->flStyle       = flStyle;
    a->flCreateFlags = pflCreateFlags ? *pflCreateFlags : 0;
    a->flClientStyle = flClientStyle;
    a->hmod          = hmod;
    a->idResources   = idResources;
    a->cbClass       = cbClass;
    a->cbTitle       = cbTitle;
    a->hwndClient    = NULLHANDLE;

    off = sizeof(*a);
    if (cbClass) { memcpy(buf + off, pszClientClass, cbClass); off += cbClass; }
    if (cbTitle) { memcpy(buf + off, pszTitle,       cbTitle); off += cbTitle; }

    rc = SendWinAsyncData(NWinCreateStdWindow, 0, buf, off);
    if (rc == 0) {
        F_DbgExitContext("NULLHANDLE (rc=0)");
        return NULLHANDLE;
    }

    if (pflCreateFlags) *pflCreateFlags = a->flCreateFlags;
    if (phwndClient)    *phwndClient    = a->hwndClient;

    F_DbgExitContext("hwndFrame=%lu, hwndClient=%lu",
                     (unsigned long)rc,
                     (unsigned long)a->hwndClient);
    return (HWND)rc;
}

/*!
 * @brief Return the child window of hwndParent with the given id.
 *
 * Mirrors OS/2 PM WinWindowFromID. Returns NULLHANDLE if no such child.
 *
 * @param [in] hwndParent  Parent window.
 * @param [in] id          Window id (FID_TITLEBAR, FID_CLIENT, ...).
 * @return Child HWND, or NULLHANDLE.
 * @retval NULLHANDLE  No child with that id.
 * @retval !=NULLHANDLE Matching child HWND.
 */
HWND APIENTRY WinWindowFromID(HWND hwndParent, ULONG id)
{
    WinWindowFromID_Args a;
    ULONG rc;

    F_DbgEnterContext("parent=%lu id=0x%lx",
                      (unsigned long)hwndParent, (unsigned long)id);

    a.hwndParent = hwndParent;
    a.id         = id;

    rc = SendWinAsync(NWinWindowFromID, 0, &a, sizeof(a));
    F_DbgExitContext("hwnd=%lu", (unsigned long)rc);
    return (HWND)rc;
}
