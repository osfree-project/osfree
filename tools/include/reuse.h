/* reuse.h - REUSE 3.3 project resolver, OS/2 API style
 * (C89 + Watcom extensions) */
#ifndef REUSE_H
#define REUSE_H

/**
 * @file reuse.h
 * @brief Public umbrella of the REUSE library.
 *
 * Includes reuse_comments.h (L2.0b) and reuse_tags.h (L2.1) for
 * backwards compatibility, and declares the L2.2 resolver API.
 */

#include "os2types.h"
#include "reusedef.h"
#include "reuseerr.h"
#include "reuse_comments.h"
#include "reuse_tags.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Handles
 * ================================================================== */

/**
 * @typedef HREUSEPROJECT
 * @brief Handle to an opened REUSE project.
 */
typedef HANDLE HREUSEPROJECT;

/**
 * @typedef HREUSEPROJECTFILE
 * @brief Handle to a per-file resolution result.
 */
typedef HANDLE HREUSEPROJECTFILE;

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
 * pszCommentsConfig is required. A project opened without a
 * comments.toml cannot process in-file SPDX tags, so ReuseOpenProject
 * returns REUSE_ERROR_NO_COMMENT_STYLE when pszCommentsConfig is
 * NULL.
 *
 * Parse errors in discovered files are recorded as REUSEERR entries
 * in the project handle; the offending source is skipped.
 *
 * @param[in]  pszDir             Target directory. Not NULL.
 * @param[in]  pszCommentsConfig  Path to comments.toml. Not NULL.
 * @param[out] phProject          Handle receiver. Not NULL. Set to
 *                                NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                  Success.
 * @retval ERROR_INVALID_PARAMETER   pszDir or phProject is NULL.
 * @retval REUSE_ERROR_NO_COMMENT_STYLE
 *                                   pszCommentsConfig is NULL.
 * @retval ERROR_OPEN_FAILED         Directory or comments.toml cannot
 *                                   be opened.
 * @retval ERROR_INVALID_DATA        comments.toml parse error.
 * @retval ERROR_NOT_ENOUGH_MEMORY   Memory allocation failure.
 *
 * @see ReuseProjectGetError, ReuseCloseProject
 */
APIRET APIENTRY ReuseOpenProject(PCSZ pszDir,
                                 PCSZ pszCommentsConfig,
                                 HREUSEPROJECT *phProject);

/**
 * @brief Close a project and release all associated memory.
 *
 * All HREUSEPROJECTFILE handles and REUSEERR records obtained from
 * this project become invalid.
 *
 * @param[in] hProject  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 *
 * @warning Do not call ReuseCloseProject twice with the same handle.
 */
APIRET APIENTRY ReuseCloseProject(HREUSEPROJECT hProject);

/* ==================================================================
 * Project metadata
 * ================================================================== */

/**
 * @brief Number of records stored in the project.
 *
 * Counts all severity levels (ERROR, WARNING, INFO). Use
 * ReuseProjectGetError by index to inspect each record's severity.
 *
 * @param[in]  hProject  Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseProjectGetRecordCount(HREUSEPROJECT hProject,
                                           PULONG pulCount);

/**
 * @brief Retrieve one diagnostic record by index.
 *
 * The returned REUSEERR is a snapshot; string fields point to data
 * owned by the project handle and remain valid until
 * ReuseCloseProject.
 *
 * @param[in]  hProject  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex   Zero-based index.
 * @param[out] pErr      Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY ReuseProjectGetError(HREUSEPROJECT hProject,
                                     ULONG ulIndex,
                                     PREUSEERR pErr);

/**
 * @brief Retrieve the project root.
 *
 * Returns the Git repository root when the target directory is
 * inside a Git repository, otherwise the target directory itself.
 *
 * Size-query convention: pszBuf == NULL && ulSize == 0 returns
 * required size in *pulUsed.
 *
 * @param[in]  hProject  Handle. Not NULLHANDLE.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hProject is NULLHANDLE, or pszBuf
 *                                  is NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No project root recorded.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseQueryProjectRoot(HREUSEPROJECT hProject,
                                      PSZ pszBuf, ULONG ulSize,
                                      PULONG pulUsed);

/* ==================================================================
 * Comment style resolution
 * ================================================================== */

/**
 * @brief Resolve the comment style for a file.
 *
 * @param[in]  hProject      Handle. Not NULLHANDLE.
 * @param[in]  pszFilePath   Path to the file. Not NULL.
 * @param[out] pfFound       Receiver for the found flag. Not NULL.
 * @param[out] phStyle       Borrowed style handle, or NULLHANDLE.
 *                           Not NULL. Do not close.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hProject is not recognized.
 */
APIRET APIENTRY ReuseQueryProjectCommentStyle(HREUSEPROJECT hProject,
                                              PCSZ pszFilePath,
                                              PBOOL pfFound,
                                              HREUSECOMMENTSTYLE *phStyle);

