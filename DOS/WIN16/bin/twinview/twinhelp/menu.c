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
/***********************************
**
**  System Includes
**
***********************************/

#include <string.h>


/***********************************
**
**  App. Includes
**
***********************************/

#include "menu.h"
#include "winmem.h"
#include "globals.h"
#include "msgbox.h"
#include "twhlprc.h"
#include "wnddata.h"
#include "hlpmacro.h"


/***********************************
**
**  Private File Defines
**
***********************************/


/*
** Information for an menu in the list.
*/
typedef HGLOBAL HMENUINFO;
typedef struct tagMENUINFO
{
  ATOM      MenuID;       /* Menu ID string converted to an ATOM. Atom used for the 
                             WM_COMMAND ID.*/
  HMENU     hMenu;        /* If the item is a popup menu, this is its menu handle. */
  HMENU     hParentMenu;  /* Menu on which the item resides. */
  HGLOBAL   hMacroString; /* Macro to run. */
  HMENUINFO Next;         /* Next MENUINFO structure. */
} 
MENUINFO;
typedef MENUINFO __far * FPMENUINFO;


/*
** Information for the menu system.
*/
typedef HMENUINFO HMENULIST;
typedef struct tagMENUSYS
{
  HWND      hDataWnd;        /* Main data window. */
  HMENU     hMenuBar;        /* Handle to help window's menu bar. */
  HMENULIST hMenuList;       /* Top of the menu list. */
} 
MENUSYS;
typedef MENUSYS __far * FPMENUSYS;


/* 
** EnumMenuInfo() callback function. 
*/
typedef BOOL ( CALLBACK * MENUINFOENUMPROC )( FPMENUINFO fpMenuInfo, LPARAM lParam );


/* 
** Data used by the MenuSelected() function. 
*/
typedef struct tagMENUSELFUNCDATA
{
  BOOL bProcessed;
  HWND hDataWnd;
  ATOM MenuID;
}
MENUSELFUNCDATA;
typedef MENUSELFUNCDATA __far * FPMENUSELFUNCDATA;


/* 
** Data used by the GetMenuInfo() function. 
*/
typedef struct tagGETMENUINFOFUNCDATA
{
  BOOL       bFound;
  ATOM       ButtonID;
  FPMENUINFO MenuInfoPtr;
}
GETMENUINFOFUNCDATA;
typedef GETMENUINFOFUNCDATA __far * FPGETWINDOWFUNCDATA;




/***********************************
**
**  Private Function Prototypes
**
***********************************/


/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/

                                                                 
/***********************************
**
**  Starts menu support system. 
**
**  Returns:
**
**     Success - TRUE.
**     Failure - FALSE.
**
***********************************/
BOOL __far __pascal StartMenuSupport( HWND hDataWnd )
{
  HMENUSYS  hMenuSys;
  FPMENUSYS MenuSysPtr;
  
  /* Allocate memory for menu support data. */
  if( !GlobalAllocMem( hDataWnd, &hMenuSys, sizeof(MENUSYS) ) )
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTSTARTMENUSYS, MB_ICONHAND | MB_OK );

    /* Set the menu sys. data to NULL. */
    WndData( hDataWnd, WDM_SETMENUSYS, (DWORD) NULL, NULL );

    /* Failure. */
    return( FALSE );
  }
  
  /* Lock data. */
  MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );
      
  /* Init. structure. */
  MenuSysPtr->hDataWnd = hDataWnd;
  MenuSysPtr->hMenuBar = 0;
  MenuSysPtr->hMenuList = 0;

  /* Unlock data. */
  GlobalUnlock( hMenuSys );

    /* Set the menu sys. data. */
  WndData( hDataWnd, WDM_SETMENUSYS, (DWORD) hMenuSys, NULL );
            
  /* Success. */
  return( TRUE );  
}


