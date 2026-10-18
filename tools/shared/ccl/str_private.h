/*!
 *
 * @file str_private.h
 *
 * @brief String helpers used by the owned-string profiles.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * The four callbacks declared here have signatures that match the
 * callback typedefs in ccl_base.h (CCL_HASH_FN, CCL_COMPARE_FN,
 * CCL_DESTROY_FN). They are used by the strset and strmap profiles
 * (Phase 1.5) to configure a hash_private table so that its keys
 * are stored as owned strings.
 *
 * Key representation. The hash_private containers store keys
 * inline as bytes of size ulKeySize. For string profiles,
 * ulKeySize = sizeof(PCSZ): each key is a PCSZ value stored in
 * place. Every callback therefore receives a pointer to the stored
 * bytes of a PCSZ (a PCSZ *), not the PCSZ itself. The callback
 * reads the string pointer out of those bytes and works with the
 * string it points to.
 */

#ifndef CCL_STR_PRIVATE_H
#define CCL_STR_PRIVATE_H

#include "os2types.h"
#include "ccl_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief FNV-1a hash of a NUL-terminated string.
 *
 * Matches CCL_HASH_FN. pKey points to the stored bytes of a PCSZ;
 * the callback reads the string pointer out of those bytes and
 * hashes the string's content. A NULL pKey or a PCSZ pointing to
 * NULL yields 0.
 *
 * FNV-1a (32-bit constants): offset basis 2166136261, prime
 * 16777619. See Fowler, Noll, Vo, "FNV hash", 1991. The hash is
 * not required to be portable across platforms; within one process
 * it is deterministic for the same input, which is all a hash
 * table needs.
 *
 * @param[in] pKey  Pointer to the stored bytes of a PCSZ.
 * @param[in] pCtx  Caller context. Ignored.
 *
 * @return A 32-bit hash value.
 */
ULONG fnv1a_str(PCVOID pKey, PVOID pCtx);

/*!
 * @brief String comparison callback matching CCL_COMPARE_FN.
 *
 * Both pA and pB point to stored bytes of a PCSZ. Reads the string
 * pointers out of those bytes and compares the strings with
 * strcmp. NULL inputs are treated as ordered before non-NULL ones,
 * so that a table that somehow ends up with a NULL string key
 * still yields a total order.
 *
 * @param[in] pA    Pointer to the stored bytes of the first PCSZ.
 * @param[in] pB    Pointer to the stored bytes of the second PCSZ.
 * @param[in] pCtx  Caller context. Ignored.
 *
 * @return Negative, zero or positive, following strcmp's contract.
 */
int strcmp_wrap(PCVOID pA, PCVOID pB, PVOID pCtx);

/*!
 * @brief Free a string owned by a CCL container.
 *
 * Matches CCL_DESTROY_FN. pElem points to the stored bytes of a
 * PCSZ; the string pointer read from those bytes is released
 * through (*CclFree). NULL pElem or a PCSZ pointing to NULL is a
 * no-op.
 *
 * The release goes through (*CclFree), the counterpart of the
 * (*CclMalloc) that str_dup_ccl used to allocate the copy. This is
 * what makes the owned-string profiles work with the SOM runtime
 * allocator: after SOM-init, the copy was allocated by the SOM
 * allocator and is released back to it here.
 *
 * @param[in] pElem  Pointer to the stored bytes of a PCSZ.
 */
void str_free_ccl(PVOID pElem);

/*!
 * @brief Duplicate a string via (*CclMalloc).
 *
 * Returns a copy of psz allocated through the current CclMalloc hook
 * (including the NUL terminator), or NULL on failure or when psz is
 * NULL. The caller owns the copy and must release it through
 * (*CclFree) (typically via str_free_ccl on the stored bytes, or by
 * wrapping the pointer in a PCSZ and passing it to
 * (*CclFree) directly).
 *
 * Used by strset.c and strmap.c when copying a caller string into
 * the container (Phase 1.5).
 *
 * @param[in] psz  Source string, or NULL.
 *
 * @return A pointer to the copy, or NULL on failure or psz == NULL.
 */
PCSZ str_dup_ccl(PCSZ psz);

#ifdef __cplusplus
}
#endif

#endif /* CCL_STR_PRIVATE_H */
