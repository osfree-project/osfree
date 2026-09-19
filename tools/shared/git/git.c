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
 *
 * References:
 *   - gitignore(5).
 *     https://git-scm.com/docs/gitignore
 */

/* ------------------------------------------------------------------ */
/* Utilities                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Check whether a directory contains a .git entry.
 *
 * Both '/' and '\\' separators are tried.
 *
 * @param[in] path  Directory path. Not NULL.
 *
 * @return 1 if .git found, 0 otherwise.
 */
static int has_git_entry(const char *path) {
    char p[1024];
    struct stat st;
    snprintf(p, sizeof(p), "%s/.git", path);
    if (stat(p, &st) == 0) return 1;
    snprintf(p, sizeof(p), "%s\\.git", path);
    if (stat(p, &st) == 0) return 1;
    return 0;
}

/**
 * @brief Remove trailing path separators.
 *
 * A bare drive letter like "C:" is left unchanged: the buffer
 * allocated by strdup has only len + 1 bytes, so appending a
 * separator would require a write past the end. Windows accepts
 * "C:" as the current directory of drive C, so the transformation
 * is not required.
 *
 * @param[in,out] s  Path buffer to trim. Not NULL.
 */
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
        /* Bare drive letter: do not extend to "C:\", the buffer is
         * only len + 1 bytes. See the function comment. */
        return;
    }
#endif
}

/* ------------------------------------------------------------------ */
/* Repository root discovery                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Find the root of the Git repository containing a directory.
 *
 * Walks up from @p pszStartDir looking for an entry named ".git"
 * (directory or file). Both '/' and '\\' are treated as path
 * separators.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *     NUL) is written.
 *   - ulSize large enough: value copied and NUL-terminated;
 *     *pulUsed is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszStartDir  Starting directory. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success. Root directory returned.
 * @retval ERROR_INVALID_PARAMETER  pszStartDir is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 * @retval ERROR_FILE_NOT_FOUND     No .git entry found.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY GitFindRepoRoot(PCSZ pszStartDir,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    char *current;
    size_t found_len;

    if (!pszStartDir) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    current = strdup(pszStartDir);
    if (!current) return ERROR_NOT_ENOUGH_MEMORY;

    for (;;) {
        char *slash;
        char *backslash;

        trim_separators(current);

        if (has_git_entry(current)) break;

        slash = strrchr(current, '/');
        backslash = strrchr(current, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (!slash) {
            free(current);
            return ERROR_FILE_NOT_FOUND;
        }

#ifdef _WIN32
        if (slash == current + 2 && current[1] == ':') {
            free(current);
            return ERROR_FILE_NOT_FOUND;
        }
#endif
        if (slash == current) {
            free(current);
            return ERROR_FILE_NOT_FOUND;
        }

        *slash = '\0';
    }

    found_len = strlen(current);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(found_len + 1);
        free(current);
        return NO_ERROR;
    }

    if (ulSize < found_len + 1) {
        if (pulUsed) *pulUsed = (ULONG)(found_len + 1);
        free(current);
        return ERROR_BUFFER_OVERFLOW;
    }

    memcpy(pszBuf, current, found_len);
    pszBuf[found_len] = '\0';
    if (pulUsed) *pulUsed = (ULONG)found_len;

    free(current);
    return NO_ERROR;
}

/**
 * @brief Check whether a directory is inside a Git repository.
 *
 * @param[in]  pszDir     Directory. Not NULL.
 * @param[out] pfIsRepo   Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszDir or pfIsRepo is NULL.
 */
