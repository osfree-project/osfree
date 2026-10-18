/*!
 *
 * @file tree_private.c
 *
 * @brief Implementation of the red-black tree engine.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * See tree_private.h for the contract. The implementation follows
 * the standard CLRS red-black tree algorithms, adapted to work
 * without a NIL sentinel: children are NULL, and the caller of
 * RbDeleteFixup passes the parent and the side of the spliced node
 * so that the fixup can start from a NULL x.
 */

#include <stddef.h>
#include <string.h>
#include "tree_private.h"
#include "ccl_alloc.h"
#include "node_private.h"

/* ==================================================================
 * Node allocation
 * ================================================================== */

/*!
 * @brief Allocate a new detached tree node.
 *
 * @param[in] ulElemSize  Size of one element in bytes.
 *
 * @return A new node, or NULL on allocation failure.
 *
 * @retval NULL  Allocation failed.
 */
PTREENODE RbAllocNode(ULONG ulElemSize) {
    PTREENODE pNode;

    pNode = (PTREENODE)ccl_node_alloc(
        (ULONG)offsetof(TREENODE, u),
        ulElemSize,
        (ULONG)sizeof(double));
    if (!pNode) return NULL;

    pNode->left   = NULL;
    pNode->right  = NULL;
    pNode->parent = NULL;
    pNode->ulRed  = RB_BLACK;
    return pNode;
}

/*!
 * @brief Release a node allocated by RbAllocNode.
 *
 * @param[in] pNode  Node to release. May be NULL.
 */
void RbFreeNode(PTREENODE pNode) {
    if (!pNode) return;
    ccl_node_free(pNode);
}

/* ==================================================================
 * Rotations
 * ================================================================== */

/*!
 * @brief Rotate the subtree rooted at pX to the left.
 *
 * @param[in,out] ppRoot  Pointer to the tree's root. Not NULL.
 * @param[in]     pX      Node to rotate around.
 */
void RbRotateLeft(PTREENODE *ppRoot, PTREENODE pX) {
    PTREENODE pY = pX->right;

    pX->right = pY->left;
    if (pY->left) pY->left->parent = pX;

    pY->parent = pX->parent;
    if (pX->parent == NULL) *ppRoot = pY;
    else if (pX == pX->parent->left) pX->parent->left = pY;
    else pX->parent->right = pY;

    pY->left = pX;
    pX->parent = pY;
}

/*!
 * @brief Rotate the subtree rooted at pX to the right.
 *
 * @param[in,out] ppRoot  Pointer to the tree's root. Not NULL.
 * @param[in]     pX      Node to rotate around.
 */
void RbRotateRight(PTREENODE *ppRoot, PTREENODE pX) {
    PTREENODE pY = pX->left;

    pX->left = pY->right;
    if (pY->right) pY->right->parent = pX;

    pY->parent = pX->parent;
    if (pX->parent == NULL) *ppRoot = pY;
    else if (pX == pX->parent->right) pX->parent->right = pY;
    else pX->parent->left = pY;

    pY->right = pX;
    pX->parent = pY;
}

/* ==================================================================
 * Insert
 * ================================================================== */

/*!
 * @brief Insert a node and restore the red-black invariants.
 *
 * @param[in,out] ppRoot      Pointer to the tree's root.
 * @param[in]     pNode       Node to insert.
 * @param[in]     pCompare    Comparator.
 * @param[out]    pfReplaced  Optional duplicate-result flag.
 *
 * @return The linked node, or the pre-existing duplicate node.
 */
