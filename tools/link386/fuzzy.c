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
 * @file fuzzy.c
 * @brief Fuzzy linker-symbol matching.
 *
 * Collects external symbols from the objects and libraries being
 * linked, then attempts to resolve mismatched symbol names (e.g.
 * decorated vs. undecorated, with or without leading underscore,
 * stdcall at-number suffix).
 */

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "demangle.h"           /* from lib_misc project */
#include "dlltool.h"
#include "error.h"
#include "fuzzy.h"
#include "hash.h"
#include "memory.h"
#include "orl.h"                /* from riscdev project */

/*!
 * @def HASH_TABLE_SIZE
 * @brief Number of buckets in the symbol hash table.
 */
#define HASH_TABLE_SIZE         2111
/*!
 * @def MAX_SYMBOL_LEN
 * @brief Maximum length of a symbol name handled by the matcher.
 */
#define MAX_SYMBOL_LEN          512

/*!
 * @def MATCH_MODE_EXACT
 * @brief Match mode: exact name.
 */
#define MATCH_MODE_EXACT                        1
/*!
 * @def MATCH_MODE_UNDERBAR_SYMBOL
 * @brief Match mode: "_symbol".
 */
#define MATCH_MODE_UNDERBAR_SYMBOL              2
/*!
 * @def MATCH_MODE_SYMBOL_UNDERBAR
 * @brief Match mode: "symbol_".
 */
#define MATCH_MODE_SYMBOL_UNDERBAR              3
/*!
 * @def MATCH_MODE_UNDERBAR_SYMBOL_AT_NUMBER
 * @brief Match mode: "_symbol@<number>".
 */
#define MATCH_MODE_UNDERBAR_SYMBOL_AT_NUMBER    4


/*
 * Types.
 */

/*!
 * @struct _ListElem
 * @brief One element in a singly-linked string list.
 */
typedef struct _ListElem {
    struct _ListElem *  next;       /*!< Next element, or NULL. */
    char                buff[1];    /*!< NUL-terminated string. */
} ListElem;

/*!
 * @struct _MatchingInfo
 * @brief Context passed to the bucket walk callback.
 */
typedef struct _MatchingInfo {
    char *              basename;   /*!< Entry name to match. Not NULL. */
    int                 findmode;   /*!< One of MATCH_MODE_*. */
    ListElem *          found;      /*!< Matched symbols so far, or NULL. */
} MatchingInfo;


/*
 * Static data.
 */

/*!
 * @brief Hash table of collected symbols.
 */
static HashTable        hashtable = NULL;
/*!
 * @brief Pool of ORL-allocated buffers to release after scanning.
 */
static ListElem *       bufflist = NULL;
/*!
 * @brief Loaded WLIB DLL handle, or NULL.
 */
static void *           dllhandle = NULL;


/*!
 * @brief Determine whether a symbol's name is mangled.
 * @param[in] symbol  Symbol name. Not NULL.
 * @retval 1  Symbol is a mangled C++ name.
 * @retval 0  Symbol is not mangled (or uses an unsupported mangling).
 * @return Non-zero if the name is mangled.
 */
static int is_name_mangled( const char *symbol )
/**********************************************/
{
    if( __is_mangled( symbol, strlen( symbol ) ) ) {
        return( 1 );
    }
    /* what about MS mangling? */

    return( 0 );
}


/*!
 * @brief Hash comparison callback.
 * @param[in] item1  First item. Not NULL.
 * @param[in] item2  Second item. Not NULL.
 * @retval 1  Items are equal strings.
 * @retval 0  Items differ.
 * @return Non-zero if the two strings are equal.
 */
static int hash_compare( const void *item1, const void *item2 )
/*************************************************************/
{
    return( !strcmp( (const char*)item1, (const char*)item2 ) );
}


/*!
 * @brief Hash a symbol name, ignoring leading underscores and trailing
 *        "@<number>"/"_" decorations.
 * @param[in] symbol  Symbol name. Not NULL.
 * @return The hash value.
 */
