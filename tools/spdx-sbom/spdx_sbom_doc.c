/* spdx_sbom_doc.c - построение SpdxDocument (C89) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "spdx_sbom_doc.h"
#include "spdx_sbom_utils.h"
#include "spdx_sbom_extracted.h"
#include "spdx_utils.h"
#include "sha1_utils.h"

static void copy_safe(char *dst, size_t dst_size, const char *src) {
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

void sbom_doc_init(SpdxDocument *doc,
                   const char *name,
                   const char *version,
                   const char *supplier,
                   const char *creator,
                   const char *license,
                   const char *copyright,
                   const char *purpose,
                   const char *binary_file,
                   int   binary_mode) {
    time_t now;
    struct tm *tm;
    char date[32];
    char safe[256];
    char ns[512];
    char file_base[256];
    char pkg_id[256];

    memset(doc, 0, sizeof(*doc));
    extracted_init(&doc->extracted_licenses);

    now = time(NULL);
    tm = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%SZ", tm);

    copy_safe(doc->spdx_version, sizeof(doc->spdx_version), "SPDX-2.3");
    copy_safe(doc->document_id, sizeof(doc->document_id), "SPDXRef-DOCUMENT");
    copy_safe(doc->created, sizeof(doc->created), date);

    if (creator && creator[0])
        copy_safe(doc->creator, sizeof(doc->creator), creator);
    else
        copy_safe(doc->creator, sizeof(doc->creator),
                  "Tool: osFree SPDX SBOM Generator");

    if (name)
        snprintf(doc->document_name, sizeof(doc->document_name), "%s SBOM", name);

    sbom_sanitize_id(name ? name : "document", safe, sizeof(safe));
    snprintf(ns, sizeof(ns), "https://osfree.org/spdxdocs/%s-%ld",
             safe, (long)now);
    copy_safe(doc->document_namespace, sizeof(doc->document_namespace), ns);
    copy_safe(doc->data_license, sizeof(doc->data_license), "CC0-1.0");

    file_base[0] = '\0';
    if (binary_file) {
        copy_safe(file_base, sizeof(file_base),
                  spdx_get_file_name(binary_file));
        sbom_remove_extension(file_base);
    }
    if (file_base[0] == '\0')
        copy_safe(file_base, sizeof(file_base), "package");

    sbom_make_package_id(file_base, binary_mode ? NULL : "Source",
                         pkg_id, sizeof(pkg_id));

    copy_safe(doc->package.spdx_id, sizeof(doc->package.spdx_id), pkg_id);
    copy_safe(doc->package.name, sizeof(doc->package.name), name);
    if (version) copy_safe(doc->package.version, sizeof(doc->package.version), version);
    if (supplier) copy_safe(doc->package.supplier, sizeof(doc->package.supplier), supplier);
    copy_safe(doc->package.license, sizeof(doc->package.license), license);
    if (copyright) copy_safe(doc->package.copyright, sizeof(doc->package.copyright), copyright);
    if (purpose) copy_safe(doc->package.purpose, sizeof(doc->package.purpose), purpose);
    doc->package.files_analyzed = 0;
    doc->package.verification_code[0] = '\0';
}

static int cmp_fileinfo(const void *a, const void *b) {
    return strcmp(((const FileInfo*)a)->name, ((const FileInfo*)b)->name);
}

void sbom_doc_compute_verification(SpdxDocument *doc) {
    FileInfo *sorted;
    size_t total_len;
    char *concat;
    char combined[41];
    int i;

    if (doc->files.count == 0) {
        doc->package.files_analyzed = 0;
        doc->package.verification_code[0] = '\0';
        return;
    }

    sorted = (FileInfo*)malloc((size_t)doc->files.count * sizeof(FileInfo));
    if (!sorted) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(sorted, doc->files.items,
           (size_t)doc->files.count * sizeof(FileInfo));
    qsort(sorted, (size_t)doc->files.count, sizeof(FileInfo), cmp_fileinfo);

    total_len = 0;
    for (i = 0; i < doc->files.count; i++)
        total_len += strlen(sorted[i].sha1);

    concat = (char*)malloc(total_len + 1);
    if (!concat) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    concat[0] = '\0';
    for (i = 0; i < doc->files.count; i++)
        strcat(concat, sorted[i].sha1);

    sha1_string(concat, combined);
    free(concat);
    free(sorted);

    memcpy(doc->package.verification_code, combined, 41);
    doc->package.files_analyzed = 1;
}

void sbom_doc_build_relationships(SpdxDocument *doc,
                                  const char *base_name_no_ext,
                                  int binary_mode) {
    doc->relationships = (Relationship*)malloc(sizeof(Relationship) * 2);
    if (!doc->relationships) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    doc->relationship_count = 0;

    copy_safe(doc->relationships[0].element_id,
              sizeof(doc->relationships[0].element_id), doc->document_id);
    copy_safe(doc->relationships[0].related_element,
              sizeof(doc->relationships[0].related_element),
              doc->package.spdx_id);
    copy_safe(doc->relationships[0].relationship_type,
              sizeof(doc->relationships[0].relationship_type), "DESCRIBES");
    doc->relationship_count = 1;

    if (binary_mode) {
        char src_id[256];
        sbom_make_package_id(base_name_no_ext, "Source", src_id, sizeof(src_id));
        copy_safe(doc->relationships[1].element_id,
                  sizeof(doc->relationships[1].element_id), doc->package.spdx_id);
        copy_safe(doc->relationships[1].related_element,
                  sizeof(doc->relationships[1].related_element), src_id);
        copy_safe(doc->relationships[1].relationship_type,
                  sizeof(doc->relationships[1].relationship_type),
                  "GENERATED_FROM");
        doc->relationship_count = 2;
    }
}

void sbom_doc_set_external(SpdxDocument *doc,
                           const char *source_sbom_path,
                           const char *source_package_id,
                           const char *checksum_sha1) {
    if (!source_sbom_path || !source_sbom_path[0]) return;

    copy_safe(doc->external_doc_id, sizeof(doc->external_doc_id),
              "DocumentRef-source");
    copy_safe(doc->external_doc_uri, sizeof(doc->external_doc_uri),
              spdx_get_file_name(source_sbom_path));
    if (checksum_sha1)
        copy_safe(doc->external_doc_checksum,
                  sizeof(doc->external_doc_checksum), checksum_sha1);

    doc->has_external_ref = 1;

    if (doc->relationship_count >= 2 &&
        strcmp(doc->relationships[1].relationship_type, "GENERATED_FROM") == 0) {
        char full[512];
        snprintf(full, sizeof(full), "%s:%s",
                 doc->external_doc_id, source_package_id);
        copy_safe(doc->relationships[1].related_element,
                  sizeof(doc->relationships[1].related_element), full);
    }
}

void sbom_doc_free(SpdxDocument *doc) {
    filelist_free(&doc->files);
    extracted_free(&doc->extracted_licenses);
    free(doc->relationships);
    doc->relationships = NULL;
    doc->relationship_count = 0;
}
