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
* Description:  Translate Microsoft NMAKE options to Watcom options.
*
****************************************************************************/

/*!
 * @file translat.c
 * @brief Translate Microsoft NMAKE options to Watcom wmake options.
 */

#include <stdlib.h>
#include <string.h>
#include "cmdline.h"
#include "error.h"
#include "nmake.h"
#include "message.h"
#include "memory.h"
#include "translat.h"
#include "system.h"

/*!
 * @def UNSUPPORTED_STR_SIZE
 * @brief Size of the buffer that accumulates unsupported option names.
 */
#define UNSUPPORTED_STR_SIZE    256


/*!
 * @brief Append one unsupported option name to the accumulated string.
 *
 * @param[in,out] optStr  Accumulated option string.  Not NULL.
 * @param[in]     opt     One-character option name.  Not NULL.
 */
static void append_unsupported( char *optStr, char *opt )
/*******************************************************/
{
    if( optStr[0] != '\0' ) {
        strcat( optStr, " -" );
    } else {
        strcat( optStr, "-" );
    }
    strcat( optStr, opt );
}


/*!
 * @brief Warn about unsupported Microsoft NMAKE options that were used.
 * @param[in] cmdOpts  Parsed options.  Not NULL.
 */
static void unsupported_opts( OPT_STORAGE *cmdOpts )
/**************************************************/
{
    char                opts[UNSUPPORTED_STR_SIZE];

    /*** Build a string listing all unsupported options that were used ***/
    opts[0] = '\0';
    if( cmdOpts->B )  append_unsupported( opts, "B" );
    if( cmdOpts->C )  append_unsupported( opts, "C" );
    if( cmdOpts->E )  append_unsupported( opts, "E" );
    if( cmdOpts->R )  append_unsupported( opts, "R" );
    if( cmdOpts->U )  append_unsupported( opts, "U" );
    if( cmdOpts->Y )  append_unsupported( opts, "Y" );
    if( cmdOpts->X )  append_unsupported( opts, "X" );
    /*** If an unsupported option was used, give a warning ***/
    if( opts[0] != '\0' ) {
        UnsupportedOptsMessage( opts );
    }

}


/*!
 * @brief Translate supported NMAKE options, targets and macros to wmake.
 *
 * @param[in] cmdOpts  Parsed options.  Not NULL.
 * @param[in,out] cmdLine  Command-line accumulator.  Not NULL.
 */
static void nmake_opts( OPT_STORAGE *cmdOpts, CmdLine *cmdLine )
/************************************************************/
{
    OPT_STRING *        optStr;

    if( cmdOpts->A ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-a" );
    }

    if( cmdOpts->D ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-d" );
    }

    if( cmdOpts->F ) {
        AppendFmtCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-f %s",cmdOpts->F_value->data );
    }

    if( cmdOpts->I ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-i" );
    }

    if( cmdOpts->K ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-k" );
    }

    if( cmdOpts->N ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-n" );
    }

    if( cmdOpts->P ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-p" );
    }

    if( cmdOpts->Q ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-q" );
    }

    if( cmdOpts->S ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-s" );
    }

    if( cmdOpts->T ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-t" );
    }

    if( cmdOpts->verbose ) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-v" );
    }

    /* transfer stored macros and targets */
    optStr = cmdOpts->t010101010101_value;
    while( optStr != NULL ) {
        AppendFmtCmdLine( cmdLine, NMAKE_OPTS_SECTION, "%s", optStr->data );
        optStr = optStr->next;
    }


}


/*!
 * @brief Activate options which are always on by default.
 * @param[in] cmdOpts      Parsed options.  Not NULL.
 * @param[in,out] cmdLine  Command-line accumulator.  Not NULL.
 */
static void default_opts( OPT_STORAGE *cmdOpts, CmdLine *cmdLine )
/**************************************************************/
{

    AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-ms" );
    if (!cmdOpts->nowopts) {
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-z" );
        AppendCmdLine( cmdLine, NMAKE_OPTS_SECTION, "-h" );
    }
}


/*!
 * @brief Forward any caller-supplied wmake options verbatim.
 * @param[in] cmdOpts      Parsed options.  Not NULL.
 * @param[in,out] cmdLine  Command-line accumulator.  Not NULL.
 */
static void watcom_opts( OPT_STORAGE *cmdOpts, CmdLine *cmdLine )
/**************************************************************/
{
    OPT_STRING *curr;

    if (cmdOpts->passwopts)
    {
        for (curr = cmdOpts->passwopts_value; curr; curr = curr->next)
        {
            AppendCmdLine(cmdLine, NMAKE_OPTS_SECTION, curr->data);
        }
    }

}


/*!
 * @brief Translate parsed Microsoft NMAKE options into Watcom wmake options.
 *
 * Handles /nologo, /help and /? directly, then calls the helpers that
 * add default, unsupported, supported and pass-through options to
 * @p cmdLine.
 *
 * @param[in]     cmdOpts  Parsed options.  Not NULL.
 * @param[in,out] cmdLine  Command-line accumulator.  Not NULL.
 */
void OptionsTranslate( OPT_STORAGE *cmdOpts, CmdLine *cmdLine )
/*************************************************************/
{
    /*** Parse the /nologo switch now so we can print the banner ***/
    if( cmdOpts->NOLOGO ) {
        QuietModeMessage();
    } else {
        BannerMessage();
    }

    if( cmdOpts->HELP || cmdOpts->_ ) {
        PrintHelpMessage();
        exit( EXIT_SUCCESS );

    }

    /*** Parse everything ***/
    default_opts( cmdOpts, cmdLine );
    unsupported_opts( cmdOpts );
    nmake_opts( cmdOpts, cmdLine );
    watcom_opts( cmdOpts, cmdLine );
}
