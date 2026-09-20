/* spdx_sbom_out_tag.c - SBOM output in tag-value format (C89) */

#include <stdio.h>
#include <string.h>
#include "spdx_sbom_out.h"
#include "ccl.h"

/**
 * @file spdx_sbom_out_tag.c
 * @brief SBOM serialization to SPDX 2.3 tag-value.
 *
 * Conforms to:
 *   - SPDX 2.3, tag-value serialization.
 *     https://spdx.github.io/spdx-spec/v2.3/
 *
 * The finished document is written to stdout. This is the
 * functional output of the serializer.
 */

/**
 * @brief Emit a field in "<Tag>: <value>" form.
 *
 * Multi-line values are wrapped in <text>...</text> as required by
 * SPDX 2.3. Trailing newlines are removed before wrapping.
 *
 * @param[in] pszTag    Tag name. Not NULL.
 * @param[in] pszValue  Value, or NULL (treated as "").
 */
static void print_text_field(PCSZ pszTag, PCSZ pszValue) {
    PCSZ pszEnd;
    size_t cbLen;

    if (!pszValue) pszValue = "";
    if (strchr(pszValue, '\n') == NULL &&
        strchr(pszValue, '\r') == NULL) {
        printf("%s: %s\n", pszTag, pszValue);
        return;
    }
    cbLen = strlen(pszValue);
    pszEnd = pszValue + cbLen;
    while (pszEnd > pszValue &&
           (pszEnd[-1] == '\n' || pszEnd[-1] == '\r'))
        pszEnd--;

    printf("%s: <text>", pszTag);
    fwrite(pszValue, 1, (size_t)(pszEnd - pszValue), stdout);
    printf("</text>\n");
}

/**
 * @brief Serialize a document as SPDX 2.3 tag-value to stdout.
 *
 * @param[in] pDoc  Document to serialize. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     An internal container is not
 *                                  recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomOutputTagValue(const SPDXDOCUMENT *pDoc) {
    const SPDXPACKAGEINFO *pPkg;
    ULONG ulCount = 0, ulIdx;
    APIRET rc;

    if (!pDoc) return ERROR_INVALID_PARAMETER;

    pPkg = &pDoc->package;

    printf("SPDXVersion: %s\n", pDoc->achSpdxVersion);
    printf("DataLicense: %s\n", pDoc->achDataLicense);
    printf("SPDXID: %s\n", pDoc->achDocumentId);
    printf("DocumentName: %s\n", pDoc->achDocumentName);
    printf("DocumentNamespace: %s\n", pDoc->achDocumentNamespace);
    printf("Creator: %s\n", pDoc->achCreator);
    printf("Created: %s\n", pDoc->achCreated);
    printf("\n");

    if (pDoc->fHasExternalRef) {
        printf("ExternalDocumentRef: %s %s SHA1: %s\n",
               pDoc->achExternalDocId, pDoc->achExternalDocUri,
               pDoc->achExternalDocChecksum);
        printf("\n");
    }

    if (pDoc->hExtractedLicenses != NULLHANDLE) {
        rc = VectorGetCount(pDoc->hExtractedLicenses, &ulCount);
        if (rc != NO_ERROR) return rc;
        for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
            SPDXEXTRACTEDLICENSEINFO info;
            if (VectorGetItem(pDoc->hExtractedLicenses, ulIdx, &info,
                              (ULONG)sizeof(info), NULL) != NO_ERROR)
                continue;
            printf("LicenseID: %s\n",
                   info.pszLicenseId ? info.pszLicenseId : "");
            print_text_field("ExtractedText",
                             info.pszExtractedText
                                 ? info.pszExtractedText : "");
            print_text_field("LicenseName",
                             (info.pszName && info.pszName[0])
                                 ? info.pszName : "NOASSERTION");
            print_text_field("LicenseComment",
                             (info.pszComment && info.pszComment[0])
                                 ? info.pszComment : "NOASSERTION");
            printf("\n");
        }
    }

    printf("##### Package: %s\n", pPkg->achName);
    printf("PackageName: %s\n", pPkg->achName);
    printf("SPDXID: %s\n", pPkg->achSpdxId);
    printf("PackageDownloadLocation: NOASSERTION\n");
    if (pPkg->achVersion[0])
        printf("PackageVersion: %s\n", pPkg->achVersion);
    if (pPkg->achSupplier[0])
        printf("PackageSupplier: %s\n", pPkg->achSupplier);
    printf("PackageLicenseConcluded: %s\n", pPkg->achLicense);
    printf("PackageLicenseDeclared: %s\n", pPkg->achLicense);

    if (pPkg->fFilesAnalyzed &&
        pPkg->hLicenseInfoFromFiles != NULLHANDLE) {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(pPkg->hLicenseInfoFromFiles, &hEnum)
                == NO_ERROR) {
            do {
                CHAR achLic[256];
                if (StrSetEnumGet(hEnum, achLic, sizeof(achLic), NULL)
                        != NO_ERROR)
                    continue;
                printf("PackageLicenseInfoFromFiles: %s\n", achLic);
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }
    if (pPkg->achCopyright[0])
        print_text_field("PackageCopyrightText", pPkg->achCopyright);
    if (pPkg->achPurpose[0])
        printf("PrimaryPackagePurpose: %s\n", pPkg->achPurpose);
    if (pPkg->fFilesAnalyzed) {
        printf("FilesAnalyzed: true\n");
        printf("PackageVerificationCode: %s\n",
               pPkg->achVerificationCode);
    } else {
        printf("FilesAnalyzed: false\n");
    }
    printf("\n");

    rc = VectorGetCount(pDoc->hFiles, &ulCount);
    if (rc != NO_ERROR) return rc;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        SPDXFILEINFO info;
        if (VectorGetItem(pDoc->hFiles, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;
        printf("FileName: %s\n", info.achName);
        printf("SPDXID: SPDXRef-File-%s\n", info.achName);
        printf("FileType: %s\n", info.achFileType);
        printf("FileChecksum: SHA1: %s\n", info.achSha1);
        printf("LicenseConcluded: %s\n", info.achLicense);
        printf("LicenseInfoInFile: %s\n", info.achLicense);
        if (info.achCopyright[0])
            print_text_field("FileCopyrightText", info.achCopyright);
        printf("\n");
    }

    rc = VectorGetCount(pDoc->hSnippets, &ulCount);
    if (rc != NO_ERROR) return rc;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        SPDXSNIPPETINFO info;
        if (VectorGetItem(pDoc->hSnippets, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;
        printf("SnippetSPDXID: %s\n", info.achSpdxId);
        printf("SnippetFromFileSPDXID: %s\n", info.achFromFileId);
        printf("SnippetLineRange: %lu:%lu\n",
               (unsigned long)info.ulLineStart,
               (unsigned long)info.ulLineEnd);
        printf("SnippetLicenseConcluded: %s\n", info.achLicense);
        printf("LicenseInfoInSnippet: %s\n", info.achLicense);
        if (info.achCopyright[0])
            print_text_field("SnippetCopyrightText", info.achCopyright);
        printf("\n");
    }

    rc = VectorGetCount(pDoc->hRelationships, &ulCount);
    if (rc != NO_ERROR) return rc;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        SPDXRELATIONSHIP rel;
        if (VectorGetItem(pDoc->hRelationships, ulIdx, &rel,
                          (ULONG)sizeof(rel), NULL) != NO_ERROR)
            continue;
        printf("Relationship: %s %s %s\n",
               rel.achElementId, rel.achRelationshipType,
               rel.achRelatedElement);
    }
    printf("\n");
    return NO_ERROR;
}
