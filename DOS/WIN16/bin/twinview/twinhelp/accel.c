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

#include "accel.h"
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
** Information for an accelerator in the list.
*/
typedef HGLOBAL HACCELINFO;
typedef struct tagACCELINFO
{
  WORD wKey;
  WORD wShiftState;
  HGLOBAL hMacroString;
  HACCELINFO Next;
} 
ACCELINFO;
typedef ACCELINFO __far * FPACCELINFO;


/*
** Information for the accelerator system.
*/
typedef HACCELINFO HACCELLIST;
typedef struct tagACCELSYS
{
  HWND       hDataWnd;        /* Main data window. */
  HACCELLIST hAccelList;      /* Top of the accelerator list. */
} 
ACCELSYS;
typedef ACCELSYS __far * FPACCELSYS;


/* 
** EnumAccelInfo() callback function. 
*/
typedef BOOL ( CALLBACK * ACCELINFOENUMPROC )( FPACCELINFO fpAccelInfo, LPARAM lParam );


/* 
** Data used by the KeyIsPressed() function. 
*/
typedef struct tagKEYPRESSFUNCDATA
{
  BOOL bProcessed;
  HWND hDataWnd;
  WORD wKey;
  WORD wShiftState;
}
KEYPRESSFUNCDATA;
typedef KEYPRESSFUNCDATA __far * FPKEYPRESSFUNCDATA;


/* 
** Data used by the AccelExists() function. 
*/
typedef struct tagEXISTSFUNCDATA
{
  BOOL bExists;
  WORD wKey;
  WORD wShiftState;
}
EXISTSFUNCDATA;
typedef EXISTSFUNCDATA __far * FPEXISTSFUNCDATA;



/***********************************
**
**  Private Function Prototypes
**
***********************************/
static void __far __pascal EnumAccelInfo( HACCELLIST hAccelList, ACCELINFOENUMPROC AccelInfoEnumProc, LPARAM lParam );
static HACCELINFO __far __pascal GetNextAccelInfo( HACCELINFO hAccelInfo );                                           
static void __far __pascal FreeAccelList( HACCELLIST __far * hAccelList );
static void __far __pascal InsertAccelInfo( HACCELLIST __far * hAccelList, HACCELINFO hNewAccelInfo );
static void __far __pascal FreeAccelInfo( HACCELINFO hAccelInfo );
static HACCELINFO __far __pascal NewAccelInfo( HWND hErrorWnd, FPACCELINFO fpAccelInfo );
static BOOL __far __pascal SaveAccelInfo( HWND hErrorWnd, HACCELLIST __far * hAccelList, FPACCELINFO fpAccelInfo );
static BOOL CALLBACK KeyIsPressedProc( FPACCELINFO fpAccelInfo, LPARAM lParam );
static BOOL __far __pascal DeleteAccelInfo( HACCELLIST __far * hAccelList, FPACCELINFO fpDelAccelInfo );
static BOOL CALLBACK AccelExistsProc( FPACCELINFO fpAccelInfo, LPARAM lParam );


/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/

                                                                 
/***********************************
**
**  Starts accelerator support system. 
**
**  Returns:
**
**     Success - TRUE.
**     Failure - FALSE.
**
***********************************/
BOOL __far __pascal StartAccelSupport( HWND hDataWnd )
{
  HACCELSYS  hAccelSys;
  FPACCELSYS AccelSysPtr;
  
  /* Allocate memory for accel. support data. */
  if( !GlobalAllocMem( hDataWnd, &hAccelSys, sizeof(ACCELSYS) ) )
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, CANTSTARTACCELSYS, MB_ICONHAND | MB_OK );

    /* Set the accel. sys. data to NULL. */
    WndData( hDataWnd, WDM_SETACCELSYS, (DWORD) NULL, NULL );

    /* Failure. */
    return( FALSE );
  }
  
  /* Lock data. */
  AccelSysPtr = ( FPACCELSYS ) GlobalLock( hAccelSys );
      
  /* Init. structure. */
  AccelSysPtr->hDataWnd = hDataWnd;
  AccelSysPtr->hAccelList = 0;

  /* Unlock data. */
  GlobalUnlock( hAccelSys );

    /* Set the accel. sys. data. */
  WndData( hDataWnd, WDM_SETACCELSYS, (DWORD) hAccelSys, NULL );
            
  /* Success. */
  return( TRUE );  
}


