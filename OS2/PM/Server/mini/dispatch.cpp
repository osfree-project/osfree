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
        /* Each non-bracket order emitted in SEGEM_INSERT becomes one
           element; advance the pointer so a subsequent GpiBeginElement
           bracket is inserted AFTER the preamble, not before the
           first element.  Bracket delimiters (GBEL/GEEL) do not
           advance here: GBEL arrives before bInElement is set, and
           the whole GBEL..GEEL bracket is one element whose pointer
           advance is performed by SrvGocaInsertElement on the GEEL
           re-pack.  Orders emitted inside an open bracket do not
           advance either. */
        if (!ps->bInElement && ps->ulEditMode == SEGEM_INSERT &&
            p[0] != OCODE_GBEL && p[0] != OCODE_GEEL)
            ps->lCurrentElement++;
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

/* ======================================================================== */
/*  Segment iteration helpers (DrawChain / DrawFrom / DrawDynamics)         */
/* ======================================================================== */

/*!
 * @brief Callback for SrvForEachSegment.
 *
 * @param [in] ps      Target PS.
 * @param [in] lSegId  Segment identifier (from GBSEG NAME field).
 * @param [in] start   Offset of the GBSEG header in ps->pbGoca.
 * @param [in] end     End offset (exclusive).
 * @param [in] f1      FLAG1 byte (SC31-6804).
 * @param [in] f2      FLAG2 byte (AFPC-0008-03).
 * @param [in] pUser   Opaque pointer passed through from the caller.
 * @return TRUE to continue iteration, FALSE to stop.
 */
typedef BOOL (*PFN_SRV_SEG_CB)(PGPI_PS ps, LONG lSegId,
                               ULONG start, ULONG end,
                               UCHAR f1, UCHAR f2, PVOID pUser);

/*!
 * @brief Iterate over all segments in ps->pbGoca.
 *
 * Walks the accumulated GOCA stream, finds each GBSEG header, computes
 * its [start, end) range (end = next GBSEG or end of buffer), and
 * invokes pfn. Stops on callback FALSE or on stream corruption.
 *
 * @param [in] ps     Target PS.
 * @param [in] pfn    Callback.
 * @param [in] pUser  Opaque pointer passed to the callback.
 * @return TRUE on full traversal, FALSE on corruption.
 */
static BOOL SrvForEachSegment(PGPI_PS ps, PFN_SRV_SEG_CB pfn, PVOID pUser)
{
    ULONG off;

    if (!ps || !ps->pbGoca || !pfn) return FALSE;

    off = 0;
    while (off < ps->cbGoca) {
        const BYTE *p = ps->pbGoca + off;
        ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
        if (cb == 0) return FALSE;

        if (p[0] == OCODE_GBSEG && cb >= 14) {
            ULONG name  = GocaGetU32(&ps->goca, p + 2);
            UCHAR f1    = p[6];
            UCHAR f2    = p[7];
            ULONG start = off;
            ULONG end   = ps->cbGoca;
            ULONG scan  = off + cb;

            while (scan < ps->cbGoca) {
                const BYTE *q = ps->pbGoca + scan;
                ULONG cq = GocaOrderLength(&ps->goca, q,
                                           ps->cbGoca - scan);
                if (cq == 0) return FALSE;
                if (q[0] == OCODE_GBSEG) { end = scan; break; }
                scan += cq;
            }

            if (!pfn(ps, (LONG)name, start, end, f1, f2, pUser))
                return TRUE;   /* callback asked to stop */

            off = end;
        } else {
            off += cb;
        }
    }
    return TRUE;
}

/*!
 * @brief Replay one segment's body through the GOCA parser.
 *
 * The GBSEG header itself is a no-op in GocaParseStream, so the whole
 * [start, end) range can be fed to it unchanged.
 *
 * @param [in] ps     Target PS.
 * @param [in] start  Offset of GBSEG in ps->pbGoca.
 * @param [in] end    End offset (exclusive).
 */
static void SrvReplaySegment(PGPI_PS ps, ULONG start, ULONG end)
{
    ps->goca.pbStream = ps->pbGoca + start;
    ps->goca.cbStream = end - start;
    GocaParseStream(&ps->goca);
}

/*!
 * @brief Save current foreground/background mix from the cached line
 *        bundle (GSMX/GSBMX synchronously update all four primitive
 *        bundles, so one cache is sufficient).
 *
 * @param [in]  ps        Target PS.
 * @param [out] pusFgMix  Saved foreground mix.
 * @param [out] pusBgMix  Saved background mix.
 */
static void SrvSaveMix(PGPI_PS ps, USHORT *pusFgMix, USHORT *pusBgMix)
{
    *pusFgMix = ps->goca.lb.usMixMode;
    *pusBgMix = ps->goca.lb.usBackMixMode;
}

/*!
 * @brief Install FM_XOR / BM_LEAVEALONE for all primitives via GSMX/GSBMX.
 *
 * @param [in] ps  Target PS.
 */
static void SrvEnterDynamicsMix(PGPI_PS ps)
{
    BYTE buf[2];
    LONG len;

    len = GocaEmitSetMix(&ps->goca, buf, sizeof(buf), (LONG)FM_XOR);
    if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);

    len = GocaEmitSetBackMix(&ps->goca, buf, sizeof(buf), (LONG)BM_LEAVEALONE);
    if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);
}

/*!
 * @brief Restore foreground/background mix saved by SrvSaveMix.
 *
 * @param [in] ps      Target PS.
 * @param [in] usFgMix Saved foreground mix.
 * @param [in] usBgMix Saved background mix.
 */
static void SrvLeaveDynamicsMix(PGPI_PS ps, USHORT usFgMix, USHORT usBgMix)
{
    BYTE buf[2];
    LONG len;

    len = GocaEmitSetMix(&ps->goca, buf, sizeof(buf), (LONG)usFgMix);
    if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);

    len = GocaEmitSetBackMix(&ps->goca, buf, sizeof(buf), (LONG)usBgMix);
    if (len >= 0) SrvParseOrder(ps, buf, (ULONG)len);
}

/*!
 * @brief Range used by DrawFrom / RemoveDynamics callbacks.
 */
typedef struct _SRV_SEGRANGE {
    LONG lFirst;
    LONG lLast;
} SRV_SEGRANGE;

/*!
 * @brief Callback for NGpiDrawChain: visible, chained, non-dynamic.
 */
static BOOL SrvCbDrawChain(PGPI_PS ps, LONG lSegId, ULONG start, ULONG end,
                           UCHAR f1, UCHAR f2, PVOID pUser)
{
    (void)lSegId; (void)pUser;

    if (f1 & GBSEG_INVISIBLE) return TRUE;
    if (f1 & GBSEG_DYNAMIC)   return TRUE;
    if (f2 & GBSEG_CHAIN)     return TRUE;

    SrvReplaySegment(ps, start, end);
    return TRUE;
}

/*!
 * @brief Callback for NGpiDrawFrom: same filter as DrawChain + range.
 */
