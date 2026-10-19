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
* Description:  Interface for DLLs pluggable into the Watcom IDE (and wmake).
*
****************************************************************************/

/*!
 * @file idedll.h
 * @brief Interface for DLLs pluggable into the Watcom IDE (and wmake).
 */

#ifndef IDEDLL_H_INCLUDED
#define IDEDLL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <sys/stat.h>

#pragma pack( 4 )

#if defined( __WATCOMC__ )
/*!
 * @def IDECALL
 * @brief Calling convention used by the IDE DLL interface (__stdcall).
 */
#define IDECALL                 __stdcall
#else
/*!
 * @def IDECALL
 * @brief Calling convention used by the IDE DLL interface (default).
 */
#define IDECALL
#endif

#ifdef IDE_PGM
/*!
 * @def IDEDLL_EXPORT
 * @brief Export marker for DLL side (no decoration when building the IDE).
 */
#define IDEDLL_EXPORT
#else
/*!
 * @def IDEDLL_EXPORT
 * @brief Export marker for DLL side (__export __stdcall).
 */
#define IDEDLL_EXPORT           __export IDECALL
#endif

/*!
 * @def IDE_CUR_DLL_VER
 * @brief Current IDE DLL interface version.
 */
#define IDE_CUR_DLL_VER         3
/*!
 * @def IDE_CUR_INFO_VER2
 * @brief IDEInitInfo version 2.
 */
#define IDE_CUR_INFO_VER2       2       //VERSION 2 (see below)
/*!
 * @def IDE_CUR_INFO_VER3
 * @brief IDEInitInfo version 3.
 */
#define IDE_CUR_INFO_VER3       3       //VERSION 3 (see below)
/*!
 * @def IDE_CUR_INFO_VER4
 * @brief IDEInitInfo version 4.
 */
#define IDE_CUR_INFO_VER4       4       //VERSION 4 (see below)
/*!
 * @def IDE_CUR_INFO_VER5
 * @brief IDEInitInfo version 5.
 */
#define IDE_CUR_INFO_VER5       5       //VERSION 5 (see below)
/*!
 * @def IDE_CUR_INFO_VER
 * @brief Current IDEInitInfo version.
 */
#define IDE_CUR_INFO_VER        6

/*!
 * @typedef IDEBool
 * @brief Boolean type used across the IDE DLL interface.
 */
typedef unsigned short  IDEBool;
/*!
 * @typedef IDEDllHdl
 * @brief Handle owned by the DLL side.
 */
typedef void            *IDEDllHdl;
/*!
 * @typedef IDECBHdl
 * @brief Handle owned by the caller (IDE/wmake) side.
 */
typedef void            *IDECBHdl;

struct IDEMsgInfo;
/*!
 * @typedef BatchFilter
 * @brief Plain-text batch output filter callback.
 * @param[in] cookie  Caller-supplied context.
 * @param[in] msg     Output line. Not NULL.
 */
typedef void (*BatchFilter)( void *cookie, const char *msg );
/*!
 * @typedef BatchDllFilter
 * @brief Structured batch output filter callback.
 * @param[in] cookie   Caller-supplied context.
 * @param[in] errinfo  Structured output. Not NULL.
 */
typedef void (*BatchDllFilter)( void *cookie, struct IDEMsgInfo *errinfo );

/*******************************************
 *
 * Error Information
 *
 *******************************************/

/*!
 * @def __IdeMsgSeverity
 * @brief X-macro list of message severity codes.
 */
#define __IdeMsgSeverity     \
  _IdeMsgSeverity( WARNING ) \
, _IdeMsgSeverity( ERROR   ) \
, _IdeMsgSeverity( NOTE    ) \
, _IdeMsgSeverity( BANNER  ) \
, _IdeMsgSeverity( DEBUG   ) \
, _IdeMsgSeverity( NOTE_MSG)

/*!
 * @typedef IDEMsgSeverity
 * @brief Message severity code.
 */
