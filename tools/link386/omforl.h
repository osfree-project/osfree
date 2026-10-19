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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/

/*!
 * @file omforl.h
 * @brief OMF entry-point prototypes implemented by the ORL interface.
 */

#ifndef OMF_ORL_INCLUDED
#define OMF_ORL_INCLUDED

#include "omftype.h"

/*!
 * @brief Create a new OMF handle.
 * @param[in] funcs  Allocator/reader callback table. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new OMF handle.
 */
omf_handle              OMFENTRY OmfInit( orl_funcs * );
/*!
 * @brief Destroy an OMF handle and all of its file handles.
 * @param[in,out] oh  OMF handle. Not NULL.
 * @retval ORL_OKAY   Success.
 * @return ORL_OKAY on success, or the first error from unlinking.
 */
orl_return              OMFENTRY OmfFini( omf_handle );

/*!
 * @brief Open an OMF file and parse its record structure.
 * @param[in,out] oh    OMF handle. Not NULL.
 * @param[in]     file  Caller's file object passed to the callbacks.
 * @retval NULL Allocation failure, or the file is not a valid OMF.
 * @return The new file handle.
 */
omf_file_handle         OMFENTRY OmfFileInit( omf_handle, void * );
/*!
 * @brief Close an OMF file handle and free it.
 * @param[in,out] ofh  OMF file handle. Not NULL.
 * @return Result of OmfRemoveFileLinks().
 */
orl_return              OMFENTRY OmfFileFini( omf_file_handle );
/*!
 * @brief Scan sections (or sections matching a name) of an OMF file.
 * @param[in,out] ofh    OMF file handle. Not NULL.
 * @param[in]     desired  Desired section name, or NULL for all.
 * @param[in]     func     Callback invoked for each matching section.
 * @retval ORL_OKAY   Scan completed.
 * @retval ORL_ERROR  Callback aborted, or hash table could not be built.
 * @return ORL_OKAY on success, or the callback's error code.
 */
orl_return              OMFENTRY OmfFileScan( omf_file_handle, char *, orl_sec_return_func );
/*!
 * @brief Get the machine type of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The machine type.
 */
orl_machine_type        OMFENTRY OmfFileGetMachineType( omf_file_handle );
/*!
 * @brief Get the file flags of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The file flags.
 */
orl_file_flags          OMFENTRY OmfFileGetFlags( omf_file_handle );
/*!
 * @brief Get the file type of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The file type.
 */
orl_file_type           OMFENTRY OmfFileGetType( omf_file_handle );
/*!
 * @brief Get the file size of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The file size in bytes.
 */
orl_file_size           OMFENTRY OmfFileGetSize( omf_file_handle );
/*!
 * @brief Get the symbol table section of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The symbol table section handle, or NULL.
 */
omf_sec_handle          OMFENTRY OmfFileGetSymbolTable( omf_file_handle );

/*!
 * @brief Get the name of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Section is not a segment type.
 * @return The NUL-terminated section name.
 */
char *                  OMFENTRY OmfSecGetName( omf_sec_handle );
/*!
 * @brief Get the base offset of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @return Always 0 (unused for OMF).
 */
orl_sec_offset          OMFENTRY OmfSecGetBase( omf_sec_handle );
/*!
 * @brief Get the declared size of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section size in bytes.
 */
orl_sec_size            OMFENTRY OmfSecGetSize( omf_sec_handle );
/*!
 * @brief Get the type of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section type.
 */
orl_sec_type            OMFENTRY OmfSecGetType( omf_sec_handle );
/*!
 * @brief Get the flags of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section flags.
 */
orl_sec_flags           OMFENTRY OmfSecGetFlags( omf_sec_handle );
/*!
 * @brief Get the alignment of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval 0 Non-segment section.
 * @return Log2 of the alignment in bytes.
 */
orl_sec_alignment       OMFENTRY OmfSecGetAlignment( omf_sec_handle );
/*!
 * @brief Get the string table section linked to a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL The section is not a string table.
 * @return The string table section handle.
 */
omf_sec_handle          OMFENTRY OmfSecGetStringTable( omf_sec_handle );
/*!
 * @brief Get the symbol table section that owns a symbol table section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The owning symbol table section handle.
 */
omf_sec_handle          OMFENTRY OmfSecGetSymbolTable( omf_sec_handle );
/*!
 * @brief Get the reloc table section that owns a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL The section is not a program-bits section.
 * @return The reloc section handle.
 */
omf_sec_handle          OMFENTRY OmfSecGetRelocTable( omf_sec_handle );
/*!
 * @brief Get the non-local symbol location of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The non-local symbol location.
 */
orl_sec_offset          OMFENTRY OmfSecGetNonLocalSymbolLoc( omf_sec_handle );
/*!
 * @brief Get the contents buffer of a section.
 * @param[in,out] sh      Section handle. Not NULL.
 * @param[out]    buffer  Receiver for the contents pointer. Not NULL.
 * @retval ORL_OKAY   Contents available.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return ORL_OKAY on success, ORL_ERROR otherwise.
 */
orl_return              OMFENTRY OmfSecGetContents( omf_sec_handle, char ** );
/*!
 * @brief Scan relocations at a specific offset in a section.
 * @param[in] sh      Section handle. Not NULL.
 * @param[in] offset  Offset to match.
 * @param[in] func    Callback invoked for each matching relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result or ORL_FALSE.
 */
orl_return              OMFENTRY OmfSecQueryReloc( omf_sec_handle, omf_sec_offset, orl_reloc_return_func );
/*!
 * @brief Scan all relocations of a section.
 * @param[in] sh    Section handle. Not NULL.
 * @param[in] func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result or ORL_FALSE.
 */
