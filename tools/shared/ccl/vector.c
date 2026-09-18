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

APIRET APIENTRY VectorGetItem(HVECTOR hVector, ULONG ulIndex,
                              PVOID pBuf, ULONG ulSize, PULONG pulUsed) {
    VECTORCTL *pCtl;
    size_t elem_size;

    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    elem_size = (size_t)pCtl->ulElemSize;

    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)elem_size;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    if ((size_t)ulSize < elem_size) {
        if (pulUsed) *pulUsed = (ULONG)elem_size;
        return ERROR_BUFFER_OVERFLOW;
    }

    memcpy(pBuf, element_at(pCtl, (unsigned long)ulIndex), elem_size);
    if (pulUsed) *pulUsed = (ULONG)elem_size;
    return NO_ERROR;
}
