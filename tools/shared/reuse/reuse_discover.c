/* reuse_discover.c - REUSE file discovery with .gitignore filtering (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reuse_discover.h"
#include "omf.h"
#include "res.h"
#include "ccl.h"
#include "git.h"
#include "path.h"

#ifdef __LINUX__
#include <dirent.h>
#include <sys/stat.h>
#else
#include <direct.h>
#include <sys/stat.h>
#include <io.h>
#endif

/**
 * @file reuse_discover.c
 * @brief Implementation of the REUSE file discovery module.
 *
 * See reuse_discover.h for the description of skip rules and
 * references.
 */

/* ------------------------------------------------------------------ */
/* Options                                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Set walk options to their default values.
 *
 * @param[out] pOpts  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pOpts is NULL.
 */
APIRET APIENTRY ReuseSetDiscoverOptionsDefault(PREUSEDISCOVEROPTIONS pOpts) {
    if (!pOpts) return ERROR_INVALID_PARAMETER;
    memset(pOpts, 0, sizeof(*pOpts));
    pOpts->fRecursive             = TRUE;
    pOpts->fSkipHidden            = TRUE;
    pOpts->fSkipVcsDirs           = TRUE;
    pOpts->fSkipLicensesDir       = TRUE;
    pOpts->fSkipReuseDir          = TRUE;
    pOpts->fSkipLicenseSidecars   = TRUE;
    pOpts->fSkipReuseToml         = TRUE;
    pOpts->fSkipLicenseFiles      = TRUE;
    pOpts->fUseGitignore          = FALSE;
    pOpts->pszRepoRoot            = NULL;
    pOpts->pGitignoreRules        = NULL;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Name checks                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Query whether a name is a VCS directory.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if VCS directory, FALSE otherwise.
 */
static BOOL is_vcs_dir(PCSZ pszName) {
    return (strcmp(pszName, ".git") == 0 ||
            strcmp(pszName, ".svn") == 0 ||
            strcmp(pszName, ".hg")  == 0 ||
            strcmp(pszName, ".bzr") == 0) ? TRUE : FALSE;
}

/**
 * @brief Query whether a name is the LICENSES directory.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if "LICENSES", FALSE otherwise.
 */
static BOOL is_licenses_dir(PCSZ pszName) {
    return (strcmp(pszName, "LICENSES") == 0) ? TRUE : FALSE;
}

/**
 * @brief Query whether a name is the .reuse directory.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if ".reuse", FALSE otherwise.
 */
static BOOL is_reuse_dir(PCSZ pszName) {
    return (strcmp(pszName, ".reuse") == 0) ? TRUE : FALSE;
}

/**
 * @brief Case-insensitive string equality.
 *
 * @param[in] pszA  First string. Not NULL.
 * @param[in] pszB  Second string. Not NULL.
 *
 * @return TRUE if equal, FALSE otherwise.
 */
static BOOL ieq(PCSZ pszA, PCSZ pszB) {
    while (*pszA && *pszB) {
        if (tolower((unsigned char)*pszA) !=
            tolower((unsigned char)*pszB))
            return FALSE;
        pszA++; pszB++;
    }
    return (*pszA == '\0' && *pszB == '\0') ? TRUE : FALSE;
}

/**
 * @brief Case-insensitive prefix comparison.
 *
 * @param[in] pszStr     String to test. Not NULL.
 * @param[in] pszPrefix  Prefix. Not NULL.
 * @param[in] cbLen      Prefix length.
 *
 * @return TRUE if @p pszStr starts with @p pszPrefix, FALSE otherwise.
 */
static BOOL ieq_prefix(PCSZ pszStr, PCSZ pszPrefix, size_t cbLen) {
    size_t i;
    for (i = 0; i < cbLen; i++) {
        if (pszStr[i] == '\0') return FALSE;
        if (tolower((unsigned char)pszStr[i]) !=
            tolower((unsigned char)pszPrefix[i]))
            return FALSE;
    }
    return TRUE;
}

/**
 * @brief Query whether a name denotes a license text file.
 *
 * Recognized names: LICENSE, LICENCE, COPYING, UNLICENSE, COPYRIGHT
 * and their variations with a '.' or '-' or '_' separator.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if license file, FALSE otherwise.
 */
static BOOL is_license_file(PCSZ pszName) {
    static PCSZ apszExact[] = {
        "license", "licence", "copying", "unlicense", "copyright"
    };
    size_t i;
    for (i = 0; i < sizeof(apszExact)/sizeof(apszExact[0]); i++)
        if (ieq(pszName, apszExact[i])) return TRUE;
    if (ieq_prefix(pszName, "license", 7) ||
        ieq_prefix(pszName, "licence", 7) ||
        ieq_prefix(pszName, "copying", 7)) {
        if (pszName[7] == '\0' || pszName[7] == '.' ||
            pszName[7] == '-' || pszName[7] == '_')
            return TRUE;
    }
    return FALSE;
}

/**
 * @brief Query whether a name denotes a .license sidecar file.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if sidecar, FALSE otherwise.
 */
static BOOL is_license_sidecar(PCSZ pszName) {
    size_t cbLen = strlen(pszName);
    return (cbLen > 8 &&
            strcmp(pszName + cbLen - 8, ".license") == 0) ? TRUE : FALSE;
}

/**
 * @brief Query whether a name is REUSE.toml.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if "REUSE.toml", FALSE otherwise.
 */
static BOOL is_reuse_toml(PCSZ pszName) {
    return (strcmp(pszName, "REUSE.toml") == 0) ? TRUE : FALSE;
}

/**
 * @brief Query whether a name is hidden (starts with '.').
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if hidden, FALSE otherwise.
 */
static BOOL is_hidden(PCSZ pszName) {
    return (pszName[0] == '.') ? TRUE : FALSE;
}

/**
 * @brief Query whether a name denotes an SPDX document.
 *
 * Recognized suffixes: .spdx, .spdx.json, .spdx.yaml, .spdx.yml,
 * .spdx.rdf, .spdx.xml.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if SPDX document, FALSE otherwise.
 */
static BOOL is_spdx_document(PCSZ pszName) {
    size_t cbLen = strlen(pszName);
    static PCSZ apszSuffixes[] = {
        ".spdx", ".spdx.json", ".spdx.yaml", ".spdx.yml",
        ".spdx.rdf", ".spdx.xml"
    };
    size_t i;
    for (i = 0; i < sizeof(apszSuffixes)/sizeof(apszSuffixes[0]); i++) {
        size_t cbSuffix = strlen(apszSuffixes[i]);
        if (cbLen >= cbSuffix &&
            strcmp(pszName + cbLen - cbSuffix, apszSuffixes[i]) == 0)
            return TRUE;
    }
    return FALSE;
}

/* ------------------------------------------------------------------ */
/* Git filtering                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Path character comparison.
 *
 * Case-sensitive on Linux, case-insensitive on Windows.
 *
 * @param[in] nA  First character.
 * @param[in] nB  Second character.
 *
 * @return TRUE if equal, FALSE otherwise.
 */
static BOOL path_char_eq(int nA, int nB) {
#ifdef _WIN32
    return (tolower((unsigned char)nA) ==
            tolower((unsigned char)nB)) ? TRUE : FALSE;
#else
    return ((unsigned char)nA == (unsigned char)nB) ? TRUE : FALSE;
#endif
}

/**
 * @brief Path prefix comparison using path_char_eq.
 *
 * @param[in] pszStr     String to test. Not NULL.
 * @param[in] pszPrefix  Prefix. Not NULL.
 * @param[in] cbLen      Prefix length.
 *
 * @return TRUE on match, FALSE otherwise.
 */
static BOOL path_prefix_eq(PCSZ pszStr, PCSZ pszPrefix, size_t cbLen) {
    size_t i;
    for (i = 0; i < cbLen; i++) {
        if (!pszStr[i]) return FALSE;
        if (!path_char_eq(pszStr[i], pszPrefix[i])) return FALSE;
    }
    return TRUE;
}

/**
 * @brief Normalize a path to use '/' separators.
 *
 * @param[in] pszPath  Path to normalize. Not NULL.
 *
 * @return malloc'd normalized path, or NULL on OOM.
 */
static PSZ to_slash(PCSZ pszPath) {
    size_t i;
    size_t cbLen = strlen(pszPath);
    PSZ pszOut = (PSZ)malloc(cbLen + 1);
    if (!pszOut) return NULL;
    for (i = 0; i < cbLen; i++)
        pszOut[i] = (pszPath[i] == '\\') ? '/' : pszPath[i];
    pszOut[cbLen] = '\0';
    return pszOut;
}

/**
 * @brief Return the path of @p pszFull relative to @p pszRoot.
 *
 * @param[in] pszRoot  Root directory. Not NULL.
 * @param[in] pszFull  Full path. Not NULL.
 *
 * @return malloc'd relative path, or NULL if @p pszFull is not under
 *         @p pszRoot or on OOM.
 */
static PSZ rel_path(PCSZ pszRoot, PCSZ pszFull) {
    size_t cbRootLen, cbFullLen;
    PSZ pszRootSlash, pszFullSlash, pszResult;
    if (!pszRoot || !pszFull) return NULL;
    pszRootSlash = to_slash(pszRoot);
    pszFullSlash = to_slash(pszFull);
    if (!pszRootSlash || !pszFullSlash) {
        free(pszRootSlash);
        free(pszFullSlash);
        return NULL;
    }
    cbRootLen = strlen(pszRootSlash);
    cbFullLen = strlen(pszFullSlash);
    if (cbFullLen < cbRootLen ||
        !path_prefix_eq(pszFullSlash, pszRootSlash, cbRootLen)) {
        free(pszRootSlash);
        free(pszFullSlash);
        return NULL;
    }
    if (cbFullLen == cbRootLen) {
        free(pszRootSlash);
        free(pszFullSlash);
        return strdup("");
    }
    if (pszFullSlash[cbRootLen] != '/') {
        free(pszRootSlash);
        free(pszFullSlash);
        return NULL;
    }
    pszResult = strdup(pszFullSlash + cbRootLen + 1);
    free(pszRootSlash);
    free(pszFullSlash);
    return pszResult;
}

/**
 * @brief Query whether a path should be skipped by .gitignore rules.
 *
 * @param[in] pOpts      Walk options. Not NULL.
 * @param[in] pszFull    Full path. Not NULL.
 * @param[in] fIsDir     TRUE for directories.
 *
 * @return TRUE if ignored, FALSE otherwise.
 */
static BOOL should_skip_by_gitignore(const REUSEDISCOVEROPTIONS *pOpts,
                                     PCSZ pszFull, BOOL fIsDir) {
    PSZ pszRel;
    BOOL fIgnored = FALSE;

    if (!pOpts->fUseGitignore) return FALSE;
    if (!pOpts->pGitignoreRules ||
        pOpts->pGitignoreRules->ulCount == 0)
        return FALSE;
    if (!pOpts->pszRepoRoot) return FALSE;

    pszRel = rel_path(pOpts->pszRepoRoot, pszFull);
    if (!pszRel) return FALSE;
    if (GitQueryIsIgnored(pOpts->pGitignoreRules, pszRel,
                          fIsDir ? TRUE : FALSE,
                          &fIgnored) != NO_ERROR) {
        fIgnored = FALSE;
    }
    free(pszRel);
    return fIgnored ? TRUE : FALSE;
}

/* ------------------------------------------------------------------ */
/* Other filters                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Query whether a file should be skipped.
 *
 * @param[in] pszName  Base name. Not NULL.
 * @param[in] pOpts    Walk options. Not NULL.
 *
 * @return TRUE if skipped, FALSE otherwise.
 */
static BOOL should_skip_file(PCSZ pszName,
                             const REUSEDISCOVEROPTIONS *pOpts) {
    if (pOpts->fSkipHidden && is_hidden(pszName)) return TRUE;
    if (pOpts->fSkipLicenseSidecars && is_license_sidecar(pszName))
        return TRUE;
    if (pOpts->fSkipReuseToml && is_reuse_toml(pszName)) return TRUE;
    if (pOpts->fSkipLicenseFiles && is_license_file(pszName)) return TRUE;
    if (is_spdx_document(pszName)) return TRUE;
    return FALSE;
}

/**
 * @brief Query whether a directory should be skipped.
 *
 * @param[in] pszName  Base name. Not NULL.
 * @param[in] pOpts    Walk options. Not NULL.
 *
 * @return TRUE if skipped, FALSE otherwise.
 */
static BOOL should_skip_dir(PCSZ pszName,
                            const REUSEDISCOVEROPTIONS *pOpts) {
    if (pOpts->fSkipHidden && is_hidden(pszName)) return TRUE;
    if (pOpts->fSkipVcsDirs && is_vcs_dir(pszName)) return TRUE;
    if (pOpts->fSkipLicensesDir && is_licenses_dir(pszName)) return TRUE;
    if (pOpts->fSkipReuseDir && is_reuse_dir(pszName)) return TRUE;
    return FALSE;
}

/* ------------------------------------------------------------------ */
/* Tree walk                                                           */
/* ------------------------------------------------------------------ */

#ifdef __LINUX__

/**
 * @brief Walk a directory tree on Linux.
 *
 * @param[in] pszDirIn  Directory. Not NULL.
 * @param[in] pOpts     Walk options. Not NULL.
 * @param[in] hOut      Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET walk_inner(PCSZ pszDirIn,
                         const REUSEDISCOVEROPTIONS *pOpts,
                         HSTRSET hOut) {
    CHAR achDir[1024];
    DIR *pDir;
    struct dirent *pEntry;
    struct stat st;
    CHAR achFull[1024];

    strncpy(achDir, pszDirIn, sizeof(achDir) - 1);
    achDir[sizeof(achDir) - 1] = '\0';
    PathRemoveTrailingSeparators(achDir);

    pDir = opendir(achDir);
    if (!pDir) return ERROR_OPEN_FAILED;

    while ((pEntry = readdir(pDir)) != NULL) {
        if (strcmp(pEntry->d_name, ".") == 0 ||
            strcmp(pEntry->d_name, "..") == 0) continue;

        PathMakeJoin(achDir, pEntry->d_name, achFull, sizeof(achFull),
                     NULL);
        if (stat(achFull, &st) != 0) continue;

        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            APIRET rc;
            if (!pOpts->fRecursive) continue;
            if (should_skip_dir(pEntry->d_name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, achFull, TRUE)) continue;
            rc = walk_inner(achFull, pOpts, hOut);
            if (rc != NO_ERROR) {
                closedir(pDir);
                return rc;
            }
        } else if (S_ISREG(st.st_mode)) {
            APIRET rc;
            if (st.st_size == 0) continue;
            if (should_skip_file(pEntry->d_name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, achFull, FALSE)) continue;
            rc = StrSetAdd(hOut, achFull);
            if (rc != NO_ERROR) {
                closedir(pDir);
                return rc;
            }
        }
    }
    closedir(pDir);
    return NO_ERROR;
}

#else

/**
 * @brief Walk a directory tree on Windows.
 *
 * @param[in] pszDirIn  Directory. Not NULL.
 * @param[in] pOpts     Walk options. Not NULL.
 * @param[in] hOut      Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET walk_inner(PCSZ pszDirIn,
                         const REUSEDISCOVEROPTIONS *pOpts,
                         HSTRSET hOut) {
    CHAR achDir[1024];
    long hFile;
    struct _finddata_t fd;
    struct stat st;
    CHAR achPattern[1100];
    CHAR achFull[1024];

    strncpy(achDir, pszDirIn, sizeof(achDir) - 1);
    achDir[sizeof(achDir) - 1] = '\0';
    PathRemoveTrailingSeparators(achDir);

    snprintf(achPattern, sizeof(achPattern), "%s\\*", achDir);
    hFile = _findfirst(achPattern, &fd);
    if (hFile == -1L) return ERROR_OPEN_FAILED;

    do {
        if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
            continue;

        PathMakeJoin(achDir, fd.name, achFull, sizeof(achFull), NULL);
        if (stat(achFull, &st) != 0) continue;

        if (st.st_mode & _S_IFDIR) {
            APIRET rc;
            if (!pOpts->fRecursive) continue;
            if (should_skip_dir(fd.name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, achFull, TRUE)) continue;
            rc = walk_inner(achFull, pOpts, hOut);
            if (rc != NO_ERROR) {
                _findclose(hFile);
                return rc;
            }
        } else if (st.st_mode & _S_IFREG) {
            APIRET rc;
            if (st.st_size == 0) continue;
            if (should_skip_file(fd.name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, achFull, FALSE)) continue;
            rc = StrSetAdd(hOut, achFull);
            if (rc != NO_ERROR) {
                _findclose(hFile);
                return rc;
            }
        }
    } while (_findnext(hFile, &fd) == 0);
    _findclose(hFile);
    return NO_ERROR;
}

#endif

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
APIRET APIENTRY ReuseDiscoverWalkTree(PCSZ pszDir,
                                      const REUSEDISCOVEROPTIONS *pOpts,
                                      HSTRSET hOut) {
    if (!pszDir || !pOpts || hOut == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;
    return walk_inner(pszDir, pOpts, hOut);
}

/* ------------------------------------------------------------------ */
/* Build artifacts                                                     */
/* ------------------------------------------------------------------ */

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
                                           HSTRSET hOut) {
    ULONG ulIdx;
    BOOL fAny = FALSE;

    if (hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    for (ulIdx = 0; ulIdx < ulObjectCount; ulIdx++) {
        APIRET rc = OmfQuerySources(papszObjectFiles[ulIdx], hOut);
        if (rc == NO_ERROR) {
            fAny = TRUE;
        } else if (rc == ERROR_NOT_ENOUGH_MEMORY) {
            return rc;
        }
        /* Other errors are per-file: skip and continue. */
    }

    for (ulIdx = 0; ulIdx < ulResCount; ulIdx++) {
        APIRET rc = ResExtractSources(papszResFiles[ulIdx], hOut);
        if (rc == NO_ERROR) {
            fAny = TRUE;
        } else if (rc == ERROR_NOT_ENOUGH_MEMORY) {
            return rc;
        }
    }

    return fAny ? NO_ERROR : ERROR_FILE_NOT_FOUND;
}

/* ------------------------------------------------------------------ */
/* Orchestrator                                                        */
/* ------------------------------------------------------------------ */

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
                              HSTRSET hOut) {
    if (!pszProjectDir || !pOpts || hOut == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    if (ulObjectCount > 0 || ulResCount > 0) {
        return ReuseDiscoverFromArtifacts(papszObjectFiles, ulObjectCount,
                                          papszResFiles, ulResCount, hOut);
    }
    return ReuseDiscoverWalkTree(pszProjectDir, pOpts, hOut);
}
