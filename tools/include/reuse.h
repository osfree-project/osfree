/* reuse.h - REUSE 3.3 project resolver, OS/2 API style
 * (C89 + Watcom extensions) */
#ifndef REUSE_H
#define REUSE_H

/**
 * @file reuse.h
 * @brief Public interface of the REUSE 3.3 project resolver.
 *
 * This is the only header that REUSE tools include. It declares
 * every handle type, every warning code, and every function that
 * a tool may call. Implementation details live in reuse_internal.h,
 * reuse_comments.h, reuse_tags.h, and are not part of this
 * interface.
 *
 * Implements the resolution algorithm of the REUSE Specification 3.3:
 *   - https://reuse.software/spec-3.3/
 *
 * Responsibilities:
 *   - Discover all REUSE.toml files from the project root to the target
 *     directory.
 *   - Read .reuse/dep5 if present (deprecated).
 *   - Optionally attach a comments.toml describing comment styles for
 *     in-file SPDX tags.
 *   - For a given file, read the adjacent .license sidecar (if any),
 *     the in-file SPDX comment tags (when a comments.toml is
 *     attached), and the matching REUSE.toml / dep5 annotations.
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
#include "spdx_tag.h"

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

/**
 * @typedef HREUSECOMMENTSCFG
 * @brief Handle to a parsed comments.toml.
 */
typedef HANDLE HREUSECOMMENTSCFG;

/**
 * @typedef HREUSECOMMENTSTYLE
 * @brief Borrowed handle to one comment style.
 */
typedef HANDLE HREUSECOMMENTSTYLE;

/**
 * @typedef HREUSETAGSREPORT
 * @brief Handle to an accumulated in-file tag warnings report.
 */
typedef HANDLE HREUSETAGSREPORT;

/**
 * @typedef PHREUSETAGSREPORT
 * @brief Pointer to a HREUSETAGSREPORT receiver.
 */
typedef HANDLE *PHREUSETAGSREPORT;

/* ==================================================================
 * Warning codes for the in-file tag report
 * ================================================================== */

/** @def REUSE_TAGS_DEPRECATED @brief A deprecated SPDX tag was found
 *  in the file. Severity: REUSE_SEV_WARNING. */
#define REUSE_TAGS_DEPRECATED       0xFF10

/** @def REUSE_TAGS_NON_ANNEX_FORM @brief A Clause 8/9 canonical tag
 *  form was used in a source file where only its Annex H spelling is
 *  valid per REUSE 3.3. Severity: REUSE_SEV_WARNING. */
#define REUSE_TAGS_NON_ANNEX_FORM   0xFF11

/** @def REUSE_TAGS_LONG_LINE @brief A tag value exceeded the internal
 *  buffer and was skipped. Severity: REUSE_SEV_WARNING. */
#define REUSE_TAGS_LONG_LINE        0xFF12

/* ==================================================================
 * Style info
 * ================================================================== */

/**
 * @struct _REUSECOMMENTSTYLEINFO
 * @brief Static properties of a comment style.
 */
typedef struct _REUSECOMMENTSTYLEINFO {
    PCSZ pszName;      /**< Configured style name.                    */
    BOOL fSingleLine;  /**< Has a single-line marker.                 */
    BOOL fMultiLine;   /**< Has a multi-line start and end marker.    */
    BOOL fSidecar;     /**< Neither; annotate via <file>.license.     */
} REUSECOMMENTSTYLEINFO, *PREUSECOMMENTSTYLEINFO;

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
 * comments.toml cannot process in-file SPDX tags, so ReuseOpenTree
 * returns REUSE_ERROR_NO_COMMENT_STYLE when pszCommentsConfig is
 * NULL.
 *
 * Parse errors in discovered files are recorded as REUSEERR entries
 * in the project handle; the offending source is skipped.
 *
 * @param[in]  pszDir             Target directory. Not NULL.
 * @param[in]  pszCommentsConfig  Path to comments.toml. Not NULL.
 * @param[out] phTree             Handle receiver. Not NULL. Set to
 *                                NULLHANDLE on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                  Success.
 * @retval ERROR_INVALID_PARAMETER   pszDir or phTree is NULL.
 * @retval REUSE_ERROR_NO_COMMENT_STYLE
 *                                   pszCommentsConfig is NULL.
 * @retval ERROR_OPEN_FAILED         Directory or comments.toml cannot
 *                                   be opened.
 * @retval ERROR_INVALID_DATA        comments.toml parse error.
 * @retval ERROR_NOT_ENOUGH_MEMORY   Memory allocation failure.
 *
 * @see ReuseCloseTree
 */