typedef unsigned IDEMsgSeverity;
/*!
 * @enum IDEMSGSEV_*
 * @brief Message severity codes.
 *
 * The enumerators are expanded from __IdeMsgSeverity.
 */
enum {
#   define _IdeMsgSeverity(a) IDEMSGSEV_ ## a
    __IdeMsgSeverity
#   undef _IdeMsgSeverity
};

/*!
 * @def IdeSeverityDiagnostic
 * @brief Mask of message severities classified as diagnostics.
 */
#define IdeSeverityDiagnostic   ( 0 \
        | ( 1 << IDEMSGSEV_WARNING )    \
        | ( 1 << IDEMSGSEV_ERROR )      \
        | ( 1 << IDEMSGSEV_NOTE )       \
        )

/*!
 * @def IdeSeverityNonDebugText
 * @brief Mask of message severities that carry non-debug text.
 */
#define IdeSeverityNonDebugText ( 0 \
        | ( 1 << IDEMSGSEV_BANNER )     \
        | ( 1 << IDEMSGSEV_NOTE_MSG )   \
        )

/*!
 * @def IdeSeverityIs
 * @brief Test whether a severity belongs to a mask.
 * @param s  Severity value.
 * @param m  Mask built from IdeSeverityDiagnostic/IdeSeverityNonDebugText.
 */
#define IdeSeverityIs( s, m )   ((( 1 << (s) ) & (m) ) != 0 )

/*!
 * @def IDE_INFO_READABLE
 * @brief The message carries "readable" text suitable for the IDE.
 */
#define IDE_INFO_READABLE       0x00000001
/*!
 * @def IDE_INFO_HAS_SRC_FILE
 * @brief The source file name is valid.
 */
#define IDE_INFO_HAS_SRC_FILE   0x00000002
/*!
 * @def IDE_INFO_HAS_SRC_LINE
 * @brief The source line number is valid.
 */
#define IDE_INFO_HAS_SRC_LINE   0x00000004
/*!
 * @def IDE_INFO_HAS_SRC_COL
 * @brief The source column number is valid.
 */
#define IDE_INFO_HAS_SRC_COL    0x00000008
/*!
 * @def IDE_INFO_HAS_HELP
 * @brief The help file and help id are valid.
 */
#define IDE_INFO_HAS_HELP       0x00000010
/*!
 * @def IDE_INFO_HAS_MSG_NUM
 * @brief The message number is valid.
 */
#define IDE_INFO_HAS_MSG_NUM    0x00000020
/*!
 * @def IDE_INFO_HAS_LNK_FILE
 * @brief The link file name is valid.
 */
#define IDE_INFO_HAS_LNK_FILE   0x00000040
/*!
 * @def IDE_INFO_HAS_LNK_SYMBOL
 * @brief The link symbol name is valid.
 */
#define IDE_INFO_HAS_LNK_SYMBOL 0x00000080
/*!
 * @def IDE_INFO_HAS_MSG_GRP
 * @brief The message group is valid.
 */
#define IDE_INFO_HAS_MSG_GRP    0x00000100