/***********************************
**
**  Stops menu support system. 
**
**  Returns: None
**
***********************************/
void __far __pascal StopMenuSupport( HWND hDataWnd )
{
  HMENUSYS  hMenuSys;
  FPMENUSYS MenuSysPtr;
            
  /* Get the menu sys. data */
  hMenuSys = (HMENUSYS) WndData( hDataWnd, WDM_GETMENUSYS, 0, NULL );
  
  /* If we have a menu support. */
  if( hMenuSys != NULL )
  {
    /* Lock menu support data. */
    MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );
        
    /* Delete the main menu and all of its submenus. */
    if( MenuSysPtr->hMenuBar != NULL )
    {
      MyDestroyMenu( &(MenuSysPtr)->hMenuList, hMenuBar );
    }
  
    /* Free menu support data. */
    GlobalUnlock( hMenuSys );
    GlobalFree( hMenuSys );
  }
}


/*********************************************************************
**
**  Runs macro for the given menu command.
**
**  Returns:
**
**      TRUE: Processed key.
**     FALSE: Not Processed key.
**
**********************************************************************/
BOOL __far __pascal MenuSelected( HWND hDataWnd, WORD MenuID )
{
  MENUSELFUNCDATA MenuSelData;
  HMENULIST hMenuList;
  HMENUSYS hMenuSys;
  FPMENUSYS MenuSysPtr;
  
  /* Get the menu sys. data */
  hMenuSys = (HMENUSYS) WndData( hDataWnd, WDM_GETMENUSYS, 0, NULL );
  
  /* If we have a menu support. */
  if( hMenuSys != NULL )
  {
    /* Lock data. */
    MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );
    
    /* Get handle to the menu list. */    
    hMenuList =  MenuSysPtr->hMenuList;

    /* Unlock data. */
    GlobalUnlock( hMenuSys );
    
    /* Init. data struct. */
    MenuSelData.bProcessed  = FALSE;
    MenuSelData.hDataWnd    = MenuSysPtr->hDataWnd;
    MenuSelData.MenuID      = MenuID;
        
    /* Find menu item and run macro. */
    EnumMenuInfo( hMenuList, (MENUINFOENUMPROC) MenuSelectedProc, (LPARAM) (FPMENUSELFUNCDATA) &MenuSelData );
  }

  /* Processed? */
  return( MenuSelData.bProcessed );
}



