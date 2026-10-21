/* spdx_tag.h - SPDX tag parser (C89) */
#ifndef SPDX_TAG_H
#define SPDX_TAG_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_tag.h
 * @brief SPDX 2.3 tag-value parser.
 *
 * Provides line-level parsing of a single SPDX 2.3 tag-value line
 * (SpdxQueryTagLine), together with static metadata for every tag
 * (SpdxQueryTagInfo). The line is expected to already have its
 * comment marker removed; comment stripping is the caller's
 * responsibility (see ReuseQueryCommentBody in the REUSE library).
 *
 * Recognized forms:
 *
 *   - Clause 6 through Clause 13 canonical tag-value (e.g.
 *     "FileCopyrightText:").
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
 * seen in the file. To distinguish the two forms at run time,
 * SpdxQueryTagLine reports whether the line used the Annex H
 * "SPDX-<Field>:" spelling via its pfInFileForm output.
 *
 * Conforms to:
 *   - REUSE Specification 3.3.
 *     https://reuse.software/spec-3.3/
 *   - SPDX 2.3, Clause 6 through Clause 13, Annex E, Annex H.
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

/* ==================================================================
 * Return codes
 * ================================================================== */

/** @def SPDX_TAG_ERROR_SYNTAX @brief Snippet structure is invalid.
 *  User range 0xFF01. */
#define SPDX_TAG_ERROR_SYNTAX        0xFF01

/* ==================================================================
 * Tag identifiers
 * ================================================================== */

/**
 * @enum _SPDXTAG
 * @brief SPDX 2.3 tag-value tag identifiers.
 *
 * One value per tag defined in SPDX 2.3. The grouping matches the
 * specification clauses (6 through 13), Annex E (short-form license
 * identifiers), Annex H (source file tags) and Annex H.3 (snippet
 * tags and delimiters).
 *
 * @c SPDXTAG_NONE marks an empty line or a line with no tag.
 * @c SPDXTAG_OTHER marks an unknown tag.
 *
 * SPDX 2.3 Annex H states that file and snippet tags are the same
 * fields as Clause 8 / Clause 9, only prefixed with "SPDX-" and used
 * inside source file comments. Both forms therefore map to the same
 * SPDXTAG value: "FileCopyrightText:" and "SPDX-FileCopyrightText:"
 * are both @c SPDXTAG_COPYRIGHT.
 *
 * "SPDX-License-Identifier:" (Annex E) has no Clause 8 counterpart
 * and keeps its own @c SPDXTAG_LICENSE_ID, distinct from
 * @c SPDXTAG_FILE_LICENSE_INFO_IN_FILE for "LicenseInfoInFile:".
 */