/*!
 * @def IDE_IS_READABLE
 * @brief Test the IDE_INFO_READABLE bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_IS_READABLE( x )    ( ((x)->flags) & IDE_INFO_READABLE )
/*!
 * @def IDE_HAS_SRC_FILE
 * @brief Test the IDE_INFO_HAS_SRC_FILE bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_SRC_FILE( x )   ( ((x)->flags) & IDE_INFO_HAS_SRC_FILE )
/*!
 * @def IDE_HAS_SRC_LINE
 * @brief Test the IDE_INFO_HAS_SRC_LINE bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_SRC_LINE( x )   ( ((x)->flags) & IDE_INFO_HAS_SRC_LINE )
/*!
 * @def IDE_HAS_SRC_COL
 * @brief Test the IDE_INFO_HAS_SRC_COL bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_SRC_COL( x )    ( ((x)->flags) & IDE_INFO_HAS_SRC_COL )
/*!
 * @def IDE_HAS_HELP
 * @brief Test the IDE_INFO_HAS_HELP bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_HELP( x )       ( ((x)->flags) & IDE_INFO_HAS_HELP )
/*!
 * @def IDE_HAS_MSG_NUM
 * @brief Test the IDE_INFO_HAS_MSG_NUM bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_MSG_NUM( x )    ( ((x)->flags) & IDE_INFO_HAS_MSG_NUM )
/*!
 * @def IDE_HAS_LNK_FILE
 * @brief Test the IDE_INFO_HAS_LNK_FILE bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_LNK_FILE( x )   ( ((x)->flags) & IDE_INFO_HAS_LNK_FILE )
/*!
 * @def IDE_HAS_LNK_SYMBOL
 * @brief Test the IDE_INFO_HAS_LNK_SYMBOL bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_LNK_SYMBOL( x ) ( ((x)->flags) & IDE_INFO_HAS_LNK_SYMBOL )
/*!
 * @def IDE_HAS_MSG_GRP
 * @brief Test the IDE_INFO_HAS_MSG_GRP bit.
 * @param x  IDEMsgInfo pointer. Not NULL.
 */
#define IDE_HAS_MSG_GRP( x )    ( ((x)->flags) & IDE_INFO_HAS_MSG_GRP )

/*!
 * @struct IDEMsgInfo
 * @brief IDE message information record.
 */
typedef struct IDEMsgInfo {             // IDE MESSAGE INFORMATION
    IDEMsgSeverity       severity;      /*!< Severity code. */
                                        // Message Information Present
    unsigned long        flags;         /*!< IDE_INFO_* flags. */
                                        // Help Iinformation
    char const           *helpfile;     /*!< Name of help file. */
    unsigned long        helpid;        /*!< Help identifier. */
                                        // Message Information
    const char           *msg;          /*!< Message text. */
    char const           *src_symbol;   /*!< Symbol name. */
    char const           *src_file;     /*!< Source or link file name. */
    unsigned long        src_line;      /*!< Source file line. */
    unsigned long        src_col;       /*!< Source file column. */
    unsigned long        msg_no;        /*!< Message number. */
    char                 msg_group[8];  /*!< Message group. */
} IDEMsgInfo;

/*!
 * @def __IdeMsgClass
 * @brief X-macro list of message info variants (IDEMsgInfo2).
 */
#define __IdeMsgClass          \
  _IdeMsgClass( LINE_COL     ) \
, _IdeMsgClass( LINE         ) \
, _IdeMsgClass( FILE         ) \
, _IdeMsgClass( GENERIC      ) \
, _IdeMsgClass( SYMBOL       ) \
, _IdeMsgClass( SYMBOL_FILE  )

/*!
 * @typedef IDEMsgClass
 * @brief Variant selector for IDEMsgInfo2.
 */
typedef unsigned IDEMsgClass;
/*!
 * @enum IDEMSGINFO_*
 * @brief Variants of IDEMsgInfo2.
 */
enum {
#   define _IdeMsgClass(a) IDEMSGINFO_ ## a
    __IdeMsgClass
#   undef _IdeMsgClass
};

/*******************************************
 * Output Information structure used by
 * version 2 DLL's
 *******************************************/

/*!
 * @struct LineColErr
 * @brief Error location with line and column.
 */
typedef struct {
    unsigned    line;           /*!< Line where error occurred. */
    unsigned    column;         /*!< Column where error occurred. */
    const char  *file;          /*!< Source file where error occurred. */
} LineColErr;

/*!
 * @struct LineErr
 * @brief Error location with line only.
 */
typedef struct {
    unsigned    line;           /*!< Line where error occurred. */
    const char  *file;          /*!< Source file where error occurred. */
} LineErr;

/*!
 * @struct FileErr
 * @brief Error location with file only.
 */
typedef struct {
    const char  *file;          /*!< Source file where error occurred. */
} FileErr;

/*!
 * @struct SymbolFileErr
 * @brief Link error with a symbol and a referencing file.
 */
