/*!
 *
 * @file tree_private.h
 *
 * @brief Internal red-black tree engine used by rbtree.c.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * Provides the single red-black tree engine behind rbtree.c. The
 * engine implements node allocation, rotations, insert-fixup,
 * delete-fixup, lookup, min/max, successor/predecessor and subtree
 * destruction. The structural surgery of delete (splice y out,
 * reconnect x, swap payloads) is done by rbtree.c because it needs
 * the container's element size and destroy callback; tree_private
 * only provides RbDeleteFixup for the fixup phase.
 *
 * Node layout. TREENODE carries left/right/parent pointers, a
 * red/black flag, and a union that provides the element area and
 * its required alignment. The union contains a double, so
 * offsetof(u) is naturally aligned to 8 on every supported target:
 * 16 on 32-bit, 32 on 64-bit. Both offsets satisfy ccl_node_alloc's
 * ulFixedPart % ulAlign == 0 requirement when ulAlign = 8.
 *
 * Element area. The engine treats u.c as the start of the element
 * and stores exactly ulElemSize bytes there. The comparator and
 * destroy callback receive a pointer to those bytes. The engine
 * itself never interprets them.
 */

#ifndef CCL_TREE_PRIVATE_H
#define CCL_TREE_PRIVATE_H

#include <stddef.h>
#include "os2types.h"
#include "ccl_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @def RB_RED
 * @brief Red node flag. Value: 1.
 */
#define RB_RED   1UL

/*!
 * @def RB_BLACK
 * @brief Black node flag. Value: 0.
 */
#define RB_BLACK 0UL

/*!
 * @struct _TREENODE
 * @brief One node in a red-black tree.
 *
 * Element bytes live in the union u. The union contains a double,
 * which forces 8-byte alignment of u. offsetof(u) is 16 on 32-bit
 * and 32 on 64-bit; both are multiples of 8, so ccl_node_alloc
 * accepts (offsetof(TREENODE, u), ulElemSize, 8) and returns a
 * node whose u is correctly aligned.
 */
typedef struct _TREENODE {
    struct _TREENODE *left;   /*!< Left child, or NULL.           */
    struct _TREENODE *right;  /*!< Right child, or NULL.          */
    struct _TREENODE *parent; /*!< Parent, or NULL for the root.  */
    ULONG             ulRed;  /*!< RB_RED or RB_BLACK.            */
    union {
        double d;             /*!< Forces 8-byte alignment of u.  */
        void  *p;
        long   l;
        char   c[1];
    } u;                      /*!< Element area (ulElemSize bytes). */
} TREENODE, *PTREENODE;

/* ==================================================================
 * Node allocation
 * ================================================================== */

/*!
 * @brief Allocate a new detached tree node with an element area.
 *
 * Calls ccl_node_alloc(offsetof(TREENODE, u), ulElemSize, 8). The
 * returned node has left, right, parent and ulRed initialized to
 * zero (NULL / NULL / NULL / RB_BLACK); the caller is expected to
 * populate u.c with the element bytes and set the node's color to
 * RB_RED before calling RbInsert.
 *
 * @param[in] ulElemSize  Size of one element in bytes. Must be > 0.
 *
 * @return A new node, or NULL on allocation failure.
 *
 * @retval NULL  Allocation failed.
 */
PTREENODE RbAllocNode(ULONG ulElemSize);

/*!
 * @brief Release a node allocated by RbAllocNode.
 *
 * Does not touch the element area. NULL is accepted and treated as
 * a no-op.
 *
 * @param[in] pNode  Node to release. May be NULL.
 */
void RbFreeNode(PTREENODE pNode);

/* ==================================================================
 * Rotations
 * ================================================================== */

/*!
 * @brief Rotate the subtree rooted at pX to the left.
 *
 * Standard CLRS rotate-left. pX must have a non-NULL right child.
 * Updates ppRoot when the rotation moves the tree root.
 *
 * @param[in,out] ppRoot  Pointer to the tree's root. Not NULL.
 * @param[in]     pX      Node to rotate around. Must have a right
 *                        child.
 */
void RbRotateLeft(PTREENODE *ppRoot, PTREENODE pX);

/*!
 * @brief Rotate the subtree rooted at pX to the right.
 *
 * Mirror of RbRotateLeft. pX must have a non-NULL left child.
 *
 * @param[in,out] ppRoot  Pointer to the tree's root. Not NULL.
 * @param[in]     pX      Node to rotate around. Must have a left
 *                        child.
 */
void RbRotateRight(PTREENODE *ppRoot, PTREENODE pX);

/* ==================================================================
 * Insert
 * ================================================================== */

