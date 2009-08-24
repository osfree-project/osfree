/***************************************************************************************
**
**
**                              Menu List Functions
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
* Private data to send to the CheckAliasNumber() callback 
* function.  The data structure is sent to the 
* CheckAliasNumber() using the lParam parameter.
*
**************************************/
typedef struct tagPROCESSITEMPRIVATEDATA
{
  WORD wParamAlias;
  BOOL bProcessed;
} 
PROCESSITEMPRIVATEDATA;
typedef PROCESSITEMPRIVATEDATA __far * FPPROCESSITEMPRIVATEDATA;


/*************************************
*
* Private data to send to the CheckAliasNumber() callback 
* function.  The data structure is sent to the 
* CheckAliasNumber() using the lParam parameter.
*
**************************************/
typedef struct tagGETALIASPRIVATEDATA
{
  WORD wParamAlias;
  BOOL Unused;
} 
GETALIASPRIVATEDATA;
typedef GETALIASPRIVATEDATA __far * FPGETALIASPRIVATEDATA;


/*************************************
*
*  Global Variables
*
**************************************/
HTVMENULIST hTVMenuList = 0;
WORD NextAlias = 1;


/*************************************
*
*  Register a TwinView menu ID.
*                  
**************************************/
WORD __far __pascal __export RegisterTVMenuItem( HWND hWnd, WORD wParam )
{
  TVMENUINFO TVMenuInfo;
  
  /* Get unique alias. */
  TVMenuInfo.wParamAlias = GetNextMenuAlias();
  
  /* Alias error - all numbers being used. */
  if( TVMenuInfo.wParamAlias == 0 ) 
  {
    /* Error. */
    return 0;
  }
  
  /* Assign user defined data. */  
  TVMenuInfo.wParam = wParam;
  TVMenuInfo.hWnd = hWnd;

  /* Create a new MENUINFO item in list. */
  if( ! SaveMenuItemInfo( hWnd, (FPTVMENUINFO) &TVMenuInfo, (HTVMENUINFO __far *) &hTVMenuList ) )
  {
    /* Error. */
    return 0;
  }
  
  /* Success. */
  return (TVMenuInfo.wParamAlias);
}


/*************************************
*
*  Unregister a TwinView menu ID.
*                  
**************************************/
void __far __pascal __export UnregisterTVMenuItem( WORD wParamAlias )
{
  DeleteMenuInfoForAlias( (HTVMENUINFO __far *) &hTVMenuList, wParamAlias );
}


/*************************************
*
*  Process an aliased menu ID by
*  sending the correct window the
*  wParam the it asked to be sent.
*                  
**************************************/
BOOL __far __pascal __export ProcessTVMenuItem( WORD wParamAlias )
{
  PROCESSITEMPRIVATEDATA ProcessItemPrivateData;

  ProcessItemPrivateData.wParamAlias = wParamAlias;
  ProcessItemPrivateData.bProcessed = FALSE;

  /* Look for alias in list. */
  EnumMenuInfo( ProcessTVMenuItemEngine, 
                (LPARAM) (FPPROCESSITEMPRIVATEDATA) &ProcessItemPrivateData );

  /* Processed status. */
  return( ProcessItemPrivateData.bProcessed );
}


/*************************************
*
*  Engine for the ProcessTVMenuItem()
*  function.
*
*  If the correct menu alias is found
*  in the TVMENUINFO, the window is sent
*  the wParam that it wanted.
*                  
**************************************/
BOOL CALLBACK ProcessTVMenuItemEngine( FPTVMENUINFO fpMenuInfo, LPARAM lParam )
{
  FPPROCESSITEMPRIVATEDATA fpProcessItemPrivateData;

  /* Cast LPARAM. */
  fpProcessItemPrivateData = ( FPPROCESSITEMPRIVATEDATA ) lParam;
  
  if( fpMenuInfo->wParamAlias == fpProcessItemPrivateData->wParamAlias )
  {
    /* Alias already being used. */
    fpProcessItemPrivateData->bProcessed = TRUE;
  
    /* Send the window the WM_COMMAND and the wParam that it wanted. */
    SendMessage( fpMenuInfo->hWnd, WM_COMMAND, fpMenuInfo->wParam, 0L ); 
    
    /* Stop the enumeration. */
    return FALSE;
  }

  /* Alias still not found in list - get next MenuInfo. */
  return TRUE;
}


