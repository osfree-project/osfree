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
 * @def GOCA_MAX_POINTS
 *
 * @def GOCA_MAX_POINTS
 * @brief Maximum point count of a single long-format order.
 * LENGTH is one byte (at most 255). For GOCA_COORD_BYTE the point size
 * is 2 bytes, giving 127 points plus the prepended current position
 * (128 total). For GOCA_COORD_SHORT (4 bytes per point) the maximum is
 * 63 points (64 total). For GOCA_COORD_LONG (8 bytes per point) the
 * maximum is 31 points (32 total).
 */
#define GOCA_MAX_POINTS  (255u / 2u + 1u)

/*!
 * @brief Fill POINTL[] from packed coordinates.
 *
 * Reads nPts consecutive points, each two geometric parameters wide,
 * using the context's coordSize and coordEndian.
 *
 * @param [in]  c      GOCA context (coordSize, coordEndian).
 * @param [in]  pData  Pointer to the first packed point.
 * @param [in]  nPts   Number of points to convert.
 * @param [out] pptl   Destination array of at least nPts elements.
 */
static void FillPtlArray(GOCA_CTX *c, const BYTE *pData, ULONG nPts, PPOINTL pptl)
{
    ULONG ptSize, i;

    ptSize = c->coordSize << 1;  /* x + y = 2 * coordSize */
    for (i = 0; i < nPts; i++) {
        const BYTE *pPt = pData + i * ptSize;
        pptl[i].x = GocaGetCoord(c, pPt);
        pptl[i].y = GocaGetCoord(c, pPt + c->coordSize);
    }
}

/* ======================================================================== */
/*  Attribute stack                                                         */
/* ======================================================================== */

/*!
 * @brief Reserve the next attribute-stack entry.
 * The entry is zeroed so that unused masks and union members are
 * deterministic.
 * @param [in] c  GOCA context.
 * @return Pointer to the reserved entry, or NULL when the stack is full.
 * @retval NULL  Stack depth limit reached (GOCA_STACK_DEPTH).
 */
static GOCA_STACK_ENTRY *GocaStackPush(GOCA_CTX *c)
{
    GOCA_STACK_ENTRY *e;

    if (c->uStackDepth >= GOCA_STACK_DEPTH) {
        return NULL;
    }
    e = &c->aStack[c->uStackDepth++];
    memset(e, 0, sizeof(*e));
    return e;
}

/*!
@brief Save primitive attributes before a Push-and-Set order changes them.
Only the primitive types with a non-zero mask are read from the DC.
A shorthand order passes the same attribute mask for every primitive
type it affects, so one Pop restores all of them.
@param [in] c         GOCA context.
@param [in] usLine    LBB_* mask, 0 to skip the line bundle.
@param [in] usChar    CBB_* mask, 0 to skip the character bundle.
@param [in] usMarker  MBB_* mask, 0 to skip the marker bundle.
@param [in] usArea    ABB_* mask, 0 to skip the area bundle.
@param [in] usImage   IBB_* mask, 0 to skip the image bundle.
@return GOCA_OK, or GOCA_EC_STACK_OVERFLOW when the stack is full.
@retval GOCA_OK                 Attributes saved.
@retval GOCA_EC_STACK_OVERFLOW  Stack depth limit reached.
*/
static LONG GocaPushAttr(GOCA_CTX *c, USHORT usLine, USHORT usChar,
                         USHORT usMarker, USHORT usArea, USHORT usImage)
{
    GOCA_STACK_ENTRY *e;
    LONG lDefs;

    GocaCallEnter("L=%04X C=%04X M=%04X A=%04X I=%04X depth=%u",
                  (unsigned)usLine, (unsigned)usChar, (unsigned)usMarker,
                  (unsigned)usArea, (unsigned)usImage,
                  (unsigned)c->uStackDepth);

    e = GocaStackPush(c);
    if (!e) {
        GocaCallResult("GOCA_EC_STACK_OVERFLOW");
        return GOCA_EC_STACK_OVERFLOW;
    }

    e->uchKind  = GOCA_STACK_ATTR;
    e->usLine   = usLine;
    e->usChar   = usChar;
    e->usMarker = usMarker;
    e->usArea   = usArea;
    e->usImage  = usImage;

    if (usLine) {
        lDefs = GreGetAttributes(c->hdc, PRIM_LINE, (ULONG)usLine,
                                 (PBUNDLE)&e->u.bundles.lb);
        e->usLineDefs = (lDefs == GPI_ALTERROR) ? 0 : (USHORT)(lDefs & usLine);
    }
    if (usChar) {
        lDefs = GreGetAttributes(c->hdc, PRIM_CHAR, (ULONG)usChar,
                                 (PBUNDLE)&e->u.bundles.cb);
        e->usCharDefs = (lDefs == GPI_ALTERROR) ? 0 : (USHORT)(lDefs & usChar);
    }
    if (usMarker) {
        lDefs = GreGetAttributes(c->hdc, PRIM_MARKER, (ULONG)usMarker,
                                 (PBUNDLE)&e->u.bundles.mb);
        e->usMarkerDefs = (lDefs == GPI_ALTERROR) ? 0 : (USHORT)(lDefs & usMarker);
    }
    if (usArea) {
        lDefs = GreGetAttributes(c->hdc, PRIM_AREA, (ULONG)usArea,
                                 (PBUNDLE)&e->u.bundles.ab);
        e->usAreaDefs = (lDefs == GPI_ALTERROR) ? 0 : (USHORT)(lDefs & usArea);
    }
    if (usImage) {
        lDefs = GreGetAttributes(c->hdc, PRIM_IMAGE, (ULONG)usImage,
                                 (PBUNDLE)&e->u.bundles.ib);
        e->usImageDefs = (lDefs == GPI_ALTERROR) ? 0 : (USHORT)(lDefs & usImage);
    }

    GocaCallResult("GOCA_OK (depth=%u)", (unsigned)c->uStackDepth);
    return GOCA_OK;
}

