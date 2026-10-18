/*!
 *
 * @file str_private.c
 *
 * @brief String helpers used by the owned-string profiles.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * See str_private.h for the contract. The four functions here have
 * signatures that match the callback typedefs in ccl_base.h
 * (CCL_HASH_FN, CCL_COMPARE_FN, CCL_DESTROY_FN) or are internal
 * helpers used by strset.c and strmap.c when copying a caller
 * string into the container.
 *
 * The string representation used by all four functions is the same
 * as in hash_private: a key is stored inline as sizeof(PCSZ) bytes
 * holding a PCSZ value. Every callback therefore receives a
 * pointer to the stored bytes of a PCSZ (a PCSZ *), reads the
 * string pointer out of those bytes, and works with the string it
 * points to.
 */

#include <string.h>
#include "str_private.h"
#include "ccl_alloc.h"

/* --------------------------------------------------------------------------
   FNV-1a hash.
   -------------------------------------------------------------------------- */

/*!
 * @brief FNV-1a hash of a NUL-terminated string.
 *
 * pKey points to the stored bytes of a PCSZ. The callback reads the
 * string pointer out of those bytes and hashes the string content.
 * A NULL pKey or a PCSZ pointing to NULL yields 0.
 *
 * FNV-1a (32-bit constants):
 *
 *     hash = 2166136261UL;   offset basis
 *     for each byte b:
 *         hash ^= b;
 *         hash *= 16777619UL; prime
 *
 * Reference: Fowler, Noll, Vo, "FNV hash", 1991.
 *
 * The hash is not required to be portable across platforms.
 * Within one process it is deterministic for the same input,
 * which is all a hash table needs.
 *
 * pCtx is ignored: the hash of a string does not depend on any
 * caller context.
 *
 * @param[in] pKey  Pointer to the stored bytes of a PCSZ.
 * @param[in] pCtx  Caller context. Ignored.
 *
 * @return A 32-bit hash value.
 *
 * @retval 0  pKey is NULL, or the PCSZ stored at pKey is NULL, or
 *            the string is empty.
 */
ULONG fnv1a_str(PCVOID pKey, PVOID pCtx) {
    PCSZ  psz;
    ULONG ulHash = 2166136261UL;

    (void)pCtx;

    if (!pKey) return 0;
    psz = *(PCSZ *)pKey;
    if (!psz) return 0;

    while (*psz) {
        ulHash ^= (ULONG)(unsigned char)*psz;
        ulHash *= 16777619UL;
        psz++;
    }
    return ulHash;
}

/* --------------------------------------------------------------------------
   String comparison.
   -------------------------------------------------------------------------- */

/*!
 * @brief String comparison callback matching CCL_COMPARE_FN.
 *
 * Both pA and pB point to stored bytes of a PCSZ. Reads the string
 * pointers out of those bytes and compares the strings with
 * strcmp. NULL inputs are treated as ordered before non-NULL ones,
 * so that a table that somehow ends up with a NULL string key
 * still yields a total order.
 *
 * pCtx is ignored: comparing two strings does not depend on any
 * caller context.
 *
 * @param[in] pA    Pointer to the stored bytes of the first PCSZ.
 * @param[in] pB    Pointer to the stored bytes of the second PCSZ.
 * @param[in] pCtx  Caller context. Ignored.
 *
 * @return Negative, zero or positive, following strcmp's contract.
 *
 * @retval -1  pA is NULL and pB is non-NULL, or pA's string is
 *             NULL and pB's is non-NULL, or strcmp finds pA < pB.
 * @retval  0  Both are NULL, both strings are NULL, or the strings
 *             are equal.
 * @retval  1  pA is non-NULL and pB is NULL, or pA's string is
 *             non-NULL and pB's is NULL, or strcmp finds pA > pB.
 */
int strcmp_wrap(PCVOID pA, PCVOID pB, PVOID pCtx) {
    PCSZ pszA;
    PCSZ pszB;

    (void)pCtx;

    if (!pA) return (!pB) ? 0 : -1;
    if (!pB) return 1;

    pszA = *(PCSZ *)pA;
    pszB = *(PCSZ *)pB;

    if (!pszA) return (!pszB) ? 0 : -1;
    if (!pszB) return 1;

    return strcmp(pszA, pszB);
}

/* --------------------------------------------------------------------------
   String ownership.
   -------------------------------------------------------------------------- */

/*!
 * @brief Free a string owned by a CCL container.
 *
 * pElem points to the stored bytes of a PCSZ; the string pointer
 * read from those bytes is released through (*CclFree). NULL pElem
 * or a PCSZ pointing to NULL is a no-op.
 *
 * The release goes through (*CclFree), the counterpart of the
 * (*CclMalloc) that str_dup_ccl used to allocate the copy. This is
 * what makes the owned-string profiles work with the SOM runtime
 * allocator: after SOM-init, the copy was allocated by the SOM
 * allocator and is released back to it here.
 *
 * @param[in] pElem  Pointer to the stored bytes of a PCSZ.
 */
void str_free_ccl(PVOID pElem) {
    PCSZ psz;

    if (!pElem) return;
    psz = *(PCSZ *)pElem;
    if (psz) (*CclFree)((void *)psz);
}

/*!
 * @brief Duplicate a string via (*CclMalloc).
 *
 * Returns a copy of psz allocated through the current CclMalloc hook
 * (including the NUL terminator), or NULL on failure or when psz is
 * NULL. The caller owns the copy and must release it through
 * (*CclFree), typically via str_free_ccl on the stored bytes, or by
 * wrapping the pointer in a PCSZ and passing it to (*CclFree)
 * directly.
 *
 * Used by strset.c and strmap.c when copying a caller string into
 * the container (Phase 1.5).
 *
 * @param[in] psz  Source string, or NULL.
 *
 * @return A pointer to the copy, or NULL on failure or psz == NULL.
 *
 * @retval NULL  psz is NULL, or allocation through (*CclMalloc)
 *               failed.
 */
PCSZ str_dup_ccl(PCSZ psz) {
    size_t cbLen;
    PCSZ   pCopy;

    if (!psz) return NULL;

    cbLen = strlen(psz) + 1;
    pCopy = (PCSZ)(*CclMalloc)(cbLen);
    if (!pCopy) return NULL;

    memcpy((void *)pCopy, psz, cbLen);
    return pCopy;
}