/*************************************
*
*  Find an alias number not already in
*  use.
*                  
**************************************/
WORD __far __pascal GetNextMenuAlias( void )
{
  GETALIASPRIVATEDATA GetAliasPrivateData;
  WORD Counter;
  
  /* Use first 500 numbers if we can. */
  if( NextAlias <= MAX_ALIAS )
  {
    NextAlias++;
    return( NextAlias - 1 ); 
  }

  /*
  ** Look for a previously registered, but now unregistered, 
  ** number to recycle.
  */
  
  /* Find a previously registered, but now unregistered, number to recycle. */
  for( Counter = 1; Counter <= MAX_ALIAS; Counter++ )
  {
    /* Assume alias is unused. */   
    GetAliasPrivateData.Unused = TRUE;
  
    /* Assign an alias to look for. */   
    GetAliasPrivateData.wParamAlias = Counter;

    /* Look for alias in list. */
    EnumMenuInfo( AliasNumberUsed, (LPARAM) (FPGETALIASPRIVATEDATA) &GetAliasPrivateData );

    /* If the alias is still marked as Unused after the search. */
    if( GetAliasPrivateData.Unused == TRUE ) return Counter;
  }

  /* Failure - All alias numbers being used - probably a programming error. */
  /* Return error value. */
  return 0;
} 


/*************************************
*
*  Determines if the wParamAlias for the
*  given MENUINFO matches the one
*  that we'd like to use.
*                  
**************************************/
BOOL CALLBACK AliasNumberUsed( FPTVMENUINFO fpMenuInfo, LPARAM lParam )
{
  FPGETALIASPRIVATEDATA fpGetAliasPrivateData;

  /* Cast LPARAM. */
  fpGetAliasPrivateData = ( FPGETALIASPRIVATEDATA ) lParam;
  
  if( fpMenuInfo->wParamAlias == fpGetAliasPrivateData->wParamAlias )
  {
    /* Alias already being used. */
    fpGetAliasPrivateData->Unused = FALSE;
  
    /* Stop the enumeration. */
    return FALSE;
  }

  /* Alias still not found in list - get next MenuInfo. */
  return TRUE;
}



/*************************************
*
*  Enumerate all MenuInfo structures
*  contained in the MenuList.
*                  
**************************************/
void __far __pascal EnumMenuInfo( MENUINFOENUMPROC MenuInfoEnumProc, LPARAM lParam )
{
  FPTVMENUINFO fpMenuInfo;
  HTVMENUINFO hMenuInfo;
  
  BOOL bReturn;
  
  
  /* Get initial menu info's handle. */
  hMenuInfo = (HTVMENUINFO) hTVMenuList;
  
  while( hMenuInfo != 0 )
  {
    /* Lock it. */
    fpMenuInfo = (FPTVMENUINFO) GlobalLock( hMenuInfo ) ;
  
    /* Call callback function. */
    bReturn = (*MenuInfoEnumProc)( fpMenuInfo, lParam );
    
    /* Stop enumerating. */
    if( bReturn == FALSE )
    {
      /* Unlock it. */
      GlobalUnlock( hMenuInfo );

      /* Stop enumerating. */
      return;
    }
    
    /* Unlock it. */
    GlobalUnlock( hMenuInfo );

    hMenuInfo = GetNextMenuInfo( hMenuInfo );
  }
}

/***************************************************************************************
**
**
**                              List Management Functions
**
**
***************************************************************************************/

