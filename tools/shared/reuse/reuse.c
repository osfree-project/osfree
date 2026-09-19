/* reuse.c - REUSE 3.3 project resolver, OS/2 API style
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

#include "reuse.h"
#include "reuse_internal.h"
#include "reuse_toml.h"
#include "dep5.h"
#include "git.h"
#include "spdx_tag.h"

/**
 * @file reuse.c
 * @brief Implementation of the REUSE 3.3 project resolver.
 *
 * Conforms to:
 *   - https://reuse.software/spec-3.3/
 *
 * The resolver discovers REUSE.toml files from the repository root
 * down to the target directory, reads .reuse/dep5 at the repository
 * root if present, and combines all matched annotations with the
 * in-file sources (sidecar and SPDX tags) according to the
 * precedence rules of REUSE 3.3 §4.1.2.
 */

/* ==================================================================
 * Small helpers
 * ================================================================== */

/**
 * @brief Duplicate a byte range into a fresh NUL-terminated string.
 *
 * @param[in] s  Source bytes. Not NULL.
 * @param[in] n  Number of bytes to copy.
 *
 * @return malloc'd string, or NULL on allocation failure.
 */
static char *dup_n(const char *s, size_t n) {
    char *r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

/**
 * @brief Duplicate a NUL-terminated string.
 *
 * @param[in] s  Source string. Not NULL.
 *
 * @return malloc'd copy, or NULL on allocation failure.
 */
static char *dup_str(const char *s) {
    return dup_n(s, strlen(s));
}

/**
 * @brief Free a heap block through a pointer-to-pointer and clear it.
 *
 * Passing NULL or a pointer to a NULL value is a no-op.
 *
 * @param[in,out] pp  Pointer to the block pointer. May be NULL.
 */
static void safe_free(void **pp) {
    if (pp && *pp) { free(*pp); *pp = NULL; }
}

/* ==================================================================
 * Path handling
 * ================================================================== */

/**
 * @brief Compare two path characters.
 *
 * Case-insensitive on Windows, case-sensitive elsewhere.
 *
 * @param[in] a  First character.
 * @param[in] b  Second character.
 *
 * @return 1 if the characters are equal under the platform rules,
 *         0 otherwise.
 */
static int path_char_eq(int a, int b) {
#ifdef _WIN32
    return tolower((unsigned char)a) == tolower((unsigned char)b);
#else
    return (unsigned char)a == (unsigned char)b;
#endif
}

/**
 * @brief Compare a string against a prefix under path_char_eq rules.
 *
 * @param[in] s       String to test. Not NULL.
 * @param[in] prefix  Prefix. Not NULL.
 * @param[in] n       Prefix length.
 *
 * @return 1 if @p s starts with @p prefix, 0 otherwise.
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
 * @brief Return the path of @p filename relative to @p base.
 *
 * If @p filename does not start with @p base followed by a separator
 * or the end of string, @p filename is returned unchanged.
 *
 * @param[in] base      Base directory. May be NULL or empty.
 * @param[in] filename  Full path. Not NULL.
 *
 * @return Pointer to the relative part, or @p filename unchanged.
 */
static const char *rel_to_dir(const char *base, const char *filename) {
    size_t blen;
    if (!base || !base[0]) return filename;
    blen = strlen(base);
    if (path_prefix_eq(filename, base, blen)) {
        const char *rest = filename + blen;
        if (*rest == '\0') return rest;
        if (*rest != '/' && *rest != '\\') return filename;
        while (*rest == '/' || *rest == '\\') rest++;
        return rest;
    }
    return filename;
}

/* ==================================================================
 * Pattern matching (* and **)
 * ================================================================== */

/**
 * @brief Match one path component against a pattern component.
 *
 * '*' matches any sequence of characters that does not cross '/'.
 * The pattern and the string must both be free of '/'.
 *
 * @param[in] pat  Pattern component. Not NULL.
 * @param[in] str  String component. Not NULL.
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
    if (*str == '\0' || *str == '/') return 0;
    if (*pat != *str) return 0;
    return match_component(pat + 1, str + 1);
}

/**
 * @brief Match a full path against a pattern.
 *
 * In addition to '*' and '?' handled by match_component, supports
 * the '**' wildcard at a component boundary: '**' alone matches any
 * suffix, and '**<slash>' matches any prefix of path components.
 *
 * @param[in] pat   Pattern. Not NULL.
 * @param[in] path  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int match_path(const char *pat, const char *path) {
    const char *pat_slash;
    const char *path_slash;

    if (pat[0] == '*' && pat[1] == '*') {
        if (pat[2] == '\0') return 1;
        if (pat[2] == '/') {
            const char *rest = pat + 3;
            if (match_path(rest, path)) return 1;
            while ((path_slash = strchr(path, '/')) != NULL) {
                path = path_slash + 1;
                if (match_path(rest, path)) return 1;
            }
            return 0;
        }
    }
    pat_slash  = strchr(pat, '/');
    path_slash = strchr(path, '/');
    if (!pat_slash) {
        if (path_slash) return 0;
        return match_component(pat, path);
    }
    if (!path_slash) return 0;
    {
        size_t plen = (size_t)(pat_slash - pat);
        size_t flen = (size_t)(path_slash - path);
        char pat_seg[256];
        char path_seg[256];
        if (plen >= sizeof(pat_seg) || flen >= sizeof(path_seg)) return 0;
        memcpy(pat_seg, pat, plen); pat_seg[plen] = '\0';
        memcpy(path_seg, path, flen); path_seg[flen] = '\0';
        if (!match_component(pat_seg, path_seg)) return 0;
        return match_path(pat_slash + 1, path_slash + 1);
    }
}

/**
 * @brief Convenience wrapper around match_path.
 *
 * @param[in] pattern   Pattern. Not NULL.
 * @param[in] filename  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int matches_pattern(const char *pattern, const char *filename) {
    return match_path(pattern, filename);
}

/* ==================================================================
 * Error recording
 * ================================================================== */

/**
 * @brief Append one REUSEERR record to a project handle.
 *
 * @param[in,out] pd           Project handle. Not NULL.
 * @param[in]     ulCode       REUSE_ERROR_* code.
 * @param[in]     ulSeverity   One of REUSE_SEV_*.
 * @param[in]     pszFile      Offending file, or NULL.
 * @param[in]     ulLine       Source line, 0 if not applicable.
 * @param[in]     pszDetail    Short human-readable detail, or NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int err_add(PREUSETREE pd, ULONG ulCode, ULONG ulSeverity,
                   PCSZ pszFile, ULONG ulLine, PCSZ pszDetail) {
    REUSEERR *pe;
    if (pd->ulErrorCount >= pd->ulErrorCapacity) {
        ULONG ncap = pd->ulErrorCapacity ? pd->ulErrorCapacity * 2 : 8;
        REUSEERR *na = (REUSEERR*)realloc(pd->paErrors,
                                          (size_t)ncap * sizeof(REUSEERR));
        if (!na) return -1;
        pd->paErrors = na;
        pd->ulErrorCapacity = ncap;
    }
    pe = &pd->paErrors[pd->ulErrorCount++];
    memset(pe, 0, sizeof(*pe));
    pe->ulCode = ulCode;
    pe->ulSeverity = ulSeverity;
    pe->ulLine = ulLine;
    if (pszFile)   strncpy(pe->achFile, pszFile, sizeof(pe->achFile) - 1);
    if (pszDetail) strncpy(pe->achDetail, pszDetail, sizeof(pe->achDetail) - 1);
    return 0;
}

/* ==================================================================
 * Config list
 * ================================================================== */

/**
 * @brief Append one config entry to a project handle.
 *
 * @param[in,out] pd            Project handle. Not NULL.
 * @param[in]     nKind         REUSECFG_KIND_*.
 * @param[in]     pszSourceDir  Directory containing the config.
 * @param[in]     nDepth        Depth from the repository root.
 * @param[in]     hToml         TOML handle, or NULLHANDLE.
 * @param[in]     hDep5         DEP5 handle, or NULLHANDLE.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int cfg_add(PREUSETREE pd, int nKind, PCSZ pszSourceDir, int nDepth,
                   HREUSETOML hToml, HDEP5DOC hDep5) {
    REUSECFG *pc;
    if (pd->ulCount >= pd->ulCapacity) {
        ULONG ncap = pd->ulCapacity ? pd->ulCapacity * 2 : 8;
        REUSECFG *na = (REUSECFG*)realloc(pd->paCfgs,
                                          (size_t)ncap * sizeof(REUSECFG));
        if (!na) return -1;
        pd->paCfgs = na;
        pd->ulCapacity = ncap;
    }
    pc = &pd->paCfgs[pd->ulCount++];
    memset(pc, 0, sizeof(*pc));
    pc->nKind = nKind;
    pc->pszSourceDir = dup_str(pszSourceDir);
    pc->nDepth = nDepth;
    pc->hToml = hToml;
    pc->hDep5 = hDep5;
    return pc->pszSourceDir ? 0 : -1;
}

/**
 * @brief Release one config entry.
 *
 * @param[in,out] pc  Config entry. Not NULL.
 */
static void cfg_free(REUSECFG *pc) {
    if (!pc) return;
    safe_free((void**)&pc->pszSourceDir);
    if (pc->hToml != NULLHANDLE) ReuseClose(pc->hToml);
    if (pc->hDep5 != NULLHANDLE) Dep5Close(pc->hDep5);
    memset(pc, 0, sizeof(*pc));
}

/**
 * @brief Release a project handle and all owned resources.
 *
 * @param[in] pd  Project handle. May be NULL.
 */
static void doc_free(PREUSETREE pd) {
    ULONG i;
    if (!pd) return;
    for (i = 0; i < pd->ulCount; i++) cfg_free(&pd->paCfgs[i]);
    free(pd->paCfgs);
    free(pd->paErrors);
    free(pd->pszProjectDir);
    free(pd->pszRepoRoot);
    free(pd);
}

/* ==================================================================
 * Discovery
 * ================================================================== */

/**
 * @brief Join a directory and a name with the platform separator.
 *
 * @param[out] dst       Destination buffer. Not NULL.
 * @param[in]  dst_size  Size of @p dst.
 * @param[in]  dir       Directory. Not NULL.
 * @param[in]  name      Entry name. Not NULL.
 */
static void join_path(char *dst, size_t dst_size,
                      const char *dir, const char *name) {
    size_t len = strlen(dir);
#ifdef __LINUX__
    if (len == 0) snprintf(dst, dst_size, "%s", name);
    else snprintf(dst, dst_size, "%s/%s", dir, name);
#else
    if (len == 0) snprintf(dst, dst_size, "%s", name);
    else snprintf(dst, dst_size, "%s\\%s", dir, name);
#endif
}

/**
 * @brief Register one REUSE.toml file as a config, if it parses.
 *
 * On parse failure, records a diagnostic and returns 0 so that the
 * caller continues with other files. Returns -1 only on fatal
 * errors (out of memory).
 *
 * @param[in,out] pd            Project handle. Not NULL.
 * @param[in]     pszPath       Path to REUSE.toml. Not NULL.
 * @param[in]     pszSourceDir  Directory containing the file.
 * @param[in]     nDepth        Depth from the repository root.
 *
 * @return 0 on success or recorded parse failure, -1 on fatal error.
 */
static int try_add_toml(PREUSETREE pd, PCSZ pszPath,
                        PCSZ pszSourceDir, int nDepth) {
    HREUSETOML hToml = NULLHANDLE;
    APIRET rc = ReuseOpen(pszPath, &hToml);
    if (rc == REUSE_NO_ERROR) {
        if (cfg_add(pd, REUSECFG_KIND_TOML, pszSourceDir, nDepth,
                    hToml, NULLHANDLE) != 0) {
            ReuseClose(hToml);
            return -1;
        }
        return 0;
    }
    /* Record and skip */
    {
        ULONG sev = REUSE_SEV_ERROR;
        const char *detail = "REUSE.toml parse failure";
        switch (rc) {
            case REUSE_ERROR_VERSION_MISSING:  detail = "'version = 1' missing"; break;
            case REUSE_ERROR_VERSION_NOT_INT:  detail = "'version' is not an integer"; break;
            case REUSE_ERROR_VERSION_UNSUP:    detail = "unsupported 'version' value"; break;
            case REUSE_ERROR_ANNOT_NO_PATH:    detail = "[[annotations]] without 'path'"; break;
            case REUSE_ERROR_ANNOT_BAD_PATH:   detail = "'path' has wrong type"; break;
            case REUSE_ERROR_ANNOT_BAD_FIELD:  detail = "field has wrong type"; break;
            case REUSE_ERROR_SYNTAX:           detail = "syntax error"; break;
            case REUSE_ERROR_OPEN_FAILED:      detail = "cannot open"; break;
            case REUSE_ERROR_READ_FAILED:      detail = "read error"; break;
            case REUSE_ERROR_OUT_OF_MEMORY:    detail = "out of memory"; break;
        }
        err_add(pd, rc, sev, pszPath, 0, detail);
    }
    return 0;
}

/**
 * @brief Discover REUSE.toml files from the repo root down to target.
 *
 * Files are registered in increasing depth order so that the
 * "closest wins" rule can be applied later.
 *
 * @param[in,out] pd           Project handle. Not NULL.
 * @param[in]     pszRepoRoot  Repository root, or NULL.
 * @param[in]     pszTarget    Target directory. Not NULL.
 *
 * @return 0 on success, -1 on fatal error.
 */
static int discover_tomls(PREUSETREE pd, PCSZ pszRepoRoot, PCSZ pszTarget) {
    char path[2048];
    char current[2048];
    FILE *f;
    const char *rel;
    int depth = 0;

    if (!pszRepoRoot) {
        /* No repo: only the target directory */
        join_path(path, sizeof(path), pszTarget, "REUSE.toml");
        f = fopen(path, "r");
        if (f) { fclose(f); try_add_toml(pd, path, pszTarget, 0); }
        return 0;
    }

    /* Root-level REUSE.toml */
    join_path(path, sizeof(path), pszRepoRoot, "REUSE.toml");
    f = fopen(path, "r");
    if (f) {
        fclose(f);
        try_add_toml(pd, path, pszRepoRoot, depth);
    }
    depth++;

    /* Walk down to the target directory */
    rel = rel_to_dir(pszRepoRoot, pszTarget);
    if (!rel || !rel[0] || rel == pszTarget) return 0;

    strncpy(current, pszRepoRoot, sizeof(current) - 1);
    current[sizeof(current) - 1] = '\0';

    {
        size_t ri = 0, rlen = strlen(rel);
        while (ri < rlen) {
            size_t start = ri;
            size_t seglen;
            while (ri < rlen && rel[ri] != '/' && rel[ri] != '\\') ri++;
            seglen = ri - start;
            if (seglen > 0) {
                size_t clen = strlen(current);
                if (clen + 1 + seglen + 1 > sizeof(current)) break;
                current[clen] = '/';
                memcpy(current + clen + 1, rel + start, seglen);
                current[clen + 1 + seglen] = '\0';

                join_path(path, sizeof(path), current, "REUSE.toml");
                f = fopen(path, "r");
                if (f) {
                    fclose(f);
                    try_add_toml(pd, path, current, depth);
                }
                depth++;
            }
            if (ri < rlen) ri++;
        }
    }
    return 0;
}

/**
 * @brief Discover and register .reuse/dep5 at the repository root.
 *
 * @param[in,out] pd           Project handle. Not NULL.
 * @param[in]     pszRepoRoot  Repository root, or NULL.
 *
 * @return 0 on success, -1 on fatal error.
 */
static int discover_dep5(PREUSETREE pd, PCSZ pszRepoRoot) {
    char path[1024];
    FILE *f;
    HDEP5DOC hDep5 = NULLHANDLE;
    APIRET rc;

    if (!pszRepoRoot || !pszRepoRoot[0]) return 0;
#ifdef __LINUX__
    snprintf(path, sizeof(path), "%s/.reuse/dep5", pszRepoRoot);
#else
    snprintf(path, sizeof(path), "%s\\.reuse\\dep5", pszRepoRoot);
#endif
    f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);

    rc = Dep5Open(path, &hDep5);
    if (rc == DEP5_NO_ERROR) {
        if (cfg_add(pd, REUSECFG_KIND_DEP5, pszRepoRoot, -1,
                    NULLHANDLE, hDep5) != 0) {
            Dep5Close(hDep5);
            return -1;
        }
        return 0;
    }
    {
        const char *detail = "DEP5 parse failure";
        switch (rc) {
            case DEP5_ERROR_INVALID_SYNTAX: detail = "syntax error"; break;
            case DEP5_ERROR_OPEN_FAILED:    detail = "cannot open"; break;
            case DEP5_ERROR_READ_FAILED:    detail = "read error"; break;
            case DEP5_ERROR_OUT_OF_MEMORY:  detail = "out of memory"; break;
        }
        err_add(pd, REUSE_ERROR_SYNTAX, REUSE_SEV_ERROR, path, 0, detail);
    }
    return 0;
}

