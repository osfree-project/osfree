/* reuse_comments.h - REUSE 3.3 comment style configuration
 * (C89 + Watcom extensions) */
#ifndef REUSE_COMMENTS_H
#define REUSE_COMMENTS_H

/**
 * @file reuse_comments.h
 * @brief Public interface of the REUSE comment-style configuration.
 *
 * Part of the REUSE library, sub-layer L2.0b (comments).
 * Umbrella header: reuse.h.
 *
 * Declares the comment-style configuration parsed from comments.toml,
 * the style resolution functions, and the low-level style operations
 * (strip a marker from one line, build a comment block from body
 * text, detect a shebang).
 */

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Handles
 * ================================================================== */

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
 * Config lifecycle
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

/* ==================================================================
 * Style resolution
 * ================================================================== */

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

/* ==================================================================
 * Style operations
 * ================================================================== */

/**
 * @brief Locate the body of one line after stripping the leading
 *        comment marker.
 *
 * Single-line styles are tried first (regexp, then literal), then
 * multi-line start / middle markers. When a multi-line start or
 * middle marker matches and the style defines a multi-line end
 * marker, a trailing end marker is trimmed from @p pszLine in place.
 * This makes single-line usages of multi-line markers such as
 * <!-- tag --> return a clean body.
 *
 * Stateless: no block state is carried between calls. For
 * multi-line styles without a multi_middle marker (html, julia,
 * bibtex), use ReuseInternalCommentBody (internal).
 *
 * @param[in]     hStyle     Borrowed style. Not NULLHANDLE.
 * @param[in,out] pszLine    Input line. Not NULL. May be modified in
 *                           place when a trailing end marker is
 *                           trimmed.
 * @param[out]    pulOffset  Byte offset in pszLine. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Marker found; *pulOffset is set.
 * @retval ERROR_FILE_NOT_FOUND     Line is not a comment.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseQueryCommentBody(HREUSECOMMENTSTYLE hStyle,
                                      PSZ pszLine,
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

#ifdef __cplusplus
}
#endif

#endif /* REUSE_COMMENTS_H */
