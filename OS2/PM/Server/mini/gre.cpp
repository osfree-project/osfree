/*!
 * @file gre.cpp
 * @brief Mini-server GRE: loads GRE2SDL presentation driver, dispatches
 *        Gre* calls to the driver's hook table, owns the DC registry.
 *
 * On the first NGreOpenDC:
 *   1. DosLoadModule("GRE2SDL.DLL")
 *   2. DosQueryProcAddr(ordinal 200, "OS2_PM_DRV_ENABLE")
 *   3. pfnEnable(EN_FILL_LDB, &flParams, &flReturns)
 *      -> driver fills flReturns.DispTab (dispatch table) and *flReturns.Flags
 *
 * For every NGreOpenDC / NGreCloseDC / NGreSetDCOwner, gre.cpp handles
 * the DC lifecycle itself (g_greDcs) and calls the driver's
 * EnableDeviceContext / CompleteOpenDC / BeginCloseDC /
 * DisableDeviceContext subfunctions.
 *
 * For every other NGre*, gre.cpp looks up the DC's pInstance and calls
 * the handler from one of two tables, chosen by bit 9 of fnCode
 * (mask 0x0200, the high byte of the low-order word):
 *
 *   - g_tblDriver[256]   for 0x40xx / 0x50xx / 0x60xx / 0x70xx,
 *                        filled by GRE2SDL in FillLogicalDeviceBlock.
 *   - g_tblInternal[256] for 0x02xx / 0x42xx, GRE-internal, never
 *                        forwarded to the driver.
 *
 * The dummy parameter inserted by the pmddim.h macros (the `0L` before
 * fnCode) is replaced with the DC's pInstance.  The driver function
 * signatures in GRE2SDL therefore have one extra ULONG in that position.
 *
 * DENPARAMS, FLParamsLst, FLReturnsLst, PFN_OS2_PM_DRV_ENABLE,
 * APIENTRY and EN_* subfunction codes are declared in pmddi.h.
 */

#include "F_platform.h"
#include "F_debug.h"
#include "Fs_dc.hpp"

#include <string.h>

#define INCL_GREALL
#define INCL_AVIOP
#define INCL_GRE_HFONT
#define INCL_WINPOINTERS
#include "pmddi.h"

/*! @def GRE_DISPATCH_SIZE
 *  @brief Number of slots in internal dispatch table.
 */
#define GRE_DISPATCH_SIZE 256

/*! @def MAXTABLEENTRIES
 *  @brief Number of slots in driver dispatch table.
 */
#define MAXTABLEENTRIES   223

#include "gre_debug.hpp"

/* ======================================================================== */
/*  Driver hook shapes (arity 2..10)                                      */
/* ======================================================================== */

typedef ULONG (__cdecl *PFN_GRE2)(ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE3)(ULONG, ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE4)(ULONG, ULONG, ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE5)(ULONG, ULONG, ULONG, ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE6)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                   ULONG);
typedef ULONG (__cdecl *PFN_GRE7)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                   ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE8)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                   ULONG, ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE9)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                   ULONG, ULONG, ULONG, ULONG);
typedef ULONG (__cdecl *PFN_GRE10)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                    ULONG, ULONG, ULONG, ULONG, ULONG);

/* ======================================================================== */
/*  Module state                                                          */
/* ======================================================================== */

/*! @brief DC registry owned by GRE; keyed by HDC. */
static Fs_DcList              g_greDcs;

/*! @brief Handle of GRE2SDL.DLL; 0 until loaded.  Never freed: the
 *         dispatch table points into the driver's code. */
static HMODULE                g_hmodGre2Sdl    = 0;

/*! @brief OS2_PM_DRV_ENABLE entry point; NULL until loaded. */
static PFN_OS2_PM_DRV_ENABLE  g_pfnEnable      = NULL;

/*! @brief TRUE after the driver has been loaded and FillLogicalDeviceBlock
 *         has succeeded. */
static BOOL                   g_bDriverLoaded  = FALSE;

/*! @brief Driver dispatch table (0x40xx / 0x50xx / 0x60xx / 0x70xx).
 *         BSS-initialized to NULL; GRE2SDL overwrites the slots it hooks
 *         in FillLogicalDeviceBlock.  Slots not hooked by the driver
 *         stay NULL. */
//static PFN                    g_tblDriver[MAXTABLEENTRIES];

/*! @brief Snapshot of g_tblDriver taken just before FillLogicalDeviceBlock,
 *         used by HookedCount() to count the slots the driver actually
 *         replaced. */
static PFN                    g_tblDriverBefore[MAXTABLEENTRIES];

/*! @brief Slots that MUST be filled by the presentation driver in
 *         FillLogicalDeviceBlock.  If the driver leaves one of them
 *         NULL, GRE rejects the driver at load time.
 *
 *         Mandatory status per IBM Device Driver Kit for OS/2,
 *         Presentation Driver Reference (PDRREF), mandatory functions
 *         table: GreQueryDeviceCaps, GreNotifyClipChange,
 *         GreDeviceSetDCOrigin are all listed as mandatory.
 *
 *         Full NGre* code (flags + slot): low byte indexes g_tblDriver. */
static const ULONG            g_mandatorySlots[] = {
    NGreQueryDeviceCaps,
    NGreNotifyClipChange,
    NGreDeviceSetDCOrigin
};

/*! @brief Number of entries in g_mandatorySlots[]. */
#define MANDATORY_COUNT \
    ((int)(sizeof(g_mandatorySlots) / sizeof(g_mandatorySlots[0])))

/* ======================================================================== */
/*  Forward declarations                                                  */
/* ======================================================================== */

static BOOL  LoadGre2Sdl(void);
static ULONG NewDc(ULONG iHab, ULONG ulType);
static BOOL  DestroyDc(ULONG hdc);

/* ======================================================================== */
/*  Stub macros (each expansion = one distinct static function with       */
/*  its own debug context and log name)                                   */
/* ======================================================================== */

