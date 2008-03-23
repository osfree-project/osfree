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

#include "btnbar.h"
#include "winmem.h"
#include "twinhelp.h"
#include "hlpmacro.h"
#include "msgbox.h"
#include "twhlprc.h"


/***********************************
**
**  Private File Defines
**
***********************************/

/* Class names. */
#define WILL_BUTTON_CLASS  "WILL_HELP_BUTTON"


/* Button bar window's extrabyte data positions. */                  
#define XBYTE_BUTTONLIST   0                                           /* Button list. */
#define XBYTE_BUTTONFONT   XBYTE_BUTTONLIST   + sizeof(HBUTTONLIST)    /* Button font. */ 
#define XBYTE_BUTTONWIDTH  XBYTE_BUTTONFONT   + sizeof(HFONT)          /* Button width */ 
#define XBYTE_BUTTONHEIGHT XBYTE_BUTTONWIDTH  + sizeof(WORD)           /* Button height.*/ 
#define XBYTE_HDATAWND     XBYTE_BUTTONHEIGHT + sizeof(WORD)           /* Data Window. */ 

/* Handle to the top of the button list. */
typedef HGLOBAL HBUTTONLIST;


/* Custom font. */
#define BUTTON_FONT_SIZE  8
#define BUTTON_FONT_FACE  "Helv"


/* 
** Height and width of a button in dialog box units. 
*/
#define BUTTON_WIDTH_UNITS    32
#define BUTTON_HEIGHT_UNITS   10


/* 
** Macros to adjust height and width of 
** a button units to screen pixel sizes.
*/
#define GetButtonWidth()  ( (BUTTON_WIDTH_UNITS * LOWORD(GetDialogBaseUnits())) / 4 )
#define GetButtonHeight() ( (BUTTON_HEIGHT_UNITS * HIWORD(GetDialogBaseUnits()) ) / 8 )


/*
** Information for a button in the list.
*/
typedef HGLOBAL HBUTTONINFO;
typedef struct tagBUTTONINFO
{
  HWND hButtonWnd;
  ATOM ButtonID;
  HGLOBAL hMacroString;
  HBUTTONINFO Next;
} 
BUTTONINFO;
typedef BUTTONINFO __far * FPBUTTONINFO;


/* 
** EnumButtonInfo() callback function. 
*/
typedef BOOL ( CALLBACK * BUTTONINFOENUMPROC )( FPBUTTONINFO fpButtonInfo, LPARAM lParam );


/* 
** Data used by the ChangeButtonsMacro() function. 
*/
typedef struct tagCHANGEBUTTONSMACROFUNCDATA
{
  BOOL  bSuccess;                       /* Was the function successsful. */
  ATOM  ButtonID;                       /* Button's ID. */
  LPSTR MacroStringPtr;                 /* Replacement macro. */
  HWND  hButtonBarWnd;                  /* Handle to the button bar window. */
}
CHANGEBUTTONSMACROFUNCDATA;
typedef CHANGEBUTTONSMACROFUNCDATA __far * FPCHANGEBUTTONSMACROFUNCDATA;


/* 
** Data used by the DisableButton() function. 
*/
typedef struct tagENABLEBUTTONFUNCDATA
{
  BOOL  bSuccess;                       /* Was the function successsful. */
  ATOM  ButtonID;                       /* Button's ID. */
  BOOL  bEnable;                        /* Enable it - FALSE means disable it. */
}
ENABLEBUTTONFUNCDATA;
typedef ENABLEBUTTONFUNCDATA __far * FPENABLEBUTTONFUNCDATA;


/* 
** Data used by the GetButtonWindow() function. 
*/
typedef struct tagGETWINDOWFUNCDATA
{
  HWND hWnd;
  ATOM ButtonID;
}
GETWINDOWFUNCDATA;
typedef GETWINDOWFUNCDATA __far * FPGETWINDOWFUNCDATA;


/* 
** Data used by the ButtonIsPressed() function. 
*/
typedef struct tagBTNPRESSEDFUNCDATA
{
  HWND hDataWnd;
  ATOM ButtonID;
  HWND hButtonBarWnd;
}
BTNPRESSEDFUNCDATA;
typedef BTNPRESSEDFUNCDATA __far * FPBTNPRESSEDFUNCDATA;


/* 
** Data used by the RepositionButtons() function. 
*/
typedef struct tagREPOSITIONFUNCDATA
{
  WORD wNextXPos;
  WORD wNextYPos;

  WORD wMaxYPos;

  WORD wBtnBarWndHeight;
  
  WORD wNumberButtons;

  WORD wButtonWidth;
  WORD wButtonHeight;
}
REPOSITIONFUNCDATA;
typedef REPOSITIONFUNCDATA __far * FPREPOSITIONFUNCDATA;


/* 
** Data used by the SetMaxButtonSize() function. 
*/
typedef struct tagCALCSIZEFUNCDATA
{
  WORD wMaxButtonWidth;
  WORD wMaxButtonHeight;
}
CALCSIZEFUNCDATA;
typedef CALCSIZEFUNCDATA __far * FPCALCSIZEFUNCDATA;



/***********************************
**
**  Private Function Prototypes
**
***********************************/