static unsigned hash_symbol_name( const void *symbol )
/****************************************************/
{
    char *              namecopy;
    char *              namestart;
    char *              p;
    unsigned *          s;
    size_t              len;
    unsigned            mask;
    unsigned            c;
    unsigned            g;
    unsigned            h;
    unsigned const      namecmpmask[5] = {
        0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF, 0xFFFFFFFF
    };

    /*
     * We want 'foo', '_foo', 'foo_', '_foo@8', and so on all to hash to
     * the same value, so strip off stuff from each end if necessary to
     * get the root name.
     */
    namecopy = DupStrMem( symbol );
    namestart = namecopy;
    while( *namestart == '_' )  namestart++;
    len = strlen( namestart );
    if( len > 0 ) {
        p = namestart + len - 1;
        while( p >= namestart  &&  ( isdigit(*p) || *p=='@' || *p=='_' ) ) {
            *p = '\0';
            p--;
        }
    }

    /*
     * Ok, now do the actual hashing.
     */
    s = (unsigned*) namestart;
    len = strlen( namestart );
    h = len;
    c = len;
    if( len > sizeof( unsigned ) ) {
        do {
            c ^= *s;
            h = ( h << 4 ) + c;
            g = h & ~0x00FFFFFF;
            h ^= g;
            h ^= g >> (4+4+4+4+4);
            ++s;
            len -= sizeof( unsigned );
        } while( len > sizeof( unsigned ) );
    }
    mask = namecmpmask[ len ];
    c ^= *s & mask;
    h = ( h << 4 ) + c;
    g = h & ~0x00FFFFFF;
    h ^= g;
    h ^= g >> (4+4+4+4+4);
    g = h & ~0x0FFF;
    h ^= g;
    h ^= g >> (4+4+4);
    h ^= h >> (2+4);

    FreeMem( namecopy );
    return( h );
}


/*!
 * @brief ORL read callback.
 * @param[in] hdl  File descriptor wrapped as a void pointer.
 * @param[in] len  Number of bytes to read.
 * @retval NULL Read failure.
 * @return Pointer into the read buffer.
 */
static void *obj_read( void *hdl, size_t len )
/*****************************************/
{
    ListElem *          newelem;

    newelem = AllocMem( sizeof( ListElem ) + len - 1 );
    newelem->next = bufflist;
    bufflist = newelem;
    if( read( (int)hdl, newelem->buff, len ) != len ) {
        FreeMem( newelem );
        return( NULL );
    }
    return( newelem->buff );
}


/*!
 * @brief ORL seek callback.
 * @param[in] hdl    File descriptor wrapped as a void pointer.
 * @param[in] pos    Offset to seek to.
 * @param[in] where  One of SEEK_SET/SEEK_CUR/SEEK_END.
 * @return The new file position.
 */
static long obj_seek( void *hdl, long pos, int where )
/****************************************************/
{
    return( lseek( (int)hdl, pos, where ) );
}


/*!
 * @brief Free a list of ListElem allocations.
 * @param[in] liststart  First element in the list, or NULL.
 */
static void free_list( ListElem *liststart )
/******************************************/
{
    ListElem *          curelem;
    ListElem *          nextelem;

    curelem = liststart;
    while( curelem != NULL ) {
        nextelem = curelem->next;
        FreeMem( curelem );
        curelem = nextelem;
    }
    bufflist = NULL;
}


/*!
 * @brief Add one more symbol to the hash table.
 * @param[in] o_symbol  ORL symbol handle. Not NULL.
 * @retval ORL_OKAY Always returns ORL_OKAY.
 * @return ORL_OKAY.
 */
