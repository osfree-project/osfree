/*!
 *
 * @file hashtable.c
 *
 * @brief Implementation of the hashtable container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * The hashtable is a thin profile over the hash_private engine
 * (hash_private.h / hash_private.c). All key/value storage,
 * bucketing, chaining, rehashing and lookup logic lives in
 * hash_private; this file only:
 *
 *   - owns the HTABLECTL control block;
 *   - validates the caller's CCLOPTS with the hashtable-specific
 *     allow/require mask pair;
 *   - delegates every operation to the corresponding Hash*
 *     function;
 *   - translates the ccl magic-check and error codes.
 *
 * Because the profile does not allocate any storage of its own
 * beyond the control block, it does not contain any (*CclMalloc)
 * or (*CclFree) calls for the storage; only for the control block
 * (via (*CclCalloc)/(*CclFree)) and indirectly through the
 * hash_private engine. The CI rule from ccl_conventions.md §12
 * (no CclRealloc / ccl_node_alloc outside core modules) is
 * therefore satisfied.
 */

#include <string.h>
#include "hashtable.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"
#include "hash_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_HTBL_MAGIC
 * @brief Magic value identifying a valid hashtable control block.
 *
 * The literal bytes are "HTBL".
 */
#define CCL_HTBL_MAGIC 0x4854424CUL  /* "HTBL" */

/*!
 * @struct _HTABLECTL
 * @brief Control block of an open hashtable.
 *
 * The engine field embeds the hash_private control block. Its
 * magic (CCL_HASH_MAGIC) is separate from HTABLECTL.ulMagic and is
 * checked by the Hash* functions themselves; HTABLECTL.ulMagic is
 * what protects the public handle from being passed to the wrong
 * container.
 */
typedef struct _HTABLECTL {
    unsigned long ulMagic;   /*!< CCL_HTBL_MAGIC.                 */
    ULONG         ulFlags;   /*!< Copy of CCLOPTS.ulFlags.        */
    HASHCTL       engine;    /*!< Embedded hash_private control.  */
} HTABLECTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a hashtable handle.
 *
 * @param[in] hTable  Hashtable handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hTable is NULL, or its magic value does not match.
 */
