/*!
 *
 * @file hash_private.c
 *
 * @brief Internal hash-table engine used by the public hash-based
 *        containers.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library. Not a public header.
 *
 * See hash_private.h for the storage model and callback contract.
 * Nodes are allocated with ccl_node_alloc, so the key and the value
 * are inline in the node and correctly aligned. The bucket array
 * (and, in external mode, the capacity and count) is reached through
 * helper functions that hide the internal/external storage choice
 * from the rest of the code.
 */

#include <string.h>
#include <stddef.h>

#include "hash_private.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"
#include "node_private.h"

/* ==================================================================
 * Storage helpers (internal vs external mode)
 * ================================================================== */

/*!
 * @brief Return the bucket array of a hash table.
 *
 * @param[in] h  Hash control block. Not NULL.
 *
 * @return The bucket array.
 */
static PVOID *hash_buckets(HASHCTL *h) {
    if (h->ppExternalBuckets) return *h->ppExternalBuckets;
    return h->pInternalBuckets;
}

/*!
 * @brief Store a new bucket array into a hash table.
 *
 * @param[in,out] h         Hash control block. Not NULL.
 * @param[in]     pBuckets  New bucket array.
 */
static void hash_set_buckets(HASHCTL *h, PVOID *pBuckets) {
    if (h->ppExternalBuckets) *h->ppExternalBuckets = pBuckets;
    else h->pInternalBuckets = pBuckets;
}

/*!
 * @brief Return the current bucket count.
 *
 * @param[in] h  Hash control block. Not NULL.
 *
 * @return The bucket count.
 */
static ULONG hash_capacity(HASHCTL *h) {
    if (h->pulExternalCapacity) return *h->pulExternalCapacity;
    return h->ulInternalCapacity;
}

/*!
 * @brief Store a new bucket count.
 *
 * @param[in,out] h           Hash control block. Not NULL.
 * @param[in]     ulCapacity  New bucket count.
 */
static void hash_set_capacity(HASHCTL *h, ULONG ulCapacity) {
    if (h->pulExternalCapacity) *h->pulExternalCapacity = ulCapacity;
    else h->ulInternalCapacity = ulCapacity;
}

/*!
 * @brief Return the current entry count.
 *
 * @param[in] h  Hash control block. Not NULL.
 *
 * @return The entry count.
 */
static ULONG hash_count(HASHCTL *h) {
    if (h->pulExternalCount) return *h->pulExternalCount;
    return h->ulInternalCount;
}

/*!
 * @brief Store a new entry count.
 *
 * @param[in,out] h        Hash control block. Not NULL.
 * @param[in]     ulCount  New entry count.
 */
static void hash_set_count(HASHCTL *h, ULONG ulCount) {
    if (h->pulExternalCount) *h->pulExternalCount = ulCount;
    else h->ulInternalCount = ulCount;
}

/* ==================================================================
 * Node helpers
 * ================================================================== */

/*!
 * @brief Allocate an unlinked HASHNODE via ccl_node_alloc.
 *
 * @param[in] h  Hash control block. Not NULL.
 *
 * @return A new node, or NULL on allocation failure.
 *
 * @retval NULL  Allocation failed.
 */
static PHASHNODE hash_alloc_node(HASHCTL *h) {
    return (PHASHNODE)ccl_node_alloc(
        (ULONG)offsetof(HASHNODE, u),
        h->ulValueOffset + h->ulValueSize,
        CCL_MAX_ALIGN);
}

/*!
 * @brief Release a HASHNODE.
 *
 * @param[in] h      Hash control block. Not NULL, unused.
 * @param[in] pNode  Node to release. Not NULL.
 */
static void hash_free_node(HASHCTL *h, PHASHNODE pNode) {
    (void)h;
    ccl_node_free(pNode);
}

/*!
 * @brief Return a pointer to the key bytes inside a node.
 *
 * @param[in] pNode  Node. Not NULL.
 *
 * @return The address of the key bytes.
 */
static UCHAR *hash_node_key(PHASHNODE pNode) {
    return (UCHAR *)pNode->u.c;
}

/*!
 * @brief Return a pointer to the value bytes inside a node.
 *
 * @param[in] h      Hash control block. Not NULL.
 * @param[in] pNode  Node. Not NULL.
 *
 * @return The address of the value bytes.
 */
