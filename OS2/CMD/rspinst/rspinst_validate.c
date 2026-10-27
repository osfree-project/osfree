/*!
 * @file rspinst_validate.c
 *
 * @brief Keyword-table validator for the OS/2 response file parser.
 *
 * @par References
 *   - IBM Redbook SG24-2010-00, section 7.1.2.
 *   - IBM Redbook SG24-2010-00, section 7.1.3.
 *   - IBM Redbook SG24-2010-00, section 7.1.5 (Include semantics
 *     reflected in cycle/depth handling).
 */

#include "rspinst_private.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ==================================================================
 * Internal helpers - report
 * ================================================================== */

/*!
 * @brief Append a record to a validation report.
 *
 * @param[in] pReport   Report. Not NULL.
 * @param[in] ulCode    Record code.
 * @param[in] ulSev     Severity.
 * @param[in] pszFile   Source file. May be NULL.
 * @param[in] ulLine    Source line.
 * @param[in] pszDetail Detail message. May be NULL.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET rsp_report_add(RSP_REPORT *pReport, ULONG ulCode,
                             ULONG ulSev, PCSZ pszFile, ULONG ulLine,
                             PCSZ pszDetail)
{
    RSPERR *pNew;
    ULONG   ulNewCap;
    size_t  cbFile;
    size_t  cbDetail;

    if (pReport->ulCount == pReport->ulCapacity) {
        ulNewCap = (pReport->ulCapacity == 0)
                 ? 32
                 : pReport->ulCapacity * 2;
        pNew = (RSPERR *)realloc((void *)pReport->paItems,
                                 (size_t)(ulNewCap * sizeof(RSPERR)));
        if (pNew == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pReport->paItems = pNew;
        pReport->ulCapacity = ulNewCap;
    }
    pNew = &pReport->paItems[pReport->ulCount];
    memset(pNew, 0, sizeof(*pNew));
    pNew->ulCode = ulCode;
    pNew->ulSeverity = ulSev;
    pNew->ulLine = ulLine;

    if (pszFile != NULL) {
        cbFile = strlen(pszFile);
        if (cbFile >= sizeof(pNew->achFile)) {
            cbFile = sizeof(pNew->achFile) - 1;
        }
        memcpy(pNew->achFile, pszFile, cbFile);
        pNew->achFile[cbFile] = '\0';
    }
    if (pszDetail != NULL) {
        cbDetail = strlen(pszDetail);
        if (cbDetail >= sizeof(pNew->achDetail)) {
            cbDetail = sizeof(pNew->achDetail) - 1;
        }
        memcpy(pNew->achDetail, pszDetail, cbDetail);
        pNew->achDetail[cbDetail] = '\0';
    }

    pReport->ulCount++;
    if (ulSev == RSP_SEV_ERROR) {
        pReport->ulErrors++;
    } else if (ulSev == RSP_SEV_WARNING) {
        pReport->ulWarnings++;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Internal helpers - numeric parsing
 * ================================================================== */

/*!
 * @brief Parse a signed integer with strict end checking.
 *
 * @param[in]  pszValue  Text. Not NULL.
 * @param[out] plValue   Receiver. Not NULL.
 *
 * @return Boolean.
 *
 * @retval TRUE   The text was a valid integer.
 * @retval FALSE  The text was not a valid integer.
 */
static BOOL rsp_parse_long(PCSZ pszValue, PLONG plValue)
{
    char *pszEnd;
    LONG  lValue;

    if (pszValue == NULL) {
        return FALSE;
    }
    while (*pszValue != '\0' && isspace((unsigned char)*pszValue)) {
        pszValue++;
    }
    if (*pszValue == '\0') {
        return FALSE;
    }
    lValue = strtol(pszValue, &pszEnd, 10);
    if (pszEnd == pszValue) {
        return FALSE;
    }
    while (*pszEnd != '\0' && isspace((unsigned char)*pszEnd)) {
        pszEnd++;
    }
    if (*pszEnd != '\0') {
        return FALSE;
    }
    *plValue = lValue;
    return TRUE;
}