static orl_return do_orl_symbol( orl_symbol_handle o_symbol )
/***********************************************************/
{
    char *              name;
    char *              namecopy;

    name = ORLSymbolGetName( o_symbol );

    if( !is_name_mangled( name ) ) {
        namecopy = DupStrMem( name );
        InsertHash( hashtable, namecopy );
    }

    return( ORL_OKAY );
}


/*!
 * @brief Collect external symbols from an object file.
 * @param[in] filename  Object file path. Not NULL.
 * @param[in] o_hnd     ORL handle. Not NULL.
 * @retval 1  Symbols collected.
 * @retval 0  File could not be opened or is not an object file.
 * @return 1 on success, 0 on failure.
 */
static int handle_obj_file( const char *filename, orl_handle o_hnd )
/******************************************************************/
{
    orl_file_handle     o_fhnd;
    orl_file_format     o_format;
    orl_file_type       o_filetype;
    orl_sec_handle      o_symtab;
    orl_return          o_rc;
    int                 fileh;

    /*** Make ORL interested in the file ***/
    fileh = open( filename, O_BINARY | O_RDONLY );
    if( fileh == -1 ) {
        return( 0 );
    }
    o_format = ORLFileIdentify( o_hnd, (void*)fileh );
    if( o_format == ORL_UNRECOGNIZED_FORMAT ) {
        close( fileh );
        return( 0 );
    }
    o_fhnd = ORLFileInit( o_hnd, (void*)fileh, o_format );
    if( o_fhnd == NULL ) {
        close( fileh );
        return( 0 );
    }
    o_filetype = ORLFileGetType( o_fhnd );
    if( o_filetype != ORL_FILE_TYPE_OBJECT ) {
        close( fileh );
        return( 0 );
    }

    /*** Scan the file's symbol table ***/
    o_symtab = ORLFileGetSymbolTable( o_fhnd );
    if( o_symtab == NULL ) {
        close( fileh );
        return( 0 );
    }
    o_rc = ORLSymbolSecScan( o_symtab, &do_orl_symbol );
    if( o_rc != ORL_OKAY ) {
        close( fileh );
        return( 0 );
    }
    o_rc = ORLFileFini( o_fhnd );
    if( o_rc != ORL_OKAY ) {
        close( fileh );
        return( 0 );
    }

    close( fileh );
    return( 1 );
}


/*!
 * @brief Locate a file along the library search paths.
 * @param[in] filename  File name to locate. Not NULL.
 * @param[in] libpaths  NULL-terminated search path array. May be NULL.
 * @retval NULL File not found.
 * @return Newly allocated full path to the file.
 */
static char *find_file( const char *filename, const char *libpaths[] )
/********************************************************************/
{
    int                 rc;
    unsigned            count;
    char *              tryme;
    size_t              len;
    const char *        p;
    int                 hasbackslash;

    /*** We might not have to go searching for it ***/
    rc = access( filename, F_OK );
    if( rc == 0 ) {
        return( DupStrMem( filename ) );
    }

    /*** Not found, so check any directories in 'libpaths' ***/
    if( libpaths != NULL ) {
        for( count=0; libpaths[count]!=NULL; count++ ) {
            /*** Determine if we need to add a backslash to the path ***/
            len = strlen( libpaths[count] );
            if( len == 0 )  Zoinks();
            p = libpaths[count] + len - 1;
            if( *p == '\\' ) {
                hasbackslash = 1;
            } else {
                hasbackslash = 0;
            }

            /*** See if the file exists here ***/
            len = strlen( libpaths[count] )  +
                  ( hasbackslash ? 0 : 1 )  +
                  strlen( filename ) + 1;
            tryme = AllocMem( len );
            sprintf( tryme, "%s%s%s", libpaths[count],
                     hasbackslash ? "" : "\\", filename );
            rc = access( tryme, F_OK );
            if( rc == 0 ) {
                return( tryme );
            } else {
                FreeMem( tryme );
            }
        }
    }

    return( NULL );
}


