/*!
 * @file test_win_draw.cpp
 * @brief Real drawing: create window, PS, line, box, sleep, destroy.
 *
 * Empirical check of the GRE2SDL presentation driver: which Gre*
 * calls reach the driver and which return GPI_ERROR (stub). The test
 * itself passes regardless of the visual result (no rendering
 * assertion), the outcome is in pmserver.log.
 *
 * Sequence:
 *   - WinCreateWindow
 *   - WinOpenWindowDC(hwnd)        -> HDC
 *   - GpiCreatePS(hab, hdc, szl, PU_PELS | GPIF_LONG | GPIA_ASSOC)
 *   - GpiSetColor(CLR_RED)          (logical index, not RGB)
 *   - GpiMove({10,10}); GpiLine({290,190})
 *   - GpiBox(DRO_OUTLINE, {290,190}, 0, 0)
 *   - DosSleep(3000)                (window visible on screen)
 *   - GpiDestroyPS
 *   - WinDestroyWindow
 *   - WinDestroyMsgQueue / WinTerminate
 *
 * Expected log entries in pmserver.log:
 *   [GRE] Gre32EntryN(fn=..., a=...)            for each Gre* call
 *   [GRE2SDL] <specific handler>                 if the driver hooks it
 *   [GRE] <stub> -> GPI_ERROR (stub)             if the driver does not
 */
#define INCL_PM
#define INCL_WINWINDOWMGR
#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_DOSPROCESS
#include <F_platform.h>

#include <stdio.h>

#ifndef CLR_RED
  #define CLR_RED 2L
#endif

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB     hab;
    HMQ     hmq;
    HWND    hwnd;
    HDC     hdc;
    HPS     hps;
    SIZEL   szl;
    POINTL  ptl;
    BOOL    rc;
    int     i;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    hmq = WinCreateMsgQueue(hab, 0);
    check(hmq != NULLHANDLE, "WinCreateMsgQueue");
    if (hmq == NULLHANDLE) goto terminate;

    hwnd = WinCreateWindow(HWND_DESKTOP, "TestClass", "DrawTest",
                           WS_VISIBLE,
                           100, 100, 300, 200,
                           NULLHANDLE, NULLHANDLE, 0, NULL, NULL);
    check(hwnd != NULLHANDLE, "WinCreateWindow");
    if (hwnd == NULLHANDLE) goto destroy_mq;

    hdc = WinOpenWindowDC(hwnd);
    check(hdc != NULLHANDLE, "WinOpenWindowDC");
    if (hdc == NULLHANDLE) goto destroy_wnd;

    szl.cx = 300;
    szl.cy = 200;
    hps = GpiCreatePS(hab, hdc, &szl, PU_PELS | GPIF_LONG | GPIA_ASSOC);
    check(hps != NULLHANDLE, "GpiCreatePS");
    if (hps == NULLHANDLE) goto destroy_wnd;

    /* Raw RGB in OS/2 BBGGRR order (not a CLR_* logical index). */
    rc = GpiSetColor(hps, 0x000000FF);
    check(rc, "GpiSetColor(0x000000FF)");

    ptl.x = 10; ptl.y = 10;
    rc = GpiMove(hps, &ptl);
    check(rc, "GpiMove({10,10})");

    ptl.x = 290; ptl.y = 190;
    rc = GpiLine(hps, &ptl);
    check(rc == 1, "GpiLine({290,190})");

    /* Box: current position {290,190} = one corner, ptl = other corner. */
    ptl.x = 10; ptl.y = 10;
    rc = GpiMove(hps, &ptl);
    check(rc, "GpiMove({10,10}) (box corner 1)");

    ptl.x = 290; ptl.y = 190;
    rc = GpiBox(hps, DRO_OUTLINE, &ptl, 0, 0);
    check(rc == 1, "GpiBox(DRO_OUTLINE, {10,10}-{290,190})");

    printf("  window visible; waiting 3 s\n");
    for (i = 0; i < 30; i++) DosSleep(100);

    check(GpiDestroyPS(hps), "GpiDestroyPS");

destroy_wnd:
    check(WinDestroyWindow(hwnd), "WinDestroyWindow");

destroy_mq:
    check(WinDestroyMsgQueue(hmq), "WinDestroyMsgQueue");

terminate:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
