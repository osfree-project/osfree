/* Dlist.c - IBM Dlist container implementation.
 * Part of the ccl container library. */

/**
 * @file Dlist.c
 * @brief Implementation of the IBM Dlist interface.
 *
 * @par Interface origin
 * The types, function names, error codes and signatures of the
 * public IBM Dlist interface are reproduced unchanged. See the
 * copyright notice below.
 *
 * @par Implementation
 * The implementation is the work of the osFree Project. The link
 * nodes are shared with the all_dlist module (see
 * linked_list_private.h): LINKED_NODE extends stack_cmp with two
 * extra fields (size, tag) used only here.
 *
 * @par Pooling
 * When the module is compiled with USE_POOLMAN defined, CreateList
 * accepts pool-size parameters, but the ccl implementation does not
 * currently maintain a node pool. Nodes are allocated on demand in
 * both variants. The API is unchanged.
 *
 * Copyright (c) International Business Machines Corp., 2000
 *   The Dlist interface implemented here originates from IBM. The
 *   type names, function names, error codes and signatures are
 *   used with the same names and signatures as in the original.
 *
 * Copyright (c) osFree Project 2026, <http://www.osFree.org>
 *   for licence see licence.txt in root directory, or project website
 */

#include <stdlib.h>
#include <string.h>
#include "dlist_private.h"
#include "linked_list_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/** @brief Magic value identifying a valid Dlist control block. */
#define CCL_DLIST_MAGIC 0x444C5354UL  /* "DLST" */

/**
 * @struct _DLISTCTL
 * @brief Control block of an open list.
 */
typedef struct _DLISTCTL {
    unsigned long ulMagic;      /**< CCL_DLIST_MAGIC.                    */
    PLINKED_NODE  pHead;        /**< First node, or NULL.                */
    PLINKED_NODE  pTail;        /**< Last node, or NULL.                 */
    PLINKED_NODE  pCurrent;     /**< Current node, or NULL when empty.   */
    CARDINAL32    ulCount;      /**< Number of nodes.                    */
} DLISTCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

static DLISTCTL *get_ctl(DLIST hList) {
    DLISTCTL *pCtl;
    if (hList == NULL) return NULL;
    pCtl = (DLISTCTL *)hList;
    if (pCtl->ulMagic != CCL_DLIST_MAGIC) return NULL;
    return pCtl;
}

static PLINKED_NODE find_node(DLISTCTL *pCtl, ADDRESS Handle) {
    PLINKED_NODE pWalk;
    if (Handle == NULL) return pCtl->pCurrent;
    pWalk = pCtl->pHead;
    while (pWalk) {
        if ((ADDRESS)pWalk == Handle) return pWalk;
        pWalk = pWalk->next;
    }
    return NULL;
}

static void unlink_node(DLISTCTL *pCtl, PLINKED_NODE pNode) {
    if (pNode->prev) pNode->prev->next = pNode->next;
    else pCtl->pHead = pNode->next;
    if (pNode->next) pNode->next->prev = pNode->prev;
    else pCtl->pTail = pNode->prev;
    pCtl->ulCount--;
}

static void insert_at_start(DLISTCTL *pCtl, PLINKED_NODE pNode) {
    pNode->prev = NULL;
    pNode->next = pCtl->pHead;
    if (pCtl->pHead) pCtl->pHead->prev = pNode;
    else pCtl->pTail = pNode;
    pCtl->pHead = pNode;
    pCtl->ulCount++;
    if (pCtl->pCurrent == NULL) pCtl->pCurrent = pNode;
}

static void insert_at_end(DLISTCTL *pCtl, PLINKED_NODE pNode) {
    pNode->prev = pCtl->pTail;
    pNode->next = NULL;
    if (pCtl->pTail) pCtl->pTail->next = pNode;
    else pCtl->pHead = pNode;
    pCtl->pTail = pNode;
    pCtl->ulCount++;
    if (pCtl->pCurrent == NULL) pCtl->pCurrent = pNode;
}

static void insert_before(DLISTCTL *pCtl, PLINKED_NODE pRef,
                          PLINKED_NODE pNode) {
    pNode->next = pRef;
    pNode->prev = pRef->prev;
    if (pRef->prev) pRef->prev->next = pNode;
    else pCtl->pHead = pNode;
    pRef->prev = pNode;
    pCtl->ulCount++;
}