static void  __far __pascal RepositionButtons( HWND hButtonBarWnd, WORD wMaxYPos, WORD __far * wBtnBarWndHeight );
static BOOL CALLBACK RepositionGivenButton( FPBUTTONINFO fpButtonInfo, LPARAM lParam );
static BOOL __far __pascal SaveButtonInfo( HWND hErrorWnd, HBUTTONLIST __far * hButtonList, FPBUTTONINFO fpButtonInfo );
static HBUTTONINFO __far __pascal NewButtonInfo( HWND hErrorWnd, FPBUTTONINFO fpButtonInfo );
static void __far __pascal FreeButtonInfo( HBUTTONINFO hButtonInfo );
static void __far __pascal InsertButtonInfo( HBUTTONLIST __far * hButtonList, HBUTTONINFO hNewButtonInfo );
static void __far __pascal FreeButtonInfoList( HBUTTONLIST __far * hButtonList );
static HBUTTONINFO __far __pascal GetNextButtonInfo( HBUTTONINFO hButtonInfo );
static void __far __pascal EnumButtonInfo( HBUTTONLIST hButtonList, BUTTONINFOENUMPROC ButtonInfoEnumProc, LPARAM lParam );
static BOOL CALLBACK ButtonIsPressed( FPBUTTONINFO fpButtonInfo, LPARAM lParam );
static WORD  __far __pascal GetNumberOfButtons( HWND hButtonBarWnd );
static BOOL CALLBACK CountButtons( FPBUTTONINFO fpButtonInfo, LPARAM lParam );
static BOOL CALLBACK FindButtonHandle( FPBUTTONINFO fpButtonInfo, LPARAM lParam );
static BOOL CALLBACK ChangeButtonsMacroProc( FPBUTTONINFO fpButtonInfo, LPARAM lParam );
static BOOL __far __pascal DeleteButtonWithID( HBUTTONLIST __far * hButtonList, ATOM ButtonID  );
static BOOL CALLBACK GetMaxButtonSizesProc( FPBUTTONINFO fpButtonInfo, LPARAM lParam );
static void __far __pascal SetMaxButtonSize( HWND hButtonBarWnd, HBUTTONLIST hButtonList );
static BOOL CALLBACK EnableButtonProc( FPBUTTONINFO fpButtonInfo, LPARAM lParam );


/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/


/*********************************************************************
**
**  Register the button bar's window class. 
**
**********************************************************************/
BOOL __far __pascal RegisterButtonBarClass( HINSTANCE hInstance )
{
  WNDCLASS  wcWndClass;

  /* 
  ** Register the Button window class 
  */
  wcWndClass.lpszClassName = WILL_BUTTON_CLASS;
  wcWndClass.lpfnWndProc = ButtonWndProc;
  wcWndClass.cbClsExtra  = 0;
  wcWndClass.cbWndExtra  = sizeof( HBUTTONLIST ) + sizeof( HFONT ) + sizeof( WORD ) + sizeof( WORD ) + sizeof(HWND);
  wcWndClass.style = CS_HREDRAW | CS_VREDRAW;
  wcWndClass.hInstance = hInstance;
  wcWndClass.hIcon = (HICON)NULL;
  wcWndClass.hCursor = (HCURSOR)NULL;
  wcWndClass.hbrBackground = GetStockObject(GRAY_BRUSH);
  wcWndClass.lpszMenuName = NULL;

  /* Error? */
  if ( ! RegisterClass( &wcWndClass) ) 
  {
    /* Failure. */
    return (FALSE);
  }

  /* Success. */
  return( TRUE );
}


/*********************************************************************
**
**  Create the button bar's window.
**
**********************************************************************/
HWND __far __pascal CreateButtonBarWnd( HWND hDataWnd, HWND hParentWnd, HINSTANCE hInstance )
{
  HWND hButtonBarWnd = 0;
  

  /* Create button bar. */
  hButtonBarWnd = CreateWindow
            ( 
              WILL_BUTTON_CLASS,
              NULL, 
              WS_CHILD | WS_VISIBLE,
              0, 0, 
              CW_USEDEFAULT, CW_USEDEFAULT, 
              hParentWnd, 
              (HMENU) NULL,
              hInstance,
              NULL 
            );
            
  /* If we have a window. */
  if( hButtonBarWnd != NULL )
  {
    /* Save handle to the macro engine. */
    SetWindowWord( hButtonBarWnd, XBYTE_HDATAWND, (WORD) hDataWnd );
  }

  return( hButtonBarWnd );
}




