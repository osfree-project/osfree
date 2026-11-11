/*!
 * @file test_gre_desktop.cpp
 * @brief GPI test: draw directly on HWND_DESKTOP via WinGetScreenPS.
 *
 * Chain:
 *   client
 *     > Fc_Ses* transport
 *     > PMSERVER DispatchGpi
 *     > GOCA emit / GocaParseStream
 *     > Gre32Entry5 / Gre32Entry4
 *     > GRE2SDL.dll (SDL window)
 *
 * Presentation space is obtained from the desktop window:
 *   hps = WinGetScreenPS(HWND_DESKTOP);
 *
 * The returned PS must be released with WinReleasePS when done.
 */

#include <stdio.h>
#include "F_platform.h"

static int g_fail = 0;

static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

int main(void)
{
    HAB    hab;
    HPS    hps;
    POINTL pt;
    int    i;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    /* Get a presentation space for the entire desktop. */
    hps = WinGetScreenPS(HWND_DESKTOP);
    check(hps != NULLHANDLE, "WinGetScreenPS(HWND_DESKTOP)");
    if (hps == NULLHANDLE) goto done;

    check(GpiSetColor(hps, CLR_RED), "GpiSetColor(CLR_RED)");

    pt.x = 100; pt.y = 100;
    check(GpiMove(hps, &pt), "GpiMove({100,100})");

    pt.x = 400; pt.y = 300;
    check(GpiLine(hps, &pt), "GpiLine({400,300})");

    printf("  waiting 3 s; the GRE2SDL window should show a red diagonal line\n");
    for (i = 0; i < 30; i++) DosSleep(100);

    check(WinReleasePS(hps), "WinReleasePS");

done:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
