/*!
 * @file res.c
 *
 * @brief Implementation of the .res source extractor.
 *
 * Extract source file names from .res files (C89).
 *
 * Resource file format (Microsoft Resource Compiler, also used by
 * OpenWatcom and Borland):
 *
 *   RESOURCEHEADER:
 *     DWORD DataSize
 *     DWORD HeaderSize
 *     ResourceType
 *     ResourceName
 *     [padding to 4 bytes]
 *
 *   ResourceType / ResourceName:
 *     - If the first WORD is 0xFFFF: ordinal (next WORD is the value).
 *     - Otherwise: length (WORD) followed by a string. Usually
 *       UTF-16, but ASCII is also possible.
 *
 *   After the header come DataSize bytes of resource data, followed
 *   by padding to a 4-byte boundary.
 *
 * The dependency resource we look for:
 *   type = 0x79 (DEP_LIST_TYPE)
 *   name = "EBWF_XFMMTUPPE" (DEP_LIST_NAME)
 *
 * The payload of that resource is a sequence of DepInfo records:
 *   DWORD time (Unix timestamp)
 *   WORD  len  (name length including the terminating '\0')
 *   char  name[len]
 *
 * Terminated by a record with time = 0 and len = 0.
 *
 * See the file header for the resource format description and
 * references.
 *
 * References:
 *   - Microsoft Resource File Formats.
 *     https://learn.microsoft.com/en-us/windows/win32/menurc/resource-file-formats
 *   - RESOURCEHEADER structure.
 *     https://learn.microsoft.com/en-us/windows/win32/menurc/resourceheader
 *
 * The dependency resource itself is Open Watcom specific and is not
 * documented by Microsoft.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "res.h"

/*!
 * @brief Resource type carrying the dependency list.
 */
#define DEP_LIST_TYPE 0x0079

/*!
 * @brief Resource name carrying the dependency list.
 */
static const CHAR achDepListName[] = "EBWF_XFMMTUPPE";

/*!
 * @brief Maximum length of a single extracted name.
 */
#define RES_NAME_MAX 512

/* ------------------------------------------------------------------ */
/* Little-endian readers                                               */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a 2-byte little-endian unsigned integer.
 *
 * @param[in] puchPos  Pointer to two bytes. Not NULL.
 *
 * @return Value in [0, 65535].
 */
static unsigned int read_u16le(const UCHAR *puchPos) {
    return (unsigned int)puchPos[0] |
           ((unsigned int)puchPos[1] << 8);
}

/*!
 * @brief Read a 4-byte little-endian unsigned integer.
 *
 * @param[in] puchPos  Pointer to four bytes. Not NULL.
 *
 * @return Value in [0, 2^32 - 1].
 */
static ULONG read_u32le(const UCHAR *puchPos) {
    return (ULONG)puchPos[0] |
           ((ULONG)puchPos[1] << 8) |
           ((ULONG)puchPos[2] << 16) |
           ((ULONG)puchPos[3] << 24);
}

/* ------------------------------------------------------------------ */
/* Resource header parsing                                             */
/* ------------------------------------------------------------------ */

/*!
 * @brief Read a resource header field (type or name).
 *
 * A field is either an ordinal (first WORD == 0xFFFF) or a
 * length-prefixed string. Strings are usually UTF-16 but may be
 * ASCII; the function detects this heuristically.
 *
 * @param[in]  puchBuf        Whole file buffer. Not NULL.
 * @param[in]  cbBufSize      Size of @p puchBuf.
 * @param[in]  cbPos          Start offset of the field.
 * @param[out] pfIsOrdinal    Receiver: TRUE if the field is an
 *                            ordinal. May be NULL.
 * @param[out] pulOrdinal     Receiver: ordinal value. May be NULL.
 * @param[in]  pszExpectName  If not NULL, the string is compared
 *                            case-sensitively to this value.
 * @param[out] pfMatched      Receiver: TRUE if the string equals
 *                            @p pszExpectName. May be NULL.
 *
 * @return Offset just past the field, or 0 on parse error.
 *
 * @retval 0  Parse error or truncated field.
 */
static size_t read_res_header_field(const UCHAR *puchBuf,
                                    size_t cbBufSize,
                                    size_t cbPos,
                                    PBOOL pfIsOrdinal,
                                    PULONG pulOrdinal,
                                    PCSZ pszExpectName,
                                    PBOOL pfMatched) {
    unsigned int unFirst;
    unsigned int unWLen;
    size_t cbIdx;
    CHAR achNameBuf[256];
    BOOL fIsUtf16;

    if (pfMatched) *pfMatched = FALSE;
    if (pfIsOrdinal) *pfIsOrdinal = FALSE;
    if (pulOrdinal) *pulOrdinal = 0;

    if (cbPos + 2 > cbBufSize) return 0;
    unFirst = read_u16le(puchBuf + cbPos);

    if (unFirst == 0xFFFF) {
        if (cbPos + 4 > cbBufSize) return 0;
        if (pfIsOrdinal) *pfIsOrdinal = TRUE;
        if (pulOrdinal) *pulOrdinal = read_u16le(puchBuf + cbPos + 2);
        return cbPos + 4;
    }

    unWLen = unFirst;
    cbPos += 2;
    if (unWLen == 0) return cbPos;

    if (cbPos + (size_t)unWLen * 2 > cbBufSize) return 0;

    fIsUtf16 = TRUE;
    for (cbIdx = 0; cbIdx < unWLen; cbIdx++) {
        if (puchBuf[cbPos + cbIdx * 2 + 1] != 0) {
            fIsUtf16 = FALSE;
            break;
        }
    }

    if (unWLen < sizeof(achNameBuf)) {
        if (fIsUtf16) {
            for (cbIdx = 0; cbIdx < unWLen; cbIdx++)
                achNameBuf[cbIdx] = (CHAR)puchBuf[cbPos + cbIdx * 2];
            achNameBuf[unWLen] = '\0';
            if (pszExpectName && pfMatched) {
                if (strcmp(achNameBuf, pszExpectName) == 0)
                    *pfMatched = TRUE;
            }
        } else {
            if (cbPos + unWLen > cbBufSize) return 0;
            memcpy(achNameBuf, puchBuf + cbPos, unWLen);
            achNameBuf[unWLen] = '\0';
            if (pszExpectName && pfMatched) {
                if (strcmp(achNameBuf, pszExpectName) == 0)
                    *pfMatched = TRUE;
            }
        }
    }

    return cbPos + (size_t)unWLen * 2;
}