static UCHAR *hash_node_value(HASHCTL *h, PHASHNODE pNode) {
    return (UCHAR *)pNode->u.c + h->ulValueOffset;
}

/* ==================================================================
 * Lookup and resize
 * ================================================================== */

/*!
 * @brief Find a node by key.
 *
 * @param[in]  h       Hash control block. Not NULL.
 * @param[in]  pKey    Pointer to the key bytes. Not NULL.
 * @param[out] ppPrev  Receives the previous node, or NULL. May be
 *                     NULL.
 *
 * @return The matching node, or NULL if not found.
 *
 * @retval NULL  The key was not found.
 */
static PHASHNODE hash_find(HASHCTL *h, PCVOID pKey, PHASHNODE *ppPrev) {
    ULONG ulHash;
    ULONG ulBucket;
    PHASHNODE pNode;
    PHASHNODE pPrev;

    ulHash   = h->pHash(pKey, NULL);
    ulBucket = ulHash % hash_capacity(h);

    pNode = (PHASHNODE)hash_buckets(h)[ulBucket];
    pPrev = NULL;
    while (pNode) {
        if (h->pCompare(hash_node_key(pNode), pKey, NULL) == 0) {
            if (ppPrev) *ppPrev = pPrev;
            return pNode;
        }
        pPrev = pNode;
        pNode = pNode->next;
    }
    if (ppPrev) *ppPrev = NULL;
    return NULL;
}

/*!
 * @brief Round a bucket count up to the next power of 2.
 *
 * @param[in] ulValue  Requested bucket count.
 *
 * @return The rounded bucket count.
 *
 * @retval 1  ulValue was 0.
 */
static ULONG hash_round_pow2(ULONG ulValue) {
    ULONG ulResult = 1;

    if (ulValue == 0) return 1;
    while (ulResult < ulValue) {
        if (ulResult > (ULONG)-1 / 2) return ulValue;
        ulResult *= 2;
    }
    return ulResult;
}

/*!
 * @brief Resize the bucket array to the requested bucket count.
 *
 * @param[in,out] h              Hash control block. Not NULL.
 * @param[in]     ulNewCapacity  Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
static APIRET hash_resize(HASHCTL *h, ULONG ulNewCapacity) {
    PVOID    *pNewBuckets;
    ULONG     ulOldCapacity;
    ULONG     ulByteSize;
    ULONG     i;
    PHASHNODE pNode;
    PHASHNODE pNext;
    ULONG     ulHash;
    ULONG     ulNewBucket;

    ulNewCapacity = hash_round_pow2(ulNewCapacity);
    ulOldCapacity = hash_capacity(h);
    if (ulNewCapacity == ulOldCapacity) return NO_ERROR;

    if (ulNewCapacity > (ULONG)-1 / sizeof(PVOID))
        return ERROR_NOT_ENOUGH_MEMORY;
    ulByteSize = ulNewCapacity * sizeof(PVOID);

    pNewBuckets = (PVOID *)(*CclMalloc)(ulByteSize);
    if (!pNewBuckets) return ERROR_NOT_ENOUGH_MEMORY;
    memset(pNewBuckets, 0, ulByteSize);

    for (i = 0; i < ulOldCapacity; i++) {
        pNode = (PHASHNODE)hash_buckets(h)[i];
        while (pNode) {
            pNext = pNode->next;
            ulHash      = h->pHash(hash_node_key(pNode), NULL);
            ulNewBucket = ulHash % ulNewCapacity;
            pNode->next = (PHASHNODE)pNewBuckets[ulNewBucket];
            pNewBuckets[ulNewBucket] = (PVOID)pNode;
            pNode = pNext;
        }
    }

    (*CclFree)(hash_buckets(h));
    hash_set_buckets(h, pNewBuckets);
    hash_set_capacity(h, ulNewCapacity);
    return NO_ERROR;
}

/* ==================================================================
 * Create
 * ================================================================== */

