/* spdx_tag.c - SPDX tags and snippet parser (C89) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx_tag.h"

/**
 * @file spdx_tag.c
 * @brief Implementation of the SPDX tags and snippet parser.
 */

/* Raised from 4096 to avoid truncating long copyright expressions. */
#define MAX_LINE 16384

/**
 * @brief Skip a UTF-8 BOM at the start of the buffer.
 *
 * @param[in] pszPos  Buffer. Not NULL.
 *
 * @return Pointer past the BOM, or @p pszPos if absent.
 */
static PCSZ skip_bom(PCSZ pszPos) {
    if ((UCHAR)pszPos[0] == 0xEF &&
        (UCHAR)pszPos[1] == 0xBB &&
        (UCHAR)pszPos[2] == 0xBF) {
        return pszPos + 3;
    }
    return pszPos;
}

/**
 * @brief Skip leading whitespace and a comment marker.
 *
 * @param[in] pszPos  Start of line. Not NULL.
 *
 * @return Pointer past the comment prefix.
 */
static PCSZ skip_comment_prefix(PCSZ pszPos) {
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    if (pszPos[0] == '/' && pszPos[1] == '/') { pszPos += 2; }
    else if (pszPos[0] == '/' && pszPos[1] == '*') { pszPos += 2; }
    else if (pszPos[0] == '#') { pszPos++; }
    else if (pszPos[0] == ';') { pszPos++; }
    else if (pszPos[0] == '%') { pszPos++; }
    else if (pszPos[0] == '-' && pszPos[1] == '-') { pszPos += 2; }
    else if (pszPos[0] == '*') { pszPos++; }
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    return pszPos;
}

/**
 * @brief Check whether the meaningful part of a line starts with
 *        REUSE-IgnoreStart or REUSE-IgnoreEnd.
 *
 * @param[in] pszLine  Line. Not NULL.
 *
 * @return 1 for Start, 2 for End, 0 otherwise.
 */
static int line_ignore_marker(PCSZ pszLine) {
    PCSZ pszPos = skip_comment_prefix(pszLine);
    if (strncmp(pszPos, "REUSE-IgnoreStart", 17) == 0) {
        CHAR ch = pszPos[17];
        if (ch == '\0' || ch == ' ' || ch == '\t' ||
            ch == '\n' || ch == '\r')
            return 1;
    }
    if (strncmp(pszPos, "REUSE-IgnoreEnd", 15) == 0) {
        CHAR ch = pszPos[15];
        if (ch == '\0' || ch == ' ' || ch == '\t' ||
            ch == '\n' || ch == '\r')
            return 2;
    }
    return 0;
}

/**
 * @brief Check whether the meaningful part of a line starts with the
 *        given tag.
 *
 * @param[in] pszLine  Line. Not NULL.
 * @param[in] pszTag   Tag. Not NULL.
 *
 * @return 1 on match, 0 otherwise.
 */
static int line_starts_with_tag(PCSZ pszLine, PCSZ pszTag) {
    size_t tlen = strlen(pszTag);
    PCSZ pszPos = skip_comment_prefix(pszLine);
    if (strncmp(pszPos, pszTag, tlen) != 0) return 0;
    if (pszPos[tlen] == ':' && pszTag[tlen - 1] != ':') return 1;
    if (pszPos[tlen] == '\0' || pszPos[tlen] == ' ' ||
        pszPos[tlen] == '\t' || pszPos[tlen] == '\n' ||
        pszPos[tlen] == '\r')
        return 1;
    return 0;
}

/**
 * @brief Return a pointer to the value after the tag.
 *
 * @param[in] pszLine         Line. Not NULL.
 * @param[in] pszTagWithColon  Tag with trailing ':'. Not NULL.
 *
 * @return Pointer to the value, or NULL if the tag is not found.
 */
static PCSZ tag_value(PCSZ pszLine, PCSZ pszTagWithColon) {
    size_t tlen = strlen(pszTagWithColon);
    PCSZ pszPos = skip_comment_prefix(pszLine);
    if (strncmp(pszPos, pszTagWithColon, tlen) != 0) return NULL;
    pszPos += tlen;
    while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
    return pszPos;
}

/**
 * @brief Strip trailing whitespace, newlines and comment-closing
 *        markers.
 *
 * @param[in,out] pszStr  String to modify. Not NULL.
 */
static void strip_trailing_markers(PSZ pszStr) {
    size_t cbLen = strlen(pszStr);
    while (cbLen > 0) {
        if (cbLen >= 2 && pszStr[cbLen-2] == '*' && pszStr[cbLen-1] == '/') {
            cbLen -= 2;
        } else if (cbLen >= 2 && pszStr[cbLen-2] == '/' &&
                   pszStr[cbLen-1] == '/') {
            cbLen -= 2;
        } else if (pszStr[cbLen-1] == ' ' || pszStr[cbLen-1] == '\t' ||
                   pszStr[cbLen-1] == '\n' || pszStr[cbLen-1] == '\r') {
            cbLen -= 1;
        } else {
            break;
        }
    }
    pszStr[cbLen] = '\0';
}

