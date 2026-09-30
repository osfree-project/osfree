/*!
 * @file tom.h
 * @brief |TOMAP internal file generator for WinHelp 3.0.
 *
 * @par Function naming
 * Function names follow OS/2 Control Program API conventions as
 * defined in OS/2 Version 2.0 - Volume 4: Writing Applications,
 * section 12.2 "Function Name Changes".
 *
 * @par References
 *  - [1] SAA Common Programming Interface, C Reference - Level 2,
 *        SC09-1308-02 (Sep 1991).
 *  - [2] OS/2 Version 2.0 - Volume 4: Writing Applications,
 *        IBM, 1993, section 12.2.
 *  - [3] Winterhoff, M., helpdeco (1997), |TOMAP format.
 *  - [4] ccl container library: vector.h.
 *
 * @par Thread safety
 * The module is single threaded.
 *
 * @par Ownership
 * HTOM is caller-owned and released with TomDestroy.
 */
#ifndef TOM_H
#define TOM_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

/*!
 * @typedef HTOM
 * @brief Handle to a |TOMAP generator. Caller-owned.
 */
typedef HANDLE HTOM;

/*!
 * @typedef PHTOM
 * @brief Pointer to a |TOMAP generator handle.
 */
typedef HTOM  *PHTOM;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty |TOMAP generator.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Create implies that a new resource
 * is created as the result of a function call.
 *
 * @param[out] phTom Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phTom is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 *
 * @sa TomDestroy
 */
APIRET APIENTRY TomCreate(PHTOM phTom);

/*!
 * @brief Destroy a |TOMAP generator and release all its data.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hTom Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY TomDestroy(HTOM hTom);

/* ==================================================================
 * Adding
 * ================================================================== */

/*!
 * @brief Append a topic offset.
 *
 * Offsets are stored in insertion order. The order matches the order
 * in which topics are added to the |TOPIC generator.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: verb "Add" for append operations.
 *
 * @param[in] hTom     Handle. Not NULLHANDLE.
 * @param[in] ulOffset Byte offset of a topic header in |TOPIC.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTom is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY TomAddOffset(HTOM hTom, ULONG ulOffset);

/* ==================================================================
 * Query
 * ================================================================== */

/*!
 * @brief Query the number of stored offsets.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Query implies that a system value
 * is returned.
 *
 * @param[in]  hTom     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY TomQueryOffsetCount(HTOM hTom, PULONG pulCount);

/* ==================================================================
 * Writing
 * ================================================================== */

/*!
 * @brief Serialize the |TOMAP file to a stream.
 *
 * Writes each stored offset as a 32-bit little-endian value.
 *
 * @param[in] hTom Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hTom or @a f is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY TomWrite(HTOM hTom, FILE* f);

#endif /* TOM_H */