/*!
@brief Save the current position before a Push-and-Set order changes it.
@param [in] c  GOCA context.
@return GOCA_OK, or GOCA_EC_STACK_OVERFLOW when the stack is full.
@retval GOCA_OK                 Position saved.
@retval GOCA_EC_STACK_OVERFLOW  Stack depth limit reached.
*/
static LONG GocaPushCurPos(GOCA_CTX *c)
{
    GOCA_STACK_ENTRY *e;

    GocaCallEnter("depth=%u", (unsigned)c->uStackDepth);

    e = GocaStackPush(c);
    if (!e) {
        GocaCallResult("GOCA_EC_STACK_OVERFLOW");
        return GOCA_EC_STACK_OVERFLOW;
    }

    e->uchKind = GOCA_STACK_CURPOS;
    GreGetCurrentPosition(c->hdc, &e->u.ptlCurPos);

    GocaCallResult("GOCA_OK (pos=(%ld,%ld) depth=%u)",
                  (long)e->u.ptlCurPos.x, (long)e->u.ptlCurPos.y,
                  (unsigned)c->uStackDepth);
    return GOCA_OK;
}

/*!
@brief Save the arc parameters before a Push-and-Set order changes them.
Arc parameters are a drawing process control, not a primitive bundle
attribute, but they share the attribute stack.
@param [in] c  GOCA context.
@return GOCA_OK, or GOCA_EC_STACK_OVERFLOW when the stack is full.
@retval GOCA_OK                 Parameters saved.
@retval GOCA_EC_STACK_OVERFLOW  Stack depth limit reached.
*/
static LONG GocaPushArcParams(GOCA_CTX *c)
{
    GOCA_STACK_ENTRY *e;

    GocaCallEnter("depth=%u", (unsigned)c->uStackDepth);

    e = GocaStackPush(c);
    if (!e) {
        GocaCallResult("GOCA_EC_STACK_OVERFLOW");
        return GOCA_EC_STACK_OVERFLOW;
    }

    e->uchKind = GOCA_STACK_ARCPARAMS;
    GreGetArcParameters(c->hdc, &e->u.ap);

    GocaCallResult("GOCA_OK (P=%ld Q=%ld R=%ld S=%ld depth=%u)",
                  (long)e->u.ap.lP, (long)e->u.ap.lQ,
                  (long)e->u.ap.lR, (long)e->u.ap.lS,
                  (unsigned)c->uStackDepth);
    return GOCA_OK;
}

/*!
@brief Restore one attribute-stack entry (GPOP, X'3F').
The most recently pushed entry is unstacked and applied to the DC.
Only the parts recorded by the matching Push are restored. One GPOP
order restores one entry; a count is expressed by repeating the order.
@param [in] c  GOCA context.
@return GOCA_OK.
@retval GOCA_OK  Entry restored, or the stack was empty.
*/
static LONG GocaPop(GOCA_CTX *c)
{
    GOCA_STACK_ENTRY *e;

    GocaCallEnter("depth=%u", (unsigned)c->uStackDepth);

    if (c->uStackDepth == 0) {
        GocaCallResult("GOCA_OK (stack empty)");
        return GOCA_OK;
    }

    e = &c->aStack[--c->uStackDepth];

    switch (e->uchKind) {
    case GOCA_STACK_ATTR:
        if (e->usLine) {
            GreSetAttributes(c->hdc, PRIM_LINE,
                             (ULONG)e->usLineDefs, (ULONG)e->usLine,
                             (PBUNDLE)&e->u.bundles.lb);
        }
        if (e->usChar) {
            GreSetAttributes(c->hdc, PRIM_CHAR,
                             (ULONG)e->usCharDefs, (ULONG)e->usChar,
                             (PBUNDLE)&e->u.bundles.cb);
        }
        if (e->usMarker) {
            GreSetAttributes(c->hdc, PRIM_MARKER,
                             (ULONG)e->usMarkerDefs, (ULONG)e->usMarker,
                             (PBUNDLE)&e->u.bundles.mb);
        }
        if (e->usArea) {
            GreSetAttributes(c->hdc, PRIM_AREA,
                             (ULONG)e->usAreaDefs, (ULONG)e->usArea,
                             (PBUNDLE)&e->u.bundles.ab);
        }
        if (e->usImage) {
            GreSetAttributes(c->hdc, PRIM_IMAGE,
                             (ULONG)e->usImageDefs, (ULONG)e->usImage,
                             (PBUNDLE)&e->u.bundles.ib);
        }
        break;
    case GOCA_STACK_CURPOS:
        GreSetCurrentPosition(c->hdc, &e->u.ptlCurPos);
        break;
    case GOCA_STACK_ARCPARAMS:
        GreSetArcParameters(c->hdc, &e->u.ap);
        break;
    }

    GocaCallResult("GOCA_OK (kind=%u depth=%u)",
                  (unsigned)e->uchKind, (unsigned)c->uStackDepth);
    return GOCA_OK;
}

/* ======================================================================== */
/*  Long-order handlers                                                     */
/* ======================================================================== */

/*!
 * @brief GSCP (0x21) Set Current Position. Data: x,y.
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order (code, length, operands).
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Declared length not equal to two
 *                                geometric parameters.
 */
