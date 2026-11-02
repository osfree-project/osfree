/*!
 * @file goca_common.h
 * @brief Common GOCA types and helpers shared by client and server.
 */
#ifndef GOCA_COMMON_H
#define GOCA_COMMON_H

#define INCL_PM
#include "F_platform.h"
#include "pmord.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Size of a coordinate in the order stream. */
typedef enum {
    GOCA_COORD_SHORT = 2,   /*!< 16-bit coordinates. */
    GOCA_COORD_LONG  = 4    /*!< 32-bit coordinates. */
} GOCA_COORD_SIZE;

/*! @brief Byte order of coordinates in the order stream. */
typedef enum {
    GOCA_ENDIAN_LE,         /*!< Least-significant byte first (x86, PC).  */
    GOCA_ENDIAN_BE          /*!< Most-significant byte first (360, S370). */
} GOCA_COORD_ENDIAN;

typedef struct _GOCA_CTX GOCA_CTX;

/*! @brief Optional fallback for orders not handled by the built-in switch. */
typedef BOOL (*PFN_GOCA_HANDLER)(GOCA_CTX *pCtx, UCHAR uCode,
                                  const BYTE *pbData, ULONG cbData);

/*!
 * @brief Parser context.
 *
 * Holds the target HDC and the four GOCA foreground bundles, which are
 * kept in sync with the DC between orders so that GOCA attribute orders
 * only touch the fields they set.
 */
struct _GOCA_CTX {
    HDC      hdc;         /*!< Target DC (Gre* handle).               */
    PBYTE    pbStream;    /*!< Order stream.                          */
    ULONG    cbStream;    /*!< Stream size.                           */
    ULONG    uPos;        /*!< Current offset.                        */

    BOOL     bInArea;     /*!< Inside GBAR..GEAR.                     */
    UCHAR    uAreaFlags;  /*!< GBAR flags byte.                       */

    POINTL   ptlCurPos;   /*!< Cached current position.               */

    LINEBUNDLE   lb;      /*!< Cached line bundle.                    */
    CHARBUNDLE   cb;      /*!< Cached character bundle.               */
    MARKERBUNDLE mb;      /*!< Cached marker bundle.                  */
    AREABUNDLE   ab;      /*!< Cached area bundle.                    */

    GOCA_COORD_SIZE   coordSize;    /*!< SHORT or LONG.                */
    GOCA_COORD_ENDIAN coordEndian;  /*!< LE or BE.                     */

    PFN_GOCA_HANDLER pfnHandler;
    void    *pUserData;

    BOOL     bDebug;      /*!< Emit per-order trace to stderr.        */

    LONG     lError;
    ULONG    uErrOffset;
    UCHAR    uErrOrder;
};

#define GOCA_OK            0
#define GOCA_ERR_TRUNC    -1   /*!< Truncated order.                */
#define GOCA_ERR_FORMAT   -2   /*!< Malformed operand / flags.      */
#define GOCA_ERR_NOMEM    -3   /*!< Out of memory.                  */
#define GOCA_ERR_USER     -4   /*!< User handler aborted.           */

/*! @brief Select coordinate size and byte order explicitly. */
static inline void GocaSetFormat(GOCA_CTX *c, GOCA_COORD_SIZE size,
                                 GOCA_COORD_ENDIAN endian)
{
    c->coordSize   = size;
    c->coordEndian = endian;
}

/*!
 * @brief Host platform byte order.
 *
 * On x86 (OS/2, Win32) this is LE; on S/370 and PowerPC-BE this is BE.
 * The wire format defaults to host order because PM GPI can write only
 * native formats (DFORM_PCSHORT/PCLONG on x86, DFORM_S370SHORT on S/370).
 * The parser can read all four; only the writer is host-limited.
 */
static inline GOCA_COORD_ENDIAN GocaHostEndian(void)
{
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) \
    && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return GOCA_ENDIAN_BE;
