#ifndef GOCA_EMIT_ARC_H
#define GOCA_EMIT_ARC_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Emit GSAP (Set Arc Parameters, long).
 * Layout: P, Q, R, S — 4 x S16.
 */
static inline LONG GocaEmitSetArcParams(GOCA_CTX *c, PBYTE buf, ULONG cb, PARCPARAMS pap)
{
    if (!c || !pap || !buf || cb < 10) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSAP;
    buf[1] = 8;
    GocaPutU16(c, buf + 2, (USHORT)pap->lP);
    GocaPutU16(c, buf + 4, (USHORT)pap->lQ);
    GocaPutU16(c, buf + 6, (USHORT)pap->lR);
    GocaPutU16(c, buf + 8, (USHORT)pap->lS);
    return 10;
}

/*! @brief Emit GCFARC (Full Arc at Current Position, long, 2-byte multiplier). */
static inline LONG GocaEmitFullArc(GOCA_CTX *c, PBYTE buf, ULONG cb, FIXED fxMultiplier)
{
    if (!c || !buf || cb < 4) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GCFARC;
    buf[1] = 2;
    GocaPutU16(c, buf + 2, (USHORT)((ULONG)fxMultiplier >> 8));
    return 4;
}

/*!
 * @brief Emit GCPARC (Partial Arc at Current Position, long).
 * Layout: center (2*coordSize), multiplier (U16 = high half of FIXED),
 *         start angle (U32), sweep angle (U32).
 */
static inline LONG GocaEmitPartialArc(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       POINTL ptlCenter, FIXED fxMultiplier,
                                       FIXED fxStartAngle, FIXED fxSweepAngle)
{
    ULONG pointSize, need, pos = 2;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize + 10;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCPARC;
    buf[1] = (BYTE)(2 * pointSize + 10);
    pos += GocaPutCoord(c, buf + pos, ptlCenter.x);
    pos += GocaPutCoord(c, buf + pos, ptlCenter.y);
    pos += GocaPutU16(c, buf + pos, (USHORT)((ULONG)fxMultiplier >> 8));
    pos += GocaPutU32(c, buf + pos, (ULONG)fxStartAngle);
    pos += GocaPutU32(c, buf + pos, (ULONG)fxSweepAngle);
    return (LONG)pos;
}

/*! @brief Emit GCBEZ (Bezier spline at current position, long). */
static inline LONG GocaEmitPolySpline(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       LONG lCount, PPOINTL aptl)
{
    ULONG pointSize, need, pos = 2;
    LONG  i;

    if (!c || lCount <= 0 || !aptl) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + (ULONG)lCount * 2 * pointSize;

    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCBEZ;
    buf[1] = (BYTE)(lCount * 2 * pointSize);
    for (i = 0; i < lCount; i++) {
        pos += GocaPutCoord(c, buf + pos, aptl[i].x);
        pos += GocaPutCoord(c, buf + pos, aptl[i].y);
    }
    return (LONG)pos;
}

/*! @brief Emit GCFLT (Fillet at current position, long). */
static inline LONG GocaEmitPolyFillet(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       LONG lCount, PPOINTL aptl)
{
    ULONG pointSize, need, pos = 2;
    LONG  i;

    if (!c || lCount <= 0 || !aptl) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + (ULONG)lCount * 2 * pointSize;

    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCFLT;
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
#endif /* GOCA_EMIT_ARC_H */
