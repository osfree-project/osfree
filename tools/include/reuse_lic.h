/* reuse_lic.h - per-file license/copyright resolution (C89) */
#ifndef REUSE_LIC_H
#define REUSE_LIC_H

#include <reuse.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Types
 * ================================================================== */

/**
 * @enum REUSELICENSESOURCE
 * @brief Where the resolved license/copyright came from.
 */
typedef enum {
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
 * Fixed-size string fields: longer values are truncated with a
 * warning on stderr.
 */
typedef struct _REUSELICENSEINFO {
    /* Main fields: license and copyright.
     * Filled from REUSE.toml, sidecar, or in-file tags, in the order
     * of priority defined by REUSE 3.3 §4.1.3. */
    char license[256];
    char copyright[512];

    /* SPDX-FileContributor: '\n'-separated, deduplicated.
     * Source: REUSE.toml only. REUSE 3.3 (lines 177-182) lists this
     * key as one of the "other keys" with unspecified semantics; the
     * project decision is to aggregate contributors from the closest
     * and aggregate annotations so that people are not lost. */
    char contributors[1024];

    /* SPDX-Package*: filled from REUSE.toml only. REUSE 3.3 lists
     * them as "other keys" (semantics not defined). Project decision:
     * closest > aggregate; override wins over everything. */
    char package_name[256];
    char package_supplier[256];
    char package_download_location[512];
    char package_comment[512];
    int  has_package_info;       /**< 1 if any package_* field is non-empty. */

    REUSELICENSESOURCE source;
    int license_from_default;    /**< 1 if license came from CLI fallback. */
    int copyright_from_default;  /**< 1 if copyright came from CLI fallback. */
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
 * Sidecar and in-file tags are read directly by this module and
 * passed into the resolver as in-file sources, where they are
 * aggregated with the REUSE.toml annotations.
 *
 * @param[in]  hTree                 Project handle. May be NULLHANDLE;
 *                                   in that case only sidecar, tags
 *                                   and defaults are used.
 * @param[in]  pszFullpath           File to resolve. Not NULL.
 * @param[in]  pszDefaultLicense     CLI fallback, or NULL.
 * @param[in]  pszDefaultCopyright   CLI fallback, or NULL.
 * @param[out] pOut                  Result receiver. Not NULL.
 *                                   Initialized on entry.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             At least one source produced a
 *                                    non-empty license or copyright.
 * @retval REUSE_ERROR_INVALID_PARAM  pszFullpath or pOut is NULL.
 * @retval REUSE_ERROR_NOT_FOUND      No source matched and no default
 *                                    was provided.
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
