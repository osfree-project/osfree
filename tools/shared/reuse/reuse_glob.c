/*!
 * @file reuse_glob.c
 *
 * @brief REUSE 3.3 glob pattern matching.
 *
 * See reuse_glob.h for the specification and for the reference to
 * the normative text.
 */

#include "reuse_glob.h"

/*!
 * @brief Recursive worker behind ReuseQueryGlobMatch.
 *
 * Walks the pattern one element at a time:
 *   - literal (including the result of an escape) must match exactly;
 *   - '*' tries every prefix of the path that does not cross a '/';
 *   - '**' (with any number of extra '*') tries every suffix of the
 *     path, including suffixes that cross '/'.
 *
 * Both arguments are non-NULL; the caller performs the NULL check
 * once, so the recursive levels stay free of it.
 *
 * @param[in] pszPat   Remaining pattern. Not NULL.
 * @param[in] pszPath  Remaining path. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   The remaining path matches the remaining pattern.
 * @retval FALSE  Otherwise.
 */
static BOOL glob_match(PCSZ pszPat, PCSZ pszPath)
{
    while (*pszPat != '\0') {
        if (*pszPat == '\\') {
            pszPat++;
            if (*pszPat == '\0') {
                /* Trailing backslash: dropped. */
                break;
            }
            if (*pszPath != *pszPat) return FALSE;
            pszPat++;
            pszPath++;
        } else if (*pszPat == '*') {
            if (pszPat[1] == '*') {
                /* '**' (or a longer run): any sequence, including '/'. */
                while (*pszPat == '*') pszPat++;
                if (*pszPat == '/') pszPat++;
                for (;;) {
                    if (glob_match(pszPat, pszPath)) return TRUE;
                    if (*pszPath == '\0') return FALSE;
                    pszPath++;
                }
            } else {
                /* '*': any sequence without '/'. */
                pszPat++;
                for (;;) {
                    if (glob_match(pszPat, pszPath)) return TRUE;
                    if (*pszPath == '\0' || *pszPath == '/')
                        return FALSE;
                    pszPath++;
                }
            }
        } else {
            if (*pszPath != *pszPat) return FALSE;
            pszPat++;
            pszPath++;
        }
    }
    return (*pszPath == '\0') ? TRUE : FALSE;
}

/*!
 * @brief Test whether a path matches a REUSE glob pattern.
 *
 * @param[in]  pszPattern  Glob pattern. Not NULL.
 * @param[in]  pszPath     Normalized path. Not NULL.
 * @param[out] pfMatched   Receiver for the match result. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 The check completed.
 * @retval ERROR_INVALID_PARAMETER  Any argument is NULL.
 */
APIRET APIENTRY ReuseQueryGlobMatch(PCSZ pszPattern, PCSZ pszPath,
                                    PBOOL pfMatched)
{
    if (!pszPattern || !pszPath || !pfMatched)
        return ERROR_INVALID_PARAMETER;
    *pfMatched = glob_match(pszPattern, pszPath);
    return NO_ERROR;
}
