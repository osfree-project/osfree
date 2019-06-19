/***************************************************************************************
**
**
**                              Window Info Functions
**
**
***************************************************************************************/

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
#include "tvlibprv.h"




/*************************************
*
* Private data sent to the IsTVWindowWndEngine() 
* function by the IsTVWindow() function.
* The data structure is sent via the lParam parameter.
*
**************************************/
typedef struct tagISTVWNDPRIVATEDATA
{
  HWND hWnd;
  BOOL bFound;
} 
ISTVWNDPRIVATEDATA;
typedef ISTVWNDPRIVATEDATA __far * FPISTVWNDPRIVATEDATA;


/*************************************
*
*  Verifies if the window is a TwinView
*  child window or not.
*
*  Returns:
*
*  TRUE:  Is a twinview window.
*  FALSE: Is not a twinview window.
*
*
**************************************/
BOOL __far __pascal __export IsTVWindow( HWND hWnd )
{
  ISTVWNDPRIVATEDATA PrivateData;

  /* Initialize private data. */
  PrivateData.hWnd = hWnd;
  PrivateData.bFound = FALSE;
  
  /* Search thru each library. */
  EnumWndInfo( IsTVWindowEngine, (LPARAM) (FPISTVWNDPRIVATEDATA) &PrivateData );

  /* Return found value. */
  return( PrivateData.bFound );
}


BOOL CALLBACK IsTVWindowEngine( FPTVWNDINFO fpWndInfo, LPARAM lParam )
{
  FPISTVWNDPRIVATEDATA fpPrivateData;
  
  
  /* Get state of window. */
  fpPrivateData = ( FPISTVWNDPRIVATEDATA ) lParam;
  
  /* Show/Hide window. */
  if( FindWndInfo( (HTVWNDINFO) GlobalHandle(SELECTOROF(fpWndInfo)), fpPrivateData->hWnd ) )
  {
    /* Mark it found. */
    fpPrivateData->bFound = TRUE;    

    /* Stop enumerating. */
    return FALSE;
  }
    
  /* Keep enumerating. */
  return TRUE;
}




/*************************************
*
*  Sends the nCmdShow to all TwinView 
*  windows via the windows ShowWindow()
*  function.
*
*  e.g. SW_SHOW, SW_HIDE, etc.
*
**************************************/
void __far __pascal __export ShowTVWindows( int nCmdShow)
{
  EnumWndInfo( HideShowAllWndEngine, (LPARAM) (int __far *) &nCmdShow );
}


BOOL CALLBACK HideShowAllWndEngine( FPTVWNDINFO fpWndInfo, LPARAM lParam )
{
  int __far * nCmdShow;
  
  /* Get state of window. */
  nCmdShow = ( int __far * ) lParam;
  
  /* Show/Hide window. */
  ShowWindow( fpWndInfo->hWnd, *nCmdShow ); 
  
  /* Keep enumerating. */
  return TRUE;
}





/*************************************
*
*  Attempts to close all windows opened
*  in each library.
*
*  Enumerates libraries to the function
*  CloseAllTVWindowsEngine().
*
**************************************/
BOOL __far __pascal CloseAllTVWindows( void )
{
  BOOL bAllWndClosed = TRUE;
  
  /* Process each library in the library list. */
  EnumTVLibInfo( CloseAllLibsWindows, (LPARAM) (BOOL __far *) &bAllWndClosed );

  return bAllWndClosed; 
}


