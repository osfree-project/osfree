/*!
 * @file path.c
 *
 * @brief Implementation of the path manipulation helpers.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 */

#include <stdlib.h>
#include <string.h>
#include "path.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/*!
 * @brief Return a pointer to the last separator in a path.
 *
 * @param[in] pszPath  Path. Not NULL.
 *
 * @return Pointer to the last '/' or '\\', or NULL if none.
 */
static PCSZ find_last_sep(PCSZ pszPath) {
    PCSZ pszSlash = strrchr(pszPath, '/');
    PCSZ pszBackslash = strrchr(pszPath, '\\');
    if (pszBackslash && (!pszSlash || pszBackslash > pszSlash))
        return pszBackslash;
    return pszSlash;
}

/* ------------------------------------------------------------------ */
/* Component extraction                                                */
/* ------------------------------------------------------------------ */

/*!
 * @brief Return a pointer to the base name inside a path.
 *
 * @param[in]  pszPath   Path. Not NULL.
 * @param[out] ppszBase  Receiver. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or ppszBase is NULL.
 */
APIRET APIENTRY PathGetBaseName(PCSZ pszPath, PCSZ *ppszBase) {
    PCSZ pszLast;

    if (!pszPath || !ppszBase) return ERROR_INVALID_PARAMETER;
    pszLast = find_last_sep(pszPath);
    *ppszBase = pszLast ? pszLast + 1 : pszPath;
    return NO_ERROR;
}

/*!
 * @brief Copy the directory part of a path into a caller buffer.
 *
 * @param[in]  pszPath    Path. Not NULL.
 * @param[out] pszDst     Output buffer. Not NULL unless size-query.
 * @param[in]  ulDstSize  Size of pszDst in bytes.
 * @param[out] pulUsed    Optional. May be NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or pszDst is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 */
