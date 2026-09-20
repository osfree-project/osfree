/* spdx_db.h - SPDX License List database (C89, OpenWatcom)
 *
 * The on-disk cache stores both the index and the details (full
 * text, template, HTML). The index is read by SpdxOpenDatabase
 * (fast, several hundred KB). Details are read on demand via fseek
 * (they are not parsed again).
 */
#ifndef SPDX_DB_H
#define SPDX_DB_H

#include "os2types.h"
#include "os2err.h"
#include "strset.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_db.h
 * @brief SPDX License List database.
 *
 * Conforms to:
 *   - SPDX License List.
 *     https://spdx.org/licenses/
 *   - SPDX 2.3, Annex D.2 (case-insensitive identifier comparison).
 *   - SPDX 2.3, Annex D (license expression grammar).
 */

/* ==================================================================
 * Return codes
 * ================================================================== */

/** @def SPDXDB_ERROR_LICENSES @brief licenses.json failed to load.
 *  User range 0xFF01. */
#define SPDXDB_ERROR_LICENSES    0xFF01
/** @def SPDXDB_ERROR_EXCEPTIONS @brief exceptions.json failed to load.
 *  User range 0xFF02. */
#define SPDXDB_ERROR_EXCEPTIONS  0xFF02
/** @def SPDXDB_ERROR_CACHE @brief Cache file could not be written.
 *  User range 0xFF03. */
#define SPDXDB_ERROR_CACHE       0xFF03

/** @def SPDX_EXPR_SYNTAX_ERROR @brief Expression grammar violation.
 *  User range 0xFF04. */
#define SPDX_EXPR_SYNTAX_ERROR   0xFF04
/** @def SPDX_EXPR_UNKNOWN_TOKEN @brief Unknown SPDX identifier.
 *  User range 0xFF05. */
#define SPDX_EXPR_UNKNOWN_TOKEN  0xFF05

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/**
 * @brief Open the SPDX database.
 *
 * @param[in] pszDbRoot    Database root. Inside it the following are
 *                         expected:
 *                           licenses.json
 *                           exceptions.json
 *                           details/<id>.json
 *                           exceptions/<id>.json
 *                         Not NULL.
 * @param[in] pszCacheFile Path to the cache file, or NULL for no
 *                         cache.
 *
 * @return APIRET
 * @retval NO_ERROR                 Full success.
 * @retval ERROR_INVALID_PARAMETER  pszDbRoot is NULL or empty.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval SPDXDB_ERROR_LICENSES    licenses.json failed to load.
 * @retval SPDXDB_ERROR_EXCEPTIONS  exceptions.json failed to load.
 * @retval SPDXDB_ERROR_CACHE       Cache could not be written.
 */
APIRET APIENTRY SpdxOpenDatabase(PCSZ pszDbRoot, PCSZ pszCacheFile);

/**
 * @brief Close the SPDX database.
 *
 * Releases every resource owned by the module, including
 * lazy-loaded details and the open cache file.
 *
 * @return APIRET
 * @retval NO_ERROR  Always.
 */
APIRET APIENTRY SpdxCloseDatabase(void);

/* ==================================================================
 * Identifier queries
 * ================================================================== */

/**
 * @brief Query the canonical form of an SPDX identifier.
 *
 * If @p pszId is a known license or exception, the canonical case
 * form is copied into @p pszBuf.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *     NUL) is written.
 *   - ulSize large enough: value copied and NUL-terminated;
 *     *pulUsed is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_FILE_NOT_FOUND     Identifier is not a known license
 *                                  or exception.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryCanonicalId(PCSZ pszId, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Predicates
 * ================================================================== */

/**
 * @brief Query whether an identifier is a known SPDX license.
 *
 * LicenseRef-* and DocumentRef-<id>:LicenseRef-* are accepted as
 * valid even though they are not in the SPDX License List.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pfValid  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfValid is NULL.
 * @retval SPDXDB_ERROR_LICENSES    The license index is not loaded;
 *                                  the query cannot be answered.
 */
APIRET APIENTRY SpdxQueryLicenseValid(PCSZ pszId, PBOOL pfValid);

/**
 * @brief Query whether an identifier is a known SPDX exception.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pfValid  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfValid is NULL.
 * @retval SPDXDB_ERROR_EXCEPTIONS  The exception index is not
 *                                  loaded; the query cannot be
 *                                  answered.
 */
APIRET APIENTRY SpdxQueryExceptionValid(PCSZ pszId, PBOOL pfValid);

/**
 * @brief Query whether a license identifier is deprecated.
 *
 * @param[in]  pszId         Identifier. Not NULL.
 * @param[out] pfDeprecated  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfDeprecated is NULL.
 */