static LONG DoGSCP(GOCA_CTX *c, const BYTE *p)
{
    POINTL pt;

    GocaCallEnter("p=%p", p);

    if (p[1] != 2u * (ULONG)c->coordSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }
    pt = PtsToPtl(c, p + 2);
    GocaTrace(c, "[GOCA]   GSCP: pos=(%ld,%ld)\n",
              (long)pt.x, (long)pt.y);
    GreSetCurrentPosition(c->hdc, &pt);

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
 * @param [in] bGiven TRUE for given position.
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length inconsistent with point size.
 */
static LONG DoLine(GOCA_CTX *c, const BYTE *p, BOOL bGiven)
{
    ULONG  len, nPts;
    POINTL aptl[GOCA_MAX_POINTS];
    ULONG  pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, bGiven=%d", p, (int)bGiven);

    len = p[1];
    if (len % pointSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }

    nPts = len / pointSize;
    FillPtlArray(c, p + 2, nPts, aptl);
    if (bGiven) {
        if (nPts == 0) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        GreSetCurrentPosition(c->hdc, &aptl[0]);
        nPts--;
    }
    GrePolyLine(c->hdc, &aptl[bGiven], (LONG)(nPts));

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GMRK (0xC2) / GCMRK (0x82).
 *
 * Draws one marker at every position of the order. For GMRK the first
 * position is taken from the operands and becomes the current position
 * before drawing; at least one point is required. For GCMRK a marker is
 * drawn at the current position first, then at every operand point; a
 * zero-length order draws a single marker at the current position.
 * The current position ends at the centre of the last marker.
 *
 * @note GrePolyMarker cPoints is the total number of markers to be
 *       drawn; the first one is placed at the current position and the
 *       remaining cPoints-1 are taken from the point array.
 *
 * @param [in] c    GOCA context.
 * @param [in] p    Pointer to the order.
 * @param [in] bGiven TRUE for given position.
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length inconsistent with point size,
 *                                or GMRK without points.
 */
static LONG DoMark(GOCA_CTX *c, const BYTE *p, BOOL bGiven)
{
    ULONG  len, nPts;
    POINTL aptl[GOCA_MAX_POINTS];
    ULONG  pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, bGiven=%d", p, (int)bGiven);

    len = p[1];
    if (len % pointSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }

    nPts = len / pointSize;
    FillPtlArray(c, p + 2, nPts, aptl);
    if (bGiven) {
        if (nPts == 0) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        GreSetCurrentPosition(c->hdc, &aptl[0]);
        nPts--;
    }
    GrePolyMarker(c->hdc, &aptl[bGiven], (LONG)(nPts+1));

    GocaCallResult("GOCA_OK");
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
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length inconsistent with offsets.
 */
static LONG DoRelativeLine(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG  len, nPts, nTotal, i, off;
    POINTL aptl[GOCA_MAX_POINTS];
    SHORT  x, y;
    ULONG  pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];

    off = 0;
    if (bCur) {
        /* GCRLINE: length is 2 * count. */
        if (len == 0 || (len & 1)) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        GreGetCurrentPosition(c->hdc, &aptl[0]);
        x = (SHORT)aptl[0].x;
        y = (SHORT)aptl[0].y;
        off = 0;
    } else {
        /* GRLINE: pointSize-byte start point, then 2 * count. */
        if (len < pointSize || ((len - pointSize) & 1)) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        x = (SHORT)GocaGetCoord(c, p + 2);
        y = (SHORT)GocaGetCoord(c, p + 2 + c->coordSize);
        off = pointSize;
    }

    nPts   = (len - off) / 2;
    nTotal = nPts + 1;

    aptl[0].x = x;
    aptl[0].y = y;

    for (i = 0; i < nPts; i++) {
        CHAR dx = (CHAR)p[2 + off + i * 2];
        CHAR dy = (CHAR)p[2 + off + i * 2 + 1];
        x = (SHORT)(x + dx);
        y = (SHORT)(y + dy);
        aptl[i + 1].x = x;
        aptl[i + 1].y = y;
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCRLINE" : "GRLINE", (unsigned)nTotal,
              (long)aptl[nTotal-1].x, (long)aptl[nTotal-1].y);

    GrePolyLine(c->hdc, aptl, (LONG)nTotal);

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
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed (empty string included).
 * @retval GOCA_EC_INVALID_LENGTH Operand shorter than one point.
 */
static LONG DoCharString(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG       len, cch;
    POINTL      ptStart;
    const BYTE *pStr;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];

    if (bCur) {
        /* GCCHST: no explicit point, string at current pos. */
        GreGetCurrentPosition(c->hdc, &ptStart);
        pStr    = p + 2;
        cch     = len;
    } else {
        /* GCHST: pointSize-byte point, then string. */
        if (len < pointSize) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
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
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand too short for corners/axes.
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
    if (len < 2) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }
    fbFlags = p[2];

    if (bCur) {
        /* GCBOX: corner (x2,y2) after flags+1 pad, optional axes. */
        if (len < 2u + pointSize) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        GreGetCurrentPosition(c->hdc, &ptStart);
        x1 = ptStart.x;
        y1 = ptStart.y;

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
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
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

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GPARC (0xE3) / GCPARC (0xA3).
 *
 * Layout (GCPARC): centre x,y, multiplier (FIXED88 or FIXED),
 *          start angle 32-bit, sweep angle 32-bit.
 *
 * GPARC additionally has a start point at the front.
 * Per pmord.h:
 *   SHORT variant: ORDERS_GCPARC  - FIXED88 multiplier (2 bytes)
 *   LONG  variant: ORDERL_GCPARC  - FIXED   multiplier (4 bytes)
 *   Angles are always LONG (4 bytes) in both variants.
 *   Multiplier and angles follow coordEndian.
 *
 * @param [in] c      GOCA context.
 * @param [in] p      Pointer to the order.
 * @param [in] bGiven TRUE for GPARC (start point at given position).
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length incorrect.
 * @retval GOCA_EC_PARC_NEGATIVE_START  Negative START angle.
 * @retval GOCA_EC_PARC_NEGATIVE_SWEEP  Negative SWEEP angle.
 */
static LONG DoPartialArc(GOCA_CTX *c, const BYTE *p, BOOL bGiven)
{
    ULONG       len, off, multSize, expectedLen;
    const BYTE *q;
    POINTL      ptCenter;
    FIXED       fxMult, fxStart, fxSweep;
    LONG        rawStart, rawSweep;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, bGiven=%d", p, (int)bGiven);

    len = p[1];
    q   = p + 2;
    off = 0;
    multSize = (c->coordSize == GOCA_COORD_LONG) ? 4u : 2u;

    /* Exact length check */
    expectedLen = (bGiven ? 2u * pointSize : pointSize) + multSize + 8u;
    if (len != expectedLen) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }

    if (bGiven) {
        /* GPARC: read start point, set current position to draw line from it */
        POINTL ptStart;
        ptStart = PtsToPtl(c, q);
        off += pointSize;
        GreSetCurrentPosition(c->hdc, &ptStart);
    }

    ptCenter = PtsToPtl(c, q + off);
    off += pointSize;

    fxMult = GocaGetMult(c, q + off);
    off += multSize;

    /* Angles: always 32-bit signed, coordEndian */
    if (c->coordEndian == GOCA_ENDIAN_LE) {
        rawStart = (LONG)(((ULONG)q[off+3] << 24) | ((ULONG)q[off+2] << 16) |
                          ((ULONG)q[off+1] << 8)  |  (ULONG)q[off]);
        rawSweep = (LONG)(((ULONG)q[off+7] << 24) | ((ULONG)q[off+6] << 16) |
                          ((ULONG)q[off+5] << 8)  |  (ULONG)q[off+4]);
    } else {
        rawStart = (LONG)GocaGetU32(q + off);
        rawSweep = (LONG)GocaGetU32(q + off + 4);
    }

    /* AFPC p. 120: negative angles are drawing process checks */
    if (rawStart < 0) {
        GocaCallResult("GOCA_EC_PARC_NEGATIVE_START");
        return GOCA_EC_PARC_NEGATIVE_START;
    }
    if (rawSweep < 0) {
        GocaCallResult("GOCA_EC_PARC_NEGATIVE_SWEEP");
        return GOCA_EC_PARC_NEGATIVE_SWEEP;
    }

    fxStart = (FIXED)rawStart;
    fxSweep = (FIXED)rawSweep;

    GocaTrace(c, "[GOCA]   %s: c=(%ld,%ld) mult=%08lX start=%08lX sweep=%08lX\n",
              bGiven ? "GPARC" : "GCPARC",
              (long)ptCenter.x, (long)ptCenter.y,
              (unsigned long)(ULONG)fxMult,
              (unsigned long)fxStart, (unsigned long)fxSweep);

    GrePartialArc(c->hdc, &ptCenter, fxMult, fxStart, fxSweep);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GFARC (0xC7) / GCFARC (0x87).
 * Data layout:
 * GCFARC: multiplier (FIXED88 or FIXED depending on coordSize)
 * GFARC:  centre x,y, then multiplier
 *
 * Multiplier size is 2 bytes for BYTE/SHORT, 4 bytes for LONG.
 * Full Arc is a line primitive; only the boundary is stroked.
 *
 * @param [in] c      GOCA context.
 * @param [in] p      Pointer to the order.
 * @param [in] bGiven TRUE for GFARC (centre at given position).
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length incorrect.
 */
static LONG DoFullArc(GOCA_CTX *c, const BYTE *p, BOOL bGiven)
{
    ULONG       len, off;
    const BYTE *q;
    FIXED       fxMult;
    ULONG       pointSize = 2u * (ULONG)c->coordSize;
    ULONG       multSize  = (c->coordSize == GOCA_COORD_LONG) ? 4u : 2u;

    GocaCallEnter("p=%p, bGiven=%d", p, (int)bGiven);

    len = p[1];
    q   = p + 2;
    off = 0;

    if (bGiven) {
        POINTL ptCenter;
        if (len != pointSize + multSize) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        ptCenter = PtsToPtl(c, q);
        off += pointSize;
        GreSetCurrentPosition(c->hdc, &ptCenter);
        GocaTrace(c, "[GOCA]   GFARC: center=(%ld,%ld)", (long)ptCenter.x, (long)ptCenter.y);
    } else {
        if (len != multSize) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        GocaTrace(c, "[GOCA]   GCFARC: center=current");
    }

    fxMult = GocaGetMult(c, q + off);
    GocaTrace(c, " mult=%08lX\n", (unsigned long)(ULONG)fxMult);

    /* Full Arc is a line primitive (curved line), stroke boundary only */
    GreFullArcBoundary(c->hdc, fxMult);

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
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length inconsistent with point size.
 */
static LONG DoBezier(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG  len, nPts, i;
    POINTL aptl[GOCA_MAX_POINTS];
    ULONG  pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];

    if (bCur) {
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        nPts = len / pointSize + 1;
        GreGetCurrentPosition(c->hdc, &aptl[0]);
        for (i = 0; i < nPts - 1; i++) {
            const BYTE *pPt = p + 2 + i * pointSize;
            aptl[i+1].x = GocaGetCoord(c, pPt);
            aptl[i+1].y = GocaGetCoord(c, pPt + c->coordSize);
        }
    } else {
        if (len < 2u * pointSize || (len % pointSize)) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        nPts = len / pointSize;
        FillPtlArray(c, p + 2, nPts, aptl);
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCBEZ" : "GBEZ", (unsigned)nPts,
              (long)aptl[nPts-1].x, (long)aptl[nPts-1].y);

    GrePolySpline(c->hdc, aptl, (LONG)nPts);

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
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Fewer than two points supplied.
 */
static LONG DoFillet(GOCA_CTX *c, const BYTE *p, ULONG cb, BOOL bCur)
{
    ULONG  len, nPts, i;
    POINTL aptl[GOCA_MAX_POINTS];
    ULONG  pointSize = 2u * (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u, bCur=%d", p, (unsigned)cb, (int)bCur);

    len = p[1];

    if (bCur) {
        /* GCFLT: points after current position; need at least 1 point. */
        if (len == 0 || (len % pointSize)) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        nPts = len / pointSize + 1;
        GreGetCurrentPosition(c->hdc, &aptl[0]);
        for (i = 0; i < nPts - 1; i++) {
            const BYTE *pPt = p + 2 + i * pointSize;
            aptl[i+1].x = GocaGetCoord(c, pPt);
            aptl[i+1].y = GocaGetCoord(c, pPt + c->coordSize);
        }
    } else {
        /* GFLT: explicit points; need at least 2 points for fillet. */
        if (len < 2u * pointSize || (len % pointSize)) {
            GocaCallResult("GOCA_EC_INVALID_LENGTH");
            return GOCA_EC_INVALID_LENGTH;
        }
        nPts = len / pointSize;
        FillPtlArray(c, p + 2, nPts, aptl);
    }

    GocaTrace(c, "[GOCA]   %s: %u pts, end=(%ld,%ld)\n",
              bCur ? "GCFLT" : "GFLT", (unsigned)nPts,
              (long)aptl[nPts-1].x, (long)aptl[nPts-1].y);

    GrePolyFillet(c->hdc, aptl, (LONG)nPts);

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
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                Order processed.
 * @retval GOCA_EC_INVALID_LENGTH Operand length not equal to four
 *                                geometric parameters.
 */
static LONG DoSetArcParams(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    ARCPARAMS ap;
    ULONG     paramSize = (ULONG)c->coordSize;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (p[1] != 4u * paramSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
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
 * Reserved flag bits and the res byte must be zero: these fields are a
 * full GOCA (pmord.h ORDERS_GSCC / ORDERL_GSCC) and are absent
 * from the AFP GOCA syntax of this order.
 * flags.setzero (0x80, GSCC_ZEROZERO):
 *   0 - a cell size of zero sets drawing default;
 *   1 - a cell size of zero sets to zero.
 * The drawing default is taken from the DC default attribute bundle
 * (GreGetDefaultAttributes, PRIM_CHAR / CBB_BOX).
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                   Order processed and CBB_BOX applied.
 * @retval GOCA_EC_INVALID_LENGTH    Operand too short or trailing bytes present.
 * @retval GOCA_EC_RESERVED_NOT_ZERO Reserved flag bits or res byte non-zero.
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
    CHARBUNDLE bun;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    dataLen = p[1];
    off = 2;
    rem = dataLen;
    if (rem < 2u * intSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
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
            GocaCallResult("GOCA_EC_RESERVED_NOT_ZERO");
            return GOCA_EC_RESERVED_NOT_ZERO;
        }
    }
    if (rem != 0) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }

    bZero = (cxInt == 0 && cyInt == 0 && cxFrac == 0 && cyFrac == 0);
    if (bZero && !(fbFlags & GSCC_ZEROZERO)) {
        /* setzero=0: a cell size of zero sets drawing default.
         * The drawing default is taken from the DC default attribute
         * bundle (GreGetDefaultAttributes, PRIM_CHAR / CBB_BOX). */
        CHARBUNDLE cbDef;
        if (GreGetDefaultAttributes(c->hdc, PRIM_CHAR, CBB_BOX,
                                    (PBUNDLE)&cbDef)) {
            bun.sizfxCell = cbDef.sizfxCell;
        } else {
            bun.sizfxCell.cx = 0;
            bun.sizfxCell.cy = 0;
        }
    } else {
        /* setzero=1: a cell size of zero sets to zero;
         * non-zero cell: value as specified. */
        bun.sizfxCell.cx = MAKEFIXED(cxInt, cxFrac);
        bun.sizfxCell.cy = MAKEFIXED(cyInt, cyFrac);
    }

    GocaTrace(c, "[GOCA]   GSCC: cell=(%ld.%04X,%ld.%04X) setzero=%u\n",
              (long)cxInt, (unsigned)cxFrac, (long)cyInt, (unsigned)cyFrac,
              (unsigned)((fbFlags & GSCC_ZEROZERO) != 0));
    GreSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_BOX, (PBUNDLE)&bun);

    GocaCallResult("GOCA_OK");
    return GOCA_OK;
}

/*!
 * @brief GSMC (0x37) / GPSMC (0x77) Set Marker Cell.
 *
 * Layout per pmord.h (ORDERS_GSMC / ORDERL_GSMC) and 64F0277 Vol.2:
 *   (len, cellx, celly, [flags, res])
 * cellx/celly: GROSOL (signed, coordSize/coordEndian aware);
 * flags/res: optional pair, res required when flags present.
 * Reserved flag bits and the res byte must be zero (AFPC-0008-03 p. 152:
 * RES bits 1-15, "only valid value").
 * flags.setzero (0x80 = GSMC_ZEROZERO):
 *   0 - a cell size of zero sets drawing default;
 *   1 - a cell size of zero sets to zero.
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                   Order processed and MBB_BOX applied.
 * @retval GOCA_EC_INVALID_LENGTH    Operand too short or trailing bytes present.
 * @retval GOCA_EC_RESERVED_NOT_ZERO Reserved flag bits or res byte non-zero.
 */
static LONG DoSetMarkerCell(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    LONG  cx, cy;
    UCHAR fbFlags = 0, uchRes = 0;
    ULONG intSize = (ULONG)c->coordSize;
    ULONG dataLen, rem, off;
    BOOL  bZero;
    MARKERBUNDLE bun;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    dataLen = p[1];
    off = 2;
    rem = dataLen;
    if (rem < 2u * intSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }
    cx = GocaGetCoord(c, p + off); off += intSize; rem -= intSize;
    cy = GocaGetCoord(c, p + off); off += intSize; rem -= intSize;

    /* Optional flags + res; res must follow flags */
    if (rem >= 2u) {
        fbFlags = p[off]; off += 1u;
        uchRes  = p[off]; off += 1u;
        rem -= 2u;
        if ((fbFlags & (UCHAR)~(UCHAR)GSMC_ZEROZERO) != 0 || uchRes != 0) {
            GocaCallResult("GOCA_EC_RESERVED_NOT_ZERO");
            return GOCA_EC_RESERVED_NOT_ZERO;
        }
    }
    if (rem != 0) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }

    bZero = (cx == 0 && cy == 0);
    if (bZero && !(fbFlags & GSMC_ZEROZERO)) {
        /* setzero=0: a cell size of zero sets drawing default.
         * MARKERBUNDLE: sizfxCell == 0 is the GRE sentinel for the
         * default marker cell (nominal symbol size). */
        bun.sizfxCell.cx = 0;
        bun.sizfxCell.cy = 0;
    } else {
        /* Non-zero cell as specified, or zero + setzero=1 (literal zero). */
        bun.sizfxCell.cx = MAKEFIXED(cx, 0);
        bun.sizfxCell.cy = MAKEFIXED(cy, 0);
    }

    GocaTrace(c, "[GOCA]   GSMC: cell=(%ld,%ld) setzero=%u\n",
              (long)cx, (long)cy,
              (unsigned)((fbFlags & GSMC_ZEROZERO) != 0));
    GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_BOX, (PBUNDLE)&bun);

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
 * specified point.  Reserved flag bits and res must be zero
 * (AFPC-0008-03 p. 157: RES1 bits 1-7 and RES2, "only valid value").
 *
 * @param [in] c   GOCA context.
 * @param [in] p   Pointer to the order.
 * @param [in] cb  Bytes available from p to the end of the stream.
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                   Order processed and ABB_REF_POINT applied.
 * @retval GOCA_EC_INVALID_LENGTH    Operand too short for flags, res and point.
 * @retval GOCA_EC_RESERVED_NOT_ZERO Reserved flag bits or res byte non-zero.
 */
static LONG DoSetPatternRefPoint(GOCA_CTX *c, const BYTE *p, ULONG cb)
{
    POINTL pt;
    UCHAR  fbFlags, uchRes;
    ULONG  pointSize = 2u * (ULONG)c->coordSize;
    AREABUNDLE bun;

    GocaCallEnter("p=%p, cb=%u", p, (unsigned)cb);

    if (p[1] < 2u + pointSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }
    fbFlags = p[2];
    uchRes  = p[3];
    if ((fbFlags & (UCHAR)~(UCHAR)GSPRP_DEFAULT) != 0 || uchRes != 0) {
        GocaCallResult("GOCA_EC_RESERVED_NOT_ZERO");
        return GOCA_EC_RESERVED_NOT_ZERO;
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
    bun.ptlRefPoint = pt;
    GreSetAttributes(c->hdc, PRIM_AREA, 0UL, ABB_REF_POINT, (PBUNDLE)&bun);

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
 * low bits) and the res byte are reserved and must be zero. The order
 * is a PM GPI extension and has no AFP GOCA counterpart.
 * The rectangle is normalized to xLeft <= xRight, yBottom <= yTop and
 * is passed to GreCharStringPos as the clip rectangle only when CLIP
 * is set; with DRAWRECT its boundary is stroked via GreBoxBoundary.
 * adx, when present (remaining length == cchString * coordSize), is
 * converted to LONG[] and passed as the per-character increment array.
 * With LEAVEPOS the current position read from the DC before the string
 * is restored afterwards; with MOVEPOS the DC keeps the position reached
 * at the end of the string.
 * UNDERSCORE/STRIKEOUT are reported in the trace: the cached
 * CHARBUNDLE exposes no character-definition bits in the provided
 * headers.
 *
 * @param [in] c      GOCA context (coordSize, coordEndian, cached
 *                    bundles).
 * @param [in] p      Pointer to the extended order (0xFE, qualifier,
 *                    SWPUSHORT length, operands).
 * @param [in] cb     Bytes available from p to the end of the stream
 *                    (the wire order length).
 * @param [in] bGiven TRUE for GCHSTE (string at the given rectangle
 *                    corner), FALSE for GCCHSTE (current position).
 * @return GOCA_OK or an exception code.
 * @retval GOCA_OK                   Order processed (empty string included).
 * @retval GOCA_EC_INVALID_LENGTH    Data length inconsistent with rect/string/adx.
 * @retval GOCA_EC_RESERVED_NOT_ZERO Reserved flag bits or res byte non-zero.
 * @retval GOCA_ERR_NOMEM            adx array allocation failed.
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

    dataLen = GocaGetU16(p + 2);               /* SWPUSHORT, always BE */
    if (dataLen < minData) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }

    q       = p + 4;
    fbFlags = q[0];
    uchRes  = q[1];
    if ((fbFlags & 0x23u) != 0 || uchRes != 0) {
        GocaCallResult("GOCA_EC_RESERVED_NOT_ZERO");
        return GOCA_EC_RESERVED_NOT_ZERO;
    }

    ptCorner0 = PtsToPtl(c, q + 2);
    ptCorner1 = PtsToPtl(c, q + 2 + pointSize);
    cch       = GocaGetU16(q + 2 + 2u * pointSize);

    consumed = minData + (ULONG)cch;
    if (dataLen < consumed) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
    }
    rem = dataLen - consumed;
    if (rem != 0 && rem != (ULONG)cch * (ULONG)c->coordSize) {
        GocaCallResult("GOCA_EC_INVALID_LENGTH");
        return GOCA_EC_INVALID_LENGTH;
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

    GreGetCurrentPosition(c->hdc, &ptOld);
    if (bGiven) {
        ptStart = ptCorner0;
    } else {
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

    /* LEAVEPOS: restore the position read before the string;
       MOVEPOS: the DC already holds the position reached at the end. */
    if (fbFlags & GCCHSTE_LEAVEPOS) {
        GreSetCurrentPosition(c->hdc, &ptOld);
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
 * bundles from the device. Must be called with a valid Gre* @c hdc.
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
 * @return GOCA_OK on success, an exception code on failure.
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
            c->lError     = GOCA_EC_TRUNCATED_ORDER;
            c->uErrOffset = c->uPos;
            c->uErrOrder  = p[0];
            GocaCallResult("GOCA_EC_TRUNCATED_ORDER");
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

            case OCODE_GPOP:
                if (p[1] != 0) { rc = GOCA_EC_RESERVED_NOT_ZERO; break; }
                rc = GocaPop(c);
                break;

            /* ============ colour (2-byte GSCOL / long GSECOL) ============ */
            case OCODE_GPSCOL:
                rc = GocaPushAttr(c, LBB_COLOR, CBB_COLOR, MBB_COLOR,
                                  ABB_COLOR, IBB_COLOR);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSCOL: {
                COLOR clr = (COLOR)p[1];
                LINEBUNDLE   lb;
                CHARBUNDLE   cb;
                MARKERBUNDLE mb;
                AREABUNDLE   ab;
                GocaCallEnter("GSCOL color=%u", (unsigned)clr);
                lb.lColor = cb.lColor = mb.lColor = ab.lColor = (LONG)clr;
                GreSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_COLOR, (PBUNDLE)&lb);
                GreSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_COLOR, (PBUNDLE)&cb);
                GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_COLOR, (PBUNDLE)&mb);
                GreSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_COLOR, (PBUNDLE)&ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSECOL:
                rc = GocaPushAttr(c, LBB_COLOR, CBB_COLOR, MBB_COLOR,
                                  ABB_COLOR, IBB_COLOR);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSECOL: {
                USHORT raw;
                COLOR  clr;
                LINEBUNDLE   lb;
                CHARBUNDLE   cb;
                MARKERBUNDLE mb;
                AREABUNDLE   ab;
                if (p[1] < 2) { rc = GOCA_EC_INVALID_LENGTH; break; }
                raw = GocaGetU16(p + 2);
                GocaCallEnter("GSECOL color=%04X", (unsigned)raw);
                clr = ((raw & 0xFF00) == 0xFF00)
                      ? (COLOR)(raw & 0x00FF)
                      : (COLOR)raw;
                lb.lColor = cb.lColor = mb.lColor = ab.lColor = (LONG)clr;
                GreSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_COLOR, (PBUNDLE)&lb);
                GreSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_COLOR, (PBUNDLE)&cb);
                GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_COLOR, (PBUNDLE)&mb);
                GreSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_COLOR, (PBUNDLE)&ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ mix ============ */
            case OCODE_GPSMX:
                rc = GocaPushAttr(c, LBB_MIX_MODE, CBB_MIX_MODE,
                                  MBB_MIX_MODE, ABB_MIX_MODE, IBB_MIX_MODE);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSMX: {
                LINEBUNDLE   lb;
                CHARBUNDLE   cb;
                MARKERBUNDLE mb;
                AREABUNDLE   ab;
                GocaCallEnter("GSMX mix=%u", (unsigned)p[1]);
                lb.usMixMode = cb.usMixMode = mb.usMixMode = ab.usMixMode = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_MIX_MODE, (PBUNDLE)&lb);
                GreSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_MIX_MODE, (PBUNDLE)&cb);
                GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_MIX_MODE, (PBUNDLE)&mb);
                GreSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_MIX_MODE, (PBUNDLE)&ab);
                GocaCallResult("GOCA_OK");
                break;
            }
            case OCODE_GPSBMX:
                rc = GocaPushAttr(c, LBB_BACK_MIX_MODE, CBB_BACK_MIX_MODE,
                                  MBB_BACK_MIX_MODE, ABB_BACK_MIX_MODE,
                                  IBB_BACK_MIX_MODE);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSBMX: {
                LINEBUNDLE   lb;
                CHARBUNDLE   cb;
                MARKERBUNDLE mb;
                AREABUNDLE   ab;
                GocaCallEnter("GSBMX backmix=%u", (unsigned)p[1]);
                lb.usBackMixMode = cb.usBackMixMode =
                mb.usBackMixMode = ab.usBackMixMode = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_LINE,   0UL, LBB_BACK_MIX_MODE, (PBUNDLE)&lb);
                GreSetAttributes(c->hdc, PRIM_CHAR,   0UL, CBB_BACK_MIX_MODE, (PBUNDLE)&cb);
                GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_BACK_MIX_MODE, (PBUNDLE)&mb);
                GreSetAttributes(c->hdc, PRIM_AREA,   0UL, ABB_BACK_MIX_MODE, (PBUNDLE)&ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ line attributes ============ */
            case OCODE_GPSLT:
                rc = GocaPushAttr(c, LBB_TYPE, 0, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSLT: {
                LINEBUNDLE lb;
                GocaCallEnter("GSLT type=%u", (unsigned)p[1]);
                lb.usType = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_LINE, 0UL, LBB_TYPE, (PBUNDLE)&lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSLW:
                rc = GocaPushAttr(c, LBB_WIDTH, 0, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSLW: {
                LINEBUNDLE lb;
                ULONG flDefs = 0UL;
                ULONG flAttrs = LBB_WIDTH;
                GocaCallEnter("GSLW width=%u", (unsigned)p[1]);
                if (p[1] == 0) {
                    /* X'00' = drawing default: bit must be in both masks */
                    flDefs = LBB_WIDTH;
                    lb.fxWidth = 0; 
                } else {
                    lb.fxWidth = MAKEFIXED(p[1], 0);
                }
                GreSetAttributes(c->hdc, PRIM_LINE, flDefs, flAttrs, (PBUNDLE)&lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSFLW:
                rc = GocaPushAttr(c, LBB_WIDTH, 0, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSFLW: {
                LINEBUNDLE lb;
                UCHAR ip, fp;
                ULONG flDefs = 0UL;
                ULONG flAttrs = LBB_WIDTH;
                if (p[1] != 2) { rc = GOCA_EC_INVALID_LENGTH; break; }
                ip = p[2]; fp = p[3];
                GocaCallEnter("GSFLW width=%u.%02X", (unsigned)ip, (unsigned)fp);
                if (ip == 0 && fp == 0) {
                    /* X'0000' = drawing default: bit must be in both masks */
                    flDefs = LBB_WIDTH;
                    lb.fxWidth = 0;
                } else {
                    lb.fxWidth = MAKEFIXED(ip, (ULONG)fp << 8);
                }
                GreSetAttributes(c->hdc, PRIM_LINE, flDefs, flAttrs, (PBUNDLE)&lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSLE:
                rc = GocaPushAttr(c, LBB_END, 0, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSLE: {
                LINEBUNDLE lb;
                GocaCallEnter("GSLE end=%u", (unsigned)p[1]);
                lb.usEnd = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_LINE, 0UL, LBB_END, (PBUNDLE)&lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSLJ:
                rc = GocaPushAttr(c, LBB_JOIN, 0, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSLJ: {
                LINEBUNDLE lb;
                GocaCallEnter("GSLJ join=%u", (unsigned)p[1]);
                lb.usJoin = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_LINE, 0UL, LBB_JOIN, (PBUNDLE)&lb);
                GocaCallResult("GOCA_OK");
                break;
            }

            /* ============ marker attributes ============ */
            case OCODE_GPSMS:
                rc = GocaPushAttr(c, 0, 0, MBB_SET, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSMS: {
                MARKERBUNDLE mb;
                GocaCallEnter("GSMS set=%u", (unsigned)p[1]);
                mb.usSet = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_SET, (PBUNDLE)&mb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSMT:
                rc = GocaPushAttr(c, 0, 0, MBB_SYMBOL, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
           case OCODE_GSMT: {
               MARKERBUNDLE mb;
               GocaCallEnter("GSMT symbol=%u", (unsigned)p[1]);
               mb.usSymbol = (USHORT)p[1];
               GreSetAttributes(c->hdc, PRIM_MARKER, 0UL, MBB_SYMBOL, (PBUNDLE)&mb);
               GocaCallResult("GOCA_OK");
               break;
           }

            /* ============ area attributes ============ */
            case OCODE_GPSPS:
                rc = GocaPushAttr(c, 0, 0, 0, ABB_SET, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSPS: {
                AREABUNDLE ab;
                GocaCallEnter("GSPS set=%u", (unsigned)p[1]);
                ab.usSet = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_AREA, 0UL, ABB_SET, (PBUNDLE)&ab);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSPT:
                rc = GocaPushAttr(c, 0, 0, 0, ABB_SYMBOL, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
           case OCODE_GSPT: {
               AREABUNDLE ab;
               GocaCallEnter("GSPT symbol=%u", (unsigned)p[1]);
               ab.usSymbol = (USHORT)p[1];
               GreSetAttributes(c->hdc, PRIM_AREA, 0UL, ABB_SYMBOL, (PBUNDLE)&ab);
               GocaCallResult("GOCA_OK");
               break;
           }

            /* ============ char attributes ============ */
            case OCODE_GPSCS:
                rc = GocaPushAttr(c, 0, CBB_SET, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSCS: {
                CHARBUNDLE cb;
                GocaCallEnter("GSCS lcid=%u", (unsigned)p[1]);
                cb.usSet = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_SET, (PBUNDLE)&cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSCD:
                rc = GocaPushAttr(c, 0, CBB_DIRECTION, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSCD: {
                CHARBUNDLE cb;
                GocaCallEnter("GSCD dir=%u", (unsigned)p[1]);
                cb.usDirection = (USHORT)p[1];
                GreSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_DIRECTION, (PBUNDLE)&cb);
                GocaCallResult("GOCA_OK");
                break;
            }

            case OCODE_GPSCR:
                rc = GocaPushAttr(c, 0, CBB_MODE, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSCR: {
                CHARBUNDLE cb;
                GocaCallEnter("GSCR prec=%u", (unsigned)(p[1] & GSCR_PRECISION));
                cb.usPrecision = (USHORT)(p[1] & GSCR_PRECISION);
                GreSetAttributes(c->hdc, PRIM_CHAR, 0UL, CBB_MODE, (PBUNDLE)&cb);
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
            case OCODE_GPSCP:
                rc = GocaPushCurPos(c);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSCP:    rc = DoGSCP(c, p); break;
            
            case OCODE_GLINE:   rc = DoLine(c, p, TRUE); break;
            case OCODE_GCLINE:  rc = DoLine(c, p, FALSE); break;
            case OCODE_GMRK:    rc = DoMark(c, p, TRUE); break;
            case OCODE_GCMRK:   rc = DoMark(c, p, FALSE); break;
            case OCODE_GFARC:   rc = DoFullArc(c, p, TRUE); break;
            case OCODE_GCFARC:  rc = DoFullArc(c, p, FALSE); break;
            case OCODE_GPARC:   rc = DoPartialArc(c, p, TRUE); break;
            case OCODE_GCPARC:  rc = DoPartialArc(c, p, FALSE); break;

            case OCODE_GRLINE:  rc = DoRelativeLine(c, p, cbOrder, FALSE); break;
            case OCODE_GCRLINE: rc = DoRelativeLine(c, p, cbOrder, TRUE);  break;
            case OCODE_GCHST:   rc = DoCharString(c, p, cbOrder, FALSE); break;
            case OCODE_GCCHST:  rc = DoCharString(c, p, cbOrder, TRUE);  break;
            case OCODE_GBOX:    rc = DoBox(c, p, cbOrder, FALSE); break;
            case OCODE_GCBOX:   rc = DoBox(c, p, cbOrder, TRUE);  break;
            case OCODE_GBEZ:    rc = DoBezier(c, p, cbOrder, FALSE); break;
            case OCODE_GCBEZ:   rc = DoBezier(c, p, cbOrder, TRUE);  break;
            case OCODE_GFLT:    rc = DoFillet(c, p, cbOrder, FALSE); break;
            case OCODE_GCFLT:   rc = DoFillet(c, p, cbOrder, TRUE);  break;

            /* ============ long: attributes ============ */
            case OCODE_GPSAP:
                rc = GocaPushArcParams(c);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSAP:    rc = DoSetArcParams(c, p, cbOrder); break;
            case OCODE_GPSCC:
                rc = GocaPushAttr(c, 0, CBB_BOX, 0, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSCC:    rc = DoSetCharCell(c, p, cbOrder); break;
            case OCODE_GPSMC:
                rc = GocaPushAttr(c, 0, 0, MBB_BOX, 0, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
            case OCODE_GSMC:    rc = DoSetMarkerCell(c, p, cbOrder); break;
            case OCODE_GPSPRP:
                rc = GocaPushAttr(c, 0, 0, 0, ABB_REF_POINT, 0);
                if (rc != GOCA_OK) break;
                /* fall through */
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
 * @param [in] lError  Error code (GOCA_OK, GOCA_EC_*, GOCA_ERR_NOMEM).
 * @return Static string describing the error; "unknown" for
 *         unrecognized codes.
 */
const char *GocaErrorName(LONG lError)
{
    switch (lError) {
    case GOCA_OK:                   return "OK";
    case GOCA_EC_UNALLOCATED_ORDER: return "EC-0001: Unallocated order codes";
    case GOCA_EC_INVALID_LENGTH:    return "EC-0003: Incorrect length specification";
    case GOCA_EC_RESERVED_NOT_ZERO: return "EC-0002: Reserved byte or bit not zero";
    case GOCA_EC_STACK_OVERFLOW:    return "EC-0005: Segment stack overflow";
    case GOCA_EC_TRUNCATED_ORDER:   return "EC-0008: Truncated order";
    case GOCA_EC_GBAR_NESTED:       return "EC-6800: A Begin Area order has been executed after another Begin Area order, without an intervening End Area order being executed";
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
    case /* X'C0' */ OCODE_GBOX:    return "GBOX";
    case /* X'81' */ OCODE_GCLINE:  return "GCLINE";
    case /* X'C1' */ OCODE_GLINE:   return "GLINE";
    case /* X'82' */ OCODE_GCMRK:   return "GCMRK";
    case /* X'C2' */ OCODE_GMRK:    return "GMRK";
    case /* X'83' */ OCODE_GCCHST:  return "GCCHST";
    case /* X'C3' */ OCODE_GCHST:   return "GCHST";
    case /* X'85' */ OCODE_GCFLT:   return "GCFLT";
    case /* X'C5' */ OCODE_GFLT:    return "GFLT";
    case /* X'87' */ OCODE_GCFARC:  return "GCFARC";
    case /* X'C7' */ OCODE_GFARC:   return "GFARC";

    case /* X'91' */ OCODE_GCBIMG:  return "GCBIMG";
    case /* X'92' */ OCODE_GIMD:    return "GIMD";
    case /* X'93' */ OCODE_GEIMG:   return "GEIMG";
    case /* X'A0' */ OCODE_GSPRP:   return "GSPRP";
    case /* X'A1' */ OCODE_GCRLINE: return "GCRLINE";
    case /* X'A3' */ OCODE_GCPARC:  return "GCPARC";
    case /* X'A5' */ OCODE_GCBEZ:   return "GCBEZ"; //?? name differs
//    case /* X'B2' */ OCODE_GSPCOL:  return "GSPCOL"; AFP GOCA
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
        case OCODE_GPOP:     return "GPOP";
        case OCODE_GPSCOL:   return "GPSCOL";
        case OCODE_GPSECOL:  return "GPSECOL";
        case OCODE_GPSMX:    return "GPSMX";
        case OCODE_GPSBMX:   return "GPSBMX";
        case OCODE_GPSLT:    return "GPSLT";
        case OCODE_GPSLW:    return "GPSLW";
        case OCODE_GPSFLW:   return "GPSFLW";
        case OCODE_GPSLE:    return "GPSLE";
        case OCODE_GPSLJ:    return "GPSLJ";
        case OCODE_GPSMS:    return "GPSMS";
        case OCODE_GPSMT:    return "GPSMT";
        case OCODE_GPSPS:    return "GPSPS";
        case OCODE_GPSPT:    return "GPSPT";
        case OCODE_GPSCS:    return "GPSCS";
        case OCODE_GPSCD:    return "GPSCD";
        case OCODE_GPSCR:    return "GPSCR";
        case OCODE_GPSCP:    return "GPSCP";
        case OCODE_GPSAP:    return "GPSAP";
        case OCODE_GPSCC:    return "GPSCC";
        case OCODE_GPSMC:    return "GPSMC";
        case OCODE_GPSPRP:   return "GPSPRP";
    default:            return "?";
    }
}

