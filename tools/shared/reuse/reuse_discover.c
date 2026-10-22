/*!
 * @file reuse_discover.c
 *
 * @brief Implementation of the REUSE file discovery module.
 *
 * Two modes:
 *   - Walk mode: recursively walk a directory tree, apply REUSE
 *     skip rules and .gitignore, collect Covered File paths.
 *   - Artifacts mode: pass each artifact to the OMF reader and, if
 *     it yields no names, to the RES reader.
 *
 * See reuse_discover.h for the public contract.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reuse_discover.h"
#include "reuse_discover_internal.h"
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

/* ==================================================================
 * Artifacts mode
 * ================================================================== */

/*!
 * @brief Extract source names from one artifact.
 *
 * Tries the OMF reader first, then the RES reader. Both readers
 * are tolerant to foreign formats: OmfQuerySources returns
 * ERROR_FILE_NOT_FOUND if no THEADR or COMENT record is found;
 * ResExtractSources returns ERROR_FILE_NOT_FOUND if the dependency
 * resource is absent. A failed first attempt therefore falls
 * through to the second.
 *
 * If the OMF attempt appends at least one name, the artifact is
 * treated as OMF and the RES attempt is skipped.
 *
 * @param[in] pszPath  Artifact path. Not NULL.
 * @param[in] hOut     Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Names extracted.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     Neither reader found any names.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET extract_from_artifact(PCSZ pszPath, HSTRSET hOut)
{
    HOMFFILE hOmf = NULLHANDLE;
    APIRET rc;
    ULONG ulBefore = 0;
    ULONG ulAfter = 0;

    /* --- Try OMF first. --- */
    StrSetGetCount(hOut, &ulBefore);

    rc = OmfOpen(pszPath, &hOmf, OMF_OPEN_READ | OMF_OPEN_EXISTING);
    if (rc == ERROR_NOT_ENOUGH_MEMORY) return rc;
    if (rc == NO_ERROR) {
        rc = OmfQuerySources(hOmf, hOut);
        OmfClose(hOmf);
        if (rc == ERROR_NOT_ENOUGH_MEMORY) return rc;
        StrSetGetCount(hOut, &ulAfter);
        if (ulAfter > ulBefore) return NO_ERROR;
        /* OMF opened but yielded no names — fall through to RES. */
    }

    /* --- Try RES. --- */
    StrSetGetCount(hOut, &ulBefore);
    rc = ResExtractSources(pszPath, hOut);
    if (rc == ERROR_NOT_ENOUGH_MEMORY) return rc;
    StrSetGetCount(hOut, &ulAfter);
    if (ulAfter > ulBefore) return NO_ERROR;

    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Discover source names from a list of artifacts.
 *
 * Per-file errors (except out-of-memory) are swallowed: a single
 * corrupt or unrecognised artifact does not abort the extraction.
 * The function reports NO_ERROR if at least one artifact yielded
 * new names; otherwise ERROR_FILE_NOT_FOUND.
 *
 * @param[in] papszArtifacts  Array of artifact paths. Not NULL.
 * @param[in] ulArtifactCount Number of entries. Greater than 0.
 * @param[in] hOut            Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 At least one artifact yielded
 *                                  source names.
 * @retval ERROR_FILE_NOT_FOUND     None yielded source names.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET discover_from_artifacts(PCSZ *papszArtifacts,
                                      ULONG ulArtifactCount,
                                      HSTRSET hOut)
{
    ULONG ulIdx;
    ULONG ulBefore = 0;
    ULONG ulAfter = 0;

    StrSetGetCount(hOut, &ulBefore);

    for (ulIdx = 0; ulIdx < ulArtifactCount; ulIdx++) {
        APIRET rc;

        if (!papszArtifacts[ulIdx]) continue;
        rc = extract_from_artifact(papszArtifacts[ulIdx], hOut);
        if (rc == ERROR_NOT_ENOUGH_MEMORY) return rc;
        /* Other errors are per-file: skip and continue. */
    }

    StrSetGetCount(hOut, &ulAfter);
    if (ulAfter == ulBefore) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}

