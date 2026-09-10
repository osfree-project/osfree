/* spdx_discover.c - единый модуль определения списка файлов (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_discover.h"
#include "omf_parser.h"
#include "res_parser.h"

#ifdef __LINUX__
#include <dirent.h>
#include <sys/stat.h>
#else
#include <direct.h>
#include <sys/stat.h>
#include <io.h>
#endif

void spdx_walk_options_default(SpdxWalkOptions *opts) {
    memset(opts, 0, sizeof(*opts));
    opts->recursive             = 1;
    opts->skip_hidden           = 1;
    opts->skip_vcs_dirs         = 1;
    opts->skip_licenses_dir     = 1;
    opts->skip_reuse_dir        = 1;
    opts->skip_license_sidecars = 1;
    opts->skip_reuse_toml       = 1;
    opts->skip_license_files    = 1;
}

static int is_vcs_dir(const char *name) {
    return strcmp(name, ".git") == 0 ||
           strcmp(name, ".svn") == 0 ||
           strcmp(name, ".hg")  == 0 ||
           strcmp(name, ".bzr") == 0;
}

static int is_licenses_dir(const char *name) {
    return strcmp(name, "LICENSES") == 0;
}

static int is_reuse_dir(const char *name) {
    return strcmp(name, ".reuse") == 0;
}

static int ieq(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int ieq_prefix(const char *str, const char *prefix, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (str[i] == '\0') return 0;
        if (tolower((unsigned char)str[i]) != tolower((unsigned char)prefix[i]))
            return 0;
    }
    return 1;
}

static int is_license_file(const char *name) {
    static const char *exact[] = {
        "license", "licence", "copying", "unlicense", "copyright"
    };
    size_t i;
    for (i = 0; i < sizeof(exact)/sizeof(exact[0]); i++)
        if (ieq(name, exact[i])) return 1;
    if (ieq_prefix(name, "license", 7) ||
        ieq_prefix(name, "licence", 7) ||
        ieq_prefix(name, "copying", 7)) {
        if (name[7] == '\0' || name[7] == '.' ||
            name[7] == '-' || name[7] == '_')
            return 1;
    }
    return 0;
}

static int is_license_sidecar(const char *name) {
    size_t len = strlen(name);
    return len > 8 && strcmp(name + len - 8, ".license") == 0;
}

static int is_reuse_toml(const char *name) {
    return strcmp(name, "REUSE.toml") == 0;
}

static int is_hidden(const char *name) {
    return name[0] == '.';
}

static int should_skip_file(const char *name, const SpdxWalkOptions *opts) {
    if (opts->skip_hidden && is_hidden(name)) return 1;
    if (opts->skip_license_sidecars && is_license_sidecar(name)) return 1;
    if (opts->skip_reuse_toml && is_reuse_toml(name)) return 1;
    if (opts->skip_license_files && is_license_file(name)) return 1;
    return 0;
}

static int should_skip_dir(const char *name, const SpdxWalkOptions *opts) {
    if (opts->skip_hidden && is_hidden(name)) return 1;
    if (opts->skip_vcs_dirs && is_vcs_dir(name)) return 1;
    if (opts->skip_licenses_dir && is_licenses_dir(name)) return 1;
    if (opts->skip_reuse_dir && is_reuse_dir(name)) return 1;
    return 0;
}

static void join_path(char *dst, size_t dst_size,
                      const char *dir, const char *name) {
    snprintf(dst, dst_size, "%s/%s", dir, name);
}

#ifdef __LINUX__

static int walk_inner(const char *dir, const SpdxWalkOptions *opts,
                      SpdxStrList *out) {
    DIR *d = opendir(dir);
    struct dirent *entry;
    struct stat st;
    char full[1024];

    if (!d) {
        fprintf(stderr, "Error: cannot open directory %s\n", dir);
        return -1;
    }
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) continue;

        join_path(full, sizeof(full), dir, entry->d_name);
        if (stat(full, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            if (!opts->recursive) continue;
            if (should_skip_dir(entry->d_name, opts)) continue;
            if (walk_inner(full, opts, out) != 0) {
                closedir(d);
                return -1;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (should_skip_file(entry->d_name, opts)) continue;
            spdx_strlist_add_unique(out, full);
        }
    }
    closedir(d);
    return 0;
}

#else

static int walk_inner(const char *dir, const SpdxWalkOptions *opts,
                      SpdxStrList *out) {
    long hFile;
    struct _finddata_t fd;
    struct stat st;
    char pattern[1024];
    char full[1024];

    snprintf(pattern, sizeof(pattern), "%s/*", dir);
    hFile = _findfirst(pattern, &fd);
    if (hFile == -1L) {
        fprintf(stderr, "Error: cannot open directory %s\n", dir);
        return -1;
    }
    do {
        if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
            continue;

        join_path(full, sizeof(full), dir, fd.name);
        if (stat(full, &st) != 0) continue;

        if (st.st_mode & _S_IFDIR) {
            if (!opts->recursive) continue;
            if (should_skip_dir(fd.name, opts)) continue;
            if (walk_inner(full, opts, out) != 0) {
                _findclose(hFile);
                return -1;
            }
        } else if (st.st_mode & _S_IFREG) {
            if (should_skip_file(fd.name, opts)) continue;
            spdx_strlist_add_unique(out, full);
        }
    } while (_findnext(hFile, &fd) == 0);
    _findclose(hFile);
    return 0;
}

#endif

int spdx_walk_tree(const char *dir, const SpdxWalkOptions *opts,
                   SpdxStrList *out) {
    return walk_inner(dir, opts, out);
}

int spdx_discover_from_artifacts(char **object_files, int object_count,
                                 char **res_files, int res_count,
                                 SpdxStrList *out) {
    int i, j;
    int any = 0;

    for (i = 0; i < object_count; i++) {
        char **sources = NULL;
        int    count = 0;
        if (omf_extract_sources(object_files[i], &sources, &count) != 0) {
            fprintf(stderr, "Warning: cannot extract sources from %s\n",
                    object_files[i]);
            continue;
        }
        for (j = 0; j < count; j++)
            spdx_strlist_add_unique(out, sources[j]);
        for (j = 0; j < count; j++) free(sources[j]);
        free(sources);
        any = 1;
    }

    for (i = 0; i < res_count; i++) {
        char **sources = NULL;
        int    count = 0;
        if (res_extract_sources(res_files[i], &sources, &count) != 0) {
            fprintf(stderr, "Warning: cannot extract sources from %s\n",
                    res_files[i]);
            continue;
        }
        for (j = 0; j < count; j++)
            spdx_strlist_add_unique(out, sources[j]);
        for (j = 0; j < count; j++) free(sources[j]);
        free(sources);
        any = 1;
    }

    return any ? 0 : -1;
}

int spdx_discover(const char *project_dir,
                  char **object_files, int object_count,
                  char **res_files, int res_count,
                  const SpdxWalkOptions *walk_opts,
                  SpdxStrList *out) {
    if (object_count > 0 || res_count > 0) {
        int rc = spdx_discover_from_artifacts(object_files, object_count,
                                              res_files, res_count, out);
        if (rc != 0) {
            fprintf(stderr,
                    "Error: no sources extracted from build artifacts\n");
            return -1;
        }
        return 0;
    }
    return spdx_walk_tree(project_dir, walk_opts, out);
}