/* ==================================================================
 * MATCH: one matched annotation from a single config
 * ================================================================== */

/**
 * @struct _MATCH
 * @brief One matched annotation from a single configuration source.
 *
 * All string fields are owned by this structure and released by
 * match_clear. NULL means the corresponding field was not present in
 * the source.
 */
typedef struct _MATCH {
    int   nPrecedence;                  /**< REUSE_PRECEDENCE_* value.     */
    int   nDepth;                       /**< Depth from repository root.   */
    int   nOrderInFile;                 /**< Order of appearance.          */

    char *pszLicense;                   /**< SPDX license expression.      */
    char *pszCopyright;                 /**< Copyright notices.            */
    char *pszContributors;              /**< '\n'-separated contributors.  */
    char *pszPackageName;               /**< SPDX-PackageName.             */
    char *pszPackageSupplier;           /**< SPDX-PackageSupplier.         */
    char *pszPackageDownloadLocation;   /**< SPDX-PackageDownloadLocation. */
    char *pszPackageComment;            /**< SPDX-PackageComment.          */
} MATCH;

/**
 * @brief Release all owned strings of a MATCH and clear its fields.
 *
 * @param[in,out] pm  Match. Not NULL.
 */
static void match_clear(MATCH *pm) {
    safe_free((void**)&pm->pszLicense);
    safe_free((void**)&pm->pszCopyright);
    safe_free((void**)&pm->pszContributors);
    safe_free((void**)&pm->pszPackageName);
    safe_free((void**)&pm->pszPackageSupplier);
    safe_free((void**)&pm->pszPackageDownloadLocation);
    safe_free((void**)&pm->pszPackageComment);
    pm->nPrecedence = 0;
    pm->nDepth = 0;
    pm->nOrderInFile = 0;
}