/*!
 * @brief Common body of HashCreate and HashCreateWithStorage.
 *
 * @param[in]  ulInitialCapacity  Initial bucket count, or 0.
 * @param[in]  ulKeySize          Size of one key, in bytes.
 * @param[in]  ulValueSize        Size of one value, in bytes.
 * @param[in]  pOpts              Callback block. Not NULL.
 * @param[out] phCtl              Caller-provided HASHCTL storage.
 * @param[in]  pStorage           External storage descriptor, or
 *                                NULL for internal mode.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Invalid argument.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
static APIRET hash_create_common(
    ULONG          ulInitialCapacity,
    ULONG          ulKeySize,
    ULONG          ulValueSize,
    PCCLOPTS       pOpts,
    PHASHCTL       phCtl,
    PHASH_EXTERNAL pStorage)
{
    PVOID *pBuckets;
    ULONG  ulCapacity;
    ULONG  ulByteSize;

    if (!phCtl) return ERROR_INVALID_PARAMETER;
    if (!pOpts) return ERROR_INVALID_PARAMETER;
    if (!pOpts->pHash) return ERROR_INVALID_PARAMETER;
    if (!pOpts->pCompare) return ERROR_INVALID_PARAMETER;
    if (ulKeySize == 0) return ERROR_INVALID_PARAMETER;

    if (ulInitialCapacity == 0) ulInitialCapacity = 8;
    ulCapacity = hash_round_pow2(ulInitialCapacity);

    if (ulCapacity > (ULONG)-1 / sizeof(PVOID))
        return ERROR_NOT_ENOUGH_MEMORY;
    ulByteSize = ulCapacity * sizeof(PVOID);

    pBuckets = (PVOID *)(*CclMalloc)(ulByteSize);
    if (!pBuckets) return ERROR_NOT_ENOUGH_MEMORY;
    memset(pBuckets, 0, ulByteSize);

    memset(phCtl, 0, sizeof(*phCtl));
    phCtl->ulMagic           = CCL_HASH_MAGIC;
    phCtl->pHash             = pOpts->pHash;
    phCtl->pCompare          = pOpts->pCompare;
    phCtl->pDestroyKey       = pOpts->pDestroyKey;
    phCtl->pDestroyValue     = pOpts->pDestroyValue;
    phCtl->ulRehashThreshold = pOpts->ulRehashThreshold
                               ? pOpts->ulRehashThreshold
                               : CCL_REHASH_THRESHOLD_DEFAULT;
    phCtl->ulKeySize         = ulKeySize;
    phCtl->ulValueSize       = ulValueSize;
    phCtl->ulValueOffset     = CCL_ALIGN_UP(ulKeySize);

    if (pStorage) {
        phCtl->ppExternalBuckets   = pStorage->ppBucketArray;
        phCtl->pulExternalCapacity = pStorage->pulBucketCount;
        phCtl->pulExternalCount    = pStorage->pulEntryCount;
        *pStorage->ppBucketArray   = pBuckets;
        *pStorage->pulBucketCount  = ulCapacity;
        *pStorage->pulEntryCount   = 0;
    } else {
        phCtl->pInternalBuckets   = pBuckets;
        phCtl->ulInternalCapacity = ulCapacity;
        phCtl->ulInternalCount    = 0;
    }

    return NO_ERROR;
}

/*!
 * @brief Create a hash table with internal storage.
 *
 * @param[in]  ulInitialCapacity  Initial bucket count, or 0.
 * @param[in]  ulKeySize          Size of one key, in bytes.
 * @param[in]  ulValueSize        Size of one value, in bytes.
 * @param[in]  pOpts              Callback block.
 * @param[out] phCtl              Caller-provided HASHCTL storage.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Invalid argument.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashCreate(
    ULONG    ulInitialCapacity,
    ULONG    ulKeySize,
    ULONG    ulValueSize,
    PCCLOPTS pOpts,
    PHASHCTL phCtl)
{
    return hash_create_common(ulInitialCapacity, ulKeySize, ulValueSize,
                              pOpts, phCtl, NULL);
}

/*!
 * @brief Create a hash table with external storage.
 *
 * @param[in]  pOpts       Callback block.
 * @param[in]  ulKeySize   Size of one key, in bytes.
 * @param[in]  ulValueSize Size of one value, in bytes.
 * @param[out] phCtl       Caller-provided HASHCTL storage.
 * @param[in]  pStorage    External storage descriptor, or NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Invalid argument.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashCreateWithStorage(
    PCCLOPTS       pOpts,
    ULONG          ulKeySize,
    ULONG          ulValueSize,
    PHASHCTL       phCtl,
    PHASH_EXTERNAL pStorage)
{
    return hash_create_common(0, ulKeySize, ulValueSize,
                              pOpts, phCtl, pStorage);
}

/* ==================================================================
 * Put / Get / Remove
 * ================================================================== */

