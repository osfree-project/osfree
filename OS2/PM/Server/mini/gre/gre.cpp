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

#include "gre.hpp"

#include <string.h>

/*! @def GRE_DISPATCH_SIZE
 *  @brief Number of slots in internal dispatch table.
 */
#define GRE_DISPATCH_SIZE 256


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
 *         Full NGre* code (flags + slot): low byte indexes g_tblDriver. 
 *
 * @todo: Display mandatory, Hardcopy mandatory tables
 *  GrePolyShortLine: This function must be supported by all presentation drivers except those that hook GrePolyLine and all of the GreArcxxx functions that are simulated by handling routines in the graphics engine.
 *  GrePolyScanline: This function must be supported by all presentation drivers except those that hook the GreDrawRLE, GreEndArea, and GreFillPath functions.
 */
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
    NGreDrawBits,                   /* 0x6022 */
    NGreDrawBorder,                 /* 0x602E */
    NGreErasePS,                    /* 0x40A7 */
    NGreGetBitmapBits,              /* 0x6030 */
    NGreGetBoundsData,              /* 0x40AF */
    NGreGetCodePage,                /* 0x40B3 */
    NGreGetDCOrigin,                /* 0x40BA */
    NGreGetLineOrigin,              /* 0x40BC */
    NGreGetPairKerningTable,        /* 0x40A0 */
    NGreGetPel,                     /* 0x6027 */
    NGreImageData,                  /* 0x4029 */
    NGreLockDevice,                 /* 0x40B5 */
    NGreNotifyClipChange,           /* 0x40A4 */
    NGreNotifyTransformChange,      /* 0x40A5 */
    NGrePolyMarker,                 /* 0x403A */
    NGreDisjointLines,              /* 0x4016 */
    NGreGetCurrentPosition,         /* 0x4017 */
    NGrePolyLine,                   /* 0x4019 */
    NGreDrawLinesInPath,            /* 0x401A */
    NGrePolyShortLine,              /* 0x401B */ // @todo see note in desc
    NGrePolyScanline,               /* 0x401C */ // @todo see note in desc
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

static const ULONG            g_mandatoryDisplaySlots[] = {
    NGreSaveScreenBits,           /* 0x402C */
    NGreRestoreScreenBits,        /* 0x402D */
    NGreDeviceSetCursor,          /* 0x402F */
    NGreSetColorCursor,           /* 0x4032 */
};

/*! @brief Number of entries in g_mandatorySlots[]. */
#define MANDATORY_COUNT ((int)(sizeof(g_mandatorySlots) / sizeof(g_mandatorySlots[0])))

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
/*  Path dispatchers (real)                                               */
/* ======================================================================== */

static ULONG __cdecl GreDrvBeginPath(ULONG hdc, ULONG idPath,
                                      ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPath().BeginPath((LONG)idPath);
}

static ULONG __cdecl GreDrvEndPath(ULONG hdc, ULONG flOptions,
                                    ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPath().EndPath(flOptions);
}

static ULONG __cdecl GreDrvCloseFigure(ULONG hdc, ULONG pInstance,
                                        ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPath().CloseFigure();
}

static ULONG __cdecl GreDrvFillPath(ULONG hdc, ULONG idPath, ULONG flOptions,
                                     ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || (LONG)idPath != 1) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetPath().FillPath(flOptions);
}

static ULONG __cdecl GreDrvOutlinePath(ULONG hdc, ULONG idPath, ULONG flOptions,
                                        ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || (LONG)idPath != 1) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetPath().OutlinePath(flOptions);
}

static ULONG __cdecl GreDrvModifyPath(ULONG hdc, ULONG idPath, ULONG cmdMode,
                                       ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || (LONG)idPath != 1) return (ULONG)FALSE;
    return (ULONG)pDc->GetPath().ModifyPath((LONG)cmdMode);
}

static ULONG __cdecl GreDrvStrokePath(ULONG hdc, ULONG idPath, ULONG flOptions,
                                       ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc || (LONG)idPath != 1) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetPath().StrokePath(flOptions);
}

static ULONG __cdecl GreDrvSavePath(ULONG hdc, ULONG cSave,
                                     ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPath().SavePath((LONG)cSave);
}

static ULONG __cdecl GreDrvRestorePath(ULONG hdc, ULONG cSave,
                                        ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPath().RestorePath((LONG)cSave);
}

/* ======================================================================== */
/*  Area dispatchers (real)                                               */
/* ======================================================================== */

