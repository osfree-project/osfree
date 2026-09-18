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
 */

/* ==================================================================
 * Small helpers
 * ================================================================== */

static char *dup_n(const char *s, size_t n) {
    char *r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

static char *dup_str(const char *s) {
    return dup_n(s, strlen(s));
}

static void safe_free(void **pp) {
    if (pp && *pp) { free(*pp); *pp = NULL; }
}

/* ==================================================================
 * Path handling
 * ================================================================== */

static int path_char_eq(int a, int b) {
#ifdef _WIN32
    return tolower((unsigned char)a) == tolower((unsigned char)b);
#else
    return (unsigned char)a == (unsigned char)b;
#endif
}

static int path_prefix_eq(const char *s, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (!s[i]) return 0;
        if (!path_char_eq(s[i], prefix[i])) return 0;
    }
    return 1;
}

/**
 * @brief Return the path of filename relative to base.
 *
 * If filename does not start with base (followed by a separator or
 * end of string), returns filename unchanged.
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

static int matches_pattern(const char *pattern, const char *filename) {
    return match_path(pattern, filename);
}

/* ==================================================================
 * Error recording
 * ================================================================== */

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

static void cfg_free(REUSECFG *pc) {
    if (!pc) return;
    safe_free((void**)&pc->pszSourceDir);
    if (pc->hToml != NULLHANDLE) ReuseClose(pc->hToml);
    if (pc->hDep5 != NULLHANDLE) Dep5Close(pc->hDep5);
    memset(pc, 0, sizeof(*pc));
}

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
 *
 * Build the path of a candidate file by joining dir and name with the
 * platform separator. Returns 0 if the resulting path fits in dst.
 * ================================================================== */

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
 * On parse failure, records a diagnostic and returns 0 (the caller
 * continues with other files). Returns -1 only on fatal errors
 * (out of memory).
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
 * @brief Discover .reuse/dep5 at the repository root.
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
 *
 * Field strings are owned by this structure. match_clear releases them.
 * ================================================================== */

typedef struct _MATCH {
    int   nPrecedence;
    int   nDepth;
    int   nOrderInFile;

    char *pszLicense;
    char *pszCopyright;
    char *pszContributors;
    char *pszPackageName;
    char *pszPackageSupplier;
    char *pszPackageDownloadLocation;
    char *pszPackageComment;
} MATCH;

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
 * Returns 0 if any annotation matched, -1 otherwise.
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
 * Returns 0 if any stanza matched, -1 otherwise.
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
 * Fills a freshly allocated REUSETREEFILE. Returns NULL on OOM.
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
            side_lic = file_get_spdx_license(sidecar);
            side_cop = file_get_spdx_copyright(sidecar);
        }
    }

    /* 2. Read tags inside the file */
    tag_lic = file_get_spdx_license(pszPath);
    tag_cop = file_get_spdx_copyright(pszPath);

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

APIRET APIENTRY ReuseTreeOpen(PCSZ pszDir, HREUSETREE *phDoc) {
    PREUSETREE pd;
    char *repo_root = NULL;

    if (!pszDir || !phDoc) return REUSE_ERROR_INVALID_PARAM;
    *phDoc = NULLHANDLE;

    pd = (PREUSETREE)calloc(1, sizeof(REUSETREE));
    if (!pd) return REUSE_ERROR_OUT_OF_MEMORY;

    pd->pszProjectDir = dup_str(pszDir);
    if (!pd->pszProjectDir) { doc_free(pd); return REUSE_ERROR_OUT_OF_MEMORY; }

    if (GitFindRepoRoot(pszDir, &repo_root) != GIT_NO_ERROR) {
        repo_root = NULL;
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

APIRET APIENTRY ReuseTreeClose(HREUSETREE hDoc) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (hDoc == NULLHANDLE) return REUSE_NO_ERROR;
    if (!pd) return REUSE_ERROR_INVALID_HANDLE;
    doc_free(pd);
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseTreeGetErrorCount(HREUSETREE hDoc, PULONG pulCount) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (!pd || !pulCount) return REUSE_ERROR_INVALID_PARAM;
    *pulCount = pd->ulErrorCount;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseTreeGetError(HREUSETREE hDoc, ULONG ulIndex,
                                  PREUSEERR pErr) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (!pd || !pErr) return REUSE_ERROR_INVALID_PARAM;
    if (ulIndex >= pd->ulErrorCount) return REUSE_ERROR_INDEX_RANGE;
    *pErr = pd->paErrors[ulIndex];
    return REUSE_NO_ERROR;
}

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

APIRET APIENTRY ReuseTreeFileGetLicense(HREUSETREEFILE hFile,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszLicense, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetCopyright(HREUSETREEFILE hFile,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszCopyright, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetContributors(HREUSETREEFILE hFile,
                                             PSZ pszBuf, ULONG ulSize,
                                             PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszContributors, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetPackageName(HREUSETREEFILE hFile,
                                            PSZ pszBuf, ULONG ulSize,
                                            PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageName, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetPackageSupplier(HREUSETREEFILE hFile,
                                                PSZ pszBuf, ULONG ulSize,
                                                PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageSupplier, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetPackageDownloadLocation(HREUSETREEFILE hFile,
                                                        PSZ pszBuf,
                                                        ULONG ulSize,
                                                        PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageDownloadLocation, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetPackageComment(HREUSETREEFILE hFile,
                                               PSZ pszBuf, ULONG ulSize,
                                               PULONG pulUsed) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf) return REUSE_ERROR_INVALID_PARAM;
    return copy_out(pf->pszPackageComment, pszBuf, ulSize, pulUsed);
}

APIRET APIENTRY ReuseTreeFileGetPrecedence(HREUSETREEFILE hFile,
                                           PULONG pulPrecedence) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf || !pulPrecedence) return REUSE_ERROR_INVALID_PARAM;
    *pulPrecedence = pf->ulPrecedence;
    return REUSE_NO_ERROR;
}

APIRET APIENTRY ReuseTreeFileGetHasReuse(HREUSETREEFILE hFile,
                                         PBOOL pfHasReuse) {
    PREUSETREEFILE pf = ReuseInternalGetFile(hFile);
    if (!pf || !pfHasReuse) return REUSE_ERROR_INVALID_PARAM;
    *pfHasReuse = pf->bHasReuse;
    return REUSE_NO_ERROR;
}