APIRET APIENTRY GitIsRepo(PCSZ pszDir, PBOOL pfIsRepo) {
    APIRET rc;

    if (!pszDir || !pfIsRepo) return ERROR_INVALID_PARAMETER;
    *pfIsRepo = FALSE_;

    rc = GitFindRepoRoot(pszDir, NULL, 0, NULL);
    if (rc == NO_ERROR) {
        *pfIsRepo = TRUE_;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Rule list                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize a rule list.
 *
 * Sets all fields to zero. No memory is allocated.
 *
 * @param[in] pList  List. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList is NULL.
 */
APIRET APIENTRY GitIgnoreListInit(GITIGNORELIST *pList) {
    if (!pList) return ERROR_INVALID_PARAMETER;
    pList->paItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return NO_ERROR;
}

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
 * @retval NO_ERROR Success. Also returned for NULL.
 */
APIRET APIENTRY GitIgnoreListFree(GITIGNORELIST *pList) {
    ULONG i;
    if (!pList) return NO_ERROR;
    for (i = 0; i < pList->ulCount; i++) {
        free(pList->paItems[i].pszPattern);
        free(pList->paItems[i].pszBaseRel);
    }
    free(pList->paItems);
    pList->paItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return NO_ERROR;
}

/**
 * @brief Append one rule to a list, growing it if necessary.
 *
 * @param[in,out] pList  List. Not NULL.
 *
 * @return Pointer to the new rule slot, or NULL on OOM.
 */
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

/**
 * @brief Remove leading and trailing whitespace in place.
 *
 * @param[in,out] s  String to trim. Not NULL.
 */
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

/**
 * @brief Parse one .gitignore line into a rule.
 *
 * Recognizes negation ('!'), anchored patterns, dir-only patterns,
 * comments ('#'), escaped '#' and '!'.
 *
 * @param[in]  line      Raw line from the file. Not NULL.
 * @param[in]  base_rel  Directory of the .gitignore relative to
 *                       the repository root. May be "".
 * @param[out] out       List to append to. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (including empty/comment).
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
static APIRET parse_rule(const char *line, const char *base_rel,
                         GITIGNORELIST *out) {
    PGITIGNORERULE r;
    char buf[1024];
    char *p;
    size_t len;

    if (!line || !line[0]) return NO_ERROR;

    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    trim_inplace(buf);
    if (buf[0] == '\0') return NO_ERROR;

    p = buf;

    /* Escaped \# and \! */
    if (p[0] == '\\' && (p[1] == '#' || p[1] == '!')) {
        p++;
    } else if (p[0] == '#') {
        return NO_ERROR;   /* Comment */
    }

    r = git_ignore_list_add(out);
    if (!r) return ERROR_NOT_ENOUGH_MEMORY;

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
        return NO_ERROR;
    }

    r->pszPattern = strdup(p);
    if (!r->pszPattern) return ERROR_NOT_ENOUGH_MEMORY;
    r->pszBaseRel = strdup(base_rel ? base_rel : "");
    if (!r->pszBaseRel) return ERROR_NOT_ENOUGH_MEMORY;

    return NO_ERROR;
}

/**
 * @brief Read a .gitignore file and append its rules to a list.
 *
 * Both '/' and '\\' separators are tried when opening the file.
 *
 * @param[in]  dir       Directory containing the .gitignore. Not NULL.
 * @param[in]  base_rel  Directory relative to the repository root.
 * @param[out] out       List to append to. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (even if no file exists).
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
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
        if (!f) return NO_ERROR;
    }
    while (fgets(line, sizeof(line), f)) {
        rc = parse_rule(line, base_rel, out);
        if (rc != NO_ERROR) {
            fclose(f);
            return rc;
        }
    }
    fclose(f);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Paths                                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Return the relative path of @p to under @p from.
 *
 * @param[in] from  Base path. Not NULL.
 * @param[in] to    Full path. Not NULL.
 *
 * @return malloc'd relative path, or NULL if @p to is not under
 *         @p from.
 */
static char *rel_path_from(const char *from, const char *to) {
    size_t flen = strlen(from);
    size_t tlen = strlen(to);
    if (flen > tlen) return NULL;
    if (strncmp(from, to, flen) != 0) return NULL;
    if (flen == tlen) return strdup("");
    if (to[flen] != '/' && to[flen] != '\\') return NULL;
    return strdup(to + flen + 1);
}

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
 * @retval NO_ERROR                 Success (even if no .gitignore
 *                                  files were found).
 * @retval ERROR_INVALID_PARAMETER  pszTargetDir or pOut is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 */
APIRET APIENTRY GitCollectGitignores(PCSZ pszRepoRoot, PCSZ pszTargetDir,
                                     GITIGNORELIST *pOut) {
    APIRET rc;

    if (!pszTargetDir || !pOut) return ERROR_INVALID_PARAMETER;

    GitIgnoreListInit(pOut);

    if (!pszRepoRoot) {
        return read_gitignore_file(pszTargetDir, "", pOut);
    }

    rc = read_gitignore_file(pszRepoRoot, "", pOut);
    if (rc != NO_ERROR) return rc;

    {
        char *rel = rel_path_from(pszRepoRoot, pszTargetDir);
        if (!rel) return NO_ERROR;

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
                    if (rc != NO_ERROR) {
                        free(rel);
                        return rc;
                    }
                }
                if (ri < rlen) ri++;
            }
        }
        free(rel);
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Matching                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Case-folding helper: lowercase on Windows, identity on
 *        Linux.
 *
 * @param[in] c  Character.
 *
 * @return Mapped character.
 */
