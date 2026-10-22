/*!
 * @file osadl.c
 *
 * @brief Implementation of the OSADL license compatibility matrix
 *        reader.
 *
 * Layer L2.0a. Depends only on os2types.h, os2err.h and json.h.
 * Does not know REUSE, SPDX, precedence, or comment markers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osadl.h"
#include "json.h"

/* ==================================================================
 * Internal structures
 * ================================================================== */

/*!
 * @def OSADL_KEY_MAX
 * @brief Maximum length (including NUL) of a license key in
 *        matrix.json. Actual identifiers are far shorter; this is
 *        a safety bound for stack buffers.
 */
#define OSADL_KEY_MAX 512

/*!
 * @def OSADL_MAGIC
 * @brief Handle-validation magic for HOSADLMATRIX.
 *
 * 'OSMX' — OSADL MatriX. Stored in ulMagic; zeroed before free so
 * that a second close on the same pointer is detected as
 * ERROR_INVALID_HANDLE (best effort; double-close remains undefined
 * per the owned-handle contract).
 */
#define OSADL_MAGIC 0x4F534D58u

/*!
 * @struct _OSADLENTRY
 * @brief One inbound verdict within an outbound row.
 */
typedef struct _OSADLENTRY {
    PSZ         pszInbound;  /*!< Inbound identifier, owned. */
    OSADLCOMPAT compat;      /*!< Verdict.                    */
} OSADLENTRY, *POSADLENTRY;

/*!
 * @struct _OSADLROW
 * @brief One outbound license and its verdicts.
 *
 * Entries are sorted by pszInbound so that OsadlQueryPair can use
 * bsearch.
 */
typedef struct _OSADLROW {
    PSZ         pszOutbound; /*!< Outbound identifier, owned.     */
    ULONG       ulCount;     /*!< Number of entries.              */
    POSADLENTRY paEntries;   /*!< Sorted entries, or NULL.        */
} OSADLROW, *POSADLROW;

/*!
 * @struct _OSADLMATRIX
 * @brief Parsed matrix.
 *
 * Rows are sorted by pszOutbound so that OsadlQueryPair can use
 * bsearch.
 */
typedef struct _OSADLMATRIX {
    ULONG     ulMagic;          /*!< OSADL_MAGIC.                   */
    CHAR      achTimestamp[64]; /*!< Snapshot timestamp from JSON.  */
    ULONG     ulRowCount;       /*!< Number of outbound rows.       */
    POSADLROW paRows;           /*!< Sorted rows, or NULL.          */
} OSADLMATRIX, *POSADLMATRIX;

/* ==================================================================
 * Internal helpers
 * ================================================================== */

/*!
 * @brief Validate a matrix handle.
 *
 * @param[in] hMatrix  Candidate handle. May be NULLHANDLE.
 *
 * @return TRUE if the handle is non-null and carries OSADL_MAGIC,
 *         FALSE otherwise.
 *
 * @retval TRUE   Valid OSADL matrix handle.
 * @retval FALSE  NULLHANDLE, foreign handle, or already closed.
 */
static BOOL osadl_valid(HOSADLMATRIX hMatrix)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    return m != NULL && m->ulMagic == OSADL_MAGIC;
}

/*!
 * @brief Read a whole file into a NUL-terminated buffer.
 *
 * @param[in]  pszPath  Path. Not NULL.
 * @param[out] ppszBuf  Receiver. Not NULL. Set to NULL on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Seek or read failed.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET osadl_read_file(PCSZ pszPath, PSZ *ppszBuf)
{
    FILE  *fp;
    long   lSize;
    PSZ    pszBuf;
    size_t cbRead;

    if (!pszPath || !ppszBuf) return ERROR_INVALID_PARAMETER;
    *ppszBuf = NULL;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return ERROR_READ_FAULT; }
    lSize = ftell(fp);
    if (lSize < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp); return ERROR_READ_FAULT;
    }

    pszBuf = (PSZ)malloc((size_t)lSize + 1);
    if (!pszBuf) { fclose(fp); return ERROR_NOT_ENOUGH_MEMORY; }

    cbRead = fread(pszBuf, 1, (size_t)lSize, fp);
    fclose(fp);
    if (cbRead != (size_t)lSize) { free(pszBuf); return ERROR_READ_FAULT; }
    pszBuf[lSize] = '\0';
    *ppszBuf = pszBuf;
    return NO_ERROR;
}

/*!
 * @brief Map a verdict string from matrix.json to an enum value.
 *
 * Recognises exactly the five strings that occur in the file:
 * "Same", "Yes", "No", "Check dependency", "Unknown".
 *
 * @param[in]  s  Verdict string. May be NULL.
 * @param[out] p  Receiver. Not NULL.
 *
 * @return TRUE if @p s matched, FALSE otherwise.
 *
 * @retval TRUE   Matched; @p *p is set.
 * @retval FALSE  Unrecognised string, or any argument is NULL.
 */
