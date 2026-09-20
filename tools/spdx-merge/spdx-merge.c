/* spdx-merge.c - merge SPDX JSON documents (C89, OpenWatcom) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "os2types.h"
#include "os2err.h"
#include "json.h"
#include "sha1.h"
#include "sha256.h"
#include "spdx_db.h"
#include "spdx.h"
#include "ccl.h"
#include "path.h"

/**
 * @file spdx-merge.c
 * @brief Merge SPDX JSON documents.
 *
 * The merge walks externalDocumentRefs recursively, verifies each
 * checksum, resolves SPDXID collisions by renaming, and outputs the
 * merged document as SPDX 2.3 JSON.
 *
 * Conforms to:
 *   - SPDX 2.3, §6.6 (external document references), §11
 *     (relationships), Annex I (checksum algorithms).
 *     https://spdx.github.io/spdx-spec/v2.3/
 */

#define MAX_DOCS 100

/* ==================================================================
 * File helpers
 * ================================================================== */

/**
 * @brief Read a whole file into a heap string.
 *
 * Uses the size-query convention of SpdxReadFileAll.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return malloc'd NUL-terminated content, or NULL on error.
 */
static PSZ read_file_to_heap(PCSZ pszPath) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxReadFileAll(pszPath, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxReadFileAll(pszPath, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    return pszOut;
}

/* ==================================================================
 * Processed / Rename
 * ================================================================== */

/**
 * @struct _SPDXPROCESSEDLIST
 * @brief List of already-processed file paths.
 */
typedef struct _SPDXPROCESSEDLIST {
    PSZ  *papszPaths;
    ULONG ulCount;
} SPDXPROCESSEDLIST, *PSPDXPROCESSEDLIST;

/**
 * @struct _SPDXRENAMEMAP
 * @brief Parallel arrays of old and new SPDX identifiers.
 */
typedef struct _SPDXRENAMEMAP {
    PSZ  *papszOldIds;
    PSZ  *papszNewIds;
    ULONG ulCount;
} SPDXRENAMEMAP, *PSPDXRENAMEMAP;

/**
 * @brief Query whether a path has already been processed.
 *
 * @param[in] pList    List. Not NULL.
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return TRUE_ if already processed.
 */
static BOOL IsProcessed(const SPDXPROCESSEDLIST *pList, PCSZ pszPath) {
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < pList->ulCount; ulIdx++)
        if (strcmp(pList->papszPaths[ulIdx], pszPath) == 0) return TRUE_;
    return FALSE_;
}

/**
 * @brief Record a path as processed.
 *
 * Terminates the process on limit overflow or allocation failure.
 *
 * @param[in,out] pList    List. Not NULL.
 * @param[in]     pszPath  Path. Not NULL.
 */
static void AddProcessed(SPDXPROCESSEDLIST *pList, PCSZ pszPath) {
    if (pList->ulCount >= MAX_DOCS) {
        fprintf(stderr,
                "ERROR: too many documents to merge (limit: %d).\n"
                "       Reduce the number of externalDocumentRefs.\n",
                MAX_DOCS);
        exit(EXIT_FAILURE);
    }
    pList->papszPaths[pList->ulCount] = strdup(pszPath);
    pList->ulCount++;
}

/**
 * @brief Build a unique replacement for a colliding SPDXID.
 *
 * @param[in] pszBase     Base identifier. Not NULL.
 * @param[in] ulCounter   Disambiguating counter.
 *
 * @return malloc'd identifier owned by the caller, or NULL on
 *         allocation failure.
 */
static PSZ MakeUniqueId(PCSZ pszBase, ULONG ulCounter) {
    PSZ pszResult = (PSZ)malloc(strlen(pszBase) + 24);
    if (!pszResult) return NULL;
    sprintf(pszResult, "%s-Duplicate%lu", pszBase,
            (unsigned long)ulCounter);
    return pszResult;
}

/**
 * @brief Record a rename.
 *
 * @param[in,out] pMap        Map. Not NULL.
 * @param[in]     pszOldId    Old identifier. Not NULL.
 * @param[in]     pszNewId    New identifier. Not NULL.
 */
static void AddRename(SPDXRENAMEMAP *pMap, PCSZ pszOldId, PCSZ pszNewId) {
    pMap->papszOldIds = (PSZ*)realloc(pMap->papszOldIds,
        (pMap->ulCount + 1) * sizeof(PSZ));
    pMap->papszNewIds = (PSZ*)realloc(pMap->papszNewIds,
        (pMap->ulCount + 1) * sizeof(PSZ));
    pMap->papszOldIds[pMap->ulCount] = strdup(pszOldId);
    pMap->papszNewIds[pMap->ulCount] = strdup(pszNewId);
    pMap->ulCount++;
}

/**
 * @brief Query the new identifier for an old one.
 *
 * @param[in] pMap      Map. Not NULL.
 * @param[in] pszOldId  Old identifier. Not NULL.
 *
 * @return New identifier, or NULL if no rename recorded.
 */
static PCSZ QueryRenamed(const SPDXRENAMEMAP *pMap, PCSZ pszOldId) {
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < pMap->ulCount; ulIdx++)
        if (strcmp(pMap->papszOldIds[ulIdx], pszOldId) == 0)
            return pMap->papszNewIds[ulIdx];
    return NULL;
}

/**
 * @brief Release all memory owned by a rename map.
 *
 * @param[in,out] pMap  Map. Not NULL.
 */
static void FreeRenameMap(SPDXRENAMEMAP *pMap) {
    ULONG ulIdx;
    for (ulIdx = 0; ulIdx < pMap->ulCount; ulIdx++) {
        free(pMap->papszOldIds[ulIdx]);
        free(pMap->papszNewIds[ulIdx]);
    }
    free(pMap->papszOldIds);
    free(pMap->papszNewIds);
    pMap->ulCount = 0;
}

/* ==================================================================
 * JSON read helpers
 * ================================================================== */

/**
 * @brief Read a string field into a fixed-size buffer.
 *
 * @param[in]  hNode    Node handle, or NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hNode is NULLHANDLE.
 * @retval ERROR_INVALID_DATA       Node is not a string.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
static APIRET ReadJsonString(HJSONNODE hNode, PSZ pszBuf, ULONG ulSize) {
    if (hNode == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    return JsonNodeGetString(hNode, pszBuf, ulSize, NULL);
}

/* ==================================================================
 * DocumentRef- handling
 * ================================================================== */

