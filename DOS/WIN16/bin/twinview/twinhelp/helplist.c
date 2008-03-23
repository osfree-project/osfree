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
#include "helplist.h"



/*************************************
*
*  Private Defines
*
**************************************/
typedef BOOL ( CALLBACK * WNDINFOENUMPROC )( FPWNDINFO fpWndInfo, LPARAM lParam );



/*************************************
*
*  Private Function Prototypes
*
**************************************/
HWNDINFO __far __pascal NewWndInfo( HWND hErrorWnd, HWND hAppWnd, HWND hHelpWnd );

void __far __pascal FreeWndInfo( HWNDINFO hWndInfo );

void __far __pascal InsertWndInfo( HWNDINFO __far * hFirstWndInfo, HWNDINFO hNewWndInfo );

void __far __pascal EnumWndInfo( WNDINFOENUMPROC WndInfoEnumProc, HWNDINFO hFirstWndInfo, LPARAM lParam );

BOOL CALLBACK GetAppWndInfo( FPWNDINFO fpWndInfo, LPARAM lParam );

HWNDINFO __far __pascal GetNextWndInfo( HWNDINFO hWndInfo );

BOOL __far __pascal GlobalAllocMem( HWND hWnd, GLOBALHANDLE __far * hGlobal, DWORD dwSize );



/*************************************
*
*  A help window has been opened.
*  Insert the new window's info into 
*  a help window info. list.
*                  
**************************************/
BOOL __far __pascal SaveWndInfo
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
*  Create a new window info. structure.
*                  
**************************************/
HWNDINFO __far __pascal NewWndInfo
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
void __far __pascal FreeWndInfo( HWNDINFO hWndInfo )
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
void __far __pascal InsertWndInfo
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
  EnumWndInfo( GetAppWndInfo, hFirstWndInfo, (LPARAM) (FPWNDINFO) &WndInfo );

  /* Return found value. */
  return( WndInfo.hHelpWnd );
}


/*************************************
*
*  EnumWndInfo() callback function 
*  used in GetHelpWnd().
*                  
**************************************/
BOOL CALLBACK GetAppWndInfo( FPWNDINFO fpWndInfo, LPARAM lParam )
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
*  Remove and free a window info. structure
*  from a window info. list.
*                  
**************************************/
BOOL __far __pascal DeleteAppWndInfo
( 
  HWNDINFO __far * hFirstWndInfo, 
  HWND hAppWnd 
)
{
  FPWNDINFO fpWndInfo, fpTmpWndInfo;
  HWNDINFO hWndInfo, hTmpWndInfo;
  

  /* Existing first node. */
  if( *hFirstWndInfo == 0 ) return FALSE;

  /* Lock top handle. */
  fpWndInfo = (FPWNDINFO) GlobalLock( *hFirstWndInfo ) ;
  
  /* If deleting top node. */
  if( fpWndInfo->hAppWnd == hAppWnd )
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
    if( fpWndInfo->hAppWnd == hAppWnd )
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
void __far __pascal DeleteAllWndInfo( HWNDINFO __far * hFirstWndInfo )
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
*  Get the next window info. structure
*  after the given hWndInfo.
*                  
**************************************/
HWNDINFO __far __pascal GetNextWndInfo( HWNDINFO hWndInfo )
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
void __far __pascal EnumWndInfo
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



/*************************************
*
*  Allocate global memory of a certain size and trap error.
*
**************************************/
BOOL __far __pascal GlobalAllocMem( HWND hWnd, GLOBALHANDLE __far * hGlobal, DWORD dwSize )
{
  /* Allocate memory for menu list. */
  *hGlobal = GlobalAlloc ( LMEM_MOVEABLE | LMEM_ZEROINIT, dwSize ) ;
                              
  /* Error retrieving memory. */
  if ( *hGlobal == NULL ) 
  {
    MessageBox( hWnd, (LPSTR)"GlobalAlloc() failed!", (LPSTR)"Memory Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Return it. */
  return TRUE;
}