#define GRE_STUB_2(name) \
static ULONG __cdecl name(ULONG a1, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_3(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_4(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_5(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_6(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_7(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_8(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG a7, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_9(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG a7, ULONG a8, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7; (void)a8; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_STUB_10(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG a7, ULONG a8, ULONG a9, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7; (void)a8; (void)a9; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_2(name) \
static ULONG __cdecl name(ULONG a1, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_3(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_4(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_5(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_6(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_7(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_8(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG a7, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_9(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG a7, ULONG a8, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7; (void)a8; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

#define GRE_DRV_10(name) \
static ULONG __cdecl name(ULONG a1, ULONG a2, ULONG a3, ULONG a4, ULONG a5, ULONG a6, ULONG a7, ULONG a8, ULONG a9, ULONG fnCode) \
{ \
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE, \
                             "[GRE] %s(fn=0x%lx)", #name, \
                             (unsigned long)fnCode); \
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6; (void)a7; (void)a8; (void)a9; \
    F_DbgCtxExitf(ctx, "GPI_ERROR (stub)"); \
    return (ULONG)GPI_ERROR; \
}

/* ======================================================================== */
/*  Real internal handlers                                                */
/* ======================================================================== */

static ULONG __cdecl GreIntOpenDC(ULONG hdcComp, ULONG lType,
                                    ULONG pszToken, ULONG lCount,
                                    ULONG pdop, ULONG pInstance,
                                    ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreOpenDC(ulType=0x%lx, fn=0x%lx)",
                             (unsigned long)lType, (unsigned long)fnCode);
    ULONG newHdc;

    (void)hdcComp; (void)pszToken; (void)lCount; (void)pdop;
    (void)pInstance;

    if (!g_bDriverLoaded && !LoadGre2Sdl()) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (driver load failed)");
        return (ULONG)-1;
    }
    newHdc = NewDc(0, lType);
    if (newHdc == (ULONG)-1) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (NewDc failed)");
        return (ULONG)-1;
    }
    F_DbgCtxExitf(ctx, "hdc=%lu", (unsigned long)newHdc);
    return newHdc;
}

static ULONG __cdecl GreIntCloseDC(ULONG hdc, ULONG pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreCloseDC(hdc=%lu, fn=0x%lx)",
                             (unsigned long)hdc, (unsigned long)fnCode);
    BOOL ok;
    (void)pInstance;
    ok = DestroyDc(hdc);
    F_DbgCtxExitf(ctx, "%d", (int)(ok ? 1 : 0));
    return ok ? (ULONG)GPI_OK : (ULONG)GPI_ERROR;
}

static ULONG __cdecl GreIntSetDCOwner(ULONG hdc, ULONG newOwner,
                                       ULONG pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreSetDCOwner(hdc=%lu, owner=%lu, fn=0x%lx)",
                             (unsigned long)hdc, (unsigned long)newOwner,
                             (unsigned long)fnCode);
    Fs_Dc *pDc;
    (void)pInstance;
    pDc = g_greDcs.Find(hdc);
    if (!pDc) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (no DC)");
        return (ULONG)GPI_ERROR;
    }
    pDc->SetHab(newOwner);
    F_DbgCtxExitf(ctx, "GPI_OK");
    return (ULONG)GPI_OK;
}

static ULONG __cdecl GreIntQueryEngineVersion(ULONG a, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreQueryEngineVersion(fn=0x%lx)",
                             (unsigned long)fnCode);
    (void)a;
    F_DbgCtxExitf(ctx, "0x0210");
    return 0x00000210UL;
}

static ULONG __cdecl GreIntQueryDeviceNames(
    ULONG pszDriverName, ULONG plDriverTypes, ULONG pszDriverNames,
    ULONG pszDeviceNames, ULONG plDriverSizes, ULONG pszLogAddress,
    ULONG pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE] GreQueryDeviceNames(fn=0x%lx)",
                             (unsigned long)fnCode);
    (void)pszDriverName; (void)pInstance;

    if (plDriverTypes) *(PLONG)plDriverTypes = 1;
    if (pszDriverNames) strcpy((char *)pszDriverNames, "GRE2SDL");
    if (pszDeviceNames) strcpy((char *)pszDeviceNames, "DISPLAY");
    if (plDriverSizes)  *(PLONG)plDriverSizes = 2;
    if (pszLogAddress)  strcpy((char *)pszLogAddress, "*");

    F_DbgCtxExitf(ctx, "1 device");
    return 1;
}

/* ======================================================================== */
/*  Driver table functions                                                  */
/* ======================================================================== */

/*  Real simulated handlers                                                 */

#include "gre_simulate.hpp"

/*! @brief Stub for empty slots — no NGre* constant exists for this slot.
 *
 * Logs the fnCode so that a stray call to an unassigned slot is traceable. */

GRE_DRV_4 (GreDrvGetArcParameters)
GRE_DRV_4 (GreDrvSetArcParameters)
GRE_DRV_4 (GreDrvArc)
GRE_DRV_7 (GreDrvPartialArc)
GRE_DRV_4 (GreDrvFullArcInterior)
GRE_DRV_4 (GreDrvFullArcBoundary)
GRE_DRV_4 (GreDrvFullArcBoth)
GRE_DRV_4 (GreDrvBoxInterior)
GRE_DRV_4 (GreDrvBoxBoundary)
GRE_DRV_4 (GreDrvBoxBoth)
GRE_DRV_5 (GreDrvPolyFillet)
GRE_DRV_6 (GreDrvPolyFilletSharp)
GRE_DRV_5 (GreDrvPolySpline)
GRE_DRV_7 (GreDrvOval)
GRE_DRV_4 (GreDrvCookWholePath)
GRE_DRV_5 (GreDrvCookPathCurves)
GRE_DRV_5 (GreDrvBoxExclusive)
GRE_DRV_10(GreDrvRenderPath)
GRE_DRV_3 (GreDrvOpenScreenChangeArea)
GRE_DRV_5 (GreDrvGetScreenChangeArea)
GRE_DRV_4 (GreDrvCloseScreenChangeArea)
GRE_DRV_5 (GreDrvDisjointLines)
GRE_DRV_4 (GreDrvGetCurrentPosition)
GRE_DRV_4 (GreDrvSetCurrentPosition)
GRE_DRV_5 (GreDrvPolyLine)
GRE_DRV_6 (GreDrvDrawLinesInPath)
GRE_DRV_4 (GreDrvPolyShortLine)
GRE_DRV_4 (GreDrvPolyScanline)
GRE_DRV_7 (GreDrvGetScreenBits)
GRE_DRV_6 (GreDrvSetScreenBits)
GRE_DRV_9 (GreDrvDrawBits)
GRE_DRV_7 (GreDrvDeviceCreateBitmap)
GRE_DRV_6 (GreDrvDeviceDeleteBitmap)
GRE_DRV_4 (GreDrvDeviceSelectBitmap)
GRE_DRV_9 (GreDrvBitblt)
GRE_DRV_4 (GreDrvGetPel)
GRE_DRV_4 (GreDrvSetPel)
GRE_DRV_6 (GreDrvImageData)
GRE_DRV_8 (GreDrvScanLR)
GRE_DRV_5 (GreDrvFloodFill)
GRE_DRV_4 (GreDrvSaveScreenBits)
GRE_DRV_6 (GreDrvRestoreScreenBits)
GRE_DRV_9 (GreDrvDrawBorder)
GRE_DRV_5 (GreDrvDeviceSetCursor)
GRE_DRV_8 (GreDrvGetBitmapBits)
GRE_DRV_8 (GreDrvSetBitmapBits)
GRE_DRV_4 (GreDrvSetColorCursor)
GRE_DRV_5 (GreDrvCharString)
GRE_DRV_10(GreDrvCharStringPos)
GRE_DRV_7 (GreDrvQueryTextBox)
GRE_DRV_9 (GreDrvQueryCharPositions)
GRE_DRV_6 (GreDrvQueryWidthTable)
GRE_DRV_5 (GreDrvPolyMarker)
GRE_DRV_5 (GreDrvCharRect)
GRE_DRV_5 (GreDrvCharStr)
GRE_DRV_5 (GreDrvScrollRect)
GRE_DRV_4 (GreDrvUpdateCursor)
GRE_DRV_4 (GreDrvDeviceSetPointer)
GRE_DRV_4 (GreDrvBeginArea)
GRE_DRV_4 (GreDrvEndArea)
GRE_DRV_4 (GreDrvBeginPath)
GRE_DRV_4 (GreDrvEndPath)
GRE_DRV_3 (GreDrvCloseFigure)
GRE_DRV_5 (GreDrvFillPath)
GRE_DRV_5 (GreDrvOutlinePath)
GRE_DRV_5 (GreDrvModifyPath)
GRE_DRV_5 (GreDrvStrokePath)
GRE_DRV_5 (GreDrvSelectClipPath)
GRE_DRV_4 (GreDrvSavePath)
GRE_DRV_4 (GreDrvRestorePath)
GRE_DRV_5 (GreDrvClip1DPath)
GRE_DRV_4 (GreDrvDrawRawPath)
GRE_DRV_6 (GreDrvDrawCookedPath)
GRE_DRV_7 (GreDrvAreaSetAttributes)
GRE_DRV_7 (GreDrvPolygonSet)
GRE_DRV_5 (GreDrvPathToRegion)
GRE_DRV_4 (GreDrvDrawRLE)
GRE_DRV_4 (GreDrvDevicePolygonSet)
GRE_DRV_5 (GreDrvPolygon3d)
GRE_DRV_5 (GreDrvGetRegionBox)
GRE_DRV_7 (GreDrvGetRegionRects)
GRE_DRV_5 (GreDrvOffsetRegion)
GRE_DRV_5 (GreDrvPtInRegion)
GRE_DRV_5 (GreDrvRectInRegion)
GRE_DRV_5 (GreDrvCreateRectRegion)
GRE_DRV_4 (GreDrvDestroyRegion)
GRE_DRV_6 (GreDrvSetRectRegion)
GRE_DRV_7 (GreDrvCombineRegion)
GRE_DRV_7 (GreDrvCombineRectRegion)
GRE_DRV_5 (GreDrvCombineShortLineRegion)
GRE_DRV_5 (GreDrvEqualRegion)
GRE_DRV_4 (GreDrvPaintRegion)
GRE_DRV_5 (GreDrvSetRegionOwner)
GRE_DRV_5 (GreDrvFrameRegion)
GRE_DRV_4 (GreDrvGetClipBox)
GRE_DRV_6 (GreDrvGetClipRects)
GRE_DRV_4 (GreDrvOffsetClipRegion)
GRE_DRV_4 (GreDrvPtVisible)
GRE_DRV_4 (GreDrvRectVisible)
GRE_DRV_3 (GreDrvQueryClipRegion)
GRE_DRV_5 (GreDrvSelectClipRegion)
GRE_DRV_4 (GreDrvIntersectClipRectangle)
GRE_DRV_4 (GreDrvExcludeClipRectangle)
GRE_DRV_4 (GreDrvSetXformRect)
GRE_DRV_4 (GreDrvSaveRegion)
GRE_DRV_4 (GreDrvRestoreRegion)
GRE_DRV_5 (GreDrvClipPathCurves)
GRE_DRV_4 (GreDrvSelectPathRegion)
GRE_DRV_4 (GreDrvRegionSelectBitmap)
GRE_DRV_6 (GreDrvCopyClipRegion)
GRE_DRV_4 (GreDrvGetPageUnits)
GRE_DRV_6 (GreDrvSetPageUnits)
GRE_DRV_4 (GreDrvGetModelXform)
GRE_DRV_5 (GreDrvSetModelXform)
GRE_DRV_4 (GreDrvGetWindowViewportXform)
GRE_DRV_5 (GreDrvSetWindowViewportXform)
GRE_DRV_4 (GreDrvGetGlobalViewingXform)
GRE_DRV_5 (GreDrvSetGlobalViewingXform)
GRE_DRV_5 (GreDrvSaveXformData)
GRE_DRV_5 (GreDrvRestoreXformData)
GRE_DRV_4 (GreDrvGetPageViewport)
GRE_DRV_5 (GreDrvSetPageViewport)
GRE_DRV_4 (GreDrvGetGraphicsField)
GRE_DRV_4 (GreDrvSetGraphicsField)
GRE_DRV_4 (GreDrvGetViewingLimits)
GRE_DRV_4 (GreDrvSetViewingLimits)
GRE_DRV_6 (GreDrvQueryViewportSize)
GRE_DRV_7 (GreDrvConvert)
GRE_DRV_8 (GreDrvConvertPath)
GRE_DRV_4 (GreDrvSaveXform)
GRE_DRV_4 (GreDrvRestoreXform)
GRE_DRV_6 (GreDrvMultiplyXforms)
GRE_DRV_6 (GreDrvConvertWithMatrix)
GRE_DRV_6 (GreDrvDeviceGetAttributes)
GRE_DRV_6 (GreDrvDeviceSetAVIOFont2)
GRE_DRV_5 (GreDrvGetPairKerningTable)
GRE_DRV_5 (GreDrvDeviceSetAVIOFont)
GRE_DRV_7 (GreDrvDeviceSetAttributes)
GRE_DRV_6 (GreDrvDeviceSetGlobalAttribute)
GRE_DRV_5 (GreDrvNotifyTransformChange)
GRE_DRV_6 (GreDrvRealizeFont)
GRE_DRV_3 (GreDrvErasePS)
GRE_DRV_4 (GreDrvSetStyleRatio)
GRE_DRV_5 (GreDrvDeviceQueryFontAttributes)
GRE_DRV_8 (GreDrvDeviceQueryFonts)
GRE_DRV_5 (GreDrvDeviceInvalidateVisRegion)
GRE_DRV_4 (GreDrvGetPickWindow)
GRE_DRV_4 (GreDrvSetPickWindow)
GRE_DRV_4 (GreDrvResetBounds)
GRE_DRV_5 (GreDrvGetBoundsData)
GRE_DRV_4 (GreDrvAccumulateBounds)
GRE_DRV_3 (GreDrvGetExtraError)
GRE_DRV_4 (GreDrvSetExtraError)
GRE_DRV_3 (GreDrvGetCodePage)
GRE_DRV_4 (GreDrvSetCodePage)
GRE_DRV_3 (GreDrvLockDevice)
GRE_DRV_3 (GreDrvUnlockDevice)
GRE_DRV_3 (GreDrvDeath)
GRE_DRV_5 (GreDrvResurrection)
GRE_DRV_4 (GreDrvGetDCOrigin)
GRE_DRV_4 (GreDrvGetLineOrigin)
GRE_DRV_5 (GreDrvSetLineOrigin)
GRE_DRV_4 (GreDrvGetStyleRatio)
GRE_DRV_5 (GreDrvQueryColorData)
GRE_DRV_7 (GreDrvQueryLogColorTable)
GRE_DRV_8 (GreDrvCreateLogColorTable)
GRE_DRV_3 (GreDrvRealizeColorTable)
GRE_DRV_3 (GreDrvUnrealizeColorTable)
GRE_DRV_7 (GreDrvQueryRealColors)
GRE_DRV_5 (GreDrvQueryNearestColor)
GRE_DRV_5 (GreDrvQueryColorIndex)
GRE_DRV_5 (GreDrvQueryRGBColor)
GRE_DRV_5 (GreDrvQueryDevFntMetrics)
GRE_DRV_4 (GreDrvDeviceEnableYInversion)
GRE_DRV_5 (GreDrvQueryDeviceBitmaps)
GRE_DRV_8 (GreDrvEscape)
GRE_DRV_6 (GreDrvQueryHardcopyCaps)
GRE_DRV_5 (GreDrvQueryDevResource2)
GRE_DRV_5 (GreDrvDeviceCreatePalette)
GRE_DRV_4 (GreDrvDeviceDeletePalette)
GRE_DRV_8 (GreDrvDeviceSetPaletteEntries)
GRE_DRV_8 (GreDrvDeviceAnimatePalette)
GRE_DRV_5 (GreDrvDeviceResizePalette)
GRE_DRV_5 (GreDrvRealizePalette)
GRE_DRV_6 (GreDrvQueryHWPaletteInfo)
GRE_DRV_3 (GreDrvUpdateColors)
GRE_DRV_6 (GreDrvQueryPaletteRealization)
GRE_DRV_6 (GreDrvGetVisRects)

/* ======================================================================== */
/*  Per-function stubs (internal table) — one distinct function each      */
/* ======================================================================== */

GRE_STUB_4 (GreIntResetDCStub)
GRE_STUB_4 (GreIntResetDC2Stub)
GRE_STUB_4 (GreIntGetHandleStub)
GRE_STUB_5 (GreIntSetHandleStub)
GRE_STUB_3 (GreIntGetProcessControlStub)
GRE_STUB_5 (GreIntSetProcessControlStub)
GRE_STUB_3 (GreIntSaveDCStub)
GRE_STUB_4 (GreIntRestoreDCStub)
GRE_STUB_3 (GreIntQueryBitmapSelectionStub)
GRE_STUB_4 (GreIntGetBitmapInfoHeaderStub)
GRE_STUB_9 (GreIntConvertBitmapDataStub)
GRE_STUB_5 (GreIntInvalidateVisRegionStub)
GRE_STUB_7 (GreIntCreateBitmapStub)
GRE_STUB_3 (GreIntDeleteBitmapStub)
GRE_STUB_4 (GreIntSelectBitmapStub)
GRE_STUB_4 (GreIntGetBitmapParametersStub)
GRE_STUB_4 (GreIntGetBitmapDimensionStub)
GRE_STUB_4 (GreIntSetBitmapDimensionStub)
GRE_STUB_5 (GreIntSetCursorStub)
GRE_STUB_6 (GreIntGetAttributesStub)
GRE_STUB_7 (GreIntSetAttributesStub)
GRE_STUB_6 (GreIntSetGlobalAttributeStub)
GRE_STUB_4 (GreIntSetBitmapOwnerStub)
GRE_STUB_6 (GreIntGetDefaultAttributesStub)
GRE_STUB_6 (GreIntSetDefaultAttributesStub)
GRE_STUB_4 (GreIntGetDefaultArcParametersStub)
GRE_STUB_4 (GreIntSetDefaultArcParametersStub)
GRE_STUB_4 (GreIntGetDefaultViewingLimitsStub)
GRE_STUB_4 (GreIntSetDefaultViewingLimitsStub)
GRE_STUB_4 (GreIntInitializeAttributesStub)
GRE_STUB_4 (GreIntDeleteSetIdStub)
GRE_STUB_4 (GreIntQueryNumberSetIdsStub)
GRE_STUB_8 (GreIntQuerySetIdsStub)
GRE_STUB_4 (GreIntQueryBitmapHandleStub)
GRE_STUB_5 (GreIntSetBitmapIDStub)
GRE_STUB_5 (GreIntCopyDCLoadDataStub)
GRE_STUB_4 (GreIntQuerySetIdUsageStub)
GRE_STUB_6 (GreIntQueryFullFontFileDescriptionsStub)
GRE_STUB_7 (GreIntQueryLogicalFontStub)
GRE_STUB_6 (GreIntCreateLogicalFontStub)
GRE_STUB_3 (GreIntLoadFontStub)
GRE_STUB_3 (GreIntUnloadFontStub)
GRE_STUB_8 (GreIntQueryFontsStub)
GRE_STUB_5 (GreIntQueryFontAttributesStub)
GRE_STUB_6 (GreIntInstallIFIStub)
GRE_STUB_3 (GreIntLoadPublicFontStub)
GRE_STUB_3 (GreIntUnLoadPublicFontStub)
GRE_STUB_3 (GreIntQueryCodePageVectorStub)
GRE_STUB_5 (GreIntQueryFontFileDescriptionsStub)
GRE_STUB_7 (GreIntQueryFaceStringStub)
GRE_STUB_3 (GreIntQueryFontActionStub)
GRE_STUB_3 (GreIntAccumulateJournalFileStub)
GRE_STUB_5 (GreIntCreateJournalFileStub)
GRE_STUB_3 (GreIntDeleteJournalFileStub)
GRE_STUB_4 (GreIntStartJournalFileStub)
GRE_STUB_4 (GreIntStopJournalFileStub)
GRE_STUB_4 (GreIntPlayJournalFileStub)
GRE_STUB_5 (GreIntOpenJournalFileStub)
GRE_STUB_6 (GreIntCreatePaletteStub)
GRE_STUB_3 (GreIntDeletePaletteStub)
GRE_STUB_4 (GreIntSelectPaletteStub)
GRE_STUB_7 (GreIntSetPaletteEntriesStub)
GRE_STUB_7 (GreIntAnimatePaletteStub)
GRE_STUB_3 (GreIntQueryPaletteStub)
GRE_STUB_7 (GreIntQueryPaletteInfoStub)
GRE_STUB_4 (GreIntSetPaletteOwnerStub)
GRE_STUB_4 (GreIntResizePaletteStub)
GRE_STUB_7 (GreIntQueryDefaultPaletteInfoStub)
GRE_STUB_5 (GreIntCreateFontHandleStub)
GRE_STUB_7 (GreIntQueryFontHandleStub)
GRE_STUB_4 (GreIntSelectFontHandleStub)
GRE_STUB_3 (GreIntDeleteFontHandleStub)
GRE_STUB_4 (GreIntSetFontHandleOwnerStub)
GRE_STUB_5 (GreIntQueryCodePageObjectStub)
GRE_STUB_7 (GreIntQueryCharOutlineStub)
GRE_STUB_8 (GreIntRealizeStringStub)
GRE_STUB_8 (GreIntQueryCharMetricsTableStub)
GRE_STUB_5 (GreIntBeginInkPathStub)
GRE_STUB_4 (GreIntEndInkPathStub)
GRE_STUB_7 (GreIntStrokeInkPathStub)
GRE_STUB_3 (GreIntPaxGetPolyFillModeStub)
GRE_STUB_4 (GreIntPaxSetPolyFillModeStub)
GRE_STUB_3 (GreIntQueryMitreLimitStub)
GRE_STUB_4 (GreIntSetMitreLimitStub)
GRE_STUB_4 (GreIntEnableYInversionStub)
GRE_STUB_3 (GreIntQueryYInversionStub)
GRE_STUB_6 (GreIntQueryPathStub)
GRE_STUB_4 (GreIntAllocateDCDataStub)
GRE_STUB_3 (GreIntQueryDCDataStub)
GRE_STUB_3 (GreIntQueryRasterCapsStub)
GRE_STUB_6 (GreIntPaxStrokeAndFillPathStub)
GRE_STUB_4 (GreIntQueryNearestPaletteIndexStub)
GRE_STUB_4 (GreIntSetFontMappingFlagsStub)
GRE_STUB_7 (GreIntQueryTabbedTextExtentStub)
GRE_STUB_10(GreIntTabbedCharStringAtStub)
GRE_STUB_4 (GreIntUnloadPDStub)

/* ======================================================================== */
/*  Internal dispatch table — static initializer                         */
/* ======================================================================== */

/*! @brief Driver dispatch table (0x40xx / 0x50xx / 0x60xx / 0x70xx).
 *
 * Statically initialized:
 *   - slots with an NGre* constant: per-function stub
 *   - empty slots (no NGre* constant): GreDrvEmptyStub
 *
 * Presentation driver overwrites the slots it hooks in FillLogicalDeviceBlock.
 * Everything else returns GPI_ERROR via the stub. */
static PFN g_tblDriver[MAXTABLEENTRIES] = {
    /* 0x00 0x4000 */ (PFN)GreDrvGetArcParameters,
    /* 0x01 0x4001 */ (PFN)GreDrvSetArcParameters,
    /* 0x02 0x4002 */ (PFN)GreDrvArc,
    /* 0x03 0x4003 */ (PFN)GreDrvPartialArc,
    /* 0x04 0x4004 */ (PFN)GreDrvFullArcInterior,
    /* 0x05 0x4005 */ (PFN)GreDrvFullArcBoundary,
    /* 0x06 0x4006 */ (PFN)GreDrvFullArcBoth,
    /* 0x07 0x4007 */ (PFN)GreDrvBoxInterior,
    /* 0x08 0x4008 */ (PFN)GreDrvBoxBoundary,
    /* 0x09 0x4009 */ (PFN)GreDrvBoxBoth,
    /* 0x0A 0x400A */ (PFN)GreDrvPolyFillet,
    /* 0x0B 0x400B */ (PFN)GreDrvPolyFilletSharp,
    /* 0x0C 0x400C */ (PFN)GreDrvPolySpline,
    /* 0x0D 0x400D */ (PFN)GreDrvOval,
    /* 0x0E 0x400E */ (PFN)GreDrvCookWholePath,
    /* 0x0F 0x400F */ (PFN)GreDrvCookPathCurves,
    /* 0x10 0x4010 */ (PFN)GreDrvBoxExclusive,
    /* 0x11 0x4011 */ (PFN)GreDrvRenderPath,
    /* 0x12 0x4012 */ (PFN)GreDrvOpenScreenChangeArea,
    /* 0x13 0x4013 */ (PFN)GreDrvGetScreenChangeArea,
    /* 0x14 0x4014 */ (PFN)GreDrvCloseScreenChangeArea,
    /* 0x15 -------- */ NULL,
    /* 0x16 0x4016 */ (PFN)GreDrvDisjointLines,
    /* 0x17 0x4017 */ (PFN)GreDrvGetCurrentPosition,
    /* 0x18 0x4018 */ (PFN)GreDrvSetCurrentPosition,
    /* 0x19 0x4019 */ (PFN)GreDrvPolyLine,
    /* 0x1A 0x401A */ (PFN)GreDrvDrawLinesInPath,
    /* 0x1B 0x401B */ (PFN)GreDrvPolyShortLine,
    /* 0x1C 0x401C */ (PFN)GreDrvPolyScanline,
    /* 0x1D 0x401D */ (PFN)GreDrvGetScreenBits,
    /* 0x1E 0x401E */ (PFN)GreDrvSetScreenBits,
    /* 0x1F -------- */ NULL,
    /* 0x20 -------- */ NULL,
    /* 0x21 -------- */ NULL,
    /* 0x22 0x6022 */ (PFN)GreDrvDrawBits,
    /* 0x23 0x6023 */ (PFN)GreDrvDeviceCreateBitmap,
    /* 0x24 0x4024 */ (PFN)GreDrvDeviceDeleteBitmap,
    /* 0x25 0x4025 */ (PFN)GreDrvDeviceSelectBitmap,
    /* 0x26 0x6026 */ (PFN)GreDrvBitblt,
    /* 0x27 0x6027 */ (PFN)GreDrvGetPel,
    /* 0x28 0x4028 */ (PFN)GreDrvSetPel,
    /* 0x29 0x4029 */ (PFN)GreDrvImageData,
    /* 0x2A 0x602A */ (PFN)GreDrvScanLR,
    /* 0x2B 0x602B */ (PFN)GreDrvFloodFill,
    /* 0x2C 0x402C */ (PFN)GreDrvSaveScreenBits,
    /* 0x2D 0x402D */ (PFN)GreDrvRestoreScreenBits,
    /* 0x2E 0x602E */ (PFN)GreDrvDrawBorder,
    /* 0x2F 0x402F */ (PFN)GreDrvDeviceSetCursor,
    /* 0x30 0x6030 */ (PFN)GreDrvGetBitmapBits,
    /* 0x31 0x6031 */ (PFN)GreDrvSetBitmapBits,
    /* 0x32 0x4032 */ (PFN)GreDrvSetColorCursor,
    /* 0x33 -------- */ NULL,
    /* 0x34 -------- */ NULL,
    /* 0x35 0x5035 */ (PFN)GreDrvCharString,
    /* 0x36 0x7036 */ (PFN)GreDrvCharStringPos,
    /* 0x37 0x5037 */ (PFN)GreDrvQueryTextBox,
    /* 0x38 0x5038 */ (PFN)GreDrvQueryCharPositions,
    /* 0x39 0x5039 */ (PFN)GreDrvQueryWidthTable,
    /* 0x3A 0x403A */ (PFN)GreDrvPolyMarker,
    /* 0x3B 0x403B */ (PFN)GreDrvCharRect,
    /* 0x3C 0x403C */ (PFN)GreDrvCharStr,
    /* 0x3D 0x403D */ (PFN)GreDrvScrollRect,
    /* 0x3E 0x403E */ (PFN)GreDrvUpdateCursor,
    /* 0x3F 0x403F */ (PFN)GreDrvDeviceSetPointer,
    /* 0x40 -------- */ NULL,
    /* 0x41 -------- */ NULL,
    /* 0x42 -------- */ NULL,
    /* 0x43 -------- */ NULL,
    /* 0x44 -------- */ NULL,
    /* 0x45 -------- */ NULL,
    /* 0x46 0x4046 */ (PFN)GreDrvBeginArea,
    /* 0x47 0x4047 */ (PFN)GreDrvEndArea,
    /* 0x48 0x4048 */ (PFN)GreDrvBeginPath,
    /* 0x49 0x4049 */ (PFN)GreDrvEndPath,
    /* 0x4A 0x404A */ (PFN)GreDrvCloseFigure,
    /* 0x4B 0x404B */ (PFN)GreDrvFillPath,
    /* 0x4C 0x404C */ (PFN)GreDrvOutlinePath,
    /* 0x4D 0x404D */ (PFN)GreDrvModifyPath,
    /* 0x4E 0x404E */ (PFN)GreDrvStrokePath,
    /* 0x4F 0x404F */ (PFN)GreDrvSelectClipPath,
    /* 0x50 0x4050 */ (PFN)GreDrvSavePath,
    /* 0x51 0x4051 */ (PFN)GreDrvRestorePath,
    /* 0x52 0x4052 */ (PFN)GreDrvClip1DPath,
    /* 0x53 0x4053 */ (PFN)GreDrvDrawRawPath,
    /* 0x54 0x4054 */ (PFN)GreDrvDrawCookedPath,
    /* 0x55 0x6055 */ (PFN)GreDrvAreaSetAttributes,
    /* 0x56 0x4056 */ (PFN)GreDrvPolygonSet,
    /* 0x57 0x4057 */ (PFN)GreDrvPathToRegion,
    /* 0x58 0x4058 */ (PFN)GreDrvDrawRLE,
    /* 0x59 0x4059 */ (PFN)GreDrvDevicePolygonSet,
    /* 0x5A -------- */ NULL,
    /* 0x5B -------- */ NULL,
    /* 0x5C 0x405C */ (PFN)GreDrvPolygon3d,
    /* 0x5D 0x405D */ (PFN)GreDrvGetRegionBox,
    /* 0x5E 0x405E */ (PFN)GreDrvGetRegionRects,
    /* 0x5F 0x405F */ (PFN)GreDrvOffsetRegion,
    /* 0x60 0x4060 */ (PFN)GreDrvPtInRegion,
    /* 0x61 0x4061 */ (PFN)GreDrvRectInRegion,
    /* 0x62 0x4062 */ (PFN)GreDrvCreateRectRegion,
    /* 0x63 0x4063 */ (PFN)GreDrvDestroyRegion,
    /* 0x64 0x4064 */ (PFN)GreDrvSetRectRegion,
    /* 0x65 0x4065 */ (PFN)GreDrvCombineRegion,
    /* 0x66 0x4066 */ (PFN)GreDrvCombineRectRegion,
    /* 0x67 0x4067 */ (PFN)GreDrvCombineShortLineRegion,
    /* 0x68 0x4068 */ (PFN)GreDrvEqualRegion,
    /* 0x69 0x4069 */ (PFN)GreDrvPaintRegion,
    /* 0x6A 0x406A */ (PFN)GreDrvSetRegionOwner,
    /* 0x6B 0x406B */ (PFN)GreDrvFrameRegion,
    /* 0x6C -------- */ NULL,
    /* 0x6D -------- */ NULL,
    /* 0x6E 0x406E */ (PFN)GreDrvGetClipBox,
    /* 0x6F 0x406F */ (PFN)GreDrvGetClipRects,
    /* 0x70 0x4070 */ (PFN)GreDrvOffsetClipRegion,
    /* 0x71 0x4071 */ (PFN)GreDrvPtVisible,
    /* 0x72 0x4072 */ (PFN)GreDrvRectVisible,
    /* 0x73 0x4073 */ (PFN)GreDrvQueryClipRegion,
    /* 0x74 0x4074 */ (PFN)GreDrvSelectClipRegion,
    /* 0x75 0x4075 */ (PFN)GreDrvIntersectClipRectangle,
    /* 0x76 0x4076 */ (PFN)GreDrvExcludeClipRectangle,
    /* 0x77 0x4077 */ (PFN)GreDrvSetXformRect,
    /* 0x78 -------- */ NULL,
    /* 0x79 -------- */ NULL,
    /* 0x7A -------- */ NULL,
    /* 0x7B 0x407B */ (PFN)GreDrvSaveRegion,
    /* 0x7C 0x407C */ (PFN)GreDrvRestoreRegion,
    /* 0x7D 0x407D */ (PFN)GreDrvClipPathCurves,
    /* 0x7E 0x407E */ (PFN)GreDrvSelectPathRegion,
    /* 0x7F 0x407F */ (PFN)GreDrvRegionSelectBitmap,
    /* 0x80 0x4080 */ (PFN)GreDrvCopyClipRegion,
    /* 0x81 0x4081 */ (PFN)GreDrvSetupDC,
    /* 0x82 -------- */ NULL,
    /* 0x83 0x4083 */ (PFN)GreDrvGetPageUnits,
    /* 0x84 0x4084 */ (PFN)GreDrvSetPageUnits,
    /* 0x85 0x4085 */ (PFN)GreDrvGetModelXform,
    /* 0x86 0x4086 */ (PFN)GreDrvSetModelXform,
    /* 0x87 0x4087 */ (PFN)GreDrvGetWindowViewportXform,
    /* 0x88 0x4088 */ (PFN)GreDrvSetWindowViewportXform,
    /* 0x89 0x4089 */ (PFN)GreDrvGetGlobalViewingXform,
    /* 0x8A 0x408A */ (PFN)GreDrvSetGlobalViewingXform,
    /* 0x8B 0x408B */ (PFN)GreDrvSaveXformData,
    /* 0x8C 0x408C */ (PFN)GreDrvRestoreXformData,
    /* 0x8D 0x408D */ (PFN)GreDrvGetPageViewport,
    /* 0x8E 0x408E */ (PFN)GreDrvSetPageViewport,
    /* 0x8F -------- */ NULL,
    /* 0x90 -------- */ NULL,
    /* 0x91 0x4091 */ (PFN)GreDrvGetGraphicsField,
    /* 0x92 0x4092 */ (PFN)GreDrvSetGraphicsField,
    /* 0x93 0x4093 */ (PFN)GreDrvGetViewingLimits,
    /* 0x94 0x4094 */ (PFN)GreDrvSetViewingLimits,
    /* 0x95 0x4095 */ (PFN)GreDrvQueryViewportSize,
    /* 0x96 0x4096 */ (PFN)GreDrvConvert,
    /* 0x97 0x4097 */ (PFN)GreDrvConvertPath,
    /* 0x98 0x4098 */ (PFN)GreDrvSaveXform,
    /* 0x99 0x4099 */ (PFN)GreDrvRestoreXform,
    /* 0x9A 0x409A */ (PFN)GreDrvMultiplyXforms,
    /* 0x9B 0x409B */ (PFN)GreDrvConvertWithMatrix,
    /* 0x9C -------- */ NULL,
    /* 0x9D 0x609D */ (PFN)GreDrvDeviceGetAttributes,
    /* 0x9E 0x409E */ (PFN)GreDrvDeviceSetAVIOFont2,
    /* 0x9F -------- */ NULL,
    /* 0xA0 0x40A0 */ (PFN)GreDrvGetPairKerningTable,
    /* 0xA1 0x40A1 */ (PFN)GreDrvDeviceSetAVIOFont,
    /* 0xA2 0x60A2 */ (PFN)GreDrvDeviceSetAttributes,
    /* 0xA3 0x60A3 */ (PFN)GreDrvDeviceSetGlobalAttribute,
    /* 0xA4 0x40A4 */ (PFN)GreDrvNotifyClipChange, // Mandatory, driver must chain
    /* 0xA5 0x40A5 */ (PFN)GreDrvNotifyTransformChange,
    /* 0xA6 0x40A6 */ (PFN)GreDrvRealizeFont,
    /* 0xA7 0x40A7 */ (PFN)GreDrvErasePS,
    /* 0xA8 0x40A8 */ (PFN)GreDrvSetStyleRatio,
    /* 0xA9 0x40A9 */ (PFN)GreDrvDeviceQueryFontAttributes,
    /* 0xAA 0x40AA */ (PFN)GreDrvDeviceQueryFonts,
    /* 0xAB 0x40AB */ (PFN)GreDrvDeviceInvalidateVisRegion,
    /* 0xAC 0x40AC */ (PFN)GreDrvGetPickWindow,
    /* 0xAD 0x40AD */ (PFN)GreDrvSetPickWindow,
    /* 0xAE 0x40AE */ (PFN)GreDrvResetBounds,
    /* 0xAF 0x40AF */ (PFN)GreDrvGetBoundsData,
    /* 0xB0 0x40B0 */ (PFN)GreDrvAccumulateBounds,
    /* 0xB1 0x40B1 */ (PFN)GreDrvGetExtraError,
    /* 0xB2 0x40B2 */ (PFN)GreDrvSetExtraError,
    /* 0xB3 0x40B3 */ (PFN)GreDrvGetCodePage,
    /* 0xB4 0x40B4 */ (PFN)GreDrvSetCodePage,
    /* 0xB5 0x40B5 */ (PFN)GreDrvLockDevice,
    /* 0xB6 0x40B6 */ (PFN)GreDrvUnlockDevice,
    /* 0xB7 0x40B7 */ (PFN)GreDrvDeath,
    /* 0xB8 0x40B8 */ (PFN)GreDrvResurrection,
    /* 0xB9 -------- */ NULL,
    /* 0xBA 0x40BA */ (PFN)GreDrvGetDCOrigin,
    /* 0xBB 0x40BB */ NULL, // Mandatory
    /* 0xBC 0x40BC */ (PFN)GreDrvGetLineOrigin,
    /* 0xBD 0x40BD */ (PFN)GreDrvSetLineOrigin,
    /* 0xBE 0x40BE */ (PFN)GreDrvGetStyleRatio,
    /* 0xBF -------- */ NULL,
    /* 0xC0 -------- */ NULL,
    /* 0xC1 -------- */ NULL,
    /* 0xC2 -------- */ NULL,
    /* 0xC3 0x60C3 */ (PFN)GreDrvQueryColorData,
    /* 0xC4 0x60C4 */ (PFN)GreDrvQueryLogColorTable,
    /* 0xC5 0x60C5 */ (PFN)GreDrvCreateLogColorTable,
    /* 0xC6 0x60C6 */ (PFN)GreDrvRealizeColorTable,
    /* 0xC7 0x60C7 */ (PFN)GreDrvUnrealizeColorTable,
    /* 0xC8 0x40C8 */ (PFN)GreDrvQueryRealColors,
    /* 0xC9 0x40C9 */ (PFN)GreDrvQueryNearestColor,
    /* 0xCA 0x60CA */ (PFN)GreDrvQueryColorIndex,
    /* 0xCB 0x60CB */ (PFN)GreDrvQueryRGBColor,
    /* 0xCC 0x60CC */ (PFN)GreDrvQueryDevFntMetrics,
    /* 0xCD 0x40CD */ (PFN)GreDrvDeviceEnableYInversion,
    /* 0xCE -------- */ NULL,
    /* 0xCF -------- */ NULL,
    /* 0xD0 0x40D0 */ (PFN)GreDrvQueryDeviceBitmaps,
    /* 0xD1 0x40D1 */ NULL, // Mandatory
    /* 0xD2 0x40D2 */ (PFN)GreDrvEscape,
    /* 0xD3 0x40D3 */ (PFN)GreDrvQueryHardcopyCaps,
    /* 0xD4 0x40D4 */ (PFN)GreDrvQueryDevResource2,
    /* 0xD5 0x40D5 */ (PFN)GreDrvDeviceCreatePalette,
    /* 0xD6 0x40D6 */ (PFN)GreDrvDeviceDeletePalette,
    /* 0xD7 0x40D7 */ (PFN)GreDrvDeviceSetPaletteEntries,
    /* 0xD8 0x40D8 */ (PFN)GreDrvDeviceAnimatePalette,
    /* 0xD9 0x40D9 */ (PFN)GreDrvDeviceResizePalette,
    /* 0xDA 0x40DA */ (PFN)GreDrvRealizePalette,
    /* 0xDB 0x40DB */ (PFN)GreDrvQueryHWPaletteInfo,
    /* 0xDC 0x40DC */ (PFN)GreDrvUpdateColors,
    /* 0xDD 0x40DD */ (PFN)GreDrvQueryPaletteRealization,
    /* 0xDE 0x40DE */ (PFN)GreDrvGetVisRects
};

/*!
 * @brief Internal dispatch table (0x02xx / 0x42xx).
 *
 */
static PFN g_tblInternal[GRE_DISPATCH_SIZE] = {
    /* 0x00 */ (PFN)GreIntOpenDC,
    /* 0x01 */ (PFN)GreIntCloseDC,
    /* 0x02 */ (PFN)GreIntResetDCStub,
    /* 0x03 */ (PFN)GreIntGetHandleStub,
    /* 0x04 */ (PFN)GreIntSetHandleStub,
    /* 0x05 */ (PFN)GreIntGetProcessControlStub,
    /* 0x06 */ (PFN)GreIntSetProcessControlStub,
    /* 0x07 */ (PFN)GreIntSaveDCStub,
    /* 0x08 */ (PFN)GreIntRestoreDCStub,
    /* 0x09 */ (PFN)GreIntQueryDeviceNames,
    /* 0x0A */ (PFN)GreIntQueryEngineVersion,
    /* 0x0B */ (PFN)GreIntSetDCOwner,
    /* 0x0C */ (PFN)GreIntQueryBitmapSelectionStub,
    /* 0x0D */ (PFN)GreIntGetBitmapInfoHeaderStub,
    /* 0x0E */ (PFN)GreIntConvertBitmapDataStub,
    /* 0x0F */ (PFN)GreIntInvalidateVisRegionStub,
    /* 0x10 */ (PFN)GreIntCreateBitmapStub,
    /* 0x11 */ (PFN)GreIntDeleteBitmapStub,
    /* 0x12 */ (PFN)GreIntSelectBitmapStub,
    /* 0x13 */ (PFN)GreIntGetBitmapParametersStub,
    /* 0x14 */ (PFN)GreIntGetBitmapDimensionStub,
    /* 0x15 */ (PFN)GreIntSetBitmapDimensionStub,
    /* 0x16 */ (PFN)GreIntSetCursorStub,
    /* 0x17 */ (PFN)GreIntGetAttributesStub,
    /* 0x18 */ (PFN)GreIntSetAttributesStub,
    /* 0x19 */ (PFN)GreIntSetGlobalAttributeStub,
    /* 0x1A */ (PFN)GreIntSetBitmapOwnerStub,
    /* 0x1B */ (PFN)GreIntGetDefaultAttributesStub,
    /* 0x1C */ (PFN)GreIntSetDefaultAttributesStub,
    /* 0x1D */ (PFN)GreIntGetDefaultArcParametersStub,
    /* 0x1E */ (PFN)GreIntSetDefaultArcParametersStub,
    /* 0x1F */ (PFN)GreIntGetDefaultViewingLimitsStub,
    /* 0x20 */ (PFN)GreIntSetDefaultViewingLimitsStub,
    /* 0x21 */ (PFN)GreIntInitializeAttributesStub,
    /* 0x22 */ (PFN)GreIntDeleteSetIdStub,
    /* 0x23 */ (PFN)GreIntQueryNumberSetIdsStub,
    /* 0x24 */ (PFN)GreIntQuerySetIdsStub,
    /* 0x25 */ (PFN)GreIntQueryBitmapHandleStub,
    /* 0x26 */ (PFN)GreIntSetBitmapIDStub,
    /* 0x27 */ (PFN)GreIntCopyDCLoadDataStub,
    /* 0x28 */ (PFN)GreIntQuerySetIdUsageStub,
    /* 0x29 */ (PFN)GreIntGetHandleStub,    /* placeholder; no API */
    /* 0x2A */ (PFN)GreIntGetHandleStub,    /* placeholder; no API */
    /* 0x2B */ (PFN)GreIntQueryFullFontFileDescriptionsStub,
    /* 0x2C */ (PFN)GreIntQueryLogicalFontStub,
    /* 0x2D */ (PFN)GreIntCreateLogicalFontStub,
    /* 0x2E */ (PFN)GreIntLoadFontStub,
    /* 0x2F */ (PFN)GreIntUnloadFontStub,
    /* 0x30 */ (PFN)GreIntQueryFontsStub,
    /* 0x31 */ (PFN)GreIntQueryFontAttributesStub,
    /* 0x32 */ (PFN)GreIntInstallIFIStub,
    /* 0x33 */ (PFN)GreIntLoadPublicFontStub,
    /* 0x34 */ (PFN)GreIntUnLoadPublicFontStub,
    /* 0x35 */ (PFN)GreIntQueryCodePageVectorStub,
    /* 0x36 */ (PFN)GreIntQueryFontFileDescriptionsStub,
    /* 0x37 */ (PFN)GreIntQueryFaceStringStub,
    /* 0x38 */ (PFN)GreIntQueryFontActionStub,
    /* 0x39 */ (PFN)GreIntAccumulateJournalFileStub,
    /* 0x3A */ (PFN)GreIntCreateJournalFileStub,
    /* 0x3B */ (PFN)GreIntDeleteJournalFileStub,
    /* 0x3C */ (PFN)GreIntStartJournalFileStub,
    /* 0x3D */ (PFN)GreIntStopJournalFileStub,
    /* 0x3E */ (PFN)GreIntPlayJournalFileStub,
    /* 0x3F */ (PFN)GreIntOpenJournalFileStub,
    /* 0x40 */ (PFN)GreIntCreatePaletteStub,
    /* 0x41 */ (PFN)GreIntDeletePaletteStub,
    /* 0x42 */ (PFN)GreIntSelectPaletteStub,
    /* 0x43 */ (PFN)GreIntSetPaletteEntriesStub,
    /* 0x44 */ (PFN)GreIntAnimatePaletteStub,
    /* 0x45 */ (PFN)GreIntQueryPaletteStub,
    /* 0x46 */ (PFN)GreIntQueryPaletteInfoStub,
    /* 0x47 */ (PFN)GreIntSetPaletteOwnerStub,
    /* 0x48 */ (PFN)GreIntResizePaletteStub,
    /* 0x49 */ (PFN)GreIntQueryDefaultPaletteInfoStub,
    /* 0x4A */ (PFN)GreIntGetHandleStub,    /* placeholder */
    /* 0x4B */ (PFN)GreIntGetHandleStub,
    /* 0x4C */ (PFN)GreIntGetHandleStub,
    /* 0x4D */ (PFN)GreIntGetHandleStub,
    /* 0x4E */ (PFN)GreIntGetHandleStub,
    /* 0x4F */ (PFN)GreIntGetHandleStub,
    /* 0x50 */ (PFN)GreIntCreateFontHandleStub,
    /* 0x51 */ (PFN)GreIntQueryFontHandleStub,
    /* 0x52 */ (PFN)GreIntSelectFontHandleStub,
    /* 0x53 */ (PFN)GreIntDeleteFontHandleStub,
    /* 0x54 */ (PFN)GreIntSetFontHandleOwnerStub,
    /* 0x55 */ (PFN)GreIntQueryCodePageObjectStub,
    /* 0x56 */ (PFN)GreIntQueryCharOutlineStub,
    /* 0x57 */ (PFN)GreIntRealizeStringStub,
    /* 0x58 */ (PFN)GreIntQueryCharMetricsTableStub,
    /* 0x59 */ (PFN)GreIntBeginInkPathStub,
    /* 0x5A */ (PFN)GreIntEndInkPathStub,
    /* 0x5B */ (PFN)GreIntStrokeInkPathStub,
    /* 0x5C */ (PFN)GreIntPaxGetPolyFillModeStub,
    /* 0x5D */ (PFN)GreIntPaxSetPolyFillModeStub,
    /* 0x5E */ (PFN)GreIntQueryMitreLimitStub,
    /* 0x5F */ (PFN)GreIntSetMitreLimitStub,
    /* 0x60 */ (PFN)GreIntEnableYInversionStub,
    /* 0x61 */ (PFN)GreIntQueryYInversionStub,
    /* 0x62 */ (PFN)GreIntQueryPathStub,
    /* 0x63 */ (PFN)GreIntAllocateDCDataStub,
    /* 0x64 */ (PFN)GreIntQueryDCDataStub,
    /* 0x65 */ (PFN)GreIntQueryRasterCapsStub,
    /* 0x66 */ (PFN)GreIntPaxStrokeAndFillPathStub,
    /* 0x67 */ (PFN)GreIntQueryNearestPaletteIndexStub,
    /* 0x68 */ (PFN)GreIntSetFontMappingFlagsStub,
    /* 0x69 */ (PFN)GreIntQueryTabbedTextExtentStub,
    /* 0x6A */ (PFN)GreIntTabbedCharStringAtStub,
    /* 0x6B */ (PFN)GreIntResetDC2Stub,
    /* 0x6C */ (PFN)GreIntGetHandleStub,    /* placeholder */
    /* 0x6D */ (PFN)GreIntGetHandleStub,
    /* 0x6E */ (PFN)GreIntUnloadPDStub,
    /* 0x6F */ (PFN)GreIntGetHandleStub,    /* placeholder */
    /* 0x70 */ (PFN)GreIntGetHandleStub,
    /* 0x71 */ (PFN)GreIntGetHandleStub,
    /* 0x72 */ (PFN)GreIntGetHandleStub,
    /* 0x73 */ (PFN)GreIntGetHandleStub,
    /* 0x74 */ (PFN)GreIntGetHandleStub,
    /* 0x75 */ (PFN)GreIntGetHandleStub,
    /* 0x76 */ (PFN)GreIntGetHandleStub,
    /* 0x77 */ (PFN)GreIntGetHandleStub,
    /* 0x78 */ (PFN)GreIntGetHandleStub,
    /* 0x79 */ (PFN)GreIntGetHandleStub,
    /* 0x7A */ (PFN)GreIntGetHandleStub,
    /* 0x7B */ (PFN)GreIntGetHandleStub,
    /* 0x7C */ (PFN)GreIntGetHandleStub,
    /* 0x7D */ (PFN)GreIntGetHandleStub,
    /* 0x7E */ (PFN)GreIntGetHandleStub,
    /* 0x7F */ (PFN)GreIntGetHandleStub,
    /* 0x80 */ (PFN)GreIntGetHandleStub,
    /* 0x81 */ (PFN)GreIntGetHandleStub,
    /* 0x82 */ (PFN)GreIntGetHandleStub,
    /* 0x83 */ (PFN)GreIntGetHandleStub,
    /* 0x84 */ (PFN)GreIntGetHandleStub,
    /* 0x85 */ (PFN)GreIntGetHandleStub,
    /* 0x86 */ (PFN)GreIntGetHandleStub,
    /* 0x87 */ (PFN)GreIntGetHandleStub,
    /* 0x88 */ (PFN)GreIntGetHandleStub,
    /* 0x89 */ (PFN)GreIntGetHandleStub,
    /* 0x8A */ (PFN)GreIntGetHandleStub,
    /* 0x8B */ (PFN)GreIntGetHandleStub,
    /* 0x8C */ (PFN)GreIntGetHandleStub,
    /* 0x8D */ (PFN)GreIntGetHandleStub,
    /* 0x8E */ (PFN)GreIntGetHandleStub,
    /* 0x8F */ (PFN)GreIntGetHandleStub,
    /* 0x90 */ (PFN)GreIntGetHandleStub,
    /* 0x91 */ (PFN)GreIntGetHandleStub,
    /* 0x92 */ (PFN)GreIntGetHandleStub,
    /* 0x93 */ (PFN)GreIntGetHandleStub,
    /* 0x94 */ (PFN)GreIntGetHandleStub,
    /* 0x95 */ (PFN)GreIntGetHandleStub,
    /* 0x96 */ (PFN)GreIntGetHandleStub,
    /* 0x97 */ (PFN)GreIntGetHandleStub,
    /* 0x98 */ (PFN)GreIntGetHandleStub,
    /* 0x99 */ (PFN)GreIntGetHandleStub,
    /* 0x9A */ (PFN)GreIntGetHandleStub,
    /* 0x9B */ (PFN)GreIntGetHandleStub,
    /* 0x9C */ (PFN)GreIntGetHandleStub,
    /* 0x9D */ (PFN)GreIntGetHandleStub,
    /* 0x9E */ (PFN)GreIntGetHandleStub,
    /* 0x9F */ (PFN)GreIntGetHandleStub,
    /* 0xA0 */ (PFN)GreIntGetHandleStub,
    /* 0xA1 */ (PFN)GreIntGetHandleStub,
    /* 0xA2 */ (PFN)GreIntGetHandleStub,
    /* 0xA3 */ (PFN)GreIntGetHandleStub,
    /* 0xA4 */ (PFN)GreIntGetHandleStub,
    /* 0xA5 */ (PFN)GreIntGetHandleStub,
    /* 0xA6 */ (PFN)GreIntGetHandleStub,
    /* 0xA7 */ (PFN)GreIntGetHandleStub,
    /* 0xA8 */ (PFN)GreIntGetHandleStub,
    /* 0xA9 */ (PFN)GreIntGetHandleStub,
    /* 0xAA */ (PFN)GreIntGetHandleStub,
    /* 0xAB */ (PFN)GreIntGetHandleStub,
    /* 0xAC */ (PFN)GreIntGetHandleStub,
    /* 0xAD */ (PFN)GreIntGetHandleStub,
    /* 0xAE */ (PFN)GreIntGetHandleStub,
    /* 0xAF */ (PFN)GreIntGetHandleStub,
    /* 0xB0 */ (PFN)GreIntGetHandleStub,
    /* 0xB1 */ (PFN)GreIntGetHandleStub,
    /* 0xB2 */ (PFN)GreIntGetHandleStub,
    /* 0xB3 */ (PFN)GreIntGetHandleStub,
    /* 0xB4 */ (PFN)GreIntGetHandleStub,
    /* 0xB5 */ (PFN)GreIntGetHandleStub,
    /* 0xB6 */ (PFN)GreIntGetHandleStub,
    /* 0xB7 */ (PFN)GreIntGetHandleStub,
    /* 0xB8 */ (PFN)GreIntGetHandleStub,
    /* 0xB9 */ (PFN)GreIntGetHandleStub,
    /* 0xBA */ (PFN)GreIntGetHandleStub,
    /* 0xBB */ (PFN)GreIntGetHandleStub,
    /* 0xBC */ (PFN)GreIntGetHandleStub,
    /* 0xBD */ (PFN)GreIntGetHandleStub,
    /* 0xBE */ (PFN)GreIntGetHandleStub,
    /* 0xBF */ (PFN)GreIntGetHandleStub,
    /* 0xC0 */ (PFN)GreIntGetHandleStub,
    /* 0xC1 */ (PFN)GreIntGetHandleStub,
    /* 0xC2 */ (PFN)GreIntGetHandleStub,
    /* 0xC3 */ (PFN)GreIntGetHandleStub,
    /* 0xC4 */ (PFN)GreIntGetHandleStub,
    /* 0xC5 */ (PFN)GreIntGetHandleStub,
    /* 0xC6 */ (PFN)GreIntGetHandleStub,
    /* 0xC7 */ (PFN)GreIntGetHandleStub,
    /* 0xC8 */ (PFN)GreIntGetHandleStub,
    /* 0xC9 */ (PFN)GreIntGetHandleStub,
    /* 0xCA */ (PFN)GreIntGetHandleStub,
    /* 0xCB */ (PFN)GreIntGetHandleStub,
    /* 0xCC */ (PFN)GreIntGetHandleStub,
    /* 0xCD */ (PFN)GreIntGetHandleStub,
    /* 0xCE */ (PFN)GreIntGetHandleStub,
    /* 0xCF */ (PFN)GreIntGetHandleStub,
    /* 0xD0 */ (PFN)GreIntGetHandleStub,
    /* 0xD1 */ (PFN)GreIntGetHandleStub,
    /* 0xD2 */ (PFN)GreIntGetHandleStub,
    /* 0xD3 */ (PFN)GreIntGetHandleStub,
    /* 0xD4 */ (PFN)GreIntGetHandleStub,
    /* 0xD5 */ (PFN)GreIntGetHandleStub,
    /* 0xD6 */ (PFN)GreIntGetHandleStub,
    /* 0xD7 */ (PFN)GreIntGetHandleStub,
    /* 0xD8 */ (PFN)GreIntGetHandleStub,
    /* 0xD9 */ (PFN)GreIntGetHandleStub,
    /* 0xDA */ (PFN)GreIntGetHandleStub,
    /* 0xDB */ (PFN)GreIntGetHandleStub,
    /* 0xDC */ (PFN)GreIntGetHandleStub,
    /* 0xDD */ (PFN)GreIntGetHandleStub,
    /* 0xDE */ (PFN)GreIntGetHandleStub,
    /* 0xDF */ (PFN)GreIntGetHandleStub,
    /* 0xE0 */ (PFN)GreIntGetHandleStub,
    /* 0xE1 */ (PFN)GreIntGetHandleStub,
    /* 0xE2 */ (PFN)GreIntGetHandleStub,
    /* 0xE3 */ (PFN)GreIntGetHandleStub,
    /* 0xE4 */ (PFN)GreIntGetHandleStub,
    /* 0xE5 */ (PFN)GreIntGetHandleStub,
    /* 0xE6 */ (PFN)GreIntGetHandleStub,
    /* 0xE7 */ (PFN)GreIntGetHandleStub,
    /* 0xE8 */ (PFN)GreIntGetHandleStub,
    /* 0xE9 */ (PFN)GreIntGetHandleStub,
    /* 0xEA */ (PFN)GreIntGetHandleStub,
    /* 0xEB */ (PFN)GreIntGetHandleStub,
    /* 0xEC */ (PFN)GreIntGetHandleStub,
    /* 0xED */ (PFN)GreIntGetHandleStub,
    /* 0xEE */ (PFN)GreIntGetHandleStub,
    /* 0xEF */ (PFN)GreIntGetHandleStub,
    /* 0xF0 */ (PFN)GreIntGetHandleStub,
    /* 0xF1 */ (PFN)GreIntGetHandleStub,
    /* 0xF2 */ (PFN)GreIntGetHandleStub,
    /* 0xF3 */ (PFN)GreIntGetHandleStub,
    /* 0xF4 */ (PFN)GreIntGetHandleStub,
    /* 0xF5 */ (PFN)GreIntGetHandleStub,
    /* 0xF6 */ (PFN)GreIntGetHandleStub,
    /* 0xF7 */ (PFN)GreIntGetHandleStub,
    /* 0xF8 */ (PFN)GreIntGetHandleStub,
    /* 0xF9 */ (PFN)GreIntGetHandleStub,
    /* 0xFA */ (PFN)GreIntGetHandleStub,
    /* 0xFB */ (PFN)GreIntGetHandleStub,
    /* 0xFC */ (PFN)GreIntGetHandleStub,
    /* 0xFD */ (PFN)GreIntGetHandleStub,
    /* 0xFE */ (PFN)GreIntGetHandleStub,
    /* 0xFF */ (PFN)GreIntGetHandleStub
};

/* ======================================================================== */
/*  Static helpers                                                        */
/* ======================================================================== */

/*!
 * @brief Count slots in g_tblDriver that the driver replaced.
 *
 * Compares g_tblDriver against the snapshot taken just before
 * FillLogicalDeviceBlock was called. Called after the driver has
 * filled the table, from LoadGre2Sdl.
 */
static int HookedCount(void)
{
    int i, n = 0;
    for (i = 0; i < MAXTABLEENTRIES; i++) {
        if (g_tblDriver[i] != g_tblDriverBefore[i]) n++;
    }
    return n;
}

/*!
 * @brief Load GRE2SDL.DLL and call OS2_PM_DRV_ENABLE(EN_FILL_LDB).
 *
 * Fills g_tblDriver via FLReturnsLst.DispTab.  The module is NOT
 * freed after load: the dispatch table points into the driver's code.
 *
 * @return TRUE if the driver was loaded and FillLogicalDeviceBlock
 *         succeeded, FALSE otherwise.
 */
static BOOL LoadGre2Sdl(void)
{
    CHAR         szError[256];
    APIRET       rc;
    FLParamsLst  flParams;
    FLReturnsLst flReturns;
    USHORT       fsDeviceFlags = 0;
    ULONG        drvRc;
    int          i;
    int          nBad;

    if (g_bDriverLoaded) return TRUE;

    memset(szError, 0, sizeof(szError));

    rc = DosLoadModule(szError, sizeof(szError), "GRE2SDL", &g_hmodGre2Sdl);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE] DosLoadModule(GRE2SDL) failed rc=%lu (%s)",
                   (unsigned long)rc, szError);
        return FALSE;
    }

    rc = DosQueryProcAddr(g_hmodGre2Sdl, 200, NULL, (PFN *)&g_pfnEnable);
    if (rc != 0 || g_pfnEnable == NULL) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE] GRE2SDL: ordinal 200 not found, rc=%lu",
                   (unsigned long)rc);
        DosFreeModule(g_hmodGre2Sdl);
        g_hmodGre2Sdl = 0;
        return FALSE;
    }

    /* Snapshot g_tblDriver before the driver overwrites slots. */
    memcpy(g_tblDriverBefore, g_tblDriver, sizeof(g_tblDriver));

    flParams.Version   = 0x00000210;   /* BCD 2.10 */
    flParams.TableSize = MAXTABLEENTRIES;

    flReturns.Flags   = &fsDeviceFlags;
    flReturns.DispTab = g_tblDriver;

    drvRc = g_pfnEnable(EN_FILL_LDB, &flParams, &flReturns);
    if (drvRc != 0) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE] GRE2SDL: FillLogicalDeviceBlock failed rc=%lu",
                   (unsigned long)drvRc);
        DosFreeModule(g_hmodGre2Sdl);
        g_hmodGre2Sdl = 0;
        g_pfnEnable   = NULL;
        return FALSE;
    }

    /* Verify that the driver filled all mandatory slots.  If any is
     * still NULL, the driver is incomplete and would crash on the
     * first call through that slot.  Reject it at load time instead. */
    nBad = 0;
    for (i = 0; i < MANDATORY_COUNT; i++) {
        ULONG fc = g_mandatorySlots[i];
        if (g_tblDriver[fc & 0xFF] == g_tblDriverBefore[fc & 0xFF]) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE] GRE2SDL: mandatory NGre 0x%04lx(%s) not hooked by driver",
                       (unsigned long)fc, GreFuncName(fc));
            nBad++;
        }
    }
    if (nBad > 0) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE] GRE2SDL: %d mandatory slot(s) missing, "
                   "rejecting driver", nBad);
        DosFreeModule(g_hmodGre2Sdl);
        g_hmodGre2Sdl = 0;
        g_pfnEnable   = NULL;
        return FALSE;
    }

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE] GRE2SDL loaded, flags=0x%04x, %d driver hooks",
               (unsigned)fsDeviceFlags, HookedCount());

    g_bDriverLoaded = TRUE;
    return TRUE;
}

