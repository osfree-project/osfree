/*!
 * @file spdx_db.c
 *
 * @brief Implementation of the SPDX license list database.
 *
 * SPDX license list database (C89, OpenWatcom). Conforms to:
 *   - SPDX License List.
 *     https://spdx.org/licenses/
 *   - SPDX 2.3, Annex D.2 (case-insensitive identifier comparison).
 *   - SPDX 2.3, Annex D (license expression grammar).
 *
 * The database is loaded from a directory that contains:
 *   - licenses.json
 *   - exceptions.json
 *   - details/<id>.json
 *   - exceptions/<id>.json
 *
 * A binary cache stores both the index and the details to avoid
 * re-parsing the JSON files on every run.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_db_private.h"
#include "json.h"
#include "sha1.h"
#include "spdx.h"

/*!
 * @def CACHE_MAGIC
 * @brief Cache file magic string.
 */
#define CACHE_MAGIC   "SPDXDB06"

/*!
 * @def CACHE_VERSION
 * @brief Cache file format version.
 */
#define CACHE_VERSION 7

/* ------------------------------------------------------------------ */
/* Internal structures                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @struct _LICENSELLIST
 * @brief Growable array of SPDXLICENSEENTRY records.
 */
typedef struct _LICENSELLIST {
    SPDXLICENSEENTRY *pItems;   /*!< Backing array.     */
    int               count;    /*!< Used entries.      */
    int               capacity; /*!< Allocated entries. */
} LICENSELLIST;

/*!
 * @struct _EXCEPTIONLIST
 * @brief Growable array of SPDXEXCEPTIONENTRY records.
 */
typedef struct _EXCEPTIONLIST {
    SPDXEXCEPTIONENTRY *pItems; /*!< Backing array.     */
    int                 count;  /*!< Used entries.      */
    int                 capacity;/*!< Allocated entries. */
} EXCEPTIONLIST;

static LICENSELLIST   g_Licenses;       /*!< Loaded license index.    */
static EXCEPTIONLIST  g_Exceptions;     /*!< Loaded exception index.  */

static FILE  *g_pCacheFp = NULL;        /*!< Open cache file, or NULL.*/
static PSZ    g_pszDetailsDir = NULL;   /*!< <db>/details.            */
static PSZ    g_pszExceptionsDir = NULL;/*!< <db>/exceptions.         */

/* ------------------------------------------------------------------ */
/* Utilities                                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Lower-case an ASCII letter.
 *
 * Unlike tolower(), the result does not depend on the current locale.
 * SPDX 2.3 Annex D.2 requires case-insensitive identifier comparison
 * on ASCII characters only.
 *
 * @param[in] c  Character.
 *
 * @return Lower-case equivalent for A-Z, unchanged otherwise.
 */
