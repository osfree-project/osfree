/*!
 *
 * @file vector.c
 *
 * @brief Implementation of the vector container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * Storage is a single contiguous array, allocated through
 * (*CclMalloc) / (*CclRealloc) and released through (*CclFree).
 * Growth doubles the capacity. The storage array is allocated
 * lazily on the first VectorAdd if ulInitialSize was 0 at create
 * time.
 *
 * Ownership. If pDestroyValue was supplied at creation, it is
 * called on the stored bytes of an element whenever that element
 * is removed (VectorRemove, VectorRemoveLast), overwritten
 * (VectorSet), or left in the vector at VectorDestroy. It is NOT
 * called by VectorInsert (no element is removed) or VectorSetRef
 * (raw write, caller manages the old value).
 */

#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_VECTOR_MAGIC
 * @brief Magic value identifying a valid vector control block.
 *
 * The literal bytes are "VECT".
 */
#define CCL_VECTOR_MAGIC 0x56454354UL  /* "VECT" */

/*!
 * @struct _VECTORCTL
 * @brief Control block of an open vector.
 */
typedef struct _VECTORCTL {
    unsigned long  ulMagic;        /*!< CCL_VECTOR_MAGIC.              */
    unsigned long  ulElemSize;     /*!< Size of one element in bytes.  */
    unsigned long  ulCount;        /*!< Number of elements stored.     */
    unsigned long  ulCapacity;     /*!< Allocated element slots.       */
    unsigned char *pbData;         /*!< Element storage, or NULL.      */
    CCL_DESTROY_FN pDestroyValue;  /*!< Optional destroy callback.     */
} VECTORCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a vector handle.
 *
 * @param[in] hVector  Vector handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hVector is NULL, or its magic value does not match.
 */
static VECTORCTL *get_ctl(HVECTOR hVector) {
    return (VECTORCTL *)ccl_validate(hVector, CCL_VECTOR_MAGIC);
}

/*!
 * @brief Return the address of an element inside the storage block.
 *
 * No bounds check is performed; the caller must ensure that ulIndex
 * is below ulCount.
 *
 * @param[in] pCtl     Control block. Not NULL.
 * @param[in] ulIndex  Element index.
 *
 * @return A pointer to the element inside pbData.
 */
static unsigned char *element_at(VECTORCTL *pCtl, unsigned long ulIndex) {
    return pCtl->pbData + (size_t)ulIndex * (size_t)pCtl->ulElemSize;
}

/*!
 * @brief Test whether a pointer points inside the vector's storage.
 *
 * Used by the debug asserts in VectorAdd / VectorInsert / VectorSet /
 * VectorSetRef to catch the common error of passing an element of
 * the same vector as the source. In a release build this helper is
 * not called (the assert is compiled out); the error becomes
 * undefined behaviour (memcpy from freed storage after realloc).
 *
 * @param[in] pCtl  Control block. Not NULL.
 * @param[in] p     Pointer to test.
 *
 * @return TRUE if p lies within [pbData, pbData + capacity *
 *         elemSize), FALSE otherwise.
 *
 * @retval TRUE   p lies inside the vector's storage.
 * @retval FALSE  p is NULL, the vector has no storage yet, or p
 *                lies outside the current capacity range.
 */
static BOOL ptr_inside(VECTORCTL *pCtl, PCVOID p) {
    const unsigned char *pc = (const unsigned char *)p;
    if (!pCtl->pbData) return FALSE;
    if (pc < pCtl->pbData) return FALSE;
    if (pc >= pCtl->pbData + (size_t)pCtl->ulCapacity
                            * (size_t)pCtl->ulElemSize) return FALSE;
    return TRUE;
}

