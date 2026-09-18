/* reuse_toml.h - REUSE.toml parser, OS/2 API style
 * (C89 + Watcom extensions) */
#ifndef REUSE_TOML_H
#define REUSE_TOML_H

/**
 * @file reuse_toml.h
 * @brief Public interface of the REUSE.toml parser.
 *
 * Parses a single REUSE.toml file according to REUSE Specification 3.3:
 *   - https://reuse.software/spec-3.3/
 *
 * The parser owns its internal buffers, allocates them in ReuseTomlOpen
 * and releases them in ReuseTomlClose. The consumer supplies buffers
 * only for the data being returned.
 */

#include "os2types.h"
#include "reusedef.h"
#include "reuseerr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================================================================
 * Handles
 * ================================================================== */

/**
 * @typedef HREUSETOML
 * @brief Handle to an opened REUSE.toml document.
 *
 * Issued by ReuseTomlOpen, released by ReuseTomlClose. All internal
 * buffers are owned by the document and released on close.
 */
typedef HANDLE HREUSETOML;

/**
 * @typedef HREUSEANN
 * @brief Handle to a single annotation inside a document.
 *
 * Borrowed handle: valid until the owning document is closed. Does
 * not need to be released separately.
 */
typedef HANDLE HREUSEANN;

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/**
 * @brief Open and parse a REUSE.toml file.
 *
 * Reads the file, verifies the mandatory "version = 1" key
 * (REUSE 3.3 §4.1.1), and extracts all [[annotations]] sections.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phToml   Handle receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR                 Success.
 * @retval REUSE_ERROR_INVALID_PARAM      pszPath or phToml is NULL.
 * @retval REUSE_ERROR_OPEN_FAILED        File cannot be opened.
 * @retval REUSE_ERROR_READ_FAILED        Read error.
 * @retval REUSE_ERROR_SYNTAX             File content is syntactically
 *                                        invalid.
 * @retval REUSE_ERROR_VERSION_MISSING    "version" key absent.
 * @retval REUSE_ERROR_VERSION_NOT_INT    "version" is not an integer.
 * @retval REUSE_ERROR_VERSION_UNSUP      "version" value is not 1.
 * @retval REUSE_ERROR_ANNOT_NO_PATH      An [[annotations]] entry
 *                                        lacks the "path" key.
 * @retval REUSE_ERROR_ANNOT_BAD_PATH     "path" is neither a string nor
 *                                        an array of strings.
 * @retval REUSE_ERROR_ANNOT_BAD_FIELD    A field in an [[annotations]]
 *                                        entry has an unsupported type.
 * @retval REUSE_ERROR_OUT_OF_MEMORY      Memory allocation failure.
 *
 * @note Release the handle with ReuseTomlClose.
 * @see ReuseTomlClose
 */
APIRET ReuseTomlOpen(PCSZ pszPath, HREUSETOML *phToml);

/**
 * @brief Close a document and release all associated memory.
 *
 * All HREUSEANN handles obtained from this document become invalid.
 *
 * @param[in] hToml  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success. Also for NULLHANDLE.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 *
 * @warning Do not call ReuseTomlClose twice with the same handle.
 * @see ReuseTomlOpen
 */
APIRET ReuseTomlClose(HREUSETOML hToml);

/* ==================================================================
 * Document-level accessors
 * ================================================================== */

/**
 * @brief Query the parsed "version" value (always 1 on success).
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET ReuseTomlGetVersion(HREUSETOML hToml, PLONGLONG pllValue);

/**
 * @brief Query the directory containing the REUSE.toml file.
 *
 * The value is the file path with the last path component removed. If
 * the path has no separators, the result is an empty string.
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only and
 * returns the required size (including NUL) in *pulUsed.
 *
 * @param[in]  hToml   Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseTomlGetSourceDir(HREUSETOML hToml, PSZ pszBuf,
                             ULONG ulSize, PULONG pulUsed);

/**
 * @brief Query the number of [[annotations]] entries.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET ReuseTomlGetAnnotationCount(HREUSETOML hToml, PULONG pulCount);

/* ==================================================================
 * Annotation access
 * ================================================================== */