static int ascii_lower(int c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

/*!
 * @brief Case-insensitive comparison of two ASCII strings.
 *
 * @param[in] pszA  First string. Not NULL.
 * @param[in] pszB  Second string. Not NULL.
 *
 * @return Negative, zero or positive, following the usual ordering
 *         contract.
 */
static int id_cmp_ci(PCSZ pszA, PCSZ pszB) {
    while (*pszA && *pszB) {
        int ca = ascii_lower((unsigned char)*pszA);
        int cb = ascii_lower((unsigned char)*pszB);
        if (ca != cb) return ca - cb;
        pszA++;
        pszB++;
    }
    return (int)(unsigned char)*pszA - (int)(unsigned char)*pszB;
}

/*!
 * @brief Copy a NUL-terminated string into a caller-supplied buffer
 *        following the size-query convention.
 *
 * @param[in]  pszSrc   Source string. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
static APIRET copy_out(PCSZ pszSrc, PSZ pszBuf,
                       ULONG ulSize, PULONG pulUsed) {
    size_t cbLen;

    if (!pszSrc) return ERROR_INVALID_PARAMETER;
    cbLen = strlen(pszSrc);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < (ULONG)cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszSrc, cbLen + 1);
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/*!
 * @brief Free a NULL-terminated array of strings.
 *
 * @param[in] papszList  Array, or NULL.
 */
static void free_strlist(PSZ *papszList) {
    int i;
    if (!papszList) return;
    for (i = 0; papszList[i]; i++) free(papszList[i]);
    free(papszList);
}

/* ------------------------------------------------------------------ */
/* License and exception lists                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Initialize an empty license list.
 *
 * @param[out] pList  List. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET license_list_init(LICENSELLIST *pList) {
    pList->count = 0;
    pList->capacity = 16;
    pList->pItems = (SPDXLICENSEENTRY*)calloc((size_t)pList->capacity,
                                              sizeof(SPDXLICENSEENTRY));
    if (!pList->pItems) return ERROR_NOT_ENOUGH_MEMORY;
    return NO_ERROR;
}

/*!
 * @brief Append a new entry to a license list, growing it if needed.
 *
 * @param[in,out] pList    List. Not NULL.
 * @param[out]    ppEntry  Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET license_list_add(LICENSELLIST *pList,
                               PSPDXLICENSEENTRY *ppEntry) {
    PSPDXLICENSEENTRY pNew;
    *ppEntry = NULL;
    if (pList->count >= pList->capacity) {
        ULONG ulNewCapacity = (ULONG)pList->capacity * 2;
        pNew = (SPDXLICENSEENTRY*)realloc(pList->pItems,
            (size_t)ulNewCapacity * sizeof(SPDXLICENSEENTRY));
        if (!pNew) return ERROR_NOT_ENOUGH_MEMORY;
        pList->pItems = pNew;
        pList->capacity = (int)ulNewCapacity;
    }
    *ppEntry = &pList->pItems[pList->count++];
    memset(*ppEntry, 0, sizeof(**ppEntry));
    return NO_ERROR;
}

/*!
 * @brief Release all memory owned by a license list.
 *
 * @param[in,out] pList  List. Not NULL.
 */
static void license_list_free(LICENSELLIST *pList) {
    int i;
    for (i = 0; i < pList->count; i++) {
        PSPDXLICENSEENTRY pEntry = &pList->pItems[i];
        free(pEntry->pszId);
        free(pEntry->pszName);
        free_strlist(pEntry->papszSeeAlso);
        free(pEntry->pszText);
        free(pEntry->pszTemplate);
        free(pEntry->pszTextHtml);
    }
    free(pList->pItems);
    pList->pItems = NULL;
    pList->count = 0;
    pList->capacity = 0;
}

/*!
 * @brief Initialize an empty exception list.
 *
 * @param[out] pList  List. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET exception_list_init(EXCEPTIONLIST *pList) {
    pList->count = 0;
    pList->capacity = 16;
    pList->pItems = (SPDXEXCEPTIONENTRY*)calloc((size_t)pList->capacity,
                                                sizeof(SPDXEXCEPTIONENTRY));
    if (!pList->pItems) return ERROR_NOT_ENOUGH_MEMORY;
    return NO_ERROR;
}

/*!
 * @brief Append a new entry to an exception list, growing it if
 *        needed.
 *
 * @param[in,out] pList    List. Not NULL.
 * @param[out]    ppEntry  Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET exception_list_add(EXCEPTIONLIST *pList,
                                 PSPDXEXCEPTIONENTRY *ppEntry) {
    PSPDXEXCEPTIONENTRY pNew;
    *ppEntry = NULL;
    if (pList->count >= pList->capacity) {
        ULONG ulNewCapacity = (ULONG)pList->capacity * 2;
        pNew = (SPDXEXCEPTIONENTRY*)realloc(pList->pItems,
            (size_t)ulNewCapacity * sizeof(SPDXEXCEPTIONENTRY));
        if (!pNew) return ERROR_NOT_ENOUGH_MEMORY;
        pList->pItems = pNew;
        pList->capacity = (int)ulNewCapacity;
    }
    *ppEntry = &pList->pItems[pList->count++];
    memset(*ppEntry, 0, sizeof(**ppEntry));
    return NO_ERROR;
}

/*!
 * @brief Release all memory owned by an exception list.
 *
 * @param[in,out] pList  List. Not NULL.
 */
static void exception_list_free(EXCEPTIONLIST *pList) {
    int i;
    for (i = 0; i < pList->count; i++) {
        PSPDXEXCEPTIONENTRY pEntry = &pList->pItems[i];
        free(pEntry->pszId);
        free(pEntry->pszName);
        free_strlist(pEntry->papszSeeAlso);
        free(pEntry->pszText);
        free(pEntry->pszTemplate);
        free(pEntry->pszTextHtml);
    }
    free(pList->pItems);
    pList->pItems = NULL;
    pList->count = 0;
    pList->capacity = 0;
}

/* ------------------------------------------------------------------ */
/* Sorting and binary search                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief qsort comparator for license entries (case-insensitive).
 *
 * @param[in] pA  First entry.
 * @param[in] pB  Second entry.
 *
 * @return Negative, zero or positive.
 */
static int cmp_lic(const void *pA, const void *pB) {
    return id_cmp_ci(((const SPDXLICENSEENTRY*)pA)->pszId,
                     ((const SPDXLICENSEENTRY*)pB)->pszId);
}

/*!
 * @brief qsort comparator for exception entries (case-insensitive).
 *
 * @param[in] pA  First entry.
 * @param[in] pB  Second entry.
 *
 * @return Negative, zero or positive.
 */
static int cmp_exc(const void *pA, const void *pB) {
    return id_cmp_ci(((const SPDXEXCEPTIONENTRY*)pA)->pszId,
                     ((const SPDXEXCEPTIONENTRY*)pB)->pszId);
}

/*!
 * @brief Binary search lower bound in the license list.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return Index of the first entry not less than @p pszId.
 */
static int lic_lower_bound(PCSZ pszId) {
    int lo = 0, hi = g_Licenses.count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (id_cmp_ci(g_Licenses.pItems[mid].pszId, pszId) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

/*!
 * @brief Binary search lower bound in the exception list.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return Index of the first entry not less than @p pszId.
 */
static int exc_lower_bound(PCSZ pszId) {
    int lo = 0, hi = g_Exceptions.count;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (id_cmp_ci(g_Exceptions.pItems[mid].pszId, pszId) < 0)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

/* ------------------------------------------------------------------ */
/* Internal lookups                                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Find a license entry by identifier.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return Pointer to the entry, or NULL if not found.
 *
 * @retval NULL  Not found.
 */
static PSPDXLICENSEENTRY license_lookup(PCSZ pszId) {
    int idx;
    if (!pszId || g_Licenses.count == 0) return NULL;
    idx = lic_lower_bound(pszId);
    if (idx < g_Licenses.count &&
        id_cmp_ci(g_Licenses.pItems[idx].pszId, pszId) == 0)
        return &g_Licenses.pItems[idx];
    return NULL;
}

/*!
 * @brief Find an exception entry by identifier.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return Pointer to the entry, or NULL if not found.
 *
 * @retval NULL  Not found.
 */
static PSPDXEXCEPTIONENTRY exception_lookup(PCSZ pszId) {
    int idx;
    if (!pszId || g_Exceptions.count == 0) return NULL;
    idx = exc_lower_bound(pszId);
    if (idx < g_Exceptions.count &&
        id_cmp_ci(g_Exceptions.pItems[idx].pszId, pszId) == 0)
        return &g_Exceptions.pItems[idx];
    return NULL;
}

/* ------------------------------------------------------------------ */
/* JSON helpers                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Duplicate a JSON string node into a fresh buffer.
 *
 * @param[in] hNode  Node handle. May be NULLHANDLE.
 *
 * @return malloc'd string, or NULL if the node is absent, not a
 *         string, or on OOM.
 *
 * @retval NULL  Node is absent, not a string, or allocation failed.
 */
static PSZ json_dup_string(HJSONNODE hNode) {
    ULONG ulSize = 0;
    PSZ pszBuf;
    if (hNode == NULLHANDLE) return NULL;
    if (JsonNodeGetString(hNode, NULL, 0, &ulSize) != NO_ERROR) return NULL;
    if (ulSize == 0) return NULL;
    pszBuf = (PSZ)malloc(ulSize);
    if (!pszBuf) return NULL;
    if (JsonNodeGetString(hNode, pszBuf, ulSize, NULL) != NO_ERROR) {
        free(pszBuf);
        return NULL;
    }
    return pszBuf;
}

/*!
 * @brief Read a boolean field from a JSON object.
 *
 * @param[in]  hParent  Object handle. Not NULLHANDLE.
 * @param[in]  pszKey   Field name. Not NULL.
 * @param[out] pfValue  Receiver. Not NULL.
 *
 * @return 1 if the field exists and is a boolean, 0 otherwise.
 *
 * @retval 1  Field present and boolean.
 * @retval 0  Field missing or wrong type.
 */
static int json_get_bool(HJSONNODE hParent, PCSZ pszKey, PBOOL pfValue) {
    HJSONNODE hChild = NULLHANDLE;
    BOOL fVal = FALSE;
    if (hParent == NULLHANDLE) return 0;
    if (JsonNodeGetChild(hParent, pszKey, &hChild) != NO_ERROR) return 0;
    if (JsonNodeGetBoolean(hChild, &fVal) != NO_ERROR) return 0;
    *pfValue = fVal;
    return 1;
}

/*!
 * @brief Convert a JSON array of strings into a NULL-terminated
 *        array of malloc'd strings.
 *
 * @param[in] hArr  Array node, or NULLHANDLE.
 *
 * @return malloc'd array, or NULL if @p hArr is absent or not an
 *         array.
 *
 * @retval NULL  Array absent, wrong type, or allocation failure.
 */
static PSZ *parse_string_array(HJSONNODE hArr) {
    ULONG ulCount, ulType, i;
    PSZ *papszOut;
    if (hArr == NULLHANDLE) return NULL;
    if (JsonNodeGetType(hArr, &ulType) != NO_ERROR) return NULL;
    if (ulType != (ULONG)JSON_ARRAY) return NULL;
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) return NULL;
    papszOut = (PSZ*)malloc((size_t)(ulCount + 1) * sizeof(PSZ));
    if (!papszOut) return NULL;
    for (i = 0; i < ulCount; i++) {
        HJSONNODE hElem = NULLHANDLE;
        papszOut[i] = NULL;
        if (JsonNodeGetElement(hArr, i, &hElem) == NO_ERROR) {
            papszOut[i] = json_dup_string(hElem);
        }
    }
    papszOut[ulCount] = NULL;
    return papszOut;
}

/* ------------------------------------------------------------------ */
/* Index loading                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Load the licenses index from licenses.json.
 *
 * @param[in] pszPath  Path to licenses.json. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_INVALID_DATA       Malformed JSON or wrong structure.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET load_licenses_index(PCSZ pszPath) {
    PSZ pszHeap;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0;
    ULONG ulIdx;
    ULONG ulNeeded = 0;
    APIRET rc;

    rc = SpdxReadFileAll(pszPath, NULL, 0, &ulNeeded);
    if (rc != NO_ERROR) return rc;

    pszHeap = (PSZ)malloc(ulNeeded);
    if (!pszHeap) return ERROR_NOT_ENOUGH_MEMORY;

    rc = SpdxReadFileAll(pszPath, pszHeap, ulNeeded, NULL);
    if (rc != NO_ERROR) { free(pszHeap); return rc; }

    if (JsonParse(pszHeap, &hDoc) != NO_ERROR) {
        free(pszHeap);
        return ERROR_INVALID_DATA;
    }
    free(pszHeap);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) {
        JsonClose(hDoc);
        return ERROR_INVALID_DATA;
    }
    if (JsonNodeGetChild(hRoot, "licenses", &hArr) != NO_ERROR) {
        JsonClose(hDoc);
        return ERROR_INVALID_DATA;
    }
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) {
        JsonClose(hDoc);
        return ERROR_INVALID_DATA;
    }

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hItem = NULLHANDLE;
        HJSONNODE hChild = NULLHANDLE;
        PSPDXLICENSEENTRY pEntry;
        PSZ pszId;

        if (JsonNodeGetElement(hArr, ulIdx, &hItem) != NO_ERROR) continue;
        if (JsonNodeGetChild(hItem, "licenseId", &hChild) != NO_ERROR)
            continue;
        pszId = json_dup_string(hChild);
        if (!pszId) continue;

        rc = license_list_add(&g_Licenses, &pEntry);
        if (rc != NO_ERROR) { free(pszId); JsonClose(hDoc); return rc; }
        pEntry->pszId = pszId;

        if (JsonNodeGetChild(hItem, "name", &hChild) == NO_ERROR)
            pEntry->pszName = json_dup_string(hChild);

        {
            BOOL fVal = FALSE;
            if (json_get_bool(hItem, "isOsiApproved", &fVal) && fVal)
                pEntry->uchFlags |= SPDXDB_FLAG_OSI;
            if (json_get_bool(hItem, "isFsfLibre", &fVal) && fVal)
                pEntry->uchFlags |= SPDXDB_FLAG_FSF_LIBRE;
            if (json_get_bool(hItem, "isDeprecatedLicenseId", &fVal) && fVal)
                pEntry->uchFlags |= SPDXDB_FLAG_DEPRECATED;
        }

        if (JsonNodeGetChild(hItem, "seeAlso", &hChild) == NO_ERROR)
            pEntry->papszSeeAlso = parse_string_array(hChild);
    }

    JsonClose(hDoc);
    return NO_ERROR;
}

/*!
 * @brief Load the exceptions index from exceptions.json.
 *
 * @param[in] pszPath  Path to exceptions.json. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_INVALID_DATA       Malformed JSON or wrong structure.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET load_exceptions_index(PCSZ pszPath) {
    PSZ pszHeap;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0;
    ULONG ulIdx;
    ULONG ulNeeded = 0;
    APIRET rc;

    rc = SpdxReadFileAll(pszPath, NULL, 0, &ulNeeded);
    if (rc != NO_ERROR) return rc;

    pszHeap = (PSZ)malloc(ulNeeded);
    if (!pszHeap) return ERROR_NOT_ENOUGH_MEMORY;

    rc = SpdxReadFileAll(pszPath, pszHeap, ulNeeded, NULL);
    if (rc != NO_ERROR) { free(pszHeap); return rc; }

    if (JsonParse(pszHeap, &hDoc) != NO_ERROR) {
        free(pszHeap);
        return ERROR_INVALID_DATA;
    }
    free(pszHeap);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) {
        JsonClose(hDoc);
        return ERROR_INVALID_DATA;
    }
    if (JsonNodeGetChild(hRoot, "exceptions", &hArr) != NO_ERROR) {
        JsonClose(hDoc);
        return ERROR_INVALID_DATA;
    }
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) {
        JsonClose(hDoc);
        return ERROR_INVALID_DATA;
    }

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hItem = NULLHANDLE;
        HJSONNODE hChild = NULLHANDLE;
        PSPDXEXCEPTIONENTRY pEntry;
        PSZ pszId;

        if (JsonNodeGetElement(hArr, ulIdx, &hItem) != NO_ERROR) continue;
        if (JsonNodeGetChild(hItem, "licenseExceptionId", &hChild)
                != NO_ERROR)
            continue;
        pszId = json_dup_string(hChild);
        if (!pszId) continue;

        rc = exception_list_add(&g_Exceptions, &pEntry);
        if (rc != NO_ERROR) { free(pszId); JsonClose(hDoc); return rc; }
        pEntry->pszId = pszId;

        if (JsonNodeGetChild(hItem, "name", &hChild) == NO_ERROR)
            pEntry->pszName = json_dup_string(hChild);

        {
            BOOL fVal = FALSE;
            if (json_get_bool(hItem, "isDeprecatedLicenseId", &fVal) && fVal)
                pEntry->uchFlags |= SPDXDB_FLAG_DEPRECATED;
        }

        if (JsonNodeGetChild(hItem, "seeAlso", &hChild) == NO_ERROR)
            pEntry->papszSeeAlso = parse_string_array(hChild);
    }

    JsonClose(hDoc);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Binary cache primitives                                             */
/* ------------------------------------------------------------------ */

/*!
 * @brief Write a 32-bit value in little-endian order.
 *
 * @param[in] f        File. Not NULL.
 * @param[in] ulValue  Value to write.
 *
 * @return 0 on success, -1 on write error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int write_u32(FILE *f, ULONG ulValue) {
    unsigned char b[4];
    b[0] = (unsigned char)(ulValue & 0xFF);
    b[1] = (unsigned char)((ulValue >> 8) & 0xFF);
    b[2] = (unsigned char)((ulValue >> 16) & 0xFF);
    b[3] = (unsigned char)((ulValue >> 24) & 0xFF);
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

/*!
 * @brief Read a 32-bit little-endian value.
 *
 * @param[in]  f        File. Not NULL.
 * @param[out] pulOut   Receiver. Not NULL.
 *
 * @return 0 on success, -1 on read error.
 *
 * @retval 0   Success.
 * @retval -1  Read error.
 */
static int read_u32(FILE *f, PULONG pulOut) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return -1;
    *pulOut = (ULONG)b[0] |
              ((ULONG)b[1] << 8) |
              ((ULONG)b[2] << 16) |
              ((ULONG)b[3] << 24);
    return 0;
}

/*!
 * @brief Write one byte.
 *
 * @param[in] f         File. Not NULL.
 * @param[in] uchValue  Byte to write.
 *
 * @return 0 on success, -1 on write error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int write_u8v(FILE *f, UCHAR uchValue) {
    return fwrite(&uchValue, 1, 1, f) == 1 ? 0 : -1;
}

/*!
 * @brief Read one byte.
 *
 * @param[in]  f          File. Not NULL.
 * @param[out] puchValue  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on read error.
 *
 * @retval 0   Success.
 * @retval -1  Read error.
 */
static int read_u8v(FILE *f, PUCHAR puchValue) {
    return fread(puchValue, 1, 1, f) == 1 ? 0 : -1;
}

/*!
 * @brief Write a length-prefixed string.
 *
 * @param[in] f       File. Not NULL.
 * @param[in] pszStr  String, or NULL (treated as "").
 *
 * @return 0 on success, -1 on write error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int write_str32(FILE *f, PCSZ pszStr) {
    size_t n = pszStr ? strlen(pszStr) : 0;
    if (write_u32(f, (ULONG)n) != 0) return -1;
    if (n > 0 && fwrite(pszStr, 1, n, f) != n) return -1;
    return 0;
}

/*!
 * @brief Read a length-prefixed string.
 *
 * @param[in]  f         File. Not NULL.
 * @param[out] ppszOut   Receiver. Not NULL. Set to NULL on error.
 *
 * @return 0 on success, -1 on read error.
 *
 * @retval 0   Success.
 * @retval -1  Read error.
 */
static int read_str32(FILE *f, PSZ *ppszOut) {
    ULONG ulLen;
    PSZ pszStr;
    *ppszOut = NULL;
    if (read_u32(f, &ulLen) != 0) return -1;
    pszStr = (PSZ)malloc(ulLen + 1);
    if (!pszStr) return -1;
    if (ulLen > 0 && fread(pszStr, 1, ulLen, f) != ulLen) {
        free(pszStr);
        return -1;
    }
    pszStr[ulLen] = '\0';
    *ppszOut = pszStr;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Cache writing                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Write a detail block (text + template + html) to the cache.
 *
 * @param[in]  f           File. Not NULL.
 * @param[in]  pszText     License text, or NULL.
 * @param[in]  pszTmpl     Standard template, or NULL.
 * @param[in]  pszHtml     HTML text, or NULL.
 * @param[out] pulOffset   Offset of the block. Not NULL.
 * @param[out] pulSize     Size of the block. Not NULL.
 *
 * @return 0 on success, -1 on write error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int write_detail_block(FILE *f, PCSZ pszText,
                              PCSZ pszTmpl, PCSZ pszHtml,
                              PULONG pulOffset, PULONG pulSize) {
    long lStart, lEnd;
    fflush(f);
    lStart = ftell(f);
    if (lStart < 0) return -1;
    if (write_str32(f, pszText ? pszText : "") != 0) return -1;
    if (write_str32(f, pszTmpl ? pszTmpl : "") != 0) return -1;
    if (write_str32(f, pszHtml ? pszHtml : "") != 0) return -1;
    fflush(f);
    lEnd = ftell(f);
    if (lEnd < 0) return -1;
    *pulOffset = (ULONG)lStart;
    *pulSize = (ULONG)(lEnd - lStart);
    return 0;
}

/*!
 * @brief Write one license index record to the cache.
 *
 * @param[in] f       File. Not NULL.
 * @param[in] pEntry  Entry. Not NULL.
 *
 * @return 0 on success, -1 on write error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int write_index_record(FILE *f, const SPDXLICENSEENTRY *pEntry) {
    int i;
    if (write_str32(f, pEntry->pszId) != 0) return -1;
    if (write_u8v(f, pEntry->uchFlags) != 0) return -1;
    if (write_str32(f, pEntry->pszName ? pEntry->pszName : "") != 0)
        return -1;
    {
        ULONG ulCount = 0;
        if (pEntry->papszSeeAlso)
            for (i = 0; pEntry->papszSeeAlso[i]; i++) ulCount++;
        if (write_u32(f, ulCount) != 0) return -1;
        for (i = 0; pEntry->papszSeeAlso && pEntry->papszSeeAlso[i]; i++)
            if (write_str32(f, pEntry->papszSeeAlso[i]) != 0) return -1;
    }
    if (write_u32(f, pEntry->ulDetailOffset) != 0) return -1;
    if (write_u32(f, pEntry->ulDetailSize) != 0) return -1;
    return 0;
}

/*!
 * @brief Write one exception index record to the cache.
 *
 * @param[in] f       File. Not NULL.
 * @param[in] pEntry  Entry. Not NULL.
 *
 * @return 0 on success, -1 on write error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int write_index_record_exc(FILE *f, const SPDXEXCEPTIONENTRY *pEntry) {
    int i;
    if (write_str32(f, pEntry->pszId) != 0) return -1;
    if (write_u8v(f, pEntry->uchFlags) != 0) return -1;
    if (write_str32(f, pEntry->pszName ? pEntry->pszName : "") != 0)
        return -1;
    {
        ULONG ulCount = 0;
        if (pEntry->papszSeeAlso)
            for (i = 0; pEntry->papszSeeAlso[i]; i++) ulCount++;
        if (write_u32(f, ulCount) != 0) return -1;
        for (i = 0; pEntry->papszSeeAlso && pEntry->papszSeeAlso[i]; i++)
            if (write_str32(f, pEntry->papszSeeAlso[i]) != 0) return -1;
    }
    if (write_u32(f, pEntry->ulDetailOffset) != 0) return -1;
    if (write_u32(f, pEntry->ulDetailSize) != 0) return -1;
    return 0;
}

/*!
 * @brief Read a license detail from details/<id>.json and write it
 *        to the cache.
 *
 * @param[in]  f           File. Not NULL.
 * @param[in]  pszId       License identifier. Not NULL.
 * @param[out] pulOffset   Offset of the block. Not NULL.
 * @param[out] pulSize     Size of the block. Not NULL.
 *
 * @return 0 on success, -1 on any error.
 *
 * @retval 0   Success.
 * @retval -1  Any error.
 */
static int write_license_detail_from_json(FILE *f, PCSZ pszId,
                                          PULONG pulOffset,
                                          PULONG pulSize) {
    CHAR achPath[2048];
    ULONG ulNeeded = 0;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    PSZ pszTextVal = NULL, pszTmplVal = NULL, pszHtmlVal = NULL;
    PSZ pszHeap = NULL;
    int rc = -1;
    APIRET arc;

    snprintf(achPath, sizeof(achPath), "%s/%s.json",
             g_pszDetailsDir, pszId);
    arc = SpdxReadFileAll(achPath, NULL, 0, &ulNeeded);
    if (arc != NO_ERROR) return -1;

    pszHeap = (PSZ)malloc(ulNeeded);
    if (!pszHeap) return -1;

    arc = SpdxReadFileAll(achPath, pszHeap, ulNeeded, NULL);
    if (arc != NO_ERROR) { free(pszHeap); return -1; }

    if (JsonParse(pszHeap, &hDoc) != NO_ERROR) {
        free(pszHeap);
        return -1;
    }
    free(pszHeap);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseText", &hChild) == NO_ERROR)
        pszTextVal = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "standardLicenseTemplate", &hChild)
            == NO_ERROR)
        pszTmplVal = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseTextHtml", &hChild) == NO_ERROR)
        pszHtmlVal = json_dup_string(hChild);

    if (write_detail_block(f, pszTextVal, pszTmplVal, pszHtmlVal,
                           pulOffset, pulSize) == 0) {
        rc = 0;
    }

    free(pszTextVal);
    free(pszTmplVal);
    free(pszHtmlVal);
    JsonClose(hDoc);
    return rc;
}

/*!
 * @brief Read an exception detail from exceptions/<id>.json and
 *        write it to the cache.
 *
 * @param[in]  f           File. Not NULL.
 * @param[in]  pszId       Exception identifier. Not NULL.
 * @param[out] pulOffset   Offset of the block. Not NULL.
 * @param[out] pulSize     Size of the block. Not NULL.
 *
 * @return 0 on success, -1 on any error.
 *
 * @retval 0   Success.
 * @retval -1  Any error.
 */
static int write_exception_detail_from_json(FILE *f, PCSZ pszId,
                                            PULONG pulOffset,
                                            PULONG pulSize) {
    CHAR achPath[2048];
    ULONG ulNeeded = 0;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    PSZ pszTextVal = NULL, pszTmplVal = NULL, pszHtmlVal = NULL;
    PSZ pszHeap = NULL;
    int rc = -1;
    APIRET arc;

    snprintf(achPath, sizeof(achPath), "%s/%s.json",
             g_pszExceptionsDir, pszId);
    arc = SpdxReadFileAll(achPath, NULL, 0, &ulNeeded);
    if (arc != NO_ERROR) return -1;

    pszHeap = (PSZ)malloc(ulNeeded);
    if (!pszHeap) return -1;

    arc = SpdxReadFileAll(achPath, pszHeap, ulNeeded, NULL);
    if (arc != NO_ERROR) { free(pszHeap); return -1; }

    if (JsonParse(pszHeap, &hDoc) != NO_ERROR) {
        free(pszHeap);
        return -1;
    }
    free(pszHeap);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseExceptionText", &hChild) == NO_ERROR)
        pszTextVal = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseExceptionTemplate", &hChild)
            == NO_ERROR)
        pszTmplVal = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "exceptionTextHtml", &hChild) == NO_ERROR)
        pszHtmlVal = json_dup_string(hChild);

    if (write_detail_block(f, pszTextVal, pszTmplVal, pszHtmlVal,
                           pulOffset, pulSize) == 0) {
        rc = 0;
    }

    free(pszTextVal);
    free(pszTmplVal);
    free(pszHtmlVal);
    JsonClose(hDoc);
    return rc;
}

