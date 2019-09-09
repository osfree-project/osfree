/***********************************************************************

    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

***********************************************************************/
/***************************************************************************************
**
**
**                              DLL Entry Functions
**
**
***************************************************************************************/
/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


/*************************************
*
*  Custom Includes
*
**************************************/
#include "twhlprc.h"
#include "twinhelp.h"
#include "msgbox.h"
#include "memory.h"
#include "globals.h"
#include "wndlist.h"
#include "tvlib.h"
#include "helpapi.h"
#include "btnbar.h"
#include "inifile.h"
#include "wndpos.h"


/*************************************
*
*  Private Defines
*
**************************************/

/* Maximum length of a help class name. */
#define MAX_CLASS_NAME   20                   

/* 
** Structure for storing data unique
** for each message redirector window.
*/
typedef HGLOBAL HWNDDATA;
typedef struct tagWNDDATA
{
  HWND hTwinViewWnd;                   /* Handle to TWINVIEW's main window. */
  HWNDINFO hWndInfoList;               /* Handle to the list managed by a message
                                          redirector window. */
} 
WNDDATA;
typedef WNDDATA __far * FPWNDDATA;


/* 
** Name of the message redirector window class when window is created
** for TWINVIEW.EXE.
**
** Window of this class is never visible.  It only redirects messages
** to a real help window.
*/
#define TWINVIEW_HELP_CLASS   "TWINVIEW_HELP"

/* 
** Window class name for all real/visible help windows.
*/
#define MS_WINDOC_CLASS   "MS_WINDOC"


/* 
** Name of the message redirector window class when testing is being done.
**
** Window of this class is never visible.  It only redirects messages
** to a real help window.
*/
#define TEST_WINHELP_CLASS  "TEST_HELP"


/*************************************
*
*  TwinView related functions.
*  Ordinally exported.
*
**************************************/
HWND __far __pascal __export TVLibOpenWindow( FPTVOPENWNDSTRUCT fpTVOpenWndStruct );
BOOL __far __pascal __export TVLibGetInfo( FPTVLIBFILEINFO fpTVLibFileInfo );
BOOL __far __pascal __export TVLibCloseWindow( HWND hWnd );


/*************************************
*
*  Function Prototypes
*
**************************************/

int __far __pascal LibMain( HINSTANCE hLibInst, WORD wDataSeg, WORD cbHeap, LPSTR lpszCmdLine );

static HWND __far __pascal GetHelpWindow( HWND hMainHelpWnd, HWND hAppWnd, HWNDINFO __far * hWndInfoList );

long __far __pascal __export SDKWndProc( HWND hWnd, WORD Msg, WORD wParam, LONG lParam );
long __far __pascal __export OneWindowWndProc( HWND hWnd, WORD Msg, WORD wParam, LONG lParam );



/*************************************
*
*  Global variables used in this file.
*
**************************************/

static char      szFilter[] = "Help Files (*.HLP)&*.HLP&";
static char      szIconText[] = "TwinHelp .HLP Viewer";
static char      szAppName[] = "TwinHelp";


/*************************************
*
*  LibMain()
*
**************************************/

