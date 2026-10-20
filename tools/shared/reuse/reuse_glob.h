/*!
 * @file reuse_glob.h
 *
 * @brief REUSE 3.3 glob pattern matching.
 *
 * Implements the globbing rules defined in REUSE Specification 3.3,
 * section "REUSE.toml", key "path" of an "[[annotations]]" table:
 *
 *   - '*' matches everything except forward slashes (path separators).
 *   - '**' matches everything including forward slashes.
 *   - A '**' immediately followed by a slash matches everything
 *     including forward slashes, and is equivalent to '**' alone.
 *   - A '***' or longer run of '*' is equivalent to '**'.
 *   - '\*' is a literal '*'.
 *   - '\\' is a literal backslash.
 *   - A backslash followed by any other character is that character,
 *     verbatim.
 *   - A trailing backslash is dropped.
 *
 * Matching is anchored: the entire path must match the entire pattern.
 * Matching is case-sensitive.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 */

#ifndef REUSE_GLOB_H
#define REUSE_GLOB_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Test whether a path matches a REUSE glob pattern.
 *
 * The path MUST be normalized to '/' before the call; a '\\' in the
 * path is a literal backslash, not a separator.
 *
 * @param[in]  pszPattern  Glob pattern. Not NULL.
 * @param[in]  pszPath     Normalized path. Not NULL.
 * @param[out] pfMatched   Receiver for the match result. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 The check completed; *pfMatched
 *                                  reports the result.
 * @retval ERROR_INVALID_PARAMETER  pszPattern, pszPath or pfMatched
 *                                  is NULL.
 */
APIRET APIENTRY ReuseQueryGlobMatch(PCSZ pszPattern, PCSZ pszPath,
                                    PBOOL pfMatched);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_GLOB_H */