/**
 * @brief Strip a leading "DocumentRef-<id>:" prefix in place.
 *
 * @param[in,out] pszStr  String to modify. Not NULL.
 */
static void StripDocumentRef(PSZ pszStr) {
    PSZ pszColon;
    if (strncmp(pszStr, "DocumentRef-", 12) == 0) {
        pszColon = strchr(pszStr, ':');
        if (pszColon)
            memmove(pszStr, pszColon + 1, strlen(pszColon + 1) + 1);
    }
}

/**
 * @brief Recursively replace renamed SPDXIDs inside a subtree.
 *
 * Only string nodes whose key is one of the ID-carrying fields are
 * modified.
 *
 * @param[in] hNode  Root of the subtree. Not NULLHANDLE.
 * @param[in] pMap   Rename map. Not NULL.
 */
static void ReplaceIdsInNode(HJSONNODE hNode, const SPDXRENAMEMAP *pMap) {
    ULONG ulType = 0;
    ULONG ulCount = 0;
    ULONG ulIdx;

    if (hNode == NULLHANDLE) return;
    if (JsonNodeGetType(hNode, &ulType) != NO_ERROR) return;

    if (ulType == (ULONG)JSON_STRING) {
        CHAR achKey[64];
        if (JsonNodeGetKey(hNode, achKey, sizeof(achKey), NULL) == NO_ERROR) {
            if (strcmp(achKey, "SPDXID") == 0 ||
                strcmp(achKey, "spdxElementId") == 0 ||
                strcmp(achKey, "relatedSpdxElement") == 0 ||
                strcmp(achKey, "spdxId") == 0 ||
                strcmp(achKey, "element") == 0 ||
                strcmp(achKey, "relatedElement") == 0) {
                CHAR achVal[512];
                PCSZ pszNewId;
                if (ReadJsonString(hNode, achVal, sizeof(achVal))
                        == NO_ERROR) {
                    StripDocumentRef(achVal);
                    pszNewId = QueryRenamed(pMap, achVal);
                    if (pszNewId)
                        JsonNodeSetValueString(hNode, pszNewId);
                }
            }
        }
        return;
    }

    if (ulType != (ULONG)JSON_OBJECT && ulType != (ULONG)JSON_ARRAY) return;

    if (JsonNodeGetCount(hNode, &ulCount) != NO_ERROR) return;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hChild = NULLHANDLE;
        if (ulType == (ULONG)JSON_OBJECT) {
            CHAR achChildKey[256];
            if (JsonNodeGetEntry(hNode, ulIdx, achChildKey,
                                 sizeof(achChildKey), NULL,
                                 &hChild) != NO_ERROR)
                continue;
        } else {
            if (JsonNodeGetElement(hNode, ulIdx, &hChild) != NO_ERROR)
                continue;
        }
        ReplaceIdsInNode(hChild, pMap);
    }
}

/* ==================================================================
 * Checksum verification
 * ================================================================== */

/**
 * @brief Verify a checksum against the file content.
 *
 * Terminates the process on any mismatch or I/O error.
 *
 * @param[in] pszFilePath      Path to the file. Not NULL.
 * @param[in] hChecksumNode    Object with 'algorithm' and
 *                             'checksumValue'. Not NULLHANDLE.
 */
static void VerifyChecksum(PCSZ pszFilePath, HJSONNODE hChecksumNode) {
    HJSONNODE hAlgoNode = NULLHANDLE;
    HJSONNODE hValueNode = NULLHANDLE;
    CHAR achAlgo[32];
    CHAR achExpected[256];
    PSZ pszActual = NULL;

    if (JsonNodeGetChild(hChecksumNode, "algorithm", &hAlgoNode)
            != NO_ERROR ||
        JsonNodeGetChild(hChecksumNode, "checksumValue", &hValueNode)
            != NO_ERROR) {
        fprintf(stderr,
                "ERROR: invalid checksum in externalDocumentRef.\n"
                "       Both 'algorithm' and 'checksumValue' fields are "
                "required.\n");
        exit(EXIT_FAILURE);
    }
    if (ReadJsonString(hAlgoNode, achAlgo, sizeof(achAlgo)) != NO_ERROR ||
        ReadJsonString(hValueNode, achExpected, sizeof(achExpected))
            != NO_ERROR) {
        fprintf(stderr,
                "ERROR: invalid checksum values in externalDocumentRef.\n"
                "       'algorithm' and 'checksumValue' must be strings.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(achAlgo, "SHA1") == 0) {
        CHAR achHex[41];
        if (Sha1File(pszFilePath, achHex, sizeof(achHex), NULL)
                == NO_ERROR)
            pszActual = strdup(achHex);
    } else if (strcmp(achAlgo, "SHA256") == 0) {
        CHAR achHex[65];
        if (Sha256File(pszFilePath, achHex, sizeof(achHex), NULL)
                == NO_ERROR)
            pszActual = strdup(achHex);
    } else {
        fprintf(stderr,
                "ERROR: unsupported checksum algorithm: %s\n"
                "       Supported: SHA1, SHA256.\n", achAlgo);
        exit(EXIT_FAILURE);
    }
    if (!pszActual) {
        fprintf(stderr,
                "ERROR: cannot compute %s for %s\n"
                "       Check that the file exists and is readable.\n",
                achAlgo, pszFilePath);
        exit(EXIT_FAILURE);
    }
    if (strcmp(pszActual, achExpected) != 0) {
        fprintf(stderr,
                "ERROR: checksum mismatch for %s\n"
                "       Expected: %s\n"
                "       Actual:   %s\n"
                "       The external document has changed since the "
                "reference was recorded.\n",
                pszFilePath, achExpected, pszActual);
        free(pszActual);
        exit(EXIT_FAILURE);
    }
    free(pszActual);
}

/* ==================================================================
 * Document validation
 * ================================================================== */

/**
 * @brief Collect all local SPDXIDs from packages, files and snippets.
 *
 * @param[in] hRoot   Document root. Not NULLHANDLE.
 * @param[in] hKnown  Destination set. Not NULLHANDLE.
 */
static void CollectLocalIds(HJSONNODE hRoot, HSTRSET hKnown) {
    static PCSZ apszSections[] = { "packages", "files", "snippets", NULL };
    int nSecIdx;

    StrSetAdd(hKnown, "SPDXRef-DOCUMENT");

    for (nSecIdx = 0; apszSections[nSecIdx]; nSecIdx++) {
        HJSONNODE hArr = NULLHANDLE;
        ULONG ulCount = 0, ulIdx;
        if (JsonNodeGetChild(hRoot, apszSections[nSecIdx], &hArr)
                != NO_ERROR)
            continue;
        if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) continue;
        for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
            HJSONNODE hItem = NULLHANDLE;
            HJSONNODE hId = NULLHANDLE;
            CHAR achId[512];
            if (JsonNodeGetElement(hArr, ulIdx, &hItem) != NO_ERROR)
                continue;
            if (JsonNodeGetChild(hItem, "SPDXID", &hId) != NO_ERROR)
                continue;
            if (ReadJsonString(hId, achId, sizeof(achId)) == NO_ERROR)
                StrSetAdd(hKnown, achId);
        }
    }
}

