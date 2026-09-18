/* reuse-sbom.c - SBOM generator (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reuse.h>
#include "sha1.h"
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "git.h"

#include "spdx_sbom_types.h"
#include "spdx_sbom_utils.h"
#include "spdx_sbom_opts.h"
#include "spdx_sbom_scan.h"
#include "reuse_lic.h"
#include "spdx_sbom_doc.h"
#include "spdx_sbom_extracted.h"
#include "spdx_sbom_out.h"

#include "spdx_discover.h"

/**
 * @brief Check whether the SBOM describes a binary artifact.
 *
 * Binary mode is enabled when a package purpose other than SOURCE is
 * requested. SOURCE means we are generating the SBOM for the source
 * tree; any other value (BINARY, LIBRARY, ...) selects the binary
 * mode, where the SBOM is built from a single binary file rather than
 * from the tree.
 *
 * @param[in] opts  Parsed command-line options. Not NULL.
 *
 * @return 1 if binary mode, 0 if source mode.
 */
static int is_binary_mode(const SbomOptions *opts) {
    return opts->package_purpose &&
           strcmp(opts->package_purpose, "SOURCE") != 0;
}

/**
 * @brief Resolve the license for the package as a whole.
 *
 * Order of resolution:
 *   1. --default-license on the command line.
 *   2. The first [[annotations]] entry with path "**" found in any
 *      REUSE.toml, scanned from the deepest (closest to the target
 *      directory) to the root.
 *
 * The resolved expression is validated against the SPDX grammar.
 *
 * On success, @p *out_license receives a malloc'd string owned by the
 * caller. If the value came from --default-license, the pointer is
 * @p opts->default_license (do not free). The caller compares the two
 * to decide whether to free.
 *
 * @param[in]  opts         Parsed options. Not NULL.
 * @param[in]  hTree        Project handle. May be NULLHANDLE.
 * @param[out] out_license  Receiver. Not NULL.
 *
 * @return 0 on success, -1 on error (message already printed).
 */
static int resolve_package_license(const SbomOptions *opts,
                                   HREUSETREE hTree,
                                   const char **out_license) {
    const char *lic = opts->default_license;
    char *heap_lic = NULL;

    if (!lic && hTree != NULLHANDLE) {
        HREUSETREEFILE hFile = NULLHANDLE;
        APIRET rc = ReuseTreeResolveFile(hTree, "**", &hFile, NULL);
        if (rc == REUSE_NO_ERROR && hFile != NULLHANDLE) {
            ULONG ulSize = 0;
            if (ReuseTreeFileGetLicense(hFile, NULL, 0, &ulSize) == REUSE_NO_ERROR
                && ulSize > 0) {
                char *buf = (char*)malloc(ulSize);
                if (buf &&
                    ReuseTreeFileGetLicense(hFile, buf, ulSize, NULL)
                        == REUSE_NO_ERROR &&
                    buf[0] != '\0') {
                    heap_lic = buf;
                    lic = heap_lic;
                } else {
                    free(buf);
                }
            }
            ReuseTreeFileClose(hFile);
        }
    }
    if (!lic || !lic[0]) {
        fprintf(stderr,
                "ERROR: no license for package.\n"
                "       Fix one of:\n"
                "         - pass --default-license=<id>;\n"
                "         - or add a [[annotations]] entry with "
                "path = \"**\" to REUSE.toml.\n");
        free(heap_lic);
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
            free(heap_lic);
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
            free(heap_lic);
            return -1;
        }
    }
    *out_license = lic;
    return 0;
}

/**
 * @brief Build the FileList for the binary artifact.
 *
 * Creates one FileInfo from the binary file path: computes SHA1,
 * assigns file type BINARY, and copies the given license and
 * copyright.
 *
 * @param[in]  opts     Parsed options. Not NULL. opts->binary_file
 *                      must be set.
 * @param[in]  license  License string. Not NULL.
 * @param[out] out      FileList receiver. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
static int build_binary_file_list(const SbomOptions *opts,
                                  const char *license,
                                  FileList *out) {
    FileInfo info;

    if (sbom_fill_file_basic(opts->binary_file,
                             SpdxGetFileName(opts->binary_file),
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

/**
 * @brief Entry point of the SBOM generator.
 *
 * Collects licensing information from the project tree (or from a
 * single binary artifact), builds an in-memory SBOM document, and
 * emits it in the requested format.
 *
 * The output format is selected with --format:
 *   spdx-json  - SPDX 2.3 JSON
 *   spdx-tag   - SPDX 2.3 tag-value
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    SbomOptions opts;
    HREUSETREE hTree = NULLHANDLE;
    SpdxDocument doc;
    HSTRSET hPaths = NULLHANDLE;
    SpdxWalkOptions walk_opts;
    GITIGNORELIST gitignore_rules;
    int has_gitignore = 0;
    char *repo_root = NULL;
    int db_errs;
    int binary_mode;
    const char *pkg_license = NULL;
    int pkg_license_is_heap = 0;
    char base_no_ext[256];

    GitIgnoreListInit(&gitignore_rules);

    if (sbom_parse_args(argc, argv, &opts) != 0) {
        GitIgnoreListFree(&gitignore_rules);
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
        GitIgnoreListFree(&gitignore_rules);
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
        GitIgnoreListFree(&gitignore_rules);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written.\n"
                "         Next run will re-parse JSON indexes.\n");

    if (GitFindRepoRoot(opts.dir, &repo_root) != GIT_NO_ERROR) {
        repo_root = NULL;
    }

    {
        APIRET rc = ReuseTreeOpen(opts.dir, &hTree);
        if (rc != REUSE_NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot open REUSE project at %s\n"
                    "       The directory is missing or unreadable.\n",
                    opts.dir);
            GitIgnoreListFree(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
        {
            ULONG ulErrs = 0;
            if (ReuseTreeGetErrorCount(hTree, &ulErrs) == REUSE_NO_ERROR &&
                ulErrs > 0) {
                fprintf(stderr,
                        "ERROR: %u REUSE.toml file(s) could not be parsed.\n"
                        "       SBOM cannot be generated reliably.\n",
                        (unsigned)ulErrs);
                ReuseTreeClose(hTree);
                GitIgnoreListFree(&gitignore_rules);
                free(repo_root);
                spdx_db_free();
                sbom_options_free(&opts);
                return 1;
            }
        }
    }

    if (!opts.no_gitignore) {
        if (GitCollectGitignores(repo_root, opts.dir, &gitignore_rules) == GIT_NO_ERROR &&
            gitignore_rules.ulCount > 0) {
            has_gitignore = 1;
        }
    }

    binary_mode = is_binary_mode(&opts);

    /* Resolve the package license BEFORE sbom_doc_init so the value is
     * fixed inside the package structure. In binary mode the same
     * resolution path is used: REUSE.toml (path="**") >
     * --default-license. */
    if (resolve_package_license(&opts, hTree, &pkg_license) != 0) {
        ReuseTreeClose(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }
    if (pkg_license != opts.default_license) pkg_license_is_heap = 1;

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
            if (pkg_license_is_heap) free((void*)pkg_license);
            ReuseTreeClose(hTree);
            GitIgnoreListFree(&gitignore_rules);
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
        if (StrSetCreate(&hPaths) != NO_ERROR) {
            sbom_doc_free(&doc);
            if (pkg_license_is_heap) free((void*)pkg_license);
            ReuseTreeClose(hTree);
            GitIgnoreListFree(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }

        if (spdx_discover(opts.dir,
                          opts.object_files, opts.object_count,
                          opts.res_files, opts.res_count,
                          &walk_opts, hPaths) != 0) {
            StrSetDestroy(hPaths);
            sbom_doc_free(&doc);
            if (pkg_license_is_heap) free((void*)pkg_license);
            ReuseTreeClose(hTree);
            GitIgnoreListFree(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }

        if (sbom_collect_files(hPaths, hTree,
                               opts.default_license,
                               opts.default_copyright,
                               &doc.files,
                               &doc.snippets) != 0) {
            StrSetDestroy(hPaths);
            sbom_doc_free(&doc);
            if (pkg_license_is_heap) free((void*)pkg_license);
            ReuseTreeClose(hTree);
            GitIgnoreListFree(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
        StrSetDestroy(hPaths);
    }

    if (extracted_collect_from_files(&doc.extracted_licenses,
                                     &doc.files, opts.dir,
                                     opts.extracted_sources,
                                     opts.extracted_count) != 0) {
        sbom_doc_free(&doc);
        if (pkg_license_is_heap) free((void*)pkg_license);
        ReuseTreeClose(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }

    sbom_doc_compute_verification(&doc);

    strncpy(base_no_ext, SpdxGetFileName(opts.binary_file),
            sizeof(base_no_ext) - 1);
    base_no_ext[sizeof(base_no_ext) - 1] = '\0';
    sbom_remove_extension(base_no_ext);

    sbom_doc_build_relationships(&doc, base_no_ext, binary_mode);

    if (binary_mode && opts.source_sbom_path) {
        char src_pkg_id[256];
        char *checksum = NULL;
        sbom_make_package_id(base_no_ext, "Source",
                             src_pkg_id, sizeof(src_pkg_id));
        if (Sha1File(opts.source_sbom_path, &checksum) != SHA1_NO_ERROR) {
            fprintf(stderr,
                    "ERROR: cannot compute SHA1 for source SBOM: %s\n"
                    "       Check that the file exists and is readable.\n",
                    opts.source_sbom_path);
            sbom_doc_free(&doc);
            if (pkg_license_is_heap) free((void*)pkg_license);
            ReuseTreeClose(hTree);
            GitIgnoreListFree(&gitignore_rules);
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
            if (pkg_license_is_heap) free((void*)pkg_license);
            ReuseTreeClose(hTree);
            GitIgnoreListFree(&gitignore_rules);
            free(repo_root);
            spdx_db_free();
            sbom_options_free(&opts);
            return 1;
        }
    }

    if (sbom_output(&doc, opts.format) != 0) {
        sbom_doc_free(&doc);
        if (pkg_license_is_heap) free((void*)pkg_license);
        ReuseTreeClose(hTree);
        GitIgnoreListFree(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        sbom_options_free(&opts);
        return 1;
    }

    sbom_doc_free(&doc);
    if (pkg_license_is_heap) free((void*)pkg_license);
    ReuseTreeClose(hTree);
    GitIgnoreListFree(&gitignore_rules);
    free(repo_root);
    spdx_db_free();
    sbom_options_free(&opts);
    return 0;
}