/* ==================================================================
 * Tags report (borrowed)
 * ================================================================== */

/**
 * @brief Query the accumulated in-file tag warnings report.
 *
 * Returns a BORROWED handle owned by the project, valid until
 * ReuseCloseProject. Do not call ReuseCloseTagsReport on it.
 *
 * @param[in]  hProject  Handle. Not NULLHANDLE.
 * @param[out] phReport  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hProject is not recognized.
 */
APIRET APIENTRY ReuseQueryProjectTagsReport(HREUSEPROJECT hProject,
                                            HREUSETAGSREPORT *phReport);

/* ==================================================================
 * Per-file resolution
 * ================================================================== */

/**
 * @brief Open a resolved file record.
 *
 * Reads the adjacent <file>.license sidecar (if present), the
 * in-file SPDX tags (using the comment style resolved from the
 * attached comments.toml), and combines them with the matching
 * REUSE.toml / DEP5 annotations according to the precedence rules
 * of REUSE 3.3 §4.1.2, §4.1.3.
 *
 * Returns REUSE_ERROR_NO_COMMENT_STYLE when the attached
 * comments.toml has no [[map]] entry matching pszPath. In that
 * case the file cannot be processed.
 *
 * A successful return does not imply the file had any licensing
 * information. Use ReuseQueryFileSource to distinguish "no sources
 * matched" from "sources matched but were empty".
 *
 * @param[in]  hProject  Handle. Not NULLHANDLE.
 * @param[in]  pszPath   Path to the file. May be absolute or
 *                       relative. Not NULL.
 * @param[out] phFile    Handle receiver. Not NULL.
 * @param[out] pErr      Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                  Success.
 * @retval ERROR_INVALID_PARAMETER   hProject, pszPath or phFile is NULL.
 * @retval ERROR_INVALID_HANDLE      Handle not recognized.
 * @retval ERROR_OPEN_FAILED         File cannot be opened.
 * @retval ERROR_READ_FAULT          Read error.
 * @retval REUSE_ERROR_NO_COMMENT_STYLE
 *                                   no [[map]] entry for the file.
 * @retval ERROR_NOT_ENOUGH_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY ReuseOpenFile(HREUSEPROJECT hProject, PCSZ pszPath,
                              HREUSEPROJECTFILE *phFile,
                              PREUSEERR pErr);

/**
 * @brief Release a file record.
 *
 * @param[in] hFile  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 */
APIRET APIENTRY ReuseCloseFile(HREUSEPROJECTFILE hFile);

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
APIRET APIENTRY ReuseQueryFileLicense(HREUSEPROJECTFILE hFile,
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
APIRET APIENTRY ReuseQueryFileCopyright(HREUSEPROJECTFILE hFile,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed);

/**
 * @brief Retrieve the resolved SPDX-FileContributor list.
 *
 * Multiple contributors are joined with '\n'.
 *
 * @return ERROR_FILE_NOT_FOUND if no contributor was defined.
 */
APIRET APIENTRY ReuseQueryFileContributors(HREUSEPROJECTFILE hFile,
                                           PSZ pszBuf, ULONG ulSize,
                                           PULONG pulUsed);

/** @brief Retrieve SPDX-PackageName. */
APIRET APIENTRY ReuseQueryFilePackageName(HREUSEPROJECTFILE hFile,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed);

/** @brief Retrieve SPDX-PackageSupplier. */
APIRET APIENTRY ReuseQueryFilePackageSupplier(HREUSEPROJECTFILE hFile,
                                              PSZ pszBuf, ULONG ulSize,
                                              PULONG pulUsed);

/** @brief Retrieve SPDX-PackageDownloadLocation. */
APIRET APIENTRY ReuseQueryFilePackageDownloadLocation(HREUSEPROJECTFILE hFile,
                                                      PSZ pszBuf,
                                                      ULONG ulSize,
                                                      PULONG pulUsed);

/** @brief Retrieve SPDX-PackageComment. */
APIRET APIENTRY ReuseQueryFilePackageComment(HREUSEPROJECTFILE hFile,
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
APIRET APIENTRY ReuseQueryFilePrecedence(HREUSEPROJECTFILE hFile,
                                         PULONG pulPrecedence);

/**
 * @brief Retrieve the source of the resolved record.
 *
 * Returns one of REUSE_LICENSE_SRC_* (see reuse_lic.h):
 *   REUSE_LICENSE_SRC_NONE     — no source matched the file;
 *   REUSE_LICENSE_SRC_TAG      — in-file SPDX tag;
 *   REUSE_LICENSE_SRC_SIDECAR  — adjacent .license sidecar;
 *   REUSE_LICENSE_SRC_REUSE    — REUSE.toml or DEP5 annotation.
 *
 * A non-NONE value means at least one source matched.
 *
 * @param[in]  hFile      Handle. Not NULLHANDLE.
 * @param[out] pulSource  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseQueryFileSource(HREUSEPROJECTFILE hFile,
                                     PULONG pulSource);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_H */
