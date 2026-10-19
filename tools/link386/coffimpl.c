/**************************************************************************
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
* Description:  Emulate long format of import library members in memory.
*               It is derived from short format of the import library members
*               in source .LIB file for each entry.
*               Emulated data is saved into coff_file_hnd structure.
*
**************************************************************************/

/*!
 * @file coffimpl.c
 * @brief In-memory emulation of long-format COFF import library members.
 *
 * Each short import library member encountered while reading a COFF
 * input file is expanded on the fly into an equivalent long-format
 * COFF object.  The resulting bytes are cached in
 * @c coff_file_hnd->implib_data and replayed to the normal COFF reader
 * via the ImportLibData callback table.
 */

#include <stdlib.h>
#include <stdio.h>
#include "walloca.h"

#include "coffimpl.h"

/*!
 * @def MAX_NUM_COFF_LIB_SECTIONS
 * @brief Maximum number of sections in an emulated import library member.
 *
 * Note before you use this for anything make sure that the values below
 * are large enough for your purposes (particularly the num sections and
 * symbols).  I put in some safety code in the string table because if the
 * user declares long function names (very long like > 512) it is possible
 * that 1k will not suffice for the string table size in import libraries.
 * The number of sections and symbols however are fixed and suffice for
 * import libraries.
 */
#define MAX_NUM_COFF_LIB_SECTIONS       8
/*!
 * @def MAX_NUM_COFF_LIB_SYMBOLS
 * @brief Maximum number of symbols in an emulated import library member.
 */
#define MAX_NUM_COFF_LIB_SYMBOLS        32
/*!
 * @def INIT_MAX_SIZE_COFF_STRING_TABLE
 * @brief Initial string-table capacity (grows as needed).
 */
#define INIT_MAX_SIZE_COFF_STRING_TABLE 1024

/*!
 * @struct coff_lib_file
 * @brief Scratch state used while building an emulated COFF object.
 */
typedef struct {
    coff_file_header    header;         /*!< File header being built. */
    coff_section_header section[MAX_NUM_COFF_LIB_SECTIONS]; /*!< Section headers. */
    coff_symbol         symbol[MAX_NUM_COFF_LIB_SYMBOLS];   /*!< Symbol records. */
    unsigned_32         string_table_size;      /*!< Current string table size. */
    unsigned_32         max_string_table_size;  /*!< Allocated string table size. */
    char                *string_table;          /*!< String table buffer. */
    coff_file_handle    coff_file_hnd;          /*!< Owner COFF file handle. */
} coff_lib_file;

/*!
 * @enum importType
 * @brief Import member naming convention.
 */
typedef enum {
    IMPORT_DESCRIPTOR,      /*!< Import descriptor record. */
    NULL_IMPORT_DESCRIPTOR, /*!< Null import descriptor. */
    NULL_THUNK_DATA,        /*!< Null thunk data. */
    ORDINAL,                /*!< Import by ordinal and name. */
    NAMED,                  /*!< Import by name only. */
    ELF,                    /*!< ELF-style name only or name and ordinal. */
    ELFRENAMED,             /*!< ELF renamed entry. */
} importType;

/*!
 * @struct import_sym
 * @brief One short-format import library member, decoded.
 */
typedef struct {
    importType  type;           /*!< Naming convention. */
    short       processor;      /*!< Target CPU. */
    char        *DLLName;       /*!< Source DLL name. */
    long        ordinal;        /*!< Import ordinal. */
    char        *symName;       /*!< Imported symbol name. */
    char        *exportedName;  /*!< Exported (public) name. */
    uint_32     time_date_stamp;/*!< COFF time-date stamp. */
} import_sym;

//IMPORT_DESCRIPT optional header

/*!
 * @brief Alpha import thunk code (12 bytes).
 */
static unsigned_8 CoffImportAxpText[] = {
 0x00,0x00,0x7F,0x27,0x00,0x00,0x7B,0xA3,0x00,0x00,0xFB,0x6B
};

/*!
 * @brief PowerPC import thunk code (24 bytes).
 */
static unsigned_8 CoffImportPpcText[] = {
 0x00,0x00,0x62,0x81,0x00,0x00,0x8B,0x81,0x04,0x00,0x41,0x90,0xA6,0x03,0x89,0x7D,
 0x04,0x00,0x4B,0x80,0x20,0x04,0x80,0x4E
};

