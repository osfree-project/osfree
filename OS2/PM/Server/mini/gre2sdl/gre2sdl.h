/*!
 * @file gre2sdl.h
 * @brief Internal definitions for the GRE2SDL presentation driver.
 *
 * The DISPATCHTABLE is an array of 32-bit function pointers. The
 * low-order byte of an NGre* function code gives the index into this
 * table (see PMDDIM.H: NGrePolyLine = 0x00004019 -> index 0x19).
 * We use this to fill the table without a full PMDDI.H definition.
 */

#ifndef GRE2SDL_H
#define GRE2SDL_H

#include "F_platform.h"

/*! @brief Maximum number of entries in DISPATCHTABLE (from GRE22 docs). */
#define GRE2SDL_MAX_DISPATCH 256

/*! @brief Type of a single dispatch table entry.
 *  All Gre* functions have a prototype of the form:
 *      ULONG APIENTRY GreXxx(ULONG a, ULONG b, ..., ULONG fnCode);
 *  We store them as generic pointers and cast at call sites.
 */
typedef ULONG (__cdecl *PFN_GRE)(ULONG, ULONG, ULONG, ULONG, ULONG,
                                  ULONG, ULONG, ULONG, ULONG, ULONG);

/*! @brief Extract the table index from an NGre* constant. */
#define GRE2SDL_INDEX(ngre) ((ngre) & 0xFF)

/* ---- Indices for the functions we hook (Tier 1) ---- */
#define IDX_GRE_OPEN_DC          GRE2SDL_INDEX(NGreOpenDC)      /* 0x00 */
#define IDX_GRE_CLOSE_DC         GRE2SDL_INDEX(NGreCloseDC)     /* 0x01 */
#define IDX_GRE_SET_CURRENT_POS  GRE2SDL_INDEX(NGreSetCurrentPosition) /* 0x18 */
#define IDX_GRE_GET_CURRENT_POS  GRE2SDL_INDEX(NGreGetCurrentPosition) /* 0x17 */
#define IDX_GRE_POLY_LINE        GRE2SDL_INDEX(NGrePolyLine)    /* 0x19 */
#define IDX_GRE_POLY_MARKER      GRE2SDL_INDEX(NGrePolyMarker)  /* 0x3A */
#define IDX_GRE_BOX_INTERIOR     GRE2SDL_INDEX(NGreBoxInterior) /* 0x07 */
#define IDX_GRE_BOX_BOUNDARY     GRE2SDL_INDEX(NGreBoxBoundary) /* 0x08 */
#define IDX_GRE_BOX_BOTH         GRE2SDL_INDEX(NGreBoxBoth)     /* 0x09 */
#define IDX_GRE_FULL_ARC_BOTH    GRE2SDL_INDEX(NGreFullArcBoth) /* 0x06 */
#define IDX_GRE_PARTIAL_ARC      GRE2SDL_INDEX(NGrePartialArc)  /* 0x03 */
#define IDX_GRE_POLY_SPLINE      GRE2SDL_INDEX(NGrePolySpline)  /* 0x0C */
#define IDX_GRE_POLY_FILLET      GRE2SDL_INDEX(NGrePolyFillet)  /* 0x0A */
#define IDX_GRE_CHAR_STRING      GRE2SDL_INDEX(NGreCharString)  /* 0x35 */
#define IDX_GRE_CHAR_STRING_POS  GRE2SDL_INDEX(NGreCharStringPos) /* 0x36 */
#define IDX_GRE_SET_ATTRIBUTES   GRE2SDL_INDEX(NGreSetAttributes) /* 0x18? */
#define IDX_GRE_GET_ATTRIBUTES   GRE2SDL_INDEX(NGreGetAttributes) /* 0x17? */

#endif /* GRE2SDL_H */