/* ------------------------------------------------------------------ */
/* Snippet list                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Initialize a snippet list.
 *
 * @param[in] pList  List. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList is NULL.
 */
APIRET APIENTRY SpdxSnippetListInit(SPDXSNIPPETLIST *pList) {
    if (!pList) return ERROR_INVALID_PARAMETER;
    pList->pItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return NO_ERROR;
}

/**
 * @brief Release all memory owned by a snippet list.
 *
 * @param[in] pList  List. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR  Success. Also for NULL.
 */
APIRET APIENTRY SpdxSnippetListFree(SPDXSNIPPETLIST *pList) {
    ULONG ulIdx;
    if (!pList) return NO_ERROR;
    for (ulIdx = 0; ulIdx < pList->ulCount; ulIdx++) {
        free(pList->pItems[ulIdx].pszLicense);
        free(pList->pItems[ulIdx].pszCopyright);
    }
    free(pList->pItems);
    pList->pItems = NULL;
    pList->ulCount = 0;
    pList->ulCapacity = 0;
    return NO_ERROR;
}

/**
 * @brief Append a zero-filled entry to a snippet list.
 *
 * @param[in,out] pList    List. Not NULL.
 * @param[out]    ppEntry  Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pList or ppEntry is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET snippetlist_add(SPDXSNIPPETLIST *pList,
                              PSPDXSNIPPET *ppEntry) {
    PSPDXSNIPPET pNew;
    *ppEntry = NULL;
    if (pList->ulCount >= pList->ulCapacity) {
        ULONG ulNewCap = pList->ulCapacity ? pList->ulCapacity * 2 : 4;
        pNew = (PSPDXSNIPPET)realloc(pList->pItems,
            (size_t)ulNewCap * sizeof(SPDXSNIPPET));
        if (!pNew) return ERROR_NOT_ENOUGH_MEMORY;
        pList->pItems = pNew;
        pList->ulCapacity = ulNewCap;
    }
    *ppEntry = &pList->pItems[pList->ulCount++];
    memset(*ppEntry, 0, sizeof(**ppEntry));
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public functions                                                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Query whether a file contains an SPDX-License-Identifier
 *        tag.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pfHasTag     Receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename or pfHasTag is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
APIRET APIENTRY SpdxQueryFileHasTag(PCSZ pszFilename, PBOOL pfHasTag) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    BOOL fIgnore = FALSE_;
    BOOL fFirstLine = TRUE_;

    if (!pszFilename || !pfHasTag) return ERROR_INVALID_PARAMETER;
    *pfHasTag = FALSE_;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;
    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszWork = achLine;
        int nMarker;

        if (fFirstLine) {
            pszWork = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE_;
        }

        nMarker = line_ignore_marker(pszWork);
        if (nMarker == 1) { fIgnore = TRUE_; continue; }
        if (nMarker == 2) { fIgnore = FALSE_; continue; }
        if (fIgnore) continue;

        if (strstr(pszWork, "SPDX-License-Identifier:")) {
            fclose(fp);
            *pfHasTag = TRUE_;
            return NO_ERROR;
        }
    }
    fclose(fp);
    return NO_ERROR;
}

/**
 * @brief Query the value of the SPDX-License-Identifier tag.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     Tag not present.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 */
APIRET APIENTRY SpdxQueryFileLicense(PCSZ pszFilename, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    PCSZ pszNeedle = "SPDX-License-Identifier:";
    size_t cbNeedle = strlen(pszNeedle);
    PSZ pszPos;
    PSZ pszStart;
    size_t cbLen;
    BOOL fIgnore = FALSE_;
    BOOL fFirstLine = TRUE_;
    ULONG ulFound = 0;

    if (!pszFilename) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszWork = achLine;
        int nMarker;

        if (fFirstLine) {
            pszWork = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE_;
        }

        nMarker = line_ignore_marker(pszWork);
        if (nMarker == 1) { fIgnore = TRUE_; continue; }
        if (nMarker == 2) { fIgnore = FALSE_; continue; }
        if (fIgnore) continue;

        pszPos = strstr(pszWork, pszNeedle);
        if (pszPos) {
            pszStart = (PSZ)skip_comment_prefix(pszPos + cbNeedle);
            strip_trailing_markers(pszStart);
            cbLen = strlen(pszStart);
            if (cbLen > 0) {
                if (pszBuf == NULL && ulSize == 0) {
                    if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
                    fclose(fp);
                    return NO_ERROR;
                }
                if (ulSize < (ULONG)cbLen + 1) {
                    if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
                    fclose(fp);
                    return ERROR_BUFFER_OVERFLOW;
                }
                memcpy(pszBuf, pszStart, cbLen);
                pszBuf[cbLen] = '\0';
                if (pulUsed) *pulUsed = (ULONG)cbLen;
                fclose(fp);
                return NO_ERROR;
            }
        }
        ulFound = ulFound;
    }
    fclose(fp);
    return ERROR_FILE_NOT_FOUND;
}

