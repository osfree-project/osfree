/* osadl.h - OSADL license compatibility matrix reader (C89) */
#ifndef OSADL_H
#define OSADL_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file osadl.h
 * @brief Reader for the OSADL "matrix.json" license compatibility
 *        matrix.
 *
 * Layer L2.0a. Depends only on os2types.h, os2err.h and json.h.
 * Does not know REUSE, SPDX, precedence, or comment markers.
 *
 * Data source:
 *   https://www.osadl.org/fileadmin/checklists/matrix.json
 *
 * Status of data: OSADL publishes the matrix as draft material,
 * "as is", without warranty. Data licence: CC-BY-4.0. The snapshot
 * carries a "timestamp" field; callers must pin that snapshot
 * (ISO/IEC 5230:2020 §3.3.2.1 reproducibility requirement).
 *
 * File structure (verified against snapshot 2026-09-30):
 *   - Flat JSON object.
 *   - Two reserved keys: "timeformat", "timestamp".
 *   - Every other key is an outbound SPDX identifier. Value is an
 *     object {inbound: verdict}.
 *   - Outbound keys may include "A WITH B" forms and
 *     "LicenseRef-scancode-*" identifiers.
 *   - File invariant: every outbound key must appear as an inbound
 *     key in every row. Additional inbound keys are allowed
 *     (e.g. "GPL-2.0-only WITH Classpath-exception-2.0" is inbound
 *     but not outbound). Violation -> OSADL_ERROR_MISSING_KEY.
 */

/* ==================================================================
 * Verdicts and error codes
 * ================================================================== */

/**
 * @enum _OSADLCOMPAT
 * @brief Verdict for a (outbound, inbound) license pair.
 *
 * Values map 1:1 to the five strings that actually occur in
 * matrix.json. The absence of a pair is reported as
 * OSADL_COMPAT_UNDEF; that string does not occur in the file.
 */
typedef enum _OSADLCOMPAT {
    OSADL_COMPAT_SAME      = 0, /**< "Same"             */
    OSADL_COMPAT_YES       = 1, /**< "Yes"              */
    OSADL_COMPAT_NO        = 2, /**< "No"               */
    OSADL_COMPAT_CHECK_DEP = 3, /**< "Check dependency" */
    OSADL_COMPAT_UNKNOWN   = 4, /**< "Unknown"          */
    OSADL_COMPAT_UNDEF     = 5  /**< pair not present   */
} OSADLCOMPAT, *POSADLCOMPAT;

/** @brief Matrix file is not valid JSON or has unexpected shape. */
#define OSADL_ERROR_SYNTAX       0xFF01

/** @brief Matrix file lacks the mandatory "timestamp" field. */
#define OSADL_ERROR_VERSION      0xFF02

/** @brief File invariant violated: some outbound key is missing
 *         from at least one row's inbound keys. */
#define OSADL_ERROR_MISSING_KEY  0xFF03

/* ==================================================================
 * Handle
 * ================================================================== */

/**
 * @typedef HOSADLMATRIX
 * @brief Handle to an opened OSADL matrix.
 *
 * Owned by the caller. Must be released with OsadlCloseMatrix.
 * NULLHANDLE is the "no matrix" value. A non-null handle carries
 * an internal magic; passing a stale or foreign handle to any
 * Osadl* function yields ERROR_INVALID_HANDLE.
 */
typedef HANDLE HOSADLMATRIX;

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Open and parse an OSADL matrix.json file.
 *
 * The file is read fully into memory and parsed; the in-memory
 * representation is kept in the handle until OsadlCloseMatrix.
 *
 * Enforces the file invariant: every outbound key must appear as
 * an inbound key in every row. Additional inbound-only keys
 * (e.g. "GPL-2.0-only WITH Classpath-exception-2.0") are allowed.
 *
 * On success @p *phMatrix receives a valid handle; on error it is
 * set to NULLHANDLE.
 *
 * @param[in]  pszPath   Path to matrix.json. Not NULL.
 * @param[out] phMatrix  Handle receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or phMatrix is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval OSADL_ERROR_SYNTAX       Malformed JSON, unexpected shape,
 *                                  or unknown verdict string.
 * @retval OSADL_ERROR_VERSION      "timestamp" field missing.
 * @retval OSADL_ERROR_MISSING_KEY  File invariant violated.
 */
