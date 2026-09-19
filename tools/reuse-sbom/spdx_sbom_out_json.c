/* spdx_sbom_out_json.c - SBOM output in JSON format (C89) */

#include <stdio.h>
#include <stdlib.h>
#include "spdx_sbom_out.h"
#include "spdx_sbom_utils.h"
#include "json.h"

/**
 * @file spdx_sbom_out_json.c
 * @brief SBOM serialization to SPDX 2.3 JSON.
 *
 * Conforms to:
 *   - SPDX 2.3, JSON serialization.
 *     https://spdx.github.io/spdx-spec/v2.3/
 *   - RFC 8259, "The JavaScript Object Notation (JSON) Data
 *     Interchange Format".
 *
 * The document is built as a JSON tree with the json module and then
 * serialized to stdout with JsonWriteFile. No manual quoting or
 * escaping is performed here.
 */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Add a string field to an object.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hObj   Object handle. Not NULLHANDLE.
 * @param[in] pszKey Field name. Not NULL.
 * @param[in] pszVal Field value. Not NULL.
 *
 * @return APIRET
 */
static APIRET json_set_string(HJSONDOC hDoc, HJSONNODE hObj,
                              PCSZ pszKey, PCSZ pszVal) {
    HJSONNODE hVal = NULLHANDLE;
    APIRET rc = JsonNewString(hDoc, pszVal, &hVal);
    if (rc != NO_ERROR) return rc;
    return JsonObjectSet(hObj, pszKey, hVal);
}

/**
 * @brief Add a boolean field to an object.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hObj   Object handle. Not NULLHANDLE.
 * @param[in] pszKey Field name. Not NULL.
 * @param[in] fVal   Boolean value.
 *
 * @return APIRET
 */
static APIRET json_set_bool(HJSONDOC hDoc, HJSONNODE hObj,
                            PCSZ pszKey, BOOL fVal) {
    HJSONNODE hVal = NULLHANDLE;
    APIRET rc = JsonNewBoolean(hDoc, fVal, &hVal);
    if (rc != NO_ERROR) return rc;
    return JsonObjectSet(hObj, pszKey, hVal);
}

/**
 * @brief Add a numeric field to an object.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hObj   Object handle. Not NULLHANDLE.
 * @param[in] pszKey Field name. Not NULL.
 * @param[in] dVal   Numeric value.
 *
 * @return APIRET
 */
static APIRET json_set_number(HJSONDOC hDoc, HJSONNODE hObj,
                              PCSZ pszKey, double dVal) {
    HJSONNODE hVal = NULLHANDLE;
    APIRET rc = JsonNewNumber(hDoc, dVal, &hVal);
    if (rc != NO_ERROR) return rc;
    return JsonObjectSet(hObj, pszKey, hVal);
}

/**
 * @brief Add a nested object field and return its handle.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  hObj    Parent object. Not NULLHANDLE.
 * @param[in]  pszKey  Field name. Not NULL.
 * @param[out] phChild Receiver for the new object handle. Not NULL.
 *
 * @return APIRET
 */
static APIRET json_set_object(HJSONDOC hDoc, HJSONNODE hObj,
                              PCSZ pszKey, HJSONNODE *phChild) {
    HJSONNODE hVal = NULLHANDLE;
    APIRET rc = JsonNewObject(hDoc, &hVal);
    if (rc != NO_ERROR) return rc;
    rc = JsonObjectSet(hObj, pszKey, hVal);
    if (rc != NO_ERROR) return rc;
    *phChild = hVal;
    return NO_ERROR;
}

/**
 * @brief Add a nested array field and return its handle.
 *
 * @param[in]  hDoc    Document handle. Not NULLHANDLE.
 * @param[in]  hObj    Parent object. Not NULLHANDLE.
 * @param[in]  pszKey  Field name. Not NULL.
 * @param[out] phChild Receiver for the new array handle. Not NULL.
 *
 * @return APIRET
 */
static APIRET json_set_array(HJSONDOC hDoc, HJSONNODE hObj,
                             PCSZ pszKey, HJSONNODE *phChild) {
    HJSONNODE hVal = NULLHANDLE;
    APIRET rc = JsonNewArray(hDoc, &hVal);
    if (rc != NO_ERROR) return rc;
    rc = JsonObjectSet(hObj, pszKey, hVal);
    if (rc != NO_ERROR) return rc;
    *phChild = hVal;
    return NO_ERROR;
}

/**
 * @brief Append a string to an array.
 *
 * @param[in] hDoc   Document handle. Not NULLHANDLE.
 * @param[in] hArr   Array handle. Not NULLHANDLE.
 * @param[in] pszVal String value. Not NULL.
 *
 * @return APIRET
 */
