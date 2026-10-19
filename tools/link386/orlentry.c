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
* Description:  Entry points for ORL routines.
*
****************************************************************************/

/*!
 * @file orlentry.c
 * @brief Public entry points for the Object Reader Library.
 *
 * These functions dispatch to the ELF, COFF or OMF backend selected
 * by the file/section/symbol handle's own @c type field.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orllevel.h"
#include "orlentry.h"
#include "orlflhnd.h"
#include "pcobj.h"

/*!
 * @brief Create a new ORL handle and its per-format sub-handles.
 * @param[in] funcs  Allocator/reader callback table. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new ORL handle.
 */
orl_handle ORLENTRY ORLInit( orl_funcs * funcs )
{
    orl_handle                  orl_hnd;

    orl_hnd = (orl_handle) funcs->alloc( sizeof( orl_handle_struct ) );
    if( !orl_hnd ) return( NULL );
    orl_hnd->error = ORL_OKAY;
    orl_hnd->funcs = funcs;
    orl_hnd->elf_hnd = ElfInit( funcs );
    if( !(orl_hnd->elf_hnd) ) {
        funcs->free( orl_hnd );
        return( NULL );
    }
    orl_hnd->coff_hnd = CoffInit( funcs );
    if( !(orl_hnd->coff_hnd) ) {
        funcs->free( orl_hnd );
        return( NULL );
    }
    orl_hnd->omf_hnd = OmfInit( funcs );
    if( !(orl_hnd->omf_hnd) ) {
        funcs->free( orl_hnd );
        return( NULL );
    }
    orl_hnd->first_file_hnd = NULL;
    return( orl_hnd );
}

/*!
 * @brief Get the last error recorded on an ORL handle.
 * @param[in] orl_hnd  ORL handle. Not NULL.
 * @return The last ORL error code.
 */
orl_return ORLENTRY ORLGetError( orl_handle orl_hnd ) {
    return( orl_hnd->error );
}

/*!
 * @brief Destroy an ORL handle and all of its file handles.
 * @param[in,out] orl_hnd  ORL handle. Not NULL.
 * @retval ORL_OKAY   Success.
 * @return ORL_OKAY on success, or the first error from unlinking.
 */
orl_return ORLENTRY ORLFini( orl_handle orl_hnd ) {
    orl_return                  error;

    if( ( error = ElfFini( orl_hnd->elf_hnd ) ) != ORL_OKAY ) return( error );
    if( ( error = CoffFini( orl_hnd->coff_hnd ) ) != ORL_OKAY ) return( error );
    if( ( error = OmfFini( orl_hnd->omf_hnd ) ) != ORL_OKAY ) return( error );
    while( orl_hnd->first_file_hnd ) {
        error = ORLRemoveFileLinks( orl_hnd->first_file_hnd );
        if( error != ORL_OKAY ) return( error );
    }
    orl_hnd->funcs->free( orl_hnd );
    return( ORL_OKAY );
}

/*!
 * @brief Identify the format of an object file by its magic bytes.
 *
 * Recognises ELF (by the ELF magic), OMF (by a valid THEADR as the
 * first record), COFF (by the machine type in the file header) and
 * PE/COFF (by the MZ/PE signature chain).
 *
 * @param[in] orl_hnd  ORL handle. Not NULL.
 * @param[in] file     Caller's file object passed to the callbacks.
 * @retval ORL_ELF  ELF object file.
 * @retval ORL_COFF  COFF or PE/COFF object file.
 * @retval ORL_OMF  OMF object file.
 * @retval ORL_UNRECOGNIZED_FORMAT  Unrecognised format or read error.
 * @return One of the ORL_ELF / ORL_COFF / ORL_OMF /
 *         ORL_UNRECOGNIZED_FORMAT constants.
 */
