/*!
 *
 * @file hash_private.h
 *
 * @brief Internal hash-table engine used by the public hash-based
 *        containers.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * Provides the single hash-table engine behind hashtable.c, set.c,
 * strmap.c, strset.c and (in Phase 5) the SOMStringTableC wrapper.
 * One implementation, several thin profiles. See the architecture
 * document (§3.14) for the rationale.
 *
 * Storage model. A hash_private table is either internal (bucket
 * array, capacity and count owned by the HASHCTL and released by
 * HashDestroy) or external (the same three values live in the
 * caller's structure and are reached through HASH_EXTERNAL
 * pointers; used by SOMStringTableC to keep its
 * table/tableSize/numberOfEntries fields as the real storage).
 *
 * Keys and values are stored inline in HASHNODE, after the chain
 * pointer. Layout inside HASHNODE.u:
 *
 *     [key bytes: ulKeySize]
 *     [padding to ulValueOffset]
 *     [value bytes: ulValueSize]
 *
 * where ulValueOffset = CCL_ALIGN_UP(ulKeySize). All callbacks
 * receive pointers to the stored bytes, not to the caller's original
 * data; the container makes one copy of key and value at insert
 * time and never looks at them again except through the callbacks.
 */

#ifndef CCL_HASH_PRIVATE_H
#define CCL_HASH_PRIVATE_H

#include "os2types.h"
#include "ccl_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Constants
 * ================================================================== */

/*!
 * @def CCL_HASH_MAGIC
 * @brief Magic value "HASH" for the HASHCTL control block.
 *
 * Big-endian value matching the convention of the other ccl magic
 * values ("VECT" = 0x56454354, "DLST" = 0x444C5354).
 */
#define CCL_HASH_MAGIC  0x48415348UL

/*!
 * @def CCL_MAX_ALIGN
 * @brief Maximum alignment for key/value data inside HASHNODE.
 *
 * The union in HASHNODE contains a double, which forces 8-byte
 * alignment on every supported target (OpenWatcom 32/64-bit, MSVC).
 * Must be a power of 2; CCL_ALIGN_UP relies on that.
 */
#define CCL_MAX_ALIGN  8

/*!
 * @def CCL_ALIGN_UP
 * @brief Round a byte count up to the next multiple of CCL_MAX_ALIGN.
 */
#define CCL_ALIGN_UP(ulValue)  \
    (((ulValue) + (CCL_MAX_ALIGN - 1)) & ~(CCL_MAX_ALIGN - 1))

/* ==================================================================
 * Node and storage types
 * ================================================================== */

/*!
 * @struct _HASHNODE
 * @brief One entry in a bucket chain.
 *
 * Memory layout:
 *
 *     [next][padding][u]
 *
 * where u contains:
 *
 *     [key bytes][pad][value bytes]
 *
 * offsetof(u) is 8 on both 32-bit and 64-bit targets, because the
 * union contains a double and the compiler therefore aligns u to an
 * 8-byte boundary. This satisfies ccl_node_alloc's requirement
 * ulFixedPart % ulAlign == 0 when ulAlign = CCL_MAX_ALIGN = 8, so
 * no explicit padding field is needed.
 *
 * The value's offset inside u is not fixed; it is
 * CCL_ALIGN_UP(ulKeySize), stored in HASHCTL.ulValueOffset, so that
 * (offsetof(u) + ulValueOffset) stays 8-aligned regardless of
 * ulKeySize.
 */
typedef struct _HASHNODE {
    struct _HASHNODE *next;  /*!< Next node in the chain, or NULL.  */
    union {
        double d;            /*!< Forces 8-byte alignment of u.    */
        void  *p;
        long   l;
        char   c[1];
    } u;                     /*!< [key bytes][pad][value bytes].    */
} HASHNODE, *PHASHNODE;

