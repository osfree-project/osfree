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
 * @file context.h
 * @brief Command line, environment, and file parsing contexts.
 */

#ifndef _CONTEXT_H
#define _CONTEXT_H


/*!
 * @def COMMAND_LINE_CONTEXT
 * @brief Context type: the process command line.
 */
#define COMMAND_LINE_CONTEXT    1
/*!
 * @def ENVIRON_VAR_CONTEXT
 * @brief Context type: an environment variable.
 */
#define ENVIRON_VAR_CONTEXT     2
/*!
 * @def COMMAND_FILE_CONTEXT
 * @brief Context type: a text file.
 */
#define COMMAND_FILE_CONTEXT    3


/*!
 * @def MAX_INDIRECTION
 * @brief Maximum acceptable context indirection depth.
 */
#define MAX_INDIRECTION         16


/*!
 * @brief Open a new command line context.
 * @return 0 on success.
 */
extern int      OpenCmdLineContext( void );
/*!
 * @brief Open a new environment variable context.
 * @param[in] envVar  Name of the environment variable.
 * @retval 0  Context opened.
 * @retval 1  Variable not set.
 */
extern int      OpenEnvironContext( const char *envVar );
/*!
 * @brief Open a new file context.
 * @param[in] filename  File to open.
 * @retval 0  Context opened.
 * @retval 1  File could not be opened.
 */
extern int      OpenFileContext( const char *filename );
/*!
 * @brief Close the current context.
 */
extern void     CloseContext( void );

/*!
 * @brief Push the current context onto the context stack.
 */
extern void     PushContext( void );
/*!
 * @brief Pop the top context off the context stack.
 */
extern void     PopContext( void );

/*!
 * @brief Get a character from the current context.
 * @return The character, or NUL at end of context.
 */
extern int      GetCharContext( void );
/*!
 * @brief Unget a character back into the current context.
 */
extern void     UngetCharContext( void );

/*!
 * @brief Mark the current position in the current context.
 */
extern void     MarkPosContext( void );
/*!
 * @brief Back up to the last marked position.
 */
extern void     GoToMarkContext( void );
/*!
 * @brief Get the current position within the current context.
 * @return The zero-based byte offset.
 */
extern int      GetPosContext( void );
/*!
 * @brief Set the current position within the current context.
 * @param[in] pos  New zero-based byte offset.
 */
extern void     SetPosContext( int pos );


#endif
