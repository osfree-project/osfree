/* reuse_toml.c - REUSE.toml parser, OS/2 API style
 * (C89 + Watcom extensions) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reuse_toml.h"
#include "reuse_toml_internal.h"
#include "toml.h"

/**
 * @file reuse_toml.c
 * @brief Implementation of the REUSE.toml parser.
 *
 * Conforms to:
 *   - https://reuse.software/spec-3.3/
 */

/* ==================================================================
 * Small helpers
 * ================================================================== */

/**
 * @brief Duplicate a byte range into a fresh NUL-terminated string.
 *
 * @param[in] pszSrc  Source bytes. Not NULL.
 * @param[in] cbLen   Number of bytes to copy.
 *
 * @return malloc'd string, or NULL on allocation failure.
 */
static PSZ dup_n(PCSZ pszSrc, size_t cbLen) {
    PSZ pszOut;
    pszOut = (PSZ)malloc(cbLen + 1);
    if (!pszOut) return NULL;
    memcpy(pszOut, pszSrc, cbLen);
    pszOut[cbLen] = '\0';
    return pszOut;
}

/**
 * @brief Duplicate a NUL-terminated string.
 *
 * @param[in] pszSrc  Source string. Not NULL.
 *
 * @return malloc'd copy, or NULL on allocation failure.
 */
static PSZ dup_str(PCSZ pszSrc) {
    return dup_n(pszSrc, strlen(pszSrc));
}

/**
 * @brief Convert a TOML node holding a string into a malloc'd string.
 *
 * @param[in] hNode  TOML node. Not NULLHANDLE.
 *
 * @return malloc'd string, or NULL on failure.
 */
static PSZ node_to_str(HTOMLNODE hNode) {
    ULONG ulSize = 0;
    PSZ pszBuf;
    if (TomlNodeGetString(hNode, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszBuf = (PSZ)malloc(ulSize);
    if (!pszBuf) return NULL;
    if (TomlNodeGetString(hNode, pszBuf, ulSize, NULL) != NO_ERROR) {
        free(pszBuf);
        return NULL;
    }
    return pszBuf;
}

/* ==================================================================
 * Field reading
 * ================================================================== */

/**
 * @brief Read a scalar string value from a TOML table.
 *
 * @param[in] hTable  TOML table. Not NULLHANDLE.
 * @param[in] pszKey  Key. Not NULL.
 *
 * @return malloc'd string, or NULL if absent or on failure.
 */
static PSZ read_scalar_string(HTOMLNODE hTable, PCSZ pszKey) {
    HTOMLNODE hChild = NULLHANDLE;
    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != NO_ERROR)
        return NULL;
    return node_to_str(hChild);
}

/**
 * @brief Read a value that is either a string or an array of strings.
 *
 * Array elements are joined with @p pszSep.
 *
 * @param[in] hTable  TOML table. Not NULLHANDLE.
 * @param[in] pszKey  Key. Not NULL.
 * @param[in] pszSep  Separator. Not NULL.
 *
 * @return malloc'd string, or NULL if absent or on failure.
 */
static PSZ read_string_or_join(HTOMLNODE hTable, PCSZ pszKey,
                               PCSZ pszSep) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, ulIdx;
    PSZ pszResult = NULL;
    size_t cbSepLen = strlen(pszSep);

    if (TomlNodeGetTableEntryByKey(hTable, pszKey, &hChild) != NO_ERROR)
        return NULL;
    if (TomlNodeGetType(hChild, &ulType) != NO_ERROR)
        return NULL;

    if (ulType == TOML_TYPE_STRING) {
        return node_to_str(hChild);
    }
    if (ulType != TOML_TYPE_ARRAY) return NULL;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HTOMLNODE hElem = NULLHANDLE;
        PSZ pszPart;
        size_t cbRlen, cbPlen;
        PSZ pszNew;

        if (TomlNodeGetArrayElement(hChild, ulIdx, &hElem) != NO_ERROR) {
            free(pszResult);
            return NULL;
        }
        pszPart = node_to_str(hElem);
        if (!pszPart) { free(pszResult); return NULL; }

        cbRlen = pszResult ? strlen(pszResult) : 0;
        cbPlen = strlen(pszPart);
        pszNew = (PSZ)realloc(pszResult,
            cbRlen + (cbRlen ? cbSepLen : 0) + cbPlen + 1);
        if (!pszNew) { free(pszPart); free(pszResult); return NULL; }
        pszResult = pszNew;
        if (cbRlen) {
            memcpy(pszResult + cbRlen, pszSep, cbSepLen);
            cbRlen += cbSepLen;
        }
        memcpy(pszResult + cbRlen, pszPart, cbPlen);
        pszResult[cbRlen + cbPlen] = '\0';
        free(pszPart);
    }
    return pszResult;
}