/*************************************
*
*  A new menu item will be added to
*  twinview's menu.
*  Insert the new menu's info into 
*  the menu list.
*                  
**************************************/
BOOL __far __pascal SaveMenuItemInfo( HWND hErrorWnd, FPTVMENUINFO fpMenuData, HTVMENUINFO __far * hFirstMenuInfo )
{
  HTVMENUINFO hNewMenuInfo;

  
  /* Create new MENUINFO structure. */
  hNewMenuInfo = NewMenuInfo( hErrorWnd, fpMenuData );
  if( hNewMenuInfo == 0 ) return FALSE; 
  
  /*
  ** Insert the TVMENUINFO structure into the Menu List.
  */
  InsertMenuInfo( hFirstMenuInfo, hNewMenuInfo );
  
  /* Return success. */
  return TRUE;
}



/*************************************
*
*  Create a new menu info. structure.
*                  
**************************************/
HTVMENUINFO __far __pascal NewMenuInfo( HWND hErrorWnd, FPTVMENUINFO fpMenuData )
{
  HTVMENUINFO hNewMenuInfo;
  FPTVMENUINFO fpNewMenuInfo;
  
  
  if( ! GlobalAllocMem( hErrorWnd, &hNewMenuInfo, sizeof(TVMENUINFO) ) ) return NULL;
  
  /* Lock it. */
  fpNewMenuInfo = (FPTVMENUINFO) GlobalLock( hNewMenuInfo ) ;

  /* Initialize structure. */
  fpNewMenuInfo->Next = 0;
  fpNewMenuInfo->wParamAlias = fpMenuData->wParamAlias;
  fpNewMenuInfo->wParam = fpMenuData->wParam;
  fpNewMenuInfo->hWnd = fpMenuData->hWnd;

  /* Unlock it. */
  GlobalUnlock( hNewMenuInfo ) ;

  /* Return it. */
  return hNewMenuInfo;
}


/*************************************
*
*  Free a menu info. structure.
*                  
**************************************/
void __far __pascal FreeMenuInfo( HTVMENUINFO hMenuInfo )
{
  /* 
  ** Performs a simply free for now.
  ** Its not a complex structure. 
  */
  
  /* Free it. */
  GlobalFree( hMenuInfo ) ;
}



/*************************************
*
*  Insert a new menu info. structure
*  at the top of a menu info. list.
*                  
**************************************/
void __far __pascal InsertMenuInfo( HTVMENUINFO __far * hFirstMenuInfo, HTVMENUINFO hNewMenuInfo )
{
  FPTVMENUINFO fpNewMenuInfo;
  
  /* Existing first node. */
  if( *hFirstMenuInfo != 0 ) 
  {
    /* Lock it. */
    fpNewMenuInfo = (FPTVMENUINFO) GlobalLock( hNewMenuInfo ) ;
  
    /* Point new node to existing top node. */
    fpNewMenuInfo->Next = *hFirstMenuInfo;

    /* Unlock it. */
    GlobalUnlock( hNewMenuInfo ) ;
  }

  /* Assign new node as top. */
  *hFirstMenuInfo = hNewMenuInfo;
}