/*!
 * @brief Compute a raw 20-byte SHA-1 of a file.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] puchOut  20-byte digest.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Read or hash error.
 */
static int compute_sha1_raw(PCSZ pszPath, UCHAR puchOut[20]) {
    CHAR achHex[41];
    int i;
    memset(puchOut, 0, 20);
    if (!pszPath) return 0;
    if (Sha1File(pszPath, achHex, sizeof(achHex), NULL) != NO_ERROR)
        return -1;
    for (i = 0; i < 20; i++) {
        CHAR b[3];
        b[0] = achHex[i*2]; b[1] = achHex[i*2+1]; b[2] = '\0';
        puchOut[i] = (UCHAR)strtol(b, NULL, 16);
    }
    return 0;
}

/*!
 * @brief Build the binary cache file.
 *
 * @param[in] pszCachePath  Cache file path. Not NULL.
 * @param[in] puchSha1Lic   SHA-1 of licenses.json.
 * @param[in] puchSha1Exc   SHA-1 of exceptions.json.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Write error.
 */
static int build_cache(PCSZ pszCachePath,
                       const UCHAR puchSha1Lic[20],
                       const UCHAR puchSha1Exc[20]) {
    FILE *f;
    ULONG ulOffLicIdx = 0, ulOffExcIdx = 0;
    long lHere;
    int i;

    f = fopen(pszCachePath, "wb");
    if (!f) return -1;

    fwrite(CACHE_MAGIC, 1, 8, f);
    write_u32(f, CACHE_VERSION);
    fwrite(puchSha1Lic, 1, 20, f);
    fwrite(puchSha1Exc, 1, 20, f);
    write_u32(f, (ULONG)g_Licenses.count);
    write_u32(f, (ULONG)g_Exceptions.count);
    write_u32(f, 0);
    write_u32(f, 0);
    write_u32(f, 0);
    write_u32(f, 0);

    fflush(f);
    ulOffLicIdx = (ULONG)ftell(f);
    for (i = 0; i < g_Licenses.count; i++) {
        PSPDXLICENSEENTRY pEntry = &g_Licenses.pItems[i];
        if (g_pszDetailsDir) {
            ULONG ulOff = 0, ulSz = 0;
            if (write_license_detail_from_json(f, pEntry->pszId,
                                               &ulOff, &ulSz) == 0) {
                pEntry->ulDetailOffset = ulOff;
                pEntry->ulDetailSize = ulSz;
            }
        }
        if (write_index_record(f, pEntry) != 0) {
            fclose(f);
            return -1;
        }
    }

    fflush(f);
    ulOffExcIdx = (ULONG)ftell(f);
    for (i = 0; i < g_Exceptions.count; i++) {
        PSPDXEXCEPTIONENTRY pEntry = &g_Exceptions.pItems[i];
        if (g_pszExceptionsDir) {
            ULONG ulOff = 0, ulSz = 0;
            if (write_exception_detail_from_json(f, pEntry->pszId,
                                                 &ulOff, &ulSz) == 0) {
                pEntry->ulDetailOffset = ulOff;
                pEntry->ulDetailSize = ulSz;
            }
        }
        if (write_index_record_exc(f, pEntry) != 0) {
            fclose(f);
            return -1;
        }
    }

    lHere = ftell(f);
    fseek(f, 0, SEEK_SET);
    fwrite(CACHE_MAGIC, 1, 8, f);
    write_u32(f, CACHE_VERSION);
    fwrite(puchSha1Lic, 1, 20, f);
    fwrite(puchSha1Exc, 1, 20, f);
    write_u32(f, (ULONG)g_Licenses.count);
    write_u32(f, (ULONG)g_Exceptions.count);
    write_u32(f, ulOffLicIdx);
    write_u32(f, ulOffExcIdx);
    write_u32(f, 0);
    write_u32(f, 0);
    fseek(f, lHere, SEEK_SET);

    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Cache reading                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read one license index record from the cache.
 *
 * @param[in]  f       File. Not NULL.
 * @param[out] pEntry  Entry receiver. Not NULL.
 *
 * @return 0 on success, -1 on read error.
 *
 * @retval 0   Success.
 * @retval -1  Read error.
 */
static int read_license_index_record(FILE *f, PSPDXLICENSEENTRY pEntry) {
    ULONG ulCount, i;
    if (read_str32(f, &pEntry->pszId) != 0) return -1;
    if (read_u8v(f, &pEntry->uchFlags) != 0) return -1;
    if (read_str32(f, &pEntry->pszName) != 0) return -1;
    if (read_u32(f, &ulCount) != 0) return -1;
    if (ulCount > 0) {
        pEntry->papszSeeAlso =
            (PSZ*)malloc((size_t)(ulCount + 1) * sizeof(PSZ));
        if (!pEntry->papszSeeAlso) return -1;
        for (i = 0; i < ulCount; i++) {
            if (read_str32(f, &pEntry->papszSeeAlso[i]) != 0) {
                pEntry->papszSeeAlso[i] = NULL;
                return -1;
            }
        }
        pEntry->papszSeeAlso[ulCount] = NULL;
    }
    if (read_u32(f, &pEntry->ulDetailOffset) != 0) return -1;
    if (read_u32(f, &pEntry->ulDetailSize) != 0) return -1;
    pEntry->fDetailLoaded = FALSE;
    return 0;
}

/*!
 * @brief Read one exception index record from the cache.
 *
 * @param[in]  f       File. Not NULL.
 * @param[out] pEntry  Entry receiver. Not NULL.
 *
 * @return 0 on success, -1 on read error.
 *
 * @retval 0   Success.
 * @retval -1  Read error.
 */
static int read_exception_index_record(FILE *f, PSPDXEXCEPTIONENTRY pEntry) {
    ULONG ulCount, i;
    if (read_str32(f, &pEntry->pszId) != 0) return -1;
    if (read_u8v(f, &pEntry->uchFlags) != 0) return -1;
    if (read_str32(f, &pEntry->pszName) != 0) return -1;
    if (read_u32(f, &ulCount) != 0) return -1;
    if (ulCount > 0) {
        pEntry->papszSeeAlso =
            (PSZ*)malloc((size_t)(ulCount + 1) * sizeof(PSZ));
        if (!pEntry->papszSeeAlso) return -1;
        for (i = 0; i < ulCount; i++) {
            if (read_str32(f, &pEntry->papszSeeAlso[i]) != 0) {
                pEntry->papszSeeAlso[i] = NULL;
                return -1;
            }
        }
        pEntry->papszSeeAlso[ulCount] = NULL;
    }
    if (read_u32(f, &pEntry->ulDetailOffset) != 0) return -1;
    if (read_u32(f, &pEntry->ulDetailSize) != 0) return -1;
    pEntry->fDetailLoaded = FALSE;
    return 0;
}

/*!
 * @brief Load the binary cache file if its content matches the
 *        expected source hashes.
 *
 * @param[in] pszPath        Cache file path. Not NULL.
 * @param[in] puchExpectLic  Expected SHA-1 of licenses.json.
 * @param[in] puchExpectExc  Expected SHA-1 of exceptions.json.
 *
 * @return 0 on success, -1 on error or mismatch.
 *
 * @retval 0   Success.
 * @retval -1  Read error, format mismatch, or hash mismatch.
 */
static int load_cache(PCSZ pszPath,
                      const UCHAR puchExpectLic[20],
                      const UCHAR puchExpectExc[20]) {
    FILE *f;
    CHAR achMagic[9];
    ULONG ulVersion, ulCntLic, ulCntExc, ulOffLic, ulOffExc, ulDummy;
    UCHAR puchSha1Lic[20], puchSha1Exc[20];
    ULONG i;

    f = fopen(pszPath, "rb");
    if (!f) return -1;
    if (fread(achMagic, 1, 8, f) != 8 ||
        memcmp(achMagic, CACHE_MAGIC, 8) != 0) { fclose(f); return -1; }
    if (read_u32(f, &ulVersion) != 0 || ulVersion != CACHE_VERSION) {
        fclose(f); return -1;
    }
    if (fread(puchSha1Lic, 1, 20, f) != 20 ||
        fread(puchSha1Exc, 1, 20, f) != 20) { fclose(f); return -1; }
    if (memcmp(puchSha1Lic, puchExpectLic, 20) != 0 ||
        memcmp(puchSha1Exc, puchExpectExc, 20) != 0) {
        fclose(f); return -1;
    }
    if (read_u32(f, &ulCntLic) != 0 ||
        read_u32(f, &ulCntExc) != 0 ||
        read_u32(f, &ulOffLic) != 0 ||
        read_u32(f, &ulOffExc) != 0 ||
        read_u32(f, &ulDummy) != 0 ||
        read_u32(f, &ulDummy) != 0) { fclose(f); return -1; }

    if (fseek(f, (long)ulOffLic, SEEK_SET) != 0) { fclose(f); return -1; }
    for (i = 0; i < ulCntLic; i++) {
        PSPDXLICENSEENTRY pEntry;
        if (license_list_add(&g_Licenses, &pEntry) != NO_ERROR) {
            fclose(f);
            return -1;
        }
        if (read_license_index_record(f, pEntry) != 0) {
            fclose(f);
            return -1;
        }
    }
    if (fseek(f, (long)ulOffExc, SEEK_SET) != 0) { fclose(f); return -1; }
    for (i = 0; i < ulCntExc; i++) {
        PSPDXEXCEPTIONENTRY pEntry;
        if (exception_list_add(&g_Exceptions, &pEntry) != NO_ERROR) {
            fclose(f);
            return -1;
        }
        if (read_exception_index_record(f, pEntry) != 0) {
            fclose(f);
            return -1;
        }
    }

    g_pCacheFp = f;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Lazy detail loading                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a detail block from the cache.
 *
 * @param[in]  ulOffset   Block offset.
 * @param[in]  ulSize     Block size.
 * @param[out] ppszText   Receiver for the text. Not NULL.
 * @param[out] ppszTmpl   Receiver for the template. Not NULL.
 * @param[out] ppszHtml   Receiver for the HTML. Not NULL.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Read error or malformed block.
 */
static int read_detail_from_cache(ULONG ulOffset, ULONG ulSize,
                                  PSZ *ppszText, PSZ *ppszTmpl,
                                  PSZ *ppszHtml) {
    PSZ pszBuf;
    long lGot;
    ULONG ulPos = 0;
    ULONG ulLen;

    *ppszText = NULL;
    *ppszTmpl = NULL;
    *ppszHtml = NULL;

    if (!g_pCacheFp || ulOffset == 0 || ulSize == 0) return -1;
    pszBuf = (PSZ)malloc(ulSize);
    if (!pszBuf) return -1;
    if (fseek(g_pCacheFp, (long)ulOffset, SEEK_SET) != 0) {
        free(pszBuf); return -1;
    }
    lGot = (long)fread(pszBuf, 1, ulSize, g_pCacheFp);
    if (lGot != (long)ulSize) { free(pszBuf); return -1; }

    if (ulPos + 4 > ulSize) { free(pszBuf); return -1; }
    ulLen = (ULONG)pszBuf[ulPos]        |
            ((ULONG)pszBuf[ulPos+1] << 8) |
            ((ULONG)pszBuf[ulPos+2] << 16)|
            ((ULONG)pszBuf[ulPos+3] << 24);
    ulPos += 4;
    if (ulPos + ulLen > ulSize) { free(pszBuf); return -1; }
    *ppszText = (PSZ)malloc(ulLen + 1);
    if (!*ppszText) { free(pszBuf); return -1; }
    memcpy(*ppszText, pszBuf + ulPos, ulLen);
    (*ppszText)[ulLen] = '\0';
    ulPos += ulLen;

    if (ulPos + 4 > ulSize) {
        free(*ppszText); *ppszText = NULL; free(pszBuf); return -1;
    }
    ulLen = (ULONG)pszBuf[ulPos]        |
            ((ULONG)pszBuf[ulPos+1] << 8) |
            ((ULONG)pszBuf[ulPos+2] << 16)|
            ((ULONG)pszBuf[ulPos+3] << 24);
    ulPos += 4;
    if (ulPos + ulLen > ulSize) {
        free(*ppszText); *ppszText = NULL; free(pszBuf); return -1;
    }
    *ppszTmpl = (PSZ)malloc(ulLen + 1);
    if (!*ppszTmpl) {
        free(*ppszText); *ppszText = NULL; free(pszBuf); return -1;
    }
    memcpy(*ppszTmpl, pszBuf + ulPos, ulLen);
    (*ppszTmpl)[ulLen] = '\0';
    ulPos += ulLen;

    if (ulPos + 4 > ulSize) {
        free(*ppszText); *ppszText = NULL;
        free(*ppszTmpl); *ppszTmpl = NULL;
        free(pszBuf); return -1;
    }
    ulLen = (ULONG)pszBuf[ulPos]        |
            ((ULONG)pszBuf[ulPos+1] << 8) |
            ((ULONG)pszBuf[ulPos+2] << 16)|
            ((ULONG)pszBuf[ulPos+3] << 24);
    ulPos += 4;
    if (ulPos + ulLen > ulSize) {
        free(*ppszText); *ppszText = NULL;
        free(*ppszTmpl); *ppszTmpl = NULL;
        free(pszBuf); return -1;
    }
    *ppszHtml = (PSZ)malloc(ulLen + 1);
    if (!*ppszHtml) {
        free(*ppszText); *ppszText = NULL;
        free(*ppszTmpl); *ppszTmpl = NULL;
        free(pszBuf); return -1;
    }
    memcpy(*ppszHtml, pszBuf + ulPos, ulLen);
    (*ppszHtml)[ulLen] = '\0';

    free(pszBuf);
    return 0;
}

/*!
 * @brief Read a license detail from details/<id>.json.
 *
 * @param[in,out] pEntry  License entry. Not NULL.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Read error or malformed JSON.
 */
static int read_license_detail_from_dir(PSPDXLICENSEENTRY pEntry) {
    CHAR achPath[2048];
    ULONG ulNeeded = 0;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    PSZ pszHeap = NULL;
    APIRET rc;

    snprintf(achPath, sizeof(achPath), "%s/%s.json",
             g_pszDetailsDir, pEntry->pszId);
    rc = SpdxReadFileAll(achPath, NULL, 0, &ulNeeded);
    if (rc != NO_ERROR) return -1;

    pszHeap = (PSZ)malloc(ulNeeded);
    if (!pszHeap) return -1;

    rc = SpdxReadFileAll(achPath, pszHeap, ulNeeded, NULL);
    if (rc != NO_ERROR) { free(pszHeap); return -1; }

    if (JsonParse(pszHeap, &hDoc) != NO_ERROR) {
        free(pszHeap);
        return -1;
    }
    free(pszHeap);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseText", &hChild) == NO_ERROR)
        pEntry->pszText = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "standardLicenseTemplate", &hChild)
            == NO_ERROR)
        pEntry->pszTemplate = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseTextHtml", &hChild) == NO_ERROR)
        pEntry->pszTextHtml = json_dup_string(hChild);

    JsonClose(hDoc);
    return 0;
}

