/*!
 * @file reuse_lic.c
 *
 * @brief Implementation of the per-file license resolution.
 *
 * Per-file license/copyright resolution (C89).
 *
 * P1: sidecar and in-file tags are no longer read by this module.
 * They are read by the resolver (ReuseOpenFile -> resolve_file ->
 * ReuseQueryFileTags). This module is a thin wrapper that copies
 * the resolved fields, applies the --default-* CLI fallbacks,
 * canonicalizes the license expression, and forwards
 * REUSE_ERROR_NO_COMMENT_STYLE as-is.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reuse_lic.h"
#include "spdx_tag.h"
#include "spdx_db.h"

#ifdef __LINUX__
#include <unistd.h>
#else
#include <io.h>
#endif

/* ------------------------------------------------------------------ */
/* String helpers                                                      */
/* ------------------------------------------------------------------ */

/*!
 * @brief Copy a NUL-terminated string into a fixed buffer, tracking
 *        truncation.
 *
 * Wrapper around strlcpy that also sets *pfTruncated when the
 * source did not fit. A NULL source is copied as an empty string.
 *
 * @param[out] pszDst       Destination buffer. Not NULL.
 * @param[in]  ulDstSize    Size of pszDst in bytes. Must be > 0.
 * @param[in]  pszSrc       Source string, or NULL.
 * @param[out] pfTruncated  Receiver for the truncation flag. May be
 *                          NULL.
 */
static void copy_field(PSZ pszDst, ULONG ulDstSize, PCSZ pszSrc,
                       PBOOL pfTruncated) {
    if (pfTruncated) *pfTruncated = FALSE;
    if (!pszSrc) { pszDst[0] = '\0'; return; }
    if (strlcpy(pszDst, pszSrc, ulDstSize) >= ulDstSize) {
        if (pfTruncated) *pfTruncated = TRUE;
    }
}

/*!
 * @brief Remove trailing whitespace and comment-closing markers.
 *
 * @param[in,out] pszStr  String to modify. Not NULL.
 */
static void strip_trailing_comments(PSZ pszStr) {
    size_t cbLen = strlen(pszStr);
    while (cbLen > 0) {
        if (pszStr[cbLen-1] == ' ' || pszStr[cbLen-1] == '\t' ||
            pszStr[cbLen-1] == '\n' || pszStr[cbLen-1] == '\r') {
            cbLen -= 1;
            continue;
        }
        if (cbLen >= 2 && pszStr[cbLen-2] == '*' && pszStr[cbLen-1] == '/') {
            if (cbLen == 2 || pszStr[cbLen-3] == ' ' ||
                pszStr[cbLen-3] == '\t') {
                cbLen -= 2;
                while (cbLen > 0 && (pszStr[cbLen-1] == ' ' ||
                                     pszStr[cbLen-1] == '\t'))
                    cbLen--;
                continue;
            }
        }
        if (cbLen >= 2 && pszStr[cbLen-2] == '/' && pszStr[cbLen-1] == '/') {
            if (cbLen == 2 || pszStr[cbLen-3] == ' ' ||
                pszStr[cbLen-3] == '\t') {
                cbLen -= 2;
                while (cbLen > 0 && (pszStr[cbLen-1] == ' ' ||
                                     pszStr[cbLen-1] == '\t'))
                    cbLen--;
                continue;
            }
        }
        break;
    }
    pszStr[cbLen] = '\0';
}

/* ------------------------------------------------------------------ */
/* Resolver field extraction                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Field accessor signature for resolver getters.
 *
 * Each implementation is one of the ReuseQueryFile* functions and
 * follows the size-query convention.
 *
 * @param[in] REUSEFIELDFN  Function-pointer type name.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_FILE_NOT_FOUND     Field absent.
 * @retval ERROR_INVALID_PARAMETER  Bad arguments.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
typedef APIRET (APIENTRY *REUSEFIELDFN)(HREUSETREEFILE, PSZ, ULONG, PULONG);

/*!
 * @brief Copy a single string field from a resolver handle into a
 *        fixed buffer.
 *
 * Silent if the field is absent. Sets *pfTruncated if the value did
 * not fit.
 *
 * @param[in]  hFile         Resolver handle. May be NULLHANDLE.
 * @param[in]  fnField       Field accessor.
 * @param[out] pszDst        Destination buffer. Not NULL.
 * @param[in]  ulDstSize     Size of pszDst in bytes.
 * @param[out] pfTruncated   Receiver for the truncation flag. May be
 *                           NULL.
 */
