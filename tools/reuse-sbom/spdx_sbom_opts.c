/* spdx_sbom_opts.c - разбор аргументов командной строки генератора SBOM (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_opts.h"

/**
 * @brief Duplicate a substring [start, start+len) into a new buffer.
 *
 * @param[in] start  Pointer to the first character. Not NULL.
 * @param[in] len    Number of characters to copy.
 *
 * @return malloc'd NUL-terminated string, or NULL on OOM.
 */
static char *dup_range(const char *start, size_t len) {
    char *p = (char*)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, start, len);
    p[len] = '\0';
    return p;
}

/**
 * @brief Append a copy of a string to a NULL-terminated list.
 *
 * The list is grown by one element. On OOM, an error is printed and
 * the process exits with EXIT_FAILURE.
 *
 * @param[in,out] list  Pointer to the list pointer. Not NULL.
 * @param[in,out] count Pointer to the element count. Not NULL.
 * @param[in]     str   Source string. Not NULL.
 * @param[in]     len   Number of characters to copy.
 */
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

/**
 * @brief Split a whitespace-separated string into a list of tokens.
 *
 * Tokens are separated by spaces or tabs. Quoting is not supported.
 *
 * @param[in]     arg   Source string. Not NULL.
 * @param[in,out] list  Receiver list. Not NULL.
 * @param[in,out] count Receiver count. Not NULL.
 */
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

/**
 * @brief Parse the --objects= option.
 *
 * @param[in]     arg   Option value. Not NULL.
 * @param[in,out] opts  Options structure. Not NULL.
 */
static void parse_object_list(const char *arg, SbomOptions *opts) {
    parse_list(arg, &opts->object_files, &opts->object_count);
}

/**
 * @brief Parse the --res= option.
 *
 * @param[in]     arg   Option value. Not NULL.
 * @param[in,out] opts  Options structure. Not NULL.
 */
static void parse_res_list(const char *arg, SbomOptions *opts) {
    parse_list(arg, &opts->res_files, &opts->res_count);
}

/**
 * @brief Parse the --extracted-license=<id>:<path> option.
 *
 * Appends one ExtractedLicenseSource entry. The id must be non-empty
 * and fit into ExtractedLicenseSource.id.
 *
 * @param[in,out] opts  Options structure. Not NULL.
 * @param[in]     arg   Option value, in the form "<id>:<path>".
 */
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

/**
 * @brief Print command-line usage.
 *
 * Lists all supported options with a short description. Invoked by
 * --help / -h. The process exits with EXIT_SUCCESS afterwards (see
 * sbom_parse_args).
 */
static void print_help(void) {
    printf("Usage: reuse-sbom [options] [<directory>]\n"
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
           "  --format=<fmt>             Output format:\n"
           "                               spdx-json  - SPDX 2.3 JSON "
           "(default)\n"
           "                               spdx-tag   - SPDX 2.3 tag-value\n"
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
           "  --no-gitignore             Do not apply .gitignore rules\n"
           "  --help, -h                 Show this help\n");
}

/**
 * @brief Parse the SBOM generator command line.
 *
 * Recognized options are listed by print_help. --help / -h print
 * usage and exit with EXIT_SUCCESS. Unknown options and missing
 * required values print an error and return -1.
 *
 * @param[in]  argc  Argument count.
 * @param[in]  argv  Argument vector.
 * @param[out] opts  Receiver. Not NULL. Must be zero-initialized by
 *                   the caller before the call; sbom_parse_args does
 *                   a memset itself.
 *
 * @return 0 on success, -1 on error.
 */
int sbom_parse_args(int argc, char *argv[], SbomOptions *opts) {
    int i;

    memset(opts, 0, sizeof(*opts));
    opts->dir = ".";
    opts->format = "spdx-json";

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
        else if (strncmp(a, "--extracted-license=", 20) == 0)
            add_extracted(opts, a + 20);
        else if (strcmp(a, "--no-gitignore") == 0)
            opts->no_gitignore = 1;
        else if (a[0] != '-')
            opts->dir = a;
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s\n"
                    "       Run 'reuse-sbom --help' for usage.\n", a);
            return -1;
        }
    }

    if (!opts->binary_file) {
        fprintf(stderr,
                "ERROR: --file=<binary file> is required.\n"
                "       Run 'reuse-sbom --help' for usage.\n");
        return -1;
    }
    if (!opts->doc_name) {
        fprintf(stderr,
                "ERROR: --name=<package name> is required.\n"
                "       Run 'reuse-sbom --help' for usage.\n");
        return -1;
    }
    if (!opts->spdx_db_root) {
        fprintf(stderr,
                "ERROR: --spdx-db=<path> is required.\n"
                "       Run 'reuse-sbom --help' for usage.\n");
        return -1;
    }
    if (strcmp(opts->format, "spdx-json") != 0 &&
        strcmp(opts->format, "spdx-tag") != 0) {
        fprintf(stderr,
                "ERROR: unsupported output format: %s\n"
                "       Supported: spdx-json, spdx-tag.\n"
                "       Run 'reuse-sbom --help' for usage.\n",
                opts->format);
        return -1;
    }
    return 0;
}

/**
 * @brief Release memory allocated by sbom_parse_args.
 *
 * Frees object_files, res_files and extracted_sources, and resets the
 * corresponding fields to NULL / 0. Safe to call on a zeroed structure.
 *
 * @param[in,out] opts  Parsed options. Not NULL.
 */
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
                                        const char *id) {
    int i;
    for (i = 0; i < opts->extracted_count; i++)
        if (strcmp(opts->extracted_sources[i].id, id) == 0)
            return opts->extracted_sources[i].path;
    return NULL;
}