orl_return              OMFENTRY OmfSecScanReloc( omf_sec_handle, orl_reloc_return_func );
/*!
 * @brief Get the class name of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Non-segment section.
 * @return Pointer to the NUL-terminated class name.
 */
char *                  OMFENTRY OmfSecGetClassName( omf_sec_handle );
/*!
 * @brief Get the combine attribute of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval ORL_SEC_COMBINE_NONE Non-segment section.
 * @return The ORL combine value.
 */
orl_sec_combine         OMFENTRY OmfSecGetCombine( omf_sec_handle );
/*!
 * @brief Get the absolute frame of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval ORL_SEC_NO_ABS_FRAME Non-segment section.
 * @return The absolute frame value.
 */
orl_sec_frame           OMFENTRY OmfSecGetAbsFrame( omf_sec_handle );
/*!
 * @brief Get the section associated with a COMDAT.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Not a COMDAT.
 * @return The associated section, or NULL.
 */
orl_sec_handle          OMFENTRY OmfSecGetAssociated( omf_sec_handle );
/*!
 * @brief Get the group of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Not grouped.
 * @return The group handle, or NULL.
 */
orl_group_handle        OMFENTRY OmfSecGetGroup( omf_sec_handle );
/*!
 * @brief Get the number of line numbers attached to a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval 0 Non-segment section.
 * @return The number of line numbers.
 */
orl_table_index         OMFENTRY OmfSecGetNumLines( omf_sec_handle );
/*!
 * @brief Get the line-number array of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Non-segment section.
 * @return Pointer to the line-number array.
 */
orl_linnum *            OMFENTRY OmfSecGetLines( omf_sec_handle );


/*!
 * @brief Convert a section handle to its OMF index.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section index.
 */
orl_table_index         OMFENTRY OmfCvtSecHdlToIdx( omf_sec_handle );
/*!
 * @brief Convert an OMF index to a section handle.
 * @param[in] ofh  File handle. Not NULL.
 * @param[in] idx  Section index.
 * @retval NULL Index not found.
 * @return The matching section handle.
 */
omf_sec_handle          OMFENTRY OmfCvtIdxToSecHdl( omf_file_handle,
                                                     orl_table_index );

/*!
 * @brief Scan a reloc section's relocation entries.
 * @param[in] sh    Reloc section handle. Not NULL.
 * @param[in] func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a reloc section.
 * @return Callback result or ORL_FALSE.
 */
orl_return              OMFENTRY OmfRelocSecScan( omf_sec_handle, orl_reloc_return_func );
/*!
 * @brief Scan a symbol table section's symbols.
 * @param[in] sh    Symbol table section handle. Not NULL.
 * @param[in] func  Callback invoked for each symbol.
 * @retval ORL_ERROR  Section is not a symbol table.
 * @return Callback result.
 */
orl_return              OMFENTRY OmfSymbolSecScan( omf_sec_handle, orl_symbol_return_func );

/*!
 * @brief Get the name of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return NUL-terminated symbol name.
 */
char *                  OMFENTRY OmfSymbolGetName( omf_symbol_handle );
/*!
 * @brief Get the value (offset) of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The symbol value.
 */
orl_symbol_value        OMFENTRY OmfSymbolGetValue( omf_symbol_handle );
/*!
 * @brief Get the binding of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The symbol binding.
 */
orl_symbol_binding      OMFENTRY OmfSymbolGetBinding( omf_symbol_handle );
/*!
 * @brief Get the type flags of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The symbol type flags.
 */
orl_symbol_type         OMFENTRY OmfSymbolGetType( omf_symbol_handle );
/*!
 * @brief Get the raw symbol info byte.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return Always 0 (not applicable to OMF).
 */
unsigned char           OMFENTRY OmfSymbolGetRawInfo( omf_symbol_handle );
/*!
 * @brief Get the section that contains a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The containing section handle, or NULL.
 */
omf_sec_handle          OMFENTRY OmfSymbolGetSecHandle( omf_symbol_handle );

/*!
 * @brief Scan a note section's comment records.
 * @param[in] hnd     Note section handle. Not NULL.
 * @param[in] cb      Note callback table. Not NULL.
 * @param[in] cookie  Callback cookie.
 * @retval ORL_ERROR  Section is not a note section.
 * @return Result of OmfParseComments().
 */
orl_return              OMFENTRY OmfNoteSecScan( omf_sec_handle,
                                                 orl_note_callbacks *, void * );

/*!
 * @brief Scan all groups of a file.
 * @param[in] hnd   File handle. Not NULL.
 * @param[in] func  Callback invoked for each group.
 * @retval ORL_OKAY  Scan completed.
 * @return Callback result.
 */
orl_return              OMFENTRY OmfGroupsScan( omf_file_handle,
                                                orl_group_return_func );
/*!
 * @brief Get the number of members of a group.
 * @param[in] hnd  Group handle. Not NULL.
 * @return The number of member segments.
 */
orl_table_index         OMFENTRY OmfGroupSize( omf_grp_handle );
/*!
 * @brief Get the name of a group.
 * @param[in] hnd  Group handle. Not NULL.
 * @return NUL-terminated group name.
 */
char *                  OMFENTRY OmfGroupName( omf_grp_handle );
/*!
 * @brief Get the name of a group member.
 * @param[in] hnd  Group handle. Not NULL.
 * @param[in] idx  Member index (zero-based).
 * @retval NULL Index out of range or member could not be resolved.
 * @return NUL-terminated member name.
 */
char *                  OMFENTRY OmfGroupMember( omf_grp_handle,
                                                 orl_table_index );

#endif
