/*!
 *
 * @file ccl_alloc.c
 *
 * @brief Default allocator hooks for the ccl library.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * This file provides the default values for the four hooks declared
 * in ccl_alloc.h. Each default is an APIENTRY wrapper around the
 * matching C runtime function, so that the calling convention of the
 * hook (APIENTRY == _System == SOMLINK on OS/2) matches the hook
 * type even though malloc/calloc/realloc/free use the default C
 * calling convention.
 *
 * This is the only translation unit in the new ccl code that calls
 * malloc/calloc/realloc/free directly. The legacy dlist.c and
 * all_dlist.c modules keep their own malloc/free calls per their
 * legacy contract (see ccl_conventions.md).
 */

#include <stdlib.h>
#include "ccl_alloc.h"

/* ==================================================================
 * Default hook implementations.
 *
 * Each default is an APIENTRY wrapper around the C runtime
 * equivalent. Wrappers are needed because CCL_MALLOC_FN and the
 * other hook types are declared with the APIENTRY calling
 * convention, while malloc/calloc/realloc/free use the default C
 * calling convention. Assigning malloc directly to CclMalloc would
 * mismatch calling conventions and is undefined behaviour.
 * ================================================================== */

/*!
 * @brief Default allocation hook. Wraps malloc.
 *
 * @param[in] cbSize  Number of bytes to allocate.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID APIENTRY ccl_default_malloc(size_t cbSize) {
    return (PVOID)malloc(cbSize);
}

/*!
 * @brief Default zero-initialising allocation hook. Wraps calloc.
 *
 * @param[in] cbNum   Number of elements.
 * @param[in] cbSize  Size of one element in bytes.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID APIENTRY ccl_default_calloc(size_t cbNum, size_t cbSize) {
    return (PVOID)calloc(cbNum, cbSize);
}

/*!
 * @brief Default resize hook. Wraps realloc.
 *
 * @param[in] pPtr    Previously allocated block, or NULL.
 * @param[in] cbSize  New size in bytes.
 *
 * @return The resized block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
static PVOID APIENTRY ccl_default_realloc(PVOID pPtr, size_t cbSize) {
    return (PVOID)realloc(pPtr, cbSize);
}

/*!
 * @brief Default release hook. Wraps free.
 *
 * @param[in] pPtr  Block to release.
 */
static void APIENTRY ccl_default_free(PVOID pPtr) {
    free(pPtr);
}

/* ==================================================================
 * Current hooks.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment before
 * any ccl allocation is made. See ccl_alloc.h for the replacement
 * protocol and the SOM-init example.
 * ================================================================== */

/*!
 * @brief Current allocation hook. Defaults to ccl_default_malloc.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_MALLOC_FN  CclMalloc  = ccl_default_malloc;

/*!
 * @brief Current zero-initialising allocation hook. Defaults to
 *        ccl_default_calloc.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_CALLOC_FN  CclCalloc  = ccl_default_calloc;

/*!
 * @brief Current resize hook. Defaults to ccl_default_realloc.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_REALLOC_FN CclRealloc = ccl_default_realloc;

/*!
 * @brief Current release hook. Defaults to ccl_default_free.
 *
 * Declared in ccl_alloc.h. Replaceable by direct assignment.
 */
CCL_FREE_FN    CclFree    = ccl_default_free;
