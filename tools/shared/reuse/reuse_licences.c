/*!
 * @file reuse_licences.c
 *
 * @brief Implementation of the LICENSES/ directory handling.
 *
 * LICENSES/ directory handling, REUSE 3.3 (C89 + Watcom extensions).
 *
 * Conforms to:
 *   - REUSE Specification 3.3, §3.2.
 *     https://reuse.software/spec-3.3/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reuse_licenses.h"
#include "reuse_internal.h"
#include "spdx.h"
#include "spdx_db.h"

#ifdef __LINUX__
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#else
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#endif

/* ==================================================================
 * Internal control block
 * ================================================================== */

/*!
 * @brief Magic value identifying a valid report handle.
 */
#define CCL_REUSELICREP_MAGIC 0x524C4350UL  /* "RLCP" */

/*!
 * @struct _REUSELICREPORT
 * @brief Control block of an open report.
 */
typedef struct _REUSELICREPORT {
    unsigned long ulMagic;      /*!< CCL_REUSELICREP_MAGIC. */
    REUSEERR     *paRecords;    /*!< Backing array, or NULL. */
    ULONG         ulCount;      /*!< Used entries.          */
    ULONG         ulCapacity;   /*!< Allocated entries.     */
} REUSELICREPORT;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Translate a public report handle into the internal pointer.
 *
 * @param[in] hReport  Handle. May be NULLHANDLE.
 *
 * @return Internal pointer, or NULL on failure.
 *
 * @retval NULL  hReport is NULLHANDLE or its magic does not match.
 */
static REUSELICREPORT *get_report(HREUSELICENSEREPORT hReport) {
    REUSELICREPORT *pCtl;
    if (hReport == NULLHANDLE) return NULL;
    pCtl = (REUSELICREPORT *)hReport;
    if (pCtl->ulMagic != CCL_REUSELICREP_MAGIC) return NULL;
    return pCtl;
}

/*!
 * @brief Append one record to a report.
 *
 * A NULLHANDLE report is accepted; the record is dropped.
 *
 * @param[in] hReport    Report handle, or NULLHANDLE.
 * @param[in] ulCode     One of REUSE_ERROR_* / REUSE_LICENSES_* /
 *                       ERROR_*.
 * @param[in] ulSeverity One of REUSE_SEV_*.
 * @param[in] pszFile    Offending path, or NULL.
 * @param[in] pszDetail  Human-readable detail, or NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success, or hReport is
 *                                  NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET AppendRecord(HREUSELICENSEREPORT hReport,
                           ULONG ulCode, ULONG ulSeverity,
                           PCSZ pszFile, PCSZ pszDetail) {
    REUSELICREPORT *pCtl;
    REUSEERR *pErr;

    if (hReport == NULLHANDLE) return NO_ERROR;
    pCtl = get_report(hReport);
    if (!pCtl) return ERROR_INVALID_HANDLE;

    if (pCtl->ulCount >= pCtl->ulCapacity) {
        ULONG ulNewCap = pCtl->ulCapacity ? pCtl->ulCapacity * 2 : 8;
        REUSEERR *paNew = (REUSEERR*)realloc(pCtl->paRecords,
                                (size_t)ulNewCap * sizeof(REUSEERR));
        if (!paNew) return ERROR_NOT_ENOUGH_MEMORY;
        pCtl->paRecords = paNew;
        pCtl->ulCapacity = ulNewCap;
    }

    pErr = &pCtl->paRecords[pCtl->ulCount++];
    memset(pErr, 0, sizeof(*pErr));
    pErr->ulCode = ulCode;
    pErr->ulSeverity = ulSeverity;
    if (pszFile)
        strncpy(pErr->achFile, pszFile, sizeof(pErr->achFile) - 1);
    if (pszDetail)
        strncpy(pErr->achDetail, pszDetail, sizeof(pErr->achDetail) - 1);
    return NO_ERROR;
}

/*!
 * @brief Query whether a name is a valid SPDX license or exception.
 *
 * @param[in] pszName  Name. Not NULL.
 *
 * @return TRUE if the name is valid, FALSE otherwise.
 *
 * @retval TRUE   Known license or exception.
 * @retval FALSE  Not found in either list.
 */
