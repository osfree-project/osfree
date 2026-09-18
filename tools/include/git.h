/* git.h - Git repository helpers without invoking external tools
 * (C89 + Watcom extensions) */
#ifndef GIT_H
#define GIT_H

/**
 * @file git.h
 * @brief Public interface of the Git helper module.
 *
 * Provides access to the Git repository that contains a directory,
 * and to the .gitignore rules that apply to files inside it. No
 * external Git binary is invoked; the module locates .git entries
 * and parses .gitignore files directly.
 *
 * Supported .gitignore syntax:
 *   '*'  - any characters except '/'
 *   '**' - any characters including '/'
 *   '?'  - a single character except '/'
 *   leading '/'  - anchored to the directory of the .gitignore file
 *   trailing '/' - matches directories only
 *   '/' in the middle - anchored, unless the pattern starts with
 *                        the '**' wildcard followed by '/'
 *   leading '!'  - negation
 *   leading '#'  - comment
 *   escaped '#' and '!' - literal '#' and '!'
 *
 * Rules are applied top-to-bottom; the last matching rule wins.
 * Case sensitivity follows the host platform: case-sensitive on
 * Linux, case-insensitive on Windows.
 */

#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Error codes
 * ================================================================== */

/** @def GIT_NO_ERROR @brief Success. */
#define GIT_NO_ERROR            0
/** @def GIT_ERROR_INVALID_PARAM @brief Invalid parameter. */
#define GIT_ERROR_INVALID_PARAM 1
/** @def GIT_ERROR_OUT_OF_MEMORY @brief Memory allocation failure. */
#define GIT_ERROR_OUT_OF_MEMORY 2
/** @def GIT_ERROR_NOT_FOUND @brief Requested item not found. */
#define GIT_ERROR_NOT_FOUND     3

/* ==================================================================
 * Types
 * ================================================================== */

/**
 * @struct _GITIGNORERULE
 * @brief One rule from a .gitignore file.
 *
 * The stored pattern has already been stripped of a leading '!'
 * (captured in @c fNegate), a leading '/' (captured in @c fAnchored)
 * and a trailing '/' (captured in @c fDirOnly). The pattern never
 * carries the directory prefix that appears in the source file.
 */
typedef struct _GITIGNORERULE {
    char *pszPattern;   /**< Pattern without '!', leading '/',
                             and trailing '/'. */
    char *pszBaseRel;   /**< Directory containing the rule, relative
                             to the repository root. "" for the root. */
    int   fNegate;      /**< 1 if the rule starts with '!'. */
    int   fAnchored;    /**< 1 if the rule starts with '/' or contains
                             a '/' in the middle. */
    int   fDirOnly;     /**< 1 if the rule ends with '/'. */
} GITIGNORERULE, *PGITIGNORERULE;

/**
 * @struct _GITIGNORELIST
 * @brief A list of .gitignore rules collected from a directory tree.
 */
typedef struct _GITIGNORELIST {
    PGITIGNORERULE paItems;    /**< Array of rules. */
    ULONG          ulCount;    /**< Number of used entries. */
    ULONG          ulCapacity; /**< Allocated capacity. */
} GITIGNORELIST, *PGITIGNORELIST;

/* ==================================================================
 * Rule list lifecycle
 * ================================================================== */

/**
 * @brief Initialize a rule list.
 *
 * Sets all fields to zero. No memory is allocated.
 *
 * @param[in] pList  List. Not NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success.
 * @retval GIT_ERROR_INVALID_PARAM pList is NULL.
 */
APIRET APIENTRY GitIgnoreListInit(GITIGNORELIST *pList);

/**
 * @brief Release all memory owned by a rule list.
 *
 * Frees the pattern and base_rel strings of every rule, then the
 * backing array, and reinitializes the structure. Passing NULL is a
 * no-op.
 *
 * @param[in] pList  List. May be NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR Success. Also returned for NULL.
 */
APIRET APIENTRY GitIgnoreListFree(GITIGNORELIST *pList);

/* ==================================================================
 * Repository discovery
 * ================================================================== */

/**
 * @brief Find the root of the Git repository containing a directory.
 *
 * Walks up from @p pszStartDir looking for an entry named ".git"
 * (directory or file). Both '/' and '\\' are treated as path
 * separators.
 *
 * On success, @p *ppszRoot receives a malloc'd string owned by the
 * caller and must be freed with @c free. On any failure,
 * @p *ppszRoot is set to NULL.
 *
 * @param[in]  pszStartDir  Starting directory. Not NULL.
 * @param[out] ppszRoot     Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success. Root directory returned.
 * @retval GIT_ERROR_INVALID_PARAM pszStartDir or ppszRoot is NULL.
 * @retval GIT_ERROR_OUT_OF_MEMORY Memory allocation failure.
 * @retval GIT_ERROR_NOT_FOUND     No .git entry found.
 */
APIRET APIENTRY GitFindRepoRoot(PCSZ pszStartDir, PSZ *ppszRoot);

/**
 * @brief Check whether a directory is inside a Git repository.
 *
 * @param[in]  pszDir     Directory. Not NULL.
 * @param[out] pfIsRepo   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success.
 * @retval GIT_ERROR_INVALID_PARAM pszDir or pfIsRepo is NULL.
 */
APIRET APIENTRY GitIsRepo(PCSZ pszDir, PBOOL pfIsRepo);

/* ==================================================================
 * .gitignore collection
 * ================================================================== */

/**
 * @brief Collect .gitignore rules from a directory tree.
 *
 * Reads .gitignore from @p pszRepoRoot, then from every directory
 * along the path down to @p pszTargetDir (inclusive). Rules from
 * upper directories come first, so the "last match wins" rule of
 * .gitignore is preserved.
 *
 * If @p pszRepoRoot is NULL, only @p pszTargetDir / .gitignore is
 * read.
 *
 * @param[in]  pszRepoRoot  Repository root, or NULL.
 * @param[in]  pszTargetDir Target directory. Not NULL.
 * @param[out] pOut         Receiver. Not NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success (even if no .gitignore
 *                                 files were found).
 * @retval GIT_ERROR_INVALID_PARAM pszTargetDir or pOut is NULL.
 * @retval GIT_ERROR_OUT_OF_MEMORY Memory allocation failure.
 */
APIRET APIENTRY GitCollectGitignores(PCSZ pszRepoRoot, PCSZ pszTargetDir,
                                     GITIGNORELIST *pOut);

/* ==================================================================
 * Rule application
 * ================================================================== */

/**
 * @brief Check whether a path is ignored by the collected rules.
 *
 * @p pszRelPath is a path relative to the repository root (or to
 * the directory passed to @ref GitCollectGitignores when
 * @p pszRepoRoot was NULL). @p fIsDir selects the directory variant
 * of dir-only rules.
 *
 * Rules are evaluated in list order; the last matching rule decides
 * whether the path is ignored or not (a '!' rule cancels a previous
 * match).
 *
 * @param[in]  pRules      List of rules. Not NULL.
 * @param[in]  pszRelPath  Relative path. Not NULL.
 * @param[in]  fIsDir      TRUE_ for a directory, FALSE_ for a file.
 * @param[out] pfIgnored   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success.
 * @retval GIT_ERROR_INVALID_PARAM Any parameter is NULL.
 */
APIRET APIENTRY GitIsIgnored(const GITIGNORELIST *pRules, PCSZ pszRelPath,
                             BOOL fIsDir, PBOOL pfIgnored);

#ifdef __cplusplus
}
#endif

#endif /* GIT_H */
