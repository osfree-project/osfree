/*!
 * @file dispatch.c
 * @brief Server-side GPI dispatcher with GOCA parser integration.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "goca_parser.hpp"   /* includes goca_common.h + pmord.h + pmddi.h */
#include "F_cmd.h"

/*!
 * @brief Unified logging function for the mini server.
 *
 * @param [in] fmt  printf-style format string.
 * @param [in] ...  Arguments for the format string.
 */
void log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

/*!
 * @def SRV_MAX_PS
 * @brief Maximum number of simultaneous server-side PS entries.
 */
#define SRV_MAX_PS 64

/*!
 * @struct _SRV_PS
 * @brief Server-side per-PS state.
 */
typedef struct _SRV_PS {
    BOOL     inUse;      /*!< Slot occupied.                              */
    HPS      hps;        /*!< Client-facing handle, or NULL if unset.     */
    ULONG    flOptions;  /*!< Last flOptions from NGpiCreatePS/NGpiSetPS. */
    GOCA_CTX goca;       /*!< GOCA parser context.                        */
} SRV_PS, *PSRV_PS;

/*!
 * @brief Server-side PS table (one entry per live PS).
 */
static SRV_PS g_srvPsTable[SRV_MAX_PS];

/*!
 * @brief Look up a PS entry by handle.
 * @param [in] hps Handle previously returned by SrvPsCreate.
 * @return Entry pointer, or NULL if not found / out of range / freed.
 */
static PSRV_PS SrvPsFind(HPS hps)
{
    int i;
    PSRV_PS ps = (PSRV_PS)hps;

    if (ps >= g_srvPsTable && ps < g_srvPsTable + SRV_MAX_PS && ps->inUse) {
        return ps;
    }

    for (i = 0; i < SRV_MAX_PS; i++) {
        if (g_srvPsTable[i].inUse &&
            g_srvPsTable[i].hps != NULL &&
            g_srvPsTable[i].hps == hps) {
            return &g_srvPsTable[i];
        }
    }
    return (PSRV_PS)NULL;
}

/*!
 * @brief Register a new PS entry.
 * @param [in] hps       Client handle, or NULL if returned to client as pointer.
 * @param [in] flOptions Options from NGpiCreatePS.
 * @return Entry pointer (used as HPS), or NULL if the table is full.
 */
static PSRV_PS SrvPsCreate(HPS hps, ULONG flOptions)
{
    int i;
    for (i = 0; i < SRV_MAX_PS; i++) {
        if (!g_srvPsTable[i].inUse) {
            g_srvPsTable[i].inUse     = TRUE;
            g_srvPsTable[i].hps       = hps;
            g_srvPsTable[i].flOptions = flOptions;
            GocaInit(&g_srvPsTable[i].goca, (HDC)hps, NULL, 0);
            GocaSetFormatFromOptions(&g_srvPsTable[i].goca, flOptions);
            return &g_srvPsTable[i];
        }
    }
    return (PSRV_PS)NULL;
}

/*!
 * @brief Find or auto-create a PS entry.
 * @param [in] hps Handle from the client.
 * @return Entry pointer, or NULL if the table is full.
 */
static PSRV_PS SrvPsGet(HPS hps)
{
    PSRV_PS ps = SrvPsFind(hps);
    if (ps) return ps;
    return SrvPsCreate(hps, 0);
}

/*!
 * @brief Release a PS entry.
 * @param [in] ps Entry to release.
 */
static void SrvPsDestroy(PSRV_PS ps)
{
    if (ps) {
        ps->inUse     = FALSE;
        ps->hps       = NULL;
        ps->flOptions = 0;
    }
}

/* ======================================================================== */
/*  Order emitters                                                          */
/* ======================================================================== */

/*!
 * @brief Feed a single order to the GOCA parser.
 * @param [in] ps       Target PS entry.
 * @param [in] pOrder   Order bytes.
 * @param [in] cbOrder  Order length.
 */
static void SrvParseOrder(SRV_PS *ps, const BYTE *pOrder, ULONG cbOrder)
{
    ps->goca.pbStream = (PBYTE)pOrder;
    ps->goca.cbStream = cbOrder;
    GocaParseStream(&ps->goca);
}

/*!
 * @brief Emit OCODE_GSCP (set current position).
 */
static void SrvEmitMove(SRV_PS *ps, POINTL ptl)
{
    BYTE  buf[10];
    ULONG pos = 0;

    buf[pos++] = OCODE_GSCP;
    buf[pos++] = (BYTE)(2 * ps->goca.coordSize);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.x);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.y);

    SrvParseOrder(ps, buf, pos);
}

/*!
 * @brief Emit OCODE_GCLINE (line from current position to endpoint).
 */
static void SrvEmitLine(SRV_PS *ps, POINTL ptl)
{
    BYTE  buf[10];
    ULONG pos = 0;

    buf[pos++] = OCODE_GCLINE;
    buf[pos++] = (BYTE)(2 * ps->goca.coordSize);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.x);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.y);

    SrvParseOrder(ps, buf, pos);
}

/*!
 * @brief Emit OCODE_GCBOX (box from current position to opposite corner).
 */
