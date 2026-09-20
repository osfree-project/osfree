/* spdx_sbom_opts.h - SBOM command line parsing (C89) */
#ifndef SPDX_SBOM_OPTS_H
#define SPDX_SBOM_OPTS_H

#include "os2types.h"
#include "os2err.h"
#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_opts.h
 * @brief Command line parsing for the SBOM generator.
 *
 * Parses the argv vector into an SBOMOPTIONS structure. No help
 * text is printed by this module: when --help or -h is seen, the
 * @c fHelpRequested flag is set and the function returns NO_ERROR.
 * The caller prints the help text and exits.
 *
 * When an option is rejected or a required option is missing, the
 * @c pszBadOption field points to the offending argument inside the
 * caller's argv (or to a static option name for a missing required
 * option). The caller formats the diagnostic message.
 *
 * Conforms to:
 *   - SPDX 2.3.
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

/**
 * @struct _SBOMOPTIONS
 * @brief Parsed command line options.
 *
 * String fields that come from argv point into argv; they must not
 * be freed by the caller. The object, resource and extracted
 * license lists are allocated by the parser and released by
 * SbomFreeOptions.
 */
typedef struct _SBOMOPTIONS {
    PCSZ  pszDir;                    /**< Target directory.        */
    PCSZ  pszOutput;                 /**< Output file, or NULL.    */
    PCSZ  pszFormat;                 /**< "spdx-json" or "spdx-tag".*/

    PCSZ  pszDefaultLicense;         /**< Fallback license, or NULL.*/
    PCSZ  pszDefaultCopyright;       /**< Fallback copyright, or NULL.*/
    PCSZ  pszDocName;                /**< Document name.           */
    PCSZ  pszPackageVersion;         /**< Package version, or NULL.*/
    PCSZ  pszPackageSupplier;        /**< Supplier, or NULL.       */
    PCSZ  pszCreator;                /**< Creator, or NULL.        */
    PCSZ  pszPackagePurpose;         /**< Primary purpose, or NULL.*/
    PCSZ  pszBinaryFile;             /**< Binary artifact path.    */

    PSZ  *papszObjectFiles;          /**< OMF object files.        */
    ULONG ulObjectCount;             /**< Number of entries.       */

    PSZ  *papszResFiles;             /**< RES resource files.      */
    ULONG ulResCount;                /**< Number of entries.       */

    PCSZ  pszSourceSbomPath;         /**< Source SBOM, or NULL.    */

    BOOL  fNoGitignore;              /**< Do not apply .gitignore. */

    PCSZ  pszSpdxDbRoot;             /**< SPDX database root.      */
    PCSZ  pszCacheFile;              /**< Cache file, or NULL.     */

    PSPDXEXTRACTEDLICENSESOURCE paExtractedSources;
                                     /**< LicenseRef sources.      */
    ULONG ulExtractedCount;          /**< Number of sources.       */

    BOOL  fHelpRequested;            /**< --help or -h was seen.   */
    PCSZ  pszBadOption;              /**< Offending argv element,
                                          or static option name.
                                          NULL if no error.        */
} SBOMOPTIONS, *PSBOMOPTIONS;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Parse the command line.
 *
 * Zeroes @p pOpts and fills it from @p argv. Sets defaults:
 * @c pszDir to ".", @c pszFormat to "spdx-json". On --help or -h,
 * sets @c fHelpRequested and returns NO_ERROR without further
 * parsing. On error, sets @c pszBadOption and returns
 * ERROR_INVALID_PARAMETER.
 *
 * @param[in]  argc   Argument count.
 * @param[in]  argv   Argument vector. Not NULL.
 * @param[out] pOpts  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success, or help was requested.
 * @retval ERROR_INVALID_PARAMETER  pOpts is NULL, an option is
 *                                  rejected, or a required option
 *                                  is missing.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomParseCommandLine(int argc, char *argv[],
                                     PSBOMOPTIONS pOpts);

/**
 * @brief Release memory owned by an options structure.
 *
 * Frees the object list, the resource list and the extracted
 * license source array. Safe to call on a zeroed structure.
 *
 * @param[in,out] pOpts  Options. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pOpts is NULL.
 */
APIRET APIENTRY SbomFreeOptions(PSBOMOPTIONS pOpts);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_OPTS_H */
