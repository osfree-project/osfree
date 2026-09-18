/* spdx_sbom_scan.h - file and snippet collection for SBOM (C89) */
#ifndef SPDX_SBOM_SCAN_H
#define SPDX_SBOM_SCAN_H

#include "spdx_sbom_types.h"
#include "spdx_utils.h"
#include <reuse.h>

int sbom_collect_files(const SpdxStrList *paths,
                       HREUSETREE hTree,
                       const char *default_license,
                       const char *default_copyright,
                       FileList *out,
                       SnippetList *snippets);

int sbom_fill_file_basic(const char *fullpath,
                         const char *display_name,
                         FileInfo *out);

#endif /* SPDX_SBOM_SCAN_H */