/*************************************
*
*  Remove and free a menu info. structure
*  from a menu info. list.
*                  
**************************************/
BOOL __far __pascal DeleteMenuInfoForAlias( HTVMENUINFO __far * hFirstMenuInfo, WORD wParamAlias  )
{
  FPTVMENUINFO fpMenuInfo, fpTmpMenuInfo;
  HTVMENUINFO hMenuInfo, hTmpMenuInfo;
  

  /* Existing first node. */
  if( *hFirstMenuInfo == 0 ) return FALSE;

  /* Lock top handle. */
  fpMenuInfo = (FPTVMENUINFO) GlobalLock( *hFirstMenuInfo ) ;
  
  /* If deleting top node. */
  if( fpMenuInfo->wParamAlias == wParamAlias )
  {
    /* Remove the menu item from menu. */
    DeleteMenu( GetTVMenuHandle(), fpMenuInfo->wParamAlias, MF_BYCOMMAND );
    DrawMenuBar( GetTVWindowHandle());

    /* Next node is to be new top node. */
    hTmpMenuInfo = fpMenuInfo->Next; 

    /* Unlock top handle. */
    GlobalUnlock( *hFirstMenuInfo );
    
    /* Free top. */
    FreeMenuInfo( *hFirstMenuInfo ); 
  
    /* Assign new top. */
    *hFirstMenuInfo = hTmpMenuInfo;

    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( *hFirstMenuInfo );
    
  /* Find correct node and remove. */
  hTmpMenuInfo = *hFirstMenuInfo;
  hMenuInfo = GetNextMenuInfo( *hFirstMenuInfo );

  while( hMenuInfo != 0 )
  {
    /* Lock current node. */
    fpMenuInfo = (FPTVMENUINFO) GlobalLock( hMenuInfo ) ;
  
    /* If deleting current node. */
    if( fpMenuInfo->wParamAlias == wParamAlias )
    {
      /* Remove the menu item from menu. */
      DeleteMenu( GetTVMenuHandle(), fpMenuInfo->wParamAlias, MF_BYCOMMAND );
      DrawMenuBar( GetTVWindowHandle());

      /* Lock previous node. */
      fpTmpMenuInfo = (FPTVMENUINFO) GlobalLock( hTmpMenuInfo ) ;

      /* Previous node's "next node" is deleted node's "next node". */
      fpTmpMenuInfo->Next = fpMenuInfo->Next; 
  
      /* Unlock previous node. */
      GlobalUnlock( hTmpMenuInfo );

      /* Unlock current node. */
      GlobalUnlock( hMenuInfo );
      
      /* Free top. */
      FreeMenuInfo( hMenuInfo ); 
  
      /* Success. */
      return TRUE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hMenuInfo ) ;

    /* Get next node. */
    hTmpMenuInfo = hMenuInfo;
    hMenuInfo = GetNextMenuInfo( hMenuInfo );
  }

  /* Did not find menu. */
  return FALSE;
}


/*************************************
*
*  Unregister all TwinView menus 
*  associated with a given window.
*                  
**************************************/
void __far __pascal DeleteMenuInfoForWnd( HWND hWnd )
{
  FPTVMENUINFO fpMenuInfo, fpLastMenuInfo;
  HTVMENUINFO hMenuInfo, hLastMenuInfo, hTmpMenuInfo;
  HTVMENUINFO __far * hFirstMenuInfo;

  BOOL bFirst = TRUE;
  

  /* Get top of menu list. */
  hFirstMenuInfo = (HTVMENUINFO __far *) &hTVMenuList;

  /* Existing first node. */
  if( *hFirstMenuInfo == 0 ) return;

  /* Point to first node in list. */
  hMenuInfo = *hFirstMenuInfo;
  
  /* 
  ** Process first node if associated 
  ** with the hWnd.
  */

  while( bFirst && hMenuInfo != 0 )
  {
    /* Lock current node. */
    fpMenuInfo = (FPTVMENUINFO) GlobalLock( hMenuInfo ) ;
  
    /* If found the correct window handle. */
    if( fpMenuInfo->hWnd == hWnd )
    {
      /* Remove the menu item from menu. */
      DeleteMenu( GetTVMenuHandle(), fpMenuInfo->wParamAlias, MF_BYCOMMAND );
      DrawMenuBar( GetTVWindowHandle());

      /* Next node is to be new top node. */
      hTmpMenuInfo = fpMenuInfo->Next; 
    
      /* Unlock top handle. */
      GlobalUnlock( hMenuInfo );
        
      /* Free top. */
      FreeMenuInfo( hMenuInfo ); 
      
      /* Assign new top. */
      *hFirstMenuInfo = hMenuInfo = hTmpMenuInfo;
      
    }
    
    /* Thru processing top node. */
    else bFirst = FALSE;
  }

  /* Empty list now? */
  if( hMenuInfo == 0 ) return; 
  

  /* 
  ** Go thru rest of nodes and
  ** delete any associated with the hWnd.
  */

  /* Last node is top node. */
  hLastMenuInfo = hMenuInfo;

  /* Lock top node. */
  fpMenuInfo = (FPTVMENUINFO) GlobalLock( hMenuInfo ) ;
  
  /* Assign Next node to temp. */
  hTmpMenuInfo = fpMenuInfo->Next; 

  /* Unlock top handle. */
  GlobalUnlock( hMenuInfo );
        
  /* New current node. */
  hMenuInfo = hTmpMenuInfo;
  
  
  while( hMenuInfo != 0 )
  {
    /* Lock current node. */
    fpMenuInfo = (FPTVMENUINFO) GlobalLock( hMenuInfo ) ;
  
    /* If found the correct window handle. */
    if( fpMenuInfo->hWnd == hWnd )
    {
      /* Remove the menu item from menu. */
      DeleteMenu( GetTVMenuHandle(), fpMenuInfo->wParamAlias, MF_BYCOMMAND );
      DrawMenuBar( GetTVWindowHandle());

      /* Lock previous node. */
      fpLastMenuInfo = (FPTVMENUINFO) GlobalLock( hLastMenuInfo ) ;
  
      /* Previous node's "next node" is deleted node's "next node". */
      fpLastMenuInfo->Next = fpMenuInfo->Next; 
    
      /* Unlock current node. */
      GlobalUnlock( hMenuInfo );
        
      /* Free node. */
      FreeMenuInfo( hMenuInfo ); 

      /* Next node now current node. */
      hMenuInfo = fpLastMenuInfo->Next; 
        
      /* Unlock previous node. */
      GlobalUnlock( hLastMenuInfo );
  
    }
    
    /* Not the correct window. */
    else
    {
      /* Unlock it. */
      GlobalUnlock( hMenuInfo ) ;

      /* Get next node. */
      hLastMenuInfo = hMenuInfo;
      hMenuInfo = GetNextMenuInfo( hMenuInfo );
    }
  }
}


/*************************************
*
*  Unregister all TwinView menus. 
*                  
**************************************/
void __far __pascal __export DeleteMenuInfoList( void )
{
  FPTVMENUINFO fpMenuInfo;
  HTVMENUINFO  hTmpMenuInfo;
  HTVMENUINFO __far * hFirstMenuInfo;


  /* Get top of menu list. */
  hFirstMenuInfo = (HTVMENUINFO __far *) &hTVMenuList;

  while( *hFirstMenuInfo != 0 )
  {
    /* Lock current node. */
    fpMenuInfo = (FPTVMENUINFO) GlobalLock( *hFirstMenuInfo ) ;
  
    /* Next node is to be new top node. */
    hTmpMenuInfo = fpMenuInfo->Next; 
    
    /* Unlock top handle. */
    GlobalUnlock( *hFirstMenuInfo );
        
    /* Free top. */
    FreeMenuInfo( *hFirstMenuInfo ); 

    /* Assign new top. */
    *hFirstMenuInfo = hTmpMenuInfo; 
  }
}


/*************************************
*
*  Get the next menu info. structure
*  after the given hMenuInfo.
*                  
**************************************/
HTVMENUINFO __far __pascal GetNextMenuInfo( HTVMENUINFO hMenuInfo )
{
  HTVMENUINFO hNextMenuInfo;
  FPTVMENUINFO fpMenuInfo;
  
  /* Handle bad. */
  if( hMenuInfo == 0 ) return 0;

  /* Lock it. */
  fpMenuInfo = (FPTVMENUINFO) GlobalLock( hMenuInfo ) ;
  
  /* Point new node to existing top node. */
  hNextMenuInfo = fpMenuInfo->Next;

  /* Unlock it. */
  GlobalUnlock( hMenuInfo ) ;

  /* Return next menu info's handle. */
  return hNextMenuInfo;
}


