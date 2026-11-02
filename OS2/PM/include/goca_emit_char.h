#ifndef GOCA_EMIT_CHAR_H
#define GOCA_EMIT_CHAR_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GSCS (Set Character Set, 2-byte). */
static inline LONG GocaEmitSetCharSet(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lSet)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSCS;
    buf[1] = (BYTE)lSet;
    return 2;
}

/*! @brief Emit GSCD (Set Character Direction, 2-byte). */
static inline LONG GocaEmitSetCharDirection(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lDir)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSCD;
    buf[1] = (BYTE)lDir;
    return 2;
}

/*! @brief Emit GSCR (Set Character Precision, 2-byte). */
static inline LONG GocaEmitSetCharPrecision(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lPrecision)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSCR;
    buf[1] = (BYTE)(lPrecision & GSCR_PRECISION);
    return 2;
}

/*!
 * @brief Emit GSCC (Set Character Cell, long).
 * Layout: cxInt (S16), cyInt (S16), cxFrac (U16), cyFrac (U16), flags, reserved.
 */
static inline LONG GocaEmitSetCharCell(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                        LONG cxInt, LONG cyInt,
                                        USHORT cxFrac, USHORT cyFrac, UCHAR flags)
{
    if (!c || !buf || cb < 12) return (LONG)GPI_ALTERROR;
    buf[0]  = OCODE_GSCC;
    buf[1]  = 10;
    GocaPutU16(c, buf + 2, (USHORT)cxInt);
    GocaPutU16(c, buf + 4, (USHORT)cyInt);
    GocaPutU16(c, buf + 6, cxFrac);
    GocaPutU16(c, buf + 8, cyFrac);
    buf[10] = flags;
    buf[11] = 0;
    return 12;
}

/*! @brief Emit GSCA (Set Character Angle, long) — gradient (dx,dy). */
static inline LONG GocaEmitSetCharAngle(GOCA_CTX *c, PBYTE buf, ULONG cb, POINTL ptlAngle)
{
    ULONG pointSize, need, pos = 2;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GSCA;
    buf[1] = (BYTE)(2 * pointSize);
    pos += GocaPutCoord(c, buf + pos, ptlAngle.x);
    pos += GocaPutCoord(c, buf + pos, ptlAngle.y);
    return (LONG)pos;
}

/*! @brief Emit GSCH (Set Character Shear, long) — shear (x,y). */
static inline LONG GocaEmitSetCharShear(GOCA_CTX *c, PBYTE buf, ULONG cb, POINTL ptlShear)
{
    ULONG pointSize, need, pos = 2;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GSCH;
    buf[1] = (BYTE)(2 * pointSize);
    pos += GocaPutCoord(c, buf + pos, ptlShear.x);
    pos += GocaPutCoord(c, buf + pos, ptlShear.y);
    return (LONG)pos;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_CHAR_H */