/*!
 * @brief Read an exception detail from exceptions/<id>.json.
 *
 * @param[in,out] pEntry  Exception entry. Not NULL.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Read error or malformed JSON.
 */
static int read_exception_detail_from_dir(PSPDXEXCEPTIONENTRY pEntry) {
    CHAR achPath[2048];
    ULONG ulNeeded = 0;
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    PSZ pszHeap = NULL;
    APIRET rc;

    snprintf(achPath, sizeof(achPath), "%s/%s.json",
             g_pszExceptionsDir, pEntry->pszId);
    rc = SpdxReadFileAll(achPath, NULL, 0, &ulNeeded);
    if (rc != NO_ERROR) return -1;

    pszHeap = (PSZ)malloc(ulNeeded);
    if (!pszHeap) return -1;

    rc = SpdxReadFileAll(achPath, pszHeap, ulNeeded, NULL);
    if (rc != NO_ERROR) { free(pszHeap); return -1; }

    if (JsonParse(pszHeap, &hDoc) != NO_ERROR) {
        free(pszHeap);
        return -1;
    }
    free(pszHeap);

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR) { JsonClose(hDoc); return -1; }

    if (JsonNodeGetChild(hRoot, "licenseExceptionText", &hChild) == NO_ERROR)
        pEntry->pszText = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "licenseExceptionTemplate", &hChild)
            == NO_ERROR)
        pEntry->pszTemplate = json_dup_string(hChild);
    if (JsonNodeGetChild(hRoot, "exceptionTextHtml", &hChild) == NO_ERROR)
        pEntry->pszTextHtml = json_dup_string(hChild);

    JsonClose(hDoc);
    return 0;
}