/*********************************************************************
**
**  Adds a new popup menu to the list and to the menu bar. 
**
**  Position is the position of the new menu.  If position is -1,
**  the menu is inserted as the last item.
**
**********************************************************************/
BOOL __far __pascal MyCreatePopup
( 
  HWND hDataWnd, 
  LPSTR lpMenuID,
  LPSTR lpMenuText,
  short int Position
)
{
  HMENUSYS hMenuSys;
  FPMENUSYS MenuSysPtr;

  /* Menu info. to store in menu info. list. */
  MENUINFO MenuInfo;
  
  BOOL bReturn;
  
  BOOL bCreatedBar;
  
  
  /* Get the menu sys. data */
  hMenuSys = (HMENUSYS) WndData( hDataWnd, WDM_GETMENUSYS, 0, NULL );
  
  /* Lock menu sys. data. */
  MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );
    
  /* Verify that the new menu's ID string is unique. */
  if( GetMenuInfo( MenuSysPtr->hMenuList, lpMenuID, &MenuInfo ) )
  {
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, MENUALREADYEXISTS, MB_ICONHAND | MB_OK );

    /* Success. */
    return( FALSE );
  }


  /* If there is no menu bar the create one and set it. */
  if( MenuSysPtr->hMenuBar == NULL ) 
  {
    /* Create the menu bar. */
    MenuSysPtr->hMenuBar = CreateMenu();
    bCreatedBar = TRUE;

    /* Error. */
    if( MenuSysPtr->hMenuBar == NULL ) 
    {
      /* Unlock menu sys. data. */
      GlobalUnlock( hMenuSys );
      
      /* Error. */ 
      MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTCREATEMENU, MB_ICONHAND | MB_OK );
  
      /* Success. */
      return( FALSE );
    }

    SetMenu( hDataWnd, MenuSysPtr->hMenuBar );
  }
  else bCreatedBar = FALSE;


  /* Create the popup menu. */
  MenuInfo.hMenu = CreatePopupMenu();
  if( MenuInfo.hMenu == NULL ) 
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTCREATEMENU, MB_ICONHAND | MB_OK );

    /* If just created the bar. */
    if( bCreatedBar == TRUE )
    {
      /* Remove the bar. */
      SetMenu( hDataWnd, NULL );
      DestroyMenu( MenuSysPtr->hMenuBar );
      MenuSysPtr->hMenuBar = NULL;
    }
    
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Success. */
    return( FALSE );
  }


  /* Add the menu to the menu bar. */
  if( ! InsertMenu( MenuSysPtr->hMenuBar, Position, MF_BYPOSITION | MF_ENABLED | MF_STRING | MF_POPUP, (UINT) MenuInfo.hMenu, lpMenuText) )
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTCREATEMENU, MB_ICONHAND | MB_OK );

    /* If just created the bar. */
    if( bCreatedBar == TRUE )
    {
      /* Remove the bar. */
      SetMenu( hDataWnd, NULL );
      DestroyMenu( MenuSysPtr->hMenuBar );
      MenuSysPtr->hMenuBar = NULL;
    }
    
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Success. */
    return( FALSE );
  }
  
  /* Init. the other menu info. */
  MenuInfo.MenuID       = AddAtom( lpMenuID );   
  MenuInfo.hParentMenu  = MenuSysPtr->hMenuBar;   
  MenuInfo.hMacroString = NULL;   

  /* Save the menu info. */
  bReturn = SaveMenuInfo( hDataWnd, &(MenuSysPtr)->hMenuList, (FPMENUINFO) &MenuInfo );
  if( bReturn == FALSE )
  {
    /* Remove the new menu from the menu bar. */
    RemoveMenu( MenuSysPtr->hMenuBar, Position, MF_BYPOSITION );
    DestroyMenu( MenuInfo.hMenu );
    
    /* If just created the bar. */
    if( bCreatedBar == TRUE )
    {
      /* Remove the bar. */
      SetMenu( hDataWnd, NULL );
      DestroyMenu( MenuSysPtr->hMenuBar );
      MenuSysPtr->hMenuBar = NULL;
    }
    
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Failure. */
    return FALSE;
  }
      
  /* Show changes. */
  DrawMenuBar( MenuSysPtr->hMenuBar );

  /* Unlock menu sys. data. */
  GlobalUnlock( hMenuSys );
    
  /* Success. */
  return( TRUE );
}