/***********************************
**
**  Stops accelerator support system. 
**
**  Returns: None
**
***********************************/
void __far __pascal StopAccelSupport( HWND hDataWnd )
{
  HACCELSYS  hAccelSys;
  FPACCELSYS AccelSysPtr;
            
  /* Get the accel. sys. data */
  hAccelSys = (HACCELSYS) WndData( hDataWnd, WDM_GETACCELSYS, 0, NULL );
  
  /* If we have a accel. support. */
  if( hAccelSys != NULL )
  {
    /* Lock accel. support data. */
    AccelSysPtr = ( FPACCELSYS ) GlobalLock( hAccelSys );
        
    /* Free accel. list. */
    FreeAccelList( &(AccelSysPtr)->hAccelList );
  
    /* Free accel. support data. */
    GlobalUnlock( hAccelSys );
    GlobalFree( hAccelSys );
  }
}


/*********************************************************************
**
**  Process key as if its an accelerator.
**
**  Returns:
**
**      TRUE: Processed key.
**     FALSE: Not Processed key.
**
**********************************************************************/
BOOL __far __pascal KeyIsPressed( HWND hDataWnd, WORD wKey )
{
  KEYPRESSFUNCDATA KeyPressData;
  HACCELLIST hAccelList;
  HACCELSYS hAccelSys;
  FPACCELSYS AccelSysPtr;
  
  BOOL bShiftPressed,
       bCtrlPressed,
       bAltPressed;          
  

  /* Get the accel. sys. data */
  hAccelSys = (HACCELSYS) WndData( hDataWnd, WDM_GETACCELSYS, 0, NULL );
  
  /* If we have a accel. support. */
  if( hAccelSys != NULL )
  {
    /* Lock data. */
    AccelSysPtr = ( FPACCELSYS ) GlobalLock( hAccelSys );
    
    /* Get handle to the accelerator list. */    
    hAccelList =  AccelSysPtr->hAccelList;

    /* Unlock data. */
    GlobalUnlock( hAccelSys );
    
    /* Init. data struct. */
    KeyPressData.bProcessed  = FALSE;
    KeyPressData.hDataWnd    = AccelSysPtr->hDataWnd;
    KeyPressData.wKey        = wKey;
    
    /* Get current shift key states. */
    if( GetKeyState( VK_SHIFT ) < 0 )
    {
      bShiftPressed = TRUE;
    } 
    else
    {
      bShiftPressed = FALSE;
    } 
    if( GetKeyState( VK_CONTROL ) < 0 )
    {
      bCtrlPressed = TRUE;
    } 
    else
    {
      bCtrlPressed = FALSE;
    } 
    if( GetKeyState( VK_MENU ) < 0 )
    {
      bAltPressed = TRUE;
    } 
    else
    {
      bAltPressed = FALSE;
    } 

    /* Set the shift state. */
    if( bShiftPressed == FALSE && bCtrlPressed == FALSE && bAltPressed == FALSE )
    {
      KeyPressData.wShiftState = 0;
    }
    else if( bShiftPressed == TRUE && bCtrlPressed == FALSE && bAltPressed == FALSE )
    {
      KeyPressData.wShiftState = 1;
    }
    else if( bShiftPressed == FALSE && bCtrlPressed == TRUE && bAltPressed == FALSE )
    {
      KeyPressData.wShiftState = 2;
    }
    else if( bShiftPressed == TRUE && bCtrlPressed == TRUE && bAltPressed == FALSE )
    {
      KeyPressData.wShiftState = 3;
    }
    else if( bShiftPressed == FALSE && bCtrlPressed == FALSE && bAltPressed == TRUE )
    {
      KeyPressData.wShiftState = 4;
    }
    else if( bShiftPressed == TRUE && bCtrlPressed == FALSE && bAltPressed == TRUE )
    {
      KeyPressData.wShiftState = 5;
    }
    else if( bShiftPressed == FALSE && bCtrlPressed == TRUE && bAltPressed == TRUE )
    {
      KeyPressData.wShiftState = 6;
    }
    else if( bShiftPressed == TRUE && bCtrlPressed == TRUE && bAltPressed == TRUE )
    {
      KeyPressData.wShiftState = 7;
    }
        
    /* Find accelerator and run macro. */
    EnumAccelInfo( hAccelList, (ACCELINFOENUMPROC) KeyIsPressedProc, (LPARAM) (FPKEYPRESSFUNCDATA) &KeyPressData );
  }

  /* Processed? */
  return( KeyPressData.bProcessed );
}



