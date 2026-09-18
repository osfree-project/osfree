/* spdx_sbom_opts.h - разбор аргументов командной строки генератора SBOM */
#ifndef SPDX_SBOM_OPTS_H
#define SPDX_SBOM_OPTS_H

#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parsed command-line options for the SBOM generator.
 */
typedef struct {
    const char *dir;                    /* target directory (default ".") */
    const char *output;                 /* output file (NULL — stdout)     */
    const char *format;                 /* "spdx-json" or "spdx-tag"       */

    const char *default_license;        /* fallback license                */
    const char *default_copyright;      /* fallback copyright              */
    const char *doc_name;               /* document name (required)        */
    const char *package_version;        /* package version                 */
    const char *package_supplier;       /* package supplier                */
    const char *creator;                /* SBOM creator                    */
    const char *package_purpose;        /* SOURCE / BINARY / ...           */
    const char *binary_file;            /* path to the binary artifact     */

    char **object_files;                /* OMF object files                */
    int    object_count;

    char **res_files;                   /* RES resource files              */
    int    res_count;

    const char *source_sbom_path;       /* source SBOM for binary mode     */

    int no_gitignore;                   /* do not apply .gitignore rules   */

    /* SPDX database */
    const char *spdx_db_root;
    const char *cache_file;

    /* LicenseRef-* text sources */
    ExtractedLicenseSource *extracted_sources;
    int extracted_count;
} SbomOptions;

/**
 * @brief Parse the command line.
 *
 * Recognized options are listed by sbom_parse_args's --help output.
 * Missing required options, unknown options, and unsupported output
 * formats cause an error message on stderr and a non-zero return.
 *
 * @param[in]  argc  Argument count.
 * @param[in]  argv  Argument vector.
 * @param[out] opts  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
int sbom_parse_args(int argc, char *argv[], SbomOptions *opts);

/**
 * @brief Release memory allocated during argument parsing.
 *
 * Frees object_files, res_files and extracted_sources. Safe to call
 * on a partially filled structure; NULL fields are ignored.
 *
 * @param[in] opts  Parsed options. Not NULL.
 */
void sbom_options_free(SbomOptions *opts);

/**
 * @brief Look up a LicenseRef text source by identifier.
 *
 * @param[in] opts  Parsed options. Not NULL.
 * @param[in] id    LicenseRef identifier. Not NULL.
 *
 * @return Path to the text file, or NULL if no source was registered
 *         with that identifier.
 */
const char *sbom_options_find_extracted(const SbomOptions *opts,
                                        const char *id);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_OPTS_H */
