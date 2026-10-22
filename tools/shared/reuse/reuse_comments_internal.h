/* reuse_comments_internal.h - internal stateful comment-body reader
 * (C89) */
#ifndef REUSE_COMMENTS_INTERNAL_H
#define REUSE_COMMENTS_INTERNAL_H

#include "os2types.h"
#include "os2err.h"
#include "reuse.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_comments_internal.h
 * @brief Internal stateful comment-body reader.
 *
 * Companion to ReuseQueryCommentBody. The public function is
 * stateless (one line at a time); this variant carries a caller-
 * supplied block state so multi-line comment styles without a
 * multi_middle marker (html, julia, bibtex) can be parsed.
 *
 * Not part of the public REUSE API. Used only by reuse_tags.c.
 */

/**
 * @brief Locate the body of one line, honouring a block-comment
 *        state across calls.
 *
 * Same contract as ReuseQueryCommentBody, but @p pfInMulti carries
 * state between lines of the same file:
 *   - On entry, *pfInMulti == TRUE means "the previous line opened
 *     a multi-line block that has not been closed yet". The function
 *     then strips multi_middle (if any), trims trailing multi_end,
 *     and clears *pfInMulti when the end marker was found.
 *   - On entry, *pfInMulti == FALSE means "outside any block".
 *     Single-line markers, then multi_start, are tried. When
 *     multi_start matches and multi_end is not on the same line,
 *     *pfInMulti is set to TRUE.
 *
 * *pfInMulti is never modified on ERROR_FILE_NOT_FOUND.
 *
 * @param[in]     hStyle     Borrowed style. Not NULLHANDLE.
 * @param[in,out] pszLine    Input line. Not NULL. May be modified in
 *                           place when a trailing end marker is
 *                           trimmed.
 * @param[in,out] pfInMulti  Block state. Not NULL.
 * @param[out]    pulOffset  Byte offset in pszLine. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Marker found; *pulOffset is set.
 * @retval ERROR_FILE_NOT_FOUND     Line is not a comment in the
 *                                  current state.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 */
APIRET APIENTRY ReuseInternalCommentBody(HREUSECOMMENTSTYLE hStyle,
                                         PSZ pszLine,
                                         PBOOL pfInMulti,
                                         PULONG pulOffset);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_COMMENTS_INTERNAL_H */