static void insert_after(DLISTCTL *pCtl, PLINKED_NODE pRef,
                         PLINKED_NODE pNode) {
    pNode->prev = pRef;
    pNode->next = pRef->next;
    if (pRef->next) pRef->next->prev = pNode;
    else pCtl->pTail = pNode;
    pRef->next = pNode;
    pCtl->ulCount++;
}

/* Common body of InsertItem and InsertObject. */
static ADDRESS insert_common(DLIST           ListToAddTo,
                             CARDINAL32      ItemSize,
                             ADDRESS         ItemLocation,
                             TAG             ItemTag,
                             ADDRESS         TargetHandle,
                             Insertion_Modes Insert_Mode,
                             BOOLEAN         MakeCurrent,
                             BOOLEAN         fCopy,
                             CARDINAL32 *    Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;
    ADDRESS      pData;

    *Error = DLIST_SUCCESS;

    if (ListToAddTo == NULL) { *Error = DLIST_BAD; return NULL; }
    pCtl = get_ctl(ListToAddTo);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }

    if (ItemSize == 0) { *Error = DLIST_ITEM_SIZE_ZERO; return NULL; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return NULL; }
    if (Insert_Mode < InsertAtStart || Insert_Mode > AppendToList) {
        *Error = DLIST_INVALID_INSERTION_MODE;
        return NULL;
    }

    if (fCopy) {
        pData = (ADDRESS)malloc((size_t)ItemSize);
        if (!pData) { *Error = DLIST_OUT_OF_MEMORY; return NULL; }
        memcpy(pData, ItemLocation, (size_t)ItemSize);
    } else {
        pData = ItemLocation;
    }

    pNode = LinkedNodeAlloc(pData, (unsigned long)ItemSize,
                            (unsigned long)ItemTag);
    if (!pNode) {
        if (fCopy) free(pData);
        *Error = DLIST_OUT_OF_MEMORY;
        return NULL;
    }

    switch (Insert_Mode) {
        case InsertAtStart:
            insert_at_start(pCtl, pNode);
            break;
        case AppendToList:
            insert_at_end(pCtl, pNode);
            break;
        case InsertBefore:
        case InsertAfter: {
            PLINKED_NODE pRef = find_node(pCtl, TargetHandle);
            if (!pRef) {
                LinkedNodeFree(pNode);
                if (fCopy) free(pData);
                *Error = DLIST_BAD_HANDLE;
                return NULL;
            }
            if (Insert_Mode == InsertBefore)
                insert_before(pCtl, pRef, pNode);
            else
                insert_after(pCtl, pRef, pNode);
            break;
        }
    }

    if (MakeCurrent) pCtl->pCurrent = pNode;
    return (ADDRESS)pNode;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

static DLIST create_default(void) {
    DLISTCTL *pCtl;

    pCtl = (DLISTCTL *)calloc(1, sizeof(DLISTCTL));
    if (!pCtl) return NULL;
    pCtl->ulMagic = CCL_DLIST_MAGIC;
    return (DLIST)pCtl;
}

#ifdef USE_POOLMAN
DLIST APIENTRY CreateList(CARDINAL32 InitialPoolSize,
                          CARDINAL32 MaximumPoolSize,
                          CARDINAL32 PoolIncrement)
{
    /* ccl does not maintain a node pool. The parameters are accepted
       for interface compatibility and ignored. */
    (void)InitialPoolSize;
    (void)MaximumPoolSize;
    (void)PoolIncrement;
    return create_default();
}
#else
DLIST APIENTRY CreateList(void)
{
    return create_default();
}
#endif

void APIENTRY DestroyList(DLIST * ListToDestroy,
                          BOOLEAN FreeItemMemory,
                          CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pCur, pNext;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToDestroy == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    if (*ListToDestroy == NULL) { *Error = DLIST_NOT_INITIALIZED; return; }
    pCtl = get_ctl(*ListToDestroy);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }

    pCur = pCtl->pHead;
    while (pCur) {
        pNext = pCur->next;
        if (FreeItemMemory && pCur->elemt) free(pCur->elemt);
        LinkedNodeFree(pCur);
        pCur = pNext;
    }

    pCtl->ulMagic = 0;
    free(pCtl);
    *ListToDestroy = NULL;
}

