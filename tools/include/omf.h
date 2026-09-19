/* omf.h - extract source file names from OMF object files (C89) */
#ifndef OMF_H
#define OMF_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file omf.h
 * @brief Extract source file names from OMF object files.
 *
 * OMF (Relocatable Object Module Format) records scanned for source
 * names:
 *   - THEADR (0x80) — module name, usually the original source file.
 *   - COMENT (0x88) — comment records with vendor-specific classes
 *     carrying dependency file lists.
 *
 * Vendor-specific COMENT classes recognized:
 *   - 0xE9 — Borland auto-dependency: timestamp (4 bytes) followed
 *     by repeated [len][name] entries.
 *   - 0xFB — OpenWatcom auto-dependency: same layout as 0xE9.
 *   - 0x88 — additional dependency class found in OpenWatcom output.
 *   - 0xE8 — Borland source file name: single [len][name] entry.
 *
 * References:
 *   - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *     Object Module Format), Linux Foundation.
 *     https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *   - JWasm / OpenWatcom WASM sources (COMENT class usage).
 *
 */

/**
 * @brief Extract source file names from an OMF object file.
 *
 * Scans the object file for THEADR and dependency COMENT records,
 * and appends every unique name found to @p hOut.
 *
 * @param[in] pszPath  Path to the OMF object file. Not NULL.
 * @param[in] hOut     Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or hOut is
 *                                  NULLHANDLE.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_FILE_NOT_FOUND     No source names found.
 *
 * @note The caller owns @p hOut and must release it with
 *       StrSetDestroy.
 */
APIRET APIENTRY OmfExtractSources(PCSZ pszPath, HSTRSET hOut);

#ifdef __cplusplus
}
#endif

#endif /* OMF_H */
