#ifndef GOCA_EMIT_AREA_H
#define GOCA_EMIT_AREA_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GSPS (Set Pattern Set, 2-byte). */
static inline LONG GocaEmitSetPatternSet(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lSet)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSPS;
    buf[1] = (BYTE)lSet;
    return 2;
}

/*! @brief Emit GSPT (Set Pattern Symbol, 2-byte). */
static inline LONG GocaEmitSetPatternSymbol(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lSymbol)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSPT;
    buf[1] = (BYTE)lSymbol;
    return 2;
}

/*!
 * @brief Emit GSPRP (Set Pattern Reference Point, long).
 * Layout: flags, reserved, point (coordSize * 2).
 */
static inline LONG GocaEmitSetPatternRefPoint(GOCA_CTX *c, PBYTE buf, ULONG cb, POINTL ptl)
{
    ULONG pointSize, need, pos = 4;

    if (!c) return (LONG)GPI_ALTERROR;
    pointSize = (ULONG)c->coordSize;
    need = 4 + 2 * pointSize;
    if (!buf || cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GSPRP;
    buf[1] = (BYTE)(2 + 2 * pointSize);
    buf[2] = 0;
    buf[3] = 0;
    pos += GocaPutCoord(c, buf + pos, ptl.x);
    pos += GocaPutCoord(c, buf + pos, ptl.y);
    return (LONG)pos;
}

/*! @brief Emit GBAR (Begin Area, 2-byte). */
static inline LONG GocaEmitBeginArea(GOCA_CTX *c, PBYTE buf, ULONG cb, ULONG flOptions)
{
    UCHAR flags = 0x80;   /* RES1 must be 1 */

    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;

    if (flOptions & BA_BOUNDARY) flags |= 0x40;
    if (flOptions & BA_WINDING)  flags |= 0x20;

    buf[0] = OCODE_GBAR;
    buf[1] = flags;
    return 2;
}

/*! @brief Emit GEAR (End Area, long, zero data). */
static inline LONG GocaEmitEndArea(GOCA_CTX *c, PBYTE buf, ULONG cb)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GEAR;
    buf[1] = 0;
    return 2;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_AREA_H */