/*!
 * @struct _HASH_EXTERNAL
 * @brief External storage descriptor for HashCreateWithStorage.
 *
 * Each field points to the caller's actual storage for the
 * corresponding value:
 *
 *   - ppBucketArray points to the caller's bucket-array pointer
 *     (so its own type is PVOID **: the bucket array is a PVOID *,
 *     and we hold a pointer to it);
 *   - pulBucketCount points to a ULONG (the capacity);
 *   - pulEntryCount points to a ULONG (the entry count).
 *
 * Used by SOMStringTableC to keep its
 * table / tableSize / numberOfEntries fields as the real storage,
 * while hash_private operates on them through these pointers:
 *
 *     table       is tablePT, i.e. PVOID *;
 *     &table      is PVOID **, which is exactly ppBucketArray;
 *     tableSize   is ULONG, its address is PULONG;
 *     numberOfEntries is ULONG, its address is PULONG.
 *
 * Callers must initialize HASH_EXTERNAL to point at real storage
 * before calling HashCreateWithStorage.
 */
typedef struct _HASH_EXTERNAL {
    PVOID **ppBucketArray;    /*!< Pointer to the bucket-array
                                   pointer.                         */
    PULONG  pulBucketCount;   /*!< Pointer to the capacity.          */
    PULONG  pulEntryCount;    /*!< Pointer to the entry count.       */
} HASH_EXTERNAL, *PHASH_EXTERNAL;

/*!
 * @struct _HASHCTL
 * @brief Control block for one hash table.
 *
 * The caller provides HASHCTL storage. For the public hash-based
 * containers (hashtable.c, set.c, strmap.c, strset.c) the HASHCTL
 * is embedded in the container's own control block. For
 * SOMStringTableC it is embedded in SOMStringTableCData.hHash.
 *
 * HashCreate / HashCreateWithStorage fill the control block in.
 * HashDestroy releases the internal buckets and nodes; the HASHCTL
 * storage itself remains the caller's responsibility.
 *
 * Callback ownership. The callbacks from CCLOPTS are copied into
 * the HASHCTL at create time; the caller's CCLOPTS block may go out
 * of scope immediately after the constructor returns.
 */
typedef struct _HASHCTL {
    /*!
     * Magic CCL_HASH_MAGIC. Checked by ccl_validate.
     */
    ULONG ulMagic;

    /*!
     * Hash callback. Required: used to compute the bucket index for
     * every operation.
     */
    CCL_HASH_FN pHash;

    /*!
     * Ordering callback. Required: used for key equality inside a
     * bucket chain and for the ordered variants (rbtree) that share
     * this engine.
     */
    CCL_COMPARE_FN pCompare;

    /*!
     * Key destruction callback, or NULL. Called on the stored key
     * bytes when an entry is removed or when a HashPut replaces an
     * existing key (the container calls it on the new key to let
     * the caller release the redundant copy; the old key stays in
     * the node).
     */
    CCL_DESTROY_FN pDestroyKey;

    /*!
     * Value destruction callback, or NULL. Called on the stored
     * value bytes when an entry is removed and, when a HashPut
     * replaces an existing entry, on the old value bytes before the
     * new value is written.
     */
    CCL_DESTROY_FN pDestroyValue;

    /*!
     * Effective rehash threshold in permille (1..1000). Set from
     * CCLOPTS.ulRehashThreshold, or to CCL_REHASH_THRESHOLD_DEFAULT
     * when the caller passed 0.
     */
    ULONG ulRehashThreshold;

    /*!
     * External mode: pointer to the caller's bucket array pointer.
     * Non-NULL in external mode (HashCreateWithStorage); NULL in
     * internal mode (HashCreate).
     */
    PVOID **ppExternalBuckets;

    /*!
     * External mode: pointer to the caller's capacity variable.
     */
    PULONG pulExternalCapacity;

    /*!
     * External mode: pointer to the caller's count variable.
     */
    PULONG pulExternalCount;

    /*!
     * Internal mode: bucket array. NULL in external mode.
     */
    PVOID *pInternalBuckets;

    /*!
     * Internal mode: number of buckets. Unused in external mode.
     */
    ULONG ulInternalCapacity;

    /*!
     * Internal mode: number of entries. Unused in external mode.
     */
    ULONG ulInternalCount;

    /*!
     * Size of one key, in bytes. Fixed per table.
     */
    ULONG ulKeySize;

    /*!
     * Size of one value, in bytes. Fixed per table. May be 0
     * (set-style containers store keys only).
     */
    ULONG ulValueSize;

    /*!
     * Precomputed offset of the value inside HASHNODE.u.
     * Equal to CCL_ALIGN_UP(ulKeySize). Cached to avoid recomputing
     * it on every operation.
     */
    ULONG ulValueOffset;
} HASHCTL, *PHASHCTL;

