/* rbtree.h - ordered set of fixed-size elements (red-black tree).
 * Part of the ccl container library. */
#ifndef CCL_RBTREE_H
#define CCL_RBTREE_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file rbtree.h
 * @brief Ordered set of fixed-size elements backed by a red-black
 *        tree.
 *
 * An rbtree stores elements by value and keeps them sorted by
 * pCompare. Access by key is O(log n). In-order access by position
 * is O(n) for GetIth; the first and last elements are O(log n) via
 * GetMin / GetMax.
 *
 * @par Element model
 * Each stored element is a single fixed-size block of ulElemSize
 * bytes. The element itself may be a plain value, a pointer, or a
 * caller-defined struct (for example the { key, value } pair used
 * by the SOM SortedMap wrapper). pCompare is responsible for the
 * layout: it receives pointers to two element blocks and returns
 * their ordering according to whatever fields the caller considers
 * significant.
 *
 * @par Duplicates
 * By default the tree is a set: pCompare == 0 on an existing key
 * causes RbtreeInsert to report no insertion (pfInserted = FALSE)
 * and the caller's element is not stored. With
 * CCL_FLAG_ALLOW_DUPLICATES the new element is always inserted, to
 * the right of every existing element with the same key. Lookup
 * and Remove work on whichever duplicate the tree descent reaches
 * first; call Remove repeatedly to drain all occurrences.
 *
 * @par Ownership
 * The tree owns the copied element bytes. If pDestroyValue is
 * supplied it is called on the stored element bytes when an
 * element is removed or the tree is destroyed. pDestroyKey is
 * rejected (see ccl_conventions.md §5.1 and S2).
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * tree is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HRBTREE
 * @brief Handle to an rbtree.
 */
typedef HANDLE HRBTREE;

/**
 * @typedef PHRBTREE
 * @brief Pointer to an rbtree handle.
 */
typedef HRBTREE *PHRBTREE;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty rbtree.
 *
 * @param[in]  ulElemSize  Size of one element in bytes. Must be > 0.
 * @param[in]  pOpts       Option block. May be NULL, but then
 *                         pCompare is missing and the call fails.
 *                         Required: pCompare. Allowed: pKeyFn (for
 *                         future SOM SortedMap support; not used
 *                         by rbtree.c in Phase 1), pDestroyValue,
 *                         CCL_FLAG_ALLOW_DUPLICATES,
 *                         CCL_FLAG_DEFAULT_MANAGED,
 *                         CCL_FLAG_DEFAULT_READONLY. pDestroyKey
 *                         is rejected.
 * @param[out] phTree      Receives the new handle. Not NULL. Set
 *                         to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phTree is NULL, ulElemSize is
 *                                  zero, or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RbtreeCreate(ULONG ulElemSize, PCCLOPTS pOpts,
                             PHRBTREE phTree);

/**
 * @brief Destroy an rbtree.
 *
 * Releases every node. If pDestroyValue was supplied at creation it
 * is called on each remaining element before its node is released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hTree  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY RbtreeDestroy(HRBTREE hTree);

/* ==================================================================
 * Insert, remove
 * ================================================================== */

/**
 * @brief Insert a copy of an element.
 *
 * In unique mode (default), if an element with the same key already
 * exists, the call is a no-op: pfInserted receives FALSE and the
 * caller's element is not stored.
 *
 * In duplicate-allowed mode (CCL_FLAG_ALLOW_DUPLICATES), a new
 * occurrence is always inserted, and pfInserted receives TRUE.
 *
 * @param[in]  hTree       Handle. Not NULLHANDLE.
 * @param[in]  pElem       Pointer to the element to copy. Not NULL.
 * @param[out] pfInserted  Optional. Receives TRUE if a new element
 *                         was stored, FALSE otherwise.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTree or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RbtreeInsert(HRBTREE hTree, PCVOID pElem,
                             PBOOL pfInserted);

/**
 * @brief Remove one element by key.
 *
 * In duplicate-allowed mode, one occurrence is removed (whichever
 * the tree descent reaches first). Call repeatedly to drain all
 * occurrences of a duplicate key.
 *
 * pDestroyValue (if supplied) is called on the removed element
 * bytes before its node is released.
 *
 * @param[in]  hTree      Handle. Not NULLHANDLE.
 * @param[in]  pKey       Pointer to the key bytes. Not NULL.
 * @param[out] pfRemoved  Optional. Receives TRUE if removed.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_PARAMETER  hTree or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeRemove(HRBTREE hTree, PCVOID pKey,
                             PBOOL pfRemoved);

/**
 * @brief Remove every element from the tree.
 *
 * pDestroyValue (if supplied) is called on each element.
 *
 * @param[in] hTree  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeClear(HRBTREE hTree);

/* ==================================================================
 * Lookup
 * ================================================================== */

/**
 * @brief Copy the element matching a key into the caller's buffer.
 *
 * Size-query convention:
 *   - pBuf == NULL && ulSize == 0: only *pulUsed = ulElemSize.
 *   - ulSize >= ulElemSize: copy the element bytes.
 *   - ulSize < ulElemSize: ERROR_BUFFER_OVERFLOW.
 *
 * In duplicate-allowed mode, one of the matching occurrences is
 * returned.
 *
 * @param[in]  hTree    Handle. Not NULLHANDLE.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTree or pKey is NULL, or pBuf
 *                                  is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No matching element.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY RbtreeFind(HRBTREE hTree, PCVOID pKey,
                           PVOID pBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Test whether a key is present.
 *
 * @param[in]  hTree      Handle. Not NULLHANDLE.
 * @param[in]  pKey       Pointer to the key bytes. Not NULL.
 * @param[out] pfPresent  Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeContains(HRBTREE hTree, PCVOID pKey,
                               PBOOL pfPresent);

/**
 * @brief Copy the minimum element into the caller's buffer.
 *
 * Size-query convention as in RbtreeFind.
 *
 * @param[in]  hTree    Handle. Not NULLHANDLE.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The tree is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL without size-query.
 */
APIRET APIENTRY RbtreeGetMin(HRBTREE hTree, PVOID pBuf,
                             ULONG ulSize, PULONG pulUsed);

/**
 * @brief Copy the maximum element into the caller's buffer.
 *
 * @param[in]  hTree    Handle. Not NULLHANDLE.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The tree is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL without size-query.
 */
APIRET APIENTRY RbtreeGetMax(HRBTREE hTree, PVOID pBuf,
                             ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Metadata and in-order access
 * ================================================================== */

/**
 * @brief Return the number of elements.
 *
 * @param[in]  hTree     Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeGetCount(HRBTREE hTree, PULONG pulCount);

/**
 * @brief Copy the i-th element in in-order position.
 *
 * ulIndex 0 yields the minimum, ulIndex count-1 yields the maximum.
 * O(n) per call. Size-query convention as in RbtreeFind.
 *
 * @param[in]  hTree    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based in-order index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTree is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY RbtreeGetIth(HRBTREE hTree, ULONG ulIndex,
                             PVOID pBuf, ULONG ulSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* CCL_RBTREE_H */
