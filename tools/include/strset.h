/* strset.h - set of strings built on top of vector.
 * Part of the ccl container library. */
#ifndef CCL_STRSET_H
#define CCL_STRSET_H

#include "os2types.h"
#include "os2err.h"

/**
 * @file strset.h
 * @brief Set of unique strings.
 *
 * A string set stores one copy of each distinct string added to it.
 * Adding a string that is already present has no effect. Lookup is
 * linear in the number of stored strings.
 *
 * @par Ownership
 * The set owns a copy of every stored string. The copies are
 * released by StrSetDestroy.
 *
 * @par Order
 * Strings are kept in the order they were first added. Access by
 * position is O(1).
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * set is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HSTRSET
 * @brief Handle to a string set.
 */
typedef HANDLE HSTRSET;

/**
 * @typedef PHSTRSET
 * @brief Pointer to a string set handle.
 */
typedef HSTRSET *PHSTRSET;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty string set.
 *
 * @param[out] phSet  Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phSet is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StrSetCreate(PHSTRSET phSet);

/**
 * @brief Destroy a string set and release every stored copy.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hSet  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY StrSetDestroy(HSTRSET hSet);

/* ==================================================================
 * Operations
 * ================================================================== */

/**
 * @brief Add a copy of a string if it is not already present.
 *
 * Empty strings and NULL are ignored silently.
 *
 * @param[in] hSet    Handle. Not NULLHANDLE.
 * @param[in] pszStr  String to add. May be NULL or empty.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (including ignore).
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StrSetAdd(HSTRSET hSet, PCSZ pszStr);

/**
 * @brief Test whether a string is already in the set.
 *
 * Comparison is case-sensitive.
 *
 * @param[in]  hSet    Handle. Not NULLHANDLE.
 * @param[in]  pszStr  String to look for. Not NULL.
 * @param[out] pfFound Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY StrSetContains(HSTRSET hSet, PCSZ pszStr, PBOOL pfFound);

/**
 * @brief Number of strings in the set.
 *
 * @param[in]  hSet      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY StrSetGetCount(HSTRSET hSet, PULONG pulCount);

/**
 * @brief Retrieve one string by position.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied and NUL-terminated; *pulUsed
 *     is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hSet     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based position.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet or pszBuf (without size-query)
 *                                  is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY StrSetGetItem(HSTRSET hSet, ULONG ulIndex,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* CCL_STRSET_H */
