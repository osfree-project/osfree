/* path.h - path manipulation helpers (C89 + Watcom extensions) */
#ifndef PATH_H
#define PATH_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file path.h
 * @brief Path manipulation helpers.
 *
 * Platform-independent path operations shared by modules that handle
 * file system paths. Both '/' and '\\' are recognised on input.
 *
 * This module supersedes the legacy all_pathmanipulation API. The
 * legacy module is kept unchanged for existing callers; new code
 * should use the Path* functions declared here.
 *
 * Functions never allocate memory on behalf of the caller. String
 * results are written into a caller-supplied buffer using the
 * size-query convention. PathGetBaseName returns a pointer into the
 * argument string.
 */

/* ==================================================================
 * Component extraction
 * ================================================================== */

/**
 * @brief Return a pointer to the base name inside a path.
 *
 * The returned pointer refers to data inside @p pszPath; the caller
 * must not free it and must keep @p pszPath alive.
 *
 * @param[in]  pszPath  Path. Not NULL.
 * @param[out] ppszBase Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or ppszBase is NULL.
 */
APIRET APIENTRY PathGetBaseName(PCSZ pszPath, PCSZ *ppszBase);

/**
 * @brief Copy the directory part of a path into a caller buffer.
 *
 * If the path has no separator, "." is returned. If the path begins
 * with a separator and has no other, that separator alone is
 * returned. If the path ends with a separator, the portion before it
 * is returned.
 *
 * Size-query convention:
 *   - pszDst == NULL, ulDstSize == 0: only *pulUsed is written.
 *   - ulDstSize large enough: value copied and NUL-terminated;
 *     *pulUsed is the length without NUL.
 *   - ulDstSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszPath   Path. Not NULL.
 * @param[out] pszDst    Output buffer. Not NULL unless size-query.
 * @param[in]  ulDstSize Size of pszDst in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or pszDst is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 */
APIRET APIENTRY PathGetDirName(PCSZ pszPath, PSZ pszDst,
                               ULONG ulDstSize, PULONG pulUsed);

/* ==================================================================
 * Construction
 * ================================================================== */

/**
 * @brief Join a directory and a name with the platform separator.
 *
 * A separator is inserted only when the directory does not already
 * end with '/' or '\\'. If @p pszDir is empty, the result is
 * @p pszName alone.
 *
 * Size-query convention as for PathGetDirName.
 *
 * @param[in]  pszDir    Directory. Not NULL. May be empty.
 * @param[in]  pszName   Entry name. Not NULL.
 * @param[out] pszDst    Output buffer. Not NULL unless size-query.
 * @param[in]  ulDstSize Size of pszDst in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszDir or pszName is NULL, or
 *                                  pszDst is NULL without
 *                                  size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 */
APIRET APIENTRY PathMakeJoin(PCSZ pszDir, PCSZ pszName, PSZ pszDst,
                             ULONG ulDstSize, PULONG pulUsed);

/* ==================================================================
 * Normalization
 * ================================================================== */

/**
 * @brief Remove trailing path separators in place.
 *
 * A bare drive letter like "C:" is left unchanged: appending a
 * separator would require writing past the end of the buffer. A
 * single leading separator alone is preserved.
 *
 * @param[in,out] pszPath  Path to trim. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL.
 */
APIRET APIENTRY PathRemoveTrailingSeparators(PSZ pszPath);

/**
 * @brief Normalize a path.
 *
 * Converts backslashes to forward slashes, removes "." components,
 * resolves ".." components, and collapses repeated separators. A
 * leading drive letter ("C:") is preserved.
 *
 * Size-query convention as for PathGetDirName.
 *
 * @param[in]  pszSrc    Input path. Not NULL.
 * @param[out] pszDst    Output buffer. Not NULL unless size-query.
 * @param[in]  ulDstSize Size of pszDst in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszDst is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY PathNormalize(PCSZ pszSrc, PSZ pszDst,
                              ULONG ulDstSize, PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* PATH_H */
