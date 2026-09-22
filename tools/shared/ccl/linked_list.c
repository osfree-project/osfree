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

/*!
 * @brief Allocate a detached node.
 *
 * The elemt, next and prev fields are initialized to NULL. The size
 * and tag fields are set to the given values. The node is not linked
 * into any list.
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

    pNode = (PLINKED_NODE)malloc(sizeof(LINKED_NODE));
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
 * @param[in] pNode  Node to release.
 */
void LinkedNodeFree(PLINKED_NODE pNode) {
    free(pNode);
}