/*!
 * @brief Insert or replace a key/value pair.
 *
 * @param[in,out] h          Hash control block. Not NULL.
 * @param[in]     pKey       Pointer to the key bytes. Not NULL.
 * @param[in]     pValue     Pointer to the value bytes.
 * @param[out]    pfReplaced Receives TRUE if the key was replaced.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  Invalid argument.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET HashPut(HASHCTL *h, PCVOID pKey, PCVOID pValue, PBOOL pfReplaced) {
    PHASHNODE pNode;
    ULONG     ulHash;
    ULONG     ulBucket;
    ULONG     ulCapacity;
    ULONG     ulCount;
    ULONG     ulLoadPermille;

    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (!pKey) return ERROR_INVALID_PARAMETER;
    if (!pValue && h->ulValueSize > 0) return ERROR_INVALID_PARAMETER;

    if (pfReplaced) *pfReplaced = FALSE;

    pNode = hash_find(h, pKey, NULL);
    if (pNode) {
        if (h->pDestroyKey) h->pDestroyKey((PVOID)pKey);
        if (h->pDestroyValue && h->ulValueSize > 0)
            h->pDestroyValue(hash_node_value(h, pNode));
        if (h->ulValueSize > 0 && pValue)
            memcpy(hash_node_value(h, pNode), pValue, h->ulValueSize);
        if (pfReplaced) *pfReplaced = TRUE;
        return NO_ERROR;
    }

    pNode = hash_alloc_node(h);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(hash_node_key(pNode), pKey, h->ulKeySize);
    if (h->ulValueSize > 0 && pValue)
        memcpy(hash_node_value(h, pNode), pValue, h->ulValueSize);
    else if (h->ulValueSize > 0)
        memset(hash_node_value(h, pNode), 0, h->ulValueSize);

    ulHash      = h->pHash(pKey, NULL);
    ulCapacity  = hash_capacity(h);
    ulBucket    = ulHash % ulCapacity;
    pNode->next = (PHASHNODE)hash_buckets(h)[ulBucket];
    hash_buckets(h)[ulBucket] = (PVOID)pNode;

    ulCount = hash_count(h) + 1;
    hash_set_count(h, ulCount);

    ulLoadPermille = (ulCount * 1000) / ulCapacity;
    if (ulLoadPermille >= h->ulRehashThreshold) {
        if (ulCapacity <= (ULONG)-1 / 2) {
            hash_resize(h, ulCapacity * 2);
        }
    }

    return NO_ERROR;
}

/*!
 * @brief Append a new key/value pair unconditionally.
 *
 * @param[in,out] h       Hash control block. Not NULL.
 * @param[in]     pKey    Pointer to the key bytes. Not NULL.
 * @param[in]     pValue  Pointer to the value bytes, or NULL when
 *                        ulValueSize == 0.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  Invalid argument.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Node allocation failed.
 */
APIRET HashPutAppend(HASHCTL *h, PCVOID pKey, PCVOID pValue) {
    PHASHNODE pNode;
    ULONG     ulHash;
    ULONG     ulBucket;
    ULONG     ulCapacity;
    ULONG     ulCount;
    ULONG     ulLoadPermille;

    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (!pKey) return ERROR_INVALID_PARAMETER;
    if (!pValue && h->ulValueSize > 0) return ERROR_INVALID_PARAMETER;

    pNode = hash_alloc_node(h);
    if (!pNode) return ERROR_NOT_ENOUGH_MEMORY;

    memcpy(hash_node_key(pNode), pKey, h->ulKeySize);
    if (h->ulValueSize > 0 && pValue)
        memcpy(hash_node_value(h, pNode), pValue, h->ulValueSize);
    else if (h->ulValueSize > 0)
        memset(hash_node_value(h, pNode), 0, h->ulValueSize);

    ulHash      = h->pHash(pKey, NULL);
    ulCapacity  = hash_capacity(h);
    ulBucket    = ulHash % ulCapacity;
    pNode->next = (PHASHNODE)hash_buckets(h)[ulBucket];
    hash_buckets(h)[ulBucket] = (PVOID)pNode;

    ulCount = hash_count(h) + 1;
    hash_set_count(h, ulCount);

    ulLoadPermille = (ulCount * 1000) / ulCapacity;
    if (ulLoadPermille >= h->ulRehashThreshold) {
        if (ulCapacity <= (ULONG)-1 / 2) {
            hash_resize(h, ulCapacity * 2);
        }
    }

    return NO_ERROR;
}

