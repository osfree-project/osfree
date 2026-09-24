/*!
 *
 * @file vector.c
 *
 * @brief Implementation of the vector container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 */

#include <stdlib.h>
#include <string.h>
#include "vector.h"

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
    unsigned long ulMagic;       /*!< CCL_VECTOR_MAGIC.              */
    unsigned long ulElemSize;    /*!< Size of one element in bytes.  */
    unsigned long ulCount;       /*!< Number of elements stored.     */
    unsigned long ulCapacity;    /*!< Allocated element slots.       */
    unsigned char *pbData;       /*!< Element storage, or NULL.      */
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
    VECTORCTL *pCtl;
    if (hVector == NULLHANDLE) return NULL;
    pCtl = (VECTORCTL *)hVector;
    if (pCtl->ulMagic != CCL_VECTOR_MAGIC) return NULL;
    return pCtl;
}

/*!
 * @brief Return the address of an element inside the storage block.
 *
 * No bounds check is performed; the caller must ensure that ulIndex
 * is below ulCount.
 *
 * @param[in] pCtl     Control block.
 * @param[in] ulIndex  Element index.
 *
 * @return A pointer to the element inside pbData.
 */
static unsigned char *element_at(VECTORCTL *pCtl, unsigned long ulIndex) {
    return pCtl->pbData + (size_t)ulIndex * (size_t)pCtl->ulElemSize;
}

/*!
 * @brief Ensure that at least ulNeeded element slots are allocated.
 *
 * Grows the storage block if necessary. Capacity is doubled until
 * the requested size is reached, starting at 8 slots.
 *
 * @param[in,out] pCtl      Control block.
 * @param[in]     ulNeeded  Minimum number of element slots required.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
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

/*!
 * @brief Create a new vector.
 *
 * @param[in]  ulElemSize  Size of one element in bytes. Must be
 *                         nonzero.
 * @param[out] phVector    Receives the new vector handle. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phVector is NULL, or ulElemSize
 *                                  is zero.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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

/*!
 * @brief Destroy a vector.
 *
 * Releases the storage block and the control block. Does not touch
 * element contents; the caller must have released any owned memory
 * first.
 *
 * @param[in] hVector  Vector handle. NULL is accepted and treated as
 *                     success.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR               Success (including NULL handle).
 * @retval ERROR_INVALID_HANDLE   The handle is not a valid vector.
 */
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

/*!
 * @brief Append one element to the vector.
 *
 * The element is copied into the vector's storage; the caller keeps
 * ownership of the source buffer.
 *
 * @param[in,out] hVector  Vector handle.
 * @param[in]     pElem    Pointer to the element to copy. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
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

/*!
 * @brief Return the number of elements stored in the vector.
 *
 * @param[in]  hVector   Vector handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
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
 * Follows the size-query convention: when pBuf is NULL and ulSize is
 * zero, only *pulUsed is filled in. When the buffer is too small,
 * *pulUsed receives the required size and ERROR_BUFFER_OVERFLOW is
 * returned.
 *
 * @param[in]  hVector   Vector handle.
 * @param[in]  ulIndex   Element index.
 * @param[out] pBuf      Destination buffer, or NULL for size-query.
 * @param[in]  ulSize    Size of pBuf in bytes.
 * @param[out] pulUsed   Optional. Receives the used or required size.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
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
