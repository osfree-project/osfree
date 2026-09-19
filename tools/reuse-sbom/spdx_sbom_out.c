/* spdx_sbom_out.c - SBOM output dispatcher (C89) */

#include <stdio.h>
#include <string.h>
#include "spdx_sbom_out.h"

/**
 * @file spdx_sbom_out.c
 * @brief Output dispatcher for the SBOM generator.
 *
 * Selects the serializer by the format name accepted by
 * spdx_sbom_opts.c: "spdx-json" for SPDX 2.3 JSON, "spdx-tag" for
 * SPDX 2.3 tag-value.
 */

/**
 * @brief Emit a document in the requested format.
 *
 * @param[in] doc     Document to serialize. Not NULL.
 * @param[in] format  Format name. Not NULL. Accepted values:
 *                    "spdx-json", "spdx-tag".
 *
 * @return 0 on success, -1 if the format is unknown.
 */
int sbom_output(const SpdxDocument *doc, const char *format) {
    if (strcmp(format, "spdx-json") == 0)
        return sbom_output_json(doc);
    if (strcmp(format, "spdx-tag") == 0)
        return sbom_output_tagvalue(doc);
    fprintf(stderr,
            "ERROR: unsupported output format: %s\n"
            "       Supported: spdx-json, spdx-tag.\n"
            "       Run 'reuse-sbom --help' for usage.\n",
            format);
    return -1;
}
