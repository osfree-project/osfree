/*!
 * @file spdx_tag.c
 *
 * @brief Implementation of the SPDX 2.3 tag-value parser.
 *
 * SPDX 2.3 tag-value parser (C89).
 *
 * Two layers coexist:
 *
 *   - Line-level parsing (SpdxQueryTagLine) with static metadata
 *     (SpdxQueryTagInfo). Used by the REUSE library (reuse_tags.c)
 *     for comment headers.
 *
 *   - Legacy file-level helpers (SpdxQueryFileHasTag,
 *     SpdxQueryFileLicense, SpdxQueryFileCopyright,
 *     SpdxQueryFileSnippets) that open the file themselves and apply
 *     a hard-coded comment prefix table. Deprecated in favour of
 *     ReuseQueryFile*; removed once the REUSE migration is complete.
 *
 * Recognized forms:
 *
 *   - Clause 6 through Clause 13 canonical tag-value.
 *   - Annex E: "SPDX-License-Identifier:".
 *   - Annex H: "SPDX-" + Clause 8 field name, e.g.
 *     "SPDX-FileCopyrightText:".
 *   - Annex H.3: "SPDX-SnippetBegin", "SPDX-SnippetEnd", and
 *     "SPDX-" + Clause 9 field name.
 *
 * Per SPDX 2.3 Annex H.2, "SPDX-<Field>:" carries the same semantics
 * as "<Field>:" from Clause 8/9. Both forms therefore map to the same
 * SPDXTAG value, and SpdxQueryTagInfo returns the canonical Clause
 * 8/9 spelling ("FileCopyrightText:") regardless of which form was
 * seen in the file. SpdxQueryTagLine reports the distinction through
 * its pfInFileForm output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_tag.h"

/*!
 * @def MAX_LINE
 * @brief Maximum line length accepted by the legacy file scanner.
 */
#define MAX_LINE 16384

/* ------------------------------------------------------------------ */
/* Legacy static helpers (used by SpdxQueryFile*)                     */
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

/*!
 * @brief Skip leading whitespace and a comment marker.
 *
 * @deprecated Legacy: to be replaced by ReuseQueryCommentBody.
 *
 * @param[in] pszPos  Start of line. Not NULL.
 *
 * @return PCSZ
 *
 * @return Pointer past the comment prefix.
 */
static PCSZ skip_comment_prefix(PCSZ pszPos) {
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    if (pszPos[0] == '/' && pszPos[1] == '/') { pszPos += 2; }
    else if (pszPos[0] == '/' && pszPos[1] == '*') { pszPos += 2; }
    else if (pszPos[0] == '#') { pszPos++; }
    else if (pszPos[0] == ';') { pszPos++; }
    else if (pszPos[0] == '%') { pszPos++; }
    else if (pszPos[0] == '-' && pszPos[1] == '-') { pszPos += 2; }
    else if (pszPos[0] == '*') { pszPos++; }
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    return pszPos;
}

/*!
 * @brief Check whether the meaningful part of a line starts with
 *        REUSE-IgnoreStart or REUSE-IgnoreEnd.
 *
 * @deprecated Legacy: to be replaced by REUSE-side handling.
 *
 * @param[in] pszLine  Line. Not NULL.
 *
 * @return int
 *
 * @retval 1  Line begins with REUSE-IgnoreStart.
 * @retval 2  Line begins with REUSE-IgnoreEnd.
 * @retval 0  Neither marker.
 */
static int line_ignore_marker(PCSZ pszLine) {
    PCSZ pszPos = skip_comment_prefix(pszLine);
    if (strncmp(pszPos, "REUSE-IgnoreStart", 17) == 0) {
        CHAR ch = pszPos[17];
        if (ch == '\0' || ch == ' ' || ch == '\t' ||
            ch == '\n' || ch == '\r')
            return 1;
    }
    if (strncmp(pszPos, "REUSE-IgnoreEnd", 15) == 0) {
        CHAR ch = pszPos[15];
        if (ch == '\0' || ch == ' ' || ch == '\t' ||
            ch == '\n' || ch == '\r')
            return 2;
    }
    return 0;
}