/*!
 * @brief Create a DC slot and register the driver instance.
 *
 * Calls EnableDeviceContext and CompleteOpenDC on the driver.
 * ulType is passed through from GreOpenDC unchanged; the driver
 * decides whether to create a visible window (OD_DIRECT) or an
 * offscreen surface (OD_MEMORY).
 *
 * @param [in] iHab     Owning HAB id.
 * @param [in] ulType   DC type (OD_DIRECT / OD_MEMORY / ...).
 * @return HDC on success, (ULONG)-1 on failure.
 * @retval !=(ULONG)-1  Valid HDC.
 * @retval (ULONG)-1    Fs_DcList::Create failed or EnableDeviceContext failed.
 */
static ULONG NewDc(ULONG iHab, ULONG ulType)
{
    ULONG     newHdc = 0;
    Fs_Dc    *pDc;
    DENPARAMS den;
    PVOID     pInstance;

    if (!g_pfnEnable) return (ULONG)-1;

    pDc = g_greDcs.Create(0, iHab, NULL, 640, 480, 32, &newHdc);
    if (!pDc) return (ULONG)-1;

    den.ulStateInfo = 0;
    den.ulType      = ulType;
    den.ulHDC       = newHdc;

    pInstance = (PVOID)g_pfnEnable(EN_ENABLE_CTX, &den, NULL);
    if (pInstance == (PVOID)-1) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE] NewDc: EnableDeviceContext failed");
        g_greDcs.Destroy(newHdc);
        return (ULONG)-1;
    }

    g_pfnEnable(EN_COMPLETE_OPENDC, (PVOID)newHdc, pInstance);
    pDc->SetInstance(pInstance);
    return newHdc;
}