static int to_lower(int c) {
#ifdef _WIN32
    return tolower((unsigned char)c);
#else
    return (unsigned char)c;
#endif
}

/**
 * @brief Match a character class starting at @p ppat.
 *
 * On entry, @p *ppat points to the opening '['. On success, @p *ppat
 * is advanced past the closing ']', and @p *pmatched receives 1 if
 * @p c matches the class (with negation applied) and 0 otherwise.
 *
 * If the opening '[' has no matching ']', the function returns 0 and
 * leaves @p *ppat unchanged; the caller treats '[' as a literal.
 *
 * Supported syntax:
 *   [abc]    - any of a, b, c
 *   [a-z]    - any character in the range
 *   [!abc]   - any character except a, b, c
 *   [^abc]   - same as [!abc]
 *   []abc]   - ']' can be included as the first character
 *
 * @param[in,out] ppat      Pointer to the pattern pointer. Not NULL.
 * @param[in]     c         Character to test.
 * @param[out]    pmatched  Receiver: 1 on match, 0 otherwise.
 *
 * @return 1 if a valid class was parsed, 0 if the pattern does not
 *         contain a valid class.
 */
static int match_class(const char **ppat, int c, int *pmatched) {
    const char *pat = *ppat;
    int negate = 0;
    int matched = 0;

    if (*pat != '[') return 0;
    pat++;
    if (*pat == '!' || *pat == '^') {
        negate = 1;
        pat++;
    }
    if (*pat == ']') {
        if (c == ']') matched = 1;
        pat++;
    }
    while (*pat && *pat != ']') {
        if (pat[1] == '-' && pat[2] && pat[2] != ']') {
            if (c >= (unsigned char)pat[0] &&
                c <= (unsigned char)pat[2])
                matched = 1;
            pat += 3;
        } else {
            if (c == (unsigned char)*pat) matched = 1;
            pat++;
        }
    }
    if (*pat != ']') {
        /* No closing ']'. Treat '[' as a literal. */
        return 0;
    }
    *ppat = pat + 1;
    *pmatched = negate ? !matched : matched;
    return 1;
}

/**
 * @brief Compare one path component.
 *
 * '*', '?' and '[...]' do not cross '/'.
 *
 * @param[in] pat  Pattern component. Not NULL.
 * @param[in] str  Path component. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
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
    if (*pat == '[') {
        int matched;
        const char *next = pat;
        if (match_class(&next, (unsigned char)*str, &matched)) {
            if (*str == '\0' || *str == '/') return 0;
            if (!matched) return 0;
            return match_component(next, str + 1);
        }
        /* Fall through: '[' has no closing ']', treat as literal. */
    }
    if (to_lower(*pat) != to_lower(*str)) return 0;
    return match_component(pat + 1, str + 1);
}

/**
 * @brief Full-pattern match against a path.
 *
 * A single '*' does not cross '/', a double '**' does.
 *
 * @param[in] pat   Pattern. Not NULL.
 * @param[in] path  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
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
    if (*pat == '[') {
        int matched;
        const char *next = pat;
        if (match_class(&next, (unsigned char)*path, &matched)) {
            if (*path == '\0' || *path == '/') return 0;
            if (!matched) return 0;
            return match_path(next, path + 1);
        }
        /* Fall through: '[' has no closing ']', treat as literal. */
    }
    if (to_lower(*pat) != to_lower(*path)) return 0;
    return match_path(pat + 1, path + 1);
}

/**
 * @brief Try a pattern at every level of the path.
 *
 * Used for non-anchored patterns that do not contain a slash in
 * the middle.
 *
 * @param[in] pat   Pattern. Not NULL.
 * @param[in] path  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
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

/**
 * @brief Check whether any path component matches @p pat.
 *
 * If @p include_last is 0, the last component is not checked.
 *
 * @param[in] pat           Pattern. Not NULL.
 * @param[in] path          Path. Not NULL.
 * @param[in] include_last  Whether to check the last component.
 *
 * @return 1 on match, 0 otherwise.
 */
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY GitIsIgnored(const GITIGNORELIST *pRules, PCSZ pszRelPath,
                             BOOL fIsDir, PBOOL pfIgnored) {
    int ignored = 0;
    ULONG i;

    if (!pRules || !pszRelPath || !pfIgnored)
        return ERROR_INVALID_PARAMETER;
    *pfIgnored = FALSE_;
    if (pRules->ulCount == 0) return NO_ERROR;

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
    return NO_ERROR;
}
