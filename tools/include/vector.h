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
 * vector on insert and read into a caller-supplied buffer on query.
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hVector or pBuf (without
 *                                  size-query) is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY VectorGetItem(HVECTOR hVector, ULONG ulIndex,
                              PVOID pBuf, ULONG ulSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* CCL_VECTOR_H */
