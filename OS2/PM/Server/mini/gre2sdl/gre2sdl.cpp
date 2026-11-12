/*!
 * @file gre2sdl.cpp
 * @brief GRE2SDL: presentation driver that renders Gre* calls via SDL2.
 *
 * @section model Driver model
 *
 * GRE2SDL is registered with GRE as a display presentation driver via
 * OS2_PM_DRV_ENABLE. It hooks a subset of Tier-1 Gre* functions by
 * overwriting entries in the dispatch table that GRE passes in
 * FillLogicalDeviceBlock (EN_FILL_LDB). Each hooked function receives
 * a per-DC instance pointer (pInstance) allocated in EnableDeviceContext.
 *
 * @section clients Per-client dispatch tables
 *
 * PMSERVER.EXE is a single process that translates requests from many
 * client processes into Gre* calls. GRE sees each client as a separate
 * "process" and issues a separate FillLogicalDeviceBlock with its own
 * dispatch table for each of them. Therefore:
 *
 *   - FillLogicalDeviceBlock runs once per client (per PMSERVER lifetime),
 *     must always be honoured, and must fill the dispatch table for the
 *     caller. It must NOT gate on global state.
 *   - Global resources (the SDL window, the offscreen surface, the draw
 *     mutex, the worker thread) belong to the PMSERVER process as a whole.
 *     They are created once, lazily, on the first FillLogicalDeviceBlock,
 *     and destroyed in DllMain(DLL_PROCESS_DETACH).
 *
 * @section rendering Rendering model
 *
 * All drawing goes to an offscreen SDL_Surface (g_pBuf). A dedicated
 * worker thread owns the SDL_Window and pumps the window's message queue
 * (SDL_PollEvent / SDL_UpdateWindowSurface) at ~60 Hz. Without a
 * dedicated owner thread the window is unresponsive between Gre* calls,
 * because SDL's message queue is thread-local and only the thread that
 * created the window can drain it.
 *
 * Draw-side Gre* hooks lock g_hmtxDraw, write into g_pBuf, and unlock.
 * The worker locks the same mutex while blitting g_pBuf to the window
 * surface. This serialises access without exposing SDL to the hook side.
 *
 * @section init Why initialisation lives in FillLogicalDeviceBlock
 *
 * Driver initialisation must NOT happen in DllMain: DllMain runs under
 * the process loader lock, and both _beginthread and SDL's internal
 * LoadLibrary/RegisterClass calls would deadlock against it. By the same
 * reason, worker creation from DllMain leaves the new thread unable to
 * run until DllMain returns, while DllMain may be waiting for it.
 * Initialisation therefore runs from the first FillLogicalDeviceBlock,
 * which is not called under the loader lock. This also matches the
 * documented role of FillLogicalDeviceBlock as the place where a
 * presentation driver allocates its global resources.
 *
 * @section state Per-DC vs per-process state
 *
 * GRE2SDL_INSTANCE (pInstance) carries per-DC state: current foreground
 * colour and current position. The window, surface, mutex and worker
 * are process-wide state (file-static globals).
 *
 * FLParamsLst, FLReturnsLst, PFN_OS2_PM_DRV_ENABLE and EN_* subfunction
 * codes are declared in pmddi.h.
 */

#include "F_platform.h"
#include "F_debug.h"
#include "F_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <process.h>

#define INCL_GREALL
#include "pmddi.h"

#include <SDL.h>

/*!
 * @brief Signature of a 6-argument Gre* hook (arity GRE_DRV_6).
 *
 * Used to save and re-dispatch the default dispatch-table entry for
 * GreNotifyClipChange, as required by OS/2 PM Ref Vol. 2.
 */
typedef ULONG (__cdecl *PFN_GRE2SDL_HOOK6)(ULONG, ULONG, ULONG, ULONG,
                                            ULONG, ULONG);

/* ======================================================================== */
/*  Process-wide (PMSERVER-level) state                                     */
/* ======================================================================== */

/*! @brief SDL window owned by the worker thread (NULL until created). */
static SDL_Window   *g_pWin        = NULL;

/*! @brief Offscreen drawing surface; hooks write, worker blits. */
static SDL_Surface  *g_pBuf        = NULL;

/*! @brief Window width in pixels, from fpmsrv.ini (default 640). */
static int           g_cxScreen    = 640;

/*! @brief Window height in pixels, from fpmsrv.ini (default 480). */
static int           g_cyScreen    = 480;

/*! @brief Mutex protecting g_pBuf against concurrent hook/worker access. */
static HMTX          g_hmtxDraw    = NULL;

/*! @brief Worker run flag. Cleared by DllMain(DETACH) and by SDL_QUIT. */
static volatile BOOL g_bRun        = FALSE;

/*! @brief Worker thread id (OS/2 TID) or handle (Win32, cast to ULONG). */
static TID           g_tidWorker   = 0;

/*! @brief Set by the worker once the window and its surface exist. */
static volatile BOOL g_bWinReady   = FALSE;

/*! @brief TRUE once Gre2SdlInit has fully succeeded. */
static volatile BOOL g_bReady      = FALSE;

/*! @brief TRUE once Gre2SdlInit has been attempted (success or not). */
static volatile BOOL g_bInitTried  = FALSE;

/*! @brief Default (GRE-side) GreNotifyClipChange entry, captured in
 *         FillLogicalDeviceBlock before the driver overwrites the
 *         dispatch-table slot.  Gre2SdlNotifyClipChange re-dispatches
 *         to this pointer after its own processing, per OS/2 PM Ref
 *         Vol. 2. */
static PFN_GRE2SDL_HOOK6 g_pfnDefaultNotifyClipChange = NULL;

/* ======================================================================== */
/*  Configuration                                                         */
/* ======================================================================== */

/*!
 * @class Gre2SdlConfig
 * @brief Reads DesktopNx/DesktopNy from fpmsrv.ini.
 *
 * Extends F_Config, which implements the "key=value" file reader and
 * calls AnalizeRecodrRead for each key. Only the two keys needed by the
 * driver are handled here; everything else is forwarded to the base
 * class so that shared settings (e.g. debugOptions) still work.
 */
class Gre2SdlConfig : public F_Config {
public:
    int DesktopNx;
    int DesktopNy;

    Gre2SdlConfig() : DesktopNx(640), DesktopNy(480)
    {
        int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE2SDL] Gre2SdlConfig::Gre2SdlConfig");
        F_DbgCtxExitf(ctx, "Nx=%d Ny=%d", DesktopNx, DesktopNy);
    }

    /*!
     * @brief Handle one key from fpmsrv.ini.
     * @param [in] name  Key name.
     * @param [in] par   Value string.
     * @return 0 if handled, 1 to delegate to F_Config.
     */
    int AnalizeRecodrRead(char *name, char *par)
    {
        int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                                 "[GRE2SDL] Gre2SdlConfig::AnalizeRecodrRead(name=%s)",
                                 name ? name : "(null)");
        if (strcmp(name, "DesktopNx") == 0) { sscanf(par, "%i", &DesktopNx); F_DbgCtxExitf(ctx, "DesktopNx=%d", DesktopNx); return 0; }
        if (strcmp(name, "DesktopNy") == 0) { sscanf(par, "%i", &DesktopNy); F_DbgCtxExitf(ctx, "DesktopNy=%d", DesktopNy); return 0; }
        {
            int rc = F_Config::AnalizeRecodrRead(name, par);
            F_DbgCtxExitf(ctx, "delegated rc=%d", rc);
            return rc;
        }
    }
};

