/* dlist.h - dynamic list and stack containers (C89) */
#ifndef DLIST_H
#define DLIST_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Handles
 * ================================================================== */

/**
 * @typedef HSTACK
 * @brief Handle to a dynamic stack.
 */
typedef HANDLE HSTACK;

/**
 * @typedef PHSTACK
 * @brief Pointer to a stack handle.
 */
typedef HSTACK *PHSTACK;

/**
 * @typedef HLIST
 * @brief Handle to a dynamic list.
 */
typedef HANDLE HLIST;

/**
 * @typedef PHLIST
 * @brief Pointer to a list handle.
 */
typedef HLIST *PHLIST;

/* ==================================================================
 * Flags
 * ================================================================== */

/**
 * @def DLIST_OWN_ELEMENTS
 * @brief On destroy, also free the element pointers.
 *
 * When set at create time, StackDestroy / ListDestroy calls free()
 * on each stored element pointer. Without this flag, only internal
 * nodes are released; element ownership remains with the caller.
 */
#define DLIST_OWN_ELEMENTS  0x0001

/* ==================================================================
 * Stack API
 *
 * A stack is a LIFO container. Elements are stored as PVOID pointers.
 * The container owns only the node that carries the pointer;
 * ownership of the element is controlled by DLIST_OWN_ELEMENTS.
 * ================================================================== */

/**
 * @brief Create an empty stack.
 *
 * @param[in]  flFlags  Combination of DLIST_* flags. Zero is valid.
 * @param[out] phStack  Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phStack is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY StackCreate(ULONG flFlags, PHSTACK phStack);

/**
 * @brief Destroy a stack.
 *
 * Releases all nodes. If DLIST_OWN_ELEMENTS was set at create time,
 * each stored element pointer is passed to free() first. Passing
 * NULLHANDLE is a no-op.
 *
 * @param[in] hStack  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 */
APIRET APIENTRY StackDestroy(HSTACK hStack);

/**
 * @brief Push an element onto the top of the stack.
 *
 * @param[in] hStack  Handle. Not NULLHANDLE.
 * @param[in] pElem   Element pointer. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY StackPush(HSTACK hStack, PVOID pElem);

/**
 * @brief Pop the top element off the stack.
 *
 * The node is released. Ownership of the element pointer is returned
 * to the caller. On an empty stack, @p *ppElem is set to NULL and
 * ERROR_NO_DATA is returned.
 *
 * @param[in]  hStack  Handle. Not NULLHANDLE.
 * @param[out] ppElem  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_DATA            Stack is empty.
 */
APIRET APIENTRY StackPop(HSTACK hStack, PPVOID ppElem);

/**
 * @brief Retrieve the top element without removing it.
 *
 * @param[in]  hStack  Handle. Not NULLHANDLE.
 * @param[out] ppElem  Receiver. Not NULL. Set to NULL on empty stack.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_DATA            Stack is empty.
 */
APIRET APIENTRY StackTop(HSTACK hStack, PPVOID ppElem);

/**
 * @brief Remove an element from the middle of the stack.
 *
 * The top of the stack is not affected. The node that carried the
 * element is released. If DLIST_OWN_ELEMENTS was set at create time,
 * the element pointer is passed to free() as well.
 *
 * @param[in] hStack  Handle. Not NULLHANDLE.
 * @param[in] pElem   Element pointer to remove.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Element not found in the stack.
 */
APIRET APIENTRY StackRemove(HSTACK hStack, PVOID pElem);

/**
 * @brief Test whether the stack is empty.
 *
 * @param[in]  hStack   Handle. Not NULLHANDLE.
 * @param[out] pfEmpty  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or pfEmpty is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY StackIsEmpty(HSTACK hStack, PBOOL pfEmpty);

/* ==================================================================
 * List API
 *
 * A list is a doubly-linked sequence with a cursor. Elements are
 * stored as PVOID pointers. The cursor lives inside the container;
 * insertion, removal and traversal all operate relative to the
 * current position.
 * ================================================================== */

/**
 * @brief Create an empty list.
 *
 * @param[in]  flFlags Combination of DLIST_* flags. Zero is valid.
 * @param[out] phList  Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ListCreate(ULONG flFlags, PHLIST phList);

/**
 * @brief Destroy a list.
 *
 * Releases all nodes. If DLIST_OWN_ELEMENTS was set at create time,
 * each stored element pointer is passed to free() first. Passing
 * NULLHANDLE is a no-op.
 *
 * @param[in] hList  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 */
APIRET APIENTRY ListDestroy(HLIST hList);

/**
 * @brief Retrieve the element at the current position.
 *
 * @param[in]  hList   Handle. Not NULLHANDLE.
 * @param[out] ppElem  Receiver. Not NULL. Set to NULL on empty list.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_DATA            List is empty.
 */
APIRET APIENTRY ListGet(HLIST hList, PPVOID ppElem);

/**
 * @brief Insert an element after the current position.
 *
 * If the list is empty, the new element becomes the only element and
 * the cursor is placed on it.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 * @param[in] pElem  Element pointer. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ListInsert(HLIST hList, PVOID pElem);

/**
 * @brief Remove the current element from the list.
 *
 * The node is released. If DLIST_OWN_ELEMENTS was set at create time,
 * the element pointer is passed to free() as well. The cursor moves
 * to the next element if one exists, otherwise to the previous one;
 * on an empty list the cursor becomes unset.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 * @retval ERROR_NO_DATA          List is empty.
 */
APIRET APIENTRY ListRemove(HLIST hList);

/**
 * @brief Append an element at the end of the list.
 *
 * The cursor is not changed.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 * @param[in] pElem  Element pointer. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ListAdd(HLIST hList, PVOID pElem);

/**
 * @brief Move the cursor to the first element.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 * @retval ERROR_NO_DATA          List is empty.
 */
APIRET APIENTRY ListSetStart(HLIST hList);

/**
 * @brief Move the cursor to the last element.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 * @retval ERROR_NO_DATA          List is empty.
 */
APIRET APIENTRY ListSetEnd(HLIST hList);

/**
 * @brief Move the cursor to the previous element.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 * @retval ERROR_NO_DATA          List is empty.
 * @retval ERROR_NO_MORE_ITEMS    Already at the first element.
 */
APIRET APIENTRY ListPrev(HLIST hList);

/**
 * @brief Move the cursor to the next element.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 * @retval ERROR_NO_DATA          List is empty.
 * @retval ERROR_NO_MORE_ITEMS    Already at the last element.
 */
APIRET APIENTRY ListNext(HLIST hList);

/**
 * @brief Test whether the list is empty.
 *
 * @param[in]  hList    Handle. Not NULLHANDLE.
 * @param[out] pfEmpty  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList or pfEmpty is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ListIsEmpty(HLIST hList, PBOOL pfEmpty);

#ifdef __cplusplus
}
#endif

#endif /* DLIST_H */
