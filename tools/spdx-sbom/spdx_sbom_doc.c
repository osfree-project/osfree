/* spdx_sbom_doc.c - SpdxDocument construction (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "spdx_sbom_doc.h"
#include "spdx_sbom_utils.h"
#include "spdx_sbom_extracted.h"
#include "ccl.h"
#include "spdx.h"
#include "spdx_db.h"
#include "sha1.h"

/**
 * @file spdx_sbom_doc.c
 * @brief Implementation of the SBOM document construction.
 */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Copy a NUL-terminated string into a fixed buffer.
 *
 * If @p pszSrc is NULL, the destination is set to an empty string.
 * If the source does not fit, it is truncated.
 *
 * @param[out] pszDst     Destination buffer. Not NULL.
 * @param[in]  ulDstSize  Size of pszDst in bytes. Must be > 0.
 * @param[in]  pszSrc     Source string, or NULL.
 */
static void copy_field(PSZ pszDst, ULONG ulDstSize, PCSZ pszSrc) {
    if (!pszSrc) { pszDst[0] = '\0'; return; }
    strncpy(pszDst, pszSrc, ulDstSize - 1);
    pszDst[ulDstSize - 1] = '\0';
}

/**
 * @brief qsort comparator for file entries, by name.
 *
 * @param[in] pA  First entry.
 * @param[in] pB  Second entry.
 *
 * @return Negative, zero or positive per strcmp.
 */
static int cmp_fileinfo(const void *pA, const void *pB) {
    return strcmp(((const SPDXFILEINFO*)pA)->achName,
                  ((const SPDXFILEINFO*)pB)->achName);
}

