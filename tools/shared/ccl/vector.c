/* vector.c - dynamic array of fixed-size elements.
 * Part of the ccl container library. */

/**
 * @file vector.c
 * @brief Implementation of the vector container.
 *
 * Copyright (c) osFree Project 2026, <http://www.osFree.org>
 *   for licence see licence.txt in root directory, or project website
 */

#include <stdlib.h>
#include <string.h>
#include "vector.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/** @brief Magic value identifying a valid vector control block. */
#define CCL_VECTOR_MAGIC 0x56454354UL  /* "VECT" */

/**
 * @struct _VECTORCTL
 * @brief Control block of an open vector.
 */
typedef struct _VECTORCTL {
    unsigned long ulMagic;       /**< CCL_VECTOR_MAGIC.                 */
    unsigned long ulElemSize;    /**< Size of one element in bytes.     */
    unsigned long ulCount;       /**< Number of elements stored.        */
    unsigned long ulCapacity;    /**< Allocated element slots.          */
    unsigned char *pbData;       /**< Element storage, or NULL.         */
} VECTORCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

static VECTORCTL *get_ctl(HVECTOR hVector) {
    VECTORCTL *pCtl;
    if (hVector == NULLHANDLE) return NULL;
    pCtl = (VECTORCTL *)hVector;
    if (pCtl->ulMagic != CCL_VECTOR_MAGIC) return NULL;
    return pCtl;
}

static unsigned char *element_at(VECTORCTL *pCtl, unsigned long ulIndex) {
    return pCtl->pbData + (size_t)ulIndex * (size_t)pCtl->ulElemSize;
}

/**
 * @brief Ensure that at least ulNeeded element slots are allocated.
 */