/* ==================================================================
 * API
 * ================================================================== */

/*!
 * @brief Create a hash table with internal storage.
 *
 * phCtl points to caller-provided HASHCTL storage. On success the
 * control block is filled in, the magic is set to CCL_HASH_MAGIC,
 * and the initial bucket array is allocated through (*CclMalloc).
 *
 * pHash and pCompare from pOpts are required: hash_private cannot
 * function without them. If either is NULL, the function returns
 * ERROR_INVALID_PARAMETER (public containers validate via
 * CclValidateOpts first; this is a defensive check).
 *
 * ulInitialCapacity 0 means "use default" (8). ulKeySize must be
 * greater than 0. ulValueSize may be 0 (set-style); when it is 0,
 * no value bytes are stored and HashGet / HashGetIthValue return
 * ERROR_NO_MORE_ITEMS.
 *
 * @param[in]  ulInitialCapacity  Initial bucket count, or 0 for
 *                                default.
 * @param[in]  ulKeySize          Size of one key, in bytes.
 * @param[in]  ulValueSize        Size of one value, in bytes
 *                                (0 allowed for set-style).
 * @param[in]  pOpts              Callback block. May be NULL, but
 *                                then pHash and pCompare are
 *                                missing and the call fails.
 * @param[out] phCtl              Caller-provided HASHCTL storage.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phCtl is NULL, ulKeySize is 0,
 *                                  pOpts is NULL, or pOpts lacks
 *                                  pHash or pCompare.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashCreate(
    ULONG     ulInitialCapacity,
    ULONG     ulKeySize,
    ULONG     ulValueSize,
    PCCLOPTS  pOpts,
    PHASHCTL  phCtl);

/*!
 * @brief Create a hash table with external storage.
 *
 * Same as HashCreate, except the bucket array, capacity and count
 * live in the caller's fields reached through pStorage. Used by
 * SOMStringTableC, which keeps table / tableSize / numberOfEntries
 * as the real storage.
 *
 * If pStorage is NULL, the function behaves like HashCreate with
 * the default capacity (no external fields). This lets a caller
 * that only sometimes uses external storage share one code path.
 *
 * @param[in]  pOpts      Callback block. Same requirements as
 *                        HashCreate.
 * @param[in]  ulKeySize  Size of one key, in bytes.
 * @param[in]  ulValueSize Size of one value, in bytes.
 * @param[out] phCtl      Caller-provided HASHCTL storage.
 * @param[in]  pStorage   External storage descriptor, or NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phCtl is NULL, ulKeySize is 0,
 *                                  pOpts is NULL, or pOpts lacks
 *                                  pHash or pCompare.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashCreateWithStorage(
    PCCLOPTS       pOpts,
    ULONG          ulKeySize,
    ULONG          ulValueSize,
    PHASHCTL       phCtl,
    PHASH_EXTERNAL pStorage);

/*!
 * @brief Insert or replace a key/value pair.
 *
 * If a matching key is found (via pCompare inside the bucket
 * chain), the new value replaces the old one: pDestroyKey is
 * called on the new key bytes (to let the caller release the
 * redundant copy), pDestroyValue is called on the old value bytes
 * (if pDestroyValue is non-NULL), then the new value is copied into
 * the node's value slot. The key already in the node stays.
 *
 * If no match is found, a new node is allocated through
 * ccl_node_alloc, the key and value bytes are copied in, and the
 * node is prepended to its bucket chain.
 *
 * After HashPut returns, the caller must not use pKey or pValue
 * again: ownership of the underlying data (if any) has transferred
 * to the container, or the redundant new key has already been
 * released via pDestroyKey.
 *
 * May trigger HashGrow if the load factor exceeds the effective
 * rehash threshold from pOpts.
 *
 * pfReplaced may be NULL if the caller does not care whether the
 * key was replaced.
 *
 * @param[in,out] h          Hash control block. Not NULL.
 * @param[in]     pKey       Pointer to the key bytes. Not NULL.
 * @param[in]     pValue     Pointer to the value bytes. May be NULL
 *                           when ulValueSize == 0.
 * @param[out]    pfReplaced Receives TRUE if the key existed and was
 *                           replaced, FALSE if a new entry was
 *                           inserted. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pKey is NULL, or pValue is NULL
 *                                  while ulValueSize > 0.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET HashPut(HASHCTL *h, PCVOID pKey, PCVOID pValue, PBOOL pfReplaced);

/*!
 * @brief Append a new key/value pair unconditionally.
 *
 * Unlike HashPut, HashPutAppend never looks for an existing key: a
 * new node is always allocated and prepended to its bucket chain.
 * Existing entries with the same key are left untouched, and
 * pDestroyKey / pDestroyValue are not called on the new key or
 * value.
 *
 * Used by the bag profiles (set with CCL_FLAG_ALLOW_DUPLICATES)
 * where every Add is required to create a new occurrence. The
 * public set and hashtable containers continue to use HashPut;
 * HashPutAppend is only reachable through set.c with the duplicate
 * flag set.
 *
 * HashPutAppend may trigger a rehash if the load factor exceeds the
 * effective rehash threshold from the CCLOPTS passed to
 * HashCreate.
 *
 * @param[in,out] h       Hash control block. Not NULL.
 * @param[in]     pKey    Pointer to the key bytes. Not NULL.
 * @param[in]     pValue  Pointer to the value bytes. May be NULL
 *                        when ulValueSize == 0.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pKey is NULL, or pValue is NULL
 *                                  while ulValueSize > 0.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET HashPutAppend(HASHCTL *h, PCVOID pKey, PCVOID pValue);

/*!
 * @brief Look up a key and copy the value into the caller's buffer.
 *
 * Size-query convention:
 *
 *   - pBuf == NULL && ulSize == 0: only *pulUsed = ulValueSize.
 *   - ulSize >= ulValueSize: copy ulValueSize bytes into pBuf;
 *     *pulUsed = ulValueSize.
 *   - ulSize < ulValueSize: ERROR_BUFFER_OVERFLOW; *pulUsed =
 *     ulValueSize.
 *
 * If the key is not found, ERROR_NO_MORE_ITEMS is returned.
 *
 * @param[in]  h        Hash control block. Not NULL.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Receives the used or required size. May be
 *                      NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pKey is NULL, or pBuf is NULL
 *                                  outside size-query.
 * @retval ERROR_NO_MORE_ITEMS      The key was not found.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET HashGet(HASHCTL *h, PCVOID pKey, PVOID pBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Remove an entry by key.
 *
 * If the key is found, its node is unlinked, pDestroyKey and
 * pDestroyValue are called on the stored key/value bytes (if
 * non-NULL), the node is released through ccl_node_free, and
 * *pfFound is set to TRUE. Otherwise *pfFound is set to FALSE and
 * nothing is changed.
 *
 * When the table contains duplicate keys (from HashPutAppend), one
 * occurrence is removed: the first one found in the bucket chain.
 *
 * @param[in,out] h       Hash control block. Not NULL.
 * @param[in]     pKey    Pointer to the key bytes. Not NULL.
 * @param[out]    pfFound Receives TRUE if the entry was found and
 *                        removed. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pKey is NULL.
 */
