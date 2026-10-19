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
* Description:  Multi-section command-line container.
*
****************************************************************************/

/*!
 * @file cmdline.c
 * @brief Multi-section command-line container.
 *
 * A CmdLine holds one or more independent argument lists (sections).
 * Items are appended one at a time with AppendCmdLine() or
 * AppendFmtCmdLine(), then merged into a single NULL-terminated argv
 * with MergeCmdLine().
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdline.h"
#include "error.h"
#include "memory.h"

/*!
 * @def GRANULARITY
 * @brief Number of extra slots reserved when a section overflows.
 */
#define GRANULARITY     30
/*!
 * @def BUFFER_SIZE
 * @brief Size of the static buffer used by AppendFmtCmdLine().
 */
#define BUFFER_SIZE     1024


/*!
 * @brief Initialize the command-line sections.
 *
 * Use AppendCmdLine() to insert an item into any of these sections,
 * and MergeCmdLine() to merge them all together.  Valid sections will
 * be in the range [0, sections-1].
 *
 * @param[in] sections  Number of sections to create.  Must be >= 1.
 * @return Pointer to the new CmdLine.
 */
CmdLine *InitCmdLine( int sections )
/**********************************/
{
    int                 count;
    CmdLine *           cmdLine;

    /*** Allocate the sections ***/
    if( sections < 1 )  Zoinks();
    cmdLine = AllocMem( sections * sizeof(struct CmdLine) );
    cmdLine->numSections = sections;
    cmdLine->merged = NULL;

    /*** Initialize each section ***/
    for( count=0; count<sections; count++ ) {
        cmdLine[count].maxItems = GRANULARITY;  /* some space to start with */
        cmdLine[count].args = AllocMem( cmdLine[count].maxItems * sizeof(char*) );
        cmdLine[count].curItems = 0;            /* no parameters stored yet */
    }

    return( cmdLine );
}


/*!
 * @brief Make a completely separate copy of a CmdLine.
 * @param[in] oldCmdLine  Source CmdLine.  Not NULL.
 * @return Newly allocated copy.
 */
CmdLine *CloneCmdLine( const CmdLine *oldCmdLine )
/************************************************/
{
    CmdLine *           newCmdLine;
    int                 countSection;
    int                 countArg;

    newCmdLine = InitCmdLine( oldCmdLine->numSections );
    for( countSection=0; countSection<oldCmdLine->numSections; countSection++ ) {
        for( countArg=0; countArg<oldCmdLine[countSection].curItems; countArg++ ) {
            AppendCmdLine( newCmdLine, countSection, oldCmdLine[countSection].args[countArg] );
        }
    }

    return( newCmdLine );
}


/*!
 * @brief Free all memory occupied by a CmdLine, invalidating it.
 * @param[in] cmdLine  CmdLine to destroy.  Not NULL.
 */
void DestroyCmdLine( CmdLine *cmdLine )
/*************************************/
{
    int                 countSection;
    int                 countArg;

    for( countSection=0; countSection<cmdLine->numSections; countSection++ ) {
        for( countArg=0; countArg<cmdLine[countSection].curItems; countArg++ ) {
            FreeMem( cmdLine[countSection].args[countArg] );
        }
        FreeMem( cmdLine[countSection].args );
    }
    FreeMem( cmdLine );
}


/*!
 * @brief Append one argument to a section.
 *
 * Reallocates memory if necessary, and does all necessary bookkeeping.
 * The stored string is duplicated.
 *
 * @param[in,out] cmdLine  CmdLine.  Not NULL.
 * @param[in]     section  Section index (0-based).  Must be valid.
 * @param[in]     parm     Argument string.  May be NULL.
 */
void AppendCmdLine( CmdLine *cmdLine, int section, const char *parm )
/*******************************************************************/
{
    /*** Allocate GRANULARITY more elements if we're out of room ***/
    if( section < 0  ||  section >= cmdLine->numSections )  Zoinks();
    if( cmdLine[section].curItems+2 > cmdLine[section].maxItems ) {
        cmdLine[section].args = ReallocMem( cmdLine[section].args, (cmdLine[section].maxItems+GRANULARITY) * sizeof(char*) );
        cmdLine[section].maxItems += GRANULARITY;
    }

    /*** Store the specified pointer and update curItems ***/
    if( parm != NULL )  parm = DupStrMem( parm );               /* copy it */
    cmdLine[section].args[ cmdLine[section].curItems ] = (char*)parm;
    cmdLine[section].args[ cmdLine[section].curItems+1 ] = NULL;/* last one */
    cmdLine[section].curItems++;
}


