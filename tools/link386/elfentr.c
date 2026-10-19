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
 * @file elfentr.c
 * @brief ELF entry points for the Object Reader Library.
 *
 * Implements the ELF-specific implementation of the ORL entry points:
 * file/segment/symbol accessors over the internal ELF handle structures
 * built by elfload.c and elflwlv.c.
 */

#include "elfentr.h"
#include "elfload.h"
#include "elfflhn.h"
#include "elflwlv.h"
#include "orlhash.h"

/*!
 * @brief Create a new ELF handle.
 * @param[in] funcs  Allocator/reader callback table. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new ELF handle.
 */
elf_handle ELFENTRY ElfInit( orl_funcs * funcs )
{
    elf_handle                                  elf_hnd;

    elf_hnd = (elf_handle) funcs->alloc( sizeof( elf_handle_struct ) );
    if( !elf_hnd ) return( NULL );
    elf_hnd->funcs = funcs;
    elf_hnd->first_file_hnd = NULL;
    return( elf_hnd );
}

/*!
 * @brief Destroy an ELF handle and all of its file handles.
 * @param[in,out] elf_hnd  ELF handle. Not NULL.
 * @retval ORL_OKAY   Success.
 * @return ORL_OKAY on success, or the first error from unlinking.
 */
orl_return ELFENTRY ElfFini( elf_handle elf_hnd )
{
    orl_return                                  error;

    while( elf_hnd->first_file_hnd != NULL ) {
        error = ElfRemoveFileLinks( elf_hnd->first_file_hnd );
        if( error != ORL_OKAY ) return( error );
    }
    elf_hnd->funcs->free( elf_hnd );
    return( ORL_OKAY );
}

/*!
 * @brief Open an ELF file and parse its structure.
 * @param[in,out] elf_hnd  ELF handle. Not NULL.
 * @param[in]     file     Caller's file object passed to the callbacks.
 * @retval NULL Allocation failure, or the file is not valid ELF.
 * @return The new file handle.
 */
elf_file_handle ELFENTRY ElfFileInit( elf_handle elf_hnd, void *file )
{
    elf_file_handle                             elf_file_hnd;

    elf_file_hnd = (elf_file_handle) elf_hnd->funcs->alloc( sizeof( elf_file_handle_struct ) );
    if( !elf_file_hnd ) {
        return( NULL );
    }
    elf_file_hnd->elf_sec_hnd = NULL;
    elf_file_hnd->file = file;
    elf_file_hnd->sec_name_hash_table = NULL;
    ElfAddFileLinks( elf_hnd, elf_file_hnd );
    if( ElfLoadFileStructure( elf_file_hnd ) != ORL_OKAY ) {
        ElfRemoveFileLinks( elf_file_hnd );
        return( NULL );
    }
    return( elf_file_hnd );
}

/*!
 * @brief Close an ELF file handle and free it.
 * @param[in,out] elf_file_hnd  ELF file handle. Not NULL.
 * @return Result of ElfRemoveFileLinks().
 */
orl_return ELFENTRY ElfFileFini( elf_file_handle elf_file_hnd )
{
    return( ElfRemoveFileLinks( elf_file_hnd ) );
}

/*!
 * @brief Scan sections (or sections matching a name) of an ELF file.
 * @param[in,out] elf_file_hnd  ELF file handle. Not NULL.
 * @param[in]     desired       Desired section name, or NULL for all.
 * @param[in]     return_func   Callback invoked for each matching section.
 * @retval ORL_OKAY  Scan completed.
 * @return ORL_OKAY on success, or the callback's error code.
 */