/* ======================================================================== */
/*  Per-DC instance data                                                  */
/* ======================================================================== */

/*!
 * @struct _GRE2SDL_INSTANCE
 * @brief Per-device-context state.
 *
 * Allocated in EnableDeviceContext (EN_ENABLE_CTX) and returned to GRE as
 * pInstance. Every hooked Gre* function receives it back through its
 * pInstance argument. Freed in DisableDeviceContext (EN_DISABLE_CTX).
 *
 * The struct deliberately does NOT contain the window, the surface, the
 * mutex or the worker. Those are process-wide (see the file-scope
 * globals) and shared by all DCs from all clients served by PMSERVER.
 */
typedef struct _GRE2SDL_INSTANCE {
    LONG   lColor;      /*!< Foreground colour, OS/2 0x00BBGGRR layout. */
    POINTL ptlCurPos;   /*!< Current position for this DC. */
    POINTL ptlOrigin;   /*!< DC origin in screen coordinates. */
    RECTL  rclClip;     /*!< Clip region in screen coordinates. */
    BOOL   fClipSet;    /*!< TRUE if rclClip is valid. */
} GRE2SDL_INSTANCE, *PGRE2SDL_INSTANCE;

/* ======================================================================== */
/*  Forward declarations of hooked handlers                               */
/* ======================================================================== */

/*! @brief GreSetCurrentPosition hook. */
static ULONG __cdecl Gre2SdlSetCurrentPosition(ULONG, ULONG, ULONG, ULONG);
/*! @brief GreGetCurrentPosition hook. */
static ULONG __cdecl Gre2SdlGetCurrentPosition(ULONG, ULONG, ULONG, ULONG);
/*! @brief GrePolyLine hook. */
static ULONG __cdecl Gre2SdlPolyLine(ULONG, ULONG, ULONG, ULONG, ULONG);
/*! @brief GrePolyMarker hook. */
static ULONG __cdecl Gre2SdlPolyMarker(ULONG, ULONG, ULONG, ULONG, ULONG);
/*! @brief GreBoxInterior hook. */
static ULONG __cdecl Gre2SdlBoxInterior(ULONG, ULONG, ULONG, ULONG);
/*! @brief GreBoxBoundary hook. */
static ULONG __cdecl Gre2SdlBoxBoundary(ULONG, ULONG, ULONG, ULONG);
/*! @brief GreBoxBoth hook. */
static ULONG __cdecl Gre2SdlBoxBoth(ULONG, ULONG, ULONG, ULONG);
/*! @brief GreFullArcBoth hook. */
static ULONG __cdecl Gre2SdlFullArcBoth(ULONG, ULONG, ULONG, ULONG);
/*! @brief GrePartialArc hook. */
static ULONG __cdecl Gre2SdlPartialArc(ULONG, ULONG, ULONG, ULONG, ULONG,
                                       ULONG, ULONG);
/*! @brief GrePolySpline hook. */
static ULONG __cdecl Gre2SdlPolySpline(ULONG, ULONG, ULONG, ULONG, ULONG);
/*! @brief GrePolyFillet hook. */
static ULONG __cdecl Gre2SdlPolyFillet(ULONG, ULONG, ULONG, ULONG, ULONG);
/*! @brief GreCharString hook. */
static ULONG __cdecl Gre2SdlCharString(ULONG, ULONG, ULONG, ULONG, ULONG);
/*! @brief GreCharStringPos hook. */
static ULONG __cdecl Gre2SdlCharStringPos(ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG);
/*! @brief GreDeviceSetAttributes hook. */
static ULONG __cdecl Gre2SdlSetAttributes(ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG, ULONG);
/*! @brief GreDeviceGetAttributes hook. */
static ULONG __cdecl Gre2SdlGetAttributes(ULONG, ULONG, ULONG, ULONG,
                                           ULONG, ULONG);
/*! @brief GreDeviceSetDCOrigin hook (mandatory for drivers). */
static ULONG __cdecl Gre2SdlDeviceSetDCOrigin(ULONG, ULONG, ULONG, ULONG);
/*! @brief GreNotifyClipChange hook (mandatory for drivers). */
static ULONG __cdecl Gre2SdlNotifyClipChange(ULONG, ULONG, ULONG, ULONG,
                                              ULONG, ULONG);

/* ======================================================================== */
/*  Canvas primitives (write to g_pBuf)                                   */
/* ======================================================================== */

/*!
 * @brief Clear g_pBuf to a solid colour.
 * @note Caller must hold g_hmtxDraw.
 */
static void CanvasClear(Uint8 r, Uint8 g, Uint8 b)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] CanvasClear(r=%u g=%u b=%u)",
                             (unsigned)r, (unsigned)g, (unsigned)b);
    if (!g_pBuf) { F_DbgCtxExitf(ctx, "no buffer"); return; }
    SDL_FillRect(g_pBuf, NULL, SDL_MapRGB(g_pBuf->format, r, g, b));
    F_DbgCtxExitf(ctx, "ok");
}

/*!
 * @brief Write one pixel into g_pBuf, clipping to its bounds.
 * @note Caller must hold g_hmtxDraw.
 */
/*!
 * @brief Write one pixel into g_pBuf, applying DC origin and clip.
 *
 * Coordinates are device coordinates (as received by Gre* hooks).
 * Screen coordinates are computed as device + DC origin.  If a clip
 * region has been set by GreNotifyClipChange, pixels outside it are
 * dropped.
 *
 * @note Caller must hold g_hmtxDraw.
 */
static void CanvasPixel(LONG x, LONG y, Uint8 r, Uint8 g, Uint8 b,
                        PGRE2SDL_INSTANCE inst)
{
    LONG    sx, sy;
    Uint32 *row;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] CanvasPixel(x=%ld y=%ld r=%u g=%u b=%u)",
                             (long)x, (long)y, (unsigned)r, (unsigned)g,
                             (unsigned)b);
    if (!g_pBuf) { F_DbgCtxExitf(ctx, "no buffer"); return; }
    if (!inst)   { F_DbgCtxExitf(ctx, "no inst");  return; }

    sx = x + inst->ptlOrigin.x;
    sy = y + inst->ptlOrigin.y;

    if (inst->fClipSet) {
        if (sx < inst->rclClip.xLeft   || sx >= inst->rclClip.xRight ||
            sy < inst->rclClip.yBottom || sy >= inst->rclClip.yTop) {
            F_DbgCtxExitf(ctx, "clipped (clip)");
            return;
        }
    }

    if (sx < 0 || sx >= g_pBuf->w || sy < 0 || sy >= g_pBuf->h) {
        F_DbgCtxExitf(ctx, "clipped (buf)");
        return;
    }

    row = (Uint32 *)((Uint8 *)g_pBuf->pixels + (size_t)sy * g_pBuf->pitch);
    row[sx] = SDL_MapRGB(g_pBuf->format, r, g, b);
    F_DbgCtxExitf(ctx, "ok");
}

/*!
 * @brief Bresenham line from (x0,y0) to (x1,y1), endpoints inclusive.
 * @note Caller must hold g_hmtxDraw.
 */