/**
 * @brief Decide whether @p a outranks @p b.
 *
 * Order of comparison: precedence, then depth, then order in file.
 *
 * @param[in] a  Candidate. Not NULL.
 * @param[in] b  Current best. Not NULL.
 *
 * @return 1 if @p a is better than @p b, 0 otherwise.
 */
static int match_better(const MATCH *a, const MATCH *b) {
    if (a->nPrecedence != b->nPrecedence)
        return a->nPrecedence > b->nPrecedence;
    if (a->nDepth != b->nDepth)
        return a->nDepth > b->nDepth;
    return a->nOrderInFile > b->nOrderInFile;
}

/* ==================================================================
 * Field extraction: TOML annotation -> MATCH
 * ================================================================== */

/**
 * @brief Read a string field from a TOML annotation.
 *
 * @param[in] hAnn  Annotation handle. Not NULLHANDLE.
 * @param[in] fn    Field accessor following the size-query pattern.
 *
 * @return malloc'd value, or NULL if absent or on failure.
 */
static char *ann_get_str(HREUSEANN hAnn, ULONG (*fn)(HREUSEANN, PSZ, ULONG, PULONG)) {
    ULONG ulSize = 0;
    char *buf;
    if (fn(hAnn, NULL, 0, &ulSize) != REUSE_NO_ERROR) return NULL;
    if (ulSize == 0) return NULL;
    buf = (char*)malloc(ulSize);
    if (!buf) return NULL;
    if (fn(hAnn, buf, ulSize, NULL) != REUSE_NO_ERROR) {
        free(buf);
        return NULL;
    }
    return buf;
}