/*********************************************************************
**
**  Adds a new accelerator to the list.
**
**********************************************************************/
BOOL __far __pascal AddAccelerator
( 
  HWND hDataWnd, 
  WORD wKey, 
  WORD wShiftState, 
  LPSTR lpMacro 
)
{
  HACCELSYS hAccelSys;
  FPACCELSYS AccelSysPtr;

  /* Pointer to macro string. */
  char __far * MacroStringPtr;

  /* Accel. info. to store in accel. info. list. */
  ACCELINFO AccelInfo;
  
  BOOL bReturn;
    
  EXISTSFUNCDATA AccelExistsData;
  
  
  /* Get the accel. sys. data */
  hAccelSys = (HACCELSYS) WndData( hDataWnd, WDM_GETACCELSYS, 0, NULL );
  
  /* Lock accel. sys. data. */
  AccelSysPtr = ( FPACCELSYS ) GlobalLock( hAccelSys );
    
  /* See if accelerator already exists. */
  AccelExistsData.bExists = FALSE;
  AccelExistsData.wKey = wKey;
  AccelExistsData.wShiftState = wShiftState;
  EnumAccelInfo( AccelSysPtr->hAccelList, (ACCELINFOENUMPROC) AccelExistsProc, (LPARAM) (FPEXISTSFUNCDATA) &AccelExistsData );
  if( AccelExistsData.bExists == TRUE )
  {
    /* Unlock accel. sys. data. */
    GlobalUnlock( hAccelSys );
    
    /* Error. */ 
    MsgBox( GetLibInst(), hDataWnd, IDS_ERRORTITLE, ACCELALREADYEXISTS, MB_ICONHAND | MB_OK );

    /* Success. */
    return( FALSE );
  }
  
  /* Allocate memory for storage of macro string. */
  if( !GlobalAllocMem( hDataWnd, &(AccelInfo).hMacroString, _fstrlen( lpMacro ) + sizeof(char) ) )
  {
    /* Unlock accel. sys. data. */
    GlobalUnlock( hAccelSys );
    
    /* Failure. */
    return( FALSE );
  }
  
  /* Copy macro string storage area. */
  MacroStringPtr = (char __far *) GlobalLock( AccelInfo.hMacroString );
  _fstrcpy( MacroStringPtr, lpMacro );
  GlobalUnlock( AccelInfo.hMacroString );
  
  /* Init. the key and shift state. */
  AccelInfo.wKey        = wKey;   
  AccelInfo.wShiftState = wShiftState;   
      
  /* Save accel. info. */
  bReturn = SaveAccelInfo( hDataWnd, &(AccelSysPtr)->hAccelList, (FPACCELINFO) &AccelInfo );
  if( bReturn == FALSE )
  {
    /* Unlock accel. sys. data. */
    GlobalUnlock( hAccelSys );
    
    /* Delete allocated info. */
    GlobalFree( AccelInfo.hMacroString ); 
    
    /* Failure. */
    return FALSE;
  }
      
  /* Unlock accel. sys. data. */
  GlobalUnlock( hAccelSys );
    
  /* Success. */
  return( TRUE );
}