/*!
 * @brief Bresenham line from (x0,y0) to (x1,y1), endpoints inclusive.
 * @note Caller must hold g_hmtxDraw.
 */
static void CanvasLine(LONG x0, LONG y0, LONG x1, LONG y1,
                       Uint8 r, Uint8 g, Uint8 b,
                       PGRE2SDL_INSTANCE inst)
{
    LONG dx =  labs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    LONG dy = -labs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    LONG err = dx + dy, e2;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] CanvasLine(%ld,%ld -> %ld,%ld)",
                             (long)x0, (long)y0, (long)x1, (long)y1);

    for (;;) {
        CanvasPixel(x0, y0, r, g, b, inst);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    F_DbgCtxExitf(ctx, "ok");
}

/*!
 * @brief Axis-aligned rectangle, filled or outlined.
 * @note Caller must hold g_hmtxDraw.
 */
/*!
 * @brief Axis-aligned rectangle, filled or outlined.
 * @note Caller must hold g_hmtxDraw.
 */
static void CanvasRect(LONG x, LONG y, LONG w, LONG h,
                       Uint8 r, Uint8 g, Uint8 b, BOOL bFill,
                       PGRE2SDL_INSTANCE inst)
{
    SDL_Rect rc;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] CanvasRect(%ld,%ld %ldx%ld fill=%d)",
                             (long)x, (long)y, (long)w, (long)h, (int)bFill);

    if (!g_pBuf || w <= 0 || h <= 0) { F_DbgCtxExitf(ctx, "no-op"); return; }
    if (!inst) { F_DbgCtxExitf(ctx, "no inst"); return; }

    if (bFill) {
        /* Fill via per-scanline CanvasLine so that origin and clip apply. */
        LONG yy;
        for (yy = 0; yy < h; yy++)
            CanvasLine(x, y + yy, x + w - 1, y + yy, r, g, b, inst);
    } else {
        CanvasLine(x,         y,         x + w - 1, y,         r, g, b, inst);
        CanvasLine(x,         y + h - 1, x + w - 1, y + h - 1, r, g, b, inst);
        CanvasLine(x,         y,         x,         y + h - 1, r, g, b, inst);
        CanvasLine(x + w - 1, y,         x + w - 1, y + h - 1, r, g, b, inst);
    }
    F_DbgCtxExitf(ctx, "ok");
}

/*!
 * @brief Split an OS/2 0x00BBGGRR colour into SDL_MapRGB components.
 */
static void Os2ColorToRgb(LONG lColor, Uint8 *r, Uint8 *g, Uint8 *b)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Os2ColorToRgb(lColor=0x%08lx)",
                             (unsigned long)lColor);
    *r = (Uint8)( lColor        & 0xFF);
    *g = (Uint8)((lColor >>  8) & 0xFF);
    *b = (Uint8)((lColor >> 16) & 0xFF);
    F_DbgCtxExitf(ctx, "r=%u g=%u b=%u", (unsigned)*r, (unsigned)*g, (unsigned)*b);
}

/* ======================================================================== */
/*  Worker thread: owns the SDL window and pumps its message queue        */
/* ======================================================================== */

/*!
 * @brief Body of the window-owning worker thread.
 *
 * Responsibilities, in order:
 *   1. Create SDL_Window and get its window surface.
 *   2. Set g_bWinReady once the window is usable.
 *   3. Loop until g_bRun is cleared or SDL_QUIT is received:
 *      drain SDL events, blit g_pBuf to the window under g_hmtxDraw,
 *      call SDL_UpdateWindowSurface.
 *   4. Destroy the window on exit.
 *
 * Threading contract: SDL's message queue is thread-local.
 * Only this thread may call SDL_PollEvent / SDL_UpdateWindowSurface
 * for g_pWin. All other SDL calls that touch the window surface must
 * go through g_hmtxDraw, which this thread holds while blitting.
 *
 * @param [in] param  Unused.
 */
static void WorkerProc(void *param)
{
    SDL_Window  *pWin;
    SDL_Surface *pScr;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] WorkerProc(param=%p)", param);

    (void)param;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: entry");

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: about to SDL_CreateWindow(%dx%d)",
               g_cxScreen, g_cyScreen);

    pWin = SDL_CreateWindow("GRE2SDL",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            g_cxScreen, g_cyScreen,
                            SDL_WINDOW_SHOWN);
    if (!pWin) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] WorkerProc: SDL_CreateWindow failed: %s",
                   SDL_GetError());
        g_bRun = FALSE;
        F_DbgCtxExitf(ctx, "SDL_CreateWindow failed");
        return;
    }
    g_pWin = pWin;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: SDL_CreateWindow -> %p", (void *)pWin);
    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: about to SDL_GetWindowSurface");

    pScr = SDL_GetWindowSurface(pWin);
    if (!pScr) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] WorkerProc: SDL_GetWindowSurface failed: %s",
                   SDL_GetError());
        SDL_DestroyWindow(pWin);
        g_pWin = NULL;
        g_bRun = FALSE;
        F_DbgCtxExitf(ctx, "SDL_GetWindowSurface failed");
        return;
    }

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: SDL_GetWindowSurface -> %p, "
               "setting g_bWinReady=TRUE", (void *)pScr);

    g_bWinReady = TRUE;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: entering main loop");

    while (g_bRun) {
        SDL_Event e;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                           "[GRE2SDL] WorkerProc: SDL_QUIT received");
                g_bRun = FALSE;
            }
        }

        DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
        SDL_BlitSurface(g_pBuf, NULL, pScr, NULL);
        DosReleaseMutexSem(g_hmtxDraw);

        SDL_UpdateWindowSurface(pWin);
        DosSleep(16);
    }

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: main loop exited, destroying window");

    SDL_DestroyWindow(pWin);
    g_pWin      = NULL;
    g_bWinReady = FALSE;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] WorkerProc: exit");
    F_DbgCtxExitf(ctx, "done");
}

/*!
 * @brief Start the worker thread.
 *
 * The Watcom _beginthread signature differs between targets:
 *   OS/2   : int  _beginthread(void (*)(void *), void *, unsigned, void *)
 *   Win32  : unsigned long _beginthread(void (*)(void *), unsigned, void *)
 *
 * @return TRUE on success, FALSE if the thread could not be created.
 */
static BOOL StartWorker(void)
{
    ULONG tid;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] StartWorker");

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] StartWorker: begin");

#if defined(__OS2__) || defined(__OS2V2__)
    tid = (ULONG)_beginthread(WorkerProc, NULL, 65536, NULL);
#else
    tid = (ULONG)_beginthread(WorkerProc, 65536, NULL);
#endif

    if (tid == (ULONG)-1) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] StartWorker: _beginthread failed");
        F_DbgCtxExitf(ctx, "FALSE (_beginthread failed)");
        return FALSE;
    }
    g_tidWorker = (TID)tid;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] StartWorker: _beginthread -> tid=%lu",
               (unsigned long)g_tidWorker);
    F_DbgCtxExitf(ctx, "TRUE (tid=%lu)", (unsigned long)g_tidWorker);
    return TRUE;
}

/* ======================================================================== */
/*  Lazy driver initialisation                                            */
/* ======================================================================== */

