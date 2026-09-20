/* reuse.h - REUSE 3.3 project resolver, OS/2 API style
 * (C89 + Watcom extensions) */
#ifndef REUSE_H
#define REUSE_H

/**
 * @file reuse.h
 * @brief Public interface of the REUSE 3.3 project resolver.
 *
 * Implements the resolution algorithm of the REUSE Specification 3.3:
 *   - https://reuse.software/spec-3.3/
 *
 * Responsibilities:
 *   - Discover all REUSE.toml files from the project root to the target
 *     directory.
 *   - Read .reuse/dep5 if present (deprecated).
 *   - For a given file, read the adjacent .license sidecar (if any) and
 *     the SPDX comment tags inside the file.
 *   - Apply precedence (override > aggregate > closest) with depth and
 *     order-in-file as tie-breakers.
 *   - Aggregate license expressions and copyright notices as required
 *     by the winning precedence.
 *
 * REUSE.toml parsing itself lives in reuse_toml.h (prefix Reuse*);
 * DEP5 parsing lives in dep5.h. This module combines their results.
 */

#include "os2types.h"
#include "reusedef.h"
#include "reuseerr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Handles
 * ================================================================== */

/**
 * @typedef HREUSETREE
 * @brief Handle to an opened REUSE project.
 */
typedef HANDLE HREUSETREE;

/**
 * @typedef HREUSETREEFILE
 * @brief Handle to a per-file resolution result.
 */
typedef HANDLE HREUSETREEFILE;

/* ==================================================================
 * Project lifecycle
 * ================================================================== */

/**
 * @brief Open a REUSE project rooted at a directory.
 *
 * Scans the Git repository that contains pszDir (if any) and collects
 * every REUSE.toml file from the repository root down to pszDir, in
 * increasing depth order. Also reads .reuse/dep5 at the repository
 * root, if present.
 *
 * Parse errors in discovered files are recorded as REUSEERR entries
 * in the project handle; the offending source is skipped.
 *
 * @param[in]  pszDir  Target directory. Not NULL.
 * @param[out] phDoc   Handle receiver. Not NULL. Set to NULLHANDLE on
 *                     error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (possibly with recorded
 *                                  diagnostics).
 * @retval ERROR_INVALID_PARAMETER  pszDir or phDoc is NULL.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 *
 * @see ReuseTreeGetError, ReuseTreeClose
 */
APIRET APIENTRY ReuseTreeOpen(PCSZ pszDir, HREUSETREE *phDoc);

/**
 * @brief Close a project and release all associated memory.
 *
 * All HREUSETREEFILE handles and REUSEERR records obtained from this
 * project become invalid.
 *
 * @param[in] hDoc  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle not recognized.
 *
 * @warning Do not call ReuseTreeClose twice with the same handle.
 */
APIRET APIENTRY ReuseTreeClose(HREUSETREE hDoc);

/* ==================================================================
 * Diagnostics
 * ================================================================== */

/**
 * @brief Number of REUSEERR records stored in the project.
 *
 * Errors, warnings and informational records are counted together.
 * Use ReuseTreeGetError to inspect severity.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseTreeGetErrorCount(HREUSETREE hDoc, PULONG pulCount);

/**
 * @brief Retrieve one diagnostic record by index.
 *
 * The returned REUSEERR is a snapshot; string fields point to data
 * owned by the project handle and remain valid until ReuseTreeClose.
 *
 * @param[in]  hDoc    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index, < ReuseTreeGetErrorCount.
 * @param[out] pErr    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY ReuseTreeGetError(HREUSETREE hDoc, ULONG ulIndex,
                                  PREUSEERR pErr);

/* ==================================================================
 * Per-file resolution
 * ================================================================== */