/**
 * @brief Populate a MATCH from one TOML annotation.
 *
 * @param[in]  hAnn    Annotation handle. Not NULLHANDLE.
 * @param[in]  nDepth  Depth from repository root.
 * @param[in]  nOrder  Order of the annotation inside the file.
 * @param[out] pm      Receiver. Not NULL.
 *
 * @return 0 on success.
 */
static int match_from_toml(HREUSEANN hAnn, int nDepth, int nOrder, MATCH *pm) {
    ULONG ulPrec = REUSE_PRECEDENCE_CLOSEST;
    ReuseAnnGetPrecedence(hAnn, &ulPrec);
    pm->nPrecedence = (int)ulPrec;
    pm->nDepth = nDepth;
    pm->nOrderInFile = nOrder;

    pm->pszLicense    = ann_get_str(hAnn, ReuseAnnGetLicense);
    pm->pszCopyright  = ann_get_str(hAnn, ReuseAnnGetCopyright);
    pm->pszPackageName = ann_get_str(hAnn, ReuseAnnGetPackageName);
    pm->pszPackageSupplier = ann_get_str(hAnn, ReuseAnnGetPackageSupplier);
    pm->pszPackageDownloadLocation = ann_get_str(hAnn,
        ReuseAnnGetPackageDownloadLocation);
    pm->pszPackageComment = ann_get_str(hAnn, ReuseAnnGetPackageComment);

    /* Contributors: join '\n' */
    {
        ULONG ulCount = 0, i;
        if (ReuseAnnGetContributorCount(hAnn, &ulCount) == REUSE_NO_ERROR &&
            ulCount > 0) {
            size_t total = 0, pos = 0;
            char *joined;
            for (i = 0; i < ulCount; i++) {
                ULONG sz = 0;
                ReuseAnnGetContributor(hAnn, i, NULL, 0, &sz);
                total += sz;
                if (i > 0) total += 1;
            }
            joined = (char*)malloc(total + 1);
            if (joined) {
                for (i = 0; i < ulCount; i++) {
                    ULONG sz = 0;
                    ReuseAnnGetContributor(hAnn, i, NULL, 0, &sz);
                    if (i > 0) joined[pos++] = '\n';
                    if (sz > 1) {
                        ReuseAnnGetContributor(hAnn, i,
                                               joined + pos, sz, NULL);
                        pos += sz - 1;
                    }
                }
                joined[pos] = '\0';
                pm->pszContributors = joined;
            }
        }
    }
    return 0;
}

/**
 * @brief Find the best matching annotation in one REUSE.toml.
 *
 * @param[in]  hToml    Document handle. Not NULLHANDLE.
 * @param[in]  nDepth   Depth from repository root.
 * @param[in]  pszRel   Path relative to the source directory.
 * @param[out] pm       Receiver. Not NULL.
 *
 * @return 0 if at least one annotation matched, -1 otherwise.
 */
static int toml_find_best(HREUSETOML hToml, int nDepth,
                          PCSZ pszRel, MATCH *pm) {
    ULONG ulCount = 0, i;
    int nFound = 0;

    if (ReuseGetAnnotationCount(hToml, &ulCount) != REUSE_NO_ERROR)
        return -1;

    for (i = 0; i < ulCount; i++) {
        HREUSEANN hAnn = NULLHANDLE;
        ULONG ulPatCount = 0, j;
        int match = 0;

        if (ReuseGetAnnotation(hToml, i, &hAnn) != REUSE_NO_ERROR)
            continue;
        if (ReuseAnnGetPathCount(hAnn, &ulPatCount) != REUSE_NO_ERROR)
            continue;

        for (j = 0; j < ulPatCount; j++) {
            char pat[1024];
            ULONG used = 0;
            if (ReuseAnnGetPath(hAnn, j, pat, sizeof(pat), &used)
                != REUSE_NO_ERROR)
                continue;
            if (matches_pattern(pat, pszRel)) { match = 1; break; }
        }

        if (match) {
            MATCH cand;
            memset(&cand, 0, sizeof(cand));
            match_from_toml(hAnn, nDepth, (int)i, &cand);
            if (!nFound || match_better(&cand, pm)) {
                match_clear(pm);
                *pm = cand;
            } else {
                match_clear(&cand);
            }
            nFound = 1;
        }
    }
    return nFound ? 0 : -1;
}

/* ==================================================================
 * Field extraction: DEP5 Files stanza -> MATCH
 * ================================================================== */