/**
 * @brief Query the concatenated SPDX-FileCopyrightText values.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pszBuf       Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize       Size of pszBuf in bytes.
 * @param[out] pulUsed      Optional. May be NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszFilename is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No copyright tag present.
 * @retval ERROR_BUFFER_OVERFLOW    pszBuf too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY SpdxQueryFileCopyright(PCSZ pszFilename, PSZ pszBuf,
                                       ULONG ulSize, PULONG pulUsed) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    PSZ pszResult = NULL;
    size_t cbResultLen = 0;
    BOOL fIgnore = FALSE_;
    BOOL fFirstLine = TRUE_;
    APIRET rc;

    if (!pszFilename) return ERROR_INVALID_PARAMETER;
    if (pszBuf != NULL && ulSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszBuf == NULL && ulSize != 0) return ERROR_INVALID_PARAMETER;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszPos;
        PSZ pszStart;
        size_t cbLen;
        int nMarker;

        if (fFirstLine) {
            pszPos = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE_;
        } else {
            pszPos = achLine;
        }

        nMarker = line_ignore_marker(pszPos);
        if (nMarker == 1) { fIgnore = TRUE_; continue; }
        if (nMarker == 2) { fIgnore = FALSE_; continue; }
        if (fIgnore) continue;

        pszPos = (PSZ)skip_comment_prefix(pszPos);

        if (strncmp(pszPos, "SPDX-FileCopyrightText:", 23) == 0) {
            pszPos += 23;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
            pszStart = pszPos;
        } else if (strncmp(pszPos, "Copyright", 9) == 0 &&
                   (pszPos[9] == ' ' || pszPos[9] == '\t')) {
            pszPos += 9;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
            pszStart = pszPos;
        } else if ((UCHAR)*pszPos == 0xC2 &&
                   (UCHAR)*(pszPos+1) == 0xA9) {
            pszPos += 2;
            while (*pszPos == ' ' || *pszPos == '\t') pszPos++;
            pszStart = pszPos;
        } else {
            continue;
        }

        strip_trailing_markers(pszStart);
        cbLen = strlen(pszStart);
        if (cbLen > 0) {
            PSZ pszNew;
            size_t cbNewSize = (pszResult ? cbResultLen + 1 + cbLen + 1
                                          : cbLen + 1);
            pszNew = (PSZ)realloc(pszResult, cbNewSize);
            if (!pszNew) {
                free(pszResult);
                fclose(fp);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            pszResult = pszNew;
            if (cbResultLen > 0) {
                pszResult[cbResultLen++] = '\n';
            }
            memcpy(pszResult + cbResultLen, pszStart, cbLen);
            cbResultLen += cbLen;
            pszResult[cbResultLen] = '\0';
        }
    }
    fclose(fp);

    if (!pszResult) return ERROR_FILE_NOT_FOUND;

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbResultLen + 1;
        free(pszResult);
        return NO_ERROR;
    }
    if (ulSize < (ULONG)cbResultLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbResultLen + 1;
        free(pszResult);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszResult, cbResultLen + 1);
    if (pulUsed) *pulUsed = (ULONG)cbResultLen;
    free(pszResult);
    rc = NO_ERROR;
    return rc;
}

/**
 * @brief Append a line to an accumulator with a '\n' separator.
 *
 * @param[in] pszAcc      Accumulator, or NULL.
 * @param[in] pcbAccLen   Pointer to the used length. Not NULL.
 * @param[in] pszLine     Line to append. Not NULL.
 *
 * @return New accumulator, or NULL on OOM.
 */
static PSZ append_line(PSZ pszAcc, size_t *pcbAccLen, PCSZ pszLine) {
    size_t cbLineLen = strlen(pszLine);
    PSZ pszNew;
    size_t cbNewSize;

    if (cbLineLen == 0) return pszAcc;

    cbNewSize = (*pcbAccLen ? *pcbAccLen + 1 : 0) + cbLineLen + 1;
    pszNew = (PSZ)realloc(pszAcc, cbNewSize);
    if (!pszNew) return pszAcc;

    if (*pcbAccLen > 0) {
        pszNew[(*pcbAccLen)++] = '\n';
    }
    memcpy(pszNew + *pcbAccLen, pszLine, cbLineLen);
    *pcbAccLen += cbLineLen;
    pszNew[*pcbAccLen] = '\0';
    return pszNew;
}