int __far __pascal LibMain
( 
  HINSTANCE hLibInst, 
  WORD wDataSeg, 
  WORD cbHeap,
  LPSTR lpszCmdLine 
)
{
  ATOM ReturnValue; 
  WNDCLASS wndclass;
  
  
  /* Save instance of library. */ 
  SetLibInst( hLibInst );
  
  /* Register special help messages. */
  SetWINHELPMsg( RegisterWindowMessage( MSWIN_HELP_MSG_STRING ) );
  SetWINDOCMsg( RegisterWindowMessage( MSWIN_DOC_MSG_STRING ) );

  /* 
  ** Register the window class for the visible main help window.
  */
  wndclass.style         = 0 ;
  wndclass.lpfnWndProc   = MainWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = sizeof( HWNDDATA );
  wndclass.hInstance     = hLibInst;
  wndclass.hIcon         = LoadIcon( hLibInst, MAIN_ICON );
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
  wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = "TWINHELP" ;
  wndclass.lpszClassName = MS_WINDOC_CLASS ;
  
  ReturnValue = RegisterClass (&wndclass) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Register the window class for the visible topic help window that
  ** is scrollable.
  */
  wndclass.style         = CS_BYTEALIGNWINDOW | CS_OWNDC;
  wndclass.lpfnWndProc   = TopicWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = sizeof( HWNDDATA );
  wndclass.hInstance     = hLibInst;
  /* 
  ** If you use the Arrow cursor for the class, windows restores
  ** that for every mouse move and movement will be jerky.
  ** So Class Cursor is made NULL and we will manually set the 
  ** cursor ourselves on mouse moves. 
  */
  wndclass.hCursor       = (HCURSOR) NULL;

  wndclass.hIcon         = (HICON) NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = JUMPSCROLL_CLASS;

  ReturnValue = RegisterClass ( &wndclass ) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Register the window class for the visible topic help window that
  ** is not scrollable.
  */
  wndclass.style         = CS_BYTEALIGNWINDOW | CS_OWNDC;
  wndclass.lpfnWndProc   = NoScrollTopicWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = sizeof( HWNDDATA );
  wndclass.hInstance     = hLibInst;
  /* 
  ** If you use the Arrow cursor for the class, windows restores
  ** that for every mouse move and movement will be jerky.
  ** So Class Cursor is made NULL and we will manually set the 
  ** cursor ourselves on mouse moves. 
  */
  wndclass.hCursor       = (HCURSOR) NULL;

  wndclass.hIcon         = (HICON) NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = JUMPNOSCROLL_CLASS;

  ReturnValue = RegisterClass ( &wndclass ) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Register the window class for the visible popup help window.
  */
  wndclass.style         = CS_BYTEALIGNWINDOW | CS_OWNDC;
  wndclass.lpfnWndProc   = PopupWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = sizeof( HWNDDATA );
  wndclass.hInstance     = hLibInst;
  /* 
  ** If you use the Arrow cursor for the class, windows restores
  ** that for every mouse move and movement will be jerky.
  ** So Class Cursor is made NULL and we will manually set the 
  ** cursor ourselves on mouse moves. 
  */
  wndclass.hCursor       = (HCURSOR)NULL;

  wndclass.hIcon         = (HICON)NULL;
  wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = POPUP_CLASS;

  ReturnValue = RegisterClass ( &wndclass ) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Register the window class for the visible main secondary help window.
  */
  wndclass.style         = 0 ;
  wndclass.lpfnWndProc   = MainSecWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = sizeof( HWNDDATA );
  wndclass.hInstance     = hLibInst;
  wndclass.hIcon         = LoadIcon( hLibInst, MAIN_ICON );
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
  wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL ;
  wndclass.lpszClassName = MAINSEC_CLASS ;
  
  ReturnValue = RegisterClass (&wndclass) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Register the window class for the visible secondary window's
  ** topic help window that is scrollable.
  */
  wndclass.style         = CS_BYTEALIGNWINDOW | CS_OWNDC;
  wndclass.lpfnWndProc   = TopicSecWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = sizeof( HWNDDATA );
  wndclass.hInstance     = hLibInst;
  /* 
  ** If you use the Arrow cursor for the class, windows restores
  ** that for every mouse move and movement will be jerky.
  ** So Class Cursor is made NULL and we will manually set the 
  ** cursor ourselves on mouse moves. 
  */
  wndclass.hCursor       = (HCURSOR) NULL;

  wndclass.hIcon         = (HICON) NULL;
  wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName  = NULL;
  wndclass.lpszClassName = TOPICSEC_CLASS;

  ReturnValue = RegisterClass ( &wndclass ) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);



  /* 
  ** Create the message redirector window class - used when running under 
  ** MS-Windows and real WinHelp() API function calls TWINVIEW.EXE 
  ** (renamed winhelp.exe) with the -X parameter.
  */
  wndclass.style         = 0 ;
  wndclass.lpfnWndProc   = SDKWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = TVL_WNDEXTRABYTES + sizeof( HGLOBAL ) ;
  wndclass.hInstance     = hLibInst;
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName  = NULL ;
  wndclass.lpszClassName = TEST_WINHELP_CLASS ;
  
  ReturnValue = RegisterClass (&wndclass) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Create the message redirector window class - used when running under 
  ** MS-Windows and real WinHelp() API function calls TWINVIEW.EXE 
  ** (renamed winhelp.exe) with the -X parameter.
  */
  wndclass.style         = 0 ;
  wndclass.lpfnWndProc   = SDKWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = TVL_WNDEXTRABYTES + sizeof( HGLOBAL ) ;
  wndclass.hInstance     = hLibInst;
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName  = NULL ;
  wndclass.lpszClassName = MS_WINHELP_CLASS ;
  
  ReturnValue = RegisterClass (&wndclass) ;
  
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* 
  ** Register the message redirector window class - used when window is created
  ** for TWINVIEW.EXE and not because of a MS-Window WinHelp() function call.
  ** Could be running other UNIX or MS-Windows.
  */
  wndclass.style         = 0 ;
  wndclass.lpfnWndProc   = OneWindowWndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = TVL_WNDEXTRABYTES + sizeof( HGLOBAL ) ;
  wndclass.hInstance     = hLibInst;
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = NULL;
  wndclass.hbrBackground = NULL;
  wndclass.lpszMenuName  = NULL ;
  wndclass.lpszClassName = TWINVIEW_HELP_CLASS ;
  
  ReturnValue = RegisterClass (&wndclass) ;
                                                                    
  /* return 1 = success; 0 = fail */
  if( ReturnValue == 0 ) return(0);


  /* Register the button bar's class. */
  if( ! RegisterButtonBarClass( GetLibInst() ) ) return( 0 );


  /* Success. */
  return(1);
}


