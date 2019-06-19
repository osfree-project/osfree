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
/*******************************************************************************************
**
**                                 MACRO ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <stdlib.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "macroeng.h"
#include "hlpmacro.h"
#include "msgbox.h"
#include "globals.h"
#include "twhlprc.h"
#include "wnddata.h"
#include "winmem.h"
#include "search.h"
#include "twinhelp.h"
#include "btnbar.h"
#include "hlpfile.h"
#include "filebuff.h"
#include "accel.h"


/***********************************
**
**  Private Defines
**
***********************************/

#define DEFAULT_HELP_TABLE_NAME  "DEFAULT_HELP_MACROS"

/* How many default macros are there. */
#define NUMDEFAULTMACROS ( sizeof DefaultMacros / sizeof DefaultMacros[0] )

typedef struct tagHLPMACRODATA
{
  HWND hDataWnd;
}
HLPMACRODATA;
typedef HLPMACRODATA __far * HLPMACRODATAPTR;


/***********************************
**
**  Private Function Prototypes
**
***********************************/

static BOOL CALLBACK ExecHelpFilesMacrosProc( FPSYSTEMREC fpSysRec, LPARAM lParam );

BOOL CALLBACK AddAcceleratorMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK BackMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK BrowseButtonsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK ChangeButtonBindingMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK ContentsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK CreateButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK DestroyButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK DisableButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK EnableButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK HistoryMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK PrevMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK NextMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK JumpContentsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK RemoveAcceleratorMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK SearchMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);

BOOL CALLBACK SetContentsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
);


/***********************************
**
**  Global Static Data
**
***********************************/

/* Default help macros. */
static MACROTABLEENTRY DefaultMacros[] =
{
  "AA",                  AddAcceleratorMacroProc,      NULL,
  "AddAccelerator",      AddAcceleratorMacroProc,      NULL,
  "Back",                BackMacroProc,                NULL,
  "BrowseButtons",       BrowseButtonsMacroProc,       NULL,
  "Contents",            ContentsMacroProc,            NULL,
  "CreateButton",        CreateButtonMacroProc,        NULL,
  "CB",                  CreateButtonMacroProc,        NULL,
  "ChangeButtonBinding", ChangeButtonBindingMacroProc, NULL,
  "CBB",                 ChangeButtonBindingMacroProc, NULL,
  "DestroyButton",       DestroyButtonMacroProc,       NULL,
  "DisableButton",       DisableButtonMacroProc,       NULL,
  "DB",                  DisableButtonMacroProc,       NULL,
  "EnableButton",        EnableButtonMacroProc,        NULL,
  "EB",                  EnableButtonMacroProc,        NULL,
  "History",             HistoryMacroProc,             NULL,
  "JumpContents",        JumpContentsMacroProc,        NULL,
  "Next",                NextMacroProc,                NULL,
  "Prev",                PrevMacroProc,                NULL,
  "RA",                  RemoveAcceleratorMacroProc,   NULL,
  "RemoveAccelerator",   RemoveAcceleratorMacroProc,   NULL,
  "Search",              SearchMacroProc,              NULL,
  "SetContents",         SetContentsMacroProc,         NULL
};


