/*! omf.c - OMF file open/close and source extraction (C89)
 *
 *  OMF (Relocatable Object Module Format) records scanned for source
 *  names:
 *    - THEADR (0x80) - module name, usually the original source file.
 *    - COMENT (0x88) - comment records with vendor-specific classes
 *      carrying dependency file lists.
 *
 *  Vendor-specific COMENT classes recognized:
 *    - 0xE9 - Borland auto-dependency: timestamp (4 bytes) followed
 *      by repeated [len][name] entries.
 *    - 0xFB - OpenWatcom auto-dependency: same layout as 0xE9.
 *    - 0x88 - additional dependency class found in OpenWatcom output.
 *    - 0xE8 - Borland source file name: single [len][name] entry.
 *
 *  References:
 *    - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *      Object Module Format), Linux Foundation.
 *      https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *    - JWasm / OpenWatcom WASM sources (COMENT class usage).
 *    - Microsoft OMF specification, "Relocatable Object Module
 *      Format", version 1.1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omf.h"
#include "omf_private.h"

/*! @file omf.c
 *  @brief Implementation of the OMF file open/close and source
 *         extractor.
 *
 *  Provides OmfOpen / OmfClose for opening an OMF file with a given
 *  access and action mode, and OmfQuerySources for extracting source
 *  file names from an OMF object opened for reading.
 *
 *  @see OmfOpen
 *  @see OmfClose
 *  @see OmfQuerySources
 */

/*! @brief Maximum length of a single extracted name. */
#define OMF_NAME_MAX 512

/* ------------------------------------------------------------------ */
/* Private accessor                                                    */
/* ------------------------------------------------------------------ */

/*! @brief Fetch the stream pointer if the handle is valid and has
 *         the expected access mode.
 *
 *  The access mode is compared against the low two bits of
 *  OMFFILE::flMode. A handle opened with OMF_OPEN_READ does not
 *  satisfy a request for OMF_OPEN_WRITE, and vice versa.
 *
 *  @param[in] hFile       Handle from OmfOpen.
 *  @param[in] flExpected  Expected access mode: OMF_OPEN_READ or
 *                         OMF_OPEN_WRITE.
 *
 *  @return The underlying FILE pointer, or NULL if @p hFile is
 *          NULLHANDLE, the access mode does not match, or the
 *          stream is missing.
 *
 *  @see OmfOpen
 */
FILE *omf_get_fp(HOMFFILE hFile, ULONG flExpected)
{
    struct OMFFILE *pFile;

    if (hFile == NULLHANDLE) return NULL;
    pFile = (struct OMFFILE *)hFile;
    if ((pFile->flMode & 0x0003) != flExpected) return NULL;
    return pFile->fp;
}

/* ------------------------------------------------------------------ */
/* Open / close                                                        */
/* ------------------------------------------------------------------ */

/*! @brief Open an OMF file for reading or writing.
 *
 *  The access flag (OMF_OPEN_READ or OMF_OPEN_WRITE) must be combined
 *  with exactly one action flag (OMF_OPEN_EXISTING, OMF_OPEN_CREATE
 *  or OMF_OPEN_TRUNCATE). For reading, OMF_OPEN_EXISTING is the only
 *  valid action. For writing, use OMF_OPEN_CREATE or
 *  OMF_OPEN_TRUNCATE.
 *
 *  @param[in]  pszPath  File path. Not NULL.
 *  @param[out] phFile   Receives the handle. Not NULL. Set to
 *                       NULLHANDLE on failure.
 *  @param[in]  flOpen   Access and action flags (OMF_OPEN_*).
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phFile is NULL,
 *                                   or the flag combination is
 *                                   invalid.
 *  @retval ERROR_OPEN_FAILED        File cannot be opened.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 *  @see OmfClose
 */