/**
 * @brief Read one field from the current DEP5 Files stanza.
 *
 * @param[in] hFind     Cursor. Not NULLHANDLE.
 * @param[in] pszField  Field name. Not NULL.
 *
 * @return malloc'd value, or NULL if absent or on failure.
 */
static char *dep5_get_field(HDEP5FIND hFind, PCSZ pszField) {
    ULONG ulSize = 0;
    char *buf;
    if (Dep5FilesGetField(hFind, pszField, NULL, 0, &ulSize)
        != DEP5_NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    buf = (char*)malloc(ulSize);
    if (!buf) return NULL;
    if (Dep5FilesGetField(hFind, pszField, buf, ulSize, NULL)
        != DEP5_NO_ERROR) {
        free(buf);
        return NULL;
    }
    return buf;
}

/**
 * @brief Populate a MATCH from one DEP5 Files stanza.
 *
 * @param[in]  hFind   Cursor. Not NULLHANDLE.
 * @param[in]  nDepth  Depth from repository root.
 * @param[in]  nOrder  Order of the stanza inside the file.
 * @param[out] pm      Receiver. Not NULL.
 *
 * @return 0 on success.
 */
static int match_from_dep5(HDEP5FIND hFind, int nDepth, int nOrder,
                           MATCH *pm) {
    pm->nPrecedence = REUSE_PRECEDENCE_CLOSEST;
    pm->nDepth = nDepth;
    pm->nOrderInFile = nOrder;

    pm->pszLicense   = dep5_get_field(hFind, "License");
    pm->pszCopyright = dep5_get_field(hFind, "Copyright");
    return 0;
}

/**
 * @brief Find the last matching Files stanza in a DEP5 document.
 *
 * Per DEP5, the last stanza that matches a file applies.
 *
 * @param[in]  hDep5   Document handle. Not NULLHANDLE.
 * @param[in]  nDepth  Depth from repository root.
 * @param[in]  pszRel  Path relative to the source directory.
 * @param[out] pm      Receiver. Not NULL.
 *
 * @return 0 if at least one stanza matched, -1 otherwise.
 */
static int dep5_find_best(HDEP5DOC hDep5, int nDepth,
                          PCSZ pszRel, MATCH *pm) {
    HDEP5FIND hFind = NULLHANDLE;
    ULONG ulTotal = 0;
    APIRET rc;
    int nFound = 0;
    int nCur = 0;

    rc = Dep5FilesFindFirst(hDep5, &hFind, &ulTotal);
    if (rc != DEP5_NO_ERROR) return -1;

    do {
        ULONG ulPatCount = 0, j;
        int match = 0;

        if (Dep5FilesGetPatternCount(hFind, &ulPatCount) != DEP5_NO_ERROR)
            continue;

        for (j = 0; j < ulPatCount; j++) {
            char pat[1024];
            ULONG used = 0;
            if (Dep5FilesGetPattern(hFind, j, pat, sizeof(pat), &used)
                != DEP5_NO_ERROR)
                continue;
            if (matches_pattern(pat, pszRel)) { match = 1; break; }
        }

        if (match) {
            match_clear(pm);
            match_from_dep5(hFind, nDepth, nCur, pm);
            nFound = 1;
        }
        nCur++;
    } while (Dep5FilesFindNext(hFind) == DEP5_NO_ERROR);

    Dep5FilesFindClose(hFind);
    return nFound ? 0 : -1;
}

/* ==================================================================
 * Config-level match dispatch
 * ================================================================== */

/**
 * @brief Find the best match for a file inside one config entry.
 *
 * @param[in]  pc      Config entry. Not NULL.
 * @param[in]  pszPath Path to the file. Not NULL.
 * @param[out] pm      Receiver. Not NULL.
 *
 * @return 0 if a match was found, -1 otherwise.
 */
static int cfg_find_best(REUSECFG *pc, PCSZ pszPath, MATCH *pm) {
    const char *rel = rel_to_dir(pc->pszSourceDir, pszPath);
    if (pc->nKind == REUSECFG_KIND_TOML)
        return toml_find_best(pc->hToml, pc->nDepth, rel, pm);
    if (pc->nKind == REUSECFG_KIND_DEP5)
        return dep5_find_best(pc->hDep5, pc->nDepth, rel, pm);
    return -1;
}

/* ==================================================================
 * Aggregation helpers
 * ================================================================== */

/**
 * @brief Combine two SPDX license expressions with AND.
 *
 * @param[in] a  First expression, or NULL.
 * @param[in] b  Second expression, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static char *aggregate_licenses(const char *a, const char *b) {
    size_t alen, blen;
    char *r;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 8);
    if (!r) return NULL;
    sprintf(r, "(%s) AND (%s)", a, b);
    return r;
}

/**
 * @brief Concatenate two copyright texts with '\n'.
 *
 * @param[in] a  First text, or NULL.
 * @param[in] b  Second text, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static char *aggregate_copyrights(const char *a, const char *b) {
    size_t alen, blen;
    char *r;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 2);
    if (!r) return NULL;
    sprintf(r, "%s\n%s", a, b);
    return r;
}

/**
 * @brief Check whether a '\n'-separated text contains a given line.
 *
 * @param[in] hay     Text. Not NULL.
 * @param[in] needle  Line to look for. Not NULL.
 * @param[in] nlen    Length of @p needle.
 *
 * @return 1 if found, 0 otherwise.
 */
static int contains_line(const char *hay, const char *needle, size_t nlen) {
    const char *p = hay;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        if (len == nlen && memcmp(p, needle, nlen) == 0) return 1;
        if (!eol) break;
        p = eol + 1;
    }
    return 0;
}