orl_return ELFENTRY ElfFileScan( elf_file_handle elf_file_hnd, char *desired, orl_sec_return_func return_func )
{
    orl_hash_data_struct *              data_struct;
    int                                 loop;
    orl_return                          error;

    if( !desired ) {
        /* global request */
        for( loop = 0; loop < elf_file_hnd->num_sections; loop++ ) {
            error = return_func( (orl_sec_handle) elf_file_hnd->elf_sec_hnd[loop] );
            if( error != ORL_OKAY ) return( error );
        }
    } else {
        if( !(elf_file_hnd->sec_name_hash_table) ) {
            error = ElfBuildSecNameHashTable( elf_file_hnd );
            if( error != ORL_OKAY ) return( error );
        }
        data_struct = ORLHashTableQuery( elf_file_hnd->sec_name_hash_table, (orl_hash_value) desired );
        while( data_struct != NULL ) {
            error = return_func( (orl_sec_handle) data_struct->data );
            if( error != ORL_OKAY ) return( error );
            data_struct = data_struct->next;
        }
    }
    return( ORL_OKAY );
}

/*!
 * @brief Get the machine type of an ELF file.
 * @param[in] elf_file_hnd  ELF file handle. Not NULL.
 * @return The machine type.
 */
orl_machine_type ELFENTRY ElfFileGetMachineType( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->machine_type );
}

/*!
 * @brief Get the file flags of an ELF file.
 * @param[in] elf_file_hnd  ELF file handle. Not NULL.
 * @return The file flags.
 */
orl_file_flags ELFENTRY ElfFileGetFlags( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->flags );
}

/*!
 * @brief Get the file type of an ELF file.
 * @param[in] elf_file_hnd  ELF file handle. Not NULL.
 * @return The file type.
 */
orl_file_type ELFENTRY ElfFileGetType( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->type );
}

/*!
 * @brief Get the file size of an ELF file.
 * @param[in] elf_file_hnd  ELF file handle. Not NULL.
 * @return The file size in bytes.
 */
orl_file_size ELFENTRY ElfFileGetSize( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->size );
}

/*!
 * @brief Get the symbol table section of an ELF file.
 * @param[in] elf_file_hnd  ELF file handle. Not NULL.
 * @return The symbol table section handle, or NULL.
 */
elf_sec_handle ELFENTRY ElfFileGetSymbolTable( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->symbol_table );
}

/*!
 * @brief Get the name of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @return NUL-terminated section name.
 */
char * ELFENTRY ElfSecGetName( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->name );
}

/*!
 * @brief Get the base address of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @return The base address.
 */
orl_sec_offset ELFENTRY ElfSecGetBase( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->base );
}

/*!
 * @brief Get the declared size of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @return The section size in bytes.
 */
orl_sec_size ELFENTRY ElfSecGetSize( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->size );
}

/*!
 * @brief Get the type of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @return The section type.
 */
orl_sec_type ELFENTRY ElfSecGetType( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->type );
}

/*!
 * @brief Get the flags of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @return The section flags.
 */
orl_sec_flags ELFENTRY ElfSecGetFlags( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->flags );
}

/*!
 * @brief Get the alignment of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @return Log2 of the alignment in bytes.
 */
orl_sec_alignment ELFENTRY ElfSecGetAlignment( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->alignment );
}

/*!
 * @brief Get the string table section linked to an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @retval NULL The section does not have an associated string table.
 * @return The string table section handle, or NULL.
 */
elf_sec_handle ELFENTRY ElfSecGetStringTable( elf_sec_handle elf_sec_hnd )
{
    switch( elf_sec_hnd->type ) {
        case ORL_SEC_TYPE_SYM_TABLE:
        case ORL_SEC_TYPE_DYN_SYM_TABLE:
            return( elf_sec_hnd->assoc.sym.string_table );
        case ORL_SEC_TYPE_IMPORT:
            return( elf_sec_hnd->assoc.import.string_table );
        default:
            return( NULL );
    }
}

/*!
 * @brief Get the symbol table section linked to an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @retval NULL The section does not have an associated symbol table.
 * @return The symbol table section handle, or NULL.
 */
