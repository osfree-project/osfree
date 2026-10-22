/* reuse_discover.h - REUSE file discovery with .gitignore filtering
 * (C89) */
#ifndef REUSE_DISCOVER_H
#define REUSE_DISCOVER_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_discover.h
 * @brief REUSE file discovery.
 *
 * Walks a directory tree and collects the set of files that are
 * Covered Files by REUSE 3.3, or extracts source file names from a
 * list of build artifacts (OMF object files and RES resource files).
 *
 * In walk mode, applies the REUSE skip rules (LICENSES/, .reuse/,
 * REUSE.toml, .license sidecars, license text files, SPDX documents,
 * VCS directories, hidden files, empty files) and the rules of
 * .gitignore. The .gitignore rules are applied unconditionally when
 * pszDir is inside a Git repository, as required by REUSE 3.3
 * "Covered and ignored Files".
 *
 * In artifacts mode, each artifact is passed to the OMF reader and,
 * if the OMF reader yields no names, to the RES reader. Names
 * extracted from artifacts are appended to hOut as-is; no path
 * normalisation is performed. Format detection is heuristic: a RES
 * file whose first bytes happen to form a valid OMF THEADR record
 * may be misclassified as OMF. This is rare in practice and does
 * not affect well-formed artifacts produced by standard toolchains.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - gitignore(5). https://git-scm.com/docs/gitignore
 */

/**
 * @brief Discover source files for a REUSE project.
 *
 * Two modes, selected by @p papszArtifacts and @p ulArtifactCount:
 *
 *   - Walk mode (@p papszArtifacts is NULL, or @p ulArtifactCount
 *     is 0): walks @p pszDir, applying the REUSE skip rules and
 *     .gitignore, and appends every Covered File path to @p hOut.
 *     The path is @p pszDir joined with the file's path relative
 *     to it; no further normalisation is performed.
 *
 *   - Artifacts mode (@p papszArtifacts is not NULL and
 *     @p ulArtifactCount is greater than 0): reads each artifact,
 *     tries the OMF reader first and, if it yields no names, the
 *     RES reader, and appends the source names it carries to
 *     @p hOut. Names are appended as-is; no path normalisation is
 *     performed. @p pszDir and @p fRecursive are ignored.
 *
 * `hOut` is a set: duplicate paths or names are silently dropped.
 * The caller owns @p hOut and must release it with StrSetDestroy.
 *
 * @param[in] pszDir          Target directory. Not NULL. Used only in
 *                            walk mode.
 * @param[in] papszArtifacts  Array of artifact paths, or NULL.
 * @param[in] ulArtifactCount Number of entries in @p papszArtifacts.
 *                            0 selects walk mode.
 * @param[in] fRecursive      TRUE to recurse into subdirectories in
 *                            walk mode. Ignored in artifacts mode.
 * @param[in] hOut            Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Walk mode: @p pszDir was
 *                                  processed (possibly empty).
 *                                  Artifacts mode: at least one
 *                                  artifact yielded source names.
 * @retval ERROR_INVALID_PARAMETER  @p pszDir or @p hOut is NULL, or
 *                                  @p papszArtifacts is NULL with
 *                                  non-zero @p ulArtifactCount, or a
 *                                  @p papszArtifacts element is NULL.
 * @retval ERROR_OPEN_FAILED        Walk mode: @p pszDir cannot be
 *                                  opened.
 * @retval ERROR_FILE_NOT_FOUND     Artifacts mode: none of the
 *                                  artifacts yielded source names.
 *                                  Typically means the artifacts
 *                                  are corrupted, in an unsupported
 *                                  format, or contain no source
 *                                  information.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseDiscoverFiles(PCSZ pszDir,
                                   PCSZ *papszArtifacts,
                                   ULONG ulArtifactCount,
                                   BOOL fRecursive,
                                   HSTRSET hOut);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_DISCOVER_H */