APIRET HashRemove(HASHCTL *h, PCVOID pKey, PBOOL pfFound);

/*!
 * @brief Force a rehash to the requested bucket count.
 *
 * ulNewCapacity is rounded up to the next power of 2. Every existing
 * entry is rehashed into the new bucket array. The old bucket array
 * is released through (*CclFree) (in internal mode) or left alone
 * (in external mode, where the caller owns the storage).
 *
 * Called internally by HashPut and HashPutAppend when the load
 * factor exceeds the effective rehash threshold. Exposed here for
 * testing and for caller-controlled growth (e.g. somstTargetCapacity
 * before the first insert).
 *
 * @param[in,out] h              Hash control block. Not NULL.
 * @param[in]     ulNewCapacity  Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed;
 *                                  the previous buckets are still in
 *                                  place.
 */
APIRET HashGrow(HASHCTL *h, ULONG ulNewCapacity);

/*!
 * @brief Release the internal storage of a hash table.
 *
 * Walks every bucket chain and releases every node through
 * ccl_node_free. In internal mode, the bucket array itself is also
 * released. In external mode, the bucket array and the
 * capacity/count fields belong to the caller and are not touched.
 *
 * The HASHCTL itself is not released; the caller owns its storage.
 * After HashDestroy, the magic is set to 0 so ccl_validate rejects
 * the control block.
 *
 * pDestroyKey and pDestroyValue are NOT called by HashDestroy.
 * Containers that own their keys/values must drain their contents
 * with an explicit loop before calling HashDestroy, or provide
 * pDestroy* and rely on HashRemove.
 *
 * @param[in,out] h  Hash control block. May be NULL (no-op).
 */