/*********************************************************************
**
**  Delete an accelerator to the list.
**
**********************************************************************/
BOOL __far __pascal RemoveAccelerator
( 
  HWND hDataWnd, 
  WORD wKey, 
  WORD wShiftState
)
{
  HACCELSYS hAccelSys;
  FPACCELSYS AccelSysPtr;

  ACCELINFO AccelInfo;

  BOOL bReturn;
  
  
  /* The key and shift state to remove. */
  AccelInfo.wKey        = wKey;   
  AccelInfo.wShiftState = wShiftState;   
      
  /* Get the accel. sys. data */
  hAccelSys = (HACCELSYS) WndData( hDataWnd, WDM_GETACCELSYS, 0, NULL );
  
  /* Lock accel. sys. data. */
  AccelSysPtr = ( FPACCELSYS ) GlobalLock( hAccelSys );

  /* Find the accel. and delete it. */
  bReturn = DeleteAccelInfo( &(AccelSysPtr)->hAccelList, (FPACCELINFO) &AccelInfo );

  /* Unlock accel. sys. data. */
  GlobalUnlock( hAccelSys );
    
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
  HACCELSYS hAccelSys;
  FPACCELSYS AccelSysPtr;

  /* Get the accel. sys. data */
  hAccelSys = (HACCELSYS) WndData( hDataWnd, WDM_GETACCELSYS, 0, NULL );
  
  /* Lock accel. sys. data. */
  AccelSysPtr = ( FPACCELSYS ) GlobalLock( hAccelSys );

  /* Free accel. list. */
  FreeAccelList( &(AccelSysPtr)->hAccelList );

  /* Unlock accel. sys. data. */
  GlobalUnlock( hAccelSys );
}




/***********************************************************************************
**
**                              Private Functions
**
************************************************************************************/


/*********************************************************************
**
**  Does an accelerator already exist?
**
**********************************************************************/
static BOOL CALLBACK AccelExistsProc( FPACCELINFO fpAccelInfo, LPARAM lParam )
{
  FPEXISTSFUNCDATA FuncDataPtr;

  /* Get key pressed data. */
  FuncDataPtr = ( FPEXISTSFUNCDATA ) lParam;
  
  /* If this is the correct accel. */
  if( FuncDataPtr->wKey == fpAccelInfo->wKey &&  
      FuncDataPtr->wShiftState == fpAccelInfo->wShiftState )
  {
    /* Processed key input. */
    FuncDataPtr->bExists = TRUE;
    
    /* Found it - stop enumerating. */
    return FALSE;
  }

  /* Get next accel.'s info. */
  return TRUE;
}



/*********************************************************************
**
**  Callback function when the accel. bar is notified that an accelerator 
*   is pressed. Runs the macro associated with the pressed accel.
**
**********************************************************************/
static BOOL CALLBACK KeyIsPressedProc( FPACCELINFO fpAccelInfo, LPARAM lParam )
{
  char __far * MacroStringPtr;
  FPKEYPRESSFUNCDATA FuncDataPtr;

  /* Get key pressed data. */
  FuncDataPtr = ( FPKEYPRESSFUNCDATA ) lParam;
  
  /* If this is the correct accel. */
  if( FuncDataPtr->wKey == fpAccelInfo->wKey &&  
      FuncDataPtr->wShiftState == fpAccelInfo->wShiftState )
  {
    /* Get the macro string. */
    MacroStringPtr = (char __far *) GlobalLock( fpAccelInfo->hMacroString );
    
    /* Run the macro. */
    RunHelpMacro( FuncDataPtr->hDataWnd, FuncDataPtr->hDataWnd, MacroStringPtr );

    /* Unlock macro string. */
    GlobalUnlock( fpAccelInfo->hMacroString );
  
    /* Processed key input. */
    FuncDataPtr->bProcessed = TRUE;
    
    /* Found it - stop enumerating. */
    return FALSE;
  }

  /* Get next accel.'s info. */
  return TRUE;
}


