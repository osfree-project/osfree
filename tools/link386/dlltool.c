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
 * @file dlltool.c
 * @brief Loader for IDE-style tool DLLs (e.g. WLIB) used by the link clone.
 */

#include <windows.h>
#include "dlltool.h"
#include "error.h"
#include "memory.h"

/*!
 * @def WLIB_DLL_FILENAME
 * @brief File name of the WLIB DLL.
 */
#define WLIB_DLL_FILENAME       "wlibd.dll"
/*!
 * @def DLL_GETVER
 * @brief Exported name of the version query function.
 */
#define DLL_GETVER              "_IDEGetVersion@0"
/*!
 * @def DLL_INITDLL
 * @brief Exported name of the initialization function.
 */
#define DLL_INITDLL             "_IDEInitDLL@12"
/*!
 * @def DLL_RUNSELF
 * @brief Exported name of the main entry point.
 */
#define DLL_RUNSELF             "_IDERunYourSelf@12"
/*!
 * @def DLL_FINIDLL
 * @brief Exported name of the shutdown function.
 */
#define DLL_FINIDLL             "_IDEFiniDLL@4"
/*!
 * @def DLL_STOPRUN
 * @brief Exported name of the stop function.
 */
#define DLL_STOPRUN             "_IDEStopRunning@0"
/*!
 * @def DLL_INITINFO
 * @brief Exported name of the init-info function.
 */
#define DLL_INITINFO            "_IDEPassInitInfo@8"


/*
 * Types.
 */

/*!
 * @typedef WBool
 * @brief Boolean type used across the DLL boundary.
 */
typedef char                    WBool;          /* really a C++ 'bool' */
/*!
 * @typedef GetVerFn
 * @brief Version query entry point.
 */
typedef unsigned IDEDLL_EXPORT  (*GetVerFn)( void );
/*!
 * @typedef InitDllFn
 * @brief Initialization entry point.
 */
typedef WBool IDEDLL_EXPORT     (*InitDllFn)( IDECBHdl hdl, IDECallBacks *cb,
                                              IDEDllHdl *info );
/*!
 * @typedef InitInfoFn
 * @brief Init-info entry point.
 */
typedef WBool IDEDLL_EXPORT     (*InitInfoFn)( IDEDllHdl hdl,
                                               IDEInitInfo *info );
/*!
 * @typedef RunSelfFn
 * @brief Main-entry-point signature.
 */
typedef WBool IDEDLL_EXPORT     (*RunSelfFn)( IDEDllHdl hdl, const char *opts,
                                              WBool *fatalerr );
/*!
 * @typedef FiniDllFn
 * @brief Shutdown entry point.
 */
typedef void IDEDLL_EXPORT      (*FiniDllFn)( IDEDllHdl hdl );
/*!
 * @typedef StopRunFn
 * @brief Stop entry point.
 */
typedef void IDEDLL_EXPORT      (*StopRunFn)( void );

/*!
 * @struct DllTool
 * @brief Loaded tool DLL state.
 */
typedef struct {
    HINSTANCE           dllhandle;      /*!< Loaded DLL instance. */
    GetVerFn            getversion;     /*!< Version query. */
    InitDllFn           initdll;        /*!< Initialization. */
    InitInfoFn          initinfo;       /*!< Init info (may be NULL). */
    RunSelfFn           runyourself;    /*!< Main entry point. */
    FiniDllFn           finidll;        /*!< Shutdown. */
    StopRunFn           stoprunning;    /*!< Stop. */
    IDECallBacks1       callbacks_ver1; /*!< Version-1 callback table. */
    IDECallBacks        callbacks;      /*!< Current callback table. */
    IDEDllHdl           dllHdl;         /*!< DLL-owned handle. */
    IDEInitInfo         initdata;       /*!< Init info struct passed to the DLL. */
} DllTool;


/*
 * If a callback function isn't given to InitDllTool, the appropriate one
 * of these do-nothing functions will be used.
 */

/*!
 * @brief No-op plain-message callback.
 * @param[in] hdl   Caller handle (unused).
 * @param[in] text  Message (unused).
 * @retval 1 Always returns 1.
 * @return 1.
 */
static IDEBool __stdcall print_message( IDECBHdl hdl, const char *text )
{
    return( 1 );
}

/*!
 * @brief No-op CRLF-message callback.
 * @param[in] hdl   Caller handle (unused).
 * @param[in] text  Message (unused).
 * @retval 1 Always returns 1.
 * @return 1.
 */
static IDEBool __stdcall print_message_crlf( IDECBHdl hdl, const char *text )
{
    return( 1 );
}

/*!
 * @brief No-op structured-message callback (v2).
 * @param[in] hdl   Caller handle (unused).
 * @param[in] info  Structured message (unused).
 * @retval 1 Always returns 1.
 * @return 1.
 */
static IDEBool __stdcall print_with_info2( IDECBHdl hdl, IDEMsgInfo2 *info )
{
    return( 1 );
}

/*!
 * @brief No-op structured-message callback (v3+).
 * @param[in] hdl   Caller handle (unused).
 * @param[in] info  Structured message (unused).
 * @retval 1 Always returns 1.
 * @return 1.
 */
static IDEBool __stdcall print_with_info( IDECBHdl hdl, IDEMsgInfo *info )
{
    return( 1 );
}


/*!
 * @brief Load and initialize an IDE tool DLL.
 * @param[in] whichtool  One of the DLLTOOL_* constants.
 * @param[in] callbacks  Callback table. Not NULL.
 * @retval NULL Load or initialization failed.
 * @return Opaque tool handle.
 */
