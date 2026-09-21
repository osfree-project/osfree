/*! omf.h - OMF (Relocatable Object Module Format) records (C89)
 *
 *  Extraction of source file names and generation of OMF records.
 *
 *  OMF records scanned for source names:
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

#ifndef OMF_H
#define OMF_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file omf.h
 *  @brief OMF record definitions, source extraction and generation API.
 *
 *  The library covers two complementary tasks:
 *    - Extraction of source file names from an existing OMF object
 *      file or library, driven by THEADR and vendor-specific COMENT
 *      records.
 *    - Generation of complete OMF records for constructing object
 *      files programmatically, as required by the FamilyAPI Binder
 *      when synthesizing the temporary import object.
 *
 *  All public handles (HOMFFILE) are opaque. Opening, reading and
 *  writing follow the OS/2 handle-based convention modelled on
 *  DosOpen / DosClose; the access and action flags are described
 *  under OmfOpen.
 *
 *  References:
 *    - TIS Portable Formats Specification, Version 1.1 (Relocatable
 *      Object Module Format), Linux Foundation.
 *      https://refspecs.linuxfoundation.org/elf/elfspec.pdf
 *    - JWasm / OpenWatcom WASM sources (COMENT class usage).
 *    - Microsoft OMF specification, "Relocatable Object Module
 *      Format", version 1.1.
 *
 *  @see OmfOpen
 *  @see OmfQuerySources
 *  @see OmfWriteTheadr
 */

/*! @name OMF record types
 *
 *  Record type bytes for the 16-bit variants (even values). The
 *  32-bit variants (odd values: 0x81, 0x89, ...) are not currently
 *  produced or consumed by this library.
 */
/*! @{ */
#define OMF_TYPE_THEADR  0x80  /*!< Translator Header Record. */
#define OMF_TYPE_COMENT  0x88  /*!< Comment Record. */
#define OMF_TYPE_MODEND  0x8A  /*!< Module End Record. */
#define OMF_TYPE_EXTDEF  0x8C  /*!< External Names Definition Record. */
#define OMF_TYPE_PUBDEF  0x90  /*!< Public Names Definition Record. */
#define OMF_TYPE_LNAMES  0x96  /*!< List of Names Record. */
#define OMF_TYPE_SEGDEF  0x98  /*!< Segment Definition Record (16-bit). */
#define OMF_TYPE_GRPDEF  0x9A  /*!< Group Definition Record. */
#define OMF_TYPE_FIXUPP  0x9C  /*!< Fixup Record (16-bit). */
#define OMF_TYPE_LEDATA  0xA0  /*!< Logical Enumerated Data Record. */
/*! @} */

/*! @name COMENT comment types
 *
 *  The comment type byte distinguishes ordinary comments from
 *  no-echo comments that older linkers must not display.
 */
/*! @{ */
#define OMF_COMENT_TYPE_NORMAL  0x00  /*!< Normal comment. */
#define OMF_COMENT_TYPE_NOECHO  0x80  /*!< No-echo comment. */
/*! @} */

/*! @name COMENT comment classes
 *
 *  Comment classes recognised by the OMF source extractor and
 *  produced by the record generation functions.
 */
/*! @{ */
#define OMF_COMENT_CLASS_CODEVIEW      0xA1  /*!< CodeView debug info. */
#define OMF_COMENT_CLASS_WATCOM_OPT    0x9B  /*!< Watcom options. */
#define OMF_COMENT_CLASS_BORLAND_DEP   0xE9  /*!< Borland auto-dependency:
                                              *   timestamp + [len][name]*. */
#define OMF_COMENT_CLASS_WATCOM_DEP    0xFB  /*!< OpenWatcom dependency:
                                              *   same layout as 0xE9. */
#define OMF_COMENT_CLASS_EXTRA_DEP     0x88  /*!< Additional dependency
                                              *   class found in
                                              *   OpenWatcom output. */
#define OMF_COMENT_CLASS_BORLAND_SRC   0xE8  /*!< Borland source file
                                              *   name: [len][name]. */
#define OMF_COMENT_CLASS_OPT_FAR_CALL  0xFE  /*!< Optimize far call. */
#define OMF_COMENT_CLASS_DEFAULT_LIB   0x9F  /*!< Default library. */
/*! @} */

/*! @name SEGDEF ACBP bits
 *
 *  The ACBP byte combines Alignment (bits 7-5), Combination (bit 4),
 *  Big (bits 3-1) and the P bit (bit 0). Watcom sets the Big bit on
 *  32-bit code segments even when the segment length fits in 16
 *  bits.
 */
