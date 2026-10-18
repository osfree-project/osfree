/*!
 *
 * @file stack.c
 *
 * @brief Implementation of the stack container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * Part of the ccl container library.
 *
 * The stack is a thin profile over vector.h. All storage management
 * (growth, copy, reallocation) lives in vector.c; this file only:
 *
 *   - owns the STACKCTL control block;
 *   - validates the caller's CCLOPTS with the stack-specific
 *     allow/require mask pair;
 *   - restricts the public API to LIFO operations (Push, Pop, Top,
 *     Clear, GetCount);
 *   - delegates every operation to the corresponding Vector*
 *     function.
 *
 * The underlying vector carries the pDestroyValue callback from
 * CCLOPTS, so VectorDestroy / VectorRemoveLast / VectorSet already
 * invoke it. The stack does not duplicate that logic; it relies on
 * the vector contract. StackTop does not call destroy because it
 * does not remove.
 */

#include <string.h>
#include "stack.h"
#include "vector.h"
#include "ccl_alloc.h"
#include "ccl_base_private.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @def CCL_STACK_MAGIC
 * @brief Magic value identifying a valid stack control block.
 *
 * The literal bytes are "STAK".
 */
#define CCL_STACK_MAGIC 0x5354414BUL  /* "STAK" */

/*!
 * @struct _STACKCTL
 * @brief Control block of an open stack.
 *
 * ulElemSize is cached for the size-query path of StackPop and
 * StackTop, where the vector API would otherwise require an extra
 * call.
 */
typedef struct _STACKCTL {
    unsigned long ulMagic;      /*!< CCL_STACK_MAGIC.               */
    unsigned long ulElemSize;   /*!< Size of one element in bytes.  */
    HVECTOR       hVector;      /*!< Underlying vector.             */
} STACKCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a stack handle.
 *
 * @param[in] hStack  Stack handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hStack is NULL, or its magic value does not match.
 */
static STACKCTL *get_ctl(HSTACK hStack) {
    return (STACKCTL *)ccl_validate(hStack, CCL_STACK_MAGIC);
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new stack.
 *
 * @param[in]  ulElemSize     Size of one element in bytes.
 * @param[in]  ulInitialSize  Initial capacity. 0 means lazy.
 * @param[in]  pOpts          Option block. May be NULL.
 * @param[out] phStack        Receives the new handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument or pOpts is invalid.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StackCreate(ULONG ulElemSize, ULONG ulInitialSize,
                            PCCLOPTS pOpts, PHSTACK phStack) {
    STACKCTL *pCtl;
    APIRET rc;

    if (!phStack) return ERROR_INVALID_PARAMETER;
    if (ulElemSize == 0) return ERROR_INVALID_PARAMETER;
    *phStack = NULLHANDLE;

    rc = CclValidateOpts(pOpts,
                         CCL_OPT_DESTROY_VALUE
                         | CCL_OPT_FLAG_MANAGED
                         | CCL_OPT_FLAG_READONLY,
                         0);
    if (rc != NO_ERROR) return rc;

    pCtl = (STACKCTL *)(*CclCalloc)(1, sizeof(STACKCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    rc = VectorCreate(ulElemSize, ulInitialSize, pOpts, &pCtl->hVector);
    if (rc != NO_ERROR) {
        (*CclFree)(pCtl);
        return rc;
    }

    pCtl->ulMagic    = CCL_STACK_MAGIC;
    pCtl->ulElemSize = (unsigned long)ulElemSize;

    *phStack = (HSTACK)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a stack.
 *
 * @param[in] hStack  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY StackDestroy(HSTACK hStack) {
    STACKCTL *pCtl;

    if (hStack == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hStack);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    VectorDestroy(pCtl->hVector);
    pCtl->ulMagic = 0;
    (*CclFree)(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Push, pop, top
 * ================================================================== */

/*!
 * @brief Push a copy of an element onto the stack.
 *
 * @param[in] hStack  Handle.
 * @param[in] pElem   Pointer to the element to copy. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or pElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StackPush(HSTACK hStack, PCVOID pElem) {
    STACKCTL *pCtl;

    if (!pElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hStack);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return VectorAdd(pCtl->hVector, pElem);
}

/*!
 * @brief Pop the top element into a caller-supplied buffer.
 *
 * @param[in]  hStack  Handle.
 * @param[out] pBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize  Size of pBuf in bytes.
 * @param[out] pulUsed Optional. Receives the used or required size.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack is NULL, or pBuf is NULL
 *                                  without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The stack is empty.
 * @retval ERROR_BUFFER_OVERFLOW    pBuf is too small.
 */
APIRET APIENTRY StackPop(HSTACK hStack, PVOID pBuf, ULONG ulSize,
                         PULONG pulUsed) {
    STACKCTL *pCtl;
    ULONG ulCount = 0;
    APIRET rc;

    pCtl = get_ctl(hStack);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Size-query path: no removal. */
    if (pBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = pCtl->ulElemSize;
        return NO_ERROR;
    }
    if (!pBuf) return ERROR_INVALID_PARAMETER;

    rc = VectorGetCount(pCtl->hVector, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    /* Copy the top element out. */
    rc = VectorGetItem(pCtl->hVector, ulCount - 1, pBuf, ulSize, pulUsed);
    if (rc != NO_ERROR) return rc;

    /* Remove it. VectorRemoveLast calls pDestroyValue if configured
       in the underlying vector (from CCLOPTS at StackCreate time). */
    return VectorRemoveLast(pCtl->hVector);
}

/*!
 * @brief Return a borrowed pointer to the top element.
 *
 * @param[in]  hStack  Handle.
 * @param[out] ppElem  Receiver for the borrowed pointer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hStack or ppElem is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      The stack is empty.
 */
APIRET APIENTRY StackTop(HSTACK hStack, PVOID *ppElem) {
    STACKCTL *pCtl;
    ULONG ulCount = 0;
    APIRET rc;

    if (!ppElem) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hStack);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    rc = VectorGetCount(pCtl->hVector, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    return VectorGetRef(pCtl->hVector, ulCount - 1, ppElem);
}

/* ==================================================================
 * Metadata and bulk operations
 * ================================================================== */

/*!
 * @brief Return the number of elements in the stack.
 *
 * @param[in]  hStack    Handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY StackGetCount(HSTACK hStack, PULONG pulCount) {
    STACKCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hStack);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return VectorGetCount(pCtl->hVector, pulCount);
}

/*!
 * @brief Remove every element from the stack.
 *
 * @param[in] hStack  Handle.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success. Also when already empty.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY StackClear(HSTACK hStack) {
    STACKCTL *pCtl;
    APIRET rc;

    pCtl = get_ctl(hStack);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Pop until empty. VectorRemoveLast invokes pDestroyValue per
       element when configured. */
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
