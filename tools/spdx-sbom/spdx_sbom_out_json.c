/* spdx_sbom_out_json.c - вывод SBOM в формате JSON (C89) */

#include <stdio.h>
#include <stdlib.h>
#include "spdx_sbom_out.h"
#include "spdx_sbom_utils.h"

static void print_str(const char *s) {
    char *esc = sbom_json_escape(s ? s : "");
    printf("%s", esc ? esc : "");
    free(esc);
}

static void print_package(const PackageInfo *pkg) {
    int i;
    printf("    {\n");
    printf("      \"SPDXID\": \"%s\",\n", pkg->spdx_id);
    printf("      \"name\": \""); print_str(pkg->name); printf("\",\n");
    printf("      \"downloadLocation\": \"NOASSERTION\",\n");
    if (pkg->version[0]) {
        printf("      \"versionInfo\": \""); print_str(pkg->version); printf("\",\n");
    } else printf("      \"versionInfo\": \"NOASSERTION\",\n");
    if (pkg->supplier[0]) {
        printf("      \"supplier\": \""); print_str(pkg->supplier); printf("\",\n");
    } else printf("      \"supplier\": \"NOASSERTION\",\n");
    printf("      \"licenseConcluded\": \""); print_str(pkg->license); printf("\",\n");
    printf("      \"licenseDeclared\": \""); print_str(pkg->license); printf("\",\n");
    if (pkg->files_analyzed && pkg->license_info_count > 0) {
        printf("      \"licenseInfoFromFiles\": [\n");
        for (i = 0; i < pkg->license_info_count; i++) {
            printf("        \"");
            print_str(pkg->license_info_from_files[i]);
            printf("\"%s\n",
                   (i + 1 < pkg->license_info_count) ? "," : "");
        }
        printf("      ],\n");
    }
    if (pkg->copyright[0]) {
        printf("      \"copyrightText\": \""); print_str(pkg->copyright); printf("\",\n");
    } else printf("      \"copyrightText\": \"NOASSERTION\",\n");
    if (pkg->purpose[0]) {
        printf("      \"primaryPackagePurpose\": \""); print_str(pkg->purpose); printf("\",\n");
    } else printf("      \"primaryPackagePurpose\": \"NOASSERTION\",\n");
    if (pkg->files_analyzed) {
        printf("      \"filesAnalyzed\": true,\n");
        printf("      \"packageVerificationCode\": {\n");
        printf("        \"packageVerificationCodeValue\": \"%s\",\n",
               pkg->verification_code);
        printf("        \"packageVerificationCodeExcludedFiles\": []\n");
        printf("      }\n");
    } else {
        printf("      \"filesAnalyzed\": false\n");
    }
    printf("    }\n");
}

static void print_file(const FileInfo *fi, int first) {
    if (!first) printf(",\n");
    printf("    {\n");
    printf("      \"fileName\": \""); print_str(fi->name); printf("\",\n");
    printf("      \"SPDXID\": \"SPDXRef-File-%s\",\n", fi->name);
    printf("      \"fileTypes\": [\"%s\"],\n", fi->file_type);
    if (fi->sha1[0])
        printf("      \"checksums\": [{\"algorithm\": \"SHA1\", "
               "\"checksumValue\": \"%s\"}],\n", fi->sha1);
    else
        printf("      \"checksums\": [{\"algorithm\": \"SHA1\", "
               "\"checksumValue\": \"\"}],\n");
    printf("      \"licenseConcluded\": \""); print_str(fi->license); printf("\",\n");
    printf("      \"licenseInfoInFiles\": [\""); print_str(fi->license); printf("\"]");
    if (fi->copyright[0]) {
        printf(",\n      \"copyrightText\": \""); print_str(fi->copyright);
        printf("\"");
    } else {
        printf(",\n      \"copyrightText\": \"NOASSERTION\"");
    }
    printf("\n    }");
}

