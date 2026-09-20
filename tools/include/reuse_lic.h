/* reuse_lic.h - per-file license/copyright resolution (C89) */
#ifndef REUSE_LIC_H
#define REUSE_LIC_H

#include "os2types.h"
#include "os2err.h"
#include <reuse.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_lic.h
 * @brief Per-file license and copyright resolution.
 *
 * Combines the REUSE resolver, the sidecar file and the in-file
 * SPDX tags with optional CLI fallbacks, producing one
 * REUSELICENSEINFO structure per file.
 *
 * Sidecar and in-file tags are read directly by this module and
 * passed into the resolver as in-file sources, where they are
 * aggregated with the REUSE.toml annotations.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 */

/* ==================================================================
 * Types
 * ================================================================== */

/**
 * @enum _REUSELICENSESOURCE
 * @brief Origin of the resolved license or copyright.
 */
typedef enum _REUSELICENSESOURCE {
    REUSE_LICENSE_SRC_NONE = 0,  /**< No source matched.                */
    REUSE_LICENSE_SRC_TAG,       /**< In-file SPDX tag.                 */
    REUSE_LICENSE_SRC_SIDECAR,   /**< Adjacent .license sidecar.        */
    REUSE_LICENSE_SRC_REUSE,     /**< REUSE.toml or DEP5 annotation.    */
    REUSE_LICENSE_SRC_DEFAULT    /**< --default-license/copyright.      */
} REUSELICENSESOURCE;

/**
 * @struct _REUSELICENSEINFO
 * @brief Resolution result for a single file.
 *
 * Fixed-size string fields. If a value does not fit, it is
 * truncated and the corresponding @c f*Truncated flag is set to
 * TRUE. The module itself does not report truncation; the caller
 * decides whether to print a warning.
 */
typedef struct _REUSELICENSEINFO {
    /* Main fields: license and copyright.
     * Filled from REUSE.toml, sidecar, or in-file tags, in the order
     * of priority defined by REUSE 3.3 §4.1.3. */
    CHAR achLicense[256];
    CHAR achCopyright[512];

    /* SPDX-FileContributor: '\n'-separated, deduplicated.
     * Source: REUSE.toml only. REUSE 3.3 lists this key as one of
     * the "other keys" with unspecified semantics; the project
     * decision is to aggregate contributors from the closest and
     * aggregate annotations so that people are not lost. */
    CHAR achContributors[1024];

    /* SPDX-Package*: filled from REUSE.toml only. REUSE 3.3 lists
     * them as "other keys" (semantics not defined). Project
     * decision: closest > aggregate; override wins over everything. */
    CHAR achPackageName[256];
    CHAR achPackageSupplier[256];
    CHAR achPackageDownloadLocation[512];
    CHAR achPackageComment[512];
    BOOL fHasPackageInfo;        /**< TRUE if any package_* is set.    */

    REUSELICENSESOURCE source;
    BOOL fLicenseFromDefault;    /**< License came from CLI fallback.   */
    BOOL fCopyrightFromDefault;  /**< Copyright came from CLI fallback. */

    /* Truncation flags: TRUE if the corresponding string field was
       longer than its buffer and got cut. */
    BOOL fLicenseTruncated;
    BOOL fCopyrightTruncated;
    BOOL fContributorsTruncated;
    BOOL fPackageNameTruncated;
    BOOL fPackageSupplierTruncated;
    BOOL fPackageDownloadLocationTruncated;
    BOOL fPackageCommentTruncated;
} REUSELICENSEINFO, *PREUSELICENSEINFO;

/* ==================================================================
 * Resolution
 * ================================================================== */

/**
 * @brief Resolve license and copyright for one file.
 *
 * Sources of information, in order of priority (REUSE 3.3 §4.1.3):
 *   1. REUSE.toml / DEP5 annotations (with precedence
 *      override > aggregate > closest).
 *   2. Sidecar <file>.license.
 *   3. SPDX tags inside the file.
 *   4. CLI fallbacks (pszDefaultLicense, pszDefaultCopyright).
 *
 * @param[in]  hTree                Project handle. May be NULLHANDLE;
 *                                  in that case only sidecar, tags
 *                                  and defaults are used.
 * @param[in]  pszFullpath          File to resolve. Not NULL.
 * @param[in]  pszDefaultLicense    CLI fallback, or NULL.
 * @param[in]  pszDefaultCopyright  CLI fallback, or NULL.
 * @param[out] pOut                 Result receiver. Not NULL.
 *                                  Initialized on entry.
 *
 * @return APIRET
 * @retval NO_ERROR                 At least one source produced a
 *                                  non-empty license or copyright.
 * @retval ERROR_INVALID_PARAMETER  pszFullpath or pOut is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No source matched and no default
 *                                  was provided.
 */
APIRET APIENTRY ReuseResolveLicense(HREUSETREE hTree,
                                    PCSZ pszFullpath,
                                    PCSZ pszDefaultLicense,
                                    PCSZ pszDefaultCopyright,
                                    PREUSELICENSEINFO pOut);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_LIC_H */
