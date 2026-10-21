/*!
 * @file reuse_tags.c
 *
 * @brief Implementation of the REUSE 3.3 file tag reader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reuse.h"
#include "spdx_tag.h"

/*!
 * @def REUSE_TAGS_MAX_LINE
 * @brief Maximum line length accepted by the file scanner.
 */
#define REUSE_TAGS_MAX_LINE 16384

/*!
 * @def REUSETAGSREPORT_MAGIC
 * @brief Magic value identifying a valid report handle.
 */
#define REUSETAGSREPORT_MAGIC 0x52544746UL  /* "RTGF" */

/* ==================================================================
 * Report implementation
 * ================================================================== */

/*!
 * @struct _REUSETAGSREPORT
 * @brief Control block of an open report.
 */
typedef struct _REUSETAGSREPORT {
    unsigned long ulMagic;      /*!< REUSETAGSREPORT_MAGIC. */
    REUSEERR     *paRecords;    /*!< Backing array.          */
    ULONG         ulCount;      /*!< Used entries.           */
    ULONG         ulCapacity;   /*!< Allocated entries.      */
} REUSETAGSREPORT, *PREUSETAGSREPORT;

/*!
 * @brief Translate a public report handle into the internal pointer.
 *
 * @param[in] hReport  Handle. May be NULLHANDLE.
 *
 * @return PREUSETAGSREPORT
 *
 * @retval NULL  Handle is NULLHANDLE or magic does not match.
 * @return       Internal pointer.
 */
static PREUSETAGSREPORT as_tags_report(HREUSETAGSREPORT hReport) {
    PREUSETAGSREPORT pCtl;
    if (hReport == NULLHANDLE) return NULL;
    pCtl = (PREUSETAGSREPORT)hReport;
    if (pCtl->ulMagic != REUSETAGSREPORT_MAGIC) return NULL;
    return pCtl;
}

/*!
 * @brief Append one record to a report.
 *
 * A NULLHANDLE report is accepted; the record is dropped.
 *
 * @param[in] hReport    Report handle, or NULLHANDLE.
 * @param[in] ulCode     REUSE_TAGS_*.
 * @param[in] ulSeverity REUSE_SEV_*.
 * @param[in] pszFile    Offending path, or NULL.
 * @param[in] ulLine     Source line, 0 if not applicable.
 * @param[in] pszDetail  Short human-readable detail, or NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success, or hReport is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET tags_report_append(HREUSETAGSREPORT hReport,
                                 ULONG ulCode, ULONG ulSeverity,
                                 PCSZ pszFile, ULONG ulLine,
                                 PCSZ pszDetail)
{
    PREUSETAGSREPORT pCtl;
    REUSEERR *pErr;

    if (hReport == NULLHANDLE) return NO_ERROR;
    pCtl = as_tags_report(hReport);
    if (pCtl == NULL) return ERROR_INVALID_HANDLE;

    if (pCtl->ulCount >= pCtl->ulCapacity) {
        ULONG ulNewCap = pCtl->ulCapacity ? pCtl->ulCapacity * 2 : 8;
        REUSEERR *paNew = (REUSEERR *)realloc(pCtl->paRecords,
                                (size_t)ulNewCap * sizeof(REUSEERR));
        if (paNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;
        pCtl->paRecords = paNew;
        pCtl->ulCapacity = ulNewCap;
    }

    pErr = &pCtl->paRecords[pCtl->ulCount++];
    memset(pErr, 0, sizeof(*pErr));
    pErr->ulCode = ulCode;
    pErr->ulSeverity = ulSeverity;
    pErr->ulLine = ulLine;
    if (pszFile != NULL)
        strncpy(pErr->achFile, pszFile, sizeof(pErr->achFile) - 1);
    if (pszDetail != NULL)
        strncpy(pErr->achDetail, pszDetail, sizeof(pErr->achDetail) - 1);
    return NO_ERROR;
}

/*!
 * @brief Create an empty report.
 *
 * @param[out] phReport  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseCreateTagsReport(PHREUSETAGSREPORT phReport) {
    PREUSETAGSREPORT pCtl;

    if (phReport == NULL) return ERROR_INVALID_PARAMETER;
    *phReport = NULLHANDLE;

    pCtl = (PREUSETAGSREPORT)calloc(1, sizeof(*pCtl));
    if (pCtl == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    pCtl->ulMagic = REUSETAGSREPORT_MAGIC;
    *phReport = (HREUSETAGSREPORT)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Close a report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
APIRET APIENTRY ReuseCloseTagsReport(HREUSETAGSREPORT hReport) {
    PREUSETAGSREPORT pCtl;

    if (hReport == NULLHANDLE) return NO_ERROR;
    pCtl = as_tags_report(hReport);
    if (pCtl == NULL) return ERROR_INVALID_HANDLE;
    free(pCtl->paRecords);
    pCtl->ulMagic = 0;
    free(pCtl);
    return NO_ERROR;
}

/*!
 * @brief Number of records in a report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY ReuseQueryTagsReportCount(HREUSETAGSREPORT hReport,
                                          PULONG pulCount)
{
    PREUSETAGSREPORT pCtl = as_tags_report(hReport);
    if (pCtl == NULL || pulCount == NULL) return ERROR_INVALID_PARAMETER;
    *pulCount = pCtl->ulCount;
    return NO_ERROR;
}

/*!
 * @brief Retrieve one record by index.
 *
 * @param[in]  hReport  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pErr is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 */
