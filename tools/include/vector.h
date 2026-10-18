/* vector.h - dynamic array of fixed-size elements.
 * Part of the ccl container library. */
#ifndef CCL_VECTOR_H
#define CCL_VECTOR_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file vector.h
 * @brief Dynamic array of fixed-size elements.
 *
 * A vector stores elements by value. The size of one element is set
 * at creation and never changes. Elements are copied into the
 * vector on insert and read into a caller-supplied buffer on query,
 * unless a borrowed pointer is requested via VectorGetRef.
 *
 * @par Order and indexing
 * Elements keep the order in which they were added. Access by
 * position is O(1). Appending at the end is O(1) amortized.
 *
 * @par Ownership
 * The vector owns only the storage for its elements. If a
 * pDestroyValue callback is passed in the option block, the vector
 * calls it on the stored bytes when an element is overwritten
 * (VectorSet) or removed (VectorRemove, VectorRemoveLast, and at
 * VectorDestroy for every remaining element). Callbacks are not
 * reentrant: they must not modify the vector.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * vector is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HVECTOR
 * @brief Handle to a vector.
 */
typedef HANDLE HVECTOR;

/**
 * @typedef PHVECTOR
 * @brief Pointer to a vector handle.
 */
typedef HVECTOR *PHVECTOR;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty vector.
 *
 * @param[in]  ulElemSize     Size of one element in bytes. Must be
 *                            greater than 0.
 * @param[in]  ulInitialSize  Initial capacity. 0 means lazy
 *                            allocation (the storage array is
 *                            allocated on the first VectorAdd).
 * @param[in]  pOpts          Option block. May be NULL. The only
 *                            allowed callbacks are pDestroyValue
 *                            and the flags
 *                            CCL_FLAG_DEFAULT_MANAGED /
 *                            CCL_FLAG_DEFAULT_READONLY.
 * @param[out] phVector       Receives the new vector handle. Not
 *                            NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phVector is NULL, ulElemSize is
 *                                  zero, or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorCreate(ULONG ulElemSize, ULONG ulInitialSize,
                             PCCLOPTS pOpts, PHVECTOR phVector);

/**
 * @brief Destroy a vector.
 *
 * Releases the storage of the vector itself. If pDestroyValue was
 * supplied at creation, it is called on every remaining element
 * before the storage is released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hVector  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY VectorDestroy(HVECTOR hVector);

/* ==================================================================
 * Adding elements
 * ================================================================== */

/**
 * @brief Append a copy of an element at the end.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 * @param[in] pElem    Pointer to the element to copy. Not NULL.
 *                     Must not point inside the same vector.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorAdd(HVECTOR hVector, PCVOID pElem);

/**
 * @brief Insert a copy of an element at ulIndex.
 *
 * Elements from ulIndex onwards are shifted up by one slot. ulIndex
 * may be equal to the current count (append); it must not exceed it.
 * VectorInsert does NOT call pDestroyValue: no existing element is
 * being removed, only shifted.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 * @param[in] ulIndex  Position. Must be <= current count.
 * @param[in] pElem    Pointer to the element to copy. Not NULL.
 *                     Must not point inside the same vector.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex > current count.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorInsert(HVECTOR hVector, ULONG ulIndex, PCVOID pElem);

/* ==================================================================
 * Removing elements
 * ================================================================== */

/**
 * @brief Remove the element at ulIndex.
 *
 * Elements after ulIndex are shifted down by one slot. If
 * pDestroyValue was supplied at creation, it is called on the
 * removed element before the shift.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 * @param[in] ulIndex  Position. Must be < current count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY VectorRemove(HVECTOR hVector, ULONG ulIndex);

/**
 * @brief Remove the last element.
 *
 * If pDestroyValue was supplied at creation, it is called on the
 * removed element.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The vector is empty.
 */
APIRET APIENTRY VectorRemoveLast(HVECTOR hVector);

/* ==================================================================
 * Replacing elements
 * ================================================================== */