/*!
 * @brief PowerPC `.pdata` unwind entry (20 bytes).
 */
static unsigned_8 CoffImportPpcPdata[] = {
 0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
 0x0D,0x00,0x00,0x00
};

/*!
 * @brief x86 import thunk code (6 bytes).
 */
static unsigned_8 CoffImportX86Text[] = {
 0xFF,0x25,0x00,0x00,0x00,0x00
};

/*!
 * @brief Initialise scratch state for an emulated COFF object.
 * @param[in,out] c_file  Scratch structure. Not NULL.
 */
static void InitCoffFile( coff_lib_file *c_file )
{
    c_file->string_table = _ClientAlloc( c_file->coff_file_hnd, INIT_MAX_SIZE_COFF_STRING_TABLE );
    c_file->max_string_table_size = INIT_MAX_SIZE_COFF_STRING_TABLE;

}

/*!
 * @brief Initialise an emulated COFF file header.
 * @param[in,out] c_file       Scratch structure. Not NULL.
 * @param[in]     processor    Target CPU.
 * @param[in]     time_stamp   COFF time-date stamp.
 * @param[in]     opt_hdr_size Size of the optional header.
 */
static void SetCoffFile( coff_lib_file *c_file, short processor,
     unsigned_32 time_stamp, unsigned_16 opt_hdr_size)
{
    c_file->header.cpu_type = processor;
    c_file->header.num_sections = 0;
    c_file->header.time_stamp = time_stamp;
    c_file->header.num_symbols = 0;
    c_file->header.opt_hdr_size = opt_hdr_size;
    c_file->header.flags = IMAGE_FILE_32BIT_MACHINE;
    c_file->string_table_size = 0;
}

/*!
 * @brief Release scratch state for an emulated COFF object.
 * @param[in,out] c_file  Scratch structure. Not NULL.
 */
static void FiniCoffLibFile( coff_lib_file *c_file )
{
    _ClientFree( c_file->coff_file_hnd, c_file->string_table );
}

/*!
 * @brief Append a NUL-terminated string to the string table.
 *
 * Grows the string table by doubling when needed.
 *
 * @param[in,out] c_file  Scratch structure. Not NULL.
 * @param[in]     name    String to append. Not NULL.
 * @param[in]     len     Length excluding the terminating NUL.
 */
static void AddCoffString( coff_lib_file  *c_file, char *name, int len )
{
    char    *x;

    len++;
    if( ( c_file->string_table_size + len ) >= c_file->max_string_table_size ) {
        c_file->max_string_table_size *= 2;
        x = _ClientAlloc( c_file->coff_file_hnd, c_file->max_string_table_size );
        if( x == NULL )
            return;
        memcpy( x, c_file->string_table, c_file->string_table_size );
        _ClientFree( c_file->coff_file_hnd, c_file->string_table );
        c_file->string_table = x;
    }
    memcpy( c_file->string_table + c_file->string_table_size, name, len );
    c_file->string_table_size += len;
}

/*!
 * @brief Append a section header to an emulated COFF object.
 *
 * Section numbering has index base 1.
 *
 * @param[in,out] c_file      Scratch structure. Not NULL.
 * @param[in]     name        Section name. Not NULL.
 * @param[in]     size        Section size in bytes.
 * @param[in]     num_relocs  Number of relocations.
 * @param[in]     flags       COFF section flags.
 * @return The one-based index of the new section.
 */
static int AddCoffSection( coff_lib_file *c_file, char *name, unsigned_32 size,
    unsigned_16 num_relocs, unsigned_32 flags )
{
    coff_section_header     *section;
    int                     len;

// Sections numbering has index base 1

    section = c_file->section + c_file->header.num_sections;
    len = strlen( name );
    memset( section, 0, COFF_SECTION_HEADER_SIZE );
    if( len > COFF_SEC_NAME_LEN ) {
        section->name[0] = '/';
        ultoa( c_file->string_table_size + 4, section->name + 1, 10 );
        AddCoffString( c_file, name, len );
    } else {
        memcpy( section->name, name, len );
    }
    section->size = size;
    section->num_relocs = num_relocs;
    section->flags = flags;
    return ++(c_file->header.num_sections);
}

