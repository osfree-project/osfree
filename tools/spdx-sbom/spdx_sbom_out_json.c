/* spdx_sbom_out_json.c - SBOM output in JSON format (C89) */

#include <stdio.h>
#include <stdlib.h>
#include "spdx_sbom_out.h"
#include "spdx_sbom_utils.h"
#include "ccl.h"
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
 * The document is built as a JSON tree with the json module and
 * then serialized to stdout with JsonWriteFile. No manual quoting
 * or escaping is performed here.
 */

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

/**
 * @brief Add a string field to an object.
 *
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] pszVal  Field value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] fVal    Boolean value.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hObj    Object handle. Not NULLHANDLE.
 * @param[in] pszKey  Field name. Not NULL.
 * @param[in] dVal    Numeric value.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[in]  hObj     Parent object. Not NULLHANDLE.
 * @param[in]  pszKey   Field name. Not NULL.
 * @param[out] phChild  Receiver for the new object handle. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @param[in]  hDoc     Document handle. Not NULLHANDLE.
 * @param[in]  hObj     Parent object. Not NULLHANDLE.
 * @param[in]  pszKey   Field name. Not NULL.
 * @param[out] phChild  Receiver for the new array handle. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @param[in] hDoc    Document handle. Not NULLHANDLE.
 * @param[in] hArr    Array handle. Not NULLHANDLE.
 * @param[in] pszVal  String value. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET build_external_ref(HJSONDOC hDoc, const SPDXDOCUMENT *pDoc,
                                 HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hChk = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "externalDocumentId",
                         pDoc->achExternalDocId);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "spdxDocument",
                         pDoc->achExternalDocUri);
    if (rc != NO_ERROR) return rc;
    rc = json_set_object(hDoc, hObj, "checksum", &hChk);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "algorithm", "SHA1");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "checksumValue",
                         pDoc->achExternalDocChecksum);
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET build_extracted_license(HJSONDOC hDoc,
                                      const SPDXEXTRACTEDLICENSEINFO *pE,
                                      HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "licenseId",
                         pE->pszLicenseId ? pE->pszLicenseId : "");
    if (rc != NO_ERROR) return rc;
    if (pE->pszName && pE->pszName[0]) {
        rc = json_set_string(hDoc, hObj, "name", pE->pszName);
        if (rc != NO_ERROR) return rc;
    }
    if (pE->pszComment && pE->pszComment[0]) {
        rc = json_set_string(hDoc, hObj, "comment", pE->pszComment);
        if (rc != NO_ERROR) return rc;
    }
    rc = json_set_string(hDoc, hObj, "extractedText",
                         pE->pszExtractedText ? pE->pszExtractedText : "");
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET build_package(HJSONDOC hDoc, const SPDXPACKAGEINFO *pPkg,
                            HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hChk = NULLHANDLE;
    HJSONNODE hEmpty = NULLHANDLE;
    HSTRSETENUM hEnum = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "SPDXID", pPkg->achSpdxId);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "name", pPkg->achName);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "downloadLocation", "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "versionInfo",
                         pPkg->achVersion[0] ? pPkg->achVersion
                                             : "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "supplier",
                         pPkg->achSupplier[0] ? pPkg->achSupplier
                                              : "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "licenseConcluded", pPkg->achLicense);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "licenseDeclared", pPkg->achLicense);
    if (rc != NO_ERROR) return rc;

    if (pPkg->fFilesAnalyzed &&
        pPkg->hLicenseInfoFromFiles != NULLHANDLE) {
        ULONG ulLicCount = 0;
        StrSetGetCount(pPkg->hLicenseInfoFromFiles, &ulLicCount);
        if (ulLicCount > 0) {
            rc = json_set_array(hDoc, hObj, "licenseInfoFromFiles", &hArr);
            if (rc != NO_ERROR) return rc;
            if (StrSetEnumFirst(pPkg->hLicenseInfoFromFiles, &hEnum)
                    == NO_ERROR) {
                do {
                    CHAR achLic[256];
                    if (StrSetEnumGet(hEnum, achLic, sizeof(achLic), NULL)
                            != NO_ERROR)
                        continue;
                    rc = json_append_string(hDoc, hArr, achLic);
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        return rc;
                    }
                } while (StrSetEnumNext(hEnum) == NO_ERROR);
                StrSetEnumClose(hEnum);
            }
        }
    }

    rc = json_set_string(hDoc, hObj, "copyrightText",
                         pPkg->achCopyright[0] ? pPkg->achCopyright
                                               : "NOASSERTION");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "primaryPackagePurpose",
                         pPkg->achPurpose[0] ? pPkg->achPurpose
                                             : "NOASSERTION");
    if (rc != NO_ERROR) return rc;

    if (pPkg->fFilesAnalyzed) {
        rc = json_set_bool(hDoc, hObj, "filesAnalyzed", TRUE_);
        if (rc != NO_ERROR) return rc;
        rc = json_set_object(hDoc, hObj, "packageVerificationCode", &hChk);
        if (rc != NO_ERROR) return rc;
        rc = json_set_string(hDoc, hChk,
                             "packageVerificationCodeValue",
                             pPkg->achVerificationCode);
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET build_file(HJSONDOC hDoc, const SPDXFILEINFO *pFi,
                         HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hChk = NULLHANDLE;
    CHAR achSpdxId[512];
    APIRET rc;

    snprintf(achSpdxId, sizeof(achSpdxId), "SPDXRef-File-%s", pFi->achName);

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "fileName", pFi->achName);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "SPDXID", achSpdxId);
    if (rc != NO_ERROR) return rc;

    rc = json_set_array(hDoc, hObj, "fileTypes", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pFi->achFileType);
    if (rc != NO_ERROR) return rc;

    rc = json_set_array(hDoc, hObj, "checksums", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = JsonNewObject(hDoc, &hChk);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "algorithm", "SHA1");
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hChk, "checksumValue",
                         pFi->achSha1[0] ? pFi->achSha1 : "");
    if (rc != NO_ERROR) return rc;
    rc = JsonArrayAppend(hArr, hChk);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "licenseConcluded", pFi->achLicense);
    if (rc != NO_ERROR) return rc;
    rc = json_set_array(hDoc, hObj, "licenseInfoInFiles", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pFi->achLicense);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "copyrightText",
                         pFi->achCopyright[0] ? pFi->achCopyright
                                              : "NOASSERTION");
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET build_snippet(HJSONDOC hDoc, const SPDXSNIPPETINFO *pS,
                            HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hRng = NULLHANDLE;
    HJSONNODE hPtr = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "SPDXID", pS->achSpdxId);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "snippetFromFile", pS->achFromFileId);
    if (rc != NO_ERROR) return rc;

    rc = json_set_array(hDoc, hObj, "ranges", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = JsonNewObject(hDoc, &hRng);
    if (rc != NO_ERROR) return rc;

    rc = json_set_object(hDoc, hRng, "startPointer", &hPtr);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hPtr, "reference", pS->achFromFileId);
    if (rc != NO_ERROR) return rc;
    rc = json_set_number(hDoc, hPtr, "offset", (double)pS->ulLineStart);
    if (rc != NO_ERROR) return rc;

    rc = json_set_object(hDoc, hRng, "endPointer", &hPtr);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hPtr, "reference", pS->achFromFileId);
    if (rc != NO_ERROR) return rc;
    rc = json_set_number(hDoc, hPtr, "offset", (double)pS->ulLineEnd);
    if (rc != NO_ERROR) return rc;

    rc = JsonArrayAppend(hArr, hRng);
    if (rc != NO_ERROR) return rc;

    rc = json_set_string(hDoc, hObj, "licenseConcluded", pS->achLicense);
    if (rc != NO_ERROR) return rc;
    rc = json_set_array(hDoc, hObj, "licenseInfoInSnippets", &hArr);
    if (rc != NO_ERROR) return rc;
    rc = json_append_string(hDoc, hArr, pS->achLicense);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "copyrightText",
                         pS->achCopyright[0] ? pS->achCopyright
                                             : "NOASSERTION");
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
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET build_relationship(HJSONDOC hDoc,
                                 const SPDXRELATIONSHIP *pR,
                                 HJSONNODE *phOut) {
    HJSONNODE hObj = NULLHANDLE;
    APIRET rc;

    rc = JsonNewObject(hDoc, &hObj);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "spdxElementId", pR->achElementId);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "relatedSpdxElement",
                         pR->achRelatedElement);
    if (rc != NO_ERROR) return rc;
    rc = json_set_string(hDoc, hObj, "relationshipType",
                         pR->achRelationshipType);
    if (rc != NO_ERROR) return rc;
    *phOut = hObj;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Serialize a document as SPDX 2.3 JSON to stdout.
 *
 * @param[in] pDoc  Document to serialize. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pDoc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_READ_FAULT         Write error.
 */
