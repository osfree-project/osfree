/* dlist.c - dynamic list and stack containers (C89) */

#include <stdlib.h>
#include "dlist.h"

/* ==================================================================
 * Internal structures (private)
 * ================================================================== */

#define DLIST_MAGIC 0x4C495344UL  /* "DLIS" */

typedef struct _DLISTNODE {
    PVOID               pElem;
    struct _DLISTNODE  *pPrev;
    struct _DLISTNODE  *pNext;
} DLISTNODE, *PDLISTNODE;

typedef struct _DLIST {
    ULONG       ulMagic;
    ULONG       flFlags;
    PDLISTNODE  pHead;
    PDLISTNODE  pTail;
    PDLISTNODE  pCurrent;
} DLIST, *PDLIST;

/* ==================================================================
 * Internal helpers (private)
 * ================================================================== */

static PDLIST as_dlist(HANDLE h) {
    PDLIST pList;
    if (!h) return NULL;
    pList = (PDLIST)h;
    if (pList->ulMagic != DLIST_MAGIC) return NULL;
    return pList;
}

static PDLISTNODE node_new(PVOID pElem) {
    PDLISTNODE pNode = (PDLISTNODE)malloc(sizeof(DLISTNODE));
    if (!pNode) return NULL;
    pNode->pElem = pElem;
    pNode->pPrev = NULL;
    pNode->pNext = NULL;
    return pNode;
}

static void node_free(PDLISTNODE pNode, ULONG flFlags) {
    if (flFlags & DLIST_OWN_ELEMENTS) {
        if (pNode->pElem) free(pNode->pElem);
    }
    free(pNode);
}

static APIRET node_unlink(PDLIST pList, PDLISTNODE pNode) {
    if (pNode->pPrev) pNode->pPrev->pNext = pNode->pNext;
    else pList->pHead = pNode->pNext;

    if (pNode->pNext) pNode->pNext->pPrev = pNode->pPrev;
    else pList->pTail = pNode->pPrev;

    return NO_ERROR;
}

/* ==================================================================
 * Stack API
 * ================================================================== */

APIRET APIENTRY StackCreate(ULONG flFlags, PHSTACK phStack) {
    PDLIST pList;

    if (!phStack) return ERROR_INVALID_PARAMETER;
    *phStack = NULLHANDLE;

    pList = (PDLIST)calloc(1, sizeof(DLIST));
    if (!pList) return ERROR_NOT_ENOUGH_MEMORY;

    pList->ulMagic = DLIST_MAGIC;
    pList->flFlags = flFlags;
    *phStack = (HSTACK)pList;
    return NO_ERROR;
}

APIRET APIENTRY StackDestroy(HSTACK hStack) {
    PDLIST pList;
    PDLISTNODE pCur, pNext;

    if (hStack == NULLHANDLE) return NO_ERROR;
    pList = as_dlist(hStack);
    if (!pList) return ERROR_INVALID_HANDLE;

    pCur = pList->pHead;
    while (pCur) {
        pNext = pCur->pNext;
        node_free(pCur, pList->flFlags);
        pCur = pNext;
    }

    pList->ulMagic = 0;
    free(pList);
    return NO_ERROR;
}

APIRET APIENTRY StackPush(HSTACK hStack, PVOID pElem) {
    PDLIST pList = as_dlist(hStack);
    PDLISTNODE pNode;

    if (!pList) return ERROR_INVALID_HANDLE;

    pNode = node_new(pElem);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    pNode->pNext = pList->pHead;
    if (pList->pHead) pList->pHead->pPrev = pNode;
    pList->pHead = pNode;
    if (!pList->pTail) pList->pTail = pNode;
    return NO_ERROR;
}

APIRET APIENTRY StackPop(HSTACK hStack, PPVOID ppElem) {
    PDLIST pList;
    PDLISTNODE pNode;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    *ppElem = NULL;

    pList = as_dlist(hStack);
    if (!pList) return ERROR_INVALID_HANDLE;

    if (!pList->pHead) return ERROR_NO_DATA;

    pNode = pList->pHead;
    *ppElem = pNode->pElem;
    pList->pHead = pNode->pNext;
    if (pList->pHead) pList->pHead->pPrev = NULL;
    else pList->pTail = NULL;

    free(pNode);
    return NO_ERROR;
}

APIRET APIENTRY StackTop(HSTACK hStack, PPVOID ppElem) {
    PDLIST pList;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    *ppElem = NULL;

    pList = as_dlist(hStack);
    if (!pList) return ERROR_INVALID_HANDLE;

    if (!pList->pHead) return ERROR_NO_DATA;

    *ppElem = pList->pHead->pElem;
    return NO_ERROR;
}

APIRET APIENTRY StackRemove(HSTACK hStack, PVOID pElem) {
    PDLIST pList;
    PDLISTNODE pCur;

    if (!pElem) return ERROR_INVALID_PARAMETER;

    pList = as_dlist(hStack);
    if (!pList) return ERROR_INVALID_HANDLE;

    pCur = pList->pHead;
    while (pCur) {
        if (pCur->pElem == pElem) {
            node_unlink(pList, pCur);
            node_free(pCur, pList->flFlags);
            return NO_ERROR;
        }
        pCur = pCur->pNext;
    }
    return ERROR_NO_MORE_ITEMS;
}

