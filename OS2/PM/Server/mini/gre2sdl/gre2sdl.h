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

#define INCL_BASE
#define INCL_PM
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


#endif /* GRE2SDL_H */