APIRET APIENTRY ReuseQueryTagsReport(HREUSETAGSREPORT hReport,
                                     ULONG ulIndex,
                                     PREUSEERR pErr)
{
    PREUSETAGSREPORT pCtl = as_tags_report(hReport);
    if (pCtl == NULL || pErr == NULL) return ERROR_INVALID_PARAMETER;
    if (ulIndex >= pCtl->ulCount) return ERROR_NO_MORE_ITEMS;
    *pErr = pCtl->paRecords[ulIndex];
    return NO_ERROR;
}

/* ==================================================================
 * Small helpers
 * ================================================================== */

/*!
 * @brief Strip a trailing LF, CR, space or tab from a string.
 *
 * @param[in,out] pszLine  String to trim in place. Not NULL.
 */
static void trim_trailing(PSZ pszLine) {
    size_t cbLen = strlen(pszLine);
    while (cbLen > 0 &&
           (pszLine[cbLen-1] == '\n' ||
            pszLine[cbLen-1] == '\r' ||
            pszLine[cbLen-1] == ' ' ||
            pszLine[cbLen-1] == '\t'))
        pszLine[--cbLen] = '\0';
}

/*!
 * @brief Strip a UTF-8 BOM at the start of a line, in place.
 *
 * @param[in,out] pszLine  String to modify. Not NULL.
 */
static void strip_bom_in_place(PSZ pszLine) {
    if ((UCHAR)pszLine[0] == 0xEF &&
        (UCHAR)pszLine[1] == 0xBB &&
        (UCHAR)pszLine[2] == 0xBF) {
        size_t cbLen = strlen(pszLine + 3);
        memmove(pszLine, pszLine + 3, cbLen + 1);
    }
}

