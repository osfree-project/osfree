/* spdx_sbom_out.c - диспетчер вывода SBOM (C89) */

#include <stdio.h>
#include <string.h>
#include "spdx_sbom_out.h"

int sbom_output(const SpdxDocument *doc, const char *format) {
    if (strcmp(format, "json") == 0)
        return sbom_output_json(doc);
    if (strcmp(format, "tagvalue") == 0 || strcmp(format, "tag") == 0)
        return sbom_output_tagvalue(doc);
    fprintf(stderr,
            "ERROR: unsupported output format: %s\n"
            "       Supported: json, tagvalue (or tag).\n"
            "       Run 'spdx-sbom --help' for usage.\n",
            format);
    return -1;
}
