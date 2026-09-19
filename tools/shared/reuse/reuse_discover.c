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
 *
 */

/* ------------------------------------------------------------------ */
/* Options                                                             */
/* ------------------------------------------------------------------ */

void APIENTRY ReuseDiscoverOptionsDefault(REUSEDISCOVEROPTIONS *pOpts) {
    memset(pOpts, 0, sizeof(*pOpts));
    pOpts->recursive             = 1;
    pOpts->skip_hidden           = 1;
    pOpts->skip_vcs_dirs         = 1;
    pOpts->skip_licenses_dir     = 1;
    pOpts->skip_reuse_dir        = 1;
    pOpts->skip_license_sidecars = 1;
    pOpts->skip_reuse_toml       = 1;
    pOpts->skip_license_files    = 1;
    pOpts->use_gitignore         = 0;
    pOpts->repo_root             = NULL;
    pOpts->gitignore_rules       = NULL;
}

/* ------------------------------------------------------------------ */
/* Name checks                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Check whether a name is a VCS directory.
 */
static int is_vcs_dir(const char *name) {
    return strcmp(name, ".git") == 0 ||
           strcmp(name, ".svn") == 0 ||
           strcmp(name, ".hg")  == 0 ||
           strcmp(name, ".bzr") == 0;
}

/**
 * @brief Check whether a name is the LICENSES directory.
 */
static int is_licenses_dir(const char *name) {
    return strcmp(name, "LICENSES") == 0;
}

/**
 * @brief Check whether a name is the .reuse directory.
 */
static int is_reuse_dir(const char *name) {
    return strcmp(name, ".reuse") == 0;
}

/**
 * @brief Case-insensitive string equality.
 */
static int ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

/**
 * @brief Case-insensitive prefix comparison.
 */
static int ieq_prefix(const char *str, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (str[i] == '\0') return 0;
        if (tolower((unsigned char)str[i]) !=
            tolower((unsigned char)prefix[i]))
            return 0;
    }
    return 1;
}

/**
 * @brief Check whether a name denotes a license text file.
 *
 * Recognized names: LICENSE, LICENCE, COPYING, UNLICENSE, COPYRIGHT
 * and their variations with a '.' or '-' or '_' separator.
 */
static int is_license_file(const char *name) {
    static const char *exact[] = {
        "license", "licence", "copying", "unlicense", "copyright"
    };
    size_t i;
    for (i = 0; i < sizeof(exact)/sizeof(exact[0]); i++)
        if (ieq(name, exact[i])) return 1;
    if (ieq_prefix(name, "license", 7) ||
        ieq_prefix(name, "licence", 7) ||
        ieq_prefix(name, "copying", 7)) {
        if (name[7] == '\0' || name[7] == '.' ||
            name[7] == '-' || name[7] == '_')
            return 1;
    }
    return 0;
}

/**
 * @brief Check whether a name denotes a .license sidecar file.
 */
static int is_license_sidecar(const char *name) {
    size_t len = strlen(name);
    return len > 8 && strcmp(name + len - 8, ".license") == 0;
}

/**
 * @brief Check whether a name is REUSE.toml.
 */
static int is_reuse_toml(const char *name) {
    return strcmp(name, "REUSE.toml") == 0;
}

/**
 * @brief Check whether a name is hidden (starts with '.').
 */
static int is_hidden(const char *name) {
    return name[0] == '.';
}

/**
 * @brief Check whether a name denotes an SPDX document.
 *
 * Recognized suffixes: .spdx, .spdx.json, .spdx.yaml, .spdx.yml,
 * .spdx.rdf, .spdx.xml.
 */