/*!
 * @brief Ensure that a dynamically grown string has room for at
 *        least @p cbNeeded bytes (including the NUL).
 *
 * @param[in,out] ppszAcc    Pointer to the accumulator. Not NULL.
 * @param[in,out] pcbCap     Pointer to the capacity. Not NULL.
 * @param[in]     cbNeeded   Required capacity in bytes.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET accum_grow(PSZ *ppszAcc, size_t *pcbCap, size_t cbNeeded) {
    size_t cbNewCap;
    PSZ pszNew;

    if (cbNeeded <= *pcbCap) return NO_ERROR;
    cbNewCap = *pcbCap ? *pcbCap * 2 : 256;
    while (cbNewCap < cbNeeded) cbNewCap *= 2;
    pszNew = (PSZ)realloc(*ppszAcc, cbNewCap);
    if (pszNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;
    *ppszAcc = pszNew;
    *pcbCap = cbNewCap;
    return NO_ERROR;
}

/*!
 * @brief Append a value to a dynamically grown string.
 *
 * If the accumulator is already non-empty, @p pszSep is inserted
 * between the existing content and @p pszValue.
 *
 * @param[in,out] ppszAcc    Pointer to the accumulator. Not NULL.
 * @param[in,out] pcbLen     Pointer to the used length. Not NULL.
 * @param[in,out] pcbCap     Pointer to the capacity. Not NULL.
 * @param[in]     pszSep     Separator. Not NULL.
 * @param[in]     cbSep      Separator length.
 * @param[in]     pszValue   Value to append. Not NULL.
 * @param[in]     cbValue    Value length.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET accum_add(PSZ *ppszAcc, size_t *pcbLen, size_t *pcbCap,
                        PCSZ pszSep, size_t cbSep,
                        PCSZ pszValue, size_t cbValue)
{
    size_t cbActualSep = (*pcbLen > 0) ? cbSep : 0;
    size_t cbNeeded = *pcbLen + cbActualSep + cbValue + 1;
    APIRET rc = accum_grow(ppszAcc, pcbCap, cbNeeded);

    if (rc != NO_ERROR) return rc;
    if (cbActualSep > 0) {
        memcpy(*ppszAcc + *pcbLen, pszSep, cbActualSep);
        *pcbLen += cbActualSep;
    }
    if (cbValue > 0) {
        memcpy(*ppszAcc + *pcbLen, pszValue, cbValue);
        *pcbLen += cbValue;
    }
    (*ppszAcc)[*pcbLen] = '\0';
    return NO_ERROR;
}

/*!
 * @brief Query whether a line is a REUSE-IgnoreStart or REUSE-IgnoreEnd
 *        marker.
 *
 * @param[in] pszBody  Meaningful part of the line. Not NULL.
 *
 * @return int
 *
 * @retval 1  REUSE-IgnoreStart.
 * @retval 2  REUSE-IgnoreEnd.
 * @retval 0  Neither marker.
 */
static int ignore_marker(PCSZ pszBody) {
    if (strncmp(pszBody, "REUSE-IgnoreStart", 17) == 0) {
        CHAR ch = pszBody[17];
        if (ch == '\0' || ch == ' ' || ch == '\t')
            return 1;
    }
    if (strncmp(pszBody, "REUSE-IgnoreEnd", 15) == 0) {
        CHAR ch = pszBody[15];
        if (ch == '\0' || ch == ' ' || ch == '\t')
            return 2;
    }
    return 0;
}

/*!
 * @brief Test whether a body carries a REUSE-specific copyright
 *        prefix (Copyright or ©).
 *
 * On match, @p ppszValue points past the prefix and any following
 * spaces/tabs. The returned pointer aliases @p pszBody.
 *
 * @param[in]  pszBody   Meaningful part of the line. Not NULL.
 * @param[out] ppszValue Receiver. Not NULL.
 *
 * @return BOOL
 *
 * @retval TRUE   A REUSE copyright prefix was recognized.
 * @retval FALSE  Otherwise.
 */
static BOOL match_reuse_copyright(PCSZ pszBody, PCSZ *ppszValue) {
    PCSZ pszValue;

    if (strncmp(pszBody, "Copyright", 9) == 0 &&
        (pszBody[9] == '\0' || pszBody[9] == ' ' || pszBody[9] == '\t')) {
        pszValue = pszBody + 9;
    } else if ((UCHAR)pszBody[0] == 0xC2 &&
               (UCHAR)pszBody[1] == 0xA9) {
        pszValue = pszBody + 2;
    } else {
        return FALSE;
    }
    while (*pszValue == ' ' || *pszValue == '\t') pszValue++;
    if (*pszValue == '\0') return FALSE;
    *ppszValue = pszValue;
    return TRUE;
}

