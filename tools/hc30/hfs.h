/*!
 * @file hfs.h
 * @brief HLP file system container.
 *
 * @par Function naming
 * Function names follow OS/2 Control Program API conventions as
 * defined in OS/2 Version 2.0 - Volume 4: Writing Applications,
 * section 12.2 "Function Name Changes".
 *
 * @par References
 *  - [1] SAA Common Programming Interface, C Reference - Level 2,
 *        SC09-1308-02 (Sep 1991).
 *  - [2] OS/2 Version 2.0 - Volume 4: Writing Applications,
 *        IBM, 1993, section 12.2.
 *  - [3] Winterhoff, M., helpdeco (1997), HELPHEADER, FILEHEADER.
 *  - [4] ccl container library: vector.h.
 *
 * @par Thread safety
 * The module is single threaded. Callers must provide locking if an
 * HLP file system handle is shared between threads.
 *
 * @par Ownership
 * HHFS is caller-owned and released with HfsDestroy. Data passed to
 * HfsAddFile and HfsAddFileFromWriter is copied into the container.
 */
#ifndef HFS_H
#define HFS_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

/* Local fallback: os2types.h does not provide VOID on host builds. */
#ifndef VOID
#define VOID void
#endif

/*!
 * @typedef HHFS
 * @brief Handle to an HLP file system. Caller-owned.
 */
typedef HANDLE HHFS;

/*!
 * @typedef PHHFS
 * @brief Pointer to an HLP file system handle.
 */
typedef HHFS  *PHHFS;

/*!
 * @typedef PFNHFSWRITER
 * @brief Callback that writes the body of one internal file.
 *
 * Invoked with the caller-supplied argument and a temporary stream.
 * The callback must write the body of the internal file to that
 * stream and return; the stream is released by the caller.
 */
typedef VOID (APIENTRY *PFNHFSWRITER)(PVOID pArg, FILE* f);

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Create an empty HLP file system.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Create implies that a new resource
 * is created as the result of a function call.
 *
 * @param[out] phHfs Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phHfs is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 *
 * @sa HfsDestroy
 */
APIRET APIENTRY HfsCreate(PHHFS phHfs);

/*!
 * @brief Destroy an HLP file system and release all its files.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hHfs File system handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY HfsDestroy(HHFS hHfs);

/* ==================================================================
 * Adding files
 * ================================================================== */

/*!
 * @brief Add an internal file from an in-memory buffer.
 *
 * The buffer is copied into the container. Duplicate file names are
 * rejected with ERROR_INVALID_PARAMETER.
 *
 * @param[in] hHfs    File system handle. Not NULLHANDLE.
 * @param[in] pszName Internal file name, e.g. "|SYSTEM". Not NULL.
 * @param[in] pvData  File body. Not NULL.
 * @param[in] ulSize  Size of the body in bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hHfs or @a pszName is NULL, or
 *                                  a file with that name already exists.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY HfsAddFile(HHFS hHfs, PCSZ pszName, PCVOID pvData, ULONG ulSize);

/*!
 * @brief Add an internal file by invoking a writer callback.
 *
 * The callback writes the body to a temporary stream, which is then
 * read into the container. The stream is released before return.
 *
 * @param[in] hHfs      File system handle. Not NULLHANDLE.
 * @param[in] pszName   Internal file name. Not NULL.
 * @param[in] pfnWriter Writer callback. Not NULL.
 * @param[in] pArg      Opaque argument passed to the callback.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 * @retval ERROR_READ_FAULT         Temporary stream is not readable.
 */
APIRET APIENTRY HfsAddFileFromWriter(HHFS hHfs, PCSZ pszName,
                                     PFNHFSWRITER pfnWriter, PVOID pArg);

/* ==================================================================
 * Query
 * ================================================================== */

/*!
 * @brief Query the number of internal files.
 *
 * @par Reference
 * OS/2 V2.0 Vol.4, section 12.2: @e Query implies that a system value
 * is returned.
 *
 * @param[in]  hHfs     File system handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY HfsQueryFileCount(HHFS hHfs, PULONG pulCount);

/*!
 * @brief Query an internal file name by index.
 *
 * Uses the size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied, NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW, *pulUsed is required size.
 *
 * @param[in]  hHfs    File system handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. May be NULL for size query.
 * @param[in]  ulSize  Size of @a pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hHfs is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      @a ulIndex out of range.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY HfsQueryFileName(HHFS hHfs, ULONG ulIndex,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Writing
 * ================================================================== */

/*!
 * @brief Write the container to a file.
 *
 * Assembles HELPHEADER, the directory B-tree (page size 0x0400,
 * BTR_FLAG_DIRECTORY), and all internal file bodies with their
 * FILEHEADER prefixes, then writes them to the named file.
 *
 * @param[in] hHfs        File system handle. Not NULLHANDLE.
 * @param[in] pszFileName Output file name. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL, or the
 *                                  container has no files.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 * @retval ERROR_FILE_NOT_FOUND     Output file cannot be created.
 */
APIRET APIENTRY HfsWrite(HHFS hHfs, PCSZ pszFileName);

#endif /* HFS_H */
