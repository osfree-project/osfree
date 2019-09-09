/***************************************************************************************
**
**
**                               Private DLL Header File
**
**
***************************************************************************************/


#ifndef TVLIBPRV_H
#define TVLIBPRV_H


/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>


/*************************************
*
*  Custom Includes
*
**************************************/
#include "tvlib.h"



/*************************************
*
*  Private Defines
*
**************************************/
#define DIR_CHAR_SEPARATOR '\\'
#define NULL_CHAR '\0'
  

/* 
**  Positions in a child window's extrabytes buffer. 
**  TVLIB.DLL uses the first 10 bytes to store data.
*/
#define TVL_GWW_WNDPROC   0    /* DWORD - Window's original WndProc(). */
#define TVL_GWW_HACCEL    4    /* WORD - Handle to app's accelerator. */
#define TVL_GWW_ENTRIES   6    /* WORD - Time's we've entered by not returned from
                                         the .TVL. */
#define TVL_GWW_OTHERDATA 8    /* WORD - not used, reserved for a 
                                  GlobalHandle to memory structure. */


/* WPARAM values sent in private TV_MSG messages. */
#define  MSGID_CLOSELIB  1


/*************************************
*
*  Prototypes and ordinal defines for 
*  the common functions exported
*  by all TwinView Libraries.
*
**************************************/

/* A TwinView Libary's TVLibGetInfo() function - Exported using @2. */
#define TVLIB_GETINFO_ORD     2

/* A TwinView Libary's TVLibOpenWindow() function - Exported using @3. */
#define TVLIB_OPENWINDOW_ORD  3

/* A TwinView Libary's TVCloseWindow() function - Exported using @4. */
#define TVLIB_CLOSEWINDOW_ORD 4



/*************************************
*
*  Private Structures
*
**************************************/

/*************************************
*
*  A single TVL library-defined menu item
*  added to the TWINVIEW.EXE's menu.
*
**************************************/
typedef HGLOBAL HTVMENUINFO;
typedef HGLOBAL HTVMENULIST;
typedef struct tagTVMENUINFO
{
  HTVMENUINFO Next;
  WORD wParamAlias;
  WORD wParam;
  HWND hWnd;
} 
TVMENUINFO;
typedef TVMENUINFO __far * FPTVMENUINFO;




/*************************************
*
*  Structure allocated by the CreateTVLibList() function 
*  The list contains information for each library
*  found in the search path.
*
**************************************/
typedef struct tagTVLIBLIST
{
  int nLibCount;
  HTVLIBINFO hTVLibInfo;
}
TVLIBLIST;
typedef TVLIBLIST __far * FPTVLIBLIST;
typedef HGLOBAL HTVLIBLIST;



/*************************************
*
*  Defines for private callback functions 
*  types defined in this library.
*
**************************************/

/* EnumTVLibFiles() callback function. */
typedef BOOL ( CALLBACK * TVLIBFILESENUMPROC )( char __far * szLibFilePath, FPTVLIBFILEINFO fpTVLibFileInfo, LPARAM lParam );

/* EnumFiles() callback function. */
typedef BOOL ( CALLBACK * WNDINFOENUMPROC )( FPTVWNDINFO fpWndInfo, LPARAM lParam );

/* EnumFiles() callback function. */
typedef BOOL ( CALLBACK * MENUINFOENUMPROC )( FPTVMENUINFO fpMenuInfo, LPARAM lParam );

/* EnumCmdLine() callback function. */
typedef BOOL ( CALLBACK * TVCMDLINEENUMPROC )( LPSTR szFilePath, LPSTR szFileArgs, LPARAM lParam );

/*************************************
*
*  Private Functions
*
**************************************/

/* EXTRBYTE.C */
void __far __pascal SetTVEntryCount( HWND hWnd, WORD wValue );
void __far __pascal IncTVEntryCount( HWND hWnd );
void __far __pascal DecTVEntryCount( HWND hWnd );
WORD __far __pascal GetTVEntryCount( HWND hWnd );
void __far __pascal SetChildWindowFilter( HWND hWnd );
void __far __pascal RemoveChildWindowFilter( HWND hWnd );

/* MESSAGE.C */
long __far __pascal __export TVFilterWndProc(HWND hWnd, WORD Msg, WORD wParam, LONG lParam);

/* TVLIB.C */
int __far __pascal LibMain( HINSTANCE hInst, WORD wDataSeg, WORD cbHeap, LPSTR lpszCmdLine );
int __far __pascal _WEP( int nExitType );

/* MEMORY.C */
BOOL __far __pascal GlobalAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwSize );
BOOL __far __pascal GlobalReAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwNewSize  );

/* FILES.C */
BOOL __far __pascal DoFileNamesMatch( char __far * szLibFilePath, char __far * szFilePath );
BOOL __far __pascal FindFile( char __far * szDir, char __far * szWildName, struct _find_t __far * FileInfo );
BOOL __far __pascal EnumTVLibFiles( HWND hWnd, char __far * szLibWildName, TVLIBFILESENUMPROC InsertLibInfo, LPARAM lParam, BOOL Yield );
BOOL __far __pascal EnumFilesProc( char __far * szLibFilePath, LPARAM lParam );
BOOL __far __pascal GetFilesExtension( char __far * szFilePath, char __far * szExtBuffer );

/* LIBLIST.C */
/*BOOL __far __pascal __export CreateTVLibList( char __far * szLibWildName ); */
/*void __far __pascal __export DestroyTVLibList( void ); */
/*void __far __pascal __export DeleteMenuInfoList( void ); */

