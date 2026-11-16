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

#define INCL_BASE
#define INCL_PM
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

/*! @brief Snapshot of g_tblDriver taken just before FillLogicalDeviceBlock,
 *         used by HookedCount() to count the slots the driver actually
 *         replaced. */
static PFN                    g_tblDriverBefore[MAXTABLEENTRIES];

/*! @brief Slots that MUST be filled by the presentation driver in
 *         FillLogicalDeviceBlock.  If the driver leaves one of them
 *         NULL, GRE rejects the driver at load time.
 *
 *         Mandatory status per EDM2 "PDRREF: Mandatory and Simulated
 *         Graphics Engine Function Reference", column "Mandatory for
 *         All Drivers" (Hard Copy = x, Display = x).
 *
 *         Full NGre* code (flags + slot): low byte indexes g_tblDriver. */
static const ULONG            g_mandatorySlots[] = {
    NGreAccumulateBounds,           /* 0x40B0 */
    NGreBitblt,                     /* 0x6026 */
    NGreCharStr,                    /* 0x403C */
    NGreCharStringPos,              /* 0x7036 */
    NGreCreateLogColorTable,        /* 0x60C5 */
    NGreDeviceCreateBitmap,         /* 0x6023 */
    NGreDeviceDeleteBitmap,         /* 0x4024 */
    NGreDeviceGetAttributes,        /* 0x609D */
    NGreDeviceQueryFontAttributes,  /* 0x40A9 */
    NGreDeviceQueryFonts,           /* 0x40AA */
    NGreDeviceSelectBitmap,         /* 0x4025 */
    NGreDeviceSetAttributes,        /* 0x60A2 */
    NGreDeviceSetDCOrigin,          /* 0x40BB */
    NGreDeviceSetGlobalAttribute,   /* 0x60A3 */
    NGreDisjointLines,              /* 0x4016 */
    NGreDrawBits,                   /* 0x6022 */
    NGreDrawBorder,                 /* 0x602E */
    NGreDrawLinesInPath,            /* 0x401A */
    NGreErasePS,                    /* 0x40A7 */
    NGreGetBitmapBits,              /* 0x6030 */
    NGreGetBoundsData,              /* 0x40AF */
    NGreGetCodePage,                /* 0x40B3 */
    NGreGetCurrentPosition,         /* 0x4017 */
    NGreGetDCOrigin,                /* 0x40BA */
    NGreGetLineOrigin,              /* 0x40BC */
    NGreGetPairKerningTable,        /* 0x40A0 */
    NGreGetPel,                     /* 0x6027 */
    NGreImageData,                  /* 0x4029 */
    NGreLockDevice,                 /* 0x40B5 */
    NGreNotifyClipChange,           /* 0x40A4 */
    NGreNotifyTransformChange,      /* 0x40A5 */
    NGrePolyLine,                   /* 0x4019 */
    NGrePolyMarker,                 /* 0x403A */
    NGrePolyScanline,               /* 0x401C */
    NGrePolyShortLine,              /* 0x401B */
    NGreQueryCharPositions,         /* 0x5038 */
    NGreQueryColorData,             /* 0x60C3 */
    NGreQueryColorIndex,            /* 0x60CA */
    NGreQueryDeviceBitmaps,         /* 0x40D0 */
    NGreQueryDeviceCaps,            /* 0x40D1 */
    NGreQueryDevResource2,          /* 0x40D4 */
    NGreQueryLogColorTable,         /* 0x60C4 */
    NGreQueryNearestColor,          /* 0x40C9 */
    NGreQueryRealColors,            /* 0x40C8 */
    NGreQueryRGBColor,              /* 0x60CB */
    NGreQueryTextBox,               /* 0x5037 */
    NGreQueryWidthTable,            /* 0x5039 */
    NGreRealizeColorTable,          /* 0x60C6 */
    NGreRealizeFont,                /* 0x40A6 */
    NGreResetBounds,                /* 0x40AE */
    NGreSetBitmapBits,              /* 0x6031 */
    NGreSetCodePage,                /* 0x40B4 */
    NGreSetCurrentPosition,         /* 0x4018 */
    NGreSetLineOrigin,              /* 0x40BD */
    NGreSetPel,                     /* 0x4028 */
    NGreUnlockDevice,               /* 0x40B6 */
    NGreUnrealizeColorTable,        /* 0x60C7 */
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
/*  Stub macros — used only for unimplemented slots                       */
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

#define GRE_DRV_2(name) GRE_STUB_2(name)
#define GRE_DRV_3(name) GRE_STUB_3(name)
#define GRE_DRV_4(name) GRE_STUB_4(name)
#define GRE_DRV_5(name) GRE_STUB_5(name)
#define GRE_DRV_6(name) GRE_STUB_6(name)
#define GRE_DRV_7(name) GRE_STUB_7(name)
#define GRE_DRV_8(name) GRE_STUB_8(name)
#define GRE_DRV_9(name) GRE_STUB_9(name)
#define GRE_DRV_10(name) GRE_STUB_10(name)

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
/*  Clip dispatchers (real)                                               */
/* ======================================================================== */

static ULONG __cdecl GreDrvGetClipBox(ULONG hdc, ULONG prclRect,
                                       ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || !prclRect) return (ULONG)RGN_ERROR;
    return (ULONG)pDc->GetClipBox(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvGetClipRects(ULONG hdc, ULONG prclBound,
                                         ULONG pControl, ULONG parclRect,
                                         ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->EnumClipRects((const RECTL *)prclBound,
                                     (PRGNRECT)pControl, (PRECTL)parclRect);
}

static ULONG __cdecl GreDrvOffsetClipRegion(ULONG hdc, ULONG pdpt,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    PPOINTL pptl = (PPOINTL)pdpt;
    (void)pInstance; (void)lFunction;
    if (!pDc || !pptl) return (ULONG)RGN_ERROR;
    return (ULONG)pDc->OffsetClipRegion(pptl->x, pptl->y);
}

static ULONG __cdecl GreDrvPtVisible(ULONG hdc, ULONG pptlPoint,
                                      ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || !pptlPoint) return (ULONG)PVIS_ERROR;
    return (ULONG)pDc->PtVisible(*(PPOINTL)pptlPoint);
}

static ULONG __cdecl GreDrvRectVisible(ULONG hdc, ULONG prclRect,
                                        ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || !prclRect) return (ULONG)RVIS_ERROR;
    return (ULONG)pDc->RectVisible(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvQueryClipRegion(ULONG hdc, ULONG pInstance,
                                            ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)HRGN_ERROR;
    Fs_Region *p = pDc->QueryClipRegion();
    return p ? (ULONG)p->ToHandle() : 0UL;
}

static ULONG __cdecl GreDrvSelectClipRegion(ULONG hdc, ULONG hrgn,
                                             ULONG phrgnOld,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    Fs_Region *pOld = NULL;
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)RGN_ERROR;
    LONG lC = pDc->SelectClipRegion(Fs_Region::FromHandle((HRGN)hrgn),
                                     phrgnOld ? &pOld : NULL);
    if (phrgnOld) *(PHRGN)phrgnOld = pOld ? pOld->ToHandle() : (HRGN)NULL;
    return (ULONG)lC;
}

