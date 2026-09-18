/* strset.c - set of strings built on top of vector.
 * Part of the ccl container library. */

/**
 * @file strset.c
 * @brief Implementation of the string set container.
 *
 * Copyright (c) osFree Project 2026, <http://www.osFree.org>
 *   for licence see licence.txt in root directory, or project website
 */

#include <stdlib.h>
#include <string.h>
#include "strset.h"
#include "vector.h"

/* ==================================================================
 * Internal control block
 * ================================================================== */

/** @brief Magic value identifying a valid string-set handle. */
#define CCL_STRSET_MAGIC 0x53535453UL  /* "SSTS" */

/**
 * @struct _STRSETCTL
 * @brief Control block of an open string set.
 */
typedef struct _STRSETCTL {
    unsigned long ulMagic;   /**< CCL_STRSET_MAGIC.          */
    HVECTOR       hVector;   /**< Underlying vector (char*). */
} STRSETCTL;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

static STRSETCTL *get_ctl(HSTRSET hSet) {
    STRSETCTL *pCtl;
    if (hSet == NULLHANDLE) return NULL;
    pCtl = (STRSETCTL *)hSet;
    if (pCtl->ulMagic != CCL_STRSET_MAGIC) return NULL;
    return pCtl;
}

/**
 * @brief Linear search for a string equal to pszStr.
 *
 * Returns TRUE_ if found and stores the position in *pulIndex.
 * Returns FALSE_ otherwise.
 */
static BOOL find_string(STRSETCTL *pCtl, PCSZ pszStr, PULONG pulIndex) {
    ULONG ulCount = 0;
    ULONG i;

    if (VectorGetCount(pCtl->hVector, &ulCount) != NO_ERROR)
        return FALSE_;

    for (i = 0; i < ulCount; i++) {
        PVOID pElem = NULL;
        if (VectorGetPtr(pCtl->hVector, i, &pElem) != NO_ERROR)
            return FALSE_;
        if (pElem && strcmp((PCSZ)pElem, pszStr) == 0) {
            if (pulIndex) *pulIndex = i;
            return TRUE_;
        }
    }
    return FALSE_;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

APIRET APIENTRY StrSetCreate(PHSTRSET phSet) {
    STRSETCTL *pCtl;
    APIRET rc;

    if (!phSet) return ERROR_INVALID_PARAMETER;
    *phSet = NULLHANDLE;

    pCtl = (STRSETCTL *)calloc(1, sizeof(STRSETCTL));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;

    rc = VectorCreate((ULONG)sizeof(char *), &pCtl->hVector);
    if (rc != NO_ERROR) {
        free(pCtl);
        return rc;
    }
    pCtl->ulMagic = CCL_STRSET_MAGIC;
    *phSet = (HSTRSET)pCtl;
    return NO_ERROR;
}

APIRET APIENTRY StrSetDestroy(HSTRSET hSet) {
    STRSETCTL *pCtl;
    ULONG ulCount = 0;
    ULONG i;

    if (hSet == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (VectorGetCount(pCtl->hVector, &ulCount) == NO_ERROR) {
        for (i = 0; i < ulCount; i++) {
            PVOID pElem = NULL;
            if (VectorGetPtr(pCtl->hVector, i, &pElem) == NO_ERROR)
                free(pElem);
        }
    }
    VectorDestroy(pCtl->hVector);
    pCtl->ulMagic = 0;
    free(pCtl);
    return NO_ERROR;
}

/* ==================================================================
 * Operations
 * ================================================================== */

APIRET APIENTRY StrSetAdd(HSTRSET hSet, PCSZ pszStr) {
    STRSETCTL *pCtl;
    char *copy;
    APIRET rc;

    if (!pszStr || !pszStr[0]) return NO_ERROR;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (find_string(pCtl, pszStr, NULL)) return NO_ERROR;

    copy = strdup(pszStr);
    if (!copy) return ERROR_NOT_ENOUGH_MEMORY;

    rc = VectorAdd(pCtl->hVector, &copy);
    if (rc != NO_ERROR) {
        free(copy);
        return rc;
    }
    return NO_ERROR;
}

APIRET APIENTRY StrSetContains(HSTRSET hSet, PCSZ pszStr, PBOOL pfFound) {
    STRSETCTL *pCtl;

    if (!pszStr || !pfFound) return ERROR_INVALID_PARAMETER;
    *pfFound = FALSE_;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pfFound = find_string(pCtl, pszStr, NULL) ? TRUE_ : FALSE_;
    return NO_ERROR;
}

APIRET APIENTRY StrSetGetCount(HSTRSET hSet, PULONG pulCount) {
    STRSETCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return VectorGetCount(pCtl->hVector, pulCount);
}

APIRET APIENTRY StrSetGetItem(HSTRSET hSet, ULONG ulIndex,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    STRSETCTL *pCtl;
    PVOID pElem = NULL;
    APIRET rc;
    size_t n;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    rc = VectorGetPtr(pCtl->hVector, ulIndex, &pElem);
    if (rc != NO_ERROR) return rc;
    if (!pElem) return ERROR_NO_MORE_ITEMS;

    n = strlen((PCSZ)pElem);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < n + 1) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pElem, n);
    pszBuf[n] = '\0';
    if (pulUsed) *pulUsed = (ULONG)n;
    return NO_ERROR;
}