static void copy_file_field(HREUSETREEFILE hFile, REUSEFIELDFN fnField,
                            PSZ pszDst, ULONG ulDstSize,
                            PBOOL pfTruncated) {
    ULONG ulSize = 0;
    APIRET rc;

    pszDst[0] = '\0';
    if (pfTruncated) *pfTruncated = FALSE;
    if (hFile == NULLHANDLE) return;

    rc = fnField(hFile, NULL, 0, &ulSize);
    if (rc != NO_ERROR) return;
    if (ulSize == 0) return;

    if (ulSize > ulDstSize) {
        PSZ pszTmp = (PSZ)malloc(ulSize);
        if (!pszTmp) return;
        rc = fnField(hFile, pszTmp, ulSize, NULL);
        if (rc == NO_ERROR) {
            copy_field(pszDst, ulDstSize, pszTmp, pfTruncated);
        }
        free(pszTmp);
        return;
    }
    rc = fnField(hFile, pszDst, ulDstSize, NULL);
    if (rc != NO_ERROR) pszDst[0] = '\0';
}

/* ------------------------------------------------------------------ */
/* Main function                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Resolve license and copyright for one file.
 *
 * Thin wrapper over ReuseOpenFile. Sidecar and in-file tags are read
 * by the resolver (see reuse.h). This module copies the resolved
 * fields, applies --default-* fallbacks, canonicalizes the license
 * expression, and forwards REUSE_ERROR_NO_COMMENT_STYLE as-is.
 *
 * @param[in]  hTree                Project handle, or NULLHANDLE.
 * @param[in]  pszFullPath          File to resolve. Not NULL.
 * @param[in]  pszDefaultLicense    CLI fallback, or NULL.
 * @param[in]  pszDefaultCopyright  CLI fallback, or NULL.
 * @param[out] pOut                 Result receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFullPath or pOut is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No source matched and no default
 *                                  was provided.
 * @retval REUSE_ERROR_NO_COMMENT_STYLE
 *                                  comments.toml not supplied, or no
 *                                  [[map]] entry for the file.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseResolveLicense(HREUSETREE hTree,
                                    PCSZ pszFullPath,
                                    PCSZ pszDefaultLicense,
                                    PCSZ pszDefaultCopyright,
                                    PREUSELICENSEINFO pOut) {
    HREUSETREEFILE hFile = NULLHANDLE;
    ULONG ulSource = REUSE_LICENSE_SRC_NONE;
    ULONG ulPrecedence = 0;
    PSZ pszCanonical = NULL;
    ULONG ulCanonSize = 0;
    APIRET rc;

    if (!pszFullPath || !pOut) return ERROR_INVALID_PARAMETER;

    /* Initialize the result. */
    pOut->achLicense[0] = '\0';
    pOut->achCopyright[0] = '\0';
    pOut->achContributors[0] = '\0';
    pOut->achPackageName[0] = '\0';
    pOut->achPackageSupplier[0] = '\0';
    pOut->achPackageDownloadLocation[0] = '\0';
    pOut->achPackageComment[0] = '\0';
    pOut->fHasPackageInfo = FALSE;
    pOut->source = REUSE_LICENSE_SRC_NONE;
    pOut->fLicenseFromDefault = FALSE;
    pOut->fCopyrightFromDefault = FALSE;
    pOut->fLicenseTruncated = FALSE;
    pOut->fCopyrightTruncated = FALSE;
    pOut->fContributorsTruncated = FALSE;
    pOut->fPackageNameTruncated = FALSE;
    pOut->fPackageSupplierTruncated = FALSE;
    pOut->fPackageDownloadLocationTruncated = FALSE;
    pOut->fPackageCommentTruncated = FALSE;

    /* 1. Ask the resolver. */
    if (hTree != NULLHANDLE) {
        rc = ReuseOpenFile(hTree, pszFullPath, &hFile, NULL);
        if (rc != NO_ERROR) return rc;
    }

    /* 2. Copy fields from the resolver handle. */
    if (hFile != NULLHANDLE) {
        copy_file_field(hFile, ReuseQueryFileLicense,
                        pOut->achLicense, sizeof(pOut->achLicense),
                        &pOut->fLicenseTruncated);
        copy_file_field(hFile, ReuseQueryFileCopyright,
                        pOut->achCopyright, sizeof(pOut->achCopyright),
                        &pOut->fCopyrightTruncated);
        copy_file_field(hFile, ReuseQueryFileContributors,
                        pOut->achContributors, sizeof(pOut->achContributors),
                        &pOut->fContributorsTruncated);
        copy_file_field(hFile, ReuseQueryFilePackageName,
                        pOut->achPackageName, sizeof(pOut->achPackageName),
                        &pOut->fPackageNameTruncated);
        copy_file_field(hFile, ReuseQueryFilePackageSupplier,
                        pOut->achPackageSupplier,
                        sizeof(pOut->achPackageSupplier),
                        &pOut->fPackageSupplierTruncated);
        copy_file_field(hFile, ReuseQueryFilePackageDownloadLocation,
                        pOut->achPackageDownloadLocation,
                        sizeof(pOut->achPackageDownloadLocation),
                        &pOut->fPackageDownloadLocationTruncated);
        copy_file_field(hFile, ReuseQueryFilePackageComment,
                        pOut->achPackageComment,
                        sizeof(pOut->achPackageComment),
                        &pOut->fPackageCommentTruncated);

        if (ReuseQueryFilePrecedence(hFile, &ulPrecedence) != NO_ERROR)
            ulPrecedence = 0;
        if (ReuseQueryFileSource(hFile, &ulSource) != NO_ERROR)
            ulSource = REUSE_LICENSE_SRC_NONE;

        ReuseCloseFile(hFile);
    }

    if (pOut->achPackageName[0] || pOut->achPackageSupplier[0] ||
        pOut->achPackageDownloadLocation[0] || pOut->achPackageComment[0]) {
        pOut->fHasPackageInfo = TRUE;
    }

    /* 3. Record the source reported by the resolver. */
    pOut->source = (REUSELICENSESOURCE)ulSource;

    /* 4. Fallback to --default-* only when the resolver produced
     *    nothing for the corresponding field. */
    if (pOut->achLicense[0] == '\0' &&
        pszDefaultLicense && pszDefaultLicense[0]) {
        copy_field(pOut->achLicense, sizeof(pOut->achLicense),
                   pszDefaultLicense, &pOut->fLicenseTruncated);
        pOut->fLicenseFromDefault = TRUE;
        if (pOut->source == REUSE_LICENSE_SRC_NONE)
            pOut->source = REUSE_LICENSE_SRC_DEFAULT;
    }
    if (pOut->achCopyright[0] == '\0' &&
        pszDefaultCopyright && pszDefaultCopyright[0]) {
        copy_field(pOut->achCopyright, sizeof(pOut->achCopyright),
                   pszDefaultCopyright, &pOut->fCopyrightTruncated);
        pOut->fCopyrightFromDefault = TRUE;
        if (pOut->source == REUSE_LICENSE_SRC_NONE)
            pOut->source = REUSE_LICENSE_SRC_DEFAULT;
    }

    /* 5. Normalize the SPDX expression (canonical case). */
    if (pOut->achLicense[0] != '\0') {
        rc = SpdxQueryExpressionCanonical(pOut->achLicense, NULL, 0,
                                          &ulCanonSize);
        if (rc == NO_ERROR && ulCanonSize > 0) {
            pszCanonical = (PSZ)malloc(ulCanonSize);
            if (pszCanonical) {
                if (SpdxQueryExpressionCanonical(pOut->achLicense,
                                                 pszCanonical,
                                                 ulCanonSize,
                                                 NULL) == NO_ERROR) {
                    copy_field(pOut->achLicense,
                               sizeof(pOut->achLicense),
                               pszCanonical,
                               &pOut->fLicenseTruncated);
                }
                free(pszCanonical);
            }
        }
    }

    /* Defensive cleanup of trailing markers (sidecar with a comment). */
    strip_trailing_comments(pOut->achLicense);
    strip_trailing_comments(pOut->achCopyright);

    if (pOut->achLicense[0] == '\0' && pOut->achCopyright[0] == '\0')
        return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}