/*!
 * @brief Query whether a string is a drive letter or "X:".
 *
 * @param[in] pszValue  Text. Not NULL.
 *
 * @return Boolean.
 *
 * @retval TRUE   The text is a drive letter.
 * @retval FALSE  Otherwise.
 *
 * @par Reference
 *   IBM OS/2 Command Reference, "File Names".
 */
static BOOL rsp_is_drive(PCSZ pszValue)
{
    if (pszValue == NULL || !isalpha((unsigned char)pszValue[0])) {
        return FALSE;
    }
    if (pszValue[1] == '\0') {
        return TRUE;
    }
    if (pszValue[1] == ':' && pszValue[2] == '\0') {
        return TRUE;
    }
    return FALSE;
}

/* ==================================================================
 * Internal helpers - list iteration
 * ================================================================== */

/*!
 * @typedef RSPLISTCB
 * @brief Callback used by rsp_for_each_list_item.
 *
 * The callback receives one item of a comma-separated list, already
 * trimmed of surrounding whitespace.
 *
 * @param[in] pszItem  One item. Not NULL.
 * @param[in] pvUser   Caller context.
 *
 * @return Non-zero continues, zero stops.
 *
 * @retval 0     Stop the iteration.
 * @retval other Continue the iteration.
 */
typedef int ( *RSPLISTCB)(PCSZ pszItem, PVOID pvUser);

/*!
 * @brief Local trim helper used by the list iterator.
 *
 * @param[in,out] pszStr  Text. Not NULL.
 *
 * @return Pointer to the first non-whitespace character.
 *
 * @retval NULL     @p pszStr was NULL.
 * @retval other    A pointer into @p pszStr.
 */
static PSZ rsp_trim_static_helper(PSZ pszStr)
{
    PSZ pszEnd;

    if (pszStr == NULL) {
        return NULL;
    }
    while (*pszStr != '\0' && isspace((unsigned char)*pszStr)) {
        pszStr++;
    }
    if (*pszStr == '\0') {
        return pszStr;
    }
    pszEnd = pszStr + strlen(pszStr) - 1;
    while (pszEnd > pszStr && isspace((unsigned char)*pszEnd)) {
        *pszEnd = '\0';
        pszEnd--;
    }
    return pszStr;
}

/*!
 * @brief Iterate over a comma-separated list.
 *
 * @param[in] pszValue  List text. Not NULL.
 * @param[in] pfnCb     Callback. Not NULL.
 * @param[in] pvUser    Context passed to @p pfnCb.
 */
static void rsp_for_each_list_item(PCSZ pszValue,
                                   RSPLISTCB pfnCb, PVOID pvUser)
{
    char  achItem[512];
    PCSZ  pszScan;

    pszScan = pszValue;
    while (*pszScan != '\0') {
        PCSZ pszComma;
        size_t cbLen;
        PSZ pszTrimmed;

        while (*pszScan == ',' || isspace((unsigned char)*pszScan)) {
            pszScan++;
        }
        if (*pszScan == '\0') {
            break;
        }
        pszComma = strchr(pszScan, ',');
        cbLen = (pszComma != NULL)
              ? (size_t)(pszComma - pszScan)
              : strlen(pszScan);
        if (cbLen >= sizeof(achItem)) {
            cbLen = sizeof(achItem) - 1;
        }
        memcpy(achItem, pszScan, cbLen);
        achItem[cbLen] = '\0';
        pszTrimmed = rsp_trim_static_helper(achItem);
        if (*pszTrimmed != '\0') {
            if (!pfnCb(pszTrimmed, pvUser)) {
                return;
            }
        }
        if (pszComma == NULL) {
            break;
        }
        pszScan = pszComma + 1;
    }
}

/* ==================================================================
 * Internal helpers - per-keyword checks
 * ================================================================== */

/*!
 * @struct _RSP_AP_CTX
 * @brief Context for the AdditionalPrinters list callback.
 *
 * Groups the report handle and the source location so the callback
 * can attach the diagnostic to the right entry.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   AdditionalPrinters.
 */
