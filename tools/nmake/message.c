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
* Description:  Banner, help and diagnostic messages for the NMAKE clone.
*
****************************************************************************/

/*!
 * @file message.c
 * @brief Banner, help and diagnostic messages for the NMAKE clone.
 */

#include <conio.h>
#include <stdio.h>
#include "banner.h"
#include "error.h"
#include "message.h"
#include "system.h"


/*!
 * @brief One line of usage text (see usagemsg.gh).
 */
static char *           usageMsg[] = {
    #include "usagemsg.gh"
    NULL
};

/*!
 * @brief Non-zero after QuietModeMessage() has been called.
 */
static int              quietMode = 0;


/*!
 * @brief Print the copyright banner once.
 */
void BannerMessage( void )
/************************/
{
    static int          alreadyPrinted;
    static char *       helpMsg = {
        banner1w( "C/C++ NMAKE Clone for " CPU_NAME " ", _NMAKE_CLONE_VERSION_ ) "\n"
        banner2( "1995" ) "\n"
        banner3 "\n"
        banner3a "\n"
    };

    if( !alreadyPrinted && !quietMode ) {
        printf( helpMsg );
        alreadyPrinted = 1;
    }
}


/*!
 * @brief Read a key from the console.
 *
 * Extended keys (prefixed by a zero byte) are folded into a single
 * return value.
 *
 * @return The key pressed.
 */
static int get_key( void )
/************************/
{
    int                 ch;

    ch = getch();
    if( ch == 0 ) {                     /* handle extended keys */
        ch = getch();
    }
    return( ch );
}

/*!
 * @brief Print the full usage message.
 *
 * Pauses every 15 lines and stops early if the user presses 'q'.
 */
void PrintHelpMessage( void )
/***************************/
{
    const int           lineCount = 15;
    int                 count;
    int                 num;
    int                 ch;

    BannerMessage();
    for( count=0,num=0; usageMsg[count]!=NULL; count++,num++ ) {
        if( num == lineCount ) {
            printf( "\t(Press return to continue)" );
            fflush( stdout );
            ch = get_key();
            printf( "\n" );
            if( ch == 'q' )  break;
            num = 0;
        }
        printf( "%s\n", usageMsg[count] );
    }
}


/*!
 * @brief Warn about one or more unsupported options.
 *
 * @param[in] opts  Space-separated list of unsupported options used.
 */
void UnsupportedOptsMessage( const char *opts )
/*********************************************/
{
    Warning( "Ignoring unsupported option(s): %s", opts );
}


/*!
 * @brief Disable printing of the copyright banner.
 */
void QuietModeMessage( void )
/***************************/
{
    quietMode = 1;
}