typedef enum _SPDXTAG {
    SPDXTAG_NONE = 0,                          /**< Empty / not a tag.          */
    SPDXTAG_OTHER,                             /**< Unknown tag.                */

    /* Document Creation Information (Clause 6) */
    SPDXTAG_DOC_VERSION,                       /**< SPDXVersion:                */
    SPDXTAG_DOC_DATA_LICENSE,                  /**< DataLicense:                */
    SPDXTAG_SPDX_ID,                           /**< SPDXID:                     */
    SPDXTAG_DOC_NAME,                          /**< DocumentName:               */
    SPDXTAG_DOC_NAMESPACE,                     /**< DocumentNamespace:          */
    SPDXTAG_DOC_EXTERNAL_REF,                  /**< ExternalDocumentRef:        */
    SPDXTAG_DOC_LICENSE_LIST_VERSION,          /**< LicenseListVersion:         */
    SPDXTAG_DOC_CREATOR,                       /**< Creator:                    */
    SPDXTAG_DOC_CREATED,                       /**< Created:                    */
    SPDXTAG_DOC_CREATOR_COMMENT,               /**< CreatorComment:             */
    SPDXTAG_DOC_COMMENT,                       /**< DocumentComment:            */

    /* Package Information (Clause 7) */
    SPDXTAG_PKG_NAME,                          /**< PackageName:                */
    SPDXTAG_PKG_VERSION,                       /**< PackageVersion:             */
    SPDXTAG_PKG_FILE_NAME,                     /**< PackageFileName:            */
    SPDXTAG_PKG_SUPPLIER,                      /**< PackageSupplier:            */
    SPDXTAG_PKG_ORIGINATOR,                    /**< PackageOriginator:          */
    SPDXTAG_PKG_DOWNLOAD_LOCATION,             /**< PackageDownloadLocation:    */
    SPDXTAG_PKG_FILES_ANALYZED,                /**< FilesAnalyzed:              */
    SPDXTAG_PKG_VERIFICATION_CODE,             /**< PackageVerificationCode:    */
    SPDXTAG_PKG_CHECKSUM,                      /**< PackageChecksum:            */
    SPDXTAG_PKG_HOME_PAGE,                     /**< PackageHomePage:            */
    SPDXTAG_PKG_SOURCE_INFO,                   /**< PackageSourceInfo:          */
    SPDXTAG_PKG_LICENSE_CONCLUDED,             /**< PackageLicenseConcluded:    */
    SPDXTAG_PKG_LICENSE_INFO_FROM_FILES,       /**< PackageLicenseInfoFromFiles: */
    SPDXTAG_PKG_LICENSE_DECLARED,              /**< PackageLicenseDeclared:     */
    SPDXTAG_PKG_LICENSE_COMMENTS,              /**< PackageLicenseComments:     */
    SPDXTAG_PKG_COPYRIGHT_TEXT,                /**< PackageCopyrightText:       */
    SPDXTAG_PKG_SUMMARY,                       /**< PackageSummary:             */
    SPDXTAG_PKG_DESCRIPTION,                   /**< PackageDescription:         */
    SPDXTAG_PKG_COMMENT,                       /**< PackageComment:             */
    SPDXTAG_PKG_EXTERNAL_REF,                  /**< ExternalRef:                */
    SPDXTAG_PKG_EXTERNAL_REF_COMMENT,          /**< ExternalRefComment:         */
    SPDXTAG_PKG_ATTRIBUTION_TEXT,              /**< PackageAttributionText:     */

    /* File Information (Clause 8) */
    SPDXTAG_FILE_NAME,                         /**< FileName:                   */
    SPDXTAG_FILE_TYPE,                         /**< FileType:                   */
    SPDXTAG_FILE_CHECKSUM,                     /**< FileChecksum:               */
    SPDXTAG_FILE_LICENSE_CONCLUDED,            /**< LicenseConcluded:           */
    SPDXTAG_FILE_LICENSE_INFO_IN_FILE,         /**< LicenseInfoInFile:          */
    SPDXTAG_FILE_LICENSE_COMMENTS,             /**< LicenseComments:            */
    SPDXTAG_COPYRIGHT,                         /**< FileCopyrightText:          */
    SPDXTAG_FILE_ARTIFACT_PROJECT_NAME,        /**< ArtifactOfProjectName:      */
    SPDXTAG_FILE_ARTIFACT_PROJECT_HOME_PAGE,   /**< ArtifactOfProjectHomePage:  */
    SPDXTAG_FILE_ARTIFACT_PROJECT_URI,         /**< ArtifactOfProjectURI:       */
    SPDXTAG_FILE_COMMENT,                      /**< FileComment:                */
    SPDXTAG_FILE_NOTICE,                       /**< FileNotice:                 */
    SPDXTAG_FILE_CONTRIBUTOR,                  /**< FileContributor:            */
    SPDXTAG_FILE_ATTRIBUTION_TEXT,             /**< FileAttributionText:        */
    SPDXTAG_FILE_DEPENDENCY,                   /**< FileDependency:             */

    /* Snippet Information (Clause 9) */
    SPDXTAG_SNIPPET_ID,                        /**< SnippetSPDXID:              */
    SPDXTAG_SNIPPET_FROM_FILE,                 /**< SnippetFromFileSPDXID:      */
    SPDXTAG_SNIPPET_BYTE_RANGE,                /**< SnippetByteRange:           */
    SPDXTAG_SNIPPET_LINE_RANGE,                /**< SnippetLineRange:           */
    SPDXTAG_SNIPPET_LICENSE_CONCLUDED,         /**< SnippetLicenseConcluded:    */
    SPDXTAG_SNIPPET_LICENSE_INFO,              /**< LicenseInfoInSnippet:       */
    SPDXTAG_SNIPPET_LICENSE_COMMENTS,          /**< SnippetLicenseComments:     */
    SPDXTAG_SNIPPET_COPYRIGHT,                 /**< SnippetCopyrightText:       */
    SPDXTAG_SNIPPET_COMMENT,                   /**< SnippetComment:             */
    SPDXTAG_SNIPPET_NAME,                      /**< SnippetName:                */
    SPDXTAG_SNIPPET_ATTRIBUTION_TEXT,          /**< SnippetAttributionText:     */

    /* Other Licensing Information (Clause 10) */
    SPDXTAG_LICENSE_ID_DEF,                    /**< LicenseID:                  */
    SPDXTAG_LICENSE_EXTRACTED_TEXT,            /**< ExtractedText:              */
    SPDXTAG_LICENSE_NAME,                      /**< LicenseName:                */
    SPDXTAG_LICENSE_CROSS_REF,                 /**< LicenseCrossReference:      */
    SPDXTAG_LICENSE_COMMENT,                   /**< LicenseComment:             */

    /* Relationships (Clause 11) */
    SPDXTAG_RELATIONSHIP,                      /**< Relationship:               */
    SPDXTAG_RELATIONSHIP_COMMENT,              /**< RelationshipComment:        */

    /* Annotations (Clause 12) */
    SPDXTAG_ANNOTATOR,                         /**< Annotator:                  */
    SPDXTAG_ANNOTATION_DATE,                   /**< AnnotationDate:             */
    SPDXTAG_ANNOTATION_TYPE,                   /**< AnnotationType:             */
    SPDXTAG_ANNOTATION_SPDXREF,                /**< SPDXREF:                    */
    SPDXTAG_ANNOTATION_COMMENT,                /**< AnnotationComment:          */

    /* Review Information (Clause 13, deprecated) */
    SPDXTAG_REVIEWER,                          /**< Reviewer:                   */
    SPDXTAG_REVIEW_DATE,                       /**< ReviewDate:                 */
    SPDXTAG_REVIEW_COMMENT,                    /**< ReviewComment:              */

    /* Annex H.3 - in-line snippet markers */
    SPDXTAG_SNIPPET_BEGIN,                     /**< SPDX-SnippetBegin           */
    SPDXTAG_SNIPPET_END,                       /**< SPDX-SnippetEnd             */

    /* Annex E - short-form license identifier */
    SPDXTAG_LICENSE_ID                         /**< SPDX-License-Identifier:    */
} SPDXTAG;