/*!
 * @brief Check whether the meaningful part of a line starts with the
 *        given tag.
 *
 * @deprecated Legacy: to be replaced by SpdxQueryTagLine.
 *
 * @param[in] pszLine  Line. Not NULL.
 * @param[in] pszTag   Tag. Not NULL.
 *
 * @return int
 *
 * @retval 1  Tag present at the start of the meaningful part.
 * @retval 0  No match.
 */
static int line_starts_with_tag(PCSZ pszLine, PCSZ pszTag) {
    size_t tlen = strlen(pszTag);
    PCSZ pszPos = skip_comment_prefix(pszLine);
    if (strncmp(pszPos, pszTag, tlen) != 0) return 0;
    if (pszPos[tlen] == ':' && pszTag[tlen - 1] != ':') return 1;
    if (pszPos[tlen] == '\0' || pszPos[tlen] == ' ' ||
        pszPos[tlen] == '\t' || pszPos[tlen] == '\n' ||
        pszPos[tlen] == '\r')
        return 1;
    return 0;
}

/*!
 * @brief Return a pointer to the value after the tag.
 *
 * @deprecated Legacy: to be replaced by SpdxQueryTagLine.
 *
 * @param[in] pszLine          Line. Not NULL.
 * @param[in] pszTagWithColon  Tag with trailing ':'. Not NULL.
 *
 * @return PCSZ
 *
 * @retval NULL  Tag not found at the start of the line.
 * @return       Pointer to the value past the tag and its spacing.
 */
static PCSZ tag_value(PCSZ pszLine, PCSZ pszTagWithColon) {
    size_t tlen = strlen(pszTagWithColon);
    PCSZ pszPos = skip_comment_prefix(pszLine);
    if (strncmp(pszPos, pszTagWithColon, tlen) != 0) return NULL;
    pszPos += tlen;
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    return pszPos;
}

/*!
 * @brief Strip trailing whitespace, newlines and comment-closing
 *        markers.
 *
 * @deprecated Legacy: to be folded into ReuseQueryCommentBody.
 *
 * @param[in,out] pszStr  String to modify. Not NULL.
 */