/*!
 * @brief Ensure that at least ulNeeded element slots are allocated.
 *
 * Grows the storage block if necessary. Capacity is doubled until
 * the requested size is reached, starting at 8 slots. The storage
 * is allocated through (*CclRealloc); on failure the old storage is
 * left untouched and the function returns
 * ERROR_NOT_ENOUGH_MEMORY.
 *
 * @param[in,out] pCtl      Control block. Not NULL.
 * @param[in]     ulNeeded  Minimum number of element slots required.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed, or the
 *                                  requested size would overflow
 *                                  the addressable range.
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

    /* Reject a capacity that would make the byte size overflow. */
    if (ulNewCapacity > (unsigned long)-1 / pCtl->ulElemSize)
        return ERROR_NOT_ENOUGH_MEMORY;

    pbNew = (unsigned char *)(*CclRealloc)(pCtl->pbData,
                                           ulNewCapacity * pCtl->ulElemSize);
    if (!pbNew) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->pbData = pbNew;
    pCtl->ulCapacity = ulNewCapacity;
    return NO_ERROR;
}

/*!
 * @brief Ensure the vector has exactly ulCapacity slots.
 *
 * If the current capacity is already >= ulNewCapacity, this is a
 * no-op. Otherwise the storage is reallocated to exactly
 * ulNewCapacity * ulElemSize bytes. Used by VectorReserveExact.
 *
 * @param[in,out] pCtl           Control block. Not NULL.
 * @param[in]     ulNewCapacity  Desired capacity.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed, or the byte
 *                                  size would overflow.
 */