#elif defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN) || defined(WORDS_BIGENDIAN)
    return GOCA_ENDIAN_BE;
#else
    return GOCA_ENDIAN_LE;   /* x86 (OS/2, Win32), i386, AMD64 */
#endif
}

/*! @brief Select coordinate size and byte order from PS flOptions. */
static inline void GocaSetFormatFromOptions(GOCA_CTX *c, ULONG flOptions)
{
    GOCA_COORD_SIZE   size   = GOCA_COORD_LONG;
    GOCA_COORD_ENDIAN endian = GocaHostEndian();

    if ((flOptions & 0x0000FF00UL) == GPIF_SHORT) {
        size = GOCA_COORD_SHORT;
    }
    GocaSetFormat(c, size, endian);
}

/*!
 * @brief Write an unsigned 16-bit value in the context's byte order.
 *
 * Used wherever a fixed 2-byte field must be encoded (e.g. the colour
 * index of GSECOL) regardless of the current coordSize.
 *
 * @param [in] c   GOCA context (uses c->coordEndian).
 * @param [out] p  Destination buffer.
 * @param [in]  v  Value to write.
 * @return Number of bytes written (always 2).
 */
static inline ULONG GocaPutU16(GOCA_CTX *c, PBYTE p, USHORT v)
{
    if (c->coordEndian == GOCA_ENDIAN_LE) {
        p[0] = (BYTE)v;
        p[1] = (BYTE)(v >> 8);
    } else {
        p[0] = (BYTE)(v >> 8);
        p[1] = (BYTE)v;
    }
    return 2;
}

/*!
 * @brief Write an unsigned 32-bit value in the context's byte order.
 * @return Number of bytes written (always 4).
 */
static inline ULONG GocaPutU32(GOCA_CTX *c, PBYTE p, ULONG v)
{
    if (c->coordEndian == GOCA_ENDIAN_LE) {
        p[0] = (BYTE)v;
        p[1] = (BYTE)(v >> 8);
        p[2] = (BYTE)(v >> 16);
        p[3] = (BYTE)(v >> 24);
    } else {
        p[0] = (BYTE)(v >> 24);
        p[1] = (BYTE)(v >> 16);
        p[2] = (BYTE)(v >> 8);
        p[3] = (BYTE)v;
    }
    return 4;
}

/*!
 * @brief Read an unsigned 16-bit value in the context's byte order.
 */
static inline USHORT GocaGetU16(GOCA_CTX *c, const BYTE *p)
{
    if (c->coordEndian == GOCA_ENDIAN_LE)
        return (USHORT)(((USHORT)p[1] << 8) | p[0]);
    return (USHORT)(((USHORT)p[0] << 8) | p[1]);
}

/*!
 * @brief Read an unsigned 32-bit value in the context's byte order.
 */
static inline ULONG GocaGetU32(GOCA_CTX *c, const BYTE *p)
{
    if (c->coordEndian == GOCA_ENDIAN_LE)
        return ((ULONG)p[3] << 24) | ((ULONG)p[2] << 16) |
               ((ULONG)p[1] << 8)  |  (ULONG)p[0];
    return ((ULONG)p[0] << 24) | ((ULONG)p[1] << 16) |
           ((ULONG)p[2] << 8)  |  (ULONG)p[3];
}

/*!
 * @brief Write one coordinate in the context's current PS format.
 * @param [in] c   GOCA context (uses c->coordSize / c->coordEndian).
 * @param [out] p  Destination buffer.
 * @param [in]  v  Coordinate value.
 * @return Number of bytes written (2 or 4).
 */
static inline ULONG GocaPutCoord(GOCA_CTX *c, PBYTE p, LONG v)
{
    if (c->coordSize == GOCA_COORD_SHORT)
        return GocaPutU16(c, p, (USHORT)v);
    return GocaPutU32(c, p, (ULONG)v);
}

#ifdef __cplusplus
}
#endif
#endif /* GOCA_COMMON_H */