/*********************************************************************************************
**
**                                   Shared Functions
**
**********************************************************************************************/

                                                                 
/***********************************
**
**  Starts macro system and loads
**  the default help macro commands. 
**
**  Returns:
**
**     Success - TRUE.
**     Failure - FALSE.
**
***********************************/
BOOL __far __pascal StartMacroSupport( HWND hDataWnd, HWND hErrorWnd )
{
  HMACROENGINE hMacroEngine;
  
  HGLOBAL         hHelpMacroData;
  HLPMACRODATAPTR HelpMacroDataPtr;
  
  /* Allocate memory for general macro data. */
  if( !GlobalAllocMem( hErrorWnd, &hHelpMacroData, sizeof(HLPMACRODATA) ) )
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, CANTSTARTMACROENGINE, MB_ICONHAND | MB_OK );

    /* Set the macro engine var. to NULL. */
    WndData( hDataWnd, WDM_SETMACROENGINE, (DWORD) NULL, NULL );
            
    /* Failure. */
    return( FALSE );
  }
  
  /* Lock data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hHelpMacroData );
      
  /* Init. structure. */
  HelpMacroDataPtr->hDataWnd = hDataWnd;

  /* Unlock data. */
  GlobalUnlock( hHelpMacroData );
      
  /* Start up an instance of the macro engine. */
  hMacroEngine = StartMacroEngine( hErrorWnd, hHelpMacroData );

  /* Error. */
  if( hMacroEngine == NULL )
  {
    /* Free help macro data. */
    GlobalFree( hHelpMacroData ); 
    
    /* Error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, CANTSTARTMACROENGINE, MB_ICONHAND | MB_OK );

    /* Set the macro engine var. to NULL. */
    WndData( hDataWnd, WDM_SETMACROENGINE, (DWORD) NULL, NULL );
            
    /* Failure. */
    return( FALSE );
  }

  /* Register default help macro table. */
  if( !RegisterMacroTable( hErrorWnd, hMacroEngine, DEFAULT_HELP_TABLE_NAME, 
                           NUMDEFAULTMACROS, DefaultMacros ) )
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, CANTSTARTMACROENGINE, MB_ICONHAND | MB_OK );

    /* Stop the macro engine. */
    StopMacroEngine( &hMacroEngine );
    
    /* Set the macro engine var. to NULL. */
    WndData( hDataWnd, WDM_SETMACROENGINE, (DWORD) NULL, NULL );
            
    /* Failure. */
    return( FALSE );
  }
  
  /* Load default macro table. */
  if( ! LoadMacroTable( hErrorWnd, hMacroEngine, DEFAULT_HELP_TABLE_NAME ) )
  {
    /* Error. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, CANTSTARTMACROENGINE, MB_ICONHAND | MB_OK );

    /* Stop the macro engine. */
    StopMacroEngine( &hMacroEngine );
    
    /* Set the macro engine var. to NULL. */
    WndData( hDataWnd, WDM_SETMACROENGINE, (DWORD) NULL, NULL );
            
    /* Failure. */
    return( FALSE );
  }

  /* Save the macro engine handle. */
  WndData( hDataWnd, WDM_SETMACROENGINE, (DWORD) hMacroEngine, NULL );

  /* Success. */
  return( TRUE );  
}


/***********************************
**
**  Stops macro system. 
**
**  Returns: None
**
***********************************/
void __far __pascal StopMacroSupport( HWND hDataWnd )
{
  HMACROENGINE hMacroEngine;
  HGLOBAL      hHelpMacroData;
  
  /* Get the macro engine's instance handle. */
  hMacroEngine = (HMACROENGINE) WndData( hDataWnd, WDM_GETMACROENGINE, 0, NULL );
            
  /* If we have a macro engine. */
  if( hMacroEngine != NULL )
  {
    /* Get the macro data that we originally sent to the engine. */
    hHelpMacroData = GetMacroEngineAppData( hMacroEngine );
    
    /* Free help macro data. */
    GlobalFree( hHelpMacroData );

    /* Stop the macro engine. */
    StopMacroEngine( &hMacroEngine );
  }

  /* Set the macro engine var. to NULL. */
  WndData( hDataWnd, WDM_SETMACROENGINE, (DWORD) NULL, NULL );
}



