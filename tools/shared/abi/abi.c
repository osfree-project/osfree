/*!
 * @file abi.c
 *
 * @brief Implementation of the writer for uni2h .abi files.
 *
 * Implements the API declared in abi.h. The document is a list of
 * blocks, each holding its own entries; serialization walks the
 * blocks in order.
 *
 * References:
 *   - uni2h v2.0 specification, section 3 ("The .abi file").
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os2types.h"
#include "os2err.h"
#include "abi.h"

/*!
 * @def ABI_INDENT_ENTRY
 * @brief Indentation string used for a whole entry clause.
 */
#define ABI_INDENT_ENTRY   "  "

/*!
 * @def ABI_INDENT_FIELD
 * @brief Indentation string used for a field inside an entry clause.
 */
#define ABI_INDENT_FIELD   "    "

/*!
 * @def ABI_KIND_MODULE
 * @brief Block kind: module block. Value: 0.
 */
#define ABI_KIND_MODULE 0

/*!
 * @def ABI_KIND_LIBRARY
 * @brief Block kind: library block. Value: 1.
 */
#define ABI_KIND_LIBRARY 1

/* ------------------------------------------------------------------ */
/* Internal representation                                             */
/* ------------------------------------------------------------------ */

/*!
 * @struct _ABI_ENTRY_REC
 * @brief One entry stored in a block.
 *
 * All string fields are owned by the document and freed by
 * AbiClose.
 */
typedef struct _ABI_ENTRY_REC {
    PSZ   pszName;        /*!< Copy of the caller's entry name. */
    PSZ   pszInternal;    /*!< NULL for a plain entry.          */
    ULONG ulOrdinal;      /*!< 0 means "not specified".         */
    PSZ   pszConvention;  /*!< NULL for the default convention. */
    BOOL  fVariable;      /*!< TRUE: no convention clause.      */
} ABI_ENTRY_REC, *PABI_ENTRY_REC;

/*!
 * @struct _ABI_BLOCK
 * @brief One block (module or library) inside a document.
 */
typedef struct _ABI_BLOCK {
    ULONG           ulKind;      /*!< ABI_KIND_MODULE or
                                  *   ABI_KIND_LIBRARY.   */
    PSZ             pszName;     /*!< Block name.          */
    PABI_ENTRY_REC  paEntries;   /*!< Entry array.         */
    ULONG           ulCount;     /*!< Used entries.        */
    ULONG           ulCapacity;  /*!< Allocated capacity.  */
} ABI_BLOCK, *PABI_BLOCK;

/*!
 * @struct abi_handle
 * @brief Internal representation behind HABIDOC.
 *
 * Not exposed to callers. abi.h declares the handle as HANDLE, so
 * the layout of this structure may change freely.
 */
struct abi_handle {
    PABI_BLOCK paBlocks;     /*!< Block array.              */
    ULONG      ulBlockCount; /*!< Number of used blocks.    */
    ULONG      ulBlockCap;   /*!< Allocated block capacity. */
    ULONG      ulOpenIndex;  /*!< Open block index, or -1.  */
};

/*!
 * @typedef ABI_HANDLE
 * @brief Alias for @c struct @c abi_handle.
 */
typedef struct abi_handle ABI_HANDLE;

/* ------------------------------------------------------------------ */
/* Growing text buffer                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @struct _ABI_BUF
 * @brief Growable NUL-terminated text buffer.
 *
 * Used only inside abi.c to build the serialized document before
 * handing it to the caller or writing it to disk.
 */
typedef struct _ABI_BUF {
    PSZ   pszData;     /*!< Backing storage.               */
    ULONG ulUsed;      /*!< Bytes used, including the NUL. */
    ULONG ulCapacity;  /*!< Allocated capacity.            */
} ABI_BUF, *PABI_BUF;

/*!
 * @brief Initialize a text buffer.
 *
 * @param[in] pBuf  Buffer. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET AbiBufInit(PABI_BUF pBuf)
{
    pBuf->ulCapacity = 256;
    pBuf->pszData = (PSZ)malloc(pBuf->ulCapacity);
    if (pBuf->pszData == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    pBuf->pszData[0] = '\0';
    pBuf->ulUsed = 1;
    return NO_ERROR;
}

/*!
 * @brief Free a text buffer.
 *
 * @param[in] pBuf  Buffer. Not NULL.
 */