/**
 * @brief Append one path entry to an annotation.
 *
 * @param[in,out] pAnn    Annotation. Not NULL.
 * @param[in]     pszVal  Path value. Not NULL.
 *
 * @return 0 on success, -1 on allocation failure.
 */
static int add_path(REUSEANN *pAnn, PCSZ pszVal) {
    PSZ *papszNew = (PSZ*)realloc(pAnn->papszPaths,
        ((size_t)pAnn->ulPathCount + 1) * sizeof(PSZ));
    if (!papszNew) return -1;
    pAnn->papszPaths = papszNew;
    pAnn->papszPaths[pAnn->ulPathCount] = dup_str(pszVal);
    if (!pAnn->papszPaths[pAnn->ulPathCount]) return -1;
    pAnn->ulPathCount++;
    return 0;
}

/**
 * @brief Read the "path" field of an annotation.
 *
 * Returns:
 *    0  - ok
 *   -1  - key absent (REUSE_ERROR_ANNOT_NO_PATH)
 *   -2  - key present but wrong type (REUSE_ERROR_ANNOT_BAD_PATH)
 *   -3  - out of memory
 *
 * @param[in]  hTable  TOML table. Not NULLHANDLE.
 * @param[out] pAnn    Annotation receiver. Not NULL.
 *
 * @return 0, -1, -2 or -3.
 */
static int read_paths_into(HTOMLNODE hTable, REUSEANN *pAnn) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, ulIdx;

    if (TomlNodeGetTableEntryByKey(hTable, "path", &hChild) != NO_ERROR)
        return -1;
    if (TomlNodeGetType(hChild, &ulType) != NO_ERROR)
        return -2;

    if (ulType == TOML_TYPE_STRING) {
        PSZ pszStr = node_to_str(hChild);
        if (!pszStr) return -3;
        if (add_path(pAnn, pszStr) != 0) { free(pszStr); return -3; }
        free(pszStr);
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -2;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HTOMLNODE hElem = NULLHANDLE;
        PSZ pszStr;
        if (TomlNodeGetArrayElement(hChild, ulIdx, &hElem) != NO_ERROR)
            return -2;
        if (TomlNodeGetType(hElem, &ulType) != NO_ERROR)
            return -2;
        if (ulType != TOML_TYPE_STRING) return -2;
        pszStr = node_to_str(hElem);
        if (!pszStr) return -3;
        if (add_path(pAnn, pszStr) != 0) { free(pszStr); return -3; }
        free(pszStr);
    }
    return 0;
}

/**
 * @brief Read the "SPDX-FileContributor" field of an annotation.
 *
 * @param[in]  hTable  TOML table. Not NULLHANDLE.
 * @param[out] pAnn    Annotation receiver. Not NULL.
 *
 * @return 0 on success, -1 on wrong type, -2 on allocation failure.
 */
static int read_contributors_into(HTOMLNODE hTable, REUSEANN *pAnn) {
    HTOMLNODE hChild = NULLHANDLE;
    ULONG ulType = 0, ulCount = 0, ulIdx;

    if (TomlNodeGetTableEntryByKey(hTable, "SPDX-FileContributor",
                                   &hChild) != NO_ERROR)
        return 0;
    if (TomlNodeGetType(hChild, &ulType) != NO_ERROR) return -1;

    if (ulType == TOML_TYPE_STRING) {
        PSZ pszStr = node_to_str(hChild);
        PSZ *papszNew;
        if (!pszStr) return -2;
        papszNew = (PSZ*)realloc(pAnn->papszContributors,
                                 ((size_t)pAnn->ulContributorCount + 1) *
                                 sizeof(PSZ));
        if (!papszNew) { free(pszStr); return -2; }
        pAnn->papszContributors = papszNew;
        pAnn->papszContributors[pAnn->ulContributorCount++] = pszStr;
        return 0;
    }
    if (ulType != TOML_TYPE_ARRAY) return -1;

    TomlNodeGetArrayCount(hChild, &ulCount);
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HTOMLNODE hElem = NULLHANDLE;
        PSZ pszStr;
        PSZ *papszNew;
        if (TomlNodeGetArrayElement(hChild, ulIdx, &hElem) != NO_ERROR)
            return -1;
        pszStr = node_to_str(hElem);
        if (!pszStr) return -2;
        papszNew = (PSZ*)realloc(pAnn->papszContributors,
                                 ((size_t)pAnn->ulContributorCount + 1) *
                                 sizeof(PSZ));
        if (!papszNew) { free(pszStr); return -2; }
        pAnn->papszContributors = papszNew;
        pAnn->papszContributors[pAnn->ulContributorCount++] = pszStr;
    }
    return 0;
}

