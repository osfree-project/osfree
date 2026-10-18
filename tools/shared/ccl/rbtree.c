/*!
 *
 * @file rbtree.c
 *
 * @brief Implementation of the rbtree container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * The rbtree is a thin profile over tree_private. All node
 * structure, rotations, insert-fixup and delete-fixup live in
 * tree_private.c; this file owns RBTREECTL, validates CCLOPTS,
 * implements the CLRS structural splice for deletion (with the
 * swap-payload variant to avoid a NIL sentinel), and translates
 * each public operation to the corresponding tree_private
 * primitive.
 *
 * Deletion uses the swap-payload technique: the target node z is
 * found, its in-order successor y (minimum of the right subtree,
 * or z itself if z has no left or no right child) is identified,
 * the element bytes of z and y are swapped, and y is spliced out.
 * This keeps the splice simple (y has at most one child) and
 * ensures that pDestroyValue fires on the actual removed element.
 */

#include <string.h>
#include "rbtree.h"
#include "tree_private.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_RBTREE_MAGIC
 * @brief Magic value identifying a valid rbtree control block.
 *
 * The literal bytes are "RBT " (with a trailing space).
 */
#define CCL_RBTREE_MAGIC 0x52425420UL  /* "RBT " */

/*!
 * @struct _RBTREECTL
 * @brief Control block of an open rbtree.
 */
typedef struct _RBTREECTL {
    unsigned long  ulMagic;      /*!< CCL_RBTREE_MAGIC.              */
    unsigned long  ulElemSize;   /*!< Size of one element.           */
    unsigned long  ulCount;      /*!< Number of elements.            */
    ULONG          ulFlags;      /*!< Copy of CCLOPTS.ulFlags.       */
    PTREENODE      pRoot;        /*!< Tree root, or NULL.            */
    CCL_COMPARE_FN pCompare;     /*!< Required ordering callback.    */
    CCL_DESTROY_FN pDestroyValue;/*!< Optional destroy callback.     */
} RBTREECTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind an rbtree handle.
 *
 * @param[in] hTree  Rbtree handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hTree is NULL, or its magic value does not match.
 */
static RBTREECTL *get_ctl(HRBTREE hTree) {
    return (RBTREECTL *)ccl_validate(hTree, CCL_RBTREE_MAGIC);
}

/*!
 * @brief Swap the element bytes of two nodes.
 *
 * Byte-by-byte swap; no destroy callback is invoked. Used by
 * deletion to move the target element into the node that will be
 * spliced out.
 *
 * @param[in,out] pA         First node. Not NULL.
 * @param[in,out] pB         Second node. Not NULL.
 * @param[in]     ulElemSize Size of the element area.
 */
static void swap_elements(PTREENODE pA, PTREENODE pB, ULONG ulElemSize) {
    UCHAR tmp;
    ULONG i;
    for (i = 0; i < ulElemSize; i++) {
        tmp          = (UCHAR)pA->u.c[i];
        pA->u.c[i]   = pB->u.c[i];
        pB->u.c[i]   = (char)tmp;
    }
}

/*!
 * @brief Copy the element of one node into a caller buffer with
 *        size-query semantics.
 *
 * @param[in]  pNode    Node. Not NULL.
 * @param[in]  ulElem   Element size.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
static APIRET copy_element(PTREENODE pNode, ULONG ulElem,
                           PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = ulElem;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < ulElem) {
        if (pulUsed) *pulUsed = ulElem;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pBuf, pNode->u.c, ulElem);
    if (pulUsed) *pulUsed = ulElem;
    return NO_ERROR;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new rbtree.
 *
 * @param[in]  ulElemSize  Size of one element in bytes.
 * @param[in]  pOpts       Option block. pCompare is required.
 * @param[out] phTree      Receives the new handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument or invalid pOpts.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RbtreeCreate(ULONG ulElemSize, PCCLOPTS pOpts,
                             PHRBTREE phTree) {
    RBTREECTL *pCtl;
    APIRET rc;

    if (!phTree) return ERROR_INVALID_PARAMETER;
    if (ulElemSize == 0) return ERROR_INVALID_PARAMETER;
    *phTree = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_COMPARE
                         | CCL_OPT_KEYFN
                         | CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_FLAG_DUPLICATES
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         CCL_OPT_COMPARE);
    if (rc != NO_ERROR) return rc;

    pCtl = (RBTREECTL *)(*CclCalloc)(1, sizeof(RBTREECTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->ulMagic        = CCL_RBTREE_MAGIC;
    pCtl->ulElemSize     = (unsigned long)ulElemSize;
    pCtl->ulCount        = 0;
    pCtl->pRoot          = NULL;
    pCtl->pCompare       = pOpts->pCompare;
    pCtl->pDestroyValue  = pOpts->pDestroyValue;
    pCtl->ulFlags        = pOpts->ulFlags;

    *phTree = (HRBTREE)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy an rbtree.
 *
 * @param[in] hTree  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY RbtreeDestroy(HRBTREE hTree) {
    RBTREECTL *pCtl;

    if (hTree == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    RbDestroySubtree(pCtl->pRoot, pCtl->pDestroyValue);
    pCtl->pRoot   = NULL;
    pCtl->ulCount = 0;
    pCtl->ulMagic = 0;
    (*CclFree)(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Insert, remove
 * ================================================================== */

