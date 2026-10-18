/* hashtable.h - hash table of fixed-size key/value pairs.
 * Part of the ccl container library. */
#ifndef CCL_HASHTABLE_H
#define CCL_HASHTABLE_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file hashtable.h
 * @brief Hash table with fixed-size keys and values.
 *
 * A hashtable stores key/value pairs by value. The sizes of the key
 * and the value are set at creation and never change. On Put, the
 * caller's key and value are copied into a node allocated from the
 * ccl node allocator; the caller's buffers may go out of scope
 * immediately afterwards.
 *
 * @par Order
 * Unordered. The iteration order of the underlying bucket array is
 * not insertion order and is not stable across rehash, except that
 * the HITERATOR interface (Phase 3) will provide snapshot semantics
 * through the generation counter.
 *
 * @par Duplicates
 * By default keys are unique: Put with an existing key replaces the
 * value. With CCL_FLAG_ALLOW_DUPLICATES, duplicate keys are
 * permitted; lookup returns the first inserted occurrence, Remove
 * removes one occurrence, GetCount counts all occurrences.
 *
 * @par Ownership
 * The hashtable owns the nodes and the copied key/value bytes. If
 * pDestroyKey / pDestroyValue are supplied, they are called on the
 * stored key / value bytes when an entry is removed or the table is
 * destroyed. Otherwise the caller remains responsible for whatever
 * the copied bytes point to (if anything).
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * hashtable is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HHASHTABLE
 * @brief Handle to a hashtable.
 */
typedef HANDLE HHASHTABLE;

/**
 * @typedef PHHASHTABLE
 * @brief Pointer to a hashtable handle.
 */
typedef HHASHTABLE *PHHASHTABLE;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty hashtable.
 *
 * @param[in]  ulKeySize         Size of one key in bytes. Must be > 0.
 * @param[in]  ulValueSize       Size of one value in bytes. Must be
 *                               > 0. For set-style use (key only),
 *                               see set.h instead.
 * @param[in]  ulInitialCapacity Initial bucket count. 0 means "use
 *                               default" (8).
 * @param[in]  pOpts             Option block. May be NULL. Required
 *                               fields: pHash and pCompare. Allowed
 *                               fields: pDestroyKey, pDestroyValue,
 *                               ulRehashThreshold. Allowed flags:
 *                               CCL_FLAG_ALLOW_DUPLICATES,
 *                               CCL_FLAG_DEFAULT_MANAGED,
 *                               CCL_FLAG_DEFAULT_READONLY.
 * @param[out] phTable           Receives the new handle. Not NULL.
 *                               Set to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument, zero key/value size,
 *                                  or pOpts lacks pHash or pCompare.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HashtableCreate(ULONG ulKeySize, ULONG ulValueSize,
                                ULONG ulInitialCapacity,
                                PCCLOPTS pOpts,
                                PHHASHTABLE phTable);

/**
 * @brief Destroy a hashtable.
 *
 * Releases every node. If pDestroyKey / pDestroyValue were supplied
 * at creation, they are called on the stored key / value bytes of
 * every remaining entry before the nodes are released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hTable  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY HashtableDestroy(HHASHTABLE hTable);

/* ==================================================================
 * Adding and replacing entries
 * ================================================================== */

/**
 * @brief Insert or replace a key/value pair.
 *
 * If the key exists (via pCompare in its bucket chain) and
 * CCL_FLAG_ALLOW_DUPLICATES is not set, the value is replaced: the
 * new value is copied over the old one, and pDestroyValue (if
 * supplied) is called on the old value bytes before the copy. The
 * caller's new key is released via pDestroyKey (if supplied) to
 * give the caller a chance to release any owned data behind the
 * redundant key copy. The key already in the table stays.
 *
 * If the key exists and CCL_FLAG_ALLOW_DUPLICATES is set, a new
 * node is allocated with a copy of the key and value, so both
 * occurrences exist.
 *
 * If the key does not exist, a new node is allocated and the key
 * and value bytes are copied in.
 *
 * After HashtablePut returns, the caller must not use pKey or
 * pValue again; ownership of any data they point to (if the caller
 * intends to transfer it) has already been dealt with through the
 * destroy callbacks, or (if no destroy callbacks were supplied)
 * the caller remains responsible for the pointed-to data, and the
 * copies made by the table are just bytes.
 *
 * A Put may trigger a rehash if the load factor exceeds the
 * effective threshold from pOpts (default 0.75 = 750 permille).
 *
 * pfReplaced may be NULL.
 *
 * @param[in]  hTable      Handle. Not NULLHANDLE.
 * @param[in]  pKey        Pointer to the key bytes. Not NULL.
 * @param[in]  pValue      Pointer to the value bytes. Not NULL.
 * @param[out] pfReplaced  Optional. Receives TRUE if the key existed
 *                         and the value was replaced (i.e. no new
 *                         node was allocated). Receives FALSE if a
 *                         new node was inserted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable, pKey or pValue is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY HashtablePut(HHASHTABLE hTable, PCVOID pKey,
                             PCVOID pValue, PBOOL pfReplaced);

/* ==================================================================
 * Lookup
 * ================================================================== */