/**
 * @brief Read the "precedence" field of an annotation.
 *
 * @param[in]  hTable  TOML table. Not NULLHANDLE.
 * @param[out] pAnn    Annotation receiver. Not NULL.
 *
 * @return 0 on success.
 */
static int read_precedence(HTOMLNODE hTable, REUSEANN *pAnn) {
    PSZ pszStr = read_scalar_string(hTable, "precedence");
    if (!pszStr) return 0;
    if (strcmp(pszStr, "closest") == 0)
        pAnn->ulPrecedence = REUSE_PRECEDENCE_CLOSEST;
    else if (strcmp(pszStr, "aggregate") == 0)
        pAnn->ulPrecedence = REUSE_PRECEDENCE_AGGREGATE;
    else if (strcmp(pszStr, "override") == 0)
        pAnn->ulPrecedence = REUSE_PRECEDENCE_OVERRIDE;
    free(pszStr);
    return 0;
}

/**
 * @brief Parse one [[annotations]] entry.
 *
 * @param[in]  hItem  TOML table for the entry. Not NULLHANDLE.
 * @param[out] pAnn   Annotation receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                     Success.
 * @retval REUSE_ERROR_ANNOT_NO_PATH    "path" key absent.
 * @retval REUSE_ERROR_ANNOT_BAD_PATH   "path" has an unsupported type.
 * @retval REUSE_ERROR_ANNOT_BAD_FIELD  A field has an unsupported type.
 * @retval ERROR_NOT_ENOUGH_MEMORY      Allocation failure.
 */
static APIRET parse_one_annotation(HTOMLNODE hItem, REUSEANN *pAnn) {
    int rc;

    memset(pAnn, 0, sizeof(*pAnn));
    pAnn->ulPrecedence = REUSE_PRECEDENCE_CLOSEST;

    rc = read_paths_into(hItem, pAnn);
    if (rc == -1) return REUSE_ERROR_ANNOT_NO_PATH;
    if (rc == -2) return REUSE_ERROR_ANNOT_BAD_PATH;
    if (rc == -3) return ERROR_NOT_ENOUGH_MEMORY;
    if (pAnn->ulPathCount == 0) return REUSE_ERROR_ANNOT_NO_PATH;

    pAnn->pszLicense   = read_string_or_join(hItem,
                          "SPDX-License-Identifier", " AND ");
    pAnn->pszCopyright = read_string_or_join(hItem,
                          "SPDX-FileCopyrightText", "\n");

    rc = read_contributors_into(hItem, pAnn);
    if (rc == -1) return REUSE_ERROR_ANNOT_BAD_FIELD;
    if (rc == -2) return ERROR_NOT_ENOUGH_MEMORY;

    read_precedence(hItem, pAnn);

    pAnn->pszPackageName = read_scalar_string(hItem, "SPDX-PackageName");
    pAnn->pszPackageSupplier =
        read_scalar_string(hItem, "SPDX-PackageSupplier");
    pAnn->pszPackageDownloadLocation =
        read_scalar_string(hItem, "SPDX-PackageDownloadLocation");
    pAnn->pszPackageComment =
        read_scalar_string(hItem, "SPDX-PackageComment");

    return NO_ERROR;
}

/* ==================================================================
 * Free helpers
 * ================================================================== */

/**
 * @brief Release all memory owned by one annotation.
 *
 * @param[in,out] pAnn  Annotation. Not NULL.
 */
static void ann_free(REUSEANN *pAnn) {
    ULONG ulIdx;
    if (!pAnn) return;
    for (ulIdx = 0; ulIdx < pAnn->ulPathCount; ulIdx++)
        free(pAnn->papszPaths[ulIdx]);
    free(pAnn->papszPaths);
    free(pAnn->pszLicense);
    free(pAnn->pszCopyright);
    for (ulIdx = 0; ulIdx < pAnn->ulContributorCount; ulIdx++)
        free(pAnn->papszContributors[ulIdx]);
    free(pAnn->papszContributors);
    free(pAnn->pszPackageName);
    free(pAnn->pszPackageSupplier);
    free(pAnn->pszPackageDownloadLocation);
    free(pAnn->pszPackageComment);
    memset(pAnn, 0, sizeof(*pAnn));
}

