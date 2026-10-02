/*!
 * @file dll.c
 *
 * @brief Implementation of the unified NE/LX DLL access library.
 *
 * The handle returned by DllOpen is a thin wrapper around either
 * an HNE or an HLX. The wrapper remembers which reader is in use
 * and dispatches each call to the corresponding Ne* or Lx* function.
 *
 * dll.h does not include newexe.h or lxexe.h; those headers are
 * included here, in this translation unit, so that callers of the
 * unified API never need to see the per-format interfaces.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os2types.h"
#include "os2err.h"
#include "mzexe.h"
#include "newexe.h"
#include "lxexe.h"
#include "dll.h"

/*!
 * @struct _DLL
 * @brief Internal representation behind HDLL.
 *
 * Not exposed to callers. dll.h declares the handle as HANDLE, so
 * the layout of this structure may change freely.
 */
struct _DLL {
    ULONG ulFormat;   /*!< DLL_FORMAT_NE or DLL_FORMAT_LX. */
    union {
        HNE hNe;      /*!< Open NE handle.                 */
        HLX hLx;      /*!< Open LX handle.                 */
    } u;
};

/*!
 * @struct _DLL_EXPORT
 * @brief Internal representation behind HDLLEXPORT.
 *
 * Not exposed to callers. The cursor stores the format tag and
 * the underlying per-format cursor.
 */
struct _DLL_EXPORT {
    ULONG ulFormat;   /*!< DLL_FORMAT_NE or DLL_FORMAT_LX. */
    union {
        HNEENUM hNe;  /*!< NE export cursor.               */
        HLXENUM hLx;  /*!< LX export cursor.               */
    } u;
};

/* ------------------------------------------------------------------ */
/* Format detection and lifecycle                                      */
/* ------------------------------------------------------------------ */

/*!
 * @brief Determine the format of a DLL file without opening it.
 *
 * Reads the MZ header, seeks to e_lfanew, and reads the signature
 * word at that offset. An unrecognized but readable file yields
 * DLL_FORMAT_UNKNOWN with a NO_ERROR return value.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] pFormat  Receives DLL_FORMAT_NE, DLL_FORMAT_LX or
 *                      DLL_FORMAT_UNKNOWN. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p pFormat is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         File is too short or has no
 *                                  valid MZ magic.
 */
APIRET APIENTRY DllDetectFormat(PCSZ pszPath, PULONG pFormat)
{
    FILE          *fp;
    struct exe_hdr mz;
    WORD           usMagic;

    if (!pszPath || !pFormat) return ERROR_INVALID_PARAMETER;
    *pFormat = DLL_FORMAT_UNKNOWN;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    if (fread(&mz, 1, sizeof(mz), fp) != sizeof(mz)) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (E_MAGIC(mz) != EMAGIC) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (fseek(fp, (long)E_LFANEW(mz), SEEK_SET) != 0) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    if (fread(&usMagic, 1, 2, fp) != 2) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    fclose(fp);

    if (usMagic == NEMAGIC)
        *pFormat = DLL_FORMAT_NE;
    else if (usMagic == LXMAGIC)
        *pFormat = DLL_FORMAT_LX;
    else
        *pFormat = DLL_FORMAT_UNKNOWN;

    return NO_ERROR;
}

/*!
 * @brief Open a DLL file.
 *
 * Detects the format by reading the MZ header and the signature
 * word at e_lfanew, then delegates to NeOpen or LxOpen. The format
 * is cached in the handle and returned by DllQueryFormat.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phDll    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on failure.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phDll is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Not a valid MZ/NE/LX file.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see DllClose
 */
APIRET APIENTRY DllOpen(PCSZ pszPath, HDLL *phDll)
{
    struct _DLL *pDll;
    ULONG        ulFormat;
    APIRET       rc;

    if (!pszPath || !phDll) return ERROR_INVALID_PARAMETER;
    *phDll = NULLHANDLE;

    rc = DllDetectFormat(pszPath, &ulFormat);
    if (rc != NO_ERROR)
        return rc;

    pDll = (struct _DLL *)calloc(1, sizeof(*pDll));
    if (!pDll)
        return ERROR_NOT_ENOUGH_MEMORY;
    pDll->ulFormat = ulFormat;

    switch (ulFormat) {
    case DLL_FORMAT_NE:
        rc = NeOpen(pszPath, &pDll->u.hNe);
        break;
    case DLL_FORMAT_LX:
        rc = LxOpen(pszPath, &pDll->u.hLx);
        break;
    default:
        rc = ERROR_READ_FAULT;
        break;
    }

    if (rc != NO_ERROR) {
        free(pDll);
        return rc;
    }

    *phDll = (HDLL)pDll;
    return NO_ERROR;
}

/*!
 * @brief Close a DLL file.
 *
 * Closes the underlying reader and releases the handle. This
 * function is idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 * @param[in] hDll  Handle from DllOpen. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 *
 * @see DllOpen
 */
APIRET APIENTRY DllClose(HDLL hDll)
{
    struct _DLL *pDll;

    if (hDll == NULLHANDLE) return NO_ERROR;
    pDll = (struct _DLL *)hDll;

    switch (pDll->ulFormat) {
    case DLL_FORMAT_NE:
        NeClose(pDll->u.hNe);
        break;
    case DLL_FORMAT_LX:
        LxClose(pDll->u.hLx);
        break;
    default:
        break;
    }
    free(pDll);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Metadata                                                            */
/* ------------------------------------------------------------------ */

/*!
 * @brief Retrieve the format of an open DLL.
 *
 * @param[in]  hDll     Handle from DllOpen. Not NULLHANDLE.
 * @param[out] pFormat  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hDll or @p pFormat is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllQueryFormat(HDLL hDll, PULONG pFormat)
{
    struct _DLL *pDll = (struct _DLL *)hDll;

    if (!pDll || !pFormat) return ERROR_INVALID_PARAMETER;
    *pFormat = pDll->ulFormat;
    return NO_ERROR;
}

/*!
 * @brief Return the module name of an open DLL.
 *
 * Delegates to NeQuerySelfModuleName or LxQuerySelfModuleName
 * depending on the format.
 *
 * @param[in]  hDll     Handle from DllOpen. Not NULLHANDLE.
 * @param[out] pszName  Output buffer. Not NULL.
 * @param[in]  cbName   Size of @p pszName in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hDll or @p pszName is NULL,
 *                                  or @p cbName is zero.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY DllQuerySelfModuleName(HDLL hDll, PSZ pszName,
                                       ULONG cbName)
{
    struct _DLL *pDll = (struct _DLL *)hDll;

    if (!pDll) return ERROR_INVALID_PARAMETER;

    switch (pDll->ulFormat) {
    case DLL_FORMAT_NE:
        return NeQuerySelfModuleName(pDll->u.hNe, pszName, cbName);
    case DLL_FORMAT_LX:
        return LxQuerySelfModuleName(pDll->u.hLx, pszName, cbName);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/* ------------------------------------------------------------------ */
/* Export cursor lifecycle                                             */
/* ------------------------------------------------------------------ */

/*!
 * @brief Open a cursor on the first export of a DLL.
 *
 * @param[in]  hDll      Handle from DllOpen. Not NULLHANDLE.
 * @param[out] phEnum    Cursor receiver. Not NULL. Set to NULLHANDLE
 *                       on error or when the module has no exports.
 * @param[out] pulCount  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hDll or @p phEnum is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No exports.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY DllExportFindFirst(HDLL hDll, HDLLEXPORT *phEnum,
                                   PULONG pulCount)
{
    struct _DLL        *pDll  = (struct _DLL *)hDll;
    struct _DLL_EXPORT *pEnum;
    APIRET              rc;

    if (!pDll || !phEnum) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;

    pEnum = (struct _DLL_EXPORT *)calloc(1, sizeof(*pEnum));
    if (!pEnum)
        return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->ulFormat = pDll->ulFormat;

    switch (pDll->ulFormat) {
    case DLL_FORMAT_NE:
        rc = NeExportFindFirst(pDll->u.hNe, &pEnum->u.hNe, pulCount);
        break;
    case DLL_FORMAT_LX:
        rc = LxExportFindFirst(pDll->u.hLx, &pEnum->u.hLx, pulCount);
        break;
    default:
        rc = ERROR_INVALID_HANDLE;
        break;
    }

    if (rc != NO_ERROR) {
        free(pEnum);
        return rc;
    }

    *phEnum = (HDLLEXPORT)pEnum;
    return NO_ERROR;
}

/*!
 * @brief Advance an export cursor to the next export.
 *
 * @param[in] hEnum  Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     @p hEnum is NULLHANDLE or its
 *                                  format tag is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more exports.
 */
APIRET APIENTRY DllExportFindNext(HDLLEXPORT hEnum)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_HANDLE;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportFindNext(pEnum->u.hNe);
    case DLL_FORMAT_LX:
        return LxExportFindNext(pEnum->u.hLx);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Close an export cursor.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Always.
 */
APIRET APIENTRY DllExportFindClose(HDLLEXPORT hEnum)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return NO_ERROR;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        NeExportFindClose(pEnum->u.hNe);
        break;
    case DLL_FORMAT_LX:
        LxExportFindClose(pEnum->u.hLx);
        break;
    default:
        break;
    }
    free(pEnum);
    return NO_ERROR;
}

/*!
 * @brief Position a new cursor on an export by name.
 *
 * @param[in]  hDll     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Name to find. Not NULL.
 * @param[out] phEnum   Cursor receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Name not present.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY DllExportFindByName(HDLL hDll, PCSZ pszName,
                                    HDLLEXPORT *phEnum)
{
    struct _DLL        *pDll  = (struct _DLL *)hDll;
    struct _DLL_EXPORT *pEnum;
    APIRET              rc;

    if (!pDll || !pszName || !phEnum) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;

    pEnum = (struct _DLL_EXPORT *)calloc(1, sizeof(*pEnum));
    if (!pEnum)
        return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->ulFormat = pDll->ulFormat;

    switch (pDll->ulFormat) {
    case DLL_FORMAT_NE:
        rc = NeExportFindByName(pDll->u.hNe, pszName, &pEnum->u.hNe);
        break;
    case DLL_FORMAT_LX:
        rc = LxExportFindByName(pDll->u.hLx, pszName, &pEnum->u.hLx);
        break;
    default:
        rc = ERROR_INVALID_HANDLE;
        break;
    }

    if (rc != NO_ERROR) {
        free(pEnum);
        return rc;
    }

    *phEnum = (HDLLEXPORT)pEnum;
    return NO_ERROR;
}

/*!
 * @brief Position a new cursor on an export by ordinal.
 *
 * @param[in]  hDll       Handle. Not NULLHANDLE.
 * @param[in]  usOrdinal  Ordinal to find (1-based).
 * @param[out] phEnum     Cursor receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hDll or @p phEnum is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Ordinal is not exported.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY DllExportFindByOrdinal(HDLL hDll, USHORT usOrdinal,
                                       HDLLEXPORT *phEnum)
{
    struct _DLL        *pDll  = (struct _DLL *)hDll;
    struct _DLL_EXPORT *pEnum;
    APIRET              rc;

    if (!pDll || !phEnum) return ERROR_INVALID_PARAMETER;
    *phEnum = NULLHANDLE;

    pEnum = (struct _DLL_EXPORT *)calloc(1, sizeof(*pEnum));
    if (!pEnum)
        return ERROR_NOT_ENOUGH_MEMORY;
    pEnum->ulFormat = pDll->ulFormat;

    switch (pDll->ulFormat) {
    case DLL_FORMAT_NE:
        rc = NeExportFindByOrdinal(pDll->u.hNe, usOrdinal,
                                   &pEnum->u.hNe);
        break;
    case DLL_FORMAT_LX:
        rc = LxExportFindByOrdinal(pDll->u.hLx, usOrdinal,
                                   &pEnum->u.hLx);
        break;
    default:
        rc = ERROR_INVALID_HANDLE;
        break;
    }

    if (rc != NO_ERROR) {
        free(pEnum);
        return rc;
    }

    *phEnum = (HDLLEXPORT)pEnum;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Export accessors                                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Retrieve the name of the current export.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
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
 */
APIRET APIENTRY DllExportGetName(HDLLEXPORT hEnum,
                                 PSZ pszBuf, ULONG ulSize,
                                 PULONG pulUsed)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportGetName(pEnum->u.hNe, pszBuf, ulSize, pulUsed);
    case DLL_FORMAT_LX:
        return LxExportGetName(pEnum->u.hLx, pszBuf, ulSize, pulUsed);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Retrieve the ordinal of the current export.
 *
 * @param[in]  hEnum       Cursor. Not NULLHANDLE.
 * @param[out] pusOrdinal  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportGetOrdinal(HDLLEXPORT hEnum, PUSHORT pusOrdinal)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportGetOrdinal(pEnum->u.hNe, pusOrdinal);
    case DLL_FORMAT_LX:
        return LxExportGetOrdinal(pEnum->u.hLx, pusOrdinal);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Retrieve the raw entry flags of the current export.
 *
 * @param[in]  hEnum     Cursor. Not NULLHANDLE.
 * @param[out] pulFlags  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportGetFlags(HDLLEXPORT hEnum, PULONG pulFlags)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportGetFlags(pEnum->u.hNe, pulFlags);
    case DLL_FORMAT_LX:
        return LxExportGetFlags(pEnum->u.hLx, pulFlags);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Query whether the current export has a real name.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
 * @param[out] pfNamed  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportIsNamed(HDLLEXPORT hEnum, PBOOL pfNamed)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportIsNamed(pEnum->u.hNe, pfNamed);
    case DLL_FORMAT_LX:
        return LxExportIsNamed(pEnum->u.hLx, pfNamed);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Query whether the current export is a variable.
 *
 * @param[in]  hEnum         Cursor. Not NULLHANDLE.
 * @param[out] pfGlobalData  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportIsGlobalData(HDLLEXPORT hEnum,
                                      PBOOL pfGlobalData)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportIsGlobalData(pEnum->u.hNe, pfGlobalData);
    case DLL_FORMAT_LX:
        return LxExportIsGlobalData(pEnum->u.hLx, pfGlobalData);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Query whether the current export is a forwarder.
 *
 * @param[in]  hEnum        Cursor. Not NULLHANDLE.
 * @param[out] pfForwarder  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportIsForwarder(HDLLEXPORT hEnum,
                                     PBOOL pfForwarder)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportIsForwarder(pEnum->u.hNe, pfForwarder);
    case DLL_FORMAT_LX:
        return LxExportIsForwarder(pEnum->u.hLx, pfForwarder);
    default:
        return ERROR_INVALID_HANDLE;
    }
}

/*!
 * @brief Determine the calling convention of the current export.
 *
 * Delegates to NeExportGetConvention or LxExportGetConvention
 * depending on the format. See the per-format documentation for
 * the details of the heuristic.
 *
 * @param[in]  hEnum    Cursor. Not NULLHANDLE.
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
 */
APIRET APIENTRY DllExportGetConvention(HDLLEXPORT hEnum,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed)
{
    struct _DLL_EXPORT *pEnum = (struct _DLL_EXPORT *)hEnum;

    if (!pEnum) return ERROR_INVALID_PARAMETER;

    switch (pEnum->ulFormat) {
    case DLL_FORMAT_NE:
        return NeExportGetConvention(pEnum->u.hNe, pszBuf, ulSize,
                                     pulUsed);
    case DLL_FORMAT_LX:
        return LxExportGetConvention(pEnum->u.hLx, pszBuf, ulSize,
                                     pulUsed);
    default:
        return ERROR_INVALID_HANDLE;
    }
}