/*********************************************************************
**
**  Adds a new menu item to a menu. 
**
**  Position is the position of the new menu item.  If position is -1,
**  the menu item is inserted as the last item.
**
**********************************************************************/
BOOL __far __pascal MyInsertMenu
( 
  HWND hDataWnd, 
  LPSTR lpMenuID,
  LPSTR lpItemID,
  LPSTR lpItemText,
  LPSTR lpMacro,
  short int Position
)
{
  HMENUSYS hMenuSys;
  FPMENUSYS MenuSysPtr;

  /* Menu info. to store in menu info. list. */
  MENUINFO MenuInfo, MenuItemInfo;
  
  BOOL bReturn;
  
  /* Pointer to macro string. */
  char __far * MacroStringPtr;

  
  /* Get the menu sys. data */
  hMenuSys = (HMENUSYS) WndData( hDataWnd, WDM_GETMENUSYS, 0, NULL );
  
  /* Lock menu sys. data. */
  MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );
    
  /* Menubar does not even exist. */
  if( MenuSysPtr->hMenuBar == NULL ) 
  {
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, MENUDOESNOTEXIST, MB_ICONHAND | MB_OK );

    /* Success. */
    return( FALSE );
  }

  /* Find the menu that the item will be added to. */
  if( GetMenuInfo( MenuSysPtr->hMenuList, lpMenuID, &MenuInfo ) )
  {
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, MENUDOESNOTEXIST, MB_ICONHAND | MB_OK );

    /* Success. */
    return( FALSE );
  }


  /* Verify that the new menu's ID string is unique. */
  if( GetMenuInfo( MenuSysPtr->hMenuList, lpItemID, &MenuItemInfo ) )
  {
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, MENUALREADYEXISTS, MB_ICONHAND | MB_OK );

    /* Success. */
    return( FALSE );
  }


  /* Allocate memory for storage of macro string. */
  if( !GlobalAllocMem( hDataWnd, &(MenuItemInfo).hMacroString, _fstrlen( lpMacro ) + sizeof(char) ) )
  {
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTCREATEMENU, MB_ICONHAND | MB_OK );

    /* Failure. */
    return( FALSE );
  }
  
  /* Copy macro string storage area. */
  MacroStringPtr = (char __far *) GlobalLock( MenuItemInfo.hMacroString );
  _fstrcpy( MacroStringPtr, lpMacro );
  GlobalUnlock( MenuItemInfo.hMacroString );

  /* Init. the other menu item info. */
  MenuItemInfo.MenuID       = AddAtom( lpItemID );   
  MenuItemInfo.hMenu        = NULL;   
  MenuItemInfo.hParentMenu  = MenuInfo->hMenu;   
  MenuItemInfo.hMacroString = NULL;   

  /* Add the menu item to the menu. */
  if( ! InsertMenu( MenuInfo->hMenu, Position, MF_BYPOSITION | MF_ENABLED | MF_STRING, MenuItemInfo.MenuID, lpItemText) )
  {
    /* Delete allocated info. */
    DeleteAtom( MenuItemInfo.ButtonID );
    GlobalFree( MenuItemInfo.hMacroString ); 

    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTCREATEMENU, MB_ICONHAND | MB_OK );

    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Success. */
    return( FALSE );
  }
  
  /* Save the menu info. */
  bReturn = SaveMenuInfo( hDataWnd, &(MenuSysPtr)->hMenuList, (FPMENUINFO) &MenuInfo );
  if( bReturn == FALSE )
  {
    /* Delete allocated info. */
    DeleteAtom( MenuItemInfo.ButtonID );
    GlobalFree( MenuItemInfo.hMacroString ); 

    /* Remove the new menu item. */
    RemoveMenu( MenuInfo->hMenu, Position, MF_BYPOSITION );
    
    /* Unlock menu sys. data. */
    GlobalUnlock( hMenuSys );
    
    /* Failure. */
    return FALSE;
  }
      
  /* Show changes. */
  DrawMenuBar( MenuSysPtr->hMenuBar );

  /* Unlock menu sys. data. */
  GlobalUnlock( hMenuSys );
    
  /* Success. */
  return( TRUE );
}


/*********************************************************************
**
**  Delete an menu to the list.
**
**********************************************************************/
BOOL __far __pascal MyDestroyMenu
( 
  HWND hDataWnd, 
  WORD wKey, 
  WORD wShiftState
)
{
  HMENUSYS hMenuSys;
  FPMENUSYS MenuSysPtr;

  MENUINFO MenuInfo;

  BOOL bReturn;
  
  
  /* The key and shift state to remove. */
  MenuInfo.wKey        = wKey;   
  MenuInfo.wShiftState = wShiftState;   
      
  /* Get the menu sys. data */
  hMenuSys = (HMENUSYS) WndData( hDataWnd, WDM_GETMENUSYS, 0, NULL );
  
  /* Lock menu sys. data. */
  MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );

  /* Find the menu and delete it. */
  bReturn = DeleteAccelInfo( &(MenuSysPtr)->hMenuList, (FPMENUINFO) &MenuInfo );

  /* Unlock menu sys. data. */
  GlobalUnlock( hMenuSys );
    
  /* Success/Failure. */
  return( bReturn );
}


/*********************************************************************
**
**  Remove all of the accelerators.
**
**********************************************************************/
void __far __pascal RemoveAllAccelerators( HWND hDataWnd )
{
  HMENUSYS hMenuSys;
  FPMENUSYS MenuSysPtr;

  /* Get the menu sys. data */
  hMenuSys = (HMENUSYS) WndData( hDataWnd, WDM_GETMENUSYS, 0, NULL );
  
  /* Lock menu sys. data. */
  MenuSysPtr = ( FPMENUSYS ) GlobalLock( hMenuSys );

  /* Free menu list. */
  FreeMenuList( &(MenuSysPtr)->hMenuList );

  /* Unlock menu sys. data. */
  GlobalUnlock( hMenuSys );
}