/* ==================================================================
 * Insert
 * ================================================================== */

ADDRESS APIENTRY InsertItem(DLIST           ListToAddTo,
                            CARDINAL32      ItemSize,
                            ADDRESS         ItemLocation,
                            TAG             ItemTag,
                            ADDRESS         TargetHandle,
                            Insertion_Modes Insert_Mode,
                            BOOLEAN         MakeCurrent,
                            CARDINAL32 *    Error)
{
    if (Error == NULL) return NULL;
    return insert_common(ListToAddTo, ItemSize, ItemLocation, ItemTag,
                         TargetHandle, Insert_Mode, MakeCurrent,
                         TRUE, Error);
}

ADDRESS APIENTRY InsertObject(DLIST           ListToAddTo,
                              CARDINAL32      ItemSize,
                              ADDRESS         ItemLocation,
                              TAG             ItemTag,
                              ADDRESS         TargetHandle,
                              Insertion_Modes Insert_Mode,
                              BOOLEAN         MakeCurrent,
                              CARDINAL32 *    Error)
{
    if (Error == NULL) return NULL;
    return insert_common(ListToAddTo, ItemSize, ItemLocation, ItemTag,
                         TargetHandle, Insert_Mode, MakeCurrent,
                         FALSE, Error);
}

/* ==================================================================
 * Delete
 * ================================================================== */

void APIENTRY DeleteItem(DLIST        ListToDeleteFrom,
                         BOOLEAN      FreeMemory,
                         ADDRESS      Handle,
                         CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode, pNewCurrent;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToDeleteFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToDeleteFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return; }

    if (pNode == pCtl->pCurrent) {
        if (pNode->next) pNewCurrent = pNode->next;
        else pNewCurrent = pNode->prev;
    } else {
        pNewCurrent = pCtl->pCurrent;
    }

    unlink_node(pCtl, pNode);
    if (FreeMemory && pNode->elemt) free(pNode->elemt);
    LinkedNodeFree(pNode);
    pCtl->pCurrent = pNewCurrent;
}

void APIENTRY DeleteAllItems(DLIST        ListToDeleteFrom,
                             BOOLEAN      FreeMemory,
                             CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pCur, pNext;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToDeleteFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToDeleteFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }

    pCur = pCtl->pHead;
    while (pCur) {
        pNext = pCur->next;
        if (FreeMemory && pCur->elemt) free(pCur->elemt);
        LinkedNodeFree(pCur);
        pCur = pNext;
    }
    pCtl->pHead = NULL;
    pCtl->pTail = NULL;
    pCtl->pCurrent = NULL;
    pCtl->ulCount = 0;
}

/* ==================================================================
 * Get (copy)
 * ================================================================== */

void APIENTRY GetItem(DLIST          ListToGetItemFrom,
                      CARDINAL32     ItemSize,
                      ADDRESS        ItemLocation,
                      TAG            ItemTag,
                      ADDRESS        Handle,
                      BOOLEAN        MakeCurrent,
                      CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return; }

    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return;
    }

    if (pNode->elemt) memcpy(ItemLocation, pNode->elemt, (size_t)ItemSize);
    if (MakeCurrent) pCtl->pCurrent = pNode;
}

void APIENTRY GetNextItem(DLIST          ListToGetItemFrom,
                          CARDINAL32     ItemSize,
                          ADDRESS        ItemLocation,
                          TAG            ItemTag,
                          CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return; }

    pNode = pCtl->pCurrent;
    if (!pNode || !pNode->next) { *Error = DLIST_END_OF_LIST; return; }
    pNode = pNode->next;

    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return;
    }

    if (pNode->elemt) memcpy(ItemLocation, pNode->elemt, (size_t)ItemSize);
    pCtl->pCurrent = pNode;
}

