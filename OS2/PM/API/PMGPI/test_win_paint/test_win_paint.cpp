/*!
 * @file test_win_paint.cpp
 * @brief WM_PAINT cycle: BeginPaint -> Gpi* -> EndPaint via message loop.
 */
#define INCL_PM
#define INCL_WINWINDOWMGR
#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>

#ifndef WM_PAINT
  #define WM_PAINT  0x0023
#endif

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB    hab;
    HMQ    hmq;
    HWND   hwnd;
    QMSG   qmsg;
    BOOL   rc;
    int    paints = 0;
    int    closes = 0;
    int    iterations = 0;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    hmq = WinCreateMsgQueue(hab, 0);
    check(hmq != NULLHANDLE, "WinCreateMsgQueue");
    if (hmq == NULLHANDLE) goto terminate;

    hwnd = WinCreateWindow(HWND_DESKTOP, "TestClass", "PaintTest",
                           WS_VISIBLE,
                           100, 100, 300, 200,
                           NULLHANDLE, HWND_TOP, 0, NULL, NULL);
    check(hwnd != NULLHANDLE, "WinCreateWindow(300x200)");
    if (hwnd == NULLHANDLE) goto destroy_mq;

    /* Force DC creation now so the first WinBeginPaint finds it. */
    check(WinOpenWindowDC(hwnd) != NULLHANDLE, "WinOpenWindowDC");

    /* Paint request #1. */
    rc = WinPostMsg(hwnd, WM_PAINT, 0, 0);
    check(rc, "WinPostMsg(WM_PAINT) #1");

    /* Close request. Handled by the loop after WM_PAINT;
     * the WM_CLOSE case destroys the window and posts WM_QUIT,
     * which makes WinGetMsg return FALSE and break the loop. */
    rc = WinPostMsg(hwnd, WM_CLOSE, 0, 0);
    check(rc, "WinPostMsg(WM_CLOSE)");

    /* Message loop. */
    while (iterations++ < 20) {
        rc = WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0);
        if (!rc) break;     /* WM_QUIT or error */

        switch (qmsg.msg) {

        case WM_PAINT: {
            HPS    hps;
            RECTL  rcl;
            POINTL pt1, pt2;

            paints++;
            printf("  WM_PAINT #%d (hwnd=%lu)\n",
                   paints, (unsigned long)qmsg.hwnd);

            rcl.xLeft = rcl.yBottom = rcl.xRight = rcl.yTop = 0;
            hps = WinBeginPaint(qmsg.hwnd, NULLHANDLE, &rcl);
            check(hps != NULLHANDLE, "  WinBeginPaint");
            if (hps) {
                /* WinBeginPaint returns the full window rect. */
                check(rcl.xLeft == 0 && rcl.yBottom == 0 &&
                      rcl.xRight == 300 && rcl.yTop == 200,
                      "  rcl == {0,0,300,200}");

                check(GpiSetColor(hps, 0x000000FF), "  GpiSetColor(red)");

                pt1.x = 10; pt1.y = 10;
                check(GpiMove(hps, &pt1), "  GpiMove({10,10})");

                pt2.x = 290; pt2.y = 190;
                check(GpiLine(hps, &pt2) == 1, "  GpiLine({290,190})");

                pt1.x = 30; pt1.y = 30;
                check(GpiMove(hps, &pt1), "  GpiMove({30,30})");
                check(GpiBox(hps, DRO_OUTLINE, &pt2, 0, 0) == 1,
                      "  GpiBox({30,30}-{290,190})");

                check(WinEndPaint(hps), "  WinEndPaint");
            }
            break;
        }

        case WM_CLOSE: {
            closes++;
            printf("  WM_CLOSE (hwnd=%lu)\n", (unsigned long)qmsg.hwnd);
            check(WinDestroyWindow(qmsg.hwnd), "  WinDestroyWindow");
            /* Terminate the loop: post WM_QUIT to the thread queue. */
            WinPostMsg(NULLHANDLE, WM_QUIT, 0, 0);
            break;
        }

        default:
            (void)WinDispatchMsg(hab, &qmsg);
            break;
        }
    }

    check(paints == 1, "exactly one WM_PAINT handled");
    check(closes == 1, "exactly one WM_CLOSE handled");

destroy_mq:
    check(WinDestroyMsgQueue(hmq), "WinDestroyMsgQueue");

terminate:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
