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
 * g_dispatchTable[fnCode & 0xFF](hdc, ..., pInstance, fnCode).
 *
 * The dummy parameter inserted by the pmddim.h macros (the `0L` before
 * fnCode) is replaced with the DC's pInstance.  The driver function
 * signatures in GRE2SDL therefore have one extra ULONG in that position.
 *
 * DENPARAMS, FLParamsLst, FLReturnsLst, PFN_OS2_PM_DRV_ENABLE,
 * DDI_CALLCONV and EN_* subfunction codes are declared in pmddi.h.
 */

#include "F_platform.h"
#include "F_debug.h"
#include "Fs_dc.hpp"

#define INCL_GRE_DCS
#include "pmddi.h"

/*! @def GRE_DISPATCH_SIZE
 *  @brief Number of slots in the local dispatch table.
 *
 *  Must be >= MAXTABLEENTRIES (223) from pmddi.h and cover fnCode & 0xFF. */
#define GRE_DISPATCH_SIZE   256

/* ======================================================================== */
/*  Driver hook shapes (arity 2..10)                                      */
/* ======================================================================== */

/*!
 * @brief Hook signature: arity 2 (a, fnCode).
 *
 * All hooks are stored in g_dispatchTable[] (type PFN) and cast to the
 * correct arity at the call site.  DDI_CALLCONV is used instead of
 * APIENTRY: on Win32, APIENTRY expands to __declspec(__pragma(...)),
 * which Watcom rejects in a typedef.  DDI_CALLCONV is APIENTRY on OS/2
 * and __stdcall on Win32 (same callee-clean stack, standard syntax).
 */
typedef ULONG (DDI_CALLCONV *PFN_GRE2)(ULONG, ULONG);

/*! @brief Hook signature: arity 3 (a, b, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE3)(ULONG, ULONG, ULONG);

/*! @brief Hook signature: arity 4 (a, b, c, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE4)(ULONG, ULONG, ULONG, ULONG);

/*! @brief Hook signature: arity 5 (a, b, c, pInstance, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE5)(ULONG, ULONG, ULONG, ULONG, ULONG);

/*! @brief Hook signature: arity 6 (a, b, c, d, pInstance, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE6)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                       ULONG);

/*! @brief Hook signature: arity 7 (a, b, c, d, e, pInstance, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE7)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                       ULONG, ULONG);

/*! @brief Hook signature: arity 8 (a, b, c, d, e, f, pInstance, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE8)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                       ULONG, ULONG, ULONG);

/*! @brief Hook signature: arity 9 (a, b, c, d, e, f, g, pInstance, fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE9)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                       ULONG, ULONG, ULONG, ULONG);

/*! @brief Hook signature: arity 10 (a, b, c, d, e, f, g, h, pInstance,
 *         fnCode). */