APIRET APIENTRY SbomOutputJson(const SPDXDOCUMENT *pDoc) {
    HJSONDOC hDoc = NULLHANDLE;
    HJSONNODE hRoot = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    HJSONNODE hChild = NULLHANDLE;
    ULONG ulCount = 0, ulIdx;
    APIRET rc;

    if (!pDoc) return ERROR_INVALID_PARAMETER;

    rc = JsonNewDoc(&hDoc);
    if (rc != NO_ERROR) return rc;

    rc = JsonNewObject(hDoc, &hRoot);
    if (rc != NO_ERROR) goto fail;

    rc = json_set_string(hDoc, hRoot, "spdxVersion",
                         pDoc->achSpdxVersion);
    if (rc != NO_ERROR) goto fail;
    rc = json_set_string(hDoc, hRoot, "SPDXID", pDoc->achDocumentId);
    if (rc != NO_ERROR) goto fail;
    rc = json_set_string(hDoc, hRoot, "name", pDoc->achDocumentName);
    if (rc != NO_ERROR) goto fail;

    rc = build_creation_info(hDoc, pDoc->achCreated, pDoc->achCreator,
                             &hChild);
    if (rc != NO_ERROR) goto fail;
    rc = JsonObjectSet(hRoot, "creationInfo", hChild);
    if (rc != NO_ERROR) goto fail;

    rc = json_set_string(hDoc, hRoot, "dataLicense", pDoc->achDataLicense);
    if (rc != NO_ERROR) goto fail;
    rc = json_set_string(hDoc, hRoot, "documentNamespace",
                         pDoc->achDocumentNamespace);
    if (rc != NO_ERROR) goto fail;

    if (pDoc->fHasExternalRef) {
        rc = json_set_array(hDoc, hRoot, "externalDocumentRefs", &hArr);
        if (rc != NO_ERROR) goto fail;
        rc = build_external_ref(hDoc, pDoc, &hChild);
        if (rc != NO_ERROR) goto fail;
        rc = JsonArrayAppend(hArr, hChild);
        if (rc != NO_ERROR) goto fail;
    }

    if (pDoc->hExtractedLicenses != NULLHANDLE) {
        rc = VectorGetCount(pDoc->hExtractedLicenses, &ulCount);
        if (rc != NO_ERROR) goto fail;
        if (ulCount > 0) {
            rc = json_set_array(hDoc, hRoot,
                                "hasExtractedLicensingInfos", &hArr);
            if (rc != NO_ERROR) goto fail;
            for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
                SPDXEXTRACTEDLICENSEINFO info;
                if (VectorGetItem(pDoc->hExtractedLicenses, ulIdx, &info,
                                  (ULONG)sizeof(info), NULL) != NO_ERROR)
                    continue;
                rc = build_extracted_license(hDoc, &info, &hChild);
                if (rc != NO_ERROR) goto fail;
                rc = JsonArrayAppend(hArr, hChild);
                if (rc != NO_ERROR) goto fail;
            }
        }
    }

    rc = json_set_array(hDoc, hRoot, "packages", &hArr);
    if (rc != NO_ERROR) goto fail;
    rc = build_package(hDoc, &pDoc->package, &hChild);
    if (rc != NO_ERROR) goto fail;
    rc = JsonArrayAppend(hArr, hChild);
    if (rc != NO_ERROR) goto fail;

    rc = json_set_array(hDoc, hRoot, "files", &hArr);
    if (rc != NO_ERROR) goto fail;
    rc = VectorGetCount(pDoc->hFiles, &ulCount);
    if (rc != NO_ERROR) goto fail;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        SPDXFILEINFO info;
        if (VectorGetItem(pDoc->hFiles, ulIdx, &info,
                          (ULONG)sizeof(info), NULL) != NO_ERROR)
            continue;
        rc = build_file(hDoc, &info, &hChild);
        if (rc != NO_ERROR) goto fail;
        rc = JsonArrayAppend(hArr, hChild);
        if (rc != NO_ERROR) goto fail;
    }

    rc = VectorGetCount(pDoc->hSnippets, &ulCount);
    if (rc != NO_ERROR) goto fail;
    if (ulCount > 0) {
        rc = json_set_array(hDoc, hRoot, "snippets", &hArr);
        if (rc != NO_ERROR) goto fail;
        for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
            SPDXSNIPPETINFO info;
            if (VectorGetItem(pDoc->hSnippets, ulIdx, &info,
                              (ULONG)sizeof(info), NULL) != NO_ERROR)
                continue;
            rc = build_snippet(hDoc, &info, &hChild);
            if (rc != NO_ERROR) goto fail;
            rc = JsonArrayAppend(hArr, hChild);
            if (rc != NO_ERROR) goto fail;
        }
    }

    rc = json_set_array(hDoc, hRoot, "relationships", &hArr);
    if (rc != NO_ERROR) goto fail;
    rc = VectorGetCount(pDoc->hRelationships, &ulCount);
    if (rc != NO_ERROR) goto fail;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        SPDXRELATIONSHIP rel;
        if (VectorGetItem(pDoc->hRelationships, ulIdx, &rel,
                          (ULONG)sizeof(rel), NULL) != NO_ERROR)
            continue;
        rc = build_relationship(hDoc, &rel, &hChild);
        if (rc != NO_ERROR) goto fail;
        rc = JsonArrayAppend(hArr, hChild);
        if (rc != NO_ERROR) goto fail;
    }

    rc = JsonSetRoot(hDoc, hRoot);
    if (rc != NO_ERROR) goto fail;

    rc = JsonWriteFile(hRoot, TRUE_, NULL);
    JsonClose(hDoc);
    return rc;

fail:
    JsonClose(hDoc);
    return rc;
}