/**
 * @brief Release a parsed document and all its annotations.
 *
 * @param[in] pd  Document. May be NULL.
 */
static void doc_free(PREUSETOMLDOC pd) {
    ULONG ulIdx;
    if (!pd) return;
    for (ulIdx = 0; ulIdx < pd->ulAnnotationCount; ulIdx++)
        ann_free(&pd->paAnnotations[ulIdx]);
    free(pd->paAnnotations);
    free(pd->pszSourceDir);
    free(pd);
}

/* ==================================================================
 * Common scalar-out helper
 * ================================================================== */

/**
 * @brief Copy a string into a caller-supplied buffer.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed is written.
 *   - ulSize large enough: value copied and NUL-terminated.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszValue  Value, or NULL.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     pszValue is NULL.
 * @retval ERROR_INVALID_PARAMETER  pszBuf is NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
static APIRET copy_field_out(PCSZ pszValue,
                             PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    size_t cbLen;
    if (!pszValue) return ERROR_FILE_NOT_FOUND;
    cbLen = strlen(pszValue);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszValue, cbLen);
    pszBuf[cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/* ==================================================================
 * Handle helpers
 * ================================================================== */

/**
 * @brief Translate a public document handle into the internal pointer.
 *
 * @param[in] h  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if h is NULLHANDLE.
 */
static PREUSETOMLDOC as_doc(HREUSETOML h) { return (PREUSETOMLDOC)h; }

/**
 * @brief Translate a public annotation handle into the internal pointer.
 *
 * @param[in] h  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL if h is NULLHANDLE.
 */
static PREUSEANN as_ann(HREUSEANN h) { return (PREUSEANN)h; }

/* ==================================================================
 * Public API
 * ================================================================== */

/**
 * @brief Open and parse a REUSE.toml file.
 *
 * Reads the file, verifies the mandatory "version = 1" key
 * (REUSE 3.3 §4.1.1), and extracts all [[annotations]] sections.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] phToml   Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 * @retval NO_ERROR                      Success.
 * @retval ERROR_INVALID_PARAMETER       pszPath or phToml is NULL.
 * @retval ERROR_OPEN_FAILED             File cannot be opened.
 * @retval ERROR_READ_FAULT              Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY       Memory allocation failure.
 * @retval REUSE_ERROR_SYNTAX            File content is syntactically
 *                                       invalid.
 * @retval REUSE_ERROR_VERSION_MISSING   "version" key absent.
 * @retval REUSE_ERROR_VERSION_NOT_INT   "version" is not an integer.
 * @retval REUSE_ERROR_VERSION_UNSUP     "version" value is not 1.
 * @retval REUSE_ERROR_ANNOT_NO_PATH     An [[annotations]] entry
 *                                       lacks the "path" key.
 * @retval REUSE_ERROR_ANNOT_BAD_PATH    "path" is neither a string nor
 *                                       an array of strings.
 * @retval REUSE_ERROR_ANNOT_BAD_FIELD   A field in an [[annotations]]
 *                                       entry has an unsupported type.
 */