void *InitDllTool( int whichtool, const DllToolCallbacks *callbacks )
/*******************************************************************/
{
    DllTool *           tool = AllocMem( sizeof( DllTool ) );
    unsigned            dllversion;
    IDECallBacks *      dllcallbacks;

    /*** Load the DLL ***/
    switch( whichtool ) {
      case DLLTOOL_WLIB:
        tool->dllhandle = LoadLibrary( WLIB_DLL_FILENAME );
        break;
      default:
        Zoinks();
    };
    if( tool->dllhandle == NULL ) {
        return( NULL );
    }

    /*** Grab the entry points ***/
    tool->getversion = (GetVerFn)GetProcAddress( tool->dllhandle, DLL_GETVER );
    tool->initdll = (InitDllFn)GetProcAddress( tool->dllhandle, DLL_INITDLL );
    tool->initinfo = (InitInfoFn)GetProcAddress( tool->dllhandle, DLL_INITINFO );
    tool->runyourself = (RunSelfFn)GetProcAddress( tool->dllhandle, DLL_RUNSELF );
    tool->finidll = (FiniDllFn)GetProcAddress( tool->dllhandle, DLL_FINIDLL );
    tool->stoprunning = (StopRunFn)GetProcAddress( tool->dllhandle, DLL_STOPRUN );
    if( tool->getversion == NULL  ||  tool->initdll == NULL  ||
        tool->runyourself == NULL  || tool->finidll == NULL  ||
        tool->stoprunning == NULL ) {
        FreeLibrary( tool->dllhandle );
        return( NULL );
    }

    /*** Set up the callbacks ***/
    dllversion = tool->getversion();
    if( dllversion == 1 ) {
        memset( &tool->callbacks_ver1, 0, sizeof( IDECallBacks1 ) );
        if( callbacks->printmessage != NULL ) {
            tool->callbacks_ver1.PrintMessage = callbacks->printmessage;
        } else {
            tool->callbacks_ver1.PrintMessage = print_message;
        }
        if( callbacks->printmessageCRLF != NULL ) {
            tool->callbacks_ver1.PrintWithCRLF = callbacks->printmessageCRLF;
        } else {
            tool->callbacks_ver1.PrintWithCRLF = print_message_crlf;
        }
        tool->callbacks_ver1.GetInfo = NULL;
        tool->callbacks_ver1.RunBatch = NULL;
        dllcallbacks = (IDECallBacks*) &tool->callbacks_ver1;
    } else if( dllversion > 1 ) {
        memset( &tool->callbacks, 0, sizeof( IDECallBacks ) );
        if( callbacks->printmessage != NULL ) {
            tool->callbacks.PrintMessage = callbacks->printmessage;
        } else {
            tool->callbacks.PrintMessage = print_message;
        }
        if( callbacks->printmessageCRLF != NULL ) {
            tool->callbacks.PrintWithCRLF = callbacks->printmessageCRLF;
        } else {
            tool->callbacks.PrintWithCRLF = print_message_crlf;
        }
        if( dllversion == 2 ) {
            if( callbacks->printwithinfo != NULL ) {
                tool->callbacks.PrintWithInfo = (IDEMsgInfoFn)callbacks->printwithinfo2;
            } else {
                tool->callbacks.PrintWithInfo = (IDEMsgInfoFn)print_with_info2;
            }
        } else {                        /* dllversion >= 3 */
            if( callbacks->printwithinfo != NULL ) {
                tool->callbacks.PrintWithInfo = callbacks->printwithinfo;
            } else {
                tool->callbacks.PrintWithInfo = print_with_info;
            }
        }
        tool->callbacks.GetInfo = NULL;
        tool->callbacks.RunBatch = NULL;
        tool->callbacks.ProgressMessage = NULL;
        dllcallbacks = &tool->callbacks;
    } else {
        FreeLibrary( tool->dllhandle );
        return( NULL );
    }

    /*** Tell the DLL to initialize itself ***/
    if( tool->initdll( (IDECBHdl)callbacks->cookie, dllcallbacks,
                       &tool->dllHdl ) ) {
        FreeLibrary( tool->dllhandle );
        return( NULL );
    }
    if( tool->initinfo != NULL ) {
        memset( &tool->initdata, 0, sizeof( tool->initdata ) );
        tool->initdata.ver = IDE_CUR_INFO_VER;
        tool->initdata.ignore_env = 1;
        tool->initdata.cmd_line_has_files = 1;
        tool->initinfo( tool->dllHdl, &tool->initdata );
    }

    return( (void*)tool );
}


/*!
 * @brief Unload a tool DLL previously opened with InitDllTool().
 * @param[in] _tool  Opaque tool handle. Not NULL.
 */
void FiniDllTool( void *_tool )
/*****************************/
{
    DllTool *           tool = (DllTool*)_tool;

    tool->finidll( tool->dllHdl );
    FreeLibrary( tool->dllhandle );
}


/*!
 * @brief Run the tool DLL with a command line.
 * @param[in] _tool    Opaque tool handle. Not NULL.
 * @param[in] cmdline  Command line. Not NULL.
 * @retval 0  Tool reported failure.
 * @retval 1  Tool succeeded.
 * @return 1 on success, 0 on failure.
 */
int RunDllTool( void *_tool, const char *cmdline )
/************************************************/
{
    DllTool *           tool = (DllTool*)_tool;
    WBool               fatal = 0;
    WBool               rc;

    rc = tool->runyourself( tool->dllHdl, cmdline, &fatal );
    if( rc || fatal ) {
        return( 0 );
    } else {
        return( 1 );
    }
}