static APIRET vector_reserve(VECTORCTL *pCtl, unsigned long ulNeeded) {
    unsigned long ulNewCapacity;
    unsigned char *pbNew;

    if (ulNeeded <= pCtl->ulCapacity) return NO_ERROR;

    ulNewCapacity = pCtl->ulCapacity ? pCtl->ulCapacity : 8;
    while (ulNewCapacity < ulNeeded) {
        if (ulNewCapacity > (unsigned long)-1 / 2) {
            ulNewCapacity = ulNeeded;
            break;
        }
        ulNewCapacity *= 2;
    }

    pbNew = (unsigned char *)realloc(pCtl->pbData,
                                     (size_t)ulNewCapacity *
                                     (size_t)pCtl->ulElemSize);
    if (!pbNew) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->pbData = pbNew;
    pCtl->ulCapacity = ulNewCapacity;
    return NO_ERROR;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

APIRET APIENTRY VectorCreate(ULONG ulElemSize, PHVECTOR phVector) {
    VECTORCTL *pCtl;

    if (!phVector) return ERROR_INVALID_PARAMETER;
    if (ulElemSize == 0) return ERROR_INVALID_PARAMETER;
    *phVector = NULLHANDLE;

    pCtl = (VECTORCTL *)calloc(1, sizeof(VECTORCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->ulMagic = CCL_VECTOR_MAGIC;
    pCtl->ulElemSize = (unsigned long)ulElemSize;

    *phVector = (HVECTOR)pCtl;
    return NO_ERROR;
}

APIRET APIENTRY VectorDestroy(HVECTOR hVector) {
    VECTORCTL *pCtl;

    if (hVector == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    free(pCtl->pbData);
    pCtl->ulMagic = 0;
    free(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Adding elements
 * ================================================================== */

APIRET APIENTRY VectorAdd(HVECTOR hVector, PCVOID pElem) {
    VECTORCTL *pCtl;
    APIRET rc;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    rc = vector_reserve(pCtl, pCtl->ulCount + 1);
    if (rc != NO_ERROR) return rc;

    memcpy(element_at(pCtl, pCtl->ulCount),
           pElem, (size_t)pCtl->ulElemSize);
    pCtl->ulCount++;
    return NO_ERROR;
}

APIRET APIENTRY VectorGrow(HVECTOR hVector, PPVOID ppNewElem) {
    VECTORCTL *pCtl;
    APIRET rc;

    if (!ppNewElem) return ERROR_INVALID_PARAMETER;
    *ppNewElem = NULL;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    rc = vector_reserve(pCtl, pCtl->ulCount + 1);
    if (rc != NO_ERROR) return rc;

    *ppNewElem = element_at(pCtl, pCtl->ulCount);
    memset(*ppNewElem, 0, (size_t)pCtl->ulElemSize);
    pCtl->ulCount++;
    return NO_ERROR;
}

/* ==================================================================
 * Access
 * ================================================================== */

APIRET APIENTRY VectorGetCount(HVECTOR hVector, PULONG pulCount) {
    VECTORCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pulCount = (ULONG)pCtl->ulCount;
    return NO_ERROR;
}

APIRET APIENTRY VectorGetPtr(HVECTOR hVector, ULONG ulIndex, PPVOID ppElem) {
    VECTORCTL *pCtl;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    *ppElem = NULL;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    *ppElem = element_at(pCtl, (unsigned long)ulIndex);
    return NO_ERROR;
}

/* ==================================================================
 * Sorting
 * ================================================================== */

/*
 * Stable bottom-up merge sort over an array of fixed-size elements.
 * The comparator receives pointers to two stored elements. A
 * temporary buffer of the same size as the data is used for merging.
 *
 * A private implementation is used instead of qsort because qsort
 * has no user context in C89 and using a file-scope global for the
 * comparator would break thread safety.
 */

static void merge_runs(unsigned char *pbBase,
                       unsigned char *pbTmp,
                       unsigned long ulElemSize,
                       unsigned long ulLo,
                       unsigned long ulMid,
                       unsigned long ulHi,
                       VECTORCOMPAREFN pfnCmp)
{
    unsigned long i = ulLo;
    unsigned long j = ulMid;
    unsigned long k = ulLo;

    while (i < ulMid && j < ulHi) {
        PCVOID pa = pbBase + (size_t)i * (size_t)ulElemSize;
        PCVOID pb = pbBase + (size_t)j * (size_t)ulElemSize;
        if (pfnCmp(pa, pb) <= 0) {
            memcpy(pbTmp + (size_t)k * (size_t)ulElemSize,
                   pa, (size_t)ulElemSize);
            i++;
        } else {
            memcpy(pbTmp + (size_t)k * (size_t)ulElemSize,
                   pb, (size_t)ulElemSize);
            j++;
        }
        k++;
    }
    while (i < ulMid) {
        memcpy(pbTmp + (size_t)k * (size_t)ulElemSize,
               pbBase + (size_t)i * (size_t)ulElemSize,
               (size_t)ulElemSize);
        i++; k++;
    }
    while (j < ulHi) {
        memcpy(pbTmp + (size_t)k * (size_t)ulElemSize,
               pbBase + (size_t)j * (size_t)ulElemSize,
               (size_t)ulElemSize);
        j++; k++;
    }
}

APIRET APIENTRY VectorSort(HVECTOR hVector, VECTORCOMPAREFN pfnCmp) {
    VECTORCTL *pCtl;
    unsigned char *pbTmp;
    unsigned long ulWidth;

    if (!pfnCmp) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pCtl->ulCount < 2) return NO_ERROR;

    pbTmp = (unsigned char *)malloc((size_t)pCtl->ulCount *
                                    (size_t)pCtl->ulElemSize);
    if (!pbTmp) return ERROR_NOT_ENOUGH_MEMORY;

    for (ulWidth = 1; ulWidth < pCtl->ulCount; ulWidth *= 2) {
        unsigned long ulLo;
        for (ulLo = 0; ulLo < pCtl->ulCount; ulLo += 2 * ulWidth) {
            unsigned long ulMid = ulLo + ulWidth;
            unsigned long ulHi  = ulLo + 2 * ulWidth;
            if (ulMid > pCtl->ulCount) ulMid = pCtl->ulCount;
            if (ulHi  > pCtl->ulCount) ulHi  = pCtl->ulCount;

            merge_runs(pCtl->pbData, pbTmp,
                       pCtl->ulElemSize,
                       ulLo, ulMid, ulHi, pfnCmp);
            /* Copy the merged run back. */
            memcpy(pCtl->pbData + (size_t)ulLo * (size_t)pCtl->ulElemSize,
                   pbTmp + (size_t)ulLo * (size_t)pCtl->ulElemSize,
                   (size_t)(ulHi - ulLo) * (size_t)pCtl->ulElemSize);
        }
    }

    free(pbTmp);
    return NO_ERROR;
}