static ULONG __cdecl GreDrvBeginArea(ULONG hdc, ULONG flOptions,
                                      ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetArea().Begin(flOptions);
}

static ULONG __cdecl GreDrvEndArea(ULONG hdc, ULONG flCancel,
                                    ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetArea().End(flCancel);
}

static ULONG __cdecl GreDrvAreaSetAttributes(ULONG hdc, ULONG lPrimType,
                                              ULONG flDefsMask, ULONG flAttrsMask,
                                              ULONG pAttrs, ULONG pInstance,
                                              ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetArea().SetAttributes((LONG)lPrimType, flDefsMask,
                                                flAttrsMask, (PVOID)pAttrs);
}


/* ======================================================================== */
/*  Transform dispatchers (real)                                          */
/* ======================================================================== */

static ULONG __cdecl GreDrvGetPageUnits(ULONG hdc, ULONG pExtent,
                                         ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetXform().GetPageUnits((SIZEL *)pExtent);
}

static ULONG __cdecl GreDrvSetPageUnits(ULONG hdc, ULONG ulUnits,
                                         ULONG lWidth, ULONG lHeight,
                                         ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetPageUnits(ulUnits,
                                                (LONG)lWidth, (LONG)lHeight);
}

static ULONG __cdecl GreDrvGetModelXform(ULONG hdc, ULONG paXform,
                                          ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().GetModelXform((XFORM *)paXform);
}

static ULONG __cdecl GreDrvSetModelXform(ULONG hdc, ULONG paXform,
                                          ULONG flOptions, ULONG pInstance,
                                          ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetModelXform((const XFORM *)paXform,
                                                 (LONG)flOptions);
}

static ULONG __cdecl GreDrvGetWindowViewportXform(ULONG hdc, ULONG paXform,
                                                   ULONG pInstance,
                                                   ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().GetWindowViewportXform((XFORM *)paXform);
}

static ULONG __cdecl GreDrvSetWindowViewportXform(ULONG hdc, ULONG paXform,
                                                   ULONG flOptions,
                                                   ULONG pInstance,
                                                   ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetWindowViewportXform((const XFORM *)paXform,
                                                         (LONG)flOptions);
}

static ULONG __cdecl GreDrvGetGlobalViewingXform(ULONG hdc, ULONG paXform,
                                                  ULONG pInstance,
                                                  ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().GetGlobalViewingXform((XFORM *)paXform);
}

static ULONG __cdecl GreDrvSetGlobalViewingXform(ULONG hdc, ULONG paXform,
                                                  ULONG flOptions,
                                                  ULONG pInstance,
                                                  ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetGlobalViewingXform((const XFORM *)paXform,
                                                         (LONG)flOptions);
}

static ULONG __cdecl GreDrvSaveXformData(ULONG hdc, ULONG ulSize,
                                          ULONG pBuffer, ULONG pInstance,
                                          ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)GPI_ERROR;
    return (ULONG)pDc->GetXform().SaveXformData(ulSize, (PBYTE)pBuffer);
}

static ULONG __cdecl GreDrvRestoreXformData(ULONG hdc, ULONG ulSize,
                                             ULONG pBuffer, ULONG pInstance,
                                             ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().RestoreXformData(ulSize,
                                                    (const BYTE *)pBuffer);
}

static ULONG __cdecl GreDrvGetPageViewport(ULONG hdc, ULONG prclViewport,
                                            ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().GetPageViewport((RECTL *)prclViewport);
}

static ULONG __cdecl GreDrvSetPageViewport(ULONG hdc, ULONG prclViewport,
                                            ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetPageViewport((const RECTL *)prclViewport);
}

static ULONG __cdecl GreDrvGetGraphicsField(ULONG hdc, ULONG prclField,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().GetGraphicsField((RECTL *)prclField);
}

static ULONG __cdecl GreDrvSetGraphicsField(ULONG hdc, ULONG prclField,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetGraphicsField((const RECTL *)prclField);
}

static ULONG __cdecl GreDrvGetViewingLimits(ULONG hdc, ULONG prclLimits,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().GetViewingLimits((RECTL *)prclLimits);
}

static ULONG __cdecl GreDrvSetViewingLimits(ULONG hdc, ULONG prclLimits,
                                             ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SetViewingLimits((const RECTL *)prclLimits);
}

static ULONG __cdecl GreDrvQueryViewportSize(ULONG hdc, ULONG ulType,
                                              ULONG psizlSize, ULONG pInstance,
                                              ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().QueryViewportSize(ulType, (SIZEL *)psizlSize);
}