APIRET APIENTRY ReuseOpenTree(PCSZ pszDir,
                              PCSZ pszCommentsConfig,
                              HREUSETREE *phTree);

/**
 * @brief Close a project and release all associated memory.
 *
 * All HREUSETREEFILE handles and REUSEERR records obtained from this
 * project become invalid.
 *
 * @param[in] hTree  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 *
 * @warning Do not call ReuseCloseTree twice with the same handle.
 */
APIRET APIENTRY ReuseCloseTree(HREUSETREE hTree);

/* ==================================================================
 * Diagnostics
 * ================================================================== */

/**
 * @brief Number of REUSEERR records stored in the project.
 *
 * Only records produced during discovery of REUSE.toml and
 * .reuse/dep5.
 *
 * @param[in]  hTree     Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseTreeGetErrorCount(HREUSETREE hTree, PULONG pulCount);

/**
 * @brief Retrieve one diagnostic record by index.
 *
 * The returned REUSEERR is a snapshot; string fields point to data
 * owned by the project handle and remain valid until ReuseCloseTree.
 *
 * @param[in]  hTree    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY ReuseTreeGetError(HREUSETREE hTree, ULONG ulIndex,
                                  PREUSEERR pErr);

/* ==================================================================
 * Comment style resolution
 * ================================================================== */

/**
 * @brief Resolve the comment style for a file.
 *
 * @param[in]  hTree         Handle. Not NULLHANDLE.
 * @param[in]  pszFilePath   Path to the file. Not NULL.
 * @param[out] pfFound       Receiver for the found flag. Not NULL.
 * @param[out] phStyle       Borrowed style handle, or NULLHANDLE.
 *                           Not NULL. Do not close.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hTree is not recognized.
 */
APIRET APIENTRY ReuseQueryTreeCommentStyle(HREUSETREE hTree,
                                           PCSZ pszFilePath,
                                           PBOOL pfFound,
                                           HREUSECOMMENTSTYLE *phStyle);

/**
 * @brief Query the accumulated in-file tag warnings report.
 *
 * Returns a borrowed handle owned by the tree, valid until
 * ReuseCloseTree. Never NULLHANDLE for a successfully opened tree.
 *
 * @param[in]  hTree     Handle. Not NULLHANDLE.
 * @param[out] phReport  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hTree is not recognized.
 */
APIRET APIENTRY ReuseQueryTreeTagsReport(HREUSETREE hTree,
                                         HREUSETAGSREPORT *phReport);

/* ==================================================================
 * Tags report
 * ================================================================== */

/**
 * @brief Create an empty tags report.
 *
 * @param[out] phReport  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseCreateTagsReport(PHREUSETAGSREPORT phReport);

/**
 * @brief Close a tags report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseCloseTagsReport(HREUSETAGSREPORT hReport);

/**
 * @brief Number of records in a tags report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseQueryTagsReportCount(HREUSETAGSREPORT hReport,
                                          PULONG pulCount);

/**
 * @brief Retrieve one tags report record by index.
 *
 * @param[in]  hReport  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pErr is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 */
APIRET APIENTRY ReuseQueryTagsReport(HREUSETAGSREPORT hReport,
                                     ULONG ulIndex,
                                     PREUSEERR pErr);

/* ==================================================================
 * Comment configuration
 * ================================================================== */

/**
 * @brief Open and parse a comments.toml file.
 *
 * @param[in]  pszConfigPath  Path to comments.toml. Not NULL.
 * @param[out] phCfg          Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_INVALID_DATA       TOML parse error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseOpenComments(PCSZ pszConfigPath,
                                  HREUSECOMMENTSCFG *phCfg);

/**
 * @brief Close a comments configuration.
 *
 * @param[in] hCfg  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseCloseComments(HREUSECOMMENTSCFG hCfg);

/**
 * @brief Resolve the comment style for a file using a config handle.
 *
 * Matches [[map]].glob first by base name, then by normalized full
 * path. The order of [[map]] entries is the order of checking; the
 * first match wins.
 *
 * @param[in]  hCfg         Config. Not NULLHANDLE.
 * @param[in]  pszFilePath  Path to the file. Not NULL.
 * @param[out] pfFound      TRUE if a [[map]] matched. Not NULL.
 * @param[out] phStyle      Borrowed style handle on match. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hCfg is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure while
 *                                  normalizing the path.
 */
