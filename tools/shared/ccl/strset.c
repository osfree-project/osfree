/*!
 *
 * @file strset.c
 *
 * @brief Implementation of the string set container.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * The string set is built on top of the ccl vector container. It
 * stores unique NUL-terminated strings owned by the set, and offers
 * enumeration cursors that stay valid across set mutations.
 */

#include <stdlib.h>
#include <string.h>
#include "strset.h"
#include "vector.h"

/* ==================================================================
 * Internal control blocks
 * ================================================================== */

/*!
 * @def CCL_STRSET_MAGIC
 * @brief Magic value identifying a valid string-set handle.
 *
 * The literal bytes are "SSTS".
 */
#define CCL_STRSET_MAGIC       0x53535453UL  /* "SSTS" */

/*!
 * @def CCL_STRSETENUM_MAGIC
 * @brief Magic value identifying a valid enumeration cursor.
 *
 * The literal bytes are "SENU".
 */
#define CCL_STRSETENUM_MAGIC   0x53454E55UL  /* "SENU" */

/*!
 * @brief Forward declaration of the string-set control block.
 */
typedef struct _STRSETCTL   STRSETCTL;

/*!
 * @brief Forward declaration of the enumeration cursor control block.
 */
typedef struct _STRSETENUM  STRSETENUM;

/*!
 * @struct _STRSETCTL
 * @brief Control block of an open string set.
 */
struct _STRSETCTL {
    unsigned long ulMagic;      /*!< CCL_STRSET_MAGIC.              */
    HVECTOR       hVector;      /*!< Underlying vector (char*).     */
    STRSETENUM   *pFirstEnum;   /*!< Head of open cursors, or NULL. */
};

/*!
 * @struct _STRSETENUM
 * @brief Control block of an open enumeration cursor.
 */
struct _STRSETENUM {
    unsigned long ulMagic;      /*!< CCL_STRSETENUM_MAGIC.          */
    STRSETCTL    *pSet;         /*!< Owning set.                    */
    ULONG         ulIndex;      /*!< Current position.              */
    STRSETENUM   *pNext;        /*!< Next cursor of the owning set. */
};

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Return the control block behind a string-set handle.
 *
 * @param[in] hSet  String-set handle.
 *
 * @return The control block, or NULL on failure.
 *
 * @retval NULL  hSet is NULL, or its magic value does not match.
 */
static STRSETCTL *get_ctl(HSTRSET hSet) {
    STRSETCTL *pCtl;
    if (hSet == NULLHANDLE) return NULL;
    pCtl = (STRSETCTL *)hSet;
    if (pCtl->ulMagic != CCL_STRSET_MAGIC) return NULL;
    return pCtl;
}

/*!
 * @brief Return the control block behind an enumeration cursor.
 *
 * @param[in] hEnum  Cursor handle.
 *
 * @return The cursor control block, or NULL on failure.
 *
 * @retval NULL  hEnum is NULL, or its magic value does not match.
 */
static STRSETENUM *get_enum(HSTRSETENUM hEnum) {
    STRSETENUM *pEnum;
    if (hEnum == NULLHANDLE) return NULL;
    pEnum = (STRSETENUM *)hEnum;
    if (pEnum->ulMagic != CCL_STRSETENUM_MAGIC) return NULL;
    return pEnum;
}

/*!
 * @brief Copy the stored pointer at ulIndex into *ppszStr.
 *
 * @param[in]  pCtl      Control block.
 * @param[in]  ulIndex   Element index.
 * @param[out] ppszStr   Receives the stored pointer. Not NULL. On
 *                       failure set to NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   The underlying vector handle is
 *                                invalid.
 * @retval ERROR_NO_MORE_ITEMS    ulIndex is beyond the last element.
 */
static APIRET get_stored_ptr(STRSETCTL *pCtl, ULONG ulIndex, PCSZ *ppszStr) {
    PCSZ pStored = NULL;
    APIRET rc;
    *ppszStr = NULL;
    rc = VectorGetItem(pCtl->hVector, ulIndex, &pStored,
                       (ULONG)sizeof(pStored), NULL);
    if (rc != NO_ERROR) return rc;
    *ppszStr = pStored;
    return NO_ERROR;
}

/*!
 * @brief Linear search for a string equal to pszStr.
 *
 * @param[in]  pCtl       Control block.
 * @param[in]  pszStr     String to find.
 * @param[out] pulIndex   Optional. Receives the position when found.
 *
 * @return TRUE if the string is found, FALSE otherwise.
 *
 * @retval TRUE   The string is present; *pulIndex holds its position.
 * @retval FALSE  The string is not present, or the count could not
 *                be obtained.
 */
