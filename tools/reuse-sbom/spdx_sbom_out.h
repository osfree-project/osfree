/* spdx_sbom_out.h - SBOM output dispatcher (C89) */
#ifndef SPDX_SBOM_OUT_H
#define SPDX_SBOM_OUT_H

#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_out.h
 * @brief Output dispatcher and serializers for the SBOM generator.
 *
 * The dispatcher selects a serializer by the format name accepted
 * by spdx_sbom_opts.c: "spdx-json" for SPDX 2.3 JSON, "spdx-tag"
 * for SPDX 2.3 tag-value.
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
int sbom_output(const SpdxDocument *doc, const char *format);

/**
 * @brief Serialize a document as SPDX 2.3 JSON to stdout.
 *
 * @param[in] doc  Document to serialize. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
int sbom_output_json(const SpdxDocument *doc);

/**
 * @brief Serialize a document as SPDX 2.3 tag-value to stdout.
 *
 * @param[in] doc  Document to serialize. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
int sbom_output_tagvalue(const SpdxDocument *doc);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_OUT_H */