/***********************************************************************************
**
**                              Private Functions
**
************************************************************************************/


/*********************************************************************
**
**  Gets the menu info for a menu entry in the menu list.
**
**********************************************************************/
BOOL __far __pascal GetMenuInfo( HMENULIST hMenuList, LPSTR lpMenuID, FPMENUINFO MenuInfoPtr )
{
  GETMENUINFOFUNCDATA FuncData;
  
  /* No list. */
  if( hButtonList == 0 ) return( FALSE );
  
  /* Initialize the data. */
  FuncData.bFound      = FALSE;
  FuncData.MenuID      = FindAtom( lpMenuID );
  FuncData.MenuInfoPtr = MenuInfoPtr;

  /* No atom regisitered means no button exists. */
  if( FuncData.MenuID == 0 ) return( FALSE );
  
  /* Find the menu entry's info. */
  EnumMenuInfo( hMenuList, (MENUINFOENUMPROC) FindMenuInfo, (LPARAM) (GETMENUINFOFUNCDATA) &FuncData );

  /* Return button window found. */
  return( FuncData.bFound );
}


/*********************************************************************
**
**  Callback function for the GetMenuInfo() function.
**
**  Finds the menu entry with the given ID and saves its info to the
**  buffer.
**
**********************************************************************/
static BOOL CALLBACK FindMenuInfo( FPMENUINFO MenuInfoPtr, LPARAM lParam )
{
  GETMENUINFOFUNCDATA FuncData;
  
  /* Get lParam data. */
  FuncData = (FPGETWINDOWFUNCDATA) lParam;

  /* Found correct button. */
  if( FuncData->MenuID == MenuInfoPtr->MenuID )
  {
    /* Save button's window handle. */
    *(FuncData.MenuInfoPtr) = *MenuInfoPtr;

    /* Found it! */
    FuncData.bFound = TRUE;

    /* Stop enumerating. */
    return FALSE;
  }

  /* Get next button's info. */
  return TRUE;
}


/*********************************************************************
**
**  Callback function when the menu bar is notified that an menu 
*   is pressed. Runs the macro associated with the pressed menu
**
**********************************************************************/
static BOOL CALLBACK MenuSelectedProc( FPMENUINFO fpMenuInfo, LPARAM lParam )
{
  char __far * MacroStringPtr;
  FPMENUSELFUNCDATA FuncDataPtr;

  /* Get key pressed data. */
  FuncDataPtr = ( FPMENUSELFUNCDATA ) lParam;
  
  /* If this is the correct menu */
  if( FuncDataPtr->MenuID == fpMenuInfo->MenuID )
  {
    /* Get the macro string. */
    MacroStringPtr = (char __far *) GlobalLock( fpMenuInfo->hMacroString );
    
    /* Run the macro. */
    RunHelpMacro( FuncDataPtr->hDataWnd, FuncDataPtr->hDataWnd, MacroStringPtr );

    /* Unlock macro string. */
    GlobalUnlock( fpMenuInfo->hMacroString );
  
    /* Processed key input. */
    FuncDataPtr->bProcessed = TRUE;
    
    /* Found it - stop enumerating. */
    return FALSE;
  }

  /* Get next menu's info. */
  return TRUE;
}



/***************************************************************************************
**
**
**                              Button list Management Functions
**
**
***************************************************************************************/


/*************************************
*
*  Add info for an menu to the list.
*                  
**************************************/
static BOOL __far __pascal SaveMenuInfo( HWND hErrorWnd, HMENULIST __far * hMenuList, FPMENUINFO fpMenuInfo )
{
  HMENUINFO hNewMenuInfo;

  
  /* Create new MENUINFO structure. */
  hNewMenuInfo = NewAccelInfo( hErrorWnd, fpMenuInfo );
  if( hNewMenuInfo == 0 ) return FALSE; 
  
  /*
  ** Insert the MENUINFO structure into the menu list.
  */
  InsertMenuInfo( hMenuList, hNewMenuInfo );
  
  /* Return success. */
  return TRUE;
}