static BOOL osadl_verdict_from_string(PCSZ s, POSADLCOMPAT p)
{
    if (!s || !p) return FALSE;
    if (strcmp(s, "Same")             == 0) { *p = OSADL_COMPAT_SAME;      return TRUE; }
    if (strcmp(s, "Yes")              == 0) { *p = OSADL_COMPAT_YES;       return TRUE; }
    if (strcmp(s, "No")               == 0) { *p = OSADL_COMPAT_NO;        return TRUE; }
    if (strcmp(s, "Check dependency") == 0) { *p = OSADL_COMPAT_CHECK_DEP; return TRUE; }
    if (strcmp(s, "Unknown")          == 0) { *p = OSADL_COMPAT_UNKNOWN;   return TRUE; }
    return FALSE;
}

/*!
 * @brief qsort comparator for rows, by pszOutbound.
 *
 * @param[in] a  First row. Not NULL.
 * @param[in] b  Second row. Not NULL.
 *
 * @return Negative, zero, or positive per strcmp of pszOutbound.
 */
static int osadl_row_cmp_qsort(const void *a, const void *b)
{
    return strcmp(((const OSADLROW *)a)->pszOutbound,
                  ((const OSADLROW *)b)->pszOutbound);
}

/*!
 * @brief bsearch comparator for rows, by pszOutbound.
 *
 * @param[in] key   Identifier to look up. Not NULL.
 * @param[in] elem  Candidate row. Not NULL.
 *
 * @return Negative, zero, or positive per strcmp.
 */
static int osadl_row_cmp_bsearch(const void *key, const void *elem)
{
    return strcmp((const char *)key,
                  ((const OSADLROW *)elem)->pszOutbound);
}

/*!
 * @brief qsort comparator for entries, by pszInbound.
 *
 * @param[in] a  First entry. Not NULL.
 * @param[in] b  Second entry. Not NULL.
 *
 * @return Negative, zero, or positive per strcmp of pszInbound.
 */
static int osadl_entry_cmp_qsort(const void *a, const void *b)
{
    return strcmp(((const OSADLENTRY *)a)->pszInbound,
                  ((const OSADLENTRY *)b)->pszInbound);
}

/*!
 * @brief bsearch comparator for entries, by pszInbound.
 *
 * @param[in] key   Identifier to look up. Not NULL.
 * @param[in] elem  Candidate entry. Not NULL.
 *
 * @return Negative, zero, or positive per strcmp.
 */
static int osadl_entry_cmp_bsearch(const void *key, const void *elem)
{
    return strcmp((const char *)key,
                  ((const OSADLENTRY *)elem)->pszInbound);
}

/*!
 * @brief Release a matrix and every owned string.
 *
 * Does not touch magic; callers that need to invalidate a handle
 * must clear ulMagic themselves before calling this.
 *
 * @param[in] m  Matrix. May be NULL.
 */
static void osadl_free(POSADLMATRIX m)
{
    ULONG i, j;
    if (!m) return;
    if (m->paRows) {
        for (i = 0; i < m->ulRowCount; i++) {
            if (m->paRows[i].pszOutbound) free(m->paRows[i].pszOutbound);
            if (m->paRows[i].paEntries) {
                for (j = 0; j < m->paRows[i].ulCount; j++) {
                    if (m->paRows[i].paEntries[j].pszInbound)
                        free(m->paRows[i].paEntries[j].pszInbound);
                }
                free(m->paRows[i].paEntries);
            }
        }
        free(m->paRows);
    }
    free(m);
}

/*!
 * @brief Find a row by outbound identifier.
 *
 * Requires the rows to be sorted (done once in OsadlOpenMatrix).
 *
 * @param[in] m            Matrix. Not NULL.
 * @param[in] pszOutbound  Identifier. Not NULL.
 *
 * @return Row pointer, or NULL if not present.
 *
 * @retval NULL  Not present.
 */
static POSADLROW osadl_find_row(POSADLMATRIX m, PCSZ pszOutbound)
{
    return (POSADLROW)bsearch(pszOutbound, m->paRows, m->ulRowCount,
                              sizeof(OSADLROW), osadl_row_cmp_bsearch);
}