/*!
 * @brief Collect external symbols from a library file by running WLIB.
 * @param[in] filename  Library file name. Not NULL.
 * @param[in] libpaths  NULL-terminated search path array. May be NULL.
 * @retval 1  Symbols collected.
 * @retval 0  File could not be found or WLIB failed.
 * @return 1 on success, 0 on failure.
 */
static int handle_lib_file( const char *filename, const char *libpaths[] )
/************************************************************************/
{
    char *              realpath;
    size_t              len;
    char *              options;
    const size_t        leeway = 10;    /* for "-tl" and such */
    int                 rc;

    realpath = find_file( filename, libpaths );
    if( realpath == NULL ) {
        Warning( "Cannot locate '%s'; skipping -- fuzzy name matching may not work",
                 filename );
        return( 0 );
    }

    len = strlen( realpath );
    options = AllocMem( leeway + len + 1 );
    sprintf( options, "-tl %s", realpath );
    rc = RunDllTool( dllhandle, options );
    FreeMem( options );
    if( rc ) {
        return( 1 );
    } else {
        return( 0 );
    }
}


/*!
 * @brief Process one line of output from the WLIB DLL.
 * @param[in] text  Output line. Not NULL.
 * @retval 1  Line was consumed (or was a non-fatal message).
 * @retval 0  Line was a diagnostic and has been reported.
 * @return 1 to continue, 0 on error.
 */
static int wlib_output( const char *text )
/****************************************/
{
    int                 badness = 0;

    if( ( strncmp( text, "Error!", 6 ) == 0 )  ||
        ( strncmp( text, "Warning!", 8 ) == 0 ) ) {
        badness = 1;
    }

    if( !badness ) {
        if( strchr( text, ' ' ) == NULL ) {
            InsertHash( hashtable, (void*)DupStrMem( (char*)text ) );
        }
        return( 1 );
    } else {
        Warning( "Message from WLIB DLL: %s", text );
        return( 0 );
    }
}


/*!
 * @brief WLIB DLL plain-message (with CRLF) callback.
 * @param[in] hdl   Caller handle (unused).
 * @param[in] text  Output line. Not NULL.
 * @retval 0  Line was a diagnostic.
 * @retval non-zero Line was consumed.
 * @return Non-zero to continue.
 */
static IDEBool __stdcall print_message_crlf( IDECBHdl hdl, const char *text )
{
    return( (IDEBool)wlib_output( text ) );
}

/*!
 * @brief WLIB DLL structured-message (v2) callback.
 * @param[in] hdl   Caller handle (unused).
 * @param[in] info  Structured output. Not NULL.
 * @retval 0  Line was a diagnostic.
 * @retval non-zero Line was consumed.
 * @return Non-zero to continue.
 */
static IDEBool __stdcall print_with_info2( IDECBHdl hdl, IDEMsgInfo2 *info )
{
    return( (IDEBool)wlib_output( info->msg ) );
}

/*!
 * @brief WLIB DLL structured-message (v3+) callback.
 * @param[in] hdl   Caller handle (unused).
 * @param[in] info  Structured output. Not NULL.
 * @retval 0  Line was a diagnostic.
 * @retval non-zero Line was consumed.
 * @return Non-zero to continue.
 */
static IDEBool __stdcall print_with_info( IDECBHdl hdl, IDEMsgInfo *info )
{
    return( (IDEBool)wlib_output( info->msg ) );
}


/*!
 * @brief Initialize fuzzy matching from object and library files.
 * @param[in] objs      Linked object files, NULL-terminated. May be NULL.
 * @param[in] libs      Linked libraries, NULL-terminated. May be NULL.
 * @param[in] libpaths  Library search paths, NULL-terminated. May be NULL.
 * @param[in] callback  Callback invoked on extraction failures. Not NULL.
 *
 * If symbols cannot be collected from a file, @p callback is called
 * with the name of the offending file. If it returns zero,
 * initialization is abandoned.
 */
