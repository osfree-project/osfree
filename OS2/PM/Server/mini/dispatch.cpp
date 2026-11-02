/*!
 * @file dispatch.c
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

/*!
 * @brief Bit mask for DCTL_* in ps->ulDrawControl.
 */
#define DCTL_BIT(c)  (1UL << ((c) - 1))

/* ======================================================================== */
/*  Order routing                                                           */
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
 * @brief Fill FLAG1 / FLAG2 of a GBSEG header from ulInitialSegAttrs.
 *
 * FLAG1 (SC31-6804, p. 102): INVISIBLE, PROPINV, PICK, PROPICK,
 * HILITE, PROPHI, DYNAMIC, FASTCH.
 * FLAG2 (AFPC-0008-03, p. 75): CHAIN (bit 0), APP_NEW (bits 5-6 = 00).
 *
 * Inverted attributes: ATTR_VISIBLE = !FLAG1.INVISIBLE,
 * ATTR_PROP_VISIBLE = !FLAG1.PROPINV, ATTR_CHAINED = !FLAG2.CHAIN.
 */
static void SrvBuildGbsegFlags(ULONG ulAttrs, UCHAR *pFlag1, UCHAR *pFlag2)
{
    UCHAR f1 = 0, f2 = 0;

    if (!(ulAttrs & SEGATTR_BIT(ATTR_VISIBLE)))         f1 |= GBSEG_INVISIBLE;
    if (!(ulAttrs & SEGATTR_BIT(ATTR_PROP_VISIBLE)))    f1 |= GBSEG_PROPINV;
    if ( (ulAttrs & SEGATTR_BIT(ATTR_DETECTABLE)))      f1 |= GBSEG_PICK;
    if ( (ulAttrs & SEGATTR_BIT(ATTR_PROP_DETECTABLE))) f1 |= GBSEG_PROPICK;
    if ( (ulAttrs & SEGATTR_BIT(ATTR_DYNAMIC)))         f1 |= GBSEG_DYNAMIC;
    if ( (ulAttrs & SEGATTR_BIT(ATTR_FASTCHAIN)))       f1 |= GBSEG_FASTCH;

    if (!(ulAttrs & SEGATTR_BIT(ATTR_CHAINED)))         f2 |= GBSEG_CHAIN;

    *pFlag1 = f1;
    *pFlag2 = f2;
}

/*!
 * @brief Route a GOCA order: record into pbGoca (DM_RETAIN) and/or
 *        dispatch through the parser (DM_DRAW).
 *
 * If no segment is open, the order is drawn immediately regardless of
 * ulDrawingMode (per PM GPI: DM_RETAIN without an open segment is an
 * error, but there is nothing to retain into — we just draw).
 *
 * An unchained open segment forces DM_RETAIN even when the caller set
 * DM_DRAW / DM_DRAWANDRETAIN.
 */
static BOOL SrvDeliver(PGPI_PS ps, const BYTE *p, ULONG cb)
{
    ULONG am;

    if (!ps || !p || cb == 0) return FALSE;

    if (ps->lOpenSeg == 0) {
        SrvParseOrder(ps, p, cb);
        return TRUE;
    }

    am = ps->ulDrawingMode;
    if (!(ps->ulInitialSegAttrs & SEGATTR_BIT(ATTR_CHAINED)))
        am = DM_RETAIN;

    if (PS_IS_RECORDING(am)) {
        if (!GpiPsGocaAppend(ps, p, cb)) return FALSE;
    }
    if (PS_IS_DRAWING(am)) {
        SrvParseOrder(ps, p, cb);
    }
    return TRUE;
}

