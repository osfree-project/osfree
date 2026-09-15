/* spdx_sbom_opts.c - разбор аргументов командной строки spdx-sbom (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_opts.h"

static char *dup_range(const char *start, size_t len) {
    char *p = (char*)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, start, len);
    p[len] = '\0';
    return p;
}

static void add_list_item(char ***list, int *count, const char *str, size_t len) {
    char *copy;
    char **new_list;

    if (len == 0) return;
    copy = dup_range(str, len);
    if (!copy) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }
    new_list = (char**)realloc(*list,
        (size_t)(*count + 1) * sizeof(char*));
    if (!new_list) {
        fprintf(stderr, "ERROR: out of memory\n");
        free(copy);
        exit(EXIT_FAILURE);
    }
    *list = new_list;
    (*list)[*count] = copy;
    (*count)++;
}

/* –азбирает строку, разделЄнную пробелами, копиру€ каждый токен. */
static void parse_list(const char *arg, char ***list, int *count) {
    const char *p = arg;
    while (*p) {
        const char *start;
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        add_list_item(list, count, start, (size_t)(p - start));
    }
}

static void parse_object_list(const char *arg, SbomOptions *opts) {
    parse_list(arg, &opts->object_files, &opts->object_count);
}

static void parse_res_list(const char *arg, SbomOptions *opts) {
    parse_list(arg, &opts->res_files, &opts->res_count);
}

static void add_extracted(SbomOptions *opts, const char *arg) {
    const char *colon = strchr(arg, ':');
    size_t id_len;
    ExtractedLicenseSource *src;

    if (!colon) {
        fprintf(stderr,
                "ERROR: invalid --extracted-license value: %s\n"
                "       Expected format: --extracted-license=<id>:<path>\n"
                "       Example: --extracted-license=LicenseRef-Custom:"
                "custom.txt\n", arg);
        exit(EXIT_FAILURE);
    }
    id_len = (size_t)(colon - arg);
    if (id_len == 0 || id_len >= sizeof(src->id)) {
        fprintf(stderr,
                "ERROR: bad LicenseRef id in --extracted-license: %s\n"
                "       The id must be non-empty and shorter than %d "
                "characters.\n", arg, (int)sizeof(src->id));
        exit(EXIT_FAILURE);
    }

    opts->extracted_sources = (ExtractedLicenseSource*)realloc(
        opts->extracted_sources,
        (size_t)(opts->extracted_count + 1) * sizeof(ExtractedLicenseSource));
    if (!opts->extracted_sources) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }
    src = &opts->extracted_sources[opts->extracted_count];
    memcpy(src->id, arg, id_len);
    src->id[id_len] = '\0';
    strncpy(src->path, colon + 1, sizeof(src->path) - 1);
    src->path[sizeof(src->path) - 1] = '\0';
    opts->extracted_count++;
}

static void print_help(void) {
    printf("Usage: spdx-sbom [options] [<directory>]\n"
           "\n"
           "Required:\n"
           "  --name=<name>              Package name\n"
           "  --file=<binary file>       Binary artifact to describe\n"
           "  --spdx-db=<path>           SPDX database root "
           "(licenses.json,\n"
           "                             exceptions.json, details/, "
           "exceptions/)\n"
           "\n"
           "Optional:\n"
           "  --version=<ver>            Package version\n"
           "  --supplier=<name>          Package supplier\n"
           "  --creator=<name>           SBOM creator\n"
           "  --purpose=<purpose>        Package purpose "
           "(SOURCE, BINARY, LIBRARY, ...)\n"
           "  --output=<file>            Write SBOM to file "
           "(default: stdout)\n"
           "  --format=json|tagvalue     Output format (default: json)\n"
           "  --default-license=<id>     Fallback license identifier\n"
           "  --default-copyright=<text> Fallback copyright text\n"
           "  --source-sbom=<file>       Source SBOM for binary mode "
           "(with --purpose != SOURCE)\n"
           "  --objects=<list>           Object files (space-separated)\n"
           "  --res=<list>               Resource files (space-separated)\n"
           "  --extracted-license=<id>:<path>\n"
           "                             Provide text for a "
           "LicenseRef-* license\n"
           "  --cache=<path>             SPDX database cache file\n"
           "  --details-dir=<path>       SPDX details directory "
           "(if not under spdx-db)\n"
           "  --exceptions-dir=<path>    SPDX exceptions directory "
           "(if not under spdx-db)\n"
           "  --no-gitignore             Do not apply .gitignore rules\n"
           "  --help, -h                 Show this help\n");
}

