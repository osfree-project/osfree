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
**                            Secondary Help Window Functions
**
**
***************************************************************************************/


/***********************************
**
**  System Includes
**
***********************************/
#include <string.h>


/*************************************
*
*  App. Includes
*
**************************************/
#include "secwnd.h"
#include "msgbox.h"
#include "winmem.h"
#include "hlpfile.h"
#include "wnddata.h"
#include "twinhelp.h"
#include "globals.h"
#include "twhlprc.h"


/*************************************
*
*  Private Defines
*
**************************************/

/*
** Secondary Window List Structure
*/
typedef struct tagSECWNDINFO
{
  WORD wSecWndNum;
  char szHelpFilePath[_MAX_PATH];
  HWND hSecWnd;
  HSECWNDINFO Next;
}
SECWNDINFO;
typedef SECWNDINFO __far * FPSECWNDINFO;

typedef BOOL ( CALLBACK * WNDINFOENUMPROC )( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam );



/*************************************
*
*  Private Function Prototypes
*
**************************************/

static BOOL CALLBACK CloseAllSecWndProc( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam );
static BOOL CALLBACK GetSecWndHandleProc( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam );
static HWND __far __pascal GetSecWndHandle
( 
  HSECWNDINFO hSecWndInfo,
  WORD wSecWndNum,
  char __far * szHelpFilePath 
);
static BOOL CALLBACK GetSecWndHandleProc( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam );
static BOOL __far __pascal InsertSecWnd
( 
  HWND hErrorWnd, 
  HSECWNDINFO __far * hFirstSecWndInfo,
  WORD wSecWndNum,
  HWND hSecWnd,
  char __far * szHelpFilePath
);
static BOOL __far __pascal RemoveSecWndInfo
( 
  HSECWNDINFO __far * hFirstSecWndInfo,
  HWND hSecWnd
);
static HSECWNDINFO __far __pascal GetNextWndInfo( HSECWNDINFO hSecWndInfo );
static void __far __pascal EnumWndInfo
( 
  WNDINFOENUMPROC WndInfoEnumProc, 
  HSECWNDINFO hFirstSecWndInfo, 
  LPARAM lParam 
);

static BOOL CALLBACK SecWndExistsProc( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam );



/***********************************  Shared Functions  *********************************/


