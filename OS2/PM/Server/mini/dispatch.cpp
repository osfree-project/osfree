/*!
 * @file dispatch.cpp
 * @brief Server-side GPI dispatcher with GOCA parser integration.
 */
#define GPI_SERVER
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "gpi_ps.h"          /* pulls in goca_parser.hpp + goca_common.h */
#include "goca_emit.h"       /* GocaEmit* order emitters */
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
 * @brief Server-side storage for the shared PS table.
 *
 * The GpiPs* helpers live as static inline in gpi_ps.h; only the table
 * itself is allocated here, one instance for the whole server.
 */
GPI_PS g_psTable[GPI_MAX_PS];

/* ======================================================================== */
/*  Order emitters                                                          */
/* ======================================================================== */

/*!
 * @brief Feed a single order to the GOCA parser.
 * @param [in] ps       Target PS entry.
 * @param [in] pOrder   Order bytes.
 * @param [in] cbOrder  Order length.
 */
static void SrvParseOrder(PGPI_PS ps, const BYTE *pOrder, ULONG cbOrder)
{
    ps->goca.pbStream = (PBYTE)pOrder;
    ps->goca.cbStream = cbOrder;
    GocaParseStream(&ps->goca);
}

/*!
 * @brief Emit attribute orders for GpiSetAttrs.
 *
 * Composite: one GPI call may emit multiple GOCA orders.
 * Individual orders are produced by the shared GocaEmit* emitters.
 */
