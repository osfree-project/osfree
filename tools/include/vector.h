/* vector.h - dynamic array of fixed-size elements.
 * Part of the ccl container library. */
#ifndef CCL_VECTOR_H
#define CCL_VECTOR_H

#include "os2types.h"
#include "os2err.h"

/**
 * @file vector.h
 * @brief Dynamic array of fixed-size elements.
 *
 * A vector stores elements by value. The size of one element is set
 * at creation and never changes. Elements are copied into the
 * vector on insert and read through a pointer to the stored slot.
 *
 * @par Order and indexing
 * Elements keep the order in which they were added. Access by
 * position is O(1). Appending at the end is O(1) amortized.
 *
 * @par Ownership
 * The vector owns only the storage for its elements. It does not
 * own anything the elements point to. Cleaning up inside an element
 * is the caller's responsibility; the caller does it before calling
 * VectorDestroy.
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

/**
 * @typedef VECTORCOMPAREFN
 * @brief Comparison callback.
 *
 * Follows the usual ordering contract: negative if the first element
 * is less than the second, zero if equal, positive if greater. Same
 * contract as the standard qsort comparator.
 */
typedef int (APIENTRY *VECTORCOMPAREFN)(PCVOID pElem1, PCVOID pElem2);

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty vector.
 *
 * @param[in]  ulElemSize  Size of one element in bytes. Must be > 0.
 * @param[out] phVector    Receiver. Not NULL. Set to NULLHANDLE on
 *                         error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phVector is NULL or ulElemSize
 *                                  is zero.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorCreate(ULONG ulElemSize, PHVECTOR phVector);

/**
 * @brief Destroy a vector.
 *
 * Releases the storage of the vector itself. It does not touch the
 * elements; the caller cleans up anything inside them before this
 * call.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hVector  Handle. May be NULLHANDLE.
 *
 * @return APIRET
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
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorAdd(HVECTOR hVector, PCVOID pElem);

/**
 * @brief Append an uninitialized slot and return a pointer to it.
 *
 * The element count is increased by one. The caller fills the slot.
 * Useful when an element is more convenient to fill in place than to
 * construct on the caller's stack.
 *
 * @param[in]  hVector    Handle. Not NULLHANDLE.
 * @param[out] ppNewElem  Receiver. Not NULL. Points to the new slot.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or ppNewElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorGrow(HVECTOR hVector, PPVOID ppNewElem);

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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY VectorGetCount(HVECTOR hVector, PULONG pulCount);

/**
 * @brief Return a pointer to an element inside the vector.
 *
 * The pointer refers to the vector's own storage. It stays valid
 * until the next call that adds to or grows the vector. The caller
 * must not free it.
 *
 * @param[in]  hVector  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] ppElem   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY VectorGetPtr(HVECTOR hVector, ULONG ulIndex, PPVOID ppElem);

/* ==================================================================
 * Sorting
 * ================================================================== */

/**
 * @brief Sort the vector in place.
 *
 * The comparator receives pointers to two stored elements. The sort
 * is stable.
 *
 * @param[in] hVector  Handle. Not NULLHANDLE.
 * @param[in] pfnCmp   Comparator. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pfnCmp is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY VectorSort(HVECTOR hVector, VECTORCOMPAREFN pfnCmp);

#ifdef __cplusplus
}
#endif

#endif /* CCL_VECTOR_H */