// these structures are for use by the linker
typedef struct {
    const char  *symbol;        /*!< Symbol that caused the error. */
    const char  *file;          /*!< File where the symbol was referenced. */
} SymbolFileErr;

/*!
 * @struct SymbolErr
 * @brief Link error with a symbol only.
 */
typedef struct {
    const char  *symbol;        /*!< Symbol that caused the error. */
} SymbolErr;


/*!
 * @struct IDEMsgInfo2
 * @brief Version-2 structured message information.
 */
typedef struct IDEMsgInfo2 {
    IDEMsgSeverity      severity;   /*!< Severity code. */
    const char          *msg;       /*!< Message text. */

    const char          *helpfile;  /*!< Name of help file. */
    unsigned long       helpid;     /*!< Help identifier (used only if helpfile is non-NULL). */

    IDEMsgClass         type;       /*!< Variant selector. */
    union {                         /*!< Variant payload. */
        LineColErr      line_col;   /*!< Line+column variant. */
        LineErr         line;       /*!< Line-only variant. */
        FileErr         file;       /*!< File-only variant. */
        SymbolFileErr   sym_file;   /*!< Symbol+file variant. */
        SymbolErr       symbol;     /*!< Symbol-only variant. */
    };
} IDEMsgInfo2;


/*!
 * @struct IDEInitInfo
 * @brief Initialization information passed to a DLL.
 */
typedef struct {
    unsigned            ver;                    /*!< Structure version. */
    unsigned char       ignore_env;             /*!< Ignore environment variables. */
    unsigned char       cmd_line_has_files;     /*!< VERSION 2: command line carries files. */
    unsigned char       console_output;         /*!< VERSION 3: DLL writes to console. */
    unsigned char       progress_messages;      /*!< VERSION 4: DLL emits progress messages. */
    unsigned char       progress_index;         /*!< VERSION 5: progress index base. */
} IDEInitInfo;

/*!
 * @typedef IDEDataInfo
 * @brief Opaque data handle used by the file I/O callbacks.
 */
typedef void IDEDataInfo;

/*!
 * @enum IDEIOOpts
 * @brief File open mode for the IDE I/O callbacks.
 */
typedef enum {
    IDE_READ                    /*!< Open for reading. */
  , IDE_WRITE                   /*!< Open for writing. */
} IDEIOOpts;

/*!
 * @typedef IDEInfoType
 * @brief Selector for IDEGetInfoFn queries.
 */
typedef unsigned        IDEInfoType;
/*!
 * @def IDE_GET_SOURCE_FILE
 * @brief Query the source file of the current item.
 */
#define IDE_GET_SOURCE_FILE     1
/*!
 * @def IDE_GET_TARGET_FILE
 * @brief Query the target file.
 */
#define IDE_GET_TARGET_FILE     2
/*!
 * @def IDE_GET_OBJ_FILE
 * @brief Query the object file.
 */
#define IDE_GET_OBJ_FILE        3
/*!
 * @def IDE_GET_LIB_FILE
 * @brief Query the library file.
 */
#define IDE_GET_LIB_FILE        4
/*!
 * @def IDE_GET_RES_FILE
 * @brief Query the resource file.
 */
#define IDE_GET_RES_FILE        5
/*!
 * @def IDE_GET_ENV_VAR
 * @brief Query an environment variable.
 */
#define IDE_GET_ENV_VAR         6
/*!
 * @def IDE_GET_AI
 * @brief Query architecture-specific information.
 */
#define IDE_GET_AI              7

/*!
 * @def IDE_DEP_FILETYPE_SOURCE
 * @brief Dependency file type: source file.
 */
// character values passed in the code parameter of IDEJavaSrcDepFile
// callback function
#define IDE_DEP_FILETYPE_SOURCE         'S'
/*!
 * @def IDE_DEP_FILETYPE_TARGET
 * @brief Dependency file type: target file.
 */
#define IDE_DEP_FILETYPE_TARGET         'T'
/*!
 * @def IDE_DEP_FILETYPE_DEP_CLS
 * @brief Dependency file type: class file.
 */
