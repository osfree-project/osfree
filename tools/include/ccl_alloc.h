/*!
 *
 * @file ccl_alloc.h
 *
 * @brief Public allocator installation contract.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. This is a public header.
 *
 * By default ccl allocates through malloc/free. A host environment
 * that provides its own allocator (for example, the SOM runtime with
 * somAlloc/somFree) installs it once at startup with CclSetAllocator,
 * before any ccl container is created. From that point on, every
 * internal ccl allocation goes through the installed hooks.
 *
 * The hooks are process-global because ccl is single threaded. The
 * swap is refused if any ccl allocation is currently live, so that a
 * block allocated by one allocator is never released by another.
 */

#ifndef CCL_ALLOC_H
#define CCL_ALLOC_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @typedef CCL_ALLOC_FN
 * @brief Allocation hook.
 *
 * Must return a pointer to at least ulSize bytes, or NULL on failure.
 * Must not return NULL for a nonzero request unless allocation
 * actually failed.
 *
 * @param[in] ulSize  Number of bytes to allocate.
 *
 * @return The allocated block, or NULL on failure.
 *
 * @retval NULL  Allocation failed.
 */
typedef PVOID (*CCL_ALLOC_FN)(ULONG ulSize);

/*!
 * @typedef CCL_FREE_FN
 * @brief Release hook.
 *
 * Called with a pointer previously returned by the matching
 * CCL_ALLOC_FN. ccl never passes NULL here.
 *
 * @param[in] pPtr  Block to release.
 */
typedef void  (*CCL_FREE_FN) (PVOID pPtr);

/*!
 * @brief Install a custom allocator.
 *
 * Must be called before any ccl container is created. If any ccl
 * allocation is currently live, the call is refused so that a block
 * allocated by one allocator is never released by another.
 *
 * @param[in] pfnAlloc  Allocation hook. Not NULL.
 * @param[in] pfnFree   Release hook. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pfnAlloc or pfnFree is NULL.
 * @retval ERROR_NOT_SUPPORTED      One or more ccl allocations are
 *                                  currently live.
 */
APIRET APIENTRY CclSetAllocator(CCL_ALLOC_FN pfnAlloc, CCL_FREE_FN pfnFree);

/*!
 * @brief Restore the default malloc/free-backed allocator.
 *
 * Same constraints as CclSetAllocator: refused if any ccl
 * allocation is currently live.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_SUPPORTED      One or more ccl allocations are
 *                                  currently live.
 */
APIRET APIENTRY CclResetAllocator(void);

#ifdef __cplusplus
}
#endif

#endif /* CCL_ALLOC_H */
