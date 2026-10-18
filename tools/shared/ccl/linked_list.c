/*!
 *
 * @file linked_list.c
 *
 * @brief Shared node layout and primitives (C89).
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. The node layout is shared with
 * the IBM-style Dlist module and with the all_dlist module; see
 * linked_list_private.h for the layout contract.
 */

#include <stdlib.h>
#include "linked_list_private.h"
#include "ccl_alloc.h"

/*!
 * @brief Allocate a detached node.
 *
 * The elemt, next and prev fields are initialized to NULL. The size
 * and tag fields are set to the given values. The node is not linked
 * into any list.
 *
 * Allocation goes through (*CclMalloc). At startup CclMalloc is a
 * wrapper around malloc; SOM-init may replace it with a SOM-alloc
 * adapter, in which case all ccl nodes end up in the SOM runtime
 * allocator without a single source change here.
 *
 * @param[in] pElem  User data pointer to store in the node.
 * @param[in] size   Element size in bytes.
 * @param[in] tag    Element tag.
 *
 * @return A pointer to the new node, or NULL on allocation failure.
 *
 * @retval NULL  Allocation failed.
 */
PLINKED_NODE LinkedNodeAlloc(PVOID pElem, ULONG size, ULONG tag) {
    PLINKED_NODE pNode;

    pNode = (PLINKED_NODE)(*CclMalloc)(sizeof(LINKED_NODE));
    if (!pNode) return NULL;

    pNode->elemt = pElem;
    pNode->next  = NULL;
    pNode->prev  = NULL;
    pNode->size  = size;
    pNode->tag   = tag;
    return pNode;
}

/*!
 * @brief Release a node.
 *
 * Does not touch the user data pointer stored in the node. The
 * caller is responsible for releasing the user data if needed.
 *
 * Release goes through (*CclFree), the counterpart of the
 * (*CclMalloc) used in LinkedNodeAlloc. The block was allocated by
 * the CclMalloc hook active at allocation time; CclFree must point
 * to the matching hook at release time. In practice SOM-init
 * installs all four hooks once and does not change them afterwards.
 *
 * @param[in] pNode  Node to release.
 */
void LinkedNodeFree(PLINKED_NODE pNode) {
    (*CclFree)(pNode);
}

/* --------------------------------------------------------------------------
   Node-level primitives.

   These three functions do the linking and unlinking work that used
   to be duplicated as static helpers in dlist.c and as inline code
   in all_dlist.c. They operate only on next and prev; elemt, size
   and tag are left untouched. Bookkeeping of the container (head,
   tail, count, current pointer, generation) is the caller's
   responsibility: the primitives do not know about container
   layout.
 -------------------------------------------------------------------------- */

/*!
 * @brief Link a node immediately after a reference node.
 *
 * Adjusts pRef->next, pRef->next->prev (if any) and initializes
 * pNew->prev / pNew->next. Does not touch elemt, size or tag. Does
 * not update any container bookkeeping; the caller is responsible
 * for that.
 *
 * @param[in]     pRef  Reference node. Must not be NULL.
 * @param[in,out] pNew  Node to insert. Its prev and next fields are
 *                      overwritten.
 */
void LinkedNodeInsertAfter(PLINKED_NODE pRef, PLINKED_NODE pNew) {
    pNew->prev = pRef;
    pNew->next = pRef->next;
    if (pRef->next) pRef->next->prev = pNew;
    pRef->next = pNew;
}

/*!
 * @brief Link a node immediately before a reference node.
 *
 * Adjusts pRef->prev, pRef->prev->next (if any) and initializes
 * pNew->prev / pNew->next. Does not touch elemt, size or tag. Does
 * not update any container bookkeeping; the caller is responsible
 * for that.
 *
 * @param[in]     pRef  Reference node. Must not be NULL.
 * @param[in,out] pNew  Node to insert. Its prev and next fields are
 *                      overwritten.
 */
void LinkedNodeInsertBefore(PLINKED_NODE pRef, PLINKED_NODE pNew) {
    pNew->next = pRef;
    pNew->prev = pRef->prev;
    if (pRef->prev) pRef->prev->next = pNew;
    pRef->prev = pNew;
}

/*!
 * @brief Unlink a node from its neighbours.
 *
 * Repairs the neighbours (prev->next and next->prev), then clears
 * pNode->prev and pNode->next so the node is no longer reachable
 * from either end. Does not touch elemt, size or tag. Does not
 * update any container bookkeeping; the caller is responsible for
 * that.
 *
 * The clearing of pNode->prev and pNode->next is deliberate: after
 * Unlink, a stale pointer to the unlinked node carries no back-link
 * into the container, which makes debugging use-after-unlink bugs
 * easier to spot.
 *
 * @param[in,out] pNode  Node to unlink. Must not be NULL.
 */
void LinkedNodeUnlink(PLINKED_NODE pNode) {
    if (pNode->prev) pNode->prev->next = pNode->next;
    if (pNode->next) pNode->next->prev = pNode->prev;
    pNode->prev = NULL;
    pNode->next = NULL;
}
