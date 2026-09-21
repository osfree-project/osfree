/*! omf_write.c - OMF record generation (C89)
 *
 *  Appends complete OMF records to a file opened by OmfOpen with
 *  OMF_OPEN_WRITE. See omf.h for the public API and omf.c for the
 *  extraction side.
 *
 *  Every record is emitted with a checksum computed over the entire
 *  record (header and payload) so that the byte-wise sum of the
 *  complete record is zero modulo 256.
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
#include <string.h>
#include "omf.h"
#include "omf_private.h"

/*! @file omf_write.c
 *  @brief Implementation of OMF record generation.
 *
 *  All record generation functions declared in omf.h are implemented
 *  here. The internal record layout follows the 16-bit OMF variant
 *  (even record type bytes), which is what the OS/2 BIND tool and
 *  WLINK produce and consume for the temporary import object.
 */

/* ------------------------------------------------------------------ */
/* Static helpers                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Begin a record: write header and start the checksum sum.
 *
 *  Writes the 3-byte OMF record header [type:1][length:2] in
 *  little-endian order and initializes @p psum with the sum of those
 *  three bytes. All subsequent payload bytes must be written through
 *  omf_put, omf_putb or omf_putw so that the running sum stays
 *  correct.
 *
 *  @param[in]  fp        Output file. Not NULL.
 *  @param[in]  uchType   Record type (OMF_TYPE_*).
 *  @param[in]  usLength  Payload length, excluding the header.
 *  @param[out] psum      Receives the initial checksum sum.
 *
 *  @return APIRET
 *  @retval NO_ERROR          Success.
 *  @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET omf_begin(FILE *fp, UCHAR uchType, USHORT usLength,
                        unsigned int *psum)
{
    UCHAR auchHdr[3];

    auchHdr[0] = uchType;
    auchHdr[1] = (UCHAR)(usLength & 0xFF);
    auchHdr[2] = (UCHAR)((usLength >> 8) & 0xFF);
    if (fwrite(auchHdr, 1, 3, fp) != 3) return ERROR_WRITE_FAULT;
    *psum = (unsigned int)auchHdr[0] + auchHdr[1] + auchHdr[2];
    return NO_ERROR;
}

/*! @brief Write @p cbLen bytes and accumulate their sum.
 *
 *  @param[in]  fp     Output file. Not NULL.
 *  @param[in]  pv     Source bytes. May be NULL if @p cbLen is 0.
 *  @param[in]  cbLen  Number of bytes.
 *  @param[out] psum   Running checksum sum. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  @p pv is NULL with non-zero
 *                                   length.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
static APIRET omf_put(FILE *fp, const void *pv, ULONG cbLen,
                      unsigned int *psum)
{
    const UCHAR *p = (const UCHAR *)pv;
    ULONG i;

    if (cbLen == 0) return NO_ERROR;
    if (!pv) return ERROR_INVALID_PARAMETER;
    if (fwrite(pv, 1, cbLen, fp) != cbLen) return ERROR_WRITE_FAULT;
    for (i = 0; i < cbLen; i++) *psum += p[i];
    return NO_ERROR;
}

/*! @brief Write the checksum byte that makes the record sum zero.
 *
 *  @param[in] fp   Output file. Not NULL.
 *  @param[in] sum  Running sum from prior omf_put* calls.
 *
 *  @return APIRET
 *  @retval NO_ERROR          Success.
 *  @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET omf_end(FILE *fp, unsigned int sum)
{
    UCHAR uchCs = (UCHAR)(-(int)sum);

    if (fwrite(&uchCs, 1, 1, fp) != 1) return ERROR_WRITE_FAULT;
    return NO_ERROR;
}

/*! @brief Write a single byte with sum tracking.
 *
 *  @param[in]  fp    Output file. Not NULL.
 *  @param[in]  b     Byte value.
 *  @param[out] psum  Running checksum sum. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR          Success.
 *  @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET omf_putb(FILE *fp, UCHAR b, unsigned int *psum)
{
    if (fwrite(&b, 1, 1, fp) != 1) return ERROR_WRITE_FAULT;
    *psum += b;
    return NO_ERROR;
}

/*! @brief Write a little-endian 16-bit value with sum tracking.
 *
 *  @param[in]  fp    Output file. Not NULL.
 *  @param[in]  w     Value.
 *  @param[out] psum  Running checksum sum. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR          Success.
 *  @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET omf_putw(FILE *fp, USHORT w, unsigned int *psum)
{
    UCHAR auch[2];

    auch[0] = (UCHAR)(w & 0xFF);
    auch[1] = (UCHAR)((w >> 8) & 0xFF);
    if (fwrite(auch, 1, 2, fp) != 2) return ERROR_WRITE_FAULT;
    *psum += auch[0] + auch[1];
    return NO_ERROR;
}

/*! @brief Write a name index in 1 or 2 bytes.
 *
 *  Indexes below 128 are written as a single byte. Larger indexes
 *  are written as two bytes in little-endian order. This matches
 *  the compact form produced by WLINK for OMF-16 records, where
 *  name indexes commonly fit in one byte.
 *
 *  @param[in]  fp       Output file. Not NULL.
 *  @param[in]  usIndex  Index value.
 *  @param[out] psum     Running checksum sum. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR          Success.
 *  @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET omf_put_index(FILE *fp, USHORT usIndex,
                            unsigned int *psum)
{
    if (usIndex < 0x80) return omf_putb(fp, (UCHAR)usIndex, psum);
    return omf_putw(fp, usIndex, psum);
}

/*! @brief Length in bytes of a name index as written by
 *         omf_put_index.
 *
 *  @param[in] usIndex  Index value.
 *
 *  @return 1 if the index is below 128, otherwise 2.
 */
