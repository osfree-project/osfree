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
 * @file cmdline.h
 * @brief Sectioned command line argument storage.
 */

#ifndef _CMDLINE_H
#define _CMDLINE_H


/*!
 * @struct CmdLine
 * @brief A set of independent command line argument sections.
 *
 * Each section is a NULL-terminated argument vector.  @c merged
 * holds the last merged vector produced by MergeCmdLine and is
 * owned by the CmdLine.
 */
typedef struct CmdLine {
    char **             args;       /*!< array of character pointers */
    char **             merged;     /*!< merged version of args */
    int                 maxItems;   /*!< # of parameters before realloc */
    int                 curItems;   /*!< # of parameters currently stored */
    int                 numSections;/*!< # of command line sections available */
} CmdLine;


/*!
 * @def INVALID_MERGE_CMDLINE
 * @brief Terminates the list of sections passed to MergeCmdLine.
 */
#define INVALID_MERGE_CMDLINE   (-1)


/*!
 * @brief Initialize a set of command line sections.
 * @param[in] sections  Number of sections.  Must be >= 1.
 * @return The newly allocated CmdLine.
 */
extern CmdLine *InitCmdLine( int sections );
/*!
 * @brief Make a completely separate copy of a CmdLine.
 * @param[in] cmdLine  CmdLine to copy.
 * @return The new CmdLine.
 */
extern CmdLine *CloneCmdLine( const CmdLine *cmdLine );
/*!
 * @brief Free all memory occupied by a CmdLine.
 * @param[in,out] cmdLine  CmdLine to destroy.
 */
extern void     DestroyCmdLine( CmdLine *cmdLine );

/*!
 * @brief Append one parameter to a section.
 * @param[in,out] cmdLine  CmdLine to modify.
 * @param[in]     section  Section index.
 * @param[in]     parm     Parameter string, or NULL for a NULL slot.
 */
extern void     AppendCmdLine( CmdLine *cmdLine, int section,
                               const char *parm );
/*!
 * @brief Append a printf-formatted parameter to a section.
 * @param[in,out] cmdLine  CmdLine to modify.
 * @param[in]     section  Section index.
 * @param[in]     format   printf-style format string.
 * @param[in]     ...      Format arguments.
 */
extern void     AppendFmtCmdLine( CmdLine *cmdLine, int section,
                                  const char *format, ... );
/*!
 * @brief Merge selected sections into a single argument vector.
 * @param[in,out] cmdLine  CmdLine to merge.
 * @param[in]     section  First section or INVALID_MERGE_CMDLINE.
 * @param[in]     ...      Additional sections, terminated by
 *                         INVALID_MERGE_CMDLINE.
 * @return The merged NULL-terminated argument vector.
 */
extern char **  MergeCmdLine( CmdLine *cmdLine, int section, ... );


#endif
