/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  OMF munging public routines.
*
****************************************************************************/

/*!
 * @file omfmunge.h
 * @brief Public entry points for OMF record munging.
 */

#ifndef OMF_MUNGE_INCLUDED
#define OMF_MUNGE_INCLUDED

#include "omfglbl.h"
#include "omftype.h"

/*!
 * @brief Add a deferred LIDATA record to the current file.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     is32    Non-zero for a 32-bit record.
 * @param[in]     seg     Segment or COMDAT index. Non-zero.
 * @param[in]     offset  Offset within the section.
 * @param[in]     buffer  LIDATA payload. Not NULL.
 * @param[in]     len     Payload length.
 * @param[in]     comdat  Non-zero when @p seg indexes a COMDAT.
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Malformed payload or section lookup failed.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddLIData( omf_file_handle ofh, int is32,
                                      omf_idx seg, orl_sec_offset offset,
                                      omf_bytes buffer, long len, int comdat );

/*!
 * @brief Add a LEDATA record to the current file.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     is32    Non-zero for a 32-bit record (unused).
 * @param[in]     seg     Segment or COMDAT index. Non-zero.
 * @param[in]     offset  Offset within the section.
 * @param[in]     buffer  LEDATA payload. Not NULL.
 * @param[in]     len     Payload length.
 * @param[in]     comdat  Non-zero when @p seg indexes a COMDAT.
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Section lookup failed or payload too large.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddLEData( omf_file_handle ofh, int is32,
                                      omf_idx seg, orl_sec_offset offset,
                                      omf_bytes buffer, long len, int comdat );

/*!
 * @brief Add an LNAME record.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     buffer  LNAME payload. Not NULL.
 * @param[in]     len     Payload length.
 * @param[in]     typ     Record type (unused).
 * @retval ORL_OKAY            Added.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddLName( omf_file_handle ofh, omf_bytes buffer,
                                     unsigned int len, omf_rectyp typ );

/*!
 * @brief Add one FIXUPP subrecord.
 * @param[in,out] ofh       OMF file handle. Not NULL.
 * @param[in]     is32      Non-zero for a 32-bit record.
 * @param[in]     mode      Segment-relative (1) or self-relative (0).
 * @param[in]     location  OMF location specifier.
 * @param[in]     offset    Offset within the section.
 * @param[in]     fmethod   Frame method.
 * @param[in]     fidx      Frame index.
 * @param[in]     tmethod   Target method.
 * @param[in]     tidx      Target index.
 * @param[in]     disp      Displacement.
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Unknown location or target/frame lookup failed.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddFixupp( omf_file_handle ofh, int is32, int mode,
                                      int location, orl_sec_offset offset,
                                      int fmethod, omf_idx fidx, int tmethod,
                                      omf_idx tidx, orl_sec_offset disp );

/*!
 * @brief Add a BAKPAT/NBKPAT record to the deferred backpatch queue.
 * @param[in,out] ofh        OMF file handle. Not NULL.
 * @param[in]     loctype    Location type.
 * @param[in]     location   Offset within the target section.
 * @param[in]     segidx     Target segment index (BAKPAT), or zero.
 * @param[in]     symidx     Target COMDAT name index (NBKPAT), or zero.
 * @param[in]     disp       Displacement to apply.
 * @retval ORL_OKAY            Queued.
 * @retval ORL_ERROR           Unknown location type.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddBakpat( omf_file_handle ofh, uint_8 loctype,
                                      orl_sec_offset location, omf_idx segidx,
                                      omf_idx symidx, orl_sec_offset disp );

/*!
 * @brief Add an EXTDEF/LEXTDEF/LEXTDEF32/COMDEF/LCOMDEF record.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     buffer  Name bytes. Not NULL.
 * @param[in]     len     Name length in bytes.
 * @param[in]     typ     Record type.
 * @retval ORL_OKAY            Added.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddExtDef( omf_file_handle ofh, omf_bytes buffer,
                                      unsigned int len, omf_rectyp typ );

/*!
 * @brief Add a COMDAT/COMDAT32 record.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     is32    Non-zero for a 32-bit record.
 * @param[in]     flags   COMDAT flags byte.
 * @param[in]     attr    COMDAT attribute byte.
 * @param[in]     align   Alignment value, or -1 to inherit from SEGDEF.
 * @param[in]     offset  Offset within the COMDAT.
 * @param[in]     seg     Explicit allocation segment index, or zero.
 * @param[in]     group   Explicit allocation group index, or zero.
 * @param[in]     frame   Explicit allocation absolute frame, or zero.
 * @param[in]     name    COMDAT name index.
 * @param[in]     buffer  Payload. Not NULL.
 * @param[in]     len     Payload length.
 * @param[in]     typ     Record type (unused).
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Malformed record or lookup failed.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddComDat( omf_file_handle ofh, int is32, int flags,
                                      int attr, int align,
                                      orl_sec_offset offset, omf_idx seg,
                                      omf_idx group, omf_frame frame,
                                      omf_idx name, omf_bytes buffer, long len,
                                      omf_rectyp typ );

/*!
 * @brief Add a SEGDEF/SEGDEF32 record.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     is32    Non-zero for a 32-bit record.
 * @param[in]     align   Alignment.
 * @param[in]     combine Combine code.
 * @param[in]     use32   Non-zero if the segment uses 32-bit addressing.
 * @param[in]     max     Non-zero if the segment is exactly 64K / 4G.
 * @param[in]     frame   Absolute frame, or ORL_SEC_NO_ABS_FRAME.
 * @param[in]     size    Declared size in bytes.
 * @param[in]     name    Segment name index.
 * @param[in]     class   Segment class index.
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Lookup failed.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddSegDef( omf_file_handle ofh, int is32,
                                      orl_sec_alignment align, int combine,
                                      int use32, int max, orl_sec_frame frame,
                                      orl_sec_size size, omf_idx name,
                                      omf_idx class );

/*!
 * @brief Add a PUBDEF/PUBDEF32/LPUBDEF/LPUBDEF32 record.
 * @param[in,out] ofh     OMF file handle. Not NULL.
 * @param[in]     is32    Non-zero for a 32-bit record (unused).
 * @param[in]     group   Group index, or zero.
 * @param[in]     seg     Segment index, or zero for absolute.
 * @param[in]     frame   Absolute frame, or zero.
 * @param[in]     name    Symbol name. Not NULL.
 * @param[in]     len     Name length in bytes.
 * @param[in]     offset  Symbol offset.
 * @param[in]     typ     Record type.
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Unsupported combination.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddPubDef( omf_file_handle ofh, int is32,
                                      omf_idx group, omf_idx seg,
                                      omf_frame frame, char *name, int len,
                                      orl_sec_offset offset, omf_rectyp typ );

/*!
 * @brief Add a GRPDEF record.
 * @param[in,out] ofh   OMF file handle. Not NULL.
 * @param[in]     name  Group name index. Non-zero.
 * @param[in]     segs  Array of segment indices. Not NULL.
 * @param[in]     size  Number of entries in @p segs.
 * @retval ORL_OKAY            Added.
 * @retval ORL_ERROR           Lookup failed.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddGrpDef( omf_file_handle ofh, omf_idx name,
                                      omf_idx *segs, int size );

/*!
 * @brief Finish work deferred by a MODEND record.
 * @param[in,out] ofh  OMF file handle. Not NULL.
 * @retval ORL_OKAY   Success.
 * @retval ORL_ERROR  Malformed record.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfModEnd( omf_file_handle ofh );

/*!
 * @brief Add a COMENT record.
 * @param[in,out] ofh    OMF file handle. Not NULL.
 * @param[in]     class  Comment class.
 * @param[in]     flags  Comment flags byte.
 * @param[in]     buff   Comment payload. Not NULL.
 * @param[in]     len    Payload length.
 * @retval ORL_OKAY            Added.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddComment( omf_file_handle ofh, uint_8 class,
                                       uint_8 flags, omf_bytes buff, long len );

/*!
 * @brief Add one LINNUM record to a section.
 * @param[in,out] sh      Section. Not NULL.
 * @param[in]     line    Line number.
 * @param[in]     offset  Offset within the section.
 * @retval ORL_OKAY            Appended.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfAddLineNum( omf_sec_handle sh, unsigned_16 line,
                                       unsigned_32 offset );

/*!
 * @brief Find a segment or COMDAT section.
 * @param[in] ofh           OMF file handle. Not NULL.
 * @param[in] seg           Segment index, or zero for a COMDAT.
 * @param[in] comdat_lname  COMDAT name index when @p seg is zero.
 * @retval NULL Not found.
 * @return The matching section.
 */