/*!
 * @brief One-time driver initialisation.
 *
 * Called from the first FillLogicalDeviceBlock. Must NOT be called from
 * DllMain: it creates the worker thread and waits for it, and DllMain
 * runs under the process loader lock, which would deadlock the worker
 * on its first LoadLibrary (SDL loads system DLLs internally, and
 * SDL_CreateWindow registers a window class).
 *
 * On failure, resources are left to DllMain(DETACH): some of them
 * (e.g. the worker thread) cannot be safely reclaimed here if the
 * worker is stuck, and the DLL unload path already handles both the
 * "fully initialised" and "partially initialised" cases.
 *
 * Idempotent: once g_bInitTried is set, subsequent calls return the
 * cached g_bReady value without retrying.
 *
 * @return TRUE if SDL, mutex, surface, worker and window are all up.
 */
static BOOL Gre2SdlInit(void)
{
    int i;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlInit");

    if (g_bReady)      { F_DbgCtxExitf(ctx, "TRUE (cached)"); return TRUE; }
    if (g_bInitTried)  { F_DbgCtxExitf(ctx, "FALSE (cached failure)"); return FALSE; }
    g_bInitTried = TRUE;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: begin");

    {
        char szIni[] = "fpmsrv.ini";
        Gre2SdlConfig cfg;
        cfg.Read(szIni);
        g_cxScreen = cfg.DesktopNx;
        g_cyScreen = cfg.DesktopNy;
    }
    if (g_cxScreen < 64 || g_cxScreen > 4096) g_cxScreen = 640;
    if (g_cyScreen < 64 || g_cyScreen > 4096) g_cyScreen = 480;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: size=%dx%d", g_cxScreen, g_cyScreen);

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: about to SDL_Init");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] Gre2SdlInit: SDL_Init failed: %s",
                   SDL_GetError());
        F_DbgCtxExitf(ctx, "FALSE (SDL_Init failed)");
        return FALSE;
    }
    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: SDL_Init ok");

    if (DosCreateMutexSem(NULL, &g_hmtxDraw, 0, FALSE) != 0) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] Gre2SdlInit: DosCreateMutexSem failed");
        g_hmtxDraw = NULL;
        F_DbgCtxExitf(ctx, "FALSE (DosCreateMutexSem failed)");
        return FALSE;
    }
    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: mutex created");

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: about to SDL_CreateRGBSurfaceWithFormat");
    g_pBuf = SDL_CreateRGBSurfaceWithFormat(0, g_cxScreen, g_cyScreen,
                                            32, SDL_PIXELFORMAT_RGBA8888);
    if (!g_pBuf) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] Gre2SdlInit: SDL_CreateRGBSurfaceWithFormat "
                   "failed: %s", SDL_GetError());
        F_DbgCtxExitf(ctx, "FALSE (SDL_CreateRGBSurfaceWithFormat failed)");
        return FALSE;
    }
    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: surface %dx%d created",
               g_pBuf->w, g_pBuf->h);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    CanvasClear(0, 0, 0);
    DosReleaseMutexSem(g_hmtxDraw);

    g_bRun = TRUE;
    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: about to StartWorker");
    if (!StartWorker()) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] Gre2SdlInit: StartWorker failed");
        g_bRun = FALSE;
        F_DbgCtxExitf(ctx, "FALSE (StartWorker failed)");
        return FALSE;
    }

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: worker started, waiting for ready");

    for (i = 0; i < 500; i++) {
        if (g_bWinReady || !g_bRun) break;
        if ((i % 50) == 0) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                       "[GRE2SDL] Gre2SdlInit: wait i=%d "
                       "g_bRun=%d g_bWinReady=%d g_pWin=%p",
                       i, (int)g_bRun, (int)g_bWinReady, (void *)g_pWin);
        }
        DosSleep(10);
    }

    if (!g_bWinReady) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                   "[GRE2SDL] Gre2SdlInit: worker did not report ready; "
                   "g_bRun=%d g_pWin=%p",
                   (int)g_bRun, (void *)g_pWin);
        F_DbgCtxExitf(ctx, "FALSE (worker not ready)");
        return FALSE;
    }

    g_bReady = TRUE;
    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] Gre2SdlInit: done, ready=1");
    F_DbgCtxExitf(ctx, "TRUE");
    return TRUE;
}

/* ======================================================================== */
/*  Process attach / detach                                               */
/* ======================================================================== */

/*!
 * @brief Process-wide lifecycle for gre2sdl.dll.
 *
 * ATTACH does nothing but log: all real work happens lazily in the first
 * FillLogicalDeviceBlock. This keeps the loader lock out of SDL and
 * thread creation.
 *
 * DETACH tears down whatever Gre2SdlInit allocated. The worker wait is
 * bounded: if the worker is stuck (e.g. the display is gone), the wait
 * gives up rather than blocking DllMain indefinitely. On both OS/2 and
 * Win32 the loader is about to kill the worker anyway, so a stuck worker
 * is at worst a small leak in a dying process.
 *
 * The cleanup runs whether or not Gre2SdlInit succeeded, so that
 * partially-allocated resources from a failed init are also released.
 */
#if defined(__WIN32__) || defined(_WIN32)
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] DllMain(reason=%lu)",
                             (unsigned long)reason);

    (void)hinst; (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] DllMain(ATTACH)");
        F_DbgCtxExitf(ctx, "TRUE (ATTACH)");
        return TRUE;
    }

    if (reason == DLL_PROCESS_DETACH) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] DllMain(DETACH): tid=%lu",
                   (unsigned long)g_tidWorker);

        g_bRun = FALSE;
        if (g_tidWorker) {
            int i;
            /* Bounded wait: g_bWinReady is TRUE while the worker is in
               its main loop and FALSE once it has exited (or before it
               started). Wait at most 3 s for it to leave the loop. */
            for (i = 0; i < 300 && g_bWinReady; i++) DosSleep(10);
            g_tidWorker = 0;
        }
        if (g_pBuf)     { SDL_FreeSurface(g_pBuf); g_pBuf = NULL; }
        if (g_hmtxDraw) { DosCloseMutexSem(g_hmtxDraw); g_hmtxDraw = NULL; }
        if (SDL_WasInit(SDL_INIT_VIDEO) != 0) SDL_Quit();
        g_bWinReady = FALSE;
        g_bReady    = FALSE;
        g_bInitTried = FALSE;

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] DllMain(DETACH): done");
        F_DbgCtxExitf(ctx, "TRUE (DETACH)");
        return TRUE;
    }

    F_DbgCtxExitf(ctx, "TRUE (passthrough reason=%lu)", (unsigned long)reason);
    return TRUE;
}
#else
/*!
 * @brief OS/2 DLL entry point (equivalent of Win32 DllMain).
 *
 * flag == 0: process attach; flag == 1: process detach. Both must
 * return non-zero on success.
 */
