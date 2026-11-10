/*!
 * @file gre2sdl.cpp
 * @brief GRE2SDL: presentation driver that renders Gre* calls via SDL2.
 *
 * Registered as a PM presentation driver. On the first DevOpenDC for a
 * process, PM calls OS2_PM_DRV_ENABLE(FillLogicalDeviceBlock), passing
 * the dispatch table via FLReturnsLst.DispTab. We replace the entries
 * for Tier 1 functions with our own handlers; the index is the low-order
 * byte of the NGre* function code.
 *
 * Per-DC state is allocated in EnableDeviceContext and passed to every
 * hooked Gre* call as pInstance. The driver does not own DCs; it owns
 * only the instance data attached to each DC.
 *
 * Rendering is done directly to SDL_Renderer. OD_MEMORY creates a hidden
 * window (offscreen); OD_DIRECT / OD_QUEUED / OD_INFO create a visible
 * window. Each primitive ends with SDL_RenderPresent().
 *
 * FLParamsLst, FLReturnsLst, PFN_OS2_PM_DRV_ENABLE and EN_* subfunction
 * codes are declared in pmddi.h.
 */

#include "F_platform.h"
#include "F_debug.h"

#define INCL_GRE_ARCS        /* BoxInterior, BoxBoundary, BoxBoth, FullArcBoth, PartialArc, PolySpline, PolyFillet */
#define INCL_GRE_LINES       /* SetCurrentPosition, GetCurrentPosition, PolyLine */
#define INCL_GRE_MARKERS     /* PolyMarker */
#define INCL_GRE_STRINGS     /* CharString, CharStringPos */
#define INCL_GRE_DEVSUPPORT  /* SetAttributes, GetAttributes */
#include "pmddi.h"

#include <SDL.h>

/* ======================================================================== */
/*  Per-DC instance data                                                  */
/* ======================================================================== */

typedef struct _GRE2SDL_INSTANCE {
    SDL_Window   *pWin;
    SDL_Renderer *pRen;
    int           width;
    int           height;
    Uint32        color;      /* current foreground colour (RGB) */
    POINTL        curPos;     /* current position */
} GRE2SDL_INSTANCE, *PGRE2SDL_INSTANCE;

/* ======================================================================== */
/*  Forward declarations of hooked handlers                               */
/* ======================================================================== */

