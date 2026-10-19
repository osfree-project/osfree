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
 * @file omfentr.c
 * @brief OMF entry points for the Object Reader Library.
 *
 * Implements the OMF-specific implementation of the ORL entry
 * points: file/segment/symbol/group accessors over the internal OMF
 * handle structures built by omfload.c and omfmunge.c.
 */

#include <assert.h>

#include "omfentr.h"
#include "omfload.h"
#include "omfmunge.h"
#include "omfflhn.h"
#include "orlhash.h"
#include "omfdrctv.h"

/*!
 * @def _IsSegType
 * @brief True for section types that carry a segment descriptor.
 * @param[in] t  Section type to test.
 */
#define _IsSegType( t )         ( ( t == ORL_SEC_TYPE_PROG_BITS ) || \
                                  ( t == ORL_SEC_TYPE_NO_BITS ) )

/*!
 * @brief Create a new OMF handle.
 * @param[in] funcs  Allocator/reader callback table. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new OMF handle.
 */
omf_handle OMFENTRY OmfInit( orl_funcs * funcs )
{
    omf_handle                                  oh;

    assert( funcs );

    oh = funcs->alloc( sizeof( omf_handle_struct ) );
    if( !oh ) return( NULL );
    oh->funcs = funcs;
    oh->first_file_hnd = NULL;
    return( oh );
}


/*!
 * @brief Destroy an OMF handle and all of its file handles.
 * @param[in,out] oh  OMF handle. Not NULL.
 * @retval ORL_OKAY   Success.
 * @return ORL_OKAY on success, or the first error from unlinking.
 */
orl_return OMFENTRY OmfFini( omf_handle oh )
{
    orl_return                                  err;

    assert( oh );

    while( oh->first_file_hnd != NULL ) {
        err = OmfRemoveFileLinks( oh->first_file_hnd );
        if( err != ORL_OKAY ) return( err );
    }
    oh->funcs->free( oh );
    return( ORL_OKAY );
}


/*!
 * @brief Open an OMF file and parse its record structure.
 * @param[in,out] oh    OMF handle. Not NULL.
 * @param[in]     file  Caller's file object passed to the callbacks.
 * @retval NULL Allocation failure, or the file is not a valid OMF.
 * @return The new file handle.
 */
omf_file_handle OMFENTRY OmfFileInit( omf_handle oh, void *file )
{
    omf_file_handle                             ofh;

    assert( oh );

    ofh = oh->funcs->alloc( sizeof( omf_file_handle_struct ) );
    if( !ofh ) return( NULL );

    memset( ofh, 0, sizeof( omf_file_handle_struct ) );
    ofh->file = file;

    OmfAddFileLinks( oh, ofh );
    if( OmfLoadFileStructure( ofh ) != ORL_OKAY ) {
        OmfRemoveFileLinks( ofh );
        return( NULL );
    }
    return( ofh );
}


/*!
 * @brief Close an OMF file handle and free it.
 * @param[in,out] ofh  OMF file handle. Not NULL.
 * @return Result of OmfRemoveFileLinks().
 */
orl_return OMFENTRY OmfFileFini( omf_file_handle ofh )
{
    assert( ofh );

    return( OmfRemoveFileLinks( ofh ) );
}


/*!
 * @brief Scan sections (or sections matching a name) of an OMF file.
 * @param[in,out] ofh    OMF file handle. Not NULL.
 * @param[in]     desired  Desired section name, or NULL for all.
 * @param[in]     func     Callback invoked for each matching section.
 * @retval ORL_OKAY   Scan completed.
 * @retval ORL_ERROR  Callback aborted, or hash table could not be built.
 * @return ORL_OKAY on success, or the callback's error code.
 */
orl_return OMFENTRY OmfFileScan( omf_file_handle ofh, char *desired,
                                 orl_sec_return_func func )
{
    orl_hash_data_struct                *ds;
    omf_sec_handle                      sh;
    omf_symbol_handle                   sym;
    orl_return                          err;

    assert( ofh );
    assert( func );

    if( desired == NULL ) {
        /* global request */
        sh = ofh->first_sec;
        while( sh ) {
            // string tables are internal sections which are not seen by
            // the user code
            if( sh->type != ORL_SEC_TYPE_STR_TABLE ) {
                err = func( (orl_sec_handle)sh );
                if( err != ORL_OKAY ) return( err );
            }
            sh = sh->next;
        }
    } else if( ofh->symbol_table ) {
        assert( ofh->symbol_table->assoc.sym.hash_tab );
        ds = ORLHashTableQuery( ofh->symbol_table->assoc.sym.hash_tab,
                                (orl_hash_value) desired );
        while( ds != NULL ) {
            sym = ds->data;
            if( ( sym->typ == ORL_SYM_TYPE_SECTION ) &&
               !( sym->flags & OMF_SYM_FLAGS_GRPDEF ) ) {
                err = func( (orl_sec_handle) sym->section );
                if( err != ORL_OKAY ) return( err );
            }
            ds = ds->next;
        }
    }
    return( ORL_OKAY );
}


