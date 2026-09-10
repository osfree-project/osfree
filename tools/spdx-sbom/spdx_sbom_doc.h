/* spdx_sbom_doc.h - построение SpdxDocument (C89) */
#ifndef SPDX_SBOM_DOC_H
#define SPDX_SBOM_DOC_H

#include "spdx_sbom_types.h"

/* Инициализирует документ: spdxVersion, document_id, дата, namespace,
   creator, data_license, имя документа, пакет. Обнуляет всю структуру
   (memset) и инициализирует пустой список extracted_licenses.
   Вызывать ДО filelist_init(&doc->files). */
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

/* Вычисляет packageVerificationCode по files. */
void sbom_doc_compute_verification(SpdxDocument *doc);

/* Заполняет relationships: DESCRIBES (документ -> пакет) и, при binary_mode,
   GENERATED_FROM (пакет -> Source-пакет). */
void sbom_doc_build_relationships(SpdxDocument *doc,
                                  const char *base_name_no_ext,
                                  int binary_mode);

/* Устанавливает внешнюю ссылку на SBOM исходников. */
void sbom_doc_set_external(SpdxDocument *doc,
                           const char *source_sbom_path,
                           const char *source_package_id,
                           const char *checksum_sha1);

void sbom_doc_free(SpdxDocument *doc);

#endif /* SPDX_SBOM_DOC_H */
