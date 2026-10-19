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
* Description:  Memory allocation and string duplication helpers.
*
****************************************************************************/

/*!
 * @file memory.c
 * @brief Memory allocation and string duplication wrappers.
 *
 * All allocation routines abort the program on failure, so callers
 * never need to check for NULL results.
 */

#include <stdlib.h>
#include <string.h>
#include "error.h"


/*!
 * @brief Allocate a block of memory, aborting on failure.
 *
 * @param[in] bytes  Size in bytes.  Must be non-zero.
 * @return Pointer to the new block.  Never NULL.
 */
void *AllocMem( size_t bytes )
/****************************/
{
    void *              p;

    if( bytes == 0 )  Zoinks();
    p = malloc( bytes );
    if( p == NULL )  FatalError( "Insufficient memory." );
    return( p );
}


/*!
 * @brief Resize an allocated block, aborting on failure.
 *
 * @param[in] buf    Existing block (may be NULL).
 * @param[in] bytes  New size in bytes.
 * @return Pointer to the resized block.  Never NULL.
 */
void *ReallocMem( void *buf, size_t bytes )
/*****************************************/
{
    void *              p;

    p = realloc( buf, bytes );
    if( p == NULL )  FatalError( "Insufficient memory." );
    return( p );
}


/*!
 * @brief Free a block of memory.
 *
 * @param[in] buf  Block to free.  May be NULL.
 */
void FreeMem( void *buf )
/***********************/
{
    free( buf );
}


/*!
 * @brief Duplicate a string, adding surrounding quotes if needed.
 *
 * If @p quote is non-zero and @p str is not already quoted with
 * matching double or single quotes, the result is wrapped in
 * @p quote characters.
 *
 * @param[in] str    Source string.  Not NULL.
 * @param[in] quote  Quote character, or zero for no quoting.
 * @return Newly allocated copy.
 */
char *DupQuoteStrMem( const char *str, char quote )
/*************************************************/
{
    char *              p;
    int                 len;
    int                 add_quote = 0;

    len = strlen( str );
    if( quote != '\0' ) {
        do {
            if( str[0] == '"'  && str[len-1] == '"'  ) break;
            if( str[0] == '\'' && str[len-1] == '\'' ) break;
            len += 2;
            add_quote = 1;
        } while( 0 );
    }
    p = AllocMem( len + 1 );
    if( add_quote ) {
        p[0] = quote;
        strcpy( &(p[1]), str );
        p[len-1] = quote;
        p[len] = '\0';
    } else {
        strcpy( p, str );
    }
    return( p );
}


/*!
 * @brief Duplicate a string.
 *
 * @param[in] str  Source string.  Not NULL.
 * @return Newly allocated copy.
 */
char *DupStrMem( const char *str )
/********************************/
{
    return( DupQuoteStrMem( str, 0 ) );
}