/*!
 * @brief Append a symbol record to an emulated COFF object.
 *
 * Symbol numbering has index base 0.
 *
 * @param[in,out] c_file     Scratch structure. Not NULL.
 * @param[in]     name       Symbol name. Not NULL.
 * @param[in]     value      Symbol value (offset).
 * @param[in]     sec_num    One-based section number, or a special value.
 * @param[in]     type       COFF symbol type.
 * @param[in]     class      COFF storage class.
 * @param[in]     num_aux    Number of auxiliary records that follow.
 * @return The zero-based index of the new symbol.
 */
static int AddCoffSymbol( coff_lib_file *c_file, char *name, unsigned_32 value,
    signed_16 sec_num,  unsigned_16 type, unsigned_8 class, unsigned_8 num_aux )
{
    coff_symbol *sym;
    int         len;

// Symbols numbering has index base 0

    sym = c_file->symbol + c_file->header.num_symbols;
    len = strlen( name );
    if( len > COFF_SYM_NAME_LEN ) {
        sym->name.non_name.zeros = 0;
        sym->name.non_name.offset = c_file->string_table_size + 4;
        AddCoffString( c_file, name, len );
    } else {
        memset( sym->name.name_string, 0, COFF_SYM_NAME_LEN );
        memcpy( sym->name.name_string, name, len );
    }
    sym->value = value;
    sym->sec_num = sec_num;
    sym->type = type;
    sym->storage_class = class;
    sym->num_aux = num_aux;
    return c_file->header.num_symbols++;
}

/*!
 * @brief Append a section symbol (with auxiliary record) to the object.
 * @param[in,out] c_file     Scratch structure. Not NULL.
 * @param[in]     selection  COMDAT selection code.
 * @param[in]     sec        One-based section index.
 * @return The zero-based index of the section symbol.
 */
static int AddCoffSymSec( coff_lib_file *c_file, unsigned_8 selection, int sec )
{
    coff_sym_section    *sym;
    char                name[9];
    coff_section_header *section;
    int                 symbol_no;

    section = c_file->section + sec - 1;
    memcpy( name, section->name, 8 );
    name[8]='\0';
    symbol_no = AddCoffSymbol( c_file, name, 0x0, sec, IMAGE_SYM_TYPE_NULL,
        IMAGE_SYM_CLASS_STATIC, 1 );
    sym = (coff_sym_section *) (c_file->symbol + c_file->header.num_symbols);
    sym->length = section->size;
    sym->num_relocs = section->num_relocs;
    sym->num_line_numbers = 0;
    sym->checksum = 0;
    if( selection == IMAGE_COMDAT_SELECT_ASSOCIATIVE ) {
        sym->number = sec;
    } else {
        sym->number = 0;
    }
    sym->selection = selection;
    c_file->header.num_symbols++;
    return symbol_no;
}

/*!
 * @def IMPLIB_LEN
 * @brief Total byte length of the emulated import library buffer.
 */
#define IMPLIB_LEN      (*((int*)coff_file_hnd->implib_data))
/*!
 * @def IMPLIB_POS
 * @brief Current read offset within the emulated import library buffer.
 */
#define IMPLIB_POS      (*((long*)(coff_file_hnd->implib_data + sizeof(int))))
/*!
 * @def IMPLIB_DATA
 * @brief Pointer to the first byte of the emulated import library payload.
 */
#define IMPLIB_DATA     (coff_file_hnd->implib_data + sizeof(int) + sizeof(long))

