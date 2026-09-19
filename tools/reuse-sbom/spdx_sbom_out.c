/* spdx_sbom_out.c - SBOM output dispatcher (C89) */

#include <string.h>
#include "spdx_sbom_out.h"

/**
 * @file spdx_sbom_out.c
 * @brief Implementation of the SBOM output dispatcher.
 */

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
APIRET APIENTRY SbomOutput(const SPDXDOCUMENT *pDoc, PCSZ pszFormat) {
    if (!pDoc || !pszFormat) return ERROR_INVALID_PARAMETER;

    if (strcmp(pszFormat, "spdx-json") == 0)
        return SbomOutputJson(pDoc);
    if (strcmp(pszFormat, "spdx-tag") == 0)
        return SbomOutputTagValue(pDoc);

    return ERROR_INVALID_DATA;
}