static void AbiBufFree(PABI_BUF pBuf)
{
    if (pBuf->pszData != NULL)
        free(pBuf->pszData);
    pBuf->pszData = NULL;
    pBuf->ulUsed = 0;
    pBuf->ulCapacity = 0;
}

/*!
 * @brief Ensure the buffer has room for extra bytes plus a NUL.
 *
 * @param[in] pBuf     Buffer. Not NULL.
 * @param[in] cbExtra  Extra bytes about to be appended.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET AbiBufReserve(PABI_BUF pBuf, ULONG cbExtra)
{
    ULONG ulNeed = pBuf->ulUsed + cbExtra;
    ULONG ulNew;
    PSZ   pszTmp;

    if (ulNeed <= pBuf->ulCapacity)
        return NO_ERROR;

    ulNew = pBuf->ulCapacity;
    while (ulNew < ulNeed)
        ulNew *= 2;

    pszTmp = (PSZ)realloc(pBuf->pszData, ulNew);
    if (pszTmp == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    pBuf->pszData = pszTmp;
    pBuf->ulCapacity = ulNew;
    return NO_ERROR;
}

/*!
 * @brief Append a NUL-terminated string.
 *
 * @param[in] pBuf    Buffer. Not NULL.
 * @param[in] pszStr  String. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET AbiBufAppend(PABI_BUF pBuf, PCSZ pszStr)
{
    size_t cb = strlen(pszStr);
    APIRET rc = AbiBufReserve(pBuf, (ULONG)cb);

    if (rc != NO_ERROR)
        return rc;
    if (cb > 0) {
        memcpy(pBuf->pszData + pBuf->ulUsed - 1, pszStr, cb);
        pBuf->ulUsed += (ULONG)cb;
    }
    pBuf->pszData[pBuf->ulUsed - 1] = '\0';
    return NO_ERROR;
}

/*!
 * @brief Append a single character.
 *
 * @param[in] pBuf   Buffer. Not NULL.
 * @param[in] chChar Character to append.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET AbiBufPutChar(PABI_BUF pBuf, int chChar)
{
    APIRET rc = AbiBufReserve(pBuf, 1);

    if (rc != NO_ERROR)
        return rc;
    pBuf->pszData[pBuf->ulUsed - 1] = (CHAR)chChar;
    pBuf->pszData[pBuf->ulUsed] = '\0';
    pBuf->ulUsed++;
    return NO_ERROR;
}

/*!
 * @brief Append an unsigned long in decimal.
 *
 * @param[in] pBuf    Buffer. Not NULL.
 * @param[in] ulValue Value.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET AbiBufAppendULong(PABI_BUF pBuf, ULONG ulValue)
{
    CHAR ach[32];

    sprintf(ach, "%lu", (unsigned long)ulValue);
    return AbiBufAppend(pBuf, ach);
}

/* ------------------------------------------------------------------ */
/* Block array helpers                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @brief Release the storage owned by a single block.
 *
 * @param[in] pBlk  Block. Not NULL.
 */
static void AbiFreeBlock(PABI_BLOCK pBlk)
{
    ULONG i;

    if (pBlk->pszName != NULL)
        free(pBlk->pszName);
    for (i = 0; i < pBlk->ulCount; i++) {
        PABI_ENTRY_REC pRec = &pBlk->paEntries[i];
        if (pRec->pszName != NULL)
            free(pRec->pszName);
        if (pRec->pszInternal != NULL)
            free(pRec->pszInternal);
        if (pRec->pszConvention != NULL)
            free(pRec->pszConvention);
    }
    if (pBlk->paEntries != NULL)
        free(pBlk->paEntries);
    memset(pBlk, 0, sizeof(*pBlk));
}

