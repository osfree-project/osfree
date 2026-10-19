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
* Description:  COFF entry points for the Object Reader Library.
*
****************************************************************************/

/*!
 * @file coffentr.c
 * @brief COFF entry points for the Object Reader Library.
 *
 * Implements the COFF-specific implementation of the ORL entry points:
 * file/segment/symbol accessors over the internal COFF handle
 * structures built by coffload.c and cofflwlv.c.
 */

#include "coffentr.h"
#include "coffflhn.h"
#include "cofflwlv.h"
#include "coffload.h"
#include "orlhash.h"

/*!
 * @brief Create a new COFF handle.
 * @param[in] funcs  Allocator/reader callback table. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new COFF handle.
 */
coff_handle COFFENTRY CoffInit( orl_funcs * funcs )
{
    coff_handle                                 coff_hnd;

    coff_hnd = (coff_handle) funcs->alloc( sizeof( coff_handle_struct ) );
    if( !coff_hnd ) return( NULL );
    coff_hnd->funcs = funcs;
    coff_hnd->first_file_hnd = NULL;
    return( coff_hnd );
}

/*!
 * @brief Destroy a COFF handle and all of its file handles.
 * @param[in,out] coff_hnd  COFF handle. Not NULL.
 * @retval ORL_OKAY   Success.
 * @return ORL_OKAY on success, or the first error from unlinking.
 */
orl_return COFFENTRY CoffFini( coff_handle coff_hnd )
{
    orl_return                                  error;

    while( coff_hnd->first_file_hnd != NULL ) {
        error = CoffRemoveFileLinks( coff_hnd->first_file_hnd );
        if( error != ORL_OKAY ) return( error );
    }
    coff_hnd->funcs->free( coff_hnd );
    return( ORL_OKAY );
}

/*!
 * @brief Open a COFF file and parse its structure.
 * @param[in,out] coff_hnd  COFF handle. Not NULL.
 * @param[in]     file      Caller's file object passed to the callbacks.
 * @retval NULL Allocation failure, or the file is not valid COFF.
 * @return The new file handle.
 */
coff_file_handle COFFENTRY CoffFileInit( coff_handle coff_hnd, void *file )
{
    coff_file_handle    coff_file_hnd;
    orl_return          return_val;

    coff_file_hnd = (coff_file_handle) coff_hnd->funcs->alloc( sizeof( coff_file_handle_struct ) );
    if( !coff_file_hnd ) return( NULL );
    coff_file_hnd->coff_sec_hnd = NULL;
    coff_file_hnd->orig_sec_hnd = NULL;
    coff_file_hnd->file = file;
    coff_file_hnd->sec_name_hash_table = NULL;
    coff_file_hnd->symbol_handles = NULL;
    coff_file_hnd->implib_data = NULL;
    CoffAddFileLinks( coff_hnd, coff_file_hnd );
    return_val = CoffLoadFileStructure( coff_file_hnd );
    if( return_val != ORL_OKAY ) {
        CoffRemoveFileLinks( coff_file_hnd );
        return( NULL );
    }
    return( coff_file_hnd );
}

/*!
 * @brief Close a COFF file handle and free it.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @return Result of CoffRemoveFileLinks().
 */
orl_return COFFENTRY CoffFileFini( coff_file_handle coff_file_hnd )
{
    return( CoffRemoveFileLinks( coff_file_hnd ) );
}

/*!
 * @brief Scan sections (or sections matching a name) of a COFF file.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     desired        Desired section name, or NULL for all.
 * @param[in]     return_func    Callback invoked for each matching section.
 * @retval ORL_OKAY  Scan completed.
 * @return ORL_OKAY on success, or the callback's error code.
 */