static ULONG APIENTRY Gre2SdlSetCurrentPosition(ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlGetCurrentPosition(ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlPolyLine(ULONG, ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlPolyMarker(ULONG, ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlBoxInterior(ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlBoxBoundary(ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlBoxBoth(ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlFullArcBoth(ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlPartialArc(ULONG, ULONG, ULONG, ULONG, ULONG,
                                       ULONG, ULONG);
static ULONG APIENTRY Gre2SdlPolySpline(ULONG, ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlPolyFillet(ULONG, ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlCharString(ULONG, ULONG, ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlCharStringPos(ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG);
static ULONG APIENTRY Gre2SdlSetAttributes(ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG, ULONG);
static ULONG APIENTRY Gre2SdlGetAttributes(ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG);

/* ======================================================================== */
/*  OS2_PM_DRV_ENABLE                                                     */
/* ======================================================================== */

extern "C" ULONG EXPENTRY OS2_PM_DRV_ENABLE(ULONG ulSubFunc,
                                            PULONG pParam1,
                                            PLONG pParam2)
{
    switch (ulSubFunc) {

    case EN_FILL_LDB: { /* FillLogicalDeviceBlock */
        PFLParamsLst  pInfo  = (PFLParamsLst)pParam1;
        PFLReturnsLst pRet   = (PFLReturnsLst)pParam2;
        ULONG         ulVersion;
        USHORT        cTableSize;
        PFN          *pTable;
        ULONG         idx;

        if (!pInfo || !pRet || !pRet->DispTab) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] FillLogicalDeviceBlock: NULL pParam1/pParam2");
            return (ULONG)-1;
        }

        ulVersion  = pInfo->Version;
        cTableSize = pInfo->TableSize;

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] FillLogicalDeviceBlock: version=0x%lx, "
                   "cTableSize=%lu", (unsigned long)ulVersion,
                   (unsigned long)cTableSize);

        pTable = pRet->DispTab;

#define HOOK(ngre, fn) \
            do { \
                idx = (ULONG)((ngre) & 0xFF); \
                if (idx < cTableSize) \
                    pTable[idx] = (PFN)(fn); \
            } while (0)

        HOOK(NGreSetCurrentPosition, Gre2SdlSetCurrentPosition);
        HOOK(NGreGetCurrentPosition, Gre2SdlGetCurrentPosition);
        HOOK(NGrePolyLine,           Gre2SdlPolyLine);
        HOOK(NGrePolyMarker,         Gre2SdlPolyMarker);
        HOOK(NGreBoxInterior,        Gre2SdlBoxInterior);
        HOOK(NGreBoxBoundary,        Gre2SdlBoxBoundary);
        HOOK(NGreBoxBoth,            Gre2SdlBoxBoth);
        HOOK(NGreFullArcBoth,        Gre2SdlFullArcBoth);
        HOOK(NGrePartialArc,         Gre2SdlPartialArc);
        HOOK(NGrePolySpline,         Gre2SdlPolySpline);
        HOOK(NGrePolyFillet,         Gre2SdlPolyFillet);
        HOOK(NGreCharString,         Gre2SdlCharString);
        HOOK(NGreCharStringPos,      Gre2SdlCharStringPos);
        HOOK(NGreSetAttributes,      Gre2SdlSetAttributes);
        HOOK(NGreGetAttributes,      Gre2SdlGetAttributes);
#undef HOOK

        /* Bit 0: each DC needs its own physical device block.
           Bit 2: device name / driver name ignored (single device).
           Bit 7: may update dispatch table in FillPhysicalDeviceBlock. */
        if (pRet->Flags) *pRet->Flags = 0x0000;

        return 0;
    }

    case EN_FILL_PDB: { /* FillPhysicalDeviceBlock */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] FillPhysicalDeviceBlock");
        return 0;
    }

    case EN_DISABLE_PDB: /* DisablePhysicalDeviceBlock */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                   "[GRE2SDL] DisablePhysicalDeviceBlock");
        return 0;

    case EN_ENABLE_CTX: { /* EnableDeviceContext */
        PDENPARAMS pDen = (PDENPARAMS)pParam1;
        PGRE2SDL_INSTANCE inst;
        Uint32 winFlags;

        if (!pDen) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] EnableDeviceContext: NULL pParam1");
            return (ULONG)-1;
        }

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] EnableDeviceContext: ulType=0x%lx, hdc=%lu",
                   (unsigned long)pDen->ulType,
                   (unsigned long)pDen->ulHDC);

        inst = (PGRE2SDL_INSTANCE)DosAllocMem(NULL,
                   sizeof(GRE2SDL_INSTANCE),
                   PAG_COMMIT | PAG_READ | PAG_WRITE);
        if (!inst) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] EnableDeviceContext: out of memory");
            return (ULONG)-1;
        }
        memset(inst, 0, sizeof(*inst));
        inst->width  = 640;
        inst->height = 480;
        inst->color  = 0x00FFFFFF;
        inst->curPos.x = 0;
        inst->curPos.y = 0;

        /* OD_MEMORY is offscreen; everything else (OD_DIRECT for display,
           OD_QUEUED, OD_INFO) gets a visible SDL window. */
        winFlags = (pDen->ulType == OD_MEMORY)
                   ? SDL_WINDOW_HIDDEN
                   : SDL_WINDOW_SHOWN;

        inst->pWin = SDL_CreateWindow("GRE2SDL",
                        SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED,
                        inst->width, inst->height,
                        winFlags);
        if (!inst->pWin) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] SDL_CreateWindow failed: %s",
                       SDL_GetError());
            goto fail;
        }

        inst->pRen = SDL_CreateRenderer(inst->pWin, -1,
                        SDL_RENDERER_SOFTWARE);
        if (!inst->pRen) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] SDL_CreateRenderer failed: %s",
                       SDL_GetError());
            goto fail;
        }

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] EnableDeviceContext -> pInstance=%p "
                   "(window %dx%d, %s)",
                   (void *)inst, inst->width, inst->height,
                   (winFlags == SDL_WINDOW_HIDDEN) ? "hidden" : "visible");
        return (ULONG)inst;

    fail:
        if (inst->pRen) SDL_DestroyRenderer(inst->pRen);
        if (inst->pWin) SDL_DestroyWindow(inst->pWin);
        DosFreeMem(inst);
        return (ULONG)-1;
    }

    case EN_DISABLE_CTX: { /* DisableDeviceContext */
        PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pParam1;
        if (inst) {
            if (inst->pRen) SDL_DestroyRenderer(inst->pRen);
            if (inst->pWin) SDL_DestroyWindow(inst->pWin);
            DosFreeMem(inst);
        }
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] DisableDeviceContext");
        return 0;
    }

    case EN_SAVE_CTX:    /* SaveDCState */
    case EN_RESTORE_CTX: /* RestoreDCState */
    case EN_RESET_CTX:   /* ResetDCState */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                   "[GRE2SDL] subfunction %02lxh", (unsigned long)ulSubFunc);
        return 0;

    case EN_COMPLETE_OPENDC: { /* CompleteOpenDC */
        PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pParam2;
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] CompleteOpenDC: hdc=%lu, pInstance=%p",
                   (unsigned long)pParam1, (void *)inst);
        return 0;
    }

    case EN_BEGIN_CLOSEDC: /* BeginCloseDC */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] BeginCloseDC");
        return 0;

    case EN_INSTALL_HOOKS: /* InstallPMEngineDisplayHooks */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_WARNING,
                   "[GRE2SDL] InstallPMEngineDisplayHooks not implemented");
        return 0;

    default:
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_WARNING,
                   "[GRE2SDL] unsupported subfunction %02lxh",
                   (unsigned long)ulSubFunc);
        return (ULONG)-1;
    }
}

