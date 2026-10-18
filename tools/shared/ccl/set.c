/*!
 *
 * @file set.c
 *
 * @brief Implementation of the set container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * The set is a thin profile over the hash_private engine with
 * ulValueSize = 0. All key storage, bucketing, chaining, rehashing
 * and lookup logic lives in hash_private; this file owns the
 * SETCTL control block, validates the caller's CCLOPTS with the
 * set-specific allow/require mask pair, and translates every
 * operation to the corresponding Hash* function.
 *
 * Destroy callback mapping. In a set, the element is the key; there
 * is no separate value. hash_private only fires pDestroyValue when
 * ulValueSize > 0, so a set that stored its callback in
 * CCLOPTS.pDestroyValue would never see it invoked. Instead, this
 * file maps pOpts->pDestroyValue into the engine's pDestroyKey:
 * HashPut (on replace) and HashRemove (on removal) fire pDestroyKey
 * on the stored key bytes, which is exactly what the set caller
 * asked for via pDestroyValue.
 *
 * The ALLOW_DUPLICATES distinction is made in SetAdd:
 *   - without the flag, HashPut is used (replace-on-existing is a
 *     no-op for ulValueSize = 0 with no pDestroyKey? No: the
 *     pDestroyKey above is the mapped destroy; on replace it fires
 *     on the *new* key, which is exactly the semantics callers
 *     expect from a set: the redundant copy is released);
 *   - with the flag, HashPutAppend is used (unconditional append).
 */

#include <string.h>
#include "set.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"
#include "hash_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_SET_MAGIC
 * @brief Magic value identifying a valid set control block.
 *
 * The literal bytes are "SET " (with a trailing space, because the
 * "SENU" magic is already taken by the legacy StrSetEnum cursor in
 * strset.c).
 */
#define CCL_SET_MAGIC 0x53455420UL  /* "SET " */

/*!
 * @struct _SETCTL
 * @brief Control block of an open set.
 */
typedef struct _SETCTL {
    unsigned long ulMagic;   /*!< CCL_SET_MAGIC.                  */
    ULONG         ulFlags;   /*!< Copy of CCLOPTS.ulFlags.        */
    HASHCTL       engine;    /*!< Embedded hash_private control.  */
} SETCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a set handle.
 *
 * @param[in] hSet  Set handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hSet is NULL, or its magic value does not match.
 */
