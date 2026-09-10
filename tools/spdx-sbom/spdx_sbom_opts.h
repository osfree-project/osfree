/* spdx_sbom_opts.h - разбор аргументов командной строки spdx-sbom */
#ifndef SPDX_SBOM_OPTS_H
#define SPDX_SBOM_OPTS_H

#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *dir;
    const char *output;
    const char *format;

    const char *default_license;
    const char *default_copyright;
    const char *doc_name;
    const char *package_version;
    const char *package_supplier;
    const char *creator;
    const char *package_purpose;
    const char *binary_file;

    char **exclude_list;
    int    exclude_count;

    char **object_files;
    int    object_count;

    char **res_files;
    int    res_count;

    const char *source_sbom_path;

    /* ѕути к базе SPDX */
    const char *licenses_json;
    const char *exceptions_json;
    const char *details_dir;
    const char *exceptions_dir;
    const char *cache_file;

    /* явно заданные тексты LicenseRef-* */
    ExtractedLicenseSource *extracted_sources;
    int extracted_count;
} SbomOptions;

int sbom_parse_args(int argc, char *argv[], SbomOptions *opts);
void sbom_options_free(SbomOptions *opts);

const char *sbom_options_find_extracted(const SbomOptions *opts,
                                        const char *id);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_OPTS_H */
