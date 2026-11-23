/*!
 * @file testclient.cpp
 * @brief Minimal GPI-level test client for FreePM transport Phase 1.
 *
 * Structure follows testgpi.cpp, stripped down to: WinInitialize ->
 * GpiCreatePS -> GpiSetColor/Move/Line -> GpiDestroyPS -> exit.
 *
 * pmclient.dll is loaded implicitly (import library). CONNECT is sent
 * on DLL load; EXIT is sent on DLL unload. pmserver.exe should shut
 * down on its own once the last session is gone.
 */

#define INCL_WIN
#include "F_platform.h"
#include <stdio.h>

int main(void)
{
    HAB    hab;
    SIZEL  szl;
    HPS    hps;
    POINTL pt1, pt2;
    BOOL   rc;

    printf("testclient: starting (pmclient.dll CONNECT already done)\n");

    hab = WinInitialize(0);
    if (!hab) { printf("testclient: WinInitialize failed\n"); return 1; }

    szl.cx = 640;
    szl.cy = 480;
    hps = GpiCreatePS(hab, NULL, &szl, GPIA_ASSOC | PU_PELS | GPIF_DEFAULT);
    if (!hps) {
        printf("testclient: GpiCreatePS failed\n");
        WinTerminate(hab);
        return 1;
    }
    printf("testclient: GpiCreatePS -> hps=%p\n", (void*)hps);

    pt1.x = 100; pt1.y = 100;
    pt2.x = 400; pt2.y = 300;

    printf("testclient: GpiSetColor(CLR_RED)\n");
    GpiSetColor(hps, CLR_RED);

    printf("testclient: GpiMove({100,100})\n");
    GpiMove(hps, &pt1);

    printf("testclient: GpiLine({400,300})\n");
    GpiLine(hps, &pt2);

    rc = GpiDestroyPS(hps);
    printf("testclient: GpiDestroyPS -> %d\n", (int)rc);

    WinTerminate(hab);

    printf("testclient: exiting (pmclient.dll unload sends EXIT)\n");
    return 0;
}