/* ======================================================================== */
/*  Tier 1: rendering                                                     */
/* ======================================================================== */

static ULONG APIENTRY Gre2SdlSetCurrentPosition(ULONG hdc, ULONG pptl,
                                                ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptl;
    (void)hdc; (void)fnCode;
    if (!inst || !p) return (ULONG)-1;
    inst->curPos = *p;
    return 0;
}

static ULONG APIENTRY Gre2SdlGetCurrentPosition(ULONG hdc, ULONG pptl,
                                                ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptl;
    (void)hdc; (void)fnCode;
    if (!inst || !p) return (ULONG)-1;
    *p = inst->curPos;
    return 0;
}

static ULONG APIENTRY Gre2SdlPolyLine(ULONG hdc, ULONG pptl, ULONG cPoints,
                                      ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL pts = (PPOINTL)pptl;
    ULONG i;
    (void)hdc; (void)fnCode;

    if (!inst || !pts || cPoints < 2) return (ULONG)-1;

    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);

    for (i = 0; i < cPoints - 1; i++) {
        SDL_RenderDrawLine(inst->pRen,
                           pts[i].x,     pts[i].y,
                           pts[i+1].x,   pts[i+1].y);
    }
    SDL_RenderPresent(inst->pRen);

    inst->curPos = pts[cPoints - 1];
    return 0;
}