/**
 * @brief Collect all externalDocumentIds from a document.
 *
 * @param[in] hRoot        Document root. Not NULLHANDLE.
 * @param[in] hExternalIds Destination set. Not NULLHANDLE.
 */
static void CollectExternalIds(HJSONNODE hRoot, HSTRSET hExternalIds) {
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0, ulIdx;

    if (JsonNodeGetChild(hRoot, "externalDocumentRefs", &hArr)
            != NO_ERROR)
        return;
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) return;
    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hRef = NULLHANDLE;
        HJSONNODE hId = NULLHANDLE;
        CHAR achId[512];
        if (JsonNodeGetElement(hArr, ulIdx, &hRef) != NO_ERROR)
            continue;
        if (JsonNodeGetChild(hRef, "externalDocumentId", &hId)
                != NO_ERROR)
            continue;
        if (ReadJsonString(hId, achId, sizeof(achId)) == NO_ERROR)
            StrSetAdd(hExternalIds, achId);
    }
}

/**
 * @brief Query whether a string is present in a set.
 *
 * @param[in] hSet    Set. May be NULLHANDLE.
 * @param[in] pszStr  String. Not NULL.
 *
 * @return TRUE_ if present.
 */
static BOOL SetContains(HSTRSET hSet, PCSZ pszStr) {
    BOOL fFound = FALSE_;
    if (hSet == NULLHANDLE) return FALSE_;
    if (StrSetContains(hSet, pszStr, &fFound) != NO_ERROR) return FALSE_;
    return fFound ? TRUE_ : FALSE_;
}

/**
 * @brief Validate relationships in a document.
 *
 * Terminates the process on any inconsistency.
 *
 * @param[in] hRoot        Document root. Not NULLHANDLE.
 * @param[in] pszFilePath  Path used in diagnostics. Not NULL.
 * @param[in] hKnown       Set of local identifiers. Not NULLHANDLE.
 * @param[in] hExternalIds Set of external identifiers. Not NULLHANDLE.
 */