static void print_snippet(const SnippetInfo *s, int first) {
    if (!first) printf(",\n");
    printf("    {\n");
    printf("      \"SPDXID\": \"%s\",\n", s->spdx_id);
    printf("      \"snippetFromFile\": \"%s\",\n", s->from_file_id);
    printf("      \"ranges\": [\n");
    printf("        {\n");
    printf("          \"startPointer\": {\"reference\": \"%s\", \"offset\": %d},\n",
           s->from_file_id, s->line_start);
    printf("          \"endPointer\": {\"reference\": \"%s\", \"offset\": %d}\n",
           s->from_file_id, s->line_end);
    printf("        }\n");
    printf("      ],\n");
    printf("      \"licenseConcluded\": \"");
    print_str(s->license);
    printf("\",\n");
    printf("      \"licenseInfoInSnippets\": [\"");
    print_str(s->license);
    printf("\"]");
    if (s->copyright[0]) {
        printf(",\n      \"copyrightText\": \"");
        print_str(s->copyright);
        printf("\"\n");
    } else {
        printf(",\n      \"copyrightText\": \"NOASSERTION\"\n");
    }
    printf("    }");
}

static void print_relationship(const Relationship *r) {
    printf("    {\n");
    printf("      \"spdxElementId\": \"%s\",\n", r->element_id);
    printf("      \"relatedSpdxElement\": \"%s\",\n", r->related_element);
    printf("      \"relationshipType\": \"%s\"\n", r->relationship_type);
    printf("    }");
}

static void print_extracted_license(const ExtractedLicenseInfo *e, int first) {
    if (!first) printf(",\n");
    printf("    {\n");
    printf("      \"licenseId\": \""); print_str(e->license_id); printf("\",\n");
    if (e->name && e->name[0]) {
        printf("      \"name\": \""); print_str(e->name); printf("\",\n");
    }
    if (e->comment && e->comment[0]) {
        printf("      \"comment\": \""); print_str(e->comment); printf("\",\n");
    }
    printf("      \"extractedText\": \"");
    print_str(e->extracted_text ? e->extracted_text : "");
    printf("\"\n");
    printf("    }");
}

int sbom_output_json(const SpdxDocument *doc) {
    int i;

    printf("{\n");
    printf("  \"spdxVersion\": \"%s\",\n", doc->spdx_version);
    printf("  \"SPDXID\": \"%s\",\n", doc->document_id);
    printf("  \"name\": \"%s\",\n", doc->document_name);
    printf("  \"creationInfo\": {\n");
    printf("    \"created\": \"%s\",\n", doc->created);
    printf("    \"creators\": [\""); print_str(doc->creator); printf("\"]\n");
    printf("  },\n");
    printf("  \"dataLicense\": \"%s\",\n", doc->data_license);
    printf("  \"documentNamespace\": \"%s\",\n", doc->document_namespace);

    if (doc->has_external_ref) {
        printf("  \"externalDocumentRefs\": [\n");
        printf("    {\n");
        printf("      \"externalDocumentId\": \"%s\",\n", doc->external_doc_id);
        printf("      \"spdxDocument\": \"");
        print_str(doc->external_doc_uri);
        printf("\",\n");
        printf("      \"checksum\": {\n");
        printf("        \"algorithm\": \"SHA1\",\n");
        printf("        \"checksumValue\": \"%s\"\n", doc->external_doc_checksum);
        printf("      }\n");
        printf("    }\n");
        printf("  ],\n");
    }

    if (doc->extracted_licenses.count > 0) {
        printf("  \"hasExtractedLicensingInfos\": [\n");
        for (i = 0; i < doc->extracted_licenses.count; i++)
            print_extracted_license(&doc->extracted_licenses.items[i], i == 0);
        printf("\n  ],\n");
    }

    printf("  \"packages\": [\n");
    print_package(&doc->package);
    printf("  ],\n");

    printf("  \"files\": [\n");
    for (i = 0; i < doc->files.count; i++)
        print_file(&doc->files.items[i], i == 0);
    printf("\n  ],\n");

    if (doc->snippets.count > 0) {
        printf("  \"snippets\": [\n");
        for (i = 0; i < doc->snippets.count; i++)
            print_snippet(&doc->snippets.items[i], i == 0);
        printf("\n  ],\n");
    }

    printf("  \"relationships\": [\n");
    for (i = 0; i < doc->relationship_count; i++) {
        if (i > 0) printf(",\n");
        print_relationship(&doc->relationships[i]);
    }
    printf("\n  ]\n");

    printf("}\n");
    return 0;
}