/*! @{ */
#define OMF_ACBP_ALIGN_ABS      0x00  /*!< Absolute. */
#define OMF_ACBP_ALIGN_BYTE     0x20  /*!< Byte aligned. */
#define OMF_ACBP_ALIGN_WORD     0x40  /*!< Word aligned. */
#define OMF_ACBP_ALIGN_PARA     0x60  /*!< Paragraph aligned. */
#define OMF_ACBP_ALIGN_PAGE     0x80  /*!< Page aligned. */
#define OMF_ACBP_ALIGN_DWORD    0xA0  /*!< Dword aligned. */
#define OMF_ACBP_COMBINE_PRIV   0x00  /*!< Private. */
#define OMF_ACBP_COMBINE_PUBLIC 0x10  /*!< Public. */
#define OMF_ACBP_BIG_BIT        0x08  /*!< Big bit as used by Watcom. */
/*! @} */

/*! @name GRPDEF member types */
/*! @{ */
#define OMF_GRPDEF_MEMBER_SEGMENT  0xFF  /*!< Member is a segment. */
#define OMF_GRPDEF_MEMBER_GROUP    0xFE  /*!< Member is a group. */
/*! @} */

/*! @name MODEND module type bits */
/*! @{ */
#define OMF_MODEND_MAIN    0x80  /*!< Module is the main module. */
#define OMF_MODEND_START   0x40  /*!< Start address present. */
/*! @} */

/*! @name FIXUPP FixDat bits
 *
 *  The FixDat byte encodes the F bit (frame datum present), the
 *  frame method, the T bit, the P bit and the target method.
 */
/*! @{ */
#define OMF_FIXDAT_F_FRAME_PRESENT  0x80  /*!< Frame datum present. */
#define OMF_FIXDAT_FRAME_MASK       0x70  /*!< Frame method mask. */
#define OMF_FIXDAT_FRAME_SHIFT      4     /*!< Frame method shift. */
#define OMF_FIXDAT_T_DISP_PRESENT   0x08  /*!< Target disp present. */
#define OMF_FIXDAT_P_8BIT           0x04  /*!< 8-bit target disp. */
#define OMF_FIXDAT_TARGET_MASK      0x03  /*!< Target method mask. */
/*! @} */

/*! @name FIXUPP frame methods
 *
 *  Values for the frame method field of the FixDat byte.
 */
/*! @{ */
#define OMF_FIXDAT_FRAME_SEGDEF     0x00  /*!< Frame is a SEGDEF. */
#define OMF_FIXDAT_FRAME_GRPDEF     0x10  /*!< Frame is a GRPDEF. */
#define OMF_FIXDAT_FRAME_EXTDEF     0x20  /*!< Frame is an EXTDEF. */
#define OMF_FIXDAT_FRAME_LOCATION   0x40  /*!< Frame is a location. */
#define OMF_FIXDAT_FRAME_TARGET     0x50  /*!< Frame is the target. */
#define OMF_FIXDAT_FRAME_NONE       0x60  /*!< No frame. */
/*! @} */

/*! @name FIXUPP target methods
 *
 *  Values for the target method field of the FixDat byte.
 */
/*! @{ */
#define OMF_FIXDAT_TARGET_SEGDEF    0x00  /*!< Target is a SEGDEF. */
#define OMF_FIXDAT_TARGET_GRPDEF    0x01  /*!< Target is a GRPDEF. */
#define OMF_FIXDAT_TARGET_EXTDEF    0x02  /*!< Target is an EXTDEF. */
#define OMF_FIXDAT_TARGET_ABSOLUTE  0x03  /*!< Target is absolute. */
/*! @} */

/*! @name OmfOpen flags
 *
 *  Modelled after the OS/2 DosOpen OpenFlag and OpenMode parameters.
 *  The access flags (bits 0-1) are mutually exclusive; exactly one
 *  action flag (bits 4-5) must be combined with them.
 *
 *  For reading, only OMF_OPEN_EXISTING is valid.
 *  For writing, use OMF_OPEN_CREATE or OMF_OPEN_TRUNCATE.
 */
/*! @{ */
#define OMF_OPEN_READ       0x0001  /*!< Open for reading only. */
#define OMF_OPEN_WRITE      0x0002  /*!< Open for writing only. */
#define OMF_OPEN_EXISTING   0x0000  /*!< Open existing file; fail if
                                     *   it does not exist. */