/***********************************
**
**  There are macros specified in
**  the project file's [CONFIG]
**  sections that are run each
**  time that the help file is loaded.
**
**  This routines get each of those
**  macro strings and executed them.
** 
**  Function returns:
**
**     Failure - FALSE.
**     Success - TRUE.
**
***********************************/
void __far __pascal ExecHelpFilesMacros( HWND hDataWnd )
{
  HTOPICDATA  hTopicData;
  FPTOPICDATA fpTopicData;
  
  FPHLPFILEINFO fpHelpFileInfo;

                                
  /* Get the main topic window's topic data. */
  hTopicData = (HTOPICDATA) WndData( hDataWnd, WDM_GETMAINTOPICDATA, 0, NULL );
            
  /* Lock the topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( hTopicData );  
  
  /* Lock help file data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( fpTopicData->hHelpFileInfo );
  
  /* Enumerate the macro system records. */
  EnumSysRecords( ExecHelpFilesMacrosProc, fpHelpFileInfo->SystemInfo.hMacroData, (LPARAM) (HWND __far *) &hDataWnd );

  /* Unlock help file data. */
  GlobalUnlock( fpTopicData->hHelpFileInfo );

  /* Unlock the topic data. */
  GlobalUnlock( hTopicData );  
}



/*************************************
*
*  EnumSysRecords() callback function 
*  used in ExecHelpFilesMacros().
*
*  Receives a macro system record and
*  runs the macro.
*                  
**************************************/
static BOOL CALLBACK ExecHelpFilesMacrosProc( FPSYSTEMREC fpSysRec, LPARAM lParam )
{
  HWND __far * hDataWndPtr;
  char __far * MacroStringPtr;
                                
  
  /* Get the handle to the main data window. */
  hDataWndPtr = (HWND __far *) lParam;
  
  /* Lock macro string data. */
  MacroStringPtr = (char __far *) GlobalLock( fpSysRec->hRecData );
    
  /* Execute the macro. */
  RunHelpMacro( *hDataWndPtr, *hDataWndPtr, MacroStringPtr );

  /* Unlock secondary window data. */
  GlobalUnlock( fpSysRec->hRecData );
  
  /* Keep enumerating. */
  return( TRUE );
}


/***********************************
**
**  Run a help macro. 
**
**  Returns:
**
**     Success - TRUE.
**     Failure - FALSE.
**
***********************************/
void __far __pascal RunHelpMacro
( 
  HWND hErrorWnd,
  HWND hDataWnd,
  char __far * MacroStringPtr
)
{
  HMACROENGINE hMacroEngine;
  MACRORETVAL  MacroRetValPtr;
  
  /* Get the macro engine's instance handle. */
  hMacroEngine = (HMACROENGINE) WndData( hDataWnd, WDM_GETMACROENGINE, 0, NULL );
            
  /* If we have a macro engine. */
  if( hMacroEngine != NULL )
  {
    ExecuteMacro( hErrorWnd, hMacroEngine, MacroStringPtr, &MacroRetValPtr );
  }
}



/******************************** MACRO CALL-BACK FUNCTIONS *********************************/ 

/***********************************
**
**  Shows the search dialog box. 
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK SearchMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Show the search dialog box. */
  ShowCurrHelpFilesKeywords( hDataWnd );
  
  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Show Back topic.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK BackMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Show the back topic. */
  ShowBackTopic( hDataWnd );
  
  /* No macro syntax errors. */
  return( TRUE );
}



/***********************************
**
**  Adds browse buttons to main window. 
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK BrowseButtonsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  
  char ButtonCaption[ MAX_BUTTON_TEXT ];
  HWND hButtonBarWnd = 0;
  BOOL bReturn;
    
  HWND  hDataWnd;


  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the button bar's window handle. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0, NULL );
            
  /* If there is not a prev. button already. */
  if( ! GetButtonWindow( hButtonBarWnd, PREV_BUTTON_ID) )
  {
    /*
    ** Create "Previous" button. 
    */
    LoadString( GetLibInst(), IDS_PREV_BUTTON, ButtonCaption, MAX_BUTTON_TEXT );
    bReturn = AddBarButton( GetLibInst(), hButtonBarWnd, ButtonCaption, PREV_BUTTON_ID, "Prev()" );
    if( bReturn == FALSE ) 
    {
      /* No macro syntax errors. */
      return( TRUE );
    }
      
    /* 
    ** Create "Next" button. 
    */
    LoadString( GetLibInst(), IDS_NEXT_BUTTON, ButtonCaption, MAX_BUTTON_TEXT );
    bReturn = AddBarButton( GetLibInst(), hButtonBarWnd, ButtonCaption, NEXT_BUTTON_ID, "Next()" );
    if( bReturn == FALSE ) 
    {
      /* No macro syntax errors. */
      return( TRUE );
    }
  
    /* Disable the buttons. */
    EnableWindow( GetButtonWindow( hButtonBarWnd, PREV_BUTTON_ID), FALSE );
    EnableWindow( GetButtonWindow( hButtonBarWnd, NEXT_BUTTON_ID), FALSE );

    /* 
    ** Tell main window to resize.  
    ** This will force all child windows to be
    ** resized and redrawn.
    */
    SendMessage( hDataWnd, WM_SIZE, 0, 0L );

    /* Set the buttons' status. */
    SetBrowseButtonsTopics( hDataWnd );
  }
  
  /* No macro syntax errors. */
  return( TRUE );
} 


