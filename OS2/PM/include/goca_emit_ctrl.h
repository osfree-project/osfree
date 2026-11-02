#ifndef GOCA_EMIT_CTRL_H
#define GOCA_EMIT_CTRL_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Emit GCOMT (Comment, long). */
static inline LONG GocaEmitComment(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                    LONG lLength, const void *pvData)
{
    ULONG need;

    if (!c || lLength < 0 || lLength > 255 || (lLength > 0 && !pvData))
        return (LONG)GPI_ALTERROR;

    need = 2 + (ULONG)lLength;
    if (!buf) return (LONG)need;
    if (cb < need) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GCOMT;
    buf[1] = (BYTE)lLength;
    if (lLength > 0) memcpy(buf + 2, pvData, (ULONG)lLength);
    return (LONG)need;
}

/*! @brief Emit GSPIK (Set Pick Identifier, long, 4-byte id). */
static inline LONG GocaEmitSetPickId(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lPickId)
{
    if (!c || !buf || cb < 6) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSPIK;
    buf[1] = 4;
    GocaPutU32(c, buf + 2, (ULONG)lPickId);
    return 6;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_CTRL_H */