#define IDE_DEP_FILETYPE_DEP_CLS        'C'
/*!
 * @def IDE_DEP_FILETYPE_DEP_ZIP
 * @brief Dependency file type: zip archive.
 */
#define IDE_DEP_FILETYPE_DEP_ZIP        'Z'

/*!
 * @typedef IDEGetInfoWParam
 * @brief WPARAM type for IDEGetInfoFn.
 */
typedef unsigned long IDEGetInfoWParam;
/*!
 * @typedef IDEGetInfoLParam
 * @brief LPARAM type for IDEGetInfoFn.
 */
typedef unsigned long IDEGetInfoLParam;

/*!
 * @typedef IDERunBatchFn
 * @brief Run a batch command via the IDE.
 */
typedef IDEBool IDECALL (*IDERunBatchFn)( IDECBHdl hdl, const char *cmdline,
                                BatchFilter cb, void *cookie );
/*!
 * @typedef IDEPrintMsgFn
 * @brief Print a plain message via the IDE.
 */
typedef IDEBool IDECALL (*IDEPrintMsgFn)( IDECBHdl hdl, const char *message );
/*!
 * @typedef IDEGetInfoFn
 * @brief Query the IDE for environment or file information.
 */
typedef IDEBool IDECALL (*IDEGetInfoFn)( IDECBHdl hdl, IDEInfoType type,
                                IDEGetInfoWParam wparam, IDEGetInfoLParam lparam );
/*!
 * @typedef IDEMsgInfoFn
 * @brief Print a structured message via the IDE.
 */
typedef IDEBool IDECALL (*IDEMsgInfoFn)( IDECBHdl hdl, IDEMsgInfo *info );
/*!
 * @typedef IDERunDllFn
 * @brief Run another IDE DLL through the IDE.
 */
typedef IDEBool IDECALL (*IDERunDllFn)( IDECBHdl hdl, const char *dllname,
                                const char *cmdline, BatchDllFilter cb,
                                void *cookie );
/*!
 * @typedef IDERunBatchCwdFn
 * @brief Run a batch command in a specified working directory.
 */
typedef IDEBool IDECALL (*IDERunBatchCwdFn)( IDECBHdl hdl,
                                const char *cmdline, const char *cwd,
                                BatchFilter cb, void *cookie );
/*!
 * @typedef IDEOpenJavaSource
 * @brief Open a Java source through the IDE.
 */
typedef IDEBool IDECALL (*IDEOpenJavaSource)( IDECBHdl hdl, const char *name, IDEIOOpts opt, IDEDataInfo **info );
/*!
 * @typedef IDEOpenClassFile
 * @brief Open a Java class file through the IDE.
 */
typedef IDEBool IDECALL (*IDEOpenClassFile)( IDECBHdl hdl, const char *name, IDEIOOpts opt, IDEDataInfo **info );
/*!
 * @typedef IDEPackageExists
 * @brief Check whether a Java package exists.
 */
typedef IDEBool IDECALL (*IDEPackageExists)( IDECBHdl hdl, const char *name );
/*!
 * @typedef IDEGetSize
 * @brief Get the size of an IDE data handle.
 */
typedef int     IDECALL (*IDEGetSize)( IDECBHdl hdl, IDEDataInfo *info );
/*!
 * @typedef IDEGetTimeStamp
 * @brief Get the timestamp of an IDE data handle.
 */
typedef time_t  IDECALL (*IDEGetTimeStamp)( IDECBHdl hdl, IDEDataInfo *info );
/*!
 * @typedef IDEIsReadOnly
 * @brief Check whether an IDE data handle is read-only.
 */
typedef IDEBool IDECALL (*IDEIsReadOnly)( IDECBHdl hdl, IDEDataInfo *info );
/*!
 * @typedef IDEReadData
 * @brief Read data from an IDE data handle.
 */
