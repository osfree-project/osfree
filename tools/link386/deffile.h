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
 * @file deffile.h
 * @brief Microsoft LINK .def file parser declarations.
 */

#ifndef _DEFFILE_H
#define _DEFFILE_H

#include "system.h"

/*!
 * @struct StringList
 * @brief Singly-linked list of strings.
 *
 * Data type for handling options which can take several values.
 * Similar to the OPT_STRING type.
 */
typedef struct StringList {
    struct StringList * next;   /*!< Next list element, or NULL. */
    char *              str;    /*!< The string. */
} StringList;


/*!
 * @struct DefInfo
 * @brief Parsed contents of a .def file.
 */
typedef struct DefInfo {
    char *              name;            /*!< NAME directive (application name). */
    int                 makeDll;         /*!< Non-zero if the DLL flag was set. */
    char *              internalDllName; /*!< LIBRARY directive (DLL internal name). */
    char *              baseAddr;        /*!< BASE directive (preferred load address). */
    char *              stub;            /*!< STUB directive (DOS stub image). */
    StringList *        description;     /*!< DESCRIPTION directive texts. */
    StringList *        exports;         /*!< EXPORTS/EXPORT directive entries. */
    char *              stacksize;       /*!< STACKSIZE directive. */
    char *              heapsize;        /*!< HEAPSIZE directive. */
    char *              version;         /*!< VERSION directive. */
} DefInfo;


/*!
 * @brief Parse a .def file.
 * @param[in] filename  Path to the .def file. Not NULL.
 * @retval NULL Parse error or file could not be opened.
 * @return The parsed DefInfo.
 */
#ifdef __TARGET_AXP__
    extern DefInfo *ParseDefFile( const char *filename, int dofuzzylink );
#else
    extern DefInfo *ParseDefFile( const char *filename );
#endif

/*!
 * @brief Release the memory owned by a DefInfo.
 * @param[in] p  DefInfo to free. Not NULL.
 */
extern void     FreeDefInfo( DefInfo *p );

#endif