static ULONG __cdecl GreDrvIntersectClipRectangle(ULONG hdc, ULONG prclRect,
                                                   ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || !prclRect) return (ULONG)RGN_ERROR;
    return (ULONG)pDc->IntersectClipRectangle(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvExcludeClipRectangle(ULONG hdc, ULONG prclRect,
                                                 ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || !prclRect) return (ULONG)RGN_ERROR;
    return (ULONG)pDc->ExcludeClipRectangle(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvSetXformRect(ULONG hdc, ULONG prclRect,
                                         ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || !prclRect) return (ULONG)RGN_ERROR;
    return (ULONG)pDc->SetXformRect(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvSaveRegion(ULONG hdc, ULONG cSave,
                                       ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    if ((LONG)cSave < 0) return (ULONG)FALSE;
    if (cSave == 1) return (ULONG)pDc->InitRegionState();
    return (ULONG)pDc->SaveRegionState();
}

static ULONG __cdecl GreDrvRestoreRegion(ULONG hdc, ULONG cSave,
                                          ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->RestoreRegionState((LONG)cSave);
}

static ULONG __cdecl GreDrvSelectPathRegion(ULONG hdc, ULONG hrgn,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->SelectPathRegion(Fs_Region::FromHandle((HRGN)hrgn));
}

static ULONG __cdecl GreDrvRegionSelectBitmap(ULONG hdc, ULONG hbm,
                                               ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)hbm; (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->RegionSelectBitmap();
}

static ULONG __cdecl GreDrvCopyClipRegion(ULONG hdc, ULONG hrgn,
                                           ULONG prclBounds, ULONG flOptions,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    Fs_Region *pDst = Fs_Region::FromHandle((HRGN)hrgn);
    (void)pInstance; (void)lFunction;
    if (!pDc || !pDst) return (ULONG)RGN_ERROR;
    return (ULONG)pDc->CopyClipRegion(*pDst, flOptions);
}

static ULONG __cdecl GreDrvSelectClipPath(ULONG hdc, ULONG idPath,
                                           ULONG flOptions,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->SelectClipPath(idPath, flOptions);
}

static ULONG __cdecl GreDrvClipPathCurves(ULONG hdc, ULONG pPath,
                                           ULONG pPipeline,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pPath; (void)pPipeline; (void)pInstance; (void)lFunction;
    return (ULONG)(pDc ? TRUE : FALSE);
}

/* ======================================================================== */
/*  Region dispatchers (real)                                             */
/* ======================================================================== */

static ULONG __cdecl GreDrvCombineRegion(ULONG hdc, ULONG hrgnDst,
                                          ULONG hrgnSrc1, ULONG hrgnSrc2,
                                          ULONG cmdMode, ULONG pInstance,
                                          ULONG lFunction)
{
    Fs_Region *pDst = Fs_Region::FromHandle((HRGN)hrgnDst);
    Fs_Region *pS1  = Fs_Region::FromHandle((HRGN)hrgnSrc1);
    Fs_Region *pS2  = Fs_Region::FromHandle((HRGN)hrgnSrc2);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pDst || !pS1 || !pS2) return (ULONG)RGN_ERROR;
    if (pDst->IsClipSelected() || pS1->IsClipSelected() || pS2->IsClipSelected())
        return (ULONG)RGN_ERROR;
    return (ULONG)pDst->Combine(*pS1, *pS2, cmdMode);
}

static ULONG __cdecl GreDrvCombineRectRegion(ULONG hdc, ULONG hrgnDst,
                                              ULONG prclRect, ULONG hrgnSrc,
                                              ULONG cmdMode, ULONG pInstance,
                                              ULONG lFunction)
{
    Fs_Region *pDst = Fs_Region::FromHandle((HRGN)hrgnDst);
    Fs_Region *pSrc = Fs_Region::FromHandle((HRGN)hrgnSrc);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pDst || !pSrc || !prclRect) return (ULONG)RGN_ERROR;
    if (pDst->IsClipSelected() || pSrc->IsClipSelected())
        return (ULONG)RGN_ERROR;
    return (ULONG)pDst->CombineRect(*pSrc, *(PRECTL)prclRect, cmdMode);
}

static ULONG __cdecl GreDrvCreateRectRegion(ULONG hdc, ULONG paRegion,
                                              ULONG cRect, ULONG pInstance,
                                              ULONG lFunction)
{
    (void)hdc; (void)pInstance; (void)lFunction;
    return (ULONG)Fs_Region::CreateFromRects(cRect, (const RECTL *)paRegion);
}

static ULONG __cdecl GreDrvDestroyRegion(ULONG hdc, ULONG hrgn,
                                          ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn) return (ULONG)FALSE;
    if (pRgn->IsClipSelected()) return (ULONG)FALSE;
    delete pRgn;
    return (ULONG)TRUE;
}