void InitFuzzy( const char *objs[], const char *libs[],
                const char *libpaths[], FuzzyInitCallback callback )
/******************************************************************/
{
    unsigned            count;
    orl_funcs           o_funcs;
    orl_handle          o_hnd;
    int                 rc = 1;
    DllToolCallbacks    dllcallbacks;

    /*** Create a hash table ***/
    hashtable = InitHash( HASH_TABLE_SIZE, hash_symbol_name, hash_compare );

    /*** Collect all external symbols from the specified object files ***/
    if( objs != NULL ) {
        /*** Initialize ORL ***/
        o_funcs.read = &obj_read;
        o_funcs.seek = &obj_seek;
        o_funcs.alloc = &AllocMem;
        o_funcs.free = &FreeMem;
        o_hnd = ORLInit( &o_funcs );
        if( o_hnd == NULL ) {
            FatalError( "Got NULL orl_handle." );
        }

        /*** Scan the file(s) ***/
        for( count=0; objs[count]!=NULL; count++ ) {
            if( !handle_obj_file( objs[count], o_hnd ) ) {
                rc = (*callback)( objs[count] );
                if( !rc )  break;
            }
        }

        /*** Tell ORL to go away ***/
        if( ORLFini( o_hnd ) != ORL_OKAY ) {
            Warning( "Error calling ORLFini." );
        }
        free_list( bufflist );
    }
    if( !rc )  return;

    /*** Collect all external symbols from the specified library files ***/
    if( libs != NULL ) {
        /*** Load the WLIB DLL ***/
        dllcallbacks.printmessage = NULL;
        dllcallbacks.printmessageCRLF = print_message_crlf;
        dllcallbacks.printwithinfo2 = print_with_info2;
        dllcallbacks.printwithinfo = print_with_info;
        dllcallbacks.cookie = NULL;
        dllhandle = InitDllTool( DLLTOOL_WLIB, &dllcallbacks );
        if( dllhandle == NULL ) {
            Warning( "Cannot load WLIB DLL -- fuzzy name matching may not work" );
        } else {
            /*** Scan the file(s) ***/
            for( count=0; libs[count]!=NULL; count++ ) {
                if( !handle_lib_file( libs[count], libpaths ) ) {
                    rc = (*callback)( libs[count] );
                    if( !rc )  break;
                }
            }
            FiniDllTool( dllhandle );
        }
    }
}


/*!
 * @brief Deinitialize the fuzzy matching module.
 */
void FiniFuzzy( void )
/********************/
{
    if( hashtable == NULL )  Zoinks();
    FiniHash( hashtable, 1 );
    hashtable = NULL;
}


/*!
 * @brief Bucket-walk callback: test one symbol against a match mode.
 * @param[in] name_  Candidate symbol name. Not NULL.
 * @param[in] info_  MatchingInfo context. Not NULL.
 * @retval 0  Unique match achieved (stop walking).
 * @retval 1  Continue walking.
 * @return Zero to stop walking, non-zero to continue.
 */