static BOOL IsValidSpdxName(PCSZ pszName) {
    BOOL fValid = FALSE;

    if (SpdxQueryLicenseValid(pszName, &fValid) == NO_ERROR && fValid)
        return TRUE;
    fValid = FALSE;
    if (SpdxQueryExceptionValid(pszName, &fValid) == NO_ERROR && fValid)
        return TRUE;
    return FALSE;
}

/*!
 * @brief Query whether a file name has an extension.
 *
 * @param[in] pszName  File name. Not NULL.
 *
 * @return TRUE if the name contains a '.' that is not at position 0.
 *
 * @retval TRUE   Contains a dot not at position 0.
 * @retval FALSE  No dot, or dot at position 0.
 */
static BOOL HasExtension(PCSZ pszName) {
    PCSZ pszDot = strrchr(pszName, '.');
    if (!pszDot) return FALSE;
    return (pszDot != pszName) ? TRUE : FALSE;
}

/*!
 * @brief Strip the license extension, or recognise the whole name.
 *
 * @param[in]  pszFname   File name. Not NULL.
 * @param[out] pszBase    Base receiver. Not NULL.
 * @param[in]  ulBaseSize Size of pszBase in bytes.
 */
static void StripLicenseExt(PCSZ pszFname, PSZ pszBase, ULONG ulBaseSize) {
    PSZ pszDot;
    size_t cbLen;

    if (IsValidSpdxName(pszFname)) {
        strncpy(pszBase, pszFname, ulBaseSize - 1);
        pszBase[ulBaseSize - 1] = '\0';
        return;
    }
    strncpy(pszBase, pszFname, ulBaseSize - 1);
    pszBase[ulBaseSize - 1] = '\0';
    pszDot = strrchr(pszBase, '.');
    if (pszDot) *pszDot = '\0';

    cbLen = strlen(pszBase);
    if (cbLen == 0) {
        strncpy(pszBase, pszFname, ulBaseSize - 1);
        pszBase[ulBaseSize - 1] = '\0';
    }
}

/*!
 * @brief Query whether a string set contains a string.
 *
 * @param[in] hSet    Set. May be NULLHANDLE.
 * @param[in] pszStr  String. Not NULL.
 *
 * @return TRUE if present, FALSE otherwise.
 *
 * @retval TRUE   The set contains the string.
 * @retval FALSE  Not present, or hSet is NULLHANDLE.
 */
static BOOL StrSetHas(HSTRSET hSet, PCSZ pszStr) {
    BOOL fFound = FALSE;
    if (hSet == NULLHANDLE) return FALSE;
    if (StrSetContains(hSet, pszStr, &fFound) != NO_ERROR) return FALSE;
    return fFound ? TRUE : FALSE;
}

/*!
 * @brief Read a whole file into a heap string.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return malloc'd NUL-terminated content, or NULL on failure.
 *
 * @retval NULL  Read error or allocation failure.
 */