APIRET APIENTRY OsadlOpenMatrix(PCSZ pszPath, HOSADLMATRIX *phMatrix);

/**
 * @brief Release an OSADL matrix.
 *
 * Passing NULLHANDLE is a no-op. A non-null handle whose magic does
 * not match is rejected with ERROR_INVALID_HANDLE. The magic is
 * cleared before free so that a second close of the same pointer is
 * detected as ERROR_INVALID_HANDLE (best effort: double-close
 * remains undefined per the owned-handle contract).
 *
 * @param[in] hMatrix  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not an OSADL matrix or
 *                                was already closed.
 */
APIRET APIENTRY OsadlCloseMatrix(HOSADLMATRIX hMatrix);

/* ==================================================================
 * Lookup
 * ================================================================== */

/**
 * @brief Look up a (outbound, inbound) license pair.
 *
 * Both identifiers are matched byte-for-byte against the strings in
 * the file. "A WITH B" forms are distinct keys. No SPDX
 * canonicalization is performed.
 *
 * If either identifier is not present in the matrix, the result is
 * OSADL_COMPAT_UNDEF; that is a normal outcome, not an error.
 *
 * @param[in]  hMatrix     Handle. Not NULLHANDLE.
 * @param[in]  pszOutbound Outbound license identifier. Not NULL.
 * @param[in]  pszInbound  Inbound license identifier. Not NULL.
 * @param[out] pCompat     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pszOutbound, pszInbound, or
 *                                  pCompat is NULL.
 */
APIRET APIENTRY OsadlQueryPair(HOSADLMATRIX hMatrix,
                               PCSZ pszOutbound,
                               PCSZ pszInbound,
                               POSADLCOMPAT pCompat);

/**
 * @brief Test whether a license identifier occurs as an outbound
 *        key in the matrix.
 *
 * The identifier is matched byte-for-byte, without canonicalization.
 * "A WITH B" forms are distinct keys.
 *
 * @param[in]  hMatrix   Handle. Not NULLHANDLE.
 * @param[in]  pszId     License identifier. Not NULL.
 * @param[out] pfPresent Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfPresent is NULL.
 */
APIRET APIENTRY OsadlQueryHasLicense(HOSADLMATRIX hMatrix,
                                     PCSZ pszId,
                                     PBOOL pfPresent);

/* ==================================================================
 * Introspection
 * ================================================================== */

/**
 * @brief Query the number of outbound licenses in the matrix.
 *
 * @param[in]  hMatrix   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY OsadlQueryLicenseCount(HOSADLMATRIX hMatrix,
                                       PULONG pulCount);

/**
 * @brief Query the outbound license identifier at a given index.
 *
 * Indices are stable for the lifetime of the handle and enumerate
 * the outbound keys in sorted order. They are unrelated to the
 * order in which the keys appeared in the source file.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written
 *     (required size including NUL).
 *   - ulSize large enough: value copied and NUL-terminated;
 *     *pulUsed is the length excluding NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  hMatrix   Handle. Not NULLHANDLE.
 * @param[in]  ulIndex   Index. Must be < OsadlQueryLicenseCount.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf.
 * @param[out] pulUsed   Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pulUsed is NULL, or ulIndex is
 *                                  out of range, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY OsadlQueryLicenseName(HOSADLMATRIX hMatrix,
                                      ULONG ulIndex,
                                      PSZ pszBuf, ULONG ulSize,
                                      PULONG pulUsed);

/**
 * @brief Query the snapshot timestamp from the source file.
 *
 * Returns the value of the "timestamp" field, as stored in the
 * file (format per "timeformat", normally ISO-8601). Use this
 * value for version pinning in downstream reports.
 *
 * Size-query convention as for OsadlQueryLicenseName.
 *
 * @param[in]  hMatrix  Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pulUsed is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY OsadlQueryTimestamp(HOSADLMATRIX hMatrix,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed);

#ifdef __cplusplus
}
#endif

#endif /* OSADL_H */
