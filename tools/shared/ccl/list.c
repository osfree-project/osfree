/*!
 *
 * @file list.c
 *
 * @brief Implementation of the list container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * The list is an elemt-model container: each LINKED_NODE holds a
 * caller-provided PVOID in the elemt field, plus the pointer size
 * in the size field (used only for validation). The tag field is
 * always 0 and unused here.
 *
 * Nodes are allocated through LinkedNodeAlloc, which in turn uses
 * (*CclMalloc); the node layout is shared with the legacy dlist and
 * all_dlist modules. See linked_list_private.h for the layout
 * contract.
 *
 * The internal control block carries a generation counter, updated
 * on every structural mutation (add, insert, remove, clear). The
 * generation will be consumed by the HITERATOR subsystem in
 * Phase 3; in Phase 1 nothing reads it, but keeping it up to date
 * avoids a layout change later.
 *
 * A MANAGED_CTX field is also present, empty for Phase 1. It will
 * be populated by the managed-iterator framework in Phase 3.5. See
 * the note above the stub definition below.
 */

#include <string.h>
#include "list.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"
#include "linked_list_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_LIST_MAGIC
 * @brief Magic value identifying a valid list control block.
 *
 * The literal bytes are "DLST". The same magic is used by the
 * legacy dlist.c module; the two control blocks differ in layout
 * but never coexist in the same process in a way that could be
 * confused, because both carry a private struct behind the same
 * public HANDLE and are only ever passed to their own accessors.
 */
#define CCL_LIST_MAGIC 0x444C5354UL  /* "DLST" */

/*!
 * @struct _MANAGED_CTX
 * @brief Stub for the Phase 3.5 managed-iterator context.
 *
 * Phase 1: declared empty, embedded in LISTCTL so that the control
 * block layout does not change when Phase 3.5 lands. No code in
 * this file touches the managed field. When the managed-iterator
 * framework (iterator_managed_private.h / iterator_managed.c) is
 * introduced, the definition here will be replaced by an #include
 * of that header and the layout will stay compatible.
 */
typedef struct _MANAGED_CTX {
    HANDLE *pIterators;     /*!< Open managed iterators. Reserved.  */
    ULONG   ulCount;        /*!< Number of iterators. Reserved.     */
    ULONG   ulCapacity;     /*!< Capacity of pIterators. Reserved.  */
} MANAGED_CTX;

/*!
 * @struct _LISTCTL
 * @brief Control block of an open list.
 *
 * pCurrent is deliberately absent: SOM lists (TDeque, CosQueue,
 * CosDeque, CosSequence, CosEqualitySequence) have no cursor. The
 * legacy Dlist module keeps its own pCurrent in DLISTCTL; the two
 * control blocks never mix.
 */
typedef struct _LISTCTL {
    unsigned long  ulMagic;        /*!< CCL_LIST_MAGIC.              */
    PLINKED_NODE   pHead;          /*!< First node, or NULL.         */
    PLINKED_NODE   pTail;          /*!< Last node, or NULL.          */
    unsigned long  ulCount;        /*!< Number of nodes.             */
    unsigned long  ulGeneration;   /*!< Phase 3 iterator generation. */
    unsigned long  ulElemSize;     /*!< Pointer size, for validation.*/
    CCL_DESTROY_FN pDestroyValue;  /*!< Optional destroy callback.   */
    CCL_COMPARE_FN pCompare;       /*!< Optional compare callback.   */
    MANAGED_CTX    managed;        /*!< Phase 3.5 stub. Reserved.    */
} LISTCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a list handle.
 *
 * @param[in] hList  List handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hList is NULL, or its magic value does not match.
 */
static LISTCTL *get_ctl(HLIST hList) {
    return (LISTCTL *)ccl_validate(hList, CCL_LIST_MAGIC);
}

/*!
 * @brief Return the node at the given index.
 *
 * Walks from whichever end is closer, so that the average walk is
 * O(count / 4). No bounds check: the caller must ensure that
 * ulIndex is below ulCount.
 *
 * @param[in] pCtl     Control block. Not NULL.
 * @param[in] ulIndex  Zero-based index.
 *
 * @return The node at ulIndex.
 */
static PLINKED_NODE node_at(LISTCTL *pCtl, unsigned long ulIndex) {
    PLINKED_NODE pNode;
    unsigned long i;

    if (ulIndex < pCtl->ulCount / 2) {
        pNode = pCtl->pHead;
        for (i = 0; i < ulIndex; i++) pNode = pNode->next;
    } else {
        pNode = pCtl->pTail;
        for (i = pCtl->ulCount - 1; i > ulIndex; i--) pNode = pNode->prev;
    }
    return pNode;
}

