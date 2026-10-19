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
* Description:  Command line parsing for NMAKE clone tool.
*
****************************************************************************/

/*!
 * @file parse.c
 * @brief Command-line parsing for the NMAKE clone tool.
 *
 * Provides the option-parsing entry points invoked from the shared
 * optparsc.gh driver, plus helpers for reading option arguments from
 * the current context (command line, environment variable or file).
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "nmake.h"
#include "cmdline.h"
#include "cmdscan.h"
#include "context.h"
#include "error.h"
#include "memory.h"
#include "message.h"
#include "parse.h"


/*!
 * @brief Initialize the OPT_STORAGE structure.
 * @param[out] cmdOpts  Option storage to initialise.  Not NULL.
 */
void InitParse( OPT_STORAGE *cmdOpts )
/************************************/
{
    OPT_INIT( cmdOpts );
}


/*!
 * @brief Destroy the OPT_STORAGE structure.
 * @param[in,out] cmdOpts  Option storage to release.  Not NULL.
 */
void FiniParse( OPT_STORAGE *cmdOpts )
/************************************/
{
    OPT_FINI( cmdOpts );
}


/*!
 * @brief Append a string to an OPT_STRING list.
 *
 * If @p quote is non-zero and @p str is not already quoted with
 * matching quotes, it is wrapped in @p quote characters.
 *
 * @param[in,out] p      List head; updated if the list is empty.
 * @param[in]     str    String to append.  Not NULL.
 * @param[in]     quote  Quote character, or zero for no quoting.
 */
static void add_string( OPT_STRING **p, char *str, char quote )
/*************************************************************/
{
    OPT_STRING *        buf;
    OPT_STRING *        curElem;
    int                 len;
    int                 add_quote = 0;

    len = strlen(str);
    if( quote != 0 ) {
        do {
            if( str[0] == '"'  && str[len-1] == '"'  ) break;
            if( str[0] == '\'' && str[len-1] == '\'' ) break;
            len += 2;
            add_quote = 1;
        } while( 0 );
    }
    /*** Make a new list item ***/
    buf = AllocMem( sizeof(OPT_STRING) + len );
    if( add_quote ) {
        buf->data[0] = quote;
        strcpy( &(buf->data[1]), str );
        buf->data[len-1] = quote;
        buf->data[len] = '\0';
    } else {
        strcpy( buf->data, str );
    }
    buf->next = NULL;

    /*** Put it at the end of the list ***/
    if( *p == NULL ) {
        *p = buf;
    } else {
        curElem = *p;
        while( curElem->next != NULL )  curElem = curElem->next;
        curElem->next = buf;
    }
}


/*!
 * @brief Report a command-line error and continue.
 *
 * Reads the offending token from the current context and issues a
 * warning.
 */
static void cmd_line_error( void )
/********************************/
{
    char *              str;

    GoToMarkContext();
    str = CmdScanString();
    Warning( "Ignoring invalid option '%s'", str );
}


/*!
 * @brief Parse the command string contained in the current context.
 *
 * Handles switches ('-' or '/'), command files ('@') and targets and
 * macros, incrementing @p itemsParsed for every item consumed.
 *
 * @param[in,out] cmdOpts      Option storage.  Not NULL.
 * @param[in,out] itemsParsed  Item counter.  Not NULL.
 */
