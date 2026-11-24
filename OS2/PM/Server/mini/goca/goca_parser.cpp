/*!
 * @file goca_parser.cpp
 * @brief GOCA drawing-order stream parser, dispatched through PMGRE.
 *
 * All calls go through <pmddim.h> macros (Gre*).  Attribute masks and
 * bundle field names come from <pmgpi.h>.  Order codes and structures
 * come from <pmord.h>.
 */

#if 0
inArea orders
GPI:
GpiBeginElement
GpiBox (with the lControl parameter set to DRO_OUTLINE)
GpiCallSegmentMatrix
GpiComment
GpiElement (containing a valid call)
GpiEndElement
GpiFullArc (with the lControl parameter set to DRO_OUTLINE)
GpiLabel
GpiLine
GpiMove
GpiPartialArc
GpiPointArc
GpiPolyFillet
GpiPolyFilletSharp
GpiPolyLine
GpiPolySpline
GpiPop (that pops a valid call)
GpiSetArcParams
GpiSetAttrMode
GpiSetAttrs (setting valid line attributes only, or foreground color/mix (only) for other primitive types)
GpiSetColor
GpiSetCurrentPosition
GpiSetLineEnd
GpiSetLineJoin
GpiSetLineType
GpiSetLineWidth
GpiSetMix
GpiSetModelTransformMatrix
AFP GOCA:
 Box
 Comment
 Cubic Bezier Curve
 Fillet
 Full Arc
 Line
 No-Operation
 Partial Arc
 Relative Line
 Set Arc Parameters
 Set Background Mix
 Set Color
 Set Current Position
 Set Custom Line Type
 Set Extended Color
 Set Fractional Line Width
 Set Line End
 Set Line Join
 Set Line Type
 Set Line Width
 Set Mix
 Set Process Color
#endif

#define INCL_PM
#include <F_platform.h>
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
/*  Small helpers                                                           */
/* ======================================================================== */

/*!
 * @brief GOCA point -> POINTL.
 *
 * Reads two consecutive geometric parameters (x, y) using the
 * context's coordSize and coordEndian.
 *
 * @param [in] c  GOCA context (coordSize, coordEndian).
 * @param [in] p  Pointer to the first byte of the packed point.
 * @return The converted point.
 */
static POINTL PtsToPtl(GOCA_CTX *c, const BYTE *p)
{
    POINTL pt;
    pt.x = GocaGetCoord(c, p);
    pt.y = GocaGetCoord(c, p + c->coordSize);
    return pt;
}

/*!
 * @brief Allocate and fill POINTL[] from packed coordinates.
 *
 * @param [in] c      GOCA context (coordSize, coordEndian).
 * @param [in] pData  Pointer to the first packed point.
 * @param [in] nPts   Number of points to convert.
 * @return Pointer to the allocated POINTL array.
 * @retval NULL  nPts is zero or DosAllocMem failed.
 */