/*!
 * @brief Call pDestroyValue on the stored bytes of one node.
 *
 * The callback receives a pointer to the elemt field itself (the
 * stored bytes), not the pointer stored there. This matches the
 * ccl convention for destroy callbacks; see ccl_conventions.md §6.
 *
 * No-op when pDestroyValue is NULL.
 *
 * @param[in] pCtl   Control block. Not NULL.
 * @param[in] pNode  Node whose elemt is being released. Not NULL.
 */
static void destroy_elemt(LISTCTL *pCtl, PLINKED_NODE pNode) {
    if (pCtl->pDestroyValue) pCtl->pDestroyValue(&pNode->elemt);
}

/*!
 * @brief Release one node.
 *
 * Calls destroy_elemt first (if pDestroyValue is set), then releases
 * the node itself through LinkedNodeFree. The node must already be
 * unlinked from any chain; this helper does not touch next/prev of
 * neighbours.
 *
 * @param[in] pCtl   Control block. Not NULL.
 * @param[in] pNode  Node to release. Not NULL.
 */
static void free_node(LISTCTL *pCtl, PLINKED_NODE pNode) {
    destroy_elemt(pCtl, pNode);
    LinkedNodeFree(pNode);
}

/*!
 * @brief Unlink a node from the list and release it.
 *
 * Updates head, tail, count and generation, then calls free_node.
 *
 * @param[in] pCtl   Control block. Not NULL.
 * @param[in] pNode  Node to remove. Not NULL and already in the list.
 */