static ULONG __cdecl GreDrvEqualRegion(ULONG hdc, ULONG hrgn1, ULONG hrgn2,
                                        ULONG pInstance, ULONG lFunction)
{
    Fs_Region *p1 = Fs_Region::FromHandle((HRGN)hrgn1);
    Fs_Region *p2 = Fs_Region::FromHandle((HRGN)hrgn2);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!p1 || !p2) return (ULONG)EQRGN_ERROR;
    if (p1->IsClipSelected() || p2->IsClipSelected())
        return (ULONG)EQRGN_ERROR;
    return (ULONG)p1->Equal(*p2);
}

static ULONG __cdecl GreDrvGetRegionBox(ULONG hdc, ULONG hrgn,
                                         ULONG prclRect, ULONG pInstance,
                                         ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !prclRect) return (ULONG)RGN_ERROR;
    if (pRgn->IsClipSelected()) return (ULONG)RGN_ERROR;
    return (ULONG)pRgn->UpdateBounds(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvGetRegionRects(ULONG hdc, ULONG hrgn,
                                           ULONG prclBound, ULONG pControl,
                                           ULONG parclRect,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn) return (ULONG)FALSE;
    if (pRgn->IsClipSelected()) return (ULONG)FALSE;
    return (ULONG)pRgn->EnumRects((const RECTL *)prclBound,
                                   (PRGNRECT)pControl, (PRECTL)parclRect);
}

