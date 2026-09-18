/* spdx_sbom_out.h - SBOM output dispatcher (C89) */
#ifndef SPDX_SBOM_OUT_H
#define SPDX_SBOM_OUT_H

#include "spdx_sbom_types.h"

/* Emits the document in the given format ("json", "tagvalue"/"tag").
 * Returns 0 on success, -1 on unknown format. */
int sbom_output(const SpdxDocument *doc, const char *format);

int sbom_output_json(const SpdxDocument *doc);
int sbom_output_tagvalue(const SpdxDocument *doc);

#endif /* SPDX_SBOM_OUT_H */
