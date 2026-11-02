#ifndef GOCA_EMIT_PRIM_H
#define GOCA_EMIT_PRIM_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GSCP (Set Current Position, long). */
static inline LONG GocaEmitMove(GOCA_CTX *c, PBYTE buf, ULONG cb, POINTL ptl)
{
    ULONG pointSize, need, pos = 2;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GSCP;
    buf[1] = (BYTE)(2 * pointSize);
    pos += GocaPutCoord(c, buf + pos, ptl.x);
    pos += GocaPutCoord(c, buf + pos, ptl.y);
    return (LONG)pos;
}

/*! @brief Emit GCLINE (Line at Current Position, long, single endpoint). */
static inline LONG GocaEmitLine(GOCA_CTX *c, PBYTE buf, ULONG cb, POINTL ptl)
{
    ULONG pointSize, need, pos = 2;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCLINE;
    buf[1] = (BYTE)(2 * pointSize);
    pos += GocaPutCoord(c, buf + pos, ptl.x);
    pos += GocaPutCoord(c, buf + pos, ptl.y);
    return (LONG)pos;
}

/*! @brief Emit GCLINE (multi-point polyline). buf == NULL queries size. */
static inline LONG GocaEmitPolyLine(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                     LONG lCount, PPOINTL aptl)
{
    ULONG pointSize, need, pos = 2;
    LONG  i;

    if (!c || lCount <= 0 || !aptl) return (LONG)GPI_ALTERROR;

    pointSize = (ULONG)c->coordSize;
    need = 2 + (ULONG)lCount * 2 * pointSize;

    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCLINE;
    buf[1] = (BYTE)(lCount * 2 * pointSize);
    for (i = 0; i < lCount; i++) {
        pos += GocaPutCoord(c, buf + pos, aptl[i].x);
        pos += GocaPutCoord(c, buf + pos, aptl[i].y);
    }
    return (LONG)pos;
}

/*!
 * @brief Emit GCBOX (Box at Current Position, long).
 * @param [in] lControl GPI DRO_FILL/DRO_OUTLINE bits.
 * @note Flag bit values (0x40 = boundary, 0x20 = fill) match the current
 *       parser; keep them in sync if the parser is updated to pmord.h
 *       GCBOX_BOUNDARY (0x20) / GCBOX_FILL (0x40).
 */
static inline LONG GocaEmitBox(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                LONG lControl, POINTL ptl,
                                LONG lHRound, LONG lVRound)
{
    ULONG pointSize, need, pos = 4;
    BOOL  bAxes;
    UCHAR flags = 0;

    if (!c) return (LONG)GPI_ALTERROR;

    if (lControl & 2) flags |= 0x40;   /* DRO_OUTLINE -> boundary */
    if (lControl & 1) flags |= 0x20;   /* DRO_FILL    -> fill     */

    pointSize = (ULONG)c->coordSize;
    bAxes = (lHRound || lVRound) ? TRUE : FALSE;
    need = 4 + 2 * pointSize + (bAxes ? 2 * pointSize : 0);

    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCBOX;
    buf[1] = (BYTE)(2 + 2 * pointSize + (bAxes ? 2 * pointSize : 0));
    buf[2] = flags;
    buf[3] = 0;
    pos += GocaPutCoord(c, buf + pos, ptl.x);
    pos += GocaPutCoord(c, buf + pos, ptl.y);
    if (bAxes) {
        pos += GocaPutCoord(c, buf + pos, lHRound);
        pos += GocaPutCoord(c, buf + pos, lVRound);
    }
    return (LONG)pos;
}

/*! @brief Emit GCMRK (single marker at current position, long). */
static inline LONG GocaEmitMarker(GOCA_CTX *c, PBYTE buf, ULONG cb, POINTL ptl)
{
    ULONG pointSize, need, pos = 2;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCMRK;
    buf[1] = (BYTE)(2 * pointSize);
    pos += GocaPutCoord(c, buf + pos, ptl.x);
    pos += GocaPutCoord(c, buf + pos, ptl.y);
    return (LONG)pos;
}

/*! @brief Emit GCMRK for a multi-point marker set. buf == NULL queries size. */
static inline LONG GocaEmitPolyMarker(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       LONG lCount, PPOINTL aptl)
{
    ULONG pointSize, need, pos = 2;
    LONG  i;

    if (!c || lCount <= 0 || !aptl) return (LONG)GPI_ALTERROR;

    pointSize = (ULONG)c->coordSize;
    need = 2 + (ULONG)lCount * 2 * pointSize;

    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCMRK;
    buf[1] = (BYTE)(lCount * 2 * pointSize);
    for (i = 0; i < lCount; i++) {
        pos += GocaPutCoord(c, buf + pos, aptl[i].x);
        pos += GocaPutCoord(c, buf + pos, aptl[i].y);
    }
    return (LONG)pos;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_PRIM_H */