static APIRET json_append_string(HJSONDOC hDoc, HJSONNODE hArr,
                                 PCSZ pszVal) {
    HJSONNODE hVal = NULLHANDLE;
    APIRET rc = JsonNewString(hDoc, pszVal, &hVal);
    if (rc != NO_ERROR) return rc;
    return JsonArrayAppend(hArr, hVal);
}

/* ------------------------------------------------------------------ */
/* Object builders                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Build the creationInfo object.
 *
 * @param[in]  hDoc     Document. Not NULLHANDLE.
 * @param[in]  pszDate  Creation timestamp. Not NULL.
 * @param[in]  pszName  Creator name. Not NULL.
 * @param[out] phOut    Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_creation_info(HJSONDOC hDoc, PCSZ pszDate,
                                  PCSZ pszName, HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "created", pszDate);
    if (rc != NO_ERROR) return rc;
    rc = json_set_array(hDoc, hObj, "creators", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pszName);
    if (rc != NO_ERROR) return rc;
    *phOut = hObj;
    return NO_ERROR;
}

/**
 * @brief Build one externalDocumentRefs entry.
 *
 * @param[in]  hDoc  Document. Not NULLHANDLE.
 * @param[in]  pDoc  Source document. Not NULL.
 * @param[out] phOut Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_external_ref(HJSONDOC hDoc, const SpdxDocument *pDoc,
                                 HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hChk = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "externalDocumentId",
                         pDoc->external_doc_id);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "spdxDocument",
                         pDoc->external_doc_uri);
    if (rc != NO_ERROR) return rc;
    rc = json_set_object(hDoc, hObj, "checksum", &hChk);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "algorithm", "SHA1");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "checksumValue",
                         pDoc->external_doc_checksum);
    if (rc != NO_ERROR) return rc;
    *phOut = hObj;
    return NO_ERROR;
}

/**
 * @brief Build one hasExtractedLicensingInfos entry.
 *
 * @param[in]  hDoc  Document. Not NULLHANDLE.
 * @param[in]  pE    Extracted license. Not NULL.
 * @param[out] phOut Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_extracted_license(HJSONDOC hDoc,
                                      const ExtractedLicenseInfo *pE,
                                      HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "licenseId",
                         pE->license_id ? pE->license_id : "");
    if (rc != NO_ERROR) return rc;
    if (pE->name && pE->name[0]) {
        rc = json_set_string(hDoc, hObj, "name", pE->name);
        if (rc != NO_ERROR) return rc;
    }
    if (pE->comment && pE->comment[0]) {
        rc = json_set_string(hDoc, hObj, "comment", pE->comment);
        if (rc != NO_ERROR) return rc;
    }
    rc = json_set_string(hDoc, hObj, "extractedText",
                         pE->extracted_text ? pE->extracted_text : "");
    if (rc != NO_ERROR) return rc;
    *phOut = hObj;
    return NO_ERROR;
}

/**
 * @brief Build the package object.
 *
 * @param[in]  hDoc  Document. Not NULLHANDLE.
 * @param[in]  pPkg  Package information. Not NULL.
 * @param[out] phOut Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_package(HJSONDOC hDoc, const PackageInfo *pPkg,
                            HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hChk = NULLHANDLE;
    HJSONNODE hEmpty = NULLHANDLE;
    APIRET rc;
    int i;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "SPDXID", pPkg->spdx_id);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "name", pPkg->name);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "downloadLocation", "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "versionInfo",
                         pPkg->version[0] ? pPkg->version : "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "supplier",
                         pPkg->supplier[0] ? pPkg->supplier : "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "licenseConcluded", pPkg->license);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "licenseDeclared", pPkg->license);
    if (rc != NO_ERROR) return rc;

    if (pPkg->files_analyzed && pPkg->license_info_count > 0) {
        rc = json_set_array(hDoc, hObj, "licenseInfoFromFiles", &hArr);
        if (rc != NO_ERROR) return rc;
        for (i = 0; i < pPkg->license_info_count; i++) {
            rc = json_append_string(hDoc, hArr,
                                    pPkg->license_info_from_files[i]);
            if (rc != NO_ERROR) return rc;
        }
    }

    rc = json_set_string(hDoc, hObj, "copyrightText",
                         pPkg->copyright[0] ? pPkg->copyright : "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "primaryPackagePurpose",
                         pPkg->purpose[0] ? pPkg->purpose : "NOASSERTION");
    if (rc != NO_ERROR) return rc;

    if (pPkg->files_analyzed) {
        rc = json_set_bool(hDoc, hObj, "filesAnalyzed", TRUE_);
        if (rc != NO_ERROR) return rc;
        rc = json_set_object(hDoc, hObj, "packageVerificationCode", &hChk);
        if (rc != NO_ERROR) return rc;
        rc = json_set_string(hDoc, hChk,
                             "packageVerificationCodeValue",
                             pPkg->verification_code);
        if (rc != NO_ERROR) return rc;
        rc = json_set_array(hDoc, hChk,
                            "packageVerificationCodeExcludedFiles",
                            &hEmpty);
        if (rc != NO_ERROR) return rc;
    } else {
        rc = json_set_bool(hDoc, hObj, "filesAnalyzed", FALSE_);
        if (rc != NO_ERROR) return rc;
    }

    *phOut = hObj;
    return NO_ERROR;
}

/**
 * @brief Build one files entry.
 *
 * @param[in]  hDoc  Document. Not NULLHANDLE.
 * @param[in]  pFi   File information. Not NULL.
 * @param[out] phOut Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_file(HJSONDOC hDoc, const FileInfo *pFi,
                         HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hChk = NULLHANDLE;
    char spdx_id[512];
    APIRET rc;

    snprintf(spdx_id, sizeof(spdx_id), "SPDXRef-File-%s", pFi->name);

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "fileName", pFi->name);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "SPDXID", spdx_id);
    if (rc != NO_ERROR) return rc;

    rc = json_set_array(hDoc, hObj, "fileTypes", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pFi->file_type);
    if (rc != NO_ERROR) return rc;

    rc = json_set_array(hDoc, hObj, "checksums", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = JsonNewObject(hDoc, &hChk);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "algorithm", "SHA1");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "checksumValue",
                         pFi->sha1[0] ? pFi->sha1 : "");
    if (rc != NO_ERROR) return rc;
    rc = JsonArrayAppend(hArr, hChk);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "licenseConcluded", pFi->license);
    if (rc != NO_ERROR) return rc;
    rc = json_set_array(hDoc, hObj, "licenseInfoInFiles", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pFi->license);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "copyrightText",
                         pFi->copyright[0] ? pFi->copyright : "NOASSERTION");
    if (rc != NO_ERROR) return rc;

    *phOut = hObj;
    return NO_ERROR;
}

/**
 * @brief Build one snippets entry.
 *
 * @param[in]  hDoc  Document. Not NULLHANDLE.
 * @param[in]  pS    Snippet information. Not NULL.
 * @param[out] phOut Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_snippet(HJSONDOC hDoc, const SnippetInfo *pS,
                            HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hRng = NULLHANDLE;
    HJSONNODE hPtr = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "SPDXID", pS->spdx_id);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "snippetFromFile", pS->from_file_id);
    if (rc != NO_ERROR) return rc;

    rc = json_set_array(hDoc, hObj, "ranges", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = JsonNewObject(hDoc, &hRng);
    if (rc != NO_ERROR) return rc;

    rc = json_set_object(hDoc, hRng, "startPointer", &hPtr);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hPtr, "reference", pS->from_file_id);
    if (rc != NO_ERROR) return rc;
    rc = json_set_number(hDoc, hPtr, "offset", (double)pS->line_start);
    if (rc != NO_ERROR) return rc;

    rc = json_set_object(hDoc, hRng, "endPointer", &hPtr);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hPtr, "reference", pS->from_file_id);
    if (rc != NO_ERROR) return rc;
    rc = json_set_number(hDoc, hPtr, "offset", (double)pS->line_end);
    if (rc != NO_ERROR) return rc;

    rc = JsonArrayAppend(hArr, hRng);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "licenseConcluded", pS->license);
    if (rc != NO_ERROR) return rc;
    rc = json_set_array(hDoc, hObj, "licenseInfoInSnippets", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pS->license);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "copyrightText",
                         pS->copyright[0] ? pS->copyright : "NOASSERTION");
    if (rc != NO_ERROR) return rc;

    *phOut = hObj;
    return NO_ERROR;
}

/**
 * @brief Build one relationships entry.
 *
 * @param[in]  hDoc  Document. Not NULLHANDLE.
 * @param[in]  pR    Relationship. Not NULL.
 * @param[out] phOut Receiver. Not NULL.
 *
 * @return APIRET
 */