void CmdStringParse( OPT_STORAGE *cmdOpts, int *itemsParsed )
/***********************************************************/
{
    int                 ch;
    char *              filename;
    char *              str;

    for( ;; ) {
        /*** Find the start of the next item ***/
        CmdScanWhitespace();
        ch = GetCharContext();
        if( ch == '\0' )  break;
        MarkPosContext();               /* mark start of switch */

        /*** Handle switches, command files, and input files ***/
        if( ch == '-'  ||  ch == '/' ) {        /* switch */
            if( OPT_PROCESS( cmdOpts ) != 0 ) {
                /*
                 * Switch didn't match, if user entered empty switch,
                 * just be silent like MS's nmake does.
                 */

                ch = GetCharContext();
                if( ch != '\0' && !isspace(ch)) {
                    cmd_line_error();
                }
            }
        } else if( ch == '@' ) {                /* command file */
            filename = CmdScanFileNameWithoutQuotes();
            PushContext();
            if( OpenFileContext( filename ) ) {
                FatalError( "Cannot open '%s'.", filename );
            }
            FreeMem( filename );
            CmdStringParse( cmdOpts, itemsParsed );
            PopContext();
        } else {                                /* targets and macros */
            UngetCharContext();
            str = CmdScanString();
            add_string( &(cmdOpts->t010101010101_value), str, 0 );
            cmdOpts->t010101010101 = 1;
        }
        (*itemsParsed)++;
    }
    CloseContext();
}


/*!
 * @brief Handle /NOWARN by disabling warning output.
 *
 * @param[in,out] cmdOpts  Option storage.  Not NULL.
 * @param[in]     x        Ignored (present for option-table signature).
 */
static void handle_nowwarn( OPT_STORAGE *cmdOpts, int x )
/*******************************************************/
{
    x = x;
    cmdOpts = cmdOpts;
    DisableWarnings( 1 );
}



/*!
 * @brief Handle the t010101010101 option (stored target/macro list).
 *
 * @param[in,out] p  Option list.  Unused.
 * @retval 1  Always succeeds.
 * @return 1.
 */
static int parse_t010101010101( OPT_STRING **p )
/******************************************/
{
    return( 1 );
}


/*!
 * @brief Destroy an OPT_STRING list.
 * @param[in,out] p  List head; set to NULL.  Not NULL.
 */
static void OPT_CLEAN_STRING( OPT_STRING **p )
/********************************************/
{
    OPT_STRING *        s;

    while( *p != NULL ) {
        s = *p;
        *p = s->next;
        FreeMem( s );
    }
}


/*!
 * @brief Common helper for options of the form `:string`.
 *
 * If @p onlyOne is non-zero, any previous string in @p p is removed
 * first.  If @p quote is non-zero, the argument is stored quoted.
 *
 * @param[in,out] p        Option list to append to.  Not NULL.
 * @param[in]     optName  Option name used in error messages.  Not NULL.
 * @param[in]     onlyOne  Non-zero to replace existing strings.
 * @param[in]     quote    Quote character, or zero for no quoting.
 * @retval 0  Parsing failed (fatal error already issued).
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int do_string_parse( OPT_STRING **p, char *optName, int onlyOne,
/*********************************************************************/
                            int quote )
{
    char *              str;

    CmdScanWhitespace();
    str = CmdScanString();
    if( str == NULL ) {
        FatalError( "/%s option requires a filename", optName );
        return( 0 );
    }
    if( onlyOne )  OPT_CLEAN_STRING( p );
    add_string( p, str, quote );
    return( 1 );
}


/*!
 * @brief Parse the /F option.
 * @param[in,out] p  Option list to replace.  Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_F( OPT_STRING **p )
/******************************************/
{
    return( do_string_parse( p, "F", 1, 0 ) );
}

/*!
 * @brief Parse the /X option.
 * @param[in,out] p  Option list to replace.  Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_X( OPT_STRING **p )
/******************************************/
{
    return( do_string_parse( p, "X", 1, 0 ) );
}

/*!
 * @brief Parse a run of combined single-letter options.
 *
 * Combined options are recognized by the absence of a '-' or '/'
 * prefix on subsequent characters.  Notably, `-L` must be handled
 * specially because it may be translated to `-NOLOGO` if it was the
 * first (or only) parameter on the command line.
 *
 * @param[in,out] cmdOpts  Option storage.  Not NULL.
 * @param[in]     x        Ignored (present for option-table signature).
 * @retval 1  Parsing completed, or terminated by whitespace.
 * @retval 0  Unknown option character.
 * @return 1 on success, 0 on unknown option.
 */
