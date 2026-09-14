/* spdx_sbom_utils.h - вспомогательные функции SPDX SBOM */
#ifndef SPDX_SBOM_UTILS_H
#define SPDX_SBOM_UTILS_H

#include <stddef.h>
#include "spdx_sbom_types.h"

void sbom_remove_extension(char *str);
void sbom_sanitize_id(const char *src, char *dst, size_t dst_size);
void sbom_make_package_id(const char *base_name, const char *suffix,
                          char *buf, size_t buf_size);
char *sbom_json_escape(const char *src);
const char *sbom_get_file_type(const char *filename);

void filelist_init(FileList *list);
void filelist_add(FileList *list, const FileInfo *info);
void filelist_free(FileList *list);

void snippetlist_init(SnippetList *list);
SnippetInfo *snippetlist_add(SnippetList *list);
void snippetlist_free(SnippetList *list);

#endif /* SPDX_SBOM_UTILS_H */