/*********************************************************************
**
**  Window procedure for the button bar's window.
**
**********************************************************************/
LONG FAR PASCAL __export ButtonWndProc ( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  /* Drawing variables. */
  HDC hDC;
  PAINTSTRUCT ps;
  RECT  Rect;
  
  /* Handle to the button info. list. */
  HBUTTONLIST hButtonList;

  /* Custom font for the button. */
  HFONT hButtonFont;

  /* Button bar window size variables. */
  WORD wClientWidth, wClientHeight;

  /* Button information. */
  WORD wBtnBarWndHeight, BtnCount;

  /* Data sent we button is pressed. */
  BTNPRESSEDFUNCDATA ButtonPressedData;

  switch ( message ) 
  {
    case WM_COMMAND:
    {
      /* Get button bar list's handle. */
      hButtonList = (HBUTTONLIST) GetWindowWord( hWnd, XBYTE_BUTTONLIST );

      /* Init. data struct. */
      ButtonPressedData.hDataWnd = (HWND) GetWindowWord( hWnd, XBYTE_HDATAWND );
      ButtonPressedData.ButtonID = (ATOM) wParam;
      ButtonPressedData.hButtonBarWnd = hWnd;
      
      /* Find button pressed and run its macro. */
      EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) ButtonIsPressed, (LPARAM) (FPBTNPRESSEDFUNCDATA) &ButtonPressedData );
      return 0;
    }

    case WM_CREATE:
    {
      /* Initialize extrabytes data for button list. */
      SetWindowWord( hWnd, XBYTE_BUTTONLIST, 0 );
      
      /* Init. width and height of buttons. */
      SetWindowWord( hWnd, XBYTE_BUTTONWIDTH, 0 );
      SetWindowWord( hWnd, XBYTE_BUTTONHEIGHT, 0 );

      /********
      **
      ** Create our custom font.
      **
      ********/
      
      hDC = GetDC( hWnd );
      
      /* Create button bar's custom font. */
      hButtonFont = CreateFont( -1 * MulDiv( BUTTON_FONT_SIZE, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ),
                                0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, BUTTON_FONT_FACE ); 
      ReleaseDC( hWnd, hDC );
      
      /* Got font? */
      if( hButtonFont == NULL ) return -1;
        
      /* Save button bar window's old font to extrabytes data. */
      SetWindowWord( hWnd, XBYTE_BUTTONFONT, (WORD) hButtonFont );

      return 0;
    }

    case WM_DESTROY:
    {
      /********
      **
      ** Delete our button list.
      **
      ********/

      /* Get button bar list's handle. */
      hButtonList = (HBUTTONLIST) GetWindowWord( hWnd, XBYTE_BUTTONLIST );

      /* Free button list. */
      FreeButtonInfoList( &hButtonList );


      /********
      **
      ** Delete our custom font.
      **
      ********/
      
      /* Get button bar's custom font. */
      hButtonFont = (HFONT) GetWindowWord( hWnd, XBYTE_BUTTONFONT );
      
      /* Delete our custom font. */
      DeleteObject( hButtonFont );

      return 0;
    }  

    case PRIVMSG_RESIZE:
    {
      /* 
      ** Get client area of main window. 
      */
      GetClientRect( GetParent( hWnd ), &Rect );
      wClientWidth = Rect.right - Rect.left;
      wClientHeight = Rect.bottom - Rect.top;

      /* Get the number of buttons in the button list. */
      BtnCount = GetNumberOfButtons( hWnd );

      /* If we have buttons in the bar. */
      if( BtnCount != 0 )
      {
        /* 
        ** Position the buttons and size the button bar window.
        */
  
        /* Reposition buttons. */
        RepositionButtons( hWnd, wClientWidth, &wBtnBarWndHeight );

        /* Set button bar's window size. */
        MoveWindow( hWnd, 0, 0, wClientWidth, wBtnBarWndHeight, TRUE );
      }
      return 0;
    }

    case WM_PAINT:
    {
      GetClientRect( hWnd, &Rect );
      hDC = BeginPaint(hWnd, &ps);
      
      /* Paint border. */
      Rect.top -= 1;
      Rect.right += 1;
      FrameRect( hDC, &Rect, GetStockObject( BLACK_BRUSH ) );
      
      EndPaint(hWnd, &ps);
      return 0;
    }
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}


/*********************************************************************
**
**  Adds a new button to the button bar.
**
**********************************************************************/
BOOL __far __pascal AddBarButton
( 
  HINSTANCE hInstance,
  HWND hButtonBarWnd, 
  LPSTR lpButtonText, 
  LPSTR lpButtonID, 
  LPSTR lpMacro 
)
{
  /* Pointer to macro string. */
  char __far * MacroStringPtr;
  
  /* Handle to the button info. list. */
  HBUTTONLIST hButtonList;

  /* Custom font for the button. */
  HFONT hButtonFont;

  /* Button info. to store in button info. list. */
  BUTTONINFO ButtonInfo;
  
  BOOL bReturn;
    
  
  /* See if the button ID is already being used by an existing window. */
  if( GetButtonWindow( hButtonBarWnd, lpButtonID ) != 0 )
  {
    /* Error. */ 
    MsgBox( hInstance, hButtonBarWnd, IDS_ERRORTITLE, BTNALREADYEXISTS, MB_ICONHAND | MB_OK );

    /* Failure. */
    return( FALSE );
  }

  /* Get button bar's button list handle. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  
  /* Allocate memory for storage of macro string. */
  if( !GlobalAllocMem( hButtonBarWnd, &(ButtonInfo).hMacroString, _fstrlen( lpMacro ) + sizeof(char) ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Copy macro string storage area. */
  MacroStringPtr = (char __far *) GlobalLock( ButtonInfo.hMacroString );
  _fstrcpy( MacroStringPtr, lpMacro );
  GlobalUnlock( ButtonInfo.hMacroString );
  
  /* Make atom for the button's ID. */
  ButtonInfo.ButtonID = AddAtom( lpButtonID );   
      
  /* Create the button. */
  ButtonInfo.hButtonWnd = CreateWindow
                          (
                            "BUTTON", 
                            lpButtonText, 
                            WS_CHILD | WS_DISABLED | BS_PUSHBUTTON, 
                            0, 0, 
                            CW_USEDEFAULT, CW_USEDEFAULT, 
                            hButtonBarWnd, 
                            (HMENU) ButtonInfo.ButtonID, 
                            hInstance, 
                            NULL
                          );
    
  /* Error? */
  if( ! ButtonInfo.hButtonWnd )
  {
    /* Delete allocated info. */
    DeleteAtom( ButtonInfo.ButtonID );
    GlobalFree( ButtonInfo.hMacroString ); 
    
    return FALSE;
  }

  /* Set button's custom font. */
  hButtonFont = (HFONT) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONFONT );
  SendMessage( ButtonInfo.hButtonWnd, WM_SETFONT, (WPARAM) hButtonFont, (LPARAM) MAKELONG((WORD) TRUE, 0) );

  /* Save button window. */
  bReturn = SaveButtonInfo( hButtonBarWnd, &hButtonList, (FPBUTTONINFO) &ButtonInfo );
  if( bReturn == FALSE )
  {
    /* Delete allocated info. */
    DeleteAtom( ButtonInfo.ButtonID );
    GlobalFree( ButtonInfo.hMacroString ); 
    
    /* Delete button. */
    DestroyWindow( ButtonInfo.hButtonWnd );
    
    return FALSE;
  }
      
  /* Set the button bar's button list handle in case it changed. */
  SetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST, (WORD) hButtonList );
  
  /* Set the maximum button size since maybe the new button is the largest. */
  SetMaxButtonSize( hButtonBarWnd, hButtonList );
      
  /* Resize window. */
  PostMessage( hButtonBarWnd, PRIVMSG_RESIZE, 0, 0L );
      
  /* Success. */
  return( TRUE );
}




