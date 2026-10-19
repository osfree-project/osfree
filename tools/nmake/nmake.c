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
* Description:  Microsoft NMAKE clone tool.
*
****************************************************************************/

/*!
 * @file nmake.c
 * @brief Microsoft NMAKE clone front-end.
 *
 * Parses the NMAKE-style command line (including the NMAKE_OPTIONS and
 * NMAKE environment variables), translates it to wmake options via
 * OptionsTranslate(), and spawns wmake.
 */

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "cmdline.h"
#include "context.h"
#include "error.h"
#include "nmake.h"
#include "message.h"
#include "parse.h"
#include "translat.h"
#include "system.h"


#if defined(__TARGET_386__) || defined(__TARGET_AXP__) || defined(__TARGET_PPC__)
/*!
 * @def MAKE
 * @brief Underlying make program that receives the translated options.
 */
    #define MAKE                "wmake"
#else
    #error Unrecognized CPU type
#endif

/*!
 * @def NMAKE_SUCCESS
 * @brief Exit status returned by nmake() on success.
 */
#define NMAKE_SUCCESS           0
/*!
 * @def NMAKE_ERROR
 * @brief Exit status returned by nmake() when wmake reported an error.
 */
#define NMAKE_ERROR             (-2)


/*!
 * @brief Top-level parsing routine.
 *
 * Processes, in order, the NMAKE_OPTIONS environment variable, the
 * NMAKE environment variable, and the command line.
 *
 * @param[in,out] cmdOpts  Option storage.  Not NULL.
 * @return Number of items parsed.
 */
static int do_parsing( OPT_STORAGE *cmdOpts )
/*******************************************/
{
    int                 itemsParsed = 0;

    /*** Process the NMAKE_OPTIONS environment variable ***/
    if( OpenEnvironContext( "NMAKE_OPTIONS" )  ==  0 ) {
        CmdStringParse( cmdOpts, &itemsParsed );
    }

    /*** Process the NMAKE environment variable ***/
    if( OpenEnvironContext( "NMAKE" )  ==  0 ) {
        CmdStringParse( cmdOpts, &itemsParsed );
    }

    /*** Process the command line ***/
    OpenCmdLineContext();
    CmdStringParse( cmdOpts, &itemsParsed );

    return( itemsParsed );
}


/*!
 * @brief Spawn the Watcom wmake with the translated options.
 *
 * Builds the MAKE/MAKEDIR/MAKEFLAGS built-in macros and appends them
 * to the command line, then invokes wmake with P_WAIT.
 *
 * @param[in] cmdOpts      Parsed options.  Not NULL.
 * @param[in,out] cmdLine  Command-line accumulator.  Not NULL.
 * @retval NMAKE_SUCCESS  wmake ran and returned success.
 * @retval NMAKE_ERROR    wmake ran but reported a non-zero exit status.
 * @return NMAKE_SUCCESS on success, NMAKE_ERROR on a reported failure.
 */
static int nmake( const OPT_STORAGE *cmdOpts, CmdLine *cmdLine )
/**************************************************************/
{
    char **             args;
    int                 rc;
    int                 count;
    char *              cwd;
    char                flagstmp[32] = {0};

    /*** get value for MAKEDIR field ***/
    cwd = getcwd( NULL, 0 );

    /*** construct MAKEFLAGS field ***/
    if( cmdOpts->A )      strcat(flagstmp, "A");
    if( cmdOpts->C )      strcat(flagstmp, "C");
    if( cmdOpts->D )      strcat(flagstmp, "D");
    if( cmdOpts->E )      strcat(flagstmp, "E");
    if( cmdOpts->NOLOGO ) strcat(flagstmp, "L");
    if( cmdOpts->N )      strcat(flagstmp, "N");
    if( cmdOpts->P )      strcat(flagstmp, "P");
    if( cmdOpts->R )      strcat(flagstmp, "R");
    if( cmdOpts->S )      strcat(flagstmp, "S");
    if( cmdOpts->U )      strcat(flagstmp, "U");
    if( cmdOpts->Y )      strcat(flagstmp, "Y");

    /*** pass builtin macros to wmake, so nmake wrapper gets called in recursive actions ***/
    AppendFmtCmdLine( cmdLine, NMAKE_OPTS_SECTION, "MAKE=\"%s\"", "nmake" );
    AppendFmtCmdLine( cmdLine, NMAKE_OPTS_SECTION, "MAKEDIR=\"%s\"", cwd );
    AppendFmtCmdLine( cmdLine, NMAKE_OPTS_SECTION, "MAKEFLAGS=\"%s\"", flagstmp );

    /*** merge commands ***/
    AppendCmdLine( cmdLine, NMAKE_PROGNAME_SECTION, MAKE );
    args = MergeCmdLine( cmdLine, INVALID_MERGE_CMDLINE );

    /*** Spawn the wmake ***/
    if( cmdOpts->showwopts ) {
        for( count=0; args[count]!=NULL; count++ ) {
            fprintf( stderr, "%s ", args[count] );
        }
        fprintf( stderr, "\n" );
    }
    if( !cmdOpts->noinvoke ) {
        rc = spawnvp( P_WAIT, MAKE, (const char **)args );
        if( rc != 0 ) {
            if( rc == -1  ||  rc == 255 ) {
                FatalError( "Unable to execute '%s'", MAKE );
            } else {
                return( NMAKE_ERROR );
            }
        }
    }
    DestroyCmdLine( cmdLine );

    return( NMAKE_SUCCESS );
}


/*!
 * @brief Program entry point.
 *
 * Installs the banner function, initializes the option and command-line
 * accumulators, parses the command line, translates the options, spawns
 * wmake, and exits.
 *
 * @param[in] argc  Argument count (unused).
 * @param[in] argv  Argument vector (unused).
 */
void main( int argc, char *argv[] )
/*********************************/
{
    OPT_STORAGE         cmdOpts;
    CmdLine *           cmdLine;


    /*** Initialize ***/
    SetBannerFuncError( BannerMessage );
    cmdLine = InitCmdLine( NMAKE_NUM_SECTIONS );

    /*** Parse the command line and translate to Watcom options ***/
    InitParse( &cmdOpts );
    do_parsing( &cmdOpts );
    OptionsTranslate( &cmdOpts, cmdLine );

    /*** Spawn the librarian ***/
    nmake( &cmdOpts, cmdLine );
    FiniParse( &cmdOpts );
    exit( EXIT_SUCCESS );
}