APIRET APIENTRY ReuseOpen(PCSZ pszPath, HREUSETOML *phToml) {
    HTOMLDOC hDoc = NULLHANDLE;
    HTOMLNODE hVer = NULLHANDLE;
    HTOMLNODE hAnn = NULLHANDLE;
    PREUSETOMLDOC pd = NULL;
    APIRET rc;
    ULONG ulType = 0, ulCount = 0, ulIdx;
    LONGLONG llVersion = 0;

    if (!pszPath || !phToml) return ERROR_INVALID_PARAMETER;
    *phToml = NULLHANDLE;

    rc = TomlOpen(pszPath, &hDoc);
    if (rc == ERROR_INVALID_PARAMETER)
        return ERROR_INVALID_PARAMETER;
    if (rc == ERROR_OPEN_FAILED)
        return ERROR_OPEN_FAILED;
    if (rc == ERROR_READ_FAULT)
        return ERROR_READ_FAULT;
    if (rc == ERROR_NOT_ENOUGH_MEMORY)
        return ERROR_NOT_ENOUGH_MEMORY;
    if (rc != NO_ERROR)
        return REUSE_ERROR_SYNTAX;

    rc = TomlQueryNode(hDoc, "version", &hVer);
    if (rc != NO_ERROR) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_MISSING;
    }
    if (TomlNodeGetType(hVer, &ulType) != NO_ERROR) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_MISSING;
    }
    if (ulType != TOML_TYPE_INTEGER) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_NOT_INT;
    }
    TomlNodeGetInteger(hVer, &llVersion);
    if (llVersion != 1) {
        TomlClose(hDoc);
        return REUSE_ERROR_VERSION_UNSUP;
    }

    pd = (PREUSETOMLDOC)calloc(1, sizeof(REUSETOMLDOC));
    if (!pd) { TomlClose(hDoc); return ERROR_NOT_ENOUGH_MEMORY; }
    pd->llVersion = llVersion;

    pd->pszSourceDir = dup_str(pszPath);
    if (pd->pszSourceDir) {
        PSZ pszSlash = strrchr(pd->pszSourceDir, '/');
        PSZ pszBackslash = strrchr(pd->pszSourceDir, '\\');
        if (pszBackslash && (!pszSlash || pszBackslash > pszSlash))
            pszSlash = pszBackslash;
        if (pszSlash) *pszSlash = '\0';
    }

    rc = TomlQueryNode(hDoc, "annotations", &hAnn);
    if (rc == NO_ERROR) {
        if (TomlNodeGetType(hAnn, &ulType) == NO_ERROR &&
            ulType == TOML_TYPE_ARRAY) {
            TomlNodeGetArrayCount(hAnn, &ulCount);
            if (ulCount > 0) {
                pd->paAnnotations =
                    (REUSEANN*)calloc(ulCount, sizeof(REUSEANN));
                if (!pd->paAnnotations) {
                    doc_free(pd);
                    TomlClose(hDoc);
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                pd->ulAnnotationCapacity = ulCount;
                for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
                    HTOMLNODE hItem = NULLHANDLE;
                    REUSEANN *pAnn =
                        &pd->paAnnotations[pd->ulAnnotationCount];
                    APIRET arc;
                    rc = TomlNodeGetArrayElement(hAnn, ulIdx, &hItem);
                    if (rc != NO_ERROR) {
                        doc_free(pd);
                        TomlClose(hDoc);
                        return REUSE_ERROR_SYNTAX;
                    }
                    arc = parse_one_annotation(hItem, pAnn);
                    if (arc != NO_ERROR) {
                        doc_free(pd);
                        TomlClose(hDoc);
                        return arc;
                    }
                    pAnn->ulOrderInFile = pd->ulAnnotationCount;
                    pd->ulAnnotationCount++;
                }
            }
        }
    }

    TomlClose(hDoc);
    *phToml = (HREUSETOML)pd;
    return NO_ERROR;
}

/**
 * @brief Close a document and release all associated memory.
 *
 * All HREUSEANN handles obtained from this document become invalid.
 *
 * @param[in] hToml  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 */
APIRET APIENTRY ReuseClose(HREUSETOML hToml) {
    PREUSETOMLDOC pd;
    if (hToml == NULLHANDLE) return NO_ERROR;
    pd = as_doc(hToml);
    if (!pd) return ERROR_INVALID_HANDLE;
    doc_free(pd);
    return NO_ERROR;
}

/**
 * @brief Query the parsed "version" value (always 1 on success).
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[out] pllValue  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hToml or pllValue is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseGetVersion(HREUSETOML hToml, PLONGLONG pllValue) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd || !pllValue) return ERROR_INVALID_PARAMETER;
    *pllValue = pd->llVersion;
    return NO_ERROR;
}

/**
 * @brief Query the directory containing the REUSE.toml file.
 *
 * The value is the file path with the last path component removed.
 * If the path has no separators, the result is an empty string.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hToml is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No source directory recorded.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseGetSourceDir(HREUSETOML hToml, PSZ pszBuf,
                                  ULONG ulSize, PULONG pulUsed) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pd->pszSourceDir, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Query the number of [[annotations]] entries.
 *
 * @param[in]  hToml     Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hToml or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseGetAnnotationCount(HREUSETOML hToml, PULONG pulCount) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd || !pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = pd->ulAnnotationCount;
    return NO_ERROR;
}

/**
 * @brief Obtain a borrowed handle to one [[annotations]] entry.
 *
 * The handle is valid until ReuseClose.
 *
 * @param[in]  hToml    Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based annotation index.
 * @param[out] phAnn    Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hToml or phAnn is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 */
