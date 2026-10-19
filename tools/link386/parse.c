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
* Description:  Command line parsing for LINK clone tool.
*
****************************************************************************/

/*!
 * @file parse.c
 * @brief Command-line parsing for the LINK clone tool.
 *
 * Provides the option-parsing entry points invoked from the shared
 * optparsc.gh driver, plus helpers for reading option arguments from
 * the current context (command line, environment variable or file).
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "link.h"
#include "cmdline.h"
#include "cmdscan.h"
#include "context.h"
#include "error.h"
#include "file.h"
#include "memory.h"
#include "message.h"
#include "parse.h"


/*!
 * @brief Initialize the OPT_STORAGE structure.
 * @param[out] cmdOpts  Option storage to initialise. Not NULL.
 */
void InitParse( OPT_STORAGE *cmdOpts )
/************************************/
{
    OPT_INIT( cmdOpts );
}


/*!
 * @brief Destroy the OPT_STORAGE structure.
 * @param[in,out] cmdOpts  Option storage to release. Not NULL.
 */
void FiniParse( OPT_STORAGE *cmdOpts )
/************************************/
{
    OPT_FINI( cmdOpts );
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
 * Handles switches ('-' or '/'), command files ('@') and input
 * filenames, incrementing @p itemsParsed for every item consumed.
 *
 * @param[in,out] cmdOpts      Option storage. Not NULL.
 * @param[in,out] itemsParsed  Item counter. Not NULL.
 */
void CmdStringParse( OPT_STORAGE *cmdOpts, int *itemsParsed )
/***********************************************************/
{
    int                 ch;
    char *              filename;

    for( ;; ) {
        /*** Find the start of the next item ***/
        CmdScanWhitespace();
        ch = GetCharContext();
        if( ch == '\0' )  break;
        MarkPosContext();               /* mark start of switch */

        /*** Handle switches, command files, and input files ***/
        if( ch == '-'  ||  ch == '/' ) {        /* switch */
            if( OPT_PROCESS( cmdOpts ) != 0 ) {
                cmd_line_error();
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
        } else {                                /* input file */
            UngetCharContext();
            filename = CmdScanFileName();
            AddFile( TYPE_DEFAULT_FILE, filename );
            FreeMem( filename );
        }
        (*itemsParsed)++;
    }
    CloseContext();
}


/*!
 * @brief Ensure the argument to /ALIGN is a power of two.
 *
 * Issues a warning and substitutes 4096 if @p *p is not a power of
 * two.
 *
 * @param[in,out] p  Value to validate; replaced on error. Not NULL.
 */
static void check_align( unsigned *p )
/************************************/
{
    int                 numFound = 0;
    unsigned            temp = *p;

    /*** Count the number of bits that are on ***/
    while( temp != 0 ) {
        if( temp & 0x0001 )  numFound++;
        temp >>= 1;
    }

    /*** Gripe if the number isn't a power of two ***/
    if( numFound != 1 ) {
        Warning( "Invalid argument %u to /ALIGN -- assuming 4096", *p );
        *p = 4096;
    }
}


/*!
 * @def skip_white
 * @brief Advance @c s past whitespace characters.
 * @param[in] s  String expression to advance.
 */
#define skip_white(s)      while (*s && isspace(*s)) ++s;
/*!
 * @def skip_nonwhite
 * @brief Advance @c s past non-whitespace characters.
 * @param[in] s  String expression to advance.
 */
#define skip_nonwhite(s)   while (*s && !isspace(*s)) ++s;

/*!
 * @brief Append a string to an OPT_STRING list.
 *
 * If @p quote is non-zero and @p str is not already quoted with
 * matching quotes, it is wrapped in @p quote characters.
 *
 * @param[in,out] p      List head; updated if the list is empty.
 * @param[in]     str    String to append. Not NULL.
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
 * @brief Destroy an OPT_STRING list.
 * @param[in,out] p  List head; set to NULL. Not NULL.
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
 * @brief Parse the /BASE:@{filename} option.
 *
 * Reads the named file, finds the line beginning with the requested
 * keyword, and stores the keyword's numeric value in @p p.
 *
 * @param[in,out] p    Option list to replace. Not NULL.
 * @param[in]     str  Buffer starting after the '@'. Not NULL.
 * @retval 0  Parsing failed (fatal error already issued).
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_base_from_file(OPT_STRING **p, char *str)
{
    FILE *stream;
    char *buffer;
    char *keyword;
    char *s;
    char *b;
    int   keylen;

    buffer = AllocMem(255);

    for (s = str + 1, b = buffer; *s && (*s != ',') && !isspace(*s); )
        *b++ = *s++;

    if (*s++ != ',')
    {
        FreeMem(buffer);
        FatalError("/BASE:@{filename} requires valid filename");
        return 0;
    }

    *b      = 0x00;
    keyword = s;

    skip_nonwhite(s);

    keylen = s - keyword;

    stream = fopen(buffer, "r");
    if (!stream)
    {
        FreeMem(buffer);
        FatalError("/BASE:@{filename} requires a valid filename");
        return 0;
    }

    while (fgets(buffer, 255, stream))
    {
        s = buffer;
        skip_white(s);

        if (*s == ';' || *s == '\0' )
            continue;

        if (!strnicmp(s, keyword, keylen) && isspace(s[keylen]))
        {
            s += keylen + 1;
            skip_white(s);

            if (!*s)
            {
                FreeMem(buffer);
                fclose(stream);
                FatalError("/BASE file contains invalid offset");
                return 0;
            }

            b = s;
            skip_nonwhite(b);
            *b = 0x00;

            OPT_CLEAN_STRING(p);
            add_string(p, s, 0);
            FreeMem(buffer);
            fclose(stream);

            return 1;
        }
    }

    FreeMem(buffer);
    fclose(stream);
    FatalError("/BASE file does not contain keyword");
    return 0;
} /* parse_base_from_file() */


/*!
 * @brief Parse the /BASE option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed (fatal error already issued).
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_base( OPT_STRING **p )
/*************************************/
{
    char *str;

    if (!CmdScanRecogChar(':'))
    {
        FatalError("/BASE requires an argument");
        return 0;
    }

    str = CmdScanString();
    if (str == NULL)
    {
        FatalError("/BASE requires an argument");
        return 0;
    }

    if (*str == '@')
    {
        return parse_base_from_file(p, str);
    }
    else
    {
        OPT_CLEAN_STRING(p);
        add_string(p, str, 0);
    }

    return 1;
} /* parse_base() */


/*!
 * @brief Common helper for options of the form `:string`.
 *
 * If @p onlyOne is non-zero, any previous string in @p p is removed
 * first.  If @p quote is non-zero, the argument is stored quoted.
 *
 * @param[in,out] p        Option list to append to. Not NULL.
 * @param[in]     optName  Option name used in error messages. Not NULL.
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

    if( !CmdScanRecogChar( ':' ) ) {
        FatalError( "/%s requires an argument", optName );
        return( 0 );
    }
    str = CmdScanString();
    if( str == NULL ) {
        FatalError( "/%s requires an argument", optName );
        return( 0 );
    }
    if( onlyOne )  OPT_CLEAN_STRING( p );
    add_string( p, str, quote );
    return( 1 );
}


/*!
 * @brief Parse the /COMMENT option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_comment( OPT_STRING **p )
/****************************************/
{
    return( do_string_parse( p, "COMMENT", 0, 0 ) );
}


/*!
 * @brief Parse the /DEBUG option.
 *
 * The `:<type>` argument is optional; if present it is stored.
 *
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 1  Parsed successfully (the option is always accepted).
 * @return 1.
 */
static int parse_debug( OPT_STRING **p )
/**************************************/
{
    char *              str;
    int                 ch;

    ch = GetCharContext();
    if( ch != ':' ) {                   /* optional :<type> */
        if( ch != '\0' )  UngetCharContext();
    } else {
        str = CmdScanString();
        if( str == NULL ) {
            FatalError( "Missing argument for /DEBUG" );
            return( 0 );
        } else {
            add_string( p, str, 0 );
            FreeMem( str );
        }
    }
    return( 1 );
}


/*!
 * @brief Parse the /DEBUGTYPE option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_debugtype( OPT_STRING **p )
/******************************************/
{
    return( do_string_parse( p, "DEBUGTYPE", 0, 0 ) );
}


/*!
 * @brief Parse the /DEF option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_def( OPT_STRING **p )
/************************************/
{
    return( do_string_parse( p, "DEF", 1, 0 ) );
}


/*!
 * @brief Parse the /DEFAULTLIB option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_defaultlib( OPT_STRING **p )
/*******************************************/
{
    return( do_string_parse( p, "DEFAULTLIB", 0, 0 ) );
}


/*!
 * @brief Parse the /ENTRY option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_entry( OPT_STRING **p )
/**************************************/
{
    return( do_string_parse( p, "ENTRY", 1, '\'' ) );
}


/*!
 * @brief Parse the /EXETYPE option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_exetype( OPT_STRING **p )
/****************************************/
{
    return( do_string_parse( p, "EXETYPE", 1, 0 ) );
}


/*!
 * @brief Parse the /EXPORT option.
 *
 * Accepts both Microsoft-style syntax
 * `/EXPORT:entryname[=internalname][,@ordinal[,NONAME]][,DATA]` and
 * Watcom-style syntax `entryName[.ordinal][=internalName]`.
 *
 * @param[in,out] optStr  Option list to append to. Not NULL.
 * @retval 0  Parsing failed (missing or malformed argument).
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_export( OPT_STRING **optStr )
/********************************************/
{
    char *              strStart;
    char *              str;
    char *              p;
    char *              entryName = NULL;
    char *              internalName = NULL;
    char *              ordinal = NULL;
    size_t              len;
    int                 retcode = 1;

    /*** Extract the export string ***/
    if( !CmdScanRecogChar( ':' ) ) {
        FatalError( "/EXPORT requires an argument" );
        return( 0 );
    }
    str = CmdScanString();
    strStart = str;
    if( str == NULL ) {
        FatalError( "/EXPORT requires an argument" );
        return( 0 );
    }

    /*** Extract the entryName ***/
    p = str;
    while( *p != '\0'  &&  *p != '='  &&  *p != ',' )  p++;
    if( p == str ) {
        retcode = 0;
    } else {
        len = p - str + 1;
        if( str[0] != '\'' || str[len-2] != '\'' ) {
            len += 2;
            entryName = AllocMem( len );
            entryName[0] = '\'';
            memcpy( &(entryName[1]), str, len-3 );
            entryName[len-2] = '\'';
        } else {
            entryName = AllocMem( len );
            memcpy( entryName, str, len-1 );
        }
        entryName[len-1] = '\0';
        str = p;
    }

    /*** Extract the internalName ***/
    if( *str == '='  &&  retcode != 0 ) {
        str++;
        p = str;
        while( *p != '\0'  &&  *p != ',' )  p++;
        if( p == str ) {
            retcode = 0;
        } else {
            len = p - str + 1;
            if( str[0] != '\'' || str[len-2] != '\'' ) {
                len += 2;
                internalName = AllocMem( len );
                internalName[0] = '\'';
                memcpy( &(internalName[1]), str, len-3 );
                internalName[len-2] = '\'';
            } else {
                internalName = AllocMem( len );
                memcpy( internalName, str, len-1 );
            }
            internalName[len-1] = '\0';
            str = p;
        }
    }

    /*** Extract the ordinal ***/
    if( *str == ','  &&  retcode != 0 ) {
        if( *(++str) != '@' ) {
            retcode = 0;
        } else {
            str++;
            p = str;
            while( *p != '\0'  &&  isdigit( *p ) )  p++;
            if( p == str ) {
                retcode = 0;
            } else {
                len = p - str + 1;
                ordinal = AllocMem( len );
                memcpy( ordinal, str, len-1 );
                ordinal[len-1] = '\0';
            }
        }
    }

    /*** Abort on error ***/
    if( retcode == 0 ) {
        if( entryName != NULL )  FreeMem( entryName );
        if( internalName != NULL )  FreeMem( internalName );
        if( ordinal != NULL )  FreeMem( ordinal );
        FreeMem( strStart );
        return( 0 );
    }

    /*** Merge together Watcom-style:  entryName[.ordinal][=internalName] ***/
    FreeMem( strStart );
    len = strlen( entryName );
    if( ordinal != NULL ) {
        len++;                          /* for '.' */
        len += strlen( ordinal );
    }
    if( internalName != NULL ) {
        len++;                          /* for '=' */
        len += strlen( internalName );
    }
    str = AllocMem( len );
    *str = '\0';
    strcat( str, entryName );
    if( ordinal != NULL ) {
        strcat( str, "." );
        strcat( str, ordinal );
    }
    if( internalName != NULL ) {
        strcat( str, "=" );
        strcat( str, internalName );
    }
    add_string( optStr, str, 0 );
    FreeMem( entryName );
    if( internalName != NULL )  FreeMem( internalName );
    if( ordinal != NULL )  FreeMem( ordinal );

    return( 1 );
}


/*!
 * @brief Parse the /HEAP option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_heap( OPT_STRING **p )
/*************************************/
{
    return( do_string_parse( p, "HEAP", 1, 0 ) );
}


/*!
 * @brief Parse the /IMPLIB option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_implib( OPT_STRING **p )
/***************************************/
{
    return( do_string_parse( p, "IMPLIB", 1, 0 ) );
}


/*!
 * @brief Parse the /INCLUDE option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_include( OPT_STRING **p )
/****************************************/
{
    return( do_string_parse( p, "INCLUDE", 0, '\'' ) );
}


/*!
 * @brief Parse the /INCREMENTAL option.
 *
 * Accepts only `:yes` or `:no`.
 *
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_incremental( OPT_STRING **p )
/********************************************/
{
    char *              str;

    if( !CmdScanRecogChar( ':' ) ) {
        FatalError( "/INCREMENTAL requires an argument" );
        return( 0 );
    }
    str = CmdScanString();
    if( str == NULL ) {
        FatalError( "/INCREMENTAL requires an argument" );
        return( 0 );
    }
    if( !stricmp( str, "yes" )  ||  !stricmp( str, "no" ) ) {
        add_string( p, str, 0 );
        return( 1 );
    } else {
        FatalError( "Invalid argument '%s' to /INCREMENTAL", str );
        return( 0 );
    }
}


/*!
 * @brief Parse the undocumented /INTERNALDLLNAME_ option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_internaldllname_( OPT_STRING **p )
/*************************************************/
{
    return( do_string_parse( p, "INTERNALDLLNAME_", 1, 0 ) );
}


/*!
 * @brief Parse the /MACHINE option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_machine( OPT_STRING **p )
/****************************************/
{
    return( do_string_parse( p, "MACHINE", 0, 0 ) );
}


/*!
 * @brief Parse the /MAP option.
 *
 * The filename argument is optional.
 *
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 1  Parsed successfully.
 * @return 1.
 */
static int parse_map( OPT_STRING **p )
/************************************/
{
    char *              str;

    if( !CmdScanRecogChar( ':' ) )  return( 1 );
    str = CmdScanString();
    if( str != NULL ) {
        add_string( p, str, 0 );
    }
    return( 1 );
}


/*!
 * @brief Parse the /ORDER option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_order( OPT_STRING **p )
/**************************************/
{
    return( do_string_parse( p, "ORDER", 0, 0 ) );
}


/*!
 * @brief Parse the /OUT option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_out( OPT_STRING **p )
/************************************/
{
    return( do_string_parse( p, "OUT", 1, 0 ) );
}


/*!
 * @brief Parse the /passwopts option.
 *
 * If the argument is wrapped in double quotes they are stripped before
 * storing.
 *
 * @param[in,out] p  Option list to append to. Not NULL.
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
 * @brief Parse the /PDB option.
 * @param[in,out] p  Option list to append to. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_pdb( OPT_STRING **p )
/************************************/
{
    return( do_string_parse( p, "PDB", 0, 0 ) );
}


/*!
 * @brief Parse the /SECTION option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_section( OPT_STRING **p )
/****************************************/
{
    return( do_string_parse( p, "SECTION", 1, 0 ) );
}


/*!
 * @brief Parse the /STACK option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_stack( OPT_STRING **p )
/**************************************/
{
    return( do_string_parse( p, "STACK", 1, 0 ) );
}


/*!
 * @brief Parse the /STUB option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_stub( OPT_STRING **p )
/*************************************/
{
    return( do_string_parse( p, "STUB", 1, 0 ) );
}


/*!
 * @brief Parse the /SUBSYSTEM option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_subsystem( OPT_STRING **p )
/******************************************/
{
    return( do_string_parse( p, "SUBSYSTEM", 1, 0 ) );
}


/*!
 * @brief Parse the /VERSION option.
 * @param[in,out] p  Option list to replace. Not NULL.
 * @retval 0  Parsing failed.
 * @retval 1  Parsed successfully.
 * @return 1 on success.
 */
static int parse_version( OPT_STRING **p )
/****************************************/
{
    return( do_string_parse( p, "VERSION", 1, 0 ) );
}


/*!
 * @brief Handle /DEBUG by implying /OPT:NOREF.
 *
 * @param[in,out] cmdOpts  Option storage. Not NULL.
 * @param[in]     x        Ignored (present for option-table signature).
 */
static void handle_debug( OPT_STORAGE *cmdOpts, int x )
/*****************************************************/
{
    x = x;
    cmdOpts->opt_level = OPT_opt_level_optnoref;

    if( cmdOpts->debug_value != NULL ) {
        // suppress this diagnostic,
        // it appears that MS ignores this option also.
        //Warning( "Ignoring unsupported argument '%s' to /DEBUG", cmdOpts->debug_value->data );
        OPT_CLEAN_STRING( &cmdOpts->debug_value );
    }
}


/*!
 * @brief Handle /NOWARN by disabling warning output.
 *
 * @param[in,out] cmdOpts  Option storage. Not NULL.
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


/*!
 * @brief Scan a ':'-prefixed number from the current context.
 *
 * No leading whitespace is permitted.
 *
 * @param[out] p  Receiver for the parsed value. Not NULL.
 * @retval 1  A number was parsed.
 * @retval 0  No number was parsed.
 * @return 1 on success.
 */
static int OPT_GET_NUMBER( unsigned *p )
/**************************************/
{
    unsigned            value;

    if( !CmdScanRecogChar( ':' ) )  return( 0 );
    if( CmdScanNumber( &value ) ) {
        *p = value;
        return( 1 );
    } else {
        return( 0 );
    }
}


/* Include after all static functions were declared */
#include "optparsc.gh"