static PPOINTL AllocPtlArray(GOCA_CTX *c, const BYTE *pData, ULONG nPts)
{
    PPOINTL pptl = NULL;
    ULONG   ptSize, i;

    if (nPts == 0) return NULL;
    if (DosAllocMem((PPVOID)&pptl, nPts * sizeof(POINTL),
                    PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        return NULL;
    }
    ptSize = c->coordSize << 1;  /* x + y = 2 * coordSize */
    for (i = 0; i < nPts; i++) {
        const BYTE *pPt = pData + i * ptSize;
        pptl[i].x = GocaGetCoord(c, pPt);
        pptl[i].y = GocaGetCoord(c, pPt + c->coordSize);
    }
    return pptl;
}

/* ======================================================================== */
/*  Long-order handlers                                                     */
/* ======================================================================== */

/*!
 * @brief GSCP (0x21) Set Current Position. Data: x,y.
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order (code, length, operands).
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 */
static LONG DoGSCP(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    ULONG  need = 2u + 2u * (ULONG)c->coordSize;

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

/*!
 * @brief GLINE (0xC1) / GCLINE (0x81).
 *
 * GCLINE operands are pointSize * count; the current position is
 * prepended. GLINE operands are a start point followed by further
 * points. Current position ends at the last point.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCLINE (start at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand length inconsistent with point size.
 * @retval GOCA_ERR_NOMEM  Point array allocation failed.
 */
static LONG DoLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nTotal, nPts, i;
    PPOINTL pptl = NULL;
    ULONG   pointSize = 2u * (ULONG)c->coordSize;

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
        if (DosAllocMem((PPVOID)&pptl, nTotal * sizeof(POINTL),
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts; i++) {
            const BYTE *pPt = p + 2 + i * pointSize;
            pptl[i+1].x = GocaGetCoord(c, pPt);
            pptl[i+1].y = GocaGetCoord(c, pPt + c->coordSize);
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

/*!
 * @brief GMRK (0xC2) / GCMRK (0x82).
 *
 * GCMRK operands are pointSize * count; the current position is
 * prepended. GMRK operands are a start point followed by further
 * points. Current position ends at the last point.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCMRK (start at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand length inconsistent with point size.
 * @retval GOCA_ERR_NOMEM  Point array allocation failed.
 */
static LONG DoMark(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nTotal, nPts, i;
    PPOINTL pptl = NULL;
    ULONG   pointSize = 2u * (ULONG)c->coordSize;

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
        if (DosAllocMem((PPVOID)&pptl, nTotal * sizeof(POINTL),
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts; i++) {
            const BYTE *pPt = p + 2 + i * pointSize;
            pptl[i+1].x = GocaGetCoord(c, pPt);
            pptl[i+1].y = GocaGetCoord(c, pPt + c->coordSize);
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
 * @brief GRLINE (0xE1) / GCRLINE (0xA1) relative line with SBIN offsets.
 *
 * GCRLINE operands are 2 * count signed byte offsets from the current
 * position. GRLINE operands are a start point followed by 2 * count
 * offsets. Current position ends at the final computed endpoint.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCRLINE (start at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand length inconsistent with offsets.
 * @retval GOCA_ERR_NOMEM  Point array allocation failed.
 */
static LONG DoRelativeLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, nTotal, i, off;
    PPOINTL pptl = NULL;
    SHORT   x, y;
    ULONG   pointSize = 2u * (ULONG)c->coordSize;

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
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        x = (SHORT)c->ptlCurPos.x;
        y = (SHORT)c->ptlCurPos.y;
        off = 0;
    } else {
        /* GRLINE: pointSize-byte start point, then 2 * count. */
        if (len < pointSize || ((len - pointSize) & 1)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        x = (SHORT)GocaGetCoord(c, p + 2);
        y = (SHORT)GocaGetCoord(c, p + 2 + c->coordSize);
        off = pointSize;
    }

    nPts   = (len - off) / 2;
    nTotal = nPts + 1;
    if (DosAllocMem((PPVOID)&pptl, nTotal * sizeof(POINTL),
                    PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
        GocaCallResult("GOCA_ERR_NOMEM");
        return GOCA_ERR_NOMEM;
    }

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

/*!
 * @brief GCHST (0xC3) / GCCHST (0x83).
 *
 * GCCHST draws the string at the current position; GCHST draws it at
 * the given point and sets the current position to that point.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCCHST (string at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed (empty string included).
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand shorter than one point.
 */
static LONG DoCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, cch;
    POINTL      ptStart;
    const BYTE *pStr;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        /* GCCHST: no explicit point, string at current pos. */
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
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
 *
 * GCBOX takes the diagonally opposite corner (current position is the
 * first corner); GBOX takes both corners. Optional HAXIS/VAXIS follow
 * the corners. Flags select boundary and interior drawing.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCBOX (first corner at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand too short for corners/axes.
 */
static LONG DoBox(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG     len;
    UCHAR     fbFlags;
    BOOL      bBoundary, bFill;
    LONG      x1, y1, x2, y2, hAxis = 0, vAxis = 0;
    BOXPARAMS bp;
    POINTL    ptStart;
    ULONG     pointSize = 2u * (ULONG)c->coordSize;
    ULONG     off;

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
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        x1 = c->ptlCurPos.x;
        y1 = c->ptlCurPos.y;

        off = 4;
        x2 = GocaGetCoord(c, p + off);
        y2 = GocaGetCoord(c, p + off + c->coordSize);
        off += pointSize;

        if (len >= off + pointSize) {
            hAxis = GocaGetCoord(c, p + off);
            vAxis = GocaGetCoord(c, p + off + c->coordSize);
        }
    } else {
        /* GBOX: both corners, optional axes. */
        if (len < 2u + 2u * pointSize) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }

        off = 4;
        x1 = GocaGetCoord(c, p + off);
        y1 = GocaGetCoord(c, p + off + c->coordSize);
        off += pointSize;

        x2 = GocaGetCoord(c, p + off);
        y2 = GocaGetCoord(c, p + off + c->coordSize);
        off += pointSize;

        if (len >= off + pointSize) {
            hAxis = GocaGetCoord(c, p + off);
            vAxis = GocaGetCoord(c, p + off + c->coordSize);
        }
    }

    /* GreBox* uses current position as one corner. */
    ptStart.x = x1; ptStart.y = y1;
    GreSetCurrentPosition(c->hdc, &ptStart);

    bp.ptl.x   = x2;
    bp.ptl.y   = y2;
    bp.sizl.cx = hAxis;
    bp.sizl.cy = vAxis;

    /* pmord.h: GCBOX_FILL=0x40, GCBOX_BOUNDARY=0x20 */
    bFill     = (fbFlags & 0x40) != 0;
    bBoundary = (fbFlags & 0x20) != 0;

    GocaTrace(c, "[GOCA]   %s: (%ld,%ld)-(%ld,%ld) fill=%d bound=%d ax=(%ld,%ld)\n",
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
 * Layout (GCPARC): centre x,y, multiplier (FIXED88 or FIXED),
 *              start angle 32-bit, sweep angle 32-bit.
 * GPARC additionally has a start point at the front (skipped here).
 *
 * Per pmord.h:
 *   SHORT variant: ORDERS_GCPARC  - FIXED88 multiplier (2 bytes)
 *   LONG  variant: ORDERL_GCPARC  - FIXED   multiplier (4 bytes)
 *   Angles are always LONG (4 bytes) in both variants.
 *   Multiplier and angles follow coordEndian.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCPARC (no leading start point).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand too short for centre/multiplier/angles.
 */
static LONG DoPartialArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, off, multSize;
    const BYTE *q;
    POINTL      ptCenter;
    FIXED       fxMult, fxStart, fxSweep;
    USHORT      rawMult16;
    ULONG       rawMult32, rawStart, rawSweep;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    q   = p + 2;
    off = 0;
    if (!bCur) off += pointSize;            /* GPARC: skip start x,y */

    multSize = (c->coordSize == GOCA_COORD_SHORT) ? 2u : 4u;

    if (len < off + pointSize + multSize + 4u + 4u) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    ptCenter = PtsToPtl(c, q + off);
    off += pointSize;

    /* Multiplier: coordEndian, size depends on coordSize */
    if (c->coordSize == GOCA_COORD_SHORT) {
        /* FIXED88: 16-bit 8.8, coordEndian */
        if (c->coordEndian == GOCA_ENDIAN_LE)
            rawMult16 = (USHORT)(((USHORT)q[off+1] << 8) | q[off]);
        else
            rawMult16 = GocaGetU16(q + off);   /* BE via SWPUSHORT */
        fxMult = (FIXED)((ULONG)rawMult16 << 8);   /* 8.8 -> 16.16 */
    } else {
        /* FIXED: 32-bit 16.16, coordEndian */
        if (c->coordEndian == GOCA_ENDIAN_LE)
            rawMult32 = ((ULONG)q[off+3] << 24) | ((ULONG)q[off+2] << 16) |
                        ((ULONG)q[off+1] << 8)  |  (ULONG)q[off];
        else
            rawMult32 = GocaGetU32(q + off);   /* BE via SWPULONG */
        fxMult = (FIXED)rawMult32;
    }
    off += multSize;

    /* Angles: always 32-bit, coordEndian */
    if (c->coordEndian == GOCA_ENDIAN_LE) {
        rawStart = ((ULONG)q[off+3] << 24) | ((ULONG)q[off+2] << 16) |
                   ((ULONG)q[off+1] << 8)  |  (ULONG)q[off];
        rawSweep = ((ULONG)q[off+7] << 24) | ((ULONG)q[off+6] << 16) |
                   ((ULONG)q[off+5] << 8)  |  (ULONG)q[off+4];
    } else {
        rawStart = GocaGetU32(q + off);
        rawSweep = GocaGetU32(q + off + 4);
    }
    fxStart = (FIXED)rawStart;
    fxSweep = (FIXED)rawSweep;

    GocaTrace(c, "[GOCA]   %s: c=(%ld,%ld) mult=%08lX start=%08lX sweep=%08lX\n",
              bCur ? "GCPARC" : "GPARC",
              (long)ptCenter.x, (long)ptCenter.y,
              (unsigned long)(ULONG)fxMult,
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
 *
 * Multiplier is FIXED88 (16-bit, 8.8 fixed-point) in both variants,
 * follows coordEndian. Converted to FIXED (16.16) via << 8.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCFARC (centre at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand too short for the multiplier.
 */
static LONG DoFullArc(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, off;
    const BYTE *q;
    FIXED       fxMult;
    USHORT      rawMult;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;

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

    /* FIXED88 multiplier: 16-bit, coordEndian */
    if (c->coordEndian == GOCA_ENDIAN_LE)
        rawMult = (USHORT)(((USHORT)q[off+1] << 8) | q[off]);
    else
        rawMult = GocaGetU16(q + off);   /* BE via SWPUSHORT */

    fxMult = (FIXED)((ULONG)rawMult << 8);   /* 8.8 -> 16.16 */

    GocaTrace(c, "[GOCA]   %s: mult=%08lX\n",
              bCur ? "GCFARC" : "GFARC", (unsigned long)(ULONG)fxMult);

    GreFullArcBoth(c->hdc, fxMult);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GBEZ (0xE5) / GCBEZ (0xA5).
 *
 * Cubic Bezier splines; the point count including the start point
 * must be 3m+1. GCBEZ prepends the current position.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCBEZ (start at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand length inconsistent with point size.
 * @retval GOCA_ERR_NOMEM  Point array allocation failed.
 */
static LONG DoBezier(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, i;
    PPOINTL pptl = NULL;
    ULONG   pointSize = 2u * (ULONG)c->coordSize;

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
        if (DosAllocMem((PPVOID)&pptl, nPts * sizeof(POINTL),
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts - 1; i++) {
            const BYTE *pPt = p + 2 + i * pointSize;
            pptl[i+1].x = GocaGetCoord(c, pPt);
            pptl[i+1].y = GocaGetCoord(c, pPt + c->coordSize);
        }
    } else {
        if (len < 2u * pointSize || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = len / pointSize;
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

/*!
 * @brief GFLT (0xC5) / GCFLT (0x85).
 *
 * Curve tangential to a set of straight lines. GCFLT prepends the
 * current position; GFLT requires at least two explicit points.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] cb   Bytes available from p to the end of the stream.
 * @param [in] bCur TRUE for GCFLT (start at current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Fewer than two points supplied.
 * @retval GOCA_ERR_NOMEM  Point array allocation failed.
 */
static LONG DoFillet(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG   len, nPts, i;
    PPOINTL pptl = NULL;
    ULONG   pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];
    if (cb < 2u + len) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    if (bCur) {
        /* GCFLT: points after current position; need at least 1 point. */
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = len / pointSize + 1;
        if (DosAllocMem((PPVOID)&pptl, nPts * sizeof(POINTL),
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        pptl[0] = c->ptlCurPos;
        for (i = 0; i < nPts - 1; i++) {
            const BYTE *pPt = p + 2 + i * pointSize;
            pptl[i+1].x = GocaGetCoord(c, pPt);
            pptl[i+1].y = GocaGetCoord(c, pPt + c->coordSize);
        }
    } else {
        /* GFLT: explicit points; need at least 2 points for fillet. */
        if (len < 2u * pointSize || (len % pointSize)) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
        nPts = len / pointSize;
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
    GocaCallResult("GOCA_OK (%u pts)", (unsigned)nPts);
    return GOCA_OK;
}

/*!
 * @brief GSAP (0x22) / GPSAP (0x62) - Set Arc Parameters. Data: P,Q,R,S.
 *
 * Four geometric parameters, each coordSize wide, coordEndian order.
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand shorter than four parameters.
 */
static LONG DoSetArcParams(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    ARCPARAMS ap;
    ULONG     paramSize = (ULONG)c->coordSize;
    ULONG     need = 2u + 4u * paramSize;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < need) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (p[1] < 4u * paramSize) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    /* P,Q,R,S: geometric parameters, coordSize + coordEndian */
    ap.lP = GocaGetCoord(c, p + 2);
    ap.lQ = GocaGetCoord(c, p + 2 + paramSize);
    ap.lR = GocaGetCoord(c, p + 2 + 2u * paramSize);
    ap.lS = GocaGetCoord(c, p + 2 + 3u * paramSize);

    GocaTrace(c, "[GOCA]   GSAP: P=%ld Q=%ld R=%ld S=%ld\n",
              (long)ap.lP, (long)ap.lQ, (long)ap.lR, (long)ap.lS);
    GreSetArcParameters(c->hdc, &ap);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GSCC (0x33) / GPSCC (0x03) Set Character Cell.
 *
 * Layout per 64F0277 Vol.2: (len, cellx, celly, [cellxf, cellyf], [flags, res]).
 * cellx/celly: GROSOL; cellxf/cellyf: GUSHORT, optional (1 byte, 1/256,
 * in GDDM 1-byte format); flags/res optional, res required with flags.
 * flags.setzero (0x80, GSCC_ZEROZERO):
 *   0 - a cell size of zero sets drawing default;
 *   1 - a cell size of zero sets to zero.
 * The drawing default is taken from the DC default attribute bundle
 * (GreGetDefaultAttributes, PRIM_CHAR / CBB_BOX).
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed and CBB_BOX applied.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand too short, trailing bytes present,
 *                         or reserved bits/bytes are non-zero.
 */
static LONG DoSetCharCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    LONG   cxInt, cyInt;
    USHORT cxFrac = 0, cyFrac = 0;
    UCHAR  fbFlags = 0, uchRes = 0;
    ULONG  intSize  = (ULONG)c->coordSize;
    ULONG  fracSize = (c->coordSize == GOCA_COORD_BYTE) ? 1u : 2u;
    ULONG  dataLen, rem, off;
    BOOL   bZero;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < 2u) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    dataLen = p[1];
    if (cb < 2u + dataLen) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    off = 2;
    rem = dataLen;
    if (rem < 2u * intSize) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    cxInt = GocaGetCoord(c, p + off); off += intSize; rem -= intSize;
    cyInt = GocaGetCoord(c, p + off); off += intSize; rem -= intSize;

    /* Optional cellxf/cellyf, coordEndian */
    if (rem >= 2u * fracSize) {
        if (fracSize == 1u) {
            /* GDDM 1-byte format: multiples of 1/256 -> normalize to 1/65536 */
            cxFrac = (USHORT)((USHORT)p[off] << 8); off += 1u;
            cyFrac = (USHORT)((USHORT)p[off] << 8); off += 1u;
        } else if (c->coordEndian == GOCA_ENDIAN_LE) {
            cxFrac = (USHORT)(((USHORT)p[off+1] << 8) | p[off]); off += 2u;
            cyFrac = (USHORT)(((USHORT)p[off+1] << 8) | p[off]); off += 2u;
        } else {
            cxFrac = GocaGetU16(p + off); off += 2u;
            cyFrac = GocaGetU16(p + off); off += 2u;
        }
        rem -= 2u * fracSize;
    }

    /* Optional flags + res; res must follow flags */
    if (rem >= 2u) {
        fbFlags = p[off]; off += 1u;
        uchRes  = p[off]; off += 1u;
        rem -= 2u;
        if ((fbFlags & (UCHAR)~(UCHAR)GSCC_ZEROZERO) != 0 || uchRes != 0) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
    }
    if (rem != 0) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    bZero = (cxInt == 0 && cyInt == 0 && cxFrac == 0 && cyFrac == 0);

    if (bZero && !(fbFlags & GSCC_ZEROZERO)) {
        /* setzero=0: a cell size of zero sets drawing default.
         * GRE exposes no char-box query; the drawing default is taken
         * from the DC default attribute bundle. */
        CHARBUNDLE cbDef;
        if (GreGetDefaultAttributes(c->hdc, PRIM_CHAR, CBB_BOX,
                                    (PBUNDLE)&cbDef)) {
            c->cb.sizfxCell = cbDef.sizfxCell;
        } else {
            c->cb.sizfxCell.cx = 0;
            c->cb.sizfxCell.cy = 0;
        }
    } else {
        /* setzero=1: a cell size of zero sets to zero;
         * non-zero cell: value as specified. */
        c->cb.sizfxCell.cx = MAKEFIXED(cxInt, cxFrac);
        c->cb.sizfxCell.cy = MAKEFIXED(cyInt, cyFrac);
    }

    GocaTrace(c, "[GOCA]   GSCC: cell=(%ld.%04X,%ld.%04X) setzero=%u\n",
              (long)cxInt, (unsigned)cxFrac, (long)cyInt, (unsigned)cyFrac,
              (unsigned)((fbFlags & GSCC_ZEROZERO) != 0));

    GreDeviceSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_BOX, (PBUNDLE)&c->cb);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GSMC (0x37) / GPSMC (0x77) Set Marker Cell.
 *
 * Layout per pmord.h (ORDERS_GSMC / ORDERL_GSMC) and 64F0277 Vol.2:
 *   (len, cellx, celly, [flags, res])
 * cellx/celly: GROSOL (signed, coordSize/coordEndian aware);
 * flags/res: optional pair, res required when flags present;
 * flags.setzero (0x80 = GSMC_ZEROZERO):
 *   0 - a cell size of zero sets drawing default;
 *   1 - a cell size of zero sets to zero.
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed and MBB_BOX applied.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand too short, trailing bytes present,
 *                         or reserved bits/bytes are non-zero.
 */
static LONG DoSetMarkerCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    LONG  cx, cy;
    UCHAR fbFlags = 0, uchRes = 0;
    ULONG intSize = (ULONG)c->coordSize;
    ULONG dataLen, rem, off;
    BOOL  bZero;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (cb < 2u) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    dataLen = p[1];
    if (cb < 2u + dataLen) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }

    off = 2;
    rem = dataLen;
    if (rem < 2u * intSize) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    cx = GocaGetCoord(c, p + off); off += intSize; rem -= intSize;
    cy = GocaGetCoord(c, p + off); off += intSize; rem -= intSize;

    /* Optional flags + res; res must follow flags */
    if (rem >= 2u) {
        fbFlags = p[off]; off += 1u;
        uchRes  = p[off]; off += 1u;
        rem -= 2u;
        if ((fbFlags & (UCHAR)~(UCHAR)GSMC_ZEROZERO) != 0 || uchRes != 0) {
            GocaCallResult("GOCA_ERR_FORMAT");
            return GOCA_ERR_FORMAT;
        }
    }
    if (rem != 0) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    bZero = (cx == 0 && cy == 0);

    if (bZero && !(fbFlags & GSMC_ZEROZERO)) {
        /* setzero=0: a cell size of zero sets drawing default.
         * MARKERBUNDLE: sizfxCell == 0 is the GRE sentinel for the
         * default marker cell (nominal symbol size). */
        c->mb.sizfxCell.cx = 0;
        c->mb.sizfxCell.cy = 0;
    } else {
        /* Non-zero cell as specified, or zero + setzero=1 (literal zero). */
        c->mb.sizfxCell.cx = MAKEFIXED(cx, 0);
        c->mb.sizfxCell.cy = MAKEFIXED(cy, 0);
    }

    GocaTrace(c, "[GOCA]   GSMC: cell=(%ld,%ld) setzero=%u\n",
              (long)cx, (long)cy,
              (unsigned)((fbFlags & GSMC_ZEROZERO) != 0));

    GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_BOX, (PBUNDLE)&c->mb);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GSPRP (0xA0) / GPSPRP (0xE0) Set Pattern Reference Point.
 *
 * Layout per pmord.h (ORDERS_GSPRP / ORDERL_GSPRP):
 *   (len, flags, res, x, y)
 * flags: GSPRP_DEFAULT (0x80) - pattern reference point is set to the
 * drawing default (origin), x/y ignored; GSPRP_SPECIFY (0x00) - use the
 * specified point.  Reserved flag bits and res must be zero.
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed and ABB_REF_POINT applied.
 * @retval GOCA_ERR_TRUNC  Order exceeds the stream.
 * @retval GOCA_ERR_FORMAT Operand too short or reserved bits/bytes
 *                         are non-zero.
 */
static LONG DoSetPatternRefPoint(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    UCHAR  fbFlags, uchRes;
    ULONG  pointSize = 2u * (ULONG)c->coordSize;

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
    uchRes  = p[3];
    if ((fbFlags & (UCHAR)~(UCHAR)GSPRP_DEFAULT) != 0 || uchRes != 0) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    pt = PtsToPtl(c, p + 4);
    if (fbFlags & GSPRP_DEFAULT) {
        /* Drawing default: pattern space origin; x/y ignored */
        pt.x = 0;
        pt.y = 0;
    }

    GocaTrace(c, "[GOCA]   GSPRP: ref=(%ld,%ld) default=%u\n",
              (long)pt.x, (long)pt.y,
              (unsigned)((fbFlags & GSPRP_DEFAULT) != 0));

    c->ab.ptlRefPoint = pt;
    GreDeviceSetAttributes(c->hdc, PRIM_AREA, 0UL, ABB_REF_POINT, (PBUNDLE)&c->ab);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GCCHSTE (0xFEB0) / GCHSTE (0xFEF0): extended character string.
 *
 * Extended-format order: (0xFE, qualifier, SWPUSHORT len, data).
 * Data per pmord.h (ORDERS_GCCHSTE / ORDERL_GCCHSTE):
 *   flags, res, rect[2] (GROSOL pairs), cchString (SWPUSHORT),
 *   string[cchString], adx[cchString] (optional, GROSOL each).
 * Flags per pmord.h: DRAWRECT 0x80, CLIP 0x40, LEAVEPOS 0x10,
 * UNDERSCORE 0x08, STRIKEOUT 0x04; bits 0x23 (DEEMPHASIZE plus the two
 * low bits) and the res byte are reserved and must be zero.
 * The rectangle is normalized to xLeft <= xRight, yBottom <= yTop and
 * is passed to GreCharStringPos as the clip rectangle only when CLIP
 * is set; with DRAWRECT its boundary is stroked via GreBoxBoundary.
 * adx, when present (remaining length == cchString * coordSize), is
 * converted to LONG[] and passed as the per-character increment array.
 * With LEAVEPOS the previous current position is restored after the
 * string; with MOVEPOS the cached position is refreshed from the DC.
 * UNDERSCORE/STRIKEOUT are reported in the trace: the cached
 * CHARBUNDLE exposes no character-definition bits in the provided
 * headers.
 *
 * @param [in] c      GOCA context (coordSize, coordEndian, cached
 *                    bundles, current position).
 * @param [in] p      Pointer to the extended order (0xFE, qualifier,
 *                    SWPUSHORT length, operands).
 * @param [in] cb     Bytes available from p to the end of the stream
 *                    (the wire order length).
 * @param [in] bGiven TRUE for GCHSTE (string at the given rectangle
 *                    corner), FALSE for GCCHSTE (current position).
 * @return GOCA_OK or a negative error code.
 * @retval GOCA_OK         Order processed (empty string included).
 * @retval GOCA_ERR_TRUNC  Order or declared length exceeds the stream.
 * @retval GOCA_ERR_FORMAT Data length inconsistent with rect/string/adx,
 *                         or reserved bits/bytes are non-zero.
 * @retval GOCA_ERR_NOMEM  adx array allocation failed.
 */
static LONG DoVLongCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bGiven)
{
    const BYTE *q;
    UCHAR       fbFlags, uchRes;
    USHORT      cch;
    const BYTE *pStr;
    POINTL      ptStart, ptOld;
    POINTL      ptCorner0, ptCorner1;
    RECTL       rcl;
    BOXPARAMS   bp;
    ULONG       fl, dataLen, consumed, rem, i;
    PLONG       plX = NULL;
    PRECTL      prcl;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;
    ULONG       minData   = 2u + 2u * pointSize + 2u;  /* flags+res+rect+cch */

    GocaCallEnter("p=%p, cb=%u, bGiven=%d", p, (unsigned)cb, (int)bGiven);

    if (cb < 4u) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    dataLen = GocaGetU16(p + 2);               /* SWPUSHORT, always BE */
    if (cb < 4u + dataLen) {
        GocaCallResult("GOCA_ERR_TRUNC");
        return GOCA_ERR_TRUNC;
    }
    if (dataLen < minData) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    q       = p + 4;
    fbFlags = q[0];
    uchRes  = q[1];
    if ((fbFlags & 0x23u) != 0 || uchRes != 0) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }

    ptCorner0 = PtsToPtl(c, q + 2);
    ptCorner1 = PtsToPtl(c, q + 2 + pointSize);
    cch       = GocaGetU16(q + 2 + 2u * pointSize);

    consumed = minData + (ULONG)cch;
    if (dataLen < consumed) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    rem = dataLen - consumed;
    if (rem != 0 && rem != (ULONG)cch * (ULONG)c->coordSize) {
        GocaCallResult("GOCA_ERR_FORMAT");
        return GOCA_ERR_FORMAT;
    }
    pStr = q + 2 + 2u * pointSize + 2;

    rcl.xLeft   = (ptCorner0.x < ptCorner1.x) ? ptCorner0.x : ptCorner1.x;
    rcl.yBottom = (ptCorner0.y < ptCorner1.y) ? ptCorner0.y : ptCorner1.y;
    rcl.xRight  = (ptCorner0.x > ptCorner1.x) ? ptCorner0.x : ptCorner1.x;
    rcl.yTop    = (ptCorner0.y > ptCorner1.y) ? ptCorner0.y : ptCorner1.y;

    if (cch == 0) {
        GocaCallResult("GOCA_OK");
        return GOCA_OK;
    }

    /* Optional adx[cch]: per-character increments, GROSOL each */
    if (rem != 0) {
        const BYTE *pAdx = pStr + cch;
        if (DosAllocMem((PPVOID)&plX, (ULONG)cch * sizeof(LONG),
                        PAG_COMMIT | PAG_READ | PAG_WRITE) != 0) {
            GocaCallResult("GOCA_ERR_NOMEM");
            return GOCA_ERR_NOMEM;
        }
        for (i = 0; i < cch; i++)
            plX[i] = GocaGetCoord(c, pAdx + i * (ULONG)c->coordSize);
    }

    ptOld = c->ptlCurPos;
    if (bGiven) {
        ptStart = ptCorner0;
    } else {
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
        ptOld   = c->ptlCurPos;
        ptStart = ptOld;
    }

    fl   = CHS_START_XY;
    prcl = (fbFlags & GCCHSTE_CLIP) ? &rcl : (PRECTL)NULL;

    GocaTrace(c, "[GOCA]   %s: %u chars at (%ld,%ld) flags=%02X clip=(%ld,%ld)-(%ld,%ld)\n",
              bGiven ? "GCHSTE" : "GCCHSTE", (unsigned)cch,
              (long)ptStart.x, (long)ptStart.y, (unsigned)fbFlags,
              (long)rcl.xLeft, (long)rcl.yBottom,
              (long)rcl.xRight, (long)rcl.yTop);

    GreCharStringPos(c->hdc, &ptStart, prcl, fl, (LONG)cch,
                     (PCH)pStr, plX, (PCSP_INFO)NULL);

    /* DRAWRECT: stroke the rectangle boundary with line attributes */
    if (fbFlags & GCCHSTE_DRAWRECT) {
        bp.ptl.x   = rcl.xLeft;
        bp.ptl.y   = rcl.yBottom;
        bp.sizl.cx = rcl.xRight - rcl.xLeft;
        bp.sizl.cy = rcl.yTop   - rcl.yBottom;
        GreBoxBoundary(c->hdc, &bp);
    }

    /* LEAVEPOS: current position unchanged; MOVEPOS: ends at string end */
    if (fbFlags & GCCHSTE_LEAVEPOS) {
        GreSetCurrentPosition(c->hdc, &ptOld);
        c->ptlCurPos = ptOld;
    } else {
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
    }

    if (plX) DosFreeMem(plX);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/* ======================================================================== */
/*  Initialisation                                                          */
/* ======================================================================== */

/*!
 * @brief Initialize the parser context for server-side use.
 *
 * Sets @c hdc, stream pointers, format from defaults, and pulls live
 * bundles plus the current position from the device. Must be called
 * with a valid Gre* @c hdc.
 *
 * @param [out] c    GOCA context to initialize.
 * @param [in]  hdc  Target DC (Gre* handle); NULL for client-side use.
 * @param [in]  pb   Pointer to the order stream.
 * @param [in]  cb   Size of the order stream in bytes.
 */
void GocaInit(GOCA_CTX *c, HDC hdc, PBYTE pb, ULONG cb)
{
    GocaCallEnter("hdc=%p, pb=%p, cb=%u", hdc, pb, (unsigned)cb);

    memset(c, 0, sizeof(*c));
    c->hdc      = hdc;
    c->pbStream = pb;
    c->cbStream = cb;

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
        GreGetCurrentPosition(c->hdc, &c->ptlCurPos);
    }

    GocaCallResult("void");
}

/* ======================================================================== */
/*  Main loop                                                               */
/* ======================================================================== */

/*!
 * @brief Parse the stream in @c pCtx->pbStream of @c pCtx->cbStream bytes.
 *
 * On failure c->lError, c->uErrOffset and c->uErrOrder identify the
 * offending order.
 *
 * @param [in] c  GOCA context (stream, format, cached bundles).
 * @return GOCA_OK on success, GOCA_ERR_* on failure.
 * @retval GOCA_OK  All orders processed.
 */
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
        ULONG       cbOrder = GocaOrderLength(p, cbAvail);
        UCHAR       oc;
        LONG        rc;

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

            switch (oc) {

            /* ============ 1/2-byte trivial ============ */
            case OCODE_GNOP1:
            case OCODE_GCOMT:
            case OCODE_GESD:
            case OCODE_GEPROL:
            case OCODE_GSGCH:
            case OCODE_GSMP:
                break;

            /* ============ colour (2-byte GSCOL / long GSECOL) ============ */
            case OCODE_GSCOL: {
                COLOR clr = (COLOR)p[1];
                GocaCallEnter("GSCOL color=%u", (unsigned)clr);
                c->lb.lColor = c->cb.lColor = c->mb.lColor =
                               c->ab.lColor = (LONG)clr;
                GreDeviceSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_COLOR, (PBUNDLE)&c->lb);
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_COLOR, (PBUNDLE)&c->cb);
                GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_COLOR, (PBUNDLE)&c->mb);
                GreDeviceSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_COLOR, (PBUNDLE)&c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSECOL: {
                USHORT raw;
                COLOR  clr;
                if (p[1] < 2) { rc = GOCA_ERR_FORMAT; break; }
                raw = GocaGetU16(p + 2);
                GocaCallEnter("GSECOL color=%04X", (unsigned)raw);
                clr = ((raw & 0xFF00) == 0xFF00)
                      ? (COLOR)(raw & 0x00FF)
                      : (COLOR)raw;
                c->lb.lColor = c->cb.lColor = c->mb.lColor =
                               c->ab.lColor = (LONG)clr;
                GreDeviceSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_COLOR, (PBUNDLE)&c->lb);
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_COLOR, (PBUNDLE)&c->cb);
                GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_COLOR, (PBUNDLE)&c->mb);
                GreDeviceSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_COLOR, (PBUNDLE)&c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ mix ============ */
            case OCODE_GSMX: {
                GocaCallEnter("GSMX mix=%u", (unsigned)p[1]);
                c->lb.usMixMode = c->cb.usMixMode =
                c->mb.usMixMode = c->ab.usMixMode = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_MIX_MODE, (PBUNDLE)&c->lb);
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_MIX_MODE, (PBUNDLE)&c->cb);
                GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_MIX_MODE, (PBUNDLE)&c->mb);
                GreDeviceSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_MIX_MODE, (PBUNDLE)&c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSBMX: {
                GocaCallEnter("GSBMX backmix=%u", (unsigned)p[1]);
                c->lb.usBackMixMode = c->cb.usBackMixMode =
                c->mb.usBackMixMode = c->ab.usBackMixMode = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_BACK_MIX_MODE, (PBUNDLE)&c->lb);
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_BACK_MIX_MODE, (PBUNDLE)&c->cb);
                GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_BACK_MIX_MODE, (PBUNDLE)&c->mb);
                GreDeviceSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_BACK_MIX_MODE, (PBUNDLE)&c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ line attributes ============ */
            case OCODE_GSLT: {
                GocaCallEnter("GSLT type=%u", (unsigned)p[1]);
                c->lb.usType = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_LINE, 0UL, LBB_TYPE, (PBUNDLE)&c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSLW: {
                FIXED fx = MAKEFIXED(p[1], 0);
                GocaCallEnter("GSLW width=%u", (unsigned)p[1]);
                c->lb.fxWidth    = fx;
                c->lb.lGeomWidth = (LONG)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_LINE, 0UL,
                                       LBB_WIDTH | LBB_GEOM_WIDTH, (PBUNDLE)&c->lb);
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
                GreDeviceSetAttributes(c->hdc, PRIM_LINE, 0UL,
                                       LBB_WIDTH | LBB_GEOM_WIDTH, (PBUNDLE)&c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSLE: {
                GocaCallEnter("GSLE end=%u", (unsigned)p[1]);
                c->lb.usEnd = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_LINE, 0UL, LBB_END, (PBUNDLE)&c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSLJ: {
                GocaCallEnter("GSLJ join=%u", (unsigned)p[1]);
                c->lb.usJoin = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_LINE, 0UL, LBB_JOIN, (PBUNDLE)&c->lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ marker attributes ============ */
            case OCODE_GSMS: {
                GocaCallEnter("GSMS set=%u", (unsigned)p[1]);
                c->mb.usSet = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_SET, (PBUNDLE)&c->mb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSMT: {
                GocaCallEnter("GSMT symbol=%u", (unsigned)p[1]);
                c->mb.usSymbol = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_SYMBOL, (PBUNDLE)&c->mb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ area attributes ============ */
            case OCODE_GSPS: {
                GocaCallEnter("GSPS set=%u", (unsigned)p[1]);
                c->ab.usSet = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_AREA, 0UL, ABB_SET, (PBUNDLE)&c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSPT: {
                GocaCallEnter("GSPT symbol=%u", (unsigned)p[1]);
                c->ab.usSymbol = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_AREA, 0UL, ABB_SYMBOL, (PBUNDLE)&c->ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ char attributes ============ */
            case OCODE_GSCS: {
                GocaCallEnter("GSCS lcid=%u", (unsigned)p[1]);
                c->cb.usSet = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_SET, (PBUNDLE)&c->cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSCD: {
                GocaCallEnter("GSCD dir=%u", (unsigned)p[1]);
                c->cb.usDirection = (USHORT)p[1];
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_DIRECTION, (PBUNDLE)&c->cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GSCR: {
                GocaCallEnter("GSCR prec=%u", (unsigned)(p[1] & GSCR_PRECISION));
                c->cb.usPrecision = (USHORT)(p[1] & GSCR_PRECISION);
                GreDeviceSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_MODE, (PBUNDLE)&c->cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ areas ============ */
            case OCODE_GBAR: {
                ULONG fl = 0;
                GocaCallEnter("GBAR flags=%02X", (unsigned)p[1]);
		if (c->bInArea)
                {
                    rc=GOCA_EC_GBAR_NESTED;
                    break;
                }
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
                    rc=GOCA_EC_UNALLOCATED_ORDER;
                    break;
                }
                break;

            default:
                rc=GOCA_EC_UNALLOCATED_ORDER;
		break;
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

/*!
 * @brief Human-readable name for a GOCA error code.
 *
 * @param [in] lError  Error code (GOCA_OK, GOCA_ERR_*, GOCA_EC_*).
 * @return Static string describing the error; "unknown" for
 *         unrecognized codes.
 */
const char *GocaErrorName(LONG lError)
{
    switch (lError) {
    case GOCA_OK:                   return "OK";
    case GOCA_EC_UNALLOCATED_ORDER: return "EC-0001: Unallocated order codes";
    case GOCA_EC_GBAR_NESTED:       return "EC-6800: A Begin Area order has been executed after another Begin Area order, without an intervening End Area order being executed";
    case GOCA_ERR_TRUNC:  return "truncated order";
    case GOCA_ERR_FORMAT: return "malformed order";
    case GOCA_ERR_NOMEM:  return "out of memory";
    default:              return "unknown";
    }
}

/*!
 * @brief Human-readable name for a GOCA order code.
 *
 * @param [in] uCode  Order code (OCODE_*).
 * @return Static string with the order mnemonic; "?" for unknown codes.
 */
const char *GocaOrderName(UCHAR uCode)
{
    switch (uCode) {
    case /* X'00' */ OCODE_GNOP1:   return "GNOP1";
    case /* X'01' */ OCODE_GCOMT:   return "GCOMT";
    case /* X'04' */ OCODE_GSGCH:   return "GSGCH";
    case /* X'08' */ OCODE_GSPS:    return "GSPS";
    case /* X'0A' */ OCODE_GSCOL:   return "GSCOL";
    case /* X'0C' */ OCODE_GSMX:    return "GSMX";
    case /* X'0D' */ OCODE_GSBMX:   return "GSBMX";
    case /* X'11' */ OCODE_GSFLW:   return "GSFLW";
    case /* X'18' */ OCODE_GSLT:    return "GSLT";
    case /* X'19' */ OCODE_GSLW:    return "GSLW";
    case /* X'1A' */ OCODE_GSLE:    return "GSLE";
    case /* X'1B' */ OCODE_GSLJ:    return "GSLJ";
//    case /* X'20' */ OCODE_GSCLT:    return "GSCLT";  AFP GOCA
    case /* X'21' */ OCODE_GSCP:    return "GSCP";
    case /* X'22' */ OCODE_GSAP:    return "GSAP";
    case /* X'26' */ OCODE_GSECOL:  return "GSECOL";
    case /* X'28' */ OCODE_GSPT:    return "GSPT";
    case /* X'29' */ OCODE_GSMT:    return "GSMT";
    case /* X'33' */ OCODE_GSCC:    return "GSCC";
//    case /* X'34' */ OCODE_GSCA:    return "GSCA";
//    case /* X'35' */ OCODE_GSCH:    return "GSCH";
    case /* X'37' */ OCODE_GSMC:    return "GSMC";
    case /* X'38' */ OCODE_GSCS:    return "GSCS";
    case /* X'39' */ OCODE_GSCR:    return "GSCR";
    case /* X'3A' */ OCODE_GSCD:    return "GSCD";
    case /* X'3B' */ OCODE_GSMP:    return "GSMP";  // Obsolete in AFP
    case /* X'3C' */ OCODE_GSMS:    return "GSMS";
    case /* X'3E' */ OCODE_GEPROL:  return "GEPROL";
//    case /* X'5E' */ OCODE_GECP:    return "GECP"; AFP COCA
    case /* X'60' */ OCODE_GEAR:    return "GEAR";
    case /* X'68' */ OCODE_GBAR:    return "GBAR";
    case /* X'80' */ OCODE_GCBOX:   return "GCBOX";
    case /* X'81' */ OCODE_GCLINE:  return "GCLINE";
    case /* X'82' */ OCODE_GCMRK:   return "GCMRK";
    case /* X'83' */ OCODE_GCCHST:  return "GCCHST";
    case /* X'85' */ OCODE_GCFLT:   return "GCFLT";
    case /* X'87' */ OCODE_GCFARC:  return "GCFARC";
    case /* X'91' */ OCODE_GCBIMG:  return "GCBIMG";
    case /* X'92' */ OCODE_GIMD:    return "GIMD";
    case /* X'93' */ OCODE_GEIMG:   return "GEIMG";
    case /* X'A0' */ OCODE_GSPRP:   return "GSPRP";
    case /* X'A1' */ OCODE_GCRLINE: return "GCRLINE";
    case /* X'A3' */ OCODE_GCPARC:  return "GCPARC";
    case /* X'A5' */ OCODE_GCBEZ:   return "GCBEZ"; //?? name differs
//    case /* X'B2' */ OCODE_GSPCOL:  return "GSPCOL"; AFP GOCA
    case /* X'C0' */ OCODE_GBOX:    return "GBOX";
    case /* X'C1' */ OCODE_GLINE:   return "GLINE";
    case /* X'C2' */ OCODE_GMRK:    return "GMRK";
    case /* X'C3' */ OCODE_GCHST:   return "GCHST";
    case /* X'C4' */ OCODE_GFLT:    return "GFLT";
    case /* X'C7' */ OCODE_GFARC:   return "GFARC";
    case /* X'D1' */ OCODE_GBIMG:   return "GBIMG";
//    case /* X'DE' */ OCODE_GBCP:   return "GBCP"; AFP GOCA
//    case /* X'DF' */ OCODE_GDPT:   return "GDPT"; AFP GOCA
    case /* X'E1' */ OCODE_GRLINE:  return "GRLINE";
    case /* X'E3' */ OCODE_GPARC:   return "GPARC";
    case /* X'E5' */ OCODE_GBEZ:   return "GBEZ"; //??? name differs

    case OCODE_GESD:    return "GESD";
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

