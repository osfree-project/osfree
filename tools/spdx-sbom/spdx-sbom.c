/* spdx-sbom.c - генератор SPDX SBOM (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reuse_parser.h>
#include "sha1_utils.h"
#include "spdx_db.h"
#include "spdx_utils.h"

#include "spdx_sbom_types.h"
#include "spdx_sbom_utils.h"
#include "spdx_sbom_opts.h"
#include "spdx_sbom_scan.h"
#include "spdx_lic.h"
#include "spdx_sbom_doc.h"
#include "spdx_sbom_extracted.h"
#include "spdx_sbom_out.h"

#include "spdx_discover.h"

static int is_binary_mode(const SbomOptions *opts) {
    return opts->package_purpose &&
           strcmp(opts->package_purpose, "SOURCE") != 0;
}

static int resolve_package_license(const SbomOptions *opts,
                                   ReuseConfig *config,
                                   const char **out_license) {
    const char *lic = opts->default_license;

    if (!lic) {
        lic = find_license_for_file(config, "**");
    }
    if (!lic || !lic[0]) {
        fprintf(stderr,
                "Error: no license for package. Use --default-license or "
                "REUSE.toml annotation with path=\"**\".\n");
        return -1;
    }
    {
        const char *bad = NULL;
        int rc = spdx_expression_validate(lic, &bad);
        if (rc == SPDX_EXPR_SYNTAX_ERROR) {
            fprintf(stderr,
                    "Error: invalid SPDX license expression for package: '%s'\n",
                    lic);
            return -1;
        }
        if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
            const char *p = bad;
            while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
            fprintf(stderr, "Error: unknown SPDX identifier in package "
                    "license: '");
            fwrite(bad, 1, (size_t)(p - bad), stderr);
            fprintf(stderr, "'\n");
            return -1;
        }
    }
    *out_license = lic;
    return 0;
}

static int resolve_binary_license(const SbomOptions *opts,
                                  ReuseConfig *config,
                                  const char **out_license) {
    const char *lic = opts->default_license;
    if (!lic && config) lic = find_license_for_file(config, "**");
    if (!lic || !lic[0]) {
        fprintf(stderr, "Error: no license for binary package\n");
        return -1;
    }
    *out_license = lic;
    return 0;
}

static int build_binary_file_list(const SbomOptions *opts,
                                  const char *license,
                                  FileList *out) {
    FileInfo info;

    if (sbom_fill_file_basic(opts->binary_file,
                             spdx_get_file_name(opts->binary_file),
                             &info) != 0)
        return -1;

    strncpy(info.file_type, "BINARY", sizeof(info.file_type) - 1);
    strncpy(info.license, license, sizeof(info.license) - 1);
    if (opts->default_copyright) {
        strncpy(info.copyright, opts->default_copyright,
                sizeof(info.copyright) - 1);
    }
    filelist_add(out, &info);
    return 0;
}

int main(int argc, char *argv[]) {
    SbomOptions opts;
    ReuseConfig *config = NULL;
    SpdxDocument doc;
    SpdxStrList paths;
    SpdxWalkOptions walk_opts;
    int db_errs;
    int binary_mode;
    const char *pkg_license = NULL;
    char toml_path[1024];
    char base_no_ext[256];

    if (sbom_parse_args(argc, argv, &opts) != 0)
        return 1;

    db_errs = spdx_db_init(opts.licenses_json, opts.exceptions_json,
                           opts.details_dir, opts.exceptions_dir,
                           opts.cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        fprintf(stderr, "Error: SPDX license database unavailable\n");
        sbom_options_free(&opts);
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        fprintf(stderr, "Error: SPDX exceptions database unavailable\n");
        sbom_options_free(&opts);
        spdx_db_free();
        return 1;
    }

#ifdef __LINUX__
    snprintf(toml_path, sizeof(toml_path), "%s/REUSE.toml", opts.dir);
#else
    snprintf(toml_path, sizeof(toml_path), "%s\\REUSE.toml", opts.dir);
#endif
    config = parse_reuse_toml(toml_path);

    binary_mode = is_binary_mode(&opts);

    if (resolve_package_license(&opts, config, &pkg_license) != 0) {
        free_reuse_config(config);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }

    sbom_doc_init(&doc,
                  opts.doc_name,
                  opts.package_version,
                  opts.package_supplier,
                  opts.creator,
                  pkg_license,
                  opts.default_copyright,
                  opts.package_purpose,
                  opts.binary_file,
                  binary_mode);

    filelist_init(&doc.files);

    if (binary_mode) {
        if (resolve_binary_license(&opts, config, &pkg_license) != 0) {
            sbom_doc_free(&doc);
            free_reuse_config(config);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
        if (build_binary_file_list(&opts, pkg_license, &doc.files) != 0) {
            sbom_doc_free(&doc);
            free_reuse_config(config);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
    } else {
        spdx_walk_options_default(&walk_opts);
        spdx_strlist_init(&paths);

        if (spdx_discover(opts.dir,
                          opts.object_files, opts.object_count,
                          opts.res_files, opts.res_count,
                          &walk_opts, &paths) != 0) {
            spdx_strlist_free(&paths);
            sbom_doc_free(&doc);
            free_reuse_config(config);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }

        if (sbom_collect_files(&paths, config,
                               opts.default_license,
                               opts.default_copyright,
                               &doc.files) != 0) {
            spdx_strlist_free(&paths);
            sbom_doc_free(&doc);
            free_reuse_config(config);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
        spdx_strlist_free(&paths);
    }

    if (extracted_collect_from_files(&doc.extracted_licenses,
                                     &doc.files, opts.dir,
                                     opts.extracted_sources,
                                     opts.extracted_count) != 0) {
        sbom_doc_free(&doc);
        free_reuse_config(config);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }

    sbom_doc_compute_verification(&doc);

    strncpy(base_no_ext, spdx_get_file_name(opts.binary_file),
            sizeof(base_no_ext) - 1);
    base_no_ext[sizeof(base_no_ext) - 1] = '\0';
    sbom_remove_extension(base_no_ext);

    sbom_doc_build_relationships(&doc, base_no_ext, binary_mode);

    if (binary_mode && opts.source_sbom_path) {
        char src_pkg_id[256];
        char *checksum;
        sbom_make_package_id(base_no_ext, "Source", src_pkg_id, sizeof(src_pkg_id));
        checksum = sha1_file(opts.source_sbom_path);
        if (!checksum) {
            fprintf(stderr, "Error: cannot compute SHA1 for %s\n",
                    opts.source_sbom_path);
            sbom_doc_free(&doc);
            free_reuse_config(config);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
        sbom_doc_set_external(&doc, opts.source_sbom_path, src_pkg_id, checksum);
        free(checksum);
    }

    if (opts.output) {
        if (!freopen(opts.output, "w", stdout)) {
            fprintf(stderr, "Cannot open output file: %s\n", opts.output);
            sbom_doc_free(&doc);
            free_reuse_config(config);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
    }

    if (sbom_output(&doc, opts.format) != 0) {
        sbom_doc_free(&doc);
        free_reuse_config(config);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }

    sbom_doc_free(&doc);
    free_reuse_config(config);
    spdx_db_free();
    sbom_options_free(&opts);
    return 0;
}