/*!
 * @brief Append a zero-filled entry to a snippet list.
 *
 * Local copy of the helper used by spdx_tag.c so that
 * reuse_tags.c does not depend on a non-public symbol.
 *
 * @param[in,out] pList    List. Not NULL.
 * @param[out]    ppEntry  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Argument is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET snippetlist_append(SPDXSNIPPETLIST *pList,
                                 PSPDXSNIPPET *ppEntry)
{
    PSPDXSNIPPET pNew;
    *ppEntry = NULL;
    if (pList->ulCount >= pList->ulCapacity) {
        ULONG ulNewCap = pList->ulCapacity ? pList->ulCapacity * 2 : 4;
        pNew = (PSPDXSNIPPET)realloc(pList->pItems,
            (size_t)ulNewCap * sizeof(SPDXSNIPPET));
        if (pNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;
        pList->pItems = pNew;
        pList->ulCapacity = ulNewCap;
    }
    *ppEntry = &pList->pItems[pList->ulCount++];
    memset(*ppEntry, 0, sizeof(**ppEntry));
    return NO_ERROR;
}

/* ==================================================================
 * ReuseQueryFileTags
 * ================================================================== */

/*!
 * @brief Read the file-level license and copyright of a file.
 *
 * @param[in]  pszPath       Path to the file. Not NULL.
 * @param[in]  hStyle        Comment style, or NULLHANDLE for a
 *                           sidecar file (raw tag-value, no comment
 *                           marker stripping).
 * @param[out] pszLicense    License buffer. Not NULL unless size-query.
 * @param[in]  ulLicSize     Size of pszLicense in bytes.
 * @param[out] pulLicUsed    Optional. Receives the used size.
 * @param[out] pszCopyright  Copyright buffer. Not NULL unless size-query.
 * @param[in]  ulCopSize     Size of pszCopyright in bytes.
 * @param[out] pulCopUsed    Optional. Receives the used size.
 * @param[in]  hReport       Warning report, or NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 At least one tag was found.
 * @retval ERROR_INVALID_PARAMETER  Bad argument.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_FILE_NOT_FOUND     No licensing information found.
 * @retval ERROR_BUFFER_OVERFLOW    A buffer is too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Unmatched snippet delimiters.
 */