typedef struct _RSP_AP_CTX {
    RSP_REPORT *pReport;
    PCSZ        pszFile;
    ULONG       ulLine;
} RSP_AP_CTX;

/*!
 * @brief Check one AdditionalPrinters entry of the form "port:index".
 *
 * @param[in] pszItem  One entry.
 * @param[in] pvUser   Pointer to RSP_AP_CTX.
 *
 * @return Boolean; always non-zero.
 *
 * @retval 1   The iteration continues.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   AdditionalPrinters.
 */
static int rsp_check_additional_printer_item(PCSZ pszItem, PVOID pvUser)
{
    RSP_AP_CTX *pCtx = (RSP_AP_CTX *)pvUser;
    PCSZ        pszColon;
    char        achPort[32];
    char        achIndex[32];
    size_t      cbPort;
    LONG        lPort;
    LONG        lIndex;

    pszColon = strchr(pszItem, ':');
    if (pszColon == NULL) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_BAD_PRINTER_ENTRY,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "AdditionalPrinters: expected port:index");
        return 1;
    }
    cbPort = (size_t)(pszColon - pszItem);
    if (cbPort == 0 || cbPort >= sizeof(achPort)) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_BAD_PRINTER_ENTRY,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "AdditionalPrinters: bad port length");
        return 1;
    }
    memcpy(achPort, pszItem, cbPort);
    achPort[cbPort] = '\0';
    strncpy(achIndex, pszColon + 1, sizeof(achIndex) - 1);
    achIndex[sizeof(achIndex) - 1] = '\0';
    if (!rsp_parse_long(achPort, &lPort) ||
        !rsp_parse_long(achIndex, &lIndex)) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_BAD_PRINTER_ENTRY,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "AdditionalPrinters: bad integer");
        return 1;
    }
    if (lPort < 1 || lPort > 7) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_OUT_OF_RANGE,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "AdditionalPrinters: port out of 1..7");
    }
    if (lIndex < 0 || lIndex > 9999) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_OUT_OF_RANGE,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "AdditionalPrinters: index out of 0..9999");
    }
    return 1;
}

/*!
 * @struct _RSP_WINOS2_CTX
 * @brief Context for the WIN-OS/2Support list callback.
 *
 * Tracks which of the special values 0, 1, 6 have been seen so
 * RspValidate can report the cross-value conflicts.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   WIN-OS/2Support.
 */
typedef struct _RSP_WINOS2_CTX {
    RSP_REPORT *pReport;
    PCSZ        pszFile;
    ULONG       ulLine;
    ULONG       ulCount;
    BOOL        fSeen0;
    BOOL        fSeen1;
    BOOL        fSeen6;
} RSP_WINOS2_CTX;

/*!
 * @brief Check one WIN-OS/2Support value.
 *
 * @param[in] pszItem  One value.
 * @param[in] pvUser   Pointer to RSP_WINOS2_CTX.
 *
 * @return Boolean; always non-zero.
 *
 * @retval 1   The iteration continues.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, keyword
 *   WIN-OS/2Support.
 */
static int rsp_check_winos2_item(PCSZ pszItem, PVOID pvUser)
{
    RSP_WINOS2_CTX *pCtx = (RSP_WINOS2_CTX *)pvUser;
    LONG            lValue;

    if (!rsp_parse_long(pszItem, &lValue)) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_NOT_INTEGER,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "WIN-OS/2Support: non-numeric value");
        return 1;
    }
    if (lValue < 0 || lValue > 6) {
        (void)rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_OUT_OF_RANGE,
                             RSP_SEV_WARNING, pCtx->pszFile, pCtx->ulLine,
                             "WIN-OS/2Support: value out of 0..6");
        return 1;
    }
    pCtx->ulCount++;
    if (lValue == 0) {
        pCtx->fSeen0 = TRUE;
    } else if (lValue == 1) {
        pCtx->fSeen1 = TRUE;
    } else if (lValue == 6) {
        pCtx->fSeen6 = TRUE;
    }
    return 1;
}

/* ==================================================================
 * Internal helpers - per-entry dispatch
 * ================================================================== */