elf_sec_handle ELFENTRY ElfSecGetSymbolTable( elf_sec_handle elf_sec_hnd )
{
    switch( elf_sec_hnd->type ) {
        case ORL_SEC_TYPE_RELOCS:
        case ORL_SEC_TYPE_RELOCS_EXPADD:
            return( elf_sec_hnd->assoc.reloc.symbol_table );
        case ORL_SEC_TYPE_EXPORT:
            return( elf_sec_hnd->assoc.export.symbol_table );
        default:
            return( NULL );
    }
}

/*!
 * @brief Get the reloc table section linked to an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @retval NULL The section has no associated reloc table.
 * @return The reloc section handle, or NULL.
 */
elf_sec_handle ELFENTRY ElfSecGetRelocTable( elf_sec_handle elf_sec_hnd )
{
    switch( elf_sec_hnd->type ) {
        case ORL_SEC_TYPE_RELOCS:
        case ORL_SEC_TYPE_RELOCS_EXPADD:
        case ORL_SEC_TYPE_SYM_TABLE:
        case ORL_SEC_TYPE_DYN_SYM_TABLE:
        case ORL_SEC_TYPE_IMPORT:
        case ORL_SEC_TYPE_EXPORT:
            return( NULL );
        default:
            return( elf_sec_hnd->assoc.normal.reloc_sec );
    }
}

/*!
 * @brief Get the contents buffer of an ELF section.
 * @param[in,out] elf_sec_hnd  Section handle. Not NULL.
 * @param[out]    buffer       Receiver for the contents pointer. Not NULL.
 * @retval ORL_OKAY   Contents available.
 * @retval ORL_ERROR  Section has no contents.
 * @return ORL_OKAY on success.
 */
orl_return ELFENTRY ElfSecGetContents( elf_sec_handle elf_sec_hnd, char **buffer )
{
    if( elf_sec_hnd->contents != NULL ) {
        *buffer = elf_sec_hnd->contents;
        return( ORL_OKAY );
    }
    return( ORL_ERROR );
}

/*!
 * @brief Scan relocations of an ELF section at a specific offset.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @param[in] sec_offset   Offset to match.
 * @param[in] return_func  Callback invoked for each matching relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result, ORL_FALSE if no match.
 */
orl_return ELFENTRY ElfSecQueryReloc( elf_sec_handle elf_sec_hnd, elf_sec_offset sec_offset, orl_reloc_return_func return_func )
{
    int                                         index;
    elf_sec_handle                              reloc_sec_hnd;
    orl_reloc *                                 reloc;
    orl_return                                  return_val;

    if( elf_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS ) return( ORL_ERROR );
    reloc_sec_hnd = elf_sec_hnd->assoc.normal.reloc_sec;
    if( reloc_sec_hnd == NULL ) return( ORL_FALSE );
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        return_val = ElfCreateRelocs( elf_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY ) return( return_val );
    }
    reloc = reloc_sec_hnd->assoc.reloc.relocs;
    return_val = ORL_FALSE;
    for( index = 0; index < reloc_sec_hnd->size; index += reloc_sec_hnd->entsize ) {
        if( reloc->offset == sec_offset ) {
            return_val = return_func( reloc );
            if( return_val != ORL_OKAY ) return( return_val );
        }
        reloc++;
    }
    return( return_val );
}

/*!
 * @brief Scan all relocations of an ELF section.
 * @param[in] elf_sec_hnd  Section handle. Not NULL.
 * @param[in] return_func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a program-bits section.
 * @return Callback result, or ORL_TRUE if none.
 */