#define OMF_OPEN_CREATE     0x0010  /*!< Create if it does not exist. */
#define OMF_OPEN_TRUNCATE   0x0020  /*!< Truncate if it exists. */
/*! @} */

#pragma pack(push, 1)

/*! @brief Common OMF record header.
 *
 *  Every OMF record starts with this 3-byte header. The payload
 *  length does not include the header itself.
 */
typedef struct _OMF_RECORD_HEADER {
    UCHAR  uchType;   /*!< Record type (see OMF_TYPE_* constants). */
    USHORT usLength;  /*!< Payload length, excluding this header. */
} OMF_RECORD_HEADER;

/*! @brief THEADR (0x80) - Translator Header Record.
 *
 *  Payload layout: [name_len:1][name:name_len]. The name is not
 *  NUL-terminated on disk.
 */
typedef struct _OMF_THEADR {
    UCHAR uchNameLen;  /*!< Length of the module name. */
} OMF_THEADR;

/*! @brief COMENT (0x88) - Comment Record.
 *
 *  Payload layout: [comment_type:1][comment_class:1][payload]. The
 *  payload layout depends on the comment class; see the file header
 *  for the list of recognized classes.
 */
typedef struct _OMF_COMENT {
    UCHAR uchCommentType;   /*!< Comment type (OMF_COMENT_TYPE_*). */
    UCHAR uchCommentClass;  /*!< Comment class (OMF_COMENT_CLASS_*). */
} OMF_COMENT;

/*! @brief EXTDEF (0x8C) - External Names Definition Record.
 *
 *  Payload is a sequence of entries:
 *  [name_len:1][name:name_len][type_index:2][reserved:1].
 */
typedef struct _OMF_EXTDEF {
    UCHAR uchNameLen;  /*!< Length of the external name. */
} OMF_EXTDEF;

/*! @brief LEDATA (0xA0) - Logical Enumerated Data Record.
 *
 *  Payload layout: [segment_index:2][offset:2][data].
 */
typedef struct _OMF_LEDATA {
    USHORT usSegmentIndex;  /*!< Segment index (1-based). */
    USHORT usOffset;        /*!< Offset within the segment. */
} OMF_LEDATA;

/*! @brief MODEND (0x8A) - Module End Record.
 *
 *  Payload layout: [module_type:1][optional end data][checksum:1].
 *  MODEND must be the last record in an object module.
 */
typedef struct _OMF_MODEND {
    UCHAR uchModuleType;  /*!< Module type flags (OMF_MODEND_*). */
} OMF_MODEND;

/*! @brief LNAMES (0x96) - List of Names Record.
 *
 *  Payload is a sequence of [len:1][name:len] entries. Names are
 *  1-indexed from the start of the record.
 */
typedef struct _OMF_LNAMES {
    UCHAR uchFirst;  /*!< First length byte. */
} OMF_LNAMES;

/*! @brief SEGDEF (0x98) - Segment Definition Record.
 *
 *  Name indexes are written in one byte if their value is less than
 *  128, otherwise in two bytes with the high bit of the first byte
 *  set. This matches the compact form produced by WLINK for the
 *  temporary object file.
 */
typedef struct _OMF_SEGDEF {
    UCHAR  uchACBP;             /*!< ACBP byte (OMF_ACBP_*). */
    USHORT usLength;            /*!< Segment length in bytes. */
    USHORT usSegmentNameIndex;  /*!< Index into LNAMES. */
    USHORT usClassNameIndex;    /*!< Index into LNAMES. */
    USHORT usOverlayNameIndex;  /*!< Index into LNAMES, or 0. */
} OMF_SEGDEF;

/*! @brief GRPDEF (0x9A) - Group Definition Record.
 *
 *  Payload layout: [group_name:1][components...], where each
 *  component is [type:1][index:1 or 2].
 */
typedef struct _OMF_GRPDEF {
    UCHAR uchGroupNameIndex;  /*!< Index into LNAMES. */
} OMF_GRPDEF;

/*! @brief PUBDEF (0x90) - Public Names Definition Record.
 *
 *  Payload layout: [group_index:1 or 2][segment_index:1 or 2]
 *  [name_len:1][name:name_len][offset:2][type_index:1 or 2].
 */
typedef struct _OMF_PUBDEF {
    USHORT usGroupIndex;    /*!< Group index, or 0. */
    USHORT usSegmentIndex;  /*!< Segment index. */
} OMF_PUBDEF;