/*!
 * @struct _RSP_VALCTX
 * @brief Context shared by the recursive validation walk.
 *
 * Tracks the presence of keywords that trigger cross-entry rules:
 * CID mandates (ExitOnError, RebootRequired) and the mutual
 * exclusion between the old and new formatting keywords.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, notes on ExitOnError,
 *   RebootRequired, and "Support for Old Keywords".
 */
typedef struct _RSP_VALCTX {
    RSP_REPORT            *pReport;
    const RSPVALIDATEOPTS *pOpts;
    BOOL                   fSeenExitOnError;
    BOOL                   fSeenRebootRequired;
    BOOL                   fSeenFormatFAT;
    BOOL                   fSeenFormatHPFS;
    BOOL                   fSeenBaseFs;
    BOOL                   fSeenFormatPartition;
} RSP_VALCTX;

/*!
 * @brief Validate one scalar entry.
 *
 * @param[in] pCtx    Context. Not NULL.
 * @param[in] pPair   Entry. Not NULL.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET rsp_validate_scalar(RSP_VALCTX *pCtx, const RSP_PAIR *pPair)
{
    const RSPKEYINFO *pInfo;
    LONG              lValue;
    APIRET            rc;

    pInfo = RspLookupKey(pPair->pszName);
    if (pInfo == NULL) {
        if (pCtx->pOpts->fWarnUnknown) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_UNKNOWN_KEY,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "unknown keyword");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }

    if (pInfo->type == RSP_KEYTYPE_DEPRECATED) {
        if (pCtx->pOpts->fWarnDeprecated) {
            char achDetail[256];
            sprintf(achDetail, "deprecated keyword; use %s",
                    pInfo->pszReplacement != NULL
                        ? pInfo->pszReplacement : "(unknown)");
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_DEPRECATED_KEY,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, achDetail);
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }

    if (strcmp(pPair->pszName, "ExitOnError") == 0) {
        pCtx->fSeenExitOnError = TRUE;
        if (!rsp_parse_long(pPair->pszValue, &lValue)) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_NOT_INTEGER,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, "not an integer");
            if (rc != NO_ERROR) {
                return rc;
            }
            return NO_ERROR;
        }
        if (lValue < 0 || lValue > 1) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_OUT_OF_RANGE,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, "expected 0 or 1");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        if (pCtx->pOpts->fRequireCidExitOnError && lValue != 1) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_CID_EXIT_ON_ERROR,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "CID requires ExitOnError=1");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }

    if (strcmp(pPair->pszName, "RebootRequired") == 0) {
        pCtx->fSeenRebootRequired = TRUE;
        if (!rsp_parse_long(pPair->pszValue, &lValue)) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_NOT_INTEGER,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, "not an integer");
            if (rc != NO_ERROR) {
                return rc;
            }
            return NO_ERROR;
        }
        if (lValue < 0 || lValue > 1) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_OUT_OF_RANGE,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, "expected 0 or 1");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        if (pCtx->pOpts->fRequireCidRebootRequired && lValue != 0) {
            rc = rsp_report_add(pCtx->pReport,
                                RSP_WARN_CID_REBOOT_REQUIRED,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "CID requires RebootRequired=0");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }

    if (strcmp(pPair->pszName, "FormatFAT") == 0) {
        pCtx->fSeenFormatFAT = TRUE;
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "FormatHPFS") == 0) {
        pCtx->fSeenFormatHPFS = TRUE;
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "BaseFileSystem") == 0) {
        pCtx->fSeenBaseFs = TRUE;
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "FormatPartition") == 0) {
        pCtx->fSeenFormatPartition = TRUE;
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "AdditionalPrinters") == 0) {
        RSP_AP_CTX stApCtx;
        stApCtx.pReport = pCtx->pReport;
        stApCtx.pszFile = pPair->pszFile;
        stApCtx.ulLine = pPair->ulLine;
        rsp_for_each_list_item(pPair->pszValue,
                               rsp_check_additional_printer_item,
                               &stApCtx);
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "WIN-OS/2Support") == 0) {
        RSP_WINOS2_CTX stWo;
        memset(&stWo, 0, sizeof(stWo));
        stWo.pReport = pCtx->pReport;
        stWo.pszFile = pPair->pszFile;
        stWo.ulLine = pPair->ulLine;
        rsp_for_each_list_item(pPair->pszValue, rsp_check_winos2_item,
                               &stWo);
        if (stWo.fSeen0 && stWo.ulCount > 1) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_WINOS2_CONFLICT,
                                RSP_SEV_ERROR, pPair->pszFile,
                                pPair->ulLine,
                                "WIN-OS/2Support 0 mixed with others");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        if (stWo.fSeen1 && stWo.ulCount > 1) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_WINOS2_REDUNDANT,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "WIN-OS/2Support 1 makes other values redundant");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        if (stWo.fSeen6 && stWo.ulCount > 1) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_WINOS2_CONTRADICT,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "WIN-OS/2Support 6 contradicts other values");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "OS2IniData") == 0) {
        PCSZ   pszValue = pPair->pszValue;
        size_t cbLen;
        if (pszValue == NULL || pszValue[0] != '/') {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_BAD_OS2INIDATA,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "OS2IniData: expected /App/Key/Value/");
            if (rc != NO_ERROR) {
                return rc;
            }
            return NO_ERROR;
        }
        cbLen = strlen(pszValue);
        if (cbLen < 2 || pszValue[cbLen - 1] != '/') {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_BAD_OS2INIDATA,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "OS2IniData: expected /App/Key/Value/");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }
    if (strcmp(pPair->pszName, "Copy") == 0) {
        PCSZ pszSpace = strchr(pPair->pszValue, ' ');
        if (pszSpace == NULL || pszSpace == pPair->pszValue) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_BAD_COPY_ENTRY,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "Copy: expected \"source destination\"");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        return NO_ERROR;
    }

    switch (pInfo->type) {
    case RSP_KEYTYPE_INTEGER:
    case RSP_KEYTYPE_BOOLEAN:
        if (!rsp_parse_long(pPair->pszValue, &lValue)) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_NOT_INTEGER,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, "not an integer");
            if (rc != NO_ERROR) {
                return rc;
            }
            break;
        }
        if (lValue < pInfo->lMin || lValue > pInfo->lMax) {
            char achDetail[128];
            sprintf(achDetail, "value %ld out of range [%ld..%ld]",
                    lValue, pInfo->lMin, pInfo->lMax);
            rc = rsp_report_add(pCtx->pReport,
                                RSP_WARN_VALUE_OUT_OF_RANGE,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, achDetail);
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        break;

    case RSP_KEYTYPE_DRIVE:
        if (!rsp_is_drive(pPair->pszValue)) {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_VALUE_NOT_DRIVE,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine,
                                "expected drive letter");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        break;

    case RSP_KEYTYPE_PATH:
        if (pPair->pszValue == NULL || pPair->pszValue[0] == '\0') {
            rc = rsp_report_add(pCtx->pReport, RSP_WARN_EMPTY_PATH,
                                RSP_SEV_WARNING, pPair->pszFile,
                                pPair->ulLine, "empty path");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        break;

    default:
        break;
    }

    return NO_ERROR;
}

/*!
 * @brief Recursively validate one block.
 *
 * @param[in] pCtx    Context. Not NULL.
 * @param[in] pBlock  Block. Not NULL.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET rsp_validate_block(RSP_VALCTX *pCtx, const RSP_BLOCK *pBlock)
{
    ULONG  i;
    APIRET rc;

    for (i = 0; i < pBlock->ulCount; i++) {
        if (pBlock->paItems[i].pBlock != NULL) {
            rc = rsp_validate_block(pCtx, pBlock->paItems[i].pBlock);
            if (rc != NO_ERROR) {
                return rc;
            }
        } else {
            rc = rsp_validate_scalar(pCtx, &pBlock->paItems[i]);
            if (rc != NO_ERROR) {
                return rc;
            }
        }
    }
    return NO_ERROR;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/*!
 * @brief Fill a validate-options block with CID defaults.
 *
 * The resulting block sets fRequireCidExitOnError and
 * fRequireCidRebootRequired to TRUE, fWarnUnknown and
 * fWarnDeprecated to TRUE, and fStrict to FALSE.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, notes on ExitOnError
 *   and RebootRequired.
 *
 * @param[out] pOpts  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pOpts is NULL.
 */
