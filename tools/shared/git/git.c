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
#include "path.h"

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
 * @brief Query whether a directory contains a .git entry.
 *
 * Both '/' and '\\' separators are tried.
 *
 * @param[in] pszPath  Directory path. Not NULL.
 *
 * @return TRUE if .git found, FALSE otherwise.
 */
static BOOL has_git_entry(PCSZ pszPath) {
    CHAR achPath[1024];
    struct stat st;
    snprintf(achPath, sizeof(achPath), "%s/.git", pszPath);
    if (stat(achPath, &st) == 0) return TRUE;
    snprintf(achPath, sizeof(achPath), "%s\\.git", pszPath);
    if (stat(achPath, &st) == 0) return TRUE;
    return FALSE;
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszStartDir is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 * @retval ERROR_FILE_NOT_FOUND     No .git entry found.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY GitFindRepoRoot(PCSZ pszStartDir,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PSZ pszCurrent;
    size_t cbFoundLen;

    if (!pszStartDir) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    pszCurrent = strdup(pszStartDir);
    if (!pszCurrent) return ERROR_NOT_ENOUGH_MEMORY;

    for (;;) {
        PSZ pszSlash;
        PSZ pszBackslash;

        PathRemoveTrailingSeparators(pszCurrent);

        if (has_git_entry(pszCurrent)) break;

        pszSlash = strrchr(pszCurrent, '/');
        pszBackslash = strrchr(pszCurrent, '\\');
        if (pszBackslash && (!pszSlash || pszBackslash > pszSlash))
            pszSlash = pszBackslash;
        if (!pszSlash) {
            free(pszCurrent);
            return ERROR_FILE_NOT_FOUND;
        }

#ifdef _WIN32
        if (pszSlash == pszCurrent + 2 && pszCurrent[1] == ':') {
            free(pszCurrent);
            return ERROR_FILE_NOT_FOUND;
        }
#endif
        if (pszSlash == pszCurrent) {
            free(pszCurrent);
            return ERROR_FILE_NOT_FOUND;
        }

        *pszSlash = '\0';
    }

    cbFoundLen = strlen(pszCurrent);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbFoundLen + 1;
        free(pszCurrent);
        return NO_ERROR;
    }

    if (ulSize < (ULONG)cbFoundLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbFoundLen + 1;
        free(pszCurrent);
        return ERROR_BUFFER_OVERFLOW;
    }

    memcpy(pszBuf, pszCurrent, cbFoundLen);
    pszBuf[cbFoundLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbFoundLen;

    free(pszCurrent);
    return NO_ERROR;
}

/**
 * @brief Query whether a directory is inside a Git repository.
 *
 * @param[in]  pszDir     Directory. Not NULL.
 * @param[out] pfIsRepo   Receiver TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszDir or pfIsRepo is NULL.
 */
APIRET APIENTRY GitQueryIsRepo(PCSZ pszDir, PBOOL pfIsRepo) {
    APIRET rc;

    if (!pszDir || !pfIsRepo) return ERROR_INVALID_PARAMETER;
    *pfIsRepo = FALSE;

    rc = GitFindRepoRoot(pszDir, NULL, 0, NULL);
    if (rc == NO_ERROR) {
        *pfIsRepo = TRUE;
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
APIRET APIENTRY GitIgnoreListInit(PGITIGNORELIST pList) {
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
 * @retval NO_ERROR  Success. Also for NULL.
 */
APIRET APIENTRY GitIgnoreListFree(PGITIGNORELIST pList) {
    ULONG ulIdx;
    if (!pList) return NO_ERROR;
    for (ulIdx = 0; ulIdx < pList->ulCount; ulIdx++) {
        free(pList->paItems[ulIdx].pszPattern);
        free(pList->paItems[ulIdx].pszBaseRel);
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
static PGITIGNORERULE git_ignore_list_add(PGITIGNORELIST pList) {
    PGITIGNORERULE pItem;
    if (pList->ulCount >= pList->ulCapacity) {
        ULONG ulNewCap = (pList->ulCapacity == 0) ? 16
                                                  : pList->ulCapacity * 2;
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
 * @param[in,out] pszStr  String to trim. Not NULL.
 */
static void trim_inplace(PSZ pszStr) {
    PSZ pszPos = pszStr;
    PSZ pszEnd;
    while (*pszPos == ' ' || *pszPos == '\t' ||
           *pszPos == '\r' || *pszPos == '\n') pszPos++;
    if (pszPos != pszStr) memmove(pszStr, pszPos, strlen(pszPos) + 1);
    pszEnd = pszStr + strlen(pszStr);
    while (pszEnd > pszStr && (pszEnd[-1] == ' ' || pszEnd[-1] == '\t' ||
                               pszEnd[-1] == '\r' || pszEnd[-1] == '\n')) {
        *--pszEnd = '\0';
    }
}

/**
 * @brief Parse one .gitignore line into a rule.
 *
 * Recognizes negation ('!'), anchored patterns, dir-only patterns,
 * comments ('#'), escaped '#' and '!'.
 *
 * @param[in]  pszLine   Raw line from the file. Not NULL.
 * @param[in]  pszBaseRel Directory of the .gitignore relative to
 *                        the repository root. May be "".
 * @param[out] pOut      List to append to. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (including empty/comment).
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_rule(PCSZ pszLine, PCSZ pszBaseRel,
                         PGITIGNORELIST pOut) {
    PGITIGNORERULE pRule;
    CHAR achBuf[1024];
    PSZ pszPos;
    size_t cbLen;

    if (!pszLine || !pszLine[0]) return NO_ERROR;

    strncpy(achBuf, pszLine, sizeof(achBuf) - 1);
    achBuf[sizeof(achBuf) - 1] = '\0';
    trim_inplace(achBuf);
    if (achBuf[0] == '\0') return NO_ERROR;

    pszPos = achBuf;

    /* Escaped \# and \! */
    if (pszPos[0] == '\\' && (pszPos[1] == '#' || pszPos[1] == '!')) {
        pszPos++;
    } else if (pszPos[0] == '#') {
        return NO_ERROR;   /* Comment */
    }

    pRule = git_ignore_list_add(pOut);
    if (!pRule) return ERROR_NOT_ENOUGH_MEMORY;

    if (pszPos[0] == '!') {
        pRule->fNegate = TRUE;
        pszPos++;
    }
    if (pszPos[0] == '/') {
        pRule->fAnchored = TRUE;
        pszPos++;
    }

    cbLen = strlen(pszPos);
    if (cbLen > 0 && pszPos[cbLen-1] == '/') {
        pRule->fDirOnly = TRUE;
        pszPos[cbLen-1] = '\0';
    }

    /* Per gitignore(5): a slash in the middle of a pattern (not only
     * leading or trailing) anchors the pattern to the directory of
     * the .gitignore file. Exception: a leading "**<slash>" means
     * "in any directory" and does not anchor the pattern. */
    if (!pRule->fAnchored && strchr(pszPos, '/') != NULL) {
        if (strncmp(pszPos, "**/", 3) != 0) {
            pRule->fAnchored = TRUE;
        }
    }

    if (pszPos[0] == '\0') {
        /* A rule consisting of just '/' is meaningless. */
        free(pRule->pszPattern);
        free(pRule->pszBaseRel);
        pOut->ulCount--;
        return NO_ERROR;
    }

    pRule->pszPattern = strdup(pszPos);
    if (!pRule->pszPattern) return ERROR_NOT_ENOUGH_MEMORY;
    pRule->pszBaseRel = strdup(pszBaseRel ? pszBaseRel : "");
    if (!pRule->pszBaseRel) return ERROR_NOT_ENOUGH_MEMORY;

    return NO_ERROR;
}

/**
 * @brief Read a .gitignore file and append its rules to a list.
 *
 * Both '/' and '\\' separators are tried when opening the file.
 *
 * @param[in]  pszDir      Directory containing the .gitignore. Not
 *                         NULL.
 * @param[in]  pszBaseRel  Directory relative to the repository root.
 * @param[out] pOut        List to append to. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (even if no file exists).
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET read_gitignore_file(PCSZ pszDir, PCSZ pszBaseRel,
                                  PGITIGNORELIST pOut) {
    CHAR achPath[1024];
    FILE *fp;
    CHAR achLine[1024];
    APIRET rc;

    snprintf(achPath, sizeof(achPath), "%s/.gitignore", pszDir);
    fp = fopen(achPath, "r");
    if (!fp) {
        snprintf(achPath, sizeof(achPath), "%s\\.gitignore", pszDir);
        fp = fopen(achPath, "r");
        if (!fp) return NO_ERROR;
    }
    while (fgets(achLine, sizeof(achLine), fp)) {
        rc = parse_rule(achLine, pszBaseRel, pOut);
        if (rc != NO_ERROR) {
            fclose(fp);
            return rc;
        }
    }
    fclose(fp);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Paths                                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Return the relative path of @p pszTo under @p pszFrom.
 *
 * @param[in] pszFrom  Base path. Not NULL.
 * @param[in] pszTo    Full path. Not NULL.
 *
 * @return malloc'd relative path, or NULL if @p pszTo is not under
 *         @p pszFrom.
 */
static PSZ rel_path_from(PCSZ pszFrom, PCSZ pszTo) {
    size_t cbFromLen = strlen(pszFrom);
    size_t cbToLen = strlen(pszTo);
    if (cbFromLen > cbToLen) return NULL;
    if (strncmp(pszFrom, pszTo, cbFromLen) != 0) return NULL;
    if (cbFromLen == cbToLen) return strdup("");
    if (pszTo[cbFromLen] != '/' && pszTo[cbFromLen] != '\\') return NULL;
    return strdup(pszTo + cbFromLen + 1);
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
                                     PGITIGNORELIST pOut) {
    APIRET rc;

    if (!pszTargetDir || !pOut) return ERROR_INVALID_PARAMETER;

    GitIgnoreListInit(pOut);

    if (!pszRepoRoot) {
        return read_gitignore_file(pszTargetDir, "", pOut);
    }

    rc = read_gitignore_file(pszRepoRoot, "", pOut);
    if (rc != NO_ERROR) return rc;

    {
        PSZ pszRel = rel_path_from(pszRepoRoot, pszTargetDir);
        if (!pszRel) return NO_ERROR;

        if (pszRel[0] != '\0') {
            CHAR achCurrent[1024];
            CHAR achBaseRel[1024];
            size_t cbRi = 0;
            size_t cbRlen = strlen(pszRel);

            strncpy(achCurrent, pszRepoRoot, sizeof(achCurrent) - 1);
            achCurrent[sizeof(achCurrent) - 1] = '\0';
            achBaseRel[0] = '\0';

            while (cbRi < cbRlen) {
                size_t cbStart = cbRi;
                size_t cbSegLen;
                while (cbRi < cbRlen && pszRel[cbRi] != '/' &&
                       pszRel[cbRi] != '\\')
                    cbRi++;
                cbSegLen = cbRi - cbStart;
                if (cbSegLen > 0) {
                    size_t cbCurrentLen = strlen(achCurrent);
                    size_t cbBaseRelLen = strlen(achBaseRel);
                    if (cbCurrentLen + 1 + cbSegLen + 1 > sizeof(achCurrent))
                        break;
                    if (cbBaseRelLen + 1 + cbSegLen + 1 > sizeof(achBaseRel))
                        break;

                    achCurrent[cbCurrentLen] = '/';
                    memcpy(achCurrent + cbCurrentLen + 1,
                           pszRel + cbStart, cbSegLen);
                    achCurrent[cbCurrentLen + 1 + cbSegLen] = '\0';

                    if (cbBaseRelLen == 0) {
                        memcpy(achBaseRel, pszRel + cbStart, cbSegLen);
                        achBaseRel[cbSegLen] = '\0';
                    } else {
                        achBaseRel[cbBaseRelLen] = '/';
                        memcpy(achBaseRel + cbBaseRelLen + 1,
                               pszRel + cbStart, cbSegLen);
                        achBaseRel[cbBaseRelLen + 1 + cbSegLen] = '\0';
                    }

                    rc = read_gitignore_file(achCurrent, achBaseRel, pOut);
                    if (rc != NO_ERROR) {
                        free(pszRel);
                        return rc;
                    }
                }
                if (cbRi < cbRlen) cbRi++;
            }
        }
        free(pszRel);
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Matching                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Case-folding helper.
 *
 * Lowercase on Windows, identity on Linux.
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
 * @brief Match a character class starting at @p ppszPat.
 *
 * On entry, @p *ppszPat points to the opening '['. On success,
 * @p *ppszPat is advanced past the closing ']', and @p pfMatched
 * receives TRUE if @p c matches the class (with negation applied).
 *
 * If the opening '[' has no matching ']', the function returns 0
 * and leaves @p *ppszPat unchanged; the caller treats '[' as a
 * literal.
 *
 * Supported syntax:
 *   [abc]    - any of a, b, c
 *   [a-z]    - any character in the range
 *   [!abc]   - any character except a, b, c
 *   [^abc]   - same as [!abc]
 *   []abc]   - ']' can be included as the first character
 *
 * @param[in,out] ppszPat    Pointer to the pattern pointer. Not NULL.
 * @param[in]     c          Character to test.
 * @param[out]    pfMatched  Receiver. Not NULL.
 *
 * @return 1 if a valid class was parsed, 0 otherwise.
 */
static int match_class(PCSZ *ppszPat, int c, PBOOL pfMatched) {
    PCSZ pszPat = *ppszPat;
    BOOL fNegate = FALSE;
    BOOL fMatched = FALSE;

    if (*pszPat != '[') return 0;
    pszPat++;
    if (*pszPat == '!' || *pszPat == '^') {
        fNegate = TRUE;
        pszPat++;
    }
    if (*pszPat == ']') {
        if (c == ']') fMatched = TRUE;
        pszPat++;
    }
    while (*pszPat && *pszPat != ']') {
        if (pszPat[1] == '-' && pszPat[2] && pszPat[2] != ']') {
            if (c >= (UCHAR)pszPat[0] && c <= (UCHAR)pszPat[2])
                fMatched = TRUE;
            pszPat += 3;
        } else {
            if (c == (UCHAR)*pszPat) fMatched = TRUE;
            pszPat++;
        }
    }
    if (*pszPat != ']') {
        /* No closing ']'. Treat '[' as a literal. */
        return 0;
    }
    *ppszPat = pszPat + 1;
    *pfMatched = fNegate ? !fMatched : fMatched;
    return 1;
}

/**
 * @brief Compare one path component.
 *
 * '*', '?' and '[...]' do not cross '/'.
 *
 * @param[in] pszPat  Pattern component. Not NULL.
 * @param[in] pszStr  Path component. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int match_component(PCSZ pszPat, PCSZ pszStr) {
    if (*pszPat == '\0') return *pszStr == '\0';
    if (*pszPat == '*') {
        while (1) {
            if (match_component(pszPat + 1, pszStr)) return 1;
            if (*pszStr == '\0' || *pszStr == '/') return 0;
            pszStr++;
        }
    }
    if (*pszPat == '?') {
        if (*pszStr == '\0' || *pszStr == '/') return 0;
        return match_component(pszPat + 1, pszStr + 1);
    }
    if (*pszPat == '[') {
        BOOL fMatched = FALSE;
        PCSZ pszNext = pszPat;
        if (match_class(&pszNext, (UCHAR)*pszStr, &fMatched)) {
            if (*pszStr == '\0' || *pszStr == '/') return 0;
            if (!fMatched) return 0;
            return match_component(pszNext, pszStr + 1);
        }
        /* Fall through: '[' has no closing ']', treat as literal. */
    }
    if (to_lower(*pszPat) != to_lower(*pszStr)) return 0;
    return match_component(pszPat + 1, pszStr + 1);
}

/**
 * @brief Full-pattern match against a path.
 *
 * A single '*' does not cross '/', a double '**' does.
 *
 * @param[in] pszPat   Pattern. Not NULL.
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int match_path(PCSZ pszPat, PCSZ pszPath) {
    if (pszPat[0] == '*' && pszPat[1] == '*') {
        if (pszPat[2] == '\0') return 1;
        if (pszPat[2] == '/') {
            if (match_path(pszPat + 3, pszPath)) return 1;
            while (*pszPath) {
                if (*pszPath == '/') {
                    if (match_path(pszPat + 3, pszPath + 1)) return 1;
                }
                pszPath++;
            }
            return 0;
        }
    }
    if (*pszPat == '\0') return *pszPath == '\0';
    if (*pszPat == '*') {
        while (1) {
            if (match_path(pszPat + 1, pszPath)) return 1;
            if (*pszPath == '\0' || *pszPath == '/') return 0;
            pszPath++;
        }
    }
    if (*pszPat == '?') {
        if (*pszPath == '\0' || *pszPath == '/') return 0;
        return match_path(pszPat + 1, pszPath + 1);
    }
    if (*pszPat == '[') {
        BOOL fMatched = FALSE;
        PCSZ pszNext = pszPat;
        if (match_class(&pszNext, (UCHAR)*pszPath, &fMatched)) {
            if (*pszPath == '\0' || *pszPath == '/') return 0;
            if (!fMatched) return 0;
            return match_path(pszNext, pszPath + 1);
        }
        /* Fall through: '[' has no closing ']', treat as literal. */
    }
    if (to_lower(*pszPat) != to_lower(*pszPath)) return 0;
    return match_path(pszPat + 1, pszPath + 1);
}

/**
 * @brief Try a pattern at every level of the path.
 *
 * Used for non-anchored patterns that do not contain a slash in
 * the middle.
 *
 * @param[in] pszPat   Pattern. Not NULL.
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int match_any_level(PCSZ pszPat, PCSZ pszPath) {
    if (match_path(pszPat, pszPath)) return 1;
    while (*pszPath) {
        if (*pszPath == '/') {
            pszPath++;
            if (match_path(pszPat, pszPath)) return 1;
        } else {
            pszPath++;
        }
    }
    return 0;
}

/**
 * @brief Query whether any path component matches @p pszPat.
 *
 * If @p fIncludeLast is FALSE, the last component is not checked.
 *
 * @param[in] pszPat        Pattern. Not NULL.
 * @param[in] pszPath       Path. Not NULL.
 * @param[in] fIncludeLast  Whether to check the last component.
 *
 * @return 1 on match, 0 otherwise.
 */
static int path_has_matching_dir(PCSZ pszPat, PCSZ pszPath,
                                 BOOL fIncludeLast) {
    while (*pszPath) {
        PCSZ pszSlash = strchr(pszPath, '/');
        size_t cbLen;
        CHAR achSeg[512];
        if (!pszSlash && !fIncludeLast) return 0;
        cbLen = pszSlash ? (size_t)(pszSlash - pszPath) : strlen(pszPath);
        if (cbLen >= sizeof(achSeg)) cbLen = sizeof(achSeg) - 1;
        memcpy(achSeg, pszPath, cbLen);
        achSeg[cbLen] = '\0';
        if (match_component(pszPat, achSeg)) return 1;
        if (!pszSlash) break;
        pszPath = pszSlash + 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Applying the rules                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Query whether a path is ignored by the collected rules.
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
 * @param[in]  fIsDir      TRUE for a directory, FALSE for a file.
 * @param[out] pfIgnored   Receiver TRUE / FALSE. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 */
APIRET APIENTRY GitQueryIsIgnored(const GITIGNORELIST *pRules,
                                  PCSZ pszRelPath,
                                  BOOL fIsDir, PBOOL pfIgnored) {
    BOOL fIgnored = FALSE;
    ULONG ulIdx;

    if (!pRules || !pszRelPath || !pfIgnored)
        return ERROR_INVALID_PARAMETER;
    *pfIgnored = FALSE;
    if (pRules->ulCount == 0) return NO_ERROR;

    for (ulIdx = 0; ulIdx < pRules->ulCount; ulIdx++) {
        const GITIGNORERULE *pRule = &pRules->paItems[ulIdx];
        PCSZ pszSub = pszRelPath;
        BOOL fMatched = FALSE;

        /* Skip rules belonging to other directories. 'pszSub' is the
         * path relative to the directory of this rule's .gitignore. */
        if (pRule->pszBaseRel && pRule->pszBaseRel[0]) {
            size_t cbBaseRel = strlen(pRule->pszBaseRel);
            if (strncmp(pszRelPath, pRule->pszBaseRel, cbBaseRel) != 0)
                continue;
            if (pszRelPath[cbBaseRel] != '/' &&
                pszRelPath[cbBaseRel] != '\\')
                continue;
            pszSub = pszRelPath + cbBaseRel + 1;
        }

        if (pRule->fDirOnly) {
            PCSZ pszPat = pRule->pszPattern;

            if (pRule->fAnchored) {
                size_t cbPatLen = strlen(pszPat);
                if (fIsDir && strcmp(pszSub, pszPat) == 0) {
                    fMatched = TRUE;
                } else if (strncmp(pszSub, pszPat, cbPatLen) == 0 &&
                           pszSub[cbPatLen] == '/') {
                    /* File or directory inside the matched directory. */
                    fMatched = TRUE;
                }
            } else {
                /* "**<slash>foo" - any directory foo at any depth. */
                if (strncmp(pszPat, "**/", 3) == 0) pszPat += 3;
                fMatched = path_has_matching_dir(pszPat, pszSub,
                                                 fIsDir) ? TRUE : FALSE;
            }
        } else {
            if (pRule->fAnchored) {
                fMatched = match_path(pRule->pszPattern, pszSub)
                               ? TRUE : FALSE;
            } else {
                fMatched = match_any_level(pRule->pszPattern, pszSub)
                               ? TRUE : FALSE;
            }
        }

        if (fMatched) {
            fIgnored = pRule->fNegate ? FALSE : TRUE;
        }
    }

    *pfIgnored = fIgnored ? TRUE : FALSE;
    return NO_ERROR;
}