static int parse_combining( OPT_STORAGE *cmdOpts, int x )
/******************************************/
{
    int                 ch;

    x = x;

    /*
     * Make sure -L is translated correctly to -NOLOGO if it is
     * a first (or the only) parameter
     */
    if( cmdOpts->L ) {
        cmdOpts->NOLOGO = 1;
    }

    /* scan for combined options */
    do {
        /* get next character */
        ch = toupper( GetCharContext() );

        switch( ch ) {
        case 'A':  cmdOpts->A = 1; break;      /* gml-option: A */
        case 'B':  cmdOpts->B = 1; break;      /* gml-option: B */
        case 'C':  cmdOpts->C = 1; break;      /* gml-option: C */
        case 'D':  cmdOpts->D = 1; break;      /* gml-option: D */
        case 'E':  cmdOpts->E = 1; break;      /* gml-option: E */
        case 'I':  cmdOpts->I = 1; break;      /* gml-option: I */
        case 'K':  cmdOpts->K = 1; break;      /* gml-option: K */
        case 'L':  cmdOpts->NOLOGO = 1; break; /* gml-option: L */
        case 'N':  cmdOpts->N = 1; break;      /* gml-option: N */
        case 'P':  cmdOpts->P = 1; break;      /* gml-option: P */
        case 'Q':  cmdOpts->Q = 1; break;      /* gml-option: Q */
        case 'R':  cmdOpts->R = 1; break;      /* gml-option: R */
        case 'S':  cmdOpts->S = 1; break;      /* gml-option: S */
        case 'T':  cmdOpts->T = 1; break;      /* gml-option: T */
        case 'U':  cmdOpts->U = 1; break;      /* gml-option: U */
        case 'Y':  cmdOpts->Y = 1; break;      /* gml-option: Y */
        case '\0': break;
        default:
            /* if character is space, return without an error */
            if( isspace( ch ) )
                return 1;

            return 0;
        }
    } while( ch != '\0' );

    /* all went nicely, return success */
    return 1;
}

/*!
 * @brief Parse the /passwopts option.
 *
 * If the argument is wrapped in double quotes they are stripped before
 * storing.
 *
 * @param[in,out] p  Option list to append to.  Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_passwopts( OPT_STRING **p )
{
    char *str;
    char *src;
    char *dst;

    if (!CmdScanRecogChar(':'))
    {
        FatalError("/passwopts:{argument} requires an argument");
        return 0;
    }

    str = CmdScanString();
    if (str == NULL)
    {
        FatalError("/passwopts requires an argument");
        return 0;
    }

    /*
     * If quoted, stip out the quote characters.
     */
    if (*str == '\"')
    {
        for (dst = str, src = str + 1; *src && (*src != '\"'); )
        {
            *dst++ = *src++;
        }

        if (*src != '\"')
        {
            FatalError("/passwopts argument is missing closing quote");
            return 0;
        }

        *dst = 0x00;
    }

    add_string(p, str, 0);
    return 1;
} /* parse_passwopts() */


/*!
 * @brief Return the next character, converted to lowercase.
 *
 * LINK's options are not case-sensitive, so callers routinely
 * convert the returned character to lowercase.  Advances the
 * context to the next character.
 *
 * @return The next character converted to lowercase.
 */
static int OPT_GET_LOWER( void )
/******************************/
{
    return( tolower( GetCharContext() ) );
}


/*!
 * @brief Recognise a character in either case.
 *
 * If the next character is @p ch in either uppercase or lowercase, it
 * is consumed and a non-zero value is returned; otherwise, it is not
 * consumed and zero is returned.
 *
 * @param[in] ch  Character to recognise.
 * @retval 1  Character matched and consumed.
 * @retval 0  Character did not match.
 * @return Non-zero if the character was recognised.
 */
static int OPT_RECOG_LOWER( int ch )
/**********************************/
{
    return( CmdScanRecogChar( ch ) );
}


/*!
 * @brief Back up one character in the current context.
 */
static void OPT_UNGET( void )
/***************************/
{
    UngetCharContext();
}


/* Include after all static functions were declared */
#include "optparsc.gh"