void APIENTRY GetPreviousItem(DLIST          ListToGetItemFrom,
                              CARDINAL32     ItemSize,
                              ADDRESS        ItemLocation,
                              TAG            ItemTag,
                              CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return; }

    pNode = pCtl->pCurrent;
    if (!pNode || !pNode->prev) {
        *Error = DLIST_ALREADY_AT_START; return;
    }
    pNode = pNode->prev;

    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return;
    }

    if (pNode->elemt) memcpy(ItemLocation, pNode->elemt, (size_t)ItemSize);
    pCtl->pCurrent = pNode;
}

/* ==================================================================
 * Get (pointer)
 * ================================================================== */

ADDRESS APIENTRY GetObject(DLIST          ListToGetItemFrom,
                           CARDINAL32     ItemSize,
                           TAG            ItemTag,
                           ADDRESS        Handle,
                           BOOLEAN        MakeCurrent,
                           CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return NULL;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return NULL; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return NULL; }
    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return NULL;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return NULL;
    }

    if (MakeCurrent) pCtl->pCurrent = pNode;
    return pNode->elemt;
}

ADDRESS APIENTRY GetNextObject(DLIST          ListToGetItemFrom,
                               CARDINAL32     ItemSize,
                               TAG            ItemTag,
                               CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return NULL;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return NULL; }

    pNode = pCtl->pCurrent;
    if (!pNode || !pNode->next) { *Error = DLIST_END_OF_LIST; return NULL; }
    pNode = pNode->next;
    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return NULL;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return NULL;
    }
    pCtl->pCurrent = pNode;
    return pNode->elemt;
}

ADDRESS APIENTRY GetPreviousObject(DLIST          ListToGetItemFrom,
                                   CARDINAL32     ItemSize,
                                   TAG            ItemTag,
                                   CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return NULL;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return NULL; }

    pNode = pCtl->pCurrent;
    if (!pNode || !pNode->prev) {
        *Error = DLIST_ALREADY_AT_START; return NULL;
    }
    pNode = pNode->prev;
    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return NULL;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return NULL;
    }
    pCtl->pCurrent = pNode;
    return pNode->elemt;
}

/* ==================================================================
 * Extract
 * ================================================================== */

void APIENTRY ExtractItem(DLIST          ListToGetItemFrom,
                          CARDINAL32     ItemSize,
                          ADDRESS        ItemLocation,
                          TAG            ItemTag,
                          ADDRESS        Handle,
                          CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode, pNewCurrent;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return; }
    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return;
    }

    if (pNode->elemt) memcpy(ItemLocation, pNode->elemt, (size_t)ItemSize);

    if (pNode == pCtl->pCurrent) {
        if (pNode->next) pNewCurrent = pNode->next;
        else pNewCurrent = pNode->prev;
    } else {
        pNewCurrent = pCtl->pCurrent;
    }

    unlink_node(pCtl, pNode);
    if (pNode->elemt) free(pNode->elemt);
    LinkedNodeFree(pNode);
    pCtl->pCurrent = pNewCurrent;
}

ADDRESS APIENTRY ExtractObject(DLIST          ListToGetItemFrom,
                               CARDINAL32     ItemSize,
                               TAG            ItemTag,
                               ADDRESS        Handle,
                               CARDINAL32 *   Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode, pNewCurrent;
    ADDRESS      pData;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetItemFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return NULL;
    }
    pCtl = get_ctl(ListToGetItemFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return NULL; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return NULL; }
    if (pNode->size != (unsigned long)ItemSize) {
        *Error = DLIST_ITEM_SIZE_WRONG; return NULL;
    }
    if (pNode->tag != (unsigned long)ItemTag) {
        *Error = DLIST_ITEM_TAG_WRONG; return NULL;
    }

    pData = pNode->elemt;

    if (pNode == pCtl->pCurrent) {
        if (pNode->next) pNewCurrent = pNode->next;
        else pNewCurrent = pNode->prev;
    } else {
        pNewCurrent = pCtl->pCurrent;
    }

    unlink_node(pCtl, pNode);
    LinkedNodeFree(pNode);
    pCtl->pCurrent = pNewCurrent;
    return pData;
}

/* ==================================================================
 * Replace
 * ================================================================== */

