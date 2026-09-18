/* spdx_sbom_scan.h - сбор списка файлов и сниппетов для SBOM (C89) */
#ifndef SPDX_SBOM_SCAN_H
#define SPDX_SBOM_SCAN_H

#include "spdx_sbom_types.h"
#include "spdx_utils.h"
#include <reuse.h>

int sbom_collect_files(const SpdxStrList *paths,
                       ReuseConfig **configs, int config_count,
                       const char *default_license,
                       const char *default_copyright,
                       FileList *out,
                       SnippetList *snippets);

int sbom_fill_file_basic(const char *fullpath,
                         const char *display_name,
                         FileInfo *out);

#endif /* SPDX_SBOM_SCAN_H */