/****************************************************
*
*  ShowSecWndTopic()
*
*  Find the correct secondary help window 
*  or create one if one doesn't exist.
*
*  Tell the window to show the specified topic.
*
*  Returns:
*
*     0 - Failure.
*     HWND - Success.
*
*****************************************************/
void __far __pascal ShowSecWndTopic
(
  HWND hDataWnd, 
  HTOPICDATA hNewTopicsData,
  WORD wSecWndNum 
)
{
  HWND hSecWnd, hTopicWnd;

  HSECWNDINFO hFirstSecWndInfo;
  FPTOPICDATA TopicDataPtr;

  char szCaption[MAX_CAPTION];
  WINDOWPLACEMENT WndPlace;
  COLORREF ScrollColor;
  COLORREF NonScrollColor;

  BOOL bOnTop;
  
  NEWTOPICSTRUCT NewTopicStruct;
  
  BOOL bFound,
       bMainWnd;
  
  
  /* Lock the topic data. */
  TopicDataPtr = (FPTOPICDATA) GlobalLock( hNewTopicsData );

  /* Get desired size and position for secondary window. */
  bFound = GetSecWndNumFileData( hDataWnd, TopicDataPtr->hHelpFileInfo, wSecWndNum, 
                                 szCaption, &WndPlace, &ScrollColor, &NonScrollColor,
                                 &bOnTop, &bMainWnd );

  /* If we did not find any info for this window. */
  if( !bFound )
  {
    /* Release the buffer lock on the new topic's data. */
    UnlockTopicData( hDataWnd, hNewTopicsData );

    /* Unlock the topic data. */
    GlobalUnlock( hNewTopicsData );
   
    return;
  }

  /* Is the secondary window is not the main help window. */
  if( !bMainWnd )
  {
    /* Get handle of secondary window data. */
    hFirstSecWndInfo = (HGLOBAL) WndData( hDataWnd, WDM_GETSECWNDDATA, 0L, NULL );
  
    /* Check to see if the secondary help window is already open. */
    hSecWnd = GetSecWndHandle( hFirstSecWndInfo, wSecWndNum, TopicDataPtr->szHelpFile );
  
    /* No existing secondary help window.  Create a secondary window. */
    if( hSecWnd == 0 )
    {
      /* Create a help window. */
      hSecWnd = CreateWindow 
                  (
                    (LPSTR) MAINSEC_CLASS,
                    (LPSTR) szCaption,
                    WS_OVERLAPPEDWINDOW,
                    WndPlace.rcNormalPosition.left, 
                    WndPlace.rcNormalPosition.top, 
                    WndPlace.rcNormalPosition.right, 
                    WndPlace.rcNormalPosition.bottom,
                    NULL, 
                    NULL, 
                    GetLibInst(), 
                    (void __far *) (HWND __far *) &hDataWnd
                  );
  
      if( !hSecWnd )
      {
        /* Cannot create the help window. */ 
        MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, IDS_WINDOWCREATE, MB_ICONHAND | MB_OK );
   
        /* Unlock the topic data. */
        GlobalUnlock( hNewTopicsData );
      }
  
      /* 
      ** Save secondary window #/HWND pair to window list. 
      */
      InsertSecWnd( hDataWnd, &hFirstSecWndInfo, wSecWndNum, hSecWnd, TopicDataPtr->szHelpFile );
  
      /* Get handle of secondary window data. */
      WndData( hDataWnd, WDM_SETSECWNDDATA, (DWORD) hFirstSecWndInfo, NULL );
  
      /* Show window. */
      ShowWindow ( hSecWnd, WndPlace.showCmd ) ;
      UpdateWindow ( hSecWnd ) ;
    }

    /* Existing secondary help window. */
    else
    {
      /* If its minimized, restore it. */
      if( IsIconic( hSecWnd ) )
      {
        ShowWindow( hSecWnd, SW_RESTORE );
      }
    }
    
    /* Unlock the topic data. */
    GlobalUnlock( hNewTopicsData );
     
    /* Set the "on-top" menu item. */
    if( bOnTop )
    {
      SetAlwaysOnTop( hDataWnd, TRUE );
    }

    /* 
    ** Build message data structure. 
    */
    NewTopicStruct.hNewTopicsData  = hNewTopicsData;
    NewTopicStruct.bUpdateBackList = FALSE;
    NewTopicStruct.wVertPos        = 0;
    NewTopicStruct.ScrollColor     = ScrollColor;
    NewTopicStruct.NonScrollColor  = NonScrollColor;
  
    /* Send the message. */
    SendMessage( hSecWnd, PRIVMSG_SHOWNEWTOPIC, 0, (LPARAM)(FPNEWTOPICSTRUCT) &NewTopicStruct );
  
    /* Set active window. */
    SetActiveWindow( hSecWnd );
  }
  
  /* Secondary window is main window. */ 
  else
  {
    /* Unlock the topic data. */
    GlobalUnlock( hNewTopicsData );
     
    /* Is hidden, make window visble. */
    if( !IsWindowVisible( hDataWnd ) )
    {
      ShowWindow( hDataWnd, SW_SHOW );
    }
    
    /* If its minimized, restore it. */
    if( IsIconic( hDataWnd ) )
    {
      ShowWindow( hDataWnd, SW_RESTORE );
    }

    /* Get the window handle for the main topic window. */
    hTopicWnd = (HWND) WndData( hDataWnd, WDM_GETMAINTOPICHWND, 0, NULL );
           
    /* Tell main topic window to show topic. */
    SendShowNewTopicMsg( hTopicWnd, hNewTopicsData, TRUE, 0 );

    /* Set active window. */
    SetActiveWindow( hDataWnd );
  }
} 