/*!
 * @brief Look up a key and copy the value into the caller's buffer.
 *
 * The key lookup is performed before the size-query branch, so a
 * missing key always returns ERROR_NO_MORE_ITEMS, including for a
 * size-query (pBuf == NULL && ulSize == 0). This is what callers
 * like SetContains rely on to distinguish "key present, size is N"
 * from "key absent".
 *
 * @param[in]  h        Hash control block. Not NULL.
 * @param[in]  pKey     Pointer to the key bytes. Not NULL.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Receives the used or required size. May be
 *                      NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  Invalid argument.
 * @retval ERROR_NO_MORE_ITEMS      The key was not found.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET HashGet(HASHCTL *h, PCVOID pKey, PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    PHASHNODE pNode;

    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (!pKey) return ERROR_INVALID_PARAMETER;

    pNode = hash_find(h, pKey, NULL);
    if (!pNode) return ERROR_NO_MORE_ITEMS;

    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = h->ulValueSize;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;

    if (ulSize < h->ulValueSize) {
        if (pulUsed) *pulUsed = h->ulValueSize;
        return ERROR_BUFFER_OVERFLOW;
    }

    if (h->ulValueSize > 0)
        memcpy(pBuf, hash_node_value(h, pNode), h->ulValueSize);
    if (pulUsed) *pulUsed = h->ulValueSize;
    return NO_ERROR;
}

/*!
 * @brief Remove an entry by key.
 *
 * @param[in,out] h       Hash control block. Not NULL.
 * @param[in]     pKey    Pointer to the key bytes. Not NULL.
 * @param[out]    pfFound Receives TRUE if the entry was found.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (including not-found).
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  pKey is NULL.
 */
APIRET HashRemove(HASHCTL *h, PCVOID pKey, PBOOL pfFound) {
    PHASHNODE pNode;
    PHASHNODE pPrev;
    ULONG     ulHash;
    ULONG     ulBucket;

    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (!pKey) return ERROR_INVALID_PARAMETER;

    if (pfFound) *pfFound = FALSE;

    pNode = hash_find(h, pKey, &pPrev);
    if (!pNode) return NO_ERROR;

    if (pPrev) {
        pPrev->next = pNode->next;
    } else {
        ulHash   = h->pHash(pKey, NULL);
        ulBucket = ulHash % hash_capacity(h);
        hash_buckets(h)[ulBucket] = (PVOID)pNode->next;
    }

    if (h->pDestroyKey) h->pDestroyKey(hash_node_key(pNode));
    if (h->pDestroyValue && h->ulValueSize > 0)
        h->pDestroyValue(hash_node_value(h, pNode));

    hash_free_node(h, pNode);
    hash_set_count(h, hash_count(h) - 1);

    if (pfFound) *pfFound = TRUE;
    return NO_ERROR;
}

/* ==================================================================
 * Grow / destroy
 * ================================================================== */

/*!
 * @brief Force a rehash to the requested bucket count.
 *
 * @param[in,out] h              Hash control block. Not NULL.
 * @param[in]     ulNewCapacity  Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashGrow(HASHCTL *h, ULONG ulNewCapacity) {
    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    return hash_resize(h, ulNewCapacity);
}

/*!
 * @brief Release the internal storage of a hash table.
 *
 * @param[in,out] h  Hash control block. May be NULL (no-op).
 */
void HashDestroy(HASHCTL *h) {
    ULONG     ulCapacity;
    ULONG     i;
    PHASHNODE pNode;
    PHASHNODE pNext;

    if (!h) return;
    if (h->ulMagic != CCL_HASH_MAGIC) return;

    ulCapacity = hash_capacity(h);
    for (i = 0; i < ulCapacity; i++) {
        pNode = (PHASHNODE)hash_buckets(h)[i];
        while (pNode) {
            pNext = pNode->next;
            hash_free_node(h, pNode);
            pNode = pNext;
        }
    }

    (*CclFree)(hash_buckets(h));
    hash_set_buckets(h, NULL);
    hash_set_capacity(h, 0);
    hash_set_count(h, 0);

    h->ulMagic = 0;
}