typedef ULONG (DDI_CALLCONV *PFN_GRE10)(ULONG, ULONG, ULONG, ULONG, ULONG,
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

/*! @brief Dispatch table filled by the driver in FillLogicalDeviceBlock. */
static PFN                    g_dispatchTable[GRE_DISPATCH_SIZE];

/*! @brief TRUE after the driver has been loaded and FillLogicalDeviceBlock
 *         has succeeded. */
static BOOL                   g_bDriverLoaded  = FALSE;

/* ======================================================================== */
/*  Static helpers                                                        */
/* ======================================================================== */

/*!
 * @brief Count non-NULL entries in g_dispatchTable.
 * @return Number of hooked functions (0..GRE_DISPATCH_SIZE).
 */
static int HookedCount(void)
{
    int i, n = 0;
    for (i = 0; i < GRE_DISPATCH_SIZE; i++)
        if (g_dispatchTable[i]) n++;
    return n;
}

/*!
 * @brief Load GRE2SDL.DLL and call OS2_PM_DRV_ENABLE(EN_FILL_LDB).
 *
 * Fills g_dispatchTable via FLReturnsLst.DispTab.  The module is NOT
 * freed after load: the dispatch table points into the driver's code.
 *
 * @return TRUE if the driver was loaded and FillLogicalDeviceBlock
 *         succeeded, FALSE otherwise.
 * @retval TRUE   Driver loaded, g_dispatchTable filled, g_pfnEnable set.
 * @retval FALSE  Load / query / FillLogicalDeviceBlock failed;
 *                g_hmodGre2Sdl and g_pfnEnable reset to 0/NULL.
 */
static BOOL LoadGre2Sdl(void)
{
    CHAR         szError[256];
    APIRET       rc;
    FLParamsLst  flParams;
    FLReturnsLst flReturns;
    USHORT       fsDeviceFlags = 0;
    ULONG        drvRc;

    if (g_bDriverLoaded) return TRUE;

    memset(szError, 0, sizeof(szError));
    memset(g_dispatchTable, 0, sizeof(g_dispatchTable));

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

    flParams.Version   = 0x00000210;   /* BCD 2.10 */
    flParams.TableSize = GRE_DISPATCH_SIZE;

    flReturns.Flags   = &fsDeviceFlags;
    flReturns.DispTab = g_dispatchTable;

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

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE] GRE2SDL loaded, flags=0x%04x, %d hooks",
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

/* ======================================================================== */
/*  Gre32EntryN                                                           */
/* ======================================================================== */

extern "C" {

/*!
 * @brief Gre32Entry2 dispatch.
 *
 * Not used by the current parser.  Kept for completeness: if a hook
 * is installed, forward; otherwise return GPI_ERROR.
 */
ULONG APIENTRY Gre32Entry2(ULONG a, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        return ((PFN_GRE2)g_dispatchTable[idx])(a, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry3 dispatch.
 *
 * NGreCloseDC is handled here (BeginCloseDC + DisableDeviceContext).
 * Everything else is forwarded to the driver hook.
 */
ULONG APIENTRY Gre32Entry3(ULONG a, ULONG b, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (fnCode == NGreCloseDC) {
        BOOL ok = DestroyDc((ULONG)a);
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                   "[GRE] GreCloseDC(hdc=%lu) -> %d",
                   (unsigned long)a, (int)ok);
        return ok ? (ULONG)GPI_OK : (ULONG)GPI_ERROR;
    }

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE3)g_dispatchTable[idx])(a, (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry4 dispatch.
 *
 * NGreSetDCOwner is handled here (records owner in Fs_Dc).
 * Everything else is forwarded to the driver hook.
 */
ULONG APIENTRY Gre32Entry4(ULONG a, ULONG b, ULONG c, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (fnCode == NGreSetDCOwner) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        if (!pDc) return (ULONG)GPI_ERROR;
        pDc->SetHab((ULONG)b);
        return (ULONG)GPI_OK;
    }

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE4)g_dispatchTable[idx])(a, b, (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry5 dispatch.
 *
 * Used by GrePolyLine, GrePolyMarker, GreCharString, GrePolySpline,
 * GrePolyFillet.  The dummy `d` becomes pInstance.
 */
ULONG APIENTRY Gre32Entry5(ULONG a, ULONG b, ULONG c, ULONG d, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE5)g_dispatchTable[idx])(a, b, c,
                                                (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry6 dispatch.
 *
 * Used by GreGetAttributes.  The dummy `e` becomes pInstance.
 */
ULONG APIENTRY Gre32Entry6(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE6)g_dispatchTable[idx])(a, b, c, d,
                                                (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry7 dispatch.
 *
 * NGreOpenDC is handled here (lazy driver load + NewDc).
 * Everything else (GreSetAttributes, GrePartialArc) is forwarded;
 * the dummy `f` becomes pInstance.
 */
ULONG APIENTRY Gre32Entry7(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (fnCode == NGreOpenDC) {
        ULONG newHdc;

        if (!g_bDriverLoaded) {
            if (!LoadGre2Sdl()) {
                F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                           "[GRE] GreOpenDC: driver not loaded");
                return (ULONG)-1;
            }
        }

        /* Type comes from GRE unchanged.  Driver decides visible window
           (OD_DIRECT) vs offscreen (OD_MEMORY) in EnableDeviceContext. */
        newHdc = NewDc(0, (ULONG)b);
        if (newHdc == (ULONG)-1) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE] GreOpenDC: NewDc failed");
            return (ULONG)-1;
        }

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE] GreOpenDC(ulType=0x%lx) -> hdc=%lu",
                   (unsigned long)b, (unsigned long)newHdc);
        return newHdc;
    }

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE7)g_dispatchTable[idx])(a, b, c, d, e,
                                                (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry8 dispatch.
 *
 * Not used by the current parser.  Kept for completeness.
 */
ULONG APIENTRY Gre32Entry8(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG g, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE8)g_dispatchTable[idx])(a, b, c, d, e, f,
                                                (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry9 dispatch.
 *
 * Not used by the current parser.  Kept for completeness.
 */
ULONG APIENTRY Gre32Entry9(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                           ULONG f, ULONG g, ULONG h, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE9)g_dispatchTable[idx])(a, b, c, d, e, f, g,
                                                (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

/*!
 * @brief Gre32Entry10 dispatch.
 *
 * Used by GreCharStringPos.  The dummy `i` becomes pInstance.
 */
ULONG APIENTRY Gre32Entry10(ULONG a, ULONG b, ULONG c, ULONG d, ULONG e,
                            ULONG f, ULONG g, ULONG h, ULONG i, ULONG fnCode)
{
    ULONG idx = fnCode & 0xFF;

    if (idx < GRE_DISPATCH_SIZE && g_dispatchTable[idx]) {
        Fs_Dc *pDc = g_greDcs.Find((ULONG)a);
        PVOID  pInst = pDc ? pDc->GetInstance() : NULL;
        return ((PFN_GRE10)g_dispatchTable[idx])(a, b, c, d, e, f, g, h,
                                                 (ULONG)pInst, fnCode);
    }
    return (ULONG)GPI_ERROR;
}

} /* extern "C" */
