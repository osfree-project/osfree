/* reuse_lic.c - per-file license/copyright resolution (C89) */

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

/**
 * @file reuse_lic.c
 * @brief Implementation of the per-file license resolution.
 */

/* ------------------------------------------------------------------ */
/* String helpers                                                      */
/* ------------------------------------------------------------------ */

/**
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

/**
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
/* In-file sources                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Read the license tag from a file into a heap buffer.
 *
 * Uses the size-query convention of SpdxQueryFileLicense.
 *
 * @param[in] pszPath  Path to the file. Not NULL.
 *
 * @return malloc'd value, or NULL if absent or on error.
 */
static PSZ read_license_value(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxQueryFileLicense(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxQueryFileLicense(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    if (pszOut[0] == '\0') { free(pszOut); return NULL; }
    return pszOut;
}

/**
 * @brief Read the copyright tags from a file into a heap buffer.
 *
 * Uses the size-query convention of SpdxQueryFileCopyright.
 *
 * @param[in] pszPath  Path to the file. Not NULL.
 *
 * @return malloc'd value, or NULL if absent or on error.
 */
static PSZ read_copyright_value(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxQueryFileCopyright(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxQueryFileCopyright(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    if (pszOut[0] == '\0') { free(pszOut); return NULL; }
    return pszOut;
}

/**
 * @brief Read the adjacent <file>.license sidecar.
 *
 * @param[in]  pszFullPath     File path. Not NULL.
 * @param[out] ppszLicense     Receiver for the license, or NULL.
 *                             Set to NULL on failure.
 * @param[out] ppszCopyright   Receiver for the copyright, or NULL.
 *                             Set to NULL on failure.
 *
 * @return APIRET
 * @retval NO_ERROR             At least one field was read.
 * @retval ERROR_FILE_NOT_FOUND Sidecar absent or empty.
 */
static APIRET read_sidecar(PCSZ pszFullPath,
                           PSZ *ppszLicense, PSZ *ppszCopyright) {
    CHAR achSidecar[1024];
    BOOL fExists;
    PSZ pszLic = NULL, pszCop = NULL;

    *ppszLicense = NULL;
    *ppszCopyright = NULL;

    snprintf(achSidecar, sizeof(achSidecar), "%s.license", pszFullPath);
#ifdef __LINUX__
    fExists = (access(achSidecar, F_OK) == 0) ? TRUE : FALSE;
#else
    fExists = (_access(achSidecar, 0) == 0) ? TRUE : FALSE;
#endif
    if (!fExists) return ERROR_FILE_NOT_FOUND;

    pszLic = read_license_value(achSidecar);
    pszCop = read_copyright_value(achSidecar);

    *ppszLicense = pszLic;
    *ppszCopyright = pszCop;

    if (!*ppszLicense && !*ppszCopyright) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}

/**
 * @brief Read in-file SPDX tags.
 *
 * @param[in]  pszFullPath     File path. Not NULL.
 * @param[out] ppszLicense     Receiver for the license, or NULL.
 * @param[out] ppszCopyright   Receiver for the copyright, or NULL.
 *
 * @return APIRET
 * @retval NO_ERROR             At least one tag was read.
 * @retval ERROR_FILE_NOT_FOUND No tags present.
 */
static APIRET read_tags(PCSZ pszFullPath,
                        PSZ *ppszLicense, PSZ *ppszCopyright) {
    PSZ pszLic = NULL, pszCop = NULL;

    *ppszLicense = NULL;
    *ppszCopyright = NULL;

    pszLic = read_license_value(pszFullPath);
    pszCop = read_copyright_value(pszFullPath);

    *ppszLicense = pszLic;
    *ppszCopyright = pszCop;

    if (!*ppszLicense && !*ppszCopyright) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Resolver field extraction                                           */
/* ------------------------------------------------------------------ */

/** @brief Field accessor signature for resolver getters. */
typedef APIRET (APIENTRY *REUSEFIELDFN)(HREUSETREEFILE, PSZ, ULONG, PULONG);

/**
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

/**
 * @brief Resolve license and copyright for one file.
 *
 * @param[in]  hTree                Project handle, or NULLHANDLE.
 * @param[in]  pszFullPath          File to resolve. Not NULL.
 * @param[in]  pszDefaultLicense    CLI fallback, or NULL.
 * @param[in]  pszDefaultCopyright  CLI fallback, or NULL.
 * @param[out] pOut                 Result receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFullPath or pOut is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No source matched.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseResolveLicense(HREUSETREE hTree,
                                    PCSZ pszFullPath,
                                    PCSZ pszDefaultLicense,
                                    PCSZ pszDefaultCopyright,
                                    PREUSELICENSEINFO pOut) {
    PSZ pszSideLicense = NULL;
    PSZ pszSideCopyright = NULL;
    PSZ pszTagLicense = NULL;
    PSZ pszTagCopyright = NULL;
    HREUSETREEFILE hFile = NULLHANDLE;
    BOOL fHasReuse = FALSE;
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

    /* 1. Read in-file sources. They are needed by the resolver for
     *    aggregation. Even if a matched annotation has override
     *    precedence, reading them is safe: override ignores in-file
     *    sources. */
    read_sidecar(pszFullPath, &pszSideLicense, &pszSideCopyright);
    read_tags(pszFullPath, &pszTagLicense, &pszTagCopyright);

    /* 2. Resolve REUSE.toml + in-file sources. The precedence rules
     *    (override > aggregate > closest) and aggregation are handled
     *    inside the resolver (see reuse.c). */
    if (hTree != NULLHANDLE) {
        rc = ReuseTreeResolveFile(hTree, pszFullPath, &hFile, NULL);
        if (rc != NO_ERROR) hFile = NULLHANDLE;
    }

    /* 3. Copy fields from the resolver handle. */
    if (hFile != NULLHANDLE) {
        copy_file_field(hFile, ReuseTreeFileGetLicense,
                        pOut->achLicense, sizeof(pOut->achLicense),
                        &pOut->fLicenseTruncated);
        copy_file_field(hFile, ReuseTreeFileGetCopyright,
                        pOut->achCopyright, sizeof(pOut->achCopyright),
                        &pOut->fCopyrightTruncated);
        copy_file_field(hFile, ReuseTreeFileGetContributors,
                        pOut->achContributors, sizeof(pOut->achContributors),
                        &pOut->fContributorsTruncated);
        copy_file_field(hFile, ReuseTreeFileGetPackageName,
                        pOut->achPackageName, sizeof(pOut->achPackageName),
                        &pOut->fPackageNameTruncated);
        copy_file_field(hFile, ReuseTreeFileGetPackageSupplier,
                        pOut->achPackageSupplier,
                        sizeof(pOut->achPackageSupplier),
                        &pOut->fPackageSupplierTruncated);
        copy_file_field(hFile, ReuseTreeFileGetPackageDownloadLocation,
                        pOut->achPackageDownloadLocation,
                        sizeof(pOut->achPackageDownloadLocation),
                        &pOut->fPackageDownloadLocationTruncated);
        copy_file_field(hFile, ReuseTreeFileGetPackageComment,
                        pOut->achPackageComment,
                        sizeof(pOut->achPackageComment),
                        &pOut->fPackageCommentTruncated);

        if (ReuseTreeFileGetPrecedence(hFile, &ulPrecedence)
                != NO_ERROR)
            ulPrecedence = 0;
        if (ReuseTreeFileGetHasReuse(hFile, &fHasReuse) != NO_ERROR)
            fHasReuse = FALSE;

        ReuseTreeFileClose(hFile);
    }

    if (pOut->achPackageName[0] || pOut->achPackageSupplier[0] ||
        pOut->achPackageDownloadLocation[0] || pOut->achPackageComment[0]) {
        pOut->fHasPackageInfo = TRUE;
    }

    /* 4. Determine the source.
     *
     * The resolver's bHasReuse is TRUE if any source matched, including
     * sidecar and in-file tags. To attribute a specific origin, prefer
     * the more specific sidecar/tag over a REUSE.toml annotation when
     * sidecar or tag content was found, and fall back to REUSE when
     * only a config matched. */
    if (pszSideLicense || pszSideCopyright) {
        pOut->source = REUSE_LICENSE_SRC_SIDECAR;
    } else if (pszTagLicense || pszTagCopyright) {
        pOut->source = REUSE_LICENSE_SRC_TAG;
    } else if (fHasReuse) {
        pOut->source = REUSE_LICENSE_SRC_REUSE;
    }

    /* 5. Fallback to --default-*. */
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

    /* 6. Normalize the SPDX expression (canonical case). */
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

    /* 7. Release temporaries. */
    if (pszSideLicense) free(pszSideLicense);
    if (pszSideCopyright) free(pszSideCopyright);
    if (pszTagLicense) free(pszTagLicense);
    if (pszTagCopyright) free(pszTagCopyright);

    if (pOut->achLicense[0] == '\0' && pOut->achCopyright[0] == '\0')
        return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}