/*!
 * @brief Get the machine type of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The machine type.
 */
orl_machine_type OMFENTRY OmfFileGetMachineType( omf_file_handle ofh )
{
    assert( ofh );

    return( ofh->machine_type );
}


/*!
 * @brief Get the file flags of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The file flags.
 */
orl_file_flags OMFENTRY OmfFileGetFlags( omf_file_handle ofh )
{
    assert( ofh );

    return( ofh->flags );
}


/*!
 * @brief Get the file type of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The file type.
 */
orl_file_type OMFENTRY OmfFileGetType( omf_file_handle ofh )
{
    assert( ofh );

    return( ofh->type );
}


/*!
 * @brief Get the file size of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The file size in bytes.
 */
orl_file_size OMFENTRY OmfFileGetSize( omf_file_handle ofh )
{
    assert( ofh );

    return( ofh->size );
}


/*!
 * @brief Get the symbol table section of an OMF file.
 * @param[in] ofh  OMF file handle. Not NULL.
 * @return The symbol table section handle, or NULL.
 */
omf_sec_handle OMFENTRY OmfFileGetSymbolTable( omf_file_handle ofh )
{
    assert( ofh );

    return( ofh->symbol_table );
}


/*!
 * @brief Get the name of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Section is not a segment type.
 * @return The NUL-terminated section name.
 */
char * OMFENTRY OmfSecGetName( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) ) {
        assert( sh->assoc.seg.sym );
        return( sh->assoc.seg.sym->name );
    }
    return( NULL );
}


/*!
 * @brief Get the base offset of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @return Always 0 (unused for OMF).
 */
orl_sec_offset OMFENTRY OmfSecGetBase( omf_sec_handle sh )
{
    sh = sh;
    assert( sh );

    // Ask Jim
    return( 0 );
}


/*!
 * @brief Get the declared size of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section size in bytes.
 */
orl_sec_size OMFENTRY OmfSecGetSize( omf_sec_handle sh )
{
    assert( sh );

    return( sh->size );
}


/*!
 * @brief Get the type of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section type.
 */
orl_sec_type OMFENTRY OmfSecGetType( omf_sec_handle sh )
{
    assert( sh );

    return( sh->type );
}


/*!
 * @brief Get the flags of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section flags.
 */
orl_sec_flags OMFENTRY OmfSecGetFlags( omf_sec_handle sh )
{
    assert( sh );

    return( sh->flags );
}


/*!
 * @brief Get the alignment of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval 0 Non-segment section.
 * @return Log2 of the alignment in bytes.
 */
orl_sec_alignment OMFENTRY OmfSecGetAlignment( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) ) {
        return( sh->assoc.seg.alignment );
    }
    return( 0 );
}


/*!
 * @brief Get the class name of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Non-segment section.
 * @return Pointer to the NUL-terminated class name.
 */
char * OMFENTRY OmfSecGetClassName( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) ) {
        return( OmfGetPtrToLName( sh->omf_file_hnd, sh->assoc.seg.class ) );
    }
    return( NULL );
}


/*!
 * @brief Get the combine attribute of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval ORL_SEC_COMBINE_NONE Non-segment section.
 * @return The ORL combine value.
 */
orl_sec_combine OMFENTRY OmfSecGetCombine( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) ) {
        return( sh->assoc.seg.combine );
    }
    return( ORL_SEC_COMBINE_NONE );
}


/*!
 * @brief Get the absolute frame of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval ORL_SEC_NO_ABS_FRAME Non-segment section.
 * @return The absolute frame value.
 */
orl_sec_frame OMFENTRY OmfSecGetAbsFrame( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) ) {
        return( sh->assoc.seg.frame );
    }
    return( ORL_SEC_NO_ABS_FRAME );
}


/*!
 * @brief Get the section associated with a COMDAT.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Not a COMDAT.
 * @return The associated section, or NULL.
 */