unsigned long _System _DLL_InitTerm(unsigned long hmod, unsigned long flag)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] _DLL_InitTerm(flag=%lu)",
                             (unsigned long)flag);

    (void)hmod;

    if (flag == 0) {
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] _DLL_InitTerm(attach)");
        F_DbgCtxExitf(ctx, "1 (attach)");
        return 1;
    }

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] _DLL_InitTerm(detach): tid=%lu",
               (unsigned long)g_tidWorker);

    g_bRun = FALSE;
    if (g_tidWorker) {
        int i;
        for (i = 0; i < 300 && g_bWinReady; i++) DosSleep(10);
        g_tidWorker = 0;
    }
    if (g_pBuf)     { SDL_FreeSurface(g_pBuf); g_pBuf = NULL; }
    if (g_hmtxDraw) { DosCloseMutexSem(g_hmtxDraw); g_hmtxDraw = NULL; }
    if (SDL_WasInit(SDL_INIT_VIDEO) != 0) SDL_Quit();
    g_bWinReady  = FALSE;
    g_bReady     = FALSE;
    g_bInitTried = FALSE;

    F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
               "[GRE2SDL] _DLL_InitTerm(detach): done");
    F_DbgCtxExitf(ctx, "1 (detach)");
    return 1;
}
#endif

/* ======================================================================== */
/*  OS2_PM_DRV_ENABLE                                                     */
/* ======================================================================== */

/*!
 * @brief OS/2 PM presentation-driver entry point.
 *
 * Subfunctions of interest:
 *   - EN_FILL_LDB:    per-client dispatch-table hook-up. Runs once for
 *                     every client that PMSERVER opens a DC for, and may
 *                     be re-invoked by GRE at any time; must always be
 *                     honoured and must always fill pRet->DispTab. Also
 *                     drives the lazy one-time global initialisation
 *                     (Gre2SdlInit), because DllMain must not do it.
 *   - EN_FILL_PDB:    no-op. GRE2SDL requests a single physical device
 *                     block (bit 0 of *pRet->Flags cleared in
 *                     EN_FILL_LDB), so there is nothing to do here.
 *   - EN_DISABLE_PDB: no-op. A display never terminates; window teardown
 *                     happens in DllMain(DETACH).
 *   - EN_ENABLE_CTX:  allocate per-DC instance data, return as pInstance.
 *   - EN_DISABLE_CTX: free the per-DC instance data.
 *
 * @param [in]  ulSubFunc  EN_* subfunction code.
 * @param [in]  pParam1    Subfunction-specific first parameter.
 * @param [in]  pParam2    Subfunction-specific second parameter.
 * @return 0 on success, (ULONG)-1 on failure.
 */
extern "C" ULONG EXPENTRY OS2_PM_DRV_ENABLE(ULONG ulSubFunc,
                                            PULONG pParam1,
                                            PLONG pParam2)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] OS2_PM_DRV_ENABLE(sub=%02lxh)",
                             (unsigned long)ulSubFunc);

    switch (ulSubFunc) {

    case EN_FILL_LDB: { /* FillLogicalDeviceBlock */
        PFLParamsLst  pInfo  = (PFLParamsLst)pParam1;
        PFLReturnsLst pRet   = (PFLReturnsLst)pParam2;
        ULONG         ulVersion;
        USHORT        cTableSize;
        PFN          *pTable;
        ULONG         idx;

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] FillLogicalDeviceBlock: enter, "
                   "g_bReady=%d g_bInitTried=%d",
                   (int)g_bReady, (int)g_bInitTried);

        if (!Gre2SdlInit()) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] FillLogicalDeviceBlock: Gre2SdlInit failed");
            F_DbgCtxExitf(ctx, "-1 (Gre2SdlInit failed)");
            return (ULONG)-1;
        }

        if (!pInfo || !pRet || !pRet->DispTab) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] FillLogicalDeviceBlock: NULL pParam1/pParam2");
            F_DbgCtxExitf(ctx, "-1 (NULL pParam1/pParam2)");
            return (ULONG)-1;
        }

        ulVersion  = pInfo->Version;
        cTableSize = pInfo->TableSize;

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] FillLogicalDeviceBlock: version=0x%lx, "
                   "cTableSize=%lu", (unsigned long)ulVersion,
                   (unsigned long)cTableSize);

        pTable = pRet->DispTab;

        /* Install hooks. Slot indices are the low-order byte of the
           corresponding NGre* code, per DDK practice. Attribute set/get
           is hooked at the GreDevice* level (0x9D / 0xA2), not at
           GreSet/GetAttributes (0x17 / 0x18). The latter two codes alias
           Set/GetCurrentPosition in a 256-slot table and must remain
           dedicated to them. */
#define HOOK(ngre, fn) \
            do { \
                idx = (ULONG)((ngre) & 0xFF); \
                if (idx < cTableSize) \
                    pTable[idx] = (PFN)(fn); \
            } while (0)

        /* Save the default (GRE-side) entry for GreNotifyClipChange
         * before overwriting the slot.  Gre2SdlNotifyClipChange will
         * re-dispatch to it, per OS/2 PM Ref Vol. 2. */
        idx = (ULONG)(NGreNotifyClipChange & 0xFF);
        if (idx < cTableSize) {
            g_pfnDefaultNotifyClipChange = (PFN_GRE2SDL_HOOK6)pTable[idx];
        } else {
            g_pfnDefaultNotifyClipChange = NULL;
        }

        HOOK(NGreSetCurrentPosition,   Gre2SdlSetCurrentPosition);
        HOOK(NGreGetCurrentPosition,   Gre2SdlGetCurrentPosition);
        HOOK(NGrePolyLine,             Gre2SdlPolyLine);
        HOOK(NGrePolyMarker,           Gre2SdlPolyMarker);
        HOOK(NGreBoxInterior,          Gre2SdlBoxInterior);
        HOOK(NGreBoxBoundary,          Gre2SdlBoxBoundary);
        HOOK(NGreBoxBoth,              Gre2SdlBoxBoth);
        HOOK(NGreFullArcBoth,          Gre2SdlFullArcBoth);
        HOOK(NGrePartialArc,           Gre2SdlPartialArc);
        HOOK(NGrePolySpline,           Gre2SdlPolySpline);
        HOOK(NGrePolyFillet,           Gre2SdlPolyFillet);
        HOOK(NGreCharString,           Gre2SdlCharString);
        HOOK(NGreCharStringPos,        Gre2SdlCharStringPos);
        HOOK(NGreDeviceSetAttributes,  Gre2SdlSetAttributes);
        HOOK(NGreDeviceGetAttributes,  Gre2SdlGetAttributes);
        HOOK(NGreDeviceSetDCOrigin,    Gre2SdlDeviceSetDCOrigin);
        HOOK(NGreNotifyClipChange,     Gre2SdlNotifyClipChange);