static HTABLECTL *get_ctl(HHASHTABLE hTable) {
    return (HTABLECTL *)ccl_validate(hTable, CCL_HTBL_MAGIC);
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new hashtable.
 *
 * @param[in]  ulKeySize         Size of one key in bytes.
 * @param[in]  ulValueSize       Size of one value in bytes.
 * @param[in]  ulInitialCapacity Initial bucket count, or 0.
 * @param[in]  pOpts             Option block. May be NULL.
 * @param[out] phTable           Receives the new handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument, zero key/value
 *                                  size, or pOpts lacks pHash or
 *                                  pCompare.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HashtableCreate(ULONG ulKeySize, ULONG ulValueSize,
                                ULONG ulInitialCapacity,
                                PCCLOPTS pOpts,
                                PHHASHTABLE phTable) {
    HTABLECTL *pCtl;
    APIRET rc;

    if (!phTable) return ERROR_INVALID_PARAMETER;
    if (ulKeySize == 0) return ERROR_INVALID_PARAMETER;
    if (ulValueSize == 0) return ERROR_INVALID_PARAMETER;
    *phTable = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_COMPARE
                         | CCL_OPT_HASH
                         | CCL_OPT_DESTROY_KEY
                         | CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_REHASH
                         | CCL_OPT_FLAG_DUPLICATES
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         CCL_OPT_COMPARE | CCL_OPT_HASH);
    if (rc != NO_ERROR) return rc;

    pCtl = (HTABLECTL *)(*CclCalloc)(1, sizeof(HTABLECTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    rc = HashCreate(ulInitialCapacity, ulKeySize, ulValueSize,
                    pOpts, &pCtl->engine);
    if (rc != NO_ERROR) {
        (*CclFree)(pCtl);
        return rc;
    }

    pCtl->ulMagic = CCL_HTBL_MAGIC;
    if (pOpts) pCtl->ulFlags = pOpts->ulFlags;

    *phTable = (HHASHTABLE)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a hashtable.
 *
 * @param[in] hTable  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY HashtableDestroy(HHASHTABLE hTable) {
    HTABLECTL *pCtl;

    if (hTable == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Drain via HashRemove so pDestroyKey / pDestroyValue fire
       exactly once per entry. HashDestroy itself does not call
       them. Calling them manually on temporary buffers would
       double-fire (HashRemove fires them again on the stored
       bytes) and would pass an uninitialised value buffer. */
    if (pCtl->engine.pDestroyKey || pCtl->engine.pDestroyValue) {
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
 * Adding and replacing entries
 * ================================================================== */

/*!
 * @brief Insert or replace a key/value pair.
 *
 * @param[in]  hTable      Handle.
 * @param[in]  pKey        Pointer to the key bytes. Not NULL.
 * @param[in]  pValue      Pointer to the value bytes. Not NULL.
 * @param[out] pfReplaced  Optional. Receives TRUE if replaced.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable, pKey or pValue is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET APIENTRY HashtablePut(HHASHTABLE hTable, PCVOID pKey,
                             PCVOID pValue, PBOOL pfReplaced) {
    HTABLECTL *pCtl;

    if (!pKey || !pValue) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashPut(&pCtl->engine, pKey, pValue, pfReplaced);
}

/* ==================================================================
 * Lookup
 * ================================================================== */

/*!
 * @brief Look up a key and copy the value into the caller's buffer.
 *
 * @param[in]  hTable   Handle.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable or pKey is NULL, or pBuf
 *                                  is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The key was not found.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HashtableGet(HHASHTABLE hTable, PCVOID pKey,
                             PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    HTABLECTL *pCtl;

    if (!pKey) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGet(&pCtl->engine, pKey, pBuf, ulSize, pulUsed);
}

/*!
 * @brief Test whether a key is present.
 *
 * @param[in]  hTable   Handle.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pfFound  Receives TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable, pKey or pfFound is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableContains(HHASHTABLE hTable, PCVOID pKey,
                                  PBOOL pfFound) {
    HTABLECTL *pCtl;
    PHASHNODE pNode;

    if (!pKey || !pfFound) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* HashGet with a size-query (NULL, 0) is the cheapest way to
       check presence without copying the value: it only looks up
       the key and returns ulValueSize in *pulUsed. Use the
       internal size 0 probe to keep this profile thin. */
    (void)pNode;
    *pfFound = FALSE;
    {
        ULONG ulUsed = 0;
        if (HashGet(&pCtl->engine, pKey, NULL, 0, &ulUsed) == NO_ERROR) {
            *pfFound = TRUE;
        }
    }
    return NO_ERROR;
}

/* ==================================================================
 * Removing
 * ================================================================== */

/*!
 * @brief Remove one entry by key.
 *
 * @param[in]  hTable    Handle.
 * @param[in]  pKey      Pointer to the key bytes. Not NULL.
 * @param[out] pfRemoved Optional. Receives TRUE if removed.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_PARAMETER  hTable or pKey is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableRemove(HHASHTABLE hTable, PCVOID pKey,
                                PBOOL pfRemoved) {
    HTABLECTL *pCtl;

    if (!pKey) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashRemove(&pCtl->engine, pKey, pfRemoved);
}

/*!
 * @brief Remove every entry from the hashtable.
 *
 * @param[in] hTable  Handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableClear(HHASHTABLE hTable) {
    HTABLECTL *pCtl;

    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Draining via HashRemove gives each entry its pDestroyKey /
       pDestroyValue call. */
    if (pCtl->engine.pDestroyKey || pCtl->engine.pDestroyValue) {
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
        /* No callbacks: we can drop every node without copying. */
        HashDestroy(&pCtl->engine);
        /* Recreate the engine with the same options. Easiest path:
           preserve the saved copy of the options is not available
           here, so instead just rebuild via HashCreate using the
           same size/callbacks kept in the engine. */
        {
            CCLOPTS opts;
            ULONG ulOldKey   = pCtl->engine.ulKeySize;
            ULONG ulOldVal   = pCtl->engine.ulValueSize;
            ULONG ulOldThr   = pCtl->engine.ulRehashThreshold;
            CCL_HASH_FN pHash        = pCtl->engine.pHash;
            CCL_COMPARE_FN pCompare  = pCtl->engine.pCompare;

            CclOptsInit(&opts);
            opts.pHash        = pHash;
            opts.pCompare     = pCompare;
            opts.ulRehashThreshold = ulOldThr;
            HashCreate(0, ulOldKey, ulOldVal, &opts, &pCtl->engine);
        }
    }
    return NO_ERROR;
}

/* ==================================================================
 * Metadata
 * ================================================================== */

/*!
 * @brief Return the number of entries.
 *
 * @param[in]  hTable    Handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableGetCount(HHASHTABLE hTable, PULONG pulCount) {
    HTABLECTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetCount(&pCtl->engine, pulCount);
}

/*!
 * @brief Return the current number of buckets.
 *
 * @param[in]  hTable    Handle.
 * @param[out] pulCount  Receives the bucket count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HashtableGetBucketCount(HHASHTABLE hTable, PULONG pulCount) {
    HTABLECTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetBucketCount(&pCtl->engine, pulCount);
}

/*!
 * @brief Force a rehash to the requested bucket count.
 *
 * @param[in] hTable        Handle.
 * @param[in] ulNewCapacity Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET APIENTRY HashtableSetBucketCount(HHASHTABLE hTable,
                                        ULONG ulNewCapacity) {
    HTABLECTL *pCtl;

    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashSetBucketCount(&pCtl->engine, ulNewCapacity);
}

/* ==================================================================
 * Ordered access
 * ================================================================== */

/*!
 * @brief Copy the key of the i-th entry into the caller's buffer.
 *
 * @param[in]  hTable   Handle.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HashtableGetIthKey(HHASHTABLE hTable, ULONG ulIndex,
                                   PVOID pBuf, ULONG ulSize,
                                   PULONG pulUsed) {
    HTABLECTL *pCtl;

    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetIthKey(&pCtl->engine, ulIndex, pBuf, ulSize, pulUsed);
}

/*!
 * @brief Copy the value of the i-th entry into the caller's buffer.
 *
 * @param[in]  hTable   Handle.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hTable is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HashtableGetIthValue(HHASHTABLE hTable, ULONG ulIndex,
                                     PVOID pBuf, ULONG ulSize,
                                     PULONG pulUsed) {
    HTABLECTL *pCtl;

    pCtl = get_ctl(hTable);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return HashGetIthValue(&pCtl->engine, ulIndex, pBuf, ulSize, pulUsed);
}
