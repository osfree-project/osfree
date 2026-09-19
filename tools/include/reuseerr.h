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
 * module returns APIRET. Common failures use the standard OS/2
 * codes from os2err.h. Codes specific to the REUSE Specification
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
 * @brief One diagnostic emitted during ReuseOpen or
 *        ReuseResolveFile.
 *
 * String fields are NUL-terminated and owned by the issuing handle
 * (project or file). They remain valid until the corresponding
 * Close. The caller must not free them.
 */
typedef struct _REUSEERR {
    ULONG ulCode;         /**< One of REUSE_ERROR_* or ERROR_*.   */
    ULONG ulSeverity;     /**< One of REUSE_SEV_*.                */
    ULONG ulLine;         /**< Source line, 0 if not applicable.  */
    CHAR  achFile[512];   /**< Offending file path, "" if none.   */
    CHAR  achDetail[256]; /**< Human-readable detail.             */
} REUSEERR, *PREUSEERR;

#ifdef __cplusplus
}
#endif

#endif /* REUSEERR_H */