/**
 * @brief Append lines of @p b to @p a, dropping duplicate lines.
 *
 * @param[in] a  First text, or NULL.
 * @param[in] b  Second text, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static char *join_lines_dedup(const char *a, const char *b) {
    size_t alen, blen, out_len, cap;
    char *out;
    const char *p;

    if (!a && !b) return NULL;
    if (!a) return dup_str(b);
    if (!b) return dup_str(a);

    alen = strlen(a);
    blen = strlen(b);
    cap = alen + blen + 2;
    out = (char*)malloc(cap);
    if (!out) return NULL;
    memcpy(out, a, alen);
    out[alen] = '\0';
    out_len = alen;

    p = b;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        if (!contains_line(out, p, len)) {
            if (out_len + 1 + len + 1 > cap) {
                char *no;
                cap = out_len + 1 + len + 2;
                no = (char*)realloc(out, cap);
                if (!no) { free(out); return NULL; }
                out = no;
            }
            out[out_len++] = '\n';
            memcpy(out + out_len, p, len);
            out_len += len;
            out[out_len] = '\0';
        }
        if (!eol) break;
        p = eol + 1;
    }
    return out;
}

/**
 * @brief Concatenate two '\n'-separated texts without deduplication.
 *
 * @param[in] a  First text, or NULL.
 * @param[in] b  Second text, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static char *join_lines_append(const char *a, const char *b) {
    size_t alen, blen;
    char *r;
    if (!a) return b ? dup_str(b) : NULL;
    if (!b) return dup_str(a);
    alen = strlen(a);
    blen = strlen(b);
    r = (char*)malloc(alen + blen + 2);
    if (!r) return NULL;
    sprintf(r, "%s\n%s", a, b);
    return r;
}

/* ==================================================================
 * Resolver
 * ================================================================== */

/**
 * @brief Resolve licensing information for one file.
 *
 * Implements the algorithm of REUSE 3.3 §4.1.3:
 *   1. Read the sidecar file <file>.license, if present.
 *   2. Read SPDX tags inside the file.
 *   3. Sidecar takes precedence over in-file tags.
 *   4. Combine annotations from all configs by precedence
 *      (override > aggregate > closest).
 *   5. Aggregate in-file sources with the closest/aggregate match.
 *
 * @param[in] pd       Project handle. Not NULL.
 * @param[in] pszPath  Path to the file. Not NULL.
 *
 * @return Freshly allocated file handle, or NULL on OOM.
 */
static PREUSETREEFILE resolve_file(PREUSETREE pd, PCSZ pszPath) {
    PREUSETREEFILE pf;
    char sidecar[1200];
    char *tag_lic = NULL;
    char *tag_cop = NULL;
    char *side_lic = NULL;
    char *side_cop = NULL;
    const char *in_lic = NULL;
    const char *in_cop = NULL;
    MATCH best_override, best_aggregate, best_closest;
    int have_override = 0, have_aggregate = 0, have_closest = 0;
    ULONG i;

    memset(&best_override, 0, sizeof(best_override));
    memset(&best_aggregate, 0, sizeof(best_aggregate));
    memset(&best_closest, 0, sizeof(best_closest));

    pf = (PREUSETREEFILE)calloc(1, sizeof(REUSETREEFILE));
    if (!pf) return NULL;

    /* 1. Read sidecar (REUSE 3.3 §4.1.3): <file>.license */
    snprintf(sidecar, sizeof(sidecar), "%s.license", pszPath);
    {
        FILE *f = fopen(sidecar, "rb");
        if (f) {
            fclose(f);
            if (SpdxFileGetLicense(sidecar, &side_lic) != SPDX_TAG_NO_ERROR)
                side_lic = NULL;
            if (SpdxFileGetCopyright(sidecar, &side_cop) != SPDX_TAG_NO_ERROR)
                side_cop = NULL;
        }
    }

    /* 2. Read tags inside the file */
    if (SpdxFileGetLicense(pszPath, &tag_lic) != SPDX_TAG_NO_ERROR)
        tag_lic = NULL;
    if (SpdxFileGetCopyright(pszPath, &tag_cop) != SPDX_TAG_NO_ERROR)
        tag_cop = NULL;

    /* 3. Sidecar takes precedence over in-file tags */
    in_lic = side_lic ? side_lic : tag_lic;
    in_cop = side_cop ? side_cop : tag_cop;

    if (in_lic || in_cop) pf->bHasReuse = TRUE_;

    /* 4. For each config, find the best matching annotation, then
     *    classify it by precedence into override/aggregate/closest. */
    for (i = 0; i < pd->ulCount; i++) {
        MATCH m;
        memset(&m, 0, sizeof(m));
        if (cfg_find_best(&pd->paCfgs[i], pszPath, &m) != 0) continue;
        pf->bHasReuse = TRUE_;

        switch (m.nPrecedence) {
            case REUSE_PRECEDENCE_OVERRIDE:
                if (!have_override || match_better(&m, &best_override)) {
                    match_clear(&best_override);
                    best_override = m;
                    have_override = 1;
                } else {
                    match_clear(&m);
                }
                break;
            case REUSE_PRECEDENCE_AGGREGATE:
                if (!have_aggregate || match_better(&m, &best_aggregate)) {
                    match_clear(&best_aggregate);
                    best_aggregate = m;
                    have_aggregate = 1;
                } else {
                    match_clear(&m);
                }
                break;
            default:
                if (!have_closest || match_better(&m, &best_closest)) {
                    match_clear(&best_closest);
                    best_closest = m;
                    have_closest = 1;
                } else {
                    match_clear(&m);
                }
                break;
        }
    }

    /* 5. Compose the result. */
    if (have_override) {
        pf->ulPrecedence = REUSE_PRECEDENCE_OVERRIDE;
        if (best_override.pszLicense)
            pf->pszLicense = dup_str(best_override.pszLicense);
        if (best_override.pszCopyright)
            pf->pszCopyright = dup_str(best_override.pszCopyright);
        if (best_override.pszContributors)
            pf->pszContributors = dup_str(best_override.pszContributors);
        if (best_override.pszPackageName)
            pf->pszPackageName = dup_str(best_override.pszPackageName);
        if (best_override.pszPackageSupplier)
            pf->pszPackageSupplier = dup_str(best_override.pszPackageSupplier);
        if (best_override.pszPackageDownloadLocation)
            pf->pszPackageDownloadLocation =
                dup_str(best_override.pszPackageDownloadLocation);
        if (best_override.pszPackageComment)
            pf->pszPackageComment = dup_str(best_override.pszPackageComment);
    } else {
        const char *base_lic = have_closest ? best_closest.pszLicense : NULL;
        const char *base_cop = have_closest ? best_closest.pszCopyright : NULL;
        if (in_lic) base_lic = in_lic;
        if (in_cop) base_cop = in_cop;

        if (!have_aggregate) {
            pf->ulPrecedence = (base_lic || base_cop || have_closest)
                ? REUSE_PRECEDENCE_CLOSEST : 0;
            if (base_lic) pf->pszLicense = dup_str(base_lic);
            if (base_cop) pf->pszCopyright = dup_str(base_cop);
            if (have_closest) {
                if (best_closest.pszContributors)
                    pf->pszContributors =
                        dup_str(best_closest.pszContributors);
                if (best_closest.pszPackageName)
                    pf->pszPackageName = dup_str(best_closest.pszPackageName);
                if (best_closest.pszPackageSupplier)
                    pf->pszPackageSupplier =
                        dup_str(best_closest.pszPackageSupplier);
                if (best_closest.pszPackageDownloadLocation)
                    pf->pszPackageDownloadLocation =
                        dup_str(best_closest.pszPackageDownloadLocation);
                if (best_closest.pszPackageComment)
                    pf->pszPackageComment =
                        dup_str(best_closest.pszPackageComment);
            }
        } else {
            pf->ulPrecedence = REUSE_PRECEDENCE_AGGREGATE;

            if (base_lic && best_aggregate.pszLicense)
                pf->pszLicense = aggregate_licenses(base_lic,
                                                    best_aggregate.pszLicense);
            else if (base_lic)
                pf->pszLicense = dup_str(base_lic);
            else if (best_aggregate.pszLicense)
                pf->pszLicense = dup_str(best_aggregate.pszLicense);

            if (base_cop && best_aggregate.pszCopyright)
                pf->pszCopyright = aggregate_copyrights(base_cop,
                                                        best_aggregate.pszCopyright);
            else if (base_cop)
                pf->pszCopyright = dup_str(base_cop);
            else if (best_aggregate.pszCopyright)
                pf->pszCopyright = dup_str(best_aggregate.pszCopyright);

            /* Contributors: aggregate with deduplication */
            {
                const char *a = have_closest
                    ? best_closest.pszContributors : NULL;
                const char *b = best_aggregate.pszContributors;
                if (a && b) pf->pszContributors = join_lines_dedup(a, b);
                else if (a) pf->pszContributors = dup_str(a);
                else if (b) pf->pszContributors = dup_str(b);
            }

            /* Package*: closest > aggregate (project decision) */
            if (have_closest && best_closest.pszPackageName)
                pf->pszPackageName = dup_str(best_closest.pszPackageName);
            else if (best_aggregate.pszPackageName)
                pf->pszPackageName = dup_str(best_aggregate.pszPackageName);

            if (have_closest && best_closest.pszPackageSupplier)
                pf->pszPackageSupplier =
                    dup_str(best_closest.pszPackageSupplier);
            else if (best_aggregate.pszPackageSupplier)
                pf->pszPackageSupplier =
                    dup_str(best_aggregate.pszPackageSupplier);

            if (have_closest && best_closest.pszPackageDownloadLocation)
                pf->pszPackageDownloadLocation =
                    dup_str(best_closest.pszPackageDownloadLocation);
            else if (best_aggregate.pszPackageDownloadLocation)
                pf->pszPackageDownloadLocation =
                    dup_str(best_aggregate.pszPackageDownloadLocation);

            if (have_closest && best_closest.pszPackageComment)
                pf->pszPackageComment =
                    dup_str(best_closest.pszPackageComment);
            else if (best_aggregate.pszPackageComment)
                pf->pszPackageComment =
                    dup_str(best_aggregate.pszPackageComment);
        }
    }

    match_clear(&best_override);
    match_clear(&best_aggregate);
    match_clear(&best_closest);

    free(tag_lic);
    free(tag_cop);
    free(side_lic);
    free(side_cop);

    return pf;
}

