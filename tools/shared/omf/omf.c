/* omf.c - extract source file names from OMF object files (C89)
 *
 * OMF (Relocatable Object Module Format) records scanned for source
 * names:
 *   - THEADR (0x80) — module name, usually the original source file.
 *   - COMENT (0x88) — comment records with vendor-specific classes
 *     carrying dependency file lists.
 *
 * Vendor-specific COMENT classes recognized:
 *   - 0xE9 — Borland auto-dependency: timestamp (4 bytes) followed
 *     by repeated [len][name] entries.
 *   - 0xFB — OpenWatcom auto-dependency: same layout as 0xE9.
 *   - 0x88 — additional dependency class found in OpenWatcom output.
 *   - 0xE8 — Borland source file name: single [len][name] entry.
 *
 * References:
 *   - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *     Object Module Format), Linux Foundation.
 *     https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *   - JWasm / OpenWatcom WASM sources (COMENT class usage).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "omf.h"

/**
 * @file omf.c
 * @brief Implementation of the OMF source extractor.
 *
 * See the file header for the list of recognized record types and
 * vendor-specific COMENT classes, and for the references.
 */

/** @brief Maximum length of a single extracted name. */
#define OMF_NAME_MAX 512

/**
 * @brief Read a 2-byte little-endian unsigned integer.
 *
 * @param[in] puchPos  Pointer to two bytes. Not NULL.
 *
 * @return Value in [0, 65535].
 */
static unsigned int read_u16(const UCHAR *puchPos) {
    return (unsigned int)puchPos[0] |
           ((unsigned int)puchPos[1] << 8);
}

/**
 * @brief Append a name of length @p cbLen to the destination set.
 *
 * The name is copied into a temporary NUL-terminated buffer before
 * being handed to StrSetAdd.
 *
 * @param[in] hOut      Destination set. Not NULLHANDLE.
 * @param[in] puchStr   Source bytes. Not NULL.
 * @param[in] cbLen     Number of bytes.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success (including cbLen == 0).
 * @retval ERROR_INVALID_PARAMETER  cbLen exceeds the internal buffer.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET add_name(HSTRSET hOut, const UCHAR *puchStr, size_t cbLen) {
    CHAR achBuf[OMF_NAME_MAX];
    if (!puchStr || cbLen == 0) return NO_ERROR;
    if (cbLen >= sizeof(achBuf)) return ERROR_INVALID_PARAMETER;
    memcpy(achBuf, puchStr, cbLen);
    achBuf[cbLen] = '\0';
    return StrSetAdd(hOut, achBuf);
}

/**
 * @brief Process a THEADR record (0x80).
 *
 * Layout: [name_len:1][name:name_len].
 *
 * @param[in] puchData  Record payload. Not NULL.
 * @param[in] cbLen     Payload length.
 * @param[in] hOut      Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 */
static APIRET process_theadr(const UCHAR *puchData, size_t cbLen,
                             HSTRSET hOut) {
    size_t cbPos = 0;
    size_t cbNameLen;

    if (cbLen < 1) return NO_ERROR;
    cbNameLen = puchData[cbPos];
    cbPos++;
    if (cbPos + cbNameLen > cbLen) return NO_ERROR;
    if (cbNameLen > 0) {
        return add_name(hOut, &puchData[cbPos], cbNameLen);
    }
    return NO_ERROR;
}

/**
 * @brief Process a COMENT record (0x88).
 *
 * Layout: [comment_type:1][comment_class:1][payload]. The payload
 * layout depends on the comment class; see the file header for the
 * list of recognized classes.
 *
 * @param[in] puchData  Record payload. Not NULL.
 * @param[in] cbLen     Payload length.
 * @param[in] hOut      Destination set. Not NULLHANDLE.
 *
 * @return APIRET
 */
