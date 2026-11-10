/*!
 * @file test_gre2sdl.cpp
 * @brief Direct test of the GRE2SDL presentation driver.
 *
 * Loads GRE2SDL.DLL, resolves OS2_PM_DRV_ENABLE by ordinal 200, calls
 * EN_FILL_LDB to fill a local dispatch table, then exercises the driver
 * through that table without going through gre.cpp or the GPI layer.
 *
 * Prereqs:
 *   - GRE2SDL.dll on PATH or next to the executable.
 *   - fpmsrv.ini (optional; DesktopNx/DesktopNy override the default
 *     window size). Defaults are 640x480.
 *
 * Exit code: 0 on success, 1 on failures, 2 on load/resolve failure.
 */

#include <stdio.h>
#include <string.h>

#include "F_platform.h"
#include "F_debug.h"

#define INCL_GRE_ARCS        /* GreBoxInterior/Boundary/Both, GreFullArcBoth, GrePartialArc, GrePolySpline, GrePolyFillet */
#define INCL_GRE_LINES       /* GreSetCurrentPosition, GreGetCurrentPosition, GrePolyLine */
#define INCL_GRE_MARKERS     /* GrePolyMarker */
#define INCL_GRE_STRINGS     /* GreCharString, GreCharStringPos */
#define INCL_GRE_DEVSUPPORT  /* GreDeviceSetAttributes, GreDeviceGetAttributes */
#define INCL_GRE_DEVMISC1    /* NGreDeviceSetAttributes, NGreDeviceGetAttributes */
#define INCL_GRE_DCS         /* NGreOpenDC, NGreCloseDC */
#include "pmddi.h"

#define GRE_DISPATCH_SIZE 256

typedef ULONG (DDI_CALLCONV *PFN_GRE4)(ULONG, ULONG, ULONG, ULONG);
typedef ULONG (DDI_CALLCONV *PFN_GRE5)(ULONG, ULONG, ULONG, ULONG, ULONG);
typedef ULONG (DDI_CALLCONV *PFN_GRE7)(ULONG, ULONG, ULONG, ULONG,
                                       ULONG, ULONG, ULONG);

static PFN g_table[GRE_DISPATCH_SIZE];
static int g_fail = 0;

static void CHECK(int cond, const char *msg)
{
    printf("[%s] %s\n", cond ? "PASS" : "FAIL", msg);
    fflush(stdout);
    if (!cond) g_fail++;
}

/*!
 * @brief Resolve OS2_PM_DRV_ENABLE from the driver.
 *
 * GRE2SDL.DLL exports this entry point by ordinal 200; there is no
 * named alias and no fallback path.
 */
static PFN_OS2_PM_DRV_ENABLE ResolveEnable(HMODULE hmod)
{
    PFN pfn = NULL;
    DosQueryProcAddr(hmod, 200, NULL, &pfn);
    return (PFN_OS2_PM_DRV_ENABLE)pfn;
}