static USHORT omf_index_len(USHORT usIndex)
{
    return (usIndex < 0x80) ? 1 : 2;
}

/*! @brief Write one FIXUP subrecord.
 *
 *  Serializes a single FIXUP subrecord according to the bits of its
 *  FixDat byte:
 *    - bit 7 (F): when set, a frame datum byte follows.
 *    - bits 6-4 : frame method (not interpreted here).
 *    - bit 3 (T): when set, a target displacement follows.
 *    - bit 2 (P): 0 = 2-byte displacement, 1 = 1-byte.
 *    - bits 1-0: target method; 0/1 = 2-byte segment or group index,
 *      2 = external index written in 1 byte if it fits, otherwise in
 *      2 bytes.
 *
 *  The Locat field is written high byte first, matching the byte
 *  order produced by the historical BIND tool for WLINK.
 *
 *  @param[in]  fp    Output file. Not NULL.
 *  @param[in]  pFix  Fixup subrecord. Not NULL.
 *  @param[out] psum  Running checksum sum. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR          Success.
 *  @retval ERROR_WRITE_FAULT Write error.
 */
static APIRET omf_write_fixup(FILE *fp, const OMF_FIXUPP_FIXUP *pFix,
                              unsigned int *psum)
{
    APIRET rc;

    /* Locat: high byte first. */
    rc = omf_putb(fp, (UCHAR)((pFix->usLocat >> 8) & 0xFF), psum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, (UCHAR)(pFix->usLocat & 0xFF), psum);
    if (rc != NO_ERROR) return rc;

    /* FixDat. */
    rc = omf_putb(fp, pFix->uchFixDat, psum);
    if (rc != NO_ERROR) return rc;

    /* Optional frame datum. */
    if (pFix->uchFixDat & OMF_FIXDAT_F_FRAME_PRESENT) {
        rc = omf_putb(fp, pFix->uchFrameDatum, psum);
        if (rc != NO_ERROR) return rc;
    }

    /* Target datum. */
    switch (pFix->uchFixDat & OMF_FIXDAT_TARGET_MASK) {
    case OMF_FIXDAT_TARGET_SEGDEF:
    case OMF_FIXDAT_TARGET_GRPDEF:
        rc = omf_putw(fp, pFix->usTargetDatum, psum);
        if (rc != NO_ERROR) return rc;
        break;
    case OMF_FIXDAT_TARGET_EXTDEF:
    default:
        if (pFix->usTargetDatum < 0x80) {
            rc = omf_putb(fp, (UCHAR)pFix->usTargetDatum, psum);
            if (rc != NO_ERROR) return rc;
        } else {
            rc = omf_putb(fp,
                (UCHAR)(0x80 | (pFix->usTargetDatum & 0x7F)), psum);
            if (rc != NO_ERROR) return rc;
            rc = omf_putb(fp,
                (UCHAR)((pFix->usTargetDatum >> 7) & 0xFF), psum);
            if (rc != NO_ERROR) return rc;
        }
        break;
    }

    /* Optional target displacement. */
    if (pFix->uchFixDat & OMF_FIXDAT_T_DISP_PRESENT) {
        if (pFix->uchFixDat & OMF_FIXDAT_P_8BIT) {
            rc = omf_putb(fp, (UCHAR)pFix->usTargetDisp, psum);
            if (rc != NO_ERROR) return rc;
        } else {
            rc = omf_putw(fp, pFix->usTargetDisp, psum);
            if (rc != NO_ERROR) return rc;
        }
    }
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API - raw record header                                      */
/* ------------------------------------------------------------------ */

/*! @brief Append a raw OMF record header (type + length).
 *
 *  Writes [type:1][length:2] in little-endian order. Does not append
 *  a checksum. Used internally by the other OmfWrite* functions and
 *  available for callers that need to emit a custom record.
 *
 *  @param[in] hFile     Handle from OmfOpen. Not NULLHANDLE, and
 *                       opened with OMF_OPEN_WRITE.
 *  @param[in] uchType   Record type (OMF_TYPE_*).
 *  @param[in] usLength  Payload length, excluding this 3-byte header.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or wrong mode.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteRecordHeader(HOMFFILE hFile, UCHAR uchType,
                            USHORT usLength)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    UCHAR auchHdr[3];

    if (!fp) return ERROR_INVALID_PARAMETER;
    auchHdr[0] = uchType;
    auchHdr[1] = (UCHAR)(usLength & 0xFF);
    auchHdr[2] = (UCHAR)((usLength >> 8) & 0xFF);
    if (fwrite(auchHdr, 1, 3, fp) != 3) return ERROR_WRITE_FAULT;
    return NO_ERROR;
}

/* ------------------------------------------------------------------ */
/* Public API - THEADR                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a THEADR record (0x80).
 *
 *  Layout: [name_len:1][name:name_len][checksum:1].
 *
 *  @param[in] hFile    Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] pszName  Module name, 1..255 chars. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle, NULL name, or name
 *                                   length out of range.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteTheadr(HOMFFILE hFile, PCSZ pszName)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    ULONG cbName;
    USHORT usLen;
    unsigned int sum;
    APIRET rc;

    if (!fp || !pszName) return ERROR_INVALID_PARAMETER;
    cbName = (ULONG)strlen(pszName);
    if (cbName == 0 || cbName > 255) return ERROR_INVALID_PARAMETER;

    usLen = (USHORT)(cbName + 2);
    rc = omf_begin(fp, OMF_TYPE_THEADR, usLen, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, (UCHAR)cbName, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put(fp, pszName, cbName, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - COMENT raw and semantic helpers                        */
/* ------------------------------------------------------------------ */

/*! @brief Append a COMENT record (0x88) with raw payload.
 *
 *  Prefer the semantic helpers below (OmfWriteComentCodeView,
 *  OmfWriteComentWatcomOptions, OmfWriteComentBorlandDependency,
 *  OmfWriteComentOptimizeFarCall, OmfWriteComentDefaultLibrary)
 *  unless the caller needs to emit a vendor-specific class with a
 *  payload whose format is not covered by the library.
 *
 *  Layout: [comment_type:1][comment_class:1][payload][checksum:1].
 *
 *  @param[in] hFile         Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] uchType       Comment type (OMF_COMENT_TYPE_*).
 *  @param[in] uchClass      Comment class (OMF_COMENT_CLASS_* or
 *                           vendor-specific).
 *  @param[in] puchPayload   Payload bytes; NULL only if
 *                           @p usPayloadLen is 0.
 *  @param[in] usPayloadLen  Payload length.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL payload with
 *                                   non-zero length.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComent(HOMFFILE hFile, UCHAR uchType, UCHAR uchClass,
                      const UCHAR *puchPayload, USHORT usPayloadLen)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    APIRET rc;

    if (!fp) return ERROR_INVALID_PARAMETER;
    if (usPayloadLen > 0 && !puchPayload)
        return ERROR_INVALID_PARAMETER;

    usLen = (USHORT)(usPayloadLen + 3);
    rc = omf_begin(fp, OMF_TYPE_COMENT, usLen, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchType, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchClass, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put(fp, puchPayload, usPayloadLen, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}

/*! @brief Append a CodeView COMENT (class 0xA1) with no payload.
 *
 *  Emits a no-echo COMENT whose class is
 *  OMF_COMENT_CLASS_CODEVIEW and whose payload is empty, matching
 *  the record produced by WLINK when building the temporary import
 *  object.
 *
 *  @param[in] hFile  Handle from OmfOpen. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentCodeView(HOMFFILE hFile)
{
    return OmfWriteComent(hFile, OMF_COMENT_TYPE_NOECHO,
                          OMF_COMENT_CLASS_CODEVIEW, NULL, 0);
}

/*! @brief Append a Watcom options COMENT (class 0x9B).
 *
 *  The payload is the vendor-specific Watcom option blob supplied by
 *  the caller. The exact byte sequence is defined by the Watcom
 *  toolchain; the library only adds the record header and checksum.
 *
 *  @param[in] hFile    Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] puchOpt  Option bytes. Not NULL.
 *  @param[in] usLen    Option bytes length; must be greater than 0.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle, NULL payload, or
 *                                   zero length.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentWatcomOptions(HOMFFILE hFile,
                                   const UCHAR *puchOpt, USHORT usLen)
{
    if (!puchOpt || usLen == 0) return ERROR_INVALID_PARAMETER;
    return OmfWriteComent(hFile, OMF_COMENT_TYPE_NOECHO,
                          OMF_COMENT_CLASS_WATCOM_OPT,
                          puchOpt, usLen);
}

/*! @brief Append a Borland auto-dependency COMENT (class 0xE9).
 *
 *  The payload consists of a 4-byte timestamp followed by one or
 *  more [len:1][name:len] entries. This helper writes a single
 *  entry. If @p pszFileName is NULL, the record carries no payload
 *  at all, matching the empty variant emitted by WLINK.
 *
 *  The timestamp value is fixed at the byte sequence historically
 *  produced by the BIND tool; its content is a DOS date/time blob
 *  whose exact value does not affect WLINK.
 *
 *  @param[in] hFile        Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] pszFileName  File name, or NULL for an empty record.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle, or the file name is
 *                                   empty or longer than 255 bytes.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentBorlandDependency(HOMFFILE hFile,
                                       PCSZ pszFileName)
{
    UCHAR auchPayload[260];
    UCHAR *p;
    ULONG cbName;

    /* Fixed 4-byte timestamp, matching the historical BIND output. */
    static const UCHAR auchTimestamp[4] = { 0x8C, 0x4C, 0x50, 0x59 };

    if (pszFileName == NULL) {
        return OmfWriteComent(hFile, OMF_COMENT_TYPE_NOECHO,
                              OMF_COMENT_CLASS_BORLAND_DEP,
                              NULL, 0);
    }

    cbName = (ULONG)strlen(pszFileName);
    if (cbName == 0 || cbName > 255) return ERROR_INVALID_PARAMETER;

    p = auchPayload;
    memcpy(p, auchTimestamp, sizeof(auchTimestamp));
    p += sizeof(auchTimestamp);
    *p++ = (UCHAR)cbName;
    memcpy(p, pszFileName, cbName);
    p += cbName;

    return OmfWriteComent(hFile, OMF_COMENT_TYPE_NOECHO,
                          OMF_COMENT_CLASS_BORLAND_DEP,
                          auchPayload, (USHORT)(p - auchPayload));
}