APIRET APIENTRY SpdxQueryLicenseDeprecated(PCSZ pszId,
                                           PBOOL pfDeprecated);

/**
 * @brief Query whether an exception identifier is deprecated.
 *
 * @param[in]  pszId         Identifier. Not NULL.
 * @param[out] pfDeprecated  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfDeprecated is NULL.
 */
APIRET APIENTRY SpdxQueryExceptionDeprecated(PCSZ pszId,
                                             PBOOL pfDeprecated);

/**
 * @brief Query whether a license is OSI-approved.
 *
 * @param[in]  pszId       Identifier. Not NULL.
 * @param[out] pfApproved  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfApproved is NULL.
 */
APIRET APIENTRY SpdxQueryLicenseOsiApproved(PCSZ pszId,
                                            PBOOL pfApproved);

/**
 * @brief Query whether a license is FSF-libre.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pfLibre  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfLibre is NULL.
 */
APIRET APIENTRY SpdxQueryLicenseFsfLibre(PCSZ pszId, PBOOL pfLibre);

/* ==================================================================
 * Text queries
 * ================================================================== */

/**
 * @brief Query the license text of a known license.
 *
 * The detail is lazy-loaded.
 *
 * Size-query convention:
 *   - pszBuf == NULL, ulSize == 0: only *pulUsed (size including
 *     NUL) is written.
 *   - ulSize large enough: value copied and NUL-terminated;
 *     *pulUsed is the length without NUL.
 *   - ulSize too small: ERROR_BUFFER_OVERFLOW; *pulUsed is the
 *     required size including NUL.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_FILE_NOT_FOUND     No text available.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryLicenseText(PCSZ pszId, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed);

/**
 * @brief Query the exception text of a known exception.
 *
 * The detail is lazy-loaded.
 *
 * Size-query convention as for SpdxQueryLicenseText.
 *
 * @param[in]  pszId    Identifier. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszId is NULL, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_FILE_NOT_FOUND     No text available.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryExceptionText(PCSZ pszId, PSZ pszBuf,
                                       ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Expression helpers
 * ================================================================== */

/**
 * @brief Query the validity of an SPDX license expression.
 *
 * On SPDX_EXPR_UNKNOWN_TOKEN, if @p ppszBadToken is not NULL, a
 * pointer to the start of the offending token inside @p pszExpr is
 * written there. The token length is determined up to the nearest
 * space, '(' or ')'.
 *
 * @param[in]  pszExpr       Expression. Not NULL.
 * @param[out] ppszBadToken  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Expression is valid.
 * @retval SPDX_EXPR_SYNTAX_ERROR   Grammar violation.
 * @retval SPDX_EXPR_UNKNOWN_TOKEN  Unknown SPDX identifier.
 */
APIRET APIENTRY SpdxQueryExpression(PCSZ pszExpr, PCSZ *ppszBadToken);

/**
 * @brief Query the canonical form of an SPDX license expression.
 *
 * Every identifier from the SPDX License List or the SPDX Exception
 * List is replaced with its canonical case (for example,
 * 'BSD-3-clause' becomes 'BSD-3-Clause'). AND / OR / WITH operators,
 * parentheses and whitespace are preserved as-is. LicenseRef-* and
 * DocumentRef-* identifiers are left unchanged.
 *
 * Size-query convention as for SpdxQueryLicenseText.
 *
 * @param[in]  pszExpr  Expression. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszExpr is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryExpressionCanonical(PCSZ pszExpr, PSZ pszBuf,
                                             ULONG ulSize, PULONG pulUsed);

/**
 * @brief Collect the SPDX identifiers from a license expression.
 *
 * Validates the expression first, then extracts every identifier
 * that is not one of the operators AND / OR / WITH and not a
 * parenthesis. Duplicates are removed by the destination set.
 *
 * @param[in]  pszExpr       Expression. Not NULL.
 * @param[in]  hOut          Destination set. Not NULLHANDLE.
 * @param[out] ppszBadToken  Optional. May be NULL. On
 *                           SPDX_EXPR_UNKNOWN_TOKEN, receives a
 *                           pointer to the offending token inside
 *                           @p pszExpr.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszExpr is NULL.
 * @retval ERROR_INVALID_HANDLE     hOut is not recognized.
 * @retval SPDX_EXPR_SYNTAX_ERROR   Grammar violation.
 * @retval SPDX_EXPR_UNKNOWN_TOKEN  Unknown SPDX identifier.
 * @retval SPDXDB_ERROR_LICENSES    The license index is not loaded.
 * @retval SPDXDB_ERROR_EXCEPTIONS  The exception index is not loaded.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxExpressionCollectIds(PCSZ pszExpr, HSTRSET hOut,
                                         PCSZ *ppszBadToken);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_DB_H */
