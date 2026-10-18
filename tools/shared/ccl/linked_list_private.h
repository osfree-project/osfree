/*!
 *
 * @file linked_list_private.h
 *
 * @brief Internal node layout shared by the ccl container modules.
 *
 * Private to the ccl library. Not a public header.
 *
 * The struct tag and the first three fields (elemt, next, prev) must
 * match the public stack_cmp type declared in all_dlist.h. Both
 * all_dlist.c and dlist.c perform typecasts between LINKED_NODE and
 * stack_cmp and rely on this layout.
 *
 * The size and tag fields are used only by dlist.c. They are ignored
 * by all_dlist.c.
 */

#ifndef CCL_LINKED_LIST_PRIVATE_H
#define CCL_LINKED_LIST_PRIVATE_H

#include "os2types.h"

/*!
 * @struct _stack_component
 * @brief One link node.
 *
 * Field order is part of the binary interface: consumers of the
 * public all_dlist.h see the first three fields under the name
 * stack_cmp. Do not reorder.
 */
typedef struct _stack_component {
    PVOID                    elemt;  /*!< User data pointer.         */
    struct _stack_component *next;   /*!< Next node, or NULL.        */
    struct _stack_component *prev;   /*!< Previous node, or NULL.    */
    ULONG                    size;   /*!< Element size (Dlist only). */
    ULONG                    tag;    /*!< Element tag (Dlist only).  */
} LINKED_NODE, *PLINKED_NODE;

/*!
 * @brief Allocate a detached node.
 *
 * Fields elemt, next, prev are initialized to NULL. size and tag are
 * set to the given values.
 *
 * @param[in] pElem  User data pointer to store in the node.
 * @param[in] size   Element size in bytes.
 * @param[in] tag    Element tag.
 *
 * @return A pointer to the new node, or NULL on allocation failure.
 */
PLINKED_NODE LinkedNodeAlloc(PVOID pElem, ULONG size, ULONG tag);

/*!
 * @brief Release a node.
 *
 * Does not touch the user data.
 *
 * @param[in] pNode  Node to release.
 */
void LinkedNodeFree(PLINKED_NODE pNode);

/*!
 * @brief Link a node immediately after a reference node.
 *
 * Adjusts pRef->next, pRef->next->prev (if any) and initializes
 * pNew->prev / pNew->next. Does not touch elemt, size or tag. Does
 * not update any container bookkeeping (head, tail, count); the
 * caller is responsible for that.
 *
 * Used by dlist.c (insert_at_end, insert_after), all_dlist.c
 * (stack_push, list_insert, list_add) and (Phase 1) list.c.
 *
 * @param[in]     pRef  Reference node. Must not be NULL.
 * @param[in,out] pNew  Node to insert. Its prev and next fields are
 *                      overwritten.
 */
void LinkedNodeInsertAfter(PLINKED_NODE pRef, PLINKED_NODE pNew);

/*!
 * @brief Link a node immediately before a reference node.
 *
 * Adjusts pRef->prev, pRef->prev->next (if any) and initializes
 * pNew->prev / pNew->next. Does not touch elemt, size or tag. Does
 * not update any container bookkeeping; the caller is responsible
 * for that.
 *
 * Used by dlist.c (insert_at_start, insert_before) and (Phase 1)
 * list.c.
 *
 * @param[in]     pRef  Reference node. Must not be NULL.
 * @param[in,out] pNew  Node to insert. Its prev and next fields are
 *                      overwritten.
 */
void LinkedNodeInsertBefore(PLINKED_NODE pRef, PLINKED_NODE pNew);

/*!
 * @brief Unlink a node from its neighbours.
 *
 * Repairs the neighbours (prev->next and next->prev), then clears
 * pNode->prev and pNode->next so the node is no longer reachable
 * from either end. Does not touch elemt, size or tag. Does not
 * update any container bookkeeping (head, tail, count); the caller
 * is responsible for that.
 *
 * The clearing of pNode->prev and pNode->next is deliberate: after
 * Unlink, a stale pointer to the unlinked node carries no back-link
 * into the container, which makes debugging use-after-unlink bugs
 * easier to spot.
 *
 * Used by dlist.c (unlink_node), all_dlist.c (stack_pop,
 * stack_remove, list_remove) and (Phase 1) list.c.
 *
 * @param[in,out] pNode  Node to unlink. Must not be NULL.
 */
void LinkedNodeUnlink(PLINKED_NODE pNode);

#endif /* CCL_LINKED_LIST_PRIVATE_H */
