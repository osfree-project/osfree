/* git.c - Git repository helpers for file filtering
 * (C89 + Watcom extensions) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef __LINUX__
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#endif

#include "git.h"

/**
 * @file git.c
 * @brief Implementation of the Git helper module.
 *
 * Locates Git repositories and applies .gitignore rules without
 * invoking the Git binary.
 */

/* ------------------------------------------------------------------ */
/* Utilities                                                           */
/* ------------------------------------------------------------------ */

static char *dup_str(const char *s) {
    size_t n;
    char *p;
    if (!s) return NULL;
    n = strlen(s);
    p = (char*)malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

static int has_git_entry(const char *path) {
    char p[1024];
    struct stat st;
    snprintf(p, sizeof(p), "%s/.git", path);
    if (stat(p, &st) == 0) return 1;
    snprintf(p, sizeof(p), "%s\\.git", path);
    if (stat(p, &st) == 0) return 1;
    return 0;
}

/* Remove trailing separators (except "C:\" on Windows). */
static void trim_separators(char *s) {
    size_t len = strlen(s);
#ifdef _WIN32
    if (len == 3 && s[1] == ':' && (s[2] == '\\' || s[2] == '/')) return;
#endif
    while (len > 1 && (s[len-1] == '/' || s[len-1] == '\\')) {
        s[--len] = '\0';
    }
#ifdef _WIN32
    if (len == 2 && s[1] == ':') {
        s[2] = '\\';
        s[3] = '\0';
    }
#endif
}

/* ------------------------------------------------------------------ */
/* Repository root discovery                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Find the root of the Git repository containing a directory.
 *
 * @param[in]  pszStartDir  Starting directory. Not NULL.
 * @param[out] ppszRoot     Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success.
 * @retval GIT_ERROR_INVALID_PARAM pszStartDir or ppszRoot is NULL.
 * @retval GIT_ERROR_OUT_OF_MEMORY Memory allocation failure.
 * @retval GIT_ERROR_NOT_FOUND     No .git entry found.
 */
APIRET APIENTRY GitFindRepoRoot(PCSZ pszStartDir, PSZ *ppszRoot) {
    char *current;

    if (!pszStartDir || !ppszRoot) return GIT_ERROR_INVALID_PARAM;
    *ppszRoot = NULL;

    current = dup_str(pszStartDir);
    if (!current) return GIT_ERROR_OUT_OF_MEMORY;

    while (1) {
        char *slash;
        char *backslash;

        trim_separators(current);

        if (has_git_entry(current)) {
            *ppszRoot = current;
            return GIT_NO_ERROR;
        }

        slash = strrchr(current, '/');
        backslash = strrchr(current, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (!slash) break;

#ifdef _WIN32
        if (slash == current + 2 && current[1] == ':') break;
#endif
        if (slash == current) break;

        *slash = '\0';
    }

    free(current);
    return GIT_ERROR_NOT_FOUND;
}

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
APIRET APIENTRY GitIsRepo(PCSZ pszDir, PBOOL pfIsRepo) {
    PSZ pszRoot = NULL;
    APIRET rc;

    if (!pszDir || !pfIsRepo) return GIT_ERROR_INVALID_PARAM;
    *pfIsRepo = FALSE_;

    rc = GitFindRepoRoot(pszDir, &pszRoot);
    if (rc == GIT_NO_ERROR) {
        *pfIsRepo = TRUE_;
        free(pszRoot);
    }
    return GIT_NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Rule list                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize a rule list.
 *
 * @param[in] pList  List. Not NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR            Success.
 * @retval GIT_ERROR_INVALID_PARAM pList is NULL.
 */
APIRET APIENTRY GitIgnoreListInit(GITIGNORELIST *pList) {
    if (!pList) return GIT_ERROR_INVALID_PARAM;
    pList->paItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return GIT_NO_ERROR;
}

/**
 * @brief Release all memory owned by a rule list.
 *
 * @param[in] pList  List. May be NULL.
 *
 * @return APIRET
 * @retval GIT_NO_ERROR Success. Also returned for NULL.
 */
APIRET APIENTRY GitIgnoreListFree(GITIGNORELIST *pList) {
    ULONG i;
    if (!pList) return GIT_NO_ERROR;
    for (i = 0; i < pList->ulCount; i++) {
        free(pList->paItems[i].pszPattern);
        free(pList->paItems[i].pszBaseRel);
    }
    free(pList->paItems);
    pList->paItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return GIT_NO_ERROR;
}

static PGITIGNORERULE git_ignore_list_add(GITIGNORELIST *pList) {
    PGITIGNORERULE pItem;
    if (pList->ulCount >= pList->ulCapacity) {
        ULONG ulNewCap = (pList->ulCapacity == 0) ? 16 : pList->ulCapacity * 2;
        PGITIGNORERULE paNew = (PGITIGNORERULE)realloc(pList->paItems,
                              (size_t)ulNewCap * sizeof(GITIGNORERULE));
        if (!paNew) return NULL;
        pList->paItems = paNew;
        pList->ulCapacity = ulNewCap;
    }
    pItem = &pList->paItems[pList->ulCount++];
    memset(pItem, 0, sizeof(*pItem));
    return pItem;
}

/* ------------------------------------------------------------------ */
/* Parsing a single .gitignore line                                    */
/* ------------------------------------------------------------------ */

static void trim_inplace(char *s) {
    char *p = s;
    char *end;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\r' || end[-1] == '\n')) {
        *--end = '\0';
    }
}

static APIRET parse_rule(const char *line, const char *base_rel,
                         GITIGNORELIST *out) {
    PGITIGNORERULE r;
    char buf[1024];
    char *p;
    size_t len;

    if (!line || !line[0]) return GIT_NO_ERROR;

    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_inplace(buf);
    if (buf[0] == '\0') return GIT_NO_ERROR;

    p = buf;

    /* Escaped \# and \! */
    if (p[0] == '\\' && (p[1] == '#' || p[1] == '!')) {
        p++;
    } else if (p[0] == '#') {
        return GIT_NO_ERROR;   /* Comment */
    }

    r = git_ignore_list_add(out);
    if (!r) return GIT_ERROR_OUT_OF_MEMORY;

    if (p[0] == '!') {
        r->fNegate = 1;
        p++;
    }
    if (p[0] == '/') {
        r->fAnchored = 1;
        p++;
    }

    len = strlen(p);
    if (len > 0 && p[len-1] == '/') {
        r->fDirOnly = 1;
        p[len-1] = '\0';
    }

    /* Per gitignore(5): a slash in the middle of a pattern (not only
     * leading or trailing) anchors the pattern to the directory of
     * the .gitignore file. Exception: a leading "**<slash>" means
     * "in any directory" and does not anchor the pattern. */
    if (!r->fAnchored && strchr(p, '/') != NULL) {
        if (strncmp(p, "**/", 3) != 0) {
            r->fAnchored = 1;
        }
    }

    if (p[0] == '\0') {
        /* A rule consisting of just '/' is meaningless. */
        free(r->pszPattern);
        free(r->pszBaseRel);
        out->ulCount--;
        return GIT_NO_ERROR;
    }

    r->pszPattern = dup_str(p);
    if (!r->pszPattern) return GIT_ERROR_OUT_OF_MEMORY;
    r->pszBaseRel = dup_str(base_rel ? base_rel : "");
    if (!r->pszBaseRel) return GIT_ERROR_OUT_OF_MEMORY;

    return GIT_NO_ERROR;
}

static APIRET read_gitignore_file(const char *dir, const char *base_rel,
                                  GITIGNORELIST *out) {
    char path[1024];
    FILE *f;
    char line[1024];
    APIRET rc;

    snprintf(path, sizeof(path), "%s/.gitignore", dir);
    f = fopen(path, "r");
    if (!f) {
        snprintf(path, sizeof(path), "%s\\.gitignore", dir);
        f = fopen(path, "r");
        if (!f) return GIT_NO_ERROR;
    }
    while (fgets(line, sizeof(line), f)) {
        rc = parse_rule(line, base_rel, out);
        if (rc != GIT_NO_ERROR) {
            fclose(f);
            return rc;
        }
    }
    fclose(f);
    return GIT_NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Paths                                                               */
/* ------------------------------------------------------------------ */

static char *rel_path_from(const char *from, const char *to) {
    size_t flen = strlen(from);
    size_t tlen = strlen(to);
    if (flen > tlen) return NULL;
    if (strncmp(from, to, flen) != 0) return NULL;
    if (flen == tlen) return dup_str("");
    if (to[flen] != '/' && to[flen] != '\\') return NULL;
    return dup_str(to + flen + 1);
}

/**
 * @brief Collect .gitignore rules from a directory tree.
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
                                     GITIGNORELIST *pOut) {
    APIRET rc;

    if (!pszTargetDir || !pOut) return GIT_ERROR_INVALID_PARAM;

    GitIgnoreListInit(pOut);

    if (!pszRepoRoot) {
        return read_gitignore_file(pszTargetDir, "", pOut);
    }

    rc = read_gitignore_file(pszRepoRoot, "", pOut);
    if (rc != GIT_NO_ERROR) return rc;

    {
        char *rel = rel_path_from(pszRepoRoot, pszTargetDir);
        if (!rel) return GIT_NO_ERROR;

        if (rel[0] != '\0') {
            char current[1024];
            char base_rel[1024];
            size_t ri = 0;
            size_t rlen = strlen(rel);

            strncpy(current, pszRepoRoot, sizeof(current) - 1);
            current[sizeof(current) - 1] = '\0';
            base_rel[0] = '\0';

            while (ri < rlen) {
                size_t start = ri;
                size_t seglen;
                while (ri < rlen && rel[ri] != '/' && rel[ri] != '\\') ri++;
                seglen = ri - start;
                if (seglen > 0) {
                    size_t clen = strlen(current);
                    size_t blen = strlen(base_rel);
                    if (clen + 1 + seglen + 1 > sizeof(current)) break;
                    if (blen + 1 + seglen + 1 > sizeof(base_rel)) break;

                    current[clen] = '/';
                    memcpy(current + clen + 1, rel + start, seglen);
                    current[clen + 1 + seglen] = '\0';

                    if (blen == 0) {
                        memcpy(base_rel, rel + start, seglen);
                        base_rel[seglen] = '\0';
                    } else {
                        base_rel[blen] = '/';
                        memcpy(base_rel + blen + 1, rel + start, seglen);
                        base_rel[blen + 1 + seglen] = '\0';
                    }

                    rc = read_gitignore_file(current, base_rel, pOut);
                    if (rc != GIT_NO_ERROR) {
                        free(rel);
                        return rc;
                    }
                }
                if (ri < rlen) ri++;
            }
        }
        free(rel);
    }
    return GIT_NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Matching                                                            */
/* ------------------------------------------------------------------ */

static int to_lower(int c) {
#ifdef _WIN32
    return tolower((unsigned char)c);
#else
    return (unsigned char)c;
#endif
}

/* Compare one path component. '*' and '?' do NOT cross '/'. */
static int match_component(const char *pat, const char *str) {
    if (*pat == '\0') return *str == '\0';
    if (*pat == '*') {
        while (1) {
            if (match_component(pat + 1, str)) return 1;
            if (*str == '\0' || *str == '/') return 0;
            str++;
        }
    }
    if (*pat == '?') {
        if (*str == '\0' || *str == '/') return 0;
        return match_component(pat + 1, str + 1);
    }
    if (to_lower(*pat) != to_lower(*str)) return 0;
    return match_component(pat + 1, str + 1);
}

/* Full-pattern match against a path. A single '*' does not cross
 * '/', a double '**' does. */
static int match_path(const char *pat, const char *path) {
    if (pat[0] == '*' && pat[1] == '*') {
        if (pat[2] == '\0') return 1;
        if (pat[2] == '/') {
            if (match_path(pat + 3, path)) return 1;
            while (*path) {
                if (*path == '/') {
                    if (match_path(pat + 3, path + 1)) return 1;
                }
                path++;
            }
            return 0;
        }
    }
    if (*pat == '\0') return *path == '\0';
    if (*pat == '*') {
        while (1) {
            if (match_path(pat + 1, path)) return 1;
            if (*path == '\0' || *path == '/') return 0;
            path++;
        }
    }
    if (*pat == '?') {
        if (*path == '\0' || *path == '/') return 0;
        return match_path(pat + 1, path + 1);
    }
    if (to_lower(*pat) != to_lower(*path)) return 0;
    return match_path(pat + 1, path + 1);
}

/* Try a pattern at every level of the path. Used for non-anchored
 * patterns that do not contain a slash in the middle. */
static int match_any_level(const char *pat, const char *path) {
    if (match_path(pat, path)) return 1;
    while (*path) {
        if (*path == '/') {
            path++;
            if (match_path(pat, path)) return 1;
        } else {
            path++;
        }
    }
    return 0;
}

/* Check whether any path component (a directory name) matches pat.
 * If include_last is 0, the last component is not checked. */
static int path_has_matching_dir(const char *pat, const char *path,
                                 int include_last) {
    while (*path) {
        const char *slash = strchr(path, '/');
        size_t len;
        char seg[512];
        if (!slash && !include_last) return 0;
        len = slash ? (size_t)(slash - path) : strlen(path);
        if (len >= sizeof(seg)) len = sizeof(seg) - 1;
        memcpy(seg, path, len);
        seg[len] = '\0';
        if (match_component(pat, seg)) return 1;
        if (!slash) break;
        path = slash + 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Applying the rules                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Check whether a path is ignored by the collected rules.
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
                             BOOL fIsDir, PBOOL pfIgnored) {
    int ignored = 0;
    ULONG i;

    if (!pRules || !pszRelPath || !pfIgnored)
        return GIT_ERROR_INVALID_PARAM;
    *pfIgnored = FALSE_;
    if (pRules->ulCount == 0) return GIT_NO_ERROR;

    for (i = 0; i < pRules->ulCount; i++) {
        const GITIGNORERULE *r = &pRules->paItems[i];
        const char *sub = pszRelPath;
        int matched = 0;

        /* Skip rules belonging to other directories. 'sub' is the
         * path relative to the directory of this rule's .gitignore. */
        if (r->pszBaseRel && r->pszBaseRel[0]) {
            size_t blen = strlen(r->pszBaseRel);
            if (strncmp(pszRelPath, r->pszBaseRel, blen) != 0) continue;
            if (pszRelPath[blen] != '/' && pszRelPath[blen] != '\\') continue;
            sub = pszRelPath + blen + 1;
        }

        if (r->fDirOnly) {
            const char *pat = r->pszPattern;

            if (r->fAnchored) {
                size_t plen = strlen(pat);
                if (fIsDir && strcmp(sub, pat) == 0) {
                    matched = 1;
                } else if (strncmp(sub, pat, plen) == 0 &&
                           sub[plen] == '/') {
                    /* File or directory inside the matched directory. */
                    matched = 1;
                }
            } else {
                /* "**<slash>foo" - any directory foo at any depth. */
                if (strncmp(pat, "**/", 3) == 0) pat += 3;
                matched = path_has_matching_dir(pat, sub, fIsDir);
            }
        } else {
            if (r->fAnchored) {
                matched = match_path(r->pszPattern, sub);
            } else {
                matched = match_any_level(r->pszPattern, sub);
            }
        }

        if (matched) {
            ignored = r->fNegate ? 0 : 1;
        }
    }

    *pfIgnored = ignored ? TRUE_ : FALSE_;
    return GIT_NO_ERROR;
}