static ULONG __cdecl GreDrvConvert(ULONG hdc, ULONG lSrc, ULONG lDst,
                                    ULONG paptlPoint, ULONG cPoints,
                                    ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().Convert((LONG)lSrc, (LONG)lDst,
                                           (PPOINTL)paptlPoint, (LONG)cPoints);
}

static ULONG __cdecl GreDrvSaveXform(ULONG hdc, ULONG cSave,
                                      ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().SaveXform((LONG)cSave);
}

static ULONG __cdecl GreDrvRestoreXform(ULONG hdc, ULONG cSave,
                                         ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().RestoreXform((LONG)cSave);
}

static ULONG __cdecl GreDrvMultiplyXforms(ULONG hdc, ULONG paXform,
                                           ULONG paNew, ULONG lMode,
                                           ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().MultiplyXforms((XFORM *)paXform,
                                                  (const XFORM *)paNew,
                                                  (LONG)lMode);
}

static ULONG __cdecl GreDrvConvertWithMatrix(ULONG hdc, ULONG paptlPoint,
                                              ULONG cPoints, ULONG pxform,
                                              ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetXform().ConvertWithMatrix((PPOINTL)paptlPoint,
                                                     (LONG)cPoints,
                                                     (const XFORM *)pxform);
}

/* ======================================================================== */
/*  Palette Manager dispatchers (real)                                    */
/* ======================================================================== */

static ULONG __cdecl GreDrvDeviceCreatePalette(ULONG hdc, ULONG pInfo,
                                                ULONG hpal, ULONG pInstance,
                                                ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    /* pInfo carries PALETTEINFO; hpal is the driver-provided HDEVPAL. */
    return (ULONG)pDc->GetPalette().Create(0, 0, 0, (const ULONG *)pInfo);
}

static ULONG __cdecl GreDrvDeviceDeletePalette(ULONG hdc, ULONG hpal,
                                                ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)hpal; (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().Delete();
}

static ULONG __cdecl GreDrvDeviceSetPaletteEntries(ULONG hdc, ULONG hpal,
                                                    ULONG ulFormat, ULONG ulStart,
                                                    ULONG cCount, ULONG pEntries,
                                                    ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)hpal; (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().SetEntries(ulFormat, ulStart, cCount,
                                                (const ULONG *)pEntries);
}

static ULONG __cdecl GreDrvDeviceAnimatePalette(ULONG hdc, ULONG hpal,
                                                 ULONG ulFormat, ULONG ulStart,
                                                 ULONG cCount, ULONG pEntries,
                                                 ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)hpal; (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().Animate(ulFormat, ulStart, cCount,
                                             (const ULONG *)pEntries);
}

static ULONG __cdecl GreDrvDeviceResizePalette(ULONG hdc, ULONG hpal,
                                                ULONG cNewSize, ULONG pInstance,
                                                ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)hpal; (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().Resize(cNewSize);
}

static ULONG __cdecl GreDrvRealizePalette(ULONG hdc, ULONG flType,
                                           ULONG pulCount, ULONG pInstance,
                                           ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().Realize(flType, (PULONG)pulCount);
}

static ULONG __cdecl GreDrvQueryHWPaletteInfo(ULONG hdc, ULONG ulStart,
                                               ULONG cCount, ULONG pulArray,
                                               ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().QueryHWInfo(ulStart, cCount,
                                                 (PULONG)pulArray);
}

static ULONG __cdecl GreDrvUpdateColors(ULONG hdc, ULONG pInstance,
                                         ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return 0UL;
    return pDc->GetPalette().UpdateColors(0);
}

static ULONG __cdecl GreDrvQueryPaletteRealization(ULONG hdc, ULONG ulStart,
                                                    ULONG cCount, ULONG pulArray,
                                                    ULONG pInstance, ULONG lFunction)
{
    Fs_Dc *pDc = g_greDcs.Find(hdc);
    (void)pInstance; (void)lFunction;
    if (!pDc) return (ULONG)FALSE;
    return (ULONG)pDc->GetPalette().QueryRealization(ulStart, cCount,
                                                      (PULONG)pulArray);
}

/* ======================================================================== */
/*  Simulated defaults from gre_simulate.hpp                              */
/*  (GreDrvSetupDC at 0x81, GreDrvNotifyClipChange at 0xA4)               */
/* ======================================================================== */

//#include "gre_simulate.hpp"