/****************************************************
*
*  DoMainSecWindow()
*
*  If there is secondary window info. for the
*  main window, we position the main window and
*  set the colors and on-top data. 
*
*  Tell the window to show the specified topic.
*
*  Returns:
*
*     FALSE - No secondary info. for main window.
*      TRUE - Secondary info. for main window.
*
*****************************************************/
BOOL __far __pascal DoMainSecWindow
(
  HWND hMainWnd, 
  HTOPICDATA hNewTopicsData,
  COLORREF __far * ScrollColor,
  COLORREF __far * NonScrollColor
)
{
  FPTOPICDATA TopicDataPtr;

  BOOL bUseSecWndCaption;
  char szCaption[MAX_CAPTION];
  WINDOWPLACEMENT WndPlace;
  BOOL bOnTop;
  
  BOOL bFoundEntry;
  
 
  /* Lock the topic data. */
  TopicDataPtr = (FPTOPICDATA) GlobalLock( hNewTopicsData );

  /* Get desired size and position for secondary window. */
  bFoundEntry = GetMainSecWndFileData( hMainWnd, TopicDataPtr->hHelpFileInfo, &bUseSecWndCaption,
                                       szCaption, &WndPlace, ScrollColor, NonScrollColor,
                                       &bOnTop );
  /* Unlock the topic data. */
  GlobalUnlock( hNewTopicsData );

  /* No secondary window data for the main window. */
  if( bFoundEntry == FALSE ) return( FALSE );

  /* Set the window's caption. */
  if( bUseSecWndCaption )
  {
    /* Set window's caption. */
    SetWindowText( hMainWnd, (LPCSTR) szCaption );
  }
  else
  {
    /* Set title from .HLP file TITLE= field. */
    SetMainWindowTitle( hMainWnd, hNewTopicsData );
  }

  /* If window should be maximized. */
  if( WndPlace.showCmd == SW_SHOWMAXIMIZED )
  {
      ShowWindow( hMainWnd, SW_SHOWMAXIMIZED );
      UpdateWindow ( hMainWnd ) ;
  }
  else
  { 
    /* If we have position and size info. */
    if( WndPlace.rcNormalPosition.left != CW_USEDEFAULT &&
        WndPlace.rcNormalPosition.top != CW_USEDEFAULT &&
        WndPlace.rcNormalPosition.right != CW_USEDEFAULT &&
        WndPlace.rcNormalPosition.bottom != CW_USEDEFAULT )
    {
      /* Move and size the window. */
      SetWindowPos( hMainWnd, NULL, 
                    WndPlace.rcNormalPosition.left, 
                    WndPlace.rcNormalPosition.top, 
                    WndPlace.rcNormalPosition.right, 
                    WndPlace.rcNormalPosition.bottom,
                    SWP_NOZORDER );
      UpdateWindow ( hMainWnd ) ;
    }
  }
  
  /* Set the "on-top" menu item. */
  if( bOnTop )
  {
    SetAlwaysOnTop( hMainWnd, TRUE );
  }

  /* Found secondary window info. */
  return( TRUE );  
} 


/*************************************
*
*  This function is called by each
*  secondary window as it closes.
*                  
**************************************/
void __far __pascal SecondaryWindowClosing( HWND hDataWnd, HWND hSecWnd )
{
  HSECWNDINFO hFirstSecWndInfo;

  /* Get the handle of info. structure. */
  hFirstSecWndInfo = (HGLOBAL) WndData( hDataWnd, WDM_GETSECWNDDATA, 0L, NULL );

  /* Remoe window from the list. */
  RemoveSecWndInfo( &hFirstSecWndInfo, hSecWnd );

  /* Get handle of secondary window data. */
  WndData( hDataWnd, WDM_SETSECWNDDATA, (DWORD) hFirstSecWndInfo, NULL );

  /* If main window is hidden. */
  if( !IsWindowVisible( hDataWnd ) && !SecWndExists( hDataWnd ) )
  {
    /* Close main window. */
    SendMessage( hDataWnd, WM_CLOSE, 0, 0L );
  }
}