/*********************************************************************
**
**  Gets the window handle of a button in the button bar 
**  based on its ID.
**
**********************************************************************/
BOOL __far __pascal DeleteBarButton
( 
  HWND hButtonBarWnd, 
  LPSTR lpButtonID
)
{
  HBUTTONLIST hButtonList;
  ATOM ButtonID;
  
  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) 
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Make ID into an atom. */
  ButtonID = FindAtom( lpButtonID );
  
  /* No atom regisitered means no button exists. */
  if( ButtonID == 0 ) return FALSE;

  /* Find the button with the I and delete it. */
  if( ! DeleteButtonWithID( &hButtonList, ButtonID ) )
  {
    /* Failure. */
    return FALSE;
  }

  /* Set the button bar's button list handle in case it changed. */
  SetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST, (WORD) hButtonList );

  /* Set the maximum button size since maybe the deleted button was the largest. */
  SetMaxButtonSize( hButtonBarWnd, hButtonList );

  /* Success. */
  return( TRUE );
}



/*************************************
*
*  Add info for a button to the list.
*                  
**************************************/
static void __far __pascal SetMaxButtonSize
( 
  HWND hButtonBarWnd, 
  HBUTTONLIST hButtonList
)
{
  CALCSIZEFUNCDATA FuncData;
       
  /* Init. data. */
  FuncData.wMaxButtonWidth = 0;
  FuncData.wMaxButtonHeight = 0;
  
  /* Position each button. */
  EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) GetMaxButtonSizesProc, (LPARAM) (FPCALCSIZEFUNCDATA) &FuncData );

  /* Set button width. */
  SetWindowWord( hButtonBarWnd, XBYTE_BUTTONWIDTH, FuncData.wMaxButtonWidth );
      
  /* Set button height. */
  SetWindowWord( hButtonBarWnd, XBYTE_BUTTONHEIGHT, FuncData.wMaxButtonHeight );
}



/*********************************************************************
**
**  Change the macro of a button in the button bar given its ID.
**
**********************************************************************/
BOOL __far __pascal ChangeButtonsMacro
( 
  HWND hButtonBarWnd, 
  LPSTR lpButtonID,
  LPSTR lpMacro 
)
{
  HBUTTONLIST hButtonList;
  CHANGEBUTTONSMACROFUNCDATA FuncData;
  
  
  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) return FALSE;
  
  /* Initialize the data. */
  FuncData.bSuccess = FALSE;
  FuncData.ButtonID = FindAtom( lpButtonID );
  FuncData.MacroStringPtr = lpMacro;
  FuncData.hButtonBarWnd = hButtonBarWnd;
  
  /* No atom regisitered means no button exists. */
  if( FuncData.ButtonID == 0 ) return FALSE;
  
  /* Position each button. */
  EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) ChangeButtonsMacroProc, (LPARAM) (FPCHANGEBUTTONSMACROFUNCDATA) &FuncData );

  /* Successful? */
  return( FuncData.bSuccess );
}



/*********************************************************************
**
**  Enable/disable the button having the specified ID.
**
**********************************************************************/
BOOL __far __pascal EnableButton
( 
  HWND  hButtonBarWnd, 
  LPSTR lpButtonID,
  BOOL  bEnable 
)
{
  HBUTTONLIST hButtonList;
  ENABLEBUTTONFUNCDATA FuncData;
  
  
  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) return FALSE;
  
  /* Initialize the data. */
  FuncData.bSuccess = FALSE;
  FuncData.ButtonID = FindAtom( lpButtonID );
  FuncData.bEnable  = bEnable;
  
  /* No atom regisitered means no button exists. */
  if( FuncData.ButtonID == 0 ) return FALSE;
  
  /* Position each button. */
  EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) EnableButtonProc, (LPARAM) (FPENABLEBUTTONFUNCDATA) &FuncData );

  /* Successful? */
  return( FuncData.bSuccess );
}



