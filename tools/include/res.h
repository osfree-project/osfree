/* res.h - extract source file names from .res files (C89) */
#ifndef RES_H
#define RES_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file res.h
 * @brief Extract source file names from OpenWatcom/Borland .res files.
 *
 * The parser reads the standard RESOURCEHEADER structure and looks
 * for a project-specific dependency resource:
 *   - resource type 0x79 (DEP_LIST_TYPE)
 *   - resource name "EBWF_XFMMTUPPE" (DEP_LIST_NAME)
 *
 * The resource payload is a sequence of DepInfo records:
 *   DWORD time (Unix timestamp)
 *   WORD  len  (name length including terminating '\0')
 *   char  name[len]
 * terminated by a record with time = 0 and len = 0.
 *
 * References:
 *   - Microsoft Resource File Formats.
 *     https://learn.microsoft.com/en-us/windows/win32/menurc/resource-file-formats
 *   - RESOURCEHEADER structure.
 *     https://learn.microsoft.com/en-us/windows/win32/menurc/resourceheader
 *
 * The dependency resource itself is Open Watcom specific and is not
 * documented by Microsoft.
 */

/**
 * @brief Extract source file names from a .res file.
 *
 * Scans the file for the DEP_LIST_TYPE / DEP_LIST_NAME resource and
 * appends every unique source name found to @p hOut.
 *
 * @param[in] pszPath  Path to the .res file. Not NULL.
 * @param[in] hOut     Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or hOut is
 *                                  NULLHANDLE.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_FILE_NOT_FOUND     Dependency resource not found.
 *
 * @note The caller owns @p hOut and must release it with
 *       StrSetDestroy.
 */
APIRET APIENTRY ResExtractSources(PCSZ pszPath, HSTRSET hOut);

#ifdef __cplusplus
}
#endif

#endif /* RES_H */