/* ==================================================================
 * Handle helpers
 * ================================================================== */

PREUSETREE ReuseInternalGetDoc(HREUSETREE hDoc) {
    return (PREUSETREE)hDoc;
}

PREUSETREEFILE ReuseInternalGetFile(HREUSETREEFILE hFile) {
    return (PREUSETREEFILE)hFile;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Open a REUSE project rooted at a directory.
 *
 * @param[in]  pszDir  Target directory. Not NULL.
 * @param[out] phDoc   Handle receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  pszDir or phDoc is NULL.
 * @retval REUSE_ERROR_OUT_OF_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseTreeOpen(PCSZ pszDir, HREUSETREE *phDoc) {
    PREUSETREE pd;
    char *repo_root = NULL;

    if (!pszDir || !phDoc) return REUSE_ERROR_INVALID_PARAM;
    *phDoc = NULLHANDLE;

    pd = (PREUSETREE)calloc(1, sizeof(REUSETREE));
    if (!pd) return REUSE_ERROR_OUT_OF_MEMORY;

    pd->pszProjectDir = dup_str(pszDir);
    if (!pd->pszProjectDir) { doc_free(pd); return REUSE_ERROR_OUT_OF_MEMORY; }

    /* Two-phase GitFindRepoRoot call: first query the size, then
     * allocate and query the value. */
    {
        ULONG ulSize = 0;
        if (GitFindRepoRoot(pszDir, NULL, 0, &ulSize) == NO_ERROR &&
            ulSize > 0) {
            repo_root = (char*)malloc(ulSize);
            if (repo_root) {
                if (GitFindRepoRoot(pszDir, repo_root, ulSize, NULL)
                        != NO_ERROR) {
                    free(repo_root);
                    repo_root = NULL;
                }
            }
        }
    }
    pd->pszRepoRoot = repo_root; /* may be NULL */

    if (discover_tomls(pd, repo_root, pszDir) != 0) {
        doc_free(pd);
        return REUSE_ERROR_OUT_OF_MEMORY;
    }
    if (discover_dep5(pd, repo_root) != 0) {
        doc_free(pd);
        return REUSE_ERROR_OUT_OF_MEMORY;
    }

    *phDoc = (HREUSETREE)pd;
    return REUSE_NO_ERROR;
}

/**
 * @brief Close a project and release all associated memory.
 *
 * @param[in] hDoc  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success. Also for NULLHANDLE.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET APIENTRY ReuseTreeClose(HREUSETREE hDoc) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (hDoc == NULLHANDLE) return REUSE_NO_ERROR;
    if (!pd) return REUSE_ERROR_INVALID_HANDLE;
    doc_free(pd);
    return REUSE_NO_ERROR;
}

/**
 * @brief Number of REUSEERR records stored in the project.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 */
APIRET APIENTRY ReuseTreeGetErrorCount(HREUSETREE hDoc, PULONG pulCount) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (!pd || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pd->ulErrorCount;
    return REUSE_NO_ERROR;
}

/**
 * @brief Retrieve one diagnostic record by index.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET APIENTRY ReuseTreeGetError(HREUSETREE hDoc, ULONG ulIndex,
                                  PREUSEERR pErr) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (!pd || !pErr) return REUSE_ERROR_INVALID_PARAM;
    if (ulIndex >= pd->ulErrorCount) return REUSE_ERROR_INDEX_RANGE;
    *pErr = pd->paErrors[ulIndex];
    return REUSE_NO_ERROR;
}

/**
 * @brief Resolve licensing information for one file.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phFile   Handle receiver. Not NULL.
 * @param[out] pErr     Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  hDoc, pszPath or phFile is NULL.
 * @retval REUSE_ERROR_OUT_OF_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseTreeResolveFile(HREUSETREE hDoc, PCSZ pszPath,
                                     HREUSETREEFILE *phFile,
                                     PREUSEERR pErr) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    PREUSETREEFILE pf;

    if (!pd || !pszPath || !phFile) return REUSE_ERROR_INVALID_PARAM;
    *phFile = NULLHANDLE;

    pf = resolve_file(pd, pszPath);
    if (!pf) return REUSE_ERROR_OUT_OF_MEMORY;

    *phFile = (HREUSETREEFILE)pf;
    (void)pErr; /* file-specific diagnostics: reserved for future use */
    return REUSE_NO_ERROR;
}

