/*!
 * @file lib.c
 *
 * @brief Implementation of the OMF library reader.
 *
 * OMF library (.LIB) reader (C89). See lib.h for the public API.
 *
 * All public functions declared in lib.h are implemented here. The
 * internal representation of HOMFLIB is defined in this translation
 * unit only; callers see it as an opaque HANDLE.
 *
 * References:
 *   - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *     Object Module Format), Linux Foundation.
 *     https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *   - OpenWatcom WLIB librarian sources.
 *   - Microsoft OMF specification, "Relocatable Object Module
 *     Format", version 1.1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "omf.h"

/*!
 * @def LIB_RECORD_MAX
 * @brief Maximum size of an OMF record processed by the reader.
 */
#define LIB_RECORD_MAX 512

/*!
 * @def LIB_NAME_MAX
 * @brief Maximum length of a module or function name, in bytes.
 */
#define LIB_NAME_MAX 256

/*!
 * @struct _LIB_IMPDEF
 * @brief Cached record for one IMPDEF entry.
 */
struct _LIB_IMPDEF {
    char   achModule[LIB_NAME_MAX];  /*!< Module (DLL) name. */
    char   achName[LIB_NAME_MAX];    /*!< Function name.     */
    USHORT usOrdinal;                /*!< Ordinal.           */
};

/*!
 * @struct OMFLIB
 * @brief Internal representation behind HOMFLIB.
 *
 * Not exposed to callers. lib.h declares the handle as HANDLE, so
 * the layout of this structure may change freely.
 */
struct OMFLIB {
    FILE *fp;   /*!< Underlying file stream. */

    struct _LIB_IMPDEF *paImports;  /*!< IMPDEF cache.    */
    ULONG ulImportCount;            /*!< Used entries.    */
    ULONG ulImportCapacity;         /*!< Allocated.       */
    BOOL  fImportsParsed;           /*!< Lazy flag.       */
};

/*!
 * @struct _LIB_ENUM
 * @brief Internal representation behind HOMFLIBENUM.
 */
struct _LIB_ENUM {
    struct OMFLIB *pOwner;   /*!< Owning handle.    */
    ULONG          ulIndex;  /*!< Current position. */
};

/* ------------------------------------------------------------------ */
/* IMPDEF payload parsing                                              */
/* ------------------------------------------------------------------ */

/*!
 * @brief Extract one IMPDEF record from a COMENT payload.
 *
 * The payload layout recognized here is:
 * @verbatim
   [type:1][class:1][subtype:1][ord_flag:1][name_len:1]
   [name:name_len][mod_len:1][module:mod_len][ordinal:2]
   @endverbatim
 *
 * @param[in]  puchBuf   Record payload. Not NULL.
 * @param[in]  cbLen     Payload length in bytes.
 * @param[out] pRec      Receiver. Not NULL.
 * @param[out] pfMatched Set to 1 on a valid IMPDEF record, 0
 *                       otherwise. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Record processed.
 */
static APIRET lib_extract_impdef(const UCHAR *puchBuf, ULONG cbLen,
                                 struct _LIB_IMPDEF *pRec,
                                 int *pfMatched)
{
    ULONG cbNameLen;
    ULONG cbModOffset;
    ULONG cbModLen;
    ULONG cbOrdOffset;

    *pfMatched = 0;

    if (cbLen < 5) return NO_ERROR;
    if (puchBuf[0] != OMF_COMENT_TYPE_NOECHO &&
        puchBuf[0] != OMF_COMENT_TYPE_NORMAL) return NO_ERROR;
    if (puchBuf[1] != LIB_COMENT_CLASS_IMPDEF) return NO_ERROR;
    if (puchBuf[2] != LIB_IMPDEF_SUBTYPE_IMPORT) return NO_ERROR;
    if (puchBuf[3] == 0) return NO_ERROR;

    cbNameLen   = puchBuf[4];
    cbModOffset = 5 + cbNameLen;
    if (cbModOffset >= cbLen) return NO_ERROR;

    cbModLen    = puchBuf[cbModOffset];
    cbOrdOffset = cbModOffset + 1 + cbModLen;
    if (cbOrdOffset + 1 >= cbLen) return NO_ERROR;
    if (cbModLen >= LIB_NAME_MAX) return NO_ERROR;
    if (cbNameLen >= LIB_NAME_MAX) return NO_ERROR;

    memcpy(pRec->achName, &puchBuf[5], cbNameLen);
    pRec->achName[cbNameLen] = '\0';

    memcpy(pRec->achModule, &puchBuf[cbModOffset + 1], cbModLen);
    pRec->achModule[cbModLen] = '\0';

    pRec->usOrdinal = (USHORT)(puchBuf[cbOrdOffset] |
                               ((USHORT)puchBuf[cbOrdOffset + 1] << 8));

    *pfMatched = 1;
    return NO_ERROR;
}