/* ==================================================================
 * Ordered access
 * ================================================================== */

/*!
 * @brief Copy the key of the i-th entry into the caller's buffer.
 *
 * @param[in]  h        Hash control block. Not NULL.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Receives the used or required size. May be
 *                      NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL outside size-query.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET HashGetIthKey(HASHCTL *h, ULONG ulIndex, PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    ULONG     ulCapacity;
    ULONG     i;
    ULONG     j;
    PHASHNODE pNode;

    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;

    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = h->ulKeySize;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;

    if (ulSize < h->ulKeySize) {
        if (pulUsed) *pulUsed = h->ulKeySize;
        return ERROR_BUFFER_OVERFLOW;
    }

    j = 0;
    ulCapacity = hash_capacity(h);
    for (i = 0; i < ulCapacity; i++) {
        pNode = (PHASHNODE)hash_buckets(h)[i];
        while (pNode) {
            if (j == ulIndex) {
                memcpy(pBuf, hash_node_key(pNode), h->ulKeySize);
                if (pulUsed) *pulUsed = h->ulKeySize;
                return NO_ERROR;
            }
            j++;
            pNode = pNode->next;
        }
    }

    return ERROR_NO_MORE_ITEMS;
}

/*!
 * @brief Copy the value of the i-th entry into the caller's buffer.
 *
 * @param[in]  h        Hash control block. Not NULL.
 * @param[in]  ulIndex  Zero-based entry index.
 * @param[out] pBuf     Destination buffer, or NULL for size-query.
 * @param[in]  ulSize   Size of pBuf in bytes.
 * @param[out] pulUsed  Receives the used or required size. May be
 *                      NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL outside size-query.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= count, or
 *                                  ulValueSize == 0.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET HashGetIthValue(HASHCTL *h, ULONG ulIndex, PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    ULONG     ulCapacity;
    ULONG     i;
    ULONG     j;
    PHASHNODE pNode;

    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;

    if (h->ulValueSize == 0) return ERROR_NO_MORE_ITEMS;

    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = h->ulValueSize;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;

    if (ulSize < h->ulValueSize) {
        if (pulUsed) *pulUsed = h->ulValueSize;
        return ERROR_BUFFER_OVERFLOW;
    }

    j = 0;
    ulCapacity = hash_capacity(h);
    for (i = 0; i < ulCapacity; i++) {
        pNode = (PHASHNODE)hash_buckets(h)[i];
        while (pNode) {
            if (j == ulIndex) {
                memcpy(pBuf, hash_node_value(h, pNode), h->ulValueSize);
                if (pulUsed) *pulUsed = h->ulValueSize;
                return NO_ERROR;
            }
            j++;
            pNode = pNode->next;
        }
    }

    return ERROR_NO_MORE_ITEMS;
}

/*!
 * @brief Copy the current entry count into *pulCount.
 *
 * @param[in]  h         Hash control block. Not NULL.
 * @param[out] pulCount  Receives the entry count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET HashGetCount(HASHCTL *h, PULONG pulCount) {
    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (!pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = hash_count(h);
    return NO_ERROR;
}

/*!
 * @brief Copy the current bucket count into *pulCount.
 *
 * @param[in]  h         Hash control block. Not NULL.
 * @param[out] pulCount  Receives the bucket count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET HashGetBucketCount(HASHCTL *h, PULONG pulCount) {
    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (!pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = hash_capacity(h);
    return NO_ERROR;
}

/*!
 * @brief Resize the bucket array to the requested count.
 *
 * @param[in,out] h              Hash control block. Not NULL.
 * @param[in]     ulNewCapacity  Requested bucket count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     h is invalid.
 * @retval ERROR_INVALID_PARAMETER  The table is not empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Bucket array allocation failed.
 */
APIRET HashSetBucketCount(HASHCTL *h, ULONG ulNewCapacity) {
    if (!ccl_validate(h, CCL_HASH_MAGIC)) return ERROR_INVALID_HANDLE;
    if (hash_count(h) != 0) return ERROR_INVALID_PARAMETER;
    return hash_resize(h, ulNewCapacity);
}
