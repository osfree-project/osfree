/*!
 *
 * @file node_private.h
 *
 * @brief Aligned node allocator for the ccl library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * Used by the containers that store an inline element inside a node
 * (TREENODE in tree_private, HASHNODE in hash_private). The node is
 * allocated from (*CclMalloc) as one block: a pointer-sized header
 * holding the raw allocation, padding to the requested alignment,
 * the fixed part of the node, then the element.
 *
 * Rationale for the layout. The fixed part of the node contains
 * pointer fields (left/right/parent in TREENODE, next in HASHNODE)
 * that must be aligned like a pointer. The element follows the
 * fixed part at pNode + ulFixedPart and must satisfy a stronger
 * alignment (typically that of double, 8 bytes). Aligning the whole
 * block to max(ulAlign, sizeof(PVOID)) and requiring ulFixedPart to
 * be a multiple of ulAlign means pNode satisfies both alignments
 * and pNode + ulFixedPart satisfies ulAlign with no further
 * arithmetic at the call site.
 *
 * 32-bit note. The alignment arithmetic in ccl_node_alloc operates
 * on ULONG-sized values, so the allocator is limited to 32-bit
 * targets (OS/2 and 32-bit host builds). A 64-bit variant will need
 * a wider pointer-sized integer, to be introduced when the need
 * arises; ccl_node_alloc itself will remain the same signature.
 */

#ifndef CCL_NODE_PRIVATE_H
#define CCL_NODE_PRIVATE_H

#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Allocate a node with an inline element area.
 *
 * Allocates a single block from (*CclMalloc) large enough for:
 *
 *   - a pointer-sized header at the very start of the block,
 *     holding the pointer returned by (*CclMalloc);
 *   - padding to reach the required alignment;
 *   - the fixed part of the node (ulFixedPart bytes);
 *   - the inline element (ulElemSize bytes).
 *
 * Requirements:
 *
 *   - ulAlign must be a power of two and at least sizeof(PVOID).
 *     Values below sizeof(PVOID) are silently raised to
 *     sizeof(PVOID), because the fixed part of every ccl node
 *     starts with at least one pointer field. Values that are not
 *     a power of two are a contract violation.
 *   - ulFixedPart must be a multiple of ulAlign. This is what
 *     guarantees that (pNode + ulFixedPart) lands on an ulAlign
 *     boundary when pNode itself does.
 *
 * On return, pNode is aligned to max(ulAlign, sizeof(PVOID)) and
 * (pNode + ulFixedPart) is aligned to ulAlign. Both properties are
 * needed by the callers: pNode for the node's pointer fields,
 * pNode + ulFixedPart for the element (which may require a stronger
 * alignment, such as 8 for a union containing double).
 *
 * Contract violations. In debug builds a failed requirement is an
 * assertion failure. In release builds the function returns NULL,
 * indistinguishable from an allocation failure; callers already
 * handle NULL. This is deliberate: the alternative, silently
 * producing a misaligned node, would cause undefined behaviour at
 * the first access to the element.
 *
 * @param[in] ulFixedPart  Size of the fixed part of the node, in
 *                         bytes. Must be a multiple of ulAlign.
 * @param[in] ulElemSize   Size of the inline element, in bytes.
 * @param[in] ulAlign      Required alignment of the element. Must
 *                         be a power of two; values below
 *                         sizeof(PVOID) are raised to sizeof(PVOID).
 *
 * @return A pointer to the fixed part of the new node, or NULL on
 *         failure.
 *
 * @retval NULL  Allocation failed, ulAlign is not a power of two,
 *               ulFixedPart is not a multiple of ulAlign, or the
 *               requested sizes overflow a ULONG.
 */
PVOID ccl_node_alloc(ULONG ulFixedPart, ULONG ulElemSize, ULONG ulAlign);

/*!
 * @brief Release a node previously allocated by ccl_node_alloc.
 *
 * The pointer passed must be the value returned by ccl_node_alloc
 * (that is, the address of the fixed part, not the raw allocation).
 * The function retrieves the raw pointer from the pointer-sized
 * header stored just before the aligned node and releases it
 * through (*CclFree).
 *
 * NULL is accepted and treated as a no-op.
 *
 * @param[in] pNode  Node to release. May be NULL.
 */
void  ccl_node_free (PVOID pNode);

#ifdef __cplusplus
}
#endif

#endif /* CCL_NODE_PRIVATE_H */