static APIRET process_coment(const UCHAR *puchData, size_t cbLen,
                             HSTRSET hOut) {
    UCHAR uchCommentType;
    UCHAR uchCommentClass;
    size_t cbPos;
    size_t cbPayloadLen;

    if (cbLen < 2) return NO_ERROR;

    uchCommentType = puchData[0];
    uchCommentClass = puchData[1];
    cbPos = 2;
    cbPayloadLen = cbLen - 2;

    (void)uchCommentType;

    switch (uchCommentClass) {
        case 0xE9:    /* Borland auto-dependency */
        case 0xFB: {  /* OpenWatcom dependency */
            APIRET rc;
            if (cbPayloadLen < 4) return NO_ERROR;
            cbPos += 4;   /* Skip the 4-byte timestamp. */

            while (cbPos + 1 <= cbLen) {
                size_t cbStrLen = puchData[cbPos];
                cbPos++;
                if (cbStrLen == 0) continue;
                if (cbPos + cbStrLen > cbLen) break;
                rc = add_name(hOut, &puchData[cbPos], cbStrLen);
                if (rc != NO_ERROR) return rc;
                cbPos += cbStrLen;
            }
            break;
        }
        case 0x88: {  /* Dependency file: timestamp + [len][name] */
            if (cbPayloadLen < 5) return NO_ERROR;
            cbPos += 4; /* Skip timestamp. */
            if (cbPos < cbLen) {
                size_t cbNameLen = puchData[cbPos];
                cbPos++;
                if (cbPos + cbNameLen <= cbLen && cbNameLen > 0) {
                    return add_name(hOut, &puchData[cbPos], cbNameLen);
                }
            }
            break;
        }
        case 0xE8: {  /* Borland source file: [len][name] */
            if (cbPayloadLen < 1) return NO_ERROR;
            if (cbPos < cbLen) {
                size_t cbNameLen = puchData[cbPos];
                cbPos++;
                if (cbPos + cbNameLen <= cbLen && cbNameLen > 0) {
                    return add_name(hOut, &puchData[cbPos], cbNameLen);
                }
            }
            break;
        }
        default:
            break;
    }
    return NO_ERROR;
}

/**
 * @brief Extract source file names from an OMF object file.
 *
 * @param[in] pszPath  Path to the OMF object file. Not NULL.
 * @param[in] hOut     Destination string set. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or hOut is invalid.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval ERROR_FILE_NOT_FOUND     No source names found.
 */
APIRET APIENTRY OmfExtractSources(PCSZ pszPath, HSTRSET hOut) {
    FILE *fp;
    UCHAR *puchBuf;
    long lFileSize;
    size_t cbReadSize;
    size_t cbPos = 0;
    ULONG ulBefore = 0;
    ULONG ulAfter = 0;
    APIRET rc;

    if (!pszPath || hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;

    StrSetGetCount(hOut, &ulBefore);

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

    /* Sequential parse of OMF records without forced alignment. */
    while (cbPos + 3 <= (size_t)lFileSize) {
        UCHAR uchRecordType = puchBuf[cbPos];
        size_t cbRecordLen;
        size_t cbDataStart;
        size_t cbDataLen;

        cbPos++;
        cbRecordLen = read_u16(&puchBuf[cbPos]);
        cbPos += 2;

        if (cbPos + cbRecordLen > (size_t)lFileSize) {
            break;
        }
        cbDataStart = cbPos;
        cbDataLen = cbRecordLen;
        cbPos += cbRecordLen;

        switch (uchRecordType) {
            case 0x80: /* THEADR */
                rc = process_theadr(&puchBuf[cbDataStart], cbDataLen, hOut);
                if (rc != NO_ERROR) { free(puchBuf); return rc; }
                break;
            case 0x88: /* COMENT */
                rc = process_coment(&puchBuf[cbDataStart], cbDataLen, hOut);
                if (rc != NO_ERROR) { free(puchBuf); return rc; }
                break;
            default:
                break;
        }
    }

    free(puchBuf);

    StrSetGetCount(hOut, &ulAfter);
    if (ulAfter == ulBefore) return ERROR_FILE_NOT_FOUND;
    return NO_ERROR;
}