/***********************************
**
**  Shows the contents topic. 
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK ContentsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Show the contents topic. */
  DoHelpContentsButton( hDataWnd );
  
  /* No macro syntax errors. */
  return( TRUE );
}



/***********************************
**
**  Shows the History window. 
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns: TRUE
**
***********************************/
BOOL CALLBACK HistoryMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Show the history window. */
  ShowHistory( hDataWnd );
  
  /* No macro syntax errors. */
  return( TRUE );
}



/***********************************
**
**  Shows the Pevious topic. 
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns: TRUE
**
***********************************/
BOOL CALLBACK PrevMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Shows the previous topic. */
  DoPrevButton( hDataWnd );
  
  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Shows the Next topic. 
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns: TRUE
**
***********************************/
BOOL CALLBACK NextMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Shows the previous topic. */
  DoNextButton( hDataWnd );
  
  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Shows the contents topic of the
**  specified help file.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK JumpContentsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  char szHelpFilePath[_MAX_PATH];
  
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the path argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szHelpFilePath, MACRO_STRING) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Shows the help file's Contents topic. */
  DoHelpContentsMsg( hDataWnd, szHelpFilePath );
  
  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Adds a button to the button bar.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK CreateButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  char szButtonID[ MAX_BUTTON_ID ];
  char szButtonText[ MAX_BUTTON_TEXT ];
  char szMacro[ MAX_MACRO_STRING ];
  
  HWND hButtonBarWnd;
 
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the button's ID argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szButtonID, MACRO_STRING) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the button's text argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szButtonText, MACRO_STRING) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the button's macro argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szMacro, MACRO_STRING) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the button bar's window handle. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0, NULL );

  /* Create the button. */
  if( AddBarButton( GetLibInst(), hButtonBarWnd, szButtonText, szButtonID, szMacro ) )
  {
    /* Enable the button. */
    EnableWindow( GetButtonWindow( hButtonBarWnd, szButtonID), TRUE );
  
    /* 
    ** Tell main window to resize.  
    ** This will force all child windows to be
    ** resized and redrawn.
    */
    SendMessage( hDataWnd, WM_SIZE, 0, 0L );
  }

  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Changes the macro assigned to 
**  a button on the button bar.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK ChangeButtonBindingMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  char szButtonID[ MAX_BUTTON_ID ];
  char szMacro[ MAX_MACRO_STRING ];
  
  HWND hButtonBarWnd;
 
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the button's ID argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szButtonID, MACRO_STRING) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the button's macro argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szMacro, MACRO_STRING) ) 
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the button bar's window handle. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0, NULL );

  /* Create the button. */
  ChangeButtonsMacro( hButtonBarWnd, szButtonID, szMacro );

  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Deletes a button from the button
**  bar.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK DestroyButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  char szButtonID[ MAX_BUTTON_ID ];
  
  HWND hButtonBarWnd;
 
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the button's ID argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szButtonID, MACRO_STRING) ) 
  {
    /* Macro syntax error. */
    return FALSE;
  }
  
  /* Get the button bar's window handle. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0, NULL );

  /* Delete the button. */
  if( DeleteBarButton( hButtonBarWnd, szButtonID ) )
  {
    /* 
    ** Tell main window to resize.  
    ** This will force all child windows to be
    ** resized and redrawn.
    */
    SendMessage( hDataWnd, WM_SIZE, 0, 0L );
  }

  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Disables a button in the button
