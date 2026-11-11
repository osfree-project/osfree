/*!
 * @file test_gre_fullps.cpp
 * @brief GPI test: full presentation space from a window DC.
 *
 * Follows the documented OS/2 pattern (PM Reference, WinOpenWindowDC,
 * Example Code, and GpiCreatePS):
 *
 *   hdc = WinOpenWindowDC(HWND_DESKTOP);
 *   hps = GpiCreatePS(hab, hdc, &pagesize,
 *                     PU_PELS | GPIF_LONG | GPIA_ASSOC);
 *
 * This is a full presentation space (segment support, full Gpi* set),
 * unlike the micro-PS returned by WinGetScreenPS.
 *
 * Chain exercised end to end:
 *   client (this binary)
 *     > WinInitialize           > NWinInitialize
 *     > WinOpenWindowDC         > NWinOpenWindowDC
 *     > GpiCreatePS             > NGpiCreatePS  (full PS, GPIA_ASSOC)
 *     > GpiSetColor/GpiMove/GpiLine  > NGpiSetColor/NGpiMove/NGpiLine
 *                                  > GOCA > Gre32EntryN > GRE2SDL
 *     > GpiDestroyPS            > NGpiDestroyPS
 *     > WinTerminate            > NWinTerminate
 *
 * The HDC from WinOpenWindowDC is owned by the window: per OS/2 PM
 * it is closed automatically when the window is destroyed and must
 * NOT be closed with DevCloseDC. HWND_DESKTOP is never destroyed, so
 * the test simply drops the HDC on exit.
 */

#define INCL_WINWINDOWMGR      /* WinInitialize / WinOpenWindowDC  */
#define INCL_GPICONTROL        /* GpiCreatePS / GpiDestroyPS       */
#define INCL_GPIPRIMITIVES     /* GpiSetColor / GpiMove / GpiLine  */
#define INCL_DOSPROCESS        /* DosSleep                          */
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
    HDC    hdc;
    HPS    hps;
    SIZEL  pagesize = { 0L, 0L };   /* 0,0 = query device page size */
    POINTL pt;
    int    i;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    /* Window DC for the desktop window. Valid HDC or NULLHANDLE. */
    hdc = WinOpenWindowDC(HWND_DESKTOP);
    check(hdc != NULLHANDLE, "WinOpenWindowDC(HWND_DESKTOP)");
    if (hdc == NULLHANDLE) goto done;

    /* Full PS associated with the window DC. */
    hps = GpiCreatePS(hab, hdc, &pagesize,
                      PU_PELS | GPIF_LONG | GPIA_ASSOC);
    check(hps != NULLHANDLE, "GpiCreatePS(PU_PELS|GPIF_LONG|GPIA_ASSOC)");
    if (hps == NULLHANDLE) goto done;

    check(GpiSetColor(hps, CLR_RED), "GpiSetColor(CLR_RED)");

    pt.x = 100; pt.y = 100;
    check(GpiMove(hps, &pt), "GpiMove({100,100})");

    pt.x = 400; pt.y = 300;
    check(GpiLine(hps, &pt), "GpiLine({400,300})");

    printf("  waiting 3 s; the GRE2SDL window should show a red diagonal line\n");
    for (i = 0; i < 30; i++) DosSleep(100);

    check(GpiDestroyPS(hps), "GpiDestroyPS");

done:
    /* No DevCloseDC: WinOpenWindowDC's HDC is owned by the window. */
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