/*************************************
*
*  This function is called by each
*  secondary window as it closes.
*                  
**************************************/
BOOL __far __pascal SecWndExists( HWND hDataWnd )
{
  BOOL bExists;
  HSECWNDINFO hFirstSecWndInfo;


  /* Get the handle of info. structure. */
  hFirstSecWndInfo = (HGLOBAL) WndData( hDataWnd, WDM_GETSECWNDDATA, 0L, NULL );

  /* No windows. */
  bExists = FALSE;

  /* Search thru each library. */
  EnumWndInfo( SecWndExistsProc, hFirstSecWndInfo, (LPARAM) (BOOL __far *) &bExists );

  /* Return result. */
  return( bExists );
}



/***********************************  Private Functions  *********************************/

/*******************************************
** 
** Set title of help window.
**
********************************************/
void __far __pascal SetMainWindowTitle( HWND hWnd, HTOPICDATA hTopicData )
{
  char szTitleText[ MAX_CAPTION ];
  
  HGLOBAL      hUserTitleText;
  char __far * fpUserTitleText;
  
  FPTOPICDATA  fpTopicData;
  
  /* Lock topic data. */
  fpTopicData = ( FPTOPICDATA ) GlobalLock( hTopicData );
  
  /* Use the user define title. */
  hUserTitleText = GetTitleText( fpTopicData->hHelpFileInfo );

  /* Unlock topic data. */
  GlobalUnlock( hTopicData );
  
  /* If there is a title. */
  if( hUserTitleText != NULL )
  {   
    /* Lock the user's title text data. */
    fpUserTitleText = (char _far *) GlobalLock( hUserTitleText );

    /* Empty string? */
    if( *fpUserTitleText == '\0' )
    {
      /* Unlock the user's title text data. */
      GlobalUnlock( hUserTitleText );

      /* Load the default title string. */
      LoadString( GetLibInst(), IDS_DEFAULTTITLE, szTitleText, sizeof( szTitleText ) - 1 );
       
      /* Set window's text to default title string. */
      SetWindowText( hWnd, (LPCSTR) szTitleText );

      /* Done. */
      return;
    }
    
    /* Set window's text to user defined title. */
    SetWindowText( hWnd, fpUserTitleText );
  
    /* Unlock the user's title text data. */
    GlobalUnlock( hUserTitleText );
    
    /* Done. */
    return;
  }

  /* Use the default title. */
  else
  {
    /* Load the caption string. */
    LoadString( GetLibInst(), IDS_DEFAULTTITLE, szTitleText, sizeof( szTitleText ) - 1 );
     
    /* Set window's text. */
    SetWindowText( hWnd, (LPCSTR) szTitleText );
  }
}


/*************************************
*
*  EnumWndInfo() callback function 
*  used in SecWndExist().
*                  
**************************************/
static BOOL CALLBACK SecWndExistsProc( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam )
{
  BOOL __far * bExistsPtr;
  
  /* Get pointer to data. */
  bExistsPtr = (BOOL __far *) lParam;
         
  /* Found a secondary window. */
  *bExistsPtr = TRUE;
      
  /* No more enumerating. */
  return( FALSE );
}