/* ======================================================================== */
/*  Remaining driver-table stubs (not implemented)                        */
/* ======================================================================== */

GRE_DRV_3 (GreDrvOpenScreenChangeArea)
GRE_DRV_5 (GreDrvGetScreenChangeArea)
GRE_DRV_4 (GreDrvCloseScreenChangeArea)
GRE_DRV_4 (GreDrvSetCurrentPosition)
GRE_DRV_7 (GreDrvGetScreenBits)
GRE_DRV_6 (GreDrvSetScreenBits)
GRE_DRV_5 (GreDrvCharRect)
GRE_DRV_5 (GreDrvCharStr)
GRE_DRV_5 (GreDrvScrollRect)
GRE_DRV_4 (GreDrvUpdateCursor)
GRE_DRV_5 (GreDrvClip1DPath)
GRE_DRV_4 (GreDrvDrawRawPath)
GRE_DRV_6 (GreDrvDrawCookedPath)
GRE_DRV_5 (GreDrvPathToRegion)
GRE_DRV_8 (GreDrvConvertPath)
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
GRE_DRV_6 (GreDrvGetVisRects)

/* ======================================================================== */
/*  Per-function stubs (internal table)                                   */
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
GRE_STUB_5 (GreIntCreateFontHandleStub)
GRE_STUB_7 (GreIntQueryFontHandleStub)
GRE_STUB_4 (GreIntSelectFontHandleStub)
GRE_STUB_3 (GreIntDeleteFontHandleStub)
GRE_STUB_4 (GreIntSetFontHandleOwnerStub)
GRE_STUB_5 (GreIntQueryCodePageObjectStub)
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
GRE_STUB_4 (GreIntUnloadPDStub)

/* ======================================================================== */
/*  g_tblDriver                                                             */
/*  @brief Driver dispatcher table. All functions CAN be hooked, some MUST  */
/*  be hooked. Some MUST be chained, some CAN be chained                    */
/* ======================================================================== */

