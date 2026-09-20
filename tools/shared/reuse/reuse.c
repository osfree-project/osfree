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
#include "path.h"
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
 * @brief Free a heap block through a pointer-to-pointer and clear it.
 *
 * Passing NULL or a pointer to a NULL value is a no-op.
 *
 * @param[in,out] pp  Pointer to the block pointer. May be NULL.
 */
static void safe_free(void **pp) {
    if (pp && *pp) { free(*pp); *pp = NULL; }
}

/**
 * @brief Query and allocate the SPDX-License-Identifier value.
 *
 * Uses the size-query convention of SpdxQueryFileLicense: first a
 * size query, then a fetch into a malloc'd buffer. Returns NULL if
 * the tag is absent or on any error.
 *
 * @param[in] pszPath  Path to the file. Not NULL.
 *
 * @return malloc'd value, or NULL on error or if the tag is absent.
 */
static PSZ query_file_license(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxQueryFileLicense(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxQueryFileLicense(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    if (pszOut[0] == '\0') { free(pszOut); return NULL; }
    return pszOut;
}

/**
 * @brief Query and allocate the SPDX-FileCopyrightText value.
 *
 * Counterpart of query_file_license for the copyright tag.
 *
 * @param[in] pszPath  Path to the file. Not NULL.
 *
 * @return malloc'd value, or NULL on error or if the tag is absent.
 */
static PSZ query_file_copyright(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxQueryFileCopyright(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxQueryFileCopyright(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    if (pszOut[0] == '\0') { free(pszOut); return NULL; }
    return pszOut;
}

/* ==================================================================
 * Path handling
 * ================================================================== */

/**
 * @brief Compare two path characters.
 *
 * Case-insensitive on Windows, case-sensitive elsewhere.
 *
 * @param[in] nA  First character.
 * @param[in] nB  Second character.
 *
 * @return 1 if the characters are equal under the platform rules,
 *         0 otherwise.
 */
static int path_char_eq(int nA, int nB) {
#ifdef _WIN32
    return tolower((unsigned char)nA) == tolower((unsigned char)nB);
#else
    return (unsigned char)nA == (unsigned char)nB;
#endif
}

/**
 * @brief Compare a string against a prefix under path_char_eq rules.
 *
 * @param[in] pszStr     String to test. Not NULL.
 * @param[in] pszPrefix  Prefix. Not NULL.
 * @param[in] cbLen      Prefix length.
 *
 * @return 1 if @p pszStr starts with @p pszPrefix, 0 otherwise.
 */
static int path_prefix_eq(PCSZ pszStr, PCSZ pszPrefix, size_t cbLen) {
    size_t i;
    for (i = 0; i < cbLen; i++) {
        if (!pszStr[i]) return 0;
        if (!path_char_eq(pszStr[i], pszPrefix[i])) return 0;
    }
    return 1;
}

/**
 * @brief Return the path of @p pszFilename relative to @p pszBase.
 *
 * If @p pszFilename does not start with @p pszBase followed by a
 * separator or the end of string, @p pszFilename is returned
 * unchanged.
 *
 * @param[in] pszBase      Base directory. May be NULL or empty.
 * @param[in] pszFilename  Full path. Not NULL.
 *
 * @return Pointer to the relative part, or @p pszFilename unchanged.
 */
static PCSZ rel_to_dir(PCSZ pszBase, PCSZ pszFilename) {
    size_t cbBaseLen;
    if (!pszBase || !pszBase[0]) return pszFilename;
    cbBaseLen = strlen(pszBase);
    if (path_prefix_eq(pszFilename, pszBase, cbBaseLen)) {
        PCSZ pszRest = pszFilename + cbBaseLen;
        if (*pszRest == '\0') return pszRest;
        if (*pszRest != '/' && *pszRest != '\\') return pszFilename;
        while (*pszRest == '/' || *pszRest == '\\') pszRest++;
        return pszRest;
    }
    return pszFilename;
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
 * @param[in] pszPat  Pattern component. Not NULL.
 * @param[in] pszStr  String component. Not NULL.
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
    if (*pszStr == '\0' || *pszStr == '/') return 0;
    if (*pszPat != *pszStr) return 0;
    return match_component(pszPat + 1, pszStr + 1);
}

/**
 * @brief Match a full path against a pattern.
 *
 * In addition to '*' and '?' handled by match_component, supports
 * the '**' wildcard at a component boundary: '**' alone matches any
 * suffix, and '**<slash>' matches any prefix of path components.
 *
 * @param[in] pszPat   Pattern. Not NULL.
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int match_path(PCSZ pszPat, PCSZ pszPath) {
    PCSZ pszPatSlash;
    PCSZ pszPathSlash;

    if (pszPat[0] == '*' && pszPat[1] == '*') {
        if (pszPat[2] == '\0') return 1;
        if (pszPat[2] == '/') {
            PCSZ pszRest = pszPat + 3;
            if (match_path(pszRest, pszPath)) return 1;
            while ((pszPathSlash = strchr(pszPath, '/')) != NULL) {
                pszPath = pszPathSlash + 1;
                if (match_path(pszRest, pszPath)) return 1;
            }
            return 0;
        }
    }
    pszPatSlash  = strchr(pszPat, '/');
    pszPathSlash = strchr(pszPath, '/');
    if (!pszPatSlash) {
        if (pszPathSlash) return 0;
        return match_component(pszPat, pszPath);
    }
    if (!pszPathSlash) return 0;
    {
        size_t cbPatLen = (size_t)(pszPatSlash - pszPat);
        size_t cbFileLen = (size_t)(pszPathSlash - pszPath);
        CHAR achPatSeg[256];
        CHAR achPathSeg[256];
        if (cbPatLen >= sizeof(achPatSeg) ||
            cbFileLen >= sizeof(achPathSeg)) return 0;
        memcpy(achPatSeg, pszPat, cbPatLen);
        achPatSeg[cbPatLen] = '\0';
        memcpy(achPathSeg, pszPath, cbFileLen);
        achPathSeg[cbFileLen] = '\0';
        if (!match_component(achPatSeg, achPathSeg)) return 0;
        return match_path(pszPatSlash + 1, pszPathSlash + 1);
    }
}

/**
 * @brief Convenience wrapper around match_path.
 *
 * @param[in] pszPattern   Pattern. Not NULL.
 * @param[in] pszFilename  Path. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int matches_pattern(PCSZ pszPattern, PCSZ pszFilename) {
    return match_path(pszPattern, pszFilename);
}

/* ==================================================================
 * Error recording
 * ================================================================== */

/**
 * @brief Append one REUSEERR record to a project handle.
 *
 * @param[in,out] pd          Project handle. Not NULL.
 * @param[in]     ulCode      One of REUSE_ERROR_* or ERROR_*.
 * @param[in]     ulSeverity  One of REUSE_SEV_*.
 * @param[in]     pszFile     Offending file, or NULL.
 * @param[in]     ulLine      Source line, 0 if not applicable.
 * @param[in]     pszDetail   Short human-readable detail, or NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int err_add(PREUSETREE pd, ULONG ulCode, ULONG ulSeverity,
                   PCSZ pszFile, ULONG ulLine, PCSZ pszDetail) {
    REUSEERR *pErr;
    if (pd->ulErrorCount >= pd->ulErrorCapacity) {
        ULONG ulNewCap = pd->ulErrorCapacity ? pd->ulErrorCapacity * 2 : 8;
        REUSEERR *paNew = (REUSEERR*)realloc(pd->paErrors,
                                (size_t)ulNewCap * sizeof(REUSEERR));
        if (!paNew) return -1;
        pd->paErrors = paNew;
        pd->ulErrorCapacity = ulNewCap;
    }
    pErr = &pd->paErrors[pd->ulErrorCount++];
    memset(pErr, 0, sizeof(*pErr));
    pErr->ulCode = ulCode;
    pErr->ulSeverity = ulSeverity;
    pErr->ulLine = ulLine;
    if (pszFile)
        strncpy(pErr->achFile, pszFile, sizeof(pErr->achFile) - 1);
    if (pszDetail)
        strncpy(pErr->achDetail, pszDetail, sizeof(pErr->achDetail) - 1);
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
    REUSECFG *pCfg;
    if (pd->ulCount >= pd->ulCapacity) {
        ULONG ulNewCap = pd->ulCapacity ? pd->ulCapacity * 2 : 8;
        REUSECFG *paNew = (REUSECFG*)realloc(pd->paCfgs,
                                (size_t)ulNewCap * sizeof(REUSECFG));
        if (!paNew) return -1;
        pd->paCfgs = paNew;
        pd->ulCapacity = ulNewCap;
    }
    pCfg = &pd->paCfgs[pd->ulCount++];
    memset(pCfg, 0, sizeof(*pCfg));
    pCfg->nKind = nKind;
    pCfg->pszSourceDir = strdup(pszSourceDir);
    pCfg->nDepth = nDepth;
    pCfg->hToml = hToml;
    pCfg->hDep5 = hDep5;
    return pCfg->pszSourceDir ? 0 : -1;
}

/**
 * @brief Release one config entry.
 *
 * @param[in,out] pCfg  Config entry. Not NULL.
 */
static void cfg_free(REUSECFG *pCfg) {
    if (!pCfg) return;
    safe_free((void**)&pCfg->pszSourceDir);
    if (pCfg->hToml != NULLHANDLE) ReuseClose(pCfg->hToml);
    if (pCfg->hDep5 != NULLHANDLE) Dep5Close(pCfg->hDep5);
    memset(pCfg, 0, sizeof(*pCfg));
}

/**
 * @brief Release a project handle and all owned resources.
 *
 * @param[in] pd  Project handle. May be NULL.
 */
static void doc_free(PREUSETREE pd) {
    ULONG ulIdx;
    if (!pd) return;
    for (ulIdx = 0; ulIdx < pd->ulCount; ulIdx++)
        cfg_free(&pd->paCfgs[ulIdx]);
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

    if (rc == NO_ERROR) {
        if (cfg_add(pd, REUSECFG_KIND_TOML, pszSourceDir, nDepth,
                    hToml, NULLHANDLE) != 0) {
            ReuseClose(hToml);
            return -1;
        }
        return 0;
    }

    {
        ULONG ulSev = REUSE_SEV_ERROR;
        PCSZ pszDetail = "REUSE.toml parse failure";

        switch (rc) {
            case REUSE_ERROR_VERSION_MISSING:
                pszDetail = "'version = 1' missing"; break;
            case REUSE_ERROR_VERSION_NOT_INT:
                pszDetail = "'version' is not an integer"; break;
            case REUSE_ERROR_VERSION_UNSUP:
                pszDetail = "unsupported 'version' value"; break;
            case REUSE_ERROR_ANNOT_NO_PATH:
                pszDetail = "[[annotations]] without 'path'"; break;
            case REUSE_ERROR_ANNOT_BAD_PATH:
                pszDetail = "'path' has wrong type"; break;
            case REUSE_ERROR_ANNOT_BAD_FIELD:
                pszDetail = "field has wrong type"; break;
            case REUSE_ERROR_SYNTAX:
                pszDetail = "syntax error"; break;
            case ERROR_OPEN_FAILED:
                pszDetail = "cannot open"; break;
            case ERROR_READ_FAULT:
                pszDetail = "read error"; break;
            case ERROR_NOT_ENOUGH_MEMORY:
                pszDetail = "out of memory"; break;
        }

        err_add(pd, rc, ulSev, pszPath, 0, pszDetail);
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
    CHAR achPath[2048];
    CHAR achCurrent[2048];
    FILE *fp;
    PCSZ pszRel;
    int nDepth = 0;

    if (!pszRepoRoot) {
        PathMakeJoin(pszTarget, "REUSE.toml", achPath, sizeof(achPath),
                     NULL);
        fp = fopen(achPath, "r");
        if (fp) { fclose(fp); try_add_toml(pd, achPath, pszTarget, 0); }
        return 0;
    }

    PathMakeJoin(pszRepoRoot, "REUSE.toml", achPath, sizeof(achPath),
                 NULL);
    fp = fopen(achPath, "r");
    if (fp) {
        fclose(fp);
        try_add_toml(pd, achPath, pszRepoRoot, nDepth);
    }
    nDepth++;

    pszRel = rel_to_dir(pszRepoRoot, pszTarget);
    if (!pszRel || !pszRel[0] || pszRel == pszTarget) return 0;

    strncpy(achCurrent, pszRepoRoot, sizeof(achCurrent) - 1);
    achCurrent[sizeof(achCurrent) - 1] = '\0';

    {
        size_t cbRi = 0, cbRLen = strlen(pszRel);
        while (cbRi < cbRLen) {
            size_t cbStart = cbRi;
            size_t cbSegLen;
            while (cbRi < cbRLen && pszRel[cbRi] != '/' &&
                   pszRel[cbRi] != '\\')
                cbRi++;
            cbSegLen = cbRi - cbStart;
            if (cbSegLen > 0) {
                size_t cbCurLen = strlen(achCurrent);
                if (cbCurLen + 1 + cbSegLen + 1 > sizeof(achCurrent)) break;
                achCurrent[cbCurLen] = '/';
                memcpy(achCurrent + cbCurLen + 1, pszRel + cbStart, cbSegLen);
                achCurrent[cbCurLen + 1 + cbSegLen] = '\0';

                PathMakeJoin(achCurrent, "REUSE.toml", achPath,
                             sizeof(achPath), NULL);
                fp = fopen(achPath, "r");
                if (fp) {
                    fclose(fp);
                    try_add_toml(pd, achPath, achCurrent, nDepth);
                }
                nDepth++;
            }
            if (cbRi < cbRLen) cbRi++;
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
    CHAR achPath[1024];
    FILE *fp;
    HDEP5DOC hDep5 = NULLHANDLE;
    APIRET rc;

    if (!pszRepoRoot || !pszRepoRoot[0]) return 0;
    {
        CHAR achReuseDir[1024];
        PathMakeJoin(pszRepoRoot, ".reuse", achReuseDir,
                     sizeof(achReuseDir), NULL);
        PathMakeJoin(achReuseDir, "dep5", achPath, sizeof(achPath), NULL);
    }
    fp = fopen(achPath, "r");
    if (!fp) return 0;
    fclose(fp);

    rc = Dep5Open(achPath, &hDep5);
    if (rc == NO_ERROR) {
        if (cfg_add(pd, REUSECFG_KIND_DEP5, pszRepoRoot, -1,
                    NULLHANDLE, hDep5) != 0) {
            Dep5Close(hDep5);
            return -1;
        }
        return 0;
    }
    {
        PCSZ pszDetail = "DEP5 parse failure";

        switch (rc) {
            case DEP5_ERROR_INVALID_SYNTAX: pszDetail = "syntax error"; break;
            case ERROR_OPEN_FAILED:         pszDetail = "cannot open"; break;
            case ERROR_READ_FAULT:          pszDetail = "read error"; break;
            case ERROR_NOT_ENOUGH_MEMORY:   pszDetail = "out of memory"; break;
        }

        err_add(pd, REUSE_ERROR_SYNTAX, REUSE_SEV_ERROR, achPath, 0,
                pszDetail);
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

    PSZ   pszLicense;                   /**< SPDX license expression.      */
    PSZ   pszCopyright;                 /**< Copyright notices.            */
    PSZ   pszContributors;              /**< '\n'-separated contributors.  */
    PSZ   pszPackageName;               /**< SPDX-PackageName.             */
    PSZ   pszPackageSupplier;           /**< SPDX-PackageSupplier.         */
    PSZ   pszPackageDownloadLocation;   /**< SPDX-PackageDownloadLocation. */
    PSZ   pszPackageComment;            /**< SPDX-PackageComment.          */
} MATCH;

/**
 * @brief Release all owned strings of a MATCH and clear its fields.
 *
 * @param[in,out] pMatch  Match. Not NULL.
 */
static void match_clear(MATCH *pMatch) {
    safe_free((void**)&pMatch->pszLicense);
    safe_free((void**)&pMatch->pszCopyright);
    safe_free((void**)&pMatch->pszContributors);
    safe_free((void**)&pMatch->pszPackageName);
    safe_free((void**)&pMatch->pszPackageSupplier);
    safe_free((void**)&pMatch->pszPackageDownloadLocation);
    safe_free((void**)&pMatch->pszPackageComment);
    pMatch->nPrecedence = 0;
    pMatch->nDepth = 0;
    pMatch->nOrderInFile = 0;
}

/**
 * @brief Decide whether @p pA outranks @p pB.
 *
 * Order of comparison: precedence, then depth, then order in file.
 *
 * @param[in] pA  Candidate. Not NULL.
 * @param[in] pB  Current best. Not NULL.
 *
 * @return 1 if @p pA is better than @p pB, 0 otherwise.
 */
static int match_better(const MATCH *pA, const MATCH *pB) {
    if (pA->nPrecedence != pB->nPrecedence)
        return pA->nPrecedence > pB->nPrecedence;
    if (pA->nDepth != pB->nDepth)
        return pA->nDepth > pB->nDepth;
    return pA->nOrderInFile > pB->nOrderInFile;
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
static PSZ ann_get_str(HREUSEANN hAnn,
                       ULONG (*fn)(HREUSEANN, PSZ, ULONG, PULONG)) {
    ULONG ulSize = 0;
    PSZ pszBuf;
    if (fn(hAnn, NULL, 0, &ulSize) != NO_ERROR) return NULL;
    if (ulSize == 0) return NULL;
    pszBuf = (PSZ)malloc(ulSize);
    if (!pszBuf) return NULL;
    if (fn(hAnn, pszBuf, ulSize, NULL) != NO_ERROR) {
        free(pszBuf);
        return NULL;
    }
    return pszBuf;
}

/**
 * @brief Populate a MATCH from one TOML annotation.
 *
 * @param[in]  hAnn    Annotation handle. Not NULLHANDLE.
 * @param[in]  nDepth  Depth from repository root.
 * @param[in]  nOrder  Order of the annotation inside the file.
 * @param[out] pMatch  Receiver. Not NULL.
 *
 * @return 0 on success.
 */
static int match_from_toml(HREUSEANN hAnn, int nDepth, int nOrder,
                           MATCH *pMatch) {
    ULONG ulPrec = REUSE_PRECEDENCE_CLOSEST;
    ReuseAnnGetPrecedence(hAnn, &ulPrec);
    pMatch->nPrecedence = (int)ulPrec;
    pMatch->nDepth = nDepth;
    pMatch->nOrderInFile = nOrder;

    pMatch->pszLicense    = ann_get_str(hAnn, ReuseAnnGetLicense);
    pMatch->pszCopyright  = ann_get_str(hAnn, ReuseAnnGetCopyright);
    pMatch->pszPackageName = ann_get_str(hAnn, ReuseAnnGetPackageName);
    pMatch->pszPackageSupplier =
        ann_get_str(hAnn, ReuseAnnGetPackageSupplier);
    pMatch->pszPackageDownloadLocation =
        ann_get_str(hAnn, ReuseAnnGetPackageDownloadLocation);
    pMatch->pszPackageComment =
        ann_get_str(hAnn, ReuseAnnGetPackageComment);

    {
        ULONG ulCount = 0, ulIdx;
        if (ReuseAnnGetContributorCount(hAnn, &ulCount) == NO_ERROR &&
            ulCount > 0) {
            size_t cbTotal = 0, cbPos = 0;
            PSZ pszJoined;
            for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
                ULONG ulSize = 0;
                ReuseAnnGetContributor(hAnn, ulIdx, NULL, 0, &ulSize);
                cbTotal += ulSize;
                if (ulIdx > 0) cbTotal += 1;
            }
            pszJoined = (PSZ)malloc(cbTotal + 1);
            if (pszJoined) {
                for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
                    ULONG ulSize = 0;
                    ReuseAnnGetContributor(hAnn, ulIdx, NULL, 0, &ulSize);
                    if (ulIdx > 0) pszJoined[cbPos++] = '\n';
                    if (ulSize > 1) {
                        ReuseAnnGetContributor(hAnn, ulIdx,
                                               pszJoined + cbPos,
                                               ulSize, NULL);
                        cbPos += ulSize - 1;
                    }
                }
                pszJoined[cbPos] = '\0';
                pMatch->pszContributors = pszJoined;
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
 * @param[out] pMatch   Receiver. Not NULL.
 *
 * @return 0 if at least one annotation matched, -1 otherwise.
 */
static int toml_find_best(HREUSETOML hToml, int nDepth,
                          PCSZ pszRel, MATCH *pMatch) {
    ULONG ulCount = 0, ulIdx;
    int fFound = 0;

    if (ReuseGetAnnotationCount(hToml, &ulCount) != NO_ERROR)
        return -1;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HREUSEANN hAnn = NULLHANDLE;
        ULONG ulPatCount = 0, ulJ;
        int fMatch = 0;

        if (ReuseGetAnnotation(hToml, ulIdx, &hAnn) != NO_ERROR)
            continue;
        if (ReuseAnnGetPathCount(hAnn, &ulPatCount) != NO_ERROR)
            continue;

        for (ulJ = 0; ulJ < ulPatCount; ulJ++) {
            CHAR achPat[1024];
            ULONG ulUsed = 0;
            if (ReuseAnnGetPath(hAnn, ulJ, achPat, sizeof(achPat), &ulUsed)
                != NO_ERROR)
                continue;
            if (matches_pattern(achPat, pszRel)) { fMatch = 1; break; }
        }

        if (fMatch) {
            MATCH cand;
            memset(&cand, 0, sizeof(cand));
            match_from_toml(hAnn, nDepth, (int)ulIdx, &cand);
            if (!fFound || match_better(&cand, pMatch)) {
                match_clear(pMatch);
                *pMatch = cand;
            } else {
                match_clear(&cand);
            }
            fFound = 1;
        }
    }
    return fFound ? 0 : -1;
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
static PSZ dep5_get_field(HDEP5FIND hFind, PCSZ pszField) {
    ULONG ulSize = 0;
    PSZ pszBuf;
    if (Dep5FilesGetField(hFind, pszField, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszBuf = (PSZ)malloc(ulSize);
    if (!pszBuf) return NULL;
    if (Dep5FilesGetField(hFind, pszField, pszBuf, ulSize, NULL)
        != NO_ERROR) {
        free(pszBuf);
        return NULL;
    }
    return pszBuf;
}

/**
 * @brief Populate a MATCH from one DEP5 Files stanza.
 *
 * @param[in]  hFind   Cursor. Not NULLHANDLE.
 * @param[in]  nDepth  Depth from repository root.
 * @param[in]  nOrder  Order of the stanza inside the file.
 * @param[out] pMatch  Receiver. Not NULL.
 *
 * @return 0 on success.
 */
static int match_from_dep5(HDEP5FIND hFind, int nDepth, int nOrder,
                           MATCH *pMatch) {
    pMatch->nPrecedence = REUSE_PRECEDENCE_CLOSEST;
    pMatch->nDepth = nDepth;
    pMatch->nOrderInFile = nOrder;

    pMatch->pszLicense   = dep5_get_field(hFind, "License");
    pMatch->pszCopyright = dep5_get_field(hFind, "Copyright");
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
 * @param[out] pMatch  Receiver. Not NULL.
 *
 * @return 0 if at least one stanza matched, -1 otherwise.
 */
static int dep5_find_best(HDEP5DOC hDep5, int nDepth,
                          PCSZ pszRel, MATCH *pMatch) {
    HDEP5FIND hFind = NULLHANDLE;
    ULONG ulTotal = 0;
    APIRET rc;
    int fFound = 0;
    int nCur = 0;

    rc = Dep5FilesFindFirst(hDep5, &hFind, &ulTotal);
    if (rc != NO_ERROR) return -1;

    do {
        ULONG ulPatCount = 0, ulJ;
        int fMatch = 0;

        if (Dep5FilesGetPatternCount(hFind, &ulPatCount) != NO_ERROR)
            continue;

        for (ulJ = 0; ulJ < ulPatCount; ulJ++) {
            CHAR achPat[1024];
            ULONG ulUsed = 0;
            if (Dep5FilesGetPattern(hFind, ulJ, achPat, sizeof(achPat),
                                    &ulUsed) != NO_ERROR)
                continue;
            if (matches_pattern(achPat, pszRel)) { fMatch = 1; break; }
        }

        if (fMatch) {
            match_clear(pMatch);
            match_from_dep5(hFind, nDepth, nCur, pMatch);
            fFound = 1;
        }
        nCur++;
    } while (Dep5FilesFindNext(hFind) == NO_ERROR);

    Dep5FilesFindClose(hFind);
    return fFound ? 0 : -1;
}

/* ==================================================================
 * Config-level match dispatch
 * ================================================================== */

/**
 * @brief Find the best match for a file inside one config entry.
 *
 * @param[in]  pCfg     Config entry. Not NULL.
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] pMatch   Receiver. Not NULL.
 *
 * @return 0 if a match was found, -1 otherwise.
 */
static int cfg_find_best(REUSECFG *pCfg, PCSZ pszPath, MATCH *pMatch) {
    PCSZ pszRel = rel_to_dir(pCfg->pszSourceDir, pszPath);
    if (pCfg->nKind == REUSECFG_KIND_TOML)
        return toml_find_best(pCfg->hToml, pCfg->nDepth, pszRel, pMatch);
    if (pCfg->nKind == REUSECFG_KIND_DEP5)
        return dep5_find_best(pCfg->hDep5, pCfg->nDepth, pszRel, pMatch);
    return -1;
}

/* ==================================================================
 * Aggregation helpers
 * ================================================================== */

/**
 * @brief Combine two SPDX license expressions with AND.
 *
 * @param[in] pszA  First expression, or NULL.
 * @param[in] pszB  Second expression, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static PSZ aggregate_licenses(PCSZ pszA, PCSZ pszB) {
    size_t cbALen, cbBLen;
    PSZ pszResult;
    if (!pszA) return pszB ? strdup(pszB) : NULL;
    if (!pszB) return strdup(pszA);
    cbALen = strlen(pszA);
    cbBLen = strlen(pszB);
    pszResult = (PSZ)malloc(cbALen + cbBLen + 8);
    if (!pszResult) return NULL;
    sprintf(pszResult, "(%s) AND (%s)", pszA, pszB);
    return pszResult;
}

/**
 * @brief Concatenate two copyright texts with '\n'.
 *
 * @param[in] pszA  First text, or NULL.
 * @param[in] pszB  Second text, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static PSZ aggregate_copyrights(PCSZ pszA, PCSZ pszB) {
    size_t cbALen, cbBLen;
    PSZ pszResult;
    if (!pszA) return pszB ? strdup(pszB) : NULL;
    if (!pszB) return strdup(pszA);
    cbALen = strlen(pszA);
    cbBLen = strlen(pszB);
    pszResult = (PSZ)malloc(cbALen + cbBLen + 2);
    if (!pszResult) return NULL;
    sprintf(pszResult, "%s\n%s", pszA, pszB);
    return pszResult;
}

/**
 * @brief Check whether a '\n'-separated text contains a given line.
 *
 * @param[in] pszHay     Text. Not NULL.
 * @param[in] pszNeedle  Line to look for. Not NULL.
 * @param[in] cbLen      Length of @p pszNeedle.
 *
 * @return 1 if found, 0 otherwise.
 */
static int contains_line(PCSZ pszHay, PCSZ pszNeedle, size_t cbLen) {
    PCSZ pszPos = pszHay;
    while (*pszPos) {
        PCSZ pszEol = strchr(pszPos, '\n');
        size_t cbLineLen = pszEol ? (size_t)(pszEol - pszPos)
                                  : strlen(pszPos);
        if (cbLineLen == cbLen && memcmp(pszPos, pszNeedle, cbLen) == 0)
            return 1;
        if (!pszEol) break;
        pszPos = pszEol + 1;
    }
    return 0;
}

/**
 * @brief Append lines of @p pszB to @p pszA, dropping duplicate lines.
 *
 * @param[in] pszA  First text, or NULL.
 * @param[in] pszB  Second text, or NULL.
 *
 * @return malloc'd combination, or NULL on allocation failure.
 */
static PSZ join_lines_dedup(PCSZ pszA, PCSZ pszB) {
    size_t cbALen, cbBLen, cbOutLen, cbCap;
    PSZ pszOut;
    PCSZ pszPos;

    if (!pszA && !pszB) return NULL;
    if (!pszA) return strdup(pszB);
    if (!pszB) return strdup(pszA);

    cbALen = strlen(pszA);
    cbBLen = strlen(pszB);
    cbCap = cbALen + cbBLen + 2;
    pszOut = (PSZ)malloc(cbCap);
    if (!pszOut) return NULL;
    memcpy(pszOut, pszA, cbALen);
    pszOut[cbALen] = '\0';
    cbOutLen = cbALen;

    pszPos = pszB;
    while (*pszPos) {
        PCSZ pszEol = strchr(pszPos, '\n');
        size_t cbLen = pszEol ? (size_t)(pszEol - pszPos) : strlen(pszPos);
        if (!contains_line(pszOut, pszPos, cbLen)) {
            if (cbOutLen + 1 + cbLen + 1 > cbCap) {
                PSZ pszNew;
                cbCap = cbOutLen + 1 + cbLen + 2;
                pszNew = (PSZ)realloc(pszOut, cbCap);
                if (!pszNew) { free(pszOut); return NULL; }
                pszOut = pszNew;
            }
            pszOut[cbOutLen++] = '\n';
            memcpy(pszOut + cbOutLen, pszPos, cbLen);
            cbOutLen += cbLen;
            pszOut[cbOutLen] = '\0';
        }
        if (!pszEol) break;
        pszPos = pszEol + 1;
    }
    return pszOut;
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
    PREUSETREEFILE pFile;
    CHAR achSidecar[1200];
    PSZ pszTagLicense = NULL;
    PSZ pszTagCopyright = NULL;
    PSZ pszSideLicense = NULL;
    PSZ pszSideCopyright = NULL;
    PCSZ pszInLicense = NULL;
    PCSZ pszInCopyright = NULL;
    MATCH bestOverride, bestAggregate, bestClosest;
    int fHaveOverride = 0, fHaveAggregate = 0, fHaveClosest = 0;
    ULONG ulIdx;

    memset(&bestOverride, 0, sizeof(bestOverride));
    memset(&bestAggregate, 0, sizeof(bestAggregate));
    memset(&bestClosest, 0, sizeof(bestClosest));

    pFile = (PREUSETREEFILE)calloc(1, sizeof(REUSETREEFILE));
    if (!pFile) return NULL;

    /* 1. Read sidecar (REUSE 3.3 §4.1.3): <file>.license */
    snprintf(achSidecar, sizeof(achSidecar), "%s.license", pszPath);
    {
        FILE *fp = fopen(achSidecar, "rb");
        if (fp) {
            fclose(fp);
            pszSideLicense = query_file_license(achSidecar);
            pszSideCopyright = query_file_copyright(achSidecar);
        }
    }

    /* 2. Read tags inside the file */
    pszTagLicense = query_file_license(pszPath);
    pszTagCopyright = query_file_copyright(pszPath);

    /* 3. Sidecar takes precedence over in-file tags */
    pszInLicense = pszSideLicense ? pszSideLicense : pszTagLicense;
    pszInCopyright = pszSideCopyright ? pszSideCopyright : pszTagCopyright;

    if (pszInLicense || pszInCopyright) pFile->bHasReuse = TRUE_;

    /* 4. For each config, find the best matching annotation, then
     *    classify it by precedence into override/aggregate/closest. */
    for (ulIdx = 0; ulIdx < pd->ulCount; ulIdx++) {
        MATCH m;
        memset(&m, 0, sizeof(m));
        if (cfg_find_best(&pd->paCfgs[ulIdx], pszPath, &m) != 0) continue;
        pFile->bHasReuse = TRUE_;

        switch (m.nPrecedence) {
            case REUSE_PRECEDENCE_OVERRIDE:
                if (!fHaveOverride || match_better(&m, &bestOverride)) {
                    match_clear(&bestOverride);
                    bestOverride = m;
                    fHaveOverride = 1;
                } else {
                    match_clear(&m);
                }
                break;
            case REUSE_PRECEDENCE_AGGREGATE:
                if (!fHaveAggregate || match_better(&m, &bestAggregate)) {
                    match_clear(&bestAggregate);
                    bestAggregate = m;
                    fHaveAggregate = 1;
                } else {
                    match_clear(&m);
                }
                break;
            default:
                if (!fHaveClosest || match_better(&m, &bestClosest)) {
                    match_clear(&bestClosest);
                    bestClosest = m;
                    fHaveClosest = 1;
                } else {
                    match_clear(&m);
                }
                break;
        }
    }

    /* 5. Compose the result. */
    if (fHaveOverride) {
        pFile->ulPrecedence = REUSE_PRECEDENCE_OVERRIDE;
        if (bestOverride.pszLicense)
            pFile->pszLicense = strdup(bestOverride.pszLicense);
        if (bestOverride.pszCopyright)
            pFile->pszCopyright = strdup(bestOverride.pszCopyright);
        if (bestOverride.pszContributors)
            pFile->pszContributors = strdup(bestOverride.pszContributors);
        if (bestOverride.pszPackageName)
            pFile->pszPackageName = strdup(bestOverride.pszPackageName);
        if (bestOverride.pszPackageSupplier)
            pFile->pszPackageSupplier =
                strdup(bestOverride.pszPackageSupplier);
        if (bestOverride.pszPackageDownloadLocation)
            pFile->pszPackageDownloadLocation =
                strdup(bestOverride.pszPackageDownloadLocation);
        if (bestOverride.pszPackageComment)
            pFile->pszPackageComment =
                strdup(bestOverride.pszPackageComment);
    } else {
        PCSZ pszBaseLicense = fHaveClosest ? bestClosest.pszLicense : NULL;
        PCSZ pszBaseCopyright = fHaveClosest ? bestClosest.pszCopyright
                                             : NULL;
        if (pszInLicense) pszBaseLicense = pszInLicense;
        if (pszInCopyright) pszBaseCopyright = pszInCopyright;

        if (!fHaveAggregate) {
            pFile->ulPrecedence = (pszBaseLicense || pszBaseCopyright ||
                                   fHaveClosest)
                ? REUSE_PRECEDENCE_CLOSEST : 0;
            if (pszBaseLicense) pFile->pszLicense = strdup(pszBaseLicense);
            if (pszBaseCopyright)
                pFile->pszCopyright = strdup(pszBaseCopyright);
            if (fHaveClosest) {
                if (bestClosest.pszContributors)
                    pFile->pszContributors =
                        strdup(bestClosest.pszContributors);
                if (bestClosest.pszPackageName)
                    pFile->pszPackageName =
                        strdup(bestClosest.pszPackageName);
                if (bestClosest.pszPackageSupplier)
                    pFile->pszPackageSupplier =
                        strdup(bestClosest.pszPackageSupplier);
                if (bestClosest.pszPackageDownloadLocation)
                    pFile->pszPackageDownloadLocation =
                        strdup(bestClosest.pszPackageDownloadLocation);
                if (bestClosest.pszPackageComment)
                    pFile->pszPackageComment =
                        strdup(bestClosest.pszPackageComment);
            }
        } else {
            pFile->ulPrecedence = REUSE_PRECEDENCE_AGGREGATE;

            if (pszBaseLicense && bestAggregate.pszLicense)
                pFile->pszLicense = aggregate_licenses(
                    pszBaseLicense, bestAggregate.pszLicense);
            else if (pszBaseLicense)
                pFile->pszLicense = strdup(pszBaseLicense);
            else if (bestAggregate.pszLicense)
                pFile->pszLicense = strdup(bestAggregate.pszLicense);

            if (pszBaseCopyright && bestAggregate.pszCopyright)
                pFile->pszCopyright = aggregate_copyrights(
                    pszBaseCopyright, bestAggregate.pszCopyright);
            else if (pszBaseCopyright)
                pFile->pszCopyright = strdup(pszBaseCopyright);
            else if (bestAggregate.pszCopyright)
                pFile->pszCopyright = strdup(bestAggregate.pszCopyright);

            {
                PCSZ pszA = fHaveClosest
                    ? bestClosest.pszContributors : NULL;
                PCSZ pszB = bestAggregate.pszContributors;
                if (pszA && pszB) pFile->pszContributors =
                    join_lines_dedup(pszA, pszB);
                else if (pszA) pFile->pszContributors = strdup(pszA);
                else if (pszB) pFile->pszContributors = strdup(pszB);
            }

            if (fHaveClosest && bestClosest.pszPackageName)
                pFile->pszPackageName =
                    strdup(bestClosest.pszPackageName);
            else if (bestAggregate.pszPackageName)
                pFile->pszPackageName =
                    strdup(bestAggregate.pszPackageName);

            if (fHaveClosest && bestClosest.pszPackageSupplier)
                pFile->pszPackageSupplier =
                    strdup(bestClosest.pszPackageSupplier);
            else if (bestAggregate.pszPackageSupplier)
                pFile->pszPackageSupplier =
                    strdup(bestAggregate.pszPackageSupplier);

            if (fHaveClosest &&
                bestClosest.pszPackageDownloadLocation)
                pFile->pszPackageDownloadLocation =
                    strdup(bestClosest.pszPackageDownloadLocation);
            else if (bestAggregate.pszPackageDownloadLocation)
                pFile->pszPackageDownloadLocation =
                    strdup(bestAggregate.pszPackageDownloadLocation);

            if (fHaveClosest && bestClosest.pszPackageComment)
                pFile->pszPackageComment =
                    strdup(bestClosest.pszPackageComment);
            else if (bestAggregate.pszPackageComment)
                pFile->pszPackageComment =
                    strdup(bestAggregate.pszPackageComment);
        }
    }

    match_clear(&bestOverride);
    match_clear(&bestAggregate);
    match_clear(&bestClosest);

    free(pszTagLicense);
    free(pszTagCopyright);
    free(pszSideLicense);
    free(pszSideCopyright);

    return pFile;
}

/* ==================================================================
 * Handle helpers
 * ================================================================== */

/**
 * @brief Translate a public project handle into the internal pointer.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 */
PREUSETREE ReuseInternalGetDoc(HREUSETREE hDoc) {
    return (PREUSETREE)hDoc;
}

/**
 * @brief Translate a public file handle into the internal pointer.
 *
 * @param[in] hFile  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 */
PREUSETREEFILE ReuseInternalGetFile(HREUSETREEFILE hFile) {
    return (PREUSETREEFILE)hFile;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Open a REUSE project rooted at a directory.
 *
 * Scans the Git repository that contains pszDir (if any) and
 * collects every REUSE.toml file from the repository root down to
 * pszDir, in increasing depth order. Also reads .reuse/dep5 at the
 * repository root, if present.
 *
 * Parse errors in discovered files are recorded as REUSEERR
 * entries in the project handle; the offending source is skipped.
 *
 * @param[in]  pszDir  Target directory. Not NULL.
 * @param[out] phDoc   Handle receiver. Not NULL. Set to NULLHANDLE on
 *                     error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (possibly with recorded
 *                                  diagnostics).
 * @retval ERROR_INVALID_PARAMETER  pszDir or phDoc is NULL.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 *
 * @see ReuseTreeGetError, ReuseTreeClose
 */
APIRET APIENTRY ReuseTreeOpen(PCSZ pszDir, HREUSETREE *phDoc) {
    PREUSETREE pd;
    PSZ pszRepoRoot = NULL;

    if (!pszDir || !phDoc) return ERROR_INVALID_PARAMETER;
    *phDoc = NULLHANDLE;

    pd = (PREUSETREE)calloc(1, sizeof(REUSETREE));
    if (!pd) return ERROR_NOT_ENOUGH_MEMORY;

    pd->pszProjectDir = strdup(pszDir);
    if (!pd->pszProjectDir) {
        doc_free(pd);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    /* Two-phase GitFindRepoRoot call: first query the size, then
     * allocate and query the value. */
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
    pd->pszRepoRoot = pszRepoRoot; /* may be NULL */

    if (discover_tomls(pd, pszRepoRoot, pszDir) != 0) {
        doc_free(pd);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (discover_dep5(pd, pszRepoRoot) != 0) {
        doc_free(pd);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *phDoc = (HREUSETREE)pd;
    return NO_ERROR;
}

/**
 * @brief Close a project and release all associated memory.
 *
 * All HREUSETREEFILE handles and REUSEERR records obtained from this
 * project become invalid.
 *
 * @param[in] hDoc  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 *
 * @warning Do not call ReuseTreeClose twice with the same handle.
 */
APIRET APIENTRY ReuseTreeClose(HREUSETREE hDoc) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (hDoc == NULLHANDLE) return NO_ERROR;
    if (!pd) return ERROR_INVALID_HANDLE;
    doc_free(pd);
    return NO_ERROR;
}

/**
 * @brief Number of REUSEERR records stored in the project.
 *
 * Errors, warnings and informational records are counted together.
 * Use ReuseTreeGetError to inspect severity.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseTreeGetErrorCount(HREUSETREE hDoc, PULONG pulCount) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (!pd || !pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = pd->ulErrorCount;
    return NO_ERROR;
}

/**
 * @brief Retrieve one diagnostic record by index.
 *
 * The returned REUSEERR is a snapshot; string fields point to data
 * owned by the project handle and remain valid until ReuseTreeClose.
 *
 * @param[in]  hDoc    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index, < ReuseTreeGetErrorCount.
 * @param[out] pErr    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY ReuseTreeGetError(HREUSETREE hDoc, ULONG ulIndex,
                                  PREUSEERR pErr) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    if (!pd || !pErr) return ERROR_INVALID_PARAMETER;
    if (ulIndex >= pd->ulErrorCount) return ERROR_NO_MORE_ITEMS;
    *pErr = pd->paErrors[ulIndex];
    return NO_ERROR;
}

/**
 * @brief Resolve licensing information for one file.
 *
 * Algorithm (REUSE 3.3 §4.1.2, §4.1.3):
 *   1. Read the adjacent <file>.license sidecar, if present.
 *   2. Read SPDX tags inside the file (outside REUSE-IgnoreStart/End
 *      regions; snippet bodies are not treated as file-level tags).
 *   3. Apply precedence across all discovered REUSE.toml files that
 *      match the file, using sidecar/tag data as the in-file source.
 *   4. Aggregate results as prescribed by the winning precedence.
 *
 * A successful resolution does not imply the file had any licensing
 * information. Use ReuseTreeFileGetHasReuse to distinguish "no
 * sources matched" from "sources matched but were empty".
 *
 * @param[in]  hDoc    Handle. Not NULLHANDLE.
 * @param[in]  pszPath Path to the file. May be absolute or relative.
 * @param[out] phFile  Handle receiver. Not NULL. Set to NULLHANDLE on
 *                     error.
 * @param[out] pErr    Optional. May be NULL. If not NULL and a
 *                     file-specific diagnostic occurred, the first
 *                     such record is stored here.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDoc, pszPath or phFile is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval REUSE_ERROR_SYNTAX       Sidecar contains a syntax error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 */
APIRET APIENTRY ReuseTreeResolveFile(HREUSETREE hDoc, PCSZ pszPath,
                                     HREUSETREEFILE *phFile,
                                     PREUSEERR pErr) {
    PREUSETREE pd = ReuseInternalGetDoc(hDoc);
    PREUSETREEFILE pFile;

    if (!pd || !pszPath || !phFile) return ERROR_INVALID_PARAMETER;
    *phFile = NULLHANDLE;

    pFile = resolve_file(pd, pszPath);
    if (!pFile) return ERROR_NOT_ENOUGH_MEMORY;

    *phFile = (HREUSETREEFILE)pFile;
    (void)pErr;
    return NO_ERROR;
}

/**
 * @brief Release a resolution handle.
 *
 * @param[in] hFile  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY ReuseTreeFileClose(HREUSETREEFILE hFile) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (hFile == NULLHANDLE) return NO_ERROR;
    if (!pFile) return ERROR_INVALID_HANDLE;
    safe_free((void**)&pFile->pszLicense);
    safe_free((void**)&pFile->pszCopyright);
    safe_free((void**)&pFile->pszContributors);
    safe_free((void**)&pFile->pszPackageName);
    safe_free((void**)&pFile->pszPackageSupplier);
    safe_free((void**)&pFile->pszPackageDownloadLocation);
    safe_free((void**)&pFile->pszPackageComment);
    free(pFile);
    return NO_ERROR;
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
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszVal   Value, or NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     pszVal is NULL.
 * @retval ERROR_INVALID_PARAMETER  pszBuf is NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
static APIRET copy_out(PCSZ pszVal, PSZ pszBuf, ULONG ulSize,
                       PULONG pulUsed) {
    size_t cbLen;
    if (!pszVal) return ERROR_FILE_NOT_FOUND;
    cbLen = strlen(pszVal);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszVal, cbLen);
    pszBuf[cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/**
 * @brief Retrieve the resolved SPDX license expression.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     The winning sources define no
 *                                  license.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetLicense(HREUSETREEFILE hFile,
                                        PSZ pszBuf, ULONG ulSize,
                                        PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszLicense, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the resolved copyright text.
 *
 * Multiple notices are joined with '\n'.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     The winning sources define no
 *                                  copyright.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetCopyright(HREUSETREEFILE hFile,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszCopyright, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the resolved SPDX-FileContributor list.
 *
 * Multiple contributors are joined with '\n'.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     No contributor was defined.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetContributors(HREUSETREEFILE hFile,
                                             PSZ pszBuf, ULONG ulSize,
                                             PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszContributors, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageName.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     Field absent.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetPackageName(HREUSETREEFILE hFile,
                                            PSZ pszBuf, ULONG ulSize,
                                            PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszPackageName, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageSupplier.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     Field absent.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetPackageSupplier(HREUSETREEFILE hFile,
                                                PSZ pszBuf, ULONG ulSize,
                                                PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszPackageSupplier, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageDownloadLocation.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     Field absent.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetPackageDownloadLocation(
    HREUSETREEFILE hFile, PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszPackageDownloadLocation, pszBuf, ulSize,
                    pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageComment.
 *
 * @param[in]  hFile    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     Field absent.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY ReuseTreeFileGetPackageComment(HREUSETREEFILE hFile,
                                               PSZ pszBuf, ULONG ulSize,
                                               PULONG pulUsed) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile) return ERROR_INVALID_PARAMETER;
    return copy_out(pFile->pszPackageComment, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the precedence of the winning source.
 *
 * @param[in]  hFile          Handle. Not NULLHANDLE.
 * @param[out] pulPrecedence  Receiver: one of REUSE_PRECEDENCE_*.
 *                            Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hFile or pulPrecedence is NULL.
 */
APIRET APIENTRY ReuseTreeFileGetPrecedence(HREUSETREEFILE hFile,
                                           PULONG pulPrecedence) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile || !pulPrecedence) return ERROR_INVALID_PARAMETER;
    *pulPrecedence = pFile->ulPrecedence;
    return NO_ERROR;
}

/**
 * @brief Whether any REUSE.toml, DEP5, sidecar or tag matched the
 *        file.
 *
 * @param[in]  hFile       Handle. Not NULLHANDLE.
 * @param[out] pfHasReuse  Receiver: TRUE_ if at least one source
 *                         matched; FALSE_ otherwise. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hFile or pfHasReuse is NULL.
 */
APIRET APIENTRY ReuseTreeFileGetHasReuse(HREUSETREEFILE hFile,
                                         PBOOL pfHasReuse) {
    PREUSETREEFILE pFile = ReuseInternalGetFile(hFile);
    if (!pFile || !pfHasReuse) return ERROR_INVALID_PARAMETER;
    *pfHasReuse = pFile->bHasReuse;
    return NO_ERROR;
}