static APIRET vector_reserve_exact(VECTORCTL *pCtl, unsigned long ulNewCapacity) {
    unsigned char *pbNew;
    unsigned long ulBytes;

    if (ulNewCapacity <= pCtl->ulCapacity) return NO_ERROR;
    if (ulNewCapacity == 0) return NO_ERROR;

    if (ulNewCapacity > (unsigned long)-1 / pCtl->ulElemSize)
        return ERROR_NOT_ENOUGH_MEMORY;
    ulBytes = ulNewCapacity * pCtl->ulElemSize;

    pbNew = (unsigned char *)(*CclRealloc)(pCtl->pbData, ulBytes);
    if (!pbNew) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->pbData = pbNew;
    pCtl->ulCapacity = ulNewCapacity;
    return NO_ERROR;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new vector.
 *
 * @param[in]  ulElemSize     Size of one element in bytes. Must be
 *                            nonzero.
 * @param[in]  ulInitialSize  Initial capacity. 0 means lazy
 *                            allocation.
 * @param[in]  pOpts          Option block. May be NULL.
 * @param[out] phVector       Receives the new vector handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phVector is NULL, ulElemSize is
 *                                  zero, or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorCreate(ULONG ulElemSize, ULONG ulInitialSize,
                             PCCLOPTS pOpts, PHVECTOR phVector) {
    VECTORCTL *pCtl;
    APIRET rc;

    if (!phVector) return ERROR_INVALID_PARAMETER;
    if (ulElemSize == 0) return ERROR_INVALID_PARAMETER;
    *phVector = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         0);
    if (rc != NO_ERROR) return rc;

    pCtl = (VECTORCTL *)(*CclCalloc)(1, sizeof(VECTORCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    pCtl->ulMagic = CCL_VECTOR_MAGIC;
    pCtl->ulElemSize = (unsigned long)ulElemSize;
    if (pOpts) pCtl->pDestroyValue = pOpts->pDestroyValue;

    if (ulInitialSize > 0) {
        rc = vector_reserve_exact(pCtl, (unsigned long)ulInitialSize);
        if (rc != NO_ERROR) {
            (*CclFree)(pCtl);
            return rc;
        }
    }

    *phVector = (HVECTOR)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a vector.
 *
 * @param[in] hVector  Vector handle. NULL is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success (including NULL handle).
 * @retval ERROR_INVALID_HANDLE   The handle is not a valid vector.
 */
APIRET APIENTRY VectorDestroy(HVECTOR hVector) {
    VECTORCTL *pCtl;
    unsigned long i;

    if (hVector == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pCtl->pDestroyValue && pCtl->pbData) {
        for (i = 0; i < pCtl->ulCount; i++)
            pCtl->pDestroyValue(element_at(pCtl, i));
    }

    if (pCtl->pbData) (*CclFree)(pCtl->pbData);
    pCtl->ulMagic = 0;
    (*CclFree)(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Adding elements
 * ================================================================== */

/*!
 * @brief Append one element to the vector.
 *
 * @param[in,out] hVector  Vector handle.
 * @param[in]     pElem    Pointer to the element to copy. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorAdd(HVECTOR hVector, PCVOID pElem) {
    VECTORCTL *pCtl;
    APIRET rc;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Q4: pElem must not point inside this same vector. If it did,
       vector_reserve could realloc the storage out from under pElem
       and the memcpy below would read from freed memory. */
    if (ptr_inside(pCtl, pElem))
        return ERROR_INVALID_PARAMETER;

    rc = vector_reserve(pCtl, pCtl->ulCount + 1);
    if (rc != NO_ERROR) return rc;

    memcpy(element_at(pCtl, pCtl->ulCount),
           pElem, (size_t)pCtl->ulElemSize);
    pCtl->ulCount++;
    return NO_ERROR;
}

/*!
 * @brief Insert a copy at ulIndex, shifting the tail up.
 *
 * @param[in] hVector  Vector handle.
 * @param[in] ulIndex  Position. Must be <= current count.
 * @param[in] pElem    Pointer to the element to copy. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pElem is NULL, or pElem points
 *                                  inside the same vector.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex > current count.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorInsert(HVECTOR hVector, ULONG ulIndex, PCVOID pElem) {
    VECTORCTL *pCtl;
    APIRET rc;
    unsigned long ulTail;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex > pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    if (ptr_inside(pCtl, pElem))
        return ERROR_INVALID_PARAMETER;

    rc = vector_reserve(pCtl, pCtl->ulCount + 1);
    if (rc != NO_ERROR) return rc;

    if ((unsigned long)ulIndex < pCtl->ulCount) {
        ulTail = pCtl->ulCount - (unsigned long)ulIndex;
        memmove(element_at(pCtl, (unsigned long)ulIndex + 1),
                element_at(pCtl, (unsigned long)ulIndex),
                (size_t)ulTail * (size_t)pCtl->ulElemSize);
    }
    memcpy(element_at(pCtl, (unsigned long)ulIndex),
           pElem, (size_t)pCtl->ulElemSize);
    pCtl->ulCount++;
    return NO_ERROR;
}

/* ==================================================================
 * Removing elements
 * ================================================================== */

/*!
 * @brief Remove the element at ulIndex, shifting the tail down.
 *
 * @param[in] hVector  Vector handle.
 * @param[in] ulIndex  Position. Must be < current count.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY VectorRemove(HVECTOR hVector, ULONG ulIndex) {
    VECTORCTL *pCtl;
    unsigned long ulTail;

    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    if (pCtl->pDestroyValue)
        pCtl->pDestroyValue(element_at(pCtl, (unsigned long)ulIndex));

    ulTail = pCtl->ulCount - (unsigned long)ulIndex - 1;
    if (ulTail > 0) {
        memmove(element_at(pCtl, (unsigned long)ulIndex),
                element_at(pCtl, (unsigned long)ulIndex + 1),
                (size_t)ulTail * (size_t)pCtl->ulElemSize);
    }
    pCtl->ulCount--;
    return NO_ERROR;
}

/*!
 * @brief Remove the last element.
 *
 * @param[in] hVector  Vector handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      The vector is empty.
 */
APIRET APIENTRY VectorRemoveLast(HVECTOR hVector) {
    VECTORCTL *pCtl;

    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if (pCtl->ulCount == 0) return ERROR_NO_MORE_ITEMS;

    pCtl->ulCount--;
    if (pCtl->pDestroyValue)
        pCtl->pDestroyValue(element_at(pCtl, pCtl->ulCount));
    return NO_ERROR;
}

/* ==================================================================
 * Replacing elements
 * ================================================================== */

/*!
 * @brief Overwrite the element at ulIndex, calling pDestroyValue on
 *        the old value if supplied.
 *
 * @param[in] hVector  Vector handle.
 * @param[in] ulIndex  Position. Must be < current count.
 * @param[in] pElem    Pointer to the new element. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pElem is NULL, or pElem points
 *                                  inside the same vector.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY VectorSet(HVECTOR hVector, ULONG ulIndex, PCVOID pElem) {
    VECTORCTL *pCtl;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    if (ptr_inside(pCtl, pElem))
        return ERROR_INVALID_PARAMETER;

    if (pCtl->pDestroyValue)
        pCtl->pDestroyValue(element_at(pCtl, (unsigned long)ulIndex));

    memcpy(element_at(pCtl, (unsigned long)ulIndex),
           pElem, (size_t)pCtl->ulElemSize);
    return NO_ERROR;
}

/*!
 * @brief Overwrite the element at ulIndex without calling
 *        pDestroyValue.
 *
 * @param[in] hVector  Vector handle.
 * @param[in] ulIndex  Position. Must be < current count.
 * @param[in] pElem    Pointer to the new element. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pElem is NULL, or pElem points
 *                                  inside the same vector.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex >= current count.
 */
APIRET APIENTRY VectorSetRef(HVECTOR hVector, ULONG ulIndex, PCVOID pElem) {
    VECTORCTL *pCtl;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    if (ptr_inside(pCtl, pElem))
        return ERROR_INVALID_PARAMETER;

    memcpy(element_at(pCtl, (unsigned long)ulIndex),
           pElem, (size_t)pCtl->ulElemSize);
    return NO_ERROR;
}

/* ==================================================================
 * Access
 * ================================================================== */

/*!
 * @brief Return the number of elements stored in the vector.
 *
 * @param[in]  hVector   Vector handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 */
APIRET APIENTRY VectorGetCount(HVECTOR hVector, PULONG pulCount) {
    VECTORCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pulCount = (ULONG)pCtl->ulCount;
    return NO_ERROR;
}

/*!
 * @brief Copy the element at the given index into a caller buffer.
 *
 * @param[in]  hVector   Vector handle.
 * @param[in]  ulIndex   Element index.
 * @param[out] pBuf      Destination buffer, or NULL for size-query.
 * @param[in]  ulSize    Size of pBuf in bytes.
 * @param[out] pulUsed   Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pBuf is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is beyond the last element.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
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

/*!
 * @brief Return a borrowed pointer to the element at ulIndex.
 *
 * @param[in]  hVector  Vector handle.
 * @param[in]  ulIndex  Element index.
 * @param[out] ppElem   Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is beyond the last element.
 */
APIRET APIENTRY VectorGetRef(HVECTOR hVector, ULONG ulIndex, PVOID *ppElem) {
    VECTORCTL *pCtl;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    if ((unsigned long)ulIndex >= pCtl->ulCount)
        return ERROR_NO_MORE_ITEMS;

    *ppElem = element_at(pCtl, (unsigned long)ulIndex);
    return NO_ERROR;
}

/* ==================================================================
 * Capacity
 * ================================================================== */

/*!
 * @brief Ensure the vector can hold at least ulNewCapacity elements.
 *
 * @param[in] hVector        Vector handle.
 * @param[in] ulNewCapacity  Minimum capacity.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorReserve(HVECTOR hVector, ULONG ulNewCapacity) {
    VECTORCTL *pCtl;

    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return vector_reserve(pCtl, (unsigned long)ulNewCapacity);
}

/*!
 * @brief Ensure the vector has exactly ulNewCapacity slots.
 *
 * @param[in] hVector        Vector handle.
 * @param[in] ulNewCapacity  Exact capacity.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid vector.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY VectorReserveExact(HVECTOR hVector, ULONG ulNewCapacity) {
    VECTORCTL *pCtl;

    pCtl = get_ctl(hVector);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return vector_reserve_exact(pCtl, (unsigned long)ulNewCapacity);
}
