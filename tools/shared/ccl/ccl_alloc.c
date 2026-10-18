/*!
 *
 * @file ccl_alloc.c
 *
 * @brief Default allocator hooks for the ccl library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. This file provides the default
 * values for the four hooks declared in ccl_alloc.h, wrapping the C
 * runtime malloc/calloc/realloc/free. They may be replaced by direct
 * assignment before the first ccl container is created, exactly as
 * SOMMalloc/SOMCalloc/SOMRealloc/SOMFree in SOMobjects.
 */

#include <stdlib.h>
#include "ccl_alloc.h"

/*!
 * @brief Default allocation hook. Wraps malloc.
 *
 * @param[in] ulSize  Number of bytes to allocate.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID default_malloc(ULONG ulSize) {
    return (PVOID)malloc((size_t)ulSize);
}

/*!
 * @brief Default zero-initialising allocation hook. Wraps calloc.
 *
 * @param[in] ulNum   Number of elements.
 * @param[in] ulSize  Size of one element in bytes.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID default_calloc(ULONG ulNum, ULONG ulSize) {
    return (PVOID)calloc((size_t)ulNum, (size_t)ulSize);
}

/*!
 * @brief Default resize hook. Wraps realloc.
 *
 * @param[in] pPtr    Previously allocated block, or NULL.
 * @param[in] ulSize  New size in bytes.
 *
 * @return The resized block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID default_realloc(PVOID pPtr, ULONG ulSize) {
    return (PVOID)realloc(pPtr, (size_t)ulSize);
}

/*!
 * @brief Default release hook. Wraps free.
 *
 * @param[in] pPtr  Block to release.
 */
static void default_free(PVOID pPtr) {
    free(pPtr);
}

/*!
 * @brief Current allocation hook. Defaults to malloc.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_MALLOC_FN  CclMalloc  = default_malloc;

/*!
 * @brief Current zero-initialising allocation hook. Defaults to
 *        calloc.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_CALLOC_FN  CclCalloc  = default_calloc;

/*!
 * @brief Current resize hook. Defaults to realloc.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_REALLOC_FN CclRealloc = default_realloc;

/*!
 * @brief Current release hook. Defaults to free.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_FREE_FN    CclFree    = default_free;