/*!
 * @brief Load a license detail on demand.
 *
 * Tries the open cache first, then details/<id>.json.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Identifier not found or read failure.
 */
static int license_load_detail(PCSZ pszId) {
    int idx;
    PSPDXLICENSEENTRY pEntry;
    idx = lic_lower_bound(pszId);
    if (idx >= g_Licenses.count ||
        id_cmp_ci(g_Licenses.pItems[idx].pszId, pszId) != 0) return -1;
    pEntry = &g_Licenses.pItems[idx];
    if (pEntry->fDetailLoaded) return 0;

    if (g_pCacheFp && pEntry->ulDetailOffset && pEntry->ulDetailSize) {
        if (read_detail_from_cache(pEntry->ulDetailOffset,
                                   pEntry->ulDetailSize,
                                   &pEntry->pszText,
                                   &pEntry->pszTemplate,
                                   &pEntry->pszTextHtml) == 0) {
            pEntry->fDetailLoaded = TRUE;
            return 0;
        }
    }
    if (g_pszDetailsDir) {
        if (read_license_detail_from_dir(pEntry) == 0) {
            pEntry->fDetailLoaded = TRUE;
            return 0;
        }
    }
    return -1;
}

/*!
 * @brief Load an exception detail on demand.
 *
 * Tries the open cache first, then exceptions/<id>.json.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return 0 on success, -1 on error.
 *
 * @retval 0   Success.
 * @retval -1  Identifier not found or read failure.
 */
