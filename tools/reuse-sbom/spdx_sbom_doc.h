/* spdx_sbom_doc.h - SpdxDocument construction (C89) */
#ifndef SPDX_SBOM_DOC_H
#define SPDX_SBOM_DOC_H

#include "spdx_sbom_types.h"

/* Initializes the document: spdxVersion, document_id, date, namespace,
   creator, data_license, document name, package. Zeroes the whole
   structure (memset) and initializes an empty extracted_licenses list.
   Call BEFORE filelist_init(&doc->files). */
void sbom_doc_init(SpdxDocument *doc,
                   const char *name,
                   const char *version,
                   const char *supplier,
                   const char *creator,
                   const char *license,
                   const char *copyright,
                   const char *purpose,
                   const char *binary_file,
                   int   binary_mode);

/* Computes packageVerificationCode from files. */
void sbom_doc_compute_verification(SpdxDocument *doc);

/* Fills relationships: DESCRIBES (document -> package) and, in binary
   mode, GENERATED_FROM (package -> Source package). */
void sbom_doc_build_relationships(SpdxDocument *doc,
                                  const char *base_name_no_ext,
                                  int binary_mode);

/* Sets an external reference to the source SBOM. */
void sbom_doc_set_external(SpdxDocument *doc,
                           const char *source_sbom_path,
                           const char *source_package_id,
                           const char *checksum_sha1);

void sbom_doc_free(SpdxDocument *doc);

#endif /* SPDX_SBOM_DOC_H */