/*!
 * @brief Open a new block and mark it current.
 *
 * Any previously open block is closed implicitly.
 *
 * @param[in] pHandle  Document handle. Not NULL.
 * @param[in] ulKind   ABI_KIND_MODULE or ABI_KIND_LIBRARY.
 * @param[in] pszName  Block name. Not NULL, not empty.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET AbiOpenBlock(ABI_HANDLE *pHandle, ULONG ulKind,
                           PCSZ pszName)
{
    PABI_BLOCK pBlk;

    pHandle->ulOpenIndex = (ULONG)-1L;

    if (pHandle->ulBlockCount == pHandle->ulBlockCap) {
        ULONG ulNew = (pHandle->ulBlockCap == 0)
                    ? 4 : pHandle->ulBlockCap * 2;
        PABI_BLOCK paTmp = (PABI_BLOCK)realloc(pHandle->paBlocks,
                                ulNew * sizeof(ABI_BLOCK));
        if (paTmp == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        pHandle->paBlocks = paTmp;
        pHandle->ulBlockCap = ulNew;
    }

    pBlk = &pHandle->paBlocks[pHandle->ulBlockCount];
    memset(pBlk, 0, sizeof(*pBlk));
    pBlk->ulKind = ulKind;
    pBlk->pszName = strdup(pszName);
    if (pBlk->pszName == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    pHandle->ulOpenIndex = pHandle->ulBlockCount;
    pHandle->ulBlockCount++;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Document lifecycle                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Create an empty document.
 *
 * @param[out] phDoc  Receives the handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p phDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiCreateDoc(HABIDOC *phDoc)
{
    ABI_HANDLE *pHandle;

    if (phDoc == NULL)
        return ERROR_INVALID_PARAMETER;

    *phDoc = NULLHANDLE;

    pHandle = (ABI_HANDLE *)calloc(1, sizeof(ABI_HANDLE));
    if (pHandle == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    pHandle->paBlocks     = NULL;
    pHandle->ulBlockCount = 0;
    pHandle->ulBlockCap   = 0;
    pHandle->ulOpenIndex  = (ULONG)-1L;

    *phDoc = (HABIDOC)pHandle;
    return NO_ERROR;
}

/*!
 * @brief Begin a new module block.
 *
 * @param[in] hDoc          Handle. Not NULLHANDLE.
 * @param[in] pszModuleName Module name. Not NULL, not empty.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiBeginModule(HABIDOC hDoc, PCSZ pszModuleName)
{
    ABI_HANDLE *pHandle = (ABI_HANDLE *)hDoc;

    if (pHandle == NULL || pszModuleName == NULL ||
        pszModuleName[0] == '\0')
        return ERROR_INVALID_PARAMETER;

    return AbiOpenBlock(pHandle, ABI_KIND_MODULE, pszModuleName);
}

/*!
 * @brief Begin a new library block.
 *
 * @param[in] hDoc           Handle. Not NULLHANDLE.
 * @param[in] pszLibraryName Library name. Not NULL, not empty.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiBeginLibrary(HABIDOC hDoc, PCSZ pszLibraryName)
{
    ABI_HANDLE *pHandle = (ABI_HANDLE *)hDoc;

    if (pHandle == NULL || pszLibraryName == NULL ||
        pszLibraryName[0] == '\0')
        return ERROR_INVALID_PARAMETER;

    return AbiOpenBlock(pHandle, ABI_KIND_LIBRARY, pszLibraryName);
}

/*!
 * @brief Append one entry to the currently open block.
 *
 * @param[in] hDoc    Handle. Not NULLHANDLE.
 * @param[in] pEntry  Entry. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments or no open block.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiAddEntry(HABIDOC hDoc, const ABI_ENTRY *pEntry)
{
    ABI_HANDLE    *pHandle = (ABI_HANDLE *)hDoc;
    PABI_BLOCK     pBlk;
    PABI_ENTRY_REC paTmp;
    ULONG          ulNew;
    ABI_ENTRY_REC  stRec;

    if (pHandle == NULL || pEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEntry->pszName == NULL || pEntry->pszName[0] == '\0')
        return ERROR_INVALID_PARAMETER;
    if (pHandle->ulOpenIndex == (ULONG)-1L)
        return ERROR_INVALID_PARAMETER;

    pBlk = &pHandle->paBlocks[pHandle->ulOpenIndex];

    memset(&stRec, 0, sizeof(stRec));

    stRec.pszName = strdup(pEntry->pszName);
    if (stRec.pszName == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    if (pEntry->pszInternal != NULL) {
        stRec.pszInternal = strdup(pEntry->pszInternal);
        if (stRec.pszInternal == NULL) {
            free(stRec.pszName);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (pEntry->pszConvention != NULL) {
        stRec.pszConvention = strdup(pEntry->pszConvention);
        if (stRec.pszConvention == NULL) {
            free(stRec.pszName);
            if (stRec.pszInternal != NULL)
                free(stRec.pszInternal);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    stRec.ulOrdinal = pEntry->ulOrdinal;
    stRec.fVariable = pEntry->fVariable;

    if (pBlk->ulCount == pBlk->ulCapacity) {
        ulNew = (pBlk->ulCapacity == 0) ? 16 : pBlk->ulCapacity * 2;
        paTmp = (PABI_ENTRY_REC)realloc(pBlk->paEntries,
                                ulNew * sizeof(ABI_ENTRY_REC));
        if (paTmp == NULL) {
            free(stRec.pszName);
            if (stRec.pszInternal != NULL)
                free(stRec.pszInternal);
            if (stRec.pszConvention != NULL)
                free(stRec.pszConvention);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pBlk->paEntries  = paTmp;
        pBlk->ulCapacity = ulNew;
    }

    pBlk->paEntries[pBlk->ulCount++] = stRec;
    return NO_ERROR;
}

/*!
 * @brief Release a document.
 *
 * @param[in] hDoc  Handle. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 */