PTREENODE RbInsert(PTREENODE *ppRoot, PTREENODE pNode,
                   CCL_COMPARE_FN pCompare, PBOOL pfReplaced) {
    PTREENODE pParent = NULL;
    PTREENODE pCur = *ppRoot;
    int cmp = 0;

    if (pfReplaced) *pfReplaced = FALSE;

    while (pCur) {
        pParent = pCur;
        cmp = pCompare((PCVOID)pNode->u.c, (PCVOID)pCur->u.c, NULL);
        if (cmp == 0 && pfReplaced != NULL) {
            *pfReplaced = TRUE;
            return pCur;
        }
        pCur = (cmp < 0) ? pCur->left : pCur->right;
    }

    pNode->left   = NULL;
    pNode->right  = NULL;
    pNode->parent = pParent;
    pNode->ulRed  = RB_RED;

    if (!pParent) {
        *ppRoot = pNode;
    } else if (cmp < 0) {
        pParent->left = pNode;
    } else {
        pParent->right = pNode;
    }

    /* Insert fixup */
    while (pNode->parent && pNode->parent->ulRed == RB_RED) {
        PTREENODE pParent = pNode->parent;
        PTREENODE pGrand  = pParent->parent;
        if (pParent == pGrand->left) {
            PTREENODE pUncle = pGrand->right;
            if (pUncle && pUncle->ulRed == RB_RED) {
                pParent->ulRed = RB_BLACK;
                pUncle->ulRed  = RB_BLACK;
                pGrand->ulRed  = RB_RED;
                pNode = pGrand;
            } else {
                if (pNode == pParent->right) {
                    pNode = pParent;
                    RbRotateLeft(ppRoot, pNode);
                    pParent = pNode->parent;
                }
                pParent->ulRed = RB_BLACK;
                pGrand->ulRed  = RB_RED;
                RbRotateRight(ppRoot, pGrand);
            }
        } else {
            PTREENODE pUncle = pGrand->left;
            if (pUncle && pUncle->ulRed == RB_RED) {
                pParent->ulRed = RB_BLACK;
                pUncle->ulRed  = RB_BLACK;
                pGrand->ulRed  = RB_RED;
                pNode = pGrand;
            } else {
                if (pNode == pParent->left) {
                    pNode = pParent;
                    RbRotateRight(ppRoot, pNode);
                    pParent = pNode->parent;
                }
                pParent->ulRed = RB_BLACK;
                pGrand->ulRed  = RB_RED;
                RbRotateLeft(ppRoot, pGrand);
            }
        }
    }
    (*ppRoot)->ulRed = RB_BLACK;
    return pNode;
}

/* ==================================================================
 * Delete fixup
 * ================================================================== */

/*!
 * @brief Restore the red-black invariants after a structural
 *        splice.
 *
 * @param[in,out] ppRoot     Pointer to the tree's root.
 * @param[in]     pX         Node that replaced y, or NULL.
 * @param[in]     pXParent   y's parent before the splice, or NULL.
 * @param[in]     fXIsLeft   TRUE if y was the left child.
 */
void RbDeleteFixup(PTREENODE *ppRoot, PTREENODE pX,
                   PTREENODE pXParent, BOOL fXIsLeft) {
    while (pX != *ppRoot && (pX == NULL || pX->ulRed == RB_BLACK)) {
        PTREENODE pW;
        if (pXParent == NULL) break;

        if (fXIsLeft) {
            pW = pXParent->right;
            if (pW == NULL) break;
            if (pW->ulRed == RB_RED) {
                pW->ulRed = RB_BLACK;
                pXParent->ulRed = RB_RED;
                RbRotateLeft(ppRoot, pXParent);
                pW = pXParent->right;
            }
            if ((pW->left  == NULL || pW->left->ulRed  == RB_BLACK) &&
                (pW->right == NULL || pW->right->ulRed == RB_BLACK)) {
                pW->ulRed = RB_RED;
                pX = pXParent;
                pXParent = pX->parent;
                fXIsLeft = (pXParent && pX == pXParent->left) ? TRUE : FALSE;
            } else {
                if (pW->right == NULL || pW->right->ulRed == RB_BLACK) {
                    if (pW->left) pW->left->ulRed = RB_BLACK;
                    pW->ulRed = RB_RED;
                    RbRotateRight(ppRoot, pW);
                    pW = pXParent->right;
                }
                pW->ulRed = pXParent->ulRed;
                pXParent->ulRed = RB_BLACK;
                if (pW->right) pW->right->ulRed = RB_BLACK;
                RbRotateLeft(ppRoot, pXParent);
                pX = *ppRoot;
                pXParent = NULL;
                fXIsLeft = FALSE;
            }
        } else {
            pW = pXParent->left;
            if (pW == NULL) break;
            if (pW->ulRed == RB_RED) {
                pW->ulRed = RB_BLACK;
                pXParent->ulRed = RB_RED;
                RbRotateRight(ppRoot, pXParent);
                pW = pXParent->left;
            }
            if ((pW->left  == NULL || pW->left->ulRed  == RB_BLACK) &&
                (pW->right == NULL || pW->right->ulRed == RB_BLACK)) {
                pW->ulRed = RB_RED;
                pX = pXParent;
                pXParent = pX->parent;
                fXIsLeft = (pXParent && pX == pXParent->left) ? TRUE : FALSE;
            } else {
                if (pW->left == NULL || pW->left->ulRed == RB_BLACK) {
                    if (pW->right) pW->right->ulRed = RB_BLACK;
                    pW->ulRed = RB_RED;
                    RbRotateLeft(ppRoot, pW);
                    pW = pXParent->left;
                }
                pW->ulRed = pXParent->ulRed;
                pXParent->ulRed = RB_BLACK;
                if (pW->left) pW->left->ulRed = RB_BLACK;
                RbRotateRight(ppRoot, pXParent);
                pX = *ppRoot;
                pXParent = NULL;
                fXIsLeft = FALSE;
            }
        }
    }
    if (pX) pX->ulRed = RB_BLACK;
}