/*********************************************************************
**
**  Gets the window handle of a button in the button bar 
**  based on its ID.
**
**********************************************************************/
HWND __far __pascal GetButtonWindow( HWND hButtonBarWnd, LPSTR lpButtonID )
{
  HBUTTONLIST hButtonList;
  GETWINDOWFUNCDATA FuncData;
  
  
  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) return 0;
  
  /* Initialize the data. */
  FuncData.hWnd = 0;
  FuncData.ButtonID = FindAtom( lpButtonID );

  /* No atom regisitered means no button exists. */
  if( FuncData.ButtonID == 0 ) return 0;
  
  /* Find the button's window handle. */
  EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) FindButtonHandle, (LPARAM) (FPGETWINDOWFUNCDATA) &FuncData );

  /* Return button window found. */
  return( FuncData.hWnd );
}



/*********************************************************************
**
**  Gets the number of buttons in the button bar.
**
**********************************************************************/
WORD __far __pascal GetNumberOfButtons( HWND hButtonBarWnd )
{
  WORD nButtons = 0;
  HBUTTONLIST hButtonList;
  

  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) return 0;
  
  /* Position each button. */
  EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) CountButtons, (LPARAM) (WORD __far *) &nButtons );

  /* Return number of buttons found. */
  return( nButtons );
}


/*********************************************************************
**
**  Remove all of the buttons in the button bar.
**
**********************************************************************/
void __far __pascal RemoveAllButtons( HWND hButtonBarWnd )
{
  HBUTTONLIST hButtonList;

  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) return;
  
  /* Free button list. */
  FreeButtonInfoList( &hButtonList );

  /* Initialize extrabytes data for button list. */
  SetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST, 0 );
}


/***********************************************************************************
**
**                              Private Functions
**
************************************************************************************/


/*********************************************************************
**
**  Callback function for the ChangeButtonsMacro() function.
**  Finds the button with the ID and changes its macro string.
**
**********************************************************************/
static BOOL CALLBACK ChangeButtonsMacroProc( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  FPCHANGEBUTTONSMACROFUNCDATA FuncData ;
  
  /* Pointer to macro string. */
  HGLOBAL      hMacroString;
  char __far * MacroStringPtr;
  

  /* Get lParam data. */
  FuncData = (FPCHANGEBUTTONSMACROFUNCDATA) lParam;

  /* Found correct button. */
  if( FuncData->ButtonID == fpButtonInfo->ButtonID )
  {
    /* Allocate memory for storage of macro string. */
    if( !GlobalAllocMem( FuncData->hButtonBarWnd, &hMacroString, 
                        _fstrlen( FuncData->MacroStringPtr ) + sizeof(char) ) )
    {
      /* Stop enumerating. */
      return( FALSE );
    }
    
    /* Copy macro string storage area. */
    MacroStringPtr = (char __far *) GlobalLock( hMacroString );
    _fstrcpy( MacroStringPtr, FuncData->MacroStringPtr );
    GlobalUnlock( hMacroString );

    /* Free the current macro string. */
    GlobalFree( fpButtonInfo->hMacroString ); 

    /* Assign new macro string. */
    fpButtonInfo->hMacroString = hMacroString;
    
    /* Successful. */
    FuncData->bSuccess = TRUE;
    
    /* Stop enumerating. */
    return FALSE;
  }

  /* Get next button's info. */
  return TRUE;
}



/*********************************************************************
**
**  Callback function for the EnableButton() function.
**  Finds the button with the ID and disables it.
**
**********************************************************************/
static BOOL CALLBACK EnableButtonProc( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  FPENABLEBUTTONFUNCDATA FuncData ;

  /* Get lParam data. */
  FuncData = (FPENABLEBUTTONFUNCDATA) lParam;

  /* Found correct button. */
  if( FuncData->ButtonID == fpButtonInfo->ButtonID )
  {
    /* Disable/Enable the button. */
    EnableWindow( fpButtonInfo->hButtonWnd, FuncData->bEnable );
    
    /* Successful. */
    FuncData->bSuccess = TRUE;
    
    /* Stop enumerating. */
    return FALSE;
  }

  /* Get next button's info. */
  return TRUE;
}



/*********************************************************************
**
**  Callback function for the GetButtonWindow() function.
**  Finds the button with the ID and saves its window handle.
**
**********************************************************************/
static BOOL CALLBACK FindButtonHandle( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  FPGETWINDOWFUNCDATA FuncData ;
  
  /* Get lParam data. */
  FuncData = (FPGETWINDOWFUNCDATA) lParam;

  /* Found correct button. */
  if( FuncData->ButtonID == fpButtonInfo->ButtonID )
  {
    /* Save button's window handle. */
    FuncData->hWnd = fpButtonInfo->hButtonWnd;

    /* Stop enumerating. */
    return FALSE;
  }

  /* Get next button's info. */
  return TRUE;
}


