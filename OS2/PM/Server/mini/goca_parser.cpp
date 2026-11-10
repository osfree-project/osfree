/*!
 * @file goca_parser.cpp
 * @brief GOCA drawing-order stream parser, dispatched through PMGRE.
 *
 * All calls go through <pmddim.h> macros (Gre*).  Attribute masks and
 * bundle field names come from <pmgpi.h>.  Order codes and structures
 * come from <pmord.h>.
 */

#define INCL_PM
#include <F_platform.h>
#define INCL_GREALL
#include "pmddi.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "goca_parser.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Debug trace                                                             */
/* ======================================================================== */

/*!
 * @brief Conditional trace routed to F_DbgPrint.
 *
 * The c->bDebug flag is no longer consulted: per-level filtering in
 * F_DbgPrint (F_DBG_SEC_GOCA_PARSE) handles enable/disable. The flag
 * is still set by GocaInit but currently unused.
 *
 * @param [in] c    Parser context (unused in the current version).
 * @param [in] fmt  printf-style format string.
 * @param [in] ...  Arguments for the format string.
 */
static void GocaTrace(GOCA_CTX *c, const char *fmt, ...)
{
    va_list args;
    char    buf[1024];

    (void)c;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    {
        size_t n = strlen(buf);
        if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = 0;
    }

    F_DbgPrint(F_DBG_SEC_GOCA_PARSE, F_DBG_LVL_VERBOSE, "%s", buf);
}

/*!
 * @def GocaCallEnter
 * @brief Open a debug context for a Do* handler.
 * @param [in] fmt  printf-style format for the arguments.
 * @param [in] ...  Arguments for the format string.
 */