/* ==================================================================
 * Lookup and navigation
 * ================================================================== */

/*!
 * @brief Find a node by key.
 *
 * @param[in] pRoot     Tree root. May be NULL.
 * @param[in] pKey      Pointer to the key bytes. Not NULL.
 * @param[in] pCompare  Comparator. Not NULL.
 *
 * @return The matching node, or NULL.
 *
 * @retval NULL  No node matches.
 */
PTREENODE RbFind(PTREENODE pRoot, PCVOID pKey, CCL_COMPARE_FN pCompare) {
    while (pRoot) {
        int cmp = pCompare((PCVOID)pRoot->u.c, pKey, NULL);
        if (cmp == 0) return pRoot;
        pRoot = (cmp > 0) ? pRoot->left : pRoot->right;
    }
    return NULL;
}

/*!
 * @brief Return the leftmost node of the subtree rooted at pNode.
 *
 * @param[in] pNode  Subtree root. Not NULL.
 *
 * @return The leftmost node.
 */
PTREENODE RbMin(PTREENODE pNode) {
    while (pNode->left) pNode = pNode->left;
    return pNode;
}

/*!
 * @brief Return the rightmost node of the subtree rooted at pNode.
 *
 * @param[in] pNode  Subtree root. Not NULL.
 *
 * @return The rightmost node.
 */
PTREENODE RbMax(PTREENODE pNode) {
    while (pNode->right) pNode = pNode->right;
    return pNode;
}

/*!
 * @brief Return the in-order successor of pNode, or NULL.
 *
 * @param[in] pNode  Node. Not NULL.
 *
 * @return The successor, or NULL.
 *
 * @retval NULL  pNode is the maximum.
 */
PTREENODE RbSuccessor(PTREENODE pNode) {
    PTREENODE pParent;
    if (pNode->right) return RbMin(pNode->right);
    pParent = pNode->parent;
    while (pParent && pNode == pParent->right) {
        pNode = pParent;
        pParent = pParent->parent;
    }
    return pParent;
}

/*!
 * @brief Return the in-order predecessor of pNode, or NULL.
 *
 * @param[in] pNode  Node. Not NULL.
 *
 * @return The predecessor, or NULL.
 *
 * @retval NULL  pNode is the minimum.
 */
PTREENODE RbPredecessor(PTREENODE pNode) {
    PTREENODE pParent;
    if (pNode->left) return RbMax(pNode->left);
    pParent = pNode->parent;
    while (pParent && pNode == pParent->left) {
        pNode = pParent;
        pParent = pParent->parent;
    }
    return pParent;
}

/* ==================================================================
 * Subtree destruction
 * ================================================================== */

/*!
 * @brief Release every node in the subtree rooted at pRoot.
 *
 * @param[in] pRoot    Subtree root. May be NULL.
 * @param[in] pDestroy Optional element destroy callback.
 */
void RbDestroySubtree(PTREENODE pRoot, CCL_DESTROY_FN pDestroy) {
    if (!pRoot) return;
    RbDestroySubtree(pRoot->left,  pDestroy);
    RbDestroySubtree(pRoot->right, pDestroy);
    if (pDestroy) pDestroy(&pRoot->u);
    RbFreeNode(pRoot);
}