orl_return COFFENTRY CoffFileScan( coff_file_handle coff_file_hnd, char *desired, orl_sec_return_func return_func )
{
    orl_hash_data_struct *              data_struct;
    int                                 loop;
    orl_return                          error;

    if( !desired ) {
        /* global request */
        for( loop = 0; loop < coff_file_hnd->num_sections; loop++ ) {
            error = return_func( (orl_sec_handle) coff_file_hnd->coff_sec_hnd[loop] );
            if( error != ORL_OKAY ) return( error );
        }
    } else {
        if( !(coff_file_hnd->sec_name_hash_table) ) {
            error = CoffBuildSecNameHashTable( coff_file_hnd );
            if( error != ORL_OKAY ) return( error );
        }
        data_struct = ORLHashTableQuery( coff_file_hnd->sec_name_hash_table, (orl_hash_value) desired );
        while( data_struct != NULL ) {
            error = return_func( (orl_sec_handle) data_struct->data );
            if( error != ORL_OKAY ) return( error );
            data_struct = data_struct->next;
        }
    }
    return( ORL_OKAY );
}

/*!
 * @brief Get the RVA of the export table of a COFF file.
 * @param[in] coff_file_hnd  COFF file handle. Not NULL.
 * @return The export-table RVA, or 0 if none.
 */
unsigned long COFFENTRY CoffExportTableRVA(coff_file_handle coff_file_hnd)
{
    return coff_file_hnd->export_table_rva;
}

/*!
 * @brief Get the machine type of a COFF file.
 * @param[in] coff_file_hnd  COFF file handle. Not NULL.
 * @return The machine type.
 */
orl_machine_type COFFENTRY CoffFileGetMachineType( coff_file_handle coff_file_hnd )
{
    return( coff_file_hnd->machine_type );
}

/*!
 * @brief Get the file flags of a COFF file.
 * @param[in] coff_file_hnd  COFF file handle. Not NULL.
 * @return The file flags.
 */
orl_file_flags COFFENTRY CoffFileGetFlags( coff_file_handle coff_file_hnd )
{
    return( coff_file_hnd->flags );
}

/*!
 * @brief Get the file type of a COFF file.
 * @param[in] coff_file_hnd  COFF file handle. Not NULL.
 * @return The file type.
 */
orl_file_type COFFENTRY CoffFileGetType( coff_file_handle coff_file_hnd )
{
    return( coff_file_hnd->type );
}

/*!
 * @brief Get the file size of a COFF file.
 * @param[in] coff_file_hnd  COFF file handle. Not NULL.
 * @return The file size in bytes.
 */
orl_file_size COFFENTRY CoffFileGetSize( coff_file_handle coff_file_hnd )
{
    return( coff_file_hnd->size );
}

/*!
 * @brief Get the symbol table section of a COFF file.
 * @param[in] coff_file_hnd  COFF file handle. Not NULL.
 * @return The symbol table section handle, or NULL.
 */
coff_sec_handle COFFENTRY CoffFileGetSymbolTable( coff_file_handle coff_file_hnd )
{
    return( coff_file_hnd->symbol_table );
}


/*!
 * @brief Get the string table section of a COFF section's owner.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The string table section handle.
 */
coff_sec_handle COFFENTRY CoffSecGetStringTable( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->coff_file_hnd->string_table );
}

/*!
 * @brief Get the symbol table section of a COFF section's owner.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The symbol table section handle.
 */
coff_sec_handle COFFENTRY CoffSecGetSymbolTable( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->coff_file_hnd->symbol_table );
}

/*!
 * @brief Get the name of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return NUL-terminated section name.
 */
char * COFFENTRY CoffSecGetName( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->name );
}

/*!
 * @brief Get the base offset of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The base offset.
 */
orl_sec_offset COFFENTRY CoffSecGetBase( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->base );
}

/*!
 * @brief Get the declared size of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The section size in bytes.
 */
orl_sec_size COFFENTRY CoffSecGetSize( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->size );
}

/*!
 * @brief Get the type of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The section type.
 */
orl_sec_type COFFENTRY CoffSecGetType( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->type );
}

/*!
 * @brief Get the flags of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The section flags.
 */
orl_sec_flags COFFENTRY CoffSecGetFlags( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->flags );
}

/*!
 * @brief Get the alignment of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return Log2 of the alignment in bytes.
 */
orl_sec_alignment COFFENTRY CoffSecGetAlignment( coff_sec_handle coff_sec_hnd )
{
    return( coff_sec_hnd->align );
}