APIRET APIENTRY StackIsEmpty(HSTACK hStack, PBOOL pfEmpty) {
    PDLIST pList;

    if (!pfEmpty) return ERROR_INVALID_PARAMETER;
    *pfEmpty = FALSE_;

    pList = as_dlist(hStack);
    if (!pList) return ERROR_INVALID_HANDLE;

    *pfEmpty = pList->pHead ? FALSE_ : TRUE_;
    return NO_ERROR;
}

/* ==================================================================
 * List API
 * ================================================================== */

APIRET APIENTRY ListCreate(ULONG flFlags, PHLIST phList) {
    PDLIST pList;

    if (!phList) return ERROR_INVALID_PARAMETER;
    *phList = NULLHANDLE;

    pList = (PDLIST)calloc(1, sizeof(DLIST));
    if (!pList) return ERROR_NOT_ENOUGH_MEMORY;

    pList->ulMagic = DLIST_MAGIC;
    pList->flFlags = flFlags;
    *phList = (HLIST)pList;
    return NO_ERROR;
}

APIRET APIENTRY ListDestroy(HLIST hList) {
    PDLIST pList;
    PDLISTNODE pCur, pNext;

    if (hList == NULLHANDLE) return NO_ERROR;
    pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;

    pCur = pList->pHead;
    while (pCur) {
        pNext = pCur->pNext;
        node_free(pCur, pList->flFlags);
        pCur = pNext;
    }

    pList->ulMagic = 0;
    free(pList);
    return NO_ERROR;
}

APIRET APIENTRY ListGet(HLIST hList, PPVOID ppElem) {
    PDLIST pList;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    *ppElem = NULL;

    pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;

    if (!pList->pCurrent) return ERROR_NO_DATA;

    *ppElem = pList->pCurrent->pElem;
    return NO_ERROR;
}

APIRET APIENTRY ListInsert(HLIST hList, PVOID pElem) {
    PDLIST pList;
    PDLISTNODE pNode;

    pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;

    pNode = node_new(pElem);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    if (!pList->pCurrent) {
        /* Empty list: the new node becomes the only element. */
        pList->pHead = pNode;
        pList->pTail = pNode;
        pList->pCurrent = pNode;
        return NO_ERROR;
    }

    /* Insert after the current node; cursor moves to the new node. */
    pNode->pPrev = pList->pCurrent;
    pNode->pNext = pList->pCurrent->pNext;
    if (pList->pCurrent->pNext) pList->pCurrent->pNext->pPrev = pNode;
    else pList->pTail = pNode;
    pList->pCurrent->pNext = pNode;
    pList->pCurrent = pNode;
    return NO_ERROR;
}

APIRET APIENTRY ListRemove(HLIST hList) {
    PDLIST pList;
    PDLISTNODE pNode;
    PDLISTNODE pNewCur;

    pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;
    if (!pList->pCurrent) return ERROR_NO_DATA;

    pNode = pList->pCurrent;

    /* Cursor moves to the next node if present, otherwise to the
     * previous one; if the list becomes empty the cursor is unset. */
    if (pNode->pNext) pNewCur = pNode->pNext;
    else pNewCur = pNode->pPrev;

    node_unlink(pList, pNode);
    node_free(pNode, pList->flFlags);
    pList->pCurrent = pNewCur;
    return NO_ERROR;
}

APIRET APIENTRY ListAdd(HLIST hList, PVOID pElem) {
    PDLIST pList;
    PDLISTNODE pNode;

    pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;

    pNode = node_new(pElem);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    if (!pList->pTail) {
        pList->pHead = pNode;
        pList->pTail = pNode;
        /* Cursor remains unset. */
        return NO_ERROR;
    }

    pNode->pPrev = pList->pTail;
    pList->pTail->pNext = pNode;
    pList->pTail = pNode;
    return NO_ERROR;
}

APIRET APIENTRY ListSetStart(HLIST hList) {
    PDLIST pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;
    if (!pList->pHead) return ERROR_NO_DATA;
    pList->pCurrent = pList->pHead;
    return NO_ERROR;
}

APIRET APIENTRY ListSetEnd(HLIST hList) {
    PDLIST pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;
    if (!pList->pTail) return ERROR_NO_DATA;
    pList->pCurrent = pList->pTail;
    return NO_ERROR;
}

APIRET APIENTRY ListPrev(HLIST hList) {
    PDLIST pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;
    if (!pList->pCurrent) return ERROR_NO_DATA;
    if (!pList->pCurrent->pPrev) return ERROR_NO_MORE_ITEMS;
    pList->pCurrent = pList->pCurrent->pPrev;
    return NO_ERROR;
}

APIRET APIENTRY ListNext(HLIST hList) {
    PDLIST pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;
    if (!pList->pCurrent) return ERROR_NO_DATA;
    if (!pList->pCurrent->pNext) return ERROR_NO_MORE_ITEMS;
    pList->pCurrent = pList->pCurrent->pNext;
    return NO_ERROR;
}

APIRET APIENTRY ListIsEmpty(HLIST hList, PBOOL pfEmpty) {
    PDLIST pList;

    if (!pfEmpty) return ERROR_INVALID_PARAMETER;
    *pfEmpty = FALSE_;

    pList = as_dlist(hList);
    if (!pList) return ERROR_INVALID_HANDLE;

    *pfEmpty = pList->pHead ? FALSE_ : TRUE_;
    return NO_ERROR;
}