/* ------------------------------------------------------------------ */
/* Document lifecycle                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Create an empty SBOM document.
 *
 * @param[out] pDoc           Receiver. Not NULL.
 * @param[in]  pszName        Document name. Not NULL.
 * @param[in]  pszVersion     Package version, or NULL.
 * @param[in]  pszSupplier    Package supplier, or NULL.
 * @param[in]  pszCreator     Creator string, or NULL.
 * @param[in]  pszLicense     Package license expression, or NULL.
 * @param[in]  pszCopyright   Package copyright, or NULL.
 * @param[in]  pszPurpose     Primary package purpose, or NULL.
 * @param[in]  pszBinaryFile  Binary artifact path, or NULL.
 * @param[in]  fBinaryMode    TRUE for a binary artifact.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc or pszName is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomCreateDocument(
    SPDXDOCUMENT *pDoc,
    PCSZ pszName,
    PCSZ pszVersion,
    PCSZ pszSupplier,
    PCSZ pszCreator,
    PCSZ pszLicense,
    PCSZ pszCopyright,
    PCSZ pszPurpose,
    PCSZ pszBinaryFile,
    BOOL fBinaryMode)
{
    time_t tNow;
    struct tm *ptm;
    CHAR achDate[32];
    CHAR achSafe[256];
    CHAR achNamespace[512];
    CHAR achFileBase[256];
    CHAR achPkgId[256];
    APIRET rc;

    if (!pDoc || !pszName) return ERROR_INVALID_PARAMETER;

    memset(pDoc, 0, sizeof(*pDoc));

    rc = SbomCreateFileList(&pDoc->hFiles);
    if (rc != NO_ERROR) return rc;

    rc = SbomCreateSnippetList(&pDoc->hSnippets);
    if (rc != NO_ERROR) {
        SbomFreeFileList(pDoc->hFiles);
        return rc;
    }

    rc = SbomCreateRelationshipList(&pDoc->hRelationships);
    if (rc != NO_ERROR) {
        SbomFreeFileList(pDoc->hFiles);
        SbomFreeSnippetList(pDoc->hSnippets);
        return rc;
    }

    rc = SbomCreateExtractedList(&pDoc->hExtractedLicenses);
    if (rc != NO_ERROR) {
        SbomFreeFileList(pDoc->hFiles);
        SbomFreeSnippetList(pDoc->hSnippets);
        SbomFreeRelationshipList(pDoc->hRelationships);
        return rc;
    }

    rc = StrSetCreate(&pDoc->package.hLicenseInfoFromFiles);
    if (rc != NO_ERROR) {
        SbomFreeFileList(pDoc->hFiles);
        SbomFreeSnippetList(pDoc->hSnippets);
        SbomFreeRelationshipList(pDoc->hRelationships);
        SbomFreeExtractedList(pDoc->hExtractedLicenses);
        return rc;
    }

    tNow = time(NULL);
    ptm = gmtime(&tNow);
    if (ptm)
        strftime(achDate, sizeof(achDate), "%Y-%m-%dT%H:%M:%SZ", ptm);
    else
        achDate[0] = '\0';

    copy_field(pDoc->achSpdxVersion, sizeof(pDoc->achSpdxVersion),
               "SPDX-2.3");
    copy_field(pDoc->achDocumentId, sizeof(pDoc->achDocumentId),
               "SPDXRef-DOCUMENT");
    copy_field(pDoc->achCreated, sizeof(pDoc->achCreated), achDate);

    if (pszCreator && pszCreator[0])
        copy_field(pDoc->achCreator, sizeof(pDoc->achCreator),
                   pszCreator);
    else
        copy_field(pDoc->achCreator, sizeof(pDoc->achCreator),
                   "Tool: osFree SPDX SBOM Generator");

    snprintf(pDoc->achDocumentName, sizeof(pDoc->achDocumentName),
             "%s SBOM", pszName);

    rc = SbomSanitizeId(pszName, achSafe, sizeof(achSafe));
    if (rc != NO_ERROR) achSafe[0] = '\0';
    snprintf(achNamespace, sizeof(achNamespace),
             "https://osfree.org/spdxdocs/%s-%ld",
             achSafe, (long)tNow);
    copy_field(pDoc->achDocumentNamespace,
               sizeof(pDoc->achDocumentNamespace), achNamespace);

    copy_field(pDoc->achDataLicense, sizeof(pDoc->achDataLicense),
               "CC0-1.0");

    achFileBase[0] = '\0';
    if (pszBinaryFile) {
        copy_field(achFileBase, sizeof(achFileBase),
                   SpdxGetFileName(pszBinaryFile));
        SbomRemoveExtension(achFileBase);
    }
    if (achFileBase[0] == '\0')
        copy_field(achFileBase, sizeof(achFileBase), "package");

    rc = SbomMakePackageId(achFileBase,
                           fBinaryMode ? NULL : "Source",
                           achPkgId, sizeof(achPkgId));
    if (rc != NO_ERROR) achPkgId[0] = '\0';

    copy_field(pDoc->package.achSpdxId,
               sizeof(pDoc->package.achSpdxId), achPkgId);
    copy_field(pDoc->package.achName,
               sizeof(pDoc->package.achName), pszName);
    if (pszVersion)
        copy_field(pDoc->package.achVersion,
                   sizeof(pDoc->package.achVersion), pszVersion);
    if (pszSupplier)
        copy_field(pDoc->package.achSupplier,
                   sizeof(pDoc->package.achSupplier), pszSupplier);
    if (pszLicense)
        copy_field(pDoc->package.achLicense,
                   sizeof(pDoc->package.achLicense), pszLicense);
    if (pszCopyright)
        copy_field(pDoc->package.achCopyright,
                   sizeof(pDoc->package.achCopyright), pszCopyright);
    if (pszPurpose)
        copy_field(pDoc->package.achPurpose,
                   sizeof(pDoc->package.achPurpose), pszPurpose);

    pDoc->package.fFilesAnalyzed = FALSE;
    pDoc->package.achVerificationCode[0] = '\0';
    pDoc->fHasExternalRef = FALSE;

    return NO_ERROR;
}

/**
 * @brief Release all resources owned by a document.
 *
 * @param[in,out] pDoc  Document. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 */