/* ==================================================================
 * Name checks (walk mode)
 * ================================================================== */

/*!
 * @brief Query whether a name is a VCS directory.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name is .git, .svn, .hg or .bzr.
 * @retval FALSE  Otherwise.
 */
static BOOL is_vcs_dir(PCSZ pszName)
{
    return (strcmp(pszName, ".git") == 0 ||
            strcmp(pszName, ".svn") == 0 ||
            strcmp(pszName, ".hg")  == 0 ||
            strcmp(pszName, ".bzr") == 0) ? TRUE : FALSE;
}

/*!
 * @brief Query whether a name is the LICENSES directory.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name is "LICENSES".
 * @retval FALSE  Otherwise.
 */
static BOOL is_licenses_dir(PCSZ pszName)
{
    return (strcmp(pszName, "LICENSES") == 0) ? TRUE : FALSE;
}

/*!
 * @brief Query whether a name is the .reuse directory.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name is ".reuse".
 * @retval FALSE  Otherwise.
 */
static BOOL is_reuse_dir(PCSZ pszName)
{
    return (strcmp(pszName, ".reuse") == 0) ? TRUE : FALSE;
}

/*!
 * @brief Case-insensitive string equality.
 *
 * @param[in] pszA  First string. Not NULL.
 * @param[in] pszB  Second string. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Strings are equal ignoring case.
 * @retval FALSE  Otherwise.
 */
static BOOL ieq(PCSZ pszA, PCSZ pszB)
{
    while (*pszA && *pszB) {
        if (tolower((unsigned char)*pszA) !=
            tolower((unsigned char)*pszB))
            return FALSE;
        pszA++; pszB++;
    }
    return (*pszA == '\0' && *pszB == '\0') ? TRUE : FALSE;
}

/*!
 * @brief Case-insensitive prefix comparison.
 *
 * @param[in] pszStr     String to test. Not NULL.
 * @param[in] pszPrefix  Prefix. Not NULL.
 * @param[in] cbLen      Prefix length.
 *
 * @return BOOL
 *
 * @retval TRUE   @p pszStr starts with @p pszPrefix.
 * @retval FALSE  Otherwise, or @p pszStr is shorter.
 */
static BOOL ieq_prefix(PCSZ pszStr, PCSZ pszPrefix, size_t cbLen)
{
    size_t i;
    for (i = 0; i < cbLen; i++) {
        if (pszStr[i] == '\0') return FALSE;
        if (tolower((unsigned char)pszStr[i]) !=
            tolower((unsigned char)pszPrefix[i]))
            return FALSE;
    }
    return TRUE;
}

/*!
 * @brief Query whether a name denotes a license text file.
 *
 * Follows REUSE Specification 3.3, §«Covered and ignored Files»:
 * the files COPYING and LICENSE, with or without file extensions.
 * LICENCE (British spelling) is recognized as a variant of
 * LICENSE. Suffixes after '.' or '-' are accepted
 * (LICENSE-MIT, COPYING.txt, LICENCE.md).
 *
 * The reference implementation (reuse-tool) uses the equivalent
 * patterns ^LICEN[CS]E([-\.].*)?$ and ^COPYING([-\.].*)?$.
 *
 * Names such as UNLICENSE and COPYRIGHT are not covered by the
 * specification and are therefore not skipped.
 *
 * Case-insensitive.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name is a license text file per REUSE 3.3.
 * @retval FALSE  Otherwise.
 */
static BOOL is_license_file(PCSZ pszName)
{
    static PCSZ apszExact[] = {
        "license", "licence", "copying"
    };
    size_t i;
    for (i = 0; i < sizeof(apszExact)/sizeof(apszExact[0]); i++)
        if (ieq(pszName, apszExact[i])) return TRUE;
    if (ieq_prefix(pszName, "license", 7) ||
        ieq_prefix(pszName, "licence", 7) ||
        ieq_prefix(pszName, "copying", 7)) {
        if (pszName[7] == '\0' || pszName[7] == '.' ||
            pszName[7] == '-')
            return TRUE;
    }
    return FALSE;
}