APIRET APIENTRY ReuseQueryCommentStyle(HREUSECOMMENTSCFG hCfg,
                                       PCSZ pszFilePath,
                                       PBOOL pfFound,
                                       HREUSECOMMENTSTYLE *phStyle);

/**
 * @brief Resolve a style by its configured name.
 *
 * @param[in]  hCfg     Config. Not NULLHANDLE.
 * @param[in]  pszName  Style name. Not NULL.
 * @param[out] pfFound  TRUE if the name is defined. Not NULL.
 * @param[out] phStyle  Borrowed style handle on match. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hCfg is not recognized.
 */
APIRET APIENTRY ReuseQueryCommentStyleByName(HREUSECOMMENTSCFG hCfg,
                                             PCSZ pszName,
                                             PBOOL pfFound,
                                             HREUSECOMMENTSTYLE *phStyle);

/**
 * @brief Query static properties of a comment style.
 *
 * @param[in]  hStyle  Borrowed style. Not NULLHANDLE.
 * @param[out] pInfo   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pInfo is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryCommentStyleInfo(HREUSECOMMENTSTYLE hStyle,
                                           PREUSECOMMENTSTYLEINFO pInfo);

/**
 * @brief Locate the body of one line after stripping its comment
 *        marker.
 *
 * @param[in]  hStyle     Borrowed style. Not NULLHANDLE.
 * @param[in]  pszLine    Input line. Not NULL.
 * @param[out] pulOffset  Byte offset in pszLine. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Marker found; *pulOffset is set.
 * @retval ERROR_FILE_NOT_FOUND     Line is not a comment.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryCommentBody(HREUSECOMMENTSTYLE hStyle,
                                      PCSZ pszLine,
                                      PULONG pulOffset);

/**
 * @brief Build a comment block for the given text.
 *
 * @param[in]  hStyle      Borrowed style. Not NULLHANDLE.
 * @param[in]  pszText     Text to comment. Not NULL.
 * @param[in]  fForceMulti TRUE to force multi-line output.
 * @param[out] pszBuf      Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize      Size of pszBuf in bytes.
 * @param[out] pulUsed     Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 * @retval ERROR_INVALID_DATA       Style cannot produce the block.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseQueryCommentBlock(HREUSECOMMENTSTYLE hStyle,
                                       PCSZ pszText, BOOL fForceMulti,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed);

/**
 * @brief Query whether the first line of a file is a recognized
 *        shebang.
 *
 * @param[in]  hStyle          Borrowed style. Not NULLHANDLE.
 * @param[in]  pszFirstLine    First line of the file. Not NULL.
 * @param[out] pfKeepFirstLine TRUE to keep line 1. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryShebang(HREUSECOMMENTSTYLE hStyle,
                                  PCSZ pszFirstLine,
                                  PBOOL pfKeepFirstLine);

/* ==================================================================
 * File tags
 * ================================================================== */

/**
 * @brief Read the file-level license and copyright of a file.
 *
 * @param[in]  pszPath       Path to the file. Not NULL.
 * @param[in]  hStyle        Comment style, or NULLHANDLE for a
 *                           sidecar file (raw tag-value, no comment
 *                           marker stripping).
 * @param[out] pszLicense    License buffer. Not NULL unless size-query.
 * @param[in]  ulLicSize     Size of pszLicense in bytes.
 * @param[out] pulLicUsed    Optional. Receives the used size.
 * @param[out] pszCopyright  Copyright buffer. Not NULL unless size-query.
 * @param[in]  ulCopSize     Size of pszCopyright in bytes.
 * @param[out] pulCopUsed    Optional. Receives the used size.
 * @param[in]  hReport       Warning report, or NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 At least one tag was found.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No licensing information found.
 * @retval ERROR_BUFFER_OVERFLOW    A buffer is too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Unmatched snippet delimiters.
 */