static void SrvEmitBox(SRV_PS *ps, LONG lControl, POINTL ptl,
                       LONG lHRound, LONG lVRound)
{
    BYTE  buf[22];
    ULONG pos = 0;
    UCHAR fbFlags = 0;
    BOOL  bAxes = (lHRound || lVRound) ? TRUE : FALSE;

    if (lControl & 2) fbFlags |= 0x40;  /* DRO_OUTLINE -> boundary */
    if (lControl & 1) fbFlags |= 0x20;  /* DRO_FILL    -> fill     */

    buf[pos++] = OCODE_GCBOX;
    buf[pos++] = (BYTE)(2 + 2 * ps->goca.coordSize +
                        (bAxes ? 2 * ps->goca.coordSize : 0));
    buf[pos++] = fbFlags;
    buf[pos++] = 0;  /* pad */
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.x);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.y);
    if (bAxes) {
        pos += GocaPutCoord(&ps->goca, buf + pos, lHRound);
        pos += GocaPutCoord(&ps->goca, buf + pos, lVRound);
    }

    SrvParseOrder(ps, buf, pos);
}

/*!
 * @brief Emit color order (GSCOL for 0..255, GSECOL for 256..0xFFFF).
 * @return TRUE if emitted, FALSE if the color value is unsupported.
 */
static BOOL SrvEmitColor(SRV_PS *ps, LONG lColor)
{
    BYTE buf[4];

    if (lColor < 0) return FALSE;

    if (lColor <= 0xFF) {
        buf[0] = OCODE_GSCOL;
        buf[1] = (BYTE)lColor;
        SrvParseOrder(ps, buf, 2);
        return TRUE;
    }

    if (lColor <= 0xFFFF) {
        buf[0] = OCODE_GSECOL;
        buf[1] = 2;
        GocaPutU16(&ps->goca, buf + 2, (USHORT)lColor);
        SrvParseOrder(ps, buf, 4);
        return TRUE;
    }

    return FALSE;
}

/*!
 * @brief Emit OCODE_GLINE for a multi-point polyline.
 */