int main(int argc, char *argv[])
{
    HMODULE       hmod = 0;
    PFN_OS2_PM_DRV_ENABLE pfnEnable = NULL;
    APIRET        rc;
    FLParamsLst   flParams;
    FLReturnsLst  flReturns;
    USHORT        fsDeviceFlags = 0;
    ULONG         drvRc;
    DENPARAMS     den;
    PVOID         pInst;
    ULONG         idx, hdc = 42;
    POINTL        pt, pts[2];
    LINEBUNDLE    lb;
    BOXPARAMS     bp;
    int           i, nHooks;

    (void)argc; (void)argv;

    /* 1. Load GRE2SDL.DLL. */
    rc = DosLoadModule(NULL, 0, "GRE2SDL", &hmod);
    if (rc != 0 || hmod == 0) {
        printf("FATAL: DosLoadModule(GRE2SDL): rc=%lu\n", (unsigned long)rc);
        return 2;
    }
    CHECK(1, "DosLoadModule(GRE2SDL)");

    /* 2. Resolve OS2_PM_DRV_ENABLE by ordinal 200. */
    pfnEnable = ResolveEnable(hmod);
    CHECK(pfnEnable != NULL, "resolve OS2_PM_DRV_ENABLE");
    if (!pfnEnable) goto done;

    /* 3. EN_FILL_LDB: fill local dispatch table. */
    for (i = 0; i < GRE_DISPATCH_SIZE; i++) g_table[i] = NULL;
    memset(&flParams,  0, sizeof(flParams));
    memset(&flReturns, 0, sizeof(flReturns));
    flParams.Version   = 0x00000210;
    flParams.TableSize = GRE_DISPATCH_SIZE;
    flReturns.Flags    = &fsDeviceFlags;
    flReturns.DispTab  = g_table;

    drvRc = pfnEnable(EN_FILL_LDB, (PULONG)&flParams, (PLONG)&flReturns);
    CHECK(drvRc == 0, "pfnEnable(EN_FILL_LDB)");
    if (drvRc != 0) goto done;

    nHooks = 0;
    for (i = 0; i < GRE_DISPATCH_SIZE; i++) if (g_table[i]) nHooks++;
    printf("  hooks installed: %d, flags=0x%04x\n",
           nHooks, (unsigned)fsDeviceFlags);
    CHECK(nHooks >= 5, "at least 5 hooks installed");

    /* 4. EN_ENABLE_CTX: allocate per-DC instance. */
    memset(&den, 0, sizeof(den));
    den.ulType = OD_DIRECT;
    den.ulHDC  = hdc;
    pInst = (PVOID)pfnEnable(EN_ENABLE_CTX, (PULONG)&den, NULL);
    CHECK(pInst != NULL && pInst != (PVOID)-1,
          "pfnEnable(EN_ENABLE_CTX) -> pInstance");
    if (pInst == NULL || pInst == (PVOID)-1) goto done;
    printf("  pInstance = %p\n", pInst);

    /* 5. EN_COMPLETE_OPENDC: driver logs only. */
    pfnEnable(EN_COMPLETE_OPENDC, (PULONG)hdc, (PLONG)pInst);

    /* 6. Draw via the dispatch table.
     *    GreDeviceSetAttributes : 0xA2, arity 7
     *    GreSetCurrentPosition  : 0x18, arity 4
     *    GrePolyLine            : 0x19, arity 5
     *    GreBoxBoundary         : 0x08, arity 4
     */

    /* 6a. Set foreground colour to red. */
    memset(&lb, 0, sizeof(lb));
    lb.lColor = 0x00FF0000;
    idx = NGreDeviceSetAttributes & 0xFF;
    if (g_table[idx]) {
        drvRc = ((PFN_GRE7)g_table[idx])(hdc, PRIM_LINE, LBB_COLOR, 0,
                                          (ULONG)&lb, (ULONG)pInst,
                                          NGreDeviceSetAttributes);
        CHECK(drvRc == 0, "GreDeviceSetAttributes(PRIM_LINE, LBB_COLOR)");
    } else {
        CHECK(0, "no NGreDeviceSetAttributes hook");
    }

    /* 6b. Set current position to (100,100). */
    pt.x = 100; pt.y = 100;
    idx = NGreSetCurrentPosition & 0xFF;
    if (g_table[idx]) {
        drvRc = ((PFN_GRE4)g_table[idx])(hdc, (ULONG)&pt, (ULONG)pInst,
                                          NGreSetCurrentPosition);
        CHECK(drvRc == 0, "GreSetCurrentPosition({100,100})");
    } else {
        CHECK(0, "no NGreSetCurrentPosition hook");
    }

    /* 6c. Draw a line from (100,100) to (400,300). */
    pts[0].x = 100; pts[0].y = 100;
    pts[1].x = 400; pts[1].y = 300;
    idx = NGrePolyLine & 0xFF;
    if (g_table[idx]) {
        drvRc = ((PFN_GRE5)g_table[idx])(hdc, (ULONG)pts, 2, (ULONG)pInst,
                                          NGrePolyLine);
        CHECK(drvRc == 0, "GrePolyLine(2 pts)");
    } else {
        CHECK(0, "no NGrePolyLine hook");
    }

    /* 6d. Set current position to (300,200). */
    pt.x = 300; pt.y = 200;
    idx = NGreSetCurrentPosition & 0xFF;
    if (g_table[idx]) {
        drvRc = ((PFN_GRE4)g_table[idx])(hdc, (ULONG)&pt, (ULONG)pInst,
                                          NGreSetCurrentPosition);
        CHECK(drvRc == 0, "GreSetCurrentPosition({300,200})");
    } else {
        CHECK(0, "no NGreSetCurrentPosition hook");
    }

    /* 6e. Draw a box from (300,200) to (500,400). */
    memset(&bp, 0, sizeof(bp));
    bp.ptl.x = 500; bp.ptl.y = 400;
    idx = NGreBoxBoundary & 0xFF;
    if (g_table[idx]) {
        drvRc = ((PFN_GRE4)g_table[idx])(hdc, (ULONG)&bp, (ULONG)pInst,
                                          NGreBoxBoundary);
        CHECK(drvRc == 0, "GreBoxBoundary");
    } else {
        CHECK(0, "no NGreBoxBoundary hook");
    }

    /* 7. Give the worker thread time to make the picture visible. */
    printf("  waiting ~3 s; a red diagonal and a red box should be visible\n");
    for (i = 0; i < 30; i++) DosSleep(100);
    CHECK(1, "wait completed (no crash)");

    /* 8. Close the DC: driver frees pInstance. */
    pfnEnable(EN_BEGIN_CLOSEDC, (PULONG)hdc, (PLONG)pInst);
    pfnEnable(EN_DISABLE_CTX,   (PULONG)pInst, NULL);
    CHECK(1, "EN_BEGIN_CLOSEDC + EN_DISABLE_CTX");

done:
    if (hmod) DosFreeModule(hmod);

    printf("\n%s (%d failures)\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
