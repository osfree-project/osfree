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
* Description:  Error, warning and informational message output.
*
****************************************************************************/

/*!
 * @file error.c
 * @brief Diagnostic message output for the NMAKE clone.
 *
 * Provides printf-style fatal-error, internal-error, warning and
 * informational routines.  A caller-supplied banner function is
 * invoked before each message; warnings and informational messages
 * can be globally suppressed with DisableWarnings().
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"

/*!
 * @brief Default banner function.  Does nothing.
 */
static void     do_nothing( void );


/*!
 * @brief Banner function invoked before every message.
 */
static void             (*bannerFunc)( void ) = do_nothing;
/*!
 * @brief Non-zero while warnings and informational messages are displayed.
 */
static int              displayWarnings = 1;


/*!
 * @brief Report a fatal error and terminate the program.
 *
 * If @p format is NULL, a default message is produced with perror().
 * Otherwise @p format and the variable arguments are formatted with
 * vfprintf().  Always exits with EXIT_FAILURE.
 *
 * @param[in] format  printf-style format string, or NULL.
 * @param[in] ...     Arguments for @p format.
 */
void FatalError( const char *format, ... )
/****************************************/
{
    va_list             args;

    (*bannerFunc)();
    if( format == NULL ) {
        perror( "Fatal error" );
    } else {
        va_start( args, format );
        fprintf( stderr, "Fatal error: " );
        vfprintf( stderr, format, args );
        fprintf( stderr, "\n" );
        va_end( args );
    }

    exit( EXIT_FAILURE );
}


/*!
 * @brief Report an internal error and terminate the program.
 *
 * Used for consistency checks that should never fail.
 *
 * @param[in] line  Source line number of the check.
 * @param[in] file  Source file name of the check.
 */
void InternalError( int line, const char *file )
/**********************************************/
{
    (*bannerFunc)();
    fprintf( stderr, "Internal error on line %d of %s. Please contact the Open Watcom maintainers at http://www.openwatcom.com\n",
             line, file );
    exit( EXIT_FAILURE );
}


/*!
 * @brief Print a warning message.
 *
 * Ignored when warnings have been disabled with DisableWarnings().
 *
 * @param[in] format  printf-style format string.
 * @param[in] ...     Arguments for @p format.
 */
void Warning( const char *format, ... )
/*************************************/
{
    va_list             args;

    if( displayWarnings ) {
        (*bannerFunc)();
        va_start( args, format );
        fprintf( stderr, "Warning: " );
        vfprintf( stderr, format, args );
        fprintf( stderr, "\n" );
        va_end( args );
    }
}


/*!
 * @brief Print an informational message.
 *
 * Obeys the same suppression flag as Warning().
 *
 * @param[in] format  printf-style format string.
 * @param[in] ...     Arguments for @p format.
 */
void Information( const char *format, ... )
/*****************************************/
{
    va_list             args;

    if( displayWarnings ) {
        (*bannerFunc)();
        va_start( args, format );
        fprintf( stderr, "Info: " );
        vfprintf( stderr, format, args );
        fprintf( stderr, "\n" );
        va_end( args );
    }
}


/*!
 * @brief Enable or disable warnings and informational messages.
 *
 * @param[in] yesno  Zero enables output, non-zero disables it.
 */
void DisableWarnings( int yesno )
/*******************************/
{
    displayWarnings = !yesno;
}


/*!
 * @brief Install a banner function invoked before every message.
 *
 * @param[in] func  Banner function.  Not NULL.
 */
void SetBannerFuncError( void (*func)( void ) )
/*********************************************/
{
    bannerFunc = func;
}


/*!
 * @brief Default banner function.  Does nothing.
 */
static void do_nothing( void )
/****************************/
{
}
