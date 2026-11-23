/*!
 * @file test_gre_wire.cpp
 * @brief End-to-end GPI test: DevOpenDC > GpiCreatePS > GpiLine.
 *
 * Exercises the full chain:
 *   client (this binary)
 *     > Fc_Ses* transport
 *     > PMSERVER DispatchGpi
 *     > GOCA emit / GocaParseStream
 *     > Gre32Entry7 / Gre32Entry5 / Gre32Entry4
 *     > GRE2SDL.dll (SDL window)
 *
 * HAB lifecycle is handled by WinInitialize / WinTerminate.
 */

#include <stdio.h>
#include "F_platform.h"

static int g_fail = 0;

/*!
 * @brief Print PASS/FAIL for one check and count failures.
 * @param [in] ok   Non-zero on success.
 * @param [in] msg  Test description.
 */
static void check(int ok, const char *msg) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if (!ok) g_fail++;
}

/*!
 * @brief Run the full DevOpenDC > GpiCreatePS > GpiLine chain.
 * @return 0 on success, 1 on failures.
 */
int main(void)
{
    HAB    hab;
    HDC    hdc;
    HPS    hps;
    POINTL pt;
    SIZEL  sizl = { 640, 480 };
    PSZ    pdop[3];
    int    i;

    hab = WinInitialize(0);
    check(hab != 0, "WinInitialize");
    if (!hab) return 1;

    pdop[0] = "DISPLAY";
    pdop[1] = "GRE2SDL";
    pdop[2] = NULL;

    hdc = DevOpenDC(hab, OD_DIRECT, "*", 2, (PDEVOPENDATA)pdop, NULLHANDLE);
    check(hdc != (HDC)0 && hdc != (HDC)-1, "DevOpenDC");
    if (hdc == (HDC)0 || hdc == (HDC)-1) goto done;

    hps = GpiCreatePS(hab, hdc, &sizl, 0);
    check(hps != (HPS)0, "GpiCreatePS");
    if (!hps) goto done;

    check(GpiSetColor(hps, CLR_RED), "GpiSetColor(CLR_RED)");
    pt.x = 100; pt.y = 100;
    check(GpiMove(hps, &pt), "GpiMove({100,100})");
    pt.x = 400; pt.y = 300;
    check(GpiLine(hps, &pt), "GpiLine({400,300})");

    printf("  waiting 3 s; the GRE2SDL window should show a red diagonal line\n");
    for (i = 0; i < 30; i++) DosSleep(100);

    check(GpiDestroyPS(hps), "GpiDestroyPS");

done:
    WinTerminate(hab);

    printf("\n%s (%d)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