/*!
 * @brief Inspect one COMENT payload as a specific IMPDEF record.
 *
 * Same payload layout as lib_extract_impdef, but only reports a
 * match when the module and ordinal also match the requested
 * values.
 *
 * @param[in]  puchBuf    Record payload. Not NULL.
 * @param[in]  cbLen      Payload length in bytes.
 * @param[in]  pszModule  Module name to match. Not NULL.
 * @param[in]  usOrdinal  Ordinal to match.
 * @param[out] pszName    Output buffer for the function name. Not
 *                        NULL.
 * @param[in]  cbName     Size of @p pszName in bytes, including
 *                        space for the NUL terminator.
 * @param[out] pfMatched  Set to 1 on match, 0 otherwise. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Record processed.
 * @retval ERROR_INVALID_PARAMETER  Match found but the name does
 *                                  not fit in @p pszName.
 */
static APIRET lib_check_impdef(const UCHAR *puchBuf, ULONG cbLen,
                               PCSZ pszModule, USHORT usOrdinal,
                               PSZ pszName, ULONG cbName,
                               int *pfMatched)
{
    struct _LIB_IMPDEF stRec;

    *pfMatched = 0;

    if (lib_extract_impdef(puchBuf, cbLen, &stRec, pfMatched)
        != NO_ERROR)
        return NO_ERROR;

    if (!*pfMatched) return NO_ERROR;
    if (stRec.usOrdinal != usOrdinal) return NO_ERROR;
    if (strcmp(stRec.achModule, pszModule) != 0) return NO_ERROR;

    if (strlen(stRec.achName) + 1 > cbName)
        return ERROR_INVALID_PARAMETER;

    strcpy(pszName, stRec.achName);
    *pfMatched = 1;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* IMPDEF cache                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Append one record to the handle's IMPDEF cache.
 *
 * @param[in] pLib  Handle. Not NULL.
 * @param[in] pRec  Record to append. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Reallocation failure.
 */
static APIRET lib_append_import(struct OMFLIB *pLib,
                                const struct _LIB_IMPDEF *pRec)
{
    struct _LIB_IMPDEF *paTmp;
    ULONG ulNew;

    if (pLib->ulImportCount == pLib->ulImportCapacity) {
        ulNew = (pLib->ulImportCapacity == 0)
              ? 64 : pLib->ulImportCapacity * 2;
        paTmp = (struct _LIB_IMPDEF *)realloc(pLib->paImports,
                    ulNew * sizeof(struct _LIB_IMPDEF));
        if (paTmp == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        pLib->paImports = paTmp;
        pLib->ulImportCapacity = ulNew;
    }
    pLib->paImports[pLib->ulImportCount++] = *pRec;
    return NO_ERROR;
}

/*!
 * @brief Scan the whole library and cache every IMPDEF record.
 *
 * Called lazily on the first LibImportFindFirst. On success, the
 * handle holds a complete list of IMPDEF records in file order.
 *
 * @param[in] pLib  Handle. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_READ_FAULT         Read error while scanning.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET lib_parse_imports(struct OMFLIB *pLib)
{
    OMF_RECORD_HEADER head;
    UCHAR  auchBuf[LIB_RECORD_MAX];
    long   lPos;
    int    fMatched;
    APIRET rc;

    if (pLib->fImportsParsed)
        return NO_ERROR;

    if (fseek(pLib->fp, 0, SEEK_SET) != 0)
        return ERROR_READ_FAULT;

    for (;;) {
        size_t cbRead = fread(&head, 1, sizeof(head), pLib->fp);
        if (cbRead != sizeof(head)) break;

        if (head.usLength > sizeof(auchBuf)) {
            if (fseek(pLib->fp, (long)head.usLength, SEEK_CUR) != 0)
                return ERROR_READ_FAULT;
            continue;
        }

        if (head.usLength > 0) {
            if (fread(auchBuf, 1, head.usLength, pLib->fp)
                != head.usLength)
                return ERROR_READ_FAULT;

            if (head.uchType == OMF_TYPE_COMENT) {
                struct _LIB_IMPDEF stRec;

                rc = lib_extract_impdef(auchBuf, head.usLength,
                                        &stRec, &fMatched);
                if (rc != NO_ERROR) return rc;

                if (fMatched) {
                    rc = lib_append_import(pLib, &stRec);
                    if (rc != NO_ERROR) return rc;
                }
            }
        }

        if (head.uchType == LIB_TYPE_TERMINATOR) break;

        if (head.uchType == OMF_TYPE_MODEND) {
            lPos = ftell(pLib->fp);
            if (lPos < 0) return ERROR_READ_FAULT;
            if ((16 - lPos % 16) != 16) {
                if (fseek(pLib->fp, (long)(16 - lPos % 16),
                          SEEK_CUR) != 0)
                    return ERROR_READ_FAULT;
            }
        }
    }

    pLib->fImportsParsed = TRUE;
    return NO_ERROR;
}

/*!
 * @brief Copy a string using the size-query convention.
 *
 * @param[in]  pszSrc   Source string. Not NULL.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszBuf is NULL without
 *                                  size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
static APIRET lib_copy_string(PCSZ pszSrc, PSZ pszBuf, ULONG ulSize,
                              PULONG pulUsed)
{
    size_t cb = strlen(pszSrc);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL)
            *pulUsed = (ULONG)(cb + 1);
        return NO_ERROR;
    }
    if (pszBuf == NULL)
        return ERROR_INVALID_PARAMETER;
    if (cb + 1 > ulSize) {
        if (pulUsed != NULL)
            *pulUsed = (ULONG)(cb + 1);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszSrc, cb + 1);
    if (pulUsed != NULL)
        *pulUsed = (ULONG)cb;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

APIRET APIENTRY LibOpen(PCSZ pszPath, HOMFLIB *phLib)
{
    FILE *fp;
    struct OMFLIB *pLib;

    if (!pszPath || !phLib) return ERROR_INVALID_PARAMETER;
    *phLib = NULLHANDLE;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    pLib = (struct OMFLIB *)calloc(1, sizeof(*pLib));
    if (!pLib) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pLib->fp = fp;
    pLib->paImports = NULL;
    pLib->ulImportCount = 0;
    pLib->ulImportCapacity = 0;
    pLib->fImportsParsed = FALSE;
    *phLib = (HOMFLIB)pLib;
    return NO_ERROR;
}

APIRET APIENTRY LibClose(HOMFLIB hLib)
{
    struct OMFLIB *pLib;

    if (hLib == NULLHANDLE) return NO_ERROR;
    pLib = (struct OMFLIB *)hLib;
    if (pLib->fp) fclose(pLib->fp);
    if (pLib->paImports) free(pLib->paImports);
    free(pLib);
    return NO_ERROR;
}

APIRET APIENTRY LibQueryFunction(HOMFLIB hLib, PCSZ pszModule,
                                 USHORT usOrdinal,
                                 PSZ pszName, ULONG cbName)
{
    struct OMFLIB *pLib;
    OMF_RECORD_HEADER head;
    UCHAR auchBuf[LIB_RECORD_MAX];
    long lPos;
    int fMatched;
    APIRET rc;

    if (hLib == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (!pszModule || !pszName || cbName == 0)
        return ERROR_INVALID_PARAMETER;

    pLib = (struct OMFLIB *)hLib;
    if (!pLib->fp) return ERROR_INVALID_PARAMETER;

    if (fseek(pLib->fp, 0, SEEK_SET) != 0)
        return ERROR_READ_FAULT;

    for (;;) {
        size_t cbRead;

        cbRead = fread(&head, 1, sizeof(head), pLib->fp);
        if (cbRead != sizeof(head)) break;

        if (head.usLength > sizeof(auchBuf)) {
            if (fseek(pLib->fp, (long)head.usLength, SEEK_CUR) != 0)
                return ERROR_READ_FAULT;
            continue;
        }

        if (head.usLength > 0) {
            if (fread(auchBuf, 1, head.usLength, pLib->fp)
                != head.usLength)
                return ERROR_READ_FAULT;

            if (head.uchType == OMF_TYPE_COMENT) {
                rc = lib_check_impdef(auchBuf, head.usLength,
                                      pszModule, usOrdinal,
                                      pszName, cbName, &fMatched);
                if (rc != NO_ERROR) return rc;
                if (fMatched) return NO_ERROR;
            }
        }

        if (head.uchType == LIB_TYPE_TERMINATOR) break;

        if (head.uchType == OMF_TYPE_MODEND) {
            lPos = ftell(pLib->fp);
            if (lPos < 0) return ERROR_READ_FAULT;
            if ((16 - lPos % 16) != 16) {
                if (fseek(pLib->fp, (long)(16 - lPos % 16),
                          SEEK_CUR) != 0)
                    return ERROR_READ_FAULT;
            }
        }
    }

    return ERROR_FILE_NOT_FOUND;
}

APIRET APIENTRY LibImportFindFirst(HOMFLIB hLib,
                                   HOMFLIBENUM *phEnum,
                                   PULONG pulCount)
{
    struct OMFLIB    *pLib;
    struct _LIB_ENUM *pEnum;
    APIRET rc;

    if (hLib == NULLHANDLE || phEnum == NULL)
        return ERROR_INVALID_PARAMETER;

    pLib = (struct OMFLIB *)hLib;
    *phEnum = NULLHANDLE;

    rc = lib_parse_imports(pLib);
    if (rc != NO_ERROR)
        return rc;

    if (pulCount != NULL)
        *pulCount = pLib->ulImportCount;

    if (pLib->ulImportCount == 0)
        return ERROR_NO_MORE_ITEMS;

    pEnum = (struct _LIB_ENUM *)malloc(sizeof(*pEnum));
    if (pEnum == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->pOwner  = pLib;
    pEnum->ulIndex = 0;

    *phEnum = (HOMFLIBENUM)pEnum;
    return NO_ERROR;
}

APIRET APIENTRY LibImportFindNext(HOMFLIBENUM hEnum)
{
    struct _LIB_ENUM *pEnum = (struct _LIB_ENUM *)hEnum;

    if (pEnum == NULL)
        return ERROR_INVALID_HANDLE;
    if (pEnum->ulIndex + 1 >= pEnum->pOwner->ulImportCount)
        return ERROR_NO_MORE_ITEMS;
    pEnum->ulIndex++;
    return NO_ERROR;
}

APIRET APIENTRY LibImportFindClose(HOMFLIBENUM hEnum)
{
    if (hEnum != NULLHANDLE)
        free(hEnum);
    return NO_ERROR;
}

APIRET APIENTRY LibImportGetModule(HOMFLIBENUM hEnum,
                                   PSZ pszBuf, ULONG ulSize,
                                   PULONG pulUsed)
{
    struct _LIB_ENUM *pEnum = (struct _LIB_ENUM *)hEnum;

    if (pEnum == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulImportCount)
        return ERROR_INVALID_PARAMETER;

    return lib_copy_string(
        pEnum->pOwner->paImports[pEnum->ulIndex].achModule,
        pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY LibImportGetName(HOMFLIBENUM hEnum,
                                 PSZ pszBuf, ULONG ulSize,
                                 PULONG pulUsed)
{
    struct _LIB_ENUM *pEnum = (struct _LIB_ENUM *)hEnum;

    if (pEnum == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulImportCount)
        return ERROR_INVALID_PARAMETER;

    return lib_copy_string(
        pEnum->pOwner->paImports[pEnum->ulIndex].achName,
        pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY LibImportGetOrdinal(HOMFLIBENUM hEnum,
                                    PUSHORT pusOrdinal)
{
    struct _LIB_ENUM *pEnum = (struct _LIB_ENUM *)hEnum;

    if (pEnum == NULL || pusOrdinal == NULL)
        return ERROR_INVALID_PARAMETER;
    if (pEnum->ulIndex >= pEnum->pOwner->ulImportCount)
        return ERROR_INVALID_PARAMETER;

    *pusOrdinal = pEnum->pOwner->paImports[pEnum->ulIndex].usOrdinal;
    return NO_ERROR;
}
