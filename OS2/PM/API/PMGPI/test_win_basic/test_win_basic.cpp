/*!
 * @file test_win_basic.cpp
 * @brief Window lifecycle: create, open DC, draw, hide, show, move, destroy.
 */
#define INCL_WINWINDOWMGR
#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB    hab;
    HWND   hwnd;
    HDC    hdc;
    HPS    hps;
    SIZEL  szl = { 0, 0 };
    POINTL pt;
    int    i;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    hwnd = WinCreateWindow(HWND_DESKTOP, "TestClass", "TestTitle",
                           WS_VISIBLE,
                           100, 100, 300, 200,
                           NULLHANDLE, NULLHANDLE, 0, NULL, NULL);
    check(hwnd != NULLHANDLE, "WinCreateWindow");
    if (hwnd == NULLHANDLE) goto done;

    hdc = WinOpenWindowDC(hwnd);
    check(hdc != NULLHANDLE, "WinOpenWindowDC(hwnd)");
    if (hdc == NULLHANDLE) goto destroy;

    hps = GpiCreatePS(hab, hdc, &szl, PU_PELS | GPIF_LONG | GPIA_ASSOC);
    check(hps != NULLHANDLE, "GpiCreatePS");
    if (hps == NULLHANDLE) goto destroy;

    check(GpiSetColor(hps, CLR_RED), "GpiSetColor(CLR_RED)");
    pt.x = 10; pt.y = 10;
    check(GpiMove(hps, &pt), "GpiMove({10,10})");
    pt.x = 290; pt.y = 190;
    check(GpiLine(hps, &pt), "GpiLine({290,190})");

    printf("  window visible; waiting 2 s\n");
    for (i = 0; i < 20; i++) DosSleep(100);

    check(WinShowWindow(hwnd, FALSE), "WinShowWindow(FALSE)");
    printf("  hidden; waiting 1 s\n");
    for (i = 0; i < 10; i++) DosSleep(100);

    check(WinShowWindow(hwnd, TRUE), "WinShowWindow(TRUE)");
    printf("  shown; waiting 1 s\n");
    for (i = 0; i < 10; i++) DosSleep(100);

    check(WinSetWindowPos(hwnd, NULLHANDLE, 200, 150, 0, 0, SWP_MOVE),
          "WinSetWindowPos(move to 200,150)");
    printf("  moved; waiting 2 s\n");
    for (i = 0; i < 20; i++) DosSleep(100);

    check(GpiDestroyPS(hps), "GpiDestroyPS");

destroy:
    check(WinDestroyWindow(hwnd), "WinDestroyWindow");

done:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