/*! @brief Append an optimize-far-call COMENT (class 0xFE).
 *
 *  The payload is the two-byte sequence historically produced by
 *  WLINK. The high bit of @p uchFlags is OR'd into the second
 *  payload byte; its exact meaning is defined by WLINK and is not
 *  interpreted by this library.
 *
 *  @param[in] hFile     Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] uchFlags  Flags byte. The high bit (0x80) is preserved
 *                       in the payload; semantics are defined by
 *                       WLINK.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentOptimizeFarCall(HOMFFILE hFile, UCHAR uchFlags)
{
    /* Payload layout historically used by WLINK: {0x4F, 0x01}. */
    static const UCHAR uchPayloadFirst = 0x4F;
    static const UCHAR uchPayloadSecond = 0x01;
    UCHAR auchOpt[2];

    auchOpt[0] = uchPayloadFirst;
    auchOpt[1] = (UCHAR)(uchPayloadSecond | (uchFlags & 0x80));

    return OmfWriteComent(hFile, OMF_COMENT_TYPE_NOECHO,
                          OMF_COMENT_CLASS_OPT_FAR_CALL,
                          auchOpt, 2);
}

/*! @brief Append a default-library COMENT (class 0x9F).
 *
 *  Records the name of a library that the linker should include by
 *  default, as produced by the INCLUDELIB directive. The payload is
 *  exactly the library name, without a length prefix.
 *
 *  @param[in] hFile       Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] pszLibName  Library name, 1..255 chars. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL/empty name.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentDefaultLibrary(HOMFFILE hFile, PCSZ pszLibName)
{
    ULONG cbName;

    if (!pszLibName) return ERROR_INVALID_PARAMETER;
    cbName = (ULONG)strlen(pszLibName);
    if (cbName == 0 || cbName > 255) return ERROR_INVALID_PARAMETER;

    return OmfWriteComent(hFile, OMF_COMENT_TYPE_NOECHO,
                          OMF_COMENT_CLASS_DEFAULT_LIB,
                          (const UCHAR *)pszLibName, (USHORT)cbName);
}

/* ------------------------------------------------------------------ */
/* Public API - EXTDEF                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append an EXTDEF record (0x8C) with one or more names.
 *
 *  Layout of each entry: [name_len:1][name:name_len][type:1]. The
 *  Type Index is written in its one-byte form (value 0), matching
 *  the record produced by WLINK. The record is terminated by a
 *  checksum byte.
 *
 *  @param[in] hFile      Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] ppszNames  Array of external names. Not NULL if
 *                        @p cNames is greater than 0.
 *  @param[in] cNames     Number of names; must be greater than 0.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle, NULL array, zero
 *                                   count, NULL/empty/oversized name.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteExtdef(HOMFFILE hFile,
                      const char * const *ppszNames,
                      unsigned int cNames)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    unsigned int i;
    APIRET rc;
    ULONG cb;

    if (!fp || !ppszNames || cNames == 0)
        return ERROR_INVALID_PARAMETER;

    usLen = 1;
    for (i = 0; i < cNames; i++) {
        if (!ppszNames[i]) return ERROR_INVALID_PARAMETER;
        cb = (ULONG)strlen(ppszNames[i]);
        if (cb == 0 || cb > 255) return ERROR_INVALID_PARAMETER;
        usLen = (USHORT)(usLen + 1 + cb + 1);
    }

    rc = omf_begin(fp, OMF_TYPE_EXTDEF, usLen, &sum);
    if (rc != NO_ERROR) return rc;

    for (i = 0; i < cNames; i++) {
        cb = (ULONG)strlen(ppszNames[i]);
        rc = omf_putb(fp, (UCHAR)cb, &sum);
        if (rc != NO_ERROR) return rc;
        rc = omf_put(fp, ppszNames[i], cb, &sum);
        if (rc != NO_ERROR) return rc;
        /* Type Index = 0, one-byte form. */
        rc = omf_putb(fp, 0x00, &sum);
        if (rc != NO_ERROR) return rc;
    }
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - LEDATA                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a LEDATA record (0xA0).
 *
 *  Layout: [segment_index:2][offset:2][data][checksum:1].
 *
 *  @param[in] hFile           Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] usSegmentIndex  Segment index (1-based).
 *  @param[in] usOffset        Offset within the segment.
 *  @param[in] puchData        Data bytes; NULL only if @p usDataLen
 *                             is 0.
 *  @param[in] usDataLen       Data length.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL payload with
 *                                   non-zero length.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteLedata(HOMFFILE hFile, USHORT usSegmentIndex,
                      USHORT usOffset,
                      const UCHAR *puchData, USHORT usDataLen)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    APIRET rc;

    if (!fp) return ERROR_INVALID_PARAMETER;
    if (usDataLen > 0 && !puchData) return ERROR_INVALID_PARAMETER;

    usLen = (USHORT)(usDataLen + 5);
    rc = omf_begin(fp, OMF_TYPE_LEDATA, usLen, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putw(fp, usSegmentIndex, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putw(fp, usOffset, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put(fp, puchData, usDataLen, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - FIXUPP                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a FIXUPP record (0x9C) from an array of fixups.
 *
 *  Layout: [thread/fixup subrecords][checksum:1]. The caller
 *  supplies the already-assembled subrecords in the form of an
 *  array of OMF_FIXUPP_FIXUP structures; this function serializes
 *  each one and appends the checksum byte.
 *
 *  The payload length is computed from the FixDat flags of each
 *  subrecord, so the caller does not need to pre-compute it.
 *
 *  @param[in] hFile     Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] paFixups  Array of fixup subrecords. Not NULL.
 *  @param[in] cFixups   Number of subrecords; must be greater than 0.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle, NULL array, or zero
 *                                   count.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteFixupp(HOMFFILE hFile,
                      const OMF_FIXUPP_FIXUP *paFixups,
                      unsigned int cFixups)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    unsigned int i;
    APIRET rc;

    if (!fp || !paFixups || cFixups == 0)
        return ERROR_INVALID_PARAMETER;

    usLen = 1;
    for (i = 0; i < cFixups; i++) {
        usLen = (USHORT)(usLen + 2 + 1);
        if (paFixups[i].uchFixDat & OMF_FIXDAT_F_FRAME_PRESENT)
            usLen++;
        switch (paFixups[i].uchFixDat & OMF_FIXDAT_TARGET_MASK) {
        case OMF_FIXDAT_TARGET_SEGDEF:
        case OMF_FIXDAT_TARGET_GRPDEF:
            usLen += 2;
            break;
        case OMF_FIXDAT_TARGET_EXTDEF:
        default:
            usLen += (paFixups[i].usTargetDatum < 0x80) ? 1 : 2;
            break;
        }
        if (paFixups[i].uchFixDat & OMF_FIXDAT_T_DISP_PRESENT) {
            usLen += (paFixups[i].uchFixDat & OMF_FIXDAT_P_8BIT) ? 1 : 2;
        }
    }

    rc = omf_begin(fp, OMF_TYPE_FIXUPP, usLen, &sum);
    if (rc != NO_ERROR) return rc;

    for (i = 0; i < cFixups; i++) {
        rc = omf_write_fixup(fp, &paFixups[i], &sum);
        if (rc != NO_ERROR) return rc;
    }
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - MODEND                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a MODEND record (0x8A) without start address.
 *
 *  Layout: [module_type:1][checksum:1].
 *
 *  @param[in] hFile          Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] uchModuleType  Module type byte (OMF_MODEND_*).
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteModend(HOMFFILE hFile, UCHAR uchModuleType)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    unsigned int sum;
    APIRET rc;

    if (!fp) return ERROR_INVALID_PARAMETER;
    rc = omf_begin(fp, OMF_TYPE_MODEND, 2, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchModuleType, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - LNAMES                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append an LNAMES record (0x96).
 *
 *  Layout: [len:1][name:len]...[checksum:1]. Names are added in
 *  order and become visible to later records at 1-based indices
 *  matching their position in this record.
 *
 *  @param[in] hFile      Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] ppszNames  Array of names. Not NULL if @p cNames is
 *                        greater than 0.
 *  @param[in] cNames     Number of names.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle, NULL array with
 *                                   non-zero count, NULL or oversized
 *                                   name.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteLnames(HOMFFILE hFile,
                      const char * const *ppszNames,
                      unsigned int cNames)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    unsigned int i;
    APIRET rc;
    ULONG cb;

    if (!fp) return ERROR_INVALID_PARAMETER;
    if (cNames > 0 && !ppszNames) return ERROR_INVALID_PARAMETER;

    usLen = 1;
    for (i = 0; i < cNames; i++) {
        if (!ppszNames[i]) return ERROR_INVALID_PARAMETER;
        cb = (ULONG)strlen(ppszNames[i]);
        if (cb > 255) return ERROR_INVALID_PARAMETER;
        usLen = (USHORT)(usLen + 1 + cb);
    }

    rc = omf_begin(fp, OMF_TYPE_LNAMES, usLen, &sum);
    if (rc != NO_ERROR) return rc;

    for (i = 0; i < cNames; i++) {
        cb = (ULONG)strlen(ppszNames[i]);
        rc = omf_putb(fp, (UCHAR)cb, &sum);
        if (rc != NO_ERROR) return rc;
        rc = omf_put(fp, ppszNames[i], cb, &sum);
        if (rc != NO_ERROR) return rc;
    }
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - SEGDEF                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a SEGDEF record (0x98).
 *
 *  Name indexes are written in one byte if their value is less than
 *  128, otherwise in two bytes with the high bit of the first byte
 *  set. This matches the compact form produced by WLINK for OMF-16
 *  records.
 *
 *  @param[in] hFile             Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] uchACBP           ACBP byte (OMF_ACBP_*).
 *  @param[in] usLength          Segment length in bytes.
 *  @param[in] usSegmentNameIdx  LNAMES index of the segment name.
 *  @param[in] usClassNameIdx    LNAMES index of the class name.
 *  @param[in] usOverlayNameIdx  LNAMES index of the overlay name,
 *                               or 0.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteSegdef(HOMFFILE hFile, UCHAR uchACBP,
                      USHORT usLength,
                      USHORT usSegmentNameIdx,
                      USHORT usClassNameIdx,
                      USHORT usOverlayNameIdx)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    APIRET rc;

    if (!fp) return ERROR_INVALID_PARAMETER;

    /* 1 acbp + 2 length + 1 checksum + three name indexes. */
    usLen = 4;
    usLen = (USHORT)(usLen + omf_index_len(usSegmentNameIdx));
    usLen = (USHORT)(usLen + omf_index_len(usClassNameIdx));
    usLen = (USHORT)(usLen + omf_index_len(usOverlayNameIdx));

    rc = omf_begin(fp, OMF_TYPE_SEGDEF, usLen, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchACBP, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putw(fp, usLength, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put_index(fp, usSegmentNameIdx, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put_index(fp, usClassNameIdx, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put_index(fp, usOverlayNameIdx, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - GRPDEF                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a GRPDEF record (0x9A).
 *
 *  Layout: [group_name:1][members...][checksum:1]. The member bytes
 *  are caller-supplied and must already be in their OMF form (each
 *  member is [type:1][index:1 or 2]).
 *
 *  @param[in] hFile         Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] uchGroupName  LNAMES index of the group name.
 *  @param[in] puchMembers   Member bytes; NULL only if
 *                           @p usMembersLen is 0.
 *  @param[in] usMembersLen  Member bytes length.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL payload with
 *                                   non-zero length.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteGrpdef(HOMFFILE hFile, UCHAR uchGroupName,
                      const UCHAR *puchMembers,
                      USHORT usMembersLen)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    USHORT usLen;
    unsigned int sum;
    APIRET rc;

    if (!fp) return ERROR_INVALID_PARAMETER;
    if (usMembersLen > 0 && !puchMembers) return ERROR_INVALID_PARAMETER;

    usLen = (USHORT)(usMembersLen + 2);
    rc = omf_begin(fp, OMF_TYPE_GRPDEF, usLen, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchGroupName, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put(fp, puchMembers, usMembersLen, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}

/* ------------------------------------------------------------------ */
/* Public API - PUBDEF                                                 */
/* ------------------------------------------------------------------ */

/*! @brief Append a PUBDEF record (0x90) with one public name.
 *
 *  Layout: [group:1][segment:1][name_len:1][name:name_len]
 *          [offset:2][type:1][checksum:1]. One-byte group and
 *          segment indices and a one-byte (zero) type index are
 *          used, matching the records produced by the OS/2 BIND
 *          tool.
 *
 *  @param[in] hFile       Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] uchGroup    Group index (0 if none).
 *  @param[in] uchSegment  Segment index.
 *  @param[in] pszName     Public name, 1..255 chars. Not NULL.
 *  @param[in] usOffset    Offset within the segment.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL/empty name.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWritePubdef(HOMFFILE hFile, UCHAR uchGroup,
                      UCHAR uchSegment, PCSZ pszName,
                      USHORT usOffset)
{
    FILE *fp = omf_get_fp(hFile, OMF_OPEN_WRITE);
    ULONG cbName;
    USHORT usLen;
    unsigned int sum;
    APIRET rc;

    if (!fp || !pszName) return ERROR_INVALID_PARAMETER;
    cbName = (ULONG)strlen(pszName);
    if (cbName == 0 || cbName > 255) return ERROR_INVALID_PARAMETER;

    usLen = (USHORT)(cbName + 7);
    rc = omf_begin(fp, OMF_TYPE_PUBDEF, usLen, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchGroup, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, uchSegment, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, (UCHAR)cbName, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_put(fp, pszName, cbName, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putw(fp, usOffset, &sum);
    if (rc != NO_ERROR) return rc;
    rc = omf_putb(fp, 0x00, &sum);
    if (rc != NO_ERROR) return rc;
    return omf_end(fp, sum);
}