static void SrvEmitSetAttrs(PGPI_PS ps, LONG lPrimType, ULONG flAttrMask,
                            PBUNDLE pb)
{
    BYTE  buf[32];
    ULONG pos;
    LONG  len;

    if (lPrimType == PRIM_LINE) {
        PLINEBUNDLE lb = (PLINEBUNDLE)pb;
        if (flAttrMask & LBB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), lb->lColor);
            if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);
        }
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
        if (flAttrMask & CBB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), cb->lColor);
            if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);
        }
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
        if (flAttrMask & MBB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), mb->lColor);
            if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);
        }
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
        if (flAttrMask & ABB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), ab->lColor);
            if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);
        }
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
 * @retval p          NGpiCreatePS: pointer to the new GPI_PS entry.
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
        PGPI_PS ps;

        if (!a || cbArgs < sizeof(*a)) {
            log("[PMSERVER] IN  : NGpiCreatePS (bad args, cbArgs=%u)\n",
                (unsigned)cbArgs);
            return (ULONG)GPI_ERROR;
        }

        ps = GpiPsCreate(NULL, a->flOptions);
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
        PGPI_PS ps = GpiPsFind(hps);

        log("[PMSERVER] IN  : NGpiSetPS(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!ps) {
            log("[PMSERVER] OUT : NGpiSetPS -> GPI_ERROR (no ps)\n");
            return (ULONG)GPI_ERROR;
        }
        if (a && cbArgs >= sizeof(*a)) {
            GpiPsSetOptions(hps, a->flOptions);
        }
        log("[PMSERVER] OUT : NGpiSetPS -> 1\n");
        return 1;
    }

    case NGpiDestroyPS: {
        PGPI_PS ps = GpiPsFind(hps);
        log("[PMSERVER] IN  : NGpiDestroyPS(hps=%p)\n", (void*)hps);
        if (ps) GpiPsDestroy(hps);
        log("[PMSERVER] OUT : NGpiDestroyPS -> %d\n",
            ps ? 1 : (int)GPI_ERROR);
        return (ULONG)(ps ? 1 : GPI_ERROR);
    }

    /* ==================================================================== */
    /*  Drawing primitives (emit GOCA orders)                               */
    /* ==================================================================== */

    case NGpiSetColor: {
        LONG lColor;
        BYTE buf[4];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetColor(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) {
            log("[PMSERVER] OUT : NGpiSetColor -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&lColor, pArgs, sizeof(LONG));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), lColor);
        if (len < 0) {
            log("[PMSERVER] OUT : NGpiSetColor -> GPI_ERROR (unsupported color 0x%lx)\n",
                (unsigned long)lColor);
            return (ULONG)GPI_ERROR;
        }
        SrvParseOrder(ps, buf, (ULONG)len);
        log("[PMSERVER] OUT : NGpiSetColor -> 1\n");
        return 1;
    }

    case NGpiMove: {
        POINTL ptl;
        BYTE buf[10];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiMove(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiMove -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitMove(&ps->goca, buf, sizeof(buf), ptl);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvParseOrder(ps, buf, (ULONG)len);
        log("[PMSERVER] OUT : NGpiMove -> 1\n");
        return 1;
    }

    case NGpiLine: {
        POINTL ptl;
        BYTE buf[10];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiLine(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiLine -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitLine(&ps->goca, buf, sizeof(buf), ptl);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvParseOrder(ps, buf, (ULONG)len);
        log("[PMSERVER] OUT : NGpiLine -> 1\n");
        return 1;
    }

    case NGpiBox: {
        PGpiBox_Args a = (PGpiBox_Args)pArgs;
        BYTE buf[22];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiBox(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < sizeof(*a)) {
            log("[PMSERVER] OUT : NGpiBox -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitBox(&ps->goca, buf, sizeof(buf),
                          a->lControl, a->ptl, a->lHRound, a->lVRound);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvParseOrder(ps, buf, (ULONG)len);
        log("[PMSERVER] OUT : NGpiBox -> 1\n");
        return 1;
    }

    case NGpiPolyLine: {
        LONG lCount;
        PPOINTL aptl;
        LONG len;
        PGPI_PS ps;
        BYTE  stackBuf[64];
        PBYTE buf = stackBuf;
        PVOID heap = NULL;

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
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitPolyLine(&ps->goca, NULL, 0, lCount, aptl);
        if (len < 0) return (ULONG)GPI_ERROR;
        if ((ULONG)len > sizeof(stackBuf)) {
            if (DosAllocMem(&heap, (ULONG)len,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
                return (ULONG)GPI_ERROR;
            buf = (PBYTE)heap;
        }
        len = GocaEmitPolyLine(&ps->goca, buf, (ULONG)len, lCount, aptl);
        if (len < 0) {
            if (heap) DosFreeMem(heap);
            return (ULONG)GPI_ERROR;
        }
        SrvParseOrder(ps, buf, (ULONG)len);
        if (heap) DosFreeMem(heap);
        log("[PMSERVER] OUT : NGpiPolyLine -> 1\n");
        return 1;
    }

    case NGpiCharString: {
        LONG lCount;
        char *pchString;
        LONG len;
        PGPI_PS ps;
        BYTE  stackBuf[64];
        PBYTE buf = stackBuf;
        PVOID heap = NULL;

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
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitCharString(&ps->goca, NULL, 0, lCount, pchString);
        if (len < 0) return (ULONG)GPI_ERROR;
        if ((ULONG)len > sizeof(stackBuf)) {
            if (DosAllocMem(&heap, (ULONG)len,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
                return (ULONG)GPI_ERROR;
            buf = (PBYTE)heap;
        }
        len = GocaEmitCharString(&ps->goca, buf, (ULONG)len, lCount, pchString);
        if (len < 0) {
            if (heap) DosFreeMem(heap);
            return (ULONG)GPI_ERROR;
        }
        SrvParseOrder(ps, buf, (ULONG)len);
        if (heap) DosFreeMem(heap);
        log("[PMSERVER] OUT : NGpiCharString -> 1\n");
        return 1;
    }

    case NGpiCharStringAt: {
        POINTL ptl;
        LONG lCount;
        char *pchString;
        LONG len;
        PGPI_PS ps;
        BYTE  stackBuf[64];
        PBYTE buf = stackBuf;
        PVOID heap = NULL;

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
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitCharStringAt(&ps->goca, NULL, 0, ptl, lCount, pchString);
        if (len < 0) return (ULONG)GPI_ERROR;
        if ((ULONG)len > sizeof(stackBuf)) {
            if (DosAllocMem(&heap, (ULONG)len,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
                return (ULONG)GPI_ERROR;
            buf = (PBYTE)heap;
        }
        len = GocaEmitCharStringAt(&ps->goca, buf, (ULONG)len, ptl, lCount, pchString);
        if (len < 0) {
            if (heap) DosFreeMem(heap);
            return (ULONG)GPI_ERROR;
        }
        SrvParseOrder(ps, buf, (ULONG)len);
        if (heap) DosFreeMem(heap);
        log("[PMSERVER] OUT : NGpiCharStringAt -> 1\n");
        return 1;
    }

    case NGpiMarker: {
        POINTL ptl;
        BYTE buf[10];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiMarker(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) {
            log("[PMSERVER] OUT : NGpiMarker -> GPI_ERROR (bad args)\n");
            return (ULONG)GPI_ERROR;
        }
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitMarker(&ps->goca, buf, sizeof(buf), ptl);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvParseOrder(ps, buf, (ULONG)len);
        log("[PMSERVER] OUT : NGpiMarker -> 1\n");
        return 1;
    }

    case NGpiPolyMarker: {
        LONG lCount;
        PPOINTL aptl;
        LONG len;
        PGPI_PS ps;
        BYTE  stackBuf[64];
        PBYTE buf = stackBuf;
        PVOID heap = NULL;

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
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        len = GocaEmitPolyMarker(&ps->goca, NULL, 0, lCount, aptl);
        if (len < 0) return (ULONG)GPI_ERROR;
        if ((ULONG)len > sizeof(stackBuf)) {
            if (DosAllocMem(&heap, (ULONG)len,
                            PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
                return (ULONG)GPI_ERROR;
            buf = (PBYTE)heap;
        }
        len = GocaEmitPolyMarker(&ps->goca, buf, (ULONG)len, lCount, aptl);
        if (len < 0) {
            if (heap) DosFreeMem(heap);
            return (ULONG)GPI_ERROR;
        }
        SrvParseOrder(ps, buf, (ULONG)len);
        if (heap) DosFreeMem(heap);
        log("[PMSERVER] OUT : NGpiPolyMarker -> 1\n");
        return 1;
    }

    case NGpiSetAttrs: {
        LONG lPrimType;
        ULONG flAttrMask;
        ULONG flDefMask;
        PBUNDLE pb;
        PGPI_PS ps;

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
        ps = GpiPsGet(hps);
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