/*!
 * @brief Shared implementation of the size-query getters.
 *
 * @param[in]  pszSrc   Source string. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success (copy or size-query).
 * @retval ERROR_INVALID_PARAMETER  pulUsed is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
static APIRET osadl_size_query(PCSZ pszSrc,
                               PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    size_t cbLen;
    if (!pulUsed) return ERROR_INVALID_PARAMETER;
    cbLen = strlen(pszSrc);
    if (!pszBuf && ulSize == 0) {
        *pulUsed = (ULONG)cbLen + 1; return NO_ERROR;
    }
    if (!pszBuf) return ERROR_INVALID_PARAMETER;
    if (ulSize < cbLen + 1) {
        *pulUsed = (ULONG)cbLen + 1; return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszSrc, cbLen);
    pszBuf[cbLen] = '\0';
    *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/* ==================================================================
 * Public API
 * ================================================================== */

/*!
 * @brief Open and parse an OSADL matrix.json file.
 *
 * Reads the file, parses it via JsonParse, and builds a sorted
 * in-memory representation. On success @p *phMatrix receives a
 * valid handle; on error it is set to NULLHANDLE.
 *
 * Enforces the file invariant: every outbound key must appear as
 * an inbound key in every row. Additional inbound-only keys
 * (e.g. "GPL-2.0-only WITH Classpath-exception-2.0") are allowed.
 *
 * @param[in]  pszPath   Path to matrix.json. Not NULL.
 * @param[out] phMatrix  Handle receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or phMatrix is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval OSADL_ERROR_SYNTAX       Malformed JSON, unexpected shape,
 *                                  or unknown verdict string.
 * @retval OSADL_ERROR_VERSION      "timestamp" field missing.
 * @retval OSADL_ERROR_MISSING_KEY  File invariant violated.
 */