orl_return ELFENTRY ElfSecScanReloc( elf_sec_handle elf_sec_hnd, orl_reloc_return_func return_func )
{
    int                                         index;
    elf_sec_handle                              reloc_sec_hnd;
    orl_reloc *                                 reloc;
    orl_return                                  return_val;

    if( elf_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS ) {
        return( ORL_ERROR );
    } else if ( !(elf_sec_hnd->assoc.normal.reloc_sec) ) {
        return( ORL_FALSE );
    }
    reloc_sec_hnd = elf_sec_hnd->assoc.normal.reloc_sec;
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        return_val = ElfCreateRelocs( elf_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY ) return( return_val );
    }
    reloc = reloc_sec_hnd->assoc.reloc.relocs;
    return_val = ORL_FALSE;
    for( index = 0; index < reloc_sec_hnd->size; index += reloc_sec_hnd->entsize ) {
        return_val = return_func( reloc );
        if( return_val != ORL_OKAY ) return( return_val );
        reloc++;
    }
    return( ORL_TRUE );
}

/*!
 * @brief Convert an ELF section handle to its index.
 *
 * One of these should be implemented O(1), but the section handles
 * get reordered in ElfLoad.
 *
 * @param[in] shdl  Section handle. Not NULL.
 * @retval 0  Section handle not found in the file.
 * @return The section index, or 0 if not found.
 */
orl_table_index ELFENTRY ElfCvtSecHdlToIdx( elf_sec_handle shdl )
{
    orl_table_index     index;
    orl_table_index     limit;
    elf_file_handle     fhdl;

    fhdl = shdl->elf_file_hnd;
    limit = fhdl->num_sections;
    for( index = 0; index < limit; index++ ) {
        if( fhdl->elf_sec_hnd[index] == shdl ) {
            return( fhdl->elf_sec_hnd[index]->index );
        }
    }
    return 0;
}

/*!
 * @brief Convert an ELF index to a section handle.
 * @param[in] fhdl  File handle. Not NULL.
 * @param[in] idx   Section index.
 * @retval 0  Index not found.
 * @return The matching section handle, or NULL.
 */
elf_sec_handle ELFENTRY ElfCvtIdxToSecHdl( elf_file_handle fhdl,
                                              orl_table_index idx )
{
    orl_table_index     index;
    orl_table_index     limit;

    limit = fhdl->num_sections;
    for( index = 0; index < limit; index++ ) {
        if( fhdl->elf_sec_hnd[index]->index == idx ) {
            return( fhdl->elf_sec_hnd[index] );
        }
    }
    return 0;
}

/*!
 * @brief Scan a reloc section's relocation entries.
 * @param[in] elf_sec_hnd  Reloc section handle. Not NULL.
 * @param[in] return_func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Section is not a reloc section.
 * @return Callback result, or ORL_TRUE if none.
 */
orl_return ELFENTRY ElfRelocSecScan( elf_sec_handle elf_sec_hnd, orl_reloc_return_func return_func )
{
    int                                         index;
    orl_reloc *                                 reloc;
    orl_return                                  return_val;

    if( elf_sec_hnd->type != ORL_SEC_TYPE_RELOCS && elf_sec_hnd->type != ORL_SEC_TYPE_RELOCS_EXPADD ) return( ORL_ERROR );
    if( elf_sec_hnd->assoc.reloc.relocs == NULL ) {
        return_val = ElfCreateRelocs( elf_sec_hnd->assoc.reloc.orig_sec, elf_sec_hnd );
        if( return_val != ORL_OKAY ) return( return_val );
    }
    reloc = elf_sec_hnd->assoc.reloc.relocs;
    return_val = ORL_FALSE;
    for( index = 0; index < elf_sec_hnd->size; index += elf_sec_hnd->entsize ) {
        return_val = return_func( reloc );
        if( return_val != ORL_OKAY ) return( return_val );
        reloc++;
    }
    return( ORL_TRUE );
}

/*!
 * @brief Scan a symbol table section's symbols.
 * @param[in] elf_sec_hnd  Symbol table section handle. Not NULL.
 * @param[in] return_func  Callback invoked for each symbol.
 * @retval ORL_ERROR  Section is not a symbol table.
 * @return Callback result.
 */
