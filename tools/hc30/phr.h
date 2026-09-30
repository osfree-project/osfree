/*!
 * @file phr.h
 * @brief |Phrases internal file generator for WinHelp 3.0.
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
 *  - [3] Winterhoff, M., helpdeco (1997), |Phrases format.
 *  - [4] ccl container library: vector.h.
 *
 * @par Thread safety
 * The module is single threaded.
 *
 * @par Ownership
 * HPHR is caller-owned and released with PhrDestroy. Phrase data is
 * copied into the generator.
 */
#ifndef PHR_H
#define PHR_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

/*!
 * @typedef HPHR
 * @brief Handle to a |Phrases generator. Caller-owned.
 */
typedef HANDLE HPHR;

/*!
 * @typedef PHPHR
 * @brief Pointer to a |Phrases generator handle.
 */
typedef HPHR  *PHPHR;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty |Phrases generator.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Create implies that a new resource
 * is created as the result of a function call.
 *
 * @param[out] phPhr Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phPhr is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 *
 * @sa PhrDestroy
 */
APIRET APIENTRY PhrCreate(PHPHR phPhr);

/*!
 * @brief Destroy a |Phrases generator and release all its data.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hPhr Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY PhrDestroy(HPHR hPhr);

/* ==================================================================
 * Adding
 * ================================================================== */

/*!
 * @brief Append a phrase.
 *
 * The data is copied into the generator.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: verb "Add" for append operations.
 *
 * @param[in] hPhr   Handle. Not NULLHANDLE.
 * @param[in] pvData Phrase bytes. Not NULL.
 * @param[in] ulLen  Length in bytes. Must be non-zero.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or @a ulLen is 0.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrAddPhrase(HPHR hPhr, PCVOID pvData, ULONG ulLen);

/* ==================================================================
 * Query
 * ================================================================== */

/*!
 * @brief Query the number of phrases.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Query implies that a system value
 * is returned.
 *
 * @param[in]  hPhr     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY PhrQueryPhraseCount(HPHR hPhr, PULONG pulCount);

/* ==================================================================
 * Writing
 * ================================================================== */

/*!
 * @brief Serialize the |Phrases file to a stream.
 *
 * Writes a 16-bit phrase count, a 16-bit flag (0x0100), the offset
 * array, and the phrase bodies. If no phrases were added, only the
 * count and flag are written.
 *
 * @param[in] hPhr Handle. Not NULLHANDLE.
 * @param[in] f    Output stream. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hPhr or @a f is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrWrite(HPHR hPhr, FILE* f);

#endif /* PHR_H */