/* ======================================================================== */
/*  Order emitters                                                          */
/* ======================================================================== */

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
            if (len >= 0) SrvDeliver(ps, buf, (ULONG)len);
        }
        if (flAttrMask & LBB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)lb->usMixMode;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & LBB_TYPE) {
            buf[0] = OCODE_GSLT; buf[1] = 1; buf[2] = (BYTE)lb->usType;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & LBB_WIDTH) {
            buf[0] = OCODE_GSLW; buf[1] = 1; buf[2] = (BYTE)lb->lGeomWidth;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & LBB_END) {
            buf[0] = OCODE_GSLE; buf[1] = 1; buf[2] = (BYTE)lb->usEnd;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & LBB_JOIN) {
            buf[0] = OCODE_GSLJ; buf[1] = 1; buf[2] = (BYTE)lb->usJoin;
            SrvDeliver(ps, buf, 3);
        }
    } else if (lPrimType == PRIM_CHAR) {
        PCHARBUNDLE cb = (PCHARBUNDLE)pb;
        if (flAttrMask & CBB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), cb->lColor);
            if (len >= 0) SrvDeliver(ps, buf, (ULONG)len);
        }
        if (flAttrMask & CBB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)cb->usMixMode;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & CBB_SET) {
            buf[0] = OCODE_GSCS; buf[1] = 1; buf[2] = (BYTE)cb->usSet;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & CBB_DIRECTION) {
            buf[0] = OCODE_GSCD; buf[1] = 1; buf[2] = (BYTE)cb->usDirection;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & CBB_MODE) {
            buf[0] = OCODE_GSCR; buf[1] = 1; buf[2] = (BYTE)cb->usPrecision;
            SrvDeliver(ps, buf, 3);
        }
    } else if (lPrimType == PRIM_MARKER) {
        PMARKERBUNDLE mb = (PMARKERBUNDLE)pb;
        if (flAttrMask & MBB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), mb->lColor);
            if (len >= 0) SrvDeliver(ps, buf, (ULONG)len);
        }
        if (flAttrMask & MBB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)mb->usMixMode;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & MBB_SET) {
            buf[0] = OCODE_GSMS; buf[1] = 1; buf[2] = (BYTE)mb->usSet;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & MBB_SYMBOL) {
            buf[0] = OCODE_GSMT; buf[1] = 1; buf[2] = (BYTE)mb->usSymbol;
            SrvDeliver(ps, buf, 3);
        }
    } else if (lPrimType == PRIM_AREA) {
        PAREABUNDLE ab = (PAREABUNDLE)pb;
        if (flAttrMask & ABB_COLOR) {
            len = GocaEmitSetColor(&ps->goca, buf, sizeof(buf), ab->lColor);
            if (len >= 0) SrvDeliver(ps, buf, (ULONG)len);
        }
        if (flAttrMask & ABB_MIX_MODE) {
            buf[0] = OCODE_GSMX; buf[1] = 1; buf[2] = (BYTE)ab->usMixMode;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & ABB_SET) {
            buf[0] = OCODE_GSPS; buf[1] = 1; buf[2] = (BYTE)ab->usSet;
            SrvDeliver(ps, buf, 3);
        }
        if (flAttrMask & ABB_SYMBOL) {
            buf[0] = OCODE_GSPT; buf[1] = 1; buf[2] = (BYTE)ab->usSymbol;
            SrvDeliver(ps, buf, 3);
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
            SrvDeliver(ps, buf, pos);
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
 */
ULONG DispatchGpi(HPS hps, ULONG ulFnCode, PBYTE pArgs, ULONG cbArgs,
                  PULONG pcbPayload)
{
    if (pcbPayload) *pcbPayload = 0;   /* default: no payload */
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
    /*  Drawing state (no GOCA)                                             */
    /* ==================================================================== */

    case NGpiSetDrawingMode: {
        LONG lMode;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetDrawingMode(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lMode = *(LONG*)pArgs;
        if (lMode != DM_DRAW && lMode != DM_RETAIN && lMode != DM_DRAWANDRETAIN)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        ps->ulDrawingMode = (ULONG)lMode;
        log("[PMSERVER] OUT : NGpiSetDrawingMode -> 1\n");
        return 1;
    }

    case NGpiQueryDrawingMode: {
        PGPI_PS ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        log("[PMSERVER] OUT : NGpiQueryDrawingMode -> %lu\n",
            (unsigned long)ps->ulDrawingMode);
        return (ULONG)ps->ulDrawingMode;
    }

    case NGpiSetDrawControl: {
        LONG lControl, lValue;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetDrawControl(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lControl, pArgs, sizeof(LONG));
        memcpy(&lValue,   pArgs + sizeof(LONG), sizeof(LONG));
        if (lControl < DCTL_ERASE || lControl > DCTL_CORRELATE)
            return (ULONG)GPI_ERROR;
        if (lValue != DCTL_ON && lValue != DCTL_OFF)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (lValue == DCTL_ON)
            ps->ulDrawControl |=  DCTL_BIT(lControl);
        else
            ps->ulDrawControl &= ~DCTL_BIT(lControl);
        log("[PMSERVER] OUT : NGpiSetDrawControl -> 1\n");
        return 1;
    }

    case NGpiQueryDrawControl: {
        LONG lControl;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiQueryDrawControl(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lControl = *(LONG*)pArgs;
        if (lControl < DCTL_ERASE || lControl > DCTL_CORRELATE)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        return (ps->ulDrawControl & DCTL_BIT(lControl)) ? DCTL_ON : DCTL_OFF;
    }

    case NGpiSetStopDraw: {
        LONG lValue;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetStopDraw(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lValue = *(LONG*)pArgs;
        if (lValue != SDW_OFF && lValue != SDW_ON) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        ps->ulStopDraw = (ULONG)lValue;
        return 1;
    }

    case NGpiQueryStopDraw: {
        PGPI_PS ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        return (ULONG)ps->ulStopDraw;
    }

    case NGpiSetEditMode: {
        LONG lMode;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetEditMode(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lMode = *(LONG*)pArgs;
        if (lMode != SEGEM_INSERT && lMode != SEGEM_REPLACE)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        ps->ulEditMode = (ULONG)lMode;
        return 1;
    }

    case NGpiQueryEditMode: {
        PGPI_PS ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        return (ULONG)ps->ulEditMode;
    }

    case NGpiSetInitialSegmentAttrs: {
        LONG lAttribute, lValue;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetInitialSegmentAttrs(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lAttribute, pArgs, sizeof(LONG));
        memcpy(&lValue,     pArgs + sizeof(LONG), sizeof(LONG));
        if (lValue != ATTR_ON && lValue != ATTR_OFF)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        switch (lAttribute) {
        case ATTR_DETECTABLE:
        case ATTR_VISIBLE:
        case ATTR_CHAINED:
        case ATTR_DYNAMIC:
        case ATTR_FASTCHAIN:
        case ATTR_PROP_DETECTABLE:
        case ATTR_PROP_VISIBLE:
            if (lValue == ATTR_ON)
                ps->ulInitialSegAttrs |=  SEGATTR_BIT(lAttribute);
            else
                ps->ulInitialSegAttrs &= ~SEGATTR_BIT(lAttribute);
            return 1;
        default:
            return (ULONG)GPI_ERROR;
        }
    }

    case NGpiQueryInitialSegmentAttrs: {
        LONG lAttribute;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiQueryInitialSegmentAttrs(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lAttribute = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        switch (lAttribute) {
        case ATTR_DETECTABLE:
        case ATTR_VISIBLE:
        case ATTR_CHAINED:
        case ATTR_DYNAMIC:
        case ATTR_FASTCHAIN:
        case ATTR_PROP_DETECTABLE:
        case ATTR_PROP_VISIBLE:
            return (ps->ulInitialSegAttrs & SEGATTR_BIT(lAttribute))
                   ? ATTR_ON : ATTR_OFF;
        default:
            return (ULONG)GPI_ERROR;
        }
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
        log("[PMSERVER] OUT : NGpiMove -> 1\n");
        return 1;
    }

    case NGpiSetCurrentPosition: {
        POINTL ptl;
        BYTE buf[10];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetCurrentPosition(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(POINTL)) return (ULONG)GPI_ERROR;
        memcpy(&ptl, pArgs, sizeof(POINTL));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        len = GocaEmitMove(&ps->goca, buf, sizeof(buf), ptl);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
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
        SrvDeliver(ps, buf, (ULONG)len);
        if (heap) DosFreeMem(heap);
        log("[PMSERVER] OUT : NGpiPolyMarker -> 1\n");
        return 1;
    }

    case NGpiSetArcParams: {
        ARCPARAMS ap;
        BYTE buf[10];
        LONG len;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiSetArcParams(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(ARCPARAMS)) return (ULONG)GPI_ERROR;
        memcpy(&ap, pArgs, sizeof(ARCPARAMS));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        len = GocaEmitSetArcParams(&ps->goca, buf, sizeof(buf), &ap);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
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
    /*  Segments                                                            */
    /* ==================================================================== */

    case NGpiOpenSegment: {
        LONG lSegId;
        PGPI_PS ps;
        BYTE gbseg[14];
        UCHAR f1, f2;
        ULONG start, end;

        log("[PMSERVER] IN  : NGpiOpenSegment(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lSegId = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (lSegId <= 0) {
            log("[PMSERVER] OUT : NGpiOpenSegment -> GPI_ERROR (bad id)\n");
            return (ULONG)GPI_ERROR;
        }
        if (ps->lOpenSeg != 0) {
            log("[PMSERVER] OUT : NGpiOpenSegment -> GPI_ERROR (already in seg %ld)\n",
                (long)ps->lOpenSeg);
            return (ULONG)GPI_ERROR;
        }
        if (ps->ulDrawingMode == DM_DRAW) {
            log("[PMSERVER] OUT : NGpiOpenSegment -> GPI_ERROR (in DM_DRAW)\n");
            return (ULONG)GPI_ERROR;
        }
        if (GpiPsGocaFindSeg(ps, lSegId, &start, &end)) {
            log("[PMSERVER] OUT : NGpiOpenSegment -> GPI_ERROR (dup seg %ld)\n",
                (long)lSegId);
            return (ULONG)GPI_ERROR;
        }

        SrvBuildGbsegFlags(ps->ulInitialSegAttrs, &f1, &f2);

        gbseg[0] = OCODE_GBSEG;
        gbseg[1] = GBSEG_LENGTH;
        GocaPutU32(&ps->goca, gbseg + 2, (ULONG)lSegId);
        gbseg[6] = f1;
        gbseg[7] = f2;
        GocaPutU16(&ps->goca, gbseg + 8, 0);   /* SEGL */
        GocaPutU32(&ps->goca, gbseg + 10, 0);  /* P/SNAME */

        if (!GpiPsGocaAppend(ps, gbseg, sizeof(gbseg)))
            return (ULONG)GPI_ERROR;
        ps->uOpenSegStart = ps->cbGoca - sizeof(gbseg);
        ps->lOpenSeg      = lSegId;

        log("[PMSERVER] OUT : NGpiOpenSegment -> 1 (off=%lu)\n",
            (unsigned long)ps->uOpenSegStart);
        return 1;
    }

    case NGpiCloseSegment: {
        PGPI_PS ps = GpiPsGet(hps);

        log("[PMSERVER] IN  : NGpiCloseSegment(hps=%p)\n", (void*)hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0) {
            log("[PMSERVER] OUT : NGpiCloseSegment -> GPI_ERROR (not in seg)\n");
            return (ULONG)GPI_ERROR;
        }
        ps->lOpenSeg      = 0;
        ps->uOpenSegStart = 0;
        log("[PMSERVER] OUT : NGpiCloseSegment -> 1\n");
        return 1;
    }

    case NGpiDrawSegment: {
        LONG lSegId;
        PGPI_PS ps;
        ULONG start, end;

        log("[PMSERVER] IN  : NGpiDrawSegment(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lSegId = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (!GpiPsGocaFindSeg(ps, lSegId, &start, &end)) {
            log("[PMSERVER] OUT : NGpiDrawSegment -> GPI_ERROR (seg %ld not found)\n",
                (long)lSegId);
            return (ULONG)GPI_ERROR;
        }

        ps->goca.pbStream = ps->pbGoca + start;
        ps->goca.cbStream = end - start;
        GocaParseStream(&ps->goca);   /* GBSEG is a no-op in the parser */

        log("[PMSERVER] OUT : NGpiDrawSegment -> 1 (off=%lu..%lu)\n",
            (unsigned long)start, (unsigned long)end);
        return 1;
    }


    /* ==================================================================== */
    /*  Segment editing                                                     */
    /* ==================================================================== */

    case NGpiDeleteSegment: {
        LONG lSegId;
        PGPI_PS ps;
        ULONG start, end;

        log("[PMSERVER] IN  : NGpiDeleteSegment(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lSegId = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (ps->lOpenSeg == lSegId) {
            log("[PMSERVER] OUT : NGpiDeleteSegment -> GPI_ERROR (seg %ld is open)\n",
                (long)lSegId);
            return (ULONG)GPI_ERROR;
        }
        if (!GpiPsGocaFindSeg(ps, lSegId, &start, &end)) {
            log("[PMSERVER] OUT : NGpiDeleteSegment -> GPI_ERROR (seg %ld not found)\n",
                (long)lSegId);
            return (ULONG)GPI_ERROR;
        }
        if (!GpiPsGocaDelete(ps, start, end))
            return (ULONG)GPI_ERROR;

        log("[PMSERVER] OUT : NGpiDeleteSegment -> 1 (removed %lu bytes)\n",
            (unsigned long)(end - start));
        return 1;
    }

    case NGpiDeleteSegments: {
        LONG lFirst, lLast;
        PGPI_PS ps;
        ULONG off;

        log("[PMSERVER] IN  : NGpiDeleteSegments(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lFirst, pArgs, sizeof(LONG));
        memcpy(&lLast,  pArgs + sizeof(LONG), sizeof(LONG));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        off = 0;
        while (off < ps->cbGoca) {
            const BYTE *p = ps->pbGoca + off;
            ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
            if (cb == 0) break;

            if (p[0] == OCODE_GBSEG && cb >= 6) {
                ULONG name = GocaGetU32(&ps->goca, p + 2);
                if ((LONG)name >= lFirst && (LONG)name <= lLast) {
                    ULONG start, end;
                    if (ps->lOpenSeg == (LONG)name) {
                        log("[PMSERVER] OUT : NGpiDeleteSegments -> GPI_ERROR (seg %lu open)\n",
                            (unsigned long)name);
                        return (ULONG)GPI_ERROR;
                    }
                    if (!GpiPsGocaFindSeg(ps, (LONG)name, &start, &end))
                        break;
                    GpiPsGocaDelete(ps, start, end);
                    off = start;   /* re-scan from same position */
                    continue;
                }
            }
            off += cb;
        }
        log("[PMSERVER] OUT : NGpiDeleteSegments -> 1\n");
        return 1;
    }

    case NGpiSetSegmentAttrs: {
        LONG lSegId, lAttr, lValue;
        PGPI_PS ps;
        ULONG start, end;
        UCHAR f1 = 0, f2 = 0;

        log("[PMSERVER] IN  : NGpiSetSegmentAttrs(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 3 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lSegId, pArgs, sizeof(LONG));
        memcpy(&lAttr,  pArgs + sizeof(LONG), sizeof(LONG));
        memcpy(&lValue, pArgs + 2 * sizeof(LONG), sizeof(LONG));
        if (lValue != ATTR_ON && lValue != ATTR_OFF)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (!GpiPsGocaFindSeg(ps, lSegId, &start, &end))
            return (ULONG)GPI_ERROR;
        if (start + 8 > end) return (ULONG)GPI_ERROR;

        f1 = ps->pbGoca[start + 6];
        f2 = ps->pbGoca[start + 7];

        switch (lAttr) {
        case ATTR_VISIBLE:
            if (lValue == ATTR_ON) f1 &= (UCHAR)~GBSEG_INVISIBLE;
            else                   f1 |= GBSEG_INVISIBLE;
            break;
        case ATTR_PROP_VISIBLE:
            if (lValue == ATTR_ON) f1 &= (UCHAR)~GBSEG_PROPINV;
            else                   f1 |= GBSEG_PROPINV;
            break;
        case ATTR_DETECTABLE:
            if (lValue == ATTR_ON) f1 |= GBSEG_PICK;
            else                   f1 &= (UCHAR)~GBSEG_PICK;
            break;
        case ATTR_PROP_DETECTABLE:
            if (lValue == ATTR_ON) f1 |= GBSEG_PROPICK;
            else                   f1 &= (UCHAR)~GBSEG_PROPICK;
            break;
        case ATTR_DYNAMIC:
            if (lValue == ATTR_ON) f1 |= GBSEG_DYNAMIC;
            else                   f1 &= (UCHAR)~GBSEG_DYNAMIC;
            break;
        case ATTR_FASTCHAIN:
            if (lValue == ATTR_ON) f1 |= GBSEG_FASTCH;
            else                   f1 &= (UCHAR)~GBSEG_FASTCH;
            break;
        case ATTR_CHAINED:
            if (lValue == ATTR_ON) f2 &= (UCHAR)~GBSEG_CHAIN;
            else                   f2 |= GBSEG_CHAIN;
            break;
        default:
            return (ULONG)GPI_ERROR;
        }

        ps->pbGoca[start + 6] = f1;
        ps->pbGoca[start + 7] = f2;
        return 1;
    }

    case NGpiQuerySegmentAttrs: {
        LONG lSegId, lAttr;
        PGPI_PS ps;
        ULONG start, end;
        UCHAR f1, f2;

        log("[PMSERVER] IN  : NGpiQuerySegmentAttrs(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lSegId, pArgs, sizeof(LONG));
        memcpy(&lAttr,  pArgs + sizeof(LONG), sizeof(LONG));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (!GpiPsGocaFindSeg(ps, lSegId, &start, &end))
            return (ULONG)GPI_ERROR;
        if (start + 8 > end) return (ULONG)GPI_ERROR;

        f1 = ps->pbGoca[start + 6];
        f2 = ps->pbGoca[start + 7];

        switch (lAttr) {
        case ATTR_VISIBLE:          return (f1 & GBSEG_INVISIBLE) ? ATTR_OFF : ATTR_ON;
        case ATTR_PROP_VISIBLE:     return (f1 & GBSEG_PROPINV)   ? ATTR_OFF : ATTR_ON;
        case ATTR_DETECTABLE:       return (f1 & GBSEG_PICK)      ? ATTR_ON  : ATTR_OFF;
        case ATTR_PROP_DETECTABLE:  return (f1 & GBSEG_PROPICK)   ? ATTR_ON  : ATTR_OFF;
        case ATTR_DYNAMIC:          return (f1 & GBSEG_DYNAMIC)   ? ATTR_ON  : ATTR_OFF;
        case ATTR_FASTCHAIN:        return (f1 & GBSEG_FASTCH)    ? ATTR_ON  : ATTR_OFF;
        case ATTR_CHAINED:          return (f2 & GBSEG_CHAIN)     ? ATTR_OFF : ATTR_ON;
        default:                    return (ULONG)GPI_ERROR;
        }
    }

    case NGpiQuerySegmentNames: {
        LONG lFirst, lLast, lMax;
        PGPI_PS ps;
        ULONG off;
        LONG  n = 0;
        LONG *pOut;

        log("[PMSERVER] IN  : NGpiQuerySegmentNames(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 3 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lFirst, pArgs, sizeof(LONG));
        memcpy(&lLast,  pArgs + sizeof(LONG), sizeof(LONG));
        memcpy(&lMax,   pArgs + 2 * sizeof(LONG), sizeof(LONG));
        if (lMax <= 0 || cbArgs < 3 * sizeof(LONG) + (ULONG)lMax * sizeof(LONG))
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        pOut = (LONG*)(pArgs + 3 * sizeof(LONG));
        off  = 0;
        while (off < ps->cbGoca && n < lMax) {
            const BYTE *p = ps->pbGoca + off;
            ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
            if (cb == 0) break;

            if (p[0] == OCODE_GBSEG && cb >= 6) {
                ULONG name = GocaGetU32(&ps->goca, p + 2);
                if ((LONG)name >= lFirst && (LONG)name <= lLast)
                    pOut[n++] = (LONG)name;
            }
            off += cb;
        }

        if (pcbPayload) *pcbPayload = cbArgs;   /* send back modified pArgs */
        log("[PMSERVER] OUT : NGpiQuerySegmentNames -> %ld names, payload=%u\n",
            (long)n, (unsigned)cbArgs);
        return (ULONG)n;
    }

    case NGpiGetData: {
        PGpiGetData_Args a = (PGpiGetData_Args)pArgs;
        PGPI_PS ps;
        ULONG start, end, avail, avail_from_off, toCopy;

        log("[PMSERVER] IN  : NGpiGetData(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < 4 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        if (a->lFormat != 0 /* DFORM_NOCONV */) {
            log("[PMSERVER] OUT : NGpiGetData -> GPI_ERROR (unsupported format %ld)\n",
                (long)a->lFormat);
            return (ULONG)GPI_ERROR;
        }
        if (a->lLength < 0 || a->lOffset < 0) return (ULONG)GPI_ERROR;
        if (cbArgs < 4 * sizeof(LONG) + (ULONG)a->lLength) return (ULONG)GPI_ERROR;

        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (!GpiPsGocaFindSeg(ps, a->lSegid, &start, &end)) {
            log("[PMSERVER] OUT : NGpiGetData -> GPI_ERROR (seg %ld not found)\n",
                (long)a->lSegid);
            return (ULONG)GPI_ERROR;
        }

        /* Skip the GBSEG header: GpiGetData returns the segment body
           only. GBSEG is 2-byte fixed code + X'0C' length = 14 bytes
           per pmord.h ORDERL_GBSEG (uchCode + uchLength + 12 payload). */
        if (end - start > 14)
            start += 14;
        else
            start = end;   /* empty body */

        avail = end - start;
        avail_from_off = ((ULONG)a->lOffset < avail) ? (avail - (ULONG)a->lOffset) : 0;
        toCopy = ((ULONG)a->lLength < avail_from_off) ? (ULONG)a->lLength : avail_from_off;
        if (toCopy > 0)
            memcpy(a->abData, ps->pbGoca + start + (ULONG)a->lOffset, toCopy);
        a->lOffset += (LONG)toCopy;

        if (pcbPayload) *pcbPayload = cbArgs;   /* send back updated pArgs */
        log("[PMSERVER] OUT : NGpiGetData -> %lu bytes (seg %ld, off %ld)\n",
            (unsigned long)toCopy, (long)a->lSegid, (long)a->lOffset);
        return (ULONG)toCopy;
    }

    case NGpiPutData: {
        PGpiPutData_Args a = (PGpiPutData_Args)pArgs;
        PGPI_PS ps;
        ULONG off, total, processed;

        log("[PMSERVER] IN  : NGpiPutData(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        if (a->lCount < 0 ||
            cbArgs < 2 * sizeof(LONG) + (ULONG)a->lCount)
            return (ULONG)GPI_ERROR;

        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        total     = (ULONG)a->lCount;
        off       = 0;
        processed = 0;

        while (off < total) {
            ULONG cb = GocaOrderLength(&ps->goca, a->abData + off, total - off);
            if (cb == 0) break;   /* trailing partial order */

            /* GBSEG in the input is a segment marker, not drawing data.
               Segment boundaries are the responsibility of OpenSegment /
               CloseSegment, not PutData. Skip it without recording or
               dispatching, but count it as processed so that *plCount
               reflects the whole consumed buffer. */
            if (a->abData[off] == OCODE_GBSEG) {
                off       += cb;
                processed += cb;
                continue;
            }

            if (!SrvDeliver(ps, a->abData + off, cb)) break;
            off       += cb;
            processed += cb;
        }

        a->lCount = (LONG)processed;

        if (pcbPayload) *pcbPayload = cbArgs;   /* send back updated pArgs */
        log("[PMSERVER] OUT : NGpiPutData -> %lu/%lu bytes processed\n",
            (unsigned long)processed, (unsigned long)total);
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
    DISPATCH_STUB(NGpiQuerySegmentPriority)
    DISPATCH_STUB(NGpiSetSegmentPriority)
    DISPATCH_STUB(NGpiDrawChain)
    DISPATCH_STUB(NGpiDrawFrom)
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
    DISPATCH_STUB(NGpiQueryCurrentPosition)
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