static DISPATCHER g_tblDriver = {
    /* 0x00 0x4000 */ (PFN)GreGetArcParameters,
    /* 0x01 0x4001 */ (PFN)GreSetArcParameters,
    /* 0x02 0x4002 */ (PFN)GreArc,
    /* 0x03 0x4003 */ (PFN)GrePartialArc,
    /* 0x04 0x4004 */ (PFN)GreFullArcInterior,
    /* 0x05 0x4005 */ (PFN)GreFullArcBoundary,
    /* 0x06 0x4006 */ (PFN)GreFullArcBoth,
    /* 0x07 0x4007 */ (PFN)GreBoxInterior,
    /* 0x08 0x4008 */ (PFN)GreBoxBoundary,
    /* 0x09 0x4009 */ (PFN)GreBoxBoth,
    /* 0x0A 0x400A */ (PFN)GrePolyFillet,
    /* 0x0B 0x400B */ (PFN)GrePolyFilletSharp,
    /* 0x0C 0x400C */ (PFN)GrePolySpline,
    /* 0x0D 0x400D */ (PFN)GreOval,
    /* 0x0E 0x400E */ (PFN)GreCookWholePath,
    /* 0x0F 0x400F */ (PFN)GreCookPathCurves,
    /* 0x10 0x4010 */ (PFN)GreBoxExclusive,
    /* 0x11 0x4011 */ (PFN)GreRenderPath,
    /* 0x12 0x4012 */ (PFN)GreDrvOpenScreenChangeArea,
    /* 0x13 0x4013 */ (PFN)GreDrvGetScreenChangeArea,
    /* 0x14 0x4014 */ (PFN)GreDrvCloseScreenChangeArea,
    /* 0x15 ------ */ NULL,
    /* 0x16 0x4016 */ NULL, // Mandatory
    /* 0x17 0x4017 */ NULL, // Mandatory
    /* 0x18 0x4018 */ NULL, // Mandatory
    /* 0x19 0x4019 */ (PFN)GrePolyLine, // Mandatory, can chain
    /* 0x1A 0x401A */ NULL, // Mandatory
    /* 0x1B 0x401B */ NULL, // Mandatory @todo see desc of Mandatory table
    /* 0x1C 0x401C */ NULL, // Mandatory @todo see desc of Mandatory table
    /* 0x1D 0x401D */ (PFN)GreDrvGetScreenBits,
    /* 0x1E 0x401E */ (PFN)GreDrvSetScreenBits,
    /* 0x1F -------- */ NULL,
    /* 0x20 -------- */ NULL,
    /* 0x21 -------- */ NULL,
    /* 0x22 0x6022 */ (PFN)GreDrawBits,
    /* 0x23 0x6023 */ NULL, // Mandatory
    /* 0x24 0x4024 */ NULL, // Mandatory
    /* 0x25 0x4025 */ NULL, // Mandatory
    /* 0x26 0x6026 */ (PFN)GreBitblt, // Mandatory, can chain
    /* 0x27 0x6027 */ NULL, // Mandatory
    /* 0x28 0x4028 */ NULL, // Mandatory
    /* 0x29 0x4029 */ NULL, // Mandatory
    /* 0x2A 0x602A */ (PFN)GreScanLR,
    /* 0x2B 0x602B */ (PFN)GreFloodFill,
    /* 0x2C 0x402C */ (PFN)GreSaveScreenBits, // Mandatory for display, simulate for others
    /* 0x2D 0x402D */ (PFN)GreRestoreScreenBits, // Mandatory for display, simulate for others
    /* 0x2E 0x602E */ NULL, // Mandatory
    /* 0x2F 0x402F */ (PFN)GreDeviceSetCursor, // Mandatory for display, simulate for others
    /* 0x30 0x6030 */ NULL, // Mandatory
    /* 0x31 0x6031 */ NULL, // Mandatory
    /* 0x32 0x4032 */ (PFN)GreSetColorCursor, // Mandatory for display, simulate for others
    /* 0x33 -------- */ NULL,
    /* 0x34 -------- */ NULL,
    /* 0x35 0x5035 */ (PFN)GreCharString, // Mandatory, can chain
    /* 0x36 0x7036 */ (PFN)GreCharStringPos, // Mandatory, can chain
    /* 0x37 0x5037 */ NULL, // Mandatory
    /* 0x38 0x5038 */ NULL, // Mandatory
    /* 0x39 0x5039 */ NULL, // Mandatory
    /* 0x3A 0x403A */ NULL, // Mandatory
    /* 0x3B 0x403B */ (PFN)GreDrvCharRect,
    /* 0x3C 0x403C */ (PFN)GreDrvCharStr,
    /* 0x3D 0x403D */ (PFN)GreDrvScrollRect,
    /* 0x3E 0x403E */ (PFN)GreDrvUpdateCursor,
    /* 0x3F 0x403F */ (PFN)GreDeviceSetPointer,
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
    /* 0x56 0x4056 */ (PFN)GrePolygonSet,
    /* 0x57 0x4057 */ (PFN)GreDrvPathToRegion,
    /* 0x58 0x4058 */ (PFN)GreDrvDrawRLE,
    /* 0x59 0x4059 */ (PFN)GreDevicePolygonSet,
    /* 0x5A -------- */ NULL,
    /* 0x5B -------- */ NULL,
    /* 0x5C 0x405C */ (PFN)GrePolygon3d,
    /* 0x5D 0x405D */ (PFN)GreGetRegionBox,
    /* 0x5E 0x405E */ (PFN)GreGetRegionRects,
    /* 0x5F 0x405F */ (PFN)GreOffsetRegion,
    /* 0x60 0x4060 */ (PFN)GrePtInRegion,
    /* 0x61 0x4061 */ (PFN)GreRectInRegion,
    /* 0x62 0x4062 */ (PFN)GreCreateRectRegion,
    /* 0x63 0x4063 */ (PFN)GreDestroyRegion,
    /* 0x64 0x4064 */ (PFN)GreSetRectRegion,
    /* 0x65 0x4065 */ (PFN)GreCombineRegion,
    /* 0x66 0x4066 */ (PFN)GreCombineRectRegion,
    /* 0x67 0x4067 */ (PFN)GreCombineShortLineRegion,
    /* 0x68 0x4068 */ (PFN)GreEqualRegion,
    /* 0x69 0x4069 */ (PFN)GrePaintRegion,
    /* 0x6A 0x406A */ (PFN)GreSetRegionOwner,
    /* 0x6B 0x406B */ (PFN)GreFrameRegion,
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
    /* 0xA4 0x40A4 */ (PFN)GreDrvNotifyClipChange,
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
    /* 0xBB 0x40BB */ NULL,
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
    /* 0xD1 0x40D1 */ NULL,
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

/* ======================================================================== */
/*  g_tblInternal                                                         */
/* ======================================================================== */

static PFN g_tblInternal[GRE_DISPATCH_SIZE] = {
    /* 0x00 */ (PFN)GreOpenDC,
    /* 0x01 */ (PFN)GreCloseDC,
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
    /* 0x29 */ (PFN)GreIntGetHandleStub,
    /* 0x2A */ (PFN)GreIntGetHandleStub,
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
    /* 0x40 */ (PFN)GreCreatePalette,
    /* 0x41 */ (PFN)GreDeletePalette,
    /* 0x42 */ (PFN)GreSelectPalette,
    /* 0x43 */ (PFN)GreSetPaletteEntries,
    /* 0x44 */ (PFN)GreAnimatePalette,
    /* 0x45 */ (PFN)GreQueryPalette,
    /* 0x46 */ (PFN)GreQueryPaletteInfo,
    /* 0x47 */ (PFN)GreSetPaletteOwner,
    /* 0x48 */ (PFN)GreResizePalette,
    /* 0x49 */ (PFN)GreQueryDefaultPaletteInfo,
    /* 0x4A */ (PFN)GreIntGetHandleStub,
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
    /* 0x56 */ (PFN)GreQueryCharOutline,
    /* 0x57 */ (PFN)GreRealizeString,
    /* 0x58 */ (PFN)GreQueryCharMetricsTable,
    /* 0x59 */ (PFN)GreBeginInkPath,
    /* 0x5A */ (PFN)GreEndInkPath,
    /* 0x5B */ (PFN)GreStrokeInkPath,
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
    /* 0x69 */ (PFN)GreQueryTabbedTextExtent,
    /* 0x6A */ (PFN)GreTabbedCharStringAt,
    /* 0x6B */ (PFN)GreIntResetDC2Stub,
    /* 0x6C */ (PFN)GreIntGetHandleStub,
    /* 0x6D */ (PFN)GreIntGetHandleStub,
    /* 0x6E */ (PFN)GreIntUnloadPDStub,
    /* 0x6F..0xFF */ (PFN)GreIntGetHandleStub
};

/* ======================================================================== */
/*  Static helpers                                                        */
/* ======================================================================== */

static int HookedCount(void)
{
    int i, n = 0;
    for (i = 0; i < MAXTABLEENTRIES; i++) {
        if (g_tblDriver[i] != g_tblDriverBefore[i]) n++;
    }
    return n;
}

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

    memcpy(g_tblDriverBefore, g_tblDriver, sizeof(g_tblDriver));

    flParams.Version   = 0x00000210;
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

static PFN *SelectTable(ULONG fnCode)
{
    return (fnCode & 0x0200UL) ? g_tblInternal : g_tblDriver;
}

static PFN GetHandler(ULONG fnCode)
{
    ULONG slot = fnCode & 0xFF;
    if (!(fnCode & 0x0200UL) && slot >= MAXTABLEENTRIES) return NULL;
    return SelectTable(fnCode)[slot];
}

/* ======================================================================== */
/*  Gre32EntryN                                                           */
/* ======================================================================== */

extern "C" {

ULONG APIENTRY Gre32Entry2(ULONG a, ULONG fnCode)
{
    int   ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                               "[GRE] Gre32Entry2(fn=0x%lx(%s), a=0x%lx)",
                               (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h   = GetHandler(fnCode);
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
    rc = ((PFN_GRE2)h)(a, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx", (unsigned long)rc);
    return rc;
}

ULONG APIENTRY Gre32Entry3(ULONG a, ULONG b, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry3(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry4(ULONG a, ULONG b, ULONG c, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry4(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry5(ULONG a, ULONG b, ULONG c, ULONG d, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry5(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry6(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry6(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry7(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry7(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry8(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG g, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry8(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry9(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG g, ULONG h_, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry9(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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

ULONG APIENTRY Gre32Entry10(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                            ULONG f, ULONG g, ULONG h_, ULONG i, ULONG fnCode)
{
    int   ctx   = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE] Gre32Entry10(fn=0x%lx(%s), a=0x%lx)",
                                 (unsigned long)fnCode, GreFuncName(fnCode), (unsigned long)a);
    PFN   h     = GetHandler(fnCode);
    PVOID pInst = NULL;
    ULONG rc;

    if (!h) {
        F_DbgCtxExitf(ctx, "GPI_ERROR (bad fnCode 0x%lx)", (unsigned long)fnCode);
        return (ULONG)GPI_ERROR;
    }
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
