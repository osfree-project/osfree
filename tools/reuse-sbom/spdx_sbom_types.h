/* spdx_sbom_types.h - common SPDX SBOM structures (C89) */
#ifndef SPDX_SBOM_TYPES_H
#define SPDX_SBOM_TYPES_H

#include "os2types.h"
#include "ccl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_types.h
 * @brief Common structures for the SBOM generator.
 *
 * The file list, snippet list, relationship list and extracted
 * license list are stored as HVECTOR containers from the ccl
 * library, each holding its element type by value.
 *
 * Conforms to:
 *   - SPDX 2.3.
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

/**
 * @struct _SPDXFILEINFO
 * @brief One file entry.
 */
typedef struct _SPDXFILEINFO {
    CHAR achName[512];         /**< Base name.               */
    CHAR achSha1[41];          /**< SHA-1 hex digest.        */
    CHAR achLicense[256];      /**< SPDX license expression. */
    CHAR achCopyright[512];    /**< Copyright text.          */
    CHAR achFileType[32];      /**< SPDX file type.          */
} SPDXFILEINFO, *PSPDXFILEINFO;

/**
 * @struct _SPDXSNIPPETINFO
 * @brief One snippet entry.
 */
typedef struct _SPDXSNIPPETINFO {
    CHAR achSpdxId[128];         /**< SPDXRef-Snippet-<n>.     */
    CHAR achFromFileId[512];     /**< SPDXRef-File-<name>.     */
    CHAR achFromFileName[512];   /**< Base name of source.     */
    ULONG ulLineStart;           /**< First line of snippet.   */
    ULONG ulLineEnd;             /**< Last line of snippet.    */
    CHAR achLicense[256];        /**< SPDX license expression. */
    CHAR achCopyright[512];      /**< Copyright text.          */
} SPDXSNIPPETINFO, *PSPDXSNIPPETINFO;

/**
 * @struct _SPDXEXTRACTEDLICENSEINFO
 * @brief One extracted license entry.
 *
 * All string fields are malloc'd and owned by the entry. They are
 * released by SbomExtractedListFree.
 */
typedef struct _SPDXEXTRACTEDLICENSEINFO {
    PSZ pszLicenseId;            /**< LicenseRef-* identifier. */
    PSZ pszExtractedText;        /**< Full license text.       */
    PSZ pszName;                 /**< Optional name.           */
    PSZ pszComment;              /**< Optional comment.        */
} SPDXEXTRACTEDLICENSEINFO, *PSPDXEXTRACTEDLICENSEINFO;

/**
 * @struct _SPDXEXTRACTEDLICENSESOURCE
 * @brief One source of LicenseRef text from the command line.
 */
typedef struct _SPDXEXTRACTEDLICENSESOURCE {
    CHAR achId[256];             /**< LicenseRef identifier.   */
    CHAR achPath[1024];          /**< Path to the text file.   */
} SPDXEXTRACTEDLICENSESOURCE, *PSPDXEXTRACTEDLICENSESOURCE;

/**
 * @struct _SPDXPACKAGEINFO
 * @brief Package-level information.
 */
typedef struct _SPDXPACKAGEINFO {
    CHAR achSpdxId[256];         /**< SPDXRef-Package-...      */
    CHAR achName[512];           /**< Package name.            */
    CHAR achVersion[128];        /**< Version.                 */
    CHAR achSupplier[256];       /**< Supplier.                */
    CHAR achLicense[256];        /**< SPDX license expression. */
    CHAR achCopyright[512];      /**< Copyright text.          */
    CHAR achPurpose[128];        /**< Primary package purpose. */
    BOOL fFilesAnalyzed;         /**< Files have been analyzed.*/
    CHAR achVerificationCode[41];/**< Package verification code. */

    HSTRSET hLicenseInfoFromFiles;/**< Unique SPDX identifiers. */
} SPDXPACKAGEINFO, *PSPDXPACKAGEINFO;

/**
 * @struct _SPDXRELATIONSHIP
 * @brief One relationship between SPDX elements.
 */
typedef struct _SPDXRELATIONSHIP {
    CHAR achElementId[256];      /**< Source element.       */
    CHAR achRelatedElement[256]; /**< Target element.       */
    CHAR achRelationshipType[64];/**< Relationship type.    */
} SPDXRELATIONSHIP, *PSPDXRELATIONSHIP;

/**
 * @struct _SPDXDOCUMENT
 * @brief Whole SBOM document.
 */
typedef struct _SPDXDOCUMENT {
    CHAR achSpdxVersion[16];       /**< SPDX-2.3.               */
    CHAR achDocumentId[256];       /**< SPDXRef-DOCUMENT.       */
    CHAR achDocumentName[512];     /**< Document name.          */
    CHAR achDocumentNamespace[512];/**< Document namespace.     */
    CHAR achCreated[32];           /**< Creation timestamp.     */
    CHAR achCreator[256];          /**< Creator string.         */
    CHAR achDataLicense[64];       /**< CC0-1.0.                */

    SPDXPACKAGEINFO package;       /**< Package info.           */
    HVECTOR hFiles;                /**< SPDXFILEINFO.           */
    HVECTOR hSnippets;             /**< SPDXSNIPPETINFO.        */
    HVECTOR hRelationships;        /**< SPDXRELATIONSHIP.       */

    BOOL fHasExternalRef;          /**< External ref present.   */
    CHAR achExternalDocId[256];    /**< DocumentRef-source.     */
    CHAR achExternalDocUri[512];   /**< External document URI.  */
    CHAR achExternalDocChecksum[128];/**< SHA1 hex digest.      */

    HVECTOR hExtractedLicenses;    /**< SPDXEXTRACTEDLICENSEINFO. */
} SPDXDOCUMENT, *PSPDXDOCUMENT;

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_TYPES_H */