/***************************************************************************************
**
**
**                              Button List Management Functions
**
**
***************************************************************************************/


/*************************************
*
*  Add info for an accelerator to the list.
*                  
**************************************/
static BOOL __far __pascal SaveAccelInfo( HWND hErrorWnd, HACCELLIST __far * hAccelList, FPACCELINFO fpAccelInfo )
{
  HACCELINFO hNewAccelInfo;

  
  /* Create new ACCELINFO structure. */
  hNewAccelInfo = NewAccelInfo( hErrorWnd, fpAccelInfo );
  if( hNewAccelInfo == 0 ) return FALSE; 
  
  /*
  ** Insert the ACCELINFO structure into the accel. List.
  */
  InsertAccelInfo( hAccelList, hNewAccelInfo );
  
  /* Return success. */
  return TRUE;
}



/*************************************
*
*  Create a new accel. info. structure.
*                  
**************************************/
static HACCELINFO __far __pascal NewAccelInfo( HWND hErrorWnd, FPACCELINFO fpAccelInfo )
{
  HACCELINFO hNewAccelInfo;
  FPACCELINFO fpNewButtonInfo;

  
  /* Allocate the new accel. info. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewAccelInfo, sizeof(ACCELINFO) ) ) return NULL;
  
  /* Lock it. */
  fpNewButtonInfo = (FPACCELINFO) GlobalLock( hNewAccelInfo ) ;

  /* Initialize general elements in the structure. */
  fpNewButtonInfo->wKey         = fpAccelInfo->wKey;
  fpNewButtonInfo->wShiftState  = fpAccelInfo->wShiftState;
  fpNewButtonInfo->hMacroString = fpAccelInfo->hMacroString;
  fpNewButtonInfo->Next         = 0;

  /* Unlock it. */
  GlobalUnlock( hNewAccelInfo ) ;

  /* Return it. */
  return hNewAccelInfo;
}



/*************************************
*
*  Free an accelerator info. structure.
*                  
**************************************/
static void __far __pascal FreeAccelInfo( HACCELINFO hAccelInfo )
{
  FPACCELINFO fpAccelInfo;
  
  /* Lock it. */
  fpAccelInfo = (FPACCELINFO) GlobalLock( hAccelInfo ) ;

  /* Delete allocated info. */
  GlobalFree( fpAccelInfo->hMacroString ); 
    
  /* Unlock it. */
  GlobalUnlock( hAccelInfo ) ;

  /* Free it. */
  GlobalFree( hAccelInfo ) ;
}



/*************************************
*
*  Insert a new accel. info. structure
*  at the top of the accel. info. list.
*                  
**************************************/
static void __far __pascal InsertAccelInfo( HACCELLIST __far * hAccelList, HACCELINFO hNewAccelInfo )
{
  FPACCELINFO fpNew;
  
  /* Already existing nodes. */
  if( *hAccelList != 0 ) 
  {
    /* Lock new node. */
    fpNew = (FPACCELINFO) GlobalLock( hNewAccelInfo ) ;
    
    /* Next node is the old top node. */
    fpNew->Next = *hAccelList;

    /* Unlock new node. */
    GlobalUnlock( hNewAccelInfo ) ;
  }

  /* New node is the top node. */
  *hAccelList = hNewAccelInfo;
}


/*************************************
*
*  Delete/Free accel. info. list. 
*                  
**************************************/
static void __far __pascal FreeAccelList( HACCELLIST __far * hAccelList )
{
  FPACCELINFO fpAccelInfo;
  HACCELINFO  hTmpAccelInfo;

  while( *hAccelList != 0 )
  {
    /* Lock current node. */
    fpAccelInfo = (FPACCELINFO) GlobalLock( *hAccelList ) ;
  
    /* Next node is to be new top node. */
    hTmpAccelInfo = fpAccelInfo->Next; 
    
    /* Unlock top handle. */
    GlobalUnlock( *hAccelList );
        
    /* Free top. */
    FreeAccelInfo( *hAccelList ); 

    /* Assign new top. */
    *hAccelList = hTmpAccelInfo; 
  }
}