static BOOL SrvCbDrawFrom(PGPI_PS ps, LONG lSegId, ULONG start, ULONG end,
                          UCHAR f1, UCHAR f2, PVOID pUser)
{
    SRV_SEGRANGE *pRange = (SRV_SEGRANGE *)pUser;

    if (lSegId < pRange->lFirst || lSegId > pRange->lLast) return TRUE;
    if (f1 & GBSEG_INVISIBLE) return TRUE;
    if (f1 & GBSEG_DYNAMIC)   return TRUE;
    if (f2 & GBSEG_CHAIN)     return TRUE;

    SrvReplaySegment(ps, start, end);
    return TRUE;
}

/*!
 * @brief Callback for NGpiDrawDynamics: visible, chained, dynamic.
 */
static BOOL SrvCbDrawDynamics(PGPI_PS ps, LONG lSegId, ULONG start, ULONG end,
                              UCHAR f1, UCHAR f2, PVOID pUser)
{
    (void)lSegId; (void)pUser;

    if (f1 & GBSEG_INVISIBLE)  return TRUE;
    if (!(f1 & GBSEG_DYNAMIC)) return TRUE;
    if (f2 & GBSEG_CHAIN)      return TRUE;

    SrvReplaySegment(ps, start, end);
    return TRUE;
}

/*!
 * @brief Callback for NGpiRemoveDynamics: visible, chained, dynamic, in range.
 */
static BOOL SrvCbRemoveDynamics(PGPI_PS ps, LONG lSegId, ULONG start, ULONG end,
                                UCHAR f1, UCHAR f2, PVOID pUser)
{
    SRV_SEGRANGE *pRange = (SRV_SEGRANGE *)pUser;

    if (lSegId < pRange->lFirst || lSegId > pRange->lLast) return TRUE;
    if (f1 & GBSEG_INVISIBLE)  return TRUE;
    if (!(f1 & GBSEG_DYNAMIC)) return TRUE;
    if (f2 & GBSEG_CHAIN)      return TRUE;

    SrvReplaySegment(ps, start, end);
    return TRUE;
}

/* ======================================================================== */
/*  Segment priority helpers (Step F)                                       */
/* ======================================================================== */

/*!
 * @brief TRUE if a segment is in the chain: CHAINED=ON and DYNAMIC=OFF.
 *
 * FLAG1 bit 6 = DYNAMIC (SC31-6804); FLAG2 bit 0 = CHAIN (inverted).
 * VISIBLE does not affect chain membership (only DrawChain output).
 */
static BOOL SrvSegInChain(UCHAR f1, UCHAR f2)
{
    if (f1 & GBSEG_DYNAMIC) return FALSE;
    if (f2 & GBSEG_CHAIN)   return FALSE;
    return TRUE;
}

/*!
 * @brief Get flags of a segment by id.
 * @param [in]  ps      Target PS.
 * @param [in]  lSegId  Segment identifier.
 * @param [out] pStart  Start offset (may be NULL).
 * @param [out] pEnd    End offset, exclusive (may be NULL).
 * @param [out] pF1     FLAG1 byte (may be NULL).
 * @param [out] pF2     FLAG2 byte (may be NULL).
 * @return TRUE if found, FALSE otherwise.
 */
static BOOL SrvSegInfo(PGPI_PS ps, LONG lSegId,
                       PULONG pStart, PULONG pEnd,
                       PBYTE pF1, PBYTE pF2)
{
    ULONG s, e;
    if (!GpiPsGocaFindSeg(ps, lSegId, &s, &e)) return FALSE;
    if (e - s < 14) return FALSE;
    if (pStart) *pStart = s;
    if (pEnd)   *pEnd   = e;
    if (pF1)    *pF1    = ps->pbGoca[s + 6];
    if (pF2)    *pF2    = ps->pbGoca[s + 7];
    return TRUE;
}

/*!
 * @brief Find first (lowest-priority) chained segment.
 * @return TRUE if found; *pStart = offset of its GBSEG.
 */
static BOOL SrvFindFirstChainSeg(PGPI_PS ps, PULONG pStart)
{
    ULONG off = 0;
    while (off < ps->cbGoca) {
        const BYTE *p = ps->pbGoca + off;
        ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
        if (cb == 0) return FALSE;
        if (p[0] == OCODE_GBSEG && cb >= 14) {
            if (SrvSegInChain(p[6], p[7])) { *pStart = off; return TRUE; }
        }
        off += cb;
    }
    return FALSE;
}

/*!
 * @brief Find last (highest-priority) chained segment.
 * @return TRUE if found; *pStart = offset of its GBSEG.
 */
static BOOL SrvFindLastChainSeg(PGPI_PS ps, PULONG pStart)
{
    ULONG off = 0, last = 0;
    BOOL  found = FALSE;
    while (off < ps->cbGoca) {
        const BYTE *p = ps->pbGoca + off;
        ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
        if (cb == 0) break;
        if (p[0] == OCODE_GBSEG && cb >= 14) {
            if (SrvSegInChain(p[6], p[7])) { last = off; found = TRUE; }
        }
        off += cb;
    }
    if (found) *pStart = last;
    return found;
}

/*!
 * @brief Cut [start, end) from pbGoca and re-insert at insertPos.
 *
 * insertPos is interpreted in the original buffer (before the cut).
 * Corrects ps->uOpenSegStart so the open segment stays valid.
 *
 * @param [in] ps         Target PS.
 * @param [in] start      Start offset of the range.
 * @param [in] end        End offset of the range (exclusive).
 * @param [in] insertPos  Offset to insert the range at (pre-cut).
 * @return TRUE on success, FALSE otherwise.
 */