orl_sec_handle OMFENTRY OmfSecGetAssociated( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) && ( sh->flags & ORL_SEC_FLAG_COMDAT ) ) {
        return( (orl_sec_handle)(sh->assoc.seg.comdat.assoc_seg) );
    }
    return( NULL );
}


/*!
 * @brief Get the group of a segment section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Not grouped.
 * @return The group handle, or NULL.
 */
orl_group_handle OMFENTRY OmfSecGetGroup( omf_sec_handle sh )
{
    assert( sh );

    if( _IsSegType( sh->type ) ) {
        if( sh->flags & ORL_SEC_FLAG_COMDAT ) {
            return( (orl_group_handle)(sh->assoc.seg.comdat.group) );
        } else if( sh->flags & ORL_SEC_FLAG_GROUPED ) {
            return( (orl_group_handle)(sh->assoc.seg.group) );
        }
    }
    return( NULL );
}


/*!
 * @brief Get the string table section linked to a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL The section is not a string table.
 * @return The string table section handle.
 */
omf_sec_handle OMFENTRY OmfSecGetStringTable( omf_sec_handle sh )
{
    assert( sh );
    assert( sh->omf_file_hnd );

    if( sh == sh->omf_file_hnd->lnames ) {
        return( sh );
    } else if( sh == sh->omf_file_hnd->extdefs ) {
        return( sh );
    }
    return( NULL );
}


/*!
 * @brief Get the symbol table section that owns a symbol table section.
 * @param[in] sh  Section handle. Not NULL.
 * @return The owning symbol table section handle.
 */
omf_sec_handle OMFENTRY OmfSecGetSymbolTable( omf_sec_handle sh )
{
    assert( sh );
    assert( sh->omf_file_hnd );

    return( sh->omf_file_hnd->symbol_table );
}


/*!
 * @brief Get the reloc table section that owns a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL The section is not a program-bits section.
 * @return The reloc section handle.
 */
omf_sec_handle OMFENTRY OmfSecGetRelocTable( omf_sec_handle sh )
{
    assert( sh );
    assert( sh->omf_file_hnd );

    if( sh->type == ORL_SEC_TYPE_PROG_BITS ) {
        return( sh->omf_file_hnd->relocs );
    }
    return( NULL );
}


/*!
 * @brief Get the contents buffer of a section.
 * @param[in,out] sh      Section handle. Not NULL.
 * @param[out]    buffer  Receiver for the contents pointer. Not NULL.
 * @retval ORL_OKAY   Contents available.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return ORL_OKAY on success, ORL_ERROR otherwise.
 */
orl_return OMFENTRY OmfSecGetContents( omf_sec_handle sh, unsigned_8 **buffer )
{
    orl_return  err;

    assert( sh );

    if( ( sh->contents != NULL ) || ( sh->type == ORL_SEC_TYPE_PROG_BITS ) ) {
        err = OmfExportSegmentContents( sh );
        if( err != ORL_OKAY ) return( err );
        *buffer = sh->contents;
        return( ORL_OKAY );
    }
    return( ORL_ERROR );
}


/*!
 * @brief Scan relocations of a section, optionally limited by offset.
 * @param[in] sh      Section handle. Not NULL.
 * @param[in] offset  Only match relocations at this offset.
 * @param[in] func    Callback invoked for each matching relocation.
 * @param[in] check   Non-zero to filter by @p offset.
 * @retval ORL_FALSE  No relocations, or none matched.
 * @return The callback's result, or ORL_FALSE if no match.
 */
static orl_return OMFENTRY relocScan( omf_sec_handle sh, omf_sec_offset offset,
                                      orl_reloc_return_func func, int check )
{
    uint_32                                     x;
    uint_32                                     num;
    orl_reloc                                   **relocs;
    orl_return                                  err;
    int                                         global;
    omf_sec_handle                              rsh;

    assert( sh );
    assert( sh->omf_file_hnd );
    assert( func );

    if( !sh->omf_file_hnd->relocs ) return( ORL_FALSE );
    relocs = sh->omf_file_hnd->relocs->assoc.reloc.relocs;
    num = sh->omf_file_hnd->relocs->assoc.reloc.num;
    if( num ) {
        assert( relocs );
    } else {
        return( ORL_FALSE );
    }
    global = sh->index == sh->omf_file_hnd->relocs->index;

    for( x = 0; x < num; x++ ) {
        rsh = (omf_sec_handle)(relocs[x]->section);
        if( global || ( sh->index == rsh->index ) ) {
            if( !check || ( relocs[x]->offset == offset ) ) {
                err = func( relocs[x] );
                if( err != ORL_OKAY ) return( err );
            }
        }
    }
    return( ORL_TRUE );
}