void APIENTRY ReplaceItem(DLIST         ListToReplaceItemIn,
                          CARDINAL32    ItemSize,
                          ADDRESS       ItemLocation,
                          TAG           ItemTag,
                          ADDRESS       Handle,
                          BOOLEAN       MakeCurrent,
                          CARDINAL32 *  Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;
    ADDRESS      pNew;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToReplaceItemIn == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToReplaceItemIn);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (ItemSize == 0) { *Error = DLIST_ITEM_SIZE_ZERO; return; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return; }

    pNew = (ADDRESS)malloc((size_t)ItemSize);
    if (!pNew) { *Error = DLIST_OUT_OF_MEMORY; return; }
    memcpy(pNew, ItemLocation, (size_t)ItemSize);

    if (pNode->elemt) free(pNode->elemt);
    pNode->elemt = pNew;
    pNode->size  = (unsigned long)ItemSize;
    pNode->tag   = (unsigned long)ItemTag;

    if (MakeCurrent) pCtl->pCurrent = pNode;
}

ADDRESS APIENTRY ReplaceObject(DLIST         ListToReplaceItemIn,
                               CARDINAL32 *  ItemSize,
                               ADDRESS       ItemLocation,
                               TAG        *  ItemTag,
                               ADDRESS       Handle,
                               BOOLEAN       MakeCurrent,
                               CARDINAL32 *  Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;
    ADDRESS      pOld;
    CARDINAL32   ulOldSize;
    TAG          ulOldTag;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToReplaceItemIn == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return NULL;
    }
    pCtl = get_ctl(ListToReplaceItemIn);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return NULL; }
    if (ItemSize == NULL || *ItemSize == 0) {
        *Error = DLIST_ITEM_SIZE_ZERO; return NULL;
    }
    if (ItemTag == NULL) { *Error = DLIST_BAD; return NULL; }
    if (ItemLocation == NULL) { *Error = DLIST_BAD_ITEM_POINTER; return NULL; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return NULL; }

    pOld       = pNode->elemt;
    ulOldSize  = pNode->size;
    ulOldTag   = pNode->tag;

    pNode->elemt = ItemLocation;
    pNode->size  = (unsigned long)*ItemSize;
    pNode->tag   = (unsigned long)*ItemTag;

    *ItemSize = ulOldSize;
    *ItemTag  = ulOldTag;

    if (MakeCurrent) pCtl->pCurrent = pNode;
    return pOld;
}

/* ==================================================================
 * Metadata
 * ================================================================== */

TAG APIENTRY GetTag(DLIST        ListToGetTagFrom,
                    ADDRESS      Handle,
                    CARDINAL32 * ItemSize,
                    CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetTagFrom == NULL || ItemSize == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return 0;
    }
    pCtl = get_ctl(ListToGetTagFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return 0; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return 0; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return 0; }

    *ItemSize = (CARDINAL32)pNode->size;
    return (TAG)pNode->tag;
}

ADDRESS APIENTRY GetHandle(DLIST        ListToGetHandleFrom,
                           CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetHandleFrom == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return NULL;
    }
    pCtl = get_ctl(ListToGetHandleFrom);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return NULL; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return NULL; }

    return (ADDRESS)pCtl->pCurrent;
}

CARDINAL32 APIENTRY GetListSize(DLIST        ListToGetSizeOf,
                                CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToGetSizeOf == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return 0;
    }
    pCtl = get_ctl(ListToGetSizeOf);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return 0; }
    return pCtl->ulCount;
}

BOOLEAN APIENTRY ListEmpty(DLIST        ListToCheck,
                           CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToCheck == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return TRUE;
    }
    pCtl = get_ctl(ListToCheck);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return TRUE; }
    return (pCtl->ulCount == 0) ? TRUE : FALSE;
}

BOOLEAN APIENTRY AtEndOfList(DLIST        ListToCheck,
                             CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToCheck == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return FALSE;
    }
    pCtl = get_ctl(ListToCheck);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return FALSE; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return FALSE; }
    return (pCtl->pCurrent == pCtl->pTail) ? TRUE : FALSE;
}

BOOLEAN APIENTRY AtStartOfList(DLIST        ListToCheck,
                               CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToCheck == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return FALSE;
    }
    pCtl = get_ctl(ListToCheck);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return FALSE; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return FALSE; }
    return (pCtl->pCurrent == pCtl->pHead) ? TRUE : FALSE;
}