/*! @brief One FIXUP subrecord of a FIXUPP record.
 *
 *  The Locat field encodes whether the subrecord is a FIXUP (bit 15
 *  set) or THREAD (bit 15 clear), together with the location type
 *  and the offset of the byte to be fixed up in the last data
 *  record.
 *
 *  FixDat encodes the F bit (frame datum present), the frame method,
 *  the T bit, the P bit and the target method. See OMF_FIXDAT_*.
 */
typedef struct _OMF_FIXUPP_FIXUP {
    USHORT usLocat;         /*!< Locat field. */
    UCHAR  uchFixDat;       /*!< FixDat byte (OMF_FIXDAT_*). */
    UCHAR  uchFrameDatum;   /*!< Frame datum (only if F bit set). */
    USHORT usTargetDatum;   /*!< Target datum index. */
    USHORT usTargetDisp;    /*!< Target displacement. */
} OMF_FIXUPP_FIXUP;

#pragma pack(pop)

/*! @brief Handle to an open OMF file.
 *
 *  Opaque. Created by OmfOpen, released by OmfClose. The internal
 *  representation is private to omf_private.h.
 *
 *  @see OmfOpen
 *  @see OmfClose
 */
typedef HANDLE HOMFFILE;

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
                        ULONG flOpen);

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
APIRET APIENTRY OmfClose(HOMFFILE hFile);

/* ------------------------------------------------------------------ */
/* Source extraction                                                   */
/* ------------------------------------------------------------------ */

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
APIRET APIENTRY OmfQuerySources(HOMFFILE hFile, HSTRSET hOut);

/* ------------------------------------------------------------------ */
/* Record generation                                                   */
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
                            USHORT usLength);

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
APIRET OmfWriteTheadr(HOMFFILE hFile, PCSZ pszName);

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
                      const UCHAR *puchPayload, USHORT usPayloadLen);

/*! @brief Append a CodeView COMENT (class 0xA1) with no payload.
 *
 *  @param[in] hFile  Handle from OmfOpen. Not NULLHANDLE.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentCodeView(HOMFFILE hFile);

/*! @brief Append a Watcom options COMENT (class 0x9B).
 *
 *  The payload is the vendor-specific Watcom option blob supplied by
 *  the caller.
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
                                   const UCHAR *puchOpt, USHORT usLen);

/*! @brief Append a Borland auto-dependency COMENT (class 0xE9).
 *
 *  The payload consists of a 4-byte timestamp followed by one or
 *  more [len:1][name:len] entries. This helper writes a single
 *  entry. If @p pszFileName is NULL, the record carries no payload
 *  at all, matching the empty variant emitted by WLINK.
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
                                       PCSZ pszFileName);

/*! @brief Append an optimize-far-call COMENT (class 0xFE).
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
APIRET OmfWriteComentOptimizeFarCall(HOMFFILE hFile, UCHAR uchFlags);

/*! @brief Append a default-library COMENT (class 0x9F).
 *
 *  Records the name of a library that the linker should include by
 *  default, as produced by the INCLUDELIB directive.
 *
 *  @param[in] hFile       Handle from OmfOpen. Not NULLHANDLE.
 *  @param[in] pszLibName  Library name, 1..255 chars. Not NULL.
 *
 *  @return APIRET
 *  @retval NO_ERROR                 Success.
 *  @retval ERROR_INVALID_PARAMETER  Bad handle or NULL/empty name.
 *  @retval ERROR_WRITE_FAULT        Write error.
 */
APIRET OmfWriteComentDefaultLibrary(HOMFFILE hFile, PCSZ pszLibName);

/*! @brief Append an EXTDEF record (0x8C) with one or more names.
 *
 *  Layout of each entry: [name_len:1][name:name_len][type:1]. The
 *  Type Index is written in its one-byte form (value 0). The record
 *  is terminated by a checksum byte.
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
                      unsigned int cNames);

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
                      const UCHAR *puchData, USHORT usDataLen);

/*! @brief Append a FIXUPP record (0x9C) from an array of fixups.
 *
 *  Layout: [thread/fixup subrecords][checksum:1]. The caller
 *  supplies the already-assembled subrecords in the form of an
 *  array of OMF_FIXUPP_FIXUP structures; this function serializes
 *  each one and appends the checksum byte.
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
                      unsigned int cFixups);

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
APIRET OmfWriteModend(HOMFFILE hFile, UCHAR uchModuleType);

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
                      unsigned int cNames);

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
                      USHORT usOverlayNameIdx);

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
                      USHORT usMembersLen);

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
                      USHORT usOffset);

#ifdef __cplusplus
}
#endif

#endif /* OMF_H */
