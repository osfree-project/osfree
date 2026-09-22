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

#endif /* CCL_LINKED_LIST_PRIVATE_H */