/*********************************************************************
**
**  Callback function when the button bar is notified that a button 
*   is pressed. Performs the action associated with the pressed button.
**
**********************************************************************/
static BOOL CALLBACK ButtonIsPressed( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  char __far * MacroStringPtr;
  FPBTNPRESSEDFUNCDATA ButtonPressedDataPtr;

  /* Get button pressed data. */
  ButtonPressedDataPtr = ( FPBTNPRESSEDFUNCDATA ) lParam;
  
  /* If this is the correct button. */
  if( ButtonPressedDataPtr->ButtonID == fpButtonInfo->ButtonID )
  {
    /* Get the macro string. */
    MacroStringPtr = (char __far *) GlobalLock( fpButtonInfo->hMacroString );
    
    /* Run the macro. */
    RunHelpMacro( ButtonPressedDataPtr->hButtonBarWnd, ButtonPressedDataPtr->hDataWnd, MacroStringPtr );

    /* Unlock macro string. */
    GlobalUnlock( fpButtonInfo->hMacroString );
  
    /* Found it - stop enumerating. */
    return FALSE;
  }

  /* Get next button's info. */
  return TRUE;
}



/*********************************************************************
**
**  Adjusts current button positions.
**
**********************************************************************/

static void  __far __pascal RepositionButtons( HWND hButtonBarWnd, WORD wMaxYPos, WORD __far * wBtnBarWndHeight )
{
  REPOSITIONFUNCDATA RepositionFuncData;
  HBUTTONLIST hButtonList;


  /* Get handle to button list. */
  hButtonList = (HBUTTONLIST) GetWindowWord( hButtonBarWnd, XBYTE_BUTTONLIST );
  if( hButtonList == 0 ) return;

  /* Init. data. */
  RepositionFuncData.wNextXPos = 0;
  RepositionFuncData.wNextYPos = 0;
  RepositionFuncData.wMaxYPos  = wMaxYPos;
  RepositionFuncData.wBtnBarWndHeight = 0;
  RepositionFuncData.wNumberButtons = 0;
  RepositionFuncData.wButtonWidth = GetWindowWord( hButtonBarWnd, XBYTE_BUTTONWIDTH );
  RepositionFuncData.wButtonHeight = GetWindowWord( hButtonBarWnd, XBYTE_BUTTONHEIGHT );
  
  /* Position each button. */
  EnumButtonInfo( hButtonList, (BUTTONINFOENUMPROC) RepositionGivenButton, (LPARAM) (FPREPOSITIONFUNCDATA) &RepositionFuncData );

  /* Get the height of the button bar. */
  *wBtnBarWndHeight = RepositionFuncData.wBtnBarWndHeight;
}





/*********************************************************************
**
**  Callback function for the RepositionButtons() function.
**  Repositions the button passed to the function.
**
**********************************************************************/
static BOOL CALLBACK RepositionGivenButton( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  FPREPOSITIONFUNCDATA fpRepositionFuncData;

  
  /* Get data passed from the RepositionButtons() function. */
  fpRepositionFuncData = (FPREPOSITIONFUNCDATA) lParam;
  
  /* Do we need to skip to the next row? */
  if( fpRepositionFuncData->wNextXPos + fpRepositionFuncData->wButtonWidth > fpRepositionFuncData->wMaxYPos )
  {
    /* Reset x-position. */
    fpRepositionFuncData->wNextXPos = 0;    

    /* If this fpRepositionFuncData the first button, don't skip to next line. */
    if( fpRepositionFuncData->wNumberButtons != 0 )
    {
      /* 
      ** Move to the next row. All buttons should be the same height so just use
      ** current button's height to calc. position of next row. 
      */
      fpRepositionFuncData->wNextYPos = fpRepositionFuncData->wNextYPos + fpRepositionFuncData->wButtonHeight;
    }
  }

  /* Move the button to next position. */
  MoveWindow( fpButtonInfo->hButtonWnd, fpRepositionFuncData->wNextXPos, fpRepositionFuncData->wNextYPos, fpRepositionFuncData->wButtonWidth, fpRepositionFuncData->wButtonHeight, TRUE );

  /* Move to the next x-position. */
  fpRepositionFuncData->wNextXPos  = fpRepositionFuncData->wNextXPos + fpRepositionFuncData->wButtonWidth;

  /* Set last known height of button bar. */
  fpRepositionFuncData->wBtnBarWndHeight = fpRepositionFuncData->wNextYPos + fpRepositionFuncData->wButtonHeight;

  /* Show button if it was just created and is not visible yet. */
  ShowWindow( fpButtonInfo->hButtonWnd, SW_SHOW ); 

  /* Another button processed. */
  fpRepositionFuncData->wNumberButtons = fpRepositionFuncData->wNumberButtons + 1;

  /* Get next button's info. */
  return TRUE;
}