APIRET APIENTRY ReuseQueryFileTags(PCSZ pszPath,
                                   HREUSECOMMENTSTYLE hStyle,
                                   PSZ pszLicense, ULONG ulLicSize,
                                   PULONG pulLicUsed,
                                   PSZ pszCopyright, ULONG ulCopSize,
                                   PULONG pulCopUsed,
                                   HREUSETAGSREPORT hReport)
{
    FILE *fp;
    CHAR achLine[REUSE_TAGS_MAX_LINE];
    CHAR achValue[REUSE_TAGS_MAX_LINE];
    ULONG ulLineNo = 0;
    BOOL fFirstLine = TRUE;
    BOOL fIgnore = FALSE;
    int nDepth = 0;
    PSZ pszLicAcc = NULL;
    size_t cbLicLen = 0, cbLicCap = 0;
    PSZ pszCopAcc = NULL;
    size_t cbCopLen = 0, cbCopCap = 0;
    BOOL fAnyFound = FALSE;
    BOOL fSizeQuery;
    APIRET rc = NO_ERROR;
    ULONG ulProbe;

    if (pszPath == NULL) return ERROR_INVALID_PARAMETER;
    if (pszLicense != NULL && ulLicSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszLicense == NULL && ulLicSize != 0) return ERROR_INVALID_PARAMETER;
    if (pszCopyright != NULL && ulCopSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszCopyright == NULL && ulCopSize != 0) return ERROR_INVALID_PARAMETER;

    if (hReport != NULLHANDLE) {
        if (ReuseQueryTagsReportCount(hReport, &ulProbe) != NO_ERROR)
            return ERROR_INVALID_HANDLE;
    }

    fSizeQuery = (pszLicense == NULL && pszCopyright == NULL) ? TRUE : FALSE;
    if (!fSizeQuery && (pszLicense == NULL || pszCopyright == NULL))
        return ERROR_INVALID_PARAMETER;

    fp = fopen(pszPath, "r");
    if (fp == NULL) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp) != NULL) {
        PCSZ pszBody;
        SPDXTAG tag;
        BOOL fDep, fInFile;
        ULONG ulOffset = 0, ulUsed = 0;
        APIRET rcLocal;
        int nMarker;

        ulLineNo++;
        if (fFirstLine) {
            strip_bom_in_place(achLine);
            fFirstLine = FALSE;
        }
        trim_trailing(achLine);

        /* P1: sidecar mode when hStyle is NULLHANDLE. */
        if (hStyle == NULLHANDLE) {
            ulOffset = 0;
        } else {
            rcLocal = ReuseQueryCommentBody(hStyle, achLine, &ulOffset);
            if (rcLocal == ERROR_FILE_NOT_FOUND) continue;
            if (rcLocal != NO_ERROR) { rc = rcLocal; goto done; }
        }
        pszBody = achLine + ulOffset;

        nMarker = ignore_marker(pszBody);
        if (nMarker == 1) { fIgnore = TRUE;  continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        rcLocal = SpdxQueryTagLine(pszBody, &tag, &fDep, &fInFile,
                                   achValue, sizeof(achValue), &ulUsed);
        if (rcLocal == ERROR_BUFFER_OVERFLOW) {
            rc = tags_report_append(hReport, REUSE_TAGS_LONG_LINE,
                                    REUSE_SEV_WARNING, pszPath, ulLineNo,
                                    "tag value truncated, skipped");
            if (rc != NO_ERROR) goto done;
            continue;
        }
        if (rcLocal != NO_ERROR) { rc = rcLocal; goto done; }

        if (fDep) {
            SPDXTAGINFO info;
            if (SpdxQueryTagInfo(tag, &info) == NO_ERROR) {
                rc = tags_report_append(hReport, REUSE_TAGS_DEPRECATED,
                                        REUSE_SEV_WARNING, pszPath, ulLineNo,
                                        info.pszTag);
                if (rc != NO_ERROR) goto done;
            }
            continue;
        }

        switch (tag) {
        case SPDXTAG_SNIPPET_BEGIN:
            nDepth++;
            if (nDepth > 1) { rc = SPDX_TAG_ERROR_SYNTAX; goto done; }
            continue;

        case SPDXTAG_SNIPPET_END:
            nDepth--;
            if (nDepth < 0) { rc = SPDX_TAG_ERROR_SYNTAX; goto done; }
            continue;

        case SPDXTAG_LICENSE_ID:
            if (nDepth > 0) continue;
            if (!fInFile || ulUsed == 0) continue;
            rc = accum_add(&pszLicAcc, &cbLicLen, &cbLicCap,
                           " AND ", 5, achValue, ulUsed);
            if (rc != NO_ERROR) goto done;
            fAnyFound = TRUE;
            break;

        case SPDXTAG_COPYRIGHT:
            if (nDepth > 0) continue;
            if (!fInFile) {
                rc = tags_report_append(hReport, REUSE_TAGS_NON_ANNEX_FORM,
                                        REUSE_SEV_WARNING, pszPath, ulLineNo,
                                        "FileCopyrightText:");
                if (rc != NO_ERROR) goto done;
                continue;
            }
            if (ulUsed == 0) continue;
            rc = accum_add(&pszCopAcc, &cbCopLen, &cbCopCap,
                           "\n", 1, achValue, ulUsed);
            if (rc != NO_ERROR) goto done;
            fAnyFound = TRUE;
            break;

        case SPDXTAG_OTHER:
            if (nDepth > 0) continue;
            {
                PCSZ pszValue;
                if (match_reuse_copyright(pszBody, &pszValue)) {
                    rc = accum_add(&pszCopAcc, &cbCopLen, &cbCopCap,
                                   "\n", 1, pszValue, strlen(pszValue));
                    if (rc != NO_ERROR) goto done;
                    fAnyFound = TRUE;
                }
            }
            break;

        default:
            continue;
        }
    }

    if (nDepth != 0) rc = SPDX_TAG_ERROR_SYNTAX;

done:
    fclose(fp);

    if (rc != NO_ERROR) {
        free(pszLicAcc);
        free(pszCopAcc);
        return rc;
    }

    if (fSizeQuery) {
        if (pulLicUsed) *pulLicUsed = (ULONG)(pszLicAcc ? cbLicLen + 1 : 1);
        if (pulCopUsed) *pulCopUsed = (ULONG)(pszCopAcc ? cbCopLen + 1 : 1);
    } else {
        ULONG ulLicReq = (ULONG)(pszLicAcc ? cbLicLen + 1 : 1);
        ULONG ulCopReq = (ULONG)(pszCopAcc ? cbCopLen + 1 : 1);
        if (ulLicSize < ulLicReq || ulCopSize < ulCopReq) {
            if (pulLicUsed) *pulLicUsed = ulLicReq;
            if (pulCopUsed) *pulCopUsed = ulCopReq;
            rc = ERROR_BUFFER_OVERFLOW;
        } else {
            if (pszLicAcc) {
                memcpy(pszLicense, pszLicAcc, cbLicLen);
                pszLicense[cbLicLen] = '\0';
                if (pulLicUsed) *pulLicUsed = (ULONG)cbLicLen;
            } else {
                pszLicense[0] = '\0';
                if (pulLicUsed) *pulLicUsed = 0;
            }
            if (pszCopAcc) {
                memcpy(pszCopyright, pszCopAcc, cbCopLen);
                pszCopyright[cbCopLen] = '\0';
                if (pulCopUsed) *pulCopUsed = (ULONG)cbCopLen;
            } else {
                pszCopyright[0] = '\0';
                if (pulCopUsed) *pulCopUsed = 0;
            }
        }
    }

    free(pszLicAcc);
    free(pszCopAcc);

    if (rc == NO_ERROR && !fAnyFound) rc = ERROR_FILE_NOT_FOUND;
    return rc;
}