/*!
 * @brief Append one formatted argument to a section.
 *
 * The stored string is generated using the printf-style format string
 * and any additional parameters.
 *
 * @param[in,out] cmdLine  CmdLine.  Not NULL.
 * @param[in]     section  Section index (0-based).  Must be valid.
 * @param[in]     format   printf-style format string.  Not NULL.
 * @param[in]     ...      Arguments for @p format.
 */
void AppendFmtCmdLine( CmdLine *cmdLine, int section, const char *format, ... )
/*****************************************************************************/
{
    static char         buf[BUFFER_SIZE];
    va_list             args;

    /*** Format it, then pass it though to AppendCmdLine ***/
    if( section < 0  ||  section >= cmdLine->numSections )  Zoinks();
    va_start( args, format );
    _vbprintf( buf, BUFFER_SIZE, format, args );
    va_end( args );
    AppendCmdLine( cmdLine, section, buf );
}


/*!
 * @brief Merge sections into a single NULL-terminated argv vector.
 *
 * Returns a pointer to the merged argument vector; the caller should
 * not modify its contents.  Any call to this function invalidates the
 * previously returned pointer for the same CmdLine.  If @p section is
 * INVALID_MERGE_CMDLINE, all sections are merged in order.  Otherwise,
 * @p section is the first item in an INVALID_MERGE_CMDLINE-terminated
 * list of section numbers; the merged command line will consist of
 * items from the specified sections, in order.
 *
 * @param[in,out] cmdLine  CmdLine.  Not NULL.
 * @param[in]     section  First section to merge, or INVALID_MERGE_CMDLINE.
 * @param[in]     ...      Additional sections, terminated by
 *                         INVALID_MERGE_CMDLINE.
 * @return Pointer to the merged, NULL-terminated argv vector.
 */
char **MergeCmdLine( CmdLine *cmdLine, int section, ... )
/*******************************************************/
{
    int                 countSection;
    int                 countArg;
    int                 total = 0;
    va_list             args;
    int                 done = 0;

    /*** Count the total number of items ***/
    if( section == INVALID_MERGE_CMDLINE ) {
        for( countSection=0; countSection<cmdLine->numSections; countSection++ ) {
            for( countArg=0; countArg<cmdLine[countSection].curItems; countArg++ ) {
                total++;
            }
        }
    } else {
        for( countArg=0; countArg<cmdLine[section].curItems; countArg++ ) {
            total++;
        }
        va_start( args, section );
        while( !done ) {
            countSection = va_arg( args, int );
            if( countSection != INVALID_MERGE_CMDLINE ) {
                for( countArg=0; countArg<cmdLine[countSection].curItems; countArg++ ) {
                    total++;
                }
            } else {
                done = 1;
            }
        }
        va_end( args );
    }

    /*** Allocate enough room for all the needed pointers ***/
    if( cmdLine->merged != NULL )  FreeMem( cmdLine->merged ); /* free old */
    cmdLine->merged = AllocMem( (total+1) * sizeof(char*) );

    /*** Add the items to the cmdLine->merged array ***/
    total = 0;
    done = 0;
    if( section == INVALID_MERGE_CMDLINE ) {
        for( countSection=0; countSection<cmdLine->numSections; countSection++ ) {
            for( countArg=0; countArg<cmdLine[countSection].curItems; countArg++ ) {
                cmdLine->merged[total++] = cmdLine[countSection].args[countArg];
            }
        }
    } else {
        for( countArg=0; countArg<cmdLine[section].curItems; countArg++ ) {
            cmdLine->merged[total++] = cmdLine[section].args[countArg];
        }
        va_start( args, section );
        while( !done ) {
            countSection = va_arg( args, int );
            if( countSection != INVALID_MERGE_CMDLINE ) {
                for( countArg=0; countArg<cmdLine[countSection].curItems; countArg++ ) {
                    cmdLine->merged[total++] = cmdLine[countSection].args[countArg];
                }
            } else {
                done = 1;
            }
        }
        va_end( args );
    }
    cmdLine->merged[total] = NULL;              /* terminate the list */

    return( cmdLine->merged );
}