/*********************************************************************
**
**  Callback function for the GetMaxButtonSizes() function.
**  Finds the maximum width and height.
**
**********************************************************************/
static BOOL CALLBACK GetMaxButtonSizesProc( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  HDC hDC;
  char szButtonText[ MAX_BUTTON_TEXT ];
  WORD wButtonWidth, wButtonHeight, wTextWidth, wTextHeight;
  DWORD dwExtent;
  
  FPCALCSIZEFUNCDATA FuncData;
  
  
  /* Get lParam data. */
  FuncData = (FPCALCSIZEFUNCDATA) lParam;

  /* Get the button's text. */
  GetWindowText( fpButtonInfo->hButtonWnd, szButtonText, MAX_BUTTON_TEXT );
  
  /* Get a DC for the button. */
  hDC = GetDC( fpButtonInfo->hButtonWnd );
    
  /* Get button's text width in pixels. */
  dwExtent = GetTextExtent( hDC, szButtonText, _fstrlen(szButtonText) );
  wTextWidth = LOWORD( dwExtent );  
  wTextHeight = HIWORD( dwExtent );  
    
  /* Release DC. */
  ReleaseDC( fpButtonInfo->hButtonWnd, hDC );

  /* Each button has the same width - longest string defines that width. */
  wButtonWidth  = wTextWidth;
  wButtonHeight = wTextHeight + ( wTextHeight * 1/4 );
      
  /* New max. button width. */
  if( FuncData->wMaxButtonWidth < wButtonWidth )
  {
    FuncData->wMaxButtonWidth = wButtonWidth;
  }
      
  /* New max. button height. */
  if( FuncData->wMaxButtonHeight < wButtonHeight )
  {
    FuncData->wMaxButtonHeight = wButtonHeight;
  }
      
  /* Get next button's info. */
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
*  Add info for a button to the list.
*                  
**************************************/
static BOOL __far __pascal SaveButtonInfo( HWND hErrorWnd, HBUTTONLIST __far * hButtonList, FPBUTTONINFO fpButtonInfo )
{
  HBUTTONINFO hNewButtonInfo;

  
  /* Create new BUTTONINFO structure. */
  hNewButtonInfo = NewButtonInfo( hErrorWnd, fpButtonInfo );
  if( hNewButtonInfo == 0 ) return FALSE; 
  
  /*
  ** Insert the BUTTONINFO structure into the button List.
  */
  InsertButtonInfo( hButtonList, hNewButtonInfo );
  
  /* Return success. */
  return TRUE;
}



/*************************************
*
*  Create a new button info. structure.
*                  
**************************************/
static HBUTTONINFO __far __pascal NewButtonInfo( HWND hErrorWnd, FPBUTTONINFO fpButtonInfo )
{
  HBUTTONINFO hNewButtonInfo;
  FPBUTTONINFO fpNewButtonInfo;

  
  /* Allocate the new button info. */
  if( ! GlobalAllocMem( hErrorWnd, &hNewButtonInfo, sizeof(BUTTONINFO) ) ) return NULL;
  
  /* Lock it. */
  fpNewButtonInfo = (FPBUTTONINFO) GlobalLock( hNewButtonInfo ) ;

  /* Initialize general elements in the structure. */
  fpNewButtonInfo->hButtonWnd   = fpButtonInfo->hButtonWnd;
  fpNewButtonInfo->ButtonID     = fpButtonInfo->ButtonID;
  fpNewButtonInfo->hMacroString = fpButtonInfo->hMacroString;
  fpNewButtonInfo->Next       = 0;

  /* Unlock it. */
  GlobalUnlock( hNewButtonInfo ) ;

  /* Return it. */
  return hNewButtonInfo;
}



/*************************************
*
*  Free a button info. structure.
*                  
**************************************/
static void __far __pascal FreeButtonInfo( HBUTTONINFO hButtonInfo )
{
  FPBUTTONINFO fpButtonInfo;
  
  /* Lock it. */
  fpButtonInfo = (FPBUTTONINFO) GlobalLock( hButtonInfo ) ;

  /* Delete allocated info. */
  DeleteAtom( fpButtonInfo->ButtonID );
  GlobalFree( fpButtonInfo->hMacroString ); 
    
  /* Delete button. */
  DestroyWindow( fpButtonInfo->hButtonWnd );
    
  /* Unlock it. */
  GlobalUnlock( hButtonInfo ) ;

  /* Free it. */
  GlobalFree( hButtonInfo ) ;
}



/*************************************
*
*  Insert a new button info. structure
*  at the end of the button info. list.
*                  
**************************************/
static void __far __pascal InsertButtonInfo( HBUTTONLIST __far * hButtonList, HBUTTONINFO hNewButtonInfo )
{
  FPBUTTONINFO fpCurrInfo;
  HBUTTONINFO hCurrent, hNext;
  
  /* No existing first node - insert it. */
  if( *hButtonList == 0 ) 
  {
    *hButtonList = hNewButtonInfo;
    return;
  }

  /* Assign current handle the top handle to the list. */
  hCurrent = *hButtonList;

  /* Move to end. */
  while( TRUE )
  {
      /* Lock current node. */
    fpCurrInfo = (FPBUTTONINFO) GlobalLock( hCurrent ) ;
    
    /* At the last node. */
    if( fpCurrInfo->Next == 0 )
    {
      /* Point new node to existing top node. */
      fpCurrInfo->Next = hNewButtonInfo;
    
      /* Unlock current node. */
      GlobalUnlock( hCurrent ) ;
    
      return;
    }

    /* Go to next node. */
    else
    {
      /* Get handle to next node. */
      hNext = fpCurrInfo->Next;
    
      /* Unlock current node. */
      GlobalUnlock( hCurrent ) ;
    
      /* Assign current node to next node. */
      hCurrent = hNext;
    }
  }
}


/*************************************
*
*  Delete/Free button info. list. 
*                  
**************************************/
static void __far __pascal FreeButtonInfoList( HBUTTONLIST __far * hButtonList )
{
  FPBUTTONINFO fpButtonInfo;
  HBUTTONINFO  hTmpButtonInfo;


  while( *hButtonList != 0 )
  {
    /* Lock current node. */
    fpButtonInfo = (FPBUTTONINFO) GlobalLock( *hButtonList ) ;
  
    /* Next node is to be new top node. */
    hTmpButtonInfo = fpButtonInfo->Next; 
    
    /* Unlock top handle. */
    GlobalUnlock( *hButtonList );
        
    /* Free top. */
    FreeButtonInfo( *hButtonList ); 

    /* Assign new top. */
    *hButtonList = hTmpButtonInfo; 
  }
}



/*************************************
*
*  Get the next button info. structure
*  after the given hButtonInfo.
*                  
**************************************/
static HBUTTONINFO __far __pascal GetNextButtonInfo( HBUTTONINFO hButtonInfo )
{
  HBUTTONINFO hNextButtonInfo;
  FPBUTTONINFO fpButtonInfo;
  
  /* Handle bad. */
  if( hButtonInfo == 0 ) return 0;

  /* Lock it. */
  fpButtonInfo = (FPBUTTONINFO) GlobalLock( hButtonInfo ) ;
  
  /* Point new node to existing top node. */
  hNextButtonInfo = fpButtonInfo->Next;

  /* Unlock it. */
  GlobalUnlock( hButtonInfo ) ;

  /* Return next button info's handle. */
  return hNextButtonInfo;
}



/*************************************
*
*  Enumerates all BUTTONINFO structures
*  contained in the button info. list.
*                  
**************************************/
static void __far __pascal EnumButtonInfo( HBUTTONLIST hButtonList, BUTTONINFOENUMPROC ButtonInfoEnumProc, LPARAM lParam )
{
  FPBUTTONINFO fpButtonInfo;
  HBUTTONINFO hButtonInfo;
  BOOL bReturn;
  
  
  /* Get first button info's handle. */
  hButtonInfo = hButtonList;
  
  while( hButtonInfo != 0 )
  {
    /* Lock it. */
    fpButtonInfo = (FPBUTTONINFO) GlobalLock( hButtonInfo ) ;
  
    /* Call callback function. */
    bReturn = (*ButtonInfoEnumProc)( fpButtonInfo, lParam );
    
    /* Stop enumerating. */
    if( bReturn == FALSE )
    {
      /* Unlock it. */
      GlobalUnlock( hButtonInfo );

      /* Stop enumerating. */
      return;
    }
    
    /* Unlock it. */
    GlobalUnlock( hButtonInfo );

    hButtonInfo = GetNextButtonInfo( hButtonInfo );
  }
}



/*********************************************************************
**
**  Callback function for the GetNumberOfButtons() function.
**  Increments the button counter.
**
**********************************************************************/
static BOOL CALLBACK CountButtons( FPBUTTONINFO fpButtonInfo, LPARAM lParam )
{
  WORD __far * nButtons;
  
  nButtons = (WORD __far *) lParam;
  *nButtons = *nButtons + 1; 
  
  /* Get next button's info. */
  return TRUE;
}


/*************************************
*
*  Remove and free a button info.
*  from the button info. list based on
*  its wButtonID value.
*                  
**************************************/
static BOOL __far __pascal DeleteButtonWithID( HBUTTONLIST __far * hButtonList, ATOM ButtonID  )
{
  FPBUTTONINFO fpButtonInfo, fpTmpButtonInfo;
  HBUTTONINFO hButtonInfo, hTmpButtonInfo;
  

  /* Existing first node. */
  if( *hButtonList == 0 ) return FALSE;

  /* Lock top handle. */
  fpButtonInfo = (FPBUTTONINFO) GlobalLock( *hButtonList ) ;
  
  /* If deleting top node. */
  if( fpButtonInfo->ButtonID == ButtonID )
  {
    /* Next node is to be new top node. */
    hTmpButtonInfo = fpButtonInfo->Next; 

    /* Unlock top handle. */
    GlobalUnlock( *hButtonList );
    
    /* Free top. */
    FreeButtonInfo( *hButtonList ); 
  
    /* Assign new top. */
    *hButtonList = hTmpButtonInfo;

    /* Success. */
    return TRUE;
  }

  /* Unlock top handle. */
  GlobalUnlock( *hButtonList );
    
  /* Find correct node and remove. */
  hTmpButtonInfo = *hButtonList;
  hButtonInfo = GetNextButtonInfo( *hButtonList );

  while( hButtonInfo != 0 )
  {
    /* Lock current node. */
    fpButtonInfo = (FPBUTTONINFO) GlobalLock( hButtonInfo ) ;
  
    /* If deleting current node. */
    if( fpButtonInfo->ButtonID == ButtonID )
    {
      /* Lock previous node. */
      fpTmpButtonInfo = (FPBUTTONINFO) GlobalLock( hTmpButtonInfo ) ;

      /* Previous node's "next node" is deleted node's "next node". */
      fpTmpButtonInfo->Next = fpButtonInfo->Next; 
  
      /* Unlock previous node. */
      GlobalUnlock( hTmpButtonInfo );

      /* Unlock current node. */
      GlobalUnlock( hButtonInfo );
      
      /* Free top. */
      FreeButtonInfo( hButtonInfo ); 
  
      /* Success. */
      return TRUE;
    }
    
    /* Unlock it. */
    GlobalUnlock( hButtonInfo ) ;

    /* Get next node. */
    hTmpButtonInfo = hButtonInfo;
    hButtonInfo = GetNextButtonInfo( hButtonInfo );
  }

  /* Did not find button. */
  return FALSE;
}