/*!
 * @brief Get the reloc table section linked to a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @retval NULL The section is itself a reloc section.
 * @return The reloc section handle, or NULL.
 */
coff_sec_handle COFFENTRY CoffSecGetRelocTable( coff_sec_handle coff_sec_hnd )
{
    if( coff_sec_hnd->type != ORL_SEC_TYPE_RELOCS ) {
        return( coff_sec_hnd->assoc.normal.reloc_sec );
    }
    return( NULL );
}

/*!
 * @brief Get the number of line numbers attached to a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @retval 0 No line numbers.
 * @return The number of line numbers.
 */
orl_table_index COFFENTRY CoffSecGetNumLines( coff_sec_handle coff_sec_hnd )
{
    if( coff_sec_hnd->hdr != NULL ) {
        return( coff_sec_hnd->hdr->num_lineno );
    }
    return( 0 );
}

/*!
 * @brief Get the line-number array of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @retval NULL No line numbers, or conversion failed.
 * @return Pointer to the line-number array.
 */
orl_linnum * COFFENTRY CoffSecGetLines( coff_sec_handle coff_sec_hnd )
{
    orl_table_index     numlines;

    numlines = CoffSecGetNumLines( coff_sec_hnd );
    if( numlines > 0 ) {
        return CoffConvertLines( coff_sec_hnd, numlines );
    }
    return( NULL );
}

/*!
 * @brief Get the file offset of a COFF section's raw data.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @return The raw-data file offset, or 0.
 */
orl_sec_offset COFFENTRY CoffSecGetOffset( coff_sec_handle coff_sec_hnd )
{
    if( coff_sec_hnd->hdr != NULL ) {
        return( coff_sec_hnd->hdr->offset );
    }
    return( 0 );
}

/*!
 * @brief Get the contents buffer of a COFF section.
 * @param[in,out] coff_sec_hnd  Section handle. Not NULL.
 * @param[out]    buffer        Receiver for the contents pointer. Not NULL.
 * @retval ORL_OKAY   Contents available.
 * @retval ORL_ERROR  Section has no contents.
 * @return ORL_OKAY on success, ORL_ERROR otherwise.
 */
orl_return COFFENTRY CoffSecGetContents( coff_sec_handle coff_sec_hnd, char **buffer )
{
    if( coff_sec_hnd->contents != NULL ) {
        *buffer = coff_sec_hnd->contents;
        return( ORL_OKAY );
    }
    return( ORL_ERROR );
}

/*!
 * @brief Scan relocations of a COFF section, optionally limited by offset.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @param[in] sec_offset    Offset to match.
 * @param[in] return_func   Callback invoked for each matching relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result, ORL_FALSE if no match.
 */
orl_return COFFENTRY CoffSecQueryReloc( coff_sec_handle coff_sec_hnd, coff_sec_offset sec_offset, orl_reloc_return_func return_func )
{
    int                                         index;
    coff_sec_handle                             reloc_sec_hnd;
    orl_reloc *                                 reloc;
    orl_return                                  return_val;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS ) return( ORL_ERROR );
    reloc_sec_hnd = coff_sec_hnd->assoc.normal.reloc_sec;
    if( reloc_sec_hnd == NULL ) return( ORL_FALSE );
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        return_val = CoffCreateRelocs( coff_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY ) return( return_val );
    }
    reloc = reloc_sec_hnd->assoc.reloc.relocs;
    return_val = ORL_FALSE;
    for( index = 0; index < reloc_sec_hnd->assoc.reloc.num_relocs; index++ ) {
        if( reloc->offset == sec_offset ) {
            return_val = return_func( reloc );
            if( return_val != ORL_OKAY ) return( return_val );
        }
        reloc++;
    }
    return( return_val );
}

/*!
 * @brief Convert a COFF section handle to its one-based index.
 *
 * @note Currently O(n); a future improvement could cache the index.
 *
 * @param[in] shdl  Section handle. Not NULL.
 * @retval 0  Section handle not found in the file.
 * @return The one-based section index, or 0 if not found.
 */