/**
 * @brief Overwrite the element at ulIndex with a copy.
 *
 * If pDestroyValue was supplied at creation, it is called on the
 * old element before the copy is made.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 * @param[in] ulIndex  Position. Must be < current count.
 * @param[in] pElem    Pointer to the new element. Not NULL.
 *                     Must not point inside the same vector.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY VectorSet(HVECTOR hVector, ULONG ulIndex, PCVOID pElem);

/**
 * @brief Overwrite the element at ulIndex without calling
 *        pDestroyValue.
 *
 * Raw write. Used by heap.c during sift operations, where the
 * elements are only being moved around, not destroyed. Callers that
 * own a pDestroyValue callback and want it not to fire use this
 * variant. Same index checks as VectorSet.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 * @param[in] ulIndex  Position. Must be < current count.
 * @param[in] pElem    Pointer to the new element. Not NULL.
 *                     Must not point inside the same vector.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY VectorSetRef(HVECTOR hVector, ULONG ulIndex, PCVOID pElem);

/* ==================================================================
 * Access
 * ================================================================== */

/**
 * @brief Number of elements currently stored.
 *
 * @param[in]  hVector   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY VectorGetCount(HVECTOR hVector, PULONG pulCount);

/**
 * @brief Copy one element into a caller-supplied buffer.
 *
 * Size-query convention:
 *   - pBuf == NULL, ulSize == 0: only *pulUsed (element size) is
 *     written, no buffer touched.
 *   - ulSize large enough: element is copied; *pulUsed is the
 *     element size.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size.
 *
 * @param[in]  hVector  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pBuf     Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pBuf (without
 *                                  size-query) is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY VectorGetItem(HVECTOR hVector, ULONG ulIndex,
                              PVOID pBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Return a borrowed pointer to the element at ulIndex.
 *
 * The returned pointer is a byte pointer into the vector's internal
 * storage. It is aligned to max_align_t (because the storage array
 * is allocated through (*CclMalloc) / (*CclRealloc)). If ulElemSize
 * is a multiple of alignof(T), then (pbData + i * ulElemSize) is
 * aligned for T, and the pointer may be dereferenced as T*.
 * Otherwise it may be misaligned for T; use VectorGetItem (memcpy
 * into a caller buffer) instead.
 *
 * The pointer stays valid until the next mutation of the vector
 * (VectorAdd, VectorInsert, VectorRemove, VectorRemoveLast,
 * VectorSet, VectorSetRef, VectorReserve with growth).
 *
 * @param[in]  hVector  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] ppElem   Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY VectorGetRef(HVECTOR hVector, ULONG ulIndex, PVOID *ppElem);

/* ==================================================================
 * Capacity
 * ================================================================== */

/**
 * @brief Ensure the vector can hold at least ulNewCapacity elements.
 *
 * Grows the storage if needed, by doubling from the current capacity
 * (or from 8, if the vector is still empty). Guarantees
 * capacity >= ulNewCapacity. Never shrinks. Passing a value below
 * the current capacity is a no-op.
 *
 * @param[in] hVector        Handle. Not NULLHANDLE.
 * @param[in] ulNewCapacity  Minimum capacity.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorReserve(HVECTOR hVector, ULONG ulNewCapacity);

/**
 * @brief Ensure the vector has exactly ulNewCapacity slots.
 *
 * Unlike VectorReserve, if growth is needed, the new capacity is
 * exactly ulNewCapacity (rounded up to the current capacity if
 * already larger). Intended for callers that need a specific size,
 * for example to reproduce a layout in somDuplicate-like code.
 *
 * @param[in] hVector        Handle. Not NULLHANDLE.
 * @param[in] ulNewCapacity  Exact capacity (or the current capacity
 *                           if already >= ulNewCapacity).
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorReserveExact(HVECTOR hVector, ULONG ulNewCapacity);

#ifdef __cplusplus
}
#endif

#endif /* CCL_VECTOR_H */
