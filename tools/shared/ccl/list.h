/* list.h - doubly linked list of pointers (SOM object model).
 * Part of the ccl container library. */
#ifndef CCL_LIST_H
#define CCL_LIST_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_base.h"

/**
 * @file list.h
 * @brief Doubly linked list of pointers.
 *
 * A list stores pointers (typically SOMObject *) in a doubly linked
 * chain of LINKED_NODE. The node layout is shared with the legacy
 * IBM Dlist and all_dlist modules (see linked_list_private.h):
 * elemt points to the caller's data, size is the pointer size used
 * for validation, tag is unused by list.c.
 *
 * @par Why elemt and not inline payload
 * SOM objects already live in the SOM heap. The list does not
 * allocate per-element storage; it only allocates the LINKED_NODE
 * that points to the object. This matches the SOM collection model
 * (TDeque, CosQueue, CosDeque, CosSequence, CosEqualitySequence)
 * and lets the same node layer serve both ccl and the legacy
 * Dlist/all_dlist modules.
 *
 * @par Order and indexing
 * Elements keep insertion order. Add and InsertAt are O(1) when
 * ulIndex is 0 or equals count; arbitrary-index insert/remove is
 * O(n). GetAt is O(n); ListGetCount is O(1).
 *
 * @par Ownership
 * The list owns its nodes. The pointed-to objects are the caller's
 * responsibility by default. If pDestroyValue is passed at creation,
 * it is called on the stored bytes (the elemt field) whenever an
 * element is removed (ListRemoveAt, ListClear) or left in the list
 * at ListDestroy. The typical SOM wrapper leaves pDestroyValue NULL,
 * because the SOM object is owned by SOM.
 *
 * @par Iteration
 * Phase 1 provides index-based access only (GetAt, ListGetCount).
 * In Phase 3 the list will additionally be exposed through the
 * unified HITERATOR interface (iterator.h). The internal control
 * block already carries a generation counter and a MANAGED_CTX for
 * that.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if a
 * list is shared between threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef HLIST
 * @brief Handle to a list.
 */
typedef HANDLE HLIST;

/**
 * @typedef PHLIST
 * @brief Pointer to a list handle.
 */
typedef HLIST *PHLIST;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Create an empty list.
 *
 * @param[in]  ulElemSize  Size of one pointer in bytes. Typically
 *                         sizeof(SOMObject *) (4 on 32-bit targets,
 *                         8 on 64-bit). Must be > 0. Stored in every
 *                         LINKED_NODE as the size field for
 *                         validation.
 * @param[in]  pOpts       Option block. May be NULL. The only
 *                         allowed fields are pCompare (used by
 *                         ListContains) and pDestroyValue, plus the
 *                         flags CCL_FLAG_DEFAULT_MANAGED /
 *                         CCL_FLAG_DEFAULT_READONLY.
 * @param[out] phList      Receives the new list handle. Not NULL.
 *                         Set to NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL, ulElemSize is
 *                                  zero, or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY ListCreate(ULONG ulElemSize, PCCLOPTS pOpts,
                           PHLIST phList);

/**
 * @brief Destroy a list.
 *
 * Releases every node. If pDestroyValue was supplied at creation,
 * it is called on the stored bytes (the elemt field) of every
 * remaining element before the nodes are released.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hList  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ListDestroy(HLIST hList);

/* ==================================================================
 * Adding elements
 * ================================================================== */

/**
 * @brief Append a pointer at the end of the list.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 * @param[in] pElem  Pointer to store. May be NULL (NULL is a valid
 *                   element value in a list of pointers).
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY ListAdd(HLIST hList, PVOID pElem);

/**
 * @brief Insert a pointer at the given index.
 *
 * Index 0 means "before the current first element"; index == count
 * means "append at the end". Elements from ulIndex onwards are
 * shifted one position to the right.
 *
 * @param[in] hList    Handle. Not NULLHANDLE.
 * @param[in] ulIndex  Position. Must be <= current count.
 * @param[in] pElem    Pointer to store. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex > current count.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY ListInsertAt(HLIST hList, ULONG ulIndex, PVOID pElem);

/* ==================================================================
 * Removing elements
 * ================================================================== */

/**
 * @brief Remove the element at ulIndex.
 *
 * If pDestroyValue was supplied at creation, it is called on the
 * removed element's stored bytes before the node is released.
 *
 * @param[in] hList    Handle. Not NULLHANDLE.
 * @param[in] ulIndex  Position. Must be < current count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY ListRemoveAt(HLIST hList, ULONG ulIndex);

/**
 * @brief Remove every element from the list.
 *
 * If pDestroyValue was supplied at creation, it is called on each
 * removed element.
 *
 * @param[in] hList  Handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when the list is
 *                                  already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ListClear(HLIST hList);

/* ==================================================================
 * Access
 * ================================================================== */

/**
 * @brief Return the number of elements currently stored.
 *
 * @param[in]  hList     Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ListGetCount(HLIST hList, PULONG pulCount);

/**
 * @brief Return a borrowed pointer to the element at ulIndex.
 *
 * This is exactly the value that was passed to ListAdd /
 * ListInsertAt. No copying is performed. The pointer stays valid
 * while the element remains in the list; removing the element
 * (ListRemoveAt, ListClear, ListDestroy) invalidates it, and so
 * does any reallocation-free mutation that removes earlier
 * elements? No — nodes themselves are not moved; only the index
 * meaning changes. The value pointed to (the SOM object) is never
 * touched by list.c.
 *
 * @param[in]  hList    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Position. Must be < current count.
 * @param[out] ppElem   Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY ListGetAt(HLIST hList, ULONG ulIndex, PVOID *ppElem);

/**
 * @brief Test whether the list contains a given pointer.
 *
 * If pCompare was supplied at creation, it is used to compare the
 * stored bytes of each element against pElem. Otherwise the
 * comparison is pointer identity: pElem is found iff it equals one
 * of the stored pointers.
 *
 * @param[in]  hList    Handle. Not NULLHANDLE.
 * @param[in]  pElem    Pointer to look for. May be NULL.
 * @param[out] pfFound  Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList or pfFound is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ListContains(HLIST hList, PVOID pElem, PBOOL pfFound);

#ifdef __cplusplus
}
#endif

#endif /* CCL_LIST_H */
