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
 * @file fuzzy.h
 * @brief Fuzzy linker-symbol matching.
 */

#ifndef _FUZZY_H
#define _FUZZY_H


/*
 * Define some types.
 */

/*!
 * @typedef FuzzyInitCallback
 * @brief Callback invoked by InitFuzzy() when a file cannot be inspected.
 * @param[in] filename  File that could not be inspected. Not NULL.
 * @retval 0  Abort initialization.
 * @retval 1  Continue with the next file.
 * @return 0 to abort, 1 to continue.
 */
typedef int             (*FuzzyInitCallback)( const char *filename );


/*
 * Function prototypes.
 */

/*!
 * @brief Initialize fuzzy matching using the given objects and libraries.
 * @param[in] objs      Linked object files, NULL-terminated. May be NULL.
 * @param[in] libs      Linked libraries, NULL-terminated. May be NULL.
 * @param[in] libpaths  Library search paths, NULL-terminated. May be NULL.
 * @param[in] callback  Callback invoked on extraction failures. Not NULL.
 */
extern void     InitFuzzy( const char *objs[], const char *libs[],
                           const char *libpaths[],
                           FuzzyInitCallback callback );

/*!
 * @brief Deinitialize the fuzzy matching module.
 */
extern void     FiniFuzzy( void );

/*!
 * @brief Fuzzy-match a symbol against the tables built by InitFuzzy().
 * @param[in] symbol  Entry name (optionally single-quoted). Not NULL.
 * @retval NULL No unique match found.
 * @return Newly allocated single-quoted symbol name, or NULL.
 */
extern char *   MatchFuzzy( const char *symbol );


#endif
