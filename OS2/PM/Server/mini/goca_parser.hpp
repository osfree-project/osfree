/*!
 * @file goca_parser.h
 * @brief GOCA drawing-order stream parser, dispatched through PMGRE (Gre*).
 *
 * Every Gre* call goes through macros declared in <pmddim.h>; every
 * attribute mask and field name is taken from <pmgpi.h>; every order
 * code and structure is taken from <pmord.h>.
 */
#ifndef GOCA_PARSER_H
#define GOCA_PARSER_H

#define INCL_PM
#include <F_platform.hpp>
#include "pmord.h"

/* GRE feature set — must precede pmddim.h */
#define INCL_GRE_ALL
#include "pmddi.h"

#ifdef __cplusplus
extern "C" {
#endif

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

const char *GocaErrorName(LONG lError);
const char *GocaOrderName(UCHAR uCode);

void GocaInit(GOCA_CTX *pCtx, HDC hdc, PBYTE pb, ULONG cb);
LONG GocaParseStream(GOCA_CTX *pCtx);

#ifdef __cplusplus
}
#endif
#endif