APIRET APIENTRY AbiClose(HABIDOC hDoc)
{
    ABI_HANDLE *pHandle = (ABI_HANDLE *)hDoc;
    ULONG       i;

    if (pHandle == NULL)
        return NO_ERROR;

    for (i = 0; i < pHandle->ulBlockCount; i++)
        AbiFreeBlock(&pHandle->paBlocks[i]);
    if (pHandle->paBlocks != NULL)
        free(pHandle->paBlocks);

    free(pHandle);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Serialization                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emit one entry into the buffer.
 *
 * The plain form is:
 * @verbatim
     entry NAME;
     begin
       number = N;
       convention = 'X';
     end;
   @endverbatim
 *
 * The number clause is emitted only in module blocks (a library has
 * no ordinal). The convention clause is emitted in both kinds of
 * block unless the entry was added with fVariable set.
 *
 * When @c pszInternal is set, is not a variable, and carries
 * neither an ordinal nor a convention, the alias form is emitted
 * instead:
 * @verbatim
     entry NAME = INTERNAL;
   @endverbatim
 *
 * @param[in] pBuf      Buffer. Not NULL.
 * @param[in] pEntry    Entry. Not NULL.
 * @param[in] fLibrary  TRUE for a library block.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET AbiEmitEntry(PABI_BUF pBuf, const ABI_ENTRY_REC *pEntry,
                           BOOL fLibrary)
{
    APIRET rc;

    rc = AbiBufAppend(pBuf, ABI_INDENT_ENTRY "entry ");
    if (rc != NO_ERROR) return rc;
    rc = AbiBufAppend(pBuf, pEntry->pszName);
    if (rc != NO_ERROR) return rc;

    if (pEntry->pszInternal != NULL) {
        rc = AbiBufAppend(pBuf, " = ");
        if (rc != NO_ERROR) return rc;
        rc = AbiBufAppend(pBuf, pEntry->pszInternal);
        if (rc != NO_ERROR) return rc;
    }

    rc = AbiBufAppend(pBuf, ";\n");
    if (rc != NO_ERROR) return rc;

    if (pEntry->pszInternal != NULL && pEntry->ulOrdinal == 0 &&
        pEntry->pszConvention == NULL && !pEntry->fVariable)
        return NO_ERROR;

    rc = AbiBufAppend(pBuf, ABI_INDENT_ENTRY "begin\n");
    if (rc != NO_ERROR) return rc;

    if (!fLibrary && pEntry->ulOrdinal != 0) {
        rc = AbiBufAppend(pBuf, ABI_INDENT_FIELD "number = ");
        if (rc != NO_ERROR) return rc;
        rc = AbiBufAppendULong(pBuf, pEntry->ulOrdinal);
        if (rc != NO_ERROR) return rc;
        rc = AbiBufAppend(pBuf, ";\n");
        if (rc != NO_ERROR) return rc;
    }

    if (!pEntry->fVariable) {
        PCSZ pszConv = (pEntry->pszConvention != NULL)
                       ? pEntry->pszConvention
                       : "_System";
        rc = AbiBufAppend(pBuf, ABI_INDENT_FIELD "convention = '");
        if (rc != NO_ERROR) return rc;
        rc = AbiBufAppend(pBuf, pszConv);
        if (rc != NO_ERROR) return rc;
        rc = AbiBufAppend(pBuf, "';\n");
        if (rc != NO_ERROR) return rc;
    }

    return AbiBufAppend(pBuf, ABI_INDENT_ENTRY "end;\n");
}

/*!
 * @brief Emit one block into the buffer.
 *
 * @param[in] pBuf  Buffer. Not NULL.
 * @param[in] pBlk  Block. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET AbiEmitBlock(PABI_BUF pBuf, const ABI_BLOCK *pBlk)
{
    BOOL   fLib = (pBlk->ulKind == ABI_KIND_LIBRARY);
    APIRET rc;
    ULONG  i;

    rc = AbiBufAppend(pBuf, fLib ? "library " : "module ");
    if (rc != NO_ERROR) return rc;
    rc = AbiBufAppend(pBuf, pBlk->pszName);
    if (rc != NO_ERROR) return rc;
    rc = AbiBufAppend(pBuf, ";\nbegin\n");
    if (rc != NO_ERROR) return rc;

    for (i = 0; i < pBlk->ulCount; i++) {
        if (i > 0) {
            rc = AbiBufPutChar(pBuf, '\n');
            if (rc != NO_ERROR) return rc;
        }
        rc = AbiEmitEntry(pBuf, &pBlk->paEntries[i], fLib);
        if (rc != NO_ERROR) return rc;
    }

    return AbiBufAppend(pBuf, "end;\n");
}

/*!
 * @brief Serialize the document into a caller-supplied buffer.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiQueryText(HABIDOC hDoc, PSZ pszBuf, ULONG ulSize,
                             PULONG pulUsed)
{
    ABI_HANDLE *pHandle = (ABI_HANDLE *)hDoc;
    ABI_BUF     stBuf;
    APIRET      rc;
    ULONG       i;

    if (pHandle == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0)
        return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && pulUsed == NULL)
        return ERROR_INVALID_PARAMETER;

    rc = AbiBufInit(&stBuf);
    if (rc != NO_ERROR)
        return rc;

    for (i = 0; i < pHandle->ulBlockCount; i++) {
        if (i > 0) {
            rc = AbiBufPutChar(&stBuf, '\n');
            if (rc != NO_ERROR) goto done;
        }
        rc = AbiEmitBlock(&stBuf, &pHandle->paBlocks[i]);
        if (rc != NO_ERROR) goto done;
    }

    if (pszBuf == NULL) {
        if (pulUsed != NULL)
            *pulUsed = stBuf.ulUsed;
        AbiBufFree(&stBuf);
        return NO_ERROR;
    }

    if (stBuf.ulUsed > ulSize) {
        if (pulUsed != NULL)
            *pulUsed = stBuf.ulUsed;
        AbiBufFree(&stBuf);
        return ERROR_BUFFER_OVERFLOW;
    }

    memcpy(pszBuf, stBuf.pszData, stBuf.ulUsed);
    if (pulUsed != NULL)
        *pulUsed = stBuf.ulUsed - 1;

    AbiBufFree(&stBuf);
    return NO_ERROR;

done:
    AbiBufFree(&stBuf);
    return rc;
}

/*!
 * @brief Write the document to a file.
 *
 * @param[in] hDoc     Handle. Not NULLHANDLE.
 * @param[in] pszPath  Output file path. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_OPEN_FAILED        Cannot open output file.
 * @retval ERROR_WRITE_FAULT        Write error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY AbiWriteFile(HABIDOC hDoc, PCSZ pszPath)
{
    FILE   *fp;
    APIRET  rc;
    ULONG   ulSize;
    PSZ     pszBuf;

    if (hDoc == NULLHANDLE || pszPath == NULL)
        return ERROR_INVALID_PARAMETER;

    rc = AbiQueryText(hDoc, NULL, 0, &ulSize);
    if (rc != NO_ERROR)
        return rc;

    pszBuf = (PSZ)malloc(ulSize);
    if (pszBuf == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    rc = AbiQueryText(hDoc, pszBuf, ulSize, NULL);
    if (rc != NO_ERROR) {
        free(pszBuf);
        return rc;
    }

    fp = fopen(pszPath, "wb");
    if (fp == NULL) {
        free(pszBuf);
        return ERROR_OPEN_FAILED;
    }

    if (fwrite(pszBuf, 1, ulSize - 1, fp) != ulSize - 1) {
        fclose(fp);
        free(pszBuf);
        return ERROR_WRITE_FAULT;
    }

    if (fclose(fp) != 0) {
        free(pszBuf);
        return ERROR_WRITE_FAULT;
    }

    free(pszBuf);
    return NO_ERROR;
}