/* LIBINFO.C */
BOOL __far __pascal InsertLibInfo( char __far * szLibFilePath, FPTVLIBFILEINFO fpTmpTVLibFileInfo, LPARAM lParam );
BOOL __far __pascal RemoveChildWindow( FPTVLIBINFO fpTVLibInfo, LPARAM lParam );
BOOL __far __pascal DoesLibSupportExt( FPTVLIBINFO fpTVLibInfo, char __far * szFilePath );
BOOL __far __pascal OpenLibrary( HWND hWnd, FPTVLIBINFO fpTVLibInfo );
void __far __pascal CloseLibrary( FPTVLIBINFO fpTVLibInfo );
void __far __pascal PostCloseLibrary( FPTVLIBINFO fpTVLibInfo  );
HTVWNDINFO __far __pascal GetLibWndInfo( FPTVLIBINFO fpTVLibInfo );
void __far __pascal DeleteLibInfoData( FPTVLIBINFO fpTVLibInfo );

/* OPEN.C */
BOOL __far __pascal OpenFileAsLibrary( FPTVLIBINFO fpTVLibInfo, LPARAM lParam );
BOOL __far __pascal OpenFileAsInputFile( FPTVLIBINFO fpTVLibInfo, LPARAM lParam );
BOOL __far __pascal GetInputFileName( HWND hWnd, char __far * szOpenFileName, FPTVLIBFILEINFO fpLibFileInfo );
BOOL __far __pascal OpenWindow( HWND hWnd, FPTVOPENWNDSTRUCT fpTVOpenWndStruct, FPTVLIBINFO fpTVLibInfo );
void __far __pascal InitWndExtraByteData( HWND hWnd );

/* TVINI.C */
BOOL __far __pascal GetTVINIFilePath( char __far * szTVINIFilePath );
BOOL __far __pascal GetTVINILibAssoc( char __far * szFileName, char __far * szLibNameBuffer );
BOOL __far __pascal GetTVINILoadLine( char __far * szBuffer );
BOOL CALLBACK OpenCmdLineWindows( LPSTR szFilePath, LPSTR szArgs, LPARAM lParam );
void __far __pascal EnumCmdLine( TVCMDLINEENUMPROC EnumCmdLineProc, LPSTR szCmdLine, LPARAM lParam );
char __far * GetNextToken( char __far ** szCurrentPos, char __far ** szNextPos, char chDelimiter );

/* WNDINFO.C */
BOOL CALLBACK IsTVWindowEngine( FPTVWNDINFO fpWndInfo, LPARAM lParam );
BOOL __far __pascal CloseAllTVWindows( void );
BOOL __far __pascal CloseAllLibsWindows( FPTVLIBINFO fpTVLibInfo, LPARAM lParam );

BOOL __far __pascal SaveWndInfo( HWND hErrorWnd, HWND hWndOpen, HTVWNDINFO __far * hTVWndInfo );
HTVWNDINFO __far __pascal NewWndInfo( HWND hErrorWnd, HWND hWndOpen );
void __far __pascal FreeWndInfo( HTVWNDINFO hWndInfo );
void __far __pascal InsertWndInfo( HTVWNDINFO __far * hTopWndInfo, HTVWNDINFO hNewWndInfo );
BOOL __far __pascal DeleteWndInfo( HTVWNDINFO __far * hFirstWndInfo, HWND hWnd );
HTVWNDINFO __far __pascal GetNextWndInfo( HTVWNDINFO hWndInfo );
BOOL __far __pascal FindWndInfo( HTVWNDINFO hFirstWndInfo, HWND hWnd );
void __far __pascal DeleteAllWndInfo( HTVWNDINFO __far * hFirstWndInfo );
void __far __pascal EnumWndInfo( WNDINFOENUMPROC WndInfoEnumProc, LPARAM lParam );
BOOL __far __pascal EnumWndInfoEngine( FPTVLIBINFO fpTVLibInfo, LPARAM lParam );
BOOL CALLBACK HideShowAllWndEngine( FPTVWNDINFO fpWndInfo, LPARAM lParam );
                                                                
/* MENULIST.C */
BOOL CALLBACK ProcessTVMenuItemEngine( FPTVMENUINFO fpMenuInfo, LPARAM lParam );

WORD __far __pascal GetNextMenuAlias( void );
BOOL CALLBACK AliasNumberUsed( FPTVMENUINFO fpMenuInfo, LPARAM lParam );

void __far __pascal EnumMenuInfo( MENUINFOENUMPROC MenuInfoEnumProc, LPARAM lParam );

BOOL __far __pascal SaveMenuItemInfo( HWND hErrorWnd, FPTVMENUINFO fpMenuData, HTVMENUINFO __far * hFirstMenuInfo );
HTVMENUINFO __far __pascal NewMenuInfo( HWND hErrorWnd, FPTVMENUINFO fpMenuData );
void __far __pascal FreeMenuInfo( HTVMENUINFO hMenuInfo );
void __far __pascal InsertMenuInfo( HTVMENUINFO __far * hFirstMenuInfo, HTVMENUINFO hNewMenuInfo );
BOOL __far __pascal DeleteMenuInfoForAlias( HTVMENUINFO __far * hFirstMenuInfo, WORD wParamAlias  );
HTVMENUINFO __far __pascal GetNextMenuInfo( HTVMENUINFO hMenuInfo );
void __far __pascal DeleteMenuInfoForWnd( HWND hWnd );



/*************************************
*
*  Global Variables
*
**************************************/
extern HINSTANCE  hDLLInst;
extern HTVLIBLIST hTVLibList;




#endif  /* #ifndef TVLIBPRV_H. */