/*!
 * @brief Allocate the emulated import library buffer if necessary.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @retval ORL_OKAY            Buffer ready.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
static int DataImpLibInit( coff_file_handle coff_file_hnd )
{
    if (coff_file_hnd->implib_data == NULL) {
        coff_file_hnd->implib_data = _ClientAlloc( coff_file_hnd, sizeof( int ) + sizeof( long ) );
        if( coff_file_hnd->implib_data == NULL )
            return( ORL_OUT_OF_MEMORY );
        IMPLIB_LEN = sizeof(int) + sizeof(long);
        IMPLIB_POS = 0;
    }
    return( ORL_OKAY );
}

/*!
 * @brief Append a chunk of data to the emulated import library buffer.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     buff           Data to append. Not NULL.
 * @param[in]     len            Length in bytes.
 * @retval ORL_OKAY            Appended.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
static int AddDataImpLib( coff_file_handle coff_file_hnd, void * buff, int len )
{
    char    *x;
        
    x = _ClientAlloc( coff_file_hnd, IMPLIB_LEN + len );
    if( x == NULL )
        return( ORL_OUT_OF_MEMORY );
    memcpy( x, coff_file_hnd->implib_data, IMPLIB_LEN );
    memcpy( x + IMPLIB_LEN, buff, len);
    _ClientFree( coff_file_hnd, coff_file_hnd->implib_data );
    coff_file_hnd->implib_data = x;
    IMPLIB_LEN += len;
    return( ORL_OKAY );
}

/*!
 * @brief Read callback for the emulated import library.
 * @param[in] _coff_file_hnd  COFF file handle (as void pointer).
 * @param[in] len             Number of bytes to read.
 * @return Pointer to the read data.
 */
static void * ImportLibRead(void * _coff_file_hnd, size_t len)
{
    coff_file_handle coff_file_hnd = _coff_file_hnd;
    IMPLIB_POS += len;
    return IMPLIB_DATA + IMPLIB_POS - len;
}

/*!
 * @brief Seek callback for the emulated import library.
 * @param[in] _coff_file_hnd  COFF file handle (as void pointer).
 * @param[in] pos             Offset to seek to.
 * @param[in] where           One of SEEK_SET, SEEK_CUR, SEEK_END.
 * @return The new read position.
 */
static long ImportLibSeek(void * _coff_file_hnd, long pos, int where)
{
    coff_file_handle coff_file_hnd = _coff_file_hnd;
    if( where == SEEK_SET ) {
        IMPLIB_POS = pos;
    } else if( where == SEEK_CUR ) {
        IMPLIB_POS += pos;
    } else {
        IMPLIB_POS = IMPLIB_LEN - sizeof(int) - sizeof(long) - pos;
    }
    return IMPLIB_POS;
}


/*!
 * @brief Append the constructed file header to the emulated object.
 *
 * Also computes raw-data and reloc pointers for each section.
 *
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     c_file         Scratch structure. Not NULL.
 */
static void CreateCoffFileHeader( coff_file_handle coff_file_hnd, coff_lib_file *c_file )
{
    unsigned    i;
    unsigned_32 d_ptr;

    d_ptr = sizeof(coff_file_header) + c_file->header.opt_hdr_size + c_file->header.num_sections * COFF_SECTION_HEADER_SIZE;
    for( i = 0; i < c_file->header.num_sections; i++ ) {
        c_file->section[i].rawdata_ptr = d_ptr;
        d_ptr += c_file->section[i].size;
        c_file->section[i].reloc_ptr = d_ptr;
        d_ptr += c_file->section[i].num_relocs * COFF_RELOC_SIZE;
    }
    c_file->header.sym_table = d_ptr;
    AddDataImpLib( coff_file_hnd, &( c_file->header ), sizeof(coff_file_header) );
}

/*!
 * @brief Append the section header table to the emulated object.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     c_file         Scratch structure. Not NULL.
 */
static void CreateCoffSections( coff_file_handle coff_file_hnd, coff_lib_file *c_file )
{
    AddDataImpLib( coff_file_hnd, c_file->section, c_file->header.num_sections * COFF_SECTION_HEADER_SIZE );
}

/*!
 * @brief Append the symbol table to the emulated object.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     c_file         Scratch structure. Not NULL.
 */
static void CreateCoffSymbols( coff_file_handle coff_file_hnd, coff_lib_file *c_file )
{
    AddDataImpLib( coff_file_hnd, c_file->symbol, c_file->header.num_symbols * COFF_SYM_SIZE );
}

/*!
 * @brief Append one COFF relocation record to the emulated object.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     offset         Relocation offset.
 * @param[in]     sym_tab_index  Symbol table index.
 * @param[in]     type           COFF relocation type.
 */