typedef int     IDECALL (*IDEReadData)( IDECBHdl hdl, IDEDataInfo *info, char *buffer, int max_len );
/*!
 * @typedef IDEClose
 * @brief Close an IDE data handle.
 */
typedef IDEBool IDECALL (*IDEClose)( IDECBHdl hdl, IDEDataInfo *info );
/*!
 * @typedef IDEReceiveOutput
 * @brief Receive output written to an IDE data handle.
 */
typedef size_t  IDECALL (*IDEReceiveOutput)( IDECBHdl hdl, IDEDataInfo *info, void const *buffer, size_t len );
/*!
 * @typedef IDEReceiveIndex
 * @brief Receive a progress index.
 */
typedef void    IDECALL (*IDEReceiveIndex)( IDECBHdl hdl, unsigned index /* 0-99 */ );

/*!
 * @typedef IDEJavaSrcDepBegin
 * @brief Begin a Java source-dependency run.
 */
typedef IDEBool IDECALL (*IDEJavaSrcDepBegin)( IDECBHdl hdl );
/*!
 * @typedef IDEJavaSrcDepFile
 * @brief Report one Java source-dependency edge.
 */
//the values passed in the code parameter of the IDEJavaSrcDepFile are
//defined as the macros IDE_DEP_FILETYPE_...
typedef IDEBool IDECALL (*IDEJavaSrcDepFile)( IDECBHdl hdl, char code, char const *filename );
/*!
 * @typedef IDEJavaSrcDepEnd
 * @brief End a Java source-dependency run.
 */
typedef IDEBool IDECALL (*IDEJavaSrcDepEnd)( IDECBHdl hdl );

/*!
 * @struct IDECallBacks1
 * @brief Version-1 IDE callback table.
 */
// structure used by version 1 DLL's
typedef struct {
    // building functions
    IDERunBatchFn               RunBatch;       /*!< Run a batch command. */
    IDEPrintMsgFn               PrintMessage;   /*!< Print a message. */
    IDEPrintMsgFn               PrintWithCRLF;  /*!< Print a message with CRLF. */

    // Query functions
    IDEGetInfoFn                GetInfo;        /*!< Query the IDE. */
} IDECallBacks1;

/*!
 * @struct IDECallBacks
 * @brief Current IDE callback table.
 */
// structure used by current version DLL's
typedef struct {
    // building functions
    IDERunBatchFn               RunBatch;       /*!< Run a batch command. */
    IDEPrintMsgFn               PrintMessage;   /*!< Print a message. */
    IDEPrintMsgFn               PrintWithCRLF;  /*!< Print a message with CRLF. */
    IDEMsgInfoFn                PrintWithInfo;  /*!< Print a structured message. */

    // Query functions
    IDEGetInfoFn                GetInfo;        /*!< Query the IDE. */

    IDEPrintMsgFn               ProgressMessage;/*!< Emit a progress message. */
    IDERunDllFn                 RunDll;         /*!< Run another IDE DLL (may be NULL). */
    IDERunBatchCwdFn            RunBatchCWD;    /*!< Run a batch command in a working directory (may be NULL). */
    IDEOpenJavaSource           OpenJavaSource; /*!< Open a Java source (may be NULL). */
    IDEOpenClassFile            OpenClassFile;  /*!< Open a Java class file (may be NULL). */
    IDEPackageExists            PackageExists;  /*!< Check a Java package (may be NULL). */
    IDEGetSize                  GetSize;        /*!< Get the size of a data handle (may be NULL). */
    IDEGetTimeStamp             GetTimeStamp;   /*!< Get the timestamp of a data handle (may be NULL). */
    IDEIsReadOnly               IsReadOnly;     /*!< Test a data handle's read-only status (may be NULL). */
    IDEReadData                 ReadData;       /*!< Read from a data handle (may be NULL). */
    IDEClose                    Close;          /*!< Close a data handle (may be NULL). */
    IDEReceiveOutput            ReceiveOutput;  /*!< Receive output for a data handle (may be NULL). */

    IDEReceiveIndex             ProgressIndex;  /*!< Receive a progress index. */

    IDEJavaSrcDepBegin          SrcDepBegin;    /*!< Begin a Java source-dependency run (may be NULL). */
    IDEJavaSrcDepFile           SrcDepFile;     /*!< Report one Java dependency edge. */
    IDEJavaSrcDepEnd            SrcDepEnd;      /*!< End a Java dependency run. */
} IDECallBacks;