/*!
 * @brief Insert a copy of an element.
 *
 * @param[in]  hTree       Handle.
 * @param[in]  pElem       Pointer to the element to copy. Not NULL.
 * @param[out] pfInserted  Optional. Receives TRUE if inserted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTree or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY RbtreeInsert(HRBTREE hTree, PCVOID pElem,
                             PBOOL pfInserted) {
    RBTREECTL *pCtl;
    PTREENODE pNode;
    BOOL fDuplicates;
    BOOL fReplaced = FALSE;
    PTREENODE pResult;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pfInserted) *pfInserted = FALSE;

    pNode = RbAllocNode(pCtl->ulElemSize);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;
    memcpy(pNode->u.c, pElem, pCtl->ulElemSize);

    fDuplicates = (pCtl->ulFlags & CCL_FLAG_ALLOW_DUPLICATES) != 0;

    if (fDuplicates) {
        /* Always insert; use pfReplaced = NULL to skip the
           duplicate check inside RbInsert. */
        pResult = RbInsert(&pCtl->pRoot, pNode, pCtl->pCompare, NULL);
        /* pResult == pNode in this mode. */
        (void)pResult;
        pCtl->ulCount++;
        if (pfInserted) *pfInserted = TRUE;
        return NO_ERROR;
    }

    pResult = RbInsert(&pCtl->pRoot, pNode, pCtl->pCompare, &fReplaced);
    if (fReplaced) {
        /* Existing key: release the freshly allocated node. */
        RbFreeNode(pNode);
        if (pfInserted) *pfInserted = FALSE;
        return NO_ERROR;
    }
    pCtl->ulCount++;
    if (pfInserted) *pfInserted = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Remove one element by key.
 *
 * @param[in]  hTree      Handle.
 * @param[in]  pKey       Pointer to the key bytes. Not NULL.
 * @param[out] pfRemoved  Optional. Receives TRUE if removed.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_PARAMETER  hTree or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeRemove(HRBTREE hTree, PCVOID pKey,
                             PBOOL pfRemoved) {
    RBTREECTL *pCtl;
    PTREENODE z, y, x, xParent;
    BOOL fYOriginalRed;
    BOOL fXIsLeft;

    if (!pKey) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pfRemoved) *pfRemoved = FALSE;

    z = RbFind(pCtl->pRoot, pKey, pCtl->pCompare);
    if (!z) return NO_ERROR;

    y = z;
    if (z->left && z->right) y = RbMin(z->right);

    /* Swap element bytes so that y carries the target element.
       No destroy fires here. */
    if (y != z)
        swap_elements(z, y, pCtl->ulElemSize);

    fYOriginalRed = (y->ulRed == RB_RED);

    /* Splice y out. y has at most one child (no left, by
       construction: y = RbMin(z->right) has no left; or y == z
       with at most one child). */
    x = (y->left) ? y->left : y->right;
    xParent = y->parent;

    if (xParent == NULL) {
        pCtl->pRoot = x;
    } else {
        fXIsLeft = (y == xParent->left);
        if (fXIsLeft) xParent->left = x;
        else          xParent->right = x;
    }
    if (x) x->parent = xParent;
    if (xParent == NULL) fXIsLeft = FALSE;
    else fXIsLeft = (x == xParent->left);

    if (!fYOriginalRed)
        RbDeleteFixup(&pCtl->pRoot, x, xParent, fXIsLeft);

    if (pCtl->pDestroyValue) pCtl->pDestroyValue(&y->u);
    RbFreeNode(y);
    pCtl->ulCount--;
    if (pfRemoved) *pfRemoved = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Remove every element from the tree.
 *
 * @param[in] hTree  Handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeClear(HRBTREE hTree) {
    RBTREECTL *pCtl;

    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    RbDestroySubtree(pCtl->pRoot, pCtl->pDestroyValue);
    pCtl->pRoot   = NULL;
    pCtl->ulCount = 0;
    return NO_ERROR;
}

