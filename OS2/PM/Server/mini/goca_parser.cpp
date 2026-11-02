/*!
 * @file goca_parser.cpp
 * @brief GOCA drawing-order stream parser, dispatched through PMGRE.
 *
 * All calls go through <pmddim.h> macros (Gre*).  Attribute masks and
 * bundle field names come from <pmgpi.h>.  Order codes and structures
 * come from <pmord.h>.
 */

#define INCL_GREALL
#define INCL_PM
#include <F_platform.hpp>
#include "pmddi.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "goca_parser.hpp"

/* ======================================================================== */
/*  Debug trace                                                             */
/* ======================================================================== */

/*!
 * @brief Conditional trace to stderr.
 * @param [in] c   Parser context (checks c->bDebug).
 * @param [in] fmt printf-style format string.
 * @param [in] ... Arguments for the format string.
 */
static void GocaTrace(GOCA_CTX *c, const char *fmt, ...)
{
    va_list args;
    if (!c || !c->bDebug) return;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

/* ======================================================================== */
/*  Big-endian readers                                                      */
/* ======================================================================== */

/*! @brief Read signed 16-bit BE. */
static SHORT RD_S16(const BYTE *p) { return (SHORT)(((USHORT)p[0]<<8)|p[1]); }

/*! @brief Read unsigned 16-bit BE. */
static USHORT RD_U16(const BYTE *p) { return (USHORT)(((USHORT)p[0]<<8)|p[1]); }

/*! @brief Read signed 32-bit BE. */
static LONG RD_S32(const BYTE *p)
{
    return ((LONG)p[0]<<24)|((LONG)p[1]<<16)|((LONG)p[2]<<8)|(LONG)p[3];
}

/*! @brief Read unsigned 32-bit BE. */
static ULONG RD_U32(const BYTE *p)
{
    return ((ULONG)p[0]<<24)|((ULONG)p[1]<<16)|((ULONG)p[2]<<8)|(ULONG)p[3];
}

/* ======================================================================== */
/*  Order length                                                            */
/* ======================================================================== */

/*!
 * @brief Wire length of the order starting at p (0 on truncation).
 *
 * Format predicates are from pmord.h:
 *   BYTE_ORDER(oc)   — 1 byte
 *   SHORT_ORDER(oc)  — 2 bytes
 *   oc == OCODE_VLONG — 4 + LE16 from p+2
 *   otherwise        — long, 2 + p[1]
 */
static ULONG OrderLength(const BYTE *p, ULONG cbAvail)
{
    UCHAR oc;
    if (cbAvail < 1) return 0;
    oc = p[0];
    if (BYTE_ORDER(oc))  return 1;
    if (oc == OCODE_VLONG) {
        if (cbAvail < 4) return 0;
        return 4u + RD_U16(p + 2);
    }
    if (SHORT_ORDER(oc)) return 2;
    if (cbAvail < 2) return 0;
    return 2u + (ULONG)p[1];
}

/* ======================================================================== */
/*  Small helpers                                                           */
/* ======================================================================== */

/*! @brief GOCA POINTS pair -> POINTL. */
static POINTL PtsToPtl(const BYTE *p)
{
    POINTL pt;
    pt.x = RD_S16(p);
    pt.y = RD_S16(p + 2);
    return pt;
}

/*! @brief Allocate and fill POINTL[] from packed BE SHORT pairs. */
static PPOINTL AllocPtlArray(const BYTE *pData, ULONG nPts)
{
    PPOINTL pptl = (PPOINTL)malloc(nPts * sizeof(POINTL));
    ULONG   i;
    if (!pptl) return NULL;
    for (i = 0; i < nPts; i++) {
        pptl[i].x = RD_S16(pData + i * 4);
        pptl[i].y = RD_S16(pData + i * 4 + 2);
    }
    return pptl;
}

/*! @brief Refresh cached current position from the device. */
static void RefreshCurPos(GOCA_CTX *c)
{
    GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
}

/*!
 * @brief Apply an attribute mask to one primitive bundle.
 * @param hdc       Target DC.
 * @param lPrimType PRIM_LINE / PRIM_CHAR / PRIM_MARKER / PRIM_AREA.
 * @param flAttrs   Mask of LBB_, CBB_, MBB_, ABB_ fields to set.
 * @param pBundle   Bundle pointer.
 * @return TRUE on success.
 */
static BOOL GocaSetAttr(HDC hdc, LONG lPrimType, ULONG flAttrs, PVOID pBundle)
{
    return GreSetAttributes(hdc, (ULONG)lPrimType, 0UL, flAttrs,
                            (PBUNDLE)pBundle);
}

/* ======================================================================== */
/*  Long-order handlers                                                     */
/* ======================================================================== */

/*! @brief GSCP (0x21) — Set Current Position. Data: 4 bytes (x,y). */
static LONG DoGSCP(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    if (cb < 6) return GOCA_ERR_TRUNC;
    pt = PtsToPtl(p + 2);
    GocaTrace(c, "[GOCA]   GSCP: pos=(%ld,%ld)\n",
              (long)pt.x, (long)pt.y);
    GreSetCurrentPosition(c->hdc, &pt);
    c->ptlCurPos = pt;
    return GOCA_OK;
}

/*! @brief GLINE (0xC1) / GCLINE (0x81). */
static LONG DoLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nTotal, nPts, i;
    PPOINTL pptl;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    if (bCur) {
        /* GCLINE: length is 4 * count; prepend current position. */
        if (len == 0 || (len & 3)) return GOCA_ERR_FORMAT;
        nPts   = len / 4;
        nTotal = nPts + 1;
        pptl   = (PPOINTL)malloc(nTotal * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {
        /* GLINE: first point is 4 bytes, rest 4 bytes each. */
        if (len < 4 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nTotal = (len - 4) / 4 + 1;
        pptl   = AllocPtlArray(p + 2, nTotal);
        if (!pptl) return GOCA_ERR_NOMEM;
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCLINE" : "GLINE", (unsigned)nTotal,
              (long)pptl[nTotal-1].x, (long)pptl[nTotal-1].y);

    GrePolyLine(c->hdc, pptl, (LONG)nTotal);
    c->ptlCurPos = pptl[nTotal - 1];
    free(pptl);
    return GOCA_OK;
}

/*! @brief GMRK (0xC2) / GCMRK (0x82). */
static LONG DoMark(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nTotal, nPts, i;
    PPOINTL pptl;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    if (bCur) {
        if (len == 0 || (len & 3)) return GOCA_ERR_FORMAT;
        nPts   = len / 4;
        nTotal = nPts + 1;
        pptl   = (PPOINTL)malloc(nTotal * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {
        if (len < 4 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nTotal = (len - 4) / 4 + 1;
        pptl   = AllocPtlArray(p + 2, nTotal);
        if (!pptl) return GOCA_ERR_NOMEM;
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCMRK" : "GMRK", (unsigned)nTotal,
              (long)pptl[nTotal-1].x, (long)pptl[nTotal-1].y);

    GrePolyMarker(c->hdc, pptl, (LONG)nTotal);
    c->ptlCurPos = pptl[nTotal - 1];
    free(pptl);
    return GOCA_OK;
}

/*!
 * @brief GRLINE (0xE1) / GCRLINE (0xA1) — relative line with SBIN offsets.
 *
 * Layout from p:
 *   0     code
 *   1     length (bytes following)
 *   2..5  (GRLINE only) start x,y as 2 x SHORT BE
 *   ..    pairs of 1-byte SBIN (dx, dy), each pair advances one point
 *
 * For GCRLINE the start point is the current position; for GRLINE the
 * first 4 bytes are given explicitly.
 */
static LONG DoRelativeLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, nTotal, i, off;
    PPOINTL pptl;
    SHORT   x, y;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    off = 0;
    if (bCur) {
        /* GCRLINE: length is 2 * count. */
        if (len == 0 || (len & 1)) return GOCA_ERR_FORMAT;
        RefreshCurPos(c);
        x = (SHORT)c->ptlCurPos.x;
        y = (SHORT)c->ptlCurPos.y;
        off = 0;
    } else {
        /* GRLINE: 4-byte start point, then 2 * count. */
        if (len < 4 || ((len - 4) & 1)) return GOCA_ERR_FORMAT;
        x = RD_S16(p + 2);
        y = RD_S16(p + 4);
        off = 4;
    }

    nPts   = (len - off) / 2;
    nTotal = nPts + 1;
    pptl   = (PPOINTL)malloc(nTotal * sizeof(POINTL));
    if (!pptl) return GOCA_ERR_NOMEM;

    pptl[0].x = x;
    pptl[0].y = y;

    for (i = 0; i < nPts; i++) {
        CHAR dx = (CHAR)p[2 + off + i * 2];
        CHAR dy = (CHAR)p[2 + off + i * 2 + 1];
        x = (SHORT)(x + dx);
        y = (SHORT)(y + dy);
        pptl[i + 1].x = x;
        pptl[i + 1].y = y;
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCRLINE" : "GRLINE", (unsigned)nTotal,
              (long)pptl[nTotal-1].x, (long)pptl[nTotal-1].y);

    GrePolyLine(c->hdc, pptl, (LONG)nTotal);
    c->ptlCurPos = pptl[nTotal - 1];
    free(pptl);
    return GOCA_OK;
}

/*! @brief GCHST (0xC3) / GCCHST (0x83). */
static LONG DoCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, cch;
    POINTL      ptStart;
    const BYTE *pStr;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    if (bCur) {
        /* GCCHST: no explicit point, string at current pos. */
        RefreshCurPos(c);
        ptStart = c->ptlCurPos;
        pStr    = p + 2;
        cch     = len;
    } else {
        /* GCHST: 4-byte point, then string. */
        if (len < 4) return GOCA_ERR_FORMAT;
        ptStart = PtsToPtl(p + 2);
        pStr    = p + 6;
        cch     = len - 4;
    }

    if (cch == 0) return GOCA_OK;

    GocaTrace(c, "[GOCA]   %s: %u chars at (%ld,%ld)\n",
              bCur ? "GCCHST" : "GCHST", (unsigned)cch,
              (long)ptStart.x, (long)ptStart.y);

    GreSetCurrentPosition(c->hdc, &ptStart);
    GreCharString(c->hdc, (LONG)cch, (PCH)pStr);
    c->ptlCurPos = ptStart;
    return GOCA_OK;
}

/*!
 * @brief GBOX (0xC0) / GCBOX (0x80).
 *
 * GOCA carries two corners (or current pos + one corner for GCBOX).
 * BOXPARAMS.ptl is the opposite corner; sizl.cx/cy are the H/V
 * corner-rounding radii (same as GpiBox's lHRound/lVRound).
 * GreBox* draws from current position to ptl.
 */
static LONG DoBox(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG     len;
    UCHAR     fbFlags;
    BOOL      bBoundary, bFill;
    SHORT     x1, y1, x2, y2, hAxis = 0, vAxis = 0;
    BOXPARAMS bp;
    POINTL    ptStart;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;
    if (len < 2) return GOCA_ERR_FORMAT;
    fbFlags = p[2];

    if (bCur) {
        /* GCBOX: corner (x2,y2) at p[4..7], optional axes at p[8..11]. */
        if (len < 6) return GOCA_ERR_FORMAT;
        RefreshCurPos(c);
        x1 = (SHORT)c->ptlCurPos.x;
        y1 = (SHORT)c->ptlCurPos.y;
        x2 = RD_S16(p + 4);
        y2 = RD_S16(p + 6);
        if (len >= 10) { hAxis = RD_S16(p + 8); vAxis = RD_S16(p + 10); }
    } else {
        /* GBOX: both corners, optional axes. */
        if (len < 10) return GOCA_ERR_FORMAT;
        x1 = RD_S16(p + 4); y1 = RD_S16(p + 6);
        x2 = RD_S16(p + 8); y2 = RD_S16(p + 10);
        if (len >= 14) { hAxis = RD_S16(p + 12); vAxis = RD_S16(p + 14); }
    }

    /* GreBox* uses current position as one corner. */
    ptStart.x = x1; ptStart.y = y1;
    GreSetCurrentPosition(c->hdc, &ptStart);

    bp.ptl.x   = x2;
    bp.ptl.y   = y2;
    bp.sizl.cx = (LONG)hAxis;
    bp.sizl.cy = (LONG)vAxis;

    bBoundary = (fbFlags & 0x40) != 0;
    bFill     = (fbFlags & 0x20) != 0;

    GocaTrace(c, "[GOCA]   %s: (%d,%d)-(%d,%d) fill=%d bound=%d ax=(%d,%d)\n",
              bCur ? "GCBOX" : "GBOX", x1, y1, x2, y2,
              bFill, bBoundary, hAxis, vAxis);

    if      (bBoundary && bFill) GreBoxBoth    (c->hdc, &bp);
    else if (bBoundary)          GreBoxBoundary(c->hdc, &bp);
    else if (bFill)              GreBoxInterior(c->hdc, &bp);

    c->ptlCurPos.x = x2;
    c->ptlCurPos.y = y2;
    return GOCA_OK;
}

/*!
 * @brief GPARC (0xE3) / GCPARC (0xA3).
 *
 * Layout (GCPARC): centre x,y (SHORT), multiplier FIXED88,
 *                  start angle 32-bit, sweep angle 32-bit.
 * GPARC additionally has a 4-byte start point at the front (ignored here).
 *
 * FIXED88 (2 bytes) is scaled up to FIXED (16.16) by shifting left 8 bits.
 * 32-bit angles are already in 1/65536-of-turn units, which is the OS/2
 * FIXED angle convention — passed straight through.
 */
static LONG DoPartialArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, off;
    const BYTE *q;
    POINTL      ptCenter;
    FIXED       fxMult, fxStart, fxSweep;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    q   = p + 2;
    off = 0;
    if (!bCur) off += 4;                    /* GPARC: skip start x,y */

    if (len < off + 4 + 2 + 4 + 4) return GOCA_ERR_FORMAT;

    ptCenter.x = RD_S16(q + off); off += 2;
    ptCenter.y = RD_S16(q + off); off += 2;
    fxMult     = (FIXED)((ULONG)RD_U16(q + off) << 8); off += 2;
    fxStart    = (FIXED)RD_U32(q + off); off += 4;
    fxSweep    = (FIXED)RD_U32(q + off);

    GocaTrace(c, "[GOCA]   %s: c=(%ld,%ld) mult=%04X start=%08lX sweep=%08lX\n",
              bCur ? "GCPARC" : "GPARC",
              (long)ptCenter.x, (long)ptCenter.y,
              (unsigned)fxMult,
              (unsigned long)fxStart, (unsigned long)fxSweep);

    GrePartialArc(c->hdc, &ptCenter, fxMult, fxStart, fxSweep);
    return GOCA_OK;
}

/*!
 * @brief GFARC (0xC7) / GCFARC (0x87).
 *
 * Data layout:
 *   GCFARC: multiplier FIXED88            (2 bytes)
 *   GFARC:  start x,y (SHORT each), then multiplier FIXED88
 */
static LONG DoFullArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, off;
    const BYTE *q;
    FIXED       fxMult;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    q   = p + 2;
    off = 0;
    if (!bCur) off += 4;                    /* GFARC: skip start x,y */

    if (len < off + 2) return GOCA_ERR_FORMAT;

    fxMult = (FIXED)((ULONG)RD_U16(q + off) << 8);

    GocaTrace(c, "[GOCA]   %s: mult=%04X\n",
              bCur ? "GCFARC" : "GFARC", (unsigned)fxMult);

    GreFullArcBoth(c->hdc, fxMult);
    return GOCA_OK;
}

/*! @brief GBEZ (0xE5) / GCBEZ (0xA5). */
static LONG DoBezier(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, i;
    PPOINTL pptl;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    if (bCur) {
        if (len == 0 || (len & 3)) return GOCA_ERR_FORMAT;
        nPts = len / 4 + 1;
        pptl = (PPOINTL)malloc(nPts * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts - 1; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {
        if (len < 8 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nPts = (len - 4) / 4 + 1;
        pptl = AllocPtlArray(p + 2, nPts);
        if (!pptl) return GOCA_ERR_NOMEM;
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCBEZ" : "GBEZ", (unsigned)nPts,
              (long)pptl[nPts-1].x, (long)pptl[nPts-1].y);

    GrePolySpline(c->hdc, pptl, (LONG)nPts);
    c->ptlCurPos = pptl[nPts - 1];
    free(pptl);
    return GOCA_OK;
}

/*! @brief GFLT (0xC5) / GCFLT (0x85). */
static LONG DoFillet(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, i;
    PPOINTL pptl;

    len = p[1];
    if (cb < 2u + len) return GOCA_ERR_TRUNC;

    if (bCur) {
        if (len == 0 || (len & 3)) return GOCA_ERR_FORMAT;
        nPts = len / 4 + 1;
        pptl = (PPOINTL)malloc(nPts * sizeof(POINTL));
        if (!pptl) return GOCA_ERR_NOMEM;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts - 1; i++) {
            pptl[i+1].x = RD_S16(p + 2 + i*4);
            pptl[i+1].y = RD_S16(p + 2 + i*4 + 2);
        }
    } else {
        if (len < 4 || ((len - 4) & 3)) return GOCA_ERR_FORMAT;
        nPts = (len - 4) / 4 + 1;
        pptl = AllocPtlArray(p + 2, nPts);
        if (!pptl) return GOCA_ERR_NOMEM;
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCFLT" : "GFLT", (unsigned)nPts,
              (long)pptl[nPts-1].x, (long)pptl[nPts-1].y);

    GrePolyFillet(c->hdc, pptl, (LONG)nPts);
    c->ptlCurPos = pptl[nPts - 1];
    free(pptl);
    return GOCA_OK;
}

/*! @brief GSAP (0x22) — Set Arc Parameters. Data: 4 x SHORT (P,Q,R,S). */
static LONG DoSetArcParams(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    ARCPARAMS ap;
    if (cb < 2u + 8) return GOCA_ERR_TRUNC;
    if (p[1] < 8) return GOCA_ERR_FORMAT;
    ap.lP = RD_S16(p + 2);
    ap.lQ = RD_S16(p + 4);
    ap.lR = RD_S16(p + 6);
    ap.lS = RD_S16(p + 8);
    GocaTrace(c, "[GOCA]   GSAP: P=%ld Q=%ld R=%ld S=%ld\n",
              (long)ap.lP, (long)ap.lQ, (long)ap.lR, (long)ap.lS);
    GreSetArcParameters(c->hdc, &ap);
    return GOCA_OK;
}

/*! @brief GSCC (0x33) — Set Character Cell. */
static LONG DoSetCharCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    SHORT  cxInt, cyInt;
    USHORT cxFrac, cyFrac;
    UCHAR  fbFlags;
    if (cb < 2u + 10) return GOCA_ERR_TRUNC;
    if (p[1] < 10) return GOCA_ERR_FORMAT;
    cxInt  = RD_S16(p + 2);
    cyInt  = RD_S16(p + 4);
    cxFrac = RD_U16(p + 6);
    cyFrac = RD_U16(p + 8);
    fbFlags = p[10];
    (void)fbFlags;
    GocaTrace(c, "[GOCA]   GSCC: cell=(%d.%04X,%d.%04X)\n",
              (int)cxInt, (unsigned)cxFrac, (int)cyInt, (unsigned)cyFrac);
    c->cb.sizfxCell.cx = MAKEFIXED(cxInt, cxFrac);
    c->cb.sizfxCell.cy = MAKEFIXED(cyInt, cyFrac);
    GocaSetAttr(c->hdc, PRIM_CHAR, CBB_BOX, &c->cb);
    return GOCA_OK;
}

/*! @brief GSMC (0x37) — Set Marker Cell. */
static LONG DoSetMarkerCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    SHORT  cx, cy;
    if (cb < 2u + 6) return GOCA_ERR_TRUNC;
    if (p[1] < 6) return GOCA_ERR_FORMAT;
    cx = RD_S16(p + 2);
    cy = RD_S16(p + 4);
    GocaTrace(c, "[GOCA]   GSMC: cell=(%d,%d)\n", (int)cx, (int)cy);
    c->mb.sizfxCell.cx = MAKEFIXED(cx, 0);
    c->mb.sizfxCell.cy = MAKEFIXED(cy, 0);
    GocaSetAttr(c->hdc, PRIM_MARKER, MBB_BOX, &c->mb);
    return GOCA_OK;
}

/*! @brief GSPRP (0xA0) — Set Pattern Reference Point. */
static LONG DoSetPatternRefPoint(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    UCHAR  fbFlags;
    if (cb < 2u + 6) return GOCA_ERR_TRUNC;
    if (p[1] < 6) return GOCA_ERR_FORMAT;
    fbFlags = p[2];
    pt = PtsToPtl(p + 4);
    GocaTrace(c, "[GOCA]   GSPRP: ref=(%ld,%ld) flags=%02X\n",
              (long)pt.x, (long)pt.y, (unsigned)fbFlags);
    c->ab.ptlRefPoint = pt;
    GocaSetAttr(c->hdc, PRIM_AREA, ABB_REF_POINT, &c->ab);
    return GOCA_OK;
}

/*!
 * @brief GCCHSTE (0xB0) / GCHSTE (0xF0) — very long character string.
 *
 * Layout from p (VLONG): code X'FE', qualifier, length(2), then:
 *   data[0]      fbFlags
 *   data[1]      reserved
 *   data[2..5]   ptRect[0] (start point)   — SHORT x, SHORT y
 *   data[6..9]   ptRect[1] (clip corner)   — SHORT x, SHORT y
 *   data[10..11] cchString                 — BE USHORT
 *   data[12..]   string
 */
static LONG DoVLongCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bGiven)
{
    const BYTE *q;
    UCHAR       fbFlags;
    USHORT      cch;
    const BYTE *pStr;
    POINTL      ptStart;
    POINTL      ptCorner0, ptCorner1;
    RECTL       rcl;
    ULONG       fl;

    if (cb < 16) return GOCA_ERR_TRUNC;
    q       = p + 4;
    fbFlags = q[0];

    ptCorner0 = PtsToPtl(q + 2);
    ptCorner1 = PtsToPtl(q + 6);

    if (bGiven) {
        ptStart = ptCorner0;
    } else {
        RefreshCurPos(c);
        ptStart = c->ptlCurPos;
    }

    rcl.xLeft   = (ptCorner0.x < ptCorner1.x) ? ptCorner0.x : ptCorner1.x;
    rcl.yBottom = (ptCorner0.y < ptCorner1.y) ? ptCorner0.y : ptCorner1.y;
    rcl.xRight  = (ptCorner0.x > ptCorner1.x) ? ptCorner0.x : ptCorner1.x;
    rcl.yTop    = (ptCorner0.y > ptCorner1.y) ? ptCorner0.y : ptCorner1.y;

    cch = RD_U16(q + 10);
    if (cb < 16u + cch) return GOCA_ERR_TRUNC;
    pStr = q + 12;
    if (cch == 0) return GOCA_OK;

    fl = 0;
    if (fbFlags & 0x40) fl |= CHS_CLIP;

    GocaTrace(c, "[GOCA]   %s: %u chars at (%ld,%ld) clip=(%ld,%ld)-(%ld,%ld)\n",
              bGiven ? "GCHSTE" : "GCCHSTE", (unsigned)cch,
              (long)ptStart.x, (long)ptStart.y,
              (long)rcl.xLeft, (long)rcl.yBottom,
              (long)rcl.xRight, (long)rcl.yTop);

    GreCharStringPos(c->hdc, &ptStart, &rcl, fl, (LONG)cch,
                     (PCH)pStr, (PLONG)NULL, (PCSP_INFO)NULL);
    c->ptlCurPos = ptStart;
    return GOCA_OK;
}

/* ======================================================================== */
/*  Initialisation                                                          */
/* ======================================================================== */

void GocaInit(GOCA_CTX *c, HDC hdc, PBYTE pb, ULONG cb)
{
    memset(c, 0, sizeof(*c));
    c->hdc      = hdc;
    c->pbStream = pb;
    c->cbStream = cb;
    c->bDebug   = TRUE;

    GocaTrace(c, "[GOCA] init: %u bytes\n", (unsigned)cb);

    /* Pull live bundles so untouched attributes keep the DC defaults. */
    GreGetAttributes(hdc, PRIM_LINE,
        LBB_COLOR|LBB_BACK_COLOR|LBB_MIX_MODE|LBB_BACK_MIX_MODE|
        LBB_WIDTH|LBB_GEOM_WIDTH|LBB_TYPE|LBB_END|LBB_JOIN,
        (PBUNDLE)&c->lb);
    GreGetAttributes(hdc, PRIM_CHAR,
        CBB_COLOR|CBB_BACK_COLOR|CBB_MIX_MODE|CBB_BACK_MIX_MODE|
        CBB_SET|CBB_MODE|CBB_BOX|CBB_ANGLE|CBB_SHEAR|
        CBB_DIRECTION|CBB_TEXT_ALIGN|CBB_EXTRA|CBB_BREAK_EXTRA,
        (PBUNDLE)&c->cb);
    GreGetAttributes(hdc, PRIM_MARKER,
        MBB_COLOR|MBB_BACK_COLOR|MBB_MIX_MODE|MBB_BACK_MIX_MODE|
        MBB_SET|MBB_SYMBOL|MBB_BOX,
        (PBUNDLE)&c->mb);
    GreGetAttributes(hdc, PRIM_AREA,
        ABB_COLOR|ABB_BACK_COLOR|ABB_MIX_MODE|ABB_BACK_MIX_MODE|
        ABB_SET|ABB_SYMBOL|ABB_REF_POINT,
        (PBUNDLE)&c->ab);

    RefreshCurPos(c);
}

/* ======================================================================== */
/*  Main loop                                                               */
/* ======================================================================== */

LONG GocaParseStream(GOCA_CTX *c)
{
    c->uPos   = 0;
    c->lError = GOCA_OK;

    GocaTrace(c, "[GOCA] ===== stream start: %u bytes =====\n",
              (unsigned)c->cbStream);

    while (c->uPos < c->cbStream) {
        const BYTE *p       = c->pbStream + c->uPos;
        ULONG       cbAvail = c->cbStream - c->uPos;
        ULONG       cbOrder = OrderLength(p, cbAvail);
        UCHAR       oc;
        LONG        rc;
        BOOL        bHandled = FALSE;

        if (cbOrder == 0 || cbOrder > cbAvail) {
            GocaTrace(c, "[GOCA] @%04X truncation: cbAvail=%u\n",
                      (unsigned)c->uPos, (unsigned)cbAvail);
            c->lError     = GOCA_ERR_TRUNC;
            c->uErrOffset = c->uPos;
            c->uErrOrder  = p[0];
            return c->lError;
        }
        oc = p[0];
        rc = GOCA_OK;

        GocaTrace(c, "[GOCA] @%04X %-8s (len=%u)\n",
                  (unsigned)c->uPos, GocaOrderName(oc), (unsigned)cbOrder);

        if (c->pfnHandler)
            bHandled = c->pfnHandler(c, oc, p, cbOrder);

        if (!bHandled) {
            switch (oc) {

            /* ============ 1/2-byte trivial ============ */
            case OCODE_GNOP1:
            case OCODE_GESD:
            case OCODE_GEPROL:
            case OCODE_GCOMT:
            case OCODE_GSGCH:
            case OCODE_GSMP:
                break;

            /* ============ colour (2-byte GSCOL / long GSECOL) ============ */
            case OCODE_GSCOL: {
                COLOR clr = (COLOR)p[1];
                GocaTrace(c, "[GOCA]   GSCOL: color=%u\n", (unsigned)clr);
                c->lb.lColor = c->cb.lColor = c->mb.lColor =
                               c->ab.lColor = (LONG)clr;
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_COLOR, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_COLOR, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_COLOR, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_COLOR, &c->ab);
                break;
            }

            case OCODE_GSECOL: {
                USHORT raw;
                COLOR  clr;
                if (p[1] < 2) { rc = GOCA_ERR_FORMAT; break; }
                raw = RD_U16(p + 2);
                GocaTrace(c, "[GOCA]   GSECOL: color=%04X\n", (unsigned)raw);
                clr = ((raw & 0xFF00) == 0xFF00)
                      ? (COLOR)(raw & 0x00FF)
                      : (COLOR)raw;
                c->lb.lColor = c->cb.lColor = c->mb.lColor =
                               c->ab.lColor = (LONG)clr;
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_COLOR, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_COLOR, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_COLOR, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_COLOR, &c->ab);
                break;
            }

            /* ============ mix ============ */
            case OCODE_GSMX:
                GocaTrace(c, "[GOCA]   GSMX: mix=%u\n", (unsigned)p[1]);
                c->lb.usMixMode = c->cb.usMixMode =
                c->mb.usMixMode = c->ab.usMixMode = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_MIX_MODE, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_MIX_MODE, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_MIX_MODE, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_MIX_MODE, &c->ab);
                break;

            case OCODE_GSBMX:
                GocaTrace(c, "[GOCA]   GSBMX: backmix=%u\n", (unsigned)p[1]);
                c->lb.usBackMixMode = c->cb.usBackMixMode =
                c->mb.usBackMixMode = c->ab.usBackMixMode = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_BACK_MIX_MODE, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_BACK_MIX_MODE, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_BACK_MIX_MODE, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_BACK_MIX_MODE, &c->ab);
                break;

            /* ============ line attributes ============ */
            case OCODE_GSLT:
                GocaTrace(c, "[GOCA]   GSLT: type=%u\n", (unsigned)p[1]);
                c->lb.usType = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE, LBB_TYPE, &c->lb);
                break;

            case OCODE_GSLW: {
                FIXED fx = MAKEFIXED(p[1], 0);
                GocaTrace(c, "[GOCA]   GSLW: width=%u\n", (unsigned)p[1]);
                c->lb.fxWidth    = fx;
                c->lb.lGeomWidth = (LONG)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE,
                            LBB_WIDTH | LBB_GEOM_WIDTH, &c->lb);
                break;
            }

            case OCODE_GSFLW: {
                UCHAR ip, fp;
                FIXED fx;
                if (p[1] < 2) { rc = GOCA_ERR_FORMAT; break; }
                ip = p[2]; fp = p[3];
                GocaTrace(c, "[GOCA]   GSFLW: width=%u.%02X\n",
                          (unsigned)ip, (unsigned)fp);
                fx = MAKEFIXED(ip, (ULONG)fp << 8);
                c->lb.fxWidth    = fx;
                c->lb.lGeomWidth = (LONG)ip;
                GocaSetAttr(c->hdc, PRIM_LINE,
                            LBB_WIDTH | LBB_GEOM_WIDTH, &c->lb);
                break;
            }

            case OCODE_GSLE:
                GocaTrace(c, "[GOCA]   GSLE: end=%u\n", (unsigned)p[1]);
                c->lb.usEnd = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE, LBB_END, &c->lb);
                break;

            case OCODE_GSLJ:
                GocaTrace(c, "[GOCA]   GSLJ: join=%u\n", (unsigned)p[1]);
                c->lb.usJoin = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE, LBB_JOIN, &c->lb);
                break;

            /* ============ marker attributes ============ */
            case OCODE_GSMS:
                GocaTrace(c, "[GOCA]   GSMS: set=%u\n", (unsigned)p[1]);
                c->mb.usSet = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_SET, &c->mb);
                break;

            case OCODE_GSMT:
                GocaTrace(c, "[GOCA]   GSMT: symbol=%u\n", (unsigned)p[1]);
                c->mb.usSymbol = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_SYMBOL, &c->mb);
                break;

            /* ============ area attributes ============ */
            case OCODE_GSPS:
                GocaTrace(c, "[GOCA]   GSPS: set=%u\n", (unsigned)p[1]);
                c->ab.usSet = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_AREA, ABB_SET, &c->ab);
                break;

            case OCODE_GSPT:
                GocaTrace(c, "[GOCA]   GSPT: symbol=%u\n", (unsigned)p[1]);
                c->ab.usSymbol = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_AREA, ABB_SYMBOL, &c->ab);
                break;

            /* ============ char attributes ============ */
            case OCODE_GSCS:
                GocaTrace(c, "[GOCA]   GSCS: lcid=%u\n", (unsigned)p[1]);
                c->cb.usSet = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_CHAR, CBB_SET, &c->cb);
                break;

            case OCODE_GSCD:
                GocaTrace(c, "[GOCA]   GSCD: dir=%u\n", (unsigned)p[1]);
                c->cb.usDirection = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_CHAR, CBB_DIRECTION, &c->cb);
                break;

            case OCODE_GSCR:
                GocaTrace(c, "[GOCA]   GSCR: prec=%u\n",
                          (unsigned)(p[1] & GSCR_PRECISION));
                c->cb.usPrecision = (USHORT)(p[1] & GSCR_PRECISION);
                GocaSetAttr(c->hdc, PRIM_CHAR, CBB_MODE, &c->cb);
                break;

            /* ============ areas ============ */
            case OCODE_GBAR: {
                ULONG fl = 0;
                GocaTrace(c, "[GOCA]   GBAR: flags=%02X\n", (unsigned)p[1]);
                c->uAreaFlags = p[1];
                c->bInArea    = TRUE;
                if (p[1] & 0x40) fl |= BA_BOUNDARY; else fl |= BA_NOBOUNDARY;
                if (p[1] & 0x20) fl |= BA_WINDING;  else fl |= BA_ALTERNATE;
                GreBeginArea(c->hdc, fl);
                break;
            }

            case OCODE_GEAR:
                GocaTrace(c, "[GOCA]   GEAR\n");
                if (c->bInArea) {
                    GreEndArea(c->hdc, 0);
                    c->bInArea = FALSE;
                }
                break;

            /* ============ long: primitives ============ */
            case OCODE_GSCP:    rc = DoGSCP(c, p, cbOrder); break;
            case OCODE_GLINE:   rc = DoLine(c, p, cbOrder, FALSE); break;
            case OCODE_GCLINE:  rc = DoLine(c, p, cbOrder, TRUE);  break;
            case OCODE_GRLINE:  rc = DoRelativeLine(c, p, cbOrder, FALSE); break;
            case OCODE_GCRLINE: rc = DoRelativeLine(c, p, cbOrder, TRUE);  break;
            case OCODE_GMRK:    rc = DoMark(c, p, cbOrder, FALSE); break;
            case OCODE_GCMRK:   rc = DoMark(c, p, cbOrder, TRUE);  break;
            case OCODE_GCHST:   rc = DoCharString(c, p, cbOrder, FALSE); break;
            case OCODE_GCCHST:  rc = DoCharString(c, p, cbOrder, TRUE);  break;
            case OCODE_GBOX:    rc = DoBox(c, p, cbOrder, FALSE); break;
            case OCODE_GCBOX:   rc = DoBox(c, p, cbOrder, TRUE);  break;
            case OCODE_GPARC:   rc = DoPartialArc(c, p, cbOrder, FALSE); break;
            case OCODE_GCPARC:  rc = DoPartialArc(c, p, cbOrder, TRUE);  break;
            case OCODE_GFARC:   rc = DoFullArc(c, p, cbOrder, FALSE); break;
            case OCODE_GCFARC:  rc = DoFullArc(c, p, cbOrder, TRUE);  break;
            case OCODE_GBEZ:    rc = DoBezier(c, p, cbOrder, FALSE); break;
            case OCODE_GCBEZ:   rc = DoBezier(c, p, cbOrder, TRUE);  break;
            case OCODE_GFLT:    rc = DoFillet(c, p, cbOrder, FALSE); break;
            case OCODE_GCFLT:   rc = DoFillet(c, p, cbOrder, TRUE);  break;

            /* ============ long: attributes ============ */
            case OCODE_GSAP:    rc = DoSetArcParams(c, p, cbOrder); break;
            case OCODE_GSCC:    rc = DoSetCharCell(c, p, cbOrder); break;
            case OCODE_GSMC:    rc = DoSetMarkerCell(c, p, cbOrder); break;
            case OCODE_GSPRP:   rc = DoSetPatternRefPoint(c, p, cbOrder); break;

            /* ============ very long ============ */
            case OCODE_VLONG:
                switch (p[1]) {
                case OCODEQ_GCHSTE:
                    rc = DoVLongCharString(c, p, cbOrder, TRUE);  break;
                case OCODEQ_GCCHSTE:
                    rc = DoVLongCharString(c, p, cbOrder, FALSE); break;
                default:
                    break;
                }
                break;

            default:
                break;
            }
        }

        if (rc != GOCA_OK) {
            GocaTrace(c, "[GOCA] @%04X -> ERR %ld (%s)\n",
                      (unsigned)c->uPos, (long)rc, GocaErrorName(rc));
            c->lError     = rc;
            c->uErrOffset = c->uPos;
            c->uErrOrder  = oc;
            return rc;
        }
        c->uPos += cbOrder;
    }

    GocaTrace(c, "[GOCA] ===== stream done: %u bytes =====\n",
              (unsigned)c->uPos);
    return GOCA_OK;
}