static PSZ ReadFileToHeap(PCSZ pszPath) {
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

/*!
 * @brief Normalize text into a heap string.
 *
 * @param[in] pszSrc  Source text. Not NULL.
 *
 * @return malloc'd normalized text, or NULL on failure.
 *
 * @retval NULL  Normalization error or allocation failure.
 */
static PSZ NormalizeToHeap(PCSZ pszSrc) {
    ULONG ulSize = 0;
    PSZ pszOut;
    if (SpdxNormalizeText(pszSrc, NULL, 0, &ulSize) != NO_ERROR)
        return NULL;
    if (ulSize == 0) return NULL;
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxNormalizeText(pszSrc, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    return pszOut;
}

/*!
 * @brief Fetch the license or exception text for an identifier.
 *
 * Tries the license table first, then the exception table. Internal
 * to LICENSES/ handling: a file in LICENSES/ may hold either.
 *
 * @param[in] pszId  Identifier. Not NULL.
 *
 * @return malloc'd text, or NULL if not found.
 *
 * @retval NULL  Not found in either table, or allocation failure.
 */
static PSZ GetDbTextHeap(PCSZ pszId) {
    ULONG ulSize = 0;
    PSZ pszOut;
    APIRET rc;

    rc = SpdxQueryLicenseText(pszId, NULL, 0, &ulSize);
    if (rc != NO_ERROR) {
        rc = SpdxQueryExceptionText(pszId, NULL, 0, &ulSize);
        if (rc != NO_ERROR) return NULL;
        pszOut = (PSZ)malloc(ulSize);
        if (!pszOut) return NULL;
        if (SpdxQueryExceptionText(pszId, pszOut, ulSize, NULL)
                != NO_ERROR) {
            free(pszOut);
            return NULL;
        }
        return pszOut;
    }
    pszOut = (PSZ)malloc(ulSize);
    if (!pszOut) return NULL;
    if (SpdxQueryLicenseText(pszId, pszOut, ulSize, NULL) != NO_ERROR) {
        free(pszOut);
        return NULL;
    }
    return pszOut;
}

/*!
 * @brief Query whether a path exists.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return TRUE if the path exists, FALSE otherwise.
 *
 * @retval TRUE   Path exists.
 * @retval FALSE  Path does not exist.
 */
static BOOL FileExists(PCSZ pszPath) {
#ifdef __LINUX__
    return (access(pszPath, F_OK) == 0) ? TRUE : FALSE;
#else
    return (_access(pszPath, 0) == 0) ? TRUE : FALSE;
#endif
}

/*!
 * @brief Create a directory, ignoring an existing one.
 *
 * @param[in] pszPath  Directory path. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Created or already existed.
 * @retval ERROR_OPEN_FAILED Cannot create and does not exist.
 */
static APIRET MakeDirectory(PCSZ pszPath) {
#ifdef __LINUX__
    if (mkdir(pszPath, 0755) == 0) return NO_ERROR;
    if (access(pszPath, F_OK) == 0) return NO_ERROR;
#else
    if (_mkdir(pszPath) == 0) return NO_ERROR;
    if (_access(pszPath, 0) == 0) return NO_ERROR;
#endif
    return ERROR_OPEN_FAILED;
}

/*!
 * @brief Write a text to a file, replacing its content.
 *
 * @param[in] pszPath  Destination path. Not NULL.
 * @param[in] pszText  Text to write, or NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_OPEN_FAILED Cannot open for writing.
 * @retval ERROR_READ_FAULT  Write error.
 */
static APIRET WriteTextFile(PCSZ pszPath, PCSZ pszText) {
    FILE *fp = fopen(pszPath, "wb");
    if (!fp) return ERROR_OPEN_FAILED;
    if (pszText && fputs(pszText, fp) == EOF) {
        fclose(fp);
        return ERROR_READ_FAULT;
    }
    fclose(fp);
    return NO_ERROR;
}

/*!
 * @brief Build the path of the LICENSES/ directory.
 *
 * @param[out] pszDst     Destination. Not NULL.
 * @param[in]  ulDstSize  Size of pszDst in bytes.
 * @param[in]  pszBase    Project root. Not NULL.
 */
static void BuildLicensesPath(PSZ pszDst, ULONG ulDstSize, PCSZ pszBase) {
#ifdef __LINUX__
    snprintf(pszDst, ulDstSize, "%s/LICENSES", pszBase);
#else
    snprintf(pszDst, ulDstSize, "%s\\LICENSES", pszBase);
#endif
}

/*!
 * @brief Build the path of one license text file.
 *
 * @param[out] pszDst      Destination. Not NULL.
 * @param[in]  ulDstSize   Size of pszDst in bytes.
 * @param[in]  pszLicPath  LICENSES/ path. Not NULL.
 * @param[in]  pszId       SPDX identifier. Not NULL.
 */
static void BuildLicenseFilePath(PSZ pszDst, ULONG ulDstSize,
                                 PCSZ pszLicPath, PCSZ pszId) {
#ifdef __LINUX__
    snprintf(pszDst, ulDstSize, "%s/%s.txt", pszLicPath, pszId);
#else
    snprintf(pszDst, ulDstSize, "%s\\%s.txt", pszLicPath, pszId);
#endif
}

/*!
 * @brief Collect base names of all files in LICENSES/.
 *
 * The directory is flat (REUSE 3.3 §3.2); subdirectories are not
 * traversed.
 *
 * @param[in]  pszLicPath  LICENSES/ path. Not NULL.
 * @param[in]  hOut        Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hOut is NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     hOut is not recognized.
 * @retval ERROR_OPEN_FAILED        Directory cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET CollectLicenseNames(PCSZ pszLicPath, HSTRSET hOut) {
#ifdef __LINUX__
    DIR *pDir;
    struct dirent *pEntry;

    pDir = opendir(pszLicPath);
    if (!pDir) return ERROR_OPEN_FAILED;

    while ((pEntry = readdir(pDir)) != NULL) {
        struct stat st;
        CHAR achFull[1200];
        APIRET rc;

        if (strcmp(pEntry->d_name, ".") == 0 ||
            strcmp(pEntry->d_name, "..") == 0) continue;

        snprintf(achFull, sizeof(achFull), "%s/%s",
                 pszLicPath, pEntry->d_name);
        if (stat(achFull, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        rc = StrSetAdd(hOut, pEntry->d_name);
        if (rc != NO_ERROR) { closedir(pDir); return rc; }
    }
    closedir(pDir);
    return NO_ERROR;
#else
    long hFile;
    struct _finddata_t fd;
    CHAR achPattern[1200];
    APIRET rc = NO_ERROR;

    snprintf(achPattern, sizeof(achPattern), "%s\\*", pszLicPath);
    hFile = _findfirst(achPattern, &fd);
    if (hFile == -1L) return ERROR_OPEN_FAILED;

    do {
        struct stat st;
        CHAR achFull[1200];

        if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
            continue;

        snprintf(achFull, sizeof(achFull), "%s\\%s",
                 pszLicPath, fd.name);
        if (stat(achFull, &st) != 0) continue;
        if (!(st.st_mode & _S_IFREG)) continue;

        rc = StrSetAdd(hOut, fd.name);
        if (rc != NO_ERROR) { _findclose(hFile); return rc; }
    } while (_findnext(hFile, &fd) == 0);
    _findclose(hFile);
    return NO_ERROR;
#endif
}

/*!
 * @brief Return the project root for the given handle.
 *
 * Uses the git repository root when available, otherwise the
 * directory passed to ReuseTreeOpen.
 *
 * @param[in] hTree  Project handle. Not NULLHANDLE.
 *
 * @return Path owned by the project handle, or NULL on failure.
 *
 * @retval NULL  Handle is not recognized.
 */
static PCSZ ProjectRoot(HREUSETREE hTree) {
    PREUSETREE pd = ReuseInternalGetDoc(hTree);
    if (!pd) return NULL;
    return pd->pszRepoRoot ? pd->pszRepoRoot : pd->pszProjectDir;
}

/* ==================================================================
 * Report lifecycle
 * ================================================================== */

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
APIRET APIENTRY ReuseLicensesReportCreate(PHREUSELICENSEREPORT phReport) {
    REUSELICREPORT *pCtl;

    if (!phReport) return ERROR_INVALID_PARAMETER;
    *phReport = NULLHANDLE;

    pCtl = (REUSELICREPORT *)calloc(1, sizeof(REUSELICREPORT));
    if (!pCtl) return ERROR_NOT_ENOUGH_MEMORY;
    pCtl->ulMagic = CCL_REUSELICREP_MAGIC;
    *phReport = (HREUSELICENSEREPORT)pCtl;
    return NO_ERROR;
}

/*!
 * @brief Release a report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   hReport is not recognized.
 */
APIRET APIENTRY ReuseLicensesReportFree(HREUSELICENSEREPORT hReport) {
    REUSELICREPORT *pCtl;
    if (hReport == NULLHANDLE) return NO_ERROR;
    pCtl = get_report(hReport);
    if (!pCtl) return ERROR_INVALID_HANDLE;
    free(pCtl->paRecords);
    pCtl->ulMagic = 0;
    free(pCtl);
    return NO_ERROR;
}

/*!
 * @brief Number of records in the report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hReport or pulCount is NULL.
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 */
APIRET APIENTRY ReuseLicensesReportGetCount(HREUSELICENSEREPORT hReport,
                                            PULONG pulCount) {
    REUSELICREPORT *pCtl = get_report(hReport);
    if (!pCtl || !pulCount) return ERROR_INVALID_PARAMETER;
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
 * @retval ERROR_INVALID_HANDLE     hReport is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      ulIndex is out of range.
 */
APIRET APIENTRY ReuseLicensesReportGet(HREUSELICENSEREPORT hReport,
                                       ULONG ulIndex,
                                       PREUSEERR pErr) {
    REUSELICREPORT *pCtl = get_report(hReport);
    if (!pCtl || !pErr) return ERROR_INVALID_PARAMETER;
    if (ulIndex >= pCtl->ulCount) return ERROR_NO_MORE_ITEMS;
    *pErr = pCtl->paRecords[ulIndex];
    return NO_ERROR;
}

/* ==================================================================
 * Validation
 * ================================================================== */

/*!
 * @brief Validate the LICENSES/ directory of a project.
 *
 * @param[in] hTree          Project handle. Not NULLHANDLE.
 * @param[in] hUsedLicenses  Set of used SPDX identifiers. Not
 *                           NULLHANDLE.
 * @param[in] hReport        Report handle, or NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Validation completed.
 * @retval ERROR_INVALID_PARAMETER  hTree or hUsedLicenses is
 *                                  NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     A handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseLicensesValidate(HREUSETREE hTree,
                                      HSTRSET hUsedLicenses,
                                      HREUSELICENSEREPORT hReport) {
    CHAR achLicPath[1200];
    PCSZ pszBase;
    HSTRSET hFilesInLic = NULLHANDLE;
    BOOL fDirExists;
    APIRET rc;

    if (hTree == NULLHANDLE || hUsedLicenses == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    pszBase = ProjectRoot(hTree);
    if (!pszBase) return ERROR_INVALID_HANDLE;

    BuildLicensesPath(achLicPath, sizeof(achLicPath), pszBase);

#ifdef __LINUX__
    fDirExists = (access(achLicPath, F_OK) == 0) ? TRUE : FALSE;
#else
    fDirExists = (_access(achLicPath, 0) == 0) ? TRUE : FALSE;
#endif

    rc = StrSetCreate(&hFilesInLic);
    if (rc != NO_ERROR) return rc;

    if (!fDirExists) {
        rc = AppendRecord(hReport, REUSE_LICENSES_DIR_MISSING,
                          REUSE_SEV_ERROR, achLicPath,
                          "LICENSES/ directory is missing");
        StrSetDestroy(hFilesInLic);
        return rc;
    }

    rc = CollectLicenseNames(achLicPath, hFilesInLic);
    if (rc != NO_ERROR) {
        StrSetDestroy(hFilesInLic);
        return rc;
    }

    /* First pass: validate each file name in LICENSES/. */
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hFilesInLic, &hEnum) == NO_ERROR) {
            do {
                CHAR achFname[512];
                CHAR achBase[256];
                CHAR achFullPath[1200];
                BOOL fDepLic = FALSE;
                BOOL fDepExc = FALSE;

                if (StrSetEnumGet(hEnum, achFname, sizeof(achFname), NULL)
                        != NO_ERROR)
                    continue;

#ifdef __LINUX__
                snprintf(achFullPath, sizeof(achFullPath), "%s/%s",
                         achLicPath, achFname);
#else
                snprintf(achFullPath, sizeof(achFullPath), "%s\\%s",
                         achLicPath, achFname);
#endif

                StripLicenseExt(achFname, achBase, sizeof(achBase));

                if (!IsValidSpdxName(achBase)) {
                    rc = AppendRecord(hReport, REUSE_LICENSES_BAD_NAME,
                                      REUSE_SEV_ERROR, achFullPath, "");
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        StrSetDestroy(hFilesInLic);
                        return rc;
                    }
                    continue;
                }

                if (!HasExtension(achFname)) {
                    rc = AppendRecord(hReport, REUSE_LICENSES_NO_EXTENSION,
                                      REUSE_SEV_WARNING, achFullPath, "");
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        StrSetDestroy(hFilesInLic);
                        return rc;
                    }
                }

                SpdxQueryLicenseDeprecated(achBase, &fDepLic);
                SpdxQueryExceptionDeprecated(achBase, &fDepExc);
                if (fDepLic || fDepExc) {
                    rc = AppendRecord(hReport, REUSE_LICENSES_DEPRECATED_ID,
                                      REUSE_SEV_WARNING, achFullPath, achBase);
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        StrSetDestroy(hFilesInLic);
                        return rc;
                    }
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    /* Second pass: every file must be used. */
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hFilesInLic, &hEnum) == NO_ERROR) {
            do {
                CHAR achFname[512];
                CHAR achBase[256];
                CHAR achFullPath[1200];

                if (StrSetEnumGet(hEnum, achFname, sizeof(achFname), NULL)
                        != NO_ERROR)
                    continue;

#ifdef __LINUX__
                snprintf(achFullPath, sizeof(achFullPath), "%s/%s",
                         achLicPath, achFname);
#else
                snprintf(achFullPath, sizeof(achFullPath), "%s\\%s",
                         achLicPath, achFname);
#endif

                StripLicenseExt(achFname, achBase, sizeof(achBase));

                if (!StrSetHas(hUsedLicenses, achBase)) {
                    rc = AppendRecord(hReport, REUSE_LICENSES_UNUSED_FILE,
                                      REUSE_SEV_ERROR, achFullPath, "");
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        StrSetDestroy(hFilesInLic);
                        return rc;
                    }
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    /* Third pass: every used identifier has a file, and the text
     * matches the SPDX database. */
    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hUsedLicenses, &hEnum) == NO_ERROR) {
            do {
                CHAR achLic[512];
                CHAR achWithTxt[512];
                BOOL fIsRef;
                BOOL fFound;
                BOOL fIsLicValid = FALSE;
                BOOL fIsExcValid = FALSE;

                if (StrSetEnumGet(hEnum, achLic, sizeof(achLic), NULL)
                        != NO_ERROR)
                    continue;

                fIsRef = (strncmp(achLic, "LicenseRef-", 11) == 0) ||
                         (strncmp(achLic, "DocumentRef-", 12) == 0);

                snprintf(achWithTxt, sizeof(achWithTxt), "%s.txt", achLic);
                fFound = StrSetHas(hFilesInLic, achLic) ||
                         StrSetHas(hFilesInLic, achWithTxt);

                if (!fIsRef) {
                    SpdxQueryLicenseValid(achLic, &fIsLicValid);
                    SpdxQueryExceptionValid(achLic, &fIsExcValid);
                    if (!fIsLicValid && !fIsExcValid) {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_BAD_ID,
                                          REUSE_SEV_ERROR, "",
                                          achLic);
                        if (rc != NO_ERROR) {
                            StrSetEnumClose(hEnum);
                            StrSetDestroy(hFilesInLic);
                            return rc;
                        }
                        continue;
                    }
                }

                if (!fFound) {
                    rc = AppendRecord(hReport,
                                      REUSE_LICENSES_MISSING_FILE,
                                      REUSE_SEV_ERROR, "",
                                      achLic);
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        StrSetDestroy(hFilesInLic);
                        return rc;
                    }
                    continue;
                }

                if (fIsRef) continue;

                {
                    CHAR achDbText[16384];
                    CHAR achFileText[16384];
                    CHAR achNormFile[16384];
                    CHAR achNormDb[16384];
                    ULONG ulDbSize = sizeof(achDbText);
                    ULONG ulFileSize = sizeof(achFileText);
                    ULONG ulNormFileSize = sizeof(achNormFile);
                    ULONG ulNormDbSize = sizeof(achNormDb);
                    PCSZ pszActualFname;
                    CHAR achFullPath[1200];
                    APIRET rcDb;
                    int fEqual = 0;

                    rcDb = SpdxQueryLicenseText(achLic, achDbText,
                                                ulDbSize, NULL);
                    if (rcDb != NO_ERROR)
                        rcDb = SpdxQueryExceptionText(achLic, achDbText,
                                                      ulDbSize, NULL);
                    if (rcDb != NO_ERROR) continue;

                    if (StrSetHas(hFilesInLic, achWithTxt))
                        pszActualFname = achWithTxt;
                    else
                        pszActualFname = achLic;

#ifdef __LINUX__
                    snprintf(achFullPath, sizeof(achFullPath),
                             "%s/%s", achLicPath, pszActualFname);
#else
                    snprintf(achFullPath, sizeof(achFullPath),
                             "%s\\%s", achLicPath, pszActualFname);
#endif

                    if (SpdxReadFileAll(achFullPath, achFileText,
                                        ulFileSize, &ulFileSize)
                            == NO_ERROR) {
                        SpdxNormalizeText(achFileText, achNormFile,
                                          ulNormFileSize,
                                          &ulNormFileSize);
                        SpdxNormalizeText(achDbText, achNormDb,
                                          ulNormDbSize,
                                          &ulNormDbSize);
                        if (strcmp(achNormFile, achNormDb) == 0)
                            fEqual = 1;
                    }

                    if (!fEqual) {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_TEXT_MISMATCH,
                                          REUSE_SEV_ERROR, achFullPath,
                                          achLic);
                        if (rc != NO_ERROR) {
                            StrSetEnumClose(hEnum);
                            StrSetDestroy(hFilesInLic);
                            return rc;
                        }
                    }
                }
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    StrSetDestroy(hFilesInLic);
    return NO_ERROR;
}