/**
 * @brief Release a resolution handle.
 *
 * @param[in] hFile  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success. Also for NULLHANDLE.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET APIENTRY ReuseTreeFileClose(HREUSETREEFILE hFile) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (hFile == NULLHANDLE) return REUSE_NO_ERROR;
    if (!pf) return REUSE_ERROR_INVALID_HANDLE;
    safe_free((void**)&pf->pszLicense);
    safe_free((void**)&pf->pszCopyright);
    safe_free((void**)&pf->pszContributors);
    safe_free((void**)&pf->pszPackageName);
    safe_free((void**)&pf->pszPackageSupplier);
    safe_free((void**)&pf->pszPackageDownloadLocation);
    safe_free((void**)&pf->pszPackageComment);
    free(pf);
    return REUSE_NO_ERROR;
}

/* ==================================================================
 * Field getters
 * ================================================================== */

/**
 * @brief Copy a NUL-terminated string to a caller buffer.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied and NUL-terminated.
 *   - ulSize too small: REUSE_ERROR_BUFFER_OVERFLOW; *pulUsed is
 *     the required size including NUL.
 *
 * @param[in]  pszVal   Value, or NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 */
static APIRET copy_out(const char *pszVal,
                       PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    size_t n;
    if (!pszVal) return REUSE_ERROR_NOT_FOUND;
    n = strlen(pszVal);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return REUSE_NO_ERROR;
    }
    if (!pszBuf) return REUSE_ERROR_INVALID_PARAM;
    if (ulSize < n + 1) {
        if (pulUsed) *pulUsed = (ULONG)(n + 1);
        return REUSE_ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszVal, n);
    pszBuf[n] = '\0';
    if (pulUsed) *pulUsed = (ULONG)n;
    return REUSE_NO_ERROR;
}

/**
 * @brief Retrieve the resolved SPDX license expression.
 */
APIRET APIENTRY ReuseTreeFileGetLicense(HREUSETREEFILE hFile,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszLicense, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the resolved copyright text.
 *
 * Multiple notices are joined with '\n'.
 */
APIRET APIENTRY ReuseTreeFileGetCopyright(HREUSETREEFILE hFile,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszCopyright, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the resolved SPDX-FileContributor list.
 *
 * Multiple contributors are joined with '\n'.
 */
APIRET APIENTRY ReuseTreeFileGetContributors(HREUSETREEFILE hFile,
                                             PSZ pszBuf, ULONG ulSize,
                                             PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszContributors, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageName.
 */
APIRET APIENTRY ReuseTreeFileGetPackageName(HREUSETREEFILE hFile,
                                            PSZ pszBuf, ULONG ulSize,
                                            PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageName, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageSupplier.
 */
APIRET APIENTRY ReuseTreeFileGetPackageSupplier(HREUSETREEFILE hFile,
                                                PSZ pszBuf, ULONG ulSize,
                                                PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageSupplier, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageDownloadLocation.
 */
APIRET APIENTRY ReuseTreeFileGetPackageDownloadLocation(HREUSETREEFILE hFile,
                                                        PSZ pszBuf,
                                                        ULONG ulSize,
                                                        PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageDownloadLocation, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageComment.
 */
APIRET APIENTRY ReuseTreeFileGetPackageComment(HREUSETREEFILE hFile,
                                               PSZ pszBuf, ULONG ulSize,
                                               PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageComment, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the precedence of the winning source.
 *
 * @param[in]  hFile          Handle. Not NULLHANDLE.
 * @param[out] pulPrecedence  Receiver: one of REUSE_PRECEDENCE_*.
 *                            Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 */
APIRET APIENTRY ReuseTreeFileGetPrecedence(HREUSETREEFILE hFile,
                                           PULONG pulPrecedence) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf || !pulPrecedence) return REUSE_ERROR_INVALID_PARAM;
    *pulPrecedence = pf->ulPrecedence;
    return REUSE_NO_ERROR;
}

/**
 * @brief Whether any REUSE.toml, DEP5, sidecar or tag matched.
 *
 * @param[in]  hFile       Handle. Not NULLHANDLE.
 * @param[out] pfHasReuse  Receiver TRUE_ / FALSE_. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 */
APIRET APIENTRY ReuseTreeFileGetHasReuse(HREUSETREEFILE hFile,
                                         PBOOL pfHasReuse) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf || !pfHasReuse) return REUSE_ERROR_INVALID_PARAM;
    *pfHasReuse = pf->bHasReuse;
    return REUSE_NO_ERROR;
}