static BOOL SrvMoveRange(PGPI_PS ps, ULONG start, ULONG end, ULONG insertPos)
{
    ULONG len, tail, openPos;
    BOOL  hasOpen;
    PVOID pv = NULL;
    BYTE *tmp;

    if (start >= end || end > ps->cbGoca) return FALSE;
    if (insertPos >= start && insertPos <= end) return TRUE; /* no-op */

    hasOpen = (ps->lOpenSeg != 0 && ps->uOpenSegStart != 0);
    openPos = ps->uOpenSegStart;
    if (hasOpen && openPos >= start && openPos < end)
        return FALSE;   /* open segment inside moved range */

    len = end - start;
    if (DosAllocMem(&pv, len, PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
        return FALSE;
    tmp = (BYTE *)pv;
    memcpy(tmp, ps->pbGoca + start, len);

    tail = ps->cbGoca - end;
    if (tail > 0)
        memmove(ps->pbGoca + start, ps->pbGoca + end, tail);
    ps->cbGoca -= len;

    if (insertPos > start) insertPos -= len;

    if (insertPos < ps->cbGoca)
        memmove(ps->pbGoca + insertPos + len,
                ps->pbGoca + insertPos,
                ps->cbGoca - insertPos);
    memcpy(ps->pbGoca + insertPos, tmp, len);
    ps->cbGoca += len;
    DosFreeMem(pv);

    if (hasOpen) {
        if (openPos >= end) openPos -= len;
        if (openPos >= insertPos) openPos += len;
        ps->uOpenSegStart = openPos;
    }
    return TRUE;
}

/* ======================================================================== */
/*  Element traversal helpers (Step H)                                      */
/* ======================================================================== */

/*!
 * @brief Offset of the first element in a segment (skip GBSEG header).
 */
static ULONG SrvSegBodyStart(PGPI_PS ps, ULONG segStart, ULONG segEnd)
{
    if (segStart < segEnd && ps->pbGoca[segStart] == OCODE_GBSEG) {
        ULONG cb = GocaOrderLength(&ps->goca, ps->pbGoca + segStart,
                                   segEnd - segStart);
        if (cb > 0) return segStart + cb;
    }
    return segStart;
}

/*!
 * @brief Locate an element within a segment by 1-based index.
 *
 * Element = single order, or GBEL ... GEEL group.
 *
 * @param [in]  ps          Target PS.
 * @param [in]  segStart    Segment start offset (GBSEG).
 * @param [in]  segEnd      Segment end offset (exclusive).
 * @param [in]  lIndex      1-based element index.
 * @param [out] pStart      Element start offset (GBEL or single order).
 * @param [out] pEnd        Element end offset (exclusive, after GEEL).
 * @param [out] pBodyStart  Body start (after GBEL, or == *pStart).
 * @param [out] pBodyEnd    Body end (before GEEL, or == *pEnd).
 * @param [out] pbHasGbel   TRUE if element is a GBEL..GEEL group.
 * @return TRUE if found, FALSE otherwise.
 */
static BOOL SrvFindElement(PGPI_PS ps, ULONG segStart, ULONG segEnd,
                           LONG lIndex,
                           PULONG pStart, PULONG pEnd,
                           PULONG pBodyStart, PULONG pBodyEnd,
                           PBOOL  pbHasGbel)
{
    ULONG off = SrvSegBodyStart(ps, segStart, segEnd);
    LONG  idx = 0;

    if (lIndex <= 0) return FALSE;

    while (off < segEnd) {
        ULONG start = off;
        ULONG len   = GocaOrderLength(&ps->goca, ps->pbGoca + off,
                                      segEnd - off);
        if (len == 0) return FALSE;

        if (ps->pbGoca[off] == OCODE_GBEL) {
            ULONG bodyStart = off + len;
            ULONG scan      = bodyStart;
            BOOL  found     = FALSE;

            while (scan < segEnd) {
                ULONG l2 = GocaOrderLength(&ps->goca, ps->pbGoca + scan,
                                           segEnd - scan);
                if (l2 == 0) return FALSE;
                if (ps->pbGoca[scan] == OCODE_GEEL) {
                    if (idx + 1 == lIndex) {
                        if (pStart)     *pStart = start;
                        if (pEnd)       *pEnd = scan + l2;
                        if (pBodyStart) *pBodyStart = bodyStart;
                        if (pBodyEnd)   *pBodyEnd = scan;
                        if (pbHasGbel)  *pbHasGbel = TRUE;
                        return TRUE;
                    }
                    idx++;
                    off = scan + l2;
                    found = TRUE;
                    break;
                }
                scan += l2;
            }
            if (!found) return FALSE;
        } else {
            if (idx + 1 == lIndex) {
                if (pStart)     *pStart = start;
                if (pEnd)       *pEnd = off + len;
                if (pBodyStart) *pBodyStart = start;
                if (pBodyEnd)   *pBodyEnd = off + len;
                if (pbHasGbel)  *pbHasGbel = FALSE;
                return TRUE;
            }
            idx++;
            off += len;
        }
    }
    return FALSE;
}

/*!
 * @brief Count elements in a segment.
 */
static LONG SrvCountElements(PGPI_PS ps, ULONG segStart, ULONG segEnd)
{
    ULONG off = SrvSegBodyStart(ps, segStart, segEnd);
    LONG  n   = 0;

    while (off < segEnd) {
        ULONG len = GocaOrderLength(&ps->goca, ps->pbGoca + off,
                                    segEnd - off);
        if (len == 0) break;

        if (ps->pbGoca[off] == OCODE_GBEL) {
            ULONG scan = off + len;
            BOOL  found = FALSE;
            while (scan < segEnd) {
                ULONG l2 = GocaOrderLength(&ps->goca, ps->pbGoca + scan,
                                           segEnd - scan);
                if (l2 == 0) break;
                if (ps->pbGoca[scan] == OCODE_GEEL) {
                    off = scan + l2;
                    found = TRUE;
                    break;
                }
                scan += l2;
            }
            if (!found) break;
        } else {
            off += len;
        }
        n++;
    }
    return n;
}

/*!
 * @brief Insert bytes into pbGoca at offset @p at.
 */
static BOOL SrvGocaInsertBytes(PGPI_PS ps, ULONG at, const BYTE *p, ULONG cb)
{
    PVOID pvNew;
    ULONG newAlloc;

    if (!ps || !p || cb == 0) return FALSE;
    if (at > ps->cbGoca) return FALSE;

    if (ps->cbGoca + cb > ps->cbGocaAlloc) {
        newAlloc = ps->cbGocaAlloc ? ps->cbGocaAlloc * 2 : 256;
        while (newAlloc < ps->cbGoca + cb) newAlloc *= 2;
        pvNew = NULL;
        if (DosAllocMem(&pvNew, newAlloc,
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
            return FALSE;
        if (ps->pbGoca && at > 0) memcpy(pvNew, ps->pbGoca, at);
        if (ps->pbGoca) DosFreeMem(ps->pbGoca);
        ps->pbGoca      = (PBYTE)pvNew;
        ps->cbGocaAlloc = newAlloc;
    }

    memmove(ps->pbGoca + at + cb, ps->pbGoca + at, ps->cbGoca - at);
    memcpy(ps->pbGoca + at, p, cb);
    ps->cbGoca += cb;
    return TRUE;
}

/*!
 * @brief Insert a complete element (single order or GBEL..GEEL) into the
 *        open segment at the position determined by lCurrentElement and
 *        ulEditMode. Updates lCurrentElement in INSERT mode.
 *
 * @param [in] ps       Target PS.
 * @param [in] segStart Segment start offset (GBSEG).
 * @param [in] segEnd   Segment end offset (exclusive).
 * @param [in] p        Element bytes.
 * @param [in] cb       Element size.
 * @return TRUE on success, FALSE on error.
 */
static BOOL SrvGocaInsertElement(PGPI_PS ps, ULONG segStart, ULONG segEnd,
                                  const BYTE *p, ULONG cb)
{
    ULONG at;

    if (ps->ulEditMode == SEGEM_REPLACE) {
        ULONG s, e, bs, be;
        BOOL  hasGbel;
        if (ps->lCurrentElement <= 0) return FALSE;
        if (!SrvFindElement(ps, segStart, segEnd, ps->lCurrentElement,
                            &s, &e, &bs, &be, &hasGbel))
            return FALSE;
        if (!GpiPsGocaDelete(ps, s, e)) return FALSE;
        at = s;
    } else {  /* SEGEM_INSERT */
        if (ps->lCurrentElement <= 0) {
            at = SrvSegBodyStart(ps, segStart, segEnd);
        } else {
            ULONG s, e, bs, be;
            BOOL  hasGbel;
            if (!SrvFindElement(ps, segStart, segEnd, ps->lCurrentElement,
                                &s, &e, &bs, &be, &hasGbel))
                return FALSE;
            at = e;
        }
    }

    if (!SrvGocaInsertBytes(ps, at, p, cb)) return FALSE;
    if (ps->ulEditMode == SEGEM_INSERT) ps->lCurrentElement++;
    return TRUE;
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
        if (ps->bInElement) return (ULONG)GPI_ERROR;
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
        /* PMERR_PATH_INCOMPLETE: an attempt was made to open a segment
           while there is an open path bracket. */
        if (ps->lOpenPath != 0) {
            log("[PMSERVER] OUT : NGpiOpenSegment -> GPI_ERROR (path open)\n");
            return (ULONG)GPI_ERROR;
        }
        if (ps->ulDrawingMode == DM_DRAW) {
            log("[PMSERVER] OUT : NGpiOpenSegment -> GPI_ERROR (in DM_DRAW)\n");
            return (ULONG)GPI_ERROR;
        }

        /* A path defined in the previous segment is bound to that
           segment; when a new segment is opened it is no longer
           available (PMERR_PATH_UNKNOWN on any path op).  Clear the
           stale defined-flag.  lOpenPath is already 0 (checked above). */
        ps->bPathDefined = FALSE;

        /* Edit an existing segment: locate its GBSEG (spec: GpiOpenSegment
           is the way to begin editing a retained segment). */
        if (GpiPsGocaFindSeg(ps, lSegId, &start, &end)) {
            ps->uOpenSegStart   = start;
            ps->lOpenSeg        = lSegId;
            ps->lCurrentElement = 0;
            ps->bInElement      = FALSE;
            ps->bPathDefined    = FALSE;
            log("[PMSERVER] OUT : NGpiOpenSegment -> 1 (edit off=%lu)\n",
                (unsigned long)start);
            return 1;
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
        ps->uOpenSegStart   = ps->cbGoca - sizeof(gbseg);
        ps->lOpenSeg        = lSegId;
        ps->lCurrentElement = 0;
        ps->bInElement      = FALSE;
        ps->bPathDefined    = FALSE;

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

        /* Implicit GpiEndElement at close (GPI Guide). */
        if (ps->bInElement) {
            BYTE  buf[2];
            ULONG segStart, segEnd;
            LONG  len;
            PVOID pvElem = NULL;
            PBYTE pElem;
            ULONG cbElem;

            len = GocaEmitEndElement(&ps->goca, buf, sizeof(buf));
            if (len < 0) return (ULONG)GPI_ERROR;
            SrvDeliver(ps, buf, (ULONG)len);

            if (GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd)) {
                cbElem = ps->cbGoca - ps->uOpenElementStart;
                if (cbElem > 0 &&
                    DosAllocMem(&pvElem, cbElem,
                                PAG_COMMIT | PAG_READ | PAG_WRITE) == 0) {
                    pElem = (PBYTE)pvElem;
                    memcpy(pElem, ps->pbGoca + ps->uOpenElementStart, cbElem);
                    GpiPsGocaDelete(ps, ps->uOpenElementStart, ps->cbGoca);
                    SrvGocaInsertElement(ps, segStart, segEnd,
                                          pElem, cbElem);
                    DosFreeMem(pvElem);
                }
            }
            ps->bInElement = FALSE;
        }

        /* Path bracket, if still open, is terminated.  Per OS/2 PM Ref
           (GpiCloseSegment - Remarks): "If an area or path is open when
           a segment is closed, the area or path is terminated.  When the
           drawing mode is draw or draw-and-retain, a warning is given,
           but the close processing continues.  No warning is given for
           retain mode."  No GEPTH order is emitted.  bPathDefined is
           cleared: a path defined in this segment cannot be consumed
           from a later segment (PMERR_PATH_UNKNOWN). */
        ps->lOpenPath    = 0;
        ps->bPathDefined = FALSE;

        ps->lOpenSeg          = 0;
        ps->uOpenSegStart     = 0;
        ps->uOpenElementStart = 0;
        ps->lCurrentElement   = 0;
        ps->bInElement        = FALSE;
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


    case NGpiDrawChain: {
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiDrawChain(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg != 0) {
            log("[PMSERVER] OUT : NGpiDrawChain -> GPI_ERROR (seg %ld open)\n",
                (long)ps->lOpenSeg);
            return (ULONG)GPI_ERROR;
        }
        SrvForEachSegment(ps, SrvCbDrawChain, NULL);
        log("[PMSERVER] OUT : NGpiDrawChain -> 1\n");
        return 1;
    }

    case NGpiDrawFrom: {
        LONG lFirst, lLast;
        SRV_SEGRANGE range;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiDrawFrom(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lFirst, pArgs, sizeof(LONG));
        memcpy(&lLast,  pArgs + sizeof(LONG), sizeof(LONG));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg != 0) {
            log("[PMSERVER] OUT : NGpiDrawFrom -> GPI_ERROR (seg %ld open)\n",
                (long)ps->lOpenSeg);
            return (ULONG)GPI_ERROR;
        }
        range.lFirst = lFirst;
        range.lLast  = lLast;
        SrvForEachSegment(ps, SrvCbDrawFrom, &range);
        log("[PMSERVER] OUT : NGpiDrawFrom -> 1\n");
        return 1;
    }

    case NGpiDrawDynamics: {
        USHORT usFgMix, usBgMix;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiDrawDynamics(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg != 0) {
            log("[PMSERVER] OUT : NGpiDrawDynamics -> GPI_ERROR (seg %ld open)\n",
                (long)ps->lOpenSeg);
            return (ULONG)GPI_ERROR;
        }
        SrvSaveMix(ps, &usFgMix, &usBgMix);
        SrvEnterDynamicsMix(ps);
        SrvForEachSegment(ps, SrvCbDrawDynamics, NULL);
        SrvLeaveDynamicsMix(ps, usFgMix, usBgMix);
        log("[PMSERVER] OUT : NGpiDrawDynamics -> 1\n");
        return 1;
    }

    case NGpiRemoveDynamics: {
        LONG lFirst, lLast;
        SRV_SEGRANGE range;
        USHORT usFgMix, usBgMix;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiRemoveDynamics(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lFirst, pArgs, sizeof(LONG));
        memcpy(&lLast,  pArgs + sizeof(LONG), sizeof(LONG));
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg != 0) {
            log("[PMSERVER] OUT : NGpiRemoveDynamics -> GPI_ERROR (seg %ld open)\n",
                (long)ps->lOpenSeg);
            return (ULONG)GPI_ERROR;
        }
        range.lFirst = lFirst;
        range.lLast  = lLast;
        SrvSaveMix(ps, &usFgMix, &usBgMix);
        SrvEnterDynamicsMix(ps);
        SrvForEachSegment(ps, SrvCbRemoveDynamics, &range);
        SrvLeaveDynamicsMix(ps, usFgMix, usBgMix);
        log("[PMSERVER] OUT : NGpiRemoveDynamics -> 1\n");
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

        /* GPI Guide: PutData cannot be used in SEGEM_REPLACE. */
        if (ps->ulEditMode == SEGEM_REPLACE)
            return (ULONG)GPI_ERROR;

        /* GPI Guide: in SEGEM_INSERT, pointer must be at the last
           element of the current segment. */
        if (ps->lOpenSeg != 0) {
            ULONG segStart, segEnd;
            LONG  n;
            if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
                return (ULONG)GPI_ERROR;
            n = SrvCountElements(ps, segStart, segEnd);
            if (ps->lCurrentElement != n)
                return (ULONG)GPI_ERROR;
        }

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

        /* Keep the pointer at the end of the current segment. */
        if (ps->lOpenSeg != 0) {
            ULONG segStart, segEnd;
            if (GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
                ps->lCurrentElement = SrvCountElements(ps, segStart, segEnd);
        }

        if (pcbPayload) *pcbPayload = cbArgs;
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
    case NGpiQuerySegmentPriority: {
        LONG lRefSegid, lOrder;
        PGPI_PS ps;

        log("[PMSERVER] IN  : NGpiQuerySegmentPriority(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG))
            return (ULONG)(LONG)GPI_ALTERROR;
        memcpy(&lRefSegid, pArgs, sizeof(LONG));
        memcpy(&lOrder,   pArgs + sizeof(LONG), sizeof(LONG));
        if (lOrder != LOWER_PRI && lOrder != HIGHER_PRI)
            return (ULONG)(LONG)GPI_ALTERROR;
        if (lRefSegid < 0)
            return (ULONG)(LONG)GPI_ALTERROR;

        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)(LONG)GPI_ALTERROR;

        if (lRefSegid == 0) {
            ULONG pos;
            if (lOrder == LOWER_PRI) {
                if (!SrvFindFirstChainSeg(ps, &pos))
                    return (ULONG)(LONG)GPI_ALTERROR; /* empty chain */
            } else {
                if (!SrvFindLastChainSeg(ps, &pos))
                    return (ULONG)(LONG)GPI_ALTERROR;
            }
            return (ULONG)GocaGetU32(&ps->goca, ps->pbGoca + pos + 2);
        }

        {
            ULONG s, e, pos;
            UCHAR f1, f2;
            if (!SrvSegInfo(ps, lRefSegid, &s, &e, &f1, &f2))
                return (ULONG)(LONG)GPI_ALTERROR;   /* SEG_NOT_FOUND */
            if (!SrvSegInChain(f1, f2))
                return (ULONG)(LONG)GPI_ALTERROR;   /* SEG_NOT_CHAINED */

            if (lOrder == LOWER_PRI) {
                /* Walk backwards: find chain segment immediately before s. */
                ULONG off = 0, prev = 0;
                BOOL  found = FALSE;
                while (off < s) {
                    const BYTE *p = ps->pbGoca + off;
                    ULONG cb = GocaOrderLength(&ps->goca, p, s - off);
                    if (cb == 0) break;
                    if (p[0] == OCODE_GBSEG && cb >= 14 &&
                        SrvSegInChain(p[6], p[7])) { prev = off; found = TRUE; }
                    off += cb;
                }
                if (!found) return 0;
                pos = prev;
            } else {
                /* Walk forwards: find chain segment immediately after e. */
                ULONG off = e;
                BOOL  found = FALSE;
                while (off < ps->cbGoca) {
                    const BYTE *p = ps->pbGoca + off;
                    ULONG cb = GocaOrderLength(&ps->goca, p, ps->cbGoca - off);
                    if (cb == 0) break;
                    if (p[0] == OCODE_GBSEG && cb >= 14 &&
                        SrvSegInChain(p[6], p[7])) { found = TRUE; break; }
                    off += cb;
                }
                if (!found) return 0;
                pos = off;
            }
            return (ULONG)GocaGetU32(&ps->goca, ps->pbGoca + pos + 2);
        }
    }
    case NGpiSetSegmentPriority: {
        LONG lSegid, lRefSegid, lOrder;
        PGPI_PS ps;
        ULONG s, e, ins;
        UCHAR f1, f2;

        log("[PMSERVER] IN  : NGpiSetSegmentPriority(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 3 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lSegid,    pArgs,                     sizeof(LONG));
        memcpy(&lRefSegid, pArgs +     sizeof(LONG),  sizeof(LONG));
        memcpy(&lOrder,    pArgs + 2 * sizeof(LONG),  sizeof(LONG));

        if (lSegid <= 0) return (ULONG)GPI_ERROR;
        if (lRefSegid < 0) return (ULONG)GPI_ERROR;
        if (lOrder != LOWER_PRI && lOrder != HIGHER_PRI)
            return (ULONG)GPI_ERROR;
        if (lSegid == lRefSegid) return 1;   /* no-op */

        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg != 0 &&
            (ps->lOpenSeg == lSegid || ps->lOpenSeg == lRefSegid))
            return (ULONG)GPI_ERROR;

        if (!SrvSegInfo(ps, lSegid, &s, &e, &f1, &f2))
            return (ULONG)GPI_ERROR;

        /* Attach lSegid to chain before searching (spec: not-chained is added). */
        if (!SrvSegInChain(f1, f2)) {
            ps->pbGoca[s + 6] &= (UCHAR)~GBSEG_DYNAMIC;
            ps->pbGoca[s + 7] &= (UCHAR)~GBSEG_CHAIN;
        }

        if (lRefSegid != 0) {
            ULONG rs, re;
            UCHAR rf1, rf2;
            if (!SrvSegInfo(ps, lRefSegid, &rs, &re, &rf1, &rf2))
                return (ULONG)GPI_ERROR;          /* SEG_NOT_FOUND */
            if (!SrvSegInChain(rf1, rf2))
                return (ULONG)GPI_ERROR;          /* SEG_NOT_CHAINED */
            ins = (lOrder == LOWER_PRI) ? rs : re;
        } else {
            ULONG pos;
            if (lOrder == LOWER_PRI) {
                if (!SrvFindFirstChainSeg(ps, &pos))
                    ins = 0;                       /* should not happen */
                else
                    ins = pos;
            } else {
                if (!SrvFindLastChainSeg(ps, &pos)) {
                    ins = ps->cbGoca;
                } else {
                    ULONG posEnd;
                    LONG  lastId = (LONG)GocaGetU32(&ps->goca,
                                                    ps->pbGoca + pos + 2);
                    if (!SrvSegInfo(ps, lastId, &pos, &posEnd, NULL, NULL))
                        return (ULONG)GPI_ERROR;
                    ins = posEnd;
                }
            }
        }

        if (!SrvMoveRange(ps, s, e, ins))
            return (ULONG)GPI_ERROR;

        log("[PMSERVER] OUT : NGpiSetSegmentPriority -> 1\n");
        return 1;
    }
    case NGpiBeginElement: {
        PGpiBeginElement_Args a = (PGpiBeginElement_Args)pArgs;
        PGPI_PS ps;
        BYTE buf[6 + GBEL_DL];
        LONG len;

        log("[PMSERVER] IN  : NGpiBeginElement(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < sizeof(*a)) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN &&
            ps->ulDrawingMode != DM_DRAWANDRETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        /* Element type for a user-defined element must be in the range
           [0x81000000, 0xFFFFFFFF] (GpiBeginElement/GpiElement - Remarks). */
        if ((ULONG)a->lType < 0x81000000UL) return (ULONG)GPI_ERROR;

        len = GocaEmitBeginElement(&ps->goca, buf, sizeof(buf),
                                    a->lType, a->szDesc);
        if (len < 0) return (ULONG)GPI_ERROR;

        ps->uOpenElementStart = ps->cbGoca;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->bInElement = TRUE;

        log("[PMSERVER] OUT : NGpiBeginElement -> 1\n");
        return 1;
    }
    case NGpiEndElement: {
        PGPI_PS ps;
        BYTE  buf[2];
        ULONG segStart, segEnd;
        LONG  len;
        PVOID pvElem = NULL;
        PBYTE pElem;
        ULONG cbElem;

        log("[PMSERVER] IN  : NGpiEndElement(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0) return (ULONG)GPI_ERROR;
        if (!ps->bInElement)   return (ULONG)GPI_ERROR;

        len = GocaEmitEndElement(&ps->goca, buf, sizeof(buf));
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);

        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;

        cbElem = ps->cbGoca - ps->uOpenElementStart;
        if (cbElem > 0 &&
            DosAllocMem(&pvElem, cbElem,
                        PAG_COMMIT | PAG_READ | PAG_WRITE) == 0) {
            pElem = (PBYTE)pvElem;
            memcpy(pElem, ps->pbGoca + ps->uOpenElementStart, cbElem);
            GpiPsGocaDelete(ps, ps->uOpenElementStart, ps->cbGoca);
            SrvGocaInsertElement(ps, segStart, segEnd, pElem, cbElem);
            DosFreeMem(pvElem);
        }
        ps->bInElement        = FALSE;
        ps->uOpenElementStart = 0;

        log("[PMSERVER] OUT : NGpiEndElement -> 1\n");
        return 1;
    }

    case NGpiLabel: {
        LONG   lLabel;
        PGPI_PS ps;
        BYTE   buf[6];
        LONG   len;
        ULONG  segStart, segEnd;

        log("[PMSERVER] IN  : NGpiLabel(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lLabel = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN &&
            ps->ulDrawingMode != DM_DRAWANDRETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;

        len = GocaEmitLabel(&ps->goca, buf, sizeof(buf), lLabel);
        if (len < 0) return (ULONG)GPI_ERROR;

        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;
        if (!SrvGocaInsertElement(ps, segStart, segEnd, buf, (ULONG)len))
            return (ULONG)GPI_ERROR;

        log("[PMSERVER] OUT : NGpiLabel -> 1\n");
        return 1;
    }

    case NGpiElement: {
        PGpiElement_Args a = (PGpiElement_Args)pArgs;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        ULONG  cbElem;
        PVOID  pvElem = NULL;
        PBYTE  pElem;
        LONG   cbGbel, cbGeel;
        BYTE   gbel[6 + GBEL_DL];
        BYTE   geel[2];
        ULONG  off;

        log("[PMSERVER] IN  : NGpiElement(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < 3 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        if (a->lLength < 0 || a->lLength > 64512) return (ULONG)GPI_ERROR;
        if (cbArgs < 3 * sizeof(LONG) + (ULONG)a->lLength)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN &&
            ps->ulDrawingMode != DM_DRAWANDRETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        /* Element type for a user-defined element must be in the range
           [0x81000000, 0xFFFFFFFF] (GpiBeginElement/GpiElement - Remarks). */
        if ((ULONG)a->lType < 0x81000000UL) return (ULONG)GPI_ERROR;

        /* Validate: no GBEL / GEEL in body. */
        off = 0;
        while (off < (ULONG)a->lLength) {
            ULONG cbO = GocaOrderLength(&ps->goca,
                                        a->abData + off,
                                        (ULONG)a->lLength - off);
            if (cbO == 0) return (ULONG)GPI_ERROR;
            if (a->abData[off] == OCODE_GBEL ||
                a->abData[off] == OCODE_GEEL) return (ULONG)GPI_ERROR;
            off += cbO;
        }

        cbGbel = GocaEmitBeginElement(&ps->goca, gbel, sizeof(gbel),
                                       a->lType, a->szDesc);
        if (cbGbel < 0) return (ULONG)GPI_ERROR;
        cbGeel = GocaEmitEndElement(&ps->goca, geel, sizeof(geel));
        if (cbGeel < 0) return (ULONG)GPI_ERROR;

        cbElem = (ULONG)cbGbel + (ULONG)a->lLength + (ULONG)cbGeel;
        if (DosAllocMem(&pvElem, cbElem,
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
            return (ULONG)GPI_ERROR;
        pElem = (PBYTE)pvElem;
        memcpy(pElem, gbel, (ULONG)cbGbel);
        if (a->lLength > 0)
            memcpy(pElem + cbGbel, a->abData, (ULONG)a->lLength);
        memcpy(pElem + cbGbel + a->lLength, geel, (ULONG)cbGeel);

        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd)) {
            DosFreeMem(pvElem);
            return (ULONG)GPI_ERROR;
        }
        if (!SrvGocaInsertElement(ps, segStart, segEnd, pElem, cbElem)) {
            DosFreeMem(pvElem);
            return (ULONG)GPI_ERROR;
        }
        DosFreeMem(pvElem);

        log("[PMSERVER] OUT : NGpiElement -> 1\n");
        return 1;
    }

    case NGpiQueryElementType: {
        PGpiQueryElementType_Args a = (PGpiQueryElementType_Args)pArgs;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        ULONG  s, e, bs, be;
        BOOL   hasGbel;
        LONG   lType;
        const char *pszDesc = "";
        ULONG  cchDesc = 0;
        ULONG  cbReq;

        log("[PMSERVER] IN  : NGpiQueryElementType(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < sizeof(*a)) return (ULONG)(LONG)GPI_ALTERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->bInElement)             return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->lCurrentElement <= 0)   return (ULONG)(LONG)GPI_ALTERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)(LONG)GPI_ALTERROR;
        if (!SrvFindElement(ps, segStart, segEnd, ps->lCurrentElement,
                            &s, &e, &bs, &be, &hasGbel))
            return (ULONG)(LONG)GPI_ALTERROR;

        if (hasGbel) {
            ULONG gbelLen = GocaOrderLength(&ps->goca, ps->pbGoca + s,
                                             segEnd - s);
            if (gbelLen < 6) return (ULONG)(LONG)GPI_ALTERROR;
            lType = (LONG)GocaGetU32(&ps->goca, ps->pbGoca + s + 2);
            cchDesc = gbelLen - 6;
            if (cchDesc > 0) pszDesc = (const char *)(ps->pbGoca + s + 6);
        } else {
            lType = (LONG)ps->pbGoca[s];
        }

        a->lType = lType;
        if (a->szData && a->lLength > 0) {
            ULONG n = cchDesc;
            if (n > (ULONG)(a->lLength - 1)) n = (ULONG)(a->lLength - 1);
            if (n > 0) memcpy(a->szData, pszDesc, n);
            a->szData[n] = '\0';
        }

        cbReq = be - bs;
        if (pcbPayload) *pcbPayload = cbArgs;
        log("[PMSERVER] OUT : NGpiQueryElementType -> %lu bytes\n",
            (unsigned long)cbReq);
        return (ULONG)cbReq;
    }

    case NGpiQueryElement: {
        PGpiQueryElement_Args a = (PGpiQueryElement_Args)pArgs;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        ULONG  s, e, bs, be;
        BOOL   hasGbel;
        ULONG  toCopy;

        log("[PMSERVER] IN  : NGpiQueryElement(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!a || cbArgs < 2 * sizeof(LONG)) return (ULONG)(LONG)GPI_ALTERROR;
        if (a->lOff < 0 || a->lMaxLength < 0) return (ULONG)(LONG)GPI_ALTERROR;
        if (cbArgs < 2 * sizeof(LONG) + (ULONG)a->lMaxLength)
            return (ULONG)(LONG)GPI_ALTERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->bInElement)             return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->lCurrentElement <= 0)   return (ULONG)(LONG)GPI_ALTERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)(LONG)GPI_ALTERROR;
        if (!SrvFindElement(ps, segStart, segEnd, ps->lCurrentElement,
                            &s, &e, &bs, &be, &hasGbel))
            return (ULONG)(LONG)GPI_ALTERROR;

        toCopy = be - bs;
        if ((ULONG)a->lOff >= toCopy) toCopy = 0;
        else toCopy -= (ULONG)a->lOff;
        if ((ULONG)a->lMaxLength < toCopy) toCopy = (ULONG)a->lMaxLength;
        if (toCopy > 0)
            memcpy(a->abData, ps->pbGoca + bs + (ULONG)a->lOff, toCopy);

        if (pcbPayload) *pcbPayload = cbArgs;
        log("[PMSERVER] OUT : NGpiQueryElement -> %lu bytes\n",
            (unsigned long)toCopy);
        return (ULONG)toCopy;
    }

    case NGpiDeleteElement: {
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        ULONG  s, e, bs, be;
        BOOL   hasGbel;

        log("[PMSERVER] IN  : NGpiDeleteElement(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        if (ps->lCurrentElement <= 0)   return (ULONG)GPI_ERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;
        if (!SrvFindElement(ps, segStart, segEnd, ps->lCurrentElement,
                            &s, &e, &bs, &be, &hasGbel))
            return (ULONG)GPI_ERROR;
        if (!GpiPsGocaDelete(ps, s, e)) return (ULONG)GPI_ERROR;
        ps->lCurrentElement--;

        log("[PMSERVER] OUT : NGpiDeleteElement -> 1\n");
        return 1;
    }

    case NGpiDeleteElementRange: {
        LONG lFirst, lLast, lTmp;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        ULONG  s1, e1, bs1, be1, s2, e2, bs2, be2;
        BOOL   has1, has2;

        log("[PMSERVER] IN  : NGpiDeleteElementRange(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lFirst, pArgs, sizeof(LONG));
        memcpy(&lLast,  pArgs + sizeof(LONG), sizeof(LONG));
        if (lFirst > lLast) { lTmp = lFirst; lFirst = lLast; lLast = lTmp; }
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        if (lFirst <= 0)                return (ULONG)GPI_ERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;
        if (!SrvFindElement(ps, segStart, segEnd, lFirst,
                            &s1, &e1, &bs1, &be1, &has1))
            return (ULONG)GPI_ERROR;
        if (!SrvFindElement(ps, segStart, segEnd, lLast,
                            &s2, &e2, &bs2, &be2, &has2))
            return (ULONG)GPI_ERROR;
        if (!GpiPsGocaDelete(ps, s1, e2)) return (ULONG)GPI_ERROR;
        ps->lCurrentElement = lFirst - 1;

        log("[PMSERVER] OUT : NGpiDeleteElementRange -> 1\n");
        return 1;
    }

    case NGpiDeleteElementsBetweenLabels: {
        LONG lFirst, lLast;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        LONG   idx;
        LONG   lIdxFirst = -1, lIdxLast = -1;
        ULONG  sFirst = 0, eFirst = 0, sLast = 0;

        log("[PMSERVER] IN  : NGpiDeleteElementsBetweenLabels(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        memcpy(&lFirst, pArgs, sizeof(LONG));
        memcpy(&lLast,  pArgs + sizeof(LONG), sizeof(LONG));
        if (lFirst < 0 || lLast < 0) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;

        /* Find first GLBL with lFirst (from element 1). */
        for (idx = 1; ; idx++) {
            ULONG s, e, bs, be;
            BOOL  hasGbel;
            if (!SrvFindElement(ps, segStart, segEnd, idx,
                                &s, &e, &bs, &be, &hasGbel))
                break;
            if (hasGbel) continue;
            if (ps->pbGoca[s] != OCODE_GLABL) continue;
            if (e - s < 6) continue;
            if ((LONG)GocaGetU32(&ps->goca, ps->pbGoca + s + 2) != lFirst)
                continue;
            lIdxFirst = idx;
            sFirst = s;
            eFirst = e;
            break;
        }
        if (lIdxFirst < 0) return (ULONG)GPI_ERROR;

        /* Find first GLBL with lLast after the first label. */
        for (idx = lIdxFirst + 1; ; idx++) {
            ULONG s, e, bs, be;
            BOOL  hasGbel;
            if (!SrvFindElement(ps, segStart, segEnd, idx,
                                &s, &e, &bs, &be, &hasGbel))
                break;
            if (hasGbel) continue;
            if (ps->pbGoca[s] != OCODE_GLABL) continue;
            if (e - s < 6) continue;
            if ((LONG)GocaGetU32(&ps->goca, ps->pbGoca + s + 2) != lLast)
                continue;
            lIdxLast = idx;
            sLast = s;
            break;
        }
        if (lIdxLast < 0) return (ULONG)GPI_ERROR;

        /* Delete all elements strictly between the two labels. */
        if (lIdxLast > lIdxFirst + 1) {
            if (!GpiPsGocaDelete(ps, eFirst, sLast))
                return (ULONG)GPI_ERROR;
        }

        /* Pointer to element immediately preceding the deleted ones. */
        ps->lCurrentElement = lIdxFirst;

        log("[PMSERVER] OUT : NGpiDeleteElementsBetweenLabels -> 1\n");
        return 1;
    }

    case NGpiQueryElementPointer: {
        PGPI_PS ps;
        log("[PMSERVER] IN  : NGpiQueryElementPointer(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)(LONG)GPI_ALTERROR;
        if (ps->bInElement)             return (ULONG)(LONG)GPI_ALTERROR;
        return (ULONG)ps->lCurrentElement;
    }

    case NGpiSetElementPointer: {
        LONG   lNew;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        LONG   n;

        log("[PMSERVER] IN  : NGpiSetElementPointer(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lNew = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;
        n = SrvCountElements(ps, segStart, segEnd);
        if (lNew < 0) lNew = 0;
        if (lNew > n) lNew = n;
        ps->lCurrentElement = lNew;
        return 1;
    }

    case NGpiOffsetElementPointer: {
        LONG   lDelta;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        LONG   n, lNew;

        log("[PMSERVER] IN  : NGpiOffsetElementPointer(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lDelta = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;
        n = SrvCountElements(ps, segStart, segEnd);
        lNew = ps->lCurrentElement + lDelta;
        if (lNew < 0) lNew = 0;
        if (lNew > n) lNew = n;
        ps->lCurrentElement = lNew;
        return 1;
    }

    case NGpiSetElementPointerAtLabel: {
        LONG   lLabel;
        PGPI_PS ps;
        ULONG  segStart, segEnd;
        LONG   idx;
        BOOL   bFound = FALSE;

        log("[PMSERVER] IN  : NGpiSetElementPointerAtLabel(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lLabel = *(LONG*)pArgs;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenSeg == 0)          return (ULONG)GPI_ERROR;
        if (ps->ulDrawingMode != DM_RETAIN) return (ULONG)GPI_ERROR;
        if (ps->bInElement)             return (ULONG)GPI_ERROR;
        if (!GpiPsGocaFindSeg(ps, ps->lOpenSeg, &segStart, &segEnd))
            return (ULONG)GPI_ERROR;

        for (idx = 1; ; idx++) {
            ULONG s, e, bs, be;
            BOOL  hasGbel;
            if (!SrvFindElement(ps, segStart, segEnd, idx,
                                &s, &e, &bs, &be, &hasGbel))
                break;
            if (idx <= ps->lCurrentElement) continue;
            if (hasGbel) continue;
            if (ps->pbGoca[s] != OCODE_GLABL) continue;
            if (e - s < 6) continue;
            if ((LONG)GocaGetU32(&ps->goca, ps->pbGoca + s + 2) != lLabel)
                continue;
            ps->lCurrentElement = idx;
            bFound = TRUE;
            break;
        }
        if (!bFound) return (ULONG)GPI_ERROR;
        return 1;
    }

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
    case NGpiBeginPath: {
        LONG lPath;
        PGPI_PS ps;
        BYTE buf[8];
        LONG len;

        log("[PMSERVER] IN  : NGpiBeginPath(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG)) return (ULONG)GPI_ERROR;
        lPath = *(LONG*)pArgs;
        if (lPath != 1) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath != 0) return (ULONG)GPI_ERROR; /* path state already active */

        len = GocaEmitBeginPath(&ps->goca, buf, sizeof(buf), lPath);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->lOpenPath    = lPath;
        ps->bPathDefined = FALSE;   /* new path replaces any previous one */
        return 1;
    }

    case NGpiEndPath: {
        PGPI_PS ps;
        BYTE buf[2];
        LONG len;

        log("[PMSERVER] IN  : NGpiEndPath(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath == 0) return (ULONG)GPI_ERROR;

        len = GocaEmitEndPath(&ps->goca, buf, sizeof(buf));
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->lOpenPath    = 0;
        ps->bPathDefined = TRUE;
        return 1;
    }

    case NGpiCloseFigure: {
        PGPI_PS ps;
        BYTE buf[2];
        LONG len;

        log("[PMSERVER] IN  : NGpiCloseFigure(hps=%p)\n", (void*)hps);
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath == 0) return (ULONG)GPI_ERROR;

        len = GocaEmitCloseFigure(&ps->goca, buf, sizeof(buf));
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        return 1;
    }

    case NGpiFillPath: {
        LONG lPath, lOptions;
        PGPI_PS ps;
        BYTE buf[8];
        LONG len;
        ULONG ulFlags = 0;

        log("[PMSERVER] IN  : NGpiFillPath(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        lPath    = *(LONG*)pArgs;
        lOptions = *(LONG*)(pArgs + sizeof(LONG));
        if (lPath != 1) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath != 0)     return (ULONG)GPI_ERROR; /* still in path state */
        if (!ps->bPathDefined)      return (ULONG)GPI_ERROR; /* no path defined     */

        if (lOptions & 0x0002L) ulFlags |= GFPTH_WINDING;   /* FPATH_WINDING */

        len = GocaEmitFillPath(&ps->goca, buf, sizeof(buf), ulFlags, lPath);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->bPathDefined = FALSE;
        return 1;
    }

    case NGpiOutlinePath: {
        LONG lPath;
        PGPI_PS ps;
        BYTE buf[8];
        LONG len;

        log("[PMSERVER] IN  : NGpiOutlinePath(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        lPath = *(LONG*)pArgs;
        if (lPath != 1) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath != 0)     return (ULONG)GPI_ERROR;
        if (!ps->bPathDefined)      return (ULONG)GPI_ERROR;

        len = GocaEmitOutlinePath(&ps->goca, buf, sizeof(buf), lPath);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->bPathDefined = FALSE;
        return 1;
    }

    case NGpiModifyPath: {
        LONG lPath, lMode;
        PGPI_PS ps;
        BYTE buf[8];
        LONG len;

        log("[PMSERVER] IN  : NGpiModifyPath(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        lPath = *(LONG*)pArgs;
        lMode = *(LONG*)(pArgs + sizeof(LONG));
        if (lPath != 1 || lMode != 6L /* MPATH_STROKE */)
            return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath != 0)     return (ULONG)GPI_ERROR;
        if (!ps->bPathDefined)      return (ULONG)GPI_ERROR;

        len = GocaEmitModifyPath(&ps->goca, buf, sizeof(buf),
                                  (UCHAR)GMPTH_STROKE, lPath);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        /* Path stays defined (modified); bPathDefined remains TRUE. */
        return 1;
    }

    case NGpiSetClipPath: {
        LONG lPath, lOptions;
        PGPI_PS ps;
        BYTE buf[8];
        LONG len;
        ULONG ulFlags = 0;

        log("[PMSERVER] IN  : NGpiSetClipPath(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < 2 * sizeof(LONG)) return (ULONG)GPI_ERROR;
        lPath    = *(LONG*)pArgs;
        lOptions = *(LONG*)(pArgs + sizeof(LONG));

        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;

        if (lPath == 0) {
            /* Reset clip path (infinite clip). Works regardless of path state. */
            len = GocaEmitSetClipPath(&ps->goca, buf, sizeof(buf), 0, 0);
            if (len < 0) return (ULONG)GPI_ERROR;
            SrvDeliver(ps, buf, (ULONG)len);
            return 1;
        }

        if (lPath != 1)         return (ULONG)GPI_ERROR;
        if (ps->lOpenPath != 0) return (ULONG)GPI_ERROR;
        if (!ps->bPathDefined)  return (ULONG)GPI_ERROR;

        if (lOptions & 0x0002L) ulFlags |= GSCPTH_WINDING;    /* SCP_WINDING */
        if (lOptions & 0x0004L) ulFlags |= GSCPTH_INTERSECT;  /* SCP_AND */

        len = GocaEmitSetClipPath(&ps->goca, buf, sizeof(buf),
                                   ulFlags, lPath);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->bPathDefined = FALSE;   /* consumed */
        return 1;
    }

    case NGpiStrokePath: {
        LONG lPath;
        ULONG flOptions;
        PGPI_PS ps;
        BYTE buf[8];
        LONG len;

        log("[PMSERVER] IN  : NGpiStrokePath(hps=%p, cbArgs=%u)\n",
            (void*)hps, (unsigned)cbArgs);
        if (!pArgs || cbArgs < sizeof(LONG) + sizeof(ULONG))
            return (ULONG)GPI_ERROR;
        lPath     = *(LONG*)pArgs;
        flOptions = *(ULONG*)(pArgs + sizeof(LONG));
        if (lPath != 1 || flOptions != 0) return (ULONG)GPI_ERROR;
        ps = GpiPsGet(hps);
        if (!ps) return (ULONG)GPI_ERROR;
        if (ps->lOpenPath != 0)     return (ULONG)GPI_ERROR;
        if (!ps->bPathDefined)      return (ULONG)GPI_ERROR;

        len = GocaEmitFillPath(&ps->goca, buf, sizeof(buf),
                                GFPTH_WINDING | GFPTH_MODIFY, lPath);
        if (len < 0) return (ULONG)GPI_ERROR;
        SrvDeliver(ps, buf, (ULONG)len);
        ps->bPathDefined = FALSE;
        return 1;
    }
    DISPATCH_STUB(NGpiPathToRegion)
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