/*************************************
*  
*  Engine for the CloseAllTVWindows()
*  function.   
*
*  Receives the each library's info.
*
*  Attempts to close window,
*  close library, and free associated
*  menu items for window.
*
**************************************/
BOOL __far __pascal CloseAllLibsWindows( FPTVLIBINFO fpTVLibInfo, LPARAM lParam )
{
  BOOL __far * bAllWndClosed;
  FPTVWNDINFO fpWndInfo;
  HTVWNDINFO hWndInfo;

  HWND hChildWnd;  
  HWND hChildsPopupWnd;  
  
  TVLIBCLOSEWINDOWPROC fpfnTVLibCloseWnd;


  /* Get LPARAM data. */
  bAllWndClosed = (BOOL __far *) lParam;
  
  /* Get library's handle to first window info. */
  hWndInfo = GetLibWndInfo( fpTVLibInfo );
  
  while( hWndInfo != 0 )
  {
    /* Lock it. */
    fpWndInfo = (FPTVWNDINFO) GlobalLock( hWndInfo ) ;
  
    /* Get the handle of the window we are closing. */
    hChildWnd = fpWndInfo->hWnd;

    /* Unlock it. */
    GlobalUnlock( hWndInfo ) ;

    /* 
    ** If we are already in the window's
    ** Wndproc() and awaiting it return,
    ** activate the last popup window.
    */
    if( GetTVEntryCount( hChildWnd ) != 0 )
    {
      /* Change focus to window. */
      MessageBeep( MB_ICONEXCLAMATION );
      hChildsPopupWnd = GetLastActivePopup( hChildWnd );
      SetActiveWindow( hChildsPopupWnd ); 

      /* Not all windows closed. */
      *bAllWndClosed = FALSE; 

      /* Stop enumerating. */
      return FALSE;
    }

    /* 
    **  Tell window to close by sending calling
    **  the TVLibWindowClose() function.
    */   
    /* Retrieve the address of the control's LibInfo() function. */
    (FARPROC) fpfnTVLibCloseWnd = GetProcAddress( fpTVLibInfo->hInstance, MAKEINTRESOURCE(TVLIB_CLOSEWINDOW_ORD) );
                                              
    /* Verify the GetProcAddress() function returned data. */
    if ( fpfnTVLibCloseWnd == NULL)
    {
      /* Not all windows closed. */
      *bAllWndClosed = FALSE; 

      /* Error message. */
      MessageBox(  NULL, (LPSTR)"GetProcAddress() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );

      /* Stop enumerating. */
      return FALSE;
    }
  
    /* 
    **  Restore the window's WndProc() to its original
    **  WndProc() so we aren't trapping for the
    **  WM_NCDESTROY message and won't therefore call
    **  PostCloseLibrary().
    */
//    RemoveChildWindowFilter( hChildWnd );

    /* Tell the library to close the window. */
    (*fpfnTVLibCloseWnd)(hChildWnd);

    /* 
    **  Window is still around. 
    */
    if( IsWindow( hChildWnd ) )
    {
      /* 
      **  Set the window's WndProc() to our special
      **  WndProc() so we can keep trapping for the
      **  WM_NCDESTROY message to call
      **  PostCloseLibrary().
      */
//      SetChildWindowFilter( hChildWnd );

      /* Not all windows closed. */
      *bAllWndClosed = FALSE; 
      
      /* Stop enumerating. */
      return FALSE;
    }
  
    /* 
    **  Window is not still around. 
    */

    /* Remove menu alias' associated with the window. */  
    DeleteMenuInfoForWnd( hChildWnd );
    
    /* Find and delete the window info. */
    DeleteWndInfo( &(fpTVLibInfo)->hTVWndInfo, hChildWnd );
  
    /* Close the library. */
    CloseLibrary( fpTVLibInfo );

    /* Get library's handle to first window info. */
    hWndInfo = GetLibWndInfo( fpTVLibInfo );
  }

  /* Keep enumerating - next library. */
  return TRUE;
}



/***********************************************************************************************/

/*************************************
*
*  A library's window has been opened.
*  Insert the new window's info into 
*  a library's window info. list.
*                  
**************************************/
BOOL __far __pascal SaveWndInfo( HWND hErrorWnd, HWND hWndOpen, HTVWNDINFO __far * hFirstWndInfo )
{
  HTVWNDINFO hNewWndInfo;

  
  /* Create new WNDINFO structure. */
  hNewWndInfo = NewWndInfo( hErrorWnd, hWndOpen );
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
HTVWNDINFO __far __pascal NewWndInfo( HWND hErrorWnd, HWND hWndOpen )
{
  HTVWNDINFO hNewWndInfo;
  FPTVWNDINFO fpNewWndInfo;
  
  
  if( ! GlobalAllocMem( hErrorWnd, &hNewWndInfo, sizeof(TVWNDINFO) ) ) return NULL;
  
  /* Lock it. */
  fpNewWndInfo = (FPTVWNDINFO) GlobalLock( hNewWndInfo ) ;

  /* Initialize structure. */
  fpNewWndInfo->Next = 0;
  fpNewWndInfo->hWnd   = hWndOpen;
  
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
void __far __pascal FreeWndInfo( HTVWNDINFO hWndInfo )
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
void __far __pascal InsertWndInfo( HTVWNDINFO __far * hFirstWndInfo, HTVWNDINFO hNewWndInfo )
{
  FPTVWNDINFO fpNewWndInfo;
  
  /* Existing first node. */
  if( *hFirstWndInfo != 0 ) 
  {
    /* Lock it. */
    fpNewWndInfo = (FPTVWNDINFO) GlobalLock( hNewWndInfo ) ;
  
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
*  Find a window info. structure
*  in a window info. list.
*                  
**************************************/
BOOL __far __pascal FindWndInfo( HTVWNDINFO hFirstWndInfo, HWND hWnd )
{
  FPTVWNDINFO fpWndInfo;
  HTVWNDINFO hWndInfo, hLastWndInfo;
  

  /* Existing first node. */
  if( hFirstWndInfo == 0 ) return FALSE;

  /* Lock top handle. */
  fpWndInfo = (FPTVWNDINFO) GlobalLock( hFirstWndInfo ) ;
  
  /* If deleting top node. */
  if( fpWndInfo->hWnd == hWnd )
  {
    /* Unlock top handle. */
    GlobalUnlock( hFirstWndInfo );
    
    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( hFirstWndInfo );
    
  /* Find correct node and remove. */
  hLastWndInfo = hFirstWndInfo;
  hWndInfo = GetNextWndInfo( hFirstWndInfo );

  while( hWndInfo != 0 )
  {
    /* Lock current node. */
    fpWndInfo = (FPTVWNDINFO) GlobalLock( hWndInfo ) ;
  
    /* If deleting current node. */
    if( fpWndInfo->hWnd == hWnd )
    {
      /* Unlock current node. */
      GlobalUnlock( hWndInfo );
      
      /* Success. */
      return TRUE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hWndInfo ) ;

    /* Get next node. */
    hLastWndInfo = hWndInfo;
    hWndInfo = GetNextWndInfo( hWndInfo );
  }

  /* Did not find window. */
  return FALSE;
}



/*************************************
*
*  Remove and free a window info. structure
*  from a window info. list.
*                  
**************************************/
BOOL __far __pascal DeleteWndInfo( HTVWNDINFO __far * hFirstWndInfo, HWND hWnd )
{
  FPTVWNDINFO fpWndInfo, fpTmpWndInfo;
  HTVWNDINFO hWndInfo, hTmpWndInfo;
  

  /* Existing first node. */
  if( *hFirstWndInfo == 0 ) return FALSE;

  /* Lock top handle. */
  fpWndInfo = (FPTVWNDINFO) GlobalLock( *hFirstWndInfo ) ;
  
  /* If deleting top node. */
  if( fpWndInfo->hWnd == hWnd )
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
    fpWndInfo = (FPTVWNDINFO) GlobalLock( hWndInfo ) ;
  
    /* If deleting current node. */
    if( fpWndInfo->hWnd == hWnd )
    {
      /* Lock previous node. */
      fpTmpWndInfo = (FPTVWNDINFO) GlobalLock( hTmpWndInfo ) ;

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
void __far __pascal DeleteAllWndInfo( HTVWNDINFO __far * hFirstWndInfo )
{
  FPTVWNDINFO fpWndInfo;
  HTVWNDINFO  hTmpWndInfo;
  
  /* Delete top node until no more. */
  while( *hFirstWndInfo != 0 )
  {
    /* Lock top handle. */
    fpWndInfo = (FPTVWNDINFO) GlobalLock( *hFirstWndInfo ) ;
    
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
HTVWNDINFO __far __pascal GetNextWndInfo( HTVWNDINFO hWndInfo )
{
  HTVWNDINFO hNextWndInfo;
  FPTVWNDINFO fpWndInfo;
  
  /* Handle bad. */
  if( hWndInfo == 0 ) return 0;

  /* Lock it. */
  fpWndInfo = (FPTVWNDINFO) GlobalLock( hWndInfo ) ;
  
  /* Point new node to existing top node. */
  hNextWndInfo = fpWndInfo->Next;

  /* Unlock it. */
  GlobalUnlock( hWndInfo ) ;

  /* Return next window info's handle. */
  return hNextWndInfo;
}



/*************************************
*
* Private data sent to the EnumWndInfoEngine() 
* function by the EnumWndInfo() function.
* The data structure is sent via the lParam parameter.
*
**************************************/
typedef struct tagENUMWNDPRIVATEDATA
{
  WNDINFOENUMPROC WndInfoEnumProc;
  LPARAM lParam;
} 
ENUMWNDPRIVATEDATA;
typedef ENUMWNDPRIVATEDATA __far * FPENUMWNDPRIVATEDATA;



/*************************************
*
*  Enumerate all WndInfo structures
*  contained in every library in the
*  TVLIBLIST.
*                  
**************************************/
void __far __pascal EnumWndInfo( WNDINFOENUMPROC WndInfoEnumProc, LPARAM lParam )
{
  ENUMWNDPRIVATEDATA EnumWndPrivateData;
    
  /* Assign private data. */
  EnumWndPrivateData.WndInfoEnumProc = WndInfoEnumProc;
  EnumWndPrivateData.lParam = lParam;
  
  /* Try to process the fpTVOpenWndStruct->szFileName as if it were a library to open. */
  EnumTVLibInfo( EnumWndInfoEngine, (LPARAM) (FPENUMWNDPRIVATEDATA) &EnumWndPrivateData );
}


/*************************************
*
*  Engine for the EnumWndInfo() function.
*
*  Enumerate all WndInfo structures
*  contained in the given library.
*                  
**************************************/
BOOL __far __pascal EnumWndInfoEngine( FPTVLIBINFO fpTVLibInfo, LPARAM lParam )
{
  FPENUMWNDPRIVATEDATA fpEnumWndPrivateData;
  
  FPTVWNDINFO fpWndInfo;
  HTVWNDINFO hWndInfo;
  
  BOOL bReturn;
  
  WNDINFOENUMPROC WndInfoEnumProc;

  
  /* Get LPARAM data. */
  fpEnumWndPrivateData = (FPENUMWNDPRIVATEDATA) lParam;
  
  /* Get initial window info's handle. */
  hWndInfo = GetLibWndInfo( fpTVLibInfo );
  
  while( hWndInfo != 0 )
  {
    /* Lock it. */
    fpWndInfo = (FPTVWNDINFO) GlobalLock( hWndInfo ) ;
  
    /* Call callback function. */
    WndInfoEnumProc = fpEnumWndPrivateData->WndInfoEnumProc;
    bReturn = (*WndInfoEnumProc)( fpWndInfo, fpEnumWndPrivateData->lParam );
    
    /* Stop enumerating. */
    if( bReturn == FALSE )
    {
      /* Unlock it. */
      GlobalUnlock( hWndInfo );

      return FALSE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hWndInfo ) ;

    hWndInfo = GetNextWndInfo( hWndInfo );
  }

  /* Keep enumerating - next library. */
  return TRUE;
}
