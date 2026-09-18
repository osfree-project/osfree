/* reusedef.h - REUSE 3.3 shared constants
 * (C89 + Watcom extensions) */
#ifndef REUSEDEF_H
#define REUSEDEF_H

/**
 * @file reusedef.h
 * @brief Constants defined by the REUSE Specification 3.3.
 *
 * Shared by reuse.h and reuse_toml.h.
 */

#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Precedence (REUSE 3.3 §4.1.2)
 * ================================================================== */

/** @def REUSE_PRECEDENCE_CLOSEST @brief "closest" (default). */
#define REUSE_PRECEDENCE_CLOSEST   1
/** @def REUSE_PRECEDENCE_AGGREGATE @brief "aggregate". */
#define REUSE_PRECEDENCE_AGGREGATE 2
/** @def REUSE_PRECEDENCE_OVERRIDE @brief "override". */
#define REUSE_PRECEDENCE_OVERRIDE  3

#ifdef __cplusplus
}
#endif

#endif /* REUSEDEF_H */
