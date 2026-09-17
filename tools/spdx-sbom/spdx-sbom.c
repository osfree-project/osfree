/* spdx-sbom.c - генератор SPDX SBOM (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reuse.h>
#include "sha1_utils.h"
#include "spdx_db.h"
#include "spdx_utils.h"
#include "git_utils.h"
#include "dep5.h"

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
                                   ReuseConfig **configs, int config_count,
                                   const char **out_license) {
    const char *lic = opts->default_license;
    int i;

    if (!lic) {
        for (i = config_count - 1; i >= 0; i--) {
            lic = find_license_for_file(configs[i], "**");
            if (lic && lic[0]) break;
            lic = NULL;
        }
    }
    if (!lic || !lic[0]) {
        fprintf(stderr,
                "ERROR: no license for package.\n"
                "       Fix one of:\n"
                "         - pass --default-license=<id>;\n"
                "         - or add a [[annotations]] entry with "
                "path = \"**\" to REUSE.toml.\n");
        return -1;
    }
    {
        const char *bad = NULL;
        int rc = spdx_expression_validate(lic, &bad);
        if (rc == SPDX_EXPR_SYNTAX_ERROR) {
            fprintf(stderr,
                    "ERROR: invalid SPDX license expression for package: "
                    "'%s'\n"
                    "       Fix the expression according to the SPDX "
                    "grammar:\n"
                    "         https://spdx.github.io/spdx-spec/v2.3/"
                    "SPDX-license-expressions/\n",
                    lic);
            return -1;
        }
        if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
            const char *p = bad;
            while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
            fprintf(stderr,
                    "ERROR: unknown SPDX identifier in package license: '");
            fwrite(bad, 1, (size_t)(p - bad), stderr);
            fprintf(stderr,
                    "'\n"
                    "       Not present in SPDX License List. Fix one of:\n"
                    "         - correct the identifier;\n"
                    "         - or use a 'LicenseRef-' identifier for a "
                    "custom license.\n"
                    "       See https://spdx.org/licenses/ for the full "
                    "list.\n");
            return -1;
        }
    }
    *out_license = lic;
    return 0;
}

static int resolve_binary_license(const SbomOptions *opts,
                                  ReuseConfig **configs, int config_count,
                                  const char **out_license) {
    const char *lic = opts->default_license;
    int i;

    if (!lic) {
        for (i = config_count - 1; i >= 0; i--) {
            lic = find_license_for_file(configs[i], "**");
            if (lic && lic[0]) break;
            lic = NULL;
        }
    }
    if (!lic || !lic[0]) {
        fprintf(stderr,
                "ERROR: no license for binary package.\n"
                "       Fix one of:\n"
                "         - pass --default-license=<id>;\n"
                "         - or add a [[annotations]] entry with "
                "path = \"**\" to REUSE.toml.\n");
        return -1;
    }
    {
        const char *bad = NULL;
        int rc = spdx_expression_validate(lic, &bad);
        if (rc == SPDX_EXPR_SYNTAX_ERROR) {
            fprintf(stderr,
                    "ERROR: invalid SPDX license expression for binary "
                    "package: '%s'\n"
                    "       Fix the expression according to the SPDX "
                    "grammar:\n"
                    "         https://spdx.github.io/spdx-spec/v2.3/"
                    "SPDX-license-expressions/\n",
                    lic);
            return -1;
        }
        if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
            const char *p = bad;
            while (*p && *p != ' ' && *p != '(' && *p != ')') p++;
            fprintf(stderr,
                    "ERROR: unknown SPDX identifier in binary package "
                    "license: '");
            fwrite(bad, 1, (size_t)(p - bad), stderr);
            fprintf(stderr,
                    "'\n"
                    "       See https://spdx.org/licenses/ for the full "
                    "list.\n");
            return -1;
        }
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
    ReuseConfig **configs = NULL;
    int config_count = 0;
    SpdxStrList toml_paths;
    SpdxDocument doc;
    SpdxStrList paths;
    SpdxWalkOptions walk_opts;
    GitIgnoreList gitignore_rules;
    int has_gitignore = 0;
    char *repo_root = NULL;
    int db_errs;
    int binary_mode;
    const char *pkg_license = NULL;
    char base_no_ext[256];

    git_ignore_list_init(&gitignore_rules);

    if (sbom_parse_args(argc, argv, &opts) != 0) {
        git_ignore_list_free(&gitignore_rules);
        return 1;
    }

    db_errs = spdx_db_init(opts.spdx_db_root, opts.cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        fprintf(stderr,
                "ERROR: SPDX license database is unavailable "
                "(licenses.json not loaded).\n"
                "       Expected at <spdx-db>/licenses.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        sbom_options_free(&opts);
        git_ignore_list_free(&gitignore_rules);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: SPDX exceptions database is unavailable "
                "(exceptions.json not loaded).\n"
                "       Expected at <spdx-db>/exceptions.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        sbom_options_free(&opts);
        git_ignore_list_free(&gitignore_rules);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written.\n"
                "         Next run will re-parse JSON indexes.\n");

    repo_root = git_find_repo_root(opts.dir);

    {
        int reuse_errors = 0;
        reuse_find_all_tomls(repo_root, opts.dir, &toml_paths);
        configs = reuse_parse_all(&toml_paths, &config_count, &reuse_errors);
        spdx_strlist_free(&toml_paths);
        if (reuse_errors > 0) {
            fprintf(stderr,
                    "ERROR: %d REUSE.toml file(s) could not be parsed.\n"
                    "       SBOM cannot be generated reliably.\n",
                    reuse_errors);
            reuse_free_all(configs, config_count);
            git_ignore_list_free(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
    }

    if (repo_root) {
        ReuseConfig *dep5 = reuse_load_dep5(repo_root);
        if (dep5) {
            ReuseConfig **na = (ReuseConfig**)realloc(configs,
                (size_t)(config_count + 1) * sizeof(ReuseConfig*));
            if (na) {
                configs = na;
                configs[config_count++] = dep5;
            } else {
                free_reuse_config(dep5);
            }
        }
    }

    if (!opts.no_gitignore) {
        if (git_collect_gitignores(repo_root, opts.dir, &gitignore_rules) == 0 &&
            gitignore_rules.count > 0) {
            has_gitignore = 1;
        }
    }

    binary_mode = is_binary_mode(&opts);

    /* Разрешаем лицензию ДО sbom_doc_init, чтобы значение было
     * зафиксировано в пакете. В binary-режиме — та же логика, что и
     * для source: REUSE.toml (path="**") > --default-license. */
    if (resolve_package_license(&opts, configs, config_count,
                                &pkg_license) != 0) {
        reuse_free_all(configs, config_count);
        git_ignore_list_free(&gitignore_rules);
        free(repo_root);
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
    snippetlist_init(&doc.snippets);

    if (binary_mode) {
        if (build_binary_file_list(&opts, pkg_license, &doc.files) != 0) {
            sbom_doc_free(&doc);
            reuse_free_all(configs, config_count);
            git_ignore_list_free(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
    } else {
        spdx_walk_options_default(&walk_opts);
        if (has_gitignore) {
            walk_opts.use_gitignore   = 1;
            walk_opts.repo_root       = repo_root ? repo_root : opts.dir;
            walk_opts.gitignore_rules = &gitignore_rules;
        }
        spdx_strlist_init(&paths);

        if (spdx_discover(opts.dir,
                          opts.object_files, opts.object_count,
                          opts.res_files, opts.res_count,
                          &walk_opts, &paths) != 0) {
            spdx_strlist_free(&paths);
            sbom_doc_free(&doc);
            reuse_free_all(configs, config_count);
            git_ignore_list_free(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }

        if (sbom_collect_files(&paths, configs, config_count,
                               opts.default_license,
                               opts.default_copyright,
                               &doc.files,
                               &doc.snippets) != 0) {
            spdx_strlist_free(&paths);
            sbom_doc_free(&doc);
            reuse_free_all(configs, config_count);
            git_ignore_list_free(&gitignore_rules);
            free(repo_root);
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
        reuse_free_all(configs, config_count);
        git_ignore_list_free(&gitignore_rules);
        free(repo_root);
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
        sbom_make_package_id(base_no_ext, "Source",
                             src_pkg_id, sizeof(src_pkg_id));
        checksum = sha1_file(opts.source_sbom_path);
        if (!checksum) {
            fprintf(stderr,
                    "ERROR: cannot compute SHA1 for source SBOM: %s\n"
                    "       Check that the file exists and is readable.\n",
                    opts.source_sbom_path);
            sbom_doc_free(&doc);
            reuse_free_all(configs, config_count);
            git_ignore_list_free(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
        sbom_doc_set_external(&doc, opts.source_sbom_path,
                              src_pkg_id, checksum);
        free(checksum);
    }

    if (opts.output) {
        if (!freopen(opts.output, "w", stdout)) {
            fprintf(stderr,
                    "ERROR: cannot open output file: %s\n"
                    "       Check directory permissions.\n",
                    opts.output);
            sbom_doc_free(&doc);
            reuse_free_all(configs, config_count);
            git_ignore_list_free(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
    }

    if (sbom_output(&doc, opts.format) != 0) {
        sbom_doc_free(&doc);
        reuse_free_all(configs, config_count);
        git_ignore_list_free(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }

    sbom_doc_free(&doc);
    reuse_free_all(configs, config_count);
    git_ignore_list_free(&gitignore_rules);
    free(repo_root);
    spdx_db_free();
    sbom_options_free(&opts);
    return 0;
}
