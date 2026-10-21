/*!
 * @file spdx_tag.c
 *
 * @brief Implementation of the SPDX 2.3 tag-value parser.
 *
 * SPDX 2.3 tag-value parser (C89).
 *
 * Provides line-level parsing (SpdxQueryTagLine) with static
 * metadata (SpdxQueryTagInfo). Comment stripping is the caller's
 * responsibility (see ReuseQueryCommentBody in the REUSE library).
 *
 * Recognized forms:
 *
 *   - Clause 6 through Clause 13 canonical tag-value.
 *   - Annex E: "SPDX-License-Identifier:".
 *   - Annex H: "SPDX-" + Clause 8 field name.
 *   - Annex H.3: "SPDX-SnippetBegin", "SPDX-SnippetEnd", and
 *     "SPDX-" + Clause 9 field name.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_tag.h"

/* ------------------------------------------------------------------ */
/* BOM skipping                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Skip a UTF-8 BOM at the start of the buffer.
 *
 * @param[in] pszPos  Buffer. Not NULL.
 *
 * @return PCSZ
 *
 * @return Pointer past the BOM, or @p pszPos if absent.
 */
static PCSZ skip_bom(PCSZ pszPos) {
    if ((UCHAR)pszPos[0] == 0xEF &&
        (UCHAR)pszPos[1] == 0xBB &&
        (UCHAR)pszPos[2] == 0xBF) {
        return pszPos + 3;
    }
    return pszPos;
}

/* ------------------------------------------------------------------ */
/* Tag-value metadata tables                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Static metadata for every non-deprecated SPDX 2.3 tag.
 *
 * Grouped by clause, then Annex E (short-form license identifier),
 * then Annex H (file tags: "SPDX-" + Clause 8 field name), then
 * Annex H.3 (snippet tags: "SPDX-" + Clause 9 field name, and the
 * SPDX-SnippetBegin / SPDX-SnippetEnd delimiters).
 *
 * Annex H forms are listed as separate rows for clarity; they map to
 * the same SPDXTAG value as their canonical Clause 8/9 counterparts
 * (per SPDX 2.3 Annex H.2).
 */