static void SrvEmitPolyLine(SRV_PS *ps, LONG lCount, PPOINTL aptl)
{
    PVOID pv = NULL;
    PBYTE p;
    ULONG pos = 0;
    LONG  i;
    ULONG pointSize = ps->goca.coordSize;
    ULONG cbOrder = 2 + lCount * 2 * pointSize;

    if (lCount <= 0) return;
    if (DosAllocMem(&pv, cbOrder, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) return;
    p = (PBYTE)pv;

    p[pos++] = OCODE_GLINE;
    p[pos++] = (BYTE)(lCount * 2 * pointSize);
    for (i = 0; i < lCount; i++) {
        pos += GocaPutCoord(&ps->goca, p + pos, aptl[i].x);
        pos += GocaPutCoord(&ps->goca, p + pos, aptl[i].y);
    }

    SrvParseOrder(ps, p, pos);
    DosFreeMem(pv);
}

/*!
 * @brief Emit OCODE_GCCHST (char string at current position).
 */
static void SrvEmitCharString(SRV_PS *ps, LONG lCount, const char *pchString)
{
    PVOID pv = NULL;
    PBYTE p;
    ULONG pos = 0;
    ULONG cbOrder = 2 + lCount;

    if (lCount <= 0) return;
    if (DosAllocMem(&pv, cbOrder, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) return;
    p = (PBYTE)pv;
    p[pos++] = OCODE_GCCHST;
    p[pos++] = (BYTE)lCount;
    memcpy(p + pos, pchString, lCount);
    pos += lCount;

    SrvParseOrder(ps, p, pos);
    DosFreeMem(pv);
}

/*!
 * @brief Emit OCODE_GCHST (char string at explicit point).
 */
static void SrvEmitCharStringAt(SRV_PS *ps, POINTL ptl, LONG lCount,
                                const char *pchString)
{
    PVOID pv = NULL;
    PBYTE p;
    ULONG pos = 0;
    ULONG pointSize = ps->goca.coordSize;
    ULONG cbOrder = 2 + 2 * pointSize + lCount;

    if (lCount <= 0) return;
    if (DosAllocMem(&pv, cbOrder, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) return;
    p = (PBYTE)pv;
    p[pos++] = OCODE_GCHST;
    p[pos++] = (BYTE)(2 * pointSize + lCount);
    pos += GocaPutCoord(&ps->goca, p + pos, ptl.x);
    pos += GocaPutCoord(&ps->goca, p + pos, ptl.y);
    memcpy(p + pos, pchString, lCount);
    pos += lCount;

    SrvParseOrder(ps, p, pos);
    DosFreeMem(pv);
}

/*!
 * @brief Emit OCODE_GMRK (single marker).
 */
static void SrvEmitMarker(SRV_PS *ps, POINTL ptl)
{
    BYTE  buf[10];
    ULONG pos = 0;

    buf[pos++] = OCODE_GMRK;
    buf[pos++] = (BYTE)(2 * ps->goca.coordSize);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.x);
    pos += GocaPutCoord(&ps->goca, buf + pos, ptl.y);

    SrvParseOrder(ps, buf, pos);
}

/*!
 * @brief Emit OCODE_GMRK for a multi-point marker set.
 */
static void SrvEmitPolyMarker(SRV_PS *ps, LONG lCount, PPOINTL aptl)
{
    PVOID pv = NULL;
    PBYTE p;
    ULONG pos = 0;
    LONG  i;
    ULONG pointSize = ps->goca.coordSize;
    ULONG cbOrder = 2 + lCount * 2 * pointSize;

    if (lCount <= 0) return;
    if (DosAllocMem(&pv, cbOrder, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) return;
    p = (PBYTE)pv;
    p[pos++] = OCODE_GMRK;
    p[pos++] = (BYTE)(lCount * 2 * pointSize);
    for (i = 0; i < lCount; i++) {
        pos += GocaPutCoord(&ps->goca, p + pos, aptl[i].x);
        pos += GocaPutCoord(&ps->goca, p + pos, aptl[i].y);
    }

    SrvParseOrder(ps, p, pos);
    DosFreeMem(pv);
}

/*!
 * @brief Emit attribute orders for GpiSetAttrs.
 */
static void SrvEmitSetAttrs(SRV_PS *ps, LONG lPrimType, ULONG flAttrMask,
                            PBUNDLE pb)
{
    BYTE  buf[32];
    ULONG pos;

    if (lPrimType == PRIM_LINE) {
        PLINEBUNDLE lb = (PLINEBUNDLE)pb;
        if (flAttrMask & LBB_COLOR) SrvEmitColor(ps, lb->lColor);
        if (flAttrMask & LBB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)lb->usMixMode;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & LBB_TYPE) {
            buf[0] = OCODE_GSLT; buf[1] = 1; buf[2] = (BYTE)lb->usType;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & LBB_WIDTH) {
            buf[0] = OCODE_GSLW; buf[1] = 1; buf[2] = (BYTE)lb->lGeomWidth;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & LBB_END) {
            buf[0] = OCODE_GSLE; buf[1] = 1; buf[2] = (BYTE)lb->usEnd;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & LBB_JOIN) {
            buf[0] = OCODE_GSLJ; buf[1] = 1; buf[2] = (BYTE)lb->usJoin;
            SrvParseOrder(ps, buf, 3);
        }
    } else if (lPrimType == PRIM_CHAR) {
        PCHARBUNDLE cb = (PCHARBUNDLE)pb;
        if (flAttrMask & CBB_COLOR) SrvEmitColor(ps, cb->lColor);
        if (flAttrMask & CBB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)cb->usMixMode;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & CBB_SET) {
            buf[0] = OCODE_GSCS; buf[1] = 1; buf[2] = (BYTE)cb->usSet;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & CBB_DIRECTION) {
            buf[0] = OCODE_GSCD; buf[1] = 1; buf[2] = (BYTE)cb->usDirection;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & CBB_MODE) {
            buf[0] = OCODE_GSCR; buf[1] = 1; buf[2] = (BYTE)cb->usPrecision;
            SrvParseOrder(ps, buf, 3);
        }
    } else if (lPrimType == PRIM_MARKER) {
        PMARKERBUNDLE mb = (PMARKERBUNDLE)pb;
        if (flAttrMask & MBB_COLOR) SrvEmitColor(ps, mb->lColor);
        if (flAttrMask & MBB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)mb->usMixMode;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & MBB_SET) {
            buf[0] = OCODE_GSMS; buf[1] = 1; buf[2] = (BYTE)mb->usSet;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & MBB_SYMBOL) {
            buf[0] = OCODE_GSMT; buf[1] = 1; buf[2] = (BYTE)mb->usSymbol;
            SrvParseOrder(ps, buf, 3);
        }
    } else if (lPrimType == PRIM_AREA) {
        PAREABUNDLE ab = (PAREABUNDLE)pb;
        if (flAttrMask & ABB_COLOR) SrvEmitColor(ps, ab->lColor);
        if (flAttrMask & ABB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)ab->usMixMode;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & ABB_SET) {
            buf[0] = OCODE_GSPS; buf[1] = 1; buf[2] = (BYTE)ab->usSet;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & ABB_SYMBOL) {
            buf[0] = OCODE_GSPT; buf[1] = 1; buf[2] = (BYTE)ab->usSymbol;
            SrvParseOrder(ps, buf, 3);
        }
        if (flAttrMask & ABB_REF_POINT) {
            ULONG pointSize = ps->goca.coordSize;
            buf[0] = OCODE_GSPRP;
            buf[1] = (BYTE)(2 + 2 * pointSize);
            buf[2] = 0;
            buf[3] = 0;
            pos = 4;
            pos += GocaPutCoord(&ps->goca, buf + pos, ab->ptlRefPoint.x);
            pos += GocaPutCoord(&ps->goca, buf + pos, ab->ptlRefPoint.y);
            SrvParseOrder(ps, buf, pos);
        }
    }
}


/*!
 * @def DISPATCH_STUB
 * @brief Log entry/exit for a single stub case and return GPI_ERROR.
 * @param [in] code  NGpi* constant from gpi_wire.h.
 */
