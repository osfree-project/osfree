/* spdx_sbom_out_tag.c - вывод SBOM в формате Tag/Value (C89) */

#include <stdio.h>
#include <string.h>
#include "spdx_sbom_out.h"

/* ¬ыводит поле в формате <Tag>: <value>.
 *
 * SPDX 2.3, раздел про tag-value:
 *   "When a field value contains a line break, it must be wrapped in
 *    <text></text> tags."
 *
 * ѕоэтому если значение содержит '\n' или '\r' Ч оборачиваем в <text>
 * и обрезаем хвостовые переводы строк, чтобы </text> шЄл сразу после
 * последней содержательной строки. */
static void print_text_field(const char *tag, const char *value) {
    const char *end;
    size_t len;

    if (!value) value = "";
    if (strchr(value, '\n') == NULL && strchr(value, '\r') == NULL) {
        printf("%s: %s\n", tag, value);
        return;
    }
    len = strlen(value);
    end = value + len;
    while (end > value && (end[-1] == '\n' || end[-1] == '\r')) end--;

    printf("%s: <text>", tag);
    fwrite(value, 1, (size_t)(end - value), stdout);
    printf("</text>\n");
}

int sbom_output_tagvalue(const SpdxDocument *doc) {
    int i;
    const PackageInfo *pkg = &doc->package;

    printf("SPDXVersion: %s\n", doc->spdx_version);
    printf("DataLicense: %s\n", doc->data_license);
    printf("SPDXID: %s\n", doc->document_id);
    printf("DocumentName: %s\n", doc->document_name);
    printf("DocumentNamespace: %s\n", doc->document_namespace);
    printf("Creator: %s\n", doc->creator);
    printf("Created: %s\n", doc->created);
    printf("\n");

    if (doc->has_external_ref) {
        printf("ExternalDocumentRef: %s %s SHA1: %s\n",
               doc->external_doc_id, doc->external_doc_uri,
               doc->external_doc_checksum);
        printf("\n");
    }

    for (i = 0; i < doc->extracted_licenses.count; i++) {
        const ExtractedLicenseInfo *e = &doc->extracted_licenses.items[i];
        printf("LicenseID: %s\n", e->license_id);
        print_text_field("ExtractedText",
                         e->extracted_text ? e->extracted_text : "");
        printf("LicenseName: %s\n",
               (e->name && e->name[0]) ? e->name : "NOASSERTION");
        printf("LicenseComment: %s\n",
               (e->comment && e->comment[0]) ? e->comment : "NOASSERTION");
        printf("\n");
    }

    printf("##### Package: %s\n", pkg->name);
    printf("PackageName: %s\n", pkg->name);
    printf("SPDXID: %s\n", pkg->spdx_id);
    printf("PackageDownloadLocation: NOASSERTION\n");
    if (pkg->version[0])
        printf("PackageVersion: %s\n", pkg->version);
    if (pkg->supplier[0])
        printf("PackageSupplier: %s\n", pkg->supplier);
    printf("PackageLicenseConcluded: %s\n", pkg->license);
    printf("PackageLicenseDeclared: %s\n", pkg->license);
    if (pkg->copyright[0])
        print_text_field("PackageCopyrightText", pkg->copyright);
    if (pkg->purpose[0])
        printf("PackagePrimaryPurpose: %s\n", pkg->purpose);
    if (pkg->files_analyzed) {
        printf("FilesAnalyzed: true\n");
        printf("PackageVerificationCode: %s\n", pkg->verification_code);
    } else {
        printf("FilesAnalyzed: false\n");
    }
    printf("\n");

    for (i = 0; i < doc->files.count; i++) {
        printf("FileName: %s\n", doc->files.items[i].name);
        printf("SPDXID: SPDXRef-File-%s\n", doc->files.items[i].name);
        printf("FileType: %s\n", doc->files.items[i].file_type);
        printf("FileChecksum: SHA1: %s\n", doc->files.items[i].sha1);
        printf("LicenseConcluded: %s\n", doc->files.items[i].license);
        printf("LicenseInfoInFile: %s\n", doc->files.items[i].license);
        if (doc->files.items[i].copyright[0])
            print_text_field("FileCopyrightText",
                             doc->files.items[i].copyright);
        printf("\n");
    }

    for (i = 0; i < doc->relationship_count; i++) {
        printf("Relationship: %s %s %s\n",
               doc->relationships[i].element_id,
               doc->relationships[i].relationship_type,
               doc->relationships[i].related_element);
    }
    printf("\n");
    return 0;
}