static ULONG __cdecl GreDrvOffsetRegion(ULONG hdc, ULONG hrgn, ULONG pdpt,
                                         ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    PPOINTL pptl = (PPOINTL)pdpt;
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !pptl) return (ULONG)FALSE;
    if (pRgn->IsClipSelected()) return (ULONG)FALSE;
    pRgn->Offset(pptl->x, pptl->y);
    return (ULONG)TRUE;
}

static ULONG __cdecl GreDrvPtInRegion(ULONG hdc, ULONG hrgn, ULONG pptl,
                                       ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !pptl) return (ULONG)PRGN_ERROR;
    if (pRgn->IsClipSelected()) return (ULONG)PRGN_ERROR;
    return (ULONG)(pRgn->Contains(*(PPOINTL)pptl) ? PRGN_INSIDE : PRGN_OUTSIDE);
}

static ULONG __cdecl GreDrvRectInRegion(ULONG hdc, ULONG hrgn,
                                         ULONG prclRect, ULONG pInstance,
                                         ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn || !prclRect) return (ULONG)RRGN_ERROR;
    if (pRgn->IsClipSelected()) return (ULONG)RRGN_ERROR;
    return (ULONG)pRgn->RectIn(*(PRECTL)prclRect);
}

static ULONG __cdecl GreDrvSetRectRegion(ULONG hdc, ULONG hrgn,
                                          ULONG paRegion, ULONG cRect,
                                          ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)pInstance; (void)lFunction;
    if (!pRgn) return (ULONG)FALSE;
    if (pRgn->IsClipSelected()) return (ULONG)FALSE;
    return (ULONG)pRgn->SetFromRects(cRect, (const RECTL *)paRegion);
}

static ULONG __cdecl GreDrvPaintRegion(ULONG hdc, ULONG hrgn,
                                        ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)pInstance; (void)lFunction;
    if (!pDc || !pRgn) return (ULONG)GPI_ERROR;
    if (pRgn->IsClipSelected()) return (ULONG)GPI_ERROR;
    return (ULONG)GPI_OK;
}

static ULONG __cdecl GreDrvFrameRegion(ULONG hdc, ULONG hrgn,
                                        ULONG pszl, ULONG pInstance,
                                        ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)pszl; (void)pInstance; (void)lFunction;
    if (!pDc || !pRgn) return (ULONG)GPI_ERROR;
    if (pRgn->IsClipSelected()) return (ULONG)GPI_ERROR;
    return (ULONG)GPI_OK;
}

static ULONG __cdecl GreDrvCombineShortLineRegion(ULONG hdc, ULONG hrgn,
                                                   ULONG psl,
                                                   ULONG pInstance, ULONG lFunction)
{
    (void)hdc; (void)hrgn; (void)psl; (void)pInstance; (void)lFunction;
    return (ULONG)RGN_ERROR;
}

static ULONG __cdecl GreDrvSetRegionOwner(ULONG hdc, ULONG hrgn, ULONG lOwner,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Region *pRgn = Fs_Region::FromHandle((HRGN)hrgn);
    (void)hdc; (void)lOwner; (void)pInstance; (void)lFunction;
    if (!pRgn) return (ULONG)FALSE;
    return (ULONG)TRUE;
}