APIRET APIENTRY OmfOpen(PCSZ pszPath, HOMFFILE *phFile,
                        ULONG flOpen)
{
    const char *pszMode;
    ULONG flAccess;
    ULONG flAction;
    FILE *fp;
    struct OMFFILE *pFile;

    if (!pszPath || !phFile) return ERROR_INVALID_PARAMETER;
    *phFile = NULLHANDLE;

    flAccess = flOpen & 0x0003;
    flAction = flOpen & 0x00F0;

    /* Exactly one access mode. */
    if (flAccess != OMF_OPEN_READ && flAccess != OMF_OPEN_WRITE)
        return ERROR_INVALID_PARAMETER;

    /* Exactly one action. */
    if (flAction != OMF_OPEN_EXISTING &&
        flAction != OMF_OPEN_CREATE &&
        flAction != OMF_OPEN_TRUNCATE)
        return ERROR_INVALID_PARAMETER;

    if (flAccess == OMF_OPEN_READ) {
        if (flAction != OMF_OPEN_EXISTING)
            return ERROR_INVALID_PARAMETER;
        pszMode = "rb";
    } else {
        if (flAction == OMF_OPEN_EXISTING)
            return ERROR_INVALID_PARAMETER;
        pszMode = "wb";
    }

    fp = fopen(pszPath, pszMode);
    if (!fp) return ERROR_OPEN_FAILED;

    pFile = (struct OMFFILE *)malloc(sizeof(*pFile));
    if (!pFile) {
        fclose(fp);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pFile->fp = fp;
    pFile->flMode = flOpen;
    *phFile = (HOMFFILE)pFile;
    return NO_ERROR;
}

/*! @brief Close an OMF file.
 *
 *  Flushes and closes the underlying stream and releases the handle.
 *  Idempotent: passing NULLHANDLE returns NO_ERROR.
 *
 *  @param[in] hFile  Handle from OmfOpen. NULLHANDLE is accepted.
 *
 *  @return APIRET
 *  @retval NO_ERROR  Always.
 *
 *  @see OmfOpen
 */
APIRET APIENTRY OmfClose(HOMFFILE hFile)
{
    struct OMFFILE *pFile;

    if (hFile == NULLHANDLE) return NO_ERROR;
    pFile = (struct OMFFILE *)hFile;
    if (pFile->fp) fclose(pFile->fp);
    free(pFile);
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Source extraction                                                   */
/* ------------------------------------------------------------------ */

/*! @brief Read a 2-byte little-endian unsigned integer.
 *
 *  Reads two bytes from @p puchPos and returns their value in
 *  little-endian order: @c puchPos[0] | (puchPos[1] << 8).
 *
 *  @param[in] puchPos  Pointer to two bytes. Not NULL.
 *
 *  @return Value in the range [0, 65535].
 */
static unsigned int omf_read_u16(const UCHAR *puchPos)
{
    return (unsigned int)puchPos[0] |
           ((unsigned int)puchPos[1] << 8);
}

/*! @brief Append a name of length @p cbLen to the destination set.
 *
 *  The name is copied into a temporary NUL-terminated buffer before
 *  being handed to StrSetAdd. Names longer than the internal buffer
 *  are rejected with ERROR_INVALID_PARAMETER.
 *
 *  @param[in] hOut     Destination set. Not NULLHANDLE.
 *  @param[in] puchStr  Source bytes. Not NULL.
 *  @param[in] cbLen    Number of bytes.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success (including cbLen == 0).
 *  @retval ERROR_INVALID_PARAMETER  @p cbLen exceeds the internal
 *                                   buffer size.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure inside
 *                                   StrSetAdd.
 */
static APIRET omf_add_name(HSTRSET hOut, const UCHAR *puchStr,
                           ULONG cbLen)
{
    CHAR achBuf[OMF_NAME_MAX];

    if (!puchStr || cbLen == 0) return NO_ERROR;
    if (cbLen >= sizeof(achBuf)) return ERROR_INVALID_PARAMETER;
    memcpy(achBuf, puchStr, cbLen);
    achBuf[cbLen] = '\0';
    return StrSetAdd(hOut, achBuf);
}

/*! @brief Process a THEADR record (0x80).
 *
 *  Layout: [name_len:1][name:name_len]. A zero-length name is
 *  silently ignored. A name that would extend past the payload end
 *  is also ignored (the record is treated as malformed). Malformed
 *  records are not reported as errors so that a single damaged
 *  record does not abort the whole extraction.
 *
 *  @param[in] puchData  Record payload. Not NULL.
 *  @param[in] cbLen     Payload length.
 *  @param[in] hOut      Destination set. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success (including malformed).
 *  @retval ERROR_INVALID_PARAMETER  Name too long for the internal
 *                                   buffer.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET omf_process_theadr(const UCHAR *puchData, ULONG cbLen,
                                 HSTRSET hOut)
{
    ULONG cbNameLen;

    if (cbLen < 1) return NO_ERROR;
    cbNameLen = puchData[0];
    if (1 + cbNameLen > cbLen) return NO_ERROR;
    if (cbNameLen > 0) {
        return omf_add_name(hOut, &puchData[1], cbNameLen);
    }
    return NO_ERROR;
}

/*! @brief Process a COMENT record (0x88).
 *
 *  Layout: [comment_type:1][comment_class:1][payload]. The payload
 *  layout depends on the comment class:
 *
 *    - OMF_COMENT_CLASS_BORLAND_DEP (0xE9) and
 *      OMF_COMENT_CLASS_WATCOM_DEP (0xFB): a 4-byte timestamp
 *      followed by repeated [len:1][name:len] entries.
 *    - OMF_COMENT_CLASS_EXTRA_DEP (0x88): a 4-byte timestamp
 *      followed by a single [len:1][name:len] entry.
 *    - OMF_COMENT_CLASS_BORLAND_SRC (0xE8): a single
 *      [len:1][name:len] entry without a timestamp.
 *
 *  Unknown classes are silently ignored. Malformed records are also
 *  ignored rather than treated as errors, so that a single damaged
 *  dependency list does not abort the whole extraction.
 *
 *  @param[in] puchData  Record payload. Not NULL.
 *  @param[in] cbLen     Payload length.
 *  @param[in] hOut      Destination set. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success (including malformed).
 *  @retval ERROR_INVALID_PARAMETER  A name is too long for the
 *                                   internal buffer.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET omf_process_coment(const UCHAR *puchData, ULONG cbLen,
                                 HSTRSET hOut)
{
    UCHAR uchCommentClass;
    ULONG cbPos;
    ULONG cbPayloadLen;

    if (cbLen < 2) return NO_ERROR;

    /* puchData[0] is the comment type; not used here. */
    uchCommentClass = puchData[1];
    cbPos = 2;
    cbPayloadLen = cbLen - 2;

    switch (uchCommentClass) {
    case OMF_COMENT_CLASS_BORLAND_DEP:
    case OMF_COMENT_CLASS_WATCOM_DEP: {
        APIRET rc;

        if (cbPayloadLen < 4) return NO_ERROR;
        cbPos += 4;   /* Skip the 4-byte timestamp. */

        while (cbPos + 1 <= cbLen) {
            ULONG cbStrLen = puchData[cbPos];

            cbPos++;
            if (cbStrLen == 0) continue;
            if (cbPos + cbStrLen > cbLen) break;
            rc = omf_add_name(hOut, &puchData[cbPos], cbStrLen);
            if (rc != NO_ERROR) return rc;
            cbPos += cbStrLen;
        }
        break;
    }

    case OMF_COMENT_CLASS_EXTRA_DEP: {
        if (cbPayloadLen < 5) return NO_ERROR;
        cbPos += 4; /* Skip the 4-byte timestamp. */
        if (cbPos < cbLen) {
            ULONG cbNameLen = puchData[cbPos];

            cbPos++;
            if (cbPos + cbNameLen <= cbLen && cbNameLen > 0) {
                return omf_add_name(hOut, &puchData[cbPos],
                                    cbNameLen);
            }
        }
        break;
    }

    case OMF_COMENT_CLASS_BORLAND_SRC: {
        if (cbPayloadLen < 1) return NO_ERROR;
        if (cbPos < cbLen) {
            ULONG cbNameLen = puchData[cbPos];

            cbPos++;
            if (cbPos + cbNameLen <= cbLen && cbNameLen > 0) {
                return omf_add_name(hOut, &puchData[cbPos],
                                    cbNameLen);
            }
        }
        break;
    }

    default:
        break;
    }
    return NO_ERROR;
}

/*! @brief Extract source file names from an OMF object file.
 *
 *  The handle must have been opened with OMF_OPEN_READ. The function
 *  reads the whole file into memory, walks it as a stream of OMF
 *  records, and appends every source name it recognizes to @p hOut:
 *
 *    - every THEADR record contributes its module name;
 *    - every COMENT record whose class is one of
 *      OMF_COMENT_CLASS_BORLAND_DEP, OMF_COMENT_CLASS_WATCOM_DEP,
 *      OMF_COMENT_CLASS_EXTRA_DEP or OMF_COMENT_CLASS_BORLAND_SRC
 *      contributes the names it carries.
 *
 *  Duplicates are handled by the set implementation; the same name
 *  may be appended twice without error.
 *
 *  If the file contains no recognizable source names at all, the
 *  function returns ERROR_FILE_NOT_FOUND. This allows callers to
 *  distinguish "processed but empty" from "processed and found
 *  names".
 *
 *  @param[in] hFile  Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] hOut   Destination string set. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 At least one name was added.
 *  @retval ERROR_INVALID_PARAMETER  @p hFile is NULLHANDLE or not
 *                                   opened for reading, or @p hOut is
 *                                   NULLHANDLE, or a name inside a
 *                                   THEADR/COMENT record is longer
 *                                   than the internal buffer.
 *  @retval ERROR_READ_FAULT         File size is zero, or read error.
 *  @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *  @retval ERROR_FILE_NOT_FOUND     No source names found.
 *
 *  @note The caller owns @p hOut and must release it with
 *        StrSetDestroy.
 *
 *  @see OmfOpen
 *  @see OmfWriteTheadr
 *  @see OmfWriteComent
 */
APIRET APIENTRY OmfQuerySources(HOMFFILE hFile, HSTRSET hOut)
{
    FILE *fp;
    UCHAR *puchBuf;
    long lFileSize;
    ULONG cbReadSize;
    ULONG cbPos = 0;
    ULONG ulBefore = 0;
    ULONG ulAfter = 0;
    APIRET rc;

    if (hOut == NULLHANDLE) return ERROR_INVALID_PARAMETER;
    fp = omf_get_fp(hFile, OMF_OPEN_READ);
    if (!fp) return ERROR_INVALID_PARAMETER;

    StrSetGetCount(hOut, &ulBefore);

    if (fseek(fp, 0, SEEK_END) != 0) return ERROR_READ_FAULT;
    lFileSize = ftell(fp);
    if (lFileSize <= 0) return ERROR_READ_FAULT;
    if (fseek(fp, 0, SEEK_SET) != 0) return ERROR_READ_FAULT;

    puchBuf = (UCHAR *)malloc((size_t)lFileSize);
    if (!puchBuf) return ERROR_NOT_ENOUGH_MEMORY;
    cbReadSize = (ULONG)fread(puchBuf, 1, (size_t)lFileSize, fp);
    if (cbReadSize != (ULONG)lFileSize) {
        free(puchBuf);
        return ERROR_READ_FAULT;
    }

    while (cbPos + 3 <= (ULONG)lFileSize) {
        UCHAR uchRecordType = puchBuf[cbPos];
        ULONG cbRecordLen;
        ULONG cbDataStart;
        ULONG cbDataLen;

        cbPos++;
        cbRecordLen = omf_read_u16(&puchBuf[cbPos]);
        cbPos += 2;

        if (cbPos + cbRecordLen > (ULONG)lFileSize) break;
        cbDataStart = cbPos;
        cbDataLen = cbRecordLen;
        cbPos += cbRecordLen;

        switch (uchRecordType) {
        case OMF_TYPE_THEADR:
            rc = omf_process_theadr(&puchBuf[cbDataStart],
                                    cbDataLen, hOut);
            if (rc != NO_ERROR) { free(puchBuf); return rc; }
            break;
        case OMF_TYPE_COMENT:
            rc = omf_process_coment(&puchBuf[cbDataStart],
                                    cbDataLen, hOut);
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
