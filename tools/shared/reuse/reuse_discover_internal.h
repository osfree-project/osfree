/* reuse_discover_internal.h - internal state of reuse_discover.c
 * (C89) */
#ifndef REUSE_DISCOVER_INTERNAL_H
#define REUSE_DISCOVER_INTERNAL_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "git.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_discover_internal.h
 * @brief Internal walk options for reuse_discover.c.
 *
 * Not part of the public API. The structure is filled by
 * discover_walk in reuse_discover.c; no external caller sees it.
 */

/**
 * @struct _REUSEDISCOVEROPTIONS
 * @brief Internal walk options.
 *
 * All skip flags are hard-wired to TRUE by discover_walk, matching
 * the REUSE 3.3 definition of Covered Files. The two gitignore
 * fields are set when the target directory is inside a Git
 * repository.
 */
typedef struct _REUSEDISCOVEROPTIONS {
    BOOL  fRecursive;             /**< Recurse into subdirectories. */
    BOOL  fSkipHidden;            /**< Skip names starting with '.'.*/
    BOOL  fSkipVcsDirs;           /**< Skip .git/.svn/.hg/.bzr.     */
    BOOL  fSkipLicensesDir;       /**< Skip LICENSES/.              */
    BOOL  fSkipReuseDir;          /**< Skip .reuse/.                */
    BOOL  fSkipLicenseSidecars;   /**< Skip *.license.              */
    BOOL  fSkipReuseToml;         /**< Skip REUSE.toml.             */
    BOOL  fSkipLicenseFiles;      /**< Skip LICENSE/COPYING/....    */

    PCSZ                 pszRepoRoot;     /**< Repo root, or NULL. */
    const GITIGNORELIST *pGitignoreRules; /**< Rules, or NULL.     */
} REUSEDISCOVEROPTIONS, *PREUSEDISCOVEROPTIONS;

#ifdef __cplusplus
}
#endif

#endif /* REUSE_DISCOVER_INTERNAL_H */
