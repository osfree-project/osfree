/* spdx_sbom_types.h - общие структуры SPDX SBOM (C89) */
#ifndef SPDX_SBOM_TYPES_H
#define SPDX_SBOM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[512];
    char sha1[41];
    char license[256];
    char copyright[512];
    char file_type[32];
} FileInfo;

typedef struct {
    FileInfo *items;
    int count;
    int capacity;
} FileList;

typedef struct {
    char spdx_id[256];
    char name[512];
    char version[128];
    char supplier[256];
    char license[256];
    char copyright[512];
    char purpose[128];
    int files_analyzed;
    char verification_code[41];
} PackageInfo;

typedef struct {
    char element_id[256];
    char related_element[256];
    char relationship_type[64];
} Relationship;

typedef struct {
    char *license_id;
    char *extracted_text;
    char *name;
    char *comment;
} ExtractedLicenseInfo;

typedef struct {
    ExtractedLicenseInfo *items;
    int count;
    int capacity;
} ExtractedLicenseList;

typedef struct {
    char id[256];
    char path[1024];
} ExtractedLicenseSource;

typedef struct {
    char spdx_version[16];
    char document_id[256];
    char document_name[512];
    char document_namespace[512];
    char created[32];
    char creator[256];
    char data_license[64];

    PackageInfo package;
    FileList files;
    Relationship *relationships;
    int relationship_count;

    int has_external_ref;
    char external_doc_id[256];
    char external_doc_uri[512];
    char external_doc_checksum[128];

    ExtractedLicenseList extracted_licenses;
} SpdxDocument;

#ifdef __cplusplus
}
#endif

#endif /* SPDX_SBOM_TYPES_H */