static void CreateCoffReloc( coff_file_handle coff_file_hnd, unsigned_32 offset, unsigned_32 sym_tab_index, unsigned_16 type )
{
    //output is buffered so no point in putting COFF_RELOC struct
    AddDataImpLib( coff_file_hnd, &offset, sizeof( offset ) );
    AddDataImpLib( coff_file_hnd, &sym_tab_index, sizeof( sym_tab_index ) );
    AddDataImpLib( coff_file_hnd, &type, sizeof( type ) );
}

/*!
 * @brief Append the string table to the emulated object.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     c_file         Scratch structure. Not NULL.
 */
static void CreateCoffStringTable( coff_file_handle coff_file_hnd, coff_lib_file *c_file )
{
    c_file->string_table_size += 4;
    AddDataImpLib( coff_file_hnd, &( c_file->string_table_size ), 4 );
    AddDataImpLib( coff_file_hnd, c_file->string_table, c_file->string_table_size - 4 );
}

/*!
 * @brief Strip name decoration according to an import name type.
 *
 * I got following information from Microsoft about name type and name
 * conversion.
 * @code
 *  IMPORT_OBJECT_ORDINAL = 0,          // Import by ordinal
 *  IMPORT_OBJECT_NAME = 1,             // Import name == public symbol name.
 *  IMPORT_OBJECT_NAME_NO_PREFIX = 2,   // Import name == public symbol name skipping leading ?, @, or optionally _.
 *  IMPORT_OBJECT_NAME_UNDECORATE = 3,  // Import name == public symbol name skipping leading ?, @, or optionally _
 *                                      // and truncating at first @
 * @endcode
 *
 * Note:
 * - IMPORT_OBJECT_NAME_NO_PREFIX is used for C symbols with underscore as prefix
 * - IMPORT_OBJECT_NAME_UNDECORATE is used for __stdcall and __fastcall name mangling
 * - __stdcall uses underscore as prefix and @nn as suffix
 * - __fastcall uses @ as prefix and @nn as suffix
 *
 * This solution is stupid, probably it needs improvement;
 * there is no more information from Microsoft.
 *
 * @param[in,out] src   Name buffer. Modified in place.
 * @param[in]     type  Import name type.
 * @return Pointer into @p src at the public symbol name.
 */
char * getImportName(char * src, int type){
    char    *end;

/*
I got following information from Microsoft about name type and name conversion.

    IMPORT_OBJECT_ORDINAL = 0,          // Import by ordinal
    IMPORT_OBJECT_NAME = 1,             // Import name == public symbol name.
    IMPORT_OBJECT_NAME_NO_PREFIX = 2,   // Import name == public symbol name skipping leading ?, @, or optionally _.
    IMPORT_OBJECT_NAME_UNDECORATE = 3,  // Import name == public symbol name skipping leading ?, @, or optionally _
                                        // and truncating at first @
*/
    // Note:
    // IMPORT_OBJECT_NAME_NO_PREFIX is used for C symbols with underscore as prefix
    // IMPORT_OBJECT_NAME_UNDECORATE is used for __stdcall and __fastcall name mangling
    // __stdcall uses underscore as prefix and @nn as suffix
    // __fastcall uses @ as prefix and @nn as suffix

    // this solution is stupid, probably it needs improvement
    // there is no more information from Microsoft 

    if( *src != 0 ) {
        switch (type) {
        case IMPORT_OBJECT_ORDINAL:
        case IMPORT_OBJECT_NAME:
            break;
        case IMPORT_OBJECT_NAME_UNDECORATE:
            // remove suffix @nn or @ if any
            end = src + strlen( src );
            while( end != src ) {
                --end;
                if( *end < '0' || *end > '9' ) {
                    if( *end == '@' ) {
                        *end = 0;
                    }
                    break;
                }
            }
            // fall through
        case IMPORT_OBJECT_NAME_NO_PREFIX:
            // remove prefix @ or _ if any
            if(( *src == '@' ) || ( *src == '_' ))
                src++;
            break;
        }
    }
    return( src );
}

