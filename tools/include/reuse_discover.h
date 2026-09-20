/* reuse_discover.h - REUSE file discovery with .gitignore filtering
 * (C89) */
#ifndef REUSE_DISCOVER_H
#define REUSE_DISCOVER_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "git.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_discover.h
 * @brief REUSE file discovery with .gitignore filtering.
 *
 * Walks a directory tree and collects the set of files that are
 * Covered Files by REUSE 3.3. Applies the REUSE skip rules
 * (LICENSES/, .reuse/, REUSE.toml, .license sidecars, license text
 * files, SPDX documents, VCS directories) and optionally filters
 * by .gitignore rules.
 *
 * Also supports extracting source file names from OMF object
 * files and OpenWatcom/Borland .res files.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - gitignore(5). https://git-scm.com/docs/gitignore
 */

/**
 * @struct _REUSEDISCOVEROPTIONS
 * @brief Walk options.
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

    BOOL                 fUseGitignore;   /**< Apply .gitignore rules. */
    PCSZ                 pszRepoRoot;     /**< Repo root, or NULL.     */
    const GITIGNORELIST *pGitignoreRules; /**< Rules, or NULL.         */
} REUSEDISCOVEROPTIONS, *PREUSEDISCOVEROPTIONS;

/**
 * @brief Set walk options to their default values.
 *
 * Defaults: recursive = TRUE, all REUSE skip flags = TRUE,
 * use_gitignore = FALSE, repo_root = NULL, gitignore_rules = NULL.
 *
 * @param[out] pOpts  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pOpts is NULL.
 */
APIRET APIENTRY ReuseSetDiscoverOptionsDefault(
    PREUSEDISCOVEROPTIONS pOpts);

/**
 * @brief Walk a directory tree and collect file paths.
 *
 * @param[in] pszDir  Root directory. Not NULL.
 * @param[in] pOpts   Options. Not NULL.
 * @param[in] hOut    Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszDir or pOpts is NULL, or hOut
 *                                  is NULLHANDLE.
 * @retval ERROR_OPEN_FAILED        A directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscoverWalkTree(
    PCSZ pszDir,
    const REUSEDISCOVEROPTIONS *pOpts,
    HSTRSET hOut);

/**
 * @brief Collect source names from OMF objects and .res files.
 *
 * @param[in] papszObjectFiles  Array of OMF object paths.
 * @param[in] ulObjectCount     Number of entries.
 * @param[in] papszResFiles     Array of .res paths.
 * @param[in] ulResCount        Number of entries.
 * @param[in] hOut              Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 At least one source name found.
 * @retval ERROR_INVALID_PARAMETER  hOut is NULLHANDLE.
 * @retval ERROR_FILE_NOT_FOUND     No source names found in any file.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscoverFromArtifacts(PSZ *papszObjectFiles,
                                           ULONG ulObjectCount,
                                           PSZ *papszResFiles,
                                           ULONG ulResCount,
                                           HSTRSET hOut);

/**
 * @brief Discover source files in a project.
 *
 * When object or resource artifacts are supplied, source names are
 * extracted from them. Otherwise, the project tree is walked.
 *
 * @param[in] pszProjectDir     Project directory. Not NULL.
 * @param[in] papszObjectFiles  Array of OMF object paths, or NULL.
 * @param[in] ulObjectCount     Number of entries.
 * @param[in] papszResFiles     Array of .res paths, or NULL.
 * @param[in] ulResCount        Number of entries.
 * @param[in] pOpts             Walk options. Not NULL.
 * @param[in] hOut              Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszProjectDir, pOpts is NULL, or
 *                                  hOut is NULLHANDLE.
 * @retval ERROR_OPEN_FAILED        A directory cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No sources discovered.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscover(PCSZ pszProjectDir,
                              PSZ *papszObjectFiles, ULONG ulObjectCount,
                              PSZ *papszResFiles, ULONG ulResCount,
                              const REUSEDISCOVEROPTIONS *pOpts,
                              HSTRSET hOut);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_DISCOVER_H */