static ULONG APIENTRY Gre2SdlPolyMarker(ULONG hdc, ULONG pptl, ULONG cPoints,
                                        ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL pts = (PPOINTL)pptl;
    ULONG i;
    (void)hdc; (void)fnCode;

    if (!inst || !pts || cPoints < 1) return (ULONG)-1;

    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);

    for (i = 0; i < cPoints; i++) {
        SDL_RenderDrawPoint(inst->pRen, pts[i].x, pts[i].y);
    }
    SDL_RenderPresent(inst->pRen);

    inst->curPos = pts[cPoints - 1];
    return 0;
}

static ULONG APIENTRY Gre2SdlBoxInterior(ULONG hdc, ULONG pBoxParams,
                                         ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBOXPARAMS bp = (PBOXPARAMS)pBoxParams;
    SDL_Rect r;
    (void)hdc; (void)fnCode;

    if (!inst || !bp) return (ULONG)-1;

    r.x = inst->curPos.x;
    r.y = inst->curPos.y;
    r.w = bp->ptl.x - inst->curPos.x;
    r.h = bp->ptl.y - inst->curPos.y;

    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);
    SDL_RenderFillRect(inst->pRen, &r);
    SDL_RenderPresent(inst->pRen);

    inst->curPos = bp->ptl;
    return 0;
}

static ULONG APIENTRY Gre2SdlBoxBoundary(ULONG hdc, ULONG pBoxParams,
                                         ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBOXPARAMS bp = (PBOXPARAMS)pBoxParams;
    SDL_Rect r;
    (void)hdc; (void)fnCode;

    if (!inst || !bp) return (ULONG)-1;

    r.x = inst->curPos.x;
    r.y = inst->curPos.y;
    r.w = bp->ptl.x - inst->curPos.x;
    r.h = bp->ptl.y - inst->curPos.y;

    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);
    SDL_RenderDrawRect(inst->pRen, &r);
    SDL_RenderPresent(inst->pRen);

    inst->curPos = bp->ptl;
    return 0;
}

static ULONG APIENTRY Gre2SdlBoxBoth(ULONG hdc, ULONG pBoxParams,
                                     ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBOXPARAMS bp = (PBOXPARAMS)pBoxParams;
    SDL_Rect r;
    (void)hdc; (void)fnCode;

    if (!inst || !bp) return (ULONG)-1;

    r.x = inst->curPos.x;
    r.y = inst->curPos.y;
    r.w = bp->ptl.x - inst->curPos.x;
    r.h = bp->ptl.y - inst->curPos.y;

    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);
    SDL_RenderFillRect(inst->pRen, &r);
    SDL_RenderDrawRect(inst->pRen, &r);
    SDL_RenderPresent(inst->pRen);

    inst->curPos = bp->ptl;
    return 0;
}

static ULONG APIENTRY Gre2SdlFullArcBoth(ULONG hdc, ULONG fxMultiplier,
                                         ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    FIXED  fx = (FIXED)fxMultiplier;
    int    rx = (int)((fx >> 16) & 0xFFFF);
    int    w;
    (void)hdc; (void)fnCode;

    if (!inst) return (ULONG)-1;

    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);

    for (w = 0; w < rx * 2; w++) {
        int h = (int)sqrt((double)(rx*rx - (w-rx)*(w-rx)));
        SDL_RenderDrawPoint(inst->pRen, inst->curPos.x + w - rx,
                            inst->curPos.y + h);
        SDL_RenderDrawPoint(inst->pRen, inst->curPos.x + w - rx,
                            inst->curPos.y - h);
    }
    SDL_RenderPresent(inst->pRen);
    return 0;
}

static ULONG APIENTRY Gre2SdlPartialArc(ULONG hdc, ULONG pptlCenter,
                                        ULONG fxMultiplier, ULONG fxStart,
                                        ULONG fxSweep, ULONG pInstance,
                                        ULONG fnCode)
{
    (void)pptlCenter; (void)fxMultiplier;
    (void)fxStart; (void)fxSweep; (void)hdc;
    return Gre2SdlFullArcBoth(hdc, fxMultiplier, pInstance, fnCode);
}