APIRET APIENTRY ReuseQueryFileTags(PCSZ pszPath,
                                   HREUSECOMMENTSTYLE hStyle,
                                   PSZ pszLicense, ULONG ulLicSize,
                                   PULONG pulLicUsed,
                                   PSZ pszCopyright, ULONG ulCopSize,
                                   PULONG pulCopUsed,
                                   HREUSETAGSREPORT hReport);

/**
 * @brief Query whether a file has any SPDX-License-Identifier tag.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style, or NULLHANDLE for a sidecar.
 * @param[out] pfHas    Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  pszPath or pfHas is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
APIRET APIENTRY ReuseQueryFileHasTag(PCSZ pszPath,
                                     HREUSECOMMENTSTYLE hStyle,
                                     PBOOL pfHas);

/**
 * @brief Read all SPDX snippets from a file.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style. Not NULLHANDLE.
 * @param[in]  hReport  Warning report, or NULLHANDLE.
 * @param[out] pOut     List receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_INVALID_PARAMETER  pszPath or pOut is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle or hReport is not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Unmatched or nested snippet
 *                                  delimiters.
 */
APIRET APIENTRY ReuseQueryFileSnippets(PCSZ pszPath,
                                       HREUSECOMMENTSTYLE hStyle,
                                       HREUSETAGSREPORT hReport,
                                       PSPDXSNIPPETLIST pOut);

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
 * @param[in]  hTree    Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path to the file. May be absolute or relative.
 * @param[out] phFile   Handle receiver. Not NULL.
 * @param[out] pErr     Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                  Success.
 * @retval ERROR_INVALID_PARAMETER   hTree, pszPath or phFile is NULL.
 * @retval ERROR_INVALID_HANDLE      Handle not recognized.
 * @retval ERROR_OPEN_FAILED         File cannot be opened.
 * @retval ERROR_READ_FAULT          Read error.
 * @retval REUSE_ERROR_NO_COMMENT_STYLE
 *                                   no [[map]] entry for the file.
 * @retval ERROR_NOT_ENOUGH_MEMORY   Memory allocation failure.
 */
APIRET APIENTRY ReuseOpenFile(HREUSETREE hTree, PCSZ pszPath,
                              HREUSETREEFILE *phFile,
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
APIRET APIENTRY ReuseCloseFile(HREUSETREEFILE hFile);

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
APIRET APIENTRY ReuseQueryFileLicense(HREUSETREEFILE hFile,
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
APIRET APIENTRY ReuseQueryFileCopyright(HREUSETREEFILE hFile,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed);

/**
 * @brief Retrieve the resolved SPDX-FileContributor list.
 *
 * Multiple contributors are joined with '\n'.
 *
 * @return ERROR_FILE_NOT_FOUND if no contributor was defined.
 */
APIRET APIENTRY ReuseQueryFileContributors(HREUSETREEFILE hFile,
                                           PSZ pszBuf, ULONG ulSize,
                                           PULONG pulUsed);

/** @brief Retrieve SPDX-PackageName. */
APIRET APIENTRY ReuseQueryFilePackageName(HREUSETREEFILE hFile,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed);

/** @brief Retrieve SPDX-PackageSupplier. */
APIRET APIENTRY ReuseQueryFilePackageSupplier(HREUSETREEFILE hFile,
                                              PSZ pszBuf, ULONG ulSize,
                                              PULONG pulUsed);

/** @brief Retrieve SPDX-PackageDownloadLocation. */
APIRET APIENTRY ReuseQueryFilePackageDownloadLocation(HREUSETREEFILE hFile,
                                                      PSZ pszBuf,
                                                      ULONG ulSize,
                                                      PULONG pulUsed);

/** @brief Retrieve SPDX-PackageComment. */
APIRET APIENTRY ReuseQueryFilePackageComment(HREUSETREEFILE hFile,
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
APIRET APIENTRY ReuseQueryFilePrecedence(HREUSETREEFILE hFile,
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
 * Replaces the former ReuseTreeFileGetHasReuse: a non-NONE value
 * means at least one source matched.
 *
 * @param[in]  hFile      Handle. Not NULLHANDLE.
 * @param[out] pulSource  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseQueryFileSource(HREUSETREEFILE hFile,
                                     PULONG pulSource);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_H */
