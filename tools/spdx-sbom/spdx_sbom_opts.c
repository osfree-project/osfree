/* spdx_sbom_opts.c - разбор аргументов командной строки spdx-sbom (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_sbom_opts.h"

static void parse_space_list(const char *arg, char ***list, int *count) {
    while (*arg) {
        while (*arg && *arg == ' ') arg++;
        if (!*arg) break;
        *list = (char**)realloc(*list, (*count + 1) * sizeof(char*));
        if (!*list) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        (*list)[*count] = (char*)arg;
        (*count)++;
        while (*arg && *arg != ' ') arg++;
        if (*arg) {
            *(char*)arg = '\0';
            arg++;
        }
    }
}

static void parse_object_list(const char *arg, SbomOptions *opts) {
    char *copy = (char*)malloc(strlen(arg) + 1);
    char *tok;
    if (!copy) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, arg);
    tok = strtok(copy, " ");
    while (tok) {
        opts->object_files = (char**)realloc(opts->object_files,
            (size_t)(opts->object_count + 1) * sizeof(char*));
        if (!opts->object_files) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        opts->object_files[opts->object_count++] = tok;
        tok = strtok(NULL, " ");
    }
}

static void parse_res_list(const char *arg, SbomOptions *opts) {
    char *copy = (char*)malloc(strlen(arg) + 1);
    char *tok;
    if (!copy) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, arg);
    tok = strtok(copy, " ");
    while (tok) {
        opts->res_files = (char**)realloc(opts->res_files,
            (size_t)(opts->res_count + 1) * sizeof(char*));
        if (!opts->res_files) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        opts->res_files[opts->res_count++] = tok;
        tok = strtok(NULL, " ");
    }
}

static void add_extracted(SbomOptions *opts, const char *arg) {
    const char *colon = strchr(arg, ':');
    size_t id_len;
    ExtractedLicenseSource *src;

    if (!colon) {
        fprintf(stderr,
                "Error: --extracted-license requires <id>:<path>, got: %s\n",
                arg);
        exit(EXIT_FAILURE);
    }
    id_len = (size_t)(colon - arg);
    if (id_len == 0 || id_len >= sizeof(src->id)) {
        fprintf(stderr, "Error: bad LicenseRef id in %s\n", arg);
        exit(EXIT_FAILURE);
    }

    opts->extracted_sources = (ExtractedLicenseSource*)realloc(
        opts->extracted_sources,
        (size_t)(opts->extracted_count + 1) * sizeof(ExtractedLicenseSource));
    if (!opts->extracted_sources) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    src = &opts->extracted_sources[opts->extracted_count];
    memcpy(src->id, arg, id_len);
    src->id[id_len] = '\0';
    strncpy(src->path, colon + 1, sizeof(src->path) - 1);
    src->path[sizeof(src->path) - 1] = '\0';
    opts->extracted_count++;
}

int sbom_parse_args(int argc, char *argv[], SbomOptions *opts) {
    int i;

    memset(opts, 0, sizeof(*opts));
    opts->dir = ".";
    opts->format = "json";

    for (i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strncmp(a, "--output=", 9) == 0)
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
        else if (strncmp(a, "--exclude=", 10) == 0)
            parse_space_list(a + 10, &opts->exclude_list, &opts->exclude_count);
        else if (strncmp(a, "--licenses-json=", 16) == 0)
            opts->licenses_json = a + 16;
        else if (strncmp(a, "--exceptions-json=", 18) == 0)
            opts->exceptions_json = a + 18;
        else if (strncmp(a, "--details-dir=", 14) == 0)
            opts->details_dir = a + 14;
        else if (strncmp(a, "--exceptions-dir=", 17) == 0)
            opts->exceptions_dir = a + 17;
        else if (strncmp(a, "--cache=", 8) == 0)
            opts->cache_file = a + 8;
        else if (strncmp(a, "--extracted-license=", 20) == 0)
            add_extracted(opts, a + 20);
        else if (a[0] != '-')
            opts->dir = a;
        else {
            fprintf(stderr, "Unknown option: %s\n", a);
            return -1;
        }
    }

    if (!opts->binary_file) {
        fprintf(stderr, "Error: --file=<binary file> is required\n");
        return -1;
    }
    if (!opts->doc_name) {
        fprintf(stderr, "Error: --name=<package name> is required\n");
        return -1;
    }
    if (!opts->licenses_json) {
        fprintf(stderr, "Error: --licenses-json=<path> is required\n");
        return -1;
    }
    if (!opts->exceptions_json) {
        fprintf(stderr, "Error: --exceptions-json=<path> is required\n");
        return -1;
    }
    if (!opts->cache_file) {
        fprintf(stderr, "Error: --cache=<path> is required\n");
        return -1;
    }
    if (strcmp(opts->format, "json") != 0 &&
        strcmp(opts->format, "tagvalue") != 0 &&
        strcmp(opts->format, "tag") != 0) {
        fprintf(stderr, "Unsupported format: %s\n", opts->format);
        return -1;
    }
    return 0;
}

void sbom_options_free(SbomOptions *opts) {
    free(opts->exclude_list);
    free(opts->object_files);
    free(opts->res_files);
    free(opts->extracted_sources);
    opts->exclude_list = NULL;
    opts->exclude_count = 0;
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