/*************************************
*
*  Get a secondary window's handle.
*                  
**************************************/
static HWND __far __pascal GetSecWndHandle
( 
  HSECWNDINFO hFirstSecWndInfo,
  WORD wSecWndNum,
  char __far * szHelpFilePath 
)
{
  SECWNDINFO SecWndInfo;

  /* Initialize private data. */
  SecWndInfo.wSecWndNum = wSecWndNum;
  _fstrcpy( SecWndInfo.szHelpFilePath, szHelpFilePath ); 
  SecWndInfo.hSecWnd    = 0;
             
  /* Search thru each library. */
  EnumWndInfo( GetSecWndHandleProc, hFirstSecWndInfo, (LPARAM) (FPSECWNDINFO) &SecWndInfo );

  /* Return found value. */
  return( SecWndInfo.hSecWnd );
}


/*************************************
*
*  EnumWndInfo() callback function 
*  used in GetSecWndHandle().
*                  
**************************************/
static BOOL CALLBACK GetSecWndHandleProc( FPSECWNDINFO SecWndInfoPtr, LPARAM lParam )
{
  FPSECWNDINFO SearchInfoPtr;
  
  /* Get pointer to search data. */
  SearchInfoPtr = (FPSECWNDINFO) lParam;
         
  /* Correct secondary window? */
  if( SearchInfoPtr->wSecWndNum == SecWndInfoPtr->wSecWndNum )
  {
    if( _fstrcmp( SearchInfoPtr->szHelpFilePath, SecWndInfoPtr->szHelpFilePath ) == 0 )
    {
      /* Save window handle. */ 
      SearchInfoPtr->hSecWnd = SecWndInfoPtr->hSecWnd;
      
      /* No more enumerating. */
      return( FALSE );
    }
  }
    
  /* Keep enumerating. */
  return TRUE;
}


/*************************************
*
*  A new secondary window has been opened.
*  Insert the new window's info into 
*  a the window info. list.
*                  
**************************************/
static BOOL __far __pascal InsertSecWnd
( 
  HWND hErrorWnd, 
  HSECWNDINFO __far * hFirstSecWndInfo,
  WORD wSecWndNum,
  HWND hSecWnd,
  char __far * szHelpFilePath
)
{
  FPSECWNDINFO SecWndInfoPtr;
  HSECWNDINFO  hNewSecWndInfo;
  

  /* Allocate a new node. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewSecWndInfo, sizeof(SECWNDINFO) ) ) return FALSE;
  
  /* Lock it. */
  SecWndInfoPtr = (FPSECWNDINFO) GlobalLock( hNewSecWndInfo ) ;

  /* Initialize structure. */
  SecWndInfoPtr->wSecWndNum  = wSecWndNum;
  SecWndInfoPtr->hSecWnd     = hSecWnd;
  _fstrcpy( SecWndInfoPtr->szHelpFilePath, szHelpFilePath );
   
  /* Existing first node. */
  if( *hFirstSecWndInfo != 0 ) 
  {
    /* Point new node to existing top node. */
    SecWndInfoPtr->Next = *hFirstSecWndInfo;
  }
  else
  {
    /* Point new node to existing top node. */
    SecWndInfoPtr->Next = 0;
  }

  /* Unlock it. */
  GlobalUnlock( hNewSecWndInfo ) ;

  /* Assign new node as top. */
  *hFirstSecWndInfo = hNewSecWndInfo;
  
  /* Return success. */
  return TRUE;
}



