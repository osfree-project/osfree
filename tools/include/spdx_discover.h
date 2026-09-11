/* spdx_discover.h - единый модуль определения списка файлов (C89) */
#ifndef SPDX_DISCOVER_H
#define SPDX_DISCOVER_H

#include "spdx_utils.h"
#include "git_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int recursive;
    int skip_hidden;
    int skip_vcs_dirs;
    int skip_licenses_dir;
    int skip_reuse_dir;
    int skip_license_sidecars;
    int skip_reuse_toml;
    int skip_license_files;

    /* Git-фильтрация */
    int use_gitignore;                 /* 1 — применять правила */
    const char *repo_root;             /* корень репозитория для вычисления rel */
    const GitIgnoreList *gitignore_rules;
} SpdxWalkOptions;

void spdx_walk_options_default(SpdxWalkOptions *opts);

int spdx_walk_tree(const char *dir, const SpdxWalkOptions *opts,
                   SpdxStrList *out);

int spdx_discover_from_artifacts(char **object_files, int object_count,
                                 char **res_files, int res_count,
                                 SpdxStrList *out);

int spdx_discover(const char *project_dir,
                  char **object_files, int object_count,
                  char **res_files, int res_count,
                  const SpdxWalkOptions *walk_opts,
                  SpdxStrList *out);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_DISCOVER_H */