int sbom_parse_args(int argc, char *argv[], SbomOptions *opts) {
    int i;

    memset(opts, 0, sizeof(*opts));
    opts->dir = ".";
    opts->format = "json";

    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            print_help();
            exit(EXIT_SUCCESS);
        }
        else if (strncmp(a, "--output=", 9) == 0)
            opts->output = a + 9;
        else if (strncmp(a, "--format=", 9) == 0)
            opts->format = a + 9;
        else if (strncmp(a, "--default-license=", 18) == 0)
            opts->default_license = a + 18;
        else if (strncmp(a, "--default-copyright=", 20) == 0)
            opts->default_copyright = a + 20;
        else if (strncmp(a, "--name=", 7) == 0)
            opts->doc_name = a + 7;
        else if (strncmp(a, "--version=", 10) == 0)
            opts->package_version = a + 10;
        else if (strncmp(a, "--supplier=", 11) == 0)
            opts->package_supplier = a + 11;
        else if (strncmp(a, "--creator=", 10) == 0)
            opts->creator = a + 10;
        else if (strncmp(a, "--purpose=", 10) == 0)
            opts->package_purpose = a + 10;
        else if (strncmp(a, "--file=", 7) == 0)
            opts->binary_file = a + 7;
        else if (strncmp(a, "--objects=", 10) == 0)
            parse_object_list(a + 10, opts);
        else if (strncmp(a, "--res=", 6) == 0)
            parse_res_list(a + 6, opts);
        else if (strncmp(a, "--source-sbom=", 14) == 0)
            opts->source_sbom_path = a + 14;
        else if (strncmp(a, "--spdx-db=", 10) == 0)
            opts->spdx_db_root = a + 10;
        else if (strncmp(a, "--cache=", 8) == 0)
            opts->cache_file = a + 8;
        else if (strncmp(a, "--details-dir=", 14) == 0)
            opts->details_dir = a + 14;
        else if (strncmp(a, "--exceptions-dir=", 17) == 0)
            opts->exceptions_dir = a + 17;
        else if (strncmp(a, "--extracted-license=", 20) == 0)
            add_extracted(opts, a + 20);
        else if (strcmp(a, "--no-gitignore") == 0)
            opts->no_gitignore = 1;
        else if (a[0] != '-')
            opts->dir = a;
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s\n"
                    "       Run 'spdx-sbom --help' for usage.\n", a);
            return -1;
        }
    }

    if (!opts->binary_file) {
        fprintf(stderr,
                "ERROR: --file=<binary file> is required.\n"
                "       Run 'spdx-sbom --help' for usage.\n");
        return -1;
    }
    if (!opts->doc_name) {
        fprintf(stderr,
                "ERROR: --name=<package name> is required.\n"
                "       Run 'spdx-sbom --help' for usage.\n");
        return -1;
    }
    if (!opts->spdx_db_root) {
        fprintf(stderr,
                "ERROR: --spdx-db=<path> is required.\n"
                "       Run 'spdx-sbom --help' for usage.\n");
        return -1;
    }
    if (strcmp(opts->format, "json") != 0 &&
        strcmp(opts->format, "tagvalue") != 0 &&
        strcmp(opts->format, "tag") != 0) {
        fprintf(stderr,
                "ERROR: unsupported output format: %s\n"
                "       Supported: json, tagvalue (or tag).\n"
                "       Run 'spdx-sbom --help' for usage.\n",
                opts->format);
        return -1;
    }
    return 0;
}

void sbom_options_free(SbomOptions *opts) {
    int i;
    if (opts->object_files) {
        for (i = 0; i < opts->object_count; i++)
            free(opts->object_files[i]);
        free(opts->object_files);
    }
    if (opts->res_files) {
        for (i = 0; i < opts->res_count; i++)
            free(opts->res_files[i]);
        free(opts->res_files);
    }
    free(opts->extracted_sources);
    opts->object_files = NULL;
    opts->object_count = 0;
    opts->res_files = NULL;
    opts->res_count = 0;
    opts->extracted_sources = NULL;
    opts->extracted_count = 0;
}

const char *sbom_options_find_extracted(const SbomOptions *opts,
                                        const char *id) {
    int i;
    for (i = 0; i < opts->extracted_count; i++)
        if (strcmp(opts->extracted_sources[i].id, id) == 0)
            return opts->extracted_sources[i].path;
    return NULL;
}