/*************************************
*
*  Remove and frees a secondary
*  window's info. from the list.
*                  
**************************************/
static BOOL __far __pascal RemoveSecWndInfo
( 
  HSECWNDINFO __far * hFirstSecWndInfo,
  HWND hSecWnd
)
{
  FPSECWNDINFO SecWndInfoPtr, TmpSecWndInfoPtr;
  HSECWNDINFO  hSecWndInfo, hTmpSecWndInfo;
  

  /* Existing first node. */
  if( *hFirstSecWndInfo == 0 ) return FALSE;

  /* Lock top handle. */
  SecWndInfoPtr = (FPSECWNDINFO) GlobalLock( *hFirstSecWndInfo ) ;
  
  /* If deleting top node. */
  if( SecWndInfoPtr->hSecWnd == hSecWnd )
  {
    /* Next node is to be new top node. */
    hTmpSecWndInfo = SecWndInfoPtr->Next; 

    /* Unlock top handle. */
    GlobalUnlock( *hFirstSecWndInfo );
    
    /* Free top. */
    GlobalFree( *hFirstSecWndInfo ); 
  
    /* Assign new top. */
    *hFirstSecWndInfo = hTmpSecWndInfo;

    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( *hFirstSecWndInfo );
    
  /* Find correct node and remove. */
  hTmpSecWndInfo = *hFirstSecWndInfo;
  hSecWndInfo = GetNextWndInfo( *hFirstSecWndInfo );

  while( hSecWndInfo != 0 )
  {
    /* Lock current node. */
    SecWndInfoPtr = (FPSECWNDINFO) GlobalLock( hSecWndInfo ) ;
  
    /* If deleting current node. */
    if( SecWndInfoPtr->hSecWnd == hSecWnd )
    {
      /* Lock previous node. */
      TmpSecWndInfoPtr = (FPSECWNDINFO) GlobalLock( hTmpSecWndInfo ) ;

      /* Previous node's "next node" is deleted node's "next node". */
      TmpSecWndInfoPtr->Next = SecWndInfoPtr->Next; 
  
      /* Unlock previous node. */
      GlobalUnlock( hTmpSecWndInfo );

      /* Unlock current node. */
      GlobalUnlock( hSecWndInfo );
      
      /* Free top. */
      GlobalFree( hSecWndInfo ); 
  
      /* Success. */
      return TRUE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hSecWndInfo ) ;

    /* Get next node. */
    hTmpSecWndInfo = hSecWndInfo;
    hSecWndInfo = GetNextWndInfo( hSecWndInfo );
  }

  /* Did not find window. */
  return FALSE;
}


/*************************************
*
*  Get the next window info. structure
*  after the given hSecWndInfo.
*                  
**************************************/
static HSECWNDINFO __far __pascal GetNextWndInfo( HSECWNDINFO hSecWndInfo )
{
  HSECWNDINFO hNextWndInfo;
  FPSECWNDINFO SecWndInfoPtr;
  
  /* Handle bad. */
  if( hSecWndInfo == 0 ) return 0;

  /* Lock it. */
  SecWndInfoPtr = (FPSECWNDINFO) GlobalLock( hSecWndInfo ) ;
  
  /* Point new node to existing top node. */
  hNextWndInfo = SecWndInfoPtr->Next;

  /* Unlock it. */
  GlobalUnlock( hSecWndInfo ) ;

  /* Return next window info's handle. */
  return hNextWndInfo;
}



/*************************************
*
*  Enumerate all SECWNDINFO structures
*  contained in the given list.
*                  
**************************************/
static void __far __pascal EnumWndInfo
( 
  WNDINFOENUMPROC WndInfoEnumProc, 
  HSECWNDINFO hFirstSecWndInfo, 
  LPARAM lParam 
)
{
  FPSECWNDINFO SecWndInfoPtr;
  HSECWNDINFO hSecWndInfo;
  
  BOOL bReturn;
  
  /* Get first window info's handle. */
  hSecWndInfo = hFirstSecWndInfo;
  
  /* Stop when we run out of windows in the list. */
  while( hSecWndInfo != 0 )
  {
    /* Lock it. */
    SecWndInfoPtr = (FPSECWNDINFO) GlobalLock( hSecWndInfo ) ;
  
    /* Call callback function. */
    bReturn = (*WndInfoEnumProc)( SecWndInfoPtr, lParam );
    
    /* Stop enumerating. */
    if( bReturn == FALSE )
    {
      /* Unlock it. */
      GlobalUnlock( hSecWndInfo );
      return;
    }
    
    /* Unlock it. */
    GlobalUnlock( hSecWndInfo ) ;

    hSecWndInfo = GetNextWndInfo( hSecWndInfo );
  }
}

