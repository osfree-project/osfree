/*!
 * @file reuse_internal.h
 *
 * @brief Private interface of the REUSE project resolver.
 *
 * Internal structures of the REUSE project resolver. Do not include
 * from consumer code.
 */

#ifndef REUSE_INTERNAL_H
#define REUSE_INTERNAL_H

#include "os2types.h"
#include "os2err.h"
#include "reuse.h"
#include "reuse_toml.h"
#include "dep5.h"

/*!
 * @brief Forward declaration of one configuration source.
 */
typedef struct _REUSECFG       REUSECFG,       *PREUSECFG;

/*!
 * @brief Forward declaration of an open project.
 */
typedef struct _REUSETREE      REUSETREE,      *PREUSETREE;

/*!
 * @brief Forward declaration of a per-file resolution result.
 */
typedef struct _REUSETREEFILE  REUSETREEFILE,  *PREUSETREEFILE;

/* ------------------------------------------------------------------
 * One discovered source: a REUSE.toml file or the .reuse/dep5 file.
 *
 * Two forms of storage:
 *   - TOML: owns a live HREUSETOML from reuse_toml; the source
 *     directory is the directory containing REUSE.toml; the depth
 *     reflects how many path components separate it from the project
 *     root (0 = root).
 *   - DEP5: owns a live HDEP5DOC from dep5; the source directory is
 *     the repository root; DEP5 has a fixed precedence relative to
 *     REUSE.toml (see REUSE 3.3 §5.2), so its depth is assigned a
 *     value lower than any TOML entry.
 * ------------------------------------------------------------------ */

/*!
 * @def REUSECFG_KIND_TOML
 * @brief Config entry holds a REUSE.toml.
 */
#define REUSECFG_KIND_TOML  1

/*!
 * @def REUSECFG_KIND_DEP5
 * @brief Config entry holds a .reuse/dep5.
 */
#define REUSECFG_KIND_DEP5  2

/*!
 * @struct _REUSECFG
 * @brief One configuration source.
 */
struct _REUSECFG {
    int         nKind;         /*!< REUSECFG_KIND_*.              */
    PSZ         pszSourceDir;  /*!< Scope for pattern matching.   */
    int         nDepth;        /*!< Smaller = closer to the root. */

    HREUSETOML  hToml;         /*!< Valid if nKind == TOML.       */
    HDEP5DOC    hDep5;         /*!< Valid if nKind == DEP5.       */
};

/* ------------------------------------------------------------------
 * One open project.
 *
 * The configuration list is kept sorted by depth (ascending). All
 * parse-time diagnostics (from REUSE.toml or DEP5) are copied into
 * the paErrors array; the corresponding sources are skipped.
 * ------------------------------------------------------------------ */

/*!
 * @struct _REUSETREE
 * @brief Open project handle.
 */
struct _REUSETREE {
    REUSECFG   *paCfgs;            /*!< Configuration sources.    */
    ULONG       ulCount;           /*!< Number of used entries.   */
    ULONG       ulCapacity;        /*!< Allocated capacity.       */

    REUSEERR   *paErrors;          /*!< Recorded diagnostics.     */
    ULONG       ulErrorCount;      /*!< Number of used entries.   */
    ULONG       ulErrorCapacity;   /*!< Allocated capacity.       */

    PSZ         pszProjectDir;     /*!< Argument to ReuseTreeOpen.*/
    PSZ         pszRepoRoot;       /*!< Git repo root, or NULL.   */
};

/* ------------------------------------------------------------------
 * One resolution result.
 *
 * All string fields are owned by this structure and released by
 * ReuseTreeFileClose. They are NULL when the corresponding field was
 * not resolved.
 *
 * ulPrecedence is 0 when nothing matched; otherwise it is one of
 * REUSE_PRECEDENCE_*. bHasReuse is TRUE when at least one source
 * matched the file (even if only sidecar or tag data was found).
 * ------------------------------------------------------------------ */

/*!
 * @struct _REUSETREEFILE
 * @brief Per-file resolution result.
 */
struct _REUSETREEFILE {
    PSZ         pszLicense;                /*!< SPDX license expr.   */
    PSZ         pszCopyright;              /*!< Copyright text.      */
    PSZ         pszContributors;           /*!< '\n'-separated.      */
    PSZ         pszPackageName;            /*!< SPDX-PackageName.    */
    PSZ         pszPackageSupplier;        /*!< SPDX-PackageSupplier.*/
    PSZ         pszPackageDownloadLocation;/*!< Download location.   */
    PSZ         pszPackageComment;         /*!< Package comment.     */
    ULONG       ulPrecedence;              /*!< REUSE_PRECEDENCE_*.  */
    BOOL        bHasReuse;                 /*!< Any source matched.  */
};

/* ------------------------------------------------------------------
 * Internal helpers (used by reuse.c and reuse_lic.c).
 * ------------------------------------------------------------------ */

/*!
 * @brief Translate a public project handle into the internal pointer.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL on failure.
 *
 * @retval NULL  hDoc is NULLHANDLE.
 */
PREUSETREE ReuseInternalGetDoc(HREUSETREE hDoc);

/*!
 * @brief Translate a public file handle into the internal pointer.
 *
 * @param[in] hFile  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL on failure.
 *
 * @retval NULL  hFile is NULLHANDLE.
 */
PREUSETREEFILE ReuseInternalGetFile(HREUSETREEFILE hFile);

#endif /* REUSE_INTERNAL_H */