**  bar.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK DisableButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  char szButtonID[ MAX_BUTTON_ID ];
  
  HWND hButtonBarWnd;
 
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the button's ID argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szButtonID, MACRO_STRING) ) 
  {
    /* Macro syntax error. */
    return FALSE;
  }
  
  /* Get the button bar's window handle. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0, NULL );

  /* Disable the button. */
  EnableButton( hButtonBarWnd, szButtonID, FALSE );

  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Enables a button in the button
**  bar.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK EnableButtonMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  char szButtonID[ MAX_BUTTON_ID ];
  
  HWND hButtonBarWnd;
 
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the button's ID argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szButtonID, MACRO_STRING) ) 
  {
    /* Macro syntax error. */
    return FALSE;
  }
  
  /* Get the button bar's window handle. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0, NULL );

  /* Enable the button. */
  EnableButton( hButtonBarWnd, szButtonID, TRUE );

  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Sets a new topic for the Contents
**  button.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK SetContentsMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;

  DWORD dwContext;
  char  szHelpFilePath[_MAX_PATH];
  
  
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the help file path argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szHelpFilePath, MACRO_STRING) ) 
  {
    /* Macro syntax error. */
    return FALSE;
  }
  
  /* Get the button's ID argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, &dwContext, MACRO_UNSIGNEDLONGINT) ) 
  {
    /* Macro syntax error. */
    return FALSE;
  }
  
  /* Set the number of the contents topic. */
  DoHelpSetContentsMsg( hDataWnd, szHelpFilePath, (long int) dwContext );
  
  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Adds a new keyboard accelerator.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK AddAcceleratorMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  WORD wKey, wShiftState;
  char szMacro[ MAX_MACRO_STRING ];
  
   
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the accelerator's key. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, &wKey, MACRO_UNSIGNEDSHORTINT) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the accelerator's shift state. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, &wShiftState, MACRO_UNSIGNEDSHORTINT) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the accelerator's macro argument. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, szMacro, MACRO_STRING) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Create the new accelerator. */
  AddAccelerator( hDataWnd, wKey, wShiftState, (LPSTR) szMacro );

  /* No macro syntax errors. */
  return( TRUE );
}


/***********************************
**
**  Removes a keyboard accelerator.
**
**  Macro returns:
**
**     No macro return value.
**
**  Function returns:
**
**      TRUE: No parsing error.
**     FALSE: Parsing error.
**
***********************************/
BOOL CALLBACK RemoveAcceleratorMacroProc
( 
  HMACROENGINE hMacroEngine, 
  HGLOBAL hAppData, 
  HGLOBAL hMacroData, 
  MACRORETVAL __far * MacroRetValPtr 
)
{
  HLPMACRODATAPTR HelpMacroDataPtr;
  HWND  hDataWnd;
  
  WORD wKey, wShiftState;
  
   
  /* Lock app. data. */
  HelpMacroDataPtr = ( HLPMACRODATAPTR ) GlobalLock( hAppData );
      
  /* Get data window's handle. */
  hDataWnd = HelpMacroDataPtr->hDataWnd;

  /* Unlock app. data. */
  GlobalUnlock( hAppData );
      
  /* Get the accelerator's key. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, &wKey, MACRO_UNSIGNEDSHORTINT) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Get the accelerator's shift state. */
  if( ! GetNextMacroArg( hDataWnd, hMacroEngine, &wShiftState, MACRO_UNSIGNEDSHORTINT) )
  {
    /* Macro syntax error. */
    return( FALSE );
  }
  
  /* Delete the accelerator. */
  RemoveAccelerator( hDataWnd, wKey, wShiftState );

  /* No macro syntax errors. */
  return( TRUE );
}
