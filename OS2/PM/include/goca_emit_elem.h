#ifndef GOCA_EMIT_ELEM_H
#define GOCA_EMIT_ELEM_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Emit GBEL (Begin Element, long).
 *
 * Layout: LEN, TYPE (GLONG), DESCR (GSTR, optional).
 * LEN = 4 + cchDesc. cchDesc is limited to GBEL_DL (251).
 *
 * @param [in] c        GOCA context.
 * @param [out] buf     Destination buffer.
 * @param [in] cb       Buffer capacity.
 * @param [in] lType    Element type (GLONG).
 * @param [in] pszDesc  NUL-terminated description; NULL means none.
 * @return Bytes written, or GPI_ALTERROR on error.
 */
static inline LONG GocaEmitBeginElement(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                        LONG lType, const char *pszDesc)
{
    ULONG cch = 0;

    if (!c || !buf) return (LONG)GPI_ALTERROR;
    if (pszDesc) {
        while (pszDesc[cch] && cch < GBEL_DL) cch++;
    }
    if (cb < 6 + cch) return (LONG)GPI_ALTERROR;

    buf[0] = OCODE_GBEL;
    buf[1] = (BYTE)(4 + cch);
    GocaPutU32(c, buf + 2, (ULONG)lType);
    if (cch > 0) memcpy(buf + 6, pszDesc, cch);
    return (LONG)(6 + cch);
}

/*!
 * @brief Emit GEEL (End Element, fixed 2-byte).
 */
static inline LONG GocaEmitEndElement(GOCA_CTX *c, PBYTE buf, ULONG cb)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GEEL;
    buf[1] = 0;
    return 2;
}

/*!
 * @brief Emit GLBL (Label, long).
 *
 * Layout: LEN = 4, LDATA (GLONG).
 */
static inline LONG GocaEmitLabel(GOCA_CTX *c, PBYTE buf, ULONG cb, LONG lLabel)
{
    if (!c || !buf || cb < 6) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GLABL;
    buf[1] = 4;
    GocaPutU32(c, buf + 2, (ULONG)lLabel);
    return 6;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_ELEM_H */