/**
 * @brief Extract all SPDX snippets from a file.
 *
 * @param[in]  pszFilename  Path to the file. Not NULL.
 * @param[out] pOut         List receiver. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_INVALID_PARAMETER  pszFilename or pOut is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Nested or unmatched snippet
 *                                  delimiters.
 */
APIRET APIENTRY SpdxQueryFileSnippets(PCSZ pszFilename,
                                      SPDXSNIPPETLIST *pOut) {
    FILE *fp;
    CHAR achLine[MAX_LINE];
    ULONG ulLineNo = 0;
    BOOL fFirstLine = TRUE_;
    BOOL fInSnippet = FALSE_;
    ULONG ulSnippetStartLine = 0;
    PSZ pszSnippetLicense = NULL;
    PSZ pszSnippetCopyright = NULL;
    size_t cbSnippetCopyrightLen = 0;
    BOOL fIgnore = FALSE_;
    APIRET rc;

    if (!pszFilename || !pOut) return ERROR_INVALID_PARAMETER;

    rc = SpdxSnippetListInit(pOut);
    if (rc != NO_ERROR) return rc;

    fp = fopen(pszFilename, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp)) {
        PSZ pszWork = achLine;
        int nMarker;

        ulLineNo++;

        if (fFirstLine) {
            pszWork = (PSZ)skip_bom(achLine);
            fFirstLine = FALSE_;
        }

        nMarker = line_ignore_marker(pszWork);
        if (nMarker == 1) { fIgnore = TRUE_; continue; }
        if (nMarker == 2) { fIgnore = FALSE_; continue; }
        if (fIgnore) continue;

        if (line_starts_with_tag(pszWork, "SPDX-SnippetBegin")) {
            if (fInSnippet) {
                free(pszSnippetLicense);
                free(pszSnippetCopyright);
                SpdxSnippetListFree(pOut);
                fclose(fp);
                return SPDX_TAG_ERROR_SYNTAX;
            }
            fInSnippet = TRUE_;
            ulSnippetStartLine = ulLineNo;
            free(pszSnippetLicense); pszSnippetLicense = NULL;
            free(pszSnippetCopyright); pszSnippetCopyright = NULL;
            cbSnippetCopyrightLen = 0;
            continue;
        }

        if (line_starts_with_tag(pszWork, "SPDX-SnippetEnd")) {
            PSPDXSNIPPET pS;
            if (!fInSnippet) {
                SpdxSnippetListFree(pOut);
                fclose(fp);
                return SPDX_TAG_ERROR_SYNTAX;
            }
            rc = snippetlist_add(pOut, &pS);
            if (rc != NO_ERROR) {
                free(pszSnippetLicense);
                free(pszSnippetCopyright);
                SpdxSnippetListFree(pOut);
                fclose(fp);
                return rc;
            }
            pS->ulLineStart = ulSnippetStartLine;
            pS->ulLineEnd = ulLineNo;
            pS->pszLicense = pszSnippetLicense;
            pS->pszCopyright = pszSnippetCopyright;
            pszSnippetLicense = NULL;
            pszSnippetCopyright = NULL;
            cbSnippetCopyrightLen = 0;
            fInSnippet = FALSE_;
            continue;
        }

        if (!fInSnippet) continue;

        {
            PCSZ pszVal;

            pszVal = tag_value(pszWork, "SPDX-SnippetCopyrightText:");
            if (pszVal) {
                CHAR achTmp[MAX_LINE];
                size_t cbL = strlen(pszVal);
                if (cbL >= sizeof(achTmp)) cbL = sizeof(achTmp) - 1;
                memcpy(achTmp, pszVal, cbL);
                achTmp[cbL] = '\0';
                strip_trailing_markers(achTmp);
                pszSnippetCopyright = append_line(pszSnippetCopyright,
                                                  &cbSnippetCopyrightLen,
                                                  achTmp);
                continue;
            }

            pszVal = tag_value(pszWork, "SPDX-License-Identifier:");
            if (pszVal) {
                CHAR achTmp[MAX_LINE];
                size_t cbL = strlen(pszVal);
                if (cbL >= sizeof(achTmp)) cbL = sizeof(achTmp) - 1;
                memcpy(achTmp, pszVal, cbL);
                achTmp[cbL] = '\0';
                strip_trailing_markers(achTmp);
                free(pszSnippetLicense);
                pszSnippetLicense = (PSZ)malloc(strlen(achTmp) + 1);
                if (pszSnippetLicense) strcpy(pszSnippetLicense, achTmp);
                continue;
            }
        }
    }

    fclose(fp);

    if (fInSnippet) {
        free(pszSnippetLicense);
        free(pszSnippetCopyright);
        SpdxSnippetListFree(pOut);
        return SPDX_TAG_ERROR_SYNTAX;
    }

    return NO_ERROR;
}
