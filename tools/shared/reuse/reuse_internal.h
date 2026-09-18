/* reuse_internal.h - internal structures of the REUSE project resolver.
 * Do not include from consumer code. */
#ifndef REUSE_INTERNAL_H
#define REUSE_INTERNAL_H

#include "reuse.h"
#include "reuse_toml.h"
#include "dep5.h"

/**
 * @file reuse_internal.h
 * @brief Private interface of the REUSE project resolver.
 */

typedef struct _REUSECFG  REUSECFG,  *PREUSECFG;
typedef struct _REUSEDOC  REUSEDOC,  *PREUSEDOC;
typedef struct _REUSEFILE REUSEFILE, *PREUSEFILE;

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

#define REUSECFG_KIND_TOML  1
#define REUSECFG_KIND_DEP5  2

struct _REUSECFG {
    int         nKind;          /* REUSECFG_KIND_*                    */
    char       *pszSourceDir;   /* scope for pattern matching         */
    int         nDepth;         /* smaller = closer to the root       */

    HREUSETOML  hToml;          /* valid if nKind == TOML             */
    HDEP5DOC    hDep5;          /* valid if nKind == DEP5             */
};

/* ------------------------------------------------------------------
 * One open project.
 *
 * The configuration list is kept sorted by depth (ascending). All
 * parse-time diagnostics (from REUSE.toml or DEP5) are copied into
 * the paErrors array; the corresponding sources are skipped.
 * ------------------------------------------------------------------ */

struct _REUSEDOC {
    REUSECFG   *paCfgs;
    ULONG       ulCount;
    ULONG       ulCapacity;

    REUSEERR   *paErrors;
    ULONG       ulErrorCount;
    ULONG       ulErrorCapacity;

    char       *pszProjectDir;  /* argument passed to ReuseOpen       */
    char       *pszRepoRoot;    /* git repository root, or NULL       */
};

/* ------------------------------------------------------------------
 * One resolution result.
 *
 * All string fields are owned by this structure and released by
 * ReuseFileClose. They are NULL when the corresponding field was not
 * resolved.
 *
 * ulPrecedence is 0 when nothing matched; otherwise it is one of
 * REUSE_PRECEDENCE_*. bHasReuse is TRUE_ when at least one source
 * matched the file (even if only sidecar or tag data was found).
 * ------------------------------------------------------------------ */

struct _REUSEFILE {
    char       *pszLicense;
    char       *pszCopyright;
    char       *pszContributors;    /* '\n'-separated                */
    char       *pszPackageName;
    char       *pszPackageSupplier;
    char       *pszPackageDownloadLocation;
    char       *pszPackageComment;
    ULONG       ulPrecedence;
    BOOL        bHasReuse;
};

/* ------------------------------------------------------------------
 * Internal helpers (used by reuse.c and reuse_lic.c).
 * ------------------------------------------------------------------ */

/**
 * @brief Translate a public project handle into the internal pointer.
 *
 * @param[in] hDoc  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 */
PREUSEDOC ReuseInternalGetDoc(HREUSEDOC hDoc);

/**
 * @brief Translate a public file handle into the internal pointer.
 *
 * @param[in] hFile  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if the handle is NULLHANDLE.
 */
PREUSEFILE ReuseInternalGetFile(HREUSEFILE hFile);

#endif /* REUSE_INTERNAL_H */