APIRET APIENTRY PathGetDirName(PCSZ pszPath, PSZ pszDst,
                               ULONG ulDstSize, PULONG pulUsed) {
    PCSZ pszLast;
    size_t cbLen;

    if (!pszPath) return ERROR_INVALID_PARAMETER;
    if (pszDst != NULL && ulDstSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszDst == NULL && ulDstSize != 0) return ERROR_INVALID_PARAMETER;

    pszLast = find_last_sep(pszPath);

    if (!pszLast) {
        cbLen = 1;
        if (pszDst == NULL && ulDstSize == 0) {
            if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
            return NO_ERROR;
        }
        if (ulDstSize < (ULONG)cbLen + 1) {
            if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
            return ERROR_BUFFER_OVERFLOW;
        }
        pszDst[0] = '.';
        pszDst[1] = '\0';
        if (pulUsed) *pulUsed = (ULONG)cbLen;
        return NO_ERROR;
    }

    if (pszLast == pszPath) {
        cbLen = 1;
        if (pszDst == NULL && ulDstSize == 0) {
            if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
            return NO_ERROR;
        }
        if (ulDstSize < (ULONG)cbLen + 1) {
            if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
            return ERROR_BUFFER_OVERFLOW;
        }
        pszDst[0] = *pszPath;
        pszDst[1] = '\0';
        if (pulUsed) *pulUsed = (ULONG)cbLen;
        return NO_ERROR;
    }

    cbLen = (size_t)(pszLast - pszPath);
    if (pszDst == NULL && ulDstSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return NO_ERROR;
    }
    if (ulDstSize < (ULONG)cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszDst, pszPath, cbLen);
    pszDst[cbLen] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Construction                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Join a directory and a name with the platform separator.
 *
 * @param[in]  pszDir     Directory. Not NULL. May be empty.
 * @param[in]  pszName    Entry name. Not NULL.
 * @param[out] pszDst     Output buffer. Not NULL unless size-query.
 * @param[in]  ulDstSize  Size of pszDst in bytes.
 * @param[out] pulUsed    Optional. May be NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszDir or pszName is NULL, or
 *                                  pszDst is NULL without
 *                                  size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 */
APIRET APIENTRY PathMakeJoin(PCSZ pszDir, PCSZ pszName, PSZ pszDst,
                             ULONG ulDstSize, PULONG pulUsed) {
    size_t cbDirLen;
    size_t cbNameLen;
    size_t cbTotal;
    int fNeedSep;
    CHAR chSep;

    if (!pszDir || !pszName) return ERROR_INVALID_PARAMETER;
    if (pszDst != NULL && ulDstSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszDst == NULL && ulDstSize != 0) return ERROR_INVALID_PARAMETER;

    cbDirLen = strlen(pszDir);
    cbNameLen = strlen(pszName);

    fNeedSep = (cbDirLen > 0 &&
                pszDir[cbDirLen - 1] != '/' &&
                pszDir[cbDirLen - 1] != '\\') ? 1 : 0;

    cbTotal = cbDirLen + (size_t)fNeedSep + cbNameLen;

    if (pszDst == NULL && ulDstSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbTotal + 1;
        return NO_ERROR;
    }
    if (ulDstSize < (ULONG)cbTotal + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbTotal + 1;
        return ERROR_BUFFER_OVERFLOW;
    }

#ifdef __LINUX__
    chSep = '/';
#else
    chSep = '\\';
#endif

    if (cbDirLen > 0) memcpy(pszDst, pszDir, cbDirLen);
    if (fNeedSep) pszDst[cbDirLen] = chSep;
    if (cbNameLen > 0)
        memcpy(pszDst + cbDirLen + (size_t)fNeedSep, pszName, cbNameLen);
    pszDst[cbTotal] = '\0';
    if (pulUsed) *pulUsed = (ULONG)cbTotal;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Normalization                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Remove trailing path separators in place.
 *
 * @param[in,out] pszPath  Path to trim. Not NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL.
 */
APIRET APIENTRY PathRemoveTrailingSeparators(PSZ pszPath) {
    size_t cbLen;

    if (!pszPath) return ERROR_INVALID_PARAMETER;
    cbLen = strlen(pszPath);

#ifdef _WIN32
    if (cbLen == 3 && pszPath[1] == ':' &&
        (pszPath[2] == '\\' || pszPath[2] == '/')) return NO_ERROR;
#endif
    while (cbLen > 1 &&
           (pszPath[cbLen-1] == '/' || pszPath[cbLen-1] == '\\')) {
        pszPath[--cbLen] = '\0';
    }
    return NO_ERROR;
}

/*!
 * @brief Normalize a path.
 *
 * @param[in]  pszSrc     Input path. Not NULL.
 * @param[out] pszDst     Output buffer. Not NULL unless size-query.
 * @param[in]  ulDstSize  Size of pszDst in bytes.
 * @param[out] pulUsed    Optional. May be NULL.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszSrc is NULL, or pszDst is
 *                                  NULL without size-query.
 * @retval ERROR_BUFFER_OVERFLOW    pszDst too small.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
APIRET APIENTRY PathNormalize(PCSZ pszSrc, PSZ pszDst,
                              ULONG ulDstSize, PULONG pulUsed) {
    PSZ pszBuf;
    PCSZ pszPos;
    PSZ pszOut;
    size_t cbIn;
    size_t cbLen;

    if (!pszSrc) return ERROR_INVALID_PARAMETER;
    if (pszDst != NULL && ulDstSize == 0) return ERROR_INVALID_PARAMETER;
    if (pszDst == NULL && ulDstSize != 0) return ERROR_INVALID_PARAMETER;

    cbIn = strlen(pszSrc);
    pszBuf = (PSZ)malloc(cbIn + 3);
    if (!pszBuf) return ERROR_NOT_ENOUGH_MEMORY;

    pszPos = (PCSZ)pszSrc;
    pszOut = pszBuf;

#ifdef _WIN32
    if (cbIn >= 2 && pszPos[0] != '/' && pszPos[0] != '\\' &&
        pszPos[1] == ':') {
        *pszOut++ = *pszPos++;
        *pszOut++ = *pszPos++;
    }
#endif

    while (*pszPos) {
        if (*pszPos == '/' || *pszPos == '\\') {
            *pszOut++ = '/';
            while (*pszPos == '/' || *pszPos == '\\') pszPos++;
        } else if (*pszPos == '.') {
            if (pszPos[1] == '/' || pszPos[1] == '\\' ||
                pszPos[1] == '\0') {
                pszPos++;
                while (*pszPos == '/' || *pszPos == '\\') pszPos++;
            } else if (pszPos[1] == '.' &&
                       (pszPos[2] == '/' || pszPos[2] == '\\' ||
                        pszPos[2] == '\0')) {
                pszPos += 2;
                while (*pszPos == '/' || *pszPos == '\\') pszPos++;
                if (pszOut > pszBuf) {
                    pszOut--;
                    while (pszOut > pszBuf && *(pszOut - 1) != '/')
                        pszOut--;
                }
            } else {
                *pszOut++ = *pszPos++;
            }
        } else {
            *pszOut++ = *pszPos++;
        }
    }
    *pszOut = '\0';

    cbLen = (size_t)(pszOut - pszBuf);

    if (pszDst == NULL && ulDstSize == 0) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        free(pszBuf);
        return NO_ERROR;
    }
    if (ulDstSize < (ULONG)cbLen + 1) {
        if (pulUsed) *pulUsed = (ULONG)cbLen + 1;
        free(pszBuf);
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszDst, pszBuf, cbLen + 1);
    if (pulUsed) *pulUsed = (ULONG)cbLen;
    free(pszBuf);
    return NO_ERROR;
}