/**
 * @brief Obtain a borrowed handle to one [[annotations]] entry.
 *
 * The handle is valid until ReuseTomlClose. It does not need to be
 * released separately.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based annotation index.
 * @param[out] phAnn    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 * @retval REUSE_ERROR_INDEX_RANGE    Index out of range.
 */
APIRET ReuseTomlGetAnnotation(HREUSETOML hToml, ULONG ulIndex,
                              HREUSEANN *phAnn);

/* ==================================================================
 * Path list
 * ================================================================== */

/**
 * @brief Number of patterns in the "path" list of an annotation.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET ReuseAnnGetPathCount(HREUSEANN hAnn, PULONG pulCount);

/**
 * @brief Retrieve one path pattern by index.
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_INDEX_RANGE     Index out of range.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetPath(HREUSEANN hAnn, ULONG ulIndex,
                       PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Scalar fields
 * ================================================================== */

/**
 * @brief Retrieve the SPDX-License-Identifier field.
 *
 * If the key's value in the file was an array, elements are joined
 * with " AND ".
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_NOT_FOUND       Field absent.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetLicense(HREUSEANN hAnn,
                          PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Retrieve the SPDX-FileCopyrightText field.
 *
 * If the key's value in the file was an array, elements are joined
 * with '\n'.
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_NOT_FOUND       Field absent.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetCopyright(HREUSEANN hAnn,
                            PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Retrieve SPDX-PackageName (scalar string).
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_NOT_FOUND       Field absent.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetPackageName(HREUSEANN hAnn,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Retrieve SPDX-PackageSupplier (scalar string).
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_NOT_FOUND       Field absent.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetPackageSupplier(HREUSEANN hAnn,
                                  PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/**
 * @brief Retrieve SPDX-PackageDownloadLocation (scalar string).
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_NOT_FOUND       Field absent.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetPackageDownloadLocation(HREUSEANN hAnn,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed);

/**
 * @brief Retrieve SPDX-PackageComment (scalar string).
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_NOT_FOUND       Field absent.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetPackageComment(HREUSEANN hAnn,
                                 PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Contributors (SPDX-FileContributor)
 * ================================================================== */

/**
 * @brief Number of entries in SPDX-FileContributor.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET ReuseAnnGetContributorCount(HREUSEANN hAnn, PULONG pulCount);

/**
 * @brief Retrieve one contributor by index.
 *
 * If the value in the file was a scalar string, it becomes a single
 * contributor with index 0.
 *
 * If pszBuf is NULL and ulSize is 0, performs a size query only.
 *
 * @param[in]  hAnn    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex Zero-based index.
 * @param[out] pszBuf  Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize  Size of pszBuf in bytes.
 * @param[out] pulUsed Optional. May be NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR              Success.
 * @retval REUSE_ERROR_INVALID_PARAM   Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE  Handle not recognized.
 * @retval REUSE_ERROR_INDEX_RANGE     Index out of range.
 * @retval REUSE_ERROR_BUFFER_OVERFLOW Buffer too small.
 */
APIRET ReuseAnnGetContributor(HREUSEANN hAnn, ULONG ulIndex,
                              PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Metadata
 * ================================================================== */

/**
 * @brief Retrieve the "precedence" value.
 *
 * If the key was absent, REUSE_PRECEDENCE_CLOSEST is returned.
 *
 * @param[in]  hAnn          Handle. Not NULLHANDLE.
 * @param[out] pulPrecedence Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET ReuseAnnGetPrecedence(HREUSEANN hAnn, PULONG pulPrecedence);

/**
 * @brief Retrieve the zero-based position of the annotation in the
 *        file (order of appearance).
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pulOrder Receiver. Not NULL.
 *
 * @return APIRET
 * @retval REUSE_NO_ERROR             Success.
 * @retval REUSE_ERROR_INVALID_PARAM  Any parameter is NULL.
 * @retval REUSE_ERROR_INVALID_HANDLE Handle not recognized.
 */
APIRET ReuseAnnGetOrderInFile(HREUSEANN hAnn, PULONG pulOrder);

#ifdef __cplusplus
}
#endif

#endif /* REUSE_TOML_H */