void HashDestroy(HASHCTL *h);

/*!
 * @brief Copy the key of the i-th entry into the caller's buffer.
 *
 * The order is bucket order, not insertion order. i ranges from 0
 * to count - 1. Size-query convention is the same as HashGet.
 *
 * @param[in]  h        Hash control block. Not NULL.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Receives the used or required size. May be
 *                      NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL outside size-query.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET HashGetIthKey(HASHCTL *h, ULONG ulIndex, PVOID pBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Copy the value of the i-th entry into the caller's buffer.
 *
 * Same as HashGetIthKey but for the value bytes. Returns
 * ERROR_NO_MORE_ITEMS if ulValueSize == 0 (no value slot).
 *
 * @param[in]  h        Hash control block. Not NULL.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Receives the used or required size. May be
 *                      NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL outside size-query.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count, or
 *                                  ulValueSize == 0.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET HashGetIthValue(HASHCTL *h, ULONG ulIndex, PVOID pBuf, ULONG ulSize, PULONG pulUsed);

/*!
 * @brief Copy the current entry count into *pulCount.
 *
 * @param[in]  h         Hash control block. Not NULL.
 * @param[out] pulCount  Receives the entry count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET HashGetCount(HASHCTL *h, PULONG pulCount);

/*!
 * @brief Copy the current bucket count into *pulCount.
 *
 * @param[in]  h         Hash control block. Not NULL.
 * @param[out] pulCount  Receives the bucket count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET HashGetBucketCount(HASHCTL *h, PULONG pulCount);

/*!
 * @brief Resize the bucket array to the requested count.
 *
 * Permitted only when the table is empty (count == 0). ulNewCapacity
 * is rounded up to the next power of 2. In internal mode the old
 * bucket array is released and a new one is allocated; in external
 * mode the caller's bucket array pointer is updated and the old one
 * remains the caller's responsibility.
 *
 * This corresponds to SOMStringTableC's somstTargetCapacity, which
 * IBM documents as valid only before any strings are added.
 *
 * @param[in,out] h              Hash control block. Not NULL.
 * @param[in]     ulNewCapacity  Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is NULL or its magic is not
 *                                  CCL_HASH_MAGIC.
 * @retval ERROR_INVALID_PARAMETER  The table is not empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashSetBucketCount(HASHCTL *h, ULONG ulNewCapacity);

#ifdef __cplusplus
}
#endif

#endif /* CCL_HASH_PRIVATE_H */