/* ======================================================================== */
/*  Diagnostics                                                             */
/* ======================================================================== */

const char *GocaErrorName(LONG lError)
{
    switch (lError) {
    case GOCA_OK:         return "OK";
    case GOCA_ERR_TRUNC:  return "truncated order";
    case GOCA_ERR_FORMAT: return "malformed order";
    case GOCA_ERR_NOMEM:  return "out of memory";
    case GOCA_ERR_USER:   return "user handler failure";
    default:              return "unknown";
    }
}

const char *GocaOrderName(UCHAR uCode)
{
    switch (uCode) {
    case OCODE_GNOP1:   return "GNOP1";
    case OCODE_GESD:    return "GESD";
    case OCODE_GEPROL:  return "GEPROL";
    case OCODE_GCOMT:   return "GCOMT";
    case OCODE_GSGCH:   return "GSGCH";
    case OCODE_GSMP:    return "GSMP";
    case OCODE_GSCOL:   return "GSCOL";
    case OCODE_GSECOL:  return "GSECOL";
    case OCODE_GSMX:    return "GSMX";
    case OCODE_GSBMX:   return "GSBMX";
    case OCODE_GSLT:    return "GSLT";
    case OCODE_GSLW:    return "GSLW";
    case OCODE_GSFLW:   return "GSFLW";
    case OCODE_GSLE:    return "GSLE";
    case OCODE_GSLJ:    return "GSLJ";
    case OCODE_GSMS:    return "GSMS";
    case OCODE_GSMT:    return "GSMT";
    case OCODE_GSPS:    return "GSPS";
    case OCODE_GSPT:    return "GSPT";
    case OCODE_GSCS:    return "GSCS";
    case OCODE_GSCD:    return "GSCD";
    case OCODE_GSCR:    return "GSCR";
    case OCODE_GBAR:    return "GBAR";
    case OCODE_GEAR:    return "GEAR";
    case OCODE_GSCP:    return "GSCP";
    case OCODE_GLINE:   return "GLINE";
    case OCODE_GCLINE:  return "GCLINE";
    case OCODE_GRLINE:  return "GRLINE";
    case OCODE_GCRLINE: return "GCRLINE";
    case OCODE_GMRK:    return "GMRK";
    case OCODE_GCMRK:   return "GCMRK";
    case OCODE_GCHST:   return "GCHST";
    case OCODE_GCCHST:  return "GCCHST";
    case OCODE_GBOX:    return "GBOX";
    case OCODE_GCBOX:   return "GCBOX";
    case OCODE_GPARC:   return "GPARC";
    case OCODE_GCPARC:  return "GCPARC";
    case OCODE_GFARC:   return "GFARC";
    case OCODE_GCFARC:  return "GCFARC";
    case OCODE_GBEZ:    return "GBEZ";
    case OCODE_GCBEZ:   return "GCBEZ";
    case OCODE_GFLT:    return "GFLT";
    case OCODE_GCFLT:   return "GCFLT";
    case OCODE_GSAP:    return "GSAP";
    case OCODE_GSCC:    return "GSCC";
    case OCODE_GSMC:    return "GSMC";
    case OCODE_GSPRP:   return "GSPRP";
    case OCODE_VLONG:   return "VLONG";
    default:            return "?";
    }
}