static void strip_trailing_markers(PSZ pszStr) {
    size_t cbLen = strlen(pszStr);
    while (cbLen > 0) {
        if (cbLen >= 2 && pszStr[cbLen-2] == '*' && pszStr[cbLen-1] == '/') {
            cbLen -= 2;
        } else if (cbLen >= 2 && pszStr[cbLen-2] == '/' &&
                   pszStr[cbLen-1] == '/') {
            cbLen -= 2;
        } else if (pszStr[cbLen-1] == ' ' || pszStr[cbLen-1] == '\t' ||
                   pszStr[cbLen-1] == '\n' || pszStr[cbLen-1] == '\r') {
            cbLen -= 1;
        } else {
            break;
        }
    }
    pszStr[cbLen] = '\0';
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
/* Snippet list                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Initialize a snippet list.
 *
 * @param[in] pList  List. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList is NULL.
 */
APIRET APIENTRY SpdxSnippetListInit(SPDXSNIPPETLIST *pList) {
    if (!pList) return ERROR_INVALID_PARAMETER;
    pList->pItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return NO_ERROR;
}

/*!
 * @brief Release all memory owned by a snippet list.
 *
 * @param[in] pList  List. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR  Success. Also for NULL.
 */
APIRET APIENTRY SpdxSnippetListFree(SPDXSNIPPETLIST *pList) {
    ULONG ulIdx;
    if (!pList) return NO_ERROR;
    for (ulIdx = 0; ulIdx < pList->ulCount; ulIdx++) {
        free(pList->pItems[ulIdx].pszLicense);
        free(pList->pItems[ulIdx].pszCopyright);
    }
    free(pList->pItems);
    pList->pItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return NO_ERROR;
}

/*!
 * @brief Append a zero-filled entry to a snippet list.
 *
 * @param[in,out] pList    List. Not NULL.
 * @param[out]    ppEntry  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList or ppEntry is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET snippetlist_add(SPDXSNIPPETLIST *pList,
                              PSPDXSNIPPET *ppEntry) {
    PSPDXSNIPPET pNew;
    *ppEntry = NULL;
    if (pList->ulCount >= pList->ulCapacity) {
        ULONG ulNewCap = pList->ulCapacity ? pList->ulCapacity * 2 : 4;
        pNew = (PSPDXSNIPPET)realloc(pList->pItems,
            (size_t)ulNewCap * sizeof(SPDXSNIPPET));
        if (!pNew) return ERROR_NOT_ENOUGH_MEMORY;
        pList->pItems = pNew;
        pList->ulCapacity = ulNewCap;
    }
    *ppEntry = &pList->pItems[pList->ulCount++];
    memset(*ppEntry, 0, sizeof(**ppEntry));
    return NO_ERROR;
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

/* ------------------------------------------------------------------ */
/* Legacy file-level queries                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Query whether a file contains an SPDX-License-Identifier
 *        tag.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pfHasTag     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename or pfHasTag is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
APIRET APIENTRY SpdxQueryFileHasTag(PCSZ pszFilename, PBOOL pfHasTag) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    BOOL fIgnore = FALSE;
    BOOL fFirstLine = TRUE;

    if (!pszFilename || !pfHasTag) return ERROR_INVALID_PARAMETER;
    *pfHasTag = FALSE;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;
    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszWork = achLine;
        int nMarker;

        if (fFirstLine) {
            pszWork = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE;
        }

        nMarker = line_ignore_marker(pszWork);
        if (nMarker == 1) { fIgnore = TRUE; continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        if (strstr(pszWork, "SPDX-License-Identifier:")) {
            fclose(fp);
            *pfHasTag = TRUE;
            return NO_ERROR;
        }
    }
    fclose(fp);
    return NO_ERROR;
}

/*!
 * @brief Query the value of the SPDX-License-Identifier tag.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     Tag not present.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryFileLicense(PCSZ pszFilename, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    PCSZ pszNeedle = "SPDX-License-Identifier:";
    size_t cbNeedle = strlen(pszNeedle);
    PSZ pszPos;
    PSZ pszStart;
    size_t cbLen;
    BOOL fIgnore = FALSE;
    BOOL fFirstLine = TRUE;

    if (!pszFilename) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszWork = achLine;
        int nMarker;

        if (fFirstLine) {
            pszWork = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE;
        }

        nMarker = line_ignore_marker(pszWork);
        if (nMarker == 1) { fIgnore = TRUE; continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        pszPos = strstr(pszWork, pszNeedle);
        if (pszPos) {
            pszStart = (PSZ)skip_comment_prefix(pszPos + cbNeedle);
            strip_trailing_markers(pszStart);
            cbLen = strlen(pszStart);
            if (cbLen > 0) {
                if (pszBuf == NULL && ulSize == 0) {
                    if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
                    fclose(fp);
                    return NO_ERROR;
                }
                if (ulSize < (ULONG)cbLen + 1) {
                    if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
                    fclose(fp);
                    return ERROR_BUFFER_OVERFLOW;
                }
                memcpy(pszBuf, pszStart, cbLen);
                pszBuf[cbLen] = '\0';
                if (pulUsed) *pulUsed = (ULONG)cbLen;
                fclose(fp);
                return NO_ERROR;
            }
        }
    }
    fclose(fp);
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Query the concatenated SPDX-FileCopyrightText values.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No copyright tag present.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxQueryFileCopyright(PCSZ pszFilename, PSZ pszBuf,
                                       ULONG ulSize, PULONG pulUsed) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    PSZ pszResult = NULL;
    size_t cbResultLen = 0;
    BOOL fIgnore = FALSE;
    BOOL fFirstLine = TRUE;

    if (!pszFilename) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszPos;
        PSZ pszStart;
        size_t cbLen;
        int nMarker;

        if (fFirstLine) {
            pszPos = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE;
        } else {
            pszPos = achLine;
        }

        nMarker = line_ignore_marker(pszPos);
        if (nMarker == 1) { fIgnore = TRUE; continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        pszPos = (PSZ)skip_comment_prefix(pszPos);

        if (strncmp(pszPos, "SPDX-FileCopyrightText:", 23) == 0) {
            pszPos += 23;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
            pszStart = pszPos;
        } else if (strncmp(pszPos, "Copyright", 9) == 0 &&
                   (pszPos[9] == ' ' || pszPos[9] == '\t')) {
            pszPos += 9;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
            pszStart = pszPos;
        } else if ((UCHAR)*pszPos == 0xC2 &&
                   (UCHAR)*(pszPos+1) == 0xA9) {
            pszPos += 2;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
            pszStart = pszPos;
        } else {
            continue;
        }

        strip_trailing_markers(pszStart);
        cbLen = strlen(pszStart);
        if (cbLen > 0) {
            PSZ pszNew;
            size_t cbNewSize = (pszResult ? cbResultLen + 1 + cbLen + 1
                                          : cbLen + 1);
            pszNew = (PSZ)realloc(pszResult, cbNewSize);
            if (!pszNew) {
                free(pszResult);
                fclose(fp);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            pszResult = pszNew;
            if (cbResultLen > 0) {
                pszResult[cbResultLen++] = '\n';
            }
            memcpy(pszResult + cbResultLen, pszStart, cbLen);
            cbResultLen += cbLen;
            pszResult[cbResultLen] = '\0';
        }
    }
    fclose(fp);

    if (!pszResult) return ERROR_FILE_NOT_FOUND;

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbResultLen + 1;
        free(pszResult);
        return NO_ERROR;
    }
    if (ulSize < (ULONG)cbResultLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbResultLen + 1;
        free(pszResult);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszResult, cbResultLen + 1);
    if (pulUsed) *pulUsed = (ULONG)cbResultLen;
    free(pszResult);
    return NO_ERROR;
}

/*!
 * @brief Append a line to an accumulator with a '\n' separator.
 *
 * @param[in] pszAcc      Accumulator, or NULL.
 * @param[in] pcbAccLen   Pointer to the used length. Not NULL.
 * @param[in] pszLine     Line to append. Not NULL.
 *
 * @return PSZ
 *
 * @retval NULL  Allocation failed.
 * @return       New accumulator.
 */
static PSZ append_line(PSZ pszAcc, size_t *pcbAccLen, PCSZ pszLine) {
    size_t cbLineLen = strlen(pszLine);
    PSZ pszNew;
    size_t cbNewSize;

    if (cbLineLen == 0) return pszAcc;

    cbNewSize = (*pcbAccLen ? *pcbAccLen + 1 : 0) + cbLineLen + 1;
    pszNew = (PSZ)realloc(pszAcc, cbNewSize);
    if (!pszNew) return pszAcc;

    if (*pcbAccLen > 0) {
        pszNew[(*pcbAccLen)++] = '\n';
    }
    memcpy(pszNew + *pcbAccLen, pszLine, cbLineLen);
    *pcbAccLen += cbLineLen;
    pszNew[*pcbAccLen] = '\0';
    return pszNew;
}

/*!
 * @brief Extract all SPDX snippets from a file.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pOut         List receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_INVALID_PARAMETER  pszFilename or pOut is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Nested or unmatched snippet
 *                                  delimiters.
 */
APIRET APIENTRY SpdxQueryFileSnippets(PCSZ pszFilename,
                                      SPDXSNIPPETLIST *pOut) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    ULONG ulLineNo = 0;
    BOOL fFirstLine = TRUE;
    BOOL fInSnippet = FALSE;
    ULONG ulSnippetStartLine = 0;
    PSZ pszSnippetLicense = NULL;
    PSZ pszSnippetCopyright = NULL;
    size_t cbSnippetCopyrightLen = 0;
    BOOL fIgnore = FALSE;
    APIRET rc;

    if (!pszFilename || !pOut) return ERROR_INVALID_PARAMETER;

    rc = SpdxSnippetListInit(pOut);
    if (rc != NO_ERROR) return rc;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszWork = achLine;
        int nMarker;

        ulLineNo++;

        if (fFirstLine) {
            pszWork = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE;
        }

        nMarker = line_ignore_marker(pszWork);
        if (nMarker == 1) { fIgnore = TRUE; continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        if (line_starts_with_tag(pszWork, "SPDX-SnippetBegin")) {
            if (fInSnippet) {
                free(pszSnippetLicense);
                free(pszSnippetCopyright);
                SpdxSnippetListFree(pOut);
                fclose(fp);
                return SPDX_TAG_ERROR_SYNTAX;
            }
            fInSnippet = TRUE;
            ulSnippetStartLine = ulLineNo;
            free(pszSnippetLicense); pszSnippetLicense = NULL;
            free(pszSnippetCopyright); pszSnippetCopyright = NULL;
            cbSnippetCopyrightLen = 0;
            continue;
        }

        if (line_starts_with_tag(pszWork, "SPDX-SnippetEnd")) {
            PSPDXSNIPPET pS;
            if (!fInSnippet) {
                SpdxSnippetListFree(pOut);
                fclose(fp);
                return SPDX_TAG_ERROR_SYNTAX;
            }
            rc = snippetlist_add(pOut, &pS);
            if (rc != NO_ERROR) {
                free(pszSnippetLicense);
                free(pszSnippetCopyright);
                SpdxSnippetListFree(pOut);
                fclose(fp);
                return rc;
            }
            pS->ulLineStart = ulSnippetStartLine;
            pS->ulLineEnd = ulLineNo;
            pS->pszLicense = pszSnippetLicense;
            pS->pszCopyright = pszSnippetCopyright;
            pszSnippetLicense = NULL;
            pszSnippetCopyright = NULL;
            cbSnippetCopyrightLen = 0;
            fInSnippet = FALSE;
            continue;
        }

        if (!fInSnippet) continue;

        {
            PCSZ pszVal;

            pszVal = tag_value(pszWork, "SPDX-SnippetCopyrightText:");
            if (pszVal) {
                CHAR achTmp[MAX_LINE];
                size_t cbL = strlen(pszVal);
                if (cbL >= sizeof(achTmp)) cbL = sizeof(achTmp) - 1;
                memcpy(achTmp, pszVal, cbL);
                achTmp[cbL] = '\0';
                strip_trailing_markers(achTmp);
                pszSnippetCopyright = append_line(pszSnippetCopyright,
                                                  &cbSnippetCopyrightLen,
                                                  achTmp);
                continue;
            }

            pszVal = tag_value(pszWork, "SPDX-License-Identifier:");
            if (pszVal) {
                CHAR achTmp[MAX_LINE];
                size_t cbL = strlen(pszVal);
                if (cbL >= sizeof(achTmp)) cbL = sizeof(achTmp) - 1;
                memcpy(achTmp, pszVal, cbL);
                achTmp[cbL] = '\0';
                strip_trailing_markers(achTmp);
                free(pszSnippetLicense);
                pszSnippetLicense = (PSZ)malloc(strlen(achTmp) + 1);
                if (pszSnippetLicense) strcpy(pszSnippetLicense, achTmp);
                continue;
            }
        }
    }

    fclose(fp);

    if (fInSnippet) {
        free(pszSnippetLicense);
        free(pszSnippetCopyright);
        SpdxSnippetListFree(pOut);
        return SPDX_TAG_ERROR_SYNTAX;
    }

    return NO_ERROR;
}
