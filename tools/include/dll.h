/*! dll.h - unified access to NE and LX DLL files
 *
 *  Hides the differences between the NE and LX formats behind a
 *  single handle-based API. Uses newexe.h and lxexe.h internally;
 *  callers must not include those headers directly.
 */

#ifndef __DLL__
#define __DLL__

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file dll.h
 *  @brief Unified access to NE and LX DLL files.
 *
 *  Provides a single handle-based API for reading the module name
 *  and enumerating the exports of an NE or LX executable. The
 *  format is detected automatically at open time and the call is
 *  delegated to the appropriate reader.
 *
 *  All handles (HDLL, HDLLEXPORT) are opaque; the internal
 *  representation is private to dll.c.
 *
 *  References:
 *    - IBM OS/2 Toolkit, "New Executable File Format".
 *    - IBM OS/2 Toolkit, "Linear Executable File Format".
 */

/* ==================================================================
 * DLL formats
 * ================================================================== */

/**
 * @def DLL_FORMAT_UNKNOWN
 * @brief Format was not recognized. Value: 0.
 */
#define DLL_FORMAT_UNKNOWN  0

/**
 * @def DLL_FORMAT_NE
 * @brief New Executable (16-bit). Value: 1.
 */
#define DLL_FORMAT_NE       1

/**
 * @def DLL_FORMAT_LX
 * @brief Linear Executable (32-bit). Value: 2.
 */
#define DLL_FORMAT_LX       2

/* ------------------------------------------------------------------ */
/* DLL file access API                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Handle to an open DLL file.
 *
 *  Opaque. Created by DllOpen and released by DllClose. The internal
 *  representation is private to dll.c.
 *
 *  @see DllOpen
 *  @see DllClose
 */
typedef HANDLE HDLL;

/*! @brief Handle to a DLL export enumeration cursor.
 *
 *  Opaque. Created by DllExportFindFirst, DllExportFindByName or
 *  DllExportFindByOrdinal; released by DllExportFindClose. The
 *  internal representation is private to dll.c.
 *
 *  @see DllExportFindFirst
 *  @see DllExportFindClose
 */
typedef HANDLE HDLLEXPORT;

/*! @brief Determine the format of a DLL file without opening it.
 *
 *  Reads the MZ header and the two bytes at e_lfanew. On success
 *  writes DLL_FORMAT_NE, DLL_FORMAT_LX or DLL_FORMAT_UNKNOWN into
 *  @p pFormat. An unrecognized but readable file is reported as
 *  DLL_FORMAT_UNKNOWN with return value NO_ERROR; an unreadable file
 *  is reported through the error code.
 *
 *  @param[in]  pszPath  Path to the file. Not NULL.
 *  @param[out] pFormat  Receives the format. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p pFormat is NULL.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_READ_FAULT         File is too short to contain a
 *                                   valid MZ header.
 */
APIRET APIENTRY DllDetectFormat(PCSZ pszPath, PULONG pFormat);

/*! @brief Open a DLL file.
 *
 *  Detects the format (NE or LX) by reading the MZ header and the
 *  signature word at e_lfanew, then delegates to NeOpen or LxOpen.
 *  The format is cached in the handle and returned by DllQueryFormat.
 *
 *  @param[in]  pszPath  Path to the file. Not NULL.
 *  @param[out] phDll    Receives the handle. Not NULL. Set to
 *                       NULLHANDLE on failure.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phDll is NULL.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_READ_FAULT         Not a valid MZ/NE/LX file.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see DllClose
 */
APIRET APIENTRY DllOpen(PCSZ pszPath, HDLL *phDll);

/*! @brief Close a DLL file.
 *
 *  Closes the underlying reader and releases the handle. This
 *  function is idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 *  @param[in] hDll  Handle from DllOpen. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR  Always.
 *
 *  @see DllOpen
 */
APIRET APIENTRY DllClose(HDLL hDll);

/*! @brief Retrieve the format of an open DLL.
 *
 *  @param[in]  hDll     Handle. Not NULLHANDLE.
 *  @param[out] pFormat  Receives DLL_FORMAT_NE or DLL_FORMAT_LX.
 *                       Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDll or @p pFormat is
 *                                   invalid.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllQueryFormat(HDLL hDll, PULONG pFormat);

/*! @brief Return the module name of an open DLL.
 *
 *  The module name is the first entry of the Resident Name Table,
 *  stored as a length-prefixed string without a terminating NUL.
 *  This function copies it into @p pszName as a NUL-terminated
 *  string.
 *
 *  @param[in]  hDll     Handle. Not NULLHANDLE.
 *  @param[out] pszName  Receives a NUL-terminated name. Not NULL.
 *  @param[in]  cbName   Size of @p pszName including the NUL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL pointer.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_READ_FAULT         Read error.
 */
APIRET APIENTRY DllQuerySelfModuleName(HDLL hDll, PSZ pszName,
                                       ULONG cbName);

/* ------------------------------------------------------------------ */
/* DLL export enumeration API                                          */
/* ------------------------------------------------------------------ */

/*! @brief Open a cursor on the first export of a DLL.
 *
 *  The cursor walks the exports in increasing ordinal order. Bundles
 *  that contain no entries, and entries whose EXPORTED bit is clear,
 *  are skipped silently. Forwarder entries (LX only) are included;
 *  use DllExportIsForwarder to tell them apart. Exports without a
 *  name are still reported; use DllExportIsNamed to tell them apart.
 *
 *  @param[in]  hDll      Handle. Not NULLHANDLE.
 *  @param[out] phEnum    Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when the module has
 *                        no exports.
 *  @param[out] pulCount  Optional. May be NULL. On success receives
 *                        the total number of exports.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDll or @p phEnum is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      Module has no exports.
 *                                   *phEnum = NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see DllExportFindNext
 *  @see DllExportFindClose
 */
APIRET APIENTRY DllExportFindFirst(HDLL hDll, HDLLEXPORT *phEnum,
                                   PULONG pulCount);

/*! @brief Advance an export cursor to the next export.
 *
 *  @param[in] hEnum  Cursor from DllExportFindFirst or one of the
 *                    find-by functions. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_NO_MORE_ITEMS      No more exports.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see DllExportFindFirst
 *  @see DllExportFindClose
 */
APIRET APIENTRY DllExportFindNext(HDLLEXPORT hEnum);

/*! @brief Close an export cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR                Success. Also for NULLHANDLE.
 *  @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 *  @see DllExportFindFirst
 */
APIRET APIENTRY DllExportFindClose(HDLLEXPORT hEnum);

/*! @brief Position a new cursor on an export by name.
 *
 *  Comparison is case-sensitive. If several entries share the same
 *  name (possible with alias entries), the one with the lowest
 *  ordinal is chosen.
 *
 *  @param[in]  hDll    Handle. Not NULLHANDLE.
 *  @param[in]  pszName Name to find. Not NULL.
 *  @param[out] phEnum  Receives the cursor. Not NULL. Set to
 *                      NULLHANDLE on error.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Name not present.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see DllExportFindFirst
 *  @see DllExportFindClose
 */
APIRET APIENTRY DllExportFindByName(HDLL hDll, PCSZ pszName,
                                    HDLLEXPORT *phEnum);

/*! @brief Position a new cursor on an export by ordinal.
 *
 *  @param[in]  hDll       Handle. Not NULLHANDLE.
 *  @param[in]  usOrdinal  Ordinal to find (1-based).
 *  @param[out] phEnum     Receives the cursor. Not NULL. Set to
 *                         NULLHANDLE on error.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hDll or @p phEnum is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_FILE_NOT_FOUND     Ordinal is not exported.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see DllExportFindFirst
 *  @see DllExportFindClose
 */
APIRET APIENTRY DllExportFindByOrdinal(HDLL hDll, USHORT usOrdinal,
                                       HDLLEXPORT *phEnum);