/**
 * @struct _SPDXTAGINFO
 * @brief Static metadata for one SPDX 2.3 tag.
 *
 * All string fields point to static data with the lifetime of the
 * program. The caller must not free them.
 *
 * @c pszDeprecated and @c pszReplacement are NULL for tags that are
 * not deprecated.
 *
 * When a tag has both a canonical Clause 8/9 spelling and an
 * Annex H spelling, SpdxQueryTagInfo returns the canonical one:
 * @c pszTag is "FileCopyrightText:" and @c pszSection is "8.8" for
 * @c SPDXTAG_COPYRIGHT, even if the file contained
 * "SPDX-FileCopyrightText:". To learn which spelling was present in
 * the file, callers inspect the pfInFileForm output of
 * SpdxQueryTagLine.
 */
typedef struct _SPDXTAGINFO {
    SPDXTAG tag;              /**< Tag identifier.                      */
    PCSZ    pszTag;           /**< Canonical tag name, with ':'.        */
    PCSZ    pszSection;       /**< SPDX 2.3 section ("8.8", "Annex H"). */
    PCSZ    pszIntroduced;    /**< Version of the spec ("1.0").         */
    PCSZ    pszDeprecated;    /**< Version deprecated, or NULL.         */
    PCSZ    pszReplacement;   /**< Textual replacement, or NULL.        */
} SPDXTAGINFO, *PSPDXTAGINFO;