/*!
 * @brief Query whether a name denotes a license text file (public
 *        wrapper).
 *
 * See reuse_discover.h for the contract. The definition lives in
 * is_license_file above; this wrapper exposes it to
 * reuse_licenses.c so both modules apply the same definition of
 * "license text file" per REUSE 3.3 §«Covered and ignored Files».
 *
 * @param[in] pszName  Base name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name is a license text file per REUSE 3.3.
 * @retval FALSE  Otherwise, or pszName is NULL.
 */
BOOL APIENTRY ReuseDiscoverIsLicenseFile(PCSZ pszName)
{
    if (!pszName) return FALSE;
    return is_license_file(pszName);
}

/*!
 * @brief Query whether a name denotes a .license sidecar file.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name ends with ".license".
 * @retval FALSE  Otherwise.
 */
static BOOL is_license_sidecar(PCSZ pszName)
{
    size_t cbLen = strlen(pszName);
    return (cbLen > 8 &&
            strcmp(pszName + cbLen - 8, ".license") == 0) ? TRUE : FALSE;
}

/*!
 * @brief Query whether a name is REUSE.toml.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name is "REUSE.toml".
 * @retval FALSE  Otherwise.
 */
static BOOL is_reuse_toml(PCSZ pszName)
{
    return (strcmp(pszName, "REUSE.toml") == 0) ? TRUE : FALSE;
}

/*!
 * @brief Query whether a name is hidden (starts with '.').
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name starts with '.'.
 * @retval FALSE  Otherwise.
 */
static BOOL is_hidden(PCSZ pszName)
{
    return (pszName[0] == '.') ? TRUE : FALSE;
}

/*!
 * @brief Query whether a name denotes an SPDX document.
 *
 * Recognized suffixes: .spdx, .spdx.json, .spdx.yaml, .spdx.yml,
 * .spdx.rdf, .spdx.xml.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Name has an SPDX document suffix.
 * @retval FALSE  Otherwise.
 */