/*************************************
*
*  Create a new menu info. structure.
*                  
**************************************/
static HMENUINFO __far __pascal NewAccelInfo( HWND hErrorWnd, FPMENUINFO fpMenuInfo )
{
  HMENUINFO hNewMenuInfo;
  FPMENUINFO fpNewMenuInfo;

  
  /* Allocate the new menu info. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewMenuInfo, sizeof(MENUINFO) ) ) return NULL;
  
  /* Lock it. */
  fpNewMenuInfo = (FPMENUINFO) GlobalLock( hNewMenuInfo ) ;

  /* Initialize general elements in the structure. */
  fpNewMenuInfo->MenuID       = fpMenuInfo->MenuID;
  fpNewMenuInfo->hMenu        = fpMenuInfo->hMenu;
  fpNewMenuInfo->hParentMenu  = fpMenuInfo->hParentMenu;
  fpNewMenuInfo->hMacroString = fpMenuInfo->hMacroString;
  fpNewMenuInfo->Next         = 0;

  /* Unlock it. */
  GlobalUnlock( hNewMenuInfo ) ;

  /* Return it. */
  return hNewMenuInfo;
}


/*************************************
*
*  Free an menu info. structure.
*                  
**************************************/
static void __far __pascal FreeAccelInfo( HMENUINFO hAccelInfo )
{
  FPMENUINFO fpMenuInfo;
  
  /* Lock it. */
  fpMenuInfo = (FPMENUINFO) GlobalLock( hAccelInfo ) ;

  /* Delete allocated info. */
  GlobalFree( fpMenuInfo->hMacroString ); 
    
  /* Unlock it. */
  GlobalUnlock( hAccelInfo ) ;

  /* Free it. */
  GlobalFree( hAccelInfo ) ;
}



/*************************************
*
*  Insert a new menu info. structure
*  at the top of the menu info. list.
*                  
**************************************/
static void __far __pascal InsertMenuInfo( HMENULIST __far * hMenuList, HMENUINFO hNewMenuInfo )
{
  FPMENUINFO fpNew;
  
  /* Already existing nodes. */
  if( *hMenuList != 0 ) 
  {
    /* Lock new node. */
    fpNew = (FPMENUINFO) GlobalLock( hNewMenuInfo ) ;
    
    /* Next node is the old top node. */
    fpNew->Next = *hMenuList;

    /* Unlock new node. */
    GlobalUnlock( hNewMenuInfo ) ;
  }

  /* New node is the top node. */
  *hMenuList = hNewMenuInfo;
}


/*************************************
*
*  Delete/Free menu info. list. 
*                  
**************************************/
static void __far __pascal FreeMenuList( HMENULIST __far * hMenuList )
{
  FPMENUINFO fpMenuInfo;
  HMENUINFO  hTmpAccelInfo;

  while( *hMenuList != 0 )
  {
    /* Lock current node. */
    fpMenuInfo = (FPMENUINFO) GlobalLock( *hMenuList ) ;
  
    /* Next node is to be new top node. */
    hTmpAccelInfo = fpMenuInfo->Next; 
    
    /* Unlock top handle. */
    GlobalUnlock( *hMenuList );
        
    /* Free top. */
    FreeAccelInfo( *hMenuList ); 

    /* Assign new top. */
    *hMenuList = hTmpAccelInfo; 
  }
}



/*************************************
*
*  Get the next menu info. structure
*  after the given hAccelInfo.
*                  
**************************************/
static HMENUINFO __far __pascal GetNextAccelInfo( HMENUINFO hAccelInfo )
{
  HMENUINFO hNextAccelInfo;
  FPMENUINFO fpMenuInfo;
  
  /* Handle bad. */
  if( hAccelInfo == 0 ) return 0;

  /* Lock it. */
  fpMenuInfo = (FPMENUINFO) GlobalLock( hAccelInfo ) ;
  
  /* Point new node to existing top node. */
  hNextAccelInfo = fpMenuInfo->Next;

  /* Unlock it. */
  GlobalUnlock( hAccelInfo ) ;

  /* Return next menu info's handle. */
  return hNextAccelInfo;
}