static void ValidateRelationships(HJSONNODE hRoot, PCSZ pszFilePath,
                                  HSTRSET hKnown, HSTRSET hExternalIds) {
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0, ulIdx;

    if (JsonNodeGetChild(hRoot, "relationships", &hArr) != NO_ERROR)
        return;
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) return;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hRel = NULLHANDLE;
        HJSONNODE hA = NULLHANDLE;
        HJSONNODE hB = NULLHANDLE;
        CHAR achA[512], achB[512];

        if (JsonNodeGetElement(hArr, ulIdx, &hRel) != NO_ERROR)
            continue;

        if (JsonNodeGetChild(hRel, "spdxElementId", &hA) != NO_ERROR ||
            JsonNodeGetChild(hRel, "relatedSpdxElement", &hB)
                != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: %s: relationship #%lu missing "
                    "'spdxElementId' or 'relatedSpdxElement'.\n",
                    pszFilePath, (unsigned long)ulIdx);
            exit(EXIT_FAILURE);
        }
        if (ReadJsonString(hA, achA, sizeof(achA)) != NO_ERROR ||
            ReadJsonString(hB, achB, sizeof(achB)) != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: %s: relationship #%lu has non-string IDs.\n",
                    pszFilePath, (unsigned long)ulIdx);
            exit(EXIT_FAILURE);
        }

        if (strncmp(achA, "DocumentRef-", 12) == 0) {
            CHAR achDocref[256];
            PCSZ pszColon = strchr(achA, ':');
            size_t cbN;
            if (!pszColon) {
                fprintf(stderr,
                        "ERROR: %s: malformed DocumentRef in "
                        "spdxElementId: %s\n", pszFilePath, achA);
                exit(EXIT_FAILURE);
            }
            cbN = (size_t)(pszColon - achA);
            if (cbN >= sizeof(achDocref)) cbN = sizeof(achDocref) - 1;
            memcpy(achDocref, achA, cbN);
            achDocref[cbN] = '\0';
            if (!SetContains(hExternalIds, achDocref)) {
                fprintf(stderr,
                        "ERROR: %s: relationship references unresolved "
                        "externalDocumentRef '%s'.\n",
                        pszFilePath, achDocref);
                exit(EXIT_FAILURE);
            }
        } else if (!SetContains(hKnown, achA)) {
            fprintf(stderr,
                    "ERROR: %s: relationship references unknown "
                    "SPDXID '%s'.\n", pszFilePath, achA);
            exit(EXIT_FAILURE);
        }

        if (strncmp(achB, "DocumentRef-", 12) == 0) {
            CHAR achDocref[256];
            PCSZ pszColon = strchr(achB, ':');
            size_t cbN;
            if (!pszColon) {
                fprintf(stderr,
                        "ERROR: %s: malformed DocumentRef in "
                        "relatedSpdxElement: %s\n", pszFilePath, achB);
                exit(EXIT_FAILURE);
            }
            cbN = (size_t)(pszColon - achB);
            if (cbN >= sizeof(achDocref)) cbN = sizeof(achDocref) - 1;
            memcpy(achDocref, achB, cbN);
            achDocref[cbN] = '\0';
            if (!SetContains(hExternalIds, achDocref)) {
                fprintf(stderr,
                        "ERROR: %s: relationship references unresolved "
                        "externalDocumentRef '%s'.\n",
                        pszFilePath, achDocref);
                exit(EXIT_FAILURE);
            }
        } else if (!SetContains(hKnown, achB)) {
            fprintf(stderr,
                    "ERROR: %s: relationship references unknown "
                    "SPDXID '%s'.\n", pszFilePath, achB);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Validate one SPDX license expression in a document field.
 *
 * Terminates the process on any violation.
 *
 * @param[in] pszFilePath  Path used in diagnostics. Not NULL.
 * @param[in] pszField     Field name. Not NULL.
 * @param[in] pszValue     Value, or NULL to skip.
 */
static void ValidateLicenseField(PCSZ pszFilePath, PCSZ pszField,
                                 PCSZ pszValue) {
    PCSZ pszBad = NULL;
    APIRET rc;

    if (!pszValue) return;
    rc = SpdxQueryExpression(pszValue, &pszBad);
    if (rc == SPDX_EXPR_SYNTAX_ERROR) {
        fprintf(stderr,
                "ERROR: %s: invalid SPDX expression in %s: '%s'\n"
                "       See https://spdx.github.io/spdx-spec/v2.3/"
                "SPDX-license-expressions/ for the grammar.\n",
                pszFilePath, pszField, pszValue);
        exit(EXIT_FAILURE);
    }
    if (rc == SPDX_EXPR_UNKNOWN_TOKEN) {
        PCSZ pszPos = pszBad;
        while (*pszPos && *pszPos != ' ' && *pszPos != '(' &&
               *pszPos != ')')
            pszPos++;
        fprintf(stderr,
                "ERROR: %s: unknown SPDX identifier in %s: '",
                pszFilePath, pszField);
        fwrite(pszBad, 1, (size_t)(pszPos - pszBad), stderr);
        fprintf(stderr,
                "'\n"
                "       See https://spdx.org/licenses/ for the full list.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Validate one whole document.
 *
 * Terminates the process on any inconsistency.
 *
 * @param[in] hRoot        Document root. Not NULLHANDLE.
 * @param[in] pszFilePath  Path used in diagnostics. Not NULL.
 */
static void ValidateDocument(HJSONNODE hRoot, PCSZ pszFilePath) {
    HSTRSET hKnown = NULLHANDLE;
    HSTRSET hExternalIds = NULLHANDLE;
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0, ulIdx;

    if (StrSetCreate(&hKnown) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }
    if (StrSetCreate(&hExternalIds) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        StrSetDestroy(hKnown);
        exit(EXIT_FAILURE);
    }

    CollectLocalIds(hRoot, hKnown);
    CollectExternalIds(hRoot, hExternalIds);

    if (JsonNodeGetChild(hRoot, "packages", &hArr) == NO_ERROR &&
        JsonNodeGetCount(hArr, &ulCount) == NO_ERROR) {
        for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
            HJSONNODE hItem = NULLHANDLE;
            HJSONNODE hField = NULLHANDLE;
            CHAR achVal[512];
            if (JsonNodeGetElement(hArr, ulIdx, &hItem) != NO_ERROR)
                continue;
            if (JsonNodeGetChild(hItem, "licenseConcluded", &hField)
                    == NO_ERROR &&
                ReadJsonString(hField, achVal, sizeof(achVal)) == NO_ERROR)
                ValidateLicenseField(pszFilePath,
                                     "package.licenseConcluded", achVal);
            if (JsonNodeGetChild(hItem, "licenseDeclared", &hField)
                    == NO_ERROR &&
                ReadJsonString(hField, achVal, sizeof(achVal)) == NO_ERROR)
                ValidateLicenseField(pszFilePath,
                                     "package.licenseDeclared", achVal);
        }
    }

    if (JsonNodeGetChild(hRoot, "files", &hArr) == NO_ERROR &&
        JsonNodeGetCount(hArr, &ulCount) == NO_ERROR) {
        for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
            HJSONNODE hItem = NULLHANDLE;
            HJSONNODE hField = NULLHANDLE;
            CHAR achVal[512];
            if (JsonNodeGetElement(hArr, ulIdx, &hItem) != NO_ERROR)
                continue;
            if (JsonNodeGetChild(hItem, "licenseConcluded", &hField)
                    == NO_ERROR &&
                ReadJsonString(hField, achVal, sizeof(achVal)) == NO_ERROR)
                ValidateLicenseField(pszFilePath,
                                     "file.licenseConcluded", achVal);
            if (JsonNodeGetChild(hItem, "licenseInfoInFiles", &hField)
                    == NO_ERROR) {
                ULONG ulLic = 0, ulJ;
                if (JsonNodeGetCount(hField, &ulLic) == NO_ERROR) {
                    for (ulJ = 0; ulJ < ulLic; ulJ++) {
                        HJSONNODE hElem = NULLHANDLE;
                        if (JsonNodeGetElement(hField, ulJ, &hElem)
                                != NO_ERROR)
                            continue;
                        if (ReadJsonString(hElem, achVal, sizeof(achVal))
                                == NO_ERROR)
                            ValidateLicenseField(pszFilePath,
                                                 "file.licenseInfoInFiles",
                                                 achVal);
                    }
                }
            }
        }
    }

    ValidateRelationships(hRoot, pszFilePath, hKnown, hExternalIds);

    StrSetDestroy(hKnown);
    StrSetDestroy(hExternalIds);
}

/* ==================================================================
 * Merge
 * ================================================================== */

/**
 * @brief Ensure that a merged array exists at a section.
 *
 * @param[in]  hDocMerged    Merged document. Not NULLHANDLE.
 * @param[in]  hMergedRoot   Merged root object. Not NULLHANDLE.
 * @param[in]  pszSection    Section name. Not NULL.
 * @param[out] phMergedArr   Receiver for the merged array. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET EnsureMergedArray(HJSONDOC hDocMerged,
                                HJSONNODE hMergedRoot,
                                PCSZ pszSection,
                                HJSONNODE *phMergedArr) {
    APIRET rc;
    if (JsonNodeGetChild(hMergedRoot, pszSection, phMergedArr) == NO_ERROR)
        return NO_ERROR;
    rc = JsonNewArray(hDocMerged, phMergedArr);
    if (rc != NO_ERROR) return rc;
    return JsonObjectSet(hMergedRoot, pszSection, *phMergedArr);
}

/**
 * @brief Merge one source subtree array into the destination array.
 *
 * Each source element is checked for SPDXID collision against the
 * already-merged array. On collision, the element is cloned with a
 * new unique SPDXID and the rename is recorded.
 *
 * @param[in] hDocMerged   Merged document. Not NULLHANDLE.
 * @param[in] hSrcArr      Source array. Not NULLHANDLE.
 * @param[in] hMergedArr   Destination array. Not NULLHANDLE.
 * @param[in] pMap         Rename map. Not NULL.
 * @param[in] pszFilePath  Source file path. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_DATA       Source element lacks a valid SPDXID.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET MergeSection(HJSONDOC hDocMerged, HJSONNODE hSrcArr,
                           HJSONNODE hMergedArr, SPDXRENAMEMAP *pMap,
                           PCSZ pszFilePath) {
    ULONG ulCount = 0, ulIdx;
    if (JsonNodeGetCount(hSrcArr, &ulCount) != NO_ERROR) return NO_ERROR;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hItem = NULLHANDLE;
        HJSONNODE hId = NULLHANDLE;
        CHAR achOldId[512];
        PCSZ pszExisting;

        if (JsonNodeGetElement(hSrcArr, ulIdx, &hItem) != NO_ERROR)
            continue;
        if (JsonNodeGetChild(hItem, "SPDXID", &hId) != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: %s: element without SPDXID.\n", pszFilePath);
            return ERROR_INVALID_DATA;
        }
        if (ReadJsonString(hId, achOldId, sizeof(achOldId)) != NO_ERROR) {
            fprintf(stderr,
                    "ERROR: %s: invalid SPDXID.\n", pszFilePath);
            return ERROR_INVALID_DATA;
        }

        pszExisting = QueryRenamed(pMap, achOldId);
        if (pszExisting == NULL) {
            BOOL fExists = FALSE_;
            ULONG ulJ, ulMCount = 0;
            if (JsonNodeGetCount(hMergedArr, &ulMCount) == NO_ERROR) {
                for (ulJ = 0; ulJ < ulMCount; ulJ++) {
                    HJSONNODE hEx = NULLHANDLE;
                    HJSONNODE hExId = NULLHANDLE;
                    CHAR achEx[512];
                    if (JsonNodeGetElement(hMergedArr, ulJ, &hEx)
                            != NO_ERROR)
                        continue;
                    if (JsonNodeGetChild(hEx, "SPDXID", &hExId)
                            != NO_ERROR)
                        continue;
                    if (ReadJsonString(hExId, achEx, sizeof(achEx))
                            != NO_ERROR)
                        continue;
                    if (strcmp(achEx, achOldId) == 0) {
                        fExists = TRUE_;
                        break;
                    }
                }
            }
            if (fExists) {
                PSZ pszNewId = MakeUniqueId(achOldId, ulIdx);
                HJSONNODE hClone = NULLHANDLE;
                APIRET rc;
                AddRename(pMap, achOldId, pszNewId);
                rc = JsonCloneNode(hDocMerged, hItem, &hClone);
                if (rc != NO_ERROR) { free(pszNewId); return rc; }
                rc = JsonNodeSetString(hDocMerged, hClone, "SPDXID",
                                       pszNewId);
                if (rc != NO_ERROR) { free(pszNewId); return rc; }
                rc = JsonArrayAppend(hMergedArr, hClone);
                free(pszNewId);
                if (rc != NO_ERROR) return rc;
            } else {
                HJSONNODE hClone = NULLHANDLE;
                APIRET rc = JsonCloneNode(hDocMerged, hItem, &hClone);
                if (rc != NO_ERROR) return rc;
                rc = JsonArrayAppend(hMergedArr, hClone);
                if (rc != NO_ERROR) return rc;
            }
        } else {
            HJSONNODE hClone = NULLHANDLE;
            APIRET rc = JsonCloneNode(hDocMerged, hItem, &hClone);
            if (rc != NO_ERROR) return rc;
            rc = JsonNodeSetString(hDocMerged, hClone, "SPDXID",
                                   pszExisting);
            if (rc != NO_ERROR) return rc;
            rc = JsonArrayAppend(hMergedArr, hClone);
            if (rc != NO_ERROR) return rc;
        }
    }
    return NO_ERROR;
}

/**
 * @brief Merge relationships from a source document.
 *
 * @param[in] hDocMerged   Merged document. Not NULLHANDLE.
 * @param[in] hSrcRoot     Source root object. Not NULLHANDLE.
 * @param[in] hMergedArr   Destination array. Not NULLHANDLE.
 * @param[in] pMap         Rename map. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET MergeRelationships(HJSONDOC hDocMerged, HJSONNODE hSrcRoot,
                                 HJSONNODE hMergedArr,
                                 SPDXRENAMEMAP *pMap) {
    HJSONNODE hArr = NULLHANDLE;
    ULONG ulCount = 0, ulIdx;

    if (JsonNodeGetChild(hSrcRoot, "relationships", &hArr) != NO_ERROR)
        return NO_ERROR;
    if (JsonNodeGetCount(hArr, &ulCount) != NO_ERROR) return NO_ERROR;

    for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
        HJSONNODE hRel = NULLHANDLE;
        HJSONNODE hClone = NULLHANDLE;
        APIRET rc;
        if (JsonNodeGetElement(hArr, ulIdx, &hRel) != NO_ERROR)
            continue;
        rc = JsonCloneNode(hDocMerged, hRel, &hClone);
        if (rc != NO_ERROR) return rc;
        ReplaceIdsInNode(hClone, pMap);
        rc = JsonArrayAppend(hMergedArr, hClone);
        if (rc != NO_ERROR) return rc;
    }
    return NO_ERROR;
}

/**
 * @brief Recursively process one SPDX document.
 *
 * Terminates the process on any error.
 *
 * @param[in] pszFilePath  File path. Not NULL.
 * @param[in] hRoot        Document root. Not NULLHANDLE.
 * @param[in] hDocMerged   Destination document. Not NULLHANDLE.
 * @param[in] hMergedRoot  Destination root. Not NULLHANDLE.
 * @param[in] pList        Processed-path list. Not NULL.
 * @param[in] pMap         Rename map. Not NULL.
 */
static void ProcessDocument(PCSZ pszFilePath, HJSONNODE hRoot,
                            HJSONDOC hDocMerged, HJSONNODE hMergedRoot,
                            SPDXPROCESSEDLIST *pList,
                            SPDXRENAMEMAP *pMap) {
    HJSONNODE hExtRefs = NULLHANDLE;
    HJSONNODE hPackages = NULLHANDLE;
    HJSONNODE hFiles = NULLHANDLE;
    HJSONNODE hSnippets = NULLHANDLE;
    HJSONNODE hMergedArr = NULLHANDLE;
    PSZ pszAbsPath;
    ULONG ulPathSize = 0;
    ULONG ulCount = 0, ulIdx;
    APIRET rc;

    if (PathNormalize(pszFilePath, NULL, 0, &ulPathSize) != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot normalize path: %s\n", pszFilePath);
        exit(EXIT_FAILURE);
    }
    pszAbsPath = (PSZ)malloc(ulPathSize);
    if (!pszAbsPath) {
        fprintf(stderr, "ERROR: out of memory\n");
        exit(EXIT_FAILURE);
    }
    if (PathNormalize(pszFilePath, pszAbsPath, ulPathSize, NULL)
            != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot normalize path: %s\n", pszFilePath);
        free(pszAbsPath);
        exit(EXIT_FAILURE);
    }

    if (IsProcessed(pList, pszAbsPath)) { free(pszAbsPath); return; }
    AddProcessed(pList, pszAbsPath);
    free(pszAbsPath);

    ValidateDocument(hRoot, pszFilePath);

    if (JsonNodeGetChild(hRoot, "externalDocumentRefs", &hExtRefs)
            == NO_ERROR &&
        JsonNodeGetCount(hExtRefs, &ulCount) == NO_ERROR) {
        for (ulIdx = 0; ulIdx < ulCount; ulIdx++) {
            HJSONNODE hRef = NULLHANDLE;
            HJSONNODE hIdNode = NULLHANDLE;
            HJSONNODE hDocNode = NULLHANDLE;
            HJSONNODE hChkNode = NULLHANDLE;
            CHAR achDocUri[2048];
            PSZ pszFullDocPath;
            PSZ pszDocText = NULL;
            HJSONDOC hExtDoc = NULLHANDLE;
            HJSONNODE hExtRoot = NULLHANDLE;

            if (JsonNodeGetElement(hExtRefs, ulIdx, &hRef) != NO_ERROR)
                continue;
            if (JsonNodeGetChild(hRef, "externalDocumentId", &hIdNode)
                    != NO_ERROR ||
                JsonNodeGetChild(hRef, "spdxDocument", &hDocNode)
                    != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: invalid externalDocumentRef in %s\n"
                        "       Both 'externalDocumentId' and "
                        "'spdxDocument' are required.\n", pszFilePath);
                exit(EXIT_FAILURE);
            }
            if (ReadJsonString(hDocNode, achDocUri, sizeof(achDocUri))
                    != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: %s: 'spdxDocument' must be a string.\n",
                        pszFilePath);
                exit(EXIT_FAILURE);
            }
            if (strncmp(achDocUri, "http://", 7) == 0 ||
                strncmp(achDocUri, "https://", 8) == 0) {
                fprintf(stderr,
                        "ERROR: %s: remote URIs are not supported: %s\n"
                        "       Use a local file path instead.\n",
                        pszFilePath, achDocUri);
                exit(EXIT_FAILURE);
            }

            if (achDocUri[0] == '/' || achDocUri[0] == '\\' ||
                (isalpha((unsigned char)achDocUri[0]) &&
                 achDocUri[1] == ':')) {
                pszFullDocPath = strdup(achDocUri);
                if (!pszFullDocPath) {
                    fprintf(stderr, "ERROR: out of memory\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                ULONG ulDirSize = 0, ulJoinSize = 0;
                PSZ pszDir;

                if (PathGetDirName(pszFilePath, NULL, 0, &ulDirSize)
                        != NO_ERROR) {
                    fprintf(stderr,
                            "ERROR: %s: cannot obtain directory\n",
                            pszFilePath);
                    exit(EXIT_FAILURE);
                }
                pszDir = (PSZ)malloc(ulDirSize);
                if (!pszDir) {
                    fprintf(stderr, "ERROR: out of memory\n");
                    exit(EXIT_FAILURE);
                }
                if (PathGetDirName(pszFilePath, pszDir, ulDirSize, NULL)
                        != NO_ERROR) {
                    fprintf(stderr,
                            "ERROR: %s: cannot obtain directory\n",
                            pszFilePath);
                    free(pszDir);
                    exit(EXIT_FAILURE);
                }

                if (PathMakeJoin(pszDir, achDocUri, NULL, 0, &ulJoinSize)
                        != NO_ERROR || ulJoinSize == 0) {
                    fprintf(stderr,
                            "ERROR: %s: cannot resolve path: %s\n",
                            pszFilePath, achDocUri);
                    free(pszDir);
                    exit(EXIT_FAILURE);
                }
                pszFullDocPath = (PSZ)malloc(ulJoinSize);
                if (!pszFullDocPath) {
                    fprintf(stderr, "ERROR: out of memory\n");
                    free(pszDir);
                    exit(EXIT_FAILURE);
                }
                if (PathMakeJoin(pszDir, achDocUri, pszFullDocPath,
                                 ulJoinSize, NULL) != NO_ERROR) {
                    fprintf(stderr,
                            "ERROR: %s: cannot resolve path: %s\n",
                            pszFilePath, achDocUri);
                    free(pszDir);
                    free(pszFullDocPath);
                    exit(EXIT_FAILURE);
                }
                free(pszDir);
            }

            if (JsonNodeGetChild(hRef, "checksum", &hChkNode) == NO_ERROR)
                VerifyChecksum(pszFullDocPath, hChkNode);
            else {
                fprintf(stderr,
                        "ERROR: %s: missing checksum for external "
                        "document: %s\n"
                        "       Each externalDocumentRef must include a "
                        "checksum.\n", pszFilePath, achDocUri);
                free(pszFullDocPath);
                exit(EXIT_FAILURE);
            }

            pszDocText = read_file_to_heap(pszFullDocPath);
            if (!pszDocText) {
                fprintf(stderr,
                        "ERROR: cannot read external document: %s\n"
                        "       Check that the file exists and is "
                        "readable.\n", pszFullDocPath);
                free(pszFullDocPath);
                exit(EXIT_FAILURE);
            }
            if (JsonParse(pszDocText, &hExtDoc) != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: invalid JSON in external document: %s\n",
                        pszFullDocPath);
                free(pszDocText);
                free(pszFullDocPath);
                exit(EXIT_FAILURE);
            }
            free(pszDocText);

            if (JsonRoot(hExtDoc, &hExtRoot) != NO_ERROR) {
                fprintf(stderr,
                        "ERROR: cannot obtain root of: %s\n",
                        pszFullDocPath);
                JsonClose(hExtDoc);
                free(pszFullDocPath);
                exit(EXIT_FAILURE);
            }

            ProcessDocument(pszFullDocPath, hExtRoot, hDocMerged,
                            hMergedRoot, pList, pMap);
            JsonClose(hExtDoc);
            free(pszFullDocPath);
        }
    }

    if (JsonNodeGetChild(hRoot, "packages", &hPackages) == NO_ERROR) {
        rc = EnsureMergedArray(hDocMerged, hMergedRoot, "packages",
                               &hMergedArr);
        if (rc != NO_ERROR) exit(EXIT_FAILURE);
        rc = MergeSection(hDocMerged, hPackages, hMergedArr, pMap,
                          pszFilePath);
        if (rc != NO_ERROR) exit(EXIT_FAILURE);
    }

    if (JsonNodeGetChild(hRoot, "files", &hFiles) == NO_ERROR) {
        rc = EnsureMergedArray(hDocMerged, hMergedRoot, "files",
                               &hMergedArr);
        if (rc != NO_ERROR) exit(EXIT_FAILURE);
        rc = MergeSection(hDocMerged, hFiles, hMergedArr, pMap,
                          pszFilePath);
        if (rc != NO_ERROR) exit(EXIT_FAILURE);
    }

    if (JsonNodeGetChild(hRoot, "snippets", &hSnippets) == NO_ERROR) {
        ULONG ulSnipCount = 0;
        if (JsonNodeGetCount(hSnippets, &ulSnipCount) == NO_ERROR &&
            ulSnipCount > 0) {
            rc = EnsureMergedArray(hDocMerged, hMergedRoot, "snippets",
                                   &hMergedArr);
            if (rc != NO_ERROR) exit(EXIT_FAILURE);
            rc = MergeSection(hDocMerged, hSnippets, hMergedArr, pMap,
                              pszFilePath);
            if (rc != NO_ERROR) exit(EXIT_FAILURE);
        }
    }

    {
        rc = EnsureMergedArray(hDocMerged, hMergedRoot, "relationships",
                               &hMergedArr);
        if (rc != NO_ERROR) exit(EXIT_FAILURE);
        rc = MergeRelationships(hDocMerged, hRoot, hMergedArr, pMap);
        if (rc != NO_ERROR) exit(EXIT_FAILURE);
    }
}

/* ==================================================================
 * CLI
 * ================================================================== */

/**
 * @brief Print command line usage.
 */
static void PrintHelp(void) {
    printf("Usage: spdx-merge --input=<file> [--output=<file>] "
           "[options]\n"
           "\n"
           "Required:\n"
           "  --input=<file>             Root SPDX JSON document to "
           "merge\n"
           "  --spdx-db=<path>           SPDX database root "
           "(licenses.json,\n"
           "                             exceptions.json, details/, "
           "exceptions/)\n"
           "\n"
           "Optional:\n"
           "  --output=<file>            Write merged document to file "
           "(default: stdout)\n"
           "  --cache=<path>             SPDX database cache file\n"
           "  --help, -h                 Show this help\n"
           "\n"
           "The merge walks externalDocumentRefs recursively, verifies "
           "each\n"
           "checksum, and resolves SPDXID collisions by renaming.\n");
}

/**
 * @brief Entry point of the SPDX merge tool.
 *
 * @param[in] argc  Argument count.
 * @param[in] argv  Argument vector.
 *
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    PCSZ pszInputFile = NULL;
    PCSZ pszOutputFile = NULL;
    PCSZ pszSpdxDbRoot = NULL;
    PCSZ pszCacheFile = NULL;
    int i;
    PSZ pszRootText = NULL;
    HJSONDOC hSrcDoc = NULLHANDLE;
    HJSONNODE hSrcRoot = NULLHANDLE;
    HJSONDOC hDocMerged = NULLHANDLE;
    HJSONNODE hMergedRoot = NULLHANDLE;
    HJSONNODE hCreation = NULLHANDLE;
    SPDXPROCESSEDLIST processed;
    SPDXRENAMEMAP renameMap;
    time_t tNow;
    struct tm *ptm;
    CHAR achDate[32];
    APIRET rcDb;
    APIRET rc;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            PrintHelp();
            return 0;
        }
        else if (strncmp(argv[i], "--input=", 8) == 0)
            pszInputFile = argv[i] + 8;
        else if (strncmp(argv[i], "--output=", 9) == 0)
            pszOutputFile = argv[i] + 9;
        else if (strncmp(argv[i], "--spdx-db=", 10) == 0)
            pszSpdxDbRoot = argv[i] + 10;
        else if (strncmp(argv[i], "--cache=", 8) == 0)
            pszCacheFile = argv[i] + 8;
        else {
            fprintf(stderr,
                    "ERROR: unknown option: %s\n"
                    "       Run 'spdx-merge --help' for usage.\n",
                    argv[i]);
            return 1;
        }
    }

    if (!pszInputFile) {
        fprintf(stderr,
                "ERROR: --input=<file> is required.\n"
                "       Run 'spdx-merge --help' for usage.\n");
        return 1;
    }
    if (!pszSpdxDbRoot) {
        fprintf(stderr,
                "ERROR: --spdx-db=<path> is required.\n"
                "       Run 'spdx-merge --help' for usage.\n");
        return 1;
    }

    rcDb = SpdxOpenDatabase(pszSpdxDbRoot, pszCacheFile);
    if (rcDb & SPDXDB_ERROR_LICENSES) {
        fprintf(stderr,
                "ERROR: SPDX license database is unavailable "
                "(licenses.json not loaded).\n"
                "       Expected at <spdx-db>/licenses.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        SpdxCloseDatabase();
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_EXCEPTIONS) {
        fprintf(stderr,
                "ERROR: SPDX exceptions database is unavailable "
                "(exceptions.json not loaded).\n"
                "       Expected at <spdx-db>/exceptions.json.\n"
                "       Cannot validate SPDX identifiers. Aborting.\n");
        SpdxCloseDatabase();
        return 1;
    }
    if (rcDb & SPDXDB_ERROR_CACHE)
        fprintf(stderr,
                "WARNING: cache could not be written.\n"
                "         Next run will re-parse JSON indexes.\n");

    pszRootText = read_file_to_heap(pszInputFile);
    if (!pszRootText) {
        fprintf(stderr,
                "ERROR: cannot read input file: %s\n"
                "       Check that the file exists and is readable.\n",
                pszInputFile);
        SpdxCloseDatabase();
        return 1;
    }
    if (JsonParse(pszRootText, &hSrcDoc) != NO_ERROR) {
        fprintf(stderr,
                "ERROR: invalid JSON in input file: %s\n", pszInputFile);
        free(pszRootText);
        SpdxCloseDatabase();
        return 1;
    }
    free(pszRootText);

    if (JsonRoot(hSrcDoc, &hSrcRoot) != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot obtain root of input file.\n");
        JsonClose(hSrcDoc);
        SpdxCloseDatabase();
        return 1;
    }

    processed.papszPaths = (PSZ*)malloc(MAX_DOCS * sizeof(PSZ));
    processed.ulCount = 0;
    renameMap.papszOldIds = NULL;
    renameMap.papszNewIds = NULL;
    renameMap.ulCount = 0;

    rc = JsonNewDoc(&hDocMerged);
    if (rc != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hSrcDoc);
        SpdxCloseDatabase();
        return 1;
    }

    rc = JsonNewObject(hDocMerged, &hMergedRoot);
    if (rc != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        SpdxCloseDatabase();
        return 1;
    }

    tNow = time(NULL);
    ptm = gmtime(&tNow);
    if (ptm)
        strftime(achDate, sizeof(achDate), "%Y-%m-%dT%H:%M:%SZ", ptm);
    else
        achDate[0] = '\0';

    if (JsonNodeSetString(hDocMerged, hMergedRoot, "spdxVersion",
                          "SPDX-2.3") != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hMergedRoot, "SPDXID",
                          "SPDXRef-DOCUMENT") != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hMergedRoot, "name",
                          "Merged SPDX Document") != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hMergedRoot, "dataLicense",
                          "CC0-1.0") != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        SpdxCloseDatabase();
        return 1;
    }

    {
        CHAR achNs[256];
        sprintf(achNs, "https://osfree.org/spdxdocs/merged-%ld",
                (long)tNow);
        if (JsonNodeSetString(hDocMerged, hMergedRoot,
                              "documentNamespace", achNs) != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            JsonClose(hDocMerged);
            JsonClose(hSrcDoc);
            SpdxCloseDatabase();
            return 1;
        }
    }

    if (JsonNewObject(hDocMerged, &hCreation) != NO_ERROR ||
        JsonNodeSetString(hDocMerged, hCreation, "created",
                          achDate) != NO_ERROR) {
        fprintf(stderr, "ERROR: out of memory\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        SpdxCloseDatabase();
        return 1;
    }
    {
        HJSONNODE hCreators = NULLHANDLE;
        HJSONNODE hToolName = NULLHANDLE;
        if (JsonNewArray(hDocMerged, &hCreators) != NO_ERROR ||
            JsonNewString(hDocMerged, "Tool: osFree SPDX Merge Tool",
                          &hToolName) != NO_ERROR ||
            JsonArrayAppend(hCreators, hToolName) != NO_ERROR ||
            JsonObjectSet(hCreation, "creators", hCreators) != NO_ERROR ||
            JsonObjectSet(hMergedRoot, "creationInfo",
                          hCreation) != NO_ERROR) {
            fprintf(stderr, "ERROR: out of memory\n");
            JsonClose(hDocMerged);
            JsonClose(hSrcDoc);
            SpdxCloseDatabase();
            return 1;
        }
    }

    ProcessDocument(pszInputFile, hSrcRoot, hDocMerged, hMergedRoot,
                    &processed, &renameMap);

    if (pszOutputFile) {
        if (!freopen(pszOutputFile, "w", stdout)) {
            fprintf(stderr,
                    "ERROR: cannot open output file: %s\n"
                    "       Check directory permissions.\n",
                    pszOutputFile);
            JsonClose(hDocMerged);
            JsonClose(hSrcDoc);
            SpdxCloseDatabase();
            return 1;
        }
    }

    rc = JsonWriteFile(hMergedRoot, TRUE_, NULL);
    if (rc != NO_ERROR) {
        fprintf(stderr, "ERROR: cannot serialize merged document.\n");
        JsonClose(hDocMerged);
        JsonClose(hSrcDoc);
        SpdxCloseDatabase();
        return 1;
    }

    JsonClose(hDocMerged);
    JsonClose(hSrcDoc);
    FreeRenameMap(&renameMap);
    for (i = 0; i < (int)processed.ulCount; i++)
        free(processed.papszPaths[i]);
    free(processed.papszPaths);
    SpdxCloseDatabase();
    return 0;
}
