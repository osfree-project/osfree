/* stack.h - LIFO stack of fixed-size elements.
 * Part of the ccl container library. */
#ifndef CCL_STACK_H
#define CCL_STACK_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file stack.h
 * @brief LIFO stack of fixed-size elements.
 *
 * A stack stores elements by value. The element size is fixed at
 * creation. Push copies the element into the underlying vector;
 * Pop copies it out into a caller-supplied buffer and removes it.
 * Top returns a borrowed pointer without removing.
 *
 * The stack is a thin profile over vector.h (see ccl_conventions.md
 * §12). All storage management (growth, copy) lives in vector; this
 * module only restricts the public API to LIFO operations and adds
 * a private control block carrying the vector handle and the
 * effective callback set.
 *
 * @par Order
 * LIFO: Pop returns the most recently pushed element that has not
 * yet been popped.
 *
 * @par Ownership
 * The stack owns only the storage for the element bytes (through
 * the underlying vector). If pDestroyValue is supplied, it is
 * called on the stored bytes when an element is popped, removed
 * via StackClear, or left in the stack at StackDestroy. The
 * callback is NOT called by StackTop (Top does not remove).
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * stack is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HSTACK
 * @brief Handle to a stack.
 */
typedef HANDLE HSTACK;

/**
 * @typedef PHSTACK
 * @brief Pointer to a stack handle.
 */
typedef HSTACK *PHSTACK;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty stack.
 *
 * @param[in]  ulElemSize     Size of one element in bytes. Must be
 *                            greater than 0.
 * @param[in]  ulInitialSize  Initial capacity. 0 means lazy
 *                            allocation (the underlying vector
 *                            allocates on the first StackPush).
 * @param[in]  pOpts          Option block. May be NULL. The only
 *                            allowed callback is pDestroyValue;
 *                            the only allowed flags are
 *                            CCL_FLAG_DEFAULT_MANAGED and
 *                            CCL_FLAG_DEFAULT_READONLY.
 * @param[out] phStack        Receives the new stack handle. Not
 *                            NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phStack is NULL, ulElemSize is
 *                                  zero, or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StackCreate(ULONG ulElemSize, ULONG ulInitialSize,
                            PCCLOPTS pOpts, PHSTACK phStack);

/**
 * @brief Destroy a stack.
 *
 * Releases the underlying vector. If pDestroyValue was supplied at
 * creation, it is called on every remaining element before the
 * vector storage is released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hStack  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY StackDestroy(HSTACK hStack);

/* ==================================================================
 * Push, pop, top
 * ================================================================== */

/**
 * @brief Push a copy of an element onto the top of the stack.
 *
 * @param[in] hStack  Handle. Not NULLHANDLE.
 * @param[in] pElem   Pointer to the element to copy. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StackPush(HSTACK hStack, PCVOID pElem);

/**
 * @brief Pop the top element into a caller-supplied buffer.
 *
 * If pBuf is NULL and ulSize is 0, only *pulUsed receives the
 * element size (size-query, no removal, no destroy). Otherwise the
 * top element is copied into pBuf and removed from the stack; if
 * pDestroyValue was supplied at creation, it is called on the
 * element after the copy and before removal.
 *
 * Size-query convention:
 *   - pBuf == NULL && ulSize == 0: only *pulUsed = element size.
 *   - ulSize >= element size: copy + remove; *pulUsed = element
 *     size.
 *   - ulSize < element size: ERROR_BUFFER_OVERFLOW; *pulUsed =
 *     element size; the stack is unchanged.
 *
 * @param[in]  hStack  Handle. Not NULLHANDLE.
 * @param[out] pBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize  Size of pBuf in bytes.
 * @param[out] pulUsed Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The stack is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY StackPop(HSTACK hStack, PVOID pBuf, ULONG ulSize,
                         PULONG pulUsed);

/**
 * @brief Return a borrowed pointer to the top element.
 *
 * The pointer is valid until the next mutation (Push, Pop, Clear or
 * Destroy). It is aligned to max_align_t (the storage is owned by
 * the underlying vector). If ulElemSize is a multiple of
 * alignof(T), the pointer may be dereferenced as T*; otherwise it
 * may be misaligned for T.
 *
 * @param[in]  hStack  Handle. Not NULLHANDLE.
 * @param[out] ppElem  Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The stack is empty.
 */
APIRET APIENTRY StackTop(HSTACK hStack, PVOID *ppElem);

/* ==================================================================
 * Metadata and bulk operations
 * ================================================================== */

/**
 * @brief Return the number of elements in the stack.
 *
 * @param[in]  hStack    Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY StackGetCount(HSTACK hStack, PULONG pulCount);

/**
 * @brief Remove every element from the stack.
 *
 * If pDestroyValue was supplied at creation, it is called on each
 * removed element. After this call, the stack is empty. The
 * underlying capacity is kept.
 *
 * @param[in] hStack  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY StackClear(HSTACK hStack);

#ifdef __cplusplus
}
#endif

#endif /* CCL_STACK_H */
