/*!
 *
 * @file ccl_alloc.h
 *
 * @brief Public allocator contract, modelled after SOMobjects.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. This is a public header.
 *
 * The allocator is a set of four global function pointers, exactly
 * like SOMMalloc/SOMCalloc/SOMRealloc/SOMFree in SOMobjects (see
 * somapi.h: somTD_SOMMalloc is declared as
 * somToken (SOMLINK *)(size_t)). A host environment that provides
 * its own allocator (for example, the SOM runtime) replaces them by
 * direct assignment at startup:
 *
 *     CclMalloc  = som_malloc_adapter;
 *     CclCalloc  = som_calloc_adapter;
 *     CclRealloc = som_realloc_adapter;
 *     CclFree    = som_free_adapter;
 *
 * There is no setter function. The replacement is the caller's
 * responsibility and must happen before any ccl container is
 * created. The hooks are process-global because ccl is single
 * threaded, and a block allocated by one hook must be released by
 * the matching hook.
 *
 * The default hook implementations (in ccl_alloc.c) are APIENTRY
 * wrappers around the C runtime malloc/calloc/realloc/free. Wrappers
 * are needed because the hook type uses the APIENTRY calling
 * convention (APIENTRY == _System == SOMLINK on OS/2) while
 * malloc/calloc/realloc/free use the default C calling convention.
 * Assigning malloc directly to CclMalloc would mismatch calling
 * conventions and is undefined behaviour.
 */

#ifndef CCL_ALLOC_H
#define CCL_ALLOC_H

#include <stddef.h>
#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @typedef CCL_MALLOC_FN
 * @brief Allocation hook, modelled after SOMMalloc.
 *
 * Must return a pointer to at least cbSize bytes, or NULL on
 * failure.
 *
 * @param[in] cbSize  Number of bytes to allocate.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
typedef PVOID (APIENTRY *CCL_MALLOC_FN)(size_t cbSize);

/*!
 * @typedef CCL_CALLOC_FN
 * @brief Zero-initialising allocation hook, modelled after
 *        SOMCalloc.
 *
 * Must return a pointer to at least cbNum * cbSize zero-filled
 * bytes, or NULL on failure.
 *
 * @param[in] cbNum   Number of elements.
 * @param[in] cbSize  Size of one element in bytes.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
typedef PVOID (APIENTRY *CCL_CALLOC_FN)(size_t cbNum, size_t cbSize);

/*!
 * @typedef CCL_REALLOC_FN
 * @brief Resize hook, modelled after SOMRealloc.
 *
 * Must return a pointer to at least cbSize bytes holding the same
 * content as pPtr (up to the smaller of the old and new sizes), or
 * NULL on failure. On success the original block is consumed. On
 * failure the original block is unchanged.
 *
 * @param[in] pPtr    Previously allocated block, or NULL.
 * @param[in] cbSize  New size in bytes.
 *
 * @return The resized block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
typedef PVOID (APIENTRY *CCL_REALLOC_FN)(PVOID pPtr, size_t cbSize);

/*!
 * @typedef CCL_FREE_FN
 * @brief Release hook, modelled after SOMFree.
 *
 * @param[in] pPtr  Block to release.
 */
typedef void  (APIENTRY *CCL_FREE_FN)(PVOID pPtr);

/*!
 * @brief Current allocation hook.
 *
 * Defaults to ccl_default_malloc, an APIENTRY wrapper around
 * malloc. Declared here, defined in ccl_alloc.c. Replaceable by
 * direct assignment before any ccl allocation is made.
 */
extern CCL_MALLOC_FN  CclMalloc;

/*!
 * @brief Current zero-initialising allocation hook.
 *
 * Defaults to ccl_default_calloc, an APIENTRY wrapper around
 * calloc. Declared here, defined in ccl_alloc.c. Replaceable by
 * direct assignment before any ccl allocation is made.
 */
extern CCL_CALLOC_FN  CclCalloc;

/*!
 * @brief Current resize hook.
 *
 * Defaults to ccl_default_realloc, an APIENTRY wrapper around
 * realloc. Declared here, defined in ccl_alloc.c. Replaceable by
 * direct assignment before any ccl allocation is made.
 */
extern CCL_REALLOC_FN CclRealloc;

/*!
 * @brief Current release hook.
 *
 * Defaults to ccl_default_free, an APIENTRY wrapper around free.
 * Declared here, defined in ccl_alloc.c. Replaceable by direct
 * assignment before any ccl allocation is made.
 */
extern CCL_FREE_FN    CclFree;

#ifdef __cplusplus
}
#endif

#endif /* CCL_ALLOC_H */