APIRET APIENTRY OsadlOpenMatrix(PCSZ pszPath, HOSADLMATRIX *phMatrix)
{
    PSZ          pszText      = NULL;
    HJSONDOC     hDoc         = NULLHANDLE;
    HJSONNODE    hRoot        = NULLHANDLE;
    HJSONNODE    hVal         = NULLHANDLE;
    HJSONNODE    hInVal       = NULLHANDLE;
    POSADLMATRIX m            = NULL;
    POSADLROW    pRow;
    APIRET       rc           = NO_ERROR;
    BOOL         fHaveTimestamp = FALSE;
    ULONG        ulType, ulTopCount, ulInnerCount, i, j;
    CHAR         achKey[OSADL_KEY_MAX];
    CHAR         achInKey[OSADL_KEY_MAX];
    CHAR         achVerdict[64];
    ULONG        ulKeyUsed, ulInKeyUsed, ulVerdictUsed;
    OSADLCOMPAT  compat;
    PSZ          pszDup;

    if (!pszPath || !phMatrix) return ERROR_INVALID_PARAMETER;
    *phMatrix = NULLHANDLE;

    rc = osadl_read_file(pszPath, &pszText);
    if (rc != NO_ERROR) return rc;

    rc = JsonParse(pszText, &hDoc);
    free(pszText);
    if (rc != NO_ERROR) return rc;

    if (JsonRoot(hDoc, &hRoot) != NO_ERROR ||
        JsonNodeGetType(hRoot, &ulType) != NO_ERROR ||
        ulType != JSON_OBJECT ||
        JsonNodeGetCount(hRoot, &ulTopCount) != NO_ERROR) {
        JsonClose(hDoc);
        return OSADL_ERROR_SYNTAX;
    }

    m = (POSADLMATRIX)calloc(1, sizeof(*m));
    if (!m) { JsonClose(hDoc); return ERROR_NOT_ENOUGH_MEMORY; }

    m->paRows = (POSADLROW)calloc(ulTopCount > 0 ? ulTopCount : 1,
                                  sizeof(OSADLROW));
    if (!m->paRows) {
        JsonClose(hDoc); free(m); return ERROR_NOT_ENOUGH_MEMORY;
    }

    for (i = 0; i < ulTopCount && rc == NO_ERROR; i++) {
        rc = JsonNodeGetEntry(hRoot, i, achKey, sizeof(achKey),
                              &ulKeyUsed, &hVal);
        if (rc == ERROR_BUFFER_OVERFLOW) { rc = OSADL_ERROR_SYNTAX; break; }
        if (rc != NO_ERROR) break;

        if (strcmp(achKey, "timeformat") == 0) continue;

        if (strcmp(achKey, "timestamp") == 0) {
            ULONG ulTsUsed = 0;
            if (JsonNodeGetString(hVal, m->achTimestamp,
                                  sizeof(m->achTimestamp), &ulTsUsed) == NO_ERROR
                && ulTsUsed > 0) {
                fHaveTimestamp = TRUE;
            }
            continue;
        }

        if (JsonNodeGetType(hVal, &ulType) != NO_ERROR ||
            ulType != JSON_OBJECT) {
            rc = OSADL_ERROR_SYNTAX; break;
        }

        pszDup = strdup(achKey);
        if (!pszDup) { rc = ERROR_NOT_ENOUGH_MEMORY; break; }

        pRow = &m->paRows[m->ulRowCount];
        pRow->pszOutbound = pszDup;
        pRow->ulCount     = 0;
        pRow->paEntries   = NULL;
        m->ulRowCount++;

        if (JsonNodeGetCount(hVal, &ulInnerCount) != NO_ERROR) {
            rc = OSADL_ERROR_SYNTAX; break;
        }
        if (ulInnerCount == 0) continue;

        pRow->paEntries = (POSADLENTRY)calloc(ulInnerCount,
                                              sizeof(OSADLENTRY));
        if (!pRow->paEntries) { rc = ERROR_NOT_ENOUGH_MEMORY; break; }

        for (j = 0; j < ulInnerCount; j++) {
            rc = JsonNodeGetEntry(hVal, j, achInKey, sizeof(achInKey),
                                  &ulInKeyUsed, &hInVal);
            if (rc == ERROR_BUFFER_OVERFLOW) { rc = OSADL_ERROR_SYNTAX; break; }
            if (rc != NO_ERROR) break;

            rc = JsonNodeGetString(hInVal, achVerdict,
                                   sizeof(achVerdict), &ulVerdictUsed);
            if (rc != NO_ERROR ||
                !osadl_verdict_from_string(achVerdict, &compat)) {
                rc = OSADL_ERROR_SYNTAX; break;
            }

            pszDup = strdup(achInKey);
            if (!pszDup) { rc = ERROR_NOT_ENOUGH_MEMORY; break; }

            pRow->paEntries[pRow->ulCount].pszInbound = pszDup;
            pRow->paEntries[pRow->ulCount].compat     = compat;
            pRow->ulCount++;
        }
    }

    if (rc == NO_ERROR && !fHaveTimestamp) rc = OSADL_ERROR_VERSION;

    if (rc == NO_ERROR) {
        ULONG k, k2;
        qsort(m->paRows, m->ulRowCount, sizeof(OSADLROW),
              osadl_row_cmp_qsort);
        for (k = 0; k < m->ulRowCount; k++) {
            if (m->paRows[k].ulCount > 0) {
                qsort(m->paRows[k].paEntries, m->paRows[k].ulCount,
                      sizeof(OSADLENTRY), osadl_entry_cmp_qsort);
            }
        }

        /* Invariant: every outbound key must be an inbound key of
         * every row. Additional inbound-only keys are allowed. */
        for (k = 0; k < m->ulRowCount && rc == NO_ERROR; k++) {
            for (k2 = 0; k2 < m->ulRowCount; k2++) {
                if (bsearch(m->paRows[k2].pszOutbound,
                            m->paRows[k].paEntries,
                            m->paRows[k].ulCount,
                            sizeof(OSADLENTRY),
                            osadl_entry_cmp_bsearch) == NULL) {
                    rc = OSADL_ERROR_MISSING_KEY;
                    break;
                }
            }
        }
    }

    JsonClose(hDoc);

    if (rc != NO_ERROR) { osadl_free(m); return rc; }
    m->ulMagic = OSADL_MAGIC;
    *phMatrix = (HOSADLMATRIX)m;
    return NO_ERROR;
}

/*!
 * @brief Release an OSADL matrix.
 *
 * Passing NULLHANDLE is a no-op. A non-null handle whose magic does
 * not match is rejected with ERROR_INVALID_HANDLE. The magic is
 * cleared before free so that a second close of the same pointer is
 * detected as ERROR_INVALID_HANDLE (best effort: double-close
 * remains undefined per the owned-handle contract).
 *
 * @param[in] hMatrix  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not an OSADL matrix or
 *                                was already closed.
 */
APIRET APIENTRY OsadlCloseMatrix(HOSADLMATRIX hMatrix)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    if (hMatrix == NULLHANDLE) return NO_ERROR;
    if (!osadl_valid(hMatrix)) return ERROR_INVALID_HANDLE;
    m->ulMagic = 0;
    osadl_free(m);
    return NO_ERROR;
}

