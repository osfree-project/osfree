/* set.h - set or bag of fixed-size keys.
 * Part of the ccl container library. */
#ifndef CCL_SET_H
#define CCL_SET_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file set.h
 * @brief Set (unique keys) or bag (duplicate keys) of fixed-size
 *        keys.
 *
 * A set stores keys by value. Each entry carries only a key; there
 * is no separate value. The set is a thin profile over the
 * hash_private engine with ulValueSize = 0 (see hashtable.h for the
 * related key/value container).
 *
 * @par Unique vs duplicates
 * By default the set is a mathematical set: each key occurs at most
 * once, SetAdd with an existing key has no effect.
 *
 * With CCL_FLAG_ALLOW_DUPLICATES the set behaves as a bag: SetAdd
 * always inserts a new occurrence, SetContains returns TRUE if at
 * least one occurrence exists, SetRemove removes one occurrence,
 * SetGetCount counts all occurrences.
 *
 * @par Ownership
 * The set owns the copied key bytes. If pDestroyValue is supplied,
 * it is called on the stored key bytes when an entry is removed or
 * the set is destroyed. There is no pDestroyKey in the set profile:
 * the stored bytes are the key, and pDestroyValue is the callback
 * that releases them (see ccl_conventions.md §6).
 *
 * @par Iteration
 * Phase 1 provides index-based access only (SetGetIth,
 * SetGetCount). The iteration order is bucket order, not insertion
 * order, and is not stable across rehash. In Phase 3 the set will
 * additionally be exposed through the unified HITERATOR interface.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * set is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HSET
 * @brief Handle to a set.
 */
typedef HANDLE HSET;

/**
 * @typedef PHHSET
 * @brief Pointer to a set handle.
 */
typedef HSET *PHHSET;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty set.
 *
 * @param[in]  ulKeySize         Size of one key in bytes. Must be > 0.
 * @param[in]  ulInitialCapacity Initial bucket count. 0 means "use
 *                               default" (8).
 * @param[in]  pOpts             Option block. May be NULL. Required
 *                               fields: pHash. Allowed fields:
 *                               pCompare (used for key equality
 *                               inside a bucket; if not supplied,
 *                               equality is a byte-for-byte memcmp
 *                               of ulKeySize bytes, which is
 *                               correct for POD keys), pKeyFn (for
 *                               KeySet/KeyBag where the element
 *                               carries additional data not exposed
 *                               through the set), pDestroyValue
 *                               (releases owned key data),
 *                               ulRehashThreshold. Allowed flags:
 *                               CCL_FLAG_ALLOW_DUPLICATES,
 *                               CCL_FLAG_DEFAULT_MANAGED,
 *                               CCL_FLAG_DEFAULT_READONLY.
 *                               pDestroyKey is rejected (see
 *                               ccl_conventions.md §5.1 and S1).
 * @param[out] phSet             Receives the new handle. Not NULL.
 *                               Set to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phSet is NULL, ulKeySize is zero,
 *                                  or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SetCreate(ULONG ulKeySize, ULONG ulInitialCapacity,
                          PCCLOPTS pOpts, PHHSET phSet);

/**
 * @brief Destroy a set.
 *
 * Releases every node. If pDestroyValue was supplied at creation,
 * it is called on the stored key bytes of every remaining entry
 * before the nodes are released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hSet  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY SetDestroy(HSET hSet);

/* ==================================================================
 * Adding entries
 * ================================================================== */

/**
 * @brief Insert a key.
 *
 * Without CCL_FLAG_ALLOW_DUPLICATES, if the key is already present
 * the call is a no-op and *pfAdded receives FALSE. With
 * CCL_FLAG_ALLOW_DUPLICATES, a new occurrence is always inserted
 * and *pfAdded receives TRUE.
 *
 * After SetAdd returns, the caller must not use pKey again;
 * ownership of any data it points to (if the caller intends to
 * transfer it) has been dealt with through pDestroyValue on
 * replace or is now owned by the set.
 *
 * pfAdded may be NULL.
 *
 * @param[in]  hSet     Handle. Not NULLHANDLE.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pfAdded  Optional. Receives TRUE if a new occurrence
 *                      was added, FALSE if the key was already
 *                      present (and the set is unique).
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY SetAdd(HSET hSet, PCVOID pKey, PBOOL pfAdded);

/* ==================================================================
 * Lookup
 * ================================================================== */

/**
 * @brief Test whether a key is present.
 *
 * With CCL_FLAG_ALLOW_DUPLICATES, TRUE is returned if at least one
 * occurrence exists.
 *
 * @param[in]  hSet      Handle. Not NULLHANDLE.
 * @param[in]  pKey      Pointer to the key bytes. Not NULL.
 * @param[out] pfPresent Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet, pKey or pfPresent is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetContains(HSET hSet, PCVOID pKey, PBOOL pfPresent);

/* ==================================================================
 * Removing entries
 * ================================================================== */

/**
 * @brief Remove one occurrence of a key.
 *
 * With CCL_FLAG_ALLOW_DUPLICATES, one occurrence is removed; other
 * occurrences of the same key remain. Without the flag, the unique
 * entry is removed if present.
 *
 * pDestroyValue (if supplied) is called on the stored key bytes
 * before the node is released.
 *
 * @param[in]  hSet       Handle. Not NULLHANDLE.
 * @param[in]  pKey       Pointer to the key bytes. Not NULL.
 * @param[out] pfRemoved  Optional. Receives TRUE if a key was
 *                        removed, FALSE otherwise.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_PARAMETER  hSet or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetRemove(HSET hSet, PCVOID pKey, PBOOL pfRemoved);

/**
 * @brief Remove every entry from the set.
 *
 * pDestroyValue (if supplied) is called on each entry before its
 * node is released. After this call, the set is empty. The bucket
 * array is kept (capacity unchanged).
 *
 * @param[in] hSet  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when the set is
 *                                  already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetClear(HSET hSet);

/* ==================================================================
 * Metadata
 * ================================================================== */

/**
 * @brief Return the number of entries.
 *
 * With CCL_FLAG_ALLOW_DUPLICATES, counts all occurrences of every
 * key.
 *
 * @param[in]  hSet      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetGetCount(HSET hSet, PULONG pulCount);

/**
 * @brief Return the current number of buckets.
 *
 * @param[in]  hSet      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the bucket count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetGetBucketCount(HSET hSet, PULONG pulCount);

/**
 * @brief Force a rehash to the requested bucket count.
 *
 * Permitted only when the set is empty (count == 0). ulNewCapacity
 * is rounded up to the next power of 2.
 *
 * @param[in] hSet          Handle. Not NULLHANDLE.
 * @param[in] ulNewCapacity Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  The set is not empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET APIENTRY SetSetBucketCount(HSET hSet, ULONG ulNewCapacity);

/* ==================================================================
 * Ordered access
 * ================================================================== */

/**
 * @brief Copy the key of the i-th entry into the caller's buffer.
 *
 * The order is bucket order, not insertion order. i ranges from 0
 * to count - 1. Size-query convention:
 *   - pBuf == NULL && ulSize == 0: only *pulUsed = ulKeySize.
 *   - ulSize >= ulKeySize: copy the key bytes; *pulUsed = ulKeySize.
 *   - ulSize < ulKeySize: ERROR_BUFFER_OVERFLOW; *pulUsed =
 *     ulKeySize.
 *
 * @param[in]  hSet     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY SetGetIth(HSET hSet, ULONG ulIndex,
                          PVOID pBuf, ULONG ulSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* CCL_SET_H */
