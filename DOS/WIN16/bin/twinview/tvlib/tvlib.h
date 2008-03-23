/***************************************************************************************
**
**
**                               Shared DLL Header File
**
**
***************************************************************************************/


#ifndef TVLIB_H
#define TVLIB_H


/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>
#include <stdlib.h>




/* Name of library to load for .TVL library functions. */
#define TVLIB_NAME  "TVLIB.DLL"

/* Wildcard to find TV library files. */
#define TVLIB_WILD "*.tvl"

/* Maximum value of an aliased menu item. */
#define MAX_ALIAS 500

/* 
** Window extra-bytes that should be allocated by a library
** for a child window class.
*/
#define TVL_WNDEXTRABYTES ( sizeof(DWORD) + (3 * sizeof(WORD)) )


/*************************************
*
*  Structure passed to a TwinView library's 
*  TVLIBOPENWINDOWPROC() function.
*
**************************************/
#define MAX_ARG_LENGTH  _MAX_PATH
typedef struct tagTVOPENWNDSTRUCT
{
  HWND hOwner;                                  /* Parent/Owner of window. */
  char szFilePath[_MAX_PATH];                   /* Filename to open. */
  char szArguments[MAX_ARG_LENGTH];             /* Arguments in addition to filename. */
  LPARAM lParam;                                /* Unique to viewer data. */
}
TVOPENWNDSTRUCT;
typedef TVOPENWNDSTRUCT __far * FPTVOPENWNDSTRUCT;



/*************************************
*
*  Structure passed to a TwinView library's 
*  TVLIBGETINFOPROC() function.
*
**************************************/
#define MAX_FILTER   256
#define MAX_ICONNAME 51
#define MAX_ICONCAPTION 101

/* Structure passed to a Control's WndProc() by the LibInfo() LPARAM parameter. */
typedef struct tagTVLIBFILEINFO
{
  char szFilter[ MAX_FILTER ];
  char szIconResouce[ MAX_ICONNAME ];
  char szIconCaption[ MAX_ICONCAPTION ];
  BOOL bAskForFile;
}
TVLIBFILEINFO;
typedef TVLIBFILEINFO __far * FPTVLIBFILEINFO;
typedef GLOBALHANDLE HTVLIBFILEINFO;


/*************************************
*
*  Structure containing information
*  for a window opened by a TV library.
*
**************************************/
typedef GLOBALHANDLE HTVWNDINFO;
typedef struct tagTVWNDINFO
{
  HTVWNDINFO Next;
  HWND hWnd;
}
TVWNDINFO;
typedef TVWNDINFO __far * FPTVWNDINFO;


/*************************************
*
*  Structure containing information 
*  for a single library found in the 
*  search path.
*
*  Passed to a callback function
*  TVLIBINFOENUMPROC specified in 
*  EnumTVLibInfo().
*
**************************************/
typedef struct tagTVLIBINFO
{
  char szLibFilePath[_MAX_PATH];
  HINSTANCE hInstance;
  HTVWNDINFO hTVWndInfo;
  HTVLIBFILEINFO hTVLibFileInfo;
}
TVLIBINFO;
typedef TVLIBINFO __far * FPTVLIBINFO;
typedef GLOBALHANDLE HTVLIBINFO;




/*************************************
*
*  Defines for callback functions 
*  for functions exported by this library.
*
**************************************/
/* EnumFiles() callback function. */
typedef BOOL ( CALLBACK * FILESENUMPROC )( char __far * szFilePath, LPARAM lParam );

/* EnumTVLibInfo() callback function. */
typedef BOOL ( CALLBACK * TVLIBINFOENUMPROC )( FPTVLIBINFO fpTVLibInfo, LPARAM lParam );


/* 
** Yield or not when calling the
** the following functions:
**
** CreateTVLibList()
*/
#define TV_YIELD    TRUE
#define TV_NOYIELD  FALSE


/* 
** Yield or not when calling the
** the following functions:
**
** OpenTVFile()
*/
#define TV_ALLOWOPENFILE TRUE
#define TV_NOOPENFILE    FALSE


/*************************************
*
*  Prototypes for functions 
*  exported and shared by this library.
*
**************************************/
long __far __pascal __export TVLibMsgProc( HWND hWnd, WORD wMsg, WORD wParam, LONG lParam );
void __far __pascal __export SetTVMsgValue( UINT TVMsgValue );
UINT __far __pascal __export GetTVMsgValue( void );

void __far __pascal __export SetTVMenuHandle( HMENU hMenu );
HMENU __far __pascal __export GetTVMenuHandle( void );

void __far __pascal __export SetTVWindowHandle( HWND hWnd );
HWND __far __pascal __export GetTVWindowHandle( void );

BOOL __far __pascal __export CanTwinViewClose( void );

void __far __pascal __export RegisterTVAccel( HWND hWnd, HACCEL hAccel );
HACCEL __far __pascal __export GetTVAccel( HWND hWnd );

BOOL __far __pascal __export OpenTVFile( HWND hWnd, FPTVOPENWNDSTRUCT fpTVOpenWndStruct, BOOL bShowOpenFile );

void __far __pascal __export TVChildWindowIsClosing( HWND hWnd );
void __far __pascal __export ShowTVWindows( int nCmdShow );
BOOL __far __pascal __export IsTVWindow( HWND hWnd );

BOOL __far __pascal __export EnumTVLibInfo( TVLIBINFOENUMPROC EnumTVLibInfoProc, LPARAM lParam );
BOOL __far __pascal __export EnumFiles( char __far * szWildName, FILESENUMPROC EnumFilesProc, LPARAM lParam, BOOL Yield );

WORD __far __pascal __export RegisterTVMenuItem( HWND hWnd, WORD wParam );
void __far __pascal __export UnregisterTVMenuItem( WORD wParamAlias );
BOOL __far __pascal __export ProcessTVMenuItem( WORD wParamAlias );

void __far __pascal __export ProcessTVCmdLine( HWND hOwnerWnd, LPSTR szCommandLine );
BOOL __far __pascal __export ProcessLoadLine( HWND hOwnerWnd );


/***** TEMPORARILY *****/
BOOL __far __pascal __export CreateTVLibList( char __far * szLibWildName, BOOL Yield );
void __far __pascal __export DestroyTVLibList( void );
void __far __pascal __export DeleteMenuInfoList( void );
/***** TEMPORARILY *****/


/*************************************
*
*  Prototypes and ordinal defines for 
*  the common functions exported
*  by all TwinView Libraries.
*
**************************************/

/* A TwinView Libary's TVLibGetInfo() function - Exported using @2. */
typedef BOOL ( CALLBACK * TVLIBGETINFOPROC )( FPTVLIBFILEINFO fpTVLibFileInfo );

/* A TwinView Libary's TVLibOpenWindow() function - Exported using @3. */
typedef HWND ( CALLBACK * TVLIBOPENWINDOWPROC )( FPTVOPENWNDSTRUCT fpTVOpenWndStruct );

/* A TwinView Libary's TVCloseWindow() function - Exported using @4. */
typedef BOOL ( CALLBACK * TVLIBCLOSEWINDOWPROC )( HWND hWnd );


#endif  /* #ifndef TVLIB_H. */

