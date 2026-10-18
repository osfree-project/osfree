/* heap.h - binary heap (priority queue) of fixed-size elements.
 * Part of the ccl container library. */
#ifndef CCL_HEAP_H
#define CCL_HEAP_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file heap.h
 * @brief Binary heap of fixed-size elements.
 *
 * A heap stores elements by value and maintains the heap invariant
 * using pCompare. HeapPush sifts up, HeapPop sifts down. HeapTop
 * returns a borrowed pointer to the current root without removing.
 *
 * @par Min-heap semantics
 * The heap is a min-heap: the root is the element that pCompare
 * orders first. Formally, for every element i:
 *
 *     pCompare(root, any_other) <= 0
 *
 * Push and Pop both preserve this invariant. To obtain a max-heap,
 * pass a pCompare that reverses the ordering (return positive when
 * the first argument is logically smaller).
 *
 * @par Relationship to vector
 * The heap is a thin profile over vector.h (see ccl_conventions.md
 * §12). All storage management lives in vector. This module adds
 * only sift_up / sift_down and restricts the public API to
 * priority-queue operations. Sift swaps operate on borrowed
 * references from VectorGetRef, so they do not trigger
 * pDestroyValue; only the final removal in HeapPop and the
 * removals in HeapClear go through VectorRemoveLast, which does.
 *
 * @par Ownership
 * The heap owns only the storage for element bytes (through the
 * underlying vector). If pDestroyValue is supplied, it is called
 * on each element removed by HeapPop, HeapClear or HeapDestroy.
 * HeapTop does not call it (no removal).
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * heap is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HHEAP
 * @brief Handle to a heap.
 */
typedef HANDLE HHEAP;

/**
 * @typedef PHHEAP
 * @brief Pointer to a heap handle.
 */
typedef HHEAP *PHHEAP;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty heap.
 *
 * @param[in]  ulElemSize  Size of one element in bytes. Must be
 *                         greater than 0.
 * @param[in]  pOpts       Option block. May be NULL, but then
 *                         pCompare is missing and the call fails.
 *                         Required: pCompare. Allowed:
 *                         pDestroyValue and the flags
 *                         CCL_FLAG_DEFAULT_MANAGED /
 *                         CCL_FLAG_DEFAULT_READONLY.
 * @param[out] phHeap      Receives the new heap handle. Not NULL.
 *                         Set to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phHeap is NULL, ulElemSize is
 *                                  zero, or pOpts lacks pCompare.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HeapCreate(ULONG ulElemSize, PCCLOPTS pOpts,
                           PHHEAP phHeap);

/**
 * @brief Destroy a heap.
 *
 * Releases the underlying vector. If pDestroyValue was supplied at
 * creation, it is called on every remaining element before the
 * vector storage is released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hHeap  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY HeapDestroy(HHEAP hHeap);

/* ==================================================================
 * Push, pop, peek
 * ================================================================== */

/**
 * @brief Push a copy of an element onto the heap.
 *
 * The element is appended at the end of the underlying storage and
 * then sifted up to restore the min-heap invariant.
 *
 * @param[in] hHeap  Handle. Not NULLHANDLE.
 * @param[in] pElem  Pointer to the element to copy. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hHeap or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HeapPush(HHEAP hHeap, PCVOID pElem);

/**
 * @brief Pop the root element into a caller-supplied buffer.
 *
 * If pBuf is NULL and ulSize is 0, only *pulUsed receives the
 * element size (size-query, no removal, no destroy). Otherwise the
 * root is copied into pBuf, the root slot is filled with the last
 * element, the last element is removed via VectorRemoveLast (which
 * fires pDestroyValue if configured) and the root is sifted down
 * to restore the min-heap invariant.
 *
 * Size-query convention:
 *   - pBuf == NULL && ulSize == 0: only *pulUsed = element size.
 *   - ulSize >= element size: copy + remove + sift down;
 *     *pulUsed = element size.
 *   - ulSize < element size: ERROR_BUFFER_OVERFLOW; *pulUsed =
 *     element size; the heap is unchanged.
 *
 * @param[in]  hHeap   Handle. Not NULLHANDLE.
 * @param[out] pBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize  Size of pBuf in bytes.
 * @param[out] pulUsed Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hHeap is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The heap is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HeapPop(HHEAP hHeap, PVOID pBuf, ULONG ulSize,
                        PULONG pulUsed);

/**
 * @brief Return a borrowed pointer to the root element.
 *
 * The root is the element that pCompare orders before all others.
 * The pointer is valid until the next mutation (Push, Pop, Clear,
 * Destroy). It is aligned to max_align_t (storage owned by the
 * underlying vector).
 *
 * @param[in]  hHeap   Handle. Not NULLHANDLE.
 * @param[out] ppElem  Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hHeap or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The heap is empty.
 */
APIRET APIENTRY HeapTop(HHEAP hHeap, PVOID *ppElem);

/* ==================================================================
 * Metadata and bulk operations
 * ================================================================== */

/**
 * @brief Return the number of elements in the heap.
 *
 * @param[in]  hHeap     Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HeapGetCount(HHEAP hHeap, PULONG pulCount);

/**
 * @brief Remove every element from the heap.
 *
 * If pDestroyValue was supplied at creation, it is called on each
 * removed element. After this call, the heap is empty. The
 * underlying capacity is kept.
 *
 * @param[in] hHeap  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HeapClear(HHEAP hHeap);

#ifdef __cplusplus
}
#endif

#endif /* CCL_HEAP_H */
