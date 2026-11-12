/*!
 * @file test_win_mq.cpp
 * @brief Message queue: create, post, get, dispatch, destroy.
 *
 * Covers the server-side MQ implementation and the full
 * post > get > dispatch cycle:
 *
 *   - WinCreateMsgQueue (sync)          -> HMQ, once per HAB
 *   - WinDestroyMsgQueue (sync)         -> BOOL, requires matching HMQ
 *   - WinPostMsg (async)                -> BOOL, requires queue on
 *                                          the window's owning HAB
 *   - WinGetMsg (async, blocking)       -> BOOL + QMSG; returns FALSE
 *                                          on WM_QUIT
 *   - WinDispatchMsg (async)            -> MRESULT; server calls
 *                                          Fs_WndList::DefWindowProc
 *
 * Message content round-trip is verified (hwnd / msg / mp1 / mp2).
 * WM_CLOSE dispatched through DefWindowProc destroys the target
 * window server-side. WM_QUIT terminates the loop without being
 * delivered into the QMSG.
 *
 * What this test does NOT cover:
 *   - WinPeekMsg (not implemented yet on the server).
 *   - WinSendMsg (synchronous dispatch without a queue).
 *   - hwndFilter / msgFirst / msgLast filtering (accepted by the
 *     server, not applied).
 *   - Cross-HAB posting (window owned by a different HAB).
 */
#define INCL_PM
#define INCL_WINWINDOWMGR
#define INCL_GPICONTROL
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>
#include <string.h>

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB   hab;
    HMQ   hmq;
    HMQ   hmq2;
    HWND  hwnd;
    HWND  hwnd2;
    QMSG  qmsg;
    BOOL  rc;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    /* 1. Create the message queue (sync, HMQ in ulResult). */
    hmq = WinCreateMsgQueue(hab, 0);
    check(hmq != NULLHANDLE, "WinCreateMsgQueue(hab, 0)");
    if (hmq == NULLHANDLE) goto terminate;

    /* 2. Second call must fail: PMERR_MSG_QUEUE_ALREADY_EXISTS. */
    hmq2 = WinCreateMsgQueue(hab, 0);
    check(hmq2 == NULLHANDLE, "WinCreateMsgQueue(again) = NULLHANDLE");
    if (hmq2 != NULLHANDLE) WinDestroyMsgQueue(hmq2);

    /* 3. Create a window to post to. */
    hwnd = WinCreateWindow(HWND_DESKTOP, "TestClass", "TestTitle",
                           WS_VISIBLE,
                           100, 100, 200, 100,
                           NULLHANDLE, NULLHANDLE, 0, NULL, NULL);
    check(hwnd != NULLHANDLE, "WinCreateWindow");
    if (hwnd == NULLHANDLE) goto destroy_mq;

    /* 4. Post a user message with non-zero params; GetMsg must return
     *    it verbatim (hwnd, msg, mp1, mp2 all round-trip through the
     *    server-side Fs_Queue). */
    rc = WinPostMsg(hwnd, WM_USER, (MPARAM)0x1111, (MPARAM)0x2222);
    check(rc, "WinPostMsg(hwnd, WM_USER, 0x1111, 0x2222)");

    memset(&qmsg, 0xEE, sizeof(qmsg));
    rc = WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0);
    check(rc, "WinGetMsg(WM_USER) = TRUE");
    if (rc) {
        check(qmsg.hwnd == hwnd,              "  qmsg.hwnd == hwnd");
        check(qmsg.msg  == WM_USER,           "  qmsg.msg  == WM_USER");
        check(qmsg.mp1  == (MPARAM)0x1111,    "  qmsg.mp1  == 0x1111");
        check(qmsg.mp2  == (MPARAM)0x2222,    "  qmsg.mp2  == 0x2222");

        /* Dispatch: server-side DefWindowProc has no handler for
         * WM_USER, returns 0 (unhandled). No crash. */
        (void)WinDispatchMsg(hab, &qmsg);
        check(1, "WinDispatchMsg(WM_USER) (no crash)");
    }

    /* 5. Post to an unknown HWND must fail. */
    rc = WinPostMsg((HWND)0xDEAD, WM_USER, 0, 0);
    check(!rc, "WinPostMsg(0xDEAD, WM_USER) = FALSE");

    /* 6. Post WM_CLOSE; dispatch runs DefWindowProc which destroys
     *    the window server-side. Verified afterwards by a failed
     *    WinPostMsg (the HWND is no longer in the WndList). */
    rc = WinPostMsg(hwnd, WM_CLOSE, 0, 0);
    check(rc, "WinPostMsg(hwnd, WM_CLOSE)");

    memset(&qmsg, 0xEE, sizeof(qmsg));
    rc = WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0);
    check(rc, "WinGetMsg(WM_CLOSE) = TRUE");
    if (rc) {
        check(qmsg.msg == WM_CLOSE, "  qmsg.msg  == WM_CLOSE");
        (void)WinDispatchMsg(hab, &qmsg);
    }

    rc = WinPostMsg(hwnd, WM_USER, 0, 0);
    check(!rc, "WinPostMsg(hwnd after WM_CLOSE) = FALSE");

    /* 7. Create a fresh window so WM_QUIT has a valid target. */
    hwnd2 = WinCreateWindow(HWND_DESKTOP, "TestClass", "TestTitle2",
                            WS_VISIBLE,
                            150, 150, 200, 100,
                            NULLHANDLE, NULLHANDLE, 0, NULL, NULL);
    check(hwnd2 != NULLHANDLE, "WinCreateWindow(2)");
    if (hwnd2 == NULLHANDLE) goto destroy_mq;

    /* 8. WM_QUIT delivered to the calling thread's queue without a
     *    target window (EDM2 WinPostMsg: hwnd == NULLHANDLE places
     *    the message on the calling thread's queue). Terminates the
     *    loop without being delivered into the QMSG. */
    rc = WinPostMsg(NULLHANDLE, WM_QUIT, 0, 0);
    check(rc, "WinPostMsg(NULLHANDLE, WM_QUIT)");

    memset(&qmsg, 0xEE, sizeof(qmsg));
    rc = WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0);
    check(!rc, "WinGetMsg(WM_QUIT) = FALSE");
    check(qmsg.msg != WM_QUIT, "  qmsg.msg != WM_QUIT (not delivered)");

    check(WinDestroyWindow(hwnd2), "WinDestroyWindow(hwnd2)");

destroy_mq:
    /* 9. Destroy the queue (sync, data = hmq). */
    check(WinDestroyMsgQueue(hmq), "WinDestroyMsgQueue(hmq)");

    /* 10. Second destroy must fail. */
    check(!WinDestroyMsgQueue(hmq), "WinDestroyMsgQueue(hmq again) = FALSE");

terminate:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