orl_table_index COFFENTRY CoffCvtSecHdlToIdx( coff_sec_handle shdl )
/******************************************************************/
// NYI: would be nice to have a O(1) way of doing this.
{
    orl_table_index     index;
    orl_table_index     limit;
    coff_file_handle    fhdl;

    fhdl = shdl->coff_file_hnd;
    limit = fhdl->f_hdr_buffer->num_sections;
    for( index = 0; index < limit; index++ ) {
        if( fhdl->orig_sec_hnd[index] == shdl ) return index + 1;
    }
    return 0;
}

/*!
 * @brief Convert a one-based COFF index to a section handle.
 * @param[in] fhdl  File handle. Not NULL.
 * @param[in] idx   One-based section index.
 * @return The matching section handle.
 */
coff_sec_handle COFFENTRY CoffCvtIdxToSecHdl( coff_file_handle fhdl,
                                              orl_table_index idx )
/******************************************************************/
{
    return fhdl->orig_sec_hnd[idx - 1];
}

/*!
 * @brief Scan all relocations of a COFF section.
 * @param[in] coff_sec_hnd  Section handle. Not NULL.
 * @param[in] return_func   Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result, or ORL_FALSE if none.
 */
orl_return COFFENTRY CoffSecScanReloc( coff_sec_handle coff_sec_hnd, orl_reloc_return_func return_func )
{
    int                                         index;
    coff_sec_handle                             reloc_sec_hnd;
    orl_reloc *                                 reloc;
    orl_return                                  return_val;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS ) return( ORL_ERROR );
    reloc_sec_hnd = coff_sec_hnd->assoc.normal.reloc_sec;
    if( reloc_sec_hnd == NULL ) return( ORL_FALSE );
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        return_val = CoffCreateRelocs( coff_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY ) return( return_val );
    }
    reloc = reloc_sec_hnd->assoc.reloc.relocs;
    return_val = ORL_FALSE;
    for( index = 0; index < reloc_sec_hnd->assoc.reloc.num_relocs; index++ ) {
        return_val = return_func( reloc );
        if( return_val != ORL_OKAY ) return( return_val );
        reloc++;
    }
    return( return_val );
}

/*!
 * @brief Scan a reloc section's relocation entries.
 * @param[in] coff_sec_hnd  Reloc section handle. Not NULL.
 * @param[in] return_func   Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a reloc section.
 * @return Callback result, or ORL_FALSE if none.
 */
orl_return COFFENTRY CoffRelocSecScan( coff_sec_handle coff_sec_hnd, orl_reloc_return_func return_func )
{
    int                                         index;
    orl_reloc *                                 reloc;
    orl_return                                  return_val;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_RELOCS ) return( ORL_ERROR );
    if( coff_sec_hnd->assoc.reloc.relocs == NULL ) {
        return_val = CoffCreateRelocs( coff_sec_hnd->assoc.reloc.orig_sec, coff_sec_hnd );
        if( return_val != ORL_OKAY ) return( return_val );
    }
    reloc = coff_sec_hnd->assoc.reloc.relocs;
    return_val = ORL_FALSE;
    for( index = 0; index < coff_sec_hnd->assoc.reloc.num_relocs; index++ ) {
        return_val = return_func( reloc );
        if( return_val != ORL_OKAY ) return( return_val );
        reloc++;
    }
    return( return_val );
}

/*!
 * @brief Scan a symbol table section's symbols.
 * @param[in] coff_sec_hnd  Symbol table section handle. Not NULL.
 * @param[in] return_func   Callback invoked for each symbol.
 * @retval ORL_ERROR  Section is not a symbol table.
 * @return Callback result.
 */
