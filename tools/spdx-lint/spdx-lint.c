/* spdx-lint.c - проверка проекта на соответствие REUSE / SPDX (C89, OpenWatcom) */

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

#include <reuse_parser.h>
#include "spdx_db.h"
#include "spdx_discover.h"
#include "spdx_utils.h"
#include "spdx_lic.h"
#include "git_utils.h"

static int error_count = 0;
static int warning_count = 0;

static const char *default_license = NULL;
static const char *default_copyright = NULL;

static char **exclude_list = NULL;
static int exclude_count = 0;

static int total_files = 0;
static int files_with_license = 0;
static int files_with_copyright = 0;
static int read_errors = 0;

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

static int is_excluded(const char *path) {
    int i;
    const char *base = spdx_get_file_name(path);
    for (i = 0; i < exclude_count; i++)
        if (strcmp(exclude_list[i], base) == 0) return 1;
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
                "       See https://spdx.github.io/spdx-spec/v2.3/"
                "SPDX-license-expressions/ for the grammar.\n",
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
                "         - or add a [[annotations]] entry in REUSE.toml.\n");
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
                        "         Check https://spdx.org/licenses/ for the "
                        "recommended replacement.\n",
                        fullpath, ids.items[m]);
                warning_count++;
            }
        }
        spdx_strlist_free(&ids);
    }
}