/* ==================================================================
 * Cursor movement
 * ================================================================== */

void APIENTRY NextItem(DLIST        ListToAdvance,
                       CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToAdvance == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToAdvance);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (pCtl->pCurrent == pCtl->pTail) {
        *Error = DLIST_END_OF_LIST; return;
    }
    pCtl->pCurrent = pCtl->pCurrent->next;
}

void APIENTRY PreviousItem(DLIST        ListToChange,
                           CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToChange == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToChange);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (pCtl->pCurrent == pCtl->pHead) {
        *Error = DLIST_ALREADY_AT_START; return;
    }
    pCtl->pCurrent = pCtl->pCurrent->prev;
}

void APIENTRY GoToStartOfList(DLIST        ListToReset,
                              CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToReset == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToReset);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    pCtl->pCurrent = pCtl->pHead;
}

void APIENTRY GoToEndOfList(DLIST        ListToSet,
                            CARDINAL32 * Error)
{
    DLISTCTL *pCtl;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToSet == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToSet);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    pCtl->pCurrent = pCtl->pTail;
}

void APIENTRY GoToSpecifiedItem(DLIST        ListToReposition,
                                ADDRESS      Handle,
                                CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToReposition == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToReposition);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }

    pNode = find_node(pCtl, Handle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return; }
    pCtl->pCurrent = pNode;
}

/* ==================================================================
 * Sorting and traversal
 * ================================================================== */

void APIENTRY SortList(DLIST        ListToSort,
                       INTEGER32 (APIENTRY * Compare)(ADDRESS Object1,
                                                     TAG Object1Tag,
                                                     ADDRESS Object2,
                                                     TAG Object2Tag,
                                                     CARDINAL32 * Error),
                       CARDINAL32 * Error)
{
    DLISTCTL     *pCtl;
    PLINKED_NODE *paA, *paB, *paTmp;
    CARDINAL32    n, i, width;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToSort == NULL || Compare == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToSort);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pCtl->ulCount < 2) return;

    n = pCtl->ulCount;
    paA = (PLINKED_NODE *)malloc((size_t)n * sizeof(PLINKED_NODE));
    paB = (PLINKED_NODE *)malloc((size_t)n * sizeof(PLINKED_NODE));
    if (!paA || !paB) {
        free(paA); free(paB);
        *Error = DLIST_OUT_OF_MEMORY;
        return;
    }

    {
        PLINKED_NODE pNode = pCtl->pHead;
        for (i = 0; i < n; i++) {
            paA[i] = pNode;
            pNode = pNode->next;
        }
    }

    /* Bottom-up stable merge sort. */
    for (width = 1; width < n; width *= 2) {
        CARDINAL32 lo;
        for (lo = 0; lo < n; lo += 2 * width) {
            CARDINAL32 mid = lo + width;
            CARDINAL32 hi  = lo + 2 * width;
            CARDINAL32 i1, i2, k;
            if (mid > n) mid = n;
            if (hi  > n) hi  = n;

            i1 = lo; i2 = mid;
            for (k = lo; k < hi; k++) {
                if (i1 < mid && i2 < hi) {
                    INTEGER32 cmp;
                    cmp = Compare(paA[i1]->elemt, (TAG)paA[i1]->tag,
                                  paA[i2]->elemt, (TAG)paA[i2]->tag,
                                  Error);
                    if (*Error != DLIST_SUCCESS) {
                        free(paA); free(paB);
                        return;
                    }
                    if (cmp <= 0) paB[k] = paA[i1++];
                    else          paB[k] = paA[i2++];
                } else if (i1 < mid) {
                    paB[k] = paA[i1++];
                } else {
                    paB[k] = paA[i2++];
                }
            }
        }
        paTmp = paA; paA = paB; paB = paTmp;
    }

    /* Relink. */
    for (i = 0; i < n; i++) {
        paA[i]->prev = (i > 0) ? paA[i-1] : NULL;
        paA[i]->next = (i + 1 < n) ? paA[i+1] : NULL;
    }
    pCtl->pHead = paA[0];
    pCtl->pTail = paA[n-1];

    free(paA);
    free(paB);
}