APIRET APIENTRY SbomFreeDocument(SPDXDOCUMENT *pDoc) {
    if (!pDoc) return ERROR_INVALID_PARAMETER;

    if (pDoc->hFiles != NULLHANDLE) SbomFreeFileList(pDoc->hFiles);
    if (pDoc->hSnippets != NULLHANDLE) SbomFreeSnippetList(pDoc->hSnippets);
    if (pDoc->hRelationships != NULLHANDLE)
        SbomFreeRelationshipList(pDoc->hRelationships);
    if (pDoc->hExtractedLicenses != NULLHANDLE)
        SbomFreeExtractedList(pDoc->hExtractedLicenses);
    if (pDoc->package.hLicenseInfoFromFiles != NULLHANDLE)
        StrSetDestroy(pDoc->package.hLicenseInfoFromFiles);

    pDoc->hFiles = NULLHANDLE;
    pDoc->hSnippets = NULLHANDLE;
    pDoc->hRelationships = NULLHANDLE;
    pDoc->hExtractedLicenses = NULLHANDLE;
    pDoc->package.hLicenseInfoFromFiles = NULLHANDLE;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* External document reference                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Set the external document reference to a source SBOM.
 *
 * @param[in,out] pDoc               Document. Not NULL.
 * @param[in]     pszSourceSbomPath  Path to the source SBOM, or NULL.
 * @param[in]     pszChecksumSha1    SHA-1 hex digest, or NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 */
APIRET APIENTRY SbomSetDocumentExternalReference(
    SPDXDOCUMENT *pDoc,
    PCSZ pszSourceSbomPath,
    PCSZ pszChecksumSha1)
{
    if (!pDoc) return ERROR_INVALID_PARAMETER;
    if (!pszSourceSbomPath || !pszSourceSbomPath[0]) return NO_ERROR;

    copy_field(pDoc->achExternalDocId, sizeof(pDoc->achExternalDocId),
               "DocumentRef-source");
    copy_field(pDoc->achExternalDocUri, sizeof(pDoc->achExternalDocUri),
               SpdxGetFileName(pszSourceSbomPath));
    if (pszChecksumSha1)
        copy_field(pDoc->achExternalDocChecksum,
                   sizeof(pDoc->achExternalDocChecksum),
                   pszChecksumSha1);

    pDoc->fHasExternalRef = TRUE;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Package verification                                                */
/* ------------------------------------------------------------------ */

/**
 * @brief Compute PackageVerificationCode and license identifiers.
 *
 * @param[in,out] pDoc  Document. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     An internal container is not
 *                                  recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         SHA-1 computation failed.
 */
APIRET APIENTRY SbomComputeVerification(SPDXDOCUMENT *pDoc) {
    ULONG ulFiles = 0, ulIdx;
    SPDXFILEINFO *paSorted;
    size_t cbTotal;
    PSZ pszConcat;
    CHAR achCombined[41];
    APIRET rc;

    if (!pDoc) return ERROR_INVALID_PARAMETER;

    rc = VectorGetCount(pDoc->hFiles, &ulFiles);
    if (rc != NO_ERROR) return rc;

    if (ulFiles == 0) {
        pDoc->package.fFilesAnalyzed = FALSE;
        pDoc->package.achVerificationCode[0] = '\0';
        return NO_ERROR;
    }

    /* 1. Fill PackageLicenseInfoFromFiles. */
    for (ulIdx = 0; ulIdx < ulFiles; ulIdx++) {
        SPDXFILEINFO info;
        HSTRSET hIds = NULLHANDLE;
        HSTRSETENUM hEnum = NULLHANDLE;

        if (VectorGetItem(pDoc->hFiles, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;

        rc = StrSetCreate(&hIds);
        if (rc != NO_ERROR) return rc;

        rc = SpdxExpressionCollectIds(info.achLicense, hIds, NULL);
        if (rc != NO_ERROR) {
            StrSetDestroy(hIds);
            return rc;
        }

        if (StrSetEnumFirst(hIds, &hEnum) == NO_ERROR) {
            do {
                CHAR achId[256];
                if (StrSetEnumGet(hEnum, achId, sizeof(achId), NULL)
                        != NO_ERROR)
                    continue;
                rc = StrSetAdd(pDoc->package.hLicenseInfoFromFiles, achId);
                if (rc != NO_ERROR) {
                    StrSetEnumClose(hEnum);
                    StrSetDestroy(hIds);
                    return rc;
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
        StrSetDestroy(hIds);
    }

    /* 2. Compute PackageVerificationCode: SHA-1 of the sorted
     *    concatenation of file SHA-1 hex digests. */
    paSorted = (SPDXFILEINFO*)malloc(
        (size_t)ulFiles * sizeof(SPDXFILEINFO));
    if (!paSorted) return ERROR_NOT_ENOUGH_MEMORY;

    for (ulIdx = 0; ulIdx < ulFiles; ulIdx++) {
        if (VectorGetItem(pDoc->hFiles, ulIdx, &paSorted[ulIdx],
                          (ULONG)sizeof(SPDXFILEINFO), NULL)
                != NO_ERROR)
            memset(&paSorted[ulIdx], 0, sizeof(SPDXFILEINFO));
    }
    qsort(paSorted, (size_t)ulFiles, sizeof(SPDXFILEINFO), cmp_fileinfo);

    cbTotal = 0;
    for (ulIdx = 0; ulIdx < ulFiles; ulIdx++)
        cbTotal += strlen(paSorted[ulIdx].achSha1);

    pszConcat = (PSZ)malloc(cbTotal + 1);
    if (!pszConcat) {
        free(paSorted);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pszConcat[0] = '\0';
    for (ulIdx = 0; ulIdx < ulFiles; ulIdx++)
        strcat(pszConcat, paSorted[ulIdx].achSha1);

    rc = Sha1String(pszConcat, achCombined, sizeof(achCombined), NULL);
    free(pszConcat);
    free(paSorted);

    if (rc != NO_ERROR) return rc;

    memcpy(pDoc->package.achVerificationCode, achCombined, 41);
    pDoc->package.fFilesAnalyzed = TRUE;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Relationships                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Build the relationships list.
 *
 * @param[in,out] pDoc              Document. Not NULL.
 * @param[in]     pszBaseNameNoExt  Base name without extension.
 * @param[in]     fBinaryMode       TRUE for a binary artifact.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL, or pszBaseNameNoExt
 *                                  is NULL in binary mode.
 * @retval ERROR_INVALID_HANDLE     An internal container is not
 *                                  recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomBuildRelationships(
    SPDXDOCUMENT *pDoc,
    PCSZ pszBaseNameNoExt,
    BOOL fBinaryMode)
{
    ULONG ulFiles = 0, ulSnippets = 0;
    ULONG ulIdx;
    SPDXRELATIONSHIP rel;
    APIRET rc;

    if (!pDoc) return ERROR_INVALID_PARAMETER;
    if (fBinaryMode && !pszBaseNameNoExt) return ERROR_INVALID_PARAMETER;

    /* 1. DESCRIBES: document -> package. */
    memset(&rel, 0, sizeof(rel));
    copy_field(rel.achElementId, sizeof(rel.achElementId),
               pDoc->achDocumentId);
    copy_field(rel.achRelatedElement, sizeof(rel.achRelatedElement),
               pDoc->package.achSpdxId);
    copy_field(rel.achRelationshipType, sizeof(rel.achRelationshipType),
               "DESCRIBES");
    rc = SbomAddRelationship(pDoc->hRelationships, &rel);
    if (rc != NO_ERROR) return rc;

    /* 2. GENERATED_FROM (binary mode only). */
    if (fBinaryMode) {
        CHAR achSrcId[256];

        rc = SbomMakePackageId(pszBaseNameNoExt, "Source",
                               achSrcId, sizeof(achSrcId));
        if (rc != NO_ERROR) return rc;

        memset(&rel, 0, sizeof(rel));
        copy_field(rel.achElementId, sizeof(rel.achElementId),
                   pDoc->package.achSpdxId);
        if (pDoc->fHasExternalRef)
            snprintf(rel.achRelatedElement,
                     sizeof(rel.achRelatedElement),
                     "%s:%s", pDoc->achExternalDocId, achSrcId);
        else
            copy_field(rel.achRelatedElement,
                       sizeof(rel.achRelatedElement), achSrcId);
        copy_field(rel.achRelationshipType,
                   sizeof(rel.achRelationshipType), "GENERATED_FROM");
        rc = SbomAddRelationship(pDoc->hRelationships, &rel);
        if (rc != NO_ERROR) return rc;
    }

    /* 3. CONTAINS: package -> file. */
    rc = VectorGetCount(pDoc->hFiles, &ulFiles);
    if (rc != NO_ERROR) return rc;

    for (ulIdx = 0; ulIdx < ulFiles; ulIdx++) {
        SPDXFILEINFO info;
        CHAR achFileId[512];

        if (VectorGetItem(pDoc->hFiles, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;

        snprintf(achFileId, sizeof(achFileId),
                 "SPDXRef-File-%s", info.achName);

        memset(&rel, 0, sizeof(rel));
        copy_field(rel.achElementId, sizeof(rel.achElementId),
                   pDoc->package.achSpdxId);
        copy_field(rel.achRelatedElement,
                   sizeof(rel.achRelatedElement), achFileId);
        copy_field(rel.achRelationshipType,
                   sizeof(rel.achRelationshipType), "CONTAINS");
        rc = SbomAddRelationship(pDoc->hRelationships, &rel);
        if (rc != NO_ERROR) return rc;
    }

    /* 4. CONTAINS: file -> snippet. */
    rc = VectorGetCount(pDoc->hSnippets, &ulSnippets);
    if (rc != NO_ERROR) return rc;

    for (ulIdx = 0; ulIdx < ulSnippets; ulIdx++) {
        SPDXSNIPPETINFO info;
        if (VectorGetItem(pDoc->hSnippets, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;

        memset(&rel, 0, sizeof(rel));
        copy_field(rel.achElementId, sizeof(rel.achElementId),
                   info.achFromFileId);
        copy_field(rel.achRelatedElement, sizeof(rel.achRelatedElement),
                   info.achSpdxId);
        copy_field(rel.achRelationshipType,
                   sizeof(rel.achRelationshipType), "CONTAINS");
        rc = SbomAddRelationship(pDoc->hRelationships, &rel);
        if (rc != NO_ERROR) return rc;
    }

    return NO_ERROR;
}
