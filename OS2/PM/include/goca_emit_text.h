#ifndef GOCA_EMIT_TEXT_H
#define GOCA_EMIT_TEXT_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GCCHST (Char String at Current Position, long). */
static inline LONG GocaEmitCharString(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       LONG lCount, const char *pchString)
{
    ULONG need;

    if (!c || lCount <= 0 || lCount > 255 || !pchString)
        return (LONG)GPI_ALTERROR;

    need = 2 + (ULONG)lCount;
    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCCHST;
    buf[1] = (BYTE)lCount;
    memcpy(buf + 2, pchString, (ULONG)lCount);
    return (LONG)need;
}

/*! @brief Emit GCHST (Char String at Given Position, long). */
static inline LONG GocaEmitCharStringAt(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                         POINTL ptl, LONG lCount, const char *pchString)
{
    ULONG pointSize, need, pos = 2;

    if (!c || lCount <= 0 || !pchString) return (LONG)GPI_ALTERROR;

    pointSize = (ULONG)c->coordSize;
    need = 2 + 2 * pointSize + (ULONG)lCount;
    if (need - 2 > 255) return (LONG)GPI_ALTERROR;   /* LEN byte */

    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCHST;
    buf[1] = (BYTE)(2 * pointSize + lCount);
    pos += GocaPutCoord(c, buf + pos, ptl.x);
    pos += GocaPutCoord(c, buf + pos, ptl.y);
    memcpy(buf + pos, pchString, (ULONG)lCount);
    return (LONG)need;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_TEXT_H */