/*!
 * @brief Expand one decoded short import library member into COFF bytes.
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @param[in]     import         Decoded import member. Not NULL.
 * @retval ORL_OKAY            Success.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
static int CoffCreateImport( coff_file_handle coff_file_hnd, import_sym * import )
{
    unsigned_16     type;
    coff_lib_file   c_file;
    unsigned_16     ordinal;
    char *          buffer;
    union {
        char b32[4];
        char b64[8];
    } bnull;
    unsigned        symbol_name_len;
    int             symbol_text_exportedName;
    int             symbol___imp_exportedName;
    int             symbol_idata6 = 0;
    int             symbol_toc = 0;
    char *          DLLSymbolName;
    int             dllsymbol_name_len;
    int             section_no;

    c_file.coff_file_hnd = coff_file_hnd;
    InitCoffFile( &c_file );
    memset( bnull.b64, 0, sizeof(bnull.b64) );

    symbol_name_len = strlen(import->exportedName);
    DLLSymbolName = alloca(symbol_name_len + 1 );
    strcpy(DLLSymbolName, import->exportedName);
    DLLSymbolName = getImportName(DLLSymbolName, import->type);
    dllsymbol_name_len = strlen(DLLSymbolName);

    buffer = alloca(max( strlen(import->DLLName), symbol_name_len) + 64 );

    SetCoffFile( &c_file, import->processor, import->time_date_stamp, 0 );
    switch( import->processor ) {
    case IMAGE_FILE_MACHINE_ALPHA:
/* .text section header */
        section_no = AddCoffSection( &c_file, ".text", sizeof(CoffImportAxpText), 3, IMAGE_SCN_ALIGN_16BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE );
        AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_NODUPLICATES, section_no );
        symbol_text_exportedName = AddCoffSymbol( &c_file, import->exportedName, 0x0, section_no, 0x20, IMAGE_SYM_CLASS_EXTERNAL, 0 );
        break;
    case IMAGE_FILE_MACHINE_POWERPC:
/* .text section header */
        section_no = AddCoffSection( &c_file, ".text", sizeof(CoffImportPpcText), 1, IMAGE_SCN_ALIGN_4BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE );
        AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_NODUPLICATES, section_no );
        strcpy( buffer, ".." );
        strcpy( buffer + 2, import->exportedName );
        symbol_text_exportedName = AddCoffSymbol( &c_file, buffer, 0x0, section_no, 0x20, IMAGE_SYM_CLASS_EXTERNAL, 0 );
/* .pdata section header */
        section_no = AddCoffSection( &c_file, ".pdata", sizeof(CoffImportPpcPdata), 4, IMAGE_SCN_ALIGN_1BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ );
        AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_ASSOCIATIVE, section_no );
/* .reldata section header */
        section_no = AddCoffSection( &c_file, ".reldata", sizeof(bnull.b64), 2, IMAGE_SCN_ALIGN_8BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |  IMAGE_SCN_MEM_WRITE );
        if( import->type != IMPORT_OBJECT_ORDINAL) {
            AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_NODUPLICATES, section_no );
        }
        AddCoffSymbol( &c_file, import->exportedName, 0x0, section_no, IMAGE_SYM_TYPE_NULL, IMAGE_SYM_CLASS_EXTERNAL, 0 );
        break;
    default:
        import->processor = IMAGE_FILE_MACHINE_I386;
    case IMAGE_FILE_MACHINE_I386:
/* .text section header */
        section_no = AddCoffSection( &c_file, ".text", sizeof(CoffImportX86Text), 1, IMAGE_SCN_ALIGN_2BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE );
        AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_NODUPLICATES, section_no );
        symbol_text_exportedName = AddCoffSymbol( &c_file, import->exportedName, 0x0, section_no, 0x20, IMAGE_SYM_CLASS_EXTERNAL, 0 );
        break;
    }
        
/* .idata$5 section header */
    if( import->type == IMPORT_OBJECT_ORDINAL) {
        section_no = AddCoffSection( &c_file, ".idata$5", sizeof(bnull.b32), 0, IMAGE_SCN_ALIGN_4BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |  IMAGE_SCN_MEM_WRITE );
    } else {
        section_no = AddCoffSection( &c_file, ".idata$5", sizeof(bnull.b32), 1, IMAGE_SCN_ALIGN_4BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |  IMAGE_SCN_MEM_WRITE );
    }
    AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_NODUPLICATES, section_no );
    strcpy( buffer, "__imp_" );
    strcat( buffer, import->exportedName );
    symbol___imp_exportedName = AddCoffSymbol( &c_file, buffer, 0x0, section_no, IMAGE_SYM_TYPE_NULL, IMAGE_SYM_CLASS_EXTERNAL, 0 );