/*************************************
*
*  _WEP()
*
**************************************/

int __far __pascal _WEP( int nExitType )
{
  /* return 1 = success; 0 = fail */
  return(1);  
}



/*************************************
*
*  TVLibOpenWindow()
*
*  Only called by TWINVIEW.EXE
*
**************************************/

HWND __far __pascal __export TVLibOpenWindow( FPTVOPENWNDSTRUCT fpTVOpenWndStruct )
{
  HWND hWnd;


  /* 
  ** Used for testing.
  */
  if( fpTVOpenWndStruct == NULL )
  {
    hWnd = CreateWindow 
                (
                  (LPSTR) TEST_WINHELP_CLASS,
                  NULL,
                  WS_OVERLAPPEDWINDOW,
                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                  NULL, 
                  NULL, 
                  GetLibInst(), 
                  NULL
                );

    if( !hWnd )
    {
      /* Cannot create the help window. */ 
      MsgBox( GetLibInst(), NULL, IDS_ERRORTITLE, IDS_WINDOWCREATE, MB_ICONHAND | MB_OK );
      return 0;
    }
  
    /* Return handle of the new message redirector window. */  
    return hWnd;
  }
  

  /* 
  ** Running under MS-Windows and real WinHelp() API function calls 
  ** TWINVIEW.EXE with the -X parameter.
  */
  else if( fpTVOpenWndStruct->lParam == 1 )
  {
    hWnd = CreateWindow 
                (
                  (LPSTR) MS_WINHELP_CLASS,
                  NULL,
                  WS_OVERLAPPEDWINDOW,
                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                  NULL, 
                  NULL, 
                  GetLibInst(), 
                  (void __far *) &(fpTVOpenWndStruct)->hOwner
                );

    if( !hWnd )
    {
      /* Cannot create the help window. */ 
      MsgBox( GetLibInst(), NULL, IDS_ERRORTITLE, IDS_WINDOWCREATE, MB_ICONHAND | MB_OK );
      return 0;
    }
  
    /* Return handle of the new message redirector window. */  
    return hWnd;
  }
  

  /* 
  ** Window is created for TWINVIEW.EXE but NOT because of a MS-Windows WinHelp() 
  ** function call.  TWINVIEW.EXE can be running in UNIX or MS-WINDOWS for this
  ** when this class is created.
  */
  else
  {
    hWnd = CreateWindow 
                (
                  (LPSTR) TWINVIEW_HELP_CLASS,
                  NULL,
                  WS_OVERLAPPEDWINDOW,
                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                  NULL, 
                  NULL, 
                  GetLibInst(), 
                  (void __far *) fpTVOpenWndStruct
                );

    if( !hWnd )
    {
      /* Cannot create the help window. */ 
      MsgBox( GetLibInst(), fpTVOpenWndStruct->hOwner, IDS_ERRORTITLE, IDS_WINDOWCREATE, MB_ICONHAND | MB_OK );
      return 0;
    }
  
    /* Return handle of the new message redirector window. */  
    return hWnd;
  }
}


