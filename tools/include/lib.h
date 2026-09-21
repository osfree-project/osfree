/*! lib.h - OMF library (.LIB) reader (C89)
 *
 *  Reads OMF library files (.LIB) as produced by the OpenWatcom
 *  librarian (WLIB) and compatible tools. Provides handle-based
 *  access to IMPDEF records, which map (module, ordinal) pairs to
 *  function names.
 *
 *  References:
 *    - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *      Object Module Format), Linux Foundation.
 *      https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *    - OpenWatcom WLIB librarian sources.
 *    - Microsoft OMF specification, "Relocatable Object Module
 *      Format", version 1.1.
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
 *  file. After opening the file with LibOpen, the caller can look up
 *  function names by module and ordinal via LibQueryFunction. The
 *  library is closed with LibClose.
 *
 *  All handles (HOMFLIB) are opaque. The internal representation is
 *  private to lib.c.
 *
 *  References:
 *    - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *      Object Module Format), Linux Foundation.
 *      https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *    - OpenWatcom WLIB librarian sources.
 *    - Microsoft OMF specification, "Relocatable Object Module
 *      Format", version 1.1.
 *
 *  @see LibOpen
 *  @see LibQueryFunction
 *  @see LibClose
 */

/*! @name OMF library record types
 *
 *  Record types specific to OMF library files. Library files use the
 *  same record structure as object files, plus a library header and
 *  terminator.
 */
/*! @{ */
#define LIB_TYPE_HEADER     0xF0  /*!< Library header record. */
#define LIB_TYPE_TERMINATOR 0xF1  /*!< Library terminator record. */
/*! @} */

/*! @name IMPDEF comment classes and subtypes
 *
 *  The IMPDEF record is encoded as a COMENT record whose comment
 *  class is LIB_COMENT_CLASS_IMPDEF and whose subtype is
 *  LIB_IMPDEF_SUBTYPE_IMPORT.
 */
/*! @{ */
#define LIB_COMENT_CLASS_IMPDEF     0xA0  /*!< IMPDEF comment class. */
#define LIB_IMPDEF_SUBTYPE_IMPORT   0x01  /*!< Import subtype. */
/*! @} */

/*! @brief Handle to an open OMF library.
 *
 *  Opaque. Created by LibOpen, released by LibClose. The internal
 *  representation is private to lib.c.
 *
 *  @see LibOpen
 *  @see LibClose
 */
typedef HANDLE HOMFLIB;

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
 *  Closes the underlying file and releases the internal handle. This
 *  function is idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 *  @param[in] hLib  Handle from LibOpen. NULLHANDLE is accepted and
 *                   ignored.
 *
 *  @return APIRET
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
 *  @param[out] pszName    Receives the NUL-terminated function name.
 *                         Not NULL.
 *  @param[in]  cbName     Size of @p pszName in bytes, including
 *                         space for the NUL terminator.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Match found and name copied.
 *  @retval ERROR_INVALID_PARAMETER  @p hLib is NULLHANDLE, or
 *                                   @p pszModule, @p pszName is NULL,
 *                                   or @p cbName is 0, or the name
 *                                   does not fit in @p pszName.
 *  @retval ERROR_READ_FAULT         Read error while scanning.
 *  @retval ERROR_FILE_NOT_FOUND     No matching IMPDEF record.
 *
 *  @see LibOpen
 */
APIRET APIENTRY LibQueryFunction(HOMFLIB hLib, PCSZ pszModule,
                                 USHORT usOrdinal,
                                 PSZ pszName, ULONG cbName);

#ifdef __cplusplus
}
#endif

#endif /* LIB_H */