APIRET APIENTRY ReuseGetAnnotation(HREUSETOML hToml, ULONG ulIndex,
                                   HREUSEANN *phAnn) {
    PREUSETOMLDOC pd = as_doc(hToml);
    if (!pd || !phAnn) return ERROR_INVALID_PARAMETER;
    *phAnn = NULLHANDLE;
    if (ulIndex >= pd->ulAnnotationCount) return ERROR_NO_MORE_ITEMS;
    *phAnn = (HREUSEANN)&pd->paAnnotations[ulIndex];
    return NO_ERROR;
}

/**
 * @brief Query the number of patterns in the "path" list.
 *
 * @param[in]  hAnn      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseAnnGetPathCount(HREUSEANN hAnn, PULONG pulCount) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = pa->ulPathCount;
    return NO_ERROR;
}

/**
 * @brief Retrieve one path pattern by index.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 * @retval ERROR_FILE_NOT_FOUND     Path entry is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetPath(HREUSEANN hAnn, ULONG ulIndex,
                                PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    if (ulIndex >= pa->ulPathCount) return ERROR_NO_MORE_ITEMS;
    return copy_field_out(pa->papszPaths[ulIndex], pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the SPDX-License-Identifier field.
 *
 * If the key's value in the file was an array, elements are joined
 * with " AND ".
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     The field is absent.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetLicense(HREUSEANN hAnn,
                                   PSZ pszBuf, ULONG ulSize, PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pa->pszLicense, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the SPDX-FileCopyrightText field.
 *
 * If the key's value in the file was an array, elements are joined
 * with '\n'.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     The field is absent.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetCopyright(HREUSEANN hAnn,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pa->pszCopyright, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageName.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     The field is absent.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetPackageName(HREUSEANN hAnn,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pa->pszPackageName, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageSupplier.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     The field is absent.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetPackageSupplier(HREUSEANN hAnn,
                                           PSZ pszBuf, ULONG ulSize,
                                           PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pa->pszPackageSupplier, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageDownloadLocation.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     The field is absent.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetPackageDownloadLocation(HREUSEANN hAnn,
                                                   PSZ pszBuf, ULONG ulSize,
                                                   PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pa->pszPackageDownloadLocation, pszBuf, ulSize,
                          pulUsed);
}

/**
 * @brief Retrieve SPDX-PackageComment.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     The field is absent.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetPackageComment(HREUSEANN hAnn,
                                          PSZ pszBuf, ULONG ulSize,
                                          PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    return copy_field_out(pa->pszPackageComment, pszBuf, ulSize, pulUsed);
}

/**
 * @brief Query the number of entries in SPDX-FileContributor.
 *
 * @param[in]  hAnn      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseAnnGetContributorCount(HREUSEANN hAnn,
                                            PULONG pulCount) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = pa->ulContributorCount;
    return NO_ERROR;
}

/**
 * @brief Retrieve one contributor by index.
 *
 * If the value in the file was a scalar string, it becomes a single
 * contributor with index 0.
 *
 * @param[in]  hAnn     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn is NULLHANDLE, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 * @retval ERROR_FILE_NOT_FOUND     Contributor entry is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY ReuseAnnGetContributor(HREUSEANN hAnn, ULONG ulIndex,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa) return ERROR_INVALID_PARAMETER;
    if (ulIndex >= pa->ulContributorCount) return ERROR_NO_MORE_ITEMS;
    return copy_field_out(pa->papszContributors[ulIndex],
                          pszBuf, ulSize, pulUsed);
}

/**
 * @brief Retrieve the "precedence" value.
 *
 * If the key was absent, REUSE_PRECEDENCE_CLOSEST is returned.
 *
 * @param[in]  hAnn           Handle. Not NULLHANDLE.
 * @param[out] pulPrecedence  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn or pulPrecedence is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseAnnGetPrecedence(HREUSEANN hAnn,
                                      PULONG pulPrecedence) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulPrecedence) return ERROR_INVALID_PARAMETER;
    *pulPrecedence = pa->ulPrecedence;
    return NO_ERROR;
}

/**
 * @brief Retrieve the zero-based position of the annotation in the
 *        file (order of appearance).
 *
 * @param[in]  hAnn      Handle. Not NULLHANDLE.
 * @param[out] pulOrder  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hAnn or pulOrder is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseAnnGetOrderInFile(HREUSEANN hAnn, PULONG pulOrder) {
    PREUSEANN pa = as_ann(hAnn);
    if (!pa || !pulOrder) return ERROR_INVALID_PARAMETER;
    *pulOrder = pa->ulOrderInFile;
    return NO_ERROR;
}