/**
 * @brief Resolve licensing information for one file.
 *
 * Algorithm (REUSE 3.3 §4.1.2, §4.1.3):
 *   1. Read the adjacent <file>.license sidecar, if present.
 *   2. Read SPDX tags inside the file (outside REUSE-IgnoreStart/End
 *      regions; snippet bodies are not treated as file-level tags).
 *   3. Apply precedence across all discovered REUSE.toml files that
 *      match the file, using sidecar/tag data as the in-file source.
 *   4. Aggregate results as prescribed by the winning precedence.
 *
 * A successful resolution does not imply the file had any licensing
 * information. Use ReuseTreeFileGetHasReuse to distinguish "no sources
 * matched" from "sources matched but were empty".
 *
 * @param[in]  hDoc    Handle. Not NULLHANDLE.
 * @param[in]  pszPath Path to the file. May be absolute or relative.
 * @param[out] phFile  Handle receiver. Not NULL. Set to NULLHANDLE on
 *                     error.
 * @param[out] pErr    Optional. May be NULL. If not NULL and a
 *                     file-specific diagnostic occurred, the first
 *                     such record is stored here.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc, pszPath or phFile is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval REUSE_ERROR_SYNTAX       Sidecar contains a syntax error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 */
APIRET APIENTRY ReuseTreeResolveFile(HREUSETREE hDoc, PCSZ pszPath,
                                     HREUSETREEFILE *phFile,
                                     PREUSEERR pErr);

/**
 * @brief Release a resolution handle.
 *
 * @param[in] hFile  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle not recognized.
 */
APIRET APIENTRY ReuseTreeFileClose(HREUSETREEFILE hFile);

/* ==================================================================
 * Resolution field accessors
 *
 * Size-query convention for all string getters:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed (size incl. NUL) is
 *     written, no buffer touched.
 *   - ulSize large enough: value copied and NUL-terminated; *pulUsed
 *     is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 * ================================================================== */

/**
 * @brief Retrieve the resolved SPDX license expression.
 *
 * @return ERROR_FILE_NOT_FOUND if the winning sources define no
 *         license.
 */
APIRET APIENTRY ReuseTreeFileGetLicense(HREUSETREEFILE hFile,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed);

/**
 * @brief Retrieve the resolved copyright text.
 *
 * Multiple notices are joined with '\n'.
 *
 * @return ERROR_FILE_NOT_FOUND if the winning sources define no
 *         copyright.
 */
APIRET APIENTRY ReuseTreeFileGetCopyright(HREUSETREEFILE hFile,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed);

/**
 * @brief Retrieve the resolved SPDX-FileContributor list.
 *
 * Multiple contributors are joined with '\n'.
 *
 * @return ERROR_FILE_NOT_FOUND if no contributor was defined.
 */
APIRET APIENTRY ReuseTreeFileGetContributors(HREUSETREEFILE hFile,
                                             PSZ pszBuf, ULONG ulSize,
                                             PULONG pulUsed);

/** @brief Retrieve SPDX-PackageName. */
APIRET APIENTRY ReuseTreeFileGetPackageName(HREUSETREEFILE hFile,
                                            PSZ pszBuf, ULONG ulSize,
                                            PULONG pulUsed);

/** @brief Retrieve SPDX-PackageSupplier. */
APIRET APIENTRY ReuseTreeFileGetPackageSupplier(HREUSETREEFILE hFile,
                                                PSZ pszBuf, ULONG ulSize,
                                                PULONG pulUsed);

/** @brief Retrieve SPDX-PackageDownloadLocation. */
APIRET APIENTRY ReuseTreeFileGetPackageDownloadLocation(HREUSETREEFILE hFile,
                                                        PSZ pszBuf,
                                                        ULONG ulSize,
                                                        PULONG pulUsed);

/** @brief Retrieve SPDX-PackageComment. */
APIRET APIENTRY ReuseTreeFileGetPackageComment(HREUSETREEFILE hFile,
                                               PSZ pszBuf, ULONG ulSize,
                                               PULONG pulUsed);

/* ==================================================================
 * Resolution metadata
 * ================================================================== */

/**
 * @brief Retrieve the precedence of the winning source.
 *
 * @param[in]  hFile          Handle. Not NULLHANDLE.
 * @param[out] pulPrecedence  Receiver: one of REUSE_PRECEDENCE_*.
 *                            Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseTreeFileGetPrecedence(HREUSETREEFILE hFile,
                                           PULONG pulPrecedence);

/**
 * @brief Whether any REUSE.toml, DEP5, sidecar or tag matched the file.
 *
 * @param[in]  hFile      Handle. Not NULLHANDLE.
 * @param[out] pfHasReuse Receiver: TRUE_ if at least one source
 *                        matched; FALSE_ otherwise. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseTreeFileGetHasReuse(HREUSETREEFILE hFile,
                                         PBOOL pfHasReuse);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_H */