/* .idata$4 section header */
    if( import->type == IMPORT_OBJECT_ORDINAL) {
        section_no = AddCoffSection( &c_file, ".idata$4", sizeof(bnull.b32), 0, IMAGE_SCN_ALIGN_4BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |  IMAGE_SCN_MEM_WRITE );
        AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_ASSOCIATIVE, section_no );
    } else {
        section_no = AddCoffSection( &c_file, ".idata$4", sizeof(bnull.b32), 1, IMAGE_SCN_ALIGN_4BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |  IMAGE_SCN_MEM_WRITE );
        AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_ASSOCIATIVE, section_no );
/* .idata$6 section header */
        section_no = AddCoffSection( &c_file, ".idata$6", sizeof(ordinal) + ( dllsymbol_name_len | 1 ) + 1, 0, IMAGE_SCN_ALIGN_2BYTES | IMAGE_SCN_LNK_COMDAT | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |  IMAGE_SCN_MEM_WRITE );
        symbol_idata6 = AddCoffSymSec( &c_file, IMAGE_COMDAT_SELECT_ASSOCIATIVE, section_no );
    }

/* other symbols */
    if( import->processor == IMAGE_FILE_MACHINE_POWERPC ) {
        symbol_toc = AddCoffSymbol( &c_file, ".toc", 0x0, 0, IMAGE_SYM_TYPE_NULL, IMAGE_SYM_CLASS_EXTERNAL, 0 );
    }
    strcpy( buffer, "__IMPORT_DESCRIPTOR_" );
    strcat( buffer, import->DLLName );
    AddCoffSymbol( &c_file, buffer, 0x0, 0, IMAGE_SYM_TYPE_NULL, IMAGE_SYM_CLASS_EXTERNAL, 0 );

/* COFF file header */

    CreateCoffFileHeader( coff_file_hnd, &c_file );

/* COFF sections header */

    CreateCoffSections( coff_file_hnd, &c_file );