orl_file_format ORLFileIdentify( orl_handle orl_hnd, void * file )
/****************************************************************/
{
    unsigned char *     magic;
    uint_16             machine_type;
    uint_16             offset;
    uint_16             len;
    unsigned char       chksum;

    magic = orl_hnd->funcs->read( file, 4 );
    if( magic == NULL ) {
        return ORL_UNRECOGNIZED_FORMAT;
    }
    if( orl_hnd->funcs->seek( file, -4, SEEK_CUR ) == -1 ) {
        return ORL_UNRECOGNIZED_FORMAT;
    }
    if( magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' &&
            magic[3] == 'F' ) {
        return ORL_ELF;
    }

    // See if this is the start of an OMF object file
    // the first record must be a THEADR record and we check that it is
    // valid, if it is then we assume that this is an OMF object file.
    if( magic[0] == CMD_THEADR ) {
        len = magic[1] | ( magic[2] << 8 );
        len -= (unsigned char)(magic[3]);
        len -= 2;
        if( !len ) {
            // This looks good so far, we must now check the record
            len = (unsigned char)(magic[3]) + 1;
            if( orl_hnd->funcs->seek( file, 4, SEEK_CUR ) == -1 ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            chksum = magic[0] + magic[1] + magic[2] + magic[3];
            magic = orl_hnd->funcs->read( file, len );
            if( orl_hnd->funcs->seek( file, -( 4 + len ), SEEK_CUR ) == -1 ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            if( magic ) {
                // Go on to check record checksum
                while( len ) {
                    chksum += (unsigned char)(*magic);
                    len--;
                    magic++;
                }
                magic--;
                if( !( *magic ) || !chksum ) {
                    // seems to be a correct OMF record to start the OBJ
                    return( ORL_OMF );
                }
            } else {
                magic = orl_hnd->funcs->read( file, 4 );
                if( magic == NULL ) {
                    return ORL_UNRECOGNIZED_FORMAT;
                } else if( orl_hnd->funcs->seek( file, -4, SEEK_CUR ) == -1 ) {
                    return ORL_UNRECOGNIZED_FORMAT;
                }
            }
        }
    }

    machine_type = *( (uint_16 *) magic );
    switch( machine_type ) {
    case IMAGE_FILE_MACHINE_I860:
    case IMAGE_FILE_MACHINE_I386A:
    case IMAGE_FILE_MACHINE_I386:
    case IMAGE_FILE_MACHINE_R3000:
    case IMAGE_FILE_MACHINE_R4000:
    case IMAGE_FILE_MACHINE_ALPHA:
    case IMAGE_FILE_MACHINE_POWERPC:
    case IMAGE_FILE_MACHINE_AMD64:
    case IMAGE_FILE_MACHINE_UNKNOWN:
        return ORL_COFF;
    }
    // Is it PE?
    if( magic[0] == 'M' && magic[1] == 'Z' ) {
        if( orl_hnd->funcs->seek( file, 0x3c, SEEK_CUR ) == -1 ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        magic = orl_hnd->funcs->read( file, 0x4 );
        if( magic == NULL ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        offset = *( (uint_16 *) magic );
        if( orl_hnd->funcs->seek( file, offset-0x40, SEEK_CUR ) == -1 ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        magic = orl_hnd->funcs->read( file, 4 );
        if( magic == NULL ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        if( magic[0]=='P' && magic[1] == 'E' && magic[2] == '\0' && magic[3] == '\0' ) {
            magic = orl_hnd->funcs->read( file, 4 );
            if( magic == NULL ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            if( orl_hnd->funcs->seek( file, -offset-8, SEEK_CUR ) == -1 ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            machine_type = *( (uint_16 *) magic );
            switch( machine_type ) {
            case IMAGE_FILE_MACHINE_I860:
            case IMAGE_FILE_MACHINE_I386:
            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
            case IMAGE_FILE_MACHINE_ALPHA:
            case IMAGE_FILE_MACHINE_POWERPC:
            case IMAGE_FILE_MACHINE_AMD64:
                return ORL_COFF;
            }
        }
    }
    return ORL_UNRECOGNIZED_FORMAT;
}

/*!
 * @brief Open an object file and build its in-memory structure.
 * @param[in,out] orl_hnd  ORL handle. Not NULL.
 * @param[in]     file     Caller's file object.
 * @param[in]     type     Format returned by ORLFileIdentify().
 * @retval NULL Allocation failure or malformed file.
 * @return The new file handle.
 */
orl_file_handle ORLENTRY ORLFileInit( orl_handle orl_hnd, void * file, orl_file_format type )
{
    orl_file_handle             orl_file_hnd;

    switch( type ) {
    case( ORL_ELF ):
        orl_file_hnd = (orl_file_handle) orl_hnd->funcs->alloc( sizeof( orl_file_handle_struct ) );
        if( !orl_file_hnd ) {
            orl_hnd->error = ORL_OUT_OF_MEMORY;
            return( NULL );
        }
        orl_file_hnd->type = ORL_ELF;
        orl_file_hnd->file_hnd.elf = ElfFileInit( orl_hnd->elf_hnd, file );
        if( orl_file_hnd->file_hnd.elf == NULL ) {
            orl_hnd->error = ORL_OUT_OF_MEMORY;
            orl_hnd->funcs->free( orl_file_hnd );
            return( NULL );
        }
        ORLAddFileLinks( orl_hnd, orl_file_hnd );
        return( orl_file_hnd );
    case( ORL_COFF ):
        orl_file_hnd = (orl_file_handle) orl_hnd->funcs->alloc( sizeof( orl_file_handle_struct ) );
        if( !orl_file_hnd ) return( NULL );
        orl_file_hnd->type = ORL_COFF;
        orl_file_hnd->file_hnd.coff = CoffFileInit( orl_hnd->coff_hnd, file );
        if( orl_file_hnd->file_hnd.coff == NULL ) {
            orl_hnd->error = ORL_OUT_OF_MEMORY;
            orl_hnd->funcs->free( orl_file_hnd );
            return( NULL );
        }
        ORLAddFileLinks( orl_hnd, orl_file_hnd );
        return( orl_file_hnd );
    case( ORL_OMF ):
        orl_file_hnd = (orl_file_handle) orl_hnd->funcs->alloc( sizeof( orl_file_handle_struct ) );
        if( !orl_file_hnd ) {
            orl_hnd->error = ORL_OUT_OF_MEMORY;
            return( NULL );
        }
        orl_file_hnd->type = ORL_OMF;
        orl_file_hnd->file_hnd.omf = OmfFileInit( orl_hnd->omf_hnd, file );
        if( orl_file_hnd->file_hnd.omf == NULL ) {
            orl_hnd->error = ORL_OUT_OF_MEMORY;
            orl_hnd->funcs->free( orl_file_hnd );
            return( NULL );
        }
        ORLAddFileLinks( orl_hnd, orl_file_hnd );
        return( orl_file_hnd );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Close a file handle and free it.
 * @param[in,out] orl_file_hnd  File handle. Not NULL.
 * @retval ORL_ERROR  The handle's @c type is not recognised.
 * @return Result of the backend close routine, or ORLRemoveFileLinks().
 */
orl_return ORLENTRY ORLFileFini( orl_file_handle orl_file_hnd )
{
    orl_return                          error = ORL_ERROR;
    /* jump table replace: */
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        error = ElfFileFini( orl_file_hnd->file_hnd.elf );
        break;
    case( ORL_COFF ):
        error = CoffFileFini( orl_file_hnd->file_hnd.coff );
        break;
    case( ORL_OMF ):
        error = OmfFileFini( orl_file_hnd->file_hnd.omf );
        break;
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }

    if( error != ORL_OKAY ) return( error );
    return( ORLRemoveFileLinks( orl_file_hnd ) );
}


/*!
 * @brief Get the RVA of the export table of a COFF file.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @return The export-table RVA, or 0 for non-COFF files.
 */
unsigned long ORLENTRY ORLExportTableRVA(orl_file_handle orl_file_hnd)
{
    unsigned long rva = 0L;

    if (orl_file_hnd->type == ORL_COFF) {
        rva = CoffExportTableRVA(orl_file_hnd->file_hnd.coff);
    }

    return rva;
}

/*!
 * @brief Scan sections (or sections matching a name) of a file.
 * @param[in,out] orl_file_hnd  File handle. Not NULL.
 * @param[in]     desired       Desired section name, or NULL for all.
 * @param[in]     return_func   Callback invoked for each matching section.
 * @retval ORL_ERROR  Unrecognised file type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLFileScan( orl_file_handle orl_file_hnd, char *desired, orl_sec_return_func return_func )
{
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        return( ElfFileScan( orl_file_hnd->file_hnd.elf, desired, return_func ) );
    case( ORL_COFF ):
        return( CoffFileScan( orl_file_hnd->file_hnd.coff, desired, return_func ) );
    case( ORL_OMF ):
        return( OmfFileScan( orl_file_hnd->file_hnd.omf, desired, return_func ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}

/*!
 * @brief Get the machine type of a file.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @return The machine type, or 0 for unrecognised file types.
 */
orl_machine_type ORLENTRY ORLFileGetMachineType( orl_file_handle orl_file_hnd )
{
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        return( ElfFileGetMachineType( orl_file_hnd->file_hnd.elf ) );
    case( ORL_COFF ):
        return( CoffFileGetMachineType( orl_file_hnd->file_hnd.coff ) );
    case( ORL_OMF ):
        return( OmfFileGetMachineType( orl_file_hnd->file_hnd.omf ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the file flags of a file.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @return The file flags, or 0 for unrecognised file types.
 */
orl_file_flags ORLENTRY ORLFileGetFlags( orl_file_handle orl_file_hnd )
{
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        return( ElfFileGetFlags( orl_file_hnd->file_hnd.elf ) );
    case( ORL_COFF ):
        return( CoffFileGetFlags( orl_file_hnd->file_hnd.coff ) );
    case( ORL_OMF ):
        return( OmfFileGetFlags( orl_file_hnd->file_hnd.omf ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the file size of a file.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @return The file size in bytes, or 0 for unrecognised file types.
 */
orl_file_size ORLENTRY ORLFileGetSize( orl_file_handle orl_file_hnd )
{
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        return( ElfFileGetSize( orl_file_hnd->file_hnd.elf ) );
    case( ORL_COFF ):
        return( CoffFileGetSize( orl_file_hnd->file_hnd.coff ) );
    case( ORL_OMF ):
        return( OmfFileGetSize( orl_file_hnd->file_hnd.omf ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the file type of a file.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @return The file type, or 0 for unrecognised file types.
 */
orl_file_type ORLENTRY ORLFileGetType( orl_file_handle orl_file_hnd )
{
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        return( ElfFileGetType( orl_file_hnd->file_hnd.elf ) );
    case( ORL_COFF ):
        return( CoffFileGetType( orl_file_hnd->file_hnd.coff ) );
    case( ORL_OMF ):
        return( OmfFileGetType( orl_file_hnd->file_hnd.omf ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the format of a file handle.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @return The ORL_ELF / ORL_COFF / ORL_OMF format constant.
 */
orl_file_format ORLENTRY ORLFileGetFormat( orl_file_handle orl_file_hnd )
{
    return( orl_file_hnd->type );
}

/*!
 * @brief Get the symbol table section of a file.
 * @param[in] orl_file_hnd  File handle. Not NULL.
 * @retval NULL File has no symbol table, or the type is unrecognised.
 * @return The symbol table section handle.
 */
orl_sec_handle ORLENTRY ORLFileGetSymbolTable( orl_file_handle orl_file_hnd )
{
    switch( orl_file_hnd->type ) {
    case( ORL_ELF ):
        return( (orl_sec_handle) ElfFileGetSymbolTable( orl_file_hnd->file_hnd.elf ) );
    case( ORL_COFF ):
        return( (orl_sec_handle) CoffFileGetSymbolTable( orl_file_hnd->file_hnd.coff ) );
    case( ORL_OMF ):
        return( (orl_sec_handle) OmfFileGetSymbolTable( orl_file_hnd->file_hnd.omf ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the name of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL Section has no name, or the type is unrecognised.
 * @return NUL-terminated section name.
 */
char * ORLENTRY ORLSecGetName( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetName( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( CoffSecGetName( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetName( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the base offset of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @return The base offset, or 0 for unrecognised section types.
 */
orl_sec_offset ORLENTRY ORLSecGetBase( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetBase( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( CoffSecGetBase( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetBase( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the declared size of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @return The section size in bytes, or 0 for unrecognised types.
 */
orl_sec_size ORLENTRY ORLSecGetSize( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetSize( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( CoffSecGetSize( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetSize( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the type of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @return The section type, or 0 for unrecognised types.
 */
orl_sec_type ORLENTRY ORLSecGetType( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetType( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( CoffSecGetType( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetType( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the alignment of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @return Log2 of the alignment in bytes, or 0 for unrecognised types.
 */
orl_sec_alignment ORLENTRY ORLSecGetAlignment( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetAlignment( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( CoffSecGetAlignment( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetAlignment( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the flags of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @return The section flags, or 0 for unrecognised types.
 */
orl_sec_flags ORLENTRY ORLSecGetFlags( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetFlags( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( CoffSecGetFlags( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetFlags( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the string table section linked to a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL Section has no associated string table.
 * @return The string table section handle, or NULL.
 */
orl_sec_handle ORLENTRY ORLSecGetStringTable( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( (orl_sec_handle) ElfSecGetStringTable( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( (orl_sec_handle) CoffSecGetStringTable( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( (orl_sec_handle) OmfSecGetStringTable( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the symbol table section linked to a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL Section has no associated symbol table.
 * @return The symbol table section handle, or NULL.
 */
orl_sec_handle ORLENTRY ORLSecGetSymbolTable( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( (orl_sec_handle) ElfSecGetSymbolTable( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( (orl_sec_handle) CoffSecGetSymbolTable( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( (orl_sec_handle) OmfSecGetSymbolTable( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the reloc table section linked to a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL Section has no associated reloc table.
 * @return The reloc section handle, or NULL.
 */
orl_sec_handle ORLENTRY ORLSecGetRelocTable( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( (orl_sec_handle) ElfSecGetRelocTable( (elf_sec_handle) orl_sec_hnd ) );
    case( ORL_COFF ):
        return( (orl_sec_handle) CoffSecGetRelocTable( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( (orl_sec_handle) OmfSecGetRelocTable( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the line-number array of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL No line numbers, or the type is unrecognised.
 * @return Pointer to the line-number array.
 */
orl_linnum * ORLENTRY ORLSecGetLines( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( NULL );
    case( ORL_COFF ):
        return( CoffSecGetLines( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetLines( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return NULL;
}

/*!
 * @brief Get the number of line numbers attached to a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval 0  Section type does not carry line numbers.
 * @return The number of line numbers, or 0 for unrecognised types.
 */
orl_table_index ORLENTRY ORLSecGetNumLines( orl_sec_handle orl_sec_hnd )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( 0 );
    case( ORL_COFF ):
        return( CoffSecGetNumLines( (coff_sec_handle) orl_sec_hnd ) );
    case( ORL_OMF ):
        return( OmfSecGetNumLines( (omf_sec_handle) orl_sec_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return 0;
}

/*!
 * @brief Get the file offset of a COFF section's raw data.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval 0  Non-COFF section.
 * @return The raw-data file offset, or 0 for non-COFF sections.
 */
orl_sec_offset ORLENTRY ORLSecGetOffset( orl_sec_handle orl_sec_hnd )
{
    if( orl_sec_hnd->type == ORL_COFF ) {
        return( CoffSecGetOffset( (coff_sec_handle) orl_sec_hnd ) );
    }
    return 0;
}

/*!
 * @brief Get the OMF class name of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL  Non-OMF section.
 * @return NUL-terminated class name, or NULL.
 */
char * ORLENTRY ORLSecGetClassName( orl_sec_handle orl_sec_hnd )
{
    if( orl_sec_hnd->type == ORL_OMF ) {
        return( OmfSecGetClassName( (omf_sec_handle) orl_sec_hnd ) );
    }
    return NULL;
}

/*!
 * @brief Get the OMF combine attribute of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval ORL_SEC_COMBINE_NONE Non-OMF section.
 * @return The ORL combine value.
 */
orl_sec_combine ORLENTRY ORLSecGetCombine( orl_sec_handle orl_sec_hnd )
{
    if( orl_sec_hnd->type == ORL_OMF ) {
        return( OmfSecGetCombine( (omf_sec_handle) orl_sec_hnd ) );
    }
    return ORL_SEC_COMBINE_NONE;
}

/*!
 * @brief Get the OMF absolute frame of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval ORL_SEC_NO_ABS_FRAME Non-OMF section.
 * @return The absolute frame value.
 */
orl_sec_frame ORLENTRY ORLSecGetAbsFrame( orl_sec_handle orl_sec_hnd )
{
    if( orl_sec_hnd->type == ORL_OMF ) {
        return( OmfSecGetAbsFrame( (omf_sec_handle) orl_sec_hnd ) );
    }
    return ORL_SEC_NO_ABS_FRAME;
}

/*!
 * @brief Get the OMF COMDAT-associated section of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL Non-OMF section, or section is not a COMDAT.
 * @return The associated section handle, or NULL.
 */
orl_sec_handle ORLENTRY ORLSecGetAssociated( orl_sec_handle orl_sec_hnd )
{
    if( orl_sec_hnd->type == ORL_OMF ) {
        return( OmfSecGetAssociated( (omf_sec_handle) orl_sec_hnd ) );
    }
    return NULL;
}

/*!
 * @brief Get the OMF group of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @retval NULL Non-OMF section, or section is not grouped.
 * @return The group handle, or NULL.
 */
orl_group_handle ORLENTRY ORLSecGetGroup( orl_sec_handle orl_sec_hnd )
{
    if( orl_sec_hnd->type == ORL_OMF ) {
        return( OmfSecGetGroup( (omf_sec_handle) orl_sec_hnd ) );
    }
    return NULL;
}

/*!
 * @brief Get the contents buffer of a section.
 * @param[in,out] orl_sec_hnd  Section handle. Not NULL.
 * @param[out]    buffer       Receiver for the contents pointer. Not NULL.
 * @retval ORL_OKAY   Contents available.
 * @retval ORL_ERROR  Section has no contents, or the type is unrecognised.
 * @return ORL_OKAY on success.
 */
orl_return ORLENTRY ORLSecGetContents( orl_sec_handle orl_sec_hnd, char **buffer )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecGetContents( (elf_sec_handle) orl_sec_hnd, buffer ) );
    case( ORL_COFF ):
        return( CoffSecGetContents( (coff_sec_handle) orl_sec_hnd, buffer ) );
    case( ORL_OMF ):
        return( OmfSecGetContents( (omf_sec_handle) orl_sec_hnd, buffer ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}

/*!
 * @brief Scan relocations of a section at a specific offset.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @param[in] sec_offset   Offset to match.
 * @param[in] return_func  Callback invoked for each matching relocation.
 * @retval ORL_ERROR  Unrecognised section type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLSecQueryReloc( orl_sec_handle orl_sec_hnd, orl_sec_offset sec_offset, orl_reloc_return_func return_func )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecQueryReloc( (elf_sec_handle) orl_sec_hnd, sec_offset, return_func ) );
    case( ORL_COFF ):
        return( CoffSecQueryReloc( (coff_sec_handle) orl_sec_hnd, sec_offset, return_func ) );
    case( ORL_OMF ):
        return( OmfSecQueryReloc( (omf_sec_handle) orl_sec_hnd, sec_offset, return_func ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}

/*!
 * @brief Scan all relocations of a section.
 * @param[in] orl_sec_hnd  Section handle. Not NULL.
 * @param[in] return_func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Unrecognised section type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLSecScanReloc( orl_sec_handle orl_sec_hnd, orl_reloc_return_func return_func )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSecScanReloc( (elf_sec_handle) orl_sec_hnd, return_func ) );
    case( ORL_COFF ):
        return( CoffSecScanReloc( (coff_sec_handle) orl_sec_hnd, return_func ) );
    case( ORL_OMF ):
        return( OmfSecScanReloc( (omf_sec_handle) orl_sec_hnd, return_func ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}


/*!
 * @brief Convert a section handle to its format-specific index.
 * @param[in] shdl  Section handle. Not NULL.
 * @return The section index, or 0 for unrecognised types.
 */
orl_table_index ORLENTRY ORLCvtSecHdlToIdx( orl_sec_handle shdl )
/***************************************************************/
{
    switch( shdl->type ) {
    case( ORL_ELF ):
        return ElfCvtSecHdlToIdx( (elf_sec_handle) shdl );
    case( ORL_COFF ):
        return CoffCvtSecHdlToIdx( (coff_sec_handle) shdl );
    case( ORL_OMF ):
        return OmfCvtSecHdlToIdx( (omf_sec_handle) shdl );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Convert a format-specific index to a section handle.
 * @param[in] fhdl  File handle. Not NULL.
 * @param[in] idx   Section index.
 * @retval NULL Index not found or unrecognised file type.
 * @return The matching section handle, or NULL.
 */
orl_sec_handle ORLENTRY ORLCvtIdxToSecHdl( orl_file_handle fhdl,
                                           orl_table_index idx )
/**************************************************************/
{
    switch( fhdl->type ) {
    case( ORL_ELF ):
        return (orl_sec_handle) ElfCvtIdxToSecHdl( fhdl->file_hnd.elf, idx );
    case( ORL_COFF ):
        return (orl_sec_handle) CoffCvtIdxToSecHdl( fhdl->file_hnd.coff, idx );
    case( ORL_OMF ):
        return (orl_sec_handle) OmfCvtIdxToSecHdl( fhdl->file_hnd.omf, idx );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Scan a reloc section's relocation entries.
 * @param[in] orl_sec_hnd  Reloc section handle. Not NULL.
 * @param[in] return_func  Callback invoked for each relocation.
 * @retval ORL_ERROR  Unrecognised section type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLRelocSecScan( orl_sec_handle orl_sec_hnd, orl_reloc_return_func return_func )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfRelocSecScan( (elf_sec_handle) orl_sec_hnd, return_func ) );
    case( ORL_COFF ):
        return( CoffRelocSecScan( (coff_sec_handle) orl_sec_hnd, return_func ) );
    case( ORL_OMF ):
        return( OmfRelocSecScan( (omf_sec_handle) orl_sec_hnd, return_func ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}

/*!
 * @brief Scan a symbol table section's symbols.
 * @param[in] orl_sec_hnd  Symbol table section handle. Not NULL.
 * @param[in] return_func  Callback invoked for each symbol.
 * @retval ORL_ERROR  Unrecognised section type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLSymbolSecScan( orl_sec_handle orl_sec_hnd, orl_symbol_return_func return_func )
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSymbolSecScan( (elf_sec_handle) orl_sec_hnd, return_func ) );
    case( ORL_COFF ):
        return( CoffSymbolSecScan( (coff_sec_handle) orl_sec_hnd, return_func ) );
    case( ORL_OMF ):
        return( OmfSymbolSecScan( (omf_sec_handle) orl_sec_hnd, return_func ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}

/*!
 * @brief Scan a note section's comment records.
 * @param[in] orl_sec_hnd  Note section handle. Not NULL.
 * @param[in] fn           Note callback table. Not NULL.
 * @param[in] cookie       Callback cookie.
 * @retval ORL_ERROR  Unrecognised section type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLNoteSecScan( orl_sec_handle orl_sec_hnd,
                                    orl_note_callbacks * fn, void *cookie )
/*************************************************************************/
{
    switch( orl_sec_hnd->type ) {
    case( ORL_ELF ):
        return( ElfNoteSecScan( (elf_sec_handle) orl_sec_hnd, fn, cookie ) );
    case( ORL_COFF ):
        return( CoffNoteSecScan( (coff_sec_handle) orl_sec_hnd, fn, cookie ) );
    case( ORL_OMF ):
        return( OmfNoteSecScan( (omf_sec_handle) orl_sec_hnd, fn, cookie ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}


/*!
 * @brief Get the name of a symbol.
 * @param[in] orl_symbol_hnd  Symbol handle. Not NULL.
 * @retval NULL Symbol has no name, or the type is unrecognised.
 * @return NUL-terminated symbol name.
 */
char * ORLENTRY ORLSymbolGetName( orl_symbol_handle orl_symbol_hnd )
{
    switch( orl_symbol_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSymbolGetName( (elf_symbol_handle) orl_symbol_hnd ) );
    case( ORL_COFF ):
        return( CoffSymbolGetName( (coff_symbol_handle) orl_symbol_hnd ) );
    case( ORL_OMF ):
        return( OmfSymbolGetName( (omf_symbol_handle) orl_symbol_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the value (offset) of a symbol.
 * @param[in] orl_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol value, or 0 for unrecognised types.
 */
orl_symbol_value ORLENTRY ORLSymbolGetValue( orl_symbol_handle orl_symbol_hnd )
{
    switch( orl_symbol_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSymbolGetValue( (elf_symbol_handle) orl_symbol_hnd ) );
    case( ORL_COFF ):
        return( CoffSymbolGetValue( (coff_symbol_handle) orl_symbol_hnd ) );
    case( ORL_OMF ):
        return( OmfSymbolGetValue( (omf_symbol_handle) orl_symbol_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the binding of a symbol.
 * @param[in] orl_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol binding, or 0 for unrecognised types.
 */
orl_symbol_binding ORLENTRY ORLSymbolGetBinding( orl_symbol_handle orl_symbol_hnd )
{
    switch( orl_symbol_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSymbolGetBinding( (elf_symbol_handle) orl_symbol_hnd ) );
    case( ORL_COFF ):
        return( CoffSymbolGetBinding( (coff_symbol_handle) orl_symbol_hnd ) );
    case( ORL_OMF ):
        return( OmfSymbolGetBinding( (omf_symbol_handle) orl_symbol_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the type flags of a symbol.
 * @param[in] orl_symbol_hnd  Symbol handle. Not NULL.
 * @return The symbol type flags, or 0 for unrecognised types.
 */
orl_symbol_type ORLENTRY ORLSymbolGetType( orl_symbol_handle orl_symbol_hnd )
{
    switch( orl_symbol_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSymbolGetType( (elf_symbol_handle) orl_symbol_hnd ) );
    case( ORL_COFF ):
        return( CoffSymbolGetType( (coff_symbol_handle) orl_symbol_hnd ) );
    case( ORL_OMF ):
        return( OmfSymbolGetType( (omf_symbol_handle) orl_symbol_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the raw info byte of a symbol.
 * @param[in] orl_symbol_hnd  Symbol handle. Not NULL.
 * @retval 0  COFF symbols have no raw info, or unrecognised symbol type.
 * @return The raw info byte, or 0 for unrecognised types.
 */
unsigned char ORLENTRY ORLSymbolGetRawInfo( orl_symbol_handle orl_symbol_hnd )
{
    switch( orl_symbol_hnd->type ) {
    case( ORL_ELF ):
        return( ElfSymbolGetRawInfo( (elf_symbol_handle) orl_symbol_hnd ) );
    case( ORL_COFF ):
        return 0; // no COFF equivilent
    case( ORL_OMF ):
        return( OmfSymbolGetRawInfo( (omf_symbol_handle) orl_symbol_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( 0 );
}

/*!
 * @brief Get the section that contains a symbol.
 * @param[in] orl_symbol_hnd  Symbol handle. Not NULL.
 * @retval NULL Symbol not bound to a section, or type unrecognised.
 * @return The containing section handle, or NULL.
 */
orl_sec_handle ORLENTRY ORLSymbolGetSecHandle( orl_symbol_handle orl_symbol_hnd )
{
    switch( orl_symbol_hnd->type ) {
    case( ORL_ELF ):
        return( (orl_sec_handle) ElfSymbolGetSecHandle( (elf_symbol_handle) orl_symbol_hnd ) );
    case( ORL_COFF ):
        return( (orl_sec_handle) CoffSymbolGetSecHandle( (coff_symbol_handle) orl_symbol_hnd ) );
    case( ORL_OMF ):
        return( (orl_sec_handle) OmfSymbolGetSecHandle( (omf_symbol_handle) orl_symbol_hnd ) );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Get the symbol referenced by a weak-external symbol.
 * @param[in] hnd  Weak-external symbol handle. Not NULL.
 * @retval NULL OMF symbols have no separate weak target.
 * @return The associated symbol handle.
 */
orl_symbol_handle ORLENTRY ORLSymbolGetAssociated( orl_symbol_handle hnd )
{
    switch( hnd->type ) {
    case( ORL_ELF ):
        return (orl_symbol_handle) ElfSymbolGetAssociated( (elf_symbol_handle) hnd );
    case( ORL_COFF ):
        return (orl_symbol_handle) CoffSymbolGetAssociated( (coff_symbol_handle) hnd );
    case( ORL_OMF ):
        return NULL;    // NYI: call to an OMF func. here.
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( NULL );
}

/*!
 * @brief Scan all groups of a file.
 * @param[in] hnd   File handle. Not NULL.
 * @param[in] func  Callback invoked for each group.
 * @retval ORL_OKAY   ELF/COFF files have no groups.
 * @retval ORL_ERROR  Unrecognised file type.
 * @return The backend scan result.
 */
orl_return ORLENTRY ORLGroupsScan( orl_file_handle hnd,
                                   orl_group_return_func func )
{
    switch( hnd->type ) {
    case( ORL_ELF ):
    case( ORL_COFF ):
        return ORL_OKAY;
    case( ORL_OMF ):
        return OmfGroupsScan( hnd->file_hnd.omf, func );
    default: break;//ORL_UNRECOGNIZED_FORMAT
    }
    return( ORL_ERROR );
}

/*!
 * @brief Get the name of a group.
 * @param[in] hnd  Group handle. Not NULL.
 * @retval NULL Non-OMF group handle.
 * @return NUL-terminated group name.
 */
char * ORLENTRY ORLGroupName( orl_group_handle hnd )
{
    if( hnd->type == ORL_OMF ) {
        return OmfGroupName( (omf_grp_handle)hnd );
    }
    return( NULL );
}

/*!
 * @brief Get the number of members of a group.
 * @param[in] hnd  Group handle. Not NULL.
 * @return The number of member segments, or 0 for non-OMF groups.
 */
orl_table_index ORLENTRY ORLGroupSize( orl_group_handle hnd )
{
    if( hnd->type == ORL_OMF ) {
        return OmfGroupSize( (omf_grp_handle)hnd );
    }
    return( 0 );
}

/*!
 * @brief Get the name of a group member.
 * @param[in] hnd  Group handle. Not NULL.
 * @param[in] idx  Member index (zero-based).
 * @retval NULL Non-OMF group handle, or member not found.
 * @return NUL-terminated member name.
 */
char * ORLENTRY ORLGroupMember( orl_group_handle hnd, orl_table_index idx )
{
    if( hnd->type == ORL_OMF ) {
        return OmfGroupMember( (omf_grp_handle)hnd, idx );
    }
    return( NULL );
}