/* ==================================================================
 * ReuseQueryFileHasTag
 * ================================================================== */

/*!
 * @brief Query whether a file has any SPDX-License-Identifier tag
 *        outside a REUSE-IgnoreStart/End block.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style, or NULLHANDLE for a sidecar.
 * @param[out] pfHas    Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Check completed.
 * @retval ERROR_INVALID_PARAMETER  pszPath or pfHas is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle is not a style handle.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 */
APIRET APIENTRY ReuseQueryFileHasTag(PCSZ pszPath,
                                     HREUSECOMMENTSTYLE hStyle,
                                     PBOOL pfHas)
{
    FILE *fp;
    CHAR achLine[REUSE_TAGS_MAX_LINE];
    CHAR achValue[REUSE_TAGS_MAX_LINE];
    BOOL fFirstLine = TRUE;
    BOOL fIgnore = FALSE;
    APIRET rc = NO_ERROR;

    if (pszPath == NULL || pfHas == NULL) return ERROR_INVALID_PARAMETER;
    *pfHas = FALSE;

    fp = fopen(pszPath, "r");
    if (fp == NULL) return ERROR_OPEN_FAILED;

    while (fgets(achLine, sizeof(achLine), fp) != NULL) {
        PCSZ pszBody;
        SPDXTAG tag;
        BOOL fDep, fInFile;
        ULONG ulOffset = 0, ulUsed = 0;
        APIRET rcLocal;
        int nMarker;

        if (fFirstLine) {
            strip_bom_in_place(achLine);
            fFirstLine = FALSE;
        }
        trim_trailing(achLine);

        if (hStyle == NULLHANDLE) {
            ulOffset = 0;
        } else {
            rcLocal = ReuseQueryCommentBody(hStyle, achLine, &ulOffset);
            if (rcLocal == ERROR_FILE_NOT_FOUND) continue;
            if (rcLocal != NO_ERROR) { rc = rcLocal; break; }
        }
        pszBody = achLine + ulOffset;

        nMarker = ignore_marker(pszBody);
        if (nMarker == 1) { fIgnore = TRUE;  continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        rcLocal = SpdxQueryTagLine(pszBody, &tag, &fDep, &fInFile,
                                   achValue, sizeof(achValue), &ulUsed);
        if (rcLocal != NO_ERROR) continue;

        if (tag == SPDXTAG_LICENSE_ID && fInFile) {
            fclose(fp);
            *pfHas = TRUE;
            return NO_ERROR;
        }
    }
    fclose(fp);
    return rc;
}

/* ==================================================================
 * ReuseQueryFileSnippets
 * ================================================================== */

/*!
 * @brief Read all SPDX snippets from a file.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[in]  hStyle   Comment style. Not NULLHANDLE.
 * @param[in]  hReport  Warning report, or NULLHANDLE.
 * @param[out] pOut     List receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (possibly no snippets).
 * @retval ERROR_INVALID_PARAMETER  pszPath or pOut is NULL.
 * @retval ERROR_INVALID_HANDLE     hStyle or hReport is not recognized.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval SPDX_TAG_ERROR_SYNTAX    Unmatched or nested snippet
 *                                  delimiters.
 */
APIRET APIENTRY ReuseQueryFileSnippets(PCSZ pszPath,
                                       HREUSECOMMENTSTYLE hStyle,
                                       HREUSETAGSREPORT hReport,
                                       PSPDXSNIPPETLIST pOut)
{
    FILE *fp;
    CHAR achLine[REUSE_TAGS_MAX_LINE];
    CHAR achValue[REUSE_TAGS_MAX_LINE];
    ULONG ulLineNo = 0;
    BOOL fFirstLine = TRUE;
    BOOL fIgnore = FALSE;
    BOOL fInSnippet = FALSE;
    ULONG ulSnippetStartLine = 0;
    PSZ pszLicAcc = NULL;
    size_t cbLicLen = 0, cbLicCap = 0;
    PSZ pszCopAcc = NULL;
    size_t cbCopLen = 0, cbCopCap = 0;
    APIRET rc;
    ULONG ulProbe;

    if (pszPath == NULL || pOut == NULL) return ERROR_INVALID_PARAMETER;

    if (hReport != NULLHANDLE) {
        if (ReuseQueryTagsReportCount(hReport, &ulProbe) != NO_ERROR)
            return ERROR_INVALID_HANDLE;
    }

    rc = SpdxSnippetListInit(pOut);
    if (rc != NO_ERROR) return rc;

    fp = fopen(pszPath, "r");
    if (fp == NULL) { SpdxSnippetListFree(pOut); return ERROR_OPEN_FAILED; }

    while (fgets(achLine, sizeof(achLine), fp) != NULL) {
        PCSZ pszBody;
        SPDXTAG tag;
        BOOL fDep, fInFile;
        ULONG ulOffset = 0, ulUsed = 0;
        APIRET rcLocal;
        int nMarker;

        ulLineNo++;
        if (fFirstLine) {
            strip_bom_in_place(achLine);
            fFirstLine = FALSE;
        }
        trim_trailing(achLine);

        if (hStyle == NULLHANDLE) {
            ulOffset = 0;
        } else {
            rcLocal = ReuseQueryCommentBody(hStyle, achLine, &ulOffset);
            if (rcLocal == ERROR_FILE_NOT_FOUND) continue;
            if (rcLocal != NO_ERROR) { rc = rcLocal; goto done; }
        }
        pszBody = achLine + ulOffset;

        nMarker = ignore_marker(pszBody);
        if (nMarker == 1) { fIgnore = TRUE;  continue; }
        if (nMarker == 2) { fIgnore = FALSE; continue; }
        if (fIgnore) continue;

        rcLocal = SpdxQueryTagLine(pszBody, &tag, &fDep, &fInFile,
                                   achValue, sizeof(achValue), &ulUsed);
        if (rcLocal == ERROR_BUFFER_OVERFLOW) {
            rc = tags_report_append(hReport, REUSE_TAGS_LONG_LINE,
                                    REUSE_SEV_WARNING, pszPath, ulLineNo,
                                    "tag value truncated, skipped");
            if (rc != NO_ERROR) goto done;
            continue;
        }
        if (rcLocal != NO_ERROR) { rc = rcLocal; goto done; }

        if (fDep) {
            SPDXTAGINFO info;
            if (SpdxQueryTagInfo(tag, &info) == NO_ERROR) {
                rc = tags_report_append(hReport, REUSE_TAGS_DEPRECATED,
                                        REUSE_SEV_WARNING, pszPath, ulLineNo,
                                        info.pszTag);
                if (rc != NO_ERROR) goto done;
            }
            continue;
        }

        switch (tag) {
        case SPDXTAG_SNIPPET_BEGIN:
            if (fInSnippet) { rc = SPDX_TAG_ERROR_SYNTAX; goto done; }
            fInSnippet = TRUE;
            ulSnippetStartLine = ulLineNo;
            cbLicLen = 0; cbCopLen = 0;
            if (pszLicAcc) pszLicAcc[0] = '\0';
            if (pszCopAcc) pszCopAcc[0] = '\0';
            continue;

        case SPDXTAG_SNIPPET_END:
            if (!fInSnippet) { rc = SPDX_TAG_ERROR_SYNTAX; goto done; }
            {
                PSPDXSNIPPET pS;
                rcLocal = snippetlist_append(pOut, &pS);
                if (rcLocal != NO_ERROR) { rc = rcLocal; goto done; }
                pS->ulLineStart = ulSnippetStartLine;
                pS->ulLineEnd = ulLineNo;
                pS->pszLicense = (pszLicAcc && cbLicLen > 0)
                                     ? (PSZ)strdup(pszLicAcc) : NULL;
                pS->pszCopyright = (pszCopAcc && cbCopLen > 0)
                                       ? (PSZ)strdup(pszCopAcc) : NULL;
            }
            fInSnippet = FALSE;
            cbLicLen = 0; cbCopLen = 0;
            if (pszLicAcc) pszLicAcc[0] = '\0';
            if (pszCopAcc) pszCopAcc[0] = '\0';
            continue;

        case SPDXTAG_LICENSE_ID:
            if (!fInSnippet) continue;
            if (!fInFile || ulUsed == 0) continue;
            rc = accum_add(&pszLicAcc, &cbLicLen, &cbLicCap,
                           " AND ", 5, achValue, ulUsed);
            if (rc != NO_ERROR) goto done;
            break;

        case SPDXTAG_SNIPPET_COPYRIGHT:
            if (!fInSnippet) continue;
            if (!fInFile) {
                rc = tags_report_append(hReport, REUSE_TAGS_NON_ANNEX_FORM,
                                        REUSE_SEV_WARNING, pszPath, ulLineNo,
                                        "SnippetCopyrightText:");
                if (rc != NO_ERROR) goto done;
                continue;
            }
            if (ulUsed == 0) continue;
            rc = accum_add(&pszCopAcc, &cbCopLen, &cbCopCap,
                           "\n", 1, achValue, ulUsed);
            if (rc != NO_ERROR) goto done;
            break;

        case SPDXTAG_OTHER:
            if (!fInSnippet) continue;
            {
                PCSZ pszValue;
                if (match_reuse_copyright(pszBody, &pszValue)) {
                    rc = accum_add(&pszCopAcc, &cbCopLen, &cbCopCap,
                                   "\n", 1, pszValue, strlen(pszValue));
                    if (rc != NO_ERROR) goto done;
                }
            }
            break;

        default:
            continue;
        }
    }

    if (fInSnippet) rc = SPDX_TAG_ERROR_SYNTAX;

done:
    fclose(fp);
    free(pszLicAcc);
    free(pszCopAcc);
    if (rc != NO_ERROR) {
        SpdxSnippetListFree(pOut);
    }
    return rc;
}
