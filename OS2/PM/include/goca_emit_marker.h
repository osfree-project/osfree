#ifndef GOCA_EMIT_MARKER_H
#define GOCA_EMIT_MARKER_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GSMS (Set Marker Set, 2-byte). */
static inline LONG GocaEmitSetMarkerSet(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lSet)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSMS;
    buf[1] = (BYTE)lSet;
    return 2;
}

/*! @brief Emit GSMT (Set Marker Symbol, 2-byte). */
static inline LONG GocaEmitSetMarkerSymbol(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lSymbol)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSMT;
    buf[1] = (BYTE)lSymbol;
    return 2;
}

/*!
 * @brief Emit GSMC (Set Marker Cell, long).
 * Layout: cx (S16), cy (S16), flags, reserved.
 */
static inline LONG GocaEmitSetMarkerCell(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                          LONG cx, LONG cy)
{
    if (!c || !buf || cb < 8) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSMC;
    buf[1] = 6;
    GocaPutU16(c, buf + 2, (USHORT)cx);
    GocaPutU16(c, buf + 4, (USHORT)cy);
    buf[6] = 0;
    buf[7] = 0;
    return 8;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_MARKER_H */