/* COFF section data */

    switch( import->processor ) {
    case IMAGE_FILE_MACHINE_ALPHA:
/* .text section data */
        AddDataImpLib( coff_file_hnd, CoffImportAxpText, sizeof(CoffImportAxpText) );
/* .text relocations records */
        CreateCoffReloc( coff_file_hnd, 0x0, symbol___imp_exportedName, IMAGE_REL_ALPHA_REFHI );
        CreateCoffReloc( coff_file_hnd, 0x0, symbol_text_exportedName, IMAGE_REL_ALPHA_PAIR );
        CreateCoffReloc( coff_file_hnd, 0x4, symbol___imp_exportedName, IMAGE_REL_ALPHA_REFLO );
        break;
    case IMAGE_FILE_MACHINE_POWERPC:
/* .text section data */
        AddDataImpLib( coff_file_hnd, CoffImportPpcText, sizeof(CoffImportPpcText) );
/* .text relocations records */
        CreateCoffReloc( coff_file_hnd, 0x0, symbol___imp_exportedName, IMAGE_REL_PPC_TOCREL14 | IMAGE_REL_PPC_TOCDEFN );
/* .pdata section data */
        AddDataImpLib( coff_file_hnd, CoffImportPpcPdata, sizeof(CoffImportPpcPdata) );
/* .pdata relocations records */
        CreateCoffReloc( coff_file_hnd, 0x0, symbol_text_exportedName, IMAGE_REL_PPC_ADDR32 );
        CreateCoffReloc( coff_file_hnd, 0x4, symbol_text_exportedName, IMAGE_REL_PPC_ADDR32 );
        CreateCoffReloc( coff_file_hnd, 0x10, symbol_text_exportedName, IMAGE_REL_PPC_ADDR32 );
        CreateCoffReloc( coff_file_hnd, 0x80410004, symbol_text_exportedName, IMAGE_REL_PPC_SECTION );
/* .reldata section data */
        AddDataImpLib( coff_file_hnd, bnull.b64, sizeof(bnull.b64) );
/* .reldata relocations records */
        CreateCoffReloc( coff_file_hnd, 0x0, symbol_text_exportedName, IMAGE_REL_PPC_ADDR32 );
        CreateCoffReloc( coff_file_hnd, 0x4, symbol_toc, IMAGE_REL_PPC_ADDR32 );
        break;
    case IMAGE_FILE_MACHINE_I386:
/* .text section data */
        AddDataImpLib( coff_file_hnd, CoffImportX86Text, sizeof(CoffImportX86Text) );
/* .text relocations records */
        CreateCoffReloc( coff_file_hnd, 0x2, symbol___imp_exportedName, IMAGE_REL_I386_DIR32 );
        break;
    }

    ordinal = import->ordinal;
    if( import->type == IMPORT_OBJECT_ORDINAL) {
/* .idata$5 section data - ordinal */
        AddDataImpLib( coff_file_hnd, &ordinal, sizeof(ordinal) );
        type = 0x8000;
        AddDataImpLib( coff_file_hnd, &type, sizeof(type) );
/* .idata$4 section data - ordinal */
        AddDataImpLib( coff_file_hnd, &ordinal, sizeof(ordinal) );
        AddDataImpLib( coff_file_hnd, &type, sizeof(type) );
    } else {
        switch( import->processor ) {
        case IMAGE_FILE_MACHINE_ALPHA:
            type = IMAGE_REL_ALPHA_REFLONGNB;
        case IMAGE_FILE_MACHINE_POWERPC:
            type = IMAGE_REL_PPC_ADDR32NB;
        case IMAGE_FILE_MACHINE_I386:
            type = IMAGE_REL_I386_DIR32NB;
        }
/* .idata$5 section data - name */
        AddDataImpLib( coff_file_hnd, bnull.b32, sizeof(bnull.b32) );
/* .idata$5 relocations records */
        CreateCoffReloc( coff_file_hnd, 0, symbol_idata6, type );
/* .idata$4 section data - name */
        AddDataImpLib( coff_file_hnd, bnull.b32, sizeof(bnull.b32) );
/* .idata$4 relocations records */
        CreateCoffReloc( coff_file_hnd, 0, symbol_idata6, type );
/* .idata$6 section data - name */
        AddDataImpLib( coff_file_hnd, &ordinal, sizeof( ordinal ) );
        AddDataImpLib( coff_file_hnd, DLLSymbolName, dllsymbol_name_len + 1 );
        if( !( dllsymbol_name_len & 1 ) ) {
            AddDataImpLib( coff_file_hnd, "\0", 1 );
        }
    }

/* COFF symbol table */

    CreateCoffSymbols( coff_file_hnd, &c_file );

/* COFF strings table */

    CreateCoffStringTable( coff_file_hnd, &c_file );
    FiniCoffLibFile( &c_file );
    return ORL_OKAY;
}

/*!
 * @brief Convert a short import library member into a long COFF object.
 *
 * Reads the short member header and data, expands it via
 * CoffCreateImport(), and installs ImportLibData as the file's
 * read/seek callbacks so the normal COFF reader can consume the
 * emulated bytes.
 *
 * @param[in,out] coff_file_hnd  COFF file handle. Not NULL.
 * @retval ORL_OKAY            Success.
 * @retval ORL_OUT_OF_MEMORY   Allocation failure.
 * @return ORL_OKAY on success.
 */
int convert_import_library(coff_file_handle coff_file_hnd)
{
    coff_import_object_header   *i_hdr;
    import_sym                  sym;

    // init import library data structure
    if ( DataImpLibInit( coff_file_hnd ) != ORL_OKAY )
        return( ORL_OUT_OF_MEMORY );

    i_hdr = (coff_import_object_header*)coff_file_hnd->f_hdr_buffer;
    sym.processor = i_hdr->machine;
    sym.exportedName = coff_file_hnd->coff_hnd->funcs->read( coff_file_hnd->file,
        i_hdr->size_of_data );
    sym.DLLName = sym.exportedName + strlen( sym.exportedName ) + 1;
    sym.time_date_stamp = i_hdr->time_date_stamp;
    sym.type = i_hdr->name_type;
    sym.ordinal = i_hdr->oh.ordinal;
    return( CoffCreateImport( coff_file_hnd, &sym ) );
}

/*!
 * @brief Read/seek callback table for the emulated import library buffer.
 */
orl_funcs ImportLibData = {ImportLibRead, ImportLibSeek, NULL, NULL};