/*************************************
*
*  TVLibCloseWindow()
*
*  Only called by TWINVIEW.EXE
*
**************************************/

BOOL __far __pascal __export TVLibCloseWindow( HWND hWnd )
{
  SendMessage( hWnd, WM_CLOSE, 0, 0L );
  return TRUE;
}


/*************************************
*
*  TVLibGetInfo()
*
*  Only called by TWINVIEW.EXE
*
**************************************/

BOOL __far __pascal __export TVLibGetInfo( FPTVLIBFILEINFO fpTVLibFileInfo )
{
  /* Assign data to structure. */
  _fstrcpy( fpTVLibFileInfo->szFilter, szFilter );
  _fstrcpy( fpTVLibFileInfo->szIconResouce, MAIN_ICON );
  _fstrcpy( fpTVLibFileInfo->szIconCaption, szIconText );
  fpTVLibFileInfo->bAskForFile = TRUE;

  /* Return success. */
  return TRUE;
}


/*************************************
*
*  SDKWndProc() Window Proc.
*
*  Intercepts WM_WINHELP and WM_WINDOC 
*  messages for all help windows and 
*  routes the messages to the 
*  appropriate help window.
*
**************************************/

long __far __pascal __export SDKWndProc( HWND hWnd, WORD Msg, WORD wParam, LONG lParam )
{
  CREATESTRUCT __far * lpCreateStruct;   /* Data passed in LPARAM. */
  HWND __far * hTwinViewWnd;             /* Handle to the TWINVIEW.EXE main window. */

  HWNDDATA        hWndData ;             /* Handle to window extrabyte data. */
  FPWNDDATA       fpWndData ;            /* Pointer to window extrabyte data. */
  
  char lpszClassName[ MAX_CLASS_NAME ];  /* Name of a window's class. */
  
  HWND hHelpWnd;                         /* Handle to a help window. */

  
  /* 
  ** Handle WinHelp special messages - WM_WINDOC & WM_WINHELP. 
  */

  if( Msg == GetWINHELPMsg() || Msg == GetWINDOCMsg() )
  {
    /* Get window list. */
    hWndData = (HWNDDATA) GetWindowWord (hWnd, TVL_WNDEXTRABYTES ) ;
    fpWndData = (FPWNDDATA) GlobalLock( hWndData );

    /* Get/create the application's help window. */
    hHelpWnd = GetHelpWindow( hWnd, (HWND) wParam, &(fpWndData)->hWndInfoList );
          
    /* Send message to correct help window. */  
    if( hHelpWnd != 0 )
    {
      /* Route the message to the correct help window. */
      SendMessage( hHelpWnd, Msg, wParam, lParam );
    }

    return 0 ;
  }


  /* 
  ** Other Windows messages. 
  */
  switch (Msg)
  {
    case WM_CREATE:
    {
      /* Allocate memory for window private data. */
      hWndData = GlobalAlloc ( LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof (WNDDATA) ) ;
                              
      /* Error retrieving memory. */
      if ( hWndData == NULL ) 
      {
        /* Allocation error. */ 
        MsgBox( GetLibInst(), hWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
        return -1;
      }

      /* Initialize the window data. */
      fpWndData = (FPWNDDATA) GlobalLock( hWndData );
      fpWndData->hTwinViewWnd = 0;
      fpWndData->hWndInfoList = 0;
      GlobalUnlock( hWndData );

      /* Save handle to allocated window data. */
      SetWindowWord ( hWnd, TVL_WNDEXTRABYTES, (WORD) hWndData ) ;
        
      /*
      ** If we are running under TWINVIEW.EXE when the last help 
      ** window is tell TWINVIEW.EXE to close also.  To do this we
      ** will need to save the window handle for TWINVIEW.EXE's 
      ** main window.
      */

      /* Get class of window. */
      GetClassName( hWnd, (LPSTR) lpszClassName, MAX_CLASS_NAME );

      /* 
      ** If we were launched by TWINVIEW.EXE, via a MS-Windows
      ** WinHelp() call, save TWINVIEW's window handle? 
      */
      if( _fstricmp( lpszClassName, MS_WINHELP_CLASS ) == 0 )
      {
        /* Get TWINVIEW's window handle.*/
        lpCreateStruct = ( CREATESTRUCT __far * ) lParam;
        hTwinViewWnd = ( HWND __far * ) lpCreateStruct->lpCreateParams;
      
        /* Save TWINVIEW's window handle.*/
        fpWndData = (FPWNDDATA) GlobalLock( hWndData );
        fpWndData->hTwinViewWnd = *hTwinViewWnd;
        GlobalUnlock( hWndData );
      }
            
      /* Success. */
      return 0;
    }
    
    
    /* 
    ** A help window closed - see if we should close this main redirector 
    ** window. 
    */
    case HELPWM_CLOSE:
    {
      /* Get window list. */
      hWndData = (HWNDDATA) GetWindowWord (hWnd, TVL_WNDEXTRABYTES ) ;
      fpWndData = (FPWNDDATA) GlobalLock( hWndData );
      
      /* Remove the window from the list. */
      RemoveHelpWndInfo( &(fpWndData)->hWndInfoList, (HWND) wParam );
          
      /* Still have help windows under this window. */
      if( GetHelpWndCount( fpWndData->hWndInfoList ) != 0 ) 
      {
        GlobalUnlock( hWndData );
        return 0;
      }
      
      /* 
      ** Last help window - close this parent window. 
      */
      
      /* 
      ** If we were launched by TWINVIEW.EXE, via a MS-Windows
      ** WinHelp() call, close down TWINVIEW.EXE.
      */
      GetClassName( hWnd, (LPSTR) lpszClassName, MAX_CLASS_NAME );
          
      if( _fstricmp( lpszClassName, MS_WINHELP_CLASS ) == 0 )
      {
        /* Tell twinview to close down. */
        PostMessage( fpWndData->hTwinViewWnd, WM_SYSCOMMAND, SC_CLOSE, 0L );
      }
                
      GlobalUnlock( hWndData );
              
      /* Destroy the window. */
      DestroyWindow( hWnd ) ;
              
      return 0;
    }

    
    case WM_DESTROY:
    {
      hWndData = (HWNDDATA) GetWindowWord (hWnd, TVL_WNDEXTRABYTES ) ;
      GlobalFree ( hWndData ) ;
      return 0 ; 
    }
  }

  // Pass unprocessed message to DefWindowProc
  return DefWindowProc (hWnd, Msg, wParam, lParam) ;
}




/*************************************
*
*  OneWindowWndProc() Window Proc.
*
*  Window is created for TWINVIEW.EXE but NOT because of a MS-Windows WinHelp() 
*  function call.  TWINVIEW.EXE can be running in UNIX or MS-WINDOWS for this
*  when this window is created.
*
*  Creates a help window.
*
**************************************/

long __far __pascal __export OneWindowWndProc( HWND hWnd, WORD Msg, WORD wParam, LONG lParam )
{
  CREATESTRUCT __far * lpCreateStruct;   /* Data passed in LPARAM. */
  FPTVOPENWNDSTRUCT fpTVOpenWndStruct;

  HWND hHelpWnd;                         /* Handle to a help window. */


  switch (Msg)
  {
    case WM_CREATE:
    {
      /* Get TWINVIEW's data.*/
      lpCreateStruct = ( CREATESTRUCT __far * ) lParam;
      fpTVOpenWndStruct = ( FPTVOPENWNDSTRUCT )  lpCreateStruct->lpCreateParams;
      
      /* Create a main help window. */
      hHelpWnd = CreateWindow 
                  (
                    (LPSTR) MS_WINDOC_CLASS,
                    (LPSTR) NULL,
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    NULL, 
                    NULL, 
                    GetLibInst(), 
                    (void __far *) (HWND __far *) &hWnd
                  );

      if( !hHelpWnd )
      {
        /* Cannot create the help window. */ 
        MsgBox( GetLibInst(), hWnd, IDS_ERRORTITLE, IDS_WINDOWCREATE, MB_ICONHAND | MB_OK );
        return -1;
      }

      /* Show window. */
      ShowWindow ( hHelpWnd, SW_SHOW ) ;
      UpdateWindow ( hHelpWnd ) ;

      /* Show the help file's default topic. */
      SendMessage( hHelpWnd, PRIVMSG_HELP_CONTENTS, 0, 
                   (LPARAM) (char __far *) fpTVOpenWndStruct->szArguments ); 
        
      /* Success. */
      return 0;
    }
    
    
    /* 
    ** A help window closed - see if we should close this redirector 
    ** window. 
    */
    case HELPWM_CLOSE:
    {
      /* 
      ** Now close ourselves down. 
      */
      PostMessage( hWnd, WM_CLOSE, 0, 0L );
        
      return 0;
    }

    
    case WM_CLOSE:
    {
      /* Destroy the window. */
      DestroyWindow( hWnd ) ;
      return 0;
    }
  }

  // Pass unprocessed message to DefWindowProc
  return DefWindowProc (hWnd, Msg, wParam, lParam) ;
}


/****************************************************
*
*  GetHelpWindow()
*
*  Find the correct help window (we allow mult. windows)
*  for the calling app. or create one if one doesn't exist.
*
*  Returns:
*
*     0 - Failure.
*     HWND - Success.
*
*****************************************************/
static HWND __far __pascal GetHelpWindow( HWND hMainHelpWnd, HWND hAppWnd, HWNDINFO __far * hWndInfoList )
{
  WINDOWPLACEMENT WndPlace;
  HWND hHelpWnd;                /* Handle to a help window for the calling app. */
  BOOL bSettingFound;
  
  /* Check to see if we already have a help window for the application. */
  hHelpWnd = GetAppsHelpWnd( *hWndInfoList, hAppWnd );

  /* No existing help window.  Create a HELPEX_HELP window. */
  if( hHelpWnd == 0 )
  {
    /* Get last saved size and position. */
    bSettingFound = ReadLastMainWndPos( &WndPlace );

    /* If should be not maximized. */
    if( WndPlace.showCmd != SW_SHOWMAXIMIZED && bSettingFound )
    {
      /* Convert virtual position to physical coords. */
      VirtualPosToScrnPos( hMainHelpWnd, &(WndPlace).rcNormalPosition );
    }
    /* Create a help window. */
    hHelpWnd = CreateWindow 
                (
                  (LPSTR) MS_WINDOC_CLASS,
                  (LPSTR) NULL,
                  WS_OVERLAPPEDWINDOW,
                  WndPlace.rcNormalPosition.left, 
                  WndPlace.rcNormalPosition.top, 
                  WndPlace.rcNormalPosition.right, 
                  WndPlace.rcNormalPosition.bottom,
                  NULL, 
                  NULL, 
                  GetLibInst(), 
                  (void __far *) (HWND __far *) &hMainHelpWnd
                );

    if( !hHelpWnd )
    {
      /* Cannot create the help window. */ 
      MsgBox( GetLibInst(), hAppWnd, IDS_ERRORTITLE, IDS_WINDOWCREATE, MB_ICONHAND | MB_OK );
      return 0;
    }

    /* Show window. */
    ShowWindow ( hHelpWnd, SW_HIDE ) ;
    UpdateWindow ( hHelpWnd ) ;

    /* If should be maximized. */
    if( WndPlace.showCmd == SW_SHOWMAXIMIZED )
    {
      ShowWindow ( hHelpWnd, SW_SHOWMAXIMIZED ) ;
      ShowWindow ( hHelpWnd, SW_HIDE ) ;
    }
    
    /* 
    ** Save App./Help Window pair to window list. 
    */
    InsertHelpWndInfo( hAppWnd, hWndInfoList, hAppWnd, hHelpWnd );
  }
  
  /* Return message sending failure/success. */
  return( hHelpWnd );
}