static BOOL find_string(STRSETCTL *pCtl, PCSZ pszStr, PULONG pulIndex) {
    ULONG ulCount = 0;
    ULONG i;

    if (VectorGetCount(pCtl->hVector, &ulCount) != NO_ERROR)
        return FALSE;

    for (i = 0; i < ulCount; i++) {
        PCSZ pStored = NULL;
        if (get_stored_ptr(pCtl, i, &pStored) != NO_ERROR)
            return FALSE;
        if (pStored && strcmp(pStored, pszStr) == 0) {
            if (pulIndex) *pulIndex = i;
            return TRUE;
        }
    }
    return FALSE;
}

/*!
 * @brief Common implementation of StrSetGetItem and StrSetEnumGet.
 *
 * Copies the string stored at ulIndex into the caller's buffer,
 * following the size-query convention.
 *
 * @param[in]  pCtl      Control block.
 * @param[in]  ulIndex   Element index.
 * @param[out] pszBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. Receives the used or required size.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszBuf is NULL without size-query.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is beyond the last string,
 *                                  or the stored pointer is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf is too small.
 */
static APIRET copy_index_string(STRSETCTL *pCtl, ULONG ulIndex,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PCSZ pStored = NULL;
    APIRET rc;
    size_t n;

    rc = get_stored_ptr(pCtl, ulIndex, &pStored);
    if (rc != NO_ERROR) return rc;
    if (!pStored) return ERROR_NO_MORE_ITEMS;

    n = strlen(pStored);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < n + 1) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pStored, n);
    pszBuf[n] = '\0';
    if (pulUsed) *pulUsed = (ULONG)n;
    return NO_ERROR;
}

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create a new string set.
 *
 * @param[out] phSet  Receives the new set handle. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phSet is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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
    pCtl->pFirstEnum = NULL;
    *phSet = (HSTRSET)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Destroy a string set.
 *
 * Releases every stored string, every open cursor of the set, and
 * the set itself. A NULL handle is accepted and treated as success.
 *
 * @param[in] hSet  String-set handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR               Success (including NULL handle).
 * @retval ERROR_INVALID_HANDLE   The handle is not a valid set.
 */
