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
 * @file context.c
 * @brief Command line, environment, and file parsing contexts.
 *
 * A context is a source of characters.  At most one context is
 * current; additional contexts may be pushed onto a fixed-size
 * stack.  The token scanners in cmdscan.c read characters through
 * this module.
 */

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "context.h"
#include "error.h"
#include "memory.h"

/*!
 * @def MAX_INDIRECTIONS
 * @brief Maximum depth of the context indirection stack.
 */
#define MAX_INDIRECTIONS        8


/*!
 * @struct Context
 * @brief One active or suspended parsing context.
 */
struct Context {
    int                 type;           /*!< context type */
    FILE *              fp;             /*!< for parsing data on disk */
    char *              data;           /*!< for parsing data in memory */
    char *              dataStart;      /*!< for parsing data in memory */
    unsigned            dataLen;        /*!< for parsing data in memory */
    long                markPos;        /*!< position of the context's mark */
};


/*!
 * @brief Currently active context.
 */
static struct Context   curContext;
/*!
 * @brief Non-zero when @c curContext has been initialised.
 */
static int              curContextInitialized = 0;

/*!
 * @brief Saved contexts for nested indirection.
 */
static struct Context   stack[MAX_INDIRECTIONS];
/*!
 * @brief Number of entries currently on @c stack.
 */
static int              stackItems = 0;



/*!
 * @brief Clear the specified context to an uninitialized state.
 * @param[out] context  Context to clear.
 */
static void clear_context( struct Context *context )
/**************************************************/
{
    memset( context, 0, sizeof(struct Context) );
    context->markPos = -1L;
}


/*!
 * @brief Open a new command line context.
 * @return 0 on success.
 */
int OpenCmdLineContext( void )
/****************************/
{
    int                 len;

    if( curContextInitialized )  Zoinks();
    clear_context( &curContext );

    /*** Make a copy of the command line ***/
    len = _bgetcmd( NULL, 0 ) + 1;
    curContext.dataStart = AllocMem( len );
    getcmd( curContext.dataStart );

    curContext.data = curContext.dataStart;
    curContext.dataLen = strlen( curContext.dataStart );
    curContextInitialized = 1;
    curContext.type = COMMAND_LINE_CONTEXT;
    return( 0 );
}


/*!
 * @brief Open a new environment variable context.
 * @param[in] envVar  Name of the environment variable.
 * @retval 0  Context opened.
 * @retval 1  Variable not set.
 */
int OpenEnvironContext( const char *envVar )
/******************************************/
{
    if( curContextInitialized )  Zoinks();
    clear_context( &curContext );

    curContext.data = getenv( envVar );
    curContext.dataStart = curContext.data;
    if( curContext.data == NULL )  return( 1 );
    curContext.dataLen = strlen( curContext.dataStart );

    curContextInitialized = 1;
    curContext.type = ENVIRON_VAR_CONTEXT;
    return( 0 );
}


/*!
 * @brief Open a new file context.
 * @param[in] filename  File to open in text mode.
 * @retval 0  Context opened.
 * @retval 1  File could not be opened.
 */
int OpenFileContext( const char *filename )
/*****************************************/
{
    if( curContextInitialized )  Zoinks();
    clear_context( &curContext );

    curContext.fp = fopen( filename, "rt" );
    if( curContext.fp == NULL )  return( 1 );

    curContextInitialized = 1;
    curContext.type = COMMAND_FILE_CONTEXT;
    return( 0 );
}


/*!
 * @brief Close the current context.
 */
void CloseContext( void )
/***********************/
{
    if( curContextInitialized ) {
        switch( curContext.type ) {
          case COMMAND_LINE_CONTEXT:
            free( curContext.dataStart );
          case ENVIRON_VAR_CONTEXT:
            break;
          case COMMAND_FILE_CONTEXT:
            fclose( curContext.fp );
            break;
          default:
            Zoinks();
        }
    }

    curContextInitialized = 0;
    clear_context( &curContext );
}


/*!
 * @brief Push the current context onto the context stack.
 *
 * The current context is disabled, but not closed.
 */
void PushContext( void )
/**********************/
{
    if( stackItems == MAX_INDIRECTIONS ) {
        FatalError( "Too many levels of context indirection." );
    }
    memcpy( &stack[stackItems], &curContext, sizeof(struct Context) );
    stackItems++;
    clear_context( &curContext );
    curContextInitialized = 0;
}


/*!
 * @brief Pop the top context off the context stack.
 *
 * The current context must be closed before calling PopContext.
 */
void PopContext( void )
/*********************/
{
    CloseContext();
    if( stackItems == 0 )  Zoinks();
    memcpy( &curContext, &stack[stackItems-1], sizeof(struct Context) );
    stackItems--;
    curContextInitialized = 1;
}


/*!
 * @brief Get a character from the current context.
 * @return The character, or NUL at end of context.
 */
int GetCharContext( void )
/************************/
{
    int                 ch;

    if( !curContextInitialized )  Zoinks();
    switch( curContext.type ) {
      case COMMAND_LINE_CONTEXT:
        /* fall through */
      case ENVIRON_VAR_CONTEXT:
        if( curContext.data - curContext.dataStart  <  curContext.dataLen ) {
            ch = *curContext.data;
        } else {
            ch = '\0';
        }
        curContext.data++;
        break;
      case COMMAND_FILE_CONTEXT:
        ch = fgetc( curContext.fp );
        if( ch == EOF )  ch = '\0';
        break;
      default:
        Zoinks();
    }
    return( ch );
}


/*!
 * @brief Unget a character back into the current context.
 */
void UngetCharContext( void )
/***************************/
{
    if( !curContextInitialized )  Zoinks();
    SetPosContext( GetPosContext() - 1 );
}


/*!
 * @brief Mark the current position in the current context.
 *
 * Only one mark at a time is stored; use GoToMarkContext to return
 * to it.
 */
void MarkPosContext( void )
/*************************/
{
    if( !curContextInitialized )  Zoinks();
    curContext.markPos = GetPosContext();
}


/*!
 * @brief Back up the current position to the last place marked by
 *        MarkPosContext.
 */
void GoToMarkContext( void )
/**************************/
{
    if( !curContextInitialized )  Zoinks();
    if( curContext.markPos == -1L )  Zoinks();
    SetPosContext( curContext.markPos );
}


/*!
 * @brief Get the current position within the current context.
 * @return The zero-based byte offset.
 */
int GetPosContext( void )
/***********************/
{
    if( !curContextInitialized )  Zoinks();
    switch( curContext.type ) {
      case COMMAND_LINE_CONTEXT:
        /* fall through */
      case ENVIRON_VAR_CONTEXT:
        return( curContext.data - curContext.dataStart );
        break;
      case COMMAND_FILE_CONTEXT:
        return( ftell( curContext.fp ) );
        break;
      default:
        Zoinks();
    }
    return( EOF );              /* dummy value so compiler won't complain */
}


/*!
 * @brief Set the current position within the current context.
 * @param[in] pos  New zero-based byte offset.
 */
void SetPosContext( int pos )
/***************************/
{
    if( !curContextInitialized )  Zoinks();
    if( pos < 0 )  Zoinks();
    switch( curContext.type ) {
      case COMMAND_LINE_CONTEXT:
        /* fall through */
      case ENVIRON_VAR_CONTEXT:
        curContext.data = curContext.dataStart + pos;
        break;
      case COMMAND_FILE_CONTEXT:
        fseek( curContext.fp, pos, SEEK_SET );
        break;
      default:
        Zoinks();
    }
}
