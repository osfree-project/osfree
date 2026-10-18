/*!
 *
 * @file heap.c
 *
 * @brief Implementation of the heap container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * The heap is a thin profile over vector.h. Storage growth and
 * element copy live in vector.c; this file owns HEAPCTL, validates
 * CCLOPTS, and implements sift_up / sift_down on top of the
 * borrowed references returned by VectorGetRef.
 *
 * Sift operations swap stored bytes directly through the borrowed
 * references, without going through VectorSet / VectorSetRef. This
 * is deliberate: a swap is not a replacement, so pDestroyValue
 * must not fire for either side. Only the final removal in
 * HeapPop and the removals in HeapClear go through
 * VectorRemoveLast, which does call pDestroyValue when configured
 * in the underlying vector.
 *
 * HeapPop uses the swap-then-remove-last pattern: the root bytes
 * are swapped with the last bytes, then the last slot (which now
 * holds the original root bytes) is removed via VectorRemoveLast.
 * This makes pDestroyValue fire on the actual popped root, not on
 * a stale copy of the root that would otherwise sit in the last
 * slot after a memcpy-based move.
 */

#include <string.h>
#include "heap.h"
#include "vector.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_HEAP_MAGIC
 * @brief Magic value identifying a valid heap control block.
 *
 * The literal bytes are "HEAP".
 */
#define CCL_HEAP_MAGIC 0x48454150UL  /* "HEAP" */

/*!
 * @struct _HEAPCTL
 * @brief Control block of an open heap.
 */
typedef struct _HEAPCTL {
    unsigned long  ulMagic;      /*!< CCL_HEAP_MAGIC.                */
    unsigned long  ulElemSize;   /*!< Size of one element in bytes.  */
    HVECTOR        hVector;      /*!< Underlying vector.             */
    CCL_COMPARE_FN pCompare;     /*!< Required ordering callback.    */
} HEAPCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a heap handle.
 *
 * @param[in] hHeap  Heap handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hHeap is NULL, or its magic value does not match.
 */
static HEAPCTL *get_ctl(HHEAP hHeap) {
    return (HEAPCTL *)ccl_validate(hHeap, CCL_HEAP_MAGIC);
}

/*!
 * @brief Swap the bytes of two element slots.
 *
 * @param[in,out] pA         First slot. Not NULL.
 * @param[in,out] pB         Second slot. Not NULL.
 * @param[in]     ulElemSize Size of one element in bytes.
 */
static void swap_bytes(unsigned char *pA, unsigned char *pB,
                       unsigned long ulElemSize) {
    unsigned long i;
    unsigned char tmp;
    for (i = 0; i < ulElemSize; i++) {
        tmp = pA[i];
        pA[i] = pB[i];
        pB[i] = tmp;
    }
}

/*!
 * @brief Restore the min-heap invariant by moving one element up.
 *
 * @param[in] pCtl   Control block. Not NULL.
 * @param[in] ulIdx  Index of the element to sift up.
 */
static void sift_up(HEAPCTL *pCtl, unsigned long ulIdx) {
    while (ulIdx > 0) {
        unsigned long ulParent = (ulIdx - 1) / 2;
        unsigned char *pChild  = NULL;
        unsigned char *pParent = NULL;

        VectorGetRef(pCtl->hVector, (ULONG)ulIdx,    (PVOID *)&pChild);
        VectorGetRef(pCtl->hVector, (ULONG)ulParent, (PVOID *)&pParent);

        if (pCtl->pCompare(pChild, pParent, NULL) < 0) {
            swap_bytes(pChild, pParent, pCtl->ulElemSize);
            ulIdx = ulParent;
        } else {
            break;
        }
    }
}

/*!
 * @brief Restore the min-heap invariant by moving one element down.
 *
 * @param[in] pCtl   Control block. Not NULL.
 * @param[in] ulIdx  Index of the element to sift down.
 */