/*************************************
*
*  Get the next accel. info. structure
*  after the given hAccelInfo.
*                  
**************************************/
static HACCELINFO __far __pascal GetNextAccelInfo( HACCELINFO hAccelInfo )
{
  HACCELINFO hNextAccelInfo;
  FPACCELINFO fpAccelInfo;
  
  /* Handle bad. */
  if( hAccelInfo == 0 ) return 0;

  /* Lock it. */
  fpAccelInfo = (FPACCELINFO) GlobalLock( hAccelInfo ) ;
  
  /* Point new node to existing top node. */
  hNextAccelInfo = fpAccelInfo->Next;

  /* Unlock it. */
  GlobalUnlock( hAccelInfo ) ;

  /* Return next accel. info's handle. */
  return hNextAccelInfo;
}



/*************************************
*
*  Enumerates all ACCELINFO structures
*  contained in the accel. info. list.
*                  
**************************************/
static void __far __pascal EnumAccelInfo( HACCELLIST hAccelList, ACCELINFOENUMPROC AccelInfoEnumProc, LPARAM lParam )
{
  FPACCELINFO fpAccelInfo;
  HACCELINFO hAccelInfo;
  BOOL bReturn;
  
  
  /* Get first accel. info's handle. */
  hAccelInfo = hAccelList;
  
  while( hAccelInfo != 0 )
  {
    /* Lock it. */
    fpAccelInfo = (FPACCELINFO) GlobalLock( hAccelInfo ) ;
  
    /* Call callback function. */
    bReturn = (*AccelInfoEnumProc)( fpAccelInfo, lParam );
    
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
*  Remove and free an accelerator info.
*  from the accel. info. list based on
*  its wButtonID value.
*                  
**************************************/
static BOOL __far __pascal DeleteAccelInfo( HACCELLIST __far * hAccelList, FPACCELINFO fpDelAccelInfo )
{
  FPACCELINFO fpAccelInfo, fpTmpAccelInfo;
  HACCELINFO hAccelInfo, hTmpAccelInfo;
  

  /* Existing first node. */
  if( *hAccelList == 0 ) return FALSE;

  /* Lock top handle. */
  fpAccelInfo = (FPACCELINFO) GlobalLock( *hAccelList ) ;
  
  /* If deleting top node. */
  if( fpAccelInfo->wKey == fpDelAccelInfo->wKey &&
      fpAccelInfo->wShiftState == fpDelAccelInfo->wShiftState )
  {
    /* Next node is to be new top node. */
    hTmpAccelInfo = fpAccelInfo->Next; 

    /* Unlock top handle. */
    GlobalUnlock( *hAccelList );
    
    /* Free top. */
    FreeAccelInfo( *hAccelList ); 
  
    /* Assign new top. */
    *hAccelList = hTmpAccelInfo;

    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( *hAccelList );
    
  /* Find correct node and remove. */
  hTmpAccelInfo = *hAccelList;
  hAccelInfo = GetNextAccelInfo( *hAccelList );

  while( hAccelInfo != 0 )
  {
    /* Lock current node. */
    fpAccelInfo = (FPACCELINFO) GlobalLock( hAccelInfo ) ;
  
    /* If deleting current node. */
    if( fpAccelInfo->wKey == fpDelAccelInfo->wKey &&
        fpAccelInfo->wShiftState == fpDelAccelInfo->wShiftState )
    {
      /* Lock previous node. */
      fpTmpAccelInfo = (FPACCELINFO) GlobalLock( hTmpAccelInfo ) ;

      /* Previous node's "next node" is deleted node's "next node". */
      fpTmpAccelInfo->Next = fpAccelInfo->Next; 
  
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

  /* Did not find accel.. */
  return FALSE;
}


