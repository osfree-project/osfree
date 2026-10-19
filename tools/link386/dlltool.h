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
 * @file dlltool.h
 * @brief Loader for IDE-style tool DLLs (e.g. WLIB) used by the link clone.
 */

#ifndef _DLLTOOL_H
#define _DLLTOOL_H

#include <windows.h>
#include "idedll.h"


/*
 * Defines for specifying which tool to use.
 */

/*!
 * @def DLLTOOL_WLIB
 * @brief Selects the WLIB DLL (the only supported tool).
 */
#define DLLTOOL_WLIB    0


/*
 * Define some types.
 */

/*!
 * @typedef IDEMsgInfo2Fn
 * @brief Version-2 structured message callback signature.
 * @param[in] hdl   Caller handle.
 * @param[in] info  Structured message. Not NULL.
 * @retval 0  Message not consumed.
 * @retval non-zero Message consumed.
 * @return Non-zero when the message was consumed.
 */
typedef IDEBool __stdcall (*IDEMsgInfo2Fn)( IDECBHdl hdl, IDEMsgInfo2 *info );

/*!
 * @struct DllToolCallbacks
 * @brief Caller-supplied callback table used by InitDllTool().
 */
typedef struct {
    IDEPrintMsgFn       printmessage;       /*!< Plain message sink. May be NULL. */
    IDEPrintMsgFn       printmessageCRLF;   /*!< Plain message sink with CRLF. May be NULL. */
    IDEMsgInfo2Fn       printwithinfo2;     /*!< Structured message sink (v2). May be NULL. */
    IDEMsgInfoFn        printwithinfo;      /*!< Structured message sink (v3+). May be NULL. */
    void *              cookie;             /*!< Passed to callback routines. */
} DllToolCallbacks;


/*
 * Function prototypes.
 */

/*!
 * @brief Load and initialize an IDE tool DLL.
 * @param[in] whichtool  One of the DLLTOOL_* constants.
 * @param[in] callbacks  Callback table. Not NULL.
 * @retval NULL Load or initialization failed.
 * @return Opaque tool handle.
 */
extern void *   InitDllTool( int whichtool,
                             const DllToolCallbacks *callbacks );

/*!
 * @brief Unload a tool DLL previously opened with InitDllTool().
 * @param[in] tool  Opaque tool handle. Not NULL.
 */
extern void     FiniDllTool( void *tool );

/*!
 * @brief Run the tool DLL with a command line.
 * @param[in] tool     Opaque tool handle. Not NULL.
 * @param[in] cmdline  Command line. Not NULL.
 * @retval 0  Tool reported failure.
 * @retval 1  Tool succeeded.
 * @return 1 on success, 0 on failure.
 */
extern int      RunDllTool( void *tool, const char *cmdline );


#endif