#undef HOOK

        /* FLReturnsLst::Flags is PUSHORT. Only bits 0 and 2 belong to
           the presentation driver; bits 3..15 are reserved and must be
           preserved.
             bit 0 = 0: single shared PDB
             bit 1 = 0: arbitrary DC count
             bit 2 = 1: ignore device/file name in DevOpenDC */
        if (pRet->Flags) {
            *pRet->Flags = (USHORT)((*pRet->Flags & 0xFFF8U) | 0x0004U);
        }

        F_DbgCtxExitf(ctx, "0 (FILL_LDB)");
        return 0;
    }

    case EN_FILL_PDB: { /* FillPhysicalDeviceBlock */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                   "[GRE2SDL] FillPhysicalDeviceBlock");
        F_DbgCtxExitf(ctx, "0 (FILL_PDB)");
        return 0;
    }

    case EN_DISABLE_PDB: /* DisablePhysicalDeviceBlock */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                   "[GRE2SDL] DisablePhysicalDeviceBlock");
        F_DbgCtxExitf(ctx, "0 (DISABLE_PDB)");
        return 0;

    case EN_ENABLE_CTX: { /* EnableDeviceContext */
        PDENPARAMS pDen = (PDENPARAMS)pParam1;
        PGRE2SDL_INSTANCE inst;
        PVOID  pv  = NULL;
        APIRET arc;

        if (!pDen) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] EnableDeviceContext: NULL pParam1");
            F_DbgCtxExitf(ctx, "-1 (NULL pParam1)");
            return (ULONG)-1;
        }

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] EnableDeviceContext: ulType=0x%lx, hdc=%lu",
                   (unsigned long)pDen->ulType,
                   (unsigned long)pDen->ulHDC);

        arc = DosAllocMem(&pv, sizeof(GRE2SDL_INSTANCE),
                          PAG_COMMIT | PAG_READ | PAG_WRITE);
        if (arc != 0 || pv == NULL) {
            F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_ERROR,
                       "[GRE2SDL] EnableDeviceContext: out of memory");
            F_DbgCtxExitf(ctx, "-1 (out of memory)");
            return (ULONG)-1;
        }
        inst = (PGRE2SDL_INSTANCE)pv;
        memset(inst, 0, sizeof(*inst));
        inst->lColor          = 0x00FFFFFF;
        inst->ptlCurPos.x     = 0;
        inst->ptlCurPos.y     = 0;
        inst->ptlOrigin.x     = 0;
        inst->ptlOrigin.y     = 0;
        inst->rclClip.xLeft   = 0;
        inst->rclClip.yBottom = 0;
        inst->rclClip.xRight  = 0;
        inst->rclClip.yTop    = 0;
        inst->fClipSet        = FALSE;

        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] EnableDeviceContext -> pInstance=%p",
                   (void *)inst);
        F_DbgCtxExitf(ctx, "pInstance=%p", (void *)inst);
        return (ULONG)inst;
    }

    case EN_DISABLE_CTX: { /* DisableDeviceContext */
        PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pParam1;
        if (inst) DosFreeMem(inst);
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] DisableDeviceContext");
        F_DbgCtxExitf(ctx, "0 (DISABLE_CTX)");
        return 0;
    }

    case EN_SAVE_CTX:    /* SaveDCState */
    case EN_RESTORE_CTX: /* RestoreDCState */
    case EN_RESET_CTX:   /* ResetDCState */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                   "[GRE2SDL] subfunction %02lxh", (unsigned long)ulSubFunc);
        F_DbgCtxExitf(ctx, "0 (ctx state)");
        return 0;

    case EN_COMPLETE_OPENDC: { /* CompleteOpenDC */
        PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pParam2;
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] CompleteOpenDC: hdc=%lu, pInstance=%p",
                   (unsigned long)pParam1, (void *)inst);
        F_DbgCtxExitf(ctx, "0 (COMPLETE_OPENDC)");
        return 0;
    }

    case EN_BEGIN_CLOSEDC: /* BeginCloseDC */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_INFO,
                   "[GRE2SDL] BeginCloseDC");
        F_DbgCtxExitf(ctx, "0 (BEGIN_CLOSEDC)");
        return 0;

    case EN_INSTALL_HOOKS: /* InstallPMEngineDisplayHooks */
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_WARNING,
                   "[GRE2SDL] InstallPMEngineDisplayHooks not implemented");
        F_DbgCtxExitf(ctx, "0 (INSTALL_HOOKS)");
        return 0;

    default:
        F_DbgPrint(F_DBG_SEC_GRE, F_DBG_LVL_WARNING,
                   "[GRE2SDL] unsupported subfunction %02lxh",
                   (unsigned long)ulSubFunc);
        F_DbgCtxExitf(ctx, "-1 (unsupported subfunction %02lxh)",
                      (unsigned long)ulSubFunc);
        return (ULONG)-1;
    }
}

/* ======================================================================== */
/*  Tier 1: rendering hooks                                               */
/* ======================================================================== */

/*!
 * @brief GreSetCurrentPosition: record the current position for this DC.
 *
 * Pure state update; no drawing.
 */
