/* spdx.c - shared SPDX utility functions.
 * Part of the reuse support layer. */

/**
 * @file spdx.c
 * @brief Implementation of the shared SPDX utility functions.
 *
 * Copyright (c) osFree Project 2026, <http://www.osFree.org>
 *   for licence see licence.txt in root directory, or project website
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spdx.h"

/* ==================================================================
 * File helpers
 * ================================================================== */

/**
 * @brief Read an entire file into a caller-supplied buffer.
 *
 * @param[in]  pszPath  Path to the file. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY SpdxReadFileAll(PCSZ pszPath, PSZ pszBuf,
                                ULONG ulSize, PULONG pulUsed) {
    FILE *fp;
    long lSize;
    size_t cbRead;

    if (!pszPath) return ERROR_INVALID_PARAMETER;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return ERROR_READ_FAULT; }
    lSize = ftell(fp);
    if (lSize < 0) { fclose(fp); return ERROR_READ_FAULT; }

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)lSize + 1;
        fclose(fp);
        return NO_ERROR;
    }
    if (!pszBuf) { fclose(fp); return ERROR_INVALID_PARAMETER; }
    if (ulSize < (ULONG)lSize + 1) {
        if (pulUsed) *pulUsed = (ULONG)lSize + 1;
        fclose(fp);
        return ERROR_BUFFER_OVERFLOW;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return ERROR_READ_FAULT; }
    cbRead = fread(pszBuf, 1, (size_t)lSize, fp);
    fclose(fp);
    if (cbRead != (size_t)lSize) return ERROR_READ_FAULT;

    pszBuf[lSize] = '\0';
    if (pulUsed) *pulUsed = (ULONG)lSize;
    return NO_ERROR;
}

/**
 * @brief Return a pointer to the base name inside a path.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return Base name, or NULL if pszPath is NULL.
 */
PCSZ APIENTRY SpdxGetFileName(PCSZ pszPath) {
    PCSZ pszSlash;
    PCSZ pszBackslash;

    if (!pszPath) return NULL;
    pszSlash = strrchr(pszPath, '/');
    pszBackslash = strrchr(pszPath, '\\');
    if (pszBackslash && (!pszSlash || pszBackslash > pszSlash))
        return pszBackslash + 1;
    if (pszSlash)
        return pszSlash + 1;
    return pszPath;
}

/* ==================================================================
 * SPDX expression helpers
 * ================================================================== */

/**
 * @brief Collect the SPDX identifiers from a license expression.
 *
 * @param[in] pszExpr  Expression. Not NULL.
 * @param[in] hOut     Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 */
APIRET APIENTRY SpdxExpressionCollectIds(PCSZ pszExpr, HSTRSET hOut) {
    PCSZ pszDelims = " \t()";
    PSZ pszCopy;
    PSZ pszTok;
    APIRET rc;

    if (!pszExpr) return ERROR_INVALID_PARAMETER;
    if (hOut == NULLHANDLE) return ERROR_INVALID_HANDLE;

    pszCopy = strdup(pszExpr);
    if (!pszCopy) return ERROR_NOT_ENOUGH_MEMORY;

    pszTok = strtok(pszCopy, pszDelims);
    while (pszTok) {
        if (strcmp(pszTok, "AND") != 0 &&
            strcmp(pszTok, "OR")  != 0 &&
            strcmp(pszTok, "WITH") != 0 &&
            pszTok[0] != '\0') {
            rc = StrSetAdd(hOut, pszTok);
            if (rc != NO_ERROR) {
                free(pszCopy);
                return rc;
            }
        }
        pszTok = strtok(NULL, pszDelims);
    }
    free(pszCopy);
    return NO_ERROR;
}

/* ==================================================================
 * Text helpers
 * ================================================================== */

/**
 * @brief Internal buffer size for the normalization pass.
 *
 * The function processes the input twice: first to compute the
 * normalized length, then to fill the caller's buffer. The first
 * pass uses a growable scratch buffer; the caller's buffer must be
 * large enough for the normalized result.
 */

/**
 * @brief Normalize text into a private scratch buffer.
 *
 * @param[in]  pszSrc   Input text. Not NULL.
 * @param[out] ppszOut  Receiver for the malloc'd normalized text.
 *                      Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET normalize_to_heap(PCSZ pszSrc, PSZ *ppszOut) {
    size_t cbIn, cbOut, i, j;
    PSZ pszBuf;
    PSZ pszPos, pszWrite;

    *ppszOut = NULL;

    cbIn = strlen(pszSrc);
    pszBuf = (PSZ)malloc(cbIn + 1);
    if (!pszBuf) return ERROR_NOT_ENOUGH_MEMORY;

    i = 0;
    if (cbIn >= 3 && (UCHAR)pszSrc[0] == 0xEF &&
                     (UCHAR)pszSrc[1] == 0xBB &&
                     (UCHAR)pszSrc[2] == 0xBF) {
        i = 3;
    }

    j = 0;
    while (i < cbIn) {
        if (pszSrc[i] == '\r') {
            pszBuf[j++] = '\n';
            if (i + 1 < cbIn && pszSrc[i+1] == '\n') i++;
            i++;
        } else {
            pszBuf[j++] = pszSrc[i++];
        }
    }
    pszBuf[j] = '\0';

    pszPos = pszBuf;
    pszWrite = pszBuf;
    while (*pszPos) {
        PSZ pszLineStart = pszPos;
        PSZ pszLineEnd;
        while (*pszPos && *pszPos != '\n') pszPos++;
        pszLineEnd = pszPos;
        while (pszLineEnd > pszLineStart &&
               (pszLineEnd[-1] == ' ' || pszLineEnd[-1] == '\t')) {
            pszLineEnd--;
        }
        memmove(pszWrite, pszLineStart, (size_t)(pszLineEnd - pszLineStart));
        pszWrite += (pszLineEnd - pszLineStart);
        if (*pszPos == '\n') {
            *pszWrite++ = '\n';
            pszPos++;
        }
    }
    *pszWrite = '\0';
    cbOut = (size_t)(pszWrite - pszBuf);
    while (cbOut > 0 && pszBuf[cbOut-1] == '\n') cbOut--;
    pszBuf[cbOut] = '\0';

    *ppszOut = pszBuf;
    return NO_ERROR;
}

/**
 * @brief Normalize text for comparison.
 *
 * @param[in]  pszSrc   Input text. Not NULL.
 * @param[out] pszBuf   Output buffer. Not NULL unless size-query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 */
APIRET APIENTRY SpdxNormalizeText(PCSZ pszSrc, PSZ pszBuf,
                                  ULONG ulSize, PULONG pulUsed) {
    PSZ pszNormalized = NULL;
    size_t cbLen;
    APIRET rc;

    if (!pszSrc) return ERROR_INVALID_PARAMETER;

    rc = normalize_to_heap(pszSrc, &pszNormalized);
    if (rc != NO_ERROR) return rc;

    cbLen = strlen(pszNormalized);

    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        free(pszNormalized);
        return NO_ERROR;
    }
    if (!pszBuf) {
        free(pszNormalized);
        return ERROR_INVALID_PARAMETER;
    }
    if (ulSize < (ULONG)cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        free(pszNormalized);
        return ERROR_BUFFER_OVERFLOW;
    }

    memcpy(pszBuf, pszNormalized, cbLen + 1);
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    free(pszNormalized);
    return NO_ERROR;
}
