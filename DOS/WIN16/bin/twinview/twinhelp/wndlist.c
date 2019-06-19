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
**                              Help Window List Functions
**
**
***************************************************************************************/


/*************************************
*
*  Custom Includes
*
**************************************/
#include "wndlist.h"
#include "msgbox.h"
#include "winmem.h"



/*************************************
*
*  Private Defines
*
**************************************/
/*
** Window List Structure
*/
typedef struct tagWNDINFO
{
  HWND hAppWnd;
  HWND hHelpWnd;
  HWNDINFO Next;
}
WNDINFO;
typedef WNDINFO __far * FPWNDINFO;


typedef BOOL ( CALLBACK * WNDINFOENUMPROC )( FPWNDINFO fpWndInfo, LPARAM lParam );



/*************************************
*
*  Private Function Prototypes
*
**************************************/
static HWNDINFO __far __pascal NewWndInfo( HWND hErrorWnd, HWND hAppWnd, HWND hHelpWnd );

static void __far __pascal FreeWndInfo( HWNDINFO hWndInfo );

static void __far __pascal InsertWndInfo( HWNDINFO __far * hFirstWndInfo, HWNDINFO hNewWndInfo );

static void __far __pascal EnumWndInfo( WNDINFOENUMPROC WndInfoEnumProc, HWNDINFO hFirstWndInfo, LPARAM lParam );

static BOOL CALLBACK GetAppsHelpWndProc( FPWNDINFO fpWndInfo, LPARAM lParam );

static BOOL CALLBACK GetHelpWndCountProc( FPWNDINFO fpWndInfo, LPARAM lParam );

static HWNDINFO __far __pascal GetNextWndInfo( HWNDINFO hWndInfo );



/***********************************  Shared Functions  *********************************/


/*************************************
*
*  A help window has been opened.
*  Insert the new window's info into 
*  a help window info. list.
*                  
**************************************/
BOOL __far __pascal InsertHelpWndInfo
( 
  HWND hErrorWnd, 
  HWNDINFO __far * hFirstWndInfo, 
  HWND hAppWnd, 
  HWND hHelpWnd 
)
{
  HWNDINFO hNewWndInfo;

  
  /* Create new WNDINFO structure. */
  hNewWndInfo = NewWndInfo( hErrorWnd, hAppWnd, hHelpWnd );
  if( hNewWndInfo == 0 ) return FALSE; 
  
  /*
  ** Insert the TVWNDINFO structure into the Window List.
  */
  InsertWndInfo( hFirstWndInfo, hNewWndInfo );
  
  /* Return success. */
  return TRUE;
}