void APIENTRY ForEachItem(DLIST        ListToProcess,
                          void (APIENTRY * ProcessItem)(ADDRESS Object,
                                                        TAG ObjectTag,
                                                        CARDINAL32 ObjectSize,
                                                        ADDRESS ObjectHandle,
                                                        ADDRESS Parameters,
                                                        CARDINAL32 * Error),
                          ADDRESS      Parameters,
                          BOOLEAN      Forward,
                          CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToProcess == NULL || ProcessItem == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToProcess);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }

    if (Forward) {
        pNode = pCtl->pHead;
        while (pNode) {
            PLINKED_NODE pNext = pNode->next;
            ProcessItem(pNode->elemt, (TAG)pNode->tag,
                        (CARDINAL32)pNode->size,
                        (ADDRESS)pNode, Parameters, Error);
            if (*Error != DLIST_SUCCESS) {
                if (*Error == DLIST_SEARCH_COMPLETE) *Error = DLIST_SUCCESS;
                return;
            }
            pNode = pNext;
        }
    } else {
        pNode = pCtl->pTail;
        while (pNode) {
            PLINKED_NODE pPrev = pNode->prev;
            ProcessItem(pNode->elemt, (TAG)pNode->tag,
                        (CARDINAL32)pNode->size,
                        (ADDRESS)pNode, Parameters, Error);
            if (*Error != DLIST_SUCCESS) {
                if (*Error == DLIST_SEARCH_COMPLETE) *Error = DLIST_SUCCESS;
                return;
            }
            pNode = pPrev;
        }
    }
}

void APIENTRY PruneList(DLIST        ListToProcess,
                        BOOLEAN (APIENTRY * KillItem)(ADDRESS Object,
                                                      TAG ObjectTag,
                                                      CARDINAL32 ObjectSize,
                                                      ADDRESS ObjectHandle,
                                                      ADDRESS Parameters,
                                                      BOOLEAN * FreeMemory,
                                                      CARDINAL32 * Error),
                        ADDRESS      Parameters,
                        CARDINAL32 * Error)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;

    if (Error) *Error = DLIST_SUCCESS;
    if (ListToProcess == NULL || KillItem == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pCtl = get_ctl(ListToProcess);
    if (!pCtl) { *Error = DLIST_NOT_INITIALIZED; return; }

    pNode = pCtl->pHead;
    while (pNode) {
        BOOLEAN    fKill;
        BOOLEAN    fFree = FALSE;
        PLINKED_NODE pNext = pNode->next;

        fKill = KillItem(pNode->elemt, (TAG)pNode->tag,
                         (CARDINAL32)pNode->size,
                         (ADDRESS)pNode, Parameters, &fFree, Error);
        if (*Error != DLIST_SUCCESS) {
            if (*Error == DLIST_SEARCH_COMPLETE) *Error = DLIST_SUCCESS;
            return;
        }

        if (fKill) {
            PLINKED_NODE pPrev = pNode->prev;
            if (pNode->prev) pNode->prev->next = pNode->next;
            else pCtl->pHead = pNode->next;
            if (pNode->next) pNode->next->prev = pNode->prev;
            else pCtl->pTail = pNode->prev;
            pCtl->ulCount--;
            if (pCtl->pCurrent == pNode) {
                if (pNext) pCtl->pCurrent = pNext;
                else pCtl->pCurrent = pPrev;
            }
            if (fFree && pNode->elemt) free(pNode->elemt);
            LinkedNodeFree(pNode);
        }
        pNode = pNext;
    }
}

/* ==================================================================
 * Combination
 * ================================================================== */

