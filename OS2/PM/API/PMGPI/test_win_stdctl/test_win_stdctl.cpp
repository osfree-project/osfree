/*!
 * @file test_win_stdctl.cpp
 * @brief S3: standard window-class procedures (WC_FRAME, WC_TITLEBAR)
 *        and DefWindowProc fallback.
 *
 * Checks that:
 *   - WM_PAINT for WC_FRAME / WC_TITLEBAR / TestClass does not destroy
 *     the window;
 *   - WM_CLOSE for WC_FRAME destroys the window (via FrameWndProc);
 *   - WM_CLOSE for TestClass destroys the window (via DefWindowProc
 *     fallback);
 *   - WM_PAINT for TestClass leaves the window alive (DefWindowProc -> 0).
 *
 * The actual drawing is verified from the server log
 * ([GRE2SDL] Gre2SdlBoxBoundary for frame; Gre2SdlBoxInterior plus two
 * Gre2SdlPolyLine calls for titlebar).
 */
#define INCL_PM
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>


static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

static HWND create(HWND parent, const char *cls, const char *title,
                   LONG x, LONG y, LONG cx, LONG cy)
{
    return WinCreateWindow(parent, cls, title,
                           WS_VISIBLE,
                           x, y, cx, cy,
                           NULLHANDLE, HWND_TOP, 0, NULL, NULL);
}

static void post_and_dispatch(HAB hab, HWND hwnd, ULONG msg)
{
    QMSG qmsg;
    BOOL rc;

    rc = WinPostMsg(hwnd, msg, 0, 0);
    check(rc, "WinPostMsg");
    if (!rc) return;

    rc = WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0);
    check(rc, "WinGetMsg");
    if (!rc) return;

    (void)WinDispatchMsg(hab, &qmsg);
}

int main(void)
{
    HAB   hab;
    HMQ   hmq;
    HWND  hwnd;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    hmq = WinCreateMsgQueue(hab, 0);
    check(hmq != NULLHANDLE, "WinCreateMsgQueue");
    if (hmq == NULLHANDLE) goto terminate;

    /* S3-1: WC_FRAME + WM_PAINT -> window stays alive. */
    hwnd = create(HWND_DESKTOP, "WC_FRAME", "Frame", 50, 50, 200, 100);
    check(hwnd != NULLHANDLE, "S3-1: create WC_FRAME");
    if (hwnd) {
        check(WinOpenWindowDC(hwnd) != NULLHANDLE, "S3-1: WinOpenWindowDC");
        post_and_dispatch(hab, hwnd, WM_PAINT);
        check(WinDestroyWindow(hwnd), "S3-1: window alive after WM_PAINT");
    }

    /* S3-2: WC_TITLEBAR + WM_PAINT -> window stays alive. */
    hwnd = create(HWND_DESKTOP, "WC_TITLEBAR", "Bar", 50, 50, 200, 20);
    check(hwnd != NULLHANDLE, "S3-2: create WC_TITLEBAR");
    if (hwnd) {
        check(WinOpenWindowDC(hwnd) != NULLHANDLE, "S3-2: WinOpenWindowDC");
        post_and_dispatch(hab, hwnd, WM_PAINT);
        check(WinDestroyWindow(hwnd), "S3-2: window alive after WM_PAINT");
    }

    /* S3-3: WC_FRAME + WM_CLOSE -> window destroyed (FrameWndProc). */
    hwnd = create(HWND_DESKTOP, "WC_FRAME", "FrameClose", 50, 50, 200, 100);
    check(hwnd != NULLHANDLE, "S3-3: create WC_FRAME");
    if (hwnd) {
        post_and_dispatch(hab, hwnd, WM_CLOSE);
        check(!WinDestroyWindow(hwnd), "S3-3: window gone after WM_CLOSE");
    }

    /* S3-4: TestClass + WM_CLOSE -> window destroyed (DefWindowProc). */
    hwnd = create(HWND_DESKTOP, "TestClass", "DefClose", 50, 50, 200, 100);
    check(hwnd != NULLHANDLE, "S3-4: create TestClass");
    if (hwnd) {
        post_and_dispatch(hab, hwnd, WM_CLOSE);
        check(!WinDestroyWindow(hwnd), "S3-4: window gone after WM_CLOSE");
    }

    /* S3-5: TestClass + WM_PAINT -> window stays alive (DefWindowProc). */
    hwnd = create(HWND_DESKTOP, "TestClass", "DefPaint", 50, 50, 200, 100);
    check(hwnd != NULLHANDLE, "S3-5: create TestClass");
    if (hwnd) {
        post_and_dispatch(hab, hwnd, WM_PAINT);
        check(WinDestroyWindow(hwnd), "S3-5: window alive after WM_PAINT");
    }

    check(WinDestroyMsgQueue(hmq), "WinDestroyMsgQueue");

terminate:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