static int is_spdx_document(const char *name) {
    size_t len = strlen(name);
    static const char *suffixes[] = {
        ".spdx", ".spdx.json", ".spdx.yaml", ".spdx.yml",
        ".spdx.rdf", ".spdx.xml"
    };
    size_t i;
    for (i = 0; i < sizeof(suffixes)/sizeof(suffixes[0]); i++) {
        size_t sl = strlen(suffixes[i]);
        if (len >= sl && strcmp(name + len - sl, suffixes[i]) == 0)
            return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Git filtering                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Path character comparison.
 *
 * Case-sensitive on Linux, case-insensitive on Windows.
 */
static int path_char_eq(int a, int b) {
#ifdef _WIN32
    return tolower((unsigned char)a) == tolower((unsigned char)b);
#else
    return (unsigned char)a == (unsigned char)b;
#endif
}

/**
 * @brief Path prefix comparison using path_char_eq.
 */
static int path_prefix_eq(const char *s, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (!s[i]) return 0;
        if (!path_char_eq(s[i], prefix[i])) return 0;
    }
    return 1;
}

/**
 * @brief Normalize a path to use '/'.
 *
 * @param[in] path  Path to normalize. Not NULL.
 *
 * @return malloc'd normalized path, or NULL on OOM.
 */
static char *to_slash(const char *path) {
    size_t i;
    size_t n = strlen(path);
    char *r = (char*)malloc(n + 1);
    if (!r) return NULL;
    for (i = 0; i < n; i++)
        r[i] = (path[i] == '\\') ? '/' : path[i];
    r[n] = '\0';
    return r;
}

/**
 * @brief Return the path of @p full relative to @p root.
 *
 * @param[in] root  Root directory. Not NULL.
 * @param[in] full  Full path. Not NULL.
 *
 * @return malloc'd relative path, or NULL if @p full is not under
 *         @p root or on OOM.
 */
static char *rel_path(const char *root, const char *full) {
    size_t rlen, flen;
    char *rroot, *rfull, *result;
    if (!root || !full) return NULL;
    rroot = to_slash(root);
    rfull = to_slash(full);
    if (!rroot || !rfull) {
        free(rroot);
        free(rfull);
        return NULL;
    }
    rlen = strlen(rroot);
    flen = strlen(rfull);
    if (flen < rlen || !path_prefix_eq(rfull, rroot, rlen)) {
        free(rroot);
        free(rfull);
        return NULL;
    }
    if (flen == rlen) {
        free(rroot);
        free(rfull);
        return strdup("");
    }
    if (rfull[rlen] != '/') {
        free(rroot);
        free(rfull);
        return NULL;
    }
    result = strdup(rfull + rlen + 1);
    free(rroot);
    free(rfull);
    return result;
}

/**
 * @brief Check whether a path should be skipped by .gitignore rules.
 *
 * @param[in] pOpts     Walk options. Not NULL.
 * @param[in] fullpath  Full path. Not NULL.
 * @param[in] is_dir    Non-zero for directories.
 *
 * @return 1 if ignored, 0 otherwise.
 */
static int should_skip_by_gitignore(const REUSEDISCOVEROPTIONS *pOpts,
                                    const char *fullpath, int is_dir) {
    char *rel;
    BOOL ignored = FALSE_;

    if (!pOpts->use_gitignore) return 0;
    if (!pOpts->gitignore_rules || pOpts->gitignore_rules->ulCount == 0)
        return 0;
    if (!pOpts->repo_root) return 0;

    rel = rel_path(pOpts->repo_root, fullpath);
    if (!rel) return 0;
    if (GitIsIgnored(pOpts->gitignore_rules, rel,
                     is_dir ? TRUE_ : FALSE_, &ignored) != NO_ERROR) {
        ignored = FALSE_;
    }
    free(rel);
    return ignored ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Other filters                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Check whether a file should be skipped.
 *
 * @param[in] name   Base name. Not NULL.
 * @param[in] pOpts  Walk options. Not NULL.
 *
 * @return 1 if the file should be skipped, 0 otherwise.
 */
static int should_skip_file(const char *name,
                            const REUSEDISCOVEROPTIONS *pOpts) {
    if (pOpts->skip_hidden && is_hidden(name)) return 1;
    if (pOpts->skip_license_sidecars && is_license_sidecar(name)) return 1;
    if (pOpts->skip_reuse_toml && is_reuse_toml(name)) return 1;
    if (pOpts->skip_license_files && is_license_file(name)) return 1;
    if (is_spdx_document(name)) return 1;
    return 0;
}

/**
 * @brief Check whether a directory should be skipped.
 *
 * @param[in] name   Base name. Not NULL.
 * @param[in] pOpts  Walk options. Not NULL.
 *
 * @return 1 if the directory should be skipped, 0 otherwise.
 */
static int should_skip_dir(const char *name,
                           const REUSEDISCOVEROPTIONS *pOpts) {
    if (pOpts->skip_hidden && is_hidden(name)) return 1;
    if (pOpts->skip_vcs_dirs && is_vcs_dir(name)) return 1;
    if (pOpts->skip_licenses_dir && is_licenses_dir(name)) return 1;
    if (pOpts->skip_reuse_dir && is_reuse_dir(name)) return 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Path normalization and joining                                      */
/* ------------------------------------------------------------------ */

/**
 * @brief Strip trailing separators from a directory path.
 *
 * @param[in]  src       Source path. Not NULL.
 * @param[out] dst       Destination buffer. Not NULL.
 * @param[in]  dst_size  Size of @p dst.
 */
static void normalize_dir(const char *src, char *dst, size_t dst_size) {
    size_t len = strlen(src);
#ifdef _WIN32
    if (len >= 2 && src[1] == ':' &&
        (len == 2 || (len == 3 && (src[2] == '\\' || src[2] == '/')))) {
        strncpy(dst, src, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
#endif
    while (len > 0 && (src[len-1] == '/' || src[len-1] == '\\'))
        len--;
    if (len >= dst_size) len = dst_size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

/**
 * @brief Join a directory and a name with the platform separator.
 */
static void join_path(char *dst, size_t dst_size,
                      const char *dir, const char *name) {
#ifdef __LINUX__
    snprintf(dst, dst_size, "%s/%s", dir, name);
#else
    snprintf(dst, dst_size, "%s\\%s", dir, name);
#endif
}

/* ------------------------------------------------------------------ */
/* Tree walk                                                           */
/* ------------------------------------------------------------------ */

#ifdef __LINUX__

static APIRET walk_inner(PCSZ pszDirIn,
                         const REUSEDISCOVEROPTIONS *pOpts,
                         HSTRSET hOut) {
    char dir[1024];
    DIR *d;
    struct dirent *entry;
    struct stat st;
    char full[1024];

    normalize_dir(pszDirIn, dir, sizeof(dir));
    d = opendir(dir);
    if (!d) return ERROR_OPEN_FAILED;

    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) continue;

        join_path(full, sizeof(full), dir, entry->d_name);
        if (stat(full, &st) != 0) continue;

        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            APIRET rc;
            if (!pOpts->recursive) continue;
            if (should_skip_dir(entry->d_name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, full, 1)) continue;
            rc = walk_inner(full, pOpts, hOut);
            if (rc != NO_ERROR) {
                closedir(d);
                return rc;
            }
        } else if (S_ISREG(st.st_mode)) {
            APIRET rc;
            if (st.st_size == 0) continue;
            if (should_skip_file(entry->d_name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, full, 0)) continue;
            rc = StrSetAdd(hOut, full);
            if (rc != NO_ERROR) {
                closedir(d);
                return rc;
            }
        }
    }
    closedir(d);
    return NO_ERROR;
}

#else

static APIRET walk_inner(PCSZ pszDirIn,
                         const REUSEDISCOVEROPTIONS *pOpts,
                         HSTRSET hOut) {
    char dir[1024];
    long hFile;
    struct _finddata_t fd;
    struct stat st;
    char pattern[1100];
    char full[1024];

    normalize_dir(pszDirIn, dir, sizeof(dir));
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    hFile = _findfirst(pattern, &fd);
    if (hFile == -1L) return ERROR_OPEN_FAILED;

    do {
        if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
            continue;

        join_path(full, sizeof(full), dir, fd.name);
        if (stat(full, &st) != 0) continue;

        if (st.st_mode & _S_IFDIR) {
            APIRET rc;
            if (!pOpts->recursive) continue;
            if (should_skip_dir(fd.name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, full, 1)) continue;
            rc = walk_inner(full, pOpts, hOut);
            if (rc != NO_ERROR) {
                _findclose(hFile);
                return rc;
            }
        } else if (st.st_mode & _S_IFREG) {
            APIRET rc;
            if (st.st_size == 0) continue;
            if (should_skip_file(fd.name, pOpts)) continue;
            if (should_skip_by_gitignore(pOpts, full, 0)) continue;
            rc = StrSetAdd(hOut, full);
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

APIRET APIENTRY ReuseDiscoverFromArtifacts(char **apszObjectFiles,
                                           ULONG ulObjectCount,
                                           char **apszResFiles,
                                           ULONG ulResCount,
                                           HSTRSET hOut) {
    ULONG i;
    int any = 0;

    if (hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    for (i = 0; i < ulObjectCount; i++) {
        APIRET rc = OmfExtractSources(apszObjectFiles[i], hOut);
        if (rc == NO_ERROR) {
            any = 1;
        } else if (rc == ERROR_NOT_ENOUGH_MEMORY) {
            return rc;
        }
        /* Other errors are per-file: skip and continue. */
    }

    for (i = 0; i < ulResCount; i++) {
        APIRET rc = ResExtractSources(apszResFiles[i], hOut);
        if (rc == NO_ERROR) {
            any = 1;
        } else if (rc == ERROR_NOT_ENOUGH_MEMORY) {
            return rc;
        }
    }

    return any ? NO_ERROR : ERROR_FILE_NOT_FOUND;
}

/* ------------------------------------------------------------------ */
/* Orchestrator                                                        */
/* ------------------------------------------------------------------ */

APIRET APIENTRY ReuseDiscover(PCSZ pszProjectDir,
                              char **apszObjectFiles, ULONG ulObjectCount,
                              char **apszResFiles, ULONG ulResCount,
                              const REUSEDISCOVEROPTIONS *pOpts,
                              HSTRSET hOut) {
    if (!pszProjectDir || !pOpts || hOut == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    if (ulObjectCount > 0 || ulResCount > 0) {
        return ReuseDiscoverFromArtifacts(apszObjectFiles, ulObjectCount,
                                          apszResFiles, ulResCount, hOut);
    }
    return ReuseDiscoverWalkTree(pszProjectDir, pOpts, hOut);
}
