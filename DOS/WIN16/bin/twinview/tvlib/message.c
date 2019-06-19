/***************************************************************************************
**
**
**                              Message Handling Functions
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
*  Globals
*
**************************************/
UINT   TV_MSG = 0;


/*************************************
*
*  Set the value of the message used for a TVLIB.DLL message.
*
**************************************/
void __far __pascal __export SetTVMsgValue( UINT TVMsgValue )
{
  TV_MSG = TVMsgValue;
}


/*************************************
*
*  Get the value of the message used for a TVLIB.DLL message.
*
**************************************/
UINT __far __pascal __export GetTVMsgValue( void )
{
  return TV_MSG;
}


/*************************************
*
*  Process a TVLIB.DLL message.
*
*  WPARAM is the action to perform.
*  LPARAM is data to use.
*
**************************************/
long __far __pascal __export TVLibMsgProc( HWND hWnd, WORD wMsg, WORD wParam, LONG lParam )
{
  switch ( wParam )
  {
    case MSGID_CLOSELIB:
    {
      /* Free a library opened by the TVLIB.DLL. */
      FreeLibrary( (HINSTANCE) lParam );
      
      /* Message processed. */
      return 0;
    }
  }

  /* Message not processed. */
  return 0;
}


/*************************************
*
*  Subclass function for all child
*  windows.
*
*  Process message for child window.
*
**************************************/
long __far __pascal __export TVFilterWndProc(HWND hWnd, WORD Msg, WORD wParam, LONG lParam)
{
  WNDPROC ChildWndProc;
  LONG lReturn;
  
  
  /* Get the child's WndProc() from our own dataspace in the child window's extrabytes. */
  ChildWndProc = (WNDPROC) GetWindowLong( hWnd, TVL_GWW_WNDPROC );

  /* Decrement the entry count. */
  IncTVEntryCount( hWnd );

  /* Send it the message. */
  lReturn = CallWindowProc( ChildWndProc, hWnd, Msg, wParam, lParam ); 

  /* Increment the entry count. */
  DecTVEntryCount( hWnd );

  /* 
  ** If the message is WM_NCDESTROY process the
  ** window as a closed window. 
  */
  if( Msg == WM_NCDESTROY )
  {
    /* Remove menu alias' associated with the window. */  
    DeleteMenuInfoForWnd( hWnd );
    
    /* Remove window from the library's window list. */
    EnumTVLibInfo( RemoveChildWindow, (LPARAM) (HWND __far *) &hWnd );
  }

  return( lReturn );
}                            
                    
                    