orl_return ELFENTRY ElfSymbolSecScan( elf_sec_handle elf_sec_hnd, orl_symbol_return_func return_func )
{
    int                                         index;
    orl_return                                  error;
    elf_symbol_handle                           elf_symbol_hnd;

    switch( elf_sec_hnd->type ) {
        case ORL_SEC_TYPE_SYM_TABLE:
        case ORL_SEC_TYPE_DYN_SYM_TABLE:
            if( !(elf_sec_hnd->assoc.sym.symbols) ) {
                error = ElfCreateSymbolHandles( elf_sec_hnd );
                if( error != ORL_OKAY ) return( error );
            }
            elf_symbol_hnd = elf_sec_hnd->assoc.sym.symbols;
            break;
        default:
            return( ORL_ERROR );
    }
    for( index = 0; index < elf_sec_hnd->size; index += elf_sec_hnd->entsize ) {
        error = return_func( (orl_symbol_handle) elf_symbol_hnd );
        if( error != ORL_OKAY ) return( error );
        elf_symbol_hnd++;
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
 * @return Result of ElfParseDrectve().
 */
orl_return ELFENTRY ElfNoteSecScan( elf_sec_handle hnd,
                                    orl_note_callbacks *cb, void *cookie )
/**************************************************************************/
{
    if( hnd->type != ORL_SEC_TYPE_NOTE ) return ORL_ERROR;
    if( strcmp( hnd->name, ".drectve" ) != 0 ) return ORL_OKAY;
    if( hnd->size == 0 ) return ORL_OKAY;
    return( ElfParseDrectve( hnd->contents, hnd->size, cb, cookie ) );
}

/*!
 * @brief Get the name of an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @return NUL-terminated symbol name, or NULL.
 */
char * ELFENTRY ElfSymbolGetName( elf_symbol_handle elf_symbol_hnd )
{
    return( elf_symbol_hnd->name );
}

/*!
 * @brief Get the value (offset) of an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol value.
 */
orl_symbol_value ELFENTRY ElfSymbolGetValue( elf_symbol_handle elf_symbol_hnd )
{
    return( elf_symbol_hnd->value );
}

/*!
 * @brief Get the binding of an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol binding.
 */
orl_symbol_binding ELFENTRY ElfSymbolGetBinding( elf_symbol_handle elf_symbol_hnd )
{
    return( elf_symbol_hnd->binding );
}

/*!
 * @brief Get the type flags of an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol type flags.
 */
orl_symbol_type ELFENTRY ElfSymbolGetType( elf_symbol_handle elf_symbol_hnd )
{
    return( elf_symbol_hnd->type );
}

/*!
 * @brief Get the raw info byte of an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @return The raw st_info byte.
 */
unsigned char ELFENTRY ElfSymbolGetRawInfo( elf_symbol_handle elf_symbol_hnd )
{
    return( elf_symbol_hnd->info );
}

/*!
 * @brief Get the section that contains an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @retval NULL Symbol is absolute, common or undefined.
 * @return The containing section handle, or NULL.
 */
elf_sec_handle ELFENTRY ElfSymbolGetSecHandle( elf_symbol_handle elf_symbol_hnd )
{
    switch( elf_symbol_hnd->shndx ) {
        case SHN_ABS:
        case SHN_COMMON:
        case SHN_UNDEF:
            return( NULL );
        default:
            return( elf_symbol_hnd->elf_file_hnd->orig_sec_hnd[elf_symbol_hnd->shndx - 1]);
    }
}

/*!
 * @brief Get the symbol referenced by an ELF symbol.
 * @param[in] elf_symbol_hnd  Symbol handle. Not NULL.
 * @return Always the same handle (ELF has no separate weak target).
 */
elf_symbol_handle ELFENTRY ElfSymbolGetAssociated( elf_symbol_handle elf_symbol_hnd )
{
    return elf_symbol_hnd;
}