/*************************************
*
*  Remove and free a help window's
*  info. from a window list.
*                  
**************************************/
BOOL __far __pascal RemoveHelpWndInfo
( 
  HWNDINFO __far * hFirstWndInfo, 
  HWND hHelpWnd 
)
{
  FPWNDINFO fpWndInfo, fpTmpWndInfo;
  HWNDINFO hWndInfo, hTmpWndInfo;
  

  /* Existing first node. */
  if( *hFirstWndInfo == 0 ) return FALSE;

  /* Lock top handle. */
  fpWndInfo = (FPWNDINFO) GlobalLock( *hFirstWndInfo ) ;
  
  /* If deleting top node. */
  if( fpWndInfo->hHelpWnd == hHelpWnd )
  {
    /* Next node is to be new top node. */
    hTmpWndInfo = fpWndInfo->Next; 

    /* Unlock top handle. */
    GlobalUnlock( *hFirstWndInfo );
    
    /* Free top. */
    FreeWndInfo( *hFirstWndInfo ); 
  
    /* Assign new top. */
    *hFirstWndInfo = hTmpWndInfo;

    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( *hFirstWndInfo );
    
  /* Find correct node and remove. */
  hTmpWndInfo = *hFirstWndInfo;
  hWndInfo = GetNextWndInfo( *hFirstWndInfo );

  while( hWndInfo != 0 )
  {
    /* Lock current node. */
    fpWndInfo = (FPWNDINFO) GlobalLock( hWndInfo ) ;
  
    /* If deleting current node. */
    if( fpWndInfo->hHelpWnd == hHelpWnd )
    {
      /* Lock previous node. */
      fpTmpWndInfo = (FPWNDINFO) GlobalLock( hTmpWndInfo ) ;

      /* Previous node's "next node" is deleted node's "next node". */
      fpTmpWndInfo->Next = fpWndInfo->Next; 
  
      /* Unlock previous node. */
      GlobalUnlock( hTmpWndInfo );

      /* Unlock current node. */
      GlobalUnlock( hWndInfo );
      
      /* Free top. */
      FreeWndInfo( hWndInfo ); 
  
      /* Success. */
      return TRUE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hWndInfo ) ;

    /* Get next node. */
    hTmpWndInfo = hWndInfo;
    hWndInfo = GetNextWndInfo( hWndInfo );
  }

  /* Did not find window. */
  return FALSE;
}



/*************************************
*
*  Removes and free ALL window info. 
*  structures from a window info. list.
*                  
**************************************/
void __far __pascal DeleteAllHlpWndInfo( HWNDINFO __far * hFirstWndInfo )
{
  FPWNDINFO fpWndInfo;
  HWNDINFO  hTmpWndInfo;
  
  /* Delete top node until no more. */
  while( *hFirstWndInfo != 0 )
  {
    /* Lock top handle. */
    fpWndInfo = (FPWNDINFO) GlobalLock( *hFirstWndInfo ) ;
    
    /* Next node is to be new top node. */
    hTmpWndInfo = fpWndInfo->Next; 
  
    /* Unlock top handle. */
    GlobalUnlock( *hFirstWndInfo );
      
    /* Free top. */
    FreeWndInfo( *hFirstWndInfo ); 
    
    /* Assign new top. */
    *hFirstWndInfo = hTmpWndInfo;
  }
}


/*************************************
*
*  Get the help window handle for an
*  app.
*                  
**************************************/
HWND __far __pascal GetAppsHelpWnd
( 
  HWNDINFO hFirstWndInfo, 
  HWND hAppWnd 
)
{
  WNDINFO WndInfo;

  /* Initialize private data. */
  WndInfo.hAppWnd = hAppWnd;
  WndInfo.hHelpWnd = 0;
  
  /* Search thru each library. */
  EnumWndInfo( GetAppsHelpWndProc, hFirstWndInfo, (LPARAM) (FPWNDINFO) &WndInfo );

  /* Return found value. */
  return( WndInfo.hHelpWnd );
}


/*************************************
*
*  Returns the number of window info. 
*  structures in the window info. list.
*                  
**************************************/
WORD __far __pascal GetHelpWndCount( HWNDINFO hFirstWndInfo )
{
  WORD wCount = 0;

  /* Count each window. */
  EnumWndInfo( GetHelpWndCountProc, hFirstWndInfo, (LPARAM) (WORD __far *) &wCount );

  /* Return found value. */
  return( wCount );
}



/***********************************  Private Functions  *********************************/


/*************************************
*
*  EnumWndInfo() callback function 
*  used in GetHelpWnd().
*                  
**************************************/
static BOOL CALLBACK GetHelpWndCountProc( FPWNDINFO fpWndInfo, LPARAM lParam )
{
  WORD __far * wCount;
  
  /* Get pointer to the current count. */
  wCount = ( WORD __far * ) lParam;
  
  /* Increment count. */
  *wCount = *wCount + 1;
    
  /* Keep enumerating. */
  return TRUE;
}


/*************************************
*
*  Create a new window info. structure.
*                  
**************************************/
static HWNDINFO __far __pascal NewWndInfo
( 
  HWND hErrorWnd, 
  HWND hAppWnd, 
  HWND hHelpWnd 
)
{
  HWNDINFO hNewWndInfo;
  FPWNDINFO fpNewWndInfo;
  
  
  if( ! GlobalAllocMem( hErrorWnd, &hNewWndInfo, sizeof(WNDINFO) ) ) return NULL;
  
  /* Lock it. */
  fpNewWndInfo = (FPWNDINFO) GlobalLock( hNewWndInfo ) ;

  /* Initialize structure. */
  fpNewWndInfo->Next     = 0;
  fpNewWndInfo->hAppWnd  = hAppWnd;
  fpNewWndInfo->hHelpWnd = hHelpWnd;
  
  /* Unlock it. */
  GlobalUnlock( hNewWndInfo ) ;

  /* Return it. */
  return hNewWndInfo;
}


/*************************************
*
*  Free a window info. structure.
*                  
**************************************/
static void __far __pascal FreeWndInfo( HWNDINFO hWndInfo )
{
  /* 
  ** Performs a simply free for now.
  ** Its not a complex structure. 
  */
  
  /* Free it. */
  GlobalFree( hWndInfo ) ;
}



/*************************************
*
*  Insert a new window info. structure
*  at the top of a window info. list.
*                  
**************************************/
static void __far __pascal InsertWndInfo
( 
  HWNDINFO __far * hFirstWndInfo, 
  HWNDINFO hNewWndInfo 
)
{
  FPWNDINFO fpNewWndInfo;
  
  /* Existing first node. */
  if( *hFirstWndInfo != 0 ) 
  {
    /* Lock it. */
    fpNewWndInfo = (FPWNDINFO) GlobalLock( hNewWndInfo ) ;
  
    /* Point new node to existing top node. */
    fpNewWndInfo->Next = *hFirstWndInfo;

    /* Unlock it. */
    GlobalUnlock( hNewWndInfo ) ;
  }

  /* Assign new node as top. */
  *hFirstWndInfo = hNewWndInfo;
}


/*************************************
*
*  EnumWndInfo() callback function 
*  used in GetHelpWnd().
*                  
**************************************/
static BOOL CALLBACK GetAppsHelpWndProc( FPWNDINFO fpWndInfo, LPARAM lParam )
{
  FPWNDINFO fpSaveWndInfo;
  
  
  /* Get state of window. */
  fpSaveWndInfo = ( FPWNDINFO ) lParam;
  
  /* Show/Hide window. */
  if( fpWndInfo->hAppWnd == fpSaveWndInfo->hAppWnd )
  {
    /* Found correct window - save data. */
    fpSaveWndInfo->hHelpWnd = fpWndInfo->hHelpWnd;    

    /* Stop enumerating. */
    return FALSE;
  }
    
  /* Keep enumerating. */
  return TRUE;
}


/*************************************
*
*  Get the next window info. structure
*  after the given hWndInfo.
*                  
**************************************/
static HWNDINFO __far __pascal GetNextWndInfo( HWNDINFO hWndInfo )
{
  HWNDINFO hNextWndInfo;
  FPWNDINFO fpWndInfo;
  
  /* Handle bad. */
  if( hWndInfo == 0 ) return 0;

  /* Lock it. */
  fpWndInfo = (FPWNDINFO) GlobalLock( hWndInfo ) ;
  
  /* Point new node to existing top node. */
  hNextWndInfo = fpWndInfo->Next;

  /* Unlock it. */
  GlobalUnlock( hWndInfo ) ;

  /* Return next window info's handle. */
  return hNextWndInfo;
}



/*************************************
*
*  Enumerate all WndInfo structures
*  contained in the given list.
*                  
**************************************/
static void __far __pascal EnumWndInfo
( 
  WNDINFOENUMPROC WndInfoEnumProc, 
  HWNDINFO hFirstWndInfo, 
  LPARAM lParam 
)
{
  FPWNDINFO fpWndInfo;
  HWNDINFO hWndInfo;
  
  BOOL bReturn;
  
  /* Get first window info's handle. */
  hWndInfo = hFirstWndInfo;
  
  /* Stop when we run out of windows in the list. */
  while( hWndInfo != 0 )
  {
    /* Lock it. */
    fpWndInfo = (FPWNDINFO) GlobalLock( hWndInfo ) ;
  
    /* Call callback function. */
    bReturn = (*WndInfoEnumProc)( fpWndInfo, lParam );
    
    /* Stop enumerating. */
    if( bReturn == FALSE )
    {
      /* Unlock it. */
      GlobalUnlock( hWndInfo );
      return;
    }
    
    /* Unlock it. */
    GlobalUnlock( hWndInfo ) ;

    hWndInfo = GetNextWndInfo( hWndInfo );
  }
}