APIRET APIENTRY RspValidateDefaults(PRSPVALIDATEOPTS pOpts)
{
    if (pOpts == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    pOpts->fRequireCidExitOnError = TRUE;
    pOpts->fRequireCidRebootRequired = TRUE;
    pOpts->fWarnUnknown = TRUE;
    pOpts->fWarnDeprecated = TRUE;
    pOpts->fStrict = FALSE;
    return NO_ERROR;
}

/*!
 * @brief Validate a parsed document.
 *
 * Every record produced by validation is appended to @p hReport.
 * The function itself returns NO_ERROR when no error-severity
 * record was produced; RSP_ERROR_VALIDATION_FAILED means at least
 * one error record is now in the report.
 *
 * @par Reference
 *   - IBM Redbook SG24-2010-00, section 7.1.2.
 *   - IBM Redbook SG24-2010-00, section 7.1.3.
 *
 * @param[in] hDoc    Handle. Not NULLHANDLE.
 * @param[in] pOpts   Policy block. Not NULL.
 * @param[in] hReport Report handle. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                       Validation completed with
 *                                       no error-severity records.
 * @retval RSP_ERROR_VALIDATION_FAILED    At least one error record
 *                                       was produced.
 * @retval ERROR_INVALID_PARAMETER        Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE           A handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY        Allocation failure.
 */
APIRET APIENTRY RspValidate(HRSPINST hDoc,
                            const RSPVALIDATEOPTS *pOpts,
                            HRSPREPORT hReport)
{
    RSP_DOC    *pDoc;
    RSP_REPORT *pReport;
    RSP_VALCTX  stCtx;
    APIRET      rc;
    ULONG       ulErrorsBefore;

    pDoc = rsp_doc_from_handle(hDoc);
    pReport = rsp_report_from_handle(hReport);
    if (pDoc == NULL || pReport == NULL || pOpts == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    memset(&stCtx, 0, sizeof(stCtx));
    stCtx.pReport = pReport;
    stCtx.pOpts = pOpts;

    ulErrorsBefore = pReport->ulErrors;

    rc = rsp_validate_block(&stCtx, &pDoc->stRoot);
    if (rc != NO_ERROR) {
        return rc;
    }

    if ((stCtx.fSeenFormatFAT || stCtx.fSeenFormatHPFS) &&
        (stCtx.fSeenBaseFs || stCtx.fSeenFormatPartition)) {
        rc = rsp_report_add(pReport, RSP_WARN_MUTEX_FORMAT_KEYWORDS,
                            RSP_SEV_ERROR, NULL, 0,
                            "FormatFAT/FormatHPFS mixed with "
                            "BaseFileSystem/FormatPartition");
        if (rc != NO_ERROR) {
            return rc;
        }
    }
    if (pOpts->fRequireCidExitOnError && !stCtx.fSeenExitOnError) {
        rc = rsp_report_add(pReport, RSP_WARN_CID_EXIT_ON_ERROR,
                            RSP_SEV_WARNING, NULL, 0,
                            "ExitOnError missing; CID requires 1");
        if (rc != NO_ERROR) {
            return rc;
        }
    }
    if (pOpts->fRequireCidRebootRequired &&
        !stCtx.fSeenRebootRequired) {
        rc = rsp_report_add(pReport, RSP_WARN_CID_REBOOT_REQUIRED,
                            RSP_SEV_WARNING, NULL, 0,
                            "RebootRequired missing; CID requires 0");
        if (rc != NO_ERROR) {
            return rc;
        }
    }

    if (pReport->ulErrors > ulErrorsBefore) {
        return RSP_ERROR_VALIDATION_FAILED;
    }
    if (pOpts->fStrict && pReport->ulWarnings > 0) {
        return RSP_ERROR_VALIDATION_FAILED;
    }
    return NO_ERROR;
}
