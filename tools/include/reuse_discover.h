/* reuse_discover.h - REUSE file discovery with .gitignore filtering (C89) */
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
 * References:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - gitignore(5).
 *
 */

/**
 * @struct _REUSEDISCOVEROPTIONS
 * @brief Walk options.
 */
typedef struct _REUSEDISCOVEROPTIONS {
    int recursive;
    int skip_hidden;
    int skip_vcs_dirs;
    int skip_licenses_dir;
    int skip_reuse_dir;
    int skip_license_sidecars;
    int skip_reuse_toml;
    int skip_license_files;

    /* Git filtering */
    int use_gitignore;
    const char *repo_root;
    const GITIGNORELIST *gitignore_rules;
} REUSEDISCOVEROPTIONS, *PREUSEDISCOVEROPTIONS;

/**
 * @brief Initialize options with defaults.
 *
 * Defaults: recursive = 1, all REUSE skip flags = 1,
 * use_gitignore = 0, repo_root = NULL, gitignore_rules = NULL.
 *
 * @param[out] pOpts  Receiver. Not NULL.
 */
void APIENTRY ReuseDiscoverOptionsDefault(REUSEDISCOVEROPTIONS *pOpts);

/**
 * @brief Walk a directory tree and collect file paths.
 *
 * @param[in] pszDir  Root directory. Not NULL.
 * @param[in] pOpts   Options. Not NULL.
 * @param[in] hOut    Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL or hOut is
 *                                  NULLHANDLE.
 * @retval ERROR_OPEN_FAILED        A directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscoverWalkTree(PCSZ pszDir,
                                      const REUSEDISCOVEROPTIONS *pOpts,
                                      HSTRSET hOut);

/**
 * @brief Collect source names from OMF objects and .res files.
 *
 * @param[in] apszObjectFiles  Array of OMF object paths.
 * @param[in] ulObjectCount    Number of entries.
 * @param[in] apszResFiles     Array of .res paths.
 * @param[in] ulResCount       Number of entries.
 * @param[in] hOut             Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 At least one source name found.
 * @retval ERROR_INVALID_PARAMETER  hOut is NULLHANDLE.
 * @retval ERROR_FILE_NOT_FOUND     No source names found in any
 *                                  file.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscoverFromArtifacts(char **apszObjectFiles,
                                           ULONG ulObjectCount,
                                           char **apszResFiles,
                                           ULONG ulResCount,
                                           HSTRSET hOut);

/**
 * @brief Discover source files in a project.
 *
 * When object or resource artifacts are supplied, source names are
 * extracted from them. Otherwise, the project tree is walked.
 *
 * @param[in] pszProjectDir    Project directory. Not NULL.
 * @param[in] apszObjectFiles  Array of OMF object paths, or NULL.
 * @param[in] ulObjectCount    Number of entries.
 * @param[in] apszResFiles     Array of .res paths, or NULL.
 * @param[in] ulResCount       Number of entries.
 * @param[in] pOpts            Walk options. Not NULL.
 * @param[in] hOut             Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszProjectDir, pOpts or hOut is
 *                                  invalid.
 * @retval ERROR_OPEN_FAILED        A directory cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No sources discovered.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscover(PCSZ pszProjectDir,
                              char **apszObjectFiles, ULONG ulObjectCount,
                              char **apszResFiles, ULONG ulResCount,
                              const REUSEDISCOVEROPTIONS *pOpts,
                              HSTRSET hOut);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_DISCOVER_H */