orl_return COFFENTRY CoffSymbolSecScan( coff_sec_handle coff_sec_hnd, orl_symbol_return_func return_func )
{
    int                                         index;
    orl_return                                  error;
    coff_symbol_handle                          coff_symbol_hnd;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_SYM_TABLE ) return( ORL_ERROR );
    if( !(coff_sec_hnd->coff_file_hnd->symbol_handles) ) {
        error = CoffCreateSymbolHandles( coff_sec_hnd->coff_file_hnd );
        if( error != ORL_OKAY ) return( error );
    }
    for( index = 0; index < coff_sec_hnd->coff_file_hnd->num_symbols; index++ ) {
        coff_symbol_hnd = &(coff_sec_hnd->coff_file_hnd->symbol_handles[index]);
        error = return_func( (orl_symbol_handle) coff_symbol_hnd );
        if( error != ORL_OKAY ) return( error );
        index += coff_symbol_hnd->symbol->num_aux;
    }
    return( ORL_OKAY );
}

/*!
 * @brief Scan a note section's directive records.
 * @param[in] hnd     Note section handle. Not NULL.
 * @param[in] cb      Note callback table. Not NULL.
 * @param[in] cookie  Callback cookie.
 * @retval ORL_OKAY   Section is not a directive note, or parsing completed.
 * @retval ORL_ERROR  Section is not a note section.
 * @return Result of CoffParseDrectve().
 */
orl_return COFFENTRY CoffNoteSecScan( coff_sec_handle hnd,
                                      orl_note_callbacks *cb, void *cookie )
/**************************************************************************/
{
    if( hnd->type != ORL_SEC_TYPE_NOTE ) return ORL_ERROR;
    if( strcmp( hnd->name, ".drectve" ) != 0 ) return ORL_OKAY;
    if( hnd->size == 0 ) return ORL_OKAY;
    return CoffParseDrectve( hnd->contents, hnd->size, cb, cookie );
}

/*!
 * @brief Get the name of a COFF symbol.
 * @param[in] coff_symbol_hnd  Symbol handle. Not NULL.
 * @return NUL-terminated symbol name.
 */
char * COFFENTRY CoffSymbolGetName( coff_symbol_handle coff_symbol_hnd )
{
    if( coff_symbol_hnd->type & ORL_SYM_TYPE_FILE ) {
        return (char *) (coff_symbol_hnd->symbol + 1);
    }
    return( coff_symbol_hnd->name );
}

/*!
 * @brief Get the value (offset) of a COFF symbol.
 * @param[in] coff_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol value.
 */
orl_symbol_value COFFENTRY CoffSymbolGetValue( coff_symbol_handle coff_symbol_hnd )
{
    return( coff_symbol_hnd->symbol->value );
}

/*!
 * @brief Get the binding of a COFF symbol.
 * @param[in] coff_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol binding.
 */
orl_symbol_binding COFFENTRY CoffSymbolGetBinding( coff_symbol_handle coff_symbol_hnd )
{
    return( coff_symbol_hnd->binding );
}

/*!
 * @brief Get the type flags of a COFF symbol.
 * @param[in] coff_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol type flags.
 */
orl_symbol_type COFFENTRY CoffSymbolGetType( coff_symbol_handle coff_symbol_hnd )
{
    return( coff_symbol_hnd->type );
}

/*!
 * @brief Get the section that contains a COFF symbol.
 * @param[in] coff_symbol_hnd  Symbol handle. Not NULL.
 * @retval NULL Symbol is not bound to a section.
 * @return The containing section handle, or NULL.
 */
coff_sec_handle COFFENTRY CoffSymbolGetSecHandle( coff_symbol_handle coff_symbol_hnd )
{
    if( coff_symbol_hnd->symbol->sec_num < 1 ) return( NULL );
    return( coff_symbol_hnd->coff_file_hnd->orig_sec_hnd[coff_symbol_hnd->symbol->sec_num - 1]);
}

/*!
 * @brief Get the symbol referenced by a weak-external COFF symbol.
 * @param[in] hnd  Weak-external symbol handle. Not NULL.
 * @return The associated symbol handle.
 */
coff_symbol_handle COFFENTRY CoffSymbolGetAssociated( coff_symbol_handle hnd )
{
    coff_sym_weak *     weak;

    weak = (coff_sym_weak *) (hnd->symbol + 1);
    return &hnd->coff_file_hnd->symbol_handles[weak->tag_index];
}
