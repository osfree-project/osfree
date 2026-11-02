#ifndef GOCA_EMIT_LINE_H
#define GOCA_EMIT_LINE_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GSLT (Set Line Type, 2-byte). */
static inline LONG GocaEmitSetLineType(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lType)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSLT;
    buf[1] = (BYTE)lType;
    return 2;
}

/*!
 * @brief Emit GSLW (2-byte) when fractional part is zero, otherwise GSFLW (long).
 * @param [in] fxWidth FIXED line width.
 */
static inline LONG GocaEmitSetLineWidth(GOCA_CTX *c, PBYTE buf, ULONG cb, FIXED fxWidth)
{
    SHORT  intPart;
    USHORT fracPart;
    BYTE   fracByte;

    if (!c) return (LONG)GPI_ALTERROR;

    intPart  = (SHORT)((fxWidth >> 16) & 0xFFFF);
    fracPart = (USHORT)(fxWidth & 0xFFFF);
    fracByte = (BYTE)(fracPart >> 8);

    if (fracByte == 0) {
        if (!buf || cb < 2) return (LONG)GPI_ALTERROR;
        buf[0] = OCODE_GSLW;
        buf[1] = (BYTE)intPart;
        return 2;
    }

    if (!buf || cb < 4) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSFLW;
    buf[1] = 2;
    buf[2] = (BYTE)intPart;
    buf[3] = fracByte;
    return 4;
}

/*! @brief Emit GSLW with integer geometric width. */
static inline LONG GocaEmitSetLineWidthGeom(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lGeomWidth)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSLW;
    buf[1] = (BYTE)lGeomWidth;
    return 2;
}

/*! @brief Emit GSLE (Set Line End, 2-byte). */
static inline LONG GocaEmitSetLineEnd(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lEnd)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSLE;
    buf[1] = (BYTE)lEnd;
    return 2;
}

/*! @brief Emit GSLJ (Set Line Join, 2-byte). */
static inline LONG GocaEmitSetLineJoin(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lJoin)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSLJ;
    buf[1] = (BYTE)lJoin;
    return 2;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_LINE_H */
