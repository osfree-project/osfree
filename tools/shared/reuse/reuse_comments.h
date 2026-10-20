/* reuse_comments.h - REUSE 3.3 comment style configuration (C89) */
#ifndef REUSE_COMMENTS_H
#define REUSE_COMMENTS_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_comments.h
 * @brief REUSE 3.3 comment style configuration.
 *
 * Loads comments.toml, which declares reusable comment styles and
 * maps them to file globs. The configuration is exposed through two
 * opaque handles:
 *
 *   - HREUSECOMMENTSCFG is created by ReuseOpenComments and released
 *     by ReuseCloseComments.
 *   - HREUSECOMMENTSTYLE is a borrowed handle obtained from a query
 *     function. It is valid for as long as its parent config is open,
 *     and it must not be closed by the caller.
 *
 * Both handles are checked against an internal magic value, so
 * passing a style handle to ReuseCloseComments or a config handle to
 * a style query is detected at runtime and reported as
 * ERROR_INVALID_HANDLE.
 *
 * A style with neither single-line nor multi-line comment markers is
 * treated as uncommentable: annotating such a file requires a sidecar
 * <file>.license.
 *
 * Paths in [[map]].glob use the same globbing rules as REUSE.toml
 * (see ReuseQueryGlobMatch).
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 */

/* ==================================================================
 * Handles and info structures
 * ================================================================== */

/**
 * @brief Opaque configuration handle created by ReuseOpenComments.
 */
typedef HANDLE HREUSECOMMENTSCFG;

/**
 * @brief Borrowed style handle obtained from a query.
 *
 * Valid for as long as its parent HREUSECOMMENTSCFG is open.
 */
typedef HANDLE HREUSECOMMENTSTYLE;

/**
 * @struct _REUSECOMMENTSTYLEINFO
 * @brief Static properties of a comment style.
 *
 * @c pszName points into the owning configuration; the caller must not
 * free it. It is valid for as long as the parent
 * HREUSECOMMENTSCFG is open.
 */
typedef struct _REUSECOMMENTSTYLEINFO {
    PCSZ pszName;      /**< Configured style name.                    */
    BOOL fSingleLine;  /**< Has a single-line marker.                 */
    BOOL fMultiLine;   /**< Has a multi-line start and end marker.    */
    BOOL fSidecar;     /**< Neither; annotate via <file>.license.     */
} REUSECOMMENTSTYLEINFO, *PREUSECOMMENTSTYLEINFO;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Open and parse a comments.toml file.
 *
 * @param[in]  pszConfigPath  Path to comments.toml. Not NULL.
 * @param[out] phCfg          Receiver. Not NULL. Set to NULLHANDLE on
 *                            error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_INVALID_DATA       TOML parse error, missing required
 *                                  key, duplicate style name, a
 *                                  [[map]].style name that does not
 *                                  match any [[style]].name, or a
 *                                  missing [[map]].glob.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseOpenComments(PCSZ pszConfigPath,
                                  HREUSECOMMENTSCFG *phCfg);

/**
 * @brief Close a config and release all styles.
 *
 * After this call all HREUSECOMMENTSTYLE handles borrowed from this
 * config become invalid.
 *
 * @param[in] hCfg  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseCloseComments(HREUSECOMMENTSCFG hCfg);

/* ==================================================================
 * Style resolution
 * ================================================================== */

/**
 * @brief Resolve the comment style for a file by [[map]] glob.
 *
 * Each [[map]].glob is matched against the base name first, then
 * against the normalized full path. The first matching [[map]] entry
 * in file order wins.
 *
 * @param[in]  hCfg         Config. Not NULLHANDLE.
 * @param[in]  pszFilePath  Path to the file. Not NULL.
 * @param[out] pfFound      TRUE if a [[map]] matched. Not NULL.
 * @param[out] phStyle      Borrowed style handle on match, NULLHANDLE
 *                          otherwise. Not NULL. Do not close.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hCfg is not recognized.
 */
APIRET APIENTRY ReuseQueryCommentStyle(HREUSECOMMENTSCFG hCfg,
                                       PCSZ pszFilePath,
                                       PBOOL pfFound,
                                       HREUSECOMMENTSTYLE *phStyle);

