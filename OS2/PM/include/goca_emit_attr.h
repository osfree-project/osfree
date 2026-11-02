#ifndef GOCA_EMIT_ATTR_H
#define GOCA_EMIT_ATTR_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Emit GSCOL (0..255) or GSECOL (256..0xFFFF).
 * @return bytes written, or -1 on error.
 */
static inline LONG GocaEmitSetColor(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lColor)
{
    if (!c || lColor < 0) return (LONG)GPI_ALTERROR;

    if (lColor <= 0xFF) {
        if (!buf || cb < 2) return (LONG)GPI_ALTERROR;
        buf[0] = OCODE_GSCOL;
        buf[1] = (BYTE)lColor;
        return 2;
    }
    if (lColor <= 0xFFFF) {
        if (!buf || cb < 4) return (LONG)GPI_ALTERROR;
        buf[0] = OCODE_GSECOL;
        buf[1] = 2;
        GocaPutU16(c, buf + 2, (USHORT)lColor);
        return 4;
    }
    return (LONG)GPI_ALTERROR;
}

/*! @brief Emit GSBCOL (Set Background Color, long order). */
static inline LONG GocaEmitSetBackColor(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lColor)
{
    if (!c || !buf || cb < 4) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSBCOL;
    buf[1] = 2;
    GocaPutU16(c, buf + 2, (USHORT)lColor);
    return 4;
}

/*! @brief Emit GSMX (Set Mix, 2-byte). */
static inline LONG GocaEmitSetMix(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lMix)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSMX;
    buf[1] = (BYTE)lMix;
    return 2;
}

/*! @brief Emit GSBMX (Set Background Mix, 2-byte). */
static inline LONG GocaEmitSetBackMix(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lMix)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSBMX;
    buf[1] = (BYTE)lMix;
    return 2;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_ATTR_H */