static int exception_load_detail(PCSZ pszId) {
    int idx;
    PSPDXEXCEPTIONENTRY pEntry;
    idx = exc_lower_bound(pszId);
    if (idx >= g_Exceptions.count ||
        id_cmp_ci(g_Exceptions.pItems[idx].pszId, pszId) != 0) return -1;
    pEntry = &g_Exceptions.pItems[idx];
    if (pEntry->fDetailLoaded) return 0;

    if (g_pCacheFp && pEntry->ulDetailOffset && pEntry->ulDetailSize) {
        if (read_detail_from_cache(pEntry->ulDetailOffset,
                                   pEntry->ulDetailSize,
                                   &pEntry->pszText,
                                   &pEntry->pszTemplate,
                                   &pEntry->pszTextHtml) == 0) {
            pEntry->fDetailLoaded = TRUE;
            return 0;
        }
    }
    if (g_pszExceptionsDir) {
        if (read_exception_detail_from_dir(pEntry) == 0) {
            pEntry->fDetailLoaded = TRUE;
            return 0;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Open the SPDX database.
 *
 * Reads the licenses and exceptions indexes, either from a binary
 * cache (if the source hashes match) or directly from the JSON
 * files. Details are loaded lazily by the text query functions.
 *
 * @param[in] pszDbRoot    Database root. Inside it the following are
 *                         expected:
 *                           licenses.json
 *                           exceptions.json
 *                           details/<id>.json
 *                           exceptions/<id>.json
 *                         Not NULL.
 * @param[in] pszCacheFile Path to the cache file, or NULL for no
 *                         cache.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Full success.
 * @retval ERROR_INVALID_PARAMETER  pszDbRoot is NULL or empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval SPDXDB_ERROR_LICENSES    licenses.json failed to load.
 * @retval SPDXDB_ERROR_EXCEPTIONS  exceptions.json failed to load.
 * @retval SPDXDB_ERROR_CACHE       Cache could not be written.
 */
APIRET APIENTRY SpdxOpenDatabase(PCSZ pszDbRoot, PCSZ pszCacheFile) {
    UCHAR puchSha1Lic[20], puchSha1Exc[20];
    APIRET rc = NO_ERROR;
    APIRET rcInit;
    int cache_ok = 0;
    CHAR achLicPath[2048];
    CHAR achExcPath[2048];
    CHAR achDetPath[2048];
    CHAR achExcDetPath[2048];

    if (!pszDbRoot || !pszDbRoot[0]) return ERROR_INVALID_PARAMETER;

    rcInit = license_list_init(&g_Licenses);
    if (rcInit != NO_ERROR) return rcInit;
    rcInit = exception_list_init(&g_Exceptions);
    if (rcInit != NO_ERROR) {
        license_list_free(&g_Licenses);
        return rcInit;
    }

    snprintf(achLicPath, sizeof(achLicPath), "%s/licenses.json", pszDbRoot);
    snprintf(achExcPath, sizeof(achExcPath), "%s/exceptions.json", pszDbRoot);
    snprintf(achDetPath, sizeof(achDetPath), "%s/details", pszDbRoot);
    snprintf(achExcDetPath, sizeof(achExcDetPath),
             "%s/exceptions", pszDbRoot);

    g_pszDetailsDir = strdup(achDetPath);
    g_pszExceptionsDir = strdup(achExcDetPath);

    if (compute_sha1_raw(achLicPath, puchSha1Lic) != 0) {
        rc |= SPDXDB_ERROR_LICENSES;
        memset(puchSha1Lic, 0, 20);
    }
    if (compute_sha1_raw(achExcPath, puchSha1Exc) != 0) {
        rc |= SPDXDB_ERROR_EXCEPTIONS;
        memset(puchSha1Exc, 0, 20);
    }

    if (pszCacheFile) {
        if (load_cache(pszCacheFile, puchSha1Lic, puchSha1Exc) == 0)
            cache_ok = 1;
    }

    if (!cache_ok) {
        if (load_licenses_index(achLicPath) != NO_ERROR)
            rc |= SPDXDB_ERROR_LICENSES;
        if (load_exceptions_index(achExcPath) != NO_ERROR)
            rc |= SPDXDB_ERROR_EXCEPTIONS;
        qsort(g_Licenses.pItems, (size_t)g_Licenses.count,
              sizeof(SPDXLICENSEENTRY), cmp_lic);
        qsort(g_Exceptions.pItems, (size_t)g_Exceptions.count,
              sizeof(SPDXEXCEPTIONENTRY), cmp_exc);

        if (pszCacheFile) {
            if (build_cache(pszCacheFile, puchSha1Lic, puchSha1Exc) != 0)
                rc |= SPDXDB_ERROR_CACHE;
        }
    }

    return rc;
}

/*!
 * @brief Close the SPDX database.
 *
 * Releases every resource owned by the module, including
 * lazy-loaded details and the open cache file.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 */
APIRET APIENTRY SpdxCloseDatabase(void) {
    license_list_free(&g_Licenses);
    exception_list_free(&g_Exceptions);
    if (g_pCacheFp) { fclose(g_pCacheFp); g_pCacheFp = NULL; }
    free(g_pszDetailsDir); g_pszDetailsDir = NULL;
    free(g_pszExceptionsDir); g_pszExceptionsDir = NULL;
    return NO_ERROR;
}

/*!
 * @brief Query the canonical form of an SPDX identifier.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_FILE_NOT_FOUND     Identifier is not a known license
 *                                  or exception.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryCanonicalId(PCSZ pszId, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed) {
    PSPDXLICENSEENTRY pLic;
    PSPDXEXCEPTIONENTRY pExc;

    if (!pszId) return ERROR_INVALID_PARAMETER;

    pLic = license_lookup(pszId);
    if (pLic)
        return copy_out(pLic->pszId, pszBuf, ulSize, pulUsed);
    pExc = exception_lookup(pszId);
    if (pExc)
        return copy_out(pExc->pszId, pszBuf, ulSize, pulUsed);
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Query whether an identifier is a known SPDX license.
 *
 * LicenseRef-* and DocumentRef-<id>:LicenseRef-* are accepted as
 * valid even though they are not in the SPDX License List.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pfValid  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfValid is NULL.
 * @retval SPDXDB_ERROR_LICENSES    The license index is not loaded;
 *                                  the query cannot be answered.
 */
APIRET APIENTRY SpdxQueryLicenseValid(PCSZ pszId, PBOOL pfValid) {
    if (!pszId || !pfValid) return ERROR_INVALID_PARAMETER;
    *pfValid = FALSE;
    if (pszId[0] == '\0') return NO_ERROR;
    if (strncmp(pszId, "LicenseRef-", 11) == 0) {
        *pfValid = TRUE;
        return NO_ERROR;
    }
    if (strncmp(pszId, "DocumentRef-", 12) == 0) {
        PCSZ pszColon = strchr(pszId, ':');
        if (pszColon && strncmp(pszColon + 1, "LicenseRef-", 11) == 0)
            *pfValid = TRUE;
        return NO_ERROR;
    }
    if (g_Licenses.count == 0) {
        return SPDXDB_ERROR_LICENSES;
    }
    *pfValid = license_lookup(pszId) ? TRUE : FALSE;
    return NO_ERROR;
}

/*!
 * @brief Query whether an identifier is a known SPDX exception.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pfValid  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfValid is NULL.
 * @retval SPDXDB_ERROR_EXCEPTIONS  The exception index is not
 *                                  loaded; the query cannot be
 *                                  answered.
 */
APIRET APIENTRY SpdxQueryExceptionValid(PCSZ pszId, PBOOL pfValid) {
    if (!pszId || !pfValid) return ERROR_INVALID_PARAMETER;
    *pfValid = FALSE;
    if (pszId[0] == '\0') return NO_ERROR;
    if (g_Exceptions.count == 0) {
        return SPDXDB_ERROR_EXCEPTIONS;
    }
    *pfValid = exception_lookup(pszId) ? TRUE : FALSE;
    return NO_ERROR;
}

/*!
 * @brief Query whether a license identifier is deprecated.
 *
 * @param[in]  pszId         Identifier. Not NULL.
 * @param[out] pfDeprecated  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfDeprecated is NULL.
 */
APIRET APIENTRY SpdxQueryLicenseDeprecated(PCSZ pszId, PBOOL pfDeprecated) {
    PSPDXLICENSEENTRY pLic;
    if (!pszId || !pfDeprecated) return ERROR_INVALID_PARAMETER;
    *pfDeprecated = FALSE;
    pLic = license_lookup(pszId);
    if (pLic && (pLic->uchFlags & SPDXDB_FLAG_DEPRECATED))
        *pfDeprecated = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Query whether an exception identifier is deprecated.
 *
 * @param[in]  pszId         Identifier. Not NULL.
 * @param[out] pfDeprecated  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfDeprecated is NULL.
 */
APIRET APIENTRY SpdxQueryExceptionDeprecated(PCSZ pszId, PBOOL pfDeprecated) {
    PSPDXEXCEPTIONENTRY pExc;
    if (!pszId || !pfDeprecated) return ERROR_INVALID_PARAMETER;
    *pfDeprecated = FALSE;
    pExc = exception_lookup(pszId);
    if (pExc && (pExc->uchFlags & SPDXDB_FLAG_DEPRECATED))
        *pfDeprecated = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Query whether a license is OSI-approved.
 *
 * @param[in]  pszId       Identifier. Not NULL.
 * @param[out] pfApproved  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfApproved is NULL.
 */
APIRET APIENTRY SpdxQueryLicenseOsiApproved(PCSZ pszId, PBOOL pfApproved) {
    PSPDXLICENSEENTRY pLic;
    if (!pszId || !pfApproved) return ERROR_INVALID_PARAMETER;
    *pfApproved = FALSE;
    pLic = license_lookup(pszId);
    if (pLic && (pLic->uchFlags & SPDXDB_FLAG_OSI))
        *pfApproved = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Query whether a license is FSF-libre.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pfLibre  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfLibre is NULL.
 */
APIRET APIENTRY SpdxQueryLicenseFsfLibre(PCSZ pszId, PBOOL pfLibre) {
    PSPDXLICENSEENTRY pLic;
    if (!pszId || !pfLibre) return ERROR_INVALID_PARAMETER;
    *pfLibre = FALSE;
    pLic = license_lookup(pszId);
    if (pLic && (pLic->uchFlags & SPDXDB_FLAG_FSF_LIBRE))
        *pfLibre = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Query the license text of a known license.
 *
 * The detail is lazy-loaded.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_FILE_NOT_FOUND     No text available.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryLicenseText(PCSZ pszId, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed) {
    int idx;
    PSPDXLICENSEENTRY pEntry;

    if (!pszId || !pszId[0]) return ERROR_INVALID_PARAMETER;

    idx = lic_lower_bound(pszId);
    if (idx >= g_Licenses.count ||
        id_cmp_ci(g_Licenses.pItems[idx].pszId, pszId) != 0)
        return ERROR_FILE_NOT_FOUND;
    pEntry = &g_Licenses.pItems[idx];
    if (!pEntry->fDetailLoaded) {
        if (license_load_detail(pEntry->pszId) != 0)
            return ERROR_FILE_NOT_FOUND;
    }
    if (!pEntry->pszText) return ERROR_FILE_NOT_FOUND;
    return copy_out(pEntry->pszText, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the exception text of a known exception.
 *
 * The detail is lazy-loaded.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_FILE_NOT_FOUND     No text available.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryExceptionText(PCSZ pszId, PSZ pszBuf,
                                       ULONG ulSize, PULONG pulUsed) {
    int idx;
    PSPDXEXCEPTIONENTRY pEntry;

    if (!pszId || !pszId[0]) return ERROR_INVALID_PARAMETER;

    idx = exc_lower_bound(pszId);
    if (idx >= g_Exceptions.count ||
        id_cmp_ci(g_Exceptions.pItems[idx].pszId, pszId) != 0)
        return ERROR_FILE_NOT_FOUND;
    pEntry = &g_Exceptions.pItems[idx];
    if (!pEntry->fDetailLoaded) {
        if (exception_load_detail(pEntry->pszId) != 0)
            return ERROR_FILE_NOT_FOUND;
    }
    if (!pEntry->pszText) return ERROR_FILE_NOT_FOUND;
    return copy_out(pEntry->pszText, pszBuf, ulSize, pulUsed);
}

/* ------------------------------------------------------------------ */
/* SPDX expression parser                                              */
/* ------------------------------------------------------------------ */

/*!
 * @struct _EXPRPARSER
 * @brief Recursive descent parser state for SPDX expressions.
 */
typedef struct _EXPRPARSER {
    PCSZ   pszPos;           /*!< Current position.             */
    PCSZ   pszBadTokenStart; /*!< Start of offending token.     */
    APIRET ulErr;            /*!< One of the SPDX_EXPR_* codes. */
} EXPRPARSER;

/*!
 * @brief Skip whitespace in the parser input.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 */
static void skip_ws(EXPRPARSER *pParser) {
    while (*pParser->pszPos && isspace((unsigned char)*pParser->pszPos))
        pParser->pszPos++;
}

/*!
 * @brief Check whether a keyword appears at @p pszPos.
 *
 * @param[in] pszPos  Position. Not NULL.
 * @param[in] pszKw   Keyword. Not NULL.
 * @param[in] kwlen   Keyword length.
 *
 * @return 1 on match, 0 otherwise.
 *
 * @retval 1  Keyword present.
 * @retval 0  No match.
 */
static int is_kw_at(PCSZ pszPos, PCSZ pszKw, int kwlen) {
    if (strncmp(pszPos, pszKw, kwlen) != 0) return 0;
    if (pszPos[kwlen] == '\0') return 1;
    if (isspace((unsigned char)pszPos[kwlen])) return 1;
    if (pszPos[kwlen] == '(' || pszPos[kwlen] == ')') return 1;
    return 0;
}

/*!
 * @brief Forward declaration of the expression parser.
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 1 on success, 0 on error.
 *
 * @retval 1  Expression parsed.
 * @retval 0  Syntax error (code stored in the parser).
 */
static int parse_expression(EXPRPARSER *pParser);

/*!
 * @brief Parse one term (identifier [WITH exception] or
 *        parenthesized group).
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 1 on success, 0 on error (error code in parser).
 *
 * @retval 1  Term parsed.
 * @retval 0  Syntax error.
 */
static int parse_term(EXPRPARSER *pParser) {
    skip_ws(pParser);
    if (*pParser->pszPos == '(') {
        pParser->pszPos++;
        if (!parse_expression(pParser)) return 0;
        skip_ws(pParser);
        if (*pParser->pszPos != ')') {
            if (pParser->ulErr == NO_ERROR)
                pParser->ulErr = SPDX_EXPR_SYNTAX_ERROR;
            return 0;
        }
        pParser->pszPos++;
        return 1;
    }
    {
        PCSZ pszIdStart = pParser->pszPos;
        int id_len;
        CHAR achId[256];
        PCSZ pszSave;
        BOOL fValid = FALSE;
        while (*pParser->pszPos && !isspace((unsigned char)*pParser->pszPos) &&
               *pParser->pszPos != '(' && *pParser->pszPos != ')')
            pParser->pszPos++;
        id_len = (int)(pParser->pszPos - pszIdStart);
        if (id_len == 0) {
            if (pParser->ulErr == NO_ERROR)
                pParser->ulErr = SPDX_EXPR_SYNTAX_ERROR;
            return 0;
        }
        if (id_len >= (int)sizeof(achId)) id_len = (int)sizeof(achId) - 1;
        memcpy(achId, pszIdStart, (size_t)id_len);
        achId[id_len] = '\0';
        SpdxQueryLicenseValid(achId, &fValid);
        if (!fValid) {
            pParser->ulErr = SPDX_EXPR_UNKNOWN_TOKEN;
            pParser->pszBadTokenStart = pszIdStart;
            return 0;
        }
        pszSave = pParser->pszPos;
        skip_ws(pParser);
        if (is_kw_at(pParser->pszPos, "WITH", 4)) {
            PCSZ pszExcStart;
            int e_len;
            CHAR achExc[256];
            BOOL fExcValid = FALSE;
            pParser->pszPos += 4;
            skip_ws(pParser);
            pszExcStart = pParser->pszPos;
            while (*pParser->pszPos &&
                   !isspace((unsigned char)*pParser->pszPos) &&
                   *pParser->pszPos != '(' && *pParser->pszPos != ')')
                pParser->pszPos++;
            e_len = (int)(pParser->pszPos - pszExcStart);
            if (e_len == 0) {
                if (pParser->ulErr == NO_ERROR)
                    pParser->ulErr = SPDX_EXPR_SYNTAX_ERROR;
                return 0;
            }
            if (e_len >= (int)sizeof(achExc)) e_len = (int)sizeof(achExc) - 1;
            memcpy(achExc, pszExcStart, (size_t)e_len);
            achExc[e_len] = '\0';
            SpdxQueryExceptionValid(achExc, &fExcValid);
            if (!fExcValid) {
                pParser->ulErr = SPDX_EXPR_UNKNOWN_TOKEN;
                pParser->pszBadTokenStart = pszExcStart;
                return 0;
            }
        } else {
            pParser->pszPos = pszSave;
        }
    }
    return 1;
}

/*!
 * @brief Parse a full expression (terms joined by AND / OR).
 *
 * @param[in,out] pParser  Parser. Not NULL.
 *
 * @return 1 on success, 0 on error.
 *
 * @retval 1  Expression parsed.
 * @retval 0  Syntax error.
 */
static int parse_expression(EXPRPARSER *pParser) {
    if (!parse_term(pParser)) return 0;
    while (1) {
        PCSZ pszSave;
        skip_ws(pParser);
        pszSave = pParser->pszPos;
        if (is_kw_at(pParser->pszPos, "AND", 3)) {
            pParser->pszPos += 3;
            if (!parse_term(pParser)) return 0;
        } else if (is_kw_at(pParser->pszPos, "OR", 2)) {
            pParser->pszPos += 2;
            if (!parse_term(pParser)) return 0;
        } else {
            pParser->pszPos = pszSave;
            break;
        }
    }
    return 1;
}

/*!
 * @brief Query the validity of an SPDX license expression.
 *
 * @param[in]  pszExpr       Expression. Not NULL.
 * @param[out] ppszBadToken  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Expression is valid.
 * @retval SPDX_EXPR_SYNTAX_ERROR   Grammar violation.
 * @retval SPDX_EXPR_UNKNOWN_TOKEN  Unknown SPDX identifier.
 */
APIRET APIENTRY SpdxQueryExpression(PCSZ pszExpr, PCSZ *ppszBadToken) {
    EXPRPARSER parser;
    if (ppszBadToken) *ppszBadToken = NULL;
    if (!pszExpr) return SPDX_EXPR_SYNTAX_ERROR;
    parser.pszPos = pszExpr;
    parser.pszBadTokenStart = NULL;
    parser.ulErr = NO_ERROR;
    if (!parse_expression(&parser)) {
        if (parser.ulErr == NO_ERROR) parser.ulErr = SPDX_EXPR_SYNTAX_ERROR;
        if (ppszBadToken && parser.pszBadTokenStart)
            *ppszBadToken = parser.pszBadTokenStart;
        return parser.ulErr;
    }
    skip_ws(&parser);
    if (*parser.pszPos != '\0') return SPDX_EXPR_SYNTAX_ERROR;
    return NO_ERROR;
}

/*!
 * @brief Build the canonical form of an SPDX expression in a heap
 *        buffer.
 *
 * Every identifier from the SPDX License List or the SPDX Exceptions
 * list is replaced with its canonical case (for example,
 * 'BSD-3-clause' becomes 'BSD-3-Clause'). AND/OR/WITH, parentheses and
 * whitespace are preserved as-is. LicenseRef-* and DocumentRef-*
 * identifiers are left unchanged.
 *
 * @param[in]  pszExpr   Expression. Not NULL.
 * @param[out] ppszOut   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET canonical_to_heap(PCSZ pszExpr, PSZ *ppszOut) {
    size_t cap = 128;
    size_t len = 0;
    PSZ pszOut;
    PCSZ pszPos;

    *ppszOut = NULL;

    pszOut = (PSZ)malloc(cap);
    if (!pszOut) return ERROR_NOT_ENOUGH_MEMORY;
    pszOut[0] = '\0';

    pszPos = pszExpr;
    while (*pszPos) {
        PCSZ pszStart;
        size_t tok_len;
        CHAR achTok[256];
        PCSZ pszCanonical;

        if (*pszPos == ' ' || *pszPos == '\t' ||
            *pszPos == '(' || *pszPos == ')') {
            if (len + 1 >= cap) {
                size_t ncap = cap * 2;
                PSZ pszNew = (PSZ)realloc(pszOut, ncap);
                if (!pszNew) { free(pszOut); return ERROR_NOT_ENOUGH_MEMORY; }
                pszOut = pszNew; cap = ncap;
            }
            pszOut[len++] = *pszPos++;
            pszOut[len] = '\0';
            continue;
        }

        pszStart = pszPos;
        while (*pszPos && !isspace((unsigned char)*pszPos) &&
               *pszPos != '(' && *pszPos != ')')
            pszPos++;
        tok_len = (size_t)(pszPos - pszStart);
        if (tok_len >= sizeof(achTok)) tok_len = sizeof(achTok) - 1;
        memcpy(achTok, pszStart, tok_len);
        achTok[tok_len] = '\0';

        if (strcmp(achTok, "AND") == 0 ||
            strcmp(achTok, "OR") == 0 ||
            strcmp(achTok, "WITH") == 0) {
            pszCanonical = achTok;
        } else if (strncmp(achTok, "LicenseRef-", 11) == 0 ||
                   strncmp(achTok, "DocumentRef-", 12) == 0) {
            pszCanonical = achTok;
        } else {
            PSPDXLICENSEENTRY pLic = license_lookup(achTok);
            PSPDXEXCEPTIONENTRY pExc = NULL;
            if (!pLic) pExc = exception_lookup(achTok);
            if (pLic) pszCanonical = pLic->pszId;
            else if (pExc) pszCanonical = pExc->pszId;
            else pszCanonical = achTok;
        }

        {
            size_t clen = strlen(pszCanonical);
            if (len + clen + 1 > cap) {
                size_t ncap = cap * 2 + clen;
                PSZ pszNew = (PSZ)realloc(pszOut, ncap);
                if (!pszNew) { free(pszOut); return ERROR_NOT_ENOUGH_MEMORY; }
                pszOut = pszNew; cap = ncap;
            }
            memcpy(pszOut + len, pszCanonical, clen);
            len += clen;
            pszOut[len] = '\0';
        }
    }

    *ppszOut = pszOut;
    return NO_ERROR;
}

/*!
 * @brief Query the canonical form of an SPDX license expression.
 *
 * @param[in]  pszExpr  Expression. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszExpr is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxQueryExpressionCanonical(PCSZ pszExpr, PSZ pszBuf,
                                             ULONG ulSize, PULONG pulUsed) {
    PSZ pszHeap = NULL;
    APIRET rc;

    if (!pszExpr) return ERROR_INVALID_PARAMETER;

    rc = canonical_to_heap(pszExpr, &pszHeap);
    if (rc != NO_ERROR) return rc;

    rc = copy_out(pszHeap, pszBuf, ulSize, pulUsed);
    free(pszHeap);
    return rc;
}

/* ------------------------------------------------------------------ */
/* Expression identifier collection                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Check whether a token is one of the SPDX operators.
 *
 * @param[in] pszTok  Token. Not NULL.
 * @param[in] cbLen   Token length.
 *
 * @return 1 if the token is AND / OR / WITH, 0 otherwise.
 *
 * @retval 1  Operator token.
 * @retval 0  Not an operator.
 */
static int is_operator_token(PCSZ pszTok, size_t cbLen) {
    if (cbLen == 3 && strncmp(pszTok, "AND", 3) == 0) return 1;
    if (cbLen == 2 && strncmp(pszTok, "OR",  2) == 0) return 1;
    if (cbLen == 4 && strncmp(pszTok, "WITH", 4) == 0) return 1;
    return 0;
}

/*!
 * @brief Collect the SPDX identifiers from a license expression.
 *
 * Validates the expression first, then extracts every identifier
 * that is not one of the operators AND / OR / WITH and not a
 * parenthesis. Duplicates are removed by the destination set.
 *
 * @param[in]  pszExpr       Expression. Not NULL.
 * @param[in]  hOut          Destination set. Not NULLHANDLE.
 * @param[out] ppszBadToken  Optional. May be NULL. On
 *                           SPDX_EXPR_UNKNOWN_TOKEN, receives a
 *                           pointer to the offending token inside
 *                           @p pszExpr.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszExpr is NULL.
 * @retval ERROR_INVALID_HANDLE     hOut is not recognized.
 * @retval SPDX_EXPR_SYNTAX_ERROR   Grammar violation.
 * @retval SPDX_EXPR_UNKNOWN_TOKEN  Unknown SPDX identifier.
 * @retval SPDXDB_ERROR_LICENSES    The license index is not loaded.
 * @retval SPDXDB_ERROR_EXCEPTIONS  The exception index is not loaded.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxExpressionCollectIds(PCSZ pszExpr, HSTRSET hOut,
                                         PCSZ *ppszBadToken) {
    PCSZ pszPos;
    APIRET rc;
    CHAR achTok[256];

    if (!pszExpr) return ERROR_INVALID_PARAMETER;
    if (hOut == NULLHANDLE) return ERROR_INVALID_HANDLE;

    rc = SpdxQueryExpression(pszExpr, ppszBadToken);
    if (rc != NO_ERROR) return rc;

    pszPos = pszExpr;
    while (*pszPos) {
        PCSZ pszStart;
        size_t cbLen;

        if (*pszPos == ' ' || *pszPos == '\t' ||
            *pszPos == '(' || *pszPos == ')') {
            pszPos++;
            continue;
        }

        pszStart = pszPos;
        while (*pszPos && !isspace((unsigned char)*pszPos) &&
               *pszPos != '(' && *pszPos != ')')
            pszPos++;
        cbLen = (size_t)(pszPos - pszStart);
        if (cbLen == 0) continue;
        if (cbLen >= sizeof(achTok)) cbLen = sizeof(achTok) - 1;

        memcpy(achTok, pszStart, cbLen);
        achTok[cbLen] = '\0';

        if (is_operator_token(achTok, cbLen)) continue;

        rc = StrSetAdd(hOut, achTok);
        if (rc != NO_ERROR) return rc;
    }

    return NO_ERROR;
}