/**
 * @brief Resolve a style by its configured name.
 *
 * Used for --comment-style overrides and for diagnostics.
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

/* ==================================================================
 * Style queries
 * ================================================================== */

/**
 * @brief Query static properties of a style.
 *
 * @param[in]  hStyle  Borrowed style. Not NULLHANDLE.
 * @param[out] pInfo   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pInfo is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is NULLHANDLE or not a style
 *                                  handle.
 */
APIRET APIENTRY ReuseQueryCommentStyleInfo(HREUSECOMMENTSTYLE hStyle,
                                           PREUSECOMMENTSTYLEINFO pInfo);

/* ==================================================================
 * Style operations
 * ================================================================== */

/**
 * @brief Locate the body of a single line after stripping the leading
 *        comment marker.
 *
 * Tried in order: single-line marker + its indent; multi-line start
 * marker (when the line is the start of a multi-line block); multi-
 * line middle marker + its indent. Leading spaces and tabs are
 * always skipped first.
 *
 * Trailing markers (multi-line end delimiter and line-comment marker
 * slash-slash) are NOT stripped; the caller is responsible for that.
 *
 * @param[in]  hStyle     Borrowed style. Not NULLHANDLE.
 * @param[in]  pszLine    Input line. Not NULL.
 * @param[out] pulOffset  Byte offset in pszLine where the body starts.
 *                        Not NULL. Written only on NO_ERROR.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Marker found; *pulOffset is set.
 * @retval ERROR_FILE_NOT_FOUND     Line does not start with any
 *                                  comment marker for this style.
 * @retval ERROR_INVALID_PARAMETER  pszLine or pulOffset is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is NULLHANDLE or not a style
 *                                  handle.
 */
APIRET APIENTRY ReuseQueryCommentBody(HREUSECOMMENTSTYLE hStyle,
                                      PCSZ pszLine,
                                      PULONG pulOffset);

/**
 * @brief Build the SPDX comment block for the given text.
 *
 * Single-line output is preferred unless fForceMulti is TRUE or the
 * style has no single-line marker. When the style has only a multi-
 * line form (e.g. the "c" style), multi-line is used unconditionally.
 *
 * The produced string ends with a single newline after the final line
 * and is NUL-terminated. Prefix selection does not depend on
 * shebangs; positioning is a separate step (see ReuseQueryShebang).
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written; the
 *     required size including the trailing NUL.
 *   - ulSize large enough: block copied and NUL-terminated;
 *     *pulUsed is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hStyle      Borrowed style. Not NULLHANDLE.
 * @param[in]  pszText     Text to comment, one line per SPDX tag.
 *                         Not NULL.
 * @param[in]  fForceMulti TRUE to force multi-line output.
 * @param[out] pszBuf      Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize      Size of pszBuf in bytes.
 * @param[out] pulUsed     Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszText is NULL, or the size-query
 *                                  convention is violated.
 * @retval ERROR_INVALID_HANDLE     hStyle is NULLHANDLE or not a style
 *                                  handle.
 * @retval ERROR_INVALID_DATA       Style has no single-line marker and
 *                                  either fForceMulti is TRUE or no
 *                                  multi-line markers are configured.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseQueryCommentBlock(HREUSECOMMENTSTYLE hStyle,
                                       PCSZ pszText, BOOL fForceMulti,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed);

/**
 * @brief Query whether the first line of a file is a shebang that
 *        this style recognizes.
 *
 * The comparison is a case-insensitive prefix match against each
 * configured shebang, after leading spaces and tabs are skipped.
 *
 * When the style has no shebangs, *pfKeepFirstLine is always FALSE
 * (the comment block goes on line 1).
 *
 * @param[in]  hStyle          Borrowed style. Not NULLHANDLE.
 * @param[in]  pszFirstLine    First line of the file. Not NULL.
 * @param[out] pfKeepFirstLine TRUE to keep line 1 and put the comment
 *                             block on line 2. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  pszFirstLine or pfKeepFirstLine is
 *                                  NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is NULLHANDLE or not a style
 *                                  handle.
 */
APIRET APIENTRY ReuseQueryShebang(HREUSECOMMENTSTYLE hStyle,
                                  PCSZ pszFirstLine,
                                  PBOOL pfKeepFirstLine);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_COMMENTS_H */