static const SPDXTAGINFO g_TagInfo[] = {
    /* Document Creation Information (Clause 6) */
    { SPDXTAG_DOC_VERSION,              "SPDXVersion:",                    "6.1",  "1.0", NULL, NULL },
    { SPDXTAG_DOC_DATA_LICENSE,         "DataLicense:",                    "6.2",  "1.0", NULL, NULL },
    { SPDXTAG_SPDX_ID,                  "SPDXID:",                         "6.3",  "2.0", NULL, NULL },
    { SPDXTAG_DOC_NAME,                 "DocumentName:",                   "6.4",  "2.0", NULL, NULL },
    { SPDXTAG_DOC_NAMESPACE,            "DocumentNamespace:",              "6.5",  "2.0", NULL, NULL },
    { SPDXTAG_DOC_EXTERNAL_REF,         "ExternalDocumentRef:",            "6.6",  "2.0", NULL, NULL },
    { SPDXTAG_DOC_LICENSE_LIST_VERSION, "LicenseListVersion:",             "6.7",  "1.2", NULL, NULL },
    { SPDXTAG_DOC_CREATOR,              "Creator:",                        "6.8",  "1.0", NULL, NULL },
    { SPDXTAG_DOC_CREATED,              "Created:",                        "6.9",  "1.0", NULL, NULL },
    { SPDXTAG_DOC_CREATOR_COMMENT,      "CreatorComment:",                 "6.10", "1.0", NULL, NULL },
    { SPDXTAG_DOC_COMMENT,              "DocumentComment:",                "6.11", "1.1", NULL, NULL },

    /* Package Information (Clause 7) */
    { SPDXTAG_PKG_NAME,                       "PackageName:",                    "7.1",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_VERSION,                    "PackageVersion:",                 "7.3",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_FILE_NAME,                  "PackageFileName:",                "7.4",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_SUPPLIER,                   "PackageSupplier:",                "7.5",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_ORIGINATOR,                 "PackageOriginator:",              "7.6",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_DOWNLOAD_LOCATION,          "PackageDownloadLocation:",        "7.7",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_FILES_ANALYZED,             "FilesAnalyzed:",                  "7.8",  "2.1", NULL, NULL },
    { SPDXTAG_PKG_VERIFICATION_CODE,          "PackageVerificationCode:",        "7.9",  "1.0", NULL, NULL },
    { SPDXTAG_PKG_CHECKSUM,                   "PackageChecksum:",                "7.10", "1.0", NULL, NULL },
    { SPDXTAG_PKG_HOME_PAGE,                  "PackageHomePage:",                "7.11", "1.2", NULL, NULL },
    { SPDXTAG_PKG_SOURCE_INFO,                "PackageSourceInfo:",              "7.12", "1.0", NULL, NULL },
    { SPDXTAG_PKG_LICENSE_CONCLUDED,          "PackageLicenseConcluded:",        "7.13", "1.0", NULL, NULL },
    { SPDXTAG_PKG_LICENSE_INFO_FROM_FILES,    "PackageLicenseInfoFromFiles:",    "7.14", "1.0", NULL, NULL },
    { SPDXTAG_PKG_LICENSE_DECLARED,           "PackageLicenseDeclared:",         "7.15", "1.0", NULL, NULL },
    { SPDXTAG_PKG_LICENSE_COMMENTS,           "PackageLicenseComments:",         "7.16", "1.0", NULL, NULL },
    { SPDXTAG_PKG_COPYRIGHT_TEXT,             "PackageCopyrightText:",           "7.17", "1.0", NULL, NULL },
    { SPDXTAG_PKG_SUMMARY,                    "PackageSummary:",                 "7.18", "1.0", NULL, NULL },
    { SPDXTAG_PKG_DESCRIPTION,                "PackageDescription:",             "7.19", "1.0", NULL, NULL },
    { SPDXTAG_PKG_COMMENT,                    "PackageComment:",                 "7.20", "2.0", NULL, NULL },
    { SPDXTAG_PKG_EXTERNAL_REF,               "ExternalRef:",                    "7.21", "2.1", NULL, NULL },
    { SPDXTAG_PKG_EXTERNAL_REF_COMMENT,       "ExternalRefComment:",             "7.22", "2.1", NULL, NULL },
    { SPDXTAG_PKG_ATTRIBUTION_TEXT,           "PackageAttributionText:",         "7.23", "2.3", NULL, NULL },

    /* File Information (Clause 8) */
    { SPDXTAG_FILE_NAME,                        "FileName:",                       "8.1",  "1.0", NULL, NULL },
    { SPDXTAG_FILE_TYPE,                        "FileType:",                       "8.3",  "1.0", NULL, NULL },
    { SPDXTAG_FILE_CHECKSUM,                    "FileChecksum:",                   "8.4",  "1.0", NULL, NULL },
    { SPDXTAG_FILE_LICENSE_CONCLUDED,           "LicenseConcluded:",               "8.5",  "1.0", NULL, NULL },
    { SPDXTAG_FILE_LICENSE_INFO_IN_FILE,        "LicenseInfoInFile:",              "8.6",  "1.0", NULL, NULL },
    { SPDXTAG_FILE_LICENSE_COMMENTS,            "LicenseComments:",                "8.7",  "1.0", NULL, NULL },
    { SPDXTAG_COPYRIGHT,                        "FileCopyrightText:",              "8.8",  "1.0", NULL, NULL },
    { SPDXTAG_FILE_COMMENT,                     "FileComment:",                    "8.12", "1.1", NULL, NULL },
    { SPDXTAG_FILE_NOTICE,                      "FileNotice:",                     "8.13", "1.2", NULL, NULL },
    { SPDXTAG_FILE_CONTRIBUTOR,                 "FileContributor:",                "8.14", "1.2", NULL, NULL },
    { SPDXTAG_FILE_ATTRIBUTION_TEXT,            "FileAttributionText:",            "8.15", "2.3", NULL, NULL },

    /* Snippet Information (Clause 9) */
    { SPDXTAG_SNIPPET_ID,                    "SnippetSPDXID:",                  "9.1",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_FROM_FILE,             "SnippetFromFileSPDXID:",          "9.2",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_BYTE_RANGE,            "SnippetByteRange:",               "9.3",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_LINE_RANGE,            "SnippetLineRange:",               "9.4",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_LICENSE_CONCLUDED,     "SnippetLicenseConcluded:",        "9.5",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_LICENSE_INFO,          "LicenseInfoInSnippet:",           "9.6",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_LICENSE_COMMENTS,      "SnippetLicenseComments:",         "9.7",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_COPYRIGHT,             "SnippetCopyrightText:",           "9.8",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_COMMENT,               "SnippetComment:",                 "9.9",  "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_NAME,                  "SnippetName:",                    "9.10", "2.1", NULL, NULL },
    { SPDXTAG_SNIPPET_ATTRIBUTION_TEXT,      "SnippetAttributionText:",         "9.11", "2.3", NULL, NULL },

    /* Other Licensing Information (Clause 10) */
    { SPDXTAG_LICENSE_ID_DEF,                "LicenseID:",                      "10.1", "1.0", NULL, NULL },
    { SPDXTAG_LICENSE_EXTRACTED_TEXT,        "ExtractedText:",                  "10.2", "1.0", NULL, NULL },
    { SPDXTAG_LICENSE_NAME,                  "LicenseName:",                    "10.3", "1.1", NULL, NULL },
    { SPDXTAG_LICENSE_CROSS_REF,             "LicenseCrossReference:",          "10.4", "1.1", NULL, NULL },
    { SPDXTAG_LICENSE_COMMENT,               "LicenseComment:",                 "10.5", "1.1", NULL, NULL },

    /* Relationships (Clause 11) */
    { SPDXTAG_RELATIONSHIP,                  "Relationship:",                   "11.1", "2.0", NULL, NULL },
    { SPDXTAG_RELATIONSHIP_COMMENT,          "RelationshipComment:",            "11.2", "2.0", NULL, NULL },

    /* Annotations (Clause 12) */
    { SPDXTAG_ANNOTATOR,                     "Annotator:",                      "12.1", "2.0", NULL, NULL },
    { SPDXTAG_ANNOTATION_DATE,               "AnnotationDate:",                 "12.2", "2.0", NULL, NULL },
    { SPDXTAG_ANNOTATION_TYPE,               "AnnotationType:",                 "12.3", "2.0", NULL, NULL },
    { SPDXTAG_ANNOTATION_SPDXREF,            "SPDXREF:",                        "12.4", "2.0", NULL, NULL },
    { SPDXTAG_ANNOTATION_COMMENT,            "AnnotationComment:",              "12.5", "2.0", NULL, NULL },

    /* Annex E - short-form license identifier */
    { SPDXTAG_LICENSE_ID,                    "SPDX-License-Identifier:",        "Annex E", "2.0", NULL, NULL },

    /* Annex H - file tags (SPDX- prefixed Clause 8 fields) */
    { SPDXTAG_FILE_NAME,                     "SPDX-FileName:",                  "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_TYPE,                     "SPDX-FileType:",                  "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_CHECKSUM,                 "SPDX-FileChecksum:",              "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_LICENSE_CONCLUDED,        "SPDX-LicenseConcluded:",          "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_LICENSE_INFO_IN_FILE,     "SPDX-LicenseInfoInFile:",         "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_LICENSE_COMMENTS,         "SPDX-LicenseComments:",           "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_COPYRIGHT,                     "SPDX-FileCopyrightText:",         "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_COMMENT,                  "SPDX-FileComment:",               "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_NOTICE,                   "SPDX-FileNotice:",                "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_CONTRIBUTOR,              "SPDX-FileContributor:",           "Annex H", "2.2", NULL, NULL },
    { SPDXTAG_FILE_ATTRIBUTION_TEXT,         "SPDX-FileAttributionText:",       "Annex H", "2.2", NULL, NULL },

    /* Annex H.3 - delimiters */
    { SPDXTAG_SNIPPET_BEGIN,                 "SPDX-SnippetBegin",               "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_END,                   "SPDX-SnippetEnd",                 "Annex H.3", "2.2", NULL, NULL },

    /* Annex H.3 - snippet tags (SPDX- prefixed Clause 9 fields) */
    { SPDXTAG_SNIPPET_ID,                    "SPDX-SnippetSPDXID:",             "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_FROM_FILE,             "SPDX-SnippetFromFileSPDXID:",     "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_BYTE_RANGE,            "SPDX-SnippetByteRange:",          "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_LINE_RANGE,            "SPDX-SnippetLineRange:",          "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_LICENSE_CONCLUDED,     "SPDX-SnippetLicenseConcluded:",   "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_LICENSE_INFO,          "SPDX-LicenseInfoInSnippet:",      "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_LICENSE_COMMENTS,      "SPDX-SnippetLicenseComments:",    "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_COPYRIGHT,             "SPDX-SnippetCopyrightText:",      "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_COMMENT,               "SPDX-SnippetComment:",            "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_NAME,                  "SPDX-SnippetName:",               "Annex H.3", "2.2", NULL, NULL },
    { SPDXTAG_SNIPPET_ATTRIBUTION_TEXT,      "SPDX-SnippetAttributionText:",    "Annex H.3", "2.2", NULL, NULL }
};

/*!
 * @brief Static metadata for every deprecated SPDX 2.3 tag.
 *
 * Includes both the canonical Clause 8/13 form and, where defined by
 * Annex H, the "SPDX-" prefixed form. Both map to the same SPDXTAG
 * value; the canonical entry comes first, so SpdxQueryTagInfo returns
 * the canonical spelling.
 */
static const SPDXTAGINFO g_DeprecatedTagInfo[] = {
    /* Clause 8 deprecated */
    { SPDXTAG_FILE_ARTIFACT_PROJECT_NAME,       "ArtifactOfProjectName:",          "8.9",  "1.0", "1.8.4",
      "Relationship to External Package (DESCRIBED_BY / CONTAINS)" },
    { SPDXTAG_FILE_ARTIFACT_PROJECT_HOME_PAGE,  "ArtifactOfProjectHomePage:",      "8.10", "1.0", "1.8.4",
      "Relationship to External Package (DESCRIBED_BY / CONTAINS)" },
    { SPDXTAG_FILE_ARTIFACT_PROJECT_URI,        "ArtifactOfProjectURI:",           "8.11", "1.0", "1.8.4",
      "Relationship to External Package (DESCRIBED_BY / CONTAINS)" },
    { SPDXTAG_FILE_DEPENDENCY,                  "FileDependency:",                 "8.16", "1.2", "2.0",
      "Relationship: DEPENDENCY_OF or BUILD_DEPENDENCY_OF" },

    /* Clause 13 deprecated */
    { SPDXTAG_REVIEWER,                         "Reviewer:",                       "13.1", "1.0", "2.0",
      "Annotation with annotationType review; use Annotator / AnnotationDate / AnnotationComment" },
    { SPDXTAG_REVIEW_DATE,                      "ReviewDate:",                     "13.2", "1.0", "2.0",
      "Annotation with annotationType review; use AnnotationDate" },
    { SPDXTAG_REVIEW_COMMENT,                   "ReviewComment:",                  "13.3", "1.0", "2.0",
      "Annotation with annotationType review; use AnnotationComment" },

    /* Annex H deprecated */
    { SPDXTAG_FILE_ARTIFACT_PROJECT_NAME,       "SPDX-ArtifactOfProjectName:",     "Annex H", "2.2", "1.8.4",
      "Relationship to External Package (DESCRIBED_BY / CONTAINS)" },
    { SPDXTAG_FILE_ARTIFACT_PROJECT_HOME_PAGE,  "SPDX-ArtifactOfProjectHomePage:", "Annex H", "2.2", "1.8.4",
      "Relationship to External Package (DESCRIBED_BY / CONTAINS)" },
    { SPDXTAG_FILE_ARTIFACT_PROJECT_URI,        "SPDX-ArtifactOfProjectURI:",      "Annex H", "2.2", "1.8.4",
      "Relationship to External Package (DESCRIBED_BY / CONTAINS)" },
    { SPDXTAG_FILE_DEPENDENCY,                  "SPDX-FileDependency:",            "Annex H", "2.2", "2.0",
      "Relationship: DEPENDENCY_OF or BUILD_DEPENDENCY_OF" }
};

/* ------------------------------------------------------------------ */
/* Tag matching helper                                                 */
/* ------------------------------------------------------------------ */

/*!
 * @brief Match a line against one tag from one metadata table.
 *
 * @param[in]  pszPos       Start of the line. Not NULL.
 * @param[in]  pInfo        Tag metadata. Not NULL.
 * @param[out] pcbTagLen    Receives the tag length on match. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   Line begins with the tag (with valid boundary).
 * @retval FALSE  No match.
 */
static BOOL tag_match(PCSZ pszPos, const SPDXTAGINFO *pInfo,
                      size_t *pcbTagLen)
{
    size_t cbTag = strlen(pInfo->pszTag);

    if (cbTag == 0) return FALSE;
    if (strncmp(pszPos, pInfo->pszTag, cbTag) != 0) return FALSE;

    if (pInfo->pszTag[cbTag - 1] != ':') {
        CHAR chAfter = pszPos[cbTag];
        if (chAfter != ':' && chAfter != ' ' && chAfter != '\t' &&
            chAfter != '\0' && chAfter != '\n' && chAfter != '\r')
            return FALSE;
    }

    *pcbTagLen = cbTag;
    return TRUE;
}

/*!
 * @brief Locate a tag in the metadata tables.
 *
 * Non-deprecated entries are checked first, then deprecated ones.
 *
 * @param[in]  pszPos      Start of the line. Not NULL.
 * @param[out] pcbTagLen   Receives the tag length on match. Not NULL.
 *
 * @return const SPDXTAGINFO *
 *
 * @retval NULL   No tag matched.
 * @return        Pointer to the metadata entry on match.
 */
static const SPDXTAGINFO *lookup_tag(PCSZ pszPos, size_t *pcbTagLen)
{
    ULONG ulIdx;
    ULONG ulCount;

    ulCount = (ULONG)(sizeof(g_TagInfo) / sizeof(g_TagInfo[0]));
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        if (tag_match(pszPos, &g_TagInfo[ulIdx], pcbTagLen))
            return &g_TagInfo[ulIdx];
    }

    ulCount = (ULONG)(sizeof(g_DeprecatedTagInfo) /
                      sizeof(g_DeprecatedTagInfo[0]));
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        if (tag_match(pszPos, &g_DeprecatedTagInfo[ulIdx], pcbTagLen))
            return &g_DeprecatedTagInfo[ulIdx];
    }

    return NULL;
}

/* ------------------------------------------------------------------ */
/* Tag-value line parser                                               */
/* ------------------------------------------------------------------ */

/*!
 * @brief Parse one SPDX 2.3 tag-value line.
 *
 * @param[in]  pszLine       Line to parse. Not NULL.
 * @param[out] pTag          Tag identifier. Not NULL.
 * @param[out] pfDeprecated  TRUE if deprecated. Not NULL.
 * @param[out] pfInFileForm  TRUE for Annex E/H/H.3 "SPDX-..." forms.
 *                           Not NULL.
 * @param[out] pszValue      Output buffer, or NULL for size query.
 * @param[in]  ulSize        Size of pszValue in bytes.
 * @param[out] pulUsed       Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Parse completed.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_BUFFER_OVERFLOW    pszValue too small.
 */
APIRET APIENTRY SpdxQueryTagLine(PCSZ pszLine, SPDXTAG *pTag,
                                 PBOOL pfDeprecated,
                                 PBOOL pfInFileForm,
                                 PSZ pszValue, ULONG ulSize,
                                 PULONG pulUsed)
{
    PCSZ pszPos;
    PCSZ pszColon;
    PCSZ pszValueStart;
    const SPDXTAGINFO *pInfo;
    size_t cbTagLen = 0;
    size_t cbValueLen;

    if (!pszLine || !pTag || !pfDeprecated || !pfInFileForm)
        return ERROR_INVALID_PARAMETER;
    if (pszValue != NULL && ulSize == 0)
        return ERROR_INVALID_PARAMETER;
    if (pszValue == NULL && ulSize != 0)
        return ERROR_INVALID_PARAMETER;

    *pTag = SPDXTAG_NONE;
    *pfDeprecated = FALSE;
    *pfInFileForm = FALSE;

    pszPos = skip_bom(pszLine);
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;

    if (*pszPos == '\0' || *pszPos == '\n' || *pszPos == '\r') {
        if (pszValue != NULL && ulSize > 0) pszValue[0] = '\0';
        if (pulUsed) *pulUsed = 1;
        return NO_ERROR;
    }

    pInfo = lookup_tag(pszPos, &cbTagLen);
    if (pInfo != NULL) {
        *pTag = pInfo->tag;
        *pfDeprecated = (pInfo->pszDeprecated != NULL) ? TRUE : FALSE;
        *pfInFileForm = (strncmp(pInfo->pszTag, "SPDX-", 5) == 0)
                            ? TRUE : FALSE;

        pszPos += cbTagLen;
        if (pszPos[0] == ':') pszPos++;
        while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
        pszValueStart = pszPos;
    } else {
        *pTag = SPDXTAG_OTHER;
        *pfDeprecated = FALSE;
        *pfInFileForm = FALSE;
        pszColon = strchr(pszPos, ':');
        if (pszColon) {
            pszPos = pszColon + 1;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
        }
        pszValueStart = pszPos;
    }

    cbValueLen = 0;
    while (pszValueStart[cbValueLen] != '\0' &&
           pszValueStart[cbValueLen] != '\n' &&
           pszValueStart[cbValueLen] != '\r')
        cbValueLen++;
    while (cbValueLen > 0 &&
           (pszValueStart[cbValueLen - 1] == ' ' ||
            pszValueStart[cbValueLen - 1] == '\t'))
        cbValueLen--;

    if (pszValue == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbValueLen + 1;
        return NO_ERROR;
    }
    if (ulSize < (ULONG)cbValueLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbValueLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszValue, pszValueStart, cbValueLen);
    pszValue[cbValueLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbValueLen;
    return NO_ERROR;
}

/*!
 * @brief Query static metadata for a tag.
 *
 * @param[in]  tag    Tag identifier.
 * @param[out] pInfo  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Metadata found.
 * @retval ERROR_INVALID_PARAMETER  pInfo is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No metadata for the tag.
 */
APIRET APIENTRY SpdxQueryTagInfo(SPDXTAG tag, SPDXTAGINFO *pInfo)
{
    ULONG ulIdx;
    ULONG ulCount;

    if (!pInfo) return ERROR_INVALID_PARAMETER;

    ulCount = (ULONG)(sizeof(g_TagInfo) / sizeof(g_TagInfo[0]));
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        if (g_TagInfo[ulIdx].tag == tag) {
            *pInfo = g_TagInfo[ulIdx];
            return NO_ERROR;
        }
    }

    ulCount = (ULONG)(sizeof(g_DeprecatedTagInfo) /
                      sizeof(g_DeprecatedTagInfo[0]));
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        if (g_DeprecatedTagInfo[ulIdx].tag == tag) {
            *pInfo = g_DeprecatedTagInfo[ulIdx];
            return NO_ERROR;
        }
    }

    return ERROR_FILE_NOT_FOUND;
}