/* ==================================================================
 * Lookup
 * ================================================================== */

/*!
 * @brief Copy the element matching a key into the caller's buffer.
 *
 * @param[in]  hTree    Handle.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTree or pKey is NULL, or pBuf
 *                                  is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No matching element.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY RbtreeFind(HRBTREE hTree, PCVOID pKey,
                           PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    RBTREECTL *pCtl;
    PTREENODE pNode;

    if (!pKey) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    pNode = RbFind(pCtl->pRoot, pKey, pCtl->pCompare);
    if (!pNode) return ERROR_NO_MORE_ITEMS;

    return copy_element(pNode, pCtl->ulElemSize, pBuf, ulSize, pulUsed);
}

/*!
 * @brief Test whether a key is present.
 *
 * @param[in]  hTree      Handle.
 * @param[in]  pKey       Pointer to the key bytes. Not NULL.
 * @param[out] pfPresent  Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeContains(HRBTREE hTree, PCVOID pKey,
                               PBOOL pfPresent) {
    RBTREECTL *pCtl;

    if (!pKey || !pfPresent) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pfPresent = (RbFind(pCtl->pRoot, pKey, pCtl->pCompare) != NULL)
                 ? TRUE : FALSE;
    return NO_ERROR;
}

/*!
 * @brief Copy the minimum element into the caller's buffer.
 *
 * @param[in]  hTree    Handle.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The tree is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL without size-query.
 */
APIRET APIENTRY RbtreeGetMin(HRBTREE hTree, PVOID pBuf,
                             ULONG ulSize, PULONG pulUsed) {
    RBTREECTL *pCtl;

    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if (!pCtl->pRoot) return ERROR_NO_MORE_ITEMS;

    return copy_element(RbMin(pCtl->pRoot), pCtl->ulElemSize,
                        pBuf, ulSize, pulUsed);
}

/*!
 * @brief Copy the maximum element into the caller's buffer.
 *
 * @param[in]  hTree    Handle.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The tree is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL without size-query.
 */
APIRET APIENTRY RbtreeGetMax(HRBTREE hTree, PVOID pBuf,
                             ULONG ulSize, PULONG pulUsed) {
    RBTREECTL *pCtl;

    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if (!pCtl->pRoot) return ERROR_NO_MORE_ITEMS;

    return copy_element(RbMax(pCtl->pRoot), pCtl->ulElemSize,
                        pBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Metadata and in-order access
 * ================================================================== */

/*!
 * @brief Return the number of elements.
 *
 * @param[in]  hTree     Handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY RbtreeGetCount(HRBTREE hTree, PULONG pulCount) {
    RBTREECTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pulCount = (ULONG)pCtl->ulCount;
    return NO_ERROR;
}

/*!
 * @brief Copy the i-th element in in-order position.
 *
 * @param[in]  hTree    Handle.
 * @param[in]  ulIndex  Zero-based in-order index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTree is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY RbtreeGetIth(HRBTREE hTree, ULONG ulIndex,
                             PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    RBTREECTL *pCtl;
    PTREENODE pNode;
    ULONG i;

    pCtl = get_ctl(hTree);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = pCtl->ulElemSize;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < pCtl->ulElemSize) {
        if (pulUsed) *pulUsed = pCtl->ulElemSize;
        return ERROR_BUFFER_OVERFLOW;
    }
    if (ulIndex >= (ULONG)pCtl->ulCount) return ERROR_NO_MORE_ITEMS;

    pNode = RbMin(pCtl->pRoot);
    for (i = 0; i < ulIndex; i++)
        pNode = RbSuccessor(pNode);
    /* pNode is guaranteed non-NULL: ulIndex < count. */

    return copy_element(pNode, pCtl->ulElemSize, pBuf, ulSize, pulUsed);
}
