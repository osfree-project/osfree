/* spdx_discover.c - единый модуль определения списка файлов (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spdx_discover.h"
#include "spdx_utils.h"
#include "git_utils.h"
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

/* ------------------------------------------------------------------ */
/* Опции                                                               */
/* ------------------------------------------------------------------ */

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
    opts->use_gitignore         = 0;
    opts->repo_root             = NULL;
    opts->gitignore_rules       = NULL;
}

/* ------------------------------------------------------------------ */
/* Служебные проверки имён                                             */
/* ------------------------------------------------------------------ */

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

/* SPDX-документы: *.spdx, *.spdx.json, *.spdx.yaml, *.spdx.yml,
 * *.spdx.rdf, *.spdx.xml */
static int is_spdx_document(const char *name) {
    size_t len = strlen(name);
    static const char *suffixes[] = {
        ".spdx", ".spdx.json", ".spdx.yaml", ".spdx.yml",
        ".spdx.rdf", ".spdx.xml"
    };
    size_t i;
    for (i = 0; i < sizeof(suffixes)/sizeof(suffixes[0]); i++) {
        size_t sl = strlen(suffixes[i]);
        if (len >= sl && strcmp(name + len - sl, suffixes[i]) == 0)
            return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Git-фильтрация                                                      */
/* ------------------------------------------------------------------ */

/* Нормализует путь к виду с '/'. Возвращает malloc-строку. */
static char *to_slash(const char *path) {
    size_t i;
    size_t n = strlen(path);
    char *r = (char*)malloc(n + 1);
    if (!r) return NULL;
    for (i = 0; i < n; i++)
        r[i] = (path[i] == '\\') ? '/' : path[i];
    r[n] = '\0';
    return r;
}

/* Возвращает относительный путь от root до full (с '/'),
 * либо NULL, если full не под root. Caller free. */
static char *rel_path(const char *root, const char *full) {
    size_t rlen, flen;
    char *rroot, *rfull, *result;
    if (!root || !full) return NULL;
    rroot = to_slash(root);
    rfull = to_slash(full);
    if (!rroot || !rfull) {
        free(rroot);
        free(rfull);
        return NULL;
    }
    rlen = strlen(rroot);
    flen = strlen(rfull);
    if (flen < rlen || strncmp(rroot, rfull, rlen) != 0) {
        free(rroot);
        free(rfull);
        return NULL;
    }
    if (flen == rlen) {
        free(rroot);
        free(rfull);
        return strdup("");
    }
    if (rfull[rlen] != '/') {
        free(rroot);
        free(rfull);
        return NULL;
    }
    result = strdup(rfull + rlen + 1);
    free(rroot);
    free(rfull);
    return result;
}

static int should_skip_by_gitignore(const SpdxWalkOptions *opts,
                                    const char *fullpath, int is_dir) {
    char *rel;
    int ignored;

    if (!opts->use_gitignore) return 0;
    if (!opts->gitignore_rules || opts->gitignore_rules->count == 0) return 0;
    if (!opts->repo_root) return 0;

    rel = rel_path(opts->repo_root, fullpath);
    if (!rel) return 0;
    ignored = git_is_ignored(opts->gitignore_rules, rel, is_dir);
    free(rel);
    return ignored;
}

/* ------------------------------------------------------------------ */
/* Прочие фильтры                                                      */
/* ------------------------------------------------------------------ */

static int should_skip_file(const char *name, const SpdxWalkOptions *opts) {
    if (opts->skip_hidden && is_hidden(name)) return 1;
    if (opts->skip_license_sidecars && is_license_sidecar(name)) return 1;
    if (opts->skip_reuse_toml && is_reuse_toml(name)) return 1;
    if (opts->skip_license_files && is_license_file(name)) return 1;
    if (is_spdx_document(name)) return 1;
    return 0;
}

static int should_skip_dir(const char *name, const SpdxWalkOptions *opts) {
    if (opts->skip_hidden && is_hidden(name)) return 1;
    if (opts->skip_vcs_dirs && is_vcs_dir(name)) return 1;
    if (opts->skip_licenses_dir && is_licenses_dir(name)) return 1;
    if (opts->skip_reuse_dir && is_reuse_dir(name)) return 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Нормализация пути и склейка                                         */
/* ------------------------------------------------------------------ */

static void normalize_dir(const char *src, char *dst, size_t dst_size) {
    size_t len = strlen(src);
#ifdef _WIN32
    if (len >= 2 && src[1] == ':' &&
        (len == 2 || (len == 3 && (src[2] == '\\' || src[2] == '/')))) {
        strncpy(dst, src, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
#endif
    while (len > 0 && (src[len-1] == '/' || src[len-1] == '\\'))
        len--;
    if (len >= dst_size) len = dst_size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void join_path(char *dst, size_t dst_size,
                      const char *dir, const char *name) {
#ifdef __LINUX__
    snprintf(dst, dst_size, "%s/%s", dir, name);
#else
    snprintf(dst, dst_size, "%s\\%s", dir, name);
#endif
}

/* ------------------------------------------------------------------ */
/* Обход дерева                                                        */
/* ------------------------------------------------------------------ */

#ifdef __LINUX__

static int walk_inner(const char *dir_in, const SpdxWalkOptions *opts,
                      SpdxStrList *out) {
    char dir[1024];
    DIR *d;
    struct dirent *entry;
    struct stat st;
    char full[1024];

    normalize_dir(dir_in, dir, sizeof(dir));
    d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Error: cannot open directory %s\n", dir);
        return -1;
    }
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) continue;

        join_path(full, sizeof(full), dir, entry->d_name);
        if (stat(full, &st) != 0) continue;

        /* Симлинки игнорируем */
        if (S_ISLNK(st.st_mode)) continue;

        if (S_ISDIR(st.st_mode)) {
            if (!opts->recursive) continue;
            if (should_skip_dir(entry->d_name, opts)) continue;
            if (should_skip_by_gitignore(opts, full, 1)) continue;
            if (walk_inner(full, opts, out) != 0) {
                closedir(d);
                return -1;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (st.st_size == 0) continue;   /* пустые игнорируем */
            if (should_skip_file(entry->d_name, opts)) continue;
            if (should_skip_by_gitignore(opts, full, 0)) continue;
            spdx_strlist_add_unique(out, full);
        }
    }
    closedir(d);
    return 0;
}

#else

static int walk_inner(const char *dir_in, const SpdxWalkOptions *opts,
                      SpdxStrList *out) {
    char dir[1024];
    long hFile;
    struct _finddata_t fd;
    struct stat st;
    char pattern[1100];
    char full[1024];

    normalize_dir(dir_in, dir, sizeof(dir));
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
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
            if (should_skip_by_gitignore(opts, full, 1)) continue;
            if (walk_inner(full, opts, out) != 0) {
                _findclose(hFile);
                return -1;
            }
        } else if (st.st_mode & _S_IFREG) {
            if (st.st_size == 0) continue;
            if (should_skip_file(fd.name, opts)) continue;
            if (should_skip_by_gitignore(opts, full, 0)) continue;
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

/* ------------------------------------------------------------------ */
/* Артефакты сборки                                                    */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */
/* Оркестратор                                                         */
/* ------------------------------------------------------------------ */

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