static BOOL is_spdx_document(PCSZ pszName)
{
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

/* ==================================================================
 * Gitignore filtering
 * ================================================================== */

/*!
 * @brief Path character comparison.
 *
 * Case-insensitive on Windows, case-sensitive elsewhere.
 *
 * @param[in] nA  First character.
 * @param[in] nB  Second character.
 *
 * @return BOOL
 *
 * @retval TRUE   Characters are equal under the platform rules.
 * @retval FALSE  Otherwise.
 */
static BOOL path_char_eq(int nA, int nB)
{
#ifdef _WIN32
    return (tolower((unsigned char)nA) ==
            tolower((unsigned char)nB)) ? TRUE : FALSE;
#else
    return ((unsigned char)nA == (unsigned char)nB) ? TRUE : FALSE;
#endif
}

/*!
 * @brief Path prefix comparison using path_char_eq.
 *
 * @param[in] pszStr     String to test. Not NULL.
 * @param[in] pszPrefix  Prefix. Not NULL.
 * @param[in] cbLen      Prefix length.
 *
 * @return BOOL
 *
 * @retval TRUE   Prefix matches.
 * @retval FALSE  Otherwise.
 */
static BOOL path_prefix_eq(PCSZ pszStr, PCSZ pszPrefix, size_t cbLen)
{
    size_t i;
    for (i = 0; i < cbLen; i++) {
        if (!pszStr[i]) return FALSE;
        if (!path_char_eq(pszStr[i], pszPrefix[i])) return FALSE;
    }
    return TRUE;
}

/*!
 * @brief Normalize a path to use '/' separators.
 *
 * @param[in] pszPath  Path to normalize. Not NULL.
 *
 * @return PSZ
 *
 * @retval NULL  Allocation failure.
 * @return       malloc'd normalized path owned by the caller.
 */
static PSZ to_slash(PCSZ pszPath)
{
    size_t i;
    size_t cbLen = strlen(pszPath);
    PSZ pszOut = (PSZ)malloc(cbLen + 1);
    if (!pszOut) return NULL;
    for (i = 0; i < cbLen; i++)
        pszOut[i] = (pszPath[i] == '\\') ? '/' : pszPath[i];
    pszOut[cbLen] = '\0';
    return pszOut;
}

/*!
 * @brief Return the path of @p pszFull relative to @p pszRoot.
 *
 * @param[in] pszRoot  Root directory. Not NULL.
 * @param[in] pszFull  Full path. Not NULL.
 *
 * @return PSZ
 *
 * @retval NULL  @p pszFull is not under @p pszRoot, or allocation
 *               failure.
 * @return       malloc'd relative path owned by the caller.
 */
static PSZ rel_path(PCSZ pszRoot, PCSZ pszFull)
{
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

/*!
 * @brief Query whether a path is ignored by .gitignore rules.
 *
 * @param[in] pOpts    Walk options. Not NULL.
 * @param[in] pszFull  Full path. Not NULL.
 * @param[in] fIsDir   TRUE for directories.
 *
 * @return BOOL
 *
 * @retval TRUE   Path is ignored.
 * @retval FALSE  Otherwise.
 */
static BOOL should_skip_by_gitignore(const REUSEDISCOVEROPTIONS *pOpts,
                                     PCSZ pszFull, BOOL fIsDir)
{
    PSZ pszRel;
    BOOL fIgnored = FALSE;

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

/* ==================================================================
 * Skip filters
 * ================================================================== */

/*!
 * @brief Query whether a file should be skipped.
 *
 * @param[in] pszName  Base name. Not NULL.
 * @param[in] pOpts    Walk options. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   File is skipped.
 * @retval FALSE  Otherwise.
 */
static BOOL should_skip_file(PCSZ pszName,
                             const REUSEDISCOVEROPTIONS *pOpts)
{
    if (pOpts->fSkipHidden && is_hidden(pszName)) return TRUE;
    if (pOpts->fSkipLicenseSidecars && is_license_sidecar(pszName))
        return TRUE;
    if (pOpts->fSkipReuseToml && is_reuse_toml(pszName)) return TRUE;
    if (pOpts->fSkipLicenseFiles && is_license_file(pszName)) return TRUE;
    if (is_spdx_document(pszName)) return TRUE;
    return FALSE;
}

/*!
 * @brief Query whether a directory should be skipped.
 *
 * @param[in] pszName  Base name. Not NULL.
 * @param[in] pOpts    Walk options. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Directory is skipped.
 * @retval FALSE  Otherwise.
 */
static BOOL should_skip_dir(PCSZ pszName,
                            const REUSEDISCOVEROPTIONS *pOpts)
{
    if (pOpts->fSkipHidden && is_hidden(pszName)) return TRUE;
    if (pOpts->fSkipVcsDirs && is_vcs_dir(pszName)) return TRUE;
    if (pOpts->fSkipLicensesDir && is_licenses_dir(pszName)) return TRUE;
    if (pOpts->fSkipReuseDir && is_reuse_dir(pszName)) return TRUE;
    return FALSE;
}

/* ==================================================================
 * Tree walk (Linux and Windows variants)
 * ================================================================== */

#ifdef __LINUX__

/*!
 * @brief Walk a directory tree on Linux.
 *
 * @param[in] pszDirIn  Directory. Not NULL.
 * @param[in] pOpts     Walk options. Not NULL.
 * @param[in] hOut      Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET walk_inner(PCSZ pszDirIn,
                         const REUSEDISCOVEROPTIONS *pOpts,
                         HSTRSET hOut)
{
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

/*!
 * @brief Walk a directory tree on Windows.
 *
 * @param[in] pszDirIn  Directory. Not NULL.
 * @param[in] pOpts     Walk options. Not NULL.
 * @param[in] hOut      Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET walk_inner(PCSZ pszDirIn,
                         const REUSEDISCOVEROPTIONS *pOpts,
                         HSTRSET hOut)
{
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

/* ==================================================================
 * Walk mode
 * ================================================================== */

/*!
 * @brief Walk a directory tree, applying skip rules and .gitignore.
 *
 * All skip flags are hard-wired to TRUE, matching the REUSE 3.3
 * definition of Covered Files. .gitignore is applied unconditionally
 * when @p pszDir is inside a Git repository.
 *
 * @param[in] pszDir     Target directory. Not NULL.
 * @param[in] fRecursive TRUE to recurse.
 * @param[in] hOut       Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (possibly empty).
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET discover_walk(PCSZ pszDir, BOOL fRecursive,
                            HSTRSET hOut)
{
    REUSEDISCOVEROPTIONS opts;
    GITIGNORELIST gitignore;
    PSZ pszRepoRoot = NULL;
    APIRET rc;

    GitIgnoreListInit(&gitignore);
    memset(&opts, 0, sizeof(opts));
    opts.fRecursive           = fRecursive;
    opts.fSkipHidden          = TRUE;
    opts.fSkipVcsDirs         = TRUE;
    opts.fSkipLicensesDir     = TRUE;
    opts.fSkipReuseDir        = TRUE;
    opts.fSkipLicenseSidecars = TRUE;
    opts.fSkipReuseToml       = TRUE;
    opts.fSkipLicenseFiles    = TRUE;

    /* Two-phase GitFindRepoRoot: first query the size, then the value. */
    {
        ULONG ulSize = 0;
        if (GitFindRepoRoot(pszDir, NULL, 0, &ulSize) == NO_ERROR &&
            ulSize > 0) {
            pszRepoRoot = (PSZ)malloc(ulSize);
            if (pszRepoRoot) {
                if (GitFindRepoRoot(pszDir, pszRepoRoot, ulSize, NULL)
                        != NO_ERROR) {
                    free(pszRepoRoot);
                    pszRepoRoot = NULL;
                }
            }
        }
    }

    if (pszRepoRoot) {
        if (GitCollectGitignores(pszRepoRoot, pszDir, &gitignore)
                == NO_ERROR && gitignore.ulCount > 0) {
            opts.pszRepoRoot     = pszRepoRoot;
            opts.pGitignoreRules = &gitignore;
        }
    }

    rc = walk_inner(pszDir, &opts, hOut);

    GitIgnoreListFree(&gitignore);
    free(pszRepoRoot);
    return rc;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/*!
 * @brief Discover source files for a REUSE project.
 *
 * See reuse_discover.h for the full contract.
 *
 * @param[in] pszDir          Target directory. Not NULL.
 * @param[in] papszArtifacts  Array of artifacts, or NULL.
 * @param[in] ulArtifactCount Number of entries.
 * @param[in] fRecursive      TRUE to recurse in walk mode.
 * @param[in] hOut            Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (walk, possibly empty;
 *                                  or artifacts yielded names).
 * @retval ERROR_INVALID_PARAMETER  Bad argument, or a NULL element
 *                                  in papszArtifacts.
 * @retval ERROR_OPEN_FAILED        Walk mode: directory cannot be
 *                                  opened.
 * @retval ERROR_FILE_NOT_FOUND     Artifacts mode: no source names.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscoverFiles(PCSZ pszDir,
                                   PCSZ *papszArtifacts,
                                   ULONG ulArtifactCount,
                                   BOOL fRecursive,
                                   HSTRSET hOut)
{
    ULONG ulIdx;

    if (!pszDir || hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    if (papszArtifacts == NULL && ulArtifactCount > 0)
        return ERROR_INVALID_PARAMETER;
    if (papszArtifacts != NULL) {
        for (ulIdx = 0; ulIdx < ulArtifactCount; ulIdx++)
            if (!papszArtifacts[ulIdx]) return ERROR_INVALID_PARAMETER;
    }

    if (papszArtifacts != NULL && ulArtifactCount > 0)
        return discover_from_artifacts(papszArtifacts,
                                       ulArtifactCount, hOut);
    return discover_walk(pszDir, fRecursive, hOut);
}
