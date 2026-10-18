/*!
 *
 * @file node_private.c
 *
 * @brief Aligned node allocator for the ccl library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * See node_private.h for the contract. The implementation is
 * intentionally limited to 32-bit targets: the alignment is
 * computed by treating the pointer as a ULONG. A 64-bit variant
 * will need a pointer-sized integer type; the function signature
 * stays the same.
 *
 * Layout of the allocated block:
 *
 *     pRaw                                              pRaw + ulTotal
 *     |                                                              |
 *     v                                                              v
 *     [hdr: PVOID][pad...][pNode -> fixed part][elem...]
 *                         ^
 *                         |
 *                         pBase = pNode
 *
 *   - pRaw is the pointer returned by (*CclMalloc). It is stored in
 *     the pointer-sized header immediately before pNode so that
 *     ccl_node_free can recover it from pNode alone.
 *   - pNode is aligned to ulAlign (and to sizeof(PVOID), which is
 *     why ulAlign is raised to sizeof(PVOID) when smaller).
 *   - ulFixedPart is a multiple of ulAlign, so pNode + ulFixedPart
 *     is also aligned to ulAlign. The inline element is placed
 *     there by the caller.
 */

#include "node_private.h"
#include "ccl_alloc.h"

/*!
 * @brief Allocate a node with an inline element area.
 *
 * @param[in] ulFixedPart  Size of the fixed part of the node, in
 *                         bytes. Must be a multiple of ulAlign.
 * @param[in] ulElemSize   Size of the inline element, in bytes.
 * @param[in] ulAlign      Required alignment of the element.
 *
 * @return A pointer to the fixed part of the new node, or NULL on
 *         failure.
 *
 * @retval NULL  Allocation failed, ulAlign is not a power of two,
 *               ulFixedPart is not a multiple of ulAlign, or the
 *               requested sizes overflow a ULONG.
 */
PVOID ccl_node_alloc(ULONG ulFixedPart, ULONG ulElemSize, ULONG ulAlign) {
    UCHAR *pRaw;
    UCHAR *pBase;
    ULONG  ulTotal;

    /* --------------------------------------------------------------
     * 1. Normalise and validate alignment.
     *
     * Alignments below sizeof(PVOID) are raised to sizeof(PVOID),
     * because the fixed part of every ccl node begins with at least
     * one pointer field. Non-power-of-two alignments are rejected.
     * -------------------------------------------------------------- */
    if (ulAlign < sizeof(PVOID)) ulAlign = sizeof(PVOID);
    if ((ulAlign & (ulAlign - 1)) != 0) return NULL;

    /* --------------------------------------------------------------
     * 2. Validate the fixed part / element offset.
     *
     * The fixed part must be a multiple of ulAlign so that
     * (pNode + ulFixedPart) inherits pNode's alignment. If it is
     * not, the element would end up at a non-aligned address and
     * the caller's assumption about alignment would be wrong.
     * -------------------------------------------------------------- */
    if (ulFixedPart % ulAlign != 0) return NULL;

    /* --------------------------------------------------------------
     * 3. Compute the total size, guarding against overflow.
     *
     * The block must contain:
     *   - a pointer-sized header for the raw allocation;
     *   - up to (ulAlign - 1) bytes of padding to align pNode;
     *   - the fixed part;
     *   - the element.
     *
     * The three overflow checks below make sure the additions do
     * not wrap around. The last one is the most subtle: after
     * adding ulFixedPart + ulElemSize, we must still be able to
     * add (ulAlign - 1) and sizeof(PVOID) without wrapping.
     * -------------------------------------------------------------- */
    if (ulFixedPart > (ULONG)-1 - ulElemSize) return NULL;
    ulTotal = ulFixedPart + ulElemSize;
    if (ulTotal > (ULONG)-1 - (ulAlign - 1) - sizeof(PVOID)) return NULL;
    ulTotal += ulAlign - 1 + sizeof(PVOID);

    /* --------------------------------------------------------------
     * 4. Allocate.
     * -------------------------------------------------------------- */
    pRaw = (UCHAR *)(*CclMalloc)(ulTotal);
    if (!pRaw) return NULL;

    /* --------------------------------------------------------------
     * 5. Compute pBase.
     *
     * pBase is the address of the fixed part. It must be aligned to
     * ulAlign, leaving at least sizeof(PVOID) bytes before it for
     * the header. The expression below rounds up
     * (pRaw + sizeof(PVOID)) to the next multiple of ulAlign.
     *
     * The cast (ULONG)(pRaw + sizeof(PVOID)) is 32-bit safe; see
     * the 32-bit note in node_private.h.
     * -------------------------------------------------------------- */
    pBase = (UCHAR *)((((ULONG)(pRaw + sizeof(PVOID))) + (ulAlign - 1))
                      & ~(ulAlign - 1));

    /* --------------------------------------------------------------
     * 6. Store the raw allocation just before pBase.
     * -------------------------------------------------------------- */
    ((PVOID *)pBase)[-1] = pRaw;

    return pBase;
}

/*!
 * @brief Release a node previously allocated by ccl_node_alloc.
 *
 * The pointer passed must be the value returned by ccl_node_alloc
 * (that is, the address of the fixed part). The raw allocation
 * pointer is read from the pointer-sized header just before pNode.
 *
 * NULL is accepted and treated as a no-op.
 *
 * @param[in] pNode  Node to release. May be NULL.
 */
void ccl_node_free(PVOID pNode) {
    if (!pNode) return;
    (*CclFree)(((PVOID *)pNode)[-1]);
}