/* ------------------------------------------------------------------ */
/* DepInfo parsing                                                     */
/* ------------------------------------------------------------------ */

/*!
 * @brief Parse the DepInfo records of the dependency resource.
 *
 * @param[in] puchData     Payload bytes. Not NULL.
 * @param[in] cbDataSize   Payload length.
 * @param[in] hOut         Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET parse_depinfo(const UCHAR *puchData, size_t cbDataSize,
                            HSTRSET hOut) {
    size_t cbPos = 0;
    CHAR achBuf[RES_NAME_MAX];

    while (cbPos + 6 <= cbDataSize) {
        ULONG ulTime = read_u32le(puchData + cbPos);
        unsigned int unLen = read_u16le(puchData + cbPos + 4);
        cbPos += 6;

        if (ulTime == 0 && unLen == 0) {
            return NO_ERROR;
        }
        if (unLen == 0) break;
        if (cbPos + unLen > cbDataSize) break;

        if (unLen > 1) {
            size_t cbNameLen = unLen - 1;
            APIRET rc;
            if (cbNameLen >= sizeof(achBuf)) cbNameLen = sizeof(achBuf) - 1;
            memcpy(achBuf, puchData + cbPos, cbNameLen);
            achBuf[cbNameLen] = '\0';
            rc = StrSetAdd(hOut, achBuf);
            if (rc != NO_ERROR) return rc;
        }
        cbPos += unLen;
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Extract source file names from a .res file.
 *
 * @param[in] pszPath  Path to the .res file. Not NULL.
 * @param[in] hOut     Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath is NULL, or hOut is
 *                                  NULLHANDLE.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_FILE_NOT_FOUND     Dependency resource not found.
 */
APIRET APIENTRY ResExtractSources(PCSZ pszPath, HSTRSET hOut) {
    FILE *fp;
    UCHAR *puchBuf;
    long lFileSize;
    size_t cbReadSize;
    size_t cbPos = 0;
    BOOL fFound = FALSE;
    APIRET rc = NO_ERROR;

    if (!pszPath || hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    fp = fopen(pszPath, "rb");
    if (!fp) return ERROR_OPEN_FAILED;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp); return ERROR_READ_FAULT;
    }
    lFileSize = ftell(fp);
    if (lFileSize <= 0) { fclose(fp); return ERROR_READ_FAULT; }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp); return ERROR_READ_FAULT;
    }

    puchBuf = (UCHAR*)malloc((size_t)lFileSize);
    if (!puchBuf) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    cbReadSize = fread(puchBuf, 1, (size_t)lFileSize, fp);
    fclose(fp);
    if (cbReadSize != (size_t)lFileSize) {
        free(puchBuf);
        return ERROR_READ_FAULT;
    }

    while (cbPos + 8 <= (size_t)lFileSize) {
        ULONG ulDataSize;
        ULONG ulHeaderSize;
        size_t cbHeaderStart;
        size_t cbHeaderEnd;
        BOOL fIsOrdinal;
        ULONG ulOrdinal;
        BOOL fMatched;
        size_t cbTypeEnd;
        size_t cbNameEnd;
        size_t cbDataStart;
        size_t cbNextPos;

        ulDataSize   = read_u32le(puchBuf + cbPos);
        ulHeaderSize = read_u32le(puchBuf + cbPos + 4);
        cbHeaderStart = cbPos + 8;
        cbHeaderEnd = cbHeaderStart + (size_t)ulHeaderSize;

        if (cbHeaderEnd > (size_t)lFileSize) break;

        /* Type field. */
        cbTypeEnd = read_res_header_field(puchBuf, (size_t)lFileSize,
                                          cbHeaderStart,
                                          &fIsOrdinal, &ulOrdinal,
                                          NULL, NULL);
        if (cbTypeEnd == 0) break;
        while ((cbTypeEnd % 4) != 0) cbTypeEnd++;

        /* Name field. */
        cbNameEnd = read_res_header_field(puchBuf, (size_t)lFileSize,
                                          cbTypeEnd,
                                          &fIsOrdinal, NULL,
                                          achDepListName, &fMatched);
        if (cbNameEnd == 0) break;
        while ((cbNameEnd % 4) != 0) cbNameEnd++;

        cbDataStart = cbHeaderEnd;
        while ((cbDataStart % 4) != 0) cbDataStart++;

        if (cbDataStart + ulDataSize > (size_t)lFileSize) break;

        if (ulOrdinal == DEP_LIST_TYPE && fMatched) {
            rc = parse_depinfo(puchBuf + cbDataStart,
                               (size_t)ulDataSize, hOut);
            if (rc != NO_ERROR) { free(puchBuf); return rc; }
            fFound = TRUE;
        }

        cbNextPos = cbDataStart + (size_t)ulDataSize;
        while ((cbNextPos % 4) != 0) cbNextPos++;
        if (cbNextPos <= cbPos) break;
        cbPos = cbNextPos;
    }

    free(puchBuf);

    if (!fFound) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}
