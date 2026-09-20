/* spdx_sbom_doc.h - SpdxDocument construction (C89) */
#ifndef SPDX_SBOM_DOC_H
#define SPDX_SBOM_DOC_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "spdx_sbom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file spdx_sbom_doc.h
 * @brief Construction and finalization of the SBOM document.
 *
 * The document is created by SbomCreateDocument, which fills the
 * fixed fields (version, id, namespace, creation time, creator)
 * and creates the empty containers for files, snippets,
 * relationships, extracted licenses, and the package's license
 * identifier set.
 *
 * After the caller has filled the containers, the document is
 * finalized by SbomComputeVerification (PackageVerificationCode
 * and PackageLicenseInfoFromFiles) and by
 * SbomBuildRelationships. External document references must be set
 * with SbomSetDocumentExternalReference before
 * SbomBuildRelationships is called, so that the GENERATED_FROM
 * relationship can point to the external document.
 *
 * Conforms to:
 *   - SPDX 2.3, §6.5, §6.6, §6.9, §7.9, §11.
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

/**
 * @brief Create an empty SBOM document.
 *
 * Fills the fixed fields (spdxVersion, document id, creation time,
 * creator, namespace, data license) and creates the empty
 * containers:
 *   - files (SPDXFILEINFO),
 *   - snippets (SPDXSNIPPETINFO),
 *   - relationships (SPDXRELATIONSHIP),
 *   - extracted licenses (SPDXEXTRACTEDLICENSEINFO),
 *   - package license identifier set (HSTRSET).
 *
 * The document must be released with SbomFreeDocument.
 *
 * @param[out] pDoc           Receiver. Not NULL.
 * @param[in]  pszName        Document name. Not NULL. Used to build
 *                            the document name and the namespace.
 * @param[in]  pszVersion     Package version, or NULL.
 * @param[in]  pszSupplier    Package supplier, or NULL.
 * @param[in]  pszCreator     Creator string, or NULL to use the
 *                            default "Tool: osFree SPDX SBOM
 *                            Generator".
 * @param[in]  pszLicense     Package license expression, or NULL.
 * @param[in]  pszCopyright   Package copyright, or NULL.
 * @param[in]  pszPurpose     Primary package purpose, or NULL.
 * @param[in]  pszBinaryFile  Binary artifact path, or NULL. Used to
 *                            derive the package base name and the
 *                            package SPDXID.
 * @param[in]  fBinaryMode    TRUE for a binary artifact, FALSE for
 *                            a source tree.
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
    BOOL fBinaryMode);

/**
 * @brief Release all resources owned by a document.
 *
 * Frees the file, snippet, relationship and extracted license
 * containers, and releases the package's license identifier set.
 * The fixed fields of the document are left unchanged except for
 * the container handles, which are set to NULLHANDLE.
 *
 * @param[in,out] pDoc  Document. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 */
APIRET APIENTRY SbomFreeDocument(SPDXDOCUMENT *pDoc);

/* ==================================================================
 * External document reference
 * ================================================================== */

/**
 * @brief Set the external document reference to a source SBOM.
 *
 * Records the external document id ("DocumentRef-source"), the
 * file name of the source SBOM, and its SHA-1 checksum. Sets the
 * document's @c fHasExternalRef flag.
 *
 * Must be called before SbomBuildRelationships: the
 * GENERATED_FROM relationship uses the reference to point at the
 * source package inside the external document.
 *
 * @param[in,out] pDoc                Document. Not NULL.
 * @param[in]     pszSourceSbomPath   Path to the source SBOM. If
 *                                    NULL or empty, the function
 *                                    does nothing.
 * @param[in]     pszChecksumSha1     SHA-1 hex digest of the
 *                                    source SBOM, or NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 */
APIRET APIENTRY SbomSetDocumentExternalReference(
    SPDXDOCUMENT *pDoc,
    PCSZ pszSourceSbomPath,
    PCSZ pszChecksumSha1);

/* ==================================================================
 * Package verification
 * ================================================================== */

/**
 * @brief Compute PackageVerificationCode and license identifiers.
 *
 * Implements SPDX 2.3 §7.9: the verification code is the SHA-1 of
 * the concatenation of the SHA-1 hashes of every file in the
 * package, sorted by file name. SHA-1 is mandatory here and cannot
 * be replaced with another algorithm.
 *
 * Also fills PackageLicenseInfoFromFiles by extracting the
 * individual identifiers from every file's license expression.
 *
 * If the document has no files, both PackageLicenseInfoFromFiles
 * and PackageVerificationCode are left empty and
 * @c package.fFilesAnalyzed is set to FALSE.
 *
 * @param[in,out] pDoc  Document. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 * @retval ERROR_INVALID_HANDLE     An internal container is not
 *                                  recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SbomComputeVerification(SPDXDOCUMENT *pDoc);

/* ==================================================================
 * Relationships
 * ================================================================== */

/**
 * @brief Build the relationships list.
 *
 * Emits the following relationships in this order:
 *   - DESCRIBES: document -> package;
 *   - GENERATED_FROM: package -> Source package (binary mode only).
 *     If the document has an external reference (set by
 *     SbomSetDocumentExternalReference), the target is
 *     "<external_doc_id>:<source_package_id>";
 *   - CONTAINS: package -> file, for each file;
 *   - CONTAINS: file -> snippet, for each snippet.
 *
 * The @c hRelationships container is assumed to be empty at entry.
 *
 * @param[in,out] pDoc                Document. Not NULL.
 * @param[in]     pszBaseNameNoExt    Base name without extension
 *                                    used to construct the Source
 *                                    package id in binary mode. Not
 *                                    NULL in binary mode.
 * @param[in]     fBinaryMode         TRUE for a binary artifact.
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
    BOOL fBinaryMode);

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_DOC_H */