#define DISPATCH_STUB(code) \
    case code: \
        log("[PMSERVER] IN  : " #code " (hps=%p, cbArgs=%u)\n", (void*)hps, (unsigned)cbArgs); \
        log("[PMSERVER] OUT : " #code " -> 0x%x\n", (unsigned int)GPI_ERROR); \
        return (ULONG)GPI_ERROR;

/*!
 * @brief Server dispatch for GPI calls.
 *
 * @param [in] hps      Presentation space handle.
 * @param [in] ulFnCode Function code (NGpi*).
 * @param [in] pArgs    Marshaled arguments.
 * @param [in] cbArgs   Size of pArgs in bytes.
 * @return GPI result.
 * @retval GPI_ERROR  Unknown or unimplemented function code.
 * @retval 1          Successful NGpiSetPS / NGpiMove / NGpiLine /
 *                    NGpiBox / NGpiSetColor / NGpiDestroyPS /
 *                    NGpiPolyLine / NGpiCharString / NGpiCharStringAt /
 *                    NGpiMarker / NGpiPolyMarker / NGpiSetAttrs.
 * @retval p          NGpiCreatePS: pointer to the new SRV_PS entry.
 */
ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs)
{
    log("[PMSERVER] Dispatch IN : FnCode=0x%x, ArgsSize=%u\n", ulFnCode, cbArgs);

    switch (ulFnCode) {

    /* ==================================================================== */
    /*  Presentation space lifecycle                                        */
    /* ==================================================================== */

    case NGpiCreatePS: {
        PGpiCreatePS_Args a = (PGpiCreatePS_Args)pArgs;
        PSRV_PS ps;

        if (!a || cbArgs < sizeof(*a)) {
            log("[PMSERVER] IN  : NGpiCreatePS (bad args, cbArgs=%u)\n",
                (unsigned)cbArgs);
            return (ULONG)GPI_ERROR;
        }

        ps = SrvPsCreate(NULL, a->flOptions);
        if (!ps) {
            log("[PMSERVER] IN  : NGpiCreatePS (PS table full)\n");
            return (ULONG)GPI_ERROR;
        }

        log("[PMSERVER] IN  : NGpiCreatePS(size={%ld,%ld}, flOptions=0x%lx) -> %p\n",
            (long)a->psizlSize.cx, (long)a->psizlSize.cy,
            (unsigned long)a->flOptions, (void*)ps);
        log("[PMSERVER] OUT : NGpiCreatePS -> %p\n", (void*)ps);
        return (ULONG)ps;
    }

    case NGpiSetPS: {
        PGpiSetPS_Args a = (PGpiSetPS_Args)pArgs;
        PSRV_PS ps = SrvPsFind(hps);

        log("[PMSERVER] IN  : NGpiSetPS(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!ps) {
            log("[PMSERVER] OUT : NGpiSetPS -> GPI_ERROR (no ps)\n");
            return (ULONG)GPI_ERROR;
        }
        if (a && cbArgs >= sizeof(*a)) {
            ps->flOptions = a->flOptions;
            GocaSetFormatFromOptions(&ps->goca, a->flOptions);
        }
        log("[PMSERVER] OUT : NGpiSetPS -> 1\n");
        return 1;
    }

    case NGpiDestroyPS: {
        PSRV_PS ps = SrvPsFind(hps);
        log("[PMSERVER] IN  : NGpiDestroyPS(hps=%p)\n", (void*)hps);
        if (ps) SrvPsDestroy(ps);
        log("[PMSERVER] OUT : NGpiDestroyPS -> %d\n",
            ps ? 1 : (int)GPI_ERROR);
        return (ULONG)(ps ? 1 : GPI_ERROR);
    }

    /* ==================================================================== */
    /*  Drawing primitives (emit GOCA orders)                               */
    /* ==================================================================== */

    case NGpiSetColor: {
        LONG lColor;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiSetColor(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) {
            log("[PMSERVER] OUT : NGpiSetColor -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&lColor, pArgs, sizeof(LONG));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (!SrvEmitColor(ps, lColor)) {
            log("[PMSERVER] OUT : NGpiSetColor -> GPI_ERROR (unsupported color 0x%lx)\n",
                (unsigned long)lColor);
            return (ULONG)GPI_ERROR;
        }
        log("[PMSERVER] OUT : NGpiSetColor -> 1\n");
        return 1;
    }

    case NGpiMove: {
        POINTL ptl;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiMove(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiMove -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitMove(ps, ptl);
        log("[PMSERVER] OUT : NGpiMove -> 1\n");
        return 1;
    }

    case NGpiLine: {
        POINTL ptl;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiLine(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiLine -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitLine(ps, ptl);
        log("[PMSERVER] OUT : NGpiLine -> 1\n");
        return 1;
    }

    case NGpiBox: {
        PGpiBox_Args a = (PGpiBox_Args)pArgs;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiBox(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < sizeof(*a)) {
            log("[PMSERVER] OUT : NGpiBox -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitBox(ps, a->lControl, a->ptl, a->lHRound, a->lVRound);
        log("[PMSERVER] OUT : NGpiBox -> 1\n");
        return 1;
    }

    case NGpiPolyLine: {
        LONG lCount;
        PPOINTL aptl;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiPolyLine(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (cbArgs < sizeof(LONG)) {
            log("[PMSERVER] OUT : NGpiPolyLine -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        lCount = *(LONG*)pArgs;
        if (lCount <= 0 ||
            cbArgs < sizeof(LONG) + (ULONG)lCount * sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiPolyLine -> GPI_ERROR (bad size)\n");
            return (ULONG)GPI_ERROR;
        }
        aptl = (PPOINTL)(pArgs + sizeof(LONG));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitPolyLine(ps, lCount, aptl);
        log("[PMSERVER] OUT : NGpiPolyLine -> 1\n");
        return 1;
    }

    case NGpiCharString: {
        LONG lCount;
        char *pchString;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiCharString(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (cbArgs < sizeof(LONG)) {
            log("[PMSERVER] OUT : NGpiCharString -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        lCount = *(LONG*)pArgs;
        if (lCount < 0 || cbArgs < sizeof(LONG) + (ULONG)lCount) {
            log("[PMSERVER] OUT : NGpiCharString -> GPI_ERROR (bad size)\n");
            return (ULONG)GPI_ERROR;
        }
        pchString = (char*)(pArgs + sizeof(LONG));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitCharString(ps, lCount, pchString);
        log("[PMSERVER] OUT : NGpiCharString -> 1\n");
        return 1;
    }

    case NGpiCharStringAt: {
        POINTL ptl;
        LONG lCount;
        char *pchString;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiCharStringAt(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (cbArgs < sizeof(POINTL) + sizeof(LONG)) {
            log("[PMSERVER] OUT : NGpiCharStringAt -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        lCount = *(LONG*)(pArgs + sizeof(POINTL));
        if (lCount < 0 ||
            cbArgs < sizeof(POINTL) + sizeof(LONG) + (ULONG)lCount) {
            log("[PMSERVER] OUT : NGpiCharStringAt -> GPI_ERROR (bad size)\n");
            return (ULONG)GPI_ERROR;
        }
        pchString = (char*)(pArgs + sizeof(POINTL) + sizeof(LONG));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitCharStringAt(ps, ptl, lCount, pchString);
        log("[PMSERVER] OUT : NGpiCharStringAt -> 1\n");
        return 1;
    }

    case NGpiMarker: {
        POINTL ptl;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiMarker(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiMarker -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitMarker(ps, ptl);
        log("[PMSERVER] OUT : NGpiMarker -> 1\n");
        return 1;
    }

    case NGpiPolyMarker: {
        LONG lCount;
        PPOINTL aptl;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiPolyMarker(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (cbArgs < sizeof(LONG)) {
            log("[PMSERVER] OUT : NGpiPolyMarker -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        lCount = *(LONG*)pArgs;
        if (lCount <= 0 ||
            cbArgs < sizeof(LONG) + (ULONG)lCount * sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiPolyMarker -> GPI_ERROR (bad size)\n");
            return (ULONG)GPI_ERROR;
        }
        aptl = (PPOINTL)(pArgs + sizeof(LONG));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitPolyMarker(ps, lCount, aptl);
        log("[PMSERVER] OUT : NGpiPolyMarker -> 1\n");
        return 1;
    }

    case NGpiSetAttrs: {
        LONG lPrimType;
        ULONG flAttrMask;
        ULONG flDefMask;
        PBUNDLE pb;
        PSRV_PS ps;

        log("[PMSERVER] IN  : NGpiSetAttrs(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (cbArgs < sizeof(LONG) + sizeof(ULONG) + sizeof(ULONG)) {
            log("[PMSERVER] OUT : NGpiSetAttrs -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        lPrimType = *(LONG*)pArgs;
        flAttrMask = *(ULONG*)(pArgs + sizeof(LONG));
        flDefMask = *(ULONG*)(pArgs + sizeof(LONG) + sizeof(ULONG));
        pb = (PBUNDLE)(pArgs + sizeof(LONG) + sizeof(ULONG) + sizeof(ULONG));
        ps = SrvPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        SrvEmitSetAttrs(ps, lPrimType, flAttrMask, pb);
        log("[PMSERVER] OUT : NGpiSetAttrs -> 1\n");
        return 1;
    }

    /* ==================================================================== */
    /*  Everything else: stub                                               */
    /* ==================================================================== */

    DISPATCH_STUB(NGpiAssociate)
    DISPATCH_STUB(NGpiRestorePS)
    DISPATCH_STUB(NGpiSavePS)
    DISPATCH_STUB(NGpiErase)
    DISPATCH_STUB(NGpiQueryDevice)
    DISPATCH_STUB(NGpiResetPS)
    DISPATCH_STUB(NGpiQueryPS)
    DISPATCH_STUB(NGpiErrorSegmentData)
    DISPATCH_STUB(NGpiQueryDrawControl)
    DISPATCH_STUB(NGpiSetDrawControl)
    DISPATCH_STUB(NGpiQueryDrawingMode)
    DISPATCH_STUB(NGpiSetDrawingMode)
    DISPATCH_STUB(NGpiQueryStopDraw)
    DISPATCH_STUB(NGpiSetStopDraw)
    DISPATCH_STUB(NGpiCorrelateChain)
    DISPATCH_STUB(NGpiQueryTag)
    DISPATCH_STUB(NGpiSetTag)
    DISPATCH_STUB(NGpiQueryPickApertureSize)
    DISPATCH_STUB(NGpiSetPickApertureSize)
    DISPATCH_STUB(NGpiQueryPickAperturePosition)
    DISPATCH_STUB(NGpiSetPickAperturePosition)
    DISPATCH_STUB(NGpiQueryBoundaryData)
    DISPATCH_STUB(NGpiResetBoundaryData)
    DISPATCH_STUB(NGpiCorrelateFrom)
    DISPATCH_STUB(NGpiCorrelateSegment)
    DISPATCH_STUB(NGpiOpenSegment)
    DISPATCH_STUB(NGpiCloseSegment)
    DISPATCH_STUB(NGpiDeleteSegment)
    DISPATCH_STUB(NGpiQueryInitialSegmentAttrs)
    DISPATCH_STUB(NGpiSetInitialSegmentAttrs)
    DISPATCH_STUB(NGpiQuerySegmentAttrs)
    DISPATCH_STUB(NGpiSetSegmentAttrs)
    DISPATCH_STUB(NGpiQuerySegmentPriority)
    DISPATCH_STUB(NGpiSetSegmentPriority)
    DISPATCH_STUB(NGpiDeleteSegments)
    DISPATCH_STUB(NGpiQuerySegmentNames)
    DISPATCH_STUB(NGpiGetData)
    DISPATCH_STUB(NGpiPutData)
    DISPATCH_STUB(NGpiDrawChain)
    DISPATCH_STUB(NGpiDrawFrom)
    DISPATCH_STUB(NGpiDrawSegment)
    DISPATCH_STUB(NGpiDrawDynamics)
    DISPATCH_STUB(NGpiRemoveDynamics)
    DISPATCH_STUB(NGpiBeginElement)
    DISPATCH_STUB(NGpiEndElement)
    DISPATCH_STUB(NGpiLabel)
    DISPATCH_STUB(NGpiElement)
    DISPATCH_STUB(NGpiQueryElement)
    DISPATCH_STUB(NGpiDeleteElement)
    DISPATCH_STUB(NGpiDeleteElementRange)
    DISPATCH_STUB(NGpiDeleteElementsBetweenLabels)
    DISPATCH_STUB(NGpiQueryEditMode)
    DISPATCH_STUB(NGpiSetEditMode)
    DISPATCH_STUB(NGpiQueryElementPointer)
    DISPATCH_STUB(NGpiSetElementPointer)
    DISPATCH_STUB(NGpiOffsetElementPointer)
    DISPATCH_STUB(NGpiQueryElementType)
    DISPATCH_STUB(NGpiSetElementPointerAtLabel)
    DISPATCH_STUB(NGpiQuerySegmentTransformMatrix)
    DISPATCH_STUB(NGpiSetSegmentTransformMatrix)
    DISPATCH_STUB(NGpiConvert)
    DISPATCH_STUB(NGpiConvertWithMatrix)
    DISPATCH_STUB(NGpiQueryModelTransformMatrix)
    DISPATCH_STUB(NGpiSetModelTransformMatrix)
    DISPATCH_STUB(NGpiCallSegmentMatrix)
    DISPATCH_STUB(NGpiQueryDefaultViewMatrix)
    DISPATCH_STUB(NGpiSetDefaultViewMatrix)
    DISPATCH_STUB(NGpiQueryPageViewport)
    DISPATCH_STUB(NGpiSetPageViewport)
    DISPATCH_STUB(NGpiQueryViewingTransformMatrix)
    DISPATCH_STUB(NGpiSetViewingTransformMatrix)
    DISPATCH_STUB(NGpiTranslate)
    DISPATCH_STUB(NGpiScale)
    DISPATCH_STUB(NGpiRotate)
    DISPATCH_STUB(NGpiSetGraphicsField)
    DISPATCH_STUB(NGpiQueryGraphicsField)
    DISPATCH_STUB(NGpiSetViewingLimits)
    DISPATCH_STUB(NGpiQueryViewingLimits)
    DISPATCH_STUB(NGpiBeginPath)
    DISPATCH_STUB(NGpiEndPath)
    DISPATCH_STUB(NGpiCloseFigure)
    DISPATCH_STUB(NGpiModifyPath)
    DISPATCH_STUB(NGpiFillPath)
    DISPATCH_STUB(NGpiSetClipPath)
    DISPATCH_STUB(NGpiOutlinePath)
    DISPATCH_STUB(NGpiPathToRegion)
    DISPATCH_STUB(NGpiStrokePath)
    DISPATCH_STUB(NGpiCreateLogColorTable)
    DISPATCH_STUB(NGpiQueryColorData)
    DISPATCH_STUB(NGpiQueryLogColorTable)
    DISPATCH_STUB(NGpiQueryRealColors)
    DISPATCH_STUB(NGpiQueryNearestColor)
    DISPATCH_STUB(NGpiQueryColorIndex)
    DISPATCH_STUB(NGpiQueryRGBColor)
    DISPATCH_STUB(NGpiCreatePalette)
    DISPATCH_STUB(NGpiDeletePalette)
    DISPATCH_STUB(NGpiSelectPalette)
    DISPATCH_STUB(NGpiAnimatePalette)
    DISPATCH_STUB(NGpiSetPaletteEntries)
    DISPATCH_STUB(NGpiQueryPalette)
    DISPATCH_STUB(NGpiQueryPaletteInfo)
    DISPATCH_STUB(NGpiResizePalette)
    DISPATCH_STUB(NGpiQueryNearestPaletteIndex)
    DISPATCH_STUB(NGpiQueryColor)
    DISPATCH_STUB(NGpiPolyLineDisjoint)
    DISPATCH_STUB(NGpiSetPattern)
    DISPATCH_STUB(NGpiQueryPattern)
    DISPATCH_STUB(NGpiBeginArea)
    DISPATCH_STUB(NGpiEndArea)
    DISPATCH_STUB(NGpiSetAttrMode)
    DISPATCH_STUB(NGpiQueryAttrMode)
    DISPATCH_STUB(NGpiQueryAttrs)
    DISPATCH_STUB(NGpiSetBackColor)
    DISPATCH_STUB(NGpiQueryBackColor)
    DISPATCH_STUB(NGpiSetMix)
    DISPATCH_STUB(NGpiQueryMix)
    DISPATCH_STUB(NGpiSetBackMix)
    DISPATCH_STUB(NGpiQueryBackMix)
    DISPATCH_STUB(NGpiSetLineType)
    DISPATCH_STUB(NGpiQueryLineType)
    DISPATCH_STUB(NGpiSetLineWidth)
    DISPATCH_STUB(NGpiQueryLineWidth)
    DISPATCH_STUB(NGpiSetLineWidthGeom)
    DISPATCH_STUB(NGpiQueryLineWidthGeom)
    DISPATCH_STUB(NGpiSetLineEnd)
    DISPATCH_STUB(NGpiQueryLineEnd)
    DISPATCH_STUB(NGpiSetLineJoin)
    DISPATCH_STUB(NGpiQueryLineJoin)
    DISPATCH_STUB(NGpiSetCurrentPosition)
    DISPATCH_STUB(NGpiQueryCurrentPosition)
    DISPATCH_STUB(NGpiSetArcParams)
    DISPATCH_STUB(NGpiQueryArcParams)
    DISPATCH_STUB(NGpiPointArc)
    DISPATCH_STUB(NGpiFullArc)
    DISPATCH_STUB(NGpiPartialArc)
    DISPATCH_STUB(NGpiOval)
    DISPATCH_STUB(NGpiPolyFillet)
    DISPATCH_STUB(NGpiPolySpline)
    DISPATCH_STUB(NGpiPolyFilletSharp)
    DISPATCH_STUB(NGpiSetPatternSet)
    DISPATCH_STUB(NGpiQueryPatternSet)
    DISPATCH_STUB(NGpiSetPatternRefPoint)
    DISPATCH_STUB(NGpiQueryPatternRefPoint)
    DISPATCH_STUB(NGpiQueryCharStringPos)
    DISPATCH_STUB(NGpiQueryCharStringPosAt)
    DISPATCH_STUB(NGpiQueryTabbedTextExtent)
    DISPATCH_STUB(NGpiQueryTextBox)
    DISPATCH_STUB(NGpiQueryDefCharBox)
    DISPATCH_STUB(NGpiSetCharSet)
    DISPATCH_STUB(NGpiQueryCharSet)
    DISPATCH_STUB(NGpiSetCharBox)
    DISPATCH_STUB(NGpiQueryCharBox)
    DISPATCH_STUB(NGpiSetCharAngle)
    DISPATCH_STUB(NGpiQueryCharAngle)
    DISPATCH_STUB(NGpiSetCharShear)
    DISPATCH_STUB(NGpiQueryCharShear)
    DISPATCH_STUB(NGpiSetCharDirection)
    DISPATCH_STUB(NGpiQueryCharDirection)
    DISPATCH_STUB(NGpiSetCharMode)
    DISPATCH_STUB(NGpiQueryCharMode)
    DISPATCH_STUB(NGpiSetTextAlignment)
    DISPATCH_STUB(NGpiQueryTextAlignment)
    DISPATCH_STUB(NGpiCharStringPos)
    DISPATCH_STUB(NGpiTabbedCharStringAt)
    DISPATCH_STUB(NGpiCharStringPosAt)
    DISPATCH_STUB(NGpiSetCharExtra)
    DISPATCH_STUB(NGpiSetCharBreakExtra)
    DISPATCH_STUB(NGpiQueryCharExtra)
    DISPATCH_STUB(NGpiQueryCharBreakExtra)
    DISPATCH_STUB(NGpiSetMarker)
    DISPATCH_STUB(NGpiSetMarkerBox)
    DISPATCH_STUB(NGpiSetMarkerSet)
    DISPATCH_STUB(NGpiQueryMarker)
    DISPATCH_STUB(NGpiQueryMarkerBox)
    DISPATCH_STUB(NGpiQueryMarkerSet)
    DISPATCH_STUB(NGpiImage)
    DISPATCH_STUB(NGpiPop)
    DISPATCH_STUB(NGpiPtVisible)
    DISPATCH_STUB(NGpiRectVisible)
    DISPATCH_STUB(NGpiComment)
    DISPATCH_STUB(NGpiQueryRasterizerCaps)
    DISPATCH_STUB(NGpiCreateLogFont)
    DISPATCH_STUB(NGpiDeleteSetId)
    DISPATCH_STUB(NGpiLoadFonts)
    DISPATCH_STUB(NGpiUnloadFonts)
    DISPATCH_STUB(NGpiQueryFonts)
    DISPATCH_STUB(NGpiQueryFontMetrics)
    DISPATCH_STUB(NGpiQueryKerningPairs)
    DISPATCH_STUB(NGpiQueryWidthTable)
    DISPATCH_STUB(NGpiQueryNumberSetIds)
    DISPATCH_STUB(NGpiQuerySetIds)
    DISPATCH_STUB(NGpiQueryFaceString)
    DISPATCH_STUB(NGpiQueryLogicalFont)
    DISPATCH_STUB(NGpiQueryFontAction)
    DISPATCH_STUB(NGpiLoadPublicFonts)
    DISPATCH_STUB(NGpiUnloadPublicFonts)
    DISPATCH_STUB(NGpiSetCp)
    DISPATCH_STUB(NGpiQueryCp)
    DISPATCH_STUB(NGpiQueryFullFontFileDescs)
    DISPATCH_STUB(NGpiSetFontMappingFlags)
    DISPATCH_STUB(NGpiQueryFontMappingFlags)
    DISPATCH_STUB(NGpiQueryFontFileDescriptions)
    DISPATCH_STUB(NGpiBitBlt)
    DISPATCH_STUB(NGpiDeleteBitmap)
    DISPATCH_STUB(NGpiLoadBitmap)
    DISPATCH_STUB(NGpiSetBitmap)
    DISPATCH_STUB(NGpiWCBitBlt)
    DISPATCH_STUB(NGpiCreateBitmap)
    DISPATCH_STUB(NGpiSetBitmapBits)
    DISPATCH_STUB(NGpiSetBitmapDimension)
    DISPATCH_STUB(NGpiSetBitmapId)
    DISPATCH_STUB(NGpiQueryBitmapBits)
    DISPATCH_STUB(NGpiQueryBitmapDimension)
    DISPATCH_STUB(NGpiQueryBitmapHandle)
    DISPATCH_STUB(NGpiQueryBitmapParameters)
    DISPATCH_STUB(NGpiQueryBitmapInfoHeader)
    DISPATCH_STUB(NGpiQueryDeviceBitmapFormats)
    DISPATCH_STUB(NGpiSetPel)
    DISPATCH_STUB(NGpiQueryPel)
    DISPATCH_STUB(NGpiFloodFill)
    DISPATCH_STUB(NGpiDrawBits)
    DISPATCH_STUB(NGpiCombineRegion)
    DISPATCH_STUB(NGpiCreateRegion)
    DISPATCH_STUB(NGpiDestroyRegion)
    DISPATCH_STUB(NGpiEqualRegion)
    DISPATCH_STUB(NGpiOffsetRegion)
    DISPATCH_STUB(NGpiPaintRegion)
    DISPATCH_STUB(NGpiFrameRegion)
    DISPATCH_STUB(NGpiPtInRegion)
    DISPATCH_STUB(NGpiQueryRegionBox)
    DISPATCH_STUB(NGpiQueryRegionRects)
    DISPATCH_STUB(NGpiRectInRegion)
    DISPATCH_STUB(NGpiSetRegion)
    DISPATCH_STUB(NGpiSetClipRegion)
    DISPATCH_STUB(NGpiQueryClipRegion)
    DISPATCH_STUB(NGpiQueryClipBox)
    DISPATCH_STUB(NGpiExcludeClipRectangle)
    DISPATCH_STUB(NGpiIntersectClipRectangle)
    DISPATCH_STUB(NGpiOffsetClipRegion)
    DISPATCH_STUB(NGpiCreateRoundRectRegion)
    DISPATCH_STUB(NGpiCreateEllipticRegion)
    DISPATCH_STUB(NGpiResumePlay)
    DISPATCH_STUB(NGpiSuspendPlay)
    DISPATCH_STUB(NGpiCopyMetaFile)
    DISPATCH_STUB(NGpiDeleteMetaFile)
    DISPATCH_STUB(NGpiLoadMetaFile)
    DISPATCH_STUB(NGpiPlayMetaFile)
    DISPATCH_STUB(NGpiQueryMetaFileBits)
    DISPATCH_STUB(NGpiQueryMetaFileLength)
    DISPATCH_STUB(NGpiSaveMetaFile)
    DISPATCH_STUB(NGpiSetMetaFileBits)
    DISPATCH_STUB(NGpiQueryDefArcParams)
    DISPATCH_STUB(NGpiQueryDefAttrs)
    DISPATCH_STUB(NGpiQueryDefTag)
    DISPATCH_STUB(NGpiQueryDefViewingLimits)
    DISPATCH_STUB(NGpiSetDefArcParams)
    DISPATCH_STUB(NGpiSetDefAttrs)
    DISPATCH_STUB(NGpiSetDefTag)
    DISPATCH_STUB(NGpiSetDefViewingLimits)
    DISPATCH_STUB(NGpiPolygons)
    DISPATCH_STUB(NGpiCreatePolygonRegion)
    DISPATCH_STUB(NGpiBeginInkPath)
    DISPATCH_STUB(NGpiEndInkPath)
    DISPATCH_STUB(NGpiStrokeInkPath)
    DISPATCH_STUB(NGpiSetDCOwner)
    DISPATCH_STUB(NGpiSetBitmapOwner)
    DISPATCH_STUB(NGpiSetRegionOwner)
    DISPATCH_STUB(NGpiInstallIFIFont)
    DISPATCH_STUB(NGpiOpenMetafile)
    DISPATCH_STUB(NGpiCloseMetafile)
    DISPATCH_STUB(NGpiConvPSH)
    DISPATCH_STUB(NGpiMtAssociate)
    DISPATCH_STUB(NGpiMtDisassociate)
    DISPATCH_STUB(NGpiEnableYInversion)
    DISPATCH_STUB(NGpiQueryYInversion)
    DISPATCH_STUB(NGpiSaveMetaFile2)
    DISPATCH_STUB(NGpiAllocateDCData)
    DISPATCH_STUB(NGpiQueryDCData)

    default:
        log("[PMSERVER] Dispatch OUT: Unknown FnCode=0x%x -> 0x%x\n",
            ulFnCode, (unsigned int)GPI_ERROR);
        return (ULONG)GPI_ERROR;
    }
}

#undef DISPATCH_STUB