APIRET APIENTRY StrSetDestroy(HSTRSET hSet) {
    STRSETCTL *pCtl;
    ULONG ulCount = 0;
    ULONG i;

    if (hSet == NULLHANDLE) return NO_ERROR;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    /* Release every open cursor of this set first. */
    {
        STRSETENUM *pEnum = pCtl->pFirstEnum;
        while (pEnum) {
            STRSETENUM *pNext = pEnum->pNext;
            pEnum->ulMagic = 0;
            pEnum->pSet = NULL;
            free(pEnum);
            pEnum = pNext;
        }
        pCtl->pFirstEnum = NULL;
    }

    if (VectorGetCount(pCtl->hVector, &ulCount) == NO_ERROR) {
        for (i = 0; i < ulCount; i++) {
            PCSZ pStored = NULL;
            if (get_stored_ptr(pCtl, i, &pStored) == NO_ERROR)
                free((void *)pStored);
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

/*!
 * @brief Add a string to the set.
 *
 * The string is copied. If an equal string is already present, the
 * call is a no-op. An empty string is ignored.
 *
 * @param[in,out] hSet    String-set handle.
 * @param[in]     pszStr  String to add. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success (including empty string).
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid set.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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

/*!
 * @brief Test whether the set contains a given string.
 *
 * @param[in]  hSet     String-set handle.
 * @param[in]  pszStr   String to look for. Not NULL.
 * @param[out] pfFound  Receives TRUE or FALSE. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszStr or pfFound is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid set.
 */
APIRET APIENTRY StrSetContains(HSTRSET hSet, PCSZ pszStr, PBOOL pfFound) {
    STRSETCTL *pCtl;

    if (!pszStr || !pfFound) return ERROR_INVALID_PARAMETER;
    *pfFound = FALSE;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    *pfFound = find_string(pCtl, pszStr, NULL) ? TRUE : FALSE;
    return NO_ERROR;
}

/*!
 * @brief Return the number of strings in the set.
 *
 * @param[in]  hSet      String-set handle.
 * @param[out] pulCount  Receives the count. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid set.
 */
APIRET APIENTRY StrSetGetCount(HSTRSET hSet, PULONG pulCount) {
    STRSETCTL *pCtl;

    if (!pulCount) return ERROR_INVALID_PARAMETER;
    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return VectorGetCount(pCtl->hVector, pulCount);
}

/*!
 * @brief Copy the string at a given index into a caller buffer.
 *
 * @param[in]  hSet      String-set handle.
 * @param[in]  ulIndex   Element index.
 * @param[out] pszBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. Receives the used or required size.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszBuf is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid set.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is beyond the last string.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf is too small.
 */
APIRET APIENTRY StrSetGetItem(HSTRSET hSet, ULONG ulIndex,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    STRSETCTL *pCtl;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    return copy_index_string(pCtl, ulIndex, pszBuf, ulSize, pulUsed);
}

/* ==================================================================
 * Enumeration
 * ================================================================== */

/*!
 * @brief Open an enumeration cursor on the first string of the set.
 *
 * @param[in]  hSet     String-set handle.
 * @param[out] phEnum   Receives the cursor handle. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phEnum is NULL.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid set.
 * @retval ERROR_NO_MORE_ITEMS      The set is empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY StrSetEnumFirst(HSTRSET hSet, HSTRSETENUM *phEnum) {
    STRSETCTL *pCtl;
    STRSETENUM *pEnum;
    ULONG ulCount = 0;

    if (!phEnum) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;

    pCtl = get_ctl(hSet);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (VectorGetCount(pCtl->hVector, &ulCount) != NO_ERROR)
        return ERROR_INVALID_HANDLE;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    pEnum = (STRSETENUM *)calloc(1, sizeof(STRSETENUM));
    if (!pEnum) return ERROR_NOT_ENOUGH_MEMORY;

    pEnum->ulMagic = CCL_STRSETENUM_MAGIC;
    pEnum->pSet = pCtl;
    pEnum->ulIndex = 0;
    pEnum->pNext = pCtl->pFirstEnum;
    pCtl->pFirstEnum = pEnum;

    *phEnum = (HSTRSETENUM)pEnum;
    return NO_ERROR;
}

/*!
 * @brief Advance an enumeration cursor to the next string.
 *
 * @param[in,out] hEnum  Cursor handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR               Success.
 * @retval ERROR_INVALID_HANDLE   The handle is not a valid cursor,
 *                                or its owning set is gone.
 * @retval ERROR_NO_MORE_ITEMS    The cursor is already on the last
 *                                string.
 */
APIRET APIENTRY StrSetEnumNext(HSTRSETENUM hEnum) {
    STRSETENUM *pEnum = get_enum(hEnum);
    ULONG ulCount = 0;

    if (!pEnum) return ERROR_INVALID_HANDLE;
    if (!pEnum->pSet) return ERROR_INVALID_HANDLE;

    if (VectorGetCount(pEnum->pSet->hVector, &ulCount) != NO_ERROR)
        return ERROR_INVALID_HANDLE;

    if (pEnum->ulIndex + 1 >= ulCount) {
        return ERROR_NO_MORE_ITEMS;
    }
    pEnum->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Copy the string at the cursor position.
 *
 * @param[in]  hEnum     Cursor handle.
 * @param[out] pszBuf    Destination buffer, or NULL for size-query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. Receives the used or required size.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszBuf is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     The handle is not a valid cursor.
 * @retval ERROR_NO_MORE_ITEMS      The cursor position is invalid.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf is too small.
 */
APIRET APIENTRY StrSetEnumGet(HSTRSETENUM hEnum,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    STRSETENUM *pEnum = get_enum(hEnum);

    if (!pEnum) return ERROR_INVALID_HANDLE;
    if (!pEnum->pSet) return ERROR_INVALID_HANDLE;

    return copy_index_string(pEnum->pSet, pEnum->ulIndex,
                             pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Close an enumeration cursor.
 *
 * A NULL handle is accepted and treated as success.
 *
 * @param[in] hEnum  Cursor handle.
 *
 * @return NO_ERROR on success.
 *
 * @retval NO_ERROR  Success (including NULL handle).
 */
APIRET APIENTRY StrSetEnumClose(HSTRSETENUM hEnum) {
    STRSETENUM *pEnum = get_enum(hEnum);

    if (!pEnum) return NO_ERROR;

    if (pEnum->pSet && pEnum->pSet->pFirstEnum) {
        STRSETENUM **pp = &pEnum->pSet->pFirstEnum;
        while (*pp) {
            if (*pp == pEnum) { *pp = pEnum->pNext; break; }
            pp = &(*pp)->pNext;
        }
    }
    pEnum->ulMagic = 0;
    pEnum->pSet = NULL;
    free(pEnum);
    return NO_ERROR;
}
