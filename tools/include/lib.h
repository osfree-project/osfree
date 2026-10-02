/*!
 * @file lib.h
 *
 * @brief OMF library (.LIB) reader.
 *
 * Reads OMF library files (.LIB) as produced by the OpenWatcom
 * librarian (WLIB) and compatible tools. Provides handle-based
 * access to IMPDEF records, which map (module, ordinal) pairs to
 * function names.
 *
 * References:
 *   - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *     Object Module Format), Linux Foundation.
 *     https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *   - OpenWatcom WLIB librarian sources.
 *   - Microsoft OMF specification, "Relocatable Object Module
 *     Format", version 1.1.
 */

#ifndef LIB_H
#define LIB_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file lib.h
 *  @brief OMF library (.LIB) reader.
 *
 *  The library exposes a handle-based interface to an OMF library
 *  file. After opening the file with LibOpen, the caller can look
 *  up function names by module and ordinal via LibQueryFunction,
 *  or walk every IMPDEF record with the LibImportFind* cursor.
 *
 *  All handles (HOMFLIB, HOMFLIBENUM) are opaque. The internal
 *  representation is private to lib.c.
 *
 *  @see LibOpen
 *  @see LibQueryFunction
 *  @see LibImportFindFirst
 *  @see LibClose
 */

/*! @name OMF library record types */
/*! @{ */

/*!
 * @def LIB_TYPE_HEADER
 * @brief Library header record. Value: 0xF0.
 */
#define LIB_TYPE_HEADER     0xF0

/*!
 * @def LIB_TYPE_TERMINATOR
 * @brief Library terminator record. Value: 0xF1.
 */
#define LIB_TYPE_TERMINATOR 0xF1

/*! @} */

/*! @name IMPDEF comment classes and subtypes */
/*! @{ */

/*!
 * @def LIB_COMENT_CLASS_IMPDEF
 * @brief IMPDEF comment class. Value: 0xA0.
 */
#define LIB_COMENT_CLASS_IMPDEF     0xA0

/*!
 * @def LIB_IMPDEF_SUBTYPE_IMPORT
 * @brief Import subtype. Value: 0x01.
 */
#define LIB_IMPDEF_SUBTYPE_IMPORT   0x01

/*! @} */

/*! @brief Handle to an open OMF library.
 *
 *  Opaque. Created by LibOpen, released by LibClose.
 *
 *  @see LibOpen
 *  @see LibClose
 */
typedef HANDLE HOMFLIB;

/*! @brief Handle to an IMPDEF enumeration cursor.
 *
 *  Opaque. Created by LibImportFindFirst, released by
 *  LibImportFindClose.
 *
 *  @see LibImportFindFirst
 *  @see LibImportFindClose
 */
typedef HANDLE HOMFLIBENUM;

/*! @brief Open an OMF library file.
 *
 *  Opens @p pszPath for binary reading and allocates an internal
 *  handle. The file remains open until LibClose is called.
 *
 *  @param[in]  pszPath  Path to the .LIB file. Not NULL.
 *  @param[out] phLib    Receives the library handle. Not NULL. Set
 *                       to NULLHANDLE on failure.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phLib is NULL.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see LibClose
 *  @see LibQueryFunction
 */
APIRET APIENTRY LibOpen(PCSZ pszPath, HOMFLIB *phLib);

/*! @brief Close an OMF library.
 *
 *  Closes the underlying file and releases the internal handle.
 *  This function is idempotent: passing NULLHANDLE returns
 *  NO_ERROR.
 *
 *  @param[in] hLib  Handle from LibOpen. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR  Always.
 *
 *  @see LibOpen
 */
APIRET APIENTRY LibClose(HOMFLIB hLib);

