/* spdx_sbom_out.h - SBOM output dispatcher (C89) */
#ifndef SPDX_SBOM_OUT_H
#define SPDX_SBOM_OUT_H

#include "os2types.h"
#include "os2err.h"
#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_out.h
 * @brief Output dispatcher and serializers for the SBOM generator.
 *
 * The dispatcher selects a serializer by name. Public format names
 * are "spdx-json" for SPDX 2.3 JSON and "spdx-tag" for SPDX 2.3
 * tag-value.
 *
 * @par Output stream
 * Both serializers write the finished document to stdout. This is
 * their functional output, not diagnostics; no other code path in
 * this module writes to stdout or stderr.
 *
 * Conforms to:
 *   - SPDX 2.3.
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - OS/2 Control Program Interface (naming, types, conventions).
 */

/* ==================================================================
 * Dispatcher
 * ================================================================== */

/**
 * @brief Emit a document in the requested format.
 *
 * @param[in] pDoc       Document to serialize. Not NULL.
 * @param[in] pszFormat  Format name. Not NULL. Accepted values:
 *                       "spdx-json" and "spdx-tag".
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc or pszFormat is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_INVALID_DATA       Unknown format name.
 */
APIRET APIENTRY SbomOutput(const SPDXDOCUMENT *pDoc, PCSZ pszFormat);

/* ==================================================================
 * Serializers
 * ================================================================== */

/**
 * @brief Serialize a document as SPDX 2.3 JSON to stdout.
 *
 * Builds the JSON tree with the json module and writes it to
 * stdout. All string escaping and number formatting is handled by
 * the json module.
 *
 * @param[in] pDoc  Document to serialize. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Write error.
 */
APIRET APIENTRY SbomOutputJson(const SPDXDOCUMENT *pDoc);

/**
 * @brief Serialize a document as SPDX 2.3 tag-value to stdout.
 *
 * Multi-line values are wrapped in <text>...</text> per SPDX 2.3.
 *
 * @param[in] pDoc  Document to serialize. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     An internal container is not
 *                                  recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomOutputTagValue(const SPDXDOCUMENT *pDoc);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_OUT_H */