void APIENTRY AppendList(DLIST        TargetList,
                         DLIST        SourceList,
                         CARDINAL32 * Error)
{
    DLISTCTL *pTarget;
    DLISTCTL *pSource;

    if (Error) *Error = DLIST_SUCCESS;
    if (TargetList == NULL || SourceList == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pTarget = get_ctl(TargetList);
    pSource = get_ctl(SourceList);
    if (!pTarget || !pSource) { *Error = DLIST_NOT_INITIALIZED; return; }

    if (pSource->ulCount == 0) return;

    if (pTarget->pTail) {
        pTarget->pTail->next = pSource->pHead;
        pSource->pHead->prev = pTarget->pTail;
    } else {
        pTarget->pHead = pSource->pHead;
        pTarget->pCurrent = pSource->pHead;
    }
    pTarget->pTail = pSource->pTail;
    pTarget->ulCount += pSource->ulCount;

    pSource->pHead = NULL;
    pSource->pTail = NULL;
    pSource->pCurrent = NULL;
    pSource->ulCount = 0;
}

void APIENTRY TransferItem(DLIST             SourceList,
                           ADDRESS           SourceHandle,
                           DLIST             TargetList,
                           ADDRESS           TargetHandle,
                           Insertion_Modes   TransferMode,
                           BOOLEAN           MakeCurrent,
                           CARDINAL32 *      Error)
{
    DLISTCTL    *pSrc;
    DLISTCTL    *pTgt;
    PLINKED_NODE pNode;
    PLINKED_NODE pRef = NULL;

    if (Error) *Error = DLIST_SUCCESS;
    if (SourceList == NULL || TargetList == NULL || Error == NULL) {
        if (Error) *Error = DLIST_BAD;
        return;
    }
    pSrc = get_ctl(SourceList);
    pTgt = get_ctl(TargetList);
    if (!pSrc || !pTgt) { *Error = DLIST_NOT_INITIALIZED; return; }
    if (pSrc->ulCount == 0) { *Error = DLIST_EMPTY; return; }
    if (TransferMode < InsertAtStart || TransferMode > AppendToList) {
        *Error = DLIST_INVALID_INSERTION_MODE;
        return;
    }

    pNode = find_node(pSrc, SourceHandle);
    if (!pNode) { *Error = DLIST_BAD_HANDLE; return; }

    if (TransferMode == InsertBefore || TransferMode == InsertAfter) {
        pRef = find_node(pTgt, TargetHandle);
        if (!pRef) { *Error = DLIST_BAD_HANDLE; return; }
    }

    /* Unlink from source. */
    if (pNode->prev) pNode->prev->next = pNode->next;
    else pSrc->pHead = pNode->next;
    if (pNode->next) pNode->next->prev = pNode->prev;
    else pSrc->pTail = pNode->prev;
    pSrc->ulCount--;
    if (pSrc->pCurrent == pNode) {
        if (pNode->next) pSrc->pCurrent = pNode->next;
        else pSrc->pCurrent = pNode->prev;
    }

    /* Insert into target. */
    switch (TransferMode) {
        case InsertAtStart: insert_at_start(pTgt, pNode); break;
        case AppendToList:  insert_at_end(pTgt, pNode);   break;
        case InsertBefore:  insert_before(pTgt, pRef, pNode); break;
        case InsertAfter:   insert_after(pTgt, pRef, pNode);  break;
    }

    if (MakeCurrent) pTgt->pCurrent = pNode;
}

/* ==================================================================
 * Integrity
 * ================================================================== */

BOOLEAN APIENTRY CheckListIntegrity(DLIST ListToCheck)
{
    DLISTCTL    *pCtl;
    PLINKED_NODE pNode;
    PLINKED_NODE pPrev;
    CARDINAL32   n;

    if (ListToCheck == NULL) return FALSE;
    pCtl = (DLISTCTL *)ListToCheck;
    if (pCtl->ulMagic != CCL_DLIST_MAGIC) return FALSE;

    pPrev = NULL;
    pNode = pCtl->pHead;
    n = 0;
    while (pNode) {
        if (pNode->prev != pPrev) return FALSE;
        pPrev = pNode;
        pNode = pNode->next;
        n++;
        if (n > pCtl->ulCount) return FALSE;  /* cycle detected */
    }
    if (pPrev != pCtl->pTail) return FALSE;
    if (n != pCtl->ulCount)   return FALSE;

    if (pCtl->ulCount == 0) {
        if (pCtl->pHead || pCtl->pTail || pCtl->pCurrent) return FALSE;
    } else {
        if (pCtl->pCurrent == NULL) return FALSE;
    }
    return TRUE;
}