static int matching_callback( const void *name_, void *info_ )
/************************************************************/
{
    const char          *name = name_;
    MatchingInfo        *info = info_;
    char                matchstr[MAX_SYMBOL_LEN+1];
    int                 addit = 0;
    const char *        p;
    ListElem *          newelem;
    ListElem *          nextelem;

    if( name == NULL )  Zoinks();
    if( strlen(info->basename)+2 > MAX_SYMBOL_LEN )  Zoinks();

    /*** Try to match this symbol ***/
    switch( info->findmode ) {
        case MATCH_MODE_EXACT:
            if( !strcmp( name, info->basename ) ) {
                addit = 1;
            }
            break;
        case MATCH_MODE_UNDERBAR_SYMBOL:
            sprintf( matchstr, "_%s", info->basename );
            if( !strcmp( name, matchstr ) ) {
                addit = 1;
            }
            break;
        case MATCH_MODE_SYMBOL_UNDERBAR:
            sprintf( matchstr, "%s_", info->basename );
            if( !strcmp( name, matchstr ) ) {
                addit = 1;
            }
            break;
        case MATCH_MODE_UNDERBAR_SYMBOL_AT_NUMBER:
            sprintf( matchstr, "_%s@", info->basename );
            if( strstr( name, matchstr ) == name ) {
                p = name + strlen( matchstr );
                while( isdigit( *p ) ) {
                    p++;
                }
                if( *p == '\0' ) {
                    addit = 1;
                }
            }
            break;
        default:
            Zoinks();
    }

    /*** If it matches, add it to the found list ***/
    if( addit ) {
        nextelem = info->found;
        newelem = AllocMem( sizeof( ListElem ) + strlen( name ) );
        strcpy( newelem->buff, name );
        newelem->next = nextelem;
        info->found = newelem;

        /*** Keep searching only if multiple matches are possible ***/
        if( info->findmode == MATCH_MODE_UNDERBAR_SYMBOL_AT_NUMBER ) {
            return( 1 );
        } else {
            return( 0 );
        }
    } else {
        return( 1 );
    }
}


/*!
 * @brief Fuzzy-match a symbol against the collected symbol tables.
 * @param[in] entryname  Entry name (optionally single-quoted). Not NULL.
 *
 * If exactly one match is found, a pointer to the real symbol name
 * (in single quotes) is returned; the caller owns the returned
 * memory. In all other cases NULL is returned.
 *
 * @retval NULL No match, or more than one match was found.
 * @return Newly allocated single-quoted matched symbol name.
 */
char *MatchFuzzy( const char *entryname )
/***************************************/
{
    MatchingInfo        info;
    char *              retval;
    char *              tmp;
    unsigned            numfound = 0;
    ListElem *          curelem;

    if( hashtable == NULL )  Zoinks();

    /*** Strip quotes from 'entryname' ***/
    tmp = DupStrMem( entryname );
    if( *tmp == '\'' ) {
        info.basename = DupStrMem( tmp + 1 );
        info.basename[ strlen(info.basename) - 1 ] = '\0';
        FreeMem( tmp );
    } else {
        info.basename = tmp;
    }

    /*** Check for an exact match ***/
    info.findmode = MATCH_MODE_EXACT;
    info.found = NULL;
    WalkBucketHash( hashtable, info.basename, matching_callback, &info );

    /*** Check for _symbol ***/
    if( info.found == NULL ) {
        info.findmode = MATCH_MODE_UNDERBAR_SYMBOL;
        WalkBucketHash( hashtable, info.basename, matching_callback, &info );
    }

    /*** Check for symbol_ ***/
    if( info.found == NULL ) {
        info.findmode = MATCH_MODE_SYMBOL_UNDERBAR;
        WalkBucketHash( hashtable, info.basename, matching_callback, &info );
    }

    /*** Check for _symbol@number ***/
    if( info.found == NULL ) {
        info.findmode = MATCH_MODE_UNDERBAR_SYMBOL_AT_NUMBER;
        WalkBucketHash( hashtable, info.basename, matching_callback, &info );
    }

    /*** Count how many matches we got ***/
    numfound = 0;
    curelem = info.found;
    while( curelem != NULL ) {
        numfound++;
        curelem = curelem->next;
    }

    /*** If there was more than one match, complain ***/
    if( numfound > 1 ) {
        Warning( "Symbol with entry name %s has %u matching internal names:",
                 entryname, numfound );
        curelem = info.found;
        while( curelem != NULL ) {
            Information( "%s", curelem->buff );
            curelem = curelem->next;
        }
        Information( "Symbol will be ignored!" );
    }

    /*** Return an appropriate value ***/
    if( numfound == 1 ) {
        retval = DupQuoteStrMem( info.found->buff, '\'' );
    } else {
        retval = NULL;
    }
    free_list( info.found );
    return( retval );
}