/*!
 * @brief Insert a node and restore the red-black invariants.
 *
 * The node's u.c area must already contain the element bytes, and
 * ulRed is set to RB_RED. The engine does the BST descent and the
 * insert-fixup. Behaviour on duplicate keys depends on pfReplaced:
 *
 *   - If pfReplaced is non-NULL (unique mode) and a node with the
 *     same key exists, that node is returned with *pfReplaced =
 *     TRUE; pNode is NOT linked (the caller is responsible for
 *     freeing it).
 *   - Otherwise (pfReplaced is NULL, or no duplicate exists), pNode
 *     is linked into the tree, *pfReplaced (if non-NULL) is set to
 *     FALSE, and pNode is returned. In duplicate-allowed mode the
 *     new node is placed to the right of every existing node with
 *     the same key, so that RbSuccessor walks duplicates in
 *     insertion order.
 *
 * @param[in,out] ppRoot      Pointer to the tree's root. Not NULL.
 * @param[in]     pNode       Node to insert. Not NULL. Its element
 *                            area must already be populated.
 * @param[in]     pCompare    Comparator. Not NULL.
 * @param[out]    pfReplaced  Optional. Receives TRUE if an existing
 *                            node with the same key was found (and
 *                            pNode was not linked).
 *
 * @return The linked node, or the pre-existing duplicate node.
 *
 * @retval pNode  New node was linked.
 * @retval other  Existing node with the same key (only when
 *                pfReplaced is non-NULL).
 */
PTREENODE RbInsert(PTREENODE *ppRoot, PTREENODE pNode,
                   CCL_COMPARE_FN pCompare, PBOOL pfReplaced);

/* ==================================================================
 * Delete fixup
 * ================================================================== */

/*!
 * @brief Restore the red-black invariants after a structural
 *        splice.
 *
 * The caller has already:
 *   - identified the node y to be removed (either z or
 *     RbSuccessor(z));
 *   - spliced y out, reconnecting x in its place (x may be NULL);
 *   - captured y's parent as pXParent and y's side as fXIsLeft
 *     before the splice;
 *   - optionally swapped the element bytes between z and y.
 *
 * This function only runs the CLRS delete-fixup loop and updates
 * the root through ppRoot. It does NOT free y and does NOT call
 * pDestroyValue; both are the caller's responsibility after the
 * fixup returns.
 *
 * @param[in,out] ppRoot     Pointer to the tree's root. Not NULL.
 * @param[in]     pX         Node that replaced y, or NULL.
 * @param[in]     pXParent   y's parent before the splice, or NULL
 *                           if y was the root.
 * @param[in]     fXIsLeft   TRUE if y was the left child of
 *                           pXParent before the splice, FALSE
 *                           otherwise.
 */
void RbDeleteFixup(PTREENODE *ppRoot, PTREENODE pX,
                   PTREENODE pXParent, BOOL fXIsLeft);

/* ==================================================================
 * Lookup and navigation
 * ================================================================== */

/*!
 * @brief Find a node by key.
 *
 * Walks down from pRoot using pCompare(&node->u, pKey, NULL).
 * Returns the first node that compares equal, or NULL if no such
 * node exists. When the tree contains duplicates, the first one
 * encountered along the descent is returned; use RbSuccessor to
 * walk the remaining duplicates.
 *
 * @param[in] pRoot     Tree root. May be NULL.
 * @param[in] pKey      Pointer to the key bytes. Not NULL.
 * @param[in] pCompare  Comparator. Not NULL.
 *
 * @return The matching node, or NULL.
 *
 * @retval NULL  No node matches.
 */
PTREENODE RbFind(PTREENODE pRoot, PCVOID pKey, CCL_COMPARE_FN pCompare);

/*!
 * @brief Return the leftmost node of the subtree rooted at pNode.
 *
 * @param[in] pNode  Subtree root. Not NULL.
 *
 * @return The leftmost node.
 */
PTREENODE RbMin(PTREENODE pNode);

/*!
 * @brief Return the rightmost node of the subtree rooted at pNode.
 *
 * @param[in] pNode  Subtree root. Not NULL.
 *
 * @return The rightmost node.
 */
PTREENODE RbMax(PTREENODE pNode);

/*!
 * @brief Return the in-order successor of pNode, or NULL.
 *
 * @param[in] pNode  Node. Not NULL.
 *
 * @return The successor, or NULL if pNode is the maximum.
 *
 * @retval NULL  pNode is the maximum.
 */
PTREENODE RbSuccessor(PTREENODE pNode);

/*!
 * @brief Return the in-order predecessor of pNode, or NULL.
 *
 * @param[in] pNode  Node. Not NULL.
 *
 * @return The predecessor, or NULL if pNode is the minimum.
 *
 * @retval NULL  pNode is the minimum.
 */
PTREENODE RbPredecessor(PTREENODE pNode);

/* ==================================================================
 * Subtree destruction
 * ================================================================== */

/*!
 * @brief Release every node in the subtree rooted at pRoot.
 *
 * Walks the subtree in post-order. For each node, if pDestroy is
 * non-NULL, it is called on the node's element bytes (&node->u)
 * before the node is freed through RbFreeNode. If pDestroy is
 * NULL, the nodes are freed without touching their element area.
 *
 * NULL pRoot is accepted and treated as a no-op.
 *
 * @param[in] pRoot    Subtree root. May be NULL.
 * @param[in] pDestroy Optional element destroy callback.
 */
void RbDestroySubtree(PTREENODE pRoot, CCL_DESTROY_FN pDestroy);

#ifdef __cplusplus
}
#endif

#endif /* CCL_TREE_PRIVATE_H */