/*!
 * @brief Scan relocations at a specific offset in a section.
 * @param[in] sh      Section handle. Not NULL.
 * @param[in] offset  Offset to match.
 * @param[in] func    Callback invoked for each matching relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result or ORL_FALSE.
 */
orl_return OMFENTRY OmfSecQueryReloc( omf_sec_handle sh, omf_sec_offset offset,
                                      orl_reloc_return_func func )
{
    assert( sh );
    assert( func );

    if( sh->type != ORL_SEC_TYPE_PROG_BITS ) return( ORL_ERROR );
    return( relocScan( sh, offset, func, 1 ) );
}


/*!
 * @brief Scan all relocations of a section.
 * @param[in] sh    Section handle. Not NULL.
 * @param[in] func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result or ORL_FALSE.
 */
orl_return OMFENTRY OmfSecScanReloc( omf_sec_handle sh,
                                     orl_reloc_return_func func )
{
    assert( sh );
    assert( func );

    if( sh->type != ORL_SEC_TYPE_PROG_BITS ) return( ORL_ERROR );
    return( relocScan( sh, 0, func, 0 ) );
}

// ask jim for next 2 calls
// One of these should be implimented O(1), but the section handles
// get reordered in OmfLoad.

/*!
 * @brief Convert a section handle to its OMF index.
 * @param[in] sh  Section handle. Not NULL.
 * @return The section index.
 */
orl_table_index OMFENTRY OmfCvtSecHdlToIdx( omf_sec_handle sh )
{
    assert( sh );

    return( sh->index );
}


/*!
 * @brief Convert an OMF index to a section handle.
 * @param[in] ofh  File handle. Not NULL.
 * @param[in] idx  Section index.
 * @retval NULL Index not found.
 * @return The matching section handle.
 */
omf_sec_handle OMFENTRY OmfCvtIdxToSecHdl( omf_file_handle ofh,
                                           orl_table_index idx )
{
    omf_sec_handle      sh;

    assert( ofh );

    sh = ofh->first_sec;
    while( sh ) {
        if( sh->index == idx ) return( sh );
        sh = sh->next;
    }
    return( NULL );
}


/*!
 * @brief Scan a reloc section's relocation entries.
 * @param[in] sh    Reloc section handle. Not NULL.
 * @param[in] func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a reloc section.
 * @return Callback result or ORL_FALSE.
 */
orl_return OMFENTRY OmfRelocSecScan( omf_sec_handle sh,
                                     orl_reloc_return_func func )
{
    assert( sh );
    assert( func );

    if( sh->type != ORL_SEC_TYPE_RELOCS ) return( ORL_ERROR );
    return( relocScan( sh, 0, func, 0 ) );
}


/*!
 * @brief Scan a symbol table section's symbols.
 * @param[in] sh    Symbol table section handle. Not NULL.
 * @param[in] func  Callback invoked for each symbol.
 * @retval ORL_ERROR  Section is not a symbol table.
 * @return Callback result.
 */
orl_return OMFENTRY OmfSymbolSecScan( omf_sec_handle sh,
                                      orl_symbol_return_func func )
{
    int                                         x;
    orl_return                                  err;
    omf_symbol_handle                           *syms;

    assert( sh );
    assert( func );

    if( sh->type != ORL_SEC_TYPE_SYM_TABLE ) return( ORL_ERROR );
    syms = sh->assoc.sym.syms;
    if( !syms ) return( ORL_ERROR );

    for( x = 0; x < sh->assoc.sym.num; x++ ) {
        err = func( (orl_symbol_handle) syms[x] );
        if( err != ORL_OKAY ) return( err );
    }
    return( ORL_OKAY );
}


/*!
 * @brief Get the number of line numbers attached to a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval 0 Non-segment section.
 * @return The number of line numbers.
 */
orl_table_index OMFENTRY OmfSecGetNumLines( omf_sec_handle sh )
{
    assert( sh );

    if( sh->type == ORL_SEC_TYPE_PROG_BITS ) {
        return( sh->assoc.seg.num_lines );
    } else {
        return( 0 );
    }
}


/*!
 * @brief Get the line-number array of a section.
 * @param[in] sh  Section handle. Not NULL.
 * @retval NULL Non-segment section.
 * @return Pointer to the line-number array.
 */
orl_linnum * OMFENTRY OmfSecGetLines( omf_sec_handle sh )
{
    assert( sh );

    if( sh->type == ORL_SEC_TYPE_PROG_BITS ) {
        return( sh->assoc.seg.lines );
    } else {
        return( NULL );
    }
}


