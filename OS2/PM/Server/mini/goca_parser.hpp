/*!
 * @file goca_parser.hpp
 * @brief GOCA drawing-order stream parser, dispatched through PMGRE (Gre*).
 *
 * Every Gre* call goes through macros declared in <pmddim.h>; every
 * attribute mask and field name is taken from <pmgpi.h>; every order
 * code and structure is taken from <pmord.h>.
 *
 * Common types (GOCA_CTX, GOCA_COORD_SIZE, GOCA_COORD_ENDIAN, GocaPutCoord,
 * GocaSetFormat, GocaSetFormatFromOptions, GOCA_OK/GOCA_ERR_*) live in
 * "goca_common.h" and are shared with the client.
 */
#ifndef GOCA_PARSER_H
#define GOCA_PARSER_H

#include "goca_common.h"

/* GRE feature set must precede pmddim.h */
#define INCL_GREALL
#include "pmddi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Human-readable name for a GOCA error code. */
const char *GocaErrorName(LONG lError);

/*! @brief Human-readable name for a GOCA order code. */
const char *GocaOrderName(UCHAR uCode);

/*!
 * @brief Initialize the parser context for server-side use.
 *
 * Sets @c hdc, stream pointers, format from defaults, and pulls live
 * bundles plus the current position from the device. Must be called
 * with a valid Gre* @c hdc.
 */
void GocaInit(GOCA_CTX *pCtx, HDC hdc, PBYTE pb, ULONG cb);

/*!
 * @brief Parse the stream in @c pCtx->pbStream of @c pCtx->cbStream bytes.
 * @return GOCA_OK on success, GOCA_ERR_* on failure.
 */
LONG GocaParseStream(GOCA_CTX *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* GOCA_PARSER_H */
