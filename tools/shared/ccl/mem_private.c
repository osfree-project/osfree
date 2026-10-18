/*!
 *
 * @file mem_private.c
 *
 * @brief Internal allocator implementation for the ccl library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * The default hooks wrap malloc/free. A single global hook pair is
 * used because the ccl library is single threaded and OS/2 exposes a
 * single heap. Callers that need the SOM runtime allocator install
 * it once at startup with CclSetAllocator.
 *
 * A live-allocation counter guards the swap: installing a new
 * allocator while blocks from the previous one are still alive would
 * make the matching free run under the wrong allocator, which is
 * undefined behaviour.
 */

#include <stdlib.h>
#include <string.h>
#include "mem_private.h"

/* ==================================================================
 * Default allocator: malloc/free.
 * ================================================================== */

/*!
 * @brief Default allocation hook.
 *
 * @param[in] ulSize  Number of bytes to allocate.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID default_alloc(ULONG ulSize) {
    return (PVOID)malloc((size_t)ulSize);
}

/*!
 * @brief Default release hook.
 *
 * @param[in] pPtr  Block to release.
 */
static void default_free(PVOID pPtr) {
    free(pPtr);
}

/*!
 * @brief Current allocation hook.
 *
 * Initialized to default_alloc at program start. Replaced only
 * through CclSetAllocator. Declared in mem_private.h.
 */
CCL_ALLOC_FN CclAlloc = default_alloc;

/*!
 * @brief Current release hook.
 *
 * Initialized to default_free at program start. Replaced only
 * through CclSetAllocator. Declared in mem_private.h.
 */
CCL_FREE_FN  CclFree  = default_free;

/*!
 * @brief Number of ccl allocations currently live.
 *
 * Incremented by ccl_mem_alloc/ccl_mem_zalloc on success,
 * decremented by ccl_mem_free on a non-NULL pointer. Used to reject
 * allocator swaps that would leave live blocks under the previous
 * allocator.
 */
static ULONG ulLiveAllocations = 0;

/* ==================================================================
 * Allocator installation.
 * ================================================================== */

/*!
 * @brief Install a custom allocator.
 *
 * @param[in] pfnAlloc  Allocation hook. Not NULL.
 * @param[in] pfnFree   Release hook. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pfnAlloc or pfnFree is NULL.
 * @retval ERROR_NOT_SUPPORTED      Live ccl allocations exist.
 */
APIRET APIENTRY CclSetAllocator(CCL_ALLOC_FN pfnAlloc, CCL_FREE_FN pfnFree) {
    if (!pfnAlloc || !pfnFree) return ERROR_INVALID_PARAMETER;
    if (ulLiveAllocations != 0) return ERROR_NOT_SUPPORTED;
    CclAlloc = pfnAlloc;
    CclFree  = pfnFree;
    return NO_ERROR;
}

/*!
 * @brief Restore the default malloc/free-backed allocator.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_SUPPORTED      Live ccl allocations exist.
 */
APIRET APIENTRY CclResetAllocator(void) {
    if (ulLiveAllocations != 0) return ERROR_NOT_SUPPORTED;
    CclAlloc = default_alloc;
    CclFree  = default_free;
    return NO_ERROR;
}

/* ==================================================================
 * Allocation.
 * ================================================================== */

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
PVOID ccl_mem_alloc(ULONG ulSize) {
    PVOID p;

    if (ulSize == 0) return NULL;
    p = CclAlloc(ulSize);
    if (p) ulLiveAllocations++;
    return p;
}

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
PVOID ccl_mem_zalloc(ULONG ulSize) {
    PVOID p = ccl_mem_alloc(ulSize);

    if (p) memset(p, 0, (size_t)ulSize);
    return p;
}

/*!
 * @brief Release a pointer previously returned by ccl_mem_alloc or
 *        ccl_mem_zalloc.
 *
 * NULL is accepted and treated as a no-op.
 *
 * @param[in] pPtr  Block to release. May be NULL.
 */
void ccl_mem_free(PVOID pPtr) {
    if (!pPtr) return;
    CclFree(pPtr);
    ulLiveAllocations--;
}