extern omf_sec_handle   OmfFindSegOrComdat( omf_file_handle ofh, omf_idx seg,
                                            omf_idx comdat_lname );

/*!
 * @brief Copy an LNAME string into a caller-supplied buffer.
 * @param[in]  lnames  String table. Not NULL.
 * @param[in]  idx     One-based name index.
 * @param[out] name    Destination buffer. Not NULL.
 * @retval -1 Lookup failed.
 * @return Length of the name in bytes.
 */
extern int              OmfGetLName( omf_sec_handle lnames, omf_idx idx,
                                     char *name );
/*!
 * @brief Return a pointer to an LNAME string.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @param[in] idx  One-based name index.
 * @retval NULL Lookup failed.
 * @return Pointer to the NUL-terminated name.
 */
extern char             *OmfGetPtrToLName( omf_file_handle ofh, omf_idx idx );

/*!
 * @brief Ensure a section's contents buffer covers its declared size.
 * @param[in,out] sh  Section. Not NULL.
 * @retval ORL_OKAY            Buffer allocated.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfExportSegmentContents( omf_sec_handle sh );

/*!
 * @brief Record the source file name from a THEADR record.
 * @param[in,out] ofh  OMF file handle. Not NULL.
 * @retval ORL_OKAY   Recorded.
 * @retval ORL_ERROR  Malformed record.
 * @return ORL_OKAY on success.
 */
extern orl_return       OmfTheadr( omf_file_handle ofh );

#endif