/* ==================================================================
 * Creation and update
 * ================================================================== */

/*!
 * @brief Create or update the LICENSES/ directory of a project.
 *
 * @param[in] hTree          Project handle. Not NULLHANDLE.
 * @param[in] hUsedLicenses  Set of used SPDX identifiers. Not
 *                           NULLHANDLE.
 * @param[in] fForce         TRUE to overwrite outdated files.
 * @param[in] fDryRun        TRUE to skip writes.
 * @param[in] hReport        Report handle, or NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Completed.
 * @retval ERROR_INVALID_PARAMETER  hTree or hUsedLicenses is
 *                                  NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE     A handle is not recognized.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY ReuseLicensesEnsure(HREUSETREE hTree,
                                    HSTRSET hUsedLicenses,
                                    BOOL fForce,
                                    BOOL fDryRun,
                                    HREUSELICENSEREPORT hReport) {
    CHAR achLicPath[1200];
    PCSZ pszBase;
    BOOL fDirExists;
    APIRET rc;

    if (hTree == NULLHANDLE || hUsedLicenses == NULLHANDLE)
        return ERROR_INVALID_PARAMETER;

    pszBase = ProjectRoot(hTree);
    if (!pszBase) return ERROR_INVALID_HANDLE;

    BuildLicensesPath(achLicPath, sizeof(achLicPath), pszBase);

#ifdef __LINUX__
    fDirExists = (access(achLicPath, F_OK) == 0) ? TRUE : FALSE;
#else
    fDirExists = (_access(achLicPath, 0) == 0) ? TRUE : FALSE;
#endif

    if (!fDirExists) {
        if (!fDryRun) {
            if (MakeDirectory(achLicPath) != NO_ERROR) {
                return AppendRecord(hReport,
                                    REUSE_LICENSES_DIR_MISSING,
                                    REUSE_SEV_ERROR, achLicPath,
                                    "cannot create LICENSES/");
            }
        }
        rc = AppendRecord(hReport, REUSE_LICENSES_DIR_CREATED,
                          REUSE_SEV_INFO, achLicPath,
                          fDryRun ? "would create LICENSES/"
                                  : "created LICENSES/");
        if (rc != NO_ERROR) return rc;
    }

    {
        HSTRSETENUM hEnum = NULLHANDLE;
        if (StrSetEnumFirst(hUsedLicenses, &hEnum) == NO_ERROR) {
            do {
                CHAR achLic[512];
                CHAR achPath[1200];
                PSZ pszDbText;
                BOOL fExists;

                if (StrSetEnumGet(hEnum, achLic, sizeof(achLic), NULL)
                        != NO_ERROR)
                    continue;

                if (strncmp(achLic, "LicenseRef-", 11) == 0 ||
                    strncmp(achLic, "DocumentRef-", 12) == 0) {
                    rc = AppendRecord(hReport,
                                      REUSE_LICENSES_MANUAL_REQUIRED,
                                      REUSE_SEV_INFO, "",
                                      achLic);
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        return rc;
                    }
                    continue;
                }

                pszDbText = GetDbTextHeap(achLic);
                if (!pszDbText) {
                    rc = AppendRecord(hReport,
                                      REUSE_LICENSES_NO_DB_TEXT,
                                      REUSE_SEV_ERROR, "",
                                      achLic);
                    if (rc != NO_ERROR) {
                        StrSetEnumClose(hEnum);
                        return rc;
                    }
                    continue;
                }

                BuildLicenseFilePath(achPath, sizeof(achPath),
                                     achLicPath, achLic);
                fExists = FileExists(achPath);

                if (fExists) {
                    PSZ pszFileText = ReadFileToHeap(achPath);
                    PSZ pszNf = NULL;
                    PSZ pszNd = NULL;
                    BOOL fEqual = FALSE;

                    if (pszFileText) {
                        pszNf = NormalizeToHeap(pszFileText);
                        pszNd = NormalizeToHeap(pszDbText);
                        if (pszNf && pszNd && strcmp(pszNf, pszNd) == 0)
                            fEqual = TRUE;
                        free(pszNf);
                        free(pszNd);
                        free(pszFileText);
                    }

                    if (fEqual) {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_FILE_UP_TO_DATE,
                                          REUSE_SEV_INFO, achPath,
                                          "up to date");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    } else if (!fForce) {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_FILE_OUTDATED,
                                          REUSE_SEV_WARNING, achPath,
                                          "text differs; use --force");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    } else if (fDryRun) {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_FILE_UPDATED,
                                          REUSE_SEV_INFO, achPath,
                                          "would update");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    } else if (WriteTextFile(achPath, pszDbText)
                                   != NO_ERROR) {
                        rc = AppendRecord(hReport,
                                          ERROR_OPEN_FAILED,
                                          REUSE_SEV_ERROR, achPath,
                                          "cannot write file");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    } else {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_FILE_UPDATED,
                                          REUSE_SEV_INFO, achPath,
                                          "updated");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    }
                } else {
                    if (fDryRun) {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_FILE_CREATED,
                                          REUSE_SEV_INFO, achPath,
                                          "would create");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    } else if (WriteTextFile(achPath, pszDbText)
                                   != NO_ERROR) {
                        rc = AppendRecord(hReport,
                                          ERROR_OPEN_FAILED,
                                          REUSE_SEV_ERROR, achPath,
                                          "cannot write file");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    } else {
                        rc = AppendRecord(hReport,
                                          REUSE_LICENSES_FILE_CREATED,
                                          REUSE_SEV_INFO, achPath,
                                          "created");
                        if (rc != NO_ERROR) {
                            free(pszDbText);
                            StrSetEnumClose(hEnum);
                            return rc;
                        }
                    }
                }
                free(pszDbText);
            } while (StrSetEnumNext(hEnum) == NO_ERROR);
            StrSetEnumClose(hEnum);
        }
    }

    return NO_ERROR;
}
