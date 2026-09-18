/* reuseerr.h - REUSE error codes and diagnostic records
 * (C89 + Watcom extensions) */
#ifndef REUSEERR_H
#define REUSEERR_H

/**
 * @file reuseerr.h
 * @brief Error codes and the REUSEERR diagnostic record.
 *
 * Shared by reuse.h and reuse_toml.h. Every function in either module
 * may return one of the REUSE_ERROR_* codes. ReuseOpen additionally
 * stores a list of REUSEERR records in the project handle; the caller
 * retrieves them with ReuseGetError.
 */

#include "os2types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Return codes
 * ================================================================== */

/** @def REUSE_NO_ERROR @brief Success. */
#define REUSE_NO_ERROR                 0
/** @def REUSE_ERROR_INVALID_PARAM @brief Invalid parameter. */
#define REUSE_ERROR_INVALID_PARAM      1
/** @def REUSE_ERROR_OPEN_FAILED @brief File or directory cannot be opened. */
#define REUSE_ERROR_OPEN_FAILED        2
/** @def REUSE_ERROR_READ_FAILED @brief Read error. */
#define REUSE_ERROR_READ_FAILED        3
/** @def REUSE_ERROR_SYNTAX @brief Syntax error in input. */
#define REUSE_ERROR_SYNTAX             4
/** @def REUSE_ERROR_VERSION_MISSING @brief "version" key absent. */
#define REUSE_ERROR_VERSION_MISSING    5
/** @def REUSE_ERROR_VERSION_NOT_INT @brief "version" is not an integer. */
#define REUSE_ERROR_VERSION_NOT_INT    6
/** @def REUSE_ERROR_VERSION_UNSUP @brief "version" value is not 1. */
#define REUSE_ERROR_VERSION_UNSUP      7
/** @def REUSE_ERROR_ANNOT_NO_PATH @brief [[annotations]] lacks "path". */
#define REUSE_ERROR_ANNOT_NO_PATH      8
/** @def REUSE_ERROR_ANNOT_BAD_PATH @brief "path" has an unsupported type. */
#define REUSE_ERROR_ANNOT_BAD_PATH     9
/** @def REUSE_ERROR_ANNOT_BAD_FIELD @brief A field has an unsupported type. */
#define REUSE_ERROR_ANNOT_BAD_FIELD   10
/** @def REUSE_ERROR_BUFFER_OVERFLOW @brief Caller-supplied buffer too small. */
#define REUSE_ERROR_BUFFER_OVERFLOW   11
/** @def REUSE_ERROR_OUT_OF_MEMORY @brief Memory allocation failure. */
#define REUSE_ERROR_OUT_OF_MEMORY     12
/** @def REUSE_ERROR_INVALID_HANDLE @brief Invalid handle. */
#define REUSE_ERROR_INVALID_HANDLE    13
/** @def REUSE_ERROR_NOT_FOUND @brief Requested field or index absent. */
#define REUSE_ERROR_NOT_FOUND         14
/** @def REUSE_ERROR_INDEX_RANGE @brief Index out of range. */
#define REUSE_ERROR_INDEX_RANGE       15

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
 * @brief One diagnostic emitted during ReuseOpen or ReuseResolveFile.
 *
 * String fields are NUL-terminated and owned by the issuing handle
 * (project or file). They remain valid until the corresponding Close.
 * The caller must not free them.
 */
typedef struct _REUSEERR {
    ULONG   ulCode;         /* one of REUSE_ERROR_*                  */
    ULONG   ulSeverity;     /* one of REUSE_SEV_*                    */
    ULONG   ulLine;         /* source line, 0 if not applicable      */
    char    achFile[512];   /* offending file path, "" if none       */
    char    achDetail[256]; /* human-readable detail                 */
} REUSEERR, *PREUSEERR;

#ifdef __cplusplus
}
#endif

#endif /* REUSEERR_H */