/*************************************
*
*  Enumerates all MENUINFO structures
*  contained in the menu info. list.
*                  
**************************************/
static void __far __pascal EnumMenuInfo( HMENULIST hMenuList, MENUINFOENUMPROC AccelInfoEnumProc, LPARAM lParam )
{
  FPMENUINFO fpMenuInfo;
  HMENUINFO hAccelInfo;
  BOOL bReturn;
  
  
  /* Get first menu info's handle. */
  hAccelInfo = hMenuList;
  
  while( hAccelInfo != 0 )
  {
    /* Lock it. */
    fpMenuInfo = (FPMENUINFO) GlobalLock( hAccelInfo ) ;
  
    /* Call callback function. */
    bReturn = (*AccelInfoEnumProc)( fpMenuInfo, lParam );
    
    /* Stop enumerating. */
    if( bReturn == FALSE )
    {
      /* Unlock it. */
      GlobalUnlock( hAccelInfo );

      /* Stop enumerating. */
      return;
    }
    
    /* Unlock it. */
    GlobalUnlock( hAccelInfo );

    /* Get the next in the list. */
    hAccelInfo = GetNextAccelInfo( hAccelInfo );
  }
}



/*************************************
*
*  Remove and free an menu info.
*  from the menu info. list based on
*  its wButtonID value.
*                  
**************************************/
static BOOL __far __pascal DeleteAccelInfo( HMENULIST __far * hMenuList, FPMENUINFO fpDelAccelInfo )
{
  FPMENUINFO fpMenuInfo, fpTmpAccelInfo;
  HMENUINFO hAccelInfo, hTmpAccelInfo;
  

  /* Existing first node. */
  if( *hMenuList == 0 ) return FALSE;

  /* Lock top handle. */
  fpMenuInfo = (FPMENUINFO) GlobalLock( *hMenuList ) ;
  
  /* If deleting top node. */
  if( fpMenuInfo->wKey == fpDelAccelInfo->wKey &&
      fpMenuInfo->wShiftState == fpDelAccelInfo->wShiftState )
  {
    /* Next node is to be new top node. */
    hTmpAccelInfo = fpMenuInfo->Next; 

    /* Unlock top handle. */
    GlobalUnlock( *hMenuList );
    
    /* Free top. */
    FreeAccelInfo( *hMenuList ); 
  
    /* Assign new top. */
    *hMenuList = hTmpAccelInfo;

    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( *hMenuList );
    
  /* Find correct node and remove. */
  hTmpAccelInfo = *hMenuList;
  hAccelInfo = GetNextAccelInfo( *hMenuList );

  while( hAccelInfo != 0 )
  {
    /* Lock current node. */
    fpMenuInfo = (FPMENUINFO) GlobalLock( hAccelInfo ) ;
  
    /* If deleting current node. */
    if( fpMenuInfo->wKey == fpDelAccelInfo->wKey &&
        fpMenuInfo->wShiftState == fpDelAccelInfo->wShiftState )
    {
      /* Lock previous node. */
      fpTmpAccelInfo = (FPMENUINFO) GlobalLock( hTmpAccelInfo ) ;

      /* Previous node's "next node" is deleted node's "next node". */
      fpTmpAccelInfo->Next = fpMenuInfo->Next; 
  
      /* Unlock previous node. */
      GlobalUnlock( hTmpAccelInfo );

      /* Unlock current node. */
      GlobalUnlock( hAccelInfo );
      
      /* Free top. */
      FreeAccelInfo( hAccelInfo ); 
  
      /* Success. */
      return TRUE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hAccelInfo ) ;

    /* Get next node. */
    hTmpAccelInfo = hAccelInfo;
    hAccelInfo = GetNextAccelInfo( hAccelInfo );
  }

  /* Did not find menu. */
  return FALSE;
}