static APIRET build_relationship(HJSONDOC hDoc, const Relationship *pR,
                                 HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "spdxElementId", pR->element_id);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "relatedSpdxElement",
                         pR->related_element);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "relationshipType",
                         pR->relationship_type);
    if (rc != NO_ERROR) return rc;
    *phOut = hObj;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Serialize an SpdxDocument as SPDX 2.3 JSON to stdout.
 *
 * The whole document is built as a JSON tree and then written out
 * with JsonWriteFile. All string escaping and number formatting is
 * handled by the json module.
 *
 * @param[in] doc  Document to serialize. Not NULL.
 *
 * @return 0 on success, -1 on error.
 */
int sbom_output_json(const SpdxDocument *doc) {
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    APIRET rc;
    int i;

    rc = JsonNewDoc(&hDoc);
    if (rc != NO_ERROR) return -1;

    rc = JsonNewObject(hDoc, &hRoot);
    if (rc != NO_ERROR) goto fail;

    rc = json_set_string(hDoc, hRoot, "spdxVersion", doc->spdx_version);
    if (rc != NO_ERROR) goto fail;
    rc = json_set_string(hDoc, hRoot, "SPDXID", doc->document_id);
    if (rc != NO_ERROR) goto fail;
    rc = json_set_string(hDoc, hRoot, "name", doc->document_name);
    if (rc != NO_ERROR) goto fail;

    rc = build_creation_info(hDoc, doc->created, doc->creator, &hChild);
    if (rc != NO_ERROR) goto fail;
    rc = JsonObjectSet(hRoot, "creationInfo", hChild);
    if (rc != NO_ERROR) goto fail;

    rc = json_set_string(hDoc, hRoot, "dataLicense", doc->data_license);
    if (rc != NO_ERROR) goto fail;
    rc = json_set_string(hDoc, hRoot, "documentNamespace",
                         doc->document_namespace);
    if (rc != NO_ERROR) goto fail;

    if (doc->has_external_ref) {
        rc = json_set_array(hDoc, hRoot, "externalDocumentRefs", &hArr);
        if (rc != NO_ERROR) goto fail;
        rc = build_external_ref(hDoc, doc, &hChild);
        if (rc != NO_ERROR) goto fail;
        rc = JsonArrayAppend(hArr, hChild);
        if (rc != NO_ERROR) goto fail;
    }

    if (doc->extracted_licenses.count > 0) {
        rc = json_set_array(hDoc, hRoot,
                            "hasExtractedLicensingInfos", &hArr);
        if (rc != NO_ERROR) goto fail;
        for (i = 0; i < doc->extracted_licenses.count; i++) {
            rc = build_extracted_license(hDoc,
                                         &doc->extracted_licenses.items[i],
                                         &hChild);
            if (rc != NO_ERROR) goto fail;
            rc = JsonArrayAppend(hArr, hChild);
            if (rc != NO_ERROR) goto fail;
        }
    }

    rc = json_set_array(hDoc, hRoot, "packages", &hArr);
    if (rc != NO_ERROR) goto fail;
    rc = build_package(hDoc, &doc->package, &hChild);
    if (rc != NO_ERROR) goto fail;
    rc = JsonArrayAppend(hArr, hChild);
    if (rc != NO_ERROR) goto fail;

    rc = json_set_array(hDoc, hRoot, "files", &hArr);
    if (rc != NO_ERROR) goto fail;
    for (i = 0; i < doc->files.count; i++) {
        rc = build_file(hDoc, &doc->files.items[i], &hChild);
        if (rc != NO_ERROR) goto fail;
        rc = JsonArrayAppend(hArr, hChild);
        if (rc != NO_ERROR) goto fail;
    }

    if (doc->snippets.count > 0) {
        rc = json_set_array(hDoc, hRoot, "snippets", &hArr);
        if (rc != NO_ERROR) goto fail;
        for (i = 0; i < doc->snippets.count; i++) {
            rc = build_snippet(hDoc, &doc->snippets.items[i], &hChild);
            if (rc != NO_ERROR) goto fail;
            rc = JsonArrayAppend(hArr, hChild);
            if (rc != NO_ERROR) goto fail;
        }
    }

    rc = json_set_array(hDoc, hRoot, "relationships", &hArr);
    if (rc != NO_ERROR) goto fail;
    for (i = 0; i < doc->relationship_count; i++) {
        rc = build_relationship(hDoc, &doc->relationships[i], &hChild);
        if (rc != NO_ERROR) goto fail;
        rc = JsonArrayAppend(hArr, hChild);
        if (rc != NO_ERROR) goto fail;
    }

    rc = JsonSetRoot(hDoc, hRoot);
    if (rc != NO_ERROR) goto fail;

    rc = JsonWriteFile(hRoot, TRUE_, NULL);
    if (rc != NO_ERROR) goto fail;

    JsonClose(hDoc);
    return 0;

fail:
    JsonClose(hDoc);
    return -1;
}