/*********************************************************
 * Routines implemented by the DLL
 *********************************************************/

/*!
 * @brief Return the interface version implemented by the DLL.
 * @return The interface version.
 */
unsigned IDEDLL_EXPORT IDEGetVersion( void );

/*!
 * @brief Initialize the DLL and receive the IDE callback table.
 * @param[in]  hdl   Caller handle.
 * @param[in]  cb    Callback table. Not NULL.
 * @param[out] info  DLL handle. Not NULL.
 * @retval 0 Failure.
 * @retval non-zero Success.
 * @return Non-zero on success.
 */
IDEBool IDEDLL_EXPORT IDEInitDLL( IDECBHdl hdl, IDECallBacks *cb, IDEDllHdl *info );

/*!
 * @brief Pass init info to the DLL (called after IDEInitDLL).
 * @param[in] hdl   DLL handle.
 * @param[in] info  Initialization info. Not NULL.
 * @retval 0 Failure.
 * @retval non-zero Success.
 * @return Non-zero on success.
 */
IDEBool IDEDLL_EXPORT IDEPassInitInfo( IDEDllHdl hdl, IDEInitInfo *info );

/*!
 * @brief Shut the DLL down.
 * @param[in] hdl  DLL handle.
 */
void IDEDLL_EXPORT IDEFiniDLL( IDEDllHdl hdl );

/*!
 * @brief Determine whether a build command would actually do anything.
 * @param[in]  hdl     DLL handle.
 * @param[in]  opts    Command line. Not NULL.
 * @param[out] status  Up-to-date status. Not NULL.
 * @retval 0 Failure.
 * @retval non-zero Success.
 * @return Non-zero on success.
 */
IDEBool IDEDLL_EXPORT IDEDetermineBuildStatus( IDEDllHdl hdl, const char *opts, IDEBool *status );

/*!
 * @brief Run the DLL's main entry point with a command line.
 * @param[in]  hdl       DLL handle.
 * @param[in]  opts      Command line. Not NULL.
 * @param[out] fatalerr  Non-zero when the failure is fatal. Not NULL.
 * @retval 0 Failure.
 * @retval non-zero Success.
 * @return Non-zero on success.
 */
IDEBool IDEDLL_EXPORT IDERunYourSelf( IDEDllHdl hdl, const char *opts, IDEBool *fatalerr );

/*!
 * @brief Run the DLL's main entry point with an argv vector.
 * @param[in]  hdl       DLL handle.
 * @param[in]  argc      Argument count.
 * @param[in]  argv      Argument vector. Not NULL.
 * @param[out] fatalerr  Non-zero when the failure is fatal. Not NULL.
 * @retval 0 Failure.
 * @retval non-zero Success.
 * @return Non-zero on success.
 */
IDEBool IDEDLL_EXPORT IDERunYourSelfArgv( IDEDllHdl hdl, int argc, char **argv, IDEBool *fatalerr );

/*!
 * @brief Ask the DLL to stop running.
 */
void IDEDLL_EXPORT IDEStopRunning( void );

/*!
 * @brief Ask the DLL to free its heap allocations.
 */
void IDEDLL_EXPORT IDEFreeHeap( void );

/**********************************************************
 * Routines the DLL should use to fill in the Output Message
 * information structure
 **********************************************************/

/*!
 * @brief Initialize a message information structure.
 * @param[in,out] info      Message info. Not NULL.
 * @param[in]     severity  Severity code.
 * @param[in]     msg       Message text. Not NULL.
 */
