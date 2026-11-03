#ifndef GOCA_EMIT_PATH_H
#define GOCA_EMIT_PATH_H

#include "goca_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Emit GBPTH (Begin Path, long).
 *
 * Layout per pmord.h ORDER_GBPTH and IBM PM Ref 64F0277:
 *   LEN = 6, RES (U16) = 0, PTHID (GLONG).
 *
 * The PM GPI layer always uses path id 1.
 */
static inline LONG GocaEmitBeginPath(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                     LONG lPath)
{
    if (!c || !buf || cb < 8) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GBPTH;
    buf[1] = 6;
    GocaPutU16(c, buf + 2, 0);
    GocaPutU32(c, buf + 4, (ULONG)lPath);
    return 8;
}

/*! @brief Emit GEPTH (End Path, fixed 2-byte). */
static inline LONG GocaEmitEndPath(GOCA_CTX *c, PBYTE buf, ULONG cb)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GEPTH;
    buf[1] = 0;
    return 2;
}

/*! @brief Emit GCFIG (Close Figure, fixed 2-byte). */
static inline LONG GocaEmitCloseFigure(GOCA_CTX *c, PBYTE buf, ULONG cb)
{
    if (!c || !buf || cb < 2) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GCFIG;
    buf[1] = 0;
    return 2;
}

/*!
 * @brief Emit GFPTH (Fill Path, long).
 *
 * Layout per pmord.h ORDER_GFPTH and IBM PM Ref 64F0277:
 *   LEN = 6, FLAGS (U8), RES (U8) = 0, PTHID (GLONG).
 *
 * @param [in] ulFlags  GFPTH_WINDING / GFPTH_MODIFY bits.
 */
static inline LONG GocaEmitFillPath(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                    ULONG ulFlags, LONG lPath)
{
    if (!c || !buf || cb < 8) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GFPTH;
    buf[1] = 6;
    buf[2] = (UCHAR)ulFlags;
    buf[3] = 0;
    GocaPutU32(c, buf + 4, (ULONG)lPath);
    return 8;
}

/*!
 * @brief Emit GOPTH (Outline Path, long).
 *
 * Layout per pmord.h ORDER_GOPTH and IBM PM Ref 64F0277:
 *   LEN = 6, FLAGS (U8) = 0, RES (U8) = 0, PTHID (GLONG) = 1.
 *
 * @warning OCODE_GOPTH changed from X'D4' to X'D9' per IBM PM Ref
 *          64F0277. Requires confirmation by test_gopth.cpp on real OS/2.
 */
static inline LONG GocaEmitOutlinePath(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       LONG lPath)
{
    if (!c || !buf || cb < 8) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GOPTH;
    buf[1] = 6;
    buf[2] = 0;
    buf[3] = 0;
    GocaPutU32(c, buf + 4, (ULONG)lPath);
    return 8;
}

/*!
 * @brief Emit GMPTH (Modify Path, long).
 *
 * Layout per pmord.h ORDER_GMPTH and IBM PM Ref 64F0277:
 *   LEN = 6, MODE (U8) = GMPTH_STROKE, RES (U8) = 0, PTHID (GLONG).
 */
static inline LONG GocaEmitModifyPath(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                      UCHAR uchMode, LONG lPath)
{
    if (!c || !buf || cb < 8) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GMPTH;
    buf[1] = 6;
    buf[2] = uchMode;
    buf[3] = 0;
    GocaPutU32(c, buf + 4, (ULONG)lPath);
    return 8;
}

/*!
 * @brief Emit GSCPTH (Set Clip Path, long).
 *
 * Layout per pmord.h ORDER_GSCPTH and IBM PM Ref 64F0277:
 *   LEN = 6, FLAGS (U8), RES (U8) = 0, PTHID (GLONG).
 *   PTHID = 0 means "no clipping".
 */
static inline LONG GocaEmitSetClipPath(GOCA_CTX *c, PBYTE buf, ULONG cb,
                                       ULONG ulFlags, LONG lPath)
{
    if (!c || !buf || cb < 8) return (LONG)GPI_ALTERROR;
    buf[0] = OCODE_GSCPTH;
    buf[1] = 6;
    buf[2] = (UCHAR)ulFlags;
    buf[3] = 0;
    GocaPutU32(c, buf + 4, (ULONG)lPath);
    return 8;
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_EMIT_PATH_H */