/*! @brief Retrieve the name of the current export.
 *
 *  Size-query convention:
 *    - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *      NUL) is written, no buffer touched.
 *    - ulSize large enough: value copied and NUL-terminated; *pulUsed
 *      is the length without NUL.
 *    - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *      required size including NUL.
 *
 *  For an anonymous export (see DllExportIsNamed) a synthetic name of
 *  the form "Ordinal<N>" is returned, so the caller always receives a
 *  usable string. The real name, when present, is taken from the
 *  Resident Name Table first and from the Non-Resident Name Table
 *  otherwise.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf is
 *                                   NULL without size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *
 *  @see DllExportIsNamed
 */
APIRET APIENTRY DllExportGetName(HDLLEXPORT hEnum,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/*! @brief Retrieve the ordinal of the current export.
 *
 *  @param[in]  hEnum       Cursor. Not NULLHANDLE.
 *  @param[out] pusOrdinal  Receives the ordinal (1-based). Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportGetOrdinal(HDLLEXPORT hEnum, PUSHORT pusOrdinal);

/*! @brief Retrieve the raw entry flags of the current export.
 *
 *  The returned value is the FLAGS byte of the entry table record
 *  in the source format (NE or LX), without interpretation. Use
 *  DllExportIsGlobalData and DllExportIsForwarder for the
 *  corresponding predicates.
 *
 *  @param[in]  hEnum     Cursor. Not NULLHANDLE.
 *  @param[out] pulFlags  Receives the flags. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportGetFlags(HDLLEXPORT hEnum, PULONG pulFlags);

/*! @brief Query whether the current export has a real name.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pfNamed  Receives TRUE if a name is present in the
 *                       Resident or Non-Resident Name Table. Not
 *                       NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *
 *  @see DllExportGetName
 */
APIRET APIENTRY DllExportIsNamed(HDLLEXPORT hEnum, PBOOL pfNamed);

/*! @brief Query whether the current export is a variable.
 *
 *  For NE this tests the GLOBALDATA bit (NEENT_GLOBALDATA); for LX
 *  it tests the SINGLEDATA bit (LXENT_SINGLEDATA).
 *
 *  @param[in]  hEnum         Cursor. Not NULLHANDLE.
 *  @param[out] pfGlobalData  Receives TRUE for a variable. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportIsGlobalData(HDLLEXPORT hEnum, PBOOL pfGlobalData);

/*! @brief Query whether the current export is a forwarder.
 *
 *  Always returns FALSE for NE; for LX returns TRUE when the entry
 *  was stored in a forwarder bundle (LXENT_FORWARDER).
 *
 *  @param[in]  hEnum        Cursor. Not NULLHANDLE.
 *  @param[out] pfForwarder  Receives TRUE for a forwarder. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DllExportIsForwarder(HDLLEXPORT hEnum, PBOOL pfForwarder);

/*! @brief Determine the calling convention of the current export.
 *
 *  Scans the first bytes of the function body for the first
 *  occurrence of one of the following instruction bytes:
 *
 *    - C3        ret        -> "_System"       (near, caller cleanup)
 *    - C2 xx xx  ret imm16  -> "_Pascal" for NE, "_stdcall" for LX
 *    - CB        retf       -> "_System"       (far, caller cleanup)
 *    - CA xx xx  retf imm16 -> "_Far16 _Pascal" for NE,
 *                              "_stdcall" for LX
 *
 *  Forwarder entries always yield "_System": the body of the target
 *  is not available in this module. If none of the instruction bytes
 *  is found within the segment or object bounds, the default
 *  convention "_System" is returned. The choice is a heuristic:
 *  neither NE nor LX stores the calling convention explicitly.
 *
 *  Size-query convention as for DllExportGetName.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf is
 *                                   NULL without size-query.
 *  @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 *  @retval ERROR_READ_FAULT         Function body cannot be read.
 */
APIRET APIENTRY DllExportGetConvention(HDLLEXPORT hEnum,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __DLL__ */