void IdeMsgInit                 // INITIALIZE MSG STRUCTURE
    ( IDEMsgInfo *info          // - message information
    , IDEMsgSeverity severity   // - message severity
    , char const *msg )         // - the message
;

/*!
 * @brief Set the help information of a message.
 * @param[in,out] info  Message info. Not NULL.
 * @param[in]     file  Help file name. Not NULL.
 * @param[in]     id    Help id.
 */
void IdeMsgSetHelp              // SET HELP INFORMATION
    ( IDEMsgInfo *info          // - message information
    , char const *file          // - help file
    , unsigned long id )        // - help id
;

/*!
 * @brief Set the link file name of a message.
 * @param[in,out] info  Message info. Not NULL.
 * @param[in]     file  File name. Not NULL.
 */
void IdeMsgSetLnkFile           // SET LINK FILE
    ( IDEMsgInfo *info          // - message information
    , char const *file )        // - file name
;

/*!
 * @brief Set the link symbol name of a message.
 * @param[in,out] info  Message info. Not NULL.
 * @param[in]     sym   Symbol name. Not NULL.
 */
void IdeMsgSetLnkSymbol         // SET LINK SYMBOL
    ( IDEMsgInfo *info          // - message information
    , char const *sym )         // - symbol
;

/*!
 * @brief Set the message group of a message.
 * @param[in,out] info   Message info. Not NULL.
 * @param[in]     group  Group name. Not NULL.
 */
void IdeMsgSetMsgGroup          // SET MESSAGE GROUP
    ( IDEMsgInfo *info          // - message information
    , char const *group )       // - group name
;

/*!
 * @brief Set the message number of a message.
 * @param[in,out] info    Message info. Not NULL.
 * @param[in]     msg_no  Message number.
 */
void IdeMsgSetMsgNo             // SET MESSAGE NUMBER
    ( IDEMsgInfo *info          // - message information
    , unsigned msg_no )         // - message number
;

/*!
 * @brief Mark a message as "readable".
 * @param[in,out] info  Message info. Not NULL.
 */
void IdeMsgSetReadable          // MARK MSG AS "READABLE"
    ( IDEMsgInfo *info )        // - message information
;

/*!
 * @brief Set the source column of a message.
 * @param[in,out] info  Message info. Not NULL.
 * @param[in]     col   Column number.
 */
void IdeMsgSetSrcColumn         // SET SOURCE COLUMN
    ( IDEMsgInfo *info          // - message information
    , unsigned col )            // - column number
;

/*!
 * @brief Set the source file of a message.
 * @param[in,out] info  Message info. Not NULL.
 * @param[in]     file  Source file name. Not NULL.
 */
void IdeMsgSetSrcFile           // SET SOURCE FILE
    ( IDEMsgInfo *info          // - message information
    , char const *file )        // - file name
;

/*!
 * @brief Set the source line of a message.
 * @param[in,out] info  Message info. Not NULL.
 * @param[in]     line  Line number.
 */
void IdeMsgSetSrcLine           // SET SOURCE LINE
    ( IDEMsgInfo *info          // - message information
    , unsigned line )           // - line number
;


/*****************************************************
 * Routines that can be used by drivers to format
 * messages from DLL's
 *****************************************************/

/*!
 * @brief Format a message and hand it to a displayer.
 * @param[in] handle     Handle of the requesting caller.
 * @param[in] info       Message info. Not NULL.
 * @param[out] buffer    Format buffer. Not NULL.
 * @param[in] bsize      Buffer size in bytes.
 * @param[in] displayer  Display callback. Not NULL.
 */
void IdeMsgFormat               // FORMAT A MESSAGE
    ( IDECBHdl handle           // - handle for requestor
    , IDEMsgInfo const *info    // - message information
    , char *buffer              // - buffer
    , unsigned bsize            // - buffer size
    , IDEMsgInfoFn displayer )  // - display function
;

/*!
 * @brief Start-of-run hook for the message formatter.
 */
void IdeMsgStartDll             // START OF FORMATING FOR A DLL (EACH TIME)
    ( void )
;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif
