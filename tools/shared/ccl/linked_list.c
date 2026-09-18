/* linked_list.c - shared node layout and primitives (C89) */

#include <stdlib.h>
#include "linked_list_private.h"

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

void LinkedNodeFree(PLINKED_NODE pNode) {
    free(pNode);
}