/**
 * @brief Look up a key and copy the value into the caller's buffer.
 *
 * Size-query convention:
 *   - pBuf == NULL && ulSize == 0: only *pulUsed = ulValueSize.
 *   - ulSize >= ulValueSize: copy the value bytes; *pulUsed =
 *     ulValueSize.
 *   - ulSize < ulValueSize: ERROR_BUFFER_OVERFLOW; *pulUsed =
 *     ulValueSize.
 *
 * If CCL_FLAG_ALLOW_DUPLICATES is set and multiple entries share
 * the key, the value of the first inserted occurrence is returned.
 *
 * @param[in]  hTable   Handle. Not NULLHANDLE.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable or pKey is NULL, or pBuf
 *                                  is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The key was not found.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HashtableGet(HHASHTABLE hTable, PCVOID pKey,
                             PVOID pBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Test whether a key is present.
 *
 * @param[in]  hTable   Handle. Not NULLHANDLE.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pfFound  Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable, pKey or pfFound is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableContains(HHASHTABLE hTable, PCVOID pKey,
                                  PBOOL pfFound);

/* ==================================================================
 * Removing
 * ================================================================== */

/**
 * @brief Remove one entry by key.
 *
 * If CCL_FLAG_ALLOW_DUPLICATES is set and multiple entries share
 * the key, one occurrence is removed (the first inserted one).
 * Otherwise the unique entry is removed if present.
 *
 * pDestroyKey / pDestroyValue (if supplied) are called on the
 * stored key/value bytes before the node is released.
 *
 * @param[in]  hTable    Handle. Not NULLHANDLE.
 * @param[in]  pKey      Pointer to the key bytes. Not NULL.
 * @param[out] pfRemoved Optional. Receives TRUE if an entry was
 *                       removed, FALSE otherwise.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_PARAMETER  hTable or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableRemove(HHASHTABLE hTable, PCVOID pKey,
                                PBOOL pfRemoved);

/**
 * @brief Remove every entry from the hashtable.
 *
 * pDestroyKey / pDestroyValue (if supplied) are called on each
 * entry before its node is released. After this call, the table is
 * empty. The bucket array is kept (capacity unchanged).
 *
 * @param[in] hTable  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when the table is
 *                                  already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableClear(HHASHTABLE hTable);

/* ==================================================================
 * Metadata
 * ================================================================== */

/**
 * @brief Return the number of entries.
 *
 * With CCL_FLAG_ALLOW_DUPLICATES, counts all occurrences of every
 * key.
 *
 * @param[in]  hTable    Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableGetCount(HHASHTABLE hTable, PULONG pulCount);

/**
 * @brief Return the current number of buckets.
 *
 * @param[in]  hTable    Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the bucket count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableGetBucketCount(HHASHTABLE hTable, PULONG pulCount);

/**
 * @brief Force a rehash to the requested bucket count.
 *
 * ulNewCapacity is rounded up to the next power of 2. Every existing
 * entry is rehashed. Called internally by HashtablePut when the load
 * factor exceeds the effective threshold. Exposed here for explicit
 * growth control (e.g. before a burst of inserts).
 *
 * @param[in] hTable        Handle. Not NULLHANDLE.
 * @param[in] ulNewCapacity Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed;
 *                                  previous buckets remain in place.
 */
APIRET APIENTRY HashtableSetBucketCount(HHASHTABLE hTable,
                                        ULONG ulNewCapacity);

/* ==================================================================
 * Ordered access
 * ================================================================== */

/**
 * @brief Copy the key of the i-th entry into the caller's buffer.
 *
 * The order is bucket order, not insertion order. i ranges from 0
 * to count - 1. Size-query convention as in HashtableGet.
 *
 * @param[in]  hTable   Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HashtableGetIthKey(HHASHTABLE hTable, ULONG ulIndex,
                                   PVOID pBuf, ULONG ulSize,
                                   PULONG pulUsed);

/**
 * @brief Copy the value of the i-th entry into the caller's buffer.
 *
 * Same as HashtableGetIthKey but for the value bytes.
 *
 * @param[in]  hTable   Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HashtableGetIthValue(HHASHTABLE hTable, ULONG ulIndex,
                                     PVOID pBuf, ULONG ulSize,
                                     PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* CCL_HASHTABLE_H */
