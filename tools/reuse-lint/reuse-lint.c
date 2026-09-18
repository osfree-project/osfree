/* reuse-lint.c - REUSE / SPDX compliance checker (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#ifdef __LINUX__
#include <unistd.h>
#else
#include <io.h>
#endif

#include <reuse.h>
#include "spdx_db.h"
#include "spdx_discover.h"
#include "spdx_utils.h"
#include "spdx_lic.h"
#include "spdx_tag.h"
#include "git.h"

static int error_count = 0;
static int warning_count = 0;

static const char *default_license = NULL;
static const char *default_copyright = NULL;

static int total_files = 0;
static int files_with_license = 0;
static int files_with_copyright = 0;
static int read_errors = 0;

static int total_snippets = 0;
static int snippets_with_license = 0;

static int has_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    if (!dot) return 0;
    return dot != name;
}

static int is_valid_spdx_name(const char *name) {
    if (spdx_license_is_valid(name)) return 1;
    if (spdx_exception_is_valid(name)) return 1;
    return 0;
}

static void print_bad_token(const char *start) {
    const char *p = start;
    if (!p) { fprintf(stderr, "(null)"); return; }
    while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')') p++;
    fwrite(start, 1, (size_t)(p - start), stderr);
}

static void check_license_expression(const char *fullpath, const char *license,
                                     SpdxStrList *used_ids) {
    const char *bad = NULL;
    int rc = spdx_expression_validate(license, &bad);

    if (rc == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "ERROR: %s: invalid SPDX license expression: '%s'\n"
                "       Fix the expression according to the SPDX grammar:\n"
                "         https://spdx.github.io/spdx-spec/v2.3/"
                "SPDX-license-expressions/\n",
                fullpath, license);
        error_count++;
    } else if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
        fprintf(stderr, "ERROR: %s: unknown SPDX identifier: '", fullpath);
        print_bad_token(bad);
        fprintf(stderr,
                "'\n"
                "       Not present in SPDX License List. Fix one of:\n"
                "         - correct the identifier;\n"
                "         - if it is a custom license, prefix it with "
                "'LicenseRef-' and add the text to LICENSES/;\n"
                "         - or add a [[annotations]] entry in REUSE.toml.\n"
                "       See https://spdx.org/licenses/ for the full list.\n");
        error_count++;
    }

    {
        SpdxStrList ids;
        int m;
        spdx_strlist_init(&ids);
        spdx_expression_collect_ids(license, &ids);
        for (m = 0; m < ids.count; m++) {
            if (rc == SPDX_EXPR_OK) {
                spdx_strlist_add_unique(used_ids, ids.items[m]);
            }
            if (spdx_license_is_deprecated(ids.items[m]) ||
                spdx_exception_is_deprecated(ids.items[m])) {
                fprintf(stderr,
                        "WARNING: %s: deprecated SPDX identifier '%s'.\n"
                        "         The SPDX License List marks this identifier "
                        "deprecated.\n"
                        "         Replace it with the current identifier "
                        "(usually a '-only' or '-or-later' variant).\n"
                        "         See https://spdx.org/licenses/ for the "
                        "recommended replacement.\n",
                        fullpath, ids.items[m]);
                warning_count++;
            }
        }
        spdx_strlist_free(&ids);
    }
}

static void process_snippets(const char *fullpath, SpdxStrList *used_licenses) {
    TagSnippetList snippets;
    int i;

    if (file_get_snippets(fullpath, &snippets) != 0) {
        error_count++;
        return;
    }

    for (i = 0; i < snippets.count; i++) {
        TagSnippet *s = &snippets.items[i];

        total_snippets++;

        if (!s->license || s->license[0] == '\0') {
            fprintf(stderr,
                    "ERROR: %s:%d-%d: snippet has no "
                    "SPDX-License-Identifier.\n"
                    "       Fix one of:\n"
                    "         - add 'SPDX-License-Identifier: <id>' inside "
                    "the snippet block;\n"
                    "         - or remove SPDX-SnippetBegin/SPDX-SnippetEnd "
                    "if the code is not a snippet.\n",
                    fullpath, s->line_start, s->line_end);
            error_count++;
            continue;
        }

        snippets_with_license++;

        {
            char label[1200];
            snprintf(label, sizeof(label), "%s:%d-%d",
                     fullpath, s->line_start, s->line_end);
            check_license_expression(label, s->license, used_licenses);
        }

        if (!s->copyright || s->copyright[0] == '\0') {
            fprintf(stderr,
                    "WARNING: %s:%d-%d: snippet has no "
                    "SPDX-SnippetCopyrightText.\n"
                    "         REUSE recommends adding a copyright notice\n"
                    "         inside the snippet.\n",
                    fullpath, s->line_start, s->line_end);
            warning_count++;
        }
    }

    tag_snippets_free(&snippets);
}

static void process_file(const char *fullpath,
                         ReuseConfig **configs, int config_count,
                         SpdxStrList *used_licenses) {
    FileLicenseInfo lic;
    FILE *f;
    const char *wcc_cmd;
#ifdef __LINUX__
    wcc_cmd = "_wcc.sh";
#else
    wcc_cmd = "_wcc.cmd";
#endif

    total_files++;

    f = fopen(fullpath, "rb");
    if (!f) {
        fprintf(stderr,
                "ERROR: cannot read file: %s\n"
                "       Fix file permissions or remove it from the project.\n",
                fullpath);
        error_count++;
        read_errors++;
        return;
    }
    fclose(f);

    if (spdx_resolve_license(configs, config_count, fullpath,
                             default_license, default_copyright,
                             &lic) != 0) {
        fprintf(stderr,
                "ERROR: %s has no licensing information at all.\n"
                "       REUSE requires each file to carry both license and\n"
                "       copyright information.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier' and "
                "'SPDX-FileCopyrightText' tags in the file header;\n"
                "         - or create a sidecar '<file>.license' next to it;\n"
                "         - or add a [[annotations]] entry in REUSE.toml;\n"
                "         - or run '%s annotate' to write them.\n"
                "       See https://reuse.software/spec/ for details.\n",
                fullpath, wcc_cmd);
        error_count++;
        return;
    }

    if (lic.license_from_default) {
        fprintf(stderr,
                "WARNING: %s: license is taken from --default-license.\n"
                "         REUSE does not allow a global CLI fallback.\n"
                "         Run '%s annotate' to write the license\n"
                "         into the file, or add a sidecar/REUSE.toml entry.\n",
                fullpath, wcc_cmd);
        warning_count++;
    }
    if (lic.copyright_from_default) {
        fprintf(stderr,
                "WARNING: %s: copyright is taken from --default-copyright.\n"
                "         REUSE does not allow a global CLI fallback.\n"
                "         Run '%s annotate' to write the copyright\n"
                "         into the file, or add a sidecar/REUSE.toml entry.\n",
                fullpath, wcc_cmd);
        warning_count++;
    }

    if (lic.license[0] != '\0') {
        files_with_license++;
    } else {
        fprintf(stderr,
                "ERROR: %s has copyright information but no license "
                "information.\n"
                "       REUSE requires both.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier: <id>' in the file "
                "header;\n"
                "         - or create '<file>.license' with the same tag;\n"
                "         - or add a [[annotations]] entry in REUSE.toml;\n"
                "         - or run '%s annotate'.\n",
                fullpath, wcc_cmd);
        error_count++;
    }
    if (lic.copyright[0] != '\0') {
        files_with_copyright++;
    } else {
        fprintf(stderr,
                "ERROR: %s has license information but no copyright "
                "information.\n"
                "       REUSE requires both.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-FileCopyrightText: <holder>' in the "
                "file header;\n"
                "         - or create '<file>.license' with the same tag;\n"
                "         - or add a [[annotations]] entry in REUSE.toml;\n"
                "         - or run '%s annotate'.\n",
                fullpath, wcc_cmd);
        error_count++;
    }

    if (lic.license[0] != '\0') {
        check_license_expression(fullpath, lic.license, used_licenses);
    }

    process_snippets(fullpath, used_licenses);
}

static void strip_license_ext(const char *fname, char *base, size_t base_size) {
    char *dot;
    size_t len;

    if (is_valid_spdx_name(fname)) {
        strncpy(base, fname, base_size - 1);
        base[base_size - 1] = '\0';
        return;
    }
    strncpy(base, fname, base_size - 1);
    base[base_size - 1] = '\0';
    dot = strrchr(base, '.');
    if (dot) *dot = '\0';

    len = strlen(base);
    if (len == 0) {
        strncpy(base, fname, base_size - 1);
        base[base_size - 1] = '\0';
    }
}

static void check_licenses_dir(const char *project_dir,
                               SpdxStrList *used_licenses) {
    char lic_path[1024];
    char example_path[1100];
    SpdxWalkOptions opts;
    SpdxStrList paths;
    SpdxStrList files_in_lic;
    int i;
    int dir_exists;
    const char *wcc_cmd;
#ifdef __LINUX__
    wcc_cmd = "_wcc.sh";
#else
    wcc_cmd = "_wcc.cmd";
#endif

#ifdef __LINUX__
    snprintf(lic_path, sizeof(lic_path), "%s/LICENSES", project_dir);
    dir_exists = (access(lic_path, F_OK) == 0);
#else
    snprintf(lic_path, sizeof(lic_path), "%s\\LICENSES", project_dir);
    dir_exists = (_access(lic_path, 0) == 0);
#endif

    spdx_strlist_init(&files_in_lic);

    if (dir_exists) {
        memset(&opts, 0, sizeof(opts));
        opts.recursive             = 0;
        opts.skip_hidden           = 1;
        opts.skip_vcs_dirs         = 0;
        opts.skip_licenses_dir     = 0;
        opts.skip_reuse_dir        = 0;
        opts.skip_license_sidecars = 0;
        opts.skip_reuse_toml       = 0;
        opts.skip_license_files    = 0;

        spdx_strlist_init(&paths);
        if (spdx_walk_tree(lic_path, &opts, &paths) == 0 && paths.count > 0) {
            for (i = 0; i < paths.count; i++) {
                const char *fname = spdx_get_file_name(paths.items[i]);
                spdx_strlist_add(&files_in_lic, fname);
            }
        }
        spdx_strlist_free(&paths);
    } else {
#ifdef __LINUX__
        snprintf(example_path, sizeof(example_path),
                 "%s/<SPDX-id>.txt", lic_path);
#else
        snprintf(example_path, sizeof(example_path),
                 "%s\\<SPDX-id>.txt", lic_path);
#endif
        fprintf(stderr,
                "ERROR: %s is missing.\n"
                "       REUSE requires every license text to be placed in\n"
                "       %s (REUSE Specification 3.3).\n"
                "       Fix one of:\n"
                "         - create the directory and add a text file for each\n"
                "           license declared by any file in the project;\n"
                "         - or run '%s annotate' to create it "
                "automatically.\n"
                "       See https://reuse.software/spec/ for details.\n",
                lic_path, example_path, wcc_cmd);
        error_count++;
    }

    for (i = 0; i < files_in_lic.count; i++) {
        const char *fname = files_in_lic.items[i];
        char base[256];

        strip_license_ext(fname, base, sizeof(base));

        if (!is_valid_spdx_name(base)) {
            fprintf(stderr,
                    "ERROR: bad license file name: %s\n"
                    "       The name must be a valid SPDX License List\n"
                    "       identifier or start with 'LicenseRef-'.\n"
                    "       Fix one of:\n"
                    "         - rename the file to a valid SPDX identifier\n"
                    "           (e.g. 'MIT.txt');\n"
                    "         - or use 'LicenseRef-<name>.txt' for a custom\n"
                    "           license.\n"
                    "       See https://spdx.org/licenses/ for the full "
                    "list.\n",
                    fname);
            error_count++;
        } else {
            if (!has_extension(fname)) {
                fprintf(stderr,
                        "WARNING: license file without extension: %s\n"
                        "         REUSE convention is to use '.txt'.\n"
                        "         Rename to '%s.txt' to follow the "
                        "convention and avoid ambiguity.\n",
                        fname, fname);
                warning_count++;
            }
            if (spdx_license_is_deprecated(base) ||
                spdx_exception_is_deprecated(base)) {
                fprintf(stderr,
                        "WARNING: deprecated license file: %s\n"
                        "         SPDX License List marks '%s' as "
                        "deprecated.\n"
                        "         Rename the file to the current identifier "
                        "and update\n"
                        "         all references in source files.\n"
                        "         See https://spdx.org/licenses/ for the "
                        "recommended replacement.\n",
                        fname, base);
                warning_count++;
            }
        }
    }

    for (i = 0; i < files_in_lic.count; i++) {
        char base[256];

        strip_license_ext(files_in_lic.items[i], base, sizeof(base));

        if (!spdx_strlist_contains(used_licenses, base)) {
            fprintf(stderr,
                    "ERROR: unused license file: %s\n"
                    "       REUSE Specification 3.3 forbids License Files\n"
                    "       for licenses under which none of the files in\n"
                    "       the project are licensed.\n"
                    "       Fix one of:\n"
                    "         - remove the file if it is no longer needed;\n"
                    "         - or add 'SPDX-License-Identifier: %s' to the "
                    "files it applies to;\n"
                    "         - or add a [[annotations]] entry in "
                    "REUSE.toml referencing this license.\n",
                    files_in_lic.items[i], base);
            error_count++;
        }
    }

    for (i = 0; i < used_licenses->count; i++) {
        const char *lic = used_licenses->items[i];
        char with_txt[512];
        char expected[1100];
        char full_path[1200];
        const char *actual_fname = NULL;
        int is_ref;
        int found;

        is_ref = (strncmp(lic, "LicenseRef-", 11) == 0) ||
                 (strncmp(lic, "DocumentRef-", 12) == 0);

        snprintf(with_txt, sizeof(with_txt), "%s.txt", lic);
        found = spdx_strlist_contains(&files_in_lic, lic) ||
                spdx_strlist_contains(&files_in_lic, with_txt);

        if (!is_ref &&
            !spdx_license_lookup(lic) &&
            !spdx_exception_lookup(lic)) {
            fprintf(stderr,
                    "ERROR: '%s' is not a known SPDX identifier.\n"
                    "       Check spelling and case against the SPDX "
                    "License List:\n"
                    "         https://spdx.org/licenses/\n",
                    lic);
            error_count++;
            continue;
        }

        if (!found) {
#ifdef __LINUX__
            snprintf(expected, sizeof(expected), "%s/%s.txt", lic_path, lic);
#else
            snprintf(expected, sizeof(expected), "%s\\%s.txt", lic_path, lic);
#endif
            fprintf(stderr,
                    "ERROR: missing license file for %s.\n"
                    "       Files declare this license but\n"
                    "       %s does not exist.\n"
                    "       REUSE requires the full license text in "
                    "LICENSES/\n"
                    "       at the project root (REUSE Specification 3.3).\n"
                    "       Fix one of:\n"
                    "         - create %s with the license text;\n"
                    "         - or run '%s annotate' to create it "
                    "automatically.\n",
                    lic, expected, expected, wcc_cmd);
            error_count++;
            continue;
        }

        if (is_ref) continue;

        {
            const char *db_text;
            char *file_text = NULL;
            char *norm_file = NULL;
            char *norm_db = NULL;
            int equal = 0;

            db_text = spdx_license_get_text(lic);
            if (!db_text) db_text = spdx_exception_get_text(lic);
            if (!db_text) continue;

            if (spdx_strlist_contains(&files_in_lic, with_txt))
                actual_fname = with_txt;
            else
                actual_fname = lic;

#ifdef __LINUX__
            snprintf(full_path, sizeof(full_path),
                     "%s/%s", lic_path, actual_fname);
#else
            snprintf(full_path, sizeof(full_path),
                     "%s\\%s", lic_path, actual_fname);
#endif

            file_text = spdx_read_file_all(full_path, NULL);
            if (file_text) {
                norm_file = spdx_normalize_text(file_text);
                norm_db = spdx_normalize_text(db_text);
                if (norm_file && norm_db &&
                    strcmp(norm_file, norm_db) == 0)
                    equal = 1;
                free(norm_file);
                free(norm_db);
                free(file_text);
            }

            if (!equal) {
                fprintf(stderr,
                        "ERROR: license text for %s does not match "
                        "the SPDX License List.\n"
                        "       File: %s\n"
                        "       To update it, run "
                        "'%s annotate'.\n",
                        lic, full_path, wcc_cmd);
                error_count++;
            }
        }
    }

    spdx_strlist_free(&files_in_lic);
}

int main(int argc, char *argv[]) {
    const char *dir = ".";
    const char *spdx_db_root = NULL;
    const char *cache_file = NULL;
    int no_gitignore = 0;
    int i;
    ReuseConfig **configs = NULL;
    int config_count = 0;
    SpdxStrList toml_paths;
    SpdxStrList used_licenses;
    SpdxStrList paths;
    int db_errs;
    SpdxWalkOptions walk_opts;
    char *repo_root = NULL;
    GITIGNORELIST gitignore_rules;
    int has_gitignore = 0;

    GitIgnoreListInit(&gitignore_rules);

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: reuse-lint [options] [<directory>]\n"
                   "  --spdx-db=<path>           SPDX database root "
                   "(licenses.json,\n"
                   "                             exceptions.json, details/, "
                   "exceptions/)\n"
                   "  --cache=<path>             SPDX database cache file\n"
                   "  --default-license=<id>     Fallback license "
                   "identifier\n"
                   "  --default-copyright=<text> Fallback copyright text\n"
                   "  --no-gitignore             Do not apply .gitignore "
                   "rules\n"
                   "  --help, -h                 Show this help\n");
            GitIgnoreListFree(&gitignore_rules);
            return 0;
        }
        if (strncmp(argv[i], "--spdx-db=", 10) == 0)
            spdx_db_root = argv[i] + 10;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            cache_file = argv[i] + 8;
        else if (strncmp(argv[i], "--default-license=", 18) == 0)
            default_license = argv[i] + 18;
        else if (strncmp(argv[i], "--default-copyright=", 20) == 0)
            default_copyright = argv[i] + 20;
        else if (strcmp(argv[i], "--no-gitignore") == 0)
            no_gitignore = 1;
        else if (argv[i][0] != '-')
            dir = argv[i];
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s\n"
                    "       Run 'reuse-lint --help' for usage.\n",
                    argv[i]);
            return 1;
        }
    }

    if (!spdx_db_root) {
        fprintf(stderr,
                "ERROR: SPDX database is not configured.\n"
                "       --spdx-db=<path> is required.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n"
                "       Run 'reuse-lint --help' for usage.\n");
        GitIgnoreListFree(&gitignore_rules);
        return 1;
    }

    db_errs = spdx_db_init(spdx_db_root, cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        fprintf(stderr, "ERROR: SPDX license database is unavailable "
                        "(licenses.json not loaded).\n"
                        "       Expected at <spdx-db>/licenses.json.\n"
                        "       Cannot validate SPDX identifiers. "
                        "Aborting.\n");
        GitIgnoreListFree(&gitignore_rules);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        fprintf(stderr, "ERROR: SPDX exceptions database is unavailable "
                        "(exceptions.json not loaded).\n"
                        "       Expected at <spdx-db>/exceptions.json.\n"
                        "       Cannot validate SPDX identifiers. "
                        "Aborting.\n");
        GitIgnoreListFree(&gitignore_rules);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_CACHE)
        fprintf(stderr, "WARNING: cache could not be written.\n"
                        "         Next run will re-parse JSON indexes.\n");

    if (GitFindRepoRoot(dir, &repo_root) != GIT_NO_ERROR) {
        repo_root = NULL;
    }

    {
        int reuse_errors = 0;
        reuse_find_all_tomls(repo_root, dir, &toml_paths);
        configs = reuse_parse_all(&toml_paths, &config_count, &reuse_errors);
        spdx_strlist_free(&toml_paths);
        if (reuse_errors > 0) {
            error_count += reuse_errors;
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

    if (!no_gitignore) {
        if (GitCollectGitignores(repo_root, dir, &gitignore_rules) == GIT_NO_ERROR &&
            gitignore_rules.ulCount > 0) {
            has_gitignore = 1;
        }
    }

    spdx_strlist_init(&used_licenses);

    spdx_walk_options_default(&walk_opts);
    walk_opts.recursive = 0;
    if (has_gitignore) {
        walk_opts.use_gitignore   = 1;
        walk_opts.repo_root       = repo_root ? repo_root : dir;
        walk_opts.gitignore_rules = &gitignore_rules;
    }

    spdx_strlist_init(&paths);
    if (spdx_walk_tree(dir, &walk_opts, &paths) != 0) {
        fprintf(stderr,
                "ERROR: cannot walk tree: %s\n"
                "       Check that the directory exists and is readable.\n",
                dir);
        spdx_strlist_free(&paths);
        spdx_strlist_free(&used_licenses);
        reuse_free_all(configs, config_count);
        GitIgnoreListFree(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        return 1;
    }
    for (i = 0; i < paths.count; i++) {
        process_file(paths.items[i], configs, config_count, &used_licenses);
    }
    spdx_strlist_free(&paths);

    fprintf(stderr, "\n=== LICENSES/ ===\n");
    check_licenses_dir(repo_root ? repo_root : dir, &used_licenses);

    fprintf(stderr, "\n");
    fprintf(stderr, "REUSE Lint summary for %s:\n", dir);
    fprintf(stderr, "  Total files:                %d\n", total_files);
    fprintf(stderr, "  Files with license info:    %d / %d\n",
            files_with_license, total_files);
    fprintf(stderr, "  Files with copyright info:  %d / %d\n",
            files_with_copyright, total_files);
    if (total_snippets > 0) {
        fprintf(stderr, "  Snippets:                   %d\n", total_snippets);
        fprintf(stderr, "  Snippets with license:      %d / %d\n",
                snippets_with_license, total_snippets);
    }
    fprintf(stderr, "  Read errors:                %d\n", read_errors);
    fprintf(stderr, "  Used licenses:              ");
    if (used_licenses.count == 0) {
        fprintf(stderr, "(none)\n");
    } else {
        int k;
        for (k = 0; k < used_licenses.count; k++)
            fprintf(stderr, "%s%s", k > 0 ? ", " : "", used_licenses.items[k]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "  Errors:                     %d\n", error_count);
    fprintf(stderr, "  Warnings:                   %d\n", warning_count);

    spdx_strlist_free(&used_licenses);
    reuse_free_all(configs, config_count);
    GitIgnoreListFree(&gitignore_rules);
    free(repo_root);
    spdx_db_free();

    return (error_count > 0) ? 1 : 0;
}