/*!
 * @brief Look up a (outbound, inbound) license pair.
 *
 * Both identifiers are matched byte-for-byte. "A WITH B" forms are
 * distinct keys. No SPDX canonicalization is performed.
 *
 * Absence of the pair is reported as OSADL_COMPAT_UNDEF; that is a
 * normal outcome, not an error.
 *
 * @param[in]  hMatrix     Handle. Not NULLHANDLE.
 * @param[in]  pszOutbound Outbound identifier. Not NULL.
 * @param[in]  pszInbound  Inbound identifier. Not NULL.
 * @param[out] pCompat     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pszOutbound, pszInbound, or
 *                                  pCompat is NULL.
 */
APIRET APIENTRY OsadlQueryPair(HOSADLMATRIX hMatrix,
                               PCSZ pszOutbound, PCSZ pszInbound,
                               POSADLCOMPAT pCompat)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    POSADLROW    pRow;
    POSADLENTRY  pEntry;

    if (!osadl_valid(hMatrix)) return ERROR_INVALID_HANDLE;
    if (!pszOutbound || !pszInbound || !pCompat)
        return ERROR_INVALID_PARAMETER;

    pRow = osadl_find_row(m, pszOutbound);
    if (!pRow) { *pCompat = OSADL_COMPAT_UNDEF; return NO_ERROR; }

    pEntry = (POSADLENTRY)bsearch(pszInbound, pRow->paEntries, pRow->ulCount,
                                  sizeof(OSADLENTRY),
                                  osadl_entry_cmp_bsearch);
    *pCompat = pEntry ? pEntry->compat : OSADL_COMPAT_UNDEF;
    return NO_ERROR;
}

/*!
 * @brief Test whether a license identifier occurs as an outbound
 *        key in the matrix.
 *
 * @param[in]  hMatrix   Handle. Not NULLHANDLE.
 * @param[in]  pszId     License identifier. Not NULL.
 * @param[out] pfPresent Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pszId or pfPresent is NULL.
 */
APIRET APIENTRY OsadlQueryHasLicense(HOSADLMATRIX hMatrix,
                                     PCSZ pszId, PBOOL pfPresent)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    if (!osadl_valid(hMatrix)) return ERROR_INVALID_HANDLE;
    if (!pszId || !pfPresent) return ERROR_INVALID_PARAMETER;
    *pfPresent = (osadl_find_row(m, pszId) != NULL) ? TRUE : FALSE;
    return NO_ERROR;
}

/*!
 * @brief Query the number of outbound licenses in the matrix.
 *
 * @param[in]  hMatrix   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pulCount is NULL.
 */
APIRET APIENTRY OsadlQueryLicenseCount(HOSADLMATRIX hMatrix, PULONG pulCount)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    if (!osadl_valid(hMatrix)) return ERROR_INVALID_HANDLE;
    if (!pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = m->ulRowCount;
    return NO_ERROR;
}

/*!
 * @brief Query the outbound license identifier at a given index.
 *
 * Indices are stable for the lifetime of the handle and enumerate
 * outbound keys in sorted order. Size-query convention applies.
 *
 * @param[in]  hMatrix   Handle. Not NULLHANDLE.
 * @param[in]  ulIndex   Index. Must be < OsadlQueryLicenseCount.
 * @param[out] pszBuf    Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize    Size of pszBuf.
 * @param[out] pulUsed   Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pulUsed is NULL, or ulIndex is
 *                                  out of range, or pszBuf is NULL
 *                                  without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY OsadlQueryLicenseName(HOSADLMATRIX hMatrix, ULONG ulIndex,
                                      PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    if (!osadl_valid(hMatrix)) return ERROR_INVALID_HANDLE;
    if (ulIndex >= m->ulRowCount) return ERROR_INVALID_PARAMETER;
    return osadl_size_query(m->paRows[ulIndex].pszOutbound,
                            pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Query the snapshot timestamp from the source file.
 *
 * Returns the value of the "timestamp" field, as stored in the
 * file. Use this value for version pinning in downstream reports.
 * Size-query convention applies.
 *
 * @param[in]  hMatrix  Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf.
 * @param[out] pulUsed  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     hMatrix is not an OSADL matrix.
 * @retval ERROR_INVALID_PARAMETER  pulUsed is NULL, or pszBuf is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY OsadlQueryTimestamp(HOSADLMATRIX hMatrix,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    POSADLMATRIX m = (POSADLMATRIX)hMatrix;
    if (!osadl_valid(hMatrix)) return ERROR_INVALID_HANDLE;
    return osadl_size_query(m->achTimestamp, pszBuf, ulSize, pulUsed);
}