/*!
 * @brief Get the name of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return NUL-terminated symbol name.
 */
char * OMFENTRY OmfSymbolGetName( omf_symbol_handle sym )
{
    assert( sym );

    return( sym->name );
}


/*!
 * @brief Get the value (offset) of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The symbol value.
 */
orl_symbol_value OMFENTRY OmfSymbolGetValue( omf_symbol_handle sym )
{
    assert( sym );

    return( sym->offset );
}


/*!
 * @brief Get the binding of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The symbol binding.
 */
orl_symbol_binding OMFENTRY OmfSymbolGetBinding( omf_symbol_handle sym )
{
    assert( sym );

    return( sym->binding );
}


/*!
 * @brief Get the type flags of a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The symbol type flags.
 */
orl_symbol_type OMFENTRY OmfSymbolGetType( omf_symbol_handle sym )
{
    assert( sym );

    return( sym->typ );
}


/*!
 * @brief Get the raw symbol info byte.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return Always 0 (not applicable to OMF).
 */
unsigned char OMFENTRY OmfSymbolGetRawInfo( omf_symbol_handle sym )
{
    sym = sym;
    assert( sym );

    return( 0 );
}


/*!
 * @brief Get the section that contains a symbol.
 * @param[in] sym  Symbol handle. Not NULL.
 * @return The containing section handle, or NULL.
 */
omf_sec_handle OMFENTRY OmfSymbolGetSecHandle( omf_symbol_handle sym )
{
    assert( sym );

    return( sym->section );
}


/*!
 * @brief Scan a note section's comment records.
 * @param[in] hnd     Note section handle. Not NULL.
 * @param[in] cb      Note callback table. Not NULL.
 * @param[in] cookie  Callback cookie.
 * @retval ORL_ERROR  Section is not a note section.
 * @return Result of OmfParseComments().
 */
orl_return OMFENTRY OmfNoteSecScan( omf_sec_handle hnd, orl_note_callbacks *cb,
                                     void *cookie )
{
    assert( hnd );
    assert( cb );

    if( hnd->type != ORL_SEC_TYPE_NOTE ) return ORL_ERROR;
    return( OmfParseComments( hnd, cb, cookie ) );
}


/*!
 * @brief Scan all groups of a file.
 * @param[in] hnd   File handle. Not NULL.
 * @param[in] func  Callback invoked for each group.
 * @retval ORL_OKAY  Scan completed.
 * @return Callback result.
 */
orl_return              OMFENTRY OmfGroupsScan( omf_file_handle hnd,
                                                orl_group_return_func func )
{
    orl_table_index     idx;
    orl_return          err;

    assert( hnd );
    assert( func );

    err = ORL_OKAY;
    for( idx = 0; idx < hnd->num_groups; idx++ ) {
        assert( hnd->groups );
        err = func( (orl_group_handle)hnd->groups[idx] );
        if( err != ORL_OKAY ) break;
    }

    return( err );
}


/*!
 * @brief Get the name of a group.
 * @param[in] hnd  Group handle. Not NULL.
 * @return NUL-terminated group name.
 */
char *                  OMFENTRY OmfGroupName( omf_grp_handle hnd )
{
    assert( hnd );
    assert( hnd->sym );

    return( hnd->sym->name );
}


/*!
 * @brief Get the number of members of a group.
 * @param[in] hnd  Group handle. Not NULL.
 * @return The number of member segments.
 */
orl_table_index         OMFENTRY OmfGroupSize( omf_grp_handle hnd )
{
    assert( hnd );

    return( hnd->size );
}


/*!
 * @brief Get the name of a group member.
 * @param[in] hnd  Group handle. Not NULL.
 * @param[in] idx  Member index (zero-based).
 * @retval NULL Index out of range or member could not be resolved.
 * @return NUL-terminated member name.
 */
char *                  OMFENTRY OmfGroupMember( omf_grp_handle hnd,
                                                 orl_table_index idx )
{
    omf_sec_handle      sh;

    assert( hnd );

    if( ( hnd->size > 0 ) && ( idx < hnd->size ) ) {
        assert( hnd->segs );
        if( hnd->segs[idx] ) {
            sh = OmfFindSegOrComdat( hnd->omf_file_hnd, hnd->segs[idx], 0 );
            if( sh ) {
                assert( sh->assoc.seg.sym );
                return( sh->assoc.seg.sym->name );
            }
        }
    }
    return( NULL );
}