#define GocaCallEnter(fmt, ...) \
    int _goca_ctx = F_DbgCtxEnterf(F_DBG_SEC_GOCA_PARSE, F_DBG_LVL_VERBOSE, \
                                   "[GOCA] IN : %s(" fmt ")", \
                                   __FUNCTION__, ##__VA_ARGS__)

/*!
 * @def GocaCallResult
 * @brief Close the debug context opened by GocaCallEnter.
 * @param [in] fmt  printf-style format for the result.
 * @param [in] ...  Arguments for the format string.
 */
#define GocaCallResult(fmt, ...) \
    F_DbgCtxExitf(_goca_ctx, fmt, ##__VA_ARGS__)

/* ======================================================================== */
/*  Endian-aware / size-aware readers                                       */
/* ======================================================================== */

/*! @brief Read signed 16-bit coordinate. */
static SHORT GocaRdS16(GOCA_CTX *c, const BYTE *p)
{
    return (SHORT)GocaGetU16(c, p);
}

/*! @brief Read signed 32-bit coordinate. */
static LONG GocaRdS32(GOCA_CTX *c, const BYTE *p)
{
    return (LONG)GocaGetU32(c, p);
}


/* ======================================================================== */
/*  Small helpers                                                           */
/* ======================================================================== */

/*! @brief GOCA point -> POINTL (size and endian aware). */
static POINTL PtsToPtl(GOCA_CTX *c, const BYTE *p)
{
    POINTL pt;
    if (c->coordSize == GOCA_COORD_SHORT) {
        pt.x = GocaRdS16(c, p);
        pt.y = GocaRdS16(c, p + 2);
    } else {
        pt.x = GocaRdS32(c, p);
        pt.y = GocaRdS32(c, p + 4);
    }
    return pt;
}

/*! @brief Allocate and fill POINTL[] from packed coordinates. */
static PPOINTL AllocPtlArray(GOCA_CTX *c, const BYTE *pData, ULONG nPts)
{
    ULONG i;
    PVOID   pv = NULL;
    PPOINTL pptl;

    if (DosAllocMem(&pv, nPts * sizeof(POINTL),
                    PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        return (PPOINTL)NULL;
    }
    pptl = (PPOINTL)pv;

    if (c->coordSize == GOCA_COORD_SHORT) {
        for (i = 0; i < nPts; i++) {
            pptl[i].x = GocaRdS16(c, pData + i * 4);
            pptl[i].y = GocaRdS16(c, pData + i * 4 + 2);
        }
    } else {
        for (i = 0; i < nPts; i++) {
            pptl[i].x = GocaRdS32(c, pData + i * 8);
            pptl[i].y = GocaRdS32(c, pData + i * 8 + 4);
        }
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
    return GreDeviceSetAttributes(hdc, (ULONG)lPrimType, 0UL, flAttrs,
                                  (PBUNDLE)pBundle);
}

/* ======================================================================== */
/*  Long-order handlers                                                     */
/* ======================================================================== */

/*! @brief GSCP (0x21) — Set Current Position. Data: x,y. */
static LONG DoGSCP(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    ULONG  need = 2u + (c->coordSize == GOCA_COORD_SHORT ? 4u : 8u);

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < need) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    pt = PtsToPtl(c, p + 2);
    GocaTrace(c, "[GOCA]   GSCP: pos=(%ld,%ld)\n",
              (long)pt.x, (long)pt.y);
    GreSetCurrentPosition(c->hdc, &pt);
    c->ptlCurPos = pt;

    GocaCallResult("GOCA_OK (pos=(%ld,%ld))", (long)pt.x, (long)pt.y);
    return GOCA_OK;
}

/*! @brief GLINE (0xC1) / GCLINE (0x81). */
static LONG DoLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nTotal, nPts, i;
    PVOID   pv = NULL;
    PPOINTL pptl;
    ULONG   pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        /* GCLINE: length is pointSize * count; prepend current position. */
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts   = len / pointSize;
        nTotal = nPts + 1;
        if (DosAllocMem(&pv, nTotal * sizeof(POINTL), PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        pptl = (PPOINTL)pv;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        if (c->coordSize == GOCA_COORD_SHORT) {
            for (i = 0; i < nPts; i++) {
                pptl[i+1].x = GocaRdS16(c, p + 2 + i*4);
                pptl[i+1].y = GocaRdS16(c, p + 2 + i*4 + 2);
            }
        } else {
            for (i = 0; i < nPts; i++) {
                pptl[i+1].x = GocaRdS32(c, p + 2 + i*8);
                pptl[i+1].y = GocaRdS32(c, p + 2 + i*8 + 4);
            }
        }
    } else {
        /* GLINE: first point is pointSize bytes, rest pointSize each. */
        if (len < pointSize || ((len - pointSize) % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nTotal = (len - pointSize) / pointSize + 1;
        pptl   = AllocPtlArray(c, p + 2, nTotal);
        if (!pptl) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCLINE" : "GLINE", (unsigned)nTotal,
              (long)pptl[nTotal-1].x, (long)pptl[nTotal-1].y);

    GrePolyLine(c->hdc, pptl, (LONG)nTotal);
    c->ptlCurPos = pptl[nTotal - 1];

    DosFreeMem(pptl);
    GocaCallResult("GOCA_OK (%u pts)", (unsigned)nTotal);
    return GOCA_OK;
}

/*! @brief GMRK (0xC2) / GCMRK (0x82). */
static LONG DoMark(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nTotal, nPts, i;
    PVOID   pv = NULL;
    PPOINTL pptl;
    ULONG   pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts   = len / pointSize;
        nTotal = nPts + 1;
        if (DosAllocMem(&pv, nTotal * sizeof(POINTL), PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        pptl = (PPOINTL)pv;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        if (c->coordSize == GOCA_COORD_SHORT) {
            for (i = 0; i < nPts; i++) {
                pptl[i+1].x = GocaRdS16(c, p + 2 + i*4);
                pptl[i+1].y = GocaRdS16(c, p + 2 + i*4 + 2);
            }
        } else {
            for (i = 0; i < nPts; i++) {
                pptl[i+1].x = GocaRdS32(c, p + 2 + i*8);
                pptl[i+1].y = GocaRdS32(c, p + 2 + i*8 + 4);
            }
        }
    } else {
        if (len < pointSize || ((len - pointSize) % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nTotal = (len - pointSize) / pointSize + 1;
        pptl   = AllocPtlArray(c, p + 2, nTotal);
        if (!pptl) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCMRK" : "GMRK", (unsigned)nTotal,
              (long)pptl[nTotal-1].x, (long)pptl[nTotal-1].y);

    GrePolyMarker(c->hdc, pptl, (LONG)nTotal);
    c->ptlCurPos = pptl[nTotal - 1];

    DosFreeMem(pptl);
    GocaCallResult("GOCA_OK (%u pts)", (unsigned)nTotal);
    return GOCA_OK;
}

/*!
 * @brief GRLINE (0xE1) / GCRLINE (0xA1) — relative line with SBIN offsets.
 */
static LONG DoRelativeLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, nTotal, i, off;
    PVOID   pv = NULL;
    PPOINTL pptl;
    SHORT   x, y;
    ULONG   pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    off = 0;
    if (bCur) {
        /* GCRLINE: length is 2 * count. */
        if (len == 0 || (len & 1)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        RefreshCurPos(c);
        x = (SHORT)c->ptlCurPos.x;
        y = (SHORT)c->ptlCurPos.y;
        off = 0;
    } else {
        /* GRLINE: pointSize-byte start point, then 2 * count. */
        if (len < pointSize || ((len - pointSize) & 1)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        x = (SHORT)((c->coordSize == GOCA_COORD_SHORT)
                    ? GocaRdS16(c, p + 2)
                    : GocaRdS32(c, p + 2));
        y = (SHORT)((c->coordSize == GOCA_COORD_SHORT)
                    ? GocaRdS16(c, p + 4)
                    : GocaRdS32(c, p + 6));
        off = pointSize;
    }

    nPts   = (len - off) / 2;
    nTotal = nPts + 1;
    if (DosAllocMem(&pv, nTotal * sizeof(POINTL), PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GocaCallResult("GOCA_ERR_NOMEM");
        return GOCA_ERR_NOMEM;
    }
    pptl = (PPOINTL)pv;

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

    DosFreeMem(pptl);
    GocaCallResult("GOCA_OK (%u pts)", (unsigned)nTotal);
    return GOCA_OK;
}

/*! @brief GCHST (0xC3) / GCCHST (0x83). */
static LONG DoCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, cch;
    POINTL      ptStart;
    const BYTE *pStr;
    ULONG       pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        /* GCCHST: no explicit point, string at current pos. */
        RefreshCurPos(c);
        ptStart = c->ptlCurPos;
        pStr    = p + 2;
        cch     = len;
    } else {
        /* GCHST: pointSize-byte point, then string. */
        if (len < pointSize) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        ptStart = PtsToPtl(c, p + 2);
        pStr    = p + 2 + pointSize;
        cch     = len - pointSize;
    }

    if (cch == 0) {
        GocaCallResult("GOCA_OK");
        return GOCA_OK;
    }

    GocaTrace(c, "[GOCA]   %s: %u chars at (%ld,%ld)\n",
              bCur ? "GCCHST" : "GCHST", (unsigned)cch,
              (long)ptStart.x, (long)ptStart.y);

    GreSetCurrentPosition(c->hdc, &ptStart);
    GreCharString(c->hdc, (LONG)cch, (PCH)pStr);
    c->ptlCurPos = ptStart;

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GBOX (0xC0) / GCBOX (0x80).
 */
static LONG DoBox(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG     len;
    UCHAR     fbFlags;
    BOOL      bBoundary, bFill;
    SHORT     x1, y1, x2, y2, hAxis = 0, vAxis = 0;
    BOXPARAMS bp;
    POINTL    ptStart;
    ULONG     pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (len < 2) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    fbFlags = p[2];

    if (bCur) {
        /* GCBOX: corner (x2,y2) after flags+1 pad, optional axes. */
        if (len < 2u + pointSize) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        RefreshCurPos(c);
        x1 = (SHORT)c->ptlCurPos.x;
        y1 = (SHORT)c->ptlCurPos.y;
        if (c->coordSize == GOCA_COORD_SHORT) {
            x2 = GocaRdS16(c, p + 4);
            y2 = GocaRdS16(c, p + 6);
            if (len >= 10) { hAxis = GocaRdS16(c, p + 8); vAxis = GocaRdS16(c, p + 10); }
        } else {
            x2 = (SHORT)GocaRdS32(c, p + 4);
            y2 = (SHORT)GocaRdS32(c, p + 8);
            if (len >= 18) { hAxis = (SHORT)GocaRdS32(c, p + 12); vAxis = (SHORT)GocaRdS32(c, p + 16); }
        }
    } else {
        /* GBOX: both corners, optional axes. */
        if (len < 2u + 2u * pointSize) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        if (c->coordSize == GOCA_COORD_SHORT) {
            x1 = GocaRdS16(c, p + 4); y1 = GocaRdS16(c, p + 6);
            x2 = GocaRdS16(c, p + 8); y2 = GocaRdS16(c, p + 10);
            if (len >= 14) { hAxis = GocaRdS16(c, p + 12); vAxis = GocaRdS16(c, p + 14); }
        } else {
            x1 = (SHORT)GocaRdS32(c, p + 4);  y1 = (SHORT)GocaRdS32(c, p + 8);
            x2 = (SHORT)GocaRdS32(c, p + 12); y2 = (SHORT)GocaRdS32(c, p + 16);
            if (len >= 26) { hAxis = (SHORT)GocaRdS32(c, p + 20); vAxis = (SHORT)GocaRdS32(c, p + 24); }
        }
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

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GPARC (0xE3) / GCPARC (0xA3).
 *
 * Layout (GCPARC): centre x,y, multiplier FIXED88,
 *                  start angle 32-bit, sweep angle 32-bit.
 * GPARC additionally has a start point at the front (ignored here).
 */
static LONG DoPartialArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, off;
    const BYTE *q;
    POINTL      ptCenter;
    FIXED       fxMult, fxStart, fxSweep;
    ULONG       pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    q   = p + 2;
    off = 0;
    if (!bCur) off += pointSize;            /* GPARC: skip start x,y */

    if (len < off + pointSize + 2 + 4 + 4) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    ptCenter = PtsToPtl(c, q + off); off += pointSize;
    fxMult   = (FIXED)((ULONG)GocaGetU16(c, q + off) << 8); off += 2;
    fxStart  = (FIXED)GocaGetU32(c, q + off); off += 4;
    fxSweep  = (FIXED)GocaGetU32(c, q + off);

    GocaTrace(c, "[GOCA]   %s: c=(%ld,%ld) mult=%04X start=%08lX sweep=%08lX\n",
              bCur ? "GCPARC" : "GPARC",
              (long)ptCenter.x, (long)ptCenter.y,
              (unsigned)fxMult,
              (unsigned long)fxStart, (unsigned long)fxSweep);

    GrePartialArc(c->hdc, &ptCenter, fxMult, fxStart, fxSweep);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GFARC (0xC7) / GCFARC (0x87).
 *
 * Data layout:
 *   GCFARC: multiplier FIXED88
 *   GFARC:  start x,y, then multiplier FIXED88
 */
static LONG DoFullArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, off;
    const BYTE *q;
    FIXED       fxMult;
    ULONG       pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    q   = p + 2;
    off = 0;
    if (!bCur) off += pointSize;            /* GFARC: skip start x,y */

    if (len < off + 2) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    fxMult = (FIXED)((ULONG)GocaGetU16(c, q + off) << 8);

    GocaTrace(c, "[GOCA]   %s: mult=%04X\n",
              bCur ? "GCFARC" : "GFARC", (unsigned)fxMult);

    GreFullArcBoth(c->hdc, fxMult);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*! @brief GBEZ (0xE5) / GCBEZ (0xA5). */
static LONG DoBezier(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, i;
    PVOID   pv = NULL;
    PPOINTL pptl;
    ULONG   pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = len / pointSize + 1;
        if (DosAllocMem(&pv, nPts * sizeof(POINTL), PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        pptl = (PPOINTL)pv;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        if (c->coordSize == GOCA_COORD_SHORT) {
            for (i = 0; i < nPts - 1; i++) {
                pptl[i+1].x = GocaRdS16(c, p + 2 + i*4);
                pptl[i+1].y = GocaRdS16(c, p + 2 + i*4 + 2);
            }
        } else {
            for (i = 0; i < nPts - 1; i++) {
                pptl[i+1].x = GocaRdS32(c, p + 2 + i*8);
                pptl[i+1].y = GocaRdS32(c, p + 2 + i*8 + 4);
            }
        }
    } else {
        if (len < 2u * pointSize || ((len - pointSize) % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = (len - pointSize) / pointSize + 1;
        pptl = AllocPtlArray(c, p + 2, nPts);
        if (!pptl) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCBEZ" : "GBEZ", (unsigned)nPts,
              (long)pptl[nPts-1].x, (long)pptl[nPts-1].y);

    GrePolySpline(c->hdc, pptl, (LONG)nPts);
    c->ptlCurPos = pptl[nPts - 1];

    DosFreeMem(pptl);
    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*! @brief GFLT (0xC5) / GCFLT (0x85). */
static LONG DoFillet(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, i;
    PVOID   pv = NULL;
    PPOINTL pptl;
    ULONG   pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = len / pointSize + 1;
        if (DosAllocMem(&pv, nPts * sizeof(POINTL), PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        pptl = (PPOINTL)pv;
        RefreshCurPos(c);
        pptl[0] = c->ptlCurPos;
        if (c->coordSize == GOCA_COORD_SHORT) {
            for (i = 0; i < nPts - 1; i++) {
                pptl[i+1].x = GocaRdS16(c, p + 2 + i*4);
                pptl[i+1].y = GocaRdS16(c, p + 2 + i*4 + 2);
            }
        } else {
            for (i = 0; i < nPts - 1; i++) {
                pptl[i+1].x = GocaRdS32(c, p + 2 + i*8);
                pptl[i+1].y = GocaRdS32(c, p + 2 + i*8 + 4);
            }
        }
    } else {
        if (len < pointSize || ((len - pointSize) % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = (len - pointSize) / pointSize + 1;
        pptl = AllocPtlArray(c, p + 2, nPts);
        if (!pptl) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCFLT" : "GFLT", (unsigned)nPts,
              (long)pptl[nPts-1].x, (long)pptl[nPts-1].y);

    GrePolyFillet(c->hdc, pptl, (LONG)nPts);
    c->ptlCurPos = pptl[nPts - 1];

    DosFreeMem(pptl);
    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*! @brief GSAP (0x22) — Set Arc Parameters. Data: 4 x SHORT (P,Q,R,S). */
static LONG DoSetArcParams(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    ARCPARAMS ap;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < 2u + 8) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (p[1] < 8) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    ap.lP = GocaRdS16(c, p + 2);
    ap.lQ = GocaRdS16(c, p + 4);
    ap.lR = GocaRdS16(c, p + 6);
    ap.lS = GocaRdS16(c, p + 8);

    GocaTrace(c, "[GOCA]   GSAP: P=%ld Q=%ld R=%ld S=%ld\n",
              (long)ap.lP, (long)ap.lQ, (long)ap.lR, (long)ap.lS);
    GreSetArcParameters(c->hdc, &ap);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*! @brief GSCC (0x33) — Set Character Cell. */
static LONG DoSetCharCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    SHORT  cxInt, cyInt;
    USHORT cxFrac, cyFrac;
    UCHAR  fbFlags;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < 2u + 10) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (p[1] < 10) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    cxInt  = GocaRdS16(c, p + 2);
    cyInt  = GocaRdS16(c, p + 4);
    cxFrac = GocaGetU16(c, p + 6);
    cyFrac = GocaGetU16(c, p + 8);
    fbFlags = p[10];
    (void)fbFlags;

    GocaTrace(c, "[GOCA]   GSCC: cell=(%d.%04X,%d.%04X)\n",
              (int)cxInt, (unsigned)cxFrac, (int)cyInt, (unsigned)cyFrac);
    c->cb.sizfxCell.cx = MAKEFIXED(cxInt, cxFrac);
    c->cb.sizfxCell.cy = MAKEFIXED(cyInt, cyFrac);
    GocaSetAttr(c->hdc, PRIM_CHAR, CBB_BOX, &c->cb);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*! @brief GSMC (0x37) — Set Marker Cell. */
static LONG DoSetMarkerCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    SHORT  cx, cy;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < 2u + 6) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (p[1] < 6) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    cx = GocaRdS16(c, p + 2);
    cy = GocaRdS16(c, p + 4);

    GocaTrace(c, "[GOCA]   GSMC: cell=(%d,%d)\n", (int)cx, (int)cy);
    c->mb.sizfxCell.cx = MAKEFIXED(cx, 0);
    c->mb.sizfxCell.cy = MAKEFIXED(cy, 0);
    GocaSetAttr(c->hdc, PRIM_MARKER, MBB_BOX, &c->mb);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*! @brief GSPRP (0xA0) — Set Pattern Reference Point. */
static LONG DoSetPatternRefPoint(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    UCHAR  fbFlags;
    ULONG  pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < 2u + 2u + pointSize) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (p[1] < 2u + pointSize) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    fbFlags = p[2];
    pt = PtsToPtl(c, p + 4);

    GocaTrace(c, "[GOCA]   GSPRP: ref=(%ld,%ld) flags=%02X\n",
              (long)pt.x, (long)pt.y, (unsigned)fbFlags);
    c->ab.ptlRefPoint = pt;
    GocaSetAttr(c->hdc, PRIM_AREA, ABB_REF_POINT, &c->ab);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GCCHSTE (0xB0) / GCHSTE (0xF0) — very long character string.
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
    ULONG       pointSize = (c->coordSize == GOCA_COORD_SHORT) ? 4u : 8u;
    ULONG       baseSize = 4u + 1u + 1u + pointSize * 2;

    GocaCallEnter("p=%p, cb=%u, bGiven=%d", p, (unsigned)cb, (int)bGiven);

    if (cb < baseSize) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    q       = p + 4;
    fbFlags = q[0];

    ptCorner0 = PtsToPtl(c, q + 2);
    ptCorner1 = PtsToPtl(c, q + 2 + pointSize);

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

    cch = GocaGetU16(c, q + 2 + pointSize * 2);
    if (cb < baseSize + cch) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    pStr = q + 2 + pointSize * 2 + 2;
    if (cch == 0) {
        GocaCallResult("GOCA_OK");
        return GOCA_OK;
    }

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

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/* ======================================================================== */
/*  Initialisation                                                          */
/* ======================================================================== */

void GocaInit(GOCA_CTX *c, HDC hdc, PBYTE pb, ULONG cb)
{
    GocaCallEnter("hdc=%p, pb=%p, cb=%u", hdc, pb, (unsigned)cb);

    memset(c, 0, sizeof(*c));
    c->hdc      = hdc;
    c->pbStream = pb;
    c->cbStream = cb;
    c->bDebug   = TRUE;

    /* Default format (x86 PC, LONG). Can be overridden by GocaSetFormatFromOptions. */
    GocaSetFormat(c, GOCA_COORD_LONG, GOCA_ENDIAN_LE);

    GocaTrace(c, "[GOCA] init: %u bytes\n", (unsigned)cb);

    /* Server only: pull live bundles so untouched attributes keep the
       DC defaults, and seed the current position. Client passes hdc == NULL. */
    if (hdc) {
        GreDeviceGetAttributes(hdc, PRIM_LINE,
            LBB_COLOR|LBB_BACK_COLOR|LBB_MIX_MODE|LBB_BACK_MIX_MODE|
            LBB_WIDTH|LBB_GEOM_WIDTH|LBB_TYPE|LBB_END|LBB_JOIN,
            (PBUNDLE)&c->lb);
        GreDeviceGetAttributes(hdc, PRIM_CHAR,
            CBB_COLOR|CBB_BACK_COLOR|CBB_MIX_MODE|CBB_BACK_MIX_MODE|
            CBB_SET|CBB_MODE|CBB_BOX|CBB_ANGLE|CBB_SHEAR|
            CBB_DIRECTION|CBB_TEXT_ALIGN|CBB_EXTRA|CBB_BREAK_EXTRA,
            (PBUNDLE)&c->cb);
        GreDeviceGetAttributes(hdc, PRIM_MARKER,
            MBB_COLOR|MBB_BACK_COLOR|MBB_MIX_MODE|MBB_BACK_MIX_MODE|
            MBB_SET|MBB_SYMBOL|MBB_BOX,
            (PBUNDLE)&c->mb);
        GreDeviceGetAttributes(hdc, PRIM_AREA,
            ABB_COLOR|ABB_BACK_COLOR|ABB_MIX_MODE|ABB_BACK_MIX_MODE|
            ABB_SET|ABB_SYMBOL|ABB_REF_POINT,
            (PBUNDLE)&c->ab);

        RefreshCurPos(c);
    }

    GocaCallResult("void");
}

/* ======================================================================== */
/*  Main loop                                                               */
/* ======================================================================== */

LONG GocaParseStream(GOCA_CTX *c)
{
    GocaCallEnter("stream=%u", (unsigned)c->cbStream);

    c->uPos   = 0;
    c->lError = GOCA_OK;

    GocaTrace(c, "[GOCA] ===== stream start: %u bytes =====\n",
              (unsigned)c->cbStream);

    while (c->uPos < c->cbStream) {
        const BYTE *p       = c->pbStream + c->uPos;
        ULONG       cbAvail = c->cbStream - c->uPos;
        ULONG       cbOrder = GocaOrderLength(c, p, cbAvail);
        UCHAR       oc;
        LONG        rc;
        BOOL        bHandled = FALSE;

        if (cbOrder == 0 || cbOrder > cbAvail) {
            GocaTrace(c, "[GOCA] @%04X truncation: cbAvail=%u\n",
                      (unsigned)c->uPos, (unsigned)cbAvail);
            c->lError     = GOCA_ERR_TRUNC;
            c->uErrOffset = c->uPos;
            c->uErrOrder  = p[0];
            GocaCallResult("GOCA_ERR_TRUNC");
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
                GocaCallEnter("GSCOL color=%u", (unsigned)clr);
                c->lb.lColor = c->cb.lColor = c->mb.lColor =
                               c->ab.lColor = (LONG)clr;
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_COLOR, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_COLOR, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_COLOR, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_COLOR, &c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSECOL: {
                USHORT raw;
                COLOR  clr;
                if (p[1] < 2) { rc = GOCA_ERR_FORMAT; break; }
                raw = GocaGetU16(c, p + 2);
                GocaCallEnter("GSECOL color=%04X", (unsigned)raw);
                clr = ((raw & 0xFF00) == 0xFF00)
                      ? (COLOR)(raw & 0x00FF)
                      : (COLOR)raw;
                c->lb.lColor = c->cb.lColor = c->mb.lColor =
                               c->ab.lColor = (LONG)clr;
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_COLOR, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_COLOR, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_COLOR, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_COLOR, &c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ mix ============ */
            case OCODE_GSMX: {
                GocaCallEnter("GSMX mix=%u", (unsigned)p[1]);
                c->lb.usMixMode = c->cb.usMixMode =
                c->mb.usMixMode = c->ab.usMixMode = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_MIX_MODE, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_MIX_MODE, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_MIX_MODE, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_MIX_MODE, &c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSBMX: {
                GocaCallEnter("GSBMX backmix=%u", (unsigned)p[1]);
                c->lb.usBackMixMode = c->cb.usBackMixMode =
                c->mb.usBackMixMode = c->ab.usBackMixMode = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE,   LBB_BACK_MIX_MODE, &c->lb);
                GocaSetAttr(c->hdc, PRIM_CHAR,   CBB_BACK_MIX_MODE, &c->cb);
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_BACK_MIX_MODE, &c->mb);
                GocaSetAttr(c->hdc, PRIM_AREA,   ABB_BACK_MIX_MODE, &c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ line attributes ============ */
            case OCODE_GSLT: {
                GocaCallEnter("GSLT type=%u", (unsigned)p[1]);
                c->lb.usType = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE, LBB_TYPE, &c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSLW: {
                FIXED fx = MAKEFIXED(p[1], 0);
                GocaCallEnter("GSLW width=%u", (unsigned)p[1]);
                c->lb.fxWidth    = fx;
                c->lb.lGeomWidth = (LONG)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE,
                            LBB_WIDTH | LBB_GEOM_WIDTH, &c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSFLW: {
                UCHAR ip, fp;
                FIXED fx;
                if (p[1] < 2) { rc = GOCA_ERR_FORMAT; break; }
                ip = p[2]; fp = p[3];
                GocaCallEnter("GSFLW width=%u.%02X", (unsigned)ip, (unsigned)fp);
                fx = MAKEFIXED(ip, (ULONG)fp << 8);
                c->lb.fxWidth    = fx;
                c->lb.lGeomWidth = (LONG)ip;
                GocaSetAttr(c->hdc, PRIM_LINE,
                            LBB_WIDTH | LBB_GEOM_WIDTH, &c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSLE: {
                GocaCallEnter("GSLE end=%u", (unsigned)p[1]);
                c->lb.usEnd = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE, LBB_END, &c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSLJ: {
                GocaCallEnter("GSLJ join=%u", (unsigned)p[1]);
                c->lb.usJoin = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_LINE, LBB_JOIN, &c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ marker attributes ============ */
            case OCODE_GSMS: {
                GocaCallEnter("GSMS set=%u", (unsigned)p[1]);
                c->mb.usSet = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_SET, &c->mb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSMT: {
                GocaCallEnter("GSMT symbol=%u", (unsigned)p[1]);
                c->mb.usSymbol = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_MARKER, MBB_SYMBOL, &c->mb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ area attributes ============ */
            case OCODE_GSPS: {
                GocaCallEnter("GSPS set=%u", (unsigned)p[1]);
                c->ab.usSet = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_AREA, ABB_SET, &c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSPT: {
                GocaCallEnter("GSPT symbol=%u", (unsigned)p[1]);
                c->ab.usSymbol = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_AREA, ABB_SYMBOL, &c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ char attributes ============ */
            case OCODE_GSCS: {
                GocaCallEnter("GSCS lcid=%u", (unsigned)p[1]);
                c->cb.usSet = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_CHAR, CBB_SET, &c->cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSCD: {
                GocaCallEnter("GSCD dir=%u", (unsigned)p[1]);
                c->cb.usDirection = (USHORT)p[1];
                GocaSetAttr(c->hdc, PRIM_CHAR, CBB_DIRECTION, &c->cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSCR: {
                GocaCallEnter("GSCR prec=%u", (unsigned)(p[1] & GSCR_PRECISION));
                c->cb.usPrecision = (USHORT)(p[1] & GSCR_PRECISION);
                GocaSetAttr(c->hdc, PRIM_CHAR, CBB_MODE, &c->cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ areas ============ */
            case OCODE_GBAR: {
                ULONG fl = 0;
                GocaCallEnter("GBAR flags=%02X", (unsigned)p[1]);
                c->uAreaFlags = p[1];
                c->bInArea    = TRUE;
                if (p[1] & 0x40) fl |= BA_BOUNDARY; else fl |= BA_NOBOUNDARY;
                if (p[1] & 0x20) fl |= BA_WINDING;  else fl |= BA_ALTERNATE;
                GreBeginArea(c->hdc, fl);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GEAR: {
                GocaCallEnter("GEAR");
                if (c->bInArea) {
                    GreEndArea(c->hdc, 0);
                    c->bInArea = FALSE;
                }
                GocaCallResult("GOCA_OK");
                break;
            }

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

            case OCODE_GBSEG:   /* segment marker; content is replayed by the
                       server's DrawSegment/DrawChain, no Gre action */
                break;
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
            GocaCallResult("ERR %ld", (long)rc);
            return rc;
        }
        c->uPos += cbOrder;
    }

    GocaTrace(c, "[GOCA] ===== stream done: %u bytes =====\n",
              (unsigned)c->uPos);
    GocaCallResult("GOCA_OK (%u bytes)", (unsigned)c->uPos);
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
    case OCODE_GBSEG:   return "GBSEG";
    case OCODE_GBEL:    return "GBEL";
    case OCODE_GEEL:    return "GEEL";
    case OCODE_GLABL:   return "GLABL";
    case OCODE_GBPTH:   return "GBPTH";
    case OCODE_GEPTH:   return "GEPTH";
    case OCODE_GCFIG:   return "GCFIG";
    case OCODE_GFPTH:   return "GFPTH";
    case OCODE_GOPTH:   return "GOPTH";
    case OCODE_GMPTH:   return "GMPTH";
    case OCODE_GSCPTH:  return "GSCPTH";
    default:            return "?";
    }
}