static ULONG APIENTRY Gre2SdlPolySpline(ULONG hdc, ULONG pptl, ULONG cPoints,
                                        ULONG pInstance, ULONG fnCode)
{
    return Gre2SdlPolyLine(hdc, pptl, cPoints, pInstance, fnCode);
}

static ULONG APIENTRY Gre2SdlPolyFillet(ULONG hdc, ULONG pptl, ULONG cPoints,
                                        ULONG pInstance, ULONG fnCode)
{
    return Gre2SdlPolyLine(hdc, pptl, cPoints, pInstance, fnCode);
}

static ULONG APIENTRY Gre2SdlCharString(ULONG hdc, ULONG cch, ULONG pch,
                                        ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    (void)hdc; (void)cch; (void)pch; (void)fnCode;

    if (!inst) return (ULONG)-1;

    /* TODO: SDL_ttf integration. For now: a single dot. */
    SDL_SetRenderDrawColor(inst->pRen,
                           (inst->color >> 16) & 0xFF,
                           (inst->color >>  8) & 0xFF,
                            inst->color        & 0xFF,
                            0xFF);
    SDL_RenderDrawPoint(inst->pRen, inst->curPos.x, inst->curPos.y);
    SDL_RenderPresent(inst->pRen);
    return 0;
}

static ULONG APIENTRY Gre2SdlCharStringPos(ULONG hdc, ULONG pptl, ULONG prcl,
                                           ULONG flOptions, ULONG cch,
                                           ULONG pch, ULONG pAdx,
                                           ULONG pCspInfo, ULONG pInstance,
                                           ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptl;
    (void)hdc; (void)prcl; (void)flOptions; (void)cch; (void)pch;
    (void)pAdx; (void)pCspInfo; (void)fnCode;

    if (!inst || !p) return (ULONG)-1;
    inst->curPos = *p;
    return Gre2SdlCharString(hdc, cch, pch, pInstance, fnCode);
}

static ULONG APIENTRY Gre2SdlSetAttributes(ULONG hdc, ULONG lPrimType,
                                           ULONG flAttrs, ULONG flDefMask,
                                           ULONG pBundle, ULONG pInstance,
                                           ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBUNDLE pb = (PBUNDLE)pBundle;
    (void)hdc; (void)flDefMask; (void)fnCode;

    if (!inst || !pb) return (ULONG)-1;

    switch (lPrimType) {
    case PRIM_LINE:
        if (flAttrs & LBB_COLOR)
            inst->color = ((PLINEBUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    case PRIM_CHAR:
        if (flAttrs & CBB_COLOR)
            inst->color = ((PCHARBUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    case PRIM_MARKER:
        if (flAttrs & MBB_COLOR)
            inst->color = ((PMARKERBUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    case PRIM_AREA:
        if (flAttrs & ABB_COLOR)
            inst->color = ((PAREABUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    }
    return 0;
}

static ULONG APIENTRY Gre2SdlGetAttributes(ULONG hdc, ULONG lPrimType,
                                           ULONG flAttrs, ULONG pBundle,
                                           ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBUNDLE pb = (PBUNDLE)pBundle;
    (void)hdc; (void)fnCode;

    if (!inst || !pb) return (ULONG)-1;

    switch (lPrimType) {
    case PRIM_LINE:
        if (flAttrs & LBB_COLOR) ((PLINEBUNDLE)pb)->lColor = inst->color;
        break;
    case PRIM_CHAR:
        if (flAttrs & CBB_COLOR) ((PCHARBUNDLE)pb)->lColor = inst->color;
        break;
    case PRIM_MARKER:
        if (flAttrs & MBB_COLOR) ((PMARKERBUNDLE)pb)->lColor = inst->color;
        break;
    case PRIM_AREA:
        if (flAttrs & ABB_COLOR) ((PAREABUNDLE)pb)->lColor = inst->color;
        break;
    }
    return 0;
}
