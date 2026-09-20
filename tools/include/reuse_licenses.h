/* reuse_licenses.h - LICENSES/ directory handling, REUSE 3.3
 * (C89 + Watcom extensions) */
#ifndef REUSE_LICENSES_H
#define REUSE_LICENSES_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "reuse.h"
#include "reuseerr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuse_licenses.h
 * @brief LICENSES/ directory handling.
 *
 * Implements the LICENSES/ requirements of the REUSE Specification
 * 3.3 §3.2:
 *   - a LICENSES/ directory MUST exist at the project root;
 *   - it MUST contain the full text of every license declared by
 *     any Covered File;
 *   - its file names MUST be SPDX identifiers or LicenseRef-*;
 *   - files for licenses not used by any Covered File MUST NOT be
 *     present;
 *   - no other files are allowed.
 *
 * Validation and creation are separate operations. Validation is
 * used by reuse-lint, creation and update by reuse-annotate. Both
 * accumulate diagnostics in a report handle; the module itself
 * never writes to the console. Diagnostics use the shared REUSEERR
 * record.
 *
 * The SPDX database must already be open (SpdxOpenDatabase) when
 * either function is called. The module does not open or close it.
 *
 * Conforms to:
 *   - REUSE Specification 3.3, §3.2.
 *     https://reuse.software/spec-3.3/
 */

/* ==================================================================
 * Report handle
 * ================================================================== */

/**
 * @typedef HREUSELICENSEREPORT
 * @brief Handle to an accumulated diagnostic report.
 */
typedef HANDLE HREUSELICENSEREPORT;

/**
 * @typedef PHREUSELICENSEREPORT
 * @brief Pointer to a report handle.
 */
typedef HREUSELICENSEREPORT *PHREUSELICENSEREPORT;

/* ==================================================================
 * Report lifecycle
 * ================================================================== */

/**
 * @brief Create an empty report.
 *
 * @param[out] phReport  Receiver. Not NULL. Set to NULLHANDLE on
 *                       error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseLicensesReportCreate(PHREUSELICENSEREPORT phReport);

/**
 * @brief Release a report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hReport is not recognized.
 */
APIRET APIENTRY ReuseLicensesReportFree(HREUSELICENSEREPORT hReport);

/**
 * @brief Number of records in the report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 */
APIRET APIENTRY ReuseLicensesReportGetCount(HREUSELICENSEREPORT hReport,
                                            PULONG pulCount);

/**
 * @brief Retrieve one record by index.
 *
 * The returned REUSEERR is a snapshot; string fields remain valid
 * until the report is freed.
 *
 * @param[in]  hReport  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pErr is NULL.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 */
APIRET APIENTRY ReuseLicensesReportGet(HREUSELICENSEREPORT hReport,
                                       ULONG ulIndex,
                                       PREUSEERR pErr);

/* ==================================================================
 * Validation
 * ================================================================== */

/**
 * @brief Validate the LICENSES/ directory of a project.
 *
 * Checks:
 *   - the directory exists;
 *   - every file name is a valid SPDX identifier or LicenseRef-*;
 *   - every used license has a corresponding file;
 *   - every file in LICENSES/ corresponds to a used license;
 *   - each file's text matches the SPDX License List.
 *
 * Diagnostics are appended to @p hReport. If @p hReport is
 * NULLHANDLE, the checks still run but no diagnostic is collected.
 *
 * @param[in] hTree          Project handle. Not NULLHANDLE.
 * @param[in] hUsedLicenses  Set of used SPDX identifiers. Not
 *                           NULLHANDLE.
 * @param[in] hReport        Report handle, or NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Validation completed.
 * @retval ERROR_INVALID_PARAMETER  hTree or hUsedLicenses is
 *                                  NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     A handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseLicensesValidate(HREUSETREE hTree,
                                      HSTRSET hUsedLicenses,
                                      HREUSELICENSEREPORT hReport);

/* ==================================================================
 * Creation and update
 * ================================================================== */

/**
 * @brief Create or update the LICENSES/ directory of a project.
 *
 * For every used SPDX identifier (except LicenseRef-* and
 * DocumentRef-*):
 *   - if LICENSES/<id>.txt does not exist, it is created with the
 *     text from the SPDX database;
 *   - if it exists but its text differs from the database, it is
 *     overwritten only when @p fForce is TRUE_;
 *   - if the text matches, nothing is done.
 *
 * The LICENSES/ directory itself is created if missing.
 *
 * Diagnostics are appended to @p hReport. If @p hReport is
 * NULLHANDLE, no diagnostic is collected.
 *
 * In dry-run mode (@p fDryRun TRUE_), no file is written.
 *
 * @param[in] hTree          Project handle. Not NULLHANDLE.
 * @param[in] hUsedLicenses  Set of used SPDX identifiers. Not
 *                           NULLHANDLE.
 * @param[in] fForce         TRUE_ to overwrite outdated files.
 * @param[in] fDryRun        TRUE_ to skip writes.
 * @param[in] hReport        Report handle, or NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Completed.
 * @retval ERROR_INVALID_PARAMETER  hTree or hUsedLicenses is
 *                                  NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     A handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseLicensesEnsure(HREUSETREE hTree,
                                    HSTRSET hUsedLicenses,
                                    BOOL fForce,
                                    BOOL fDryRun,
                                    HREUSELICENSEREPORT hReport);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_LICENSES_H */