static void process_file(const char *fullpath,
                         ReuseConfig **configs, int config_count,
                         SpdxStrList *used_licenses) {
    FileLicenseInfo lic;
    const char *name = spdx_get_file_name(fullpath);
    FILE *f;

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

    if (spdx_resolve_license(configs, config_count, fullpath, name,
                             default_license, default_copyright,
                             &lic) != 0) {
        fprintf(stderr,
                "ERROR: %s has no license information.\n"
                "       REUSE requires each file to carry license and "
                "copyright information.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier' and "
                "'SPDX-FileCopyrightText' tags in the file header;\n"
                "         - or create a sidecar '<file>.license' next to it;\n"
                "         - or add a [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        error_count++;
        return;
    }

    if (lic.license_from_default) {
        fprintf(stderr,
                "WARNING: %s: license is taken from --default-license.\n"
                "         REUSE does not allow a global CLI fallback.\n"
                "         For REUSE compliance, add one of:\n"
                "           - 'SPDX-License-Identifier: <id>' tag in the file;\n"
                "           - <file>.license sidecar;\n"
                "           - [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        warning_count++;
    }
    if (lic.copyright_from_default) {
        fprintf(stderr,
                "WARNING: %s: copyright is taken from --default-copyright.\n"
                "         REUSE does not allow a global CLI fallback.\n"
                "         For REUSE compliance, add one of:\n"
                "           - 'SPDX-FileCopyrightText: <holder>' tag in the file;\n"
                "           - <file>.license sidecar;\n"
                "           - [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        warning_count++;
    }

    if (lic.license[0] != '\0') {
        files_with_license++;
    } else {
        fprintf(stderr,
                "ERROR: %s has no license information.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-License-Identifier: <id>' in the file "
                "header;\n"
                "         - or create '<file>.license' with the same tag;\n"
                "         - or add [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        error_count++;
    }
    if (lic.copyright[0] != '\0') {
        files_with_copyright++;
    } else {
        fprintf(stderr,
                "ERROR: %s has no copyright information.\n"
                "       Fix one of:\n"
                "         - add 'SPDX-FileCopyrightText: <holder>' in the "
                "file header;\n"
                "         - or create '<file>.license' with the same tag;\n"
                "         - or add [[annotations]] entry in REUSE.toml.\n",
                fullpath);
        error_count++;
    }

    if (lic.license[0] != '\0') {
        check_license_expression(fullpath, lic.license, used_licenses);
    }
}

/* Печатает URL для скачивания текста лицензии или исключения. */
static void print_license_url(const char *id) {
    if (strncmp(id, "LicenseRef-", 11) == 0) return;

    if (spdx_license_lookup(id) != NULL) {
        fprintf(stderr,
                "       Download license text from:\n"
                "         https://raw.githubusercontent.com/spdx/"
                "license-list-data/main/text/%s.txt\n",
                id);
    } else if (spdx_exception_lookup(id) != NULL) {
        fprintf(stderr,
                "       Download license text from:\n"
                "         https://raw.githubusercontent.com/spdx/"
                "license-list-data/main/text/exceptions/%s.txt\n",
                id);
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
                "       Create the directory and add a text file for each\n"
                "       license declared by any file in the project.\n",
                lic_path, example_path);
        error_count++;
    }

    for (i = 0; i < files_in_lic.count; i++) {
        const char *fname = files_in_lic.items[i];
        char base[256];
        char *dot;
        strncpy(base, fname, sizeof(base) - 1);
        base[sizeof(base) - 1] = '\0';
        dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        if (!is_valid_spdx_name(base)) {
            fprintf(stderr,
                    "ERROR: Bad license file name: %s\n"
                    "       The name must be a valid SPDX License List "
                    "identifier or start with 'LicenseRef-'.\n"
                    "       Fix one of:\n"
                    "         - rename the file to a valid SPDX identifier "
                    "(e.g. 'MIT.txt');\n"
                    "         - or use 'LicenseRef-<name>.txt' for a custom "
                    "license.\n"
                    "       See https://spdx.org/licenses/ for the full list.\n",
                    fname);
            error_count++;
        } else {
            if (!has_extension(fname)) {
                fprintf(stderr,
                        "WARNING: License file without extension: %s\n"
                        "         REUSE convention is to use '.txt'.\n"
                        "         Rename to '%s.txt' to follow the convention "
                        "and avoid ambiguity.\n",
                        fname, fname);
                warning_count++;
            }
            if (spdx_license_is_deprecated(base) ||
                spdx_exception_is_deprecated(base)) {
                fprintf(stderr,
                        "WARNING: Deprecated license file: %s\n"
                        "         SPDX License List marks '%s' as "
                        "deprecated.\n"
                        "         Rename the file to the current identifier "
                        "and update all references in source files.\n"
                        "         Check https://spdx.org/licenses/ for the "
                        "recommended replacement.\n",
                        fname, base);
                warning_count++;
            }
        }
    }

    for (i = 0; i < files_in_lic.count; i++) {
        char base[256];
        char *dot;
        strncpy(base, files_in_lic.items[i], sizeof(base) - 1);
        base[sizeof(base) - 1] = '\0';
        dot = strrchr(base, '.');
        if (dot) *dot = '\0';

        if (!spdx_strlist_contains(used_licenses, base)) {
            fprintf(stderr,
                    "ERROR: Unused license file: %s\n"
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
        int found;

        snprintf(with_txt, sizeof(with_txt), "%s.txt", lic);
        found = spdx_strlist_contains(&files_in_lic, lic) ||
                spdx_strlist_contains(&files_in_lic, with_txt);
        if (found) continue;

#ifdef __LINUX__
        snprintf(expected, sizeof(expected), "%s/%s.txt", lic_path, lic);
#else
        snprintf(expected, sizeof(expected), "%s\\%s.txt", lic_path, lic);
#endif

        if (!spdx_license_lookup(lic) && !spdx_exception_lookup(lic) &&
            strncmp(lic, "LicenseRef-", 11) != 0) {
            fprintf(stderr,
                    "ERROR: '%s' is not a known SPDX identifier.\n"
                    "       SPDX License Identifiers are case-sensitive.\n"
                    "       Check spelling and case against the SPDX "
                    "License List:\n"
                    "         https://spdx.org/licenses/\n",
                    lic);
            error_count++;
        }

        fprintf(stderr,
                "ERROR: Missing license file for %s.\n"
                "       Files declare this license but\n"
                "       %s does not exist.\n"
                "       REUSE requires the full license text in LICENSES/\n"
                "       at the project root (REUSE Specification 3.3).\n"
                "       Fix one of:\n"
                "         - create %s with the license text;\n"
                "         - or, if this is a custom license not on the SPDX\n"
                "           License List, change the identifier in the source\n"
                "           files to 'LicenseRef-<name>' and create\n"
                "           LICENSES/LicenseRef-<name>.txt with the license text.\n",
                lic, expected, expected);
        print_license_url(lic);
        error_count++;
    }

    spdx_strlist_free(&files_in_lic);
}

int main(int argc, char *argv[]) {
    const char *dir = ".";
    const char *licenses_json = NULL;
    const char *exceptions_json = NULL;
    const char *details_dir = NULL;
    const char *exceptions_dir = NULL;
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
    GitIgnoreList gitignore_rules;
    int has_gitignore = 0;

    git_ignore_list_init(&gitignore_rules);

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--licenses-json=", 16) == 0)
            licenses_json = argv[i] + 16;
        else if (strncmp(argv[i], "--exceptions-json=", 18) == 0)
            exceptions_json = argv[i] + 18;
        else if (strncmp(argv[i], "--details-dir=", 14) == 0)
            details_dir = argv[i] + 14;
        else if (strncmp(argv[i], "--exceptions-dir=", 17) == 0)
            exceptions_dir = argv[i] + 17;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            cache_file = argv[i] + 8;
        else if (strncmp(argv[i], "--default-license=", 18) == 0)
            default_license = argv[i] + 18;
        else if (strncmp(argv[i], "--default-copyright=", 20) == 0)
            default_copyright = argv[i] + 20;
        else if (strcmp(argv[i], "--no-gitignore") == 0)
            no_gitignore = 1;
        else if (strncmp(argv[i], "--exclude=", 10) == 0) {
            char *arg = argv[i] + 10;
            while (*arg) {
                while (*arg && *arg == ' ') arg++;
                if (!*arg) break;
                exclude_list = (char**)realloc(exclude_list,
                                               (exclude_count + 1) * sizeof(char*));
                if (!exclude_list) {
                    fprintf(stderr, "Memory allocation failed\n");
                    return 1;
                }
                exclude_list[exclude_count++] = arg;
                while (*arg && *arg != ' ') arg++;
                if (*arg) { *arg = '\0'; arg++; }
            }
        }
        else if (argv[i][0] != '-')
            dir = argv[i];
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (!licenses_json || !exceptions_json) {
        fprintf(stderr,
                "ERROR: SPDX database is not configured.\n"
                "       --licenses-json=<path> and --exceptions-json=<path> "
                "are required.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        git_ignore_list_free(&gitignore_rules);
        free(exclude_list);
        return 1;
    }

    db_errs = spdx_db_init(licenses_json, exceptions_json,
                           details_dir, exceptions_dir, cache_file);
    if (db_errs & SPDX_DB_ERR_LICENSES) {
        fprintf(stderr, "ERROR: SPDX license database is unavailable "
                        "(licenses.json not loaded).\n"
                        "       Cannot validate SPDX identifiers. "
                        "Aborting.\n");
        git_ignore_list_free(&gitignore_rules);
        free(exclude_list);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_EXCEPTIONS) {
        fprintf(stderr, "ERROR: SPDX exceptions database is unavailable "
                        "(exceptions.json not loaded).\n"
                        "       Cannot validate SPDX identifiers. "
                        "Aborting.\n");
        git_ignore_list_free(&gitignore_rules);
        free(exclude_list);
        spdx_db_free();
        return 1;
    }
    if (db_errs & SPDX_DB_ERR_CACHE)
        fprintf(stderr, "WARNING: cache could not be written.\n"
                        "         Next run will re-parse JSON indexes.\n");

    /* --- Иерархия REUSE.toml --- */
    repo_root = git_find_repo_root(dir);

    reuse_find_all_tomls(repo_root, dir, &toml_paths);
    configs = reuse_parse_all(&toml_paths, &config_count);
    spdx_strlist_free(&toml_paths);

    /* --- Git-фильтрация --- */
    if (!no_gitignore) {
        if (git_collect_gitignores(repo_root, dir, &gitignore_rules) == 0 &&
            gitignore_rules.count > 0) {
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
        fprintf(stderr, "Cannot walk tree: %s\n", dir);
        spdx_strlist_free(&paths);
        spdx_strlist_free(&used_licenses);
        reuse_free_all(configs, config_count);
        git_ignore_list_free(&gitignore_rules);
        free(repo_root);
        spdx_db_free();
        free(exclude_list);
        return 1;
    }
    for (i = 0; i < paths.count; i++) {
        if (is_excluded(paths.items[i])) continue;
        process_file(paths.items[i], configs, config_count, &used_licenses);
    }
    spdx_strlist_free(&paths);

    check_licenses_dir(repo_root ? repo_root : dir, &used_licenses);

    fprintf(stderr, "\n");
    fprintf(stderr, "SPDX Lint summary for %s:\n", dir);
    fprintf(stderr, "  Total files:                %d\n", total_files);
    fprintf(stderr, "  Files with license info:    %d / %d\n",
            files_with_license, total_files);
    fprintf(stderr, "  Files with copyright info:  %d / %d\n",
            files_with_copyright, total_files);
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
    fprintf(stderr, "\n");
    fprintf(stderr, "SPDX Lint complete. Errors: %d, Warnings: %d\n",
            error_count, warning_count);

    spdx_strlist_free(&used_licenses);
    reuse_free_all(configs, config_count);
    git_ignore_list_free(&gitignore_rules);
    free(repo_root);
    spdx_db_free();
    free(exclude_list);

    return (error_count > 0) ? 1 : 0;
}
