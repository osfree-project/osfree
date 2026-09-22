/* reuseerr.h - REUSE error codes and diagnostic records
 * (C89 + Watcom extensions) */
#ifndef REUSEERR_H
#define REUSEERR_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file reuseerr.h
 * @brief Error codes and the REUSEERR diagnostic record.
 *
 * Shared by reuse.h and reuse_toml.h. Every function in either
 * module returns APIRET. Codes specific to the REUSE Specification
 * are declared here, in the user range 0xFF00 - 0xFFFF as required
 * by the OS/2 Control Program Interface.
 */

/* ==================================================================
 * REUSE-specific return codes
 * ================================================================== */

/**
 * @def REUSE_ERROR_SYNTAX
 * @brief Syntax error in a REUSE.toml or DEP5 file.
 *        User range 0xFF01.
 */
#define REUSE_ERROR_SYNTAX             0xFF01

/**
 * @def REUSE_ERROR_VERSION_MISSING
 * @brief "version" key absent. User range 0xFF02.
 */
#define REUSE_ERROR_VERSION_MISSING    0xFF02

/**
 * @def REUSE_ERROR_VERSION_NOT_INT
 * @brief "version" is not an integer. User range 0xFF03.
 */
#define REUSE_ERROR_VERSION_NOT_INT    0xFF03

/**
 * @def REUSE_ERROR_VERSION_UNSUP
 * @brief "version" value is not supported. User range 0xFF04.
 */
#define REUSE_ERROR_VERSION_UNSUP      0xFF04

/**
 * @def REUSE_ERROR_ANNOT_NO_PATH
 * @brief [[annotations]] entry lacks the "path" key.
 *        User range 0xFF05.
 */
#define REUSE_ERROR_ANNOT_NO_PATH      0xFF05

/**
 * @def REUSE_ERROR_ANNOT_BAD_PATH
 * @brief "path" has an unsupported type. User range 0xFF06.
 */
#define REUSE_ERROR_ANNOT_BAD_PATH     0xFF06

/**
 * @def REUSE_ERROR_ANNOT_BAD_FIELD
 * @brief A field in an [[annotations]] entry has an unsupported
 *        type. User range 0xFF07.
 */
#define REUSE_ERROR_ANNOT_BAD_FIELD    0xFF07

/* ==================================================================
 * LICENSES/ diagnostic codes (used by reuse_licenses.h)
 * ================================================================== */

/**
 * @def REUSE_LICENSES_DIR_MISSING
 * @brief LICENSES/ directory does not exist. User range 0xFF20.
 */
#define REUSE_LICENSES_DIR_MISSING       0xFF20

/**
 * @def REUSE_LICENSES_DIR_CREATED
 * @brief LICENSES/ directory was created. User range 0xFF21.
 */
#define REUSE_LICENSES_DIR_CREATED       0xFF21

/**
 * @def REUSE_LICENSES_BAD_NAME
 * @brief File name is not a valid SPDX identifier. User range 0xFF22.
 */
#define REUSE_LICENSES_BAD_NAME          0xFF22

/**
 * @def REUSE_LICENSES_UNUSED_FILE
 * @brief File in LICENSES/ matches no used license.
 *        User range 0xFF23.
 */
#define REUSE_LICENSES_UNUSED_FILE       0xFF23

/**
 * @def REUSE_LICENSES_MISSING_FILE
 * @brief Used license has no file in LICENSES/. User range 0xFF24.
 */
#define REUSE_LICENSES_MISSING_FILE      0xFF24

/**
 * @def REUSE_LICENSES_DEPRECATED_ID
 * @brief Identifier is marked deprecated by SPDX.
 *        User range 0xFF25.
 */
#define REUSE_LICENSES_DEPRECATED_ID     0xFF25

/**
 * @def REUSE_LICENSES_NO_EXTENSION
 * @brief License file has no extension. User range 0xFF26.
 */
#define REUSE_LICENSES_NO_EXTENSION      0xFF26

/**
 * @def REUSE_LICENSES_TEXT_MISMATCH
 * @brief License text differs from the SPDX database.
 *        User range 0xFF27.
 */
#define REUSE_LICENSES_TEXT_MISMATCH     0xFF27

/**
 * @def REUSE_LICENSES_FILE_CREATED
 * @brief License file was created. User range 0xFF28.
 */
#define REUSE_LICENSES_FILE_CREATED      0xFF28

/**
 * @def REUSE_LICENSES_FILE_UPDATED
 * @brief License file was overwritten. User range 0xFF29.
 */
#define REUSE_LICENSES_FILE_UPDATED      0xFF29

/**
 * @def REUSE_LICENSES_FILE_UP_TO_DATE
 * @brief License file matches the database. User range 0xFF2A.
 */
#define REUSE_LICENSES_FILE_UP_TO_DATE   0xFF2A

/**
 * @def REUSE_LICENSES_FILE_OUTDATED
 * @brief License file differs; --force needed. User range 0xFF2B.
 */
#define REUSE_LICENSES_FILE_OUTDATED     0xFF2B

/**
 * @def REUSE_LICENSES_MANUAL_REQUIRED
 * @brief LicenseRef-* has no text in the database.
 *        User range 0xFF2C.
 */
#define REUSE_LICENSES_MANUAL_REQUIRED   0xFF2C

/**
 * @def REUSE_LICENSES_NO_DB_TEXT
 * @brief No text for the identifier in the SPDX database.
 *        User range 0xFF2D.
 */
#define REUSE_LICENSES_NO_DB_TEXT        0xFF2D

#define REUSE_LICENSES_BAD_ID            0xFF2E

/* ==================================================================
 * Severity
 * ================================================================== */

/**
 * @def REUSE_SEV_ERROR
 * @brief Violation of a MUST requirement of the REUSE Specification.
 */
#define REUSE_SEV_ERROR    1

/**
 * @def REUSE_SEV_WARNING
 * @brief Violation of a SHOULD requirement, or a condition that
 *        prevents a clean result without being a MUST violation.
 */
#define REUSE_SEV_WARNING  2

/**
 * @def REUSE_SEV_INFO
 * @brief Purely informational. No requirement is violated.
 */
#define REUSE_SEV_INFO     3

/* ==================================================================
 * Diagnostic record
 * ================================================================== */

/**
 * @struct _REUSEERR
 * @brief One diagnostic emitted during ReuseOpen, ReuseResolveFile,
 *        or LICENSES/ validation.
 *
 * String fields are NUL-terminated and owned by the issuing handle
 * (project, file, or licenses report). They remain valid until the
 * corresponding Close. The caller must not free them.
 */
typedef struct _REUSEERR {
    ULONG ulCode;         /**< One of REUSE_ERROR_*, REUSE_LICENSES_*,
                               SPDXDB_*, or ERROR_*.              */
    ULONG ulSeverity;     /**< One of REUSE_SEV_*.                */
    ULONG ulLine;         /**< Source line, 0 if not applicable.  */
    CHAR  achFile[512];   /**< Offending file path, "" if none.   */
    CHAR  achDetail[256]; /**< Human-readable detail.             */
} REUSEERR, *PREUSEERR;

#ifdef __cplusplus
}
#endif

#endif /* REUSEERR_H */