/*! @brief Look up a function name by module and ordinal.
 *
 *  Scans the library from the beginning for an IMPDEF record whose
 *  module name equals @p pszModule and whose ordinal equals
 *  @p usOrdinal. On a match, the function name is copied into
 *  @p pszName as a NUL-terminated string.
 *
 *  The scan walks OMF records sequentially. Records longer than the
 *  internal 512-byte buffer are skipped. After each MODEND record
 *  the file position is aligned to the next 16-byte boundary, as
 *  required by the OMF library format. The scan stops at a library
 *  terminator record.
 *
 *  @param[in]  hLib       Handle from LibOpen. Not NULLHANDLE.
 *  @param[in]  pszModule  Module name to match. Not NULL.
 *  @param[in]  usOrdinal  Function ordinal to match.
 *  @param[out] pszName    Receives the NUL-terminated function
 *                         name. Not NULL.
 *  @param[in]  cbName     Size of @p pszName in bytes, including
 *                         space for the NUL terminator.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Match found and name copied.
 *  @retval ERROR_INVALID_PARAMETER  @p hLib is NULLHANDLE, or
 *                                   @p pszModule, @p pszName is
 *                                   NULL, or @p cbName is 0, or
 *                                   the name does not fit in
 *                                   @p pszName.
 *  @retval ERROR_READ_FAULT         Read error while scanning.
 *  @retval ERROR_FILE_NOT_FOUND     No matching IMPDEF record.
 *
 *  @see LibOpen
 */
APIRET APIENTRY LibQueryFunction(HOMFLIB hLib, PCSZ pszModule,
                                 USHORT usOrdinal,
                                 PSZ pszName, ULONG cbName);

/*! @brief Open a cursor on the first IMPDEF record.
 *
 *  The library is scanned once and every IMPDEF record is cached
 *  in the handle. The cursor walks the records in the order in
 *  which they appear in the file.
 *
 *  @param[in]  hLib      Handle from LibOpen. Not NULLHANDLE.
 *  @param[out] phEnum    Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when the library
 *                        contains no IMPDEF records.
 *  @param[out] pulCount  Optional. May be NULL. On success
 *                        receives the total number of IMPDEF
 *                        records.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hLib or @p phEnum is NULL.
 *  @retval ERROR_NO_MORE_ITEMS      No IMPDEF records.
 *                                   *phEnum = NULLHANDLE.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_READ_FAULT         Read error.
 *
 *  @see LibImportFindNext
 *  @see LibImportFindClose
 */
APIRET APIENTRY LibImportFindFirst(HOMFLIB hLib,
                                   HOMFLIBENUM *phEnum,
                                   PULONG pulCount);

/*! @brief Advance an IMPDEF cursor to the next record.
 *
 *  @param[in] hEnum  Cursor from LibImportFindFirst. Not
 *                    NULLHANDLE.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_HANDLE     @p hEnum is NULLHANDLE.
 *  @retval ERROR_NO_MORE_ITEMS      No more records.
 *
 *  @see LibImportFindFirst
 *  @see LibImportFindClose
 */
APIRET APIENTRY LibImportFindNext(HOMFLIBENUM hEnum);

/*! @brief Close an IMPDEF cursor.
 *
 *  Passing NULLHANDLE is a no-op.
 *
 *  @param[in] hEnum  Cursor. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR  Always.
 *
 *  @see LibImportFindFirst
 */
APIRET APIENTRY LibImportFindClose(HOMFLIBENUM hEnum);

/*! @brief Retrieve the module name of the current IMPDEF record.
 *
 *  Size-query convention:
 *    - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *      NUL) is written, no buffer touched.
 *    - ulSize large enough: value copied and NUL-terminated;
 *      *pulUsed is the length without NUL.
 *    - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *      required size including NUL.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf
 *                                   is NULL without size-query.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY LibImportGetModule(HOMFLIBENUM hEnum,
                                   PSZ pszBuf, ULONG ulSize,
                                   PULONG pulUsed);

/*! @brief Retrieve the function name of the current IMPDEF record.
 *
 *  Size-query convention as for LibImportGetModule.
 *
 *  @param[in]  hEnum    Cursor. Not NULLHANDLE.
 *  @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 *  @param[in]  ulSize   Size of @p pszBuf in bytes.
 *  @param[out] pulUsed  Optional. May be NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum is NULL, or @p pszBuf
 *                                   is NULL without size-query.
 *  @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY LibImportGetName(HOMFLIBENUM hEnum,
                                 PSZ pszBuf, ULONG ulSize,
                                 PULONG pulUsed);

/*! @brief Retrieve the ordinal of the current IMPDEF record.
 *
 *  @param[in]  hEnum       Cursor. Not NULLHANDLE.
 *  @param[out] pusOrdinal  Receives the ordinal. Not NULL.
 *
 *  @return APIRET
 *
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p hEnum or @p pusOrdinal is
 *                                   NULL.
 */
APIRET APIENTRY LibImportGetOrdinal(HOMFLIBENUM hEnum,
                                    PUSHORT pusOrdinal);

#ifdef __cplusplus
}
#endif

#endif /* LIB_H */