static ULONG __cdecl Gre2SdlSetCurrentPosition(ULONG hdc, ULONG pptl,
                                                ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptl;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlSetCurrentPosition(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;
    if (!inst || !p) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }
    inst->ptlCurPos = *p;
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GreGetCurrentPosition: return the current position for this DC.
 */
static ULONG __cdecl Gre2SdlGetCurrentPosition(ULONG hdc, ULONG pptl,
                                                ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptl;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlGetCurrentPosition(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;
    if (!inst || !p) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }
    *p = inst->ptlCurPos;
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GrePolyLine: draw a polyline in the DC's foreground colour.
 *
 * Each segment is drawn with Bresenham into g_pBuf under g_hmtxDraw.
 * The current position is updated to the last point.
 */
static ULONG __cdecl Gre2SdlPolyLine(ULONG hdc, ULONG pptl, ULONG cPoints,
                                      ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL pts = (PPOINTL)pptl;
    Uint8   r, g, b;
    ULONG   i;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlPolyLine(fn=0x%lx, hdc=%lu, cPoints=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc,
                             (unsigned long)cPoints);

    (void)hdc; (void)fnCode;

    if (!inst || !pts || cPoints < 2) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    for (i = 0; i < cPoints - 1; i++) {
        CanvasLine(pts[i].x,   pts[i].y,
                   pts[i+1].x, pts[i+1].y, r, g, b, inst);
    }
    DosReleaseMutexSem(g_hmtxDraw);

    inst->ptlCurPos = pts[cPoints - 1];
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GrePolyMarker: draw marker pixels in the DC's foreground colour.
 */
static ULONG __cdecl Gre2SdlPolyMarker(ULONG hdc, ULONG pptl, ULONG cPoints,
                                        ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL pts = (PPOINTL)pptl;
    Uint8   r, g, b;
    ULONG   i;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlPolyMarker(fn=0x%lx, hdc=%lu, cPoints=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc,
                             (unsigned long)cPoints);

    (void)hdc; (void)fnCode;

    if (!inst || !pts || cPoints < 1) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    for (i = 0; i < cPoints; i++) {
        CanvasPixel(pts[i].x, pts[i].y, r, g, b, inst);
    }
    DosReleaseMutexSem(g_hmtxDraw);

    inst->ptlCurPos = pts[cPoints - 1];
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GreBoxInterior: filled rectangle from current position to bp->ptl.
 */
static ULONG __cdecl Gre2SdlBoxInterior(ULONG hdc, ULONG pBoxParams,
                                         ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBOXPARAMS bp = (PBOXPARAMS)pBoxParams;
    Uint8 r, g, b;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlBoxInterior(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;

    if (!inst || !bp) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    CanvasRect(inst->ptlCurPos.x, inst->ptlCurPos.y,
               bp->ptl.x - inst->ptlCurPos.x,
               bp->ptl.y - inst->ptlCurPos.y,
               r, g, b, TRUE, inst);
    DosReleaseMutexSem(g_hmtxDraw);

    inst->ptlCurPos = bp->ptl;
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GreBoxBoundary: outlined rectangle from current position to bp->ptl.
 */
static ULONG __cdecl Gre2SdlBoxBoundary(ULONG hdc, ULONG pBoxParams,
                                         ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBOXPARAMS bp = (PBOXPARAMS)pBoxParams;
    Uint8 r, g, b;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlBoxBoundary(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;

    if (!inst || !bp) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    CanvasRect(inst->ptlCurPos.x, inst->ptlCurPos.y,
               bp->ptl.x - inst->ptlCurPos.x,
               bp->ptl.y - inst->ptlCurPos.y,
               r, g, b, FALSE, inst);
    DosReleaseMutexSem(g_hmtxDraw);

    inst->ptlCurPos = bp->ptl;
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GreBoxBoth: filled rectangle with an outline.
 */
static ULONG __cdecl Gre2SdlBoxBoth(ULONG hdc, ULONG pBoxParams,
                                     ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBOXPARAMS bp = (PBOXPARAMS)pBoxParams;
    Uint8 r, g, b;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlBoxBoth(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;

    if (!inst || !bp) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    CanvasRect(inst->ptlCurPos.x, inst->ptlCurPos.y,
               bp->ptl.x - inst->ptlCurPos.x,
               bp->ptl.y - inst->ptlCurPos.y,
               r, g, b, TRUE, inst);
    CanvasRect(inst->ptlCurPos.x, inst->ptlCurPos.y,
               bp->ptl.x - inst->ptlCurPos.x,
               bp->ptl.y - inst->ptlCurPos.y,
               r, g, b, FALSE, inst);
    DosReleaseMutexSem(g_hmtxDraw);

    inst->ptlCurPos = bp->ptl;
    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GreFullArcBoth: approximate a full circle from the current point.
 *
 * fxMultiplier is a FIXED radius in the high 16 bits. The circle is
 * rasterised by scanning x and solving y = sqrt(r^2 - (x-r)^2), one pair
 * of points per column. Fine enough for a first-pass display driver.
 */
static ULONG __cdecl Gre2SdlFullArcBoth(ULONG hdc, ULONG fxMultiplier,
                                         ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    FIXED  fx = (FIXED)fxMultiplier;
    int    rx = (int)((fx >> 16) & 0xFFFF);
    Uint8  r, g, b;
    int    w;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlFullArcBoth(fn=0x%lx, hdc=%lu, fxMultiplier=0x%lx)",
                             (unsigned long)fnCode, (unsigned long)hdc,
                             (unsigned long)fxMultiplier);

    (void)hdc; (void)fnCode;

    if (!inst || rx <= 0) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    for (w = 0; w < rx * 2; w++) {
        int h = (int)sqrt((double)(rx * rx - (w - rx) * (w - rx)));
        CanvasPixel(inst->ptlCurPos.x + w - rx, inst->ptlCurPos.y + h,
                    r, g, b, inst);
        CanvasPixel(inst->ptlCurPos.x + w - rx, inst->ptlCurPos.y - h,
                    r, g, b, inst);
    }
    DosReleaseMutexSem(g_hmtxDraw);

    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GrePartialArc: currently delegates to GreFullArcBoth.
 *
 * Start angle and sweep are not honoured yet; TODO when the transform
 * layer arrives.
 */
static ULONG __cdecl Gre2SdlPartialArc(ULONG hdc, ULONG pptlCenter,
                                        ULONG fxMultiplier, ULONG fxStart,
                                        ULONG fxSweep, ULONG pInstance,
                                        ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlPartialArc(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);
    ULONG rc;
    (void)pptlCenter; (void)fxStart; (void)fxSweep;
    rc = Gre2SdlFullArcBoth(hdc, fxMultiplier, pInstance, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (delegated)", (unsigned long)rc);
    return rc;
}

/*!
 * @brief GrePolySpline: currently delegates to GrePolyLine.
 */
static ULONG __cdecl Gre2SdlPolySpline(ULONG hdc, ULONG pptl, ULONG cPoints,
                                        ULONG pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlPolySpline(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);
    ULONG rc = Gre2SdlPolyLine(hdc, pptl, cPoints, pInstance, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (delegated)", (unsigned long)rc);
    return rc;
}

/*!
 * @brief GrePolyFillet: currently delegates to GrePolyLine.
 */
static ULONG __cdecl Gre2SdlPolyFillet(ULONG hdc, ULONG pptl, ULONG cPoints,
                                        ULONG pInstance, ULONG fnCode)
{
    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlPolyFillet(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);
    ULONG rc = Gre2SdlPolyLine(hdc, pptl, cPoints, pInstance, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx (delegated)", (unsigned long)rc);
    return rc;
}

/*!
 * @brief GreCharString: placeholder that draws a single pixel.
 *
 * Real glyph rendering requires SDL_ttf and an IFI/font-resolution path.
 * Until that is in place, this preserves the current-position contract
 * (the position is not advanced, matching "no visible advance").
 */
static ULONG __cdecl Gre2SdlCharString(ULONG hdc, ULONG cch, ULONG pch,
                                        ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    Uint8 r, g, b;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlCharString(fn=0x%lx, hdc=%lu, cch=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc,
                             (unsigned long)cch);

    (void)hdc; (void)cch; (void)pch; (void)fnCode;

    if (!inst) { F_DbgCtxExitf(ctx, "-1 (no inst)"); return (ULONG)-1; }

    Os2ColorToRgb(inst->lColor, &r, &g, &b);

    DosRequestMutexSem(g_hmtxDraw, SEM_INDEFINITE_WAIT);
    CanvasPixel(inst->ptlCurPos.x, inst->ptlCurPos.y, r, g, b, inst);
    DosReleaseMutexSem(g_hmtxDraw);

    F_DbgCtxExitf(ctx, "0");
    return 0;
}

/*!
 * @brief GreCharStringPos: set current position, then draw at it.
 */
static ULONG __cdecl Gre2SdlCharStringPos(ULONG hdc, ULONG pptl, ULONG prcl,
                                           ULONG flOptions, ULONG cch,
                                           ULONG pch, ULONG pAdx,
                                           ULONG pCspInfo, ULONG pInstance,
                                           ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptl;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlCharStringPos(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);
    ULONG rc;

    (void)hdc; (void)prcl; (void)flOptions; (void)cch; (void)pch;
    (void)pAdx; (void)pCspInfo; (void)fnCode;

    if (!inst || !p) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }
    inst->ptlCurPos = *p;
    rc = Gre2SdlCharString(hdc, cch, pch, pInstance, fnCode);
    F_DbgCtxExitf(ctx, "0x%lx", (unsigned long)rc);
    return rc;
}

/*!
 * @brief GreDeviceSetAttributes: update the DC's foreground colour.
 *
 * Accepts any of PRIM_LINE / PRIM_CHAR / PRIM_MARKER / PRIM_AREA. Only
 * the colour attribute is currently honoured; other attributes are
 * accepted and ignored until the drawing layer needs them.
 */
static ULONG __cdecl Gre2SdlSetAttributes(ULONG hdc, ULONG lPrimType,
                                           ULONG flAttrs, ULONG flDefMask,
                                           ULONG pBundle, ULONG pInstance,
                                           ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBUNDLE pb = (PBUNDLE)pBundle;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlSetAttributes(fn=0x%lx, hdc=%lu, lPrimType=0x%lx, flAttrs=0x%lx)",
                             (unsigned long)fnCode, (unsigned long)hdc,
                             (unsigned long)lPrimType, (unsigned long)flAttrs);

    (void)hdc; (void)flDefMask; (void)fnCode;

    if (!inst || !pb) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    switch (lPrimType) {
    case PRIM_LINE:
        if (flAttrs & LBB_COLOR)
            inst->lColor = ((PLINEBUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    case PRIM_CHAR:
        if (flAttrs & CBB_COLOR)
            inst->lColor = ((PCHARBUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    case PRIM_MARKER:
        if (flAttrs & MBB_COLOR)
            inst->lColor = ((PMARKERBUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    case PRIM_AREA:
        if (flAttrs & ABB_COLOR)
            inst->lColor = ((PAREABUNDLE)pb)->lColor & 0x00FFFFFF;
        break;
    }
    F_DbgCtxExitf(ctx, "0 (lColor=0x%06lx)", (unsigned long)(inst->lColor & 0x00FFFFFF));
    return 0;
}

/*!
 * @brief GreDeviceGetAttributes: return the DC's foreground colour.
 */
static ULONG __cdecl Gre2SdlGetAttributes(ULONG hdc, ULONG lPrimType,
                                           ULONG flAttrs, ULONG pBundle,
                                           ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PBUNDLE pb = (PBUNDLE)pBundle;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlGetAttributes(fn=0x%lx, hdc=%lu, lPrimType=0x%lx, flAttrs=0x%lx)",
                             (unsigned long)fnCode, (unsigned long)hdc,
                             (unsigned long)lPrimType, (unsigned long)flAttrs);

    (void)hdc; (void)fnCode;

    if (!inst || !pb) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }

    switch (lPrimType) {
    case PRIM_LINE:
        if (flAttrs & LBB_COLOR) ((PLINEBUNDLE)pb)->lColor = inst->lColor;
        break;
    case PRIM_CHAR:
        if (flAttrs & CBB_COLOR) ((PCHARBUNDLE)pb)->lColor = inst->lColor;
        break;
    case PRIM_MARKER:
        if (flAttrs & MBB_COLOR) ((PMARKERBUNDLE)pb)->lColor = inst->lColor;
        break;
    case PRIM_AREA:
        if (flAttrs & ABB_COLOR) ((PAREABUNDLE)pb)->lColor = inst->lColor;
        break;
    }
    F_DbgCtxExitf(ctx, "0 (lColor=0x%06lx)", (unsigned long)(inst->lColor & 0x00FFFFFF));
    return 0;
}

/*!
 * @brief GreDeviceSetDCOrigin: record the DC origin (screen coordinates).
 *
 * Mandatory for all drivers.  The origin is added to every device
 * coordinate before conversion to screen coordinates, per OS/2 PM Ref
 * Vol. 2 ("the presentation driver must add it to all device
 * coordinates to make them screen coordinates").
 *
 * @param [in] hdc       Device context handle.
 * @param [in] pptlDC    Pointer to the DC origin in screen coordinates.
 * @param [in] pInstance Per-DC instance data (GRE2SDL_INSTANCE *).
 * @param [in] fnCode    NGreDeviceSetDCOrigin.
 * @return GPI_OK on success, (ULONG)-1 on failure.
 * @retval 0          Origin recorded.
 * @retval (ULONG)-1  Bad arguments (NULL inst or pptlDC).
 */
static ULONG __cdecl Gre2SdlDeviceSetDCOrigin(ULONG hdc, ULONG pptlDC,
                                               ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PPOINTL p = (PPOINTL)pptlDC;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlDeviceSetDCOrigin(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;

    if (!inst || !p) { F_DbgCtxExitf(ctx, "-1 (bad args)"); return (ULONG)-1; }
    inst->ptlOrigin = *p;
    F_DbgCtxExitf(ctx, "0 (origin=(%ld,%ld))", (long)p->x, (long)p->y);
    return 0;
}

/*!
 * @brief GreNotifyClipChange: cache the new clip region (screen coords).
 *
 * Mandatory for all drivers.  The rectangle is inclusive on left and
 * bottom, exclusive on right and top.  Only the bounding rectangle is
 * cached; the full region (cRect rects) is not enumerated.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] prclBound  Bounding rectangle of the new clip region in
 *                        screen coordinates.
 * @param [in] cRect      Number of rectangles in the new clip region.
 * @param [in] idClipPath Current ClipPath ID (unused for display).
 * @param [in] pInstance  Per-DC instance data (GRE2SDL_INSTANCE *).
 * @param [in] fnCode     NGreNotifyClipChange.
 * @return GPI_OK on success, (ULONG)-1 on failure.
 * @retval 0          Clip region cached.
 * @retval (ULONG)-1  Bad arguments (NULL inst or prclBound).
 */
/*!
 * @brief GreNotifyClipChange: cache the new clip region (screen coords).
 *
 * Mandatory for all drivers.  The rectangle is inclusive on left and
 * bottom, exclusive on right and top.  Only the bounding rectangle is
 * cached; the full region (cRect rects) is not enumerated.
 *
 * On completion re-dispatches to the default (GRE-side) entry saved
 * in FillLogicalDeviceBlock, per OS/2 PM Ref Vol. 2.
 *
 * @param [in] hdc        Device context handle.
 * @param [in] prclBound  Bounding rectangle of the new clip region in
 *                        screen coordinates.
 * @param [in] cRect      Number of rectangles in the new clip region.
 * @param [in] idClipPath Current ClipPath ID (unused for display).
 * @param [in] pInstance  Per-DC instance data (GRE2SDL_INSTANCE *).
 * @param [in] fnCode     NGreNotifyClipChange.
 * @return GPI_OK on success, (ULONG)-1 on failure.
 * @retval 0          Clip region cached, GRE-side entry dispatched.
 * @retval (ULONG)-1  Bad arguments (NULL inst or prclBound).
 */
static ULONG __cdecl Gre2SdlNotifyClipChange(ULONG hdc, ULONG prclBound,
                                              ULONG cRect, ULONG idClipPath,
                                              ULONG pInstance, ULONG fnCode)
{
    PGRE2SDL_INSTANCE inst = (PGRE2SDL_INSTANCE)pInstance;
    PRECTL p = (PRECTL)prclBound;
    ULONG  rc;

    int ctx = F_DbgCtxEnterf(F_DBG_SEC_GRE, F_DBG_LVL_VERBOSE,
                             "[GRE2SDL] Gre2SdlNotifyClipChange(fn=0x%lx, hdc=%lu)",
                             (unsigned long)fnCode, (unsigned long)hdc);

    (void)hdc; (void)fnCode;

    if (!inst || !p) {
        F_DbgCtxExitf(ctx, "-1 (bad args)");
        rc = (ULONG)-1;
    } else {
        inst->rclClip  = *p;
        inst->fClipSet = TRUE;
        F_DbgCtxExitf(ctx, "0 (clip=(%ld,%ld)-(%ld,%ld))",
                      (long)p->xLeft, (long)p->yBottom,
                      (long)p->xRight, (long)p->yTop);
        rc = 0;
    }

    if (g_pfnDefaultNotifyClipChange) {
        g_pfnDefaultNotifyClipChange(hdc, prclBound, cRect, idClipPath,
                                      pInstance, fnCode);
    }
    return rc;
}