static void unlink_and_free(LISTCTL *pCtl, PLINKED_NODE pNode) {
    if (pNode == pCtl->pHead) pCtl->pHead = pNode->next;
    if (pNode == pCtl->pTail) pCtl->pTail = pNode->prev;
    LinkedNodeUnlink(pNode);
    pCtl->ulCount--;
    pCtl->ulGeneration++;
    free_node(pCtl, pNode);
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new list.
 *
 * @param[in]  ulElemSize  Size of one pointer in bytes.
 * @param[in]  pOpts       Option block. May be NULL.
 * @param[out] phList      Receives the new list handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phList is NULL, ulElemSize is
 *                                  zero, or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY ListCreate(ULONG ulElemSize, PCCLOPTS pOpts, PHLIST phList) {
    LISTCTL *pCtl;
    APIRET rc;

    if (!phList) return ERROR_INVALID_PARAMETER;
    if (ulElemSize == 0) return ERROR_INVALID_PARAMETER;
    *phList = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_COMPARE
                         | CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         0);
    if (rc != NO_ERROR) return rc;

    pCtl = (LISTCTL *)(*CclCalloc)(1, sizeof(LISTCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->ulMagic    = CCL_LIST_MAGIC;
    pCtl->ulElemSize = (unsigned long)ulElemSize;
    if (pOpts) {
        pCtl->pDestroyValue = pOpts->pDestroyValue;
        pCtl->pCompare      = pOpts->pCompare;
    }

    *phList = (HLIST)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a list.
 *
 * @param[in] hList  List handle. NULL is accepted and treated as
 *                   success.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success (including NULL handle).
 * @retval ERROR_INVALID_HANDLE   The handle is not a valid list.
 */
APIRET APIENTRY ListDestroy(HLIST hList) {
    LISTCTL *pCtl;
    PLINKED_NODE pNode, pNext;

    if (hList == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    pNode = pCtl->pHead;
    while (pNode) {
        pNext = pNode->next;
        free_node(pCtl, pNode);
        pNode = pNext;
    }

    pCtl->ulMagic = 0;
    (*CclFree)(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Adding elements
 * ================================================================== */

/*!
 * @brief Append a pointer at the end of the list.
 *
 * @param[in] hList  List handle.
 * @param[in] pElem  Pointer to store. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY ListAdd(HLIST hList, PVOID pElem) {
    LISTCTL *pCtl;
    PLINKED_NODE pNode;

    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    pNode = LinkedNodeAlloc(pElem, pCtl->ulElemSize, 0);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    if (pCtl->pTail) {
        LinkedNodeInsertAfter(pCtl->pTail, pNode);
    } else {
        pCtl->pHead = pNode;
    }
    pCtl->pTail = pNode;
    pCtl->ulCount++;
    pCtl->ulGeneration++;
    return NO_ERROR;
}

/*!
 * @brief Insert a pointer at the given index.
 *
 * @param[in] hList    List handle.
 * @param[in] ulIndex  Position. Must be <= current count.
 * @param[in] pElem    Pointer to store. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex > current count.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY ListInsertAt(HLIST hList, ULONG ulIndex, PVOID pElem) {
    LISTCTL *pCtl;
    PLINKED_NODE pNode;

    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex > pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    pNode = LinkedNodeAlloc(pElem, pCtl->ulElemSize, 0);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    if (pCtl->ulCount == 0) {
        pCtl->pHead = pNode;
        pCtl->pTail = pNode;
    } else if ((unsigned long)ulIndex == 0) {
        LinkedNodeInsertBefore(pCtl->pHead, pNode);
        pCtl->pHead = pNode;
    } else if ((unsigned long)ulIndex == pCtl->ulCount) {
        LinkedNodeInsertAfter(pCtl->pTail, pNode);
        pCtl->pTail = pNode;
    } else {
        PLINKED_NODE pRef = node_at(pCtl, (unsigned long)ulIndex);
        LinkedNodeInsertBefore(pRef, pNode);
    }
    pCtl->ulCount++;
    pCtl->ulGeneration++;
    return NO_ERROR;
}

/* ==================================================================
 * Removing elements
 * ================================================================== */

/*!
 * @brief Remove the element at ulIndex.
 *
 * @param[in] hList    List handle.
 * @param[in] ulIndex  Position. Must be < current count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY ListRemoveAt(HLIST hList, ULONG ulIndex) {
    LISTCTL *pCtl;

    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    unlink_and_free(pCtl, node_at(pCtl, (unsigned long)ulIndex));
    return NO_ERROR;
}

/*!
 * @brief Remove every element from the list.
 *
 * @param[in] hList  List handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when the list is
 *                                  already empty.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 */
APIRET APIENTRY ListClear(HLIST hList) {
    LISTCTL *pCtl;
    PLINKED_NODE pNode, pNext;

    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    pNode = pCtl->pHead;
    while (pNode) {
        pNext = pNode->next;
        free_node(pCtl, pNode);
        pNode = pNext;
    }
    pCtl->pHead = NULL;
    pCtl->pTail = NULL;
    pCtl->ulCount = 0;
    pCtl->ulGeneration++;
    return NO_ERROR;
}

/* ==================================================================
 * Access
 * ================================================================== */

/*!
 * @brief Return the number of elements currently stored.
 *
 * @param[in]  hList     List handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 */
APIRET APIENTRY ListGetCount(HLIST hList, PULONG pulCount) {
    LISTCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pulCount = (ULONG)pCtl->ulCount;
    return NO_ERROR;
}

/*!
 * @brief Return a borrowed pointer to the element at ulIndex.
 *
 * @param[in]  hList    List handle.
 * @param[in]  ulIndex  Position. Must be < current count.
 * @param[out] ppElem   Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY ListGetAt(HLIST hList, ULONG ulIndex, PVOID *ppElem) {
    LISTCTL *pCtl;
    PLINKED_NODE pNode;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    pNode = node_at(pCtl, (unsigned long)ulIndex);
    *ppElem = pNode->elemt;
    return NO_ERROR;
}

/*!
 * @brief Test whether the list contains a given pointer.
 *
 * If pCompare is set, it is called as pCompare(&stored, &pElem, NULL)
 * for each stored element. Otherwise pointer identity is used.
 *
 * @param[in]  hList    List handle.
 * @param[in]  pElem    Pointer to look for. May be NULL.
 * @param[out] pfFound  Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hList or pfFound is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid list.
 */
APIRET APIENTRY ListContains(HLIST hList, PVOID pElem, PBOOL pfFound) {
    LISTCTL *pCtl;
    PLINKED_NODE pNode;

    if (!pfFound) return ERROR_INVALID_PARAMETER;
    *pfFound = FALSE;
    pCtl = get_ctl(hList);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    for (pNode = pCtl->pHead; pNode; pNode = pNode->next) {
        if (pCtl->pCompare) {
            if (pCtl->pCompare(&pNode->elemt, &pElem, NULL) == 0) {
                *pfFound = TRUE;
                return NO_ERROR;
            }
        } else if (pNode->elemt == pElem) {
            *pfFound = TRUE;
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}