static void sift_down(HEAPCTL *pCtl, unsigned long ulIdx) {
    ULONG ulCount = 0;

    VectorGetCount(pCtl->hVector, &ulCount);

    for (;;) {
        unsigned long ulLeft     = 2 * ulIdx + 1;
        unsigned long ulRight    = 2 * ulIdx + 2;
        unsigned long ulSmallest = ulIdx;
        unsigned char *pSmallest = NULL;
        unsigned char *pChild    = NULL;

        if (ulLeft >= (unsigned long)ulCount) break;

        VectorGetRef(pCtl->hVector, (ULONG)ulSmallest, (PVOID *)&pSmallest);
        VectorGetRef(pCtl->hVector, (ULONG)ulLeft,     (PVOID *)&pChild);

        if (pCtl->pCompare(pChild, pSmallest, NULL) < 0) {
            ulSmallest = ulLeft;
            pSmallest  = pChild;
        }
        if (ulRight < (unsigned long)ulCount) {
            VectorGetRef(pCtl->hVector, (ULONG)ulRight, (PVOID *)&pChild);
            if (pCtl->pCompare(pChild, pSmallest, NULL) < 0) {
                ulSmallest = ulRight;
            }
        }
        if (ulSmallest == ulIdx) break;

        {
            unsigned char *pCur = NULL;
            VectorGetRef(pCtl->hVector, (ULONG)ulIdx,      (PVOID *)&pCur);
            VectorGetRef(pCtl->hVector, (ULONG)ulSmallest, (PVOID *)&pSmallest);
            swap_bytes(pCur, pSmallest, pCtl->ulElemSize);
        }
        ulIdx = ulSmallest;
    }
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new heap.
 *
 * @param[in]  ulElemSize  Size of one element in bytes.
 * @param[in]  pOpts       Option block. pCompare is required.
 * @param[out] phHeap      Receives the new handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument or pOpts lacks
 *                                  pCompare.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HeapCreate(ULONG ulElemSize, PCCLOPTS pOpts,
                           PHHEAP phHeap) {
    HEAPCTL *pCtl;
    APIRET rc;
    CCLOPTS vecOpts;

    if (!phHeap) return ERROR_INVALID_PARAMETER;
    if (ulElemSize == 0) return ERROR_INVALID_PARAMETER;
    *phHeap = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_COMPARE
                         | CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         CCL_OPT_COMPARE);
    if (rc != NO_ERROR) return rc;

    pCtl = (HEAPCTL *)(*CclCalloc)(1, sizeof(HEAPCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    /* VectorCreate's ulAllowed does not include CCL_OPT_COMPARE.
       Build a vector-specific copy of the options with only the
       fields vector accepts. */
    CclOptsInit(&vecOpts);
    vecOpts.pDestroyValue = pOpts->pDestroyValue;
    vecOpts.ulFlags       = pOpts->ulFlags;

    rc = VectorCreate(ulElemSize, 0, &vecOpts, &pCtl->hVector);
    if (rc != NO_ERROR) {
        (*CclFree)(pCtl);
        return rc;
    }

    pCtl->ulMagic    = CCL_HEAP_MAGIC;
    pCtl->ulElemSize = (unsigned long)ulElemSize;
    pCtl->pCompare   = pOpts->pCompare;

    *phHeap = (HHEAP)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a heap.
 *
 * @param[in] hHeap  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY HeapDestroy(HHEAP hHeap) {
    HEAPCTL *pCtl;

    if (hHeap == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hHeap);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    VectorDestroy(pCtl->hVector);
    pCtl->ulMagic = 0;
    (*CclFree)(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Push, pop, peek
 * ================================================================== */

/*!
 * @brief Push a copy of an element onto the heap.
 *
 * @param[in] hHeap  Handle.
 * @param[in] pElem  Pointer to the element to copy. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hHeap or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HeapPush(HHEAP hHeap, PCVOID pElem) {
    HEAPCTL *pCtl;
    ULONG ulCount = 0;
    APIRET rc;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hHeap);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    rc = VectorAdd(pCtl->hVector, pElem);
    if (rc != NO_ERROR) return rc;

    VectorGetCount(pCtl->hVector, &ulCount);
    sift_up(pCtl, (unsigned long)(ulCount - 1));
    return NO_ERROR;
}

/*!
 * @brief Pop the root element into a caller-supplied buffer.
 *
 * @param[in]  hHeap   Handle.
 * @param[out] pBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize  Size of pBuf in bytes.
 * @param[out] pulUsed Optional.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hHeap is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The heap is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY HeapPop(HHEAP hHeap, PVOID pBuf, ULONG ulSize,
                        PULONG pulUsed) {
    HEAPCTL *pCtl;
    ULONG ulCount = 0;
    APIRET rc;

    pCtl = get_ctl(hHeap);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = pCtl->ulElemSize;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;

    rc = VectorGetCount(pCtl->hVector, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    rc = VectorGetItem(pCtl->hVector, 0, pBuf, ulSize, pulUsed);
    if (rc != NO_ERROR) return rc;

    if (ulCount == 1) {
        return VectorRemoveLast(pCtl->hVector);
    }

    /* Swap root and last so that VectorRemoveLast's pDestroyValue
       fires on the actual popped root, not on a stale copy. */
    {
        unsigned char *pRoot = NULL;
        unsigned char *pLast = NULL;
        VectorGetRef(pCtl->hVector, 0,          (PVOID *)&pRoot);
        VectorGetRef(pCtl->hVector, ulCount - 1,(PVOID *)&pLast);
        swap_bytes(pRoot, pLast, pCtl->ulElemSize);
    }

    rc = VectorRemoveLast(pCtl->hVector);
    if (rc != NO_ERROR) return rc;

    sift_down(pCtl, 0);
    return NO_ERROR;
}

/*!
 * @brief Return a borrowed pointer to the root element.
 *
 * @param[in]  hHeap   Handle.
 * @param[out] ppElem  Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hHeap or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The heap is empty.
 */
APIRET APIENTRY HeapTop(HHEAP hHeap, PVOID *ppElem) {
    HEAPCTL *pCtl;
    ULONG ulCount = 0;
    APIRET rc;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hHeap);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    rc = VectorGetCount(pCtl->hVector, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    return VectorGetRef(pCtl->hVector, 0, ppElem);
}

/* ==================================================================
 * Metadata and bulk operations
 * ================================================================== */

/*!
 * @brief Return the number of elements in the heap.
 *
 * @param[in]  hHeap     Handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HeapGetCount(HHEAP hHeap, PULONG pulCount) {
    HEAPCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hHeap);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return VectorGetCount(pCtl->hVector, pulCount);
}

/*!
 * @brief Remove every element from the heap.
 *
 * @param[in] hHeap  Handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HeapClear(HHEAP hHeap) {
    HEAPCTL *pCtl;
    APIRET rc;

    pCtl = get_ctl(hHeap);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    for (;;) {
        ULONG ulCount = 0;
        rc = VectorGetCount(pCtl->hVector, &ulCount);
        if (rc != NO_ERROR) return rc;
        if (ulCount == 0) break;
        rc = VectorRemoveLast(pCtl->hVector);
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}