/* ==================================================================
 * Tag-value line parser
 * ================================================================== */

/**
 * @brief Parse one SPDX 2.3 tag-value line.
 *
 * The line is expected to already have its comment marker removed;
 * stripping is the caller's responsibility (see
 * ReuseQueryCommentBody). A UTF-8 BOM at the start of the line is
 * skipped.
 *
 * Both the Clause 6-13 form (FileCopyrightText:) and the Annex H
 * form (SPDX-FileCopyrightText:) are recognized and map to the same
 * @c SPDXTAG value. @p pfInFileForm reports which of the two forms
 * was actually present:
 *
 *   - TRUE  - the matched tag starts with "SPDX-": Annex E
 *             (SPDX-License-Identifier:), Annex H
 *             (SPDX-<Clause 8 field>:) or Annex H.3
 *             (SPDX-SnippetBegin, SPDX-SnippetEnd,
 *             SPDX-<Clause 9 field>:).
 *   - FALSE - the matched tag is the Clause 6-13 canonical spelling,
 *             or the line carries no recognized tag
 *             (SPDXTAG_NONE / SPDXTAG_OTHER).
 *
 * Size-query convention:
 *   - @p pszValue == NULL, @p ulSize == 0: only @p *pulUsed is
 *     written.
 *   - @p ulSize large enough: value copied and NUL-terminated;
 *     @p *pulUsed is the length without NUL.
 *   - @p ulSize too small: ERROR_BUFFER_OVERFLOW; @p *pulUsed is
 *     the required size including NUL.
 *
 * For an empty line, @p *pTag is @c SPDXTAG_NONE and the value is
 * empty. For a line that contains no recognized tag, @p *pTag is
 * @c SPDXTAG_OTHER; if the line contains a ':', the value is what
 * follows the first ':', otherwise the value is the whole line.
 *
 * @param[in]  pszLine       Line to parse. Not NULL.
 * @param[out] pTag          Tag identifier. Not NULL.
 * @param[out] pfDeprecated  TRUE if the tag is deprecated in SPDX 2.3.
 *                           Not NULL.
 * @param[out] pfInFileForm  TRUE if the matched tag is an Annex E,
 *                           Annex H or Annex H.3 "SPDX-..." form.
 *                           FALSE for canonical Clause 6-13 forms
 *                           and for SPDXTAG_NONE / SPDXTAG_OTHER.
 *                           Not NULL.
 * @param[out] pszValue      Output buffer for the value, or NULL for
 *                           size query.
 * @param[in]  ulSize        Size of @p pszValue in bytes.
 * @param[out] pulUsed       Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Parse completed.
 * @retval ERROR_INVALID_PARAMETER  @p pszLine, @p pTag,
 *                                  @p pfDeprecated or @p pfInFileForm
 *                                  is NULL, or the size-query
 *                                  convention is violated.
 * @retval ERROR_BUFFER_OVERFLOW    @p pszValue is too small.
 */
APIRET APIENTRY SpdxQueryTagLine(PCSZ pszLine, SPDXTAG *pTag,
                                 PBOOL pfDeprecated,
                                 PBOOL pfInFileForm,
                                 PSZ pszValue, ULONG ulSize,
                                 PULONG pulUsed);

/**
 * @brief Query static metadata for a tag.
 *
 * Returns the canonical Clause 8/9 form of the tag (for example,
 * "FileCopyrightText:" for @c SPDXTAG_COPYRIGHT). Annex E and
 * Annex H forms are recognized by SpdxQueryTagLine but not returned
 * here; use SpdxQueryTagLine's pfInFileForm output to learn which
 * form was present in the file.
 *
 * @param[in]  tag    Tag identifier.
 * @param[out] pInfo  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Metadata found.
 * @retval ERROR_INVALID_PARAMETER  @p pInfo is NULL.
 * @retval ERROR_FILE_NOT_FOUND     No metadata for @c SPDXTAG_NONE or
 *                                  @c SPDXTAG_OTHER.
 */
APIRET APIENTRY SpdxQueryTagInfo(SPDXTAG tag, SPDXTAGINFO *pInfo);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_TAG_H */