/*!
 * @brief Close a DC: BeginCloseDC + DisableDeviceContext + free slot.
 *
 * @param [in] hdc  DC handle.
 * @return TRUE if the DC was found and closed.
 * @retval TRUE   DC found, driver notified, slot freed.
 * @retval FALSE  No DC with this hdc.
 */
static BOOL DestroyDc(ULONG hdc)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    if (!pDc) return FALSE;

    if (g_pfnEnable) {
        g_pfnEnable(EN_BEGIN_CLOSEDC, (PVOID)hdc, pDc->GetInstance());
        g_pfnEnable(EN_DISABLE_CTX,   pDc->GetInstance(), NULL);
    }
    g_greDcs.Destroy(hdc);
    return TRUE;
}

/*!
 * @brief Select dispatch table by bit 9 of fnCode (mask 0x0200).
 *
 * bit 9 set   -> internal (0x02xx, 0x42xx)
 * bit 9 clear -> driver   (0x40xx, 0x50xx, 0x60xx, 0x70xx)
 */
static PFN *SelectTable(ULONG fnCode)
{
    return (fnCode & 0x0200UL) ? g_tblInternal : g_tblDriver;
}

/* ======================================================================== */
/*  Gre32EntryN                                                           */
/* ======================================================================== */

extern "C" {

/*!
 * @brief Gre32Entry2 dispatch (arity 2: a, fnCode).
 *
 * No pInstance slot: arity-2 hooks receive a and fnCode only.
 */
ULONG APIENTRY Gre32Entry2(ULONG a, ULONG fnCode)
{
    int   ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                               "[GRE] Gre32Entry2(fn=0x%lx(%s), a=0x%lx)",
                               (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h   = SelectTable(fnCode)[fnCode & 0xFF];
    ULONG rc  = ((PFN_GRE2)h)(a, fnCode);

    F_DbgCtxExitf(ctx, "0x%lx", (unsigned long)rc);
    return rc;
}

/*!
 * @brief Gre32Entry3 dispatch (arity 3: a, b, fnCode).
 *
 * b is the dummy slot in pmddim.h macros; replaced with pInstance
 * when bit 0x40 (per-DC operation) is set.
 */
ULONG APIENTRY Gre32Entry3(ULONG a, ULONG b, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry3(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)b;
    rc = ((PFN_GRE3)h)(a, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry4 dispatch (arity 4: a, b, c, fnCode).
 */
ULONG APIENTRY Gre32Entry4(ULONG a, ULONG b, ULONG c, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry4(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)c;
    rc = ((PFN_GRE4)h)(a, b, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry5 dispatch (arity 5: a, b, c, d, fnCode).
 */
ULONG APIENTRY Gre32Entry5(ULONG a, ULONG b, ULONG c, ULONG d, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry5(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)d;
    rc = ((PFN_GRE5)h)(a, b, c, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry6 dispatch (arity 6: a, b, c, d, e, fnCode).
 */
ULONG APIENTRY Gre32Entry6(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry6(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)e;
    rc = ((PFN_GRE6)h)(a, b, c, d, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry7 dispatch (arity 7: a, b, c, d, e, f, fnCode).
 */
ULONG APIENTRY Gre32Entry7(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry7(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)f;
    rc = ((PFN_GRE7)h)(a, b, c, d, e, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry8 dispatch (arity 8: a, b, c, d, e, f, g, fnCode).
 */
ULONG APIENTRY Gre32Entry8(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG g, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry8(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)g;
    rc = ((PFN_GRE8)h)(a, b, c, d, e, f, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry9 dispatch (arity 9: a, b, c, d, e, f, g, h_, fnCode).
 */
ULONG APIENTRY Gre32Entry9(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG g, ULONG h_, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry9(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)h_;
    rc = ((PFN_GRE9)h)(a, b, c, d, e, f, g, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

/*!
 * @brief Gre32Entry10 dispatch (arity 10: a, b, c, d, e, f, g, h_, i, fnCode).
 */
ULONG APIENTRY Gre32Entry10(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                            ULONG f, ULONG g, ULONG h_, ULONG i, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry10(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = SelectTable(fnCode)[fnCode & 0xFF];
    PVOID pInst = NULL;
    ULONG rc;

    if (fnCode & 0x4000) {
        Fs_Dc *pDc = g_greDcs.Find(a);
        if (!pDc) {
            F_DbgCtxExitf(ctx, "GPI_ERROR (no DC for hdc=%lu)",
                          (unsigned long)a);
            return (ULONG)GPI_ERROR;
        }
        pInst = pDc->GetInstance();
    }
    (void)i;
    rc = ((PFN_GRE10)h)(a, b, c, d, e, f, g, h_, (ULONG)pInst, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (pInst=%p)", (unsigned long)rc, pInst);
    return rc;
}

} /* extern "C" */