static SETCTL *get_ctl(HSET hSet) {
    return (SETCTL *)ccl_validate(hSet, CCL_SET_MAGIC);
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new set.
 *
 * @param[in]  ulKeySize         Size of one key in bytes.
 * @param[in]  ulInitialCapacity Initial bucket count, or 0.
 * @param[in]  pOpts             Option block. May be NULL.
 * @param[out] phSet             Receives the new handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument or invalid pOpts.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY SetCreate(ULONG ulKeySize, ULONG ulInitialCapacity,
                          PCCLOPTS pOpts, PHHSET phSet) {
    SETCTL *pCtl;
    APIRET rc;
    CCLOPTS hashOpts;

    if (!phSet) return ERROR_INVALID_PARAMETER;
    if (ulKeySize == 0) return ERROR_INVALID_PARAMETER;
    *phSet = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_COMPARE
                         | CCL_OPT_HASH
                         | CCL_OPT_KEYFN
                         | CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_REHASH
                         | CCL_OPT_FLAG_DUPLICATES
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         CCL_OPT_HASH);
    if (rc != NO_ERROR) return rc;

    /* Build a hash-engine options block: the set's pDestroyValue is
       the destroy for the stored key bytes, because there is no
       separate value. Passing it through pDestroyKey makes
       HashPut/HashRemove fire it correctly. */
    CclOptsInit(&hashOpts);
    hashOpts.pHash             = pOpts->pHash;
    hashOpts.pCompare          = pOpts->pCompare;
    hashOpts.pDestroyKey       = pOpts->pDestroyValue;
    hashOpts.pDestroyValue     = NULL;
    hashOpts.ulRehashThreshold = pOpts->ulRehashThreshold;

    pCtl = (SETCTL *)(*CclCalloc)(1, sizeof(SETCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    rc = HashCreate(ulInitialCapacity, ulKeySize, 0, &hashOpts,
                    &pCtl->engine);
    if (rc != NO_ERROR) {
        (*CclFree)(pCtl);
        return rc;
    }

    pCtl->ulMagic = CCL_SET_MAGIC;
    pCtl->ulFlags = pOpts->ulFlags;

    *phSet = (HSET)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a set.
 *
 * @param[in] hSet  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY SetDestroy(HSET hSet) {
    SETCTL *pCtl;

    if (hSet == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Drain via HashRemove so the mapped destroy (pDestroyKey in
       the engine) fires exactly once per entry. HashDestroy itself
       does not fire any destroy callback. */
    if (pCtl->engine.pDestroyKey) {
        ULONG ulKeySize = pCtl->engine.ulKeySize;
        UCHAR *pbKey = (UCHAR *)(*CclMalloc)(ulKeySize);
        if (pbKey) {
            for (;;) {
                ULONG ulCount = 0;
                HashGetCount(&pCtl->engine, &ulCount);
                if (ulCount == 0) break;
                if (HashGetIthKey(&pCtl->engine, 0, pbKey, ulKeySize,
                                  NULL) != NO_ERROR) break;
                if (HashRemove(&pCtl->engine, pbKey, NULL) != NO_ERROR)
                    break;
            }
            (*CclFree)(pbKey);
        }
    }

    HashDestroy(&pCtl->engine);
    pCtl->ulMagic = 0;
    (*CclFree)(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Adding entries
 * ================================================================== */

/*!
 * @brief Insert a key.
 *
 * @param[in]  hSet     Handle.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pfAdded  Optional. Receives TRUE if a new occurrence
 *                      was added.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY SetAdd(HSET hSet, PCVOID pKey, PBOOL pfAdded) {
    SETCTL *pCtl;
    APIRET rc;
    BOOL fReplaced = FALSE;

    if (!pKey) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pCtl->ulFlags & CCL_FLAG_ALLOW_DUPLICATES) {
        rc = HashPutAppend(&pCtl->engine, pKey, NULL);
        if (rc == NO_ERROR && pfAdded) *pfAdded = TRUE;
        return rc;
    }

    rc = HashPut(&pCtl->engine, pKey, NULL, &fReplaced);
    if (rc == NO_ERROR && pfAdded) *pfAdded = (fReplaced ? FALSE : TRUE);
    return rc;
}

/* ==================================================================
 * Lookup
 * ================================================================== */

/*!
 * @brief Test whether a key is present.
 *
 * @param[in]  hSet      Handle.
 * @param[in]  pKey      Pointer to the key bytes. Not NULL.
 * @param[out] pfPresent Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet, pKey or pfPresent is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetContains(HSET hSet, PCVOID pKey, PBOOL pfPresent) {
    SETCTL *pCtl;
    ULONG ulUsed = 0;

    if (!pKey || !pfPresent) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pfPresent = FALSE;
    if (HashGet(&pCtl->engine, pKey, NULL, 0, &ulUsed) == NO_ERROR)
        *pfPresent = TRUE;
    return NO_ERROR;
}

/* ==================================================================
 * Removing entries
 * ================================================================== */

/*!
 * @brief Remove one occurrence of a key.
 *
 * @param[in]  hSet       Handle.
 * @param[in]  pKey       Pointer to the key bytes. Not NULL.
 * @param[out] pfRemoved  Optional. Receives TRUE if removed.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_PARAMETER  hSet or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetRemove(HSET hSet, PCVOID pKey, PBOOL pfRemoved) {
    SETCTL *pCtl;

    if (!pKey) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashRemove(&pCtl->engine, pKey, pfRemoved);
}

/*!
 * @brief Remove every entry from the set.
 *
 * @param[in] hSet  Handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetClear(HSET hSet) {
    SETCTL *pCtl;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pCtl->engine.pDestroyKey) {
        ULONG ulKeySize = pCtl->engine.ulKeySize;
        UCHAR *pbKey = (UCHAR *)(*CclMalloc)(ulKeySize);
        if (pbKey) {
            for (;;) {
                ULONG ulCount = 0;
                HashGetCount(&pCtl->engine, &ulCount);
                if (ulCount == 0) break;
                if (HashGetIthKey(&pCtl->engine, 0, pbKey, ulKeySize,
                                  NULL) != NO_ERROR) break;
                if (HashRemove(&pCtl->engine, pbKey, NULL) != NO_ERROR)
                    break;
            }
            (*CclFree)(pbKey);
        }
    } else {
        /* No destroy callback: rebuild the engine with the same
           parameters. */
        CCLOPTS opts;
        ULONG ulOldKey = pCtl->engine.ulKeySize;
        ULONG ulOldThr = pCtl->engine.ulRehashThreshold;
        CCL_HASH_FN pHash       = pCtl->engine.pHash;
        CCL_COMPARE_FN pCompare = pCtl->engine.pCompare;

        HashDestroy(&pCtl->engine);
        CclOptsInit(&opts);
        opts.pHash             = pHash;
        opts.pCompare          = pCompare;
        opts.ulRehashThreshold = ulOldThr;
        HashCreate(0, ulOldKey, 0, &opts, &pCtl->engine);
    }
    return NO_ERROR;
}

/* ==================================================================
 * Metadata
 * ================================================================== */

/*!
 * @brief Return the number of entries.
 *
 * @param[in]  hSet      Handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetGetCount(HSET hSet, PULONG pulCount) {
    SETCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetCount(&pCtl->engine, pulCount);
}

/*!
 * @brief Return the current number of buckets.
 *
 * @param[in]  hSet      Handle.
 * @param[out] pulCount  Receives the bucket count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY SetGetBucketCount(HSET hSet, PULONG pulCount) {
    SETCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetBucketCount(&pCtl->engine, pulCount);
}

/*!
 * @brief Force a rehash to the requested bucket count.
 *
 * @param[in] hSet          Handle.
 * @param[in] ulNewCapacity Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_INVALID_PARAMETER  The set is not empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET APIENTRY SetSetBucketCount(HSET hSet, ULONG ulNewCapacity) {
    SETCTL *pCtl;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashSetBucketCount(&pCtl->engine, ulNewCapacity);
}

/* ==================================================================
 * Ordered access
 * ================================================================== */

/*!
 * @brief Copy the key of the i-th entry into the caller's buffer.
 *
 * @param[in]  hSet     Handle.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hSet is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY SetGetIth(HSET hSet, ULONG ulIndex,
                          PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    SETCTL *pCtl;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetIthKey(&pCtl->engine, ulIndex, pBuf, ulSize, pulUsed);
}
