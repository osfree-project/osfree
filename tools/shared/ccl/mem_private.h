/*!
 *
 * @file mem_private.h
 *
 * @brief Internal allocator interface for the ccl library (C89).
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * The public allocator contract (CCL_ALLOC_FN, CCL_FREE_FN,
 * CclSetAllocator, CclResetAllocator) lives in ccl_alloc.h. This
 * header adds the internal helpers and the current hook variables
 * used only by the library itself.
 *
 * All ccl internal allocations go through ccl_mem_alloc,
 * ccl_mem_zalloc and ccl_mem_free: control blocks, element storage
 * arrays, link nodes, and any other structure owned by a ccl
 * container. The user data stored inside a generic container
 * (payload of legacy Dlist, SOM objects inside list, strings inside
 * strset) is a separate concern and follows the ownership rules of
 * each container.
 */

#ifndef CCL_MEM_PRIVATE_H
#define CCL_MEM_PRIVATE_H

#include "os2types.h"
#include "os2err.h"
#include "ccl_alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Current allocation hook.
 *
 * Initialized to a malloc wrapper at program start. Replaced only
 * through CclSetAllocator. Used by mem_private.c; declared here so
 * that any library source can inspect the current allocator if
 * needed.
 */
extern CCL_ALLOC_FN CclAlloc;

/*!
 * @brief Current release hook.
 *
 * Initialized to a free wrapper at program start. Replaced only
 * through CclSetAllocator.
 */
extern CCL_FREE_FN  CclFree;

/*!
 * @brief Allocate ulSize bytes via CclAlloc.
 *
 * @param[in] ulSize  Number of bytes.
 *
 * @return The allocated block, or NULL on failure or when ulSize is
 *         zero.
 *
 * @retval NULL  Allocation failed, or ulSize is zero.
 */
PVOID ccl_mem_alloc(ULONG ulSize);

/*!
 * @brief Allocate ulSize zero-filled bytes via CclAlloc.
 *
 * @param[in] ulSize  Number of bytes.
 *
 * @return The allocated block, or NULL on failure or when ulSize is
 *         zero.
 *
 * @retval NULL  Allocation failed, or ulSize is zero.
 */
PVOID ccl_mem_zalloc(ULONG ulSize);

/*!
 * @brief Release a pointer previously returned by ccl_mem_alloc or
 *        ccl_mem_zalloc.
 *
 * NULL is accepted and treated as a no-op.
 *
 * @param[in] pPtr  Block to release. May be NULL.
 */
void  ccl_mem_free(PVOID pPtr);

#ifdef __cplusplus
}
#endif

#endif /* CCL_MEM_PRIVATE_H */
