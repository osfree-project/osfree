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
 * @par Iteration
 * To walk the set without copying, open an enumeration cursor with
 * StrSetEnumFirst and advance it with StrSetEnumNext. Each step
 * retrieves the current string into a caller-supplied buffer via
 * StrSetEnumGet, using the standard size-query convention. Cursors
 * are released by StrSetEnumClose, or automatically by
 * StrSetDestroy.
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

/**
 * @typedef HSTRSETENUM
 * @brief Handle to an enumeration cursor over a string set.
 *
 * Issued by StrSetEnumFirst, released by StrSetEnumClose or by
 * StrSetDestroy of the owning set.
 */
typedef HANDLE HSTRSETENUM;

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
 * Any enumeration cursors opened on the set are also released.
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

/* ==================================================================
 * Enumeration
 * ================================================================== */

/**
 * @brief Open an enumeration cursor on a set.
 *
 * On success, the cursor is positioned on the first string of the
 * set. The cursor must be released by StrSetEnumClose (or by
 * StrSetDestroy of the owning set).
 *
 * @param[in]  hSet    Handle. Not NULLHANDLE.
 * @param[out] phEnum  Cursor receiver. Not NULL. Set to NULLHANDLE
 *                     on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet or phEnum is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The set is empty. No cursor
 *                                  created.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StrSetEnumFirst(HSTRSET hSet, HSTRSETENUM *phEnum);

/**
 * @brief Advance an enumeration cursor to the next string.
 *
 * If the cursor is already on the last string, the cursor is not
 * moved and ERROR_NO_MORE_ITEMS is returned.
 *
 * @param[in] hEnum  Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Cursor is on the last string.
 */
APIRET APIENTRY StrSetEnumNext(HSTRSETENUM hEnum);

/**
 * @brief Retrieve the string at the cursor.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied and NUL-terminated; *pulUsed
 *     is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hEnum or pszBuf (without
 *                                  size-query) is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY StrSetEnumGet(HSTRSETENUM hEnum,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Close an enumeration cursor.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hEnum  Cursor. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY StrSetEnumClose(HSTRSETENUM hEnum);

#ifdef __cplusplus
}
#endif

#endif /* CCL_STRSET_H */
