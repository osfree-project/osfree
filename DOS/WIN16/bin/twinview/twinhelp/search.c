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
**                                   HISTORY ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <string.h>
#include <ctype.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "hlpfile.h"
#include "display.h"
#include "wnddata.h"
#include "globals.h"
#include "twhlprc.h"
#include "search.h"


/***********************************
**
**  Private Defines
**
***********************************/

#define EDITKEYWORDOLDPROC "EditKeywordOldProc"

/* 
** Data stored for each search window
** managed by the DLL.
*/
typedef struct tagSEARCHWNDDATA
{
  char       chKey;                 /* Keyword MULTIKEY character to use. */
  char __far * KeywordPtr;          /* Keyword to initially use. */
  HWND       hDataWnd;              /* Main help window used for storage of general data. */
  HGLOBAL    hHelpFileInfo;         /* Help file containing the keyword/topic data. */
  DWORD      dwKWDATAOffset;        /* Offset of keyword's topics in KWDATA table. */
  BOOL       bInTopicList;          /* Are we in the topic's list. */
}
SEARCHWNDDATA;
typedef SEARCHWNDDATA __far * FPSEARCHWNDDATA;
typedef HGLOBAL HSEARCHWNDDATA;


/* 
** Data stored for each keyword edit
** control in the dialog box.
*/
typedef struct tagEDITSEARCHWNDDATA
{
  HWND    hDlgWnd;     /* Handle of dialog box containing edit control. */
  WNDPROC OldWndProc;  /* Edit control's old window procedure before subclassing. */
}
EDITSEARCHWNDDATA;
typedef EDITSEARCHWNDDATA __far * EDITSEARCHWNDDATAPTR;


/***********************************
**
**  Private Function Prototypes
**
***********************************/

BOOL CALLBACK __export SearchDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
static BOOL __far __pascal GotoSearchTopic( FPSEARCHWNDDATA fpSearchWndData, DWORD dwTopicCharOffset );
static void __far __pascal CloseUpDialog( HWND hWnd );
static BOOL __far __pascal ShowSearchTopics( HWND hWnd,  HGLOBAL hHelpFileInfo, DWORD dwKeyWordIndex, DWORD __far * dwKWDATAOffset );
static void __far __pascal ShowEditKeyword( HWND hWnd, WORD wKeyWordIndex );
static void __far __pascal SelectClosestKeyword( HWND hWnd );
LRESULT CALLBACK __export EditKeywordWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
static void __far __pascal SelectFirstTopic( HWND hWnd );
static BOOL __far __pascal ShowTopics( HWND hWnd, DWORD dwSelected );

                            
/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/

/**********************************************************
**  
**  HELP_KEY message
**
**  Looks for the specified keyword in the 'K' keyword 
**  list.
**
**  If the keyword is found and it only has one topic
**  associated with it, the topic is displayed.
**
**  If there is more than one match, displays
**  the Search dialog box and the the topics. 
**  
***********************************************************/
BOOL __far __pascal DoHelpKey_PartialKeyMsg( HWND hWnd, UINT SrchMsg, HGLOBAL hHelpParams, FPHELPPARAMS fpHelpParams )
{
  SEARCHWNDDATA SearchWndData;

  HGLOBAL hTopicData;           /* Handle to the topic's elements list. */                
  HGLOBAL hHelpFileInfo;        /* Handle to the help file's general info. */
  DWORD   dwTopicCharOffset;    /* Location of topic data. */
  WORD    wFoundTopics;         /* Number of topics associated with the keyword. */

  char __far * KeywordPtr;      /* Pointer to the keyword string. */
  
  short int nResult;

  HWND hMainTopicWnd;           /* Window handle for the main topic window. */
                      
  BOOL bDlgAlreadyDisplayed;    /* Is the dialog already being displayed. */
  
                      
  /* Open and load the help file's info. */
  hHelpFileInfo = OpenHelpFile( hWnd, fpHelpParams->Path );
  if( hHelpFileInfo == NULL )
  {
    /* Unlock handle to message stucture. */
    GlobalUnlock( hHelpParams );
        
    /* Failure. */
    return( FALSE );
  }
 
  /* Load the keyword data. */
  if( ! LoadKeywordData( hWnd, 'K', hHelpFileInfo ) )
  {
    /* Unlock handle to message stucture. */
    GlobalUnlock( hHelpParams );
        
    /* Failed. */
    return( FALSE );
  }

  /* Get a pointer to the keyword string. */
  KeywordPtr =  (char __far *) ( (BYTE __far *)fpHelpParams + fpHelpParams->ExtraDataOffset );

  /* Get the number of the default topic. */
  wFoundTopics = GetKeyTopic( hWnd, hHelpFileInfo, KeywordPtr, &dwTopicCharOffset );

  /* If no topics found and HELP_KEY. */
  if( wFoundTopics == 0 && SrchMsg == HELP_KEY )
  {
    /* Free any file info. that we loaded into memory. */
    FreeKeywordData( hHelpFileInfo );
    
    /* Unlock handle to message stucture. */
    GlobalUnlock( hHelpParams );
        
    /* Failed. */
    return( FALSE );
  }
    
  /* If one topic found. */
  else if( wFoundTopics == 1 )
  { 
    /* Free any file info. that we loaded into memory. */
    FreeKeywordData( hHelpFileInfo );

    /* Get the topic's data. */
    hTopicData = LockTopicData( hWnd, fpHelpParams->Path, dwTopicCharOffset );
    if( hTopicData == NULL )
    {
      /* Unlock handle to message stucture. */
      GlobalUnlock( hHelpParams );
        
      /* Failure. */
      return( FALSE );
    }
  
    /* Get the window handle for the main topic window. */
    hMainTopicWnd = (HWND) WndData( hWnd, WDM_GETMAINTOPICHWND, 0, NULL );
             
    /* Inform main topic window of new topic to load. */
    SendShowNewTopicMsg( hMainTopicWnd, hTopicData, TRUE, 0 ); 

    /* Unlock handle to message stucture. */
    GlobalUnlock( hHelpParams );
        
    /* Success. */
    return( TRUE );
  }
  

  /* Unlock handle to message stucture - don't need it anymore. */
  GlobalUnlock( hHelpParams );
        
  /* 
  ** We are current in a SendMessage() call.  We can't open up the 
  ** Search dialog box until we reply to the call and are out of it.
  */
  ReplyMessage( 0 );
  
  /* 
  ** More than one topic found or 
  **
  ** No topic's found and HELP_PARTIALKEY
  */

  /* Is the Search dialog box already displayed. */
  bDlgAlreadyDisplayed = (BOOL) WndData( hWnd, WDM_GETSEARCHDLGDISPLAYED, 0, NULL );
             
  /* The Search dialog box is already displayed. */
  if( bDlgAlreadyDisplayed == TRUE )
  {
    /* Free any file info. that we loaded into memory. */
    FreeKeywordData( hHelpFileInfo );
    
    /* Failed. */
    return( FALSE );
  }

  /* Get the window handle for the main topic window. */
  hMainTopicWnd = (HWND) WndData( hWnd, WDM_GETMAINTOPICHWND, 0, NULL );
             
  /* Init. the window input data. */
  SearchWndData.hDataWnd      = hWnd;
  SearchWndData.chKey         = 'K';
  SearchWndData.hHelpFileInfo = hHelpFileInfo;
        
  /* HELP_KEY message. */
  if( SrchMsg == HELP_KEY )
  {
    /* Use keyword initially. */
    SearchWndData.KeywordPtr = KeywordPtr;
  }
  /* HELP_PARTIALKEY message. */
  else
  {
    /* Use no keyword initially. */
    SearchWndData.KeywordPtr = NULL;
  }

  /* The Search dialog box is being displayed. */
  WndData( hWnd, WDM_SETSEARCHDLGDISPLAYED, (DWORD) TRUE, NULL );
             
  /* Show search dialog. */
  nResult = DialogBoxParam( GetLibInst(), "DLG_SEARCH", hMainTopicWnd, SearchDlgProc, (LPARAM) (FPSEARCHWNDDATA) &SearchWndData );
  
  /* The Search dialog box is not being displayed. */
  WndData( hWnd, WDM_SETSEARCHDLGDISPLAYED, (DWORD) FALSE, NULL );

  /* Free any file info. that we loaded into memory. */
  FreeKeywordData( hHelpFileInfo );
  
  /* No error. */
  if( nResult == TRUE )
  {
    /* Success. */
    return( TRUE );
  }
  else  /* FALSE or -1. */
  {
    /* Failure. */
    return( FALSE );
  }
}
 

/**********************************************************
**  
**  HELP_MULTKEY message
**
**  Looks for the specified keyword in the specified
**  keyword list.
**
**  If the keyword is found, the first topic
**  associated with it is displayed.
**
***********************************************************/
BOOL __far __pascal DoHelpMultiKeyMsg( HWND hWnd, FPHELPPARAMS fpHelpParams )
{
  MULTIKEYHELP __far * MultiKeyPtr;
  
  HGLOBAL hTopicData;           /* Handle to the topic's elements list. */                
  HGLOBAL hHelpFileInfo;        /* Handle to the help file's general info. */
  DWORD   dwTopicCharOffset;    /* Location of topic data. */
  WORD    wFoundTopics;         /* Number of topics associated with the keyword. */
  HWND hMainTopicWnd;           /* Window handle for the main topic window. */
                      
                      
  /* Open and load the help file's info. */
  hHelpFileInfo = OpenHelpFile( hWnd, fpHelpParams->Path );
  if( hHelpFileInfo == NULL )
  {
    /* Failure. */
    return( FALSE );
  }
 
  /* Get multikey data. */
  MultiKeyPtr = ( MULTIKEYHELP __far * ) ( ( BYTE __far * ) fpHelpParams + fpHelpParams->ExtraDataOffset );

  /* Load the keyword data. */
  if( ! LoadKeywordData( hWnd, MultiKeyPtr->mkKeylist, hHelpFileInfo ) )
  {
    /* Failed. */
    return( FALSE );
  }

  /* Get the number of the default topic. */
  wFoundTopics = GetKeyTopic( hWnd, hHelpFileInfo, MultiKeyPtr->szKeyphrase, &dwTopicCharOffset );

  /* If no topics found. */
  if( wFoundTopics == 0 )
  {
    /* Free any file info. that we loaded into memory. */
    FreeKeywordData( hHelpFileInfo );
    
    /* Failed. */
    return( FALSE );
  }
    
  /*
  ** If one or more topics found. 
  */

  /* Free any file info. that we loaded into memory. */
  FreeKeywordData( hHelpFileInfo );

  /* Get the topic's data. */
  hTopicData = LockTopicData( hWnd, fpHelpParams->Path, dwTopicCharOffset );
  if( hTopicData == NULL )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Get the window handle for the main topic window. */
  hMainTopicWnd = (HWND) WndData( hWnd, WDM_GETMAINTOPICHWND, 0, NULL );
             
  /* Inform main topic window of new topic to load. */
  SendShowNewTopicMsg( hMainTopicWnd, hTopicData, TRUE, 0 ); 

  /* Success. */
  return( TRUE );
}



/**********************************************************
**  
**  Load in the appropriate keyword table and displays
**  the Search dialog box. 
**  
***********************************************************/
BOOL __far __pascal ShowCurrHelpFilesKeywords( HWND hDataWnd )
{
  HGLOBAL hDisplayInfo;         /* Handle to the topic window's display info. */
  FPDISPLAYINFO fpDisplayInfo;  /* Pointer to the topic window's display info. */
  FPTOPICDATA fpTopicData;      /* Pointer to the topic's window's topic data. */
  
  HWND    hMainTopicWnd;
  SEARCHWNDDATA SearchWndData;
  
  BOOL bDlgAlreadyDisplayed;
  
  
  /* Is the Search dialog box already displayed. */
  bDlgAlreadyDisplayed = (BOOL) WndData( hDataWnd, WDM_GETSEARCHDLGDISPLAYED, 0, NULL );
             
  /* The Search dialog box is already displayed. */
  if( bDlgAlreadyDisplayed == TRUE )
  {
    /* Failed. */
    return( FALSE );
  }

  /* Get the window handle for the main topic window. */
  hMainTopicWnd = (HWND) WndData( hDataWnd, WDM_GETMAINTOPICHWND, 0, NULL );

  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hMainTopicWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* No file info loaded. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    /* Failed but not due to error. */
    return TRUE;
  }
           
  /* Lock the topic info. */
  fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );

  /* Load the keyword data. */
  if( ! LoadKeywordData( hMainTopicWnd, 'K', fpTopicData->hHelpFileInfo ) )
  {
    /* Unlock the topic info. */
    GlobalUnlock( fpDisplayInfo->hTopicData );
    
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
  
    /* Failed but not due to error. */
    return TRUE;
  }

  /* Init. the window input data. */
  SearchWndData.hDataWnd      = hDataWnd;
  SearchWndData.chKey         = 'K';
  SearchWndData.KeywordPtr    = NULL;
  SearchWndData.hHelpFileInfo = fpTopicData->hHelpFileInfo;
      
  /* Unlock the topic info. */
  GlobalUnlock( fpDisplayInfo->hTopicData );
    
  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
    
  /* The Search dialog box is being displayed. */
  WndData( hDataWnd, WDM_SETSEARCHDLGDISPLAYED, (DWORD) TRUE, NULL );
             
  /* Show search dialog. */
  DialogBoxParam( GetLibInst(), "DLG_SEARCH", hMainTopicWnd, SearchDlgProc, (LPARAM) (FPSEARCHWNDDATA) &SearchWndData );

  /* The Search dialog box is not being displayed. */
  WndData( hDataWnd, WDM_SETSEARCHDLGDISPLAYED, (DWORD) FALSE, NULL );
             
  /* Free any file info. that we loaded into memory. */
  FreeKeywordData( SearchWndData.hHelpFileInfo );

  /* Success. */
  return( TRUE );
}



/***********************************************************************************
**
**                              Private Functions
**
************************************************************************************/

BOOL CALLBACK __export SearchDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
  HSEARCHWNDDATA  hSearchWndData;
  FPSEARCHWNDDATA fpSearchWndData;
  
  FPSEARCHWNDDATA fpWndDataIn;
  
  DWORD dwSelected;
  DWORD dwTopicOffset;
  
  BOOL  bSuccess;
  
  HGLOBAL hEditSearchWndData;
  EDITSEARCHWNDDATAPTR EditSearchWndDataPtr;

  switch( Msg ) 
  {
    /* Window initialization. */
    case WM_INITDIALOG:
    {  
      /* Allocate memory for edit control window proc. pointer. */
      if( !GlobalAllocMem( hWnd, &hEditSearchWndData, sizeof(EDITSEARCHWNDDATA) ) )
      {
        /* Failure. */
        return( -1 );
      }
  
      /* Subclass edit control and store the dialog box's window handle. */
      EditSearchWndDataPtr = ( EDITSEARCHWNDDATAPTR ) GlobalLock( hEditSearchWndData );
      EditSearchWndDataPtr->OldWndProc = (WNDPROC) GetWindowLong( GetDlgItem( hWnd, IDC_EDITKEYWORD ), GWL_WNDPROC );
      EditSearchWndDataPtr->hDlgWnd = hWnd;
      GlobalUnlock( hEditSearchWndData );
      SetProp( GetDlgItem( hWnd, IDC_EDITKEYWORD ), EDITKEYWORDOLDPROC, hEditSearchWndData );
      SetWindowLong( GetDlgItem( hWnd, IDC_EDITKEYWORD ), GWL_WNDPROC, (LONG) EditKeywordWndProc );

      /* Allocate memory for window data. */
      if( !GlobalAllocMem( hWnd, &hSearchWndData, sizeof(SEARCHWNDDATA) ) )
      {
        /* Free edit control data. */
        GlobalFree( hEditSearchWndData );
        
        /* Failure. */
        return( -1 );
      }
  
      /* Lock window data. */
      fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );
      
      /* Save topic window data passed in. */
      fpWndDataIn = ( FPSEARCHWNDDATA ) lParam;
      fpSearchWndData->chKey         = fpWndDataIn->chKey;
      fpSearchWndData->KeywordPtr    = fpWndDataIn->KeywordPtr;
      fpSearchWndData->hDataWnd      = fpWndDataIn->hDataWnd;
      fpSearchWndData->hHelpFileInfo = fpWndDataIn->hHelpFileInfo;
      
      /* Not in the topic's list box by default. */
      fpSearchWndData->bInTopicList = FALSE;
      
      /* Show keywords. */
      if( !ShowKeywords( GetDlgItem( hWnd, IDC_KEYWORDS ), fpSearchWndData->chKey, fpSearchWndData->hHelpFileInfo ) )
      {
        /* Unlock window data. */
        GlobalUnlock( hSearchWndData );
      
        /* Perform closing operations. */
        CloseUpDialog( hWnd );

        /* Failure. */
        return( -1 );
      }
      
      /* If we do have a keyword we're looking for. */
      if( fpSearchWndData->KeywordPtr != NULL )
      {
        /* Select the keyword in list. */
        dwSelected = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_SELECTSTRING, 
                     (WPARAM) -1, (LPARAM) (LPCSTR) fpSearchWndData->KeywordPtr );

        /* If we failed to find a match. */
        if( dwSelected == LB_ERR )
        {  
          /* Which keyword to select first? */
          if( fpSearchWndData->chKey == 'K' )
          {
            /* 
            ** If we are displaying the default keyword list,
            ** get last index. 
            */
            dwSelected = GetLastKeywordIndex( fpSearchWndData->hHelpFileInfo );
          }
          else
          {
            /* Select first item in combo box. */
            dwSelected = 0;
          }
  
          /* Select keyword in list. */
          dwSelected = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_SETCURSEL, (WPARAM) dwSelected, 0L );
        }
      }
      
      /* If we don't have a keyword. */
      else
      {      
        /* Which keyword to select first? */
        if( fpSearchWndData->chKey == 'K' )
        {
          /* 
          ** If we are displaying the default keyword list,
          ** get last index. 
          */
          dwSelected = GetLastKeywordIndex( fpSearchWndData->hHelpFileInfo );
        }
        else
        {
          /* Select first item in combo box. */
          dwSelected = 0;
        }

        /* Select keyword in list. */
        dwSelected = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_SETCURSEL, (WPARAM) dwSelected, 0L );
      }
      
      /* If there is an item, show item's topics. */
      if( dwSelected != LB_ERR )
      {  
        /* Show selected keyword in edit control. */
        ShowEditKeyword( hWnd, (WORD) dwSelected );

        /* Select all the text in the edit control. */
        SendMessage( GetDlgItem( hWnd, IDC_EDITKEYWORD ), EM_SETSEL, 1, MAKELPARAM( 0, -1) );
        
        /* Show the topics assoicated with the keyword. */
        if( !ShowSearchTopics( hWnd, fpSearchWndData->hHelpFileInfo, dwSelected, &(fpSearchWndData)->dwKWDATAOffset ) )
        {
          /* Perform closing operations. */
          CloseUpDialog( hWnd );
                  
          /* Close dialog box window. */
          return( -1 );
        }
      }
      
      /* Unlock window data. */
      GlobalUnlock( hSearchWndData );
      
      /* Save handle to the window data. */
      SetWindowLong( hWnd, DWL_USER, (long) (WORD) hSearchWndData ); 

      /* Success. */
      return( TRUE );
    }
    

    case WM_COMMAND:
    {
      switch( wParam ) 
      {
        case IDC_DEFBUTTON:
        {
          /* Save handle to the window data. */
          hSearchWndData = ( HSEARCHWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 
        
          /* Lock window data. */
          fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );

          /* If we are in the topic's list box. */
          if( fpSearchWndData->bInTopicList == TRUE )
          {
            /* Unlock window data. */
            GlobalUnlock( hSearchWndData );

            /* Act as if the GOTO button was pressed. */
            PostMessage( hWnd, WM_COMMAND, IDC_SEARCHGOTO, 0L );
          }
          else
          {
            /* Unlock window data. */
            GlobalUnlock( hSearchWndData );

            /* Select first topic. */
            SelectFirstTopic( hWnd );
          }
          
          /* Processed message. */
          return TRUE;
        }
        

        case IDCANCEL:
        {
          /* Perform closing operations. */
          CloseUpDialog( hWnd );
    
          /* Close dialog box window. */
          EndDialog( hWnd, FALSE );
        }
        

        case IDC_KEYWORDS:
        {
          /* New keyword selected. */
          if( HIWORD(lParam) == LBN_SELCHANGE ) 
          { 
            /* Get selected keyword. */
            dwSelected = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_GETCURSEL, 0, 0L );

            /* If there is a keyword to show. */
            if( dwSelected != LB_ERR )
            {
              /* Show the topics for the seelcted keyword. */
              if( ! ShowTopics( hWnd, dwSelected ) )
              {
                /* Perform closing operations. */
                CloseUpDialog( hWnd );
                          
                /* Close dialog box window. */
                EndDialog( hWnd, FALSE );
              }

              /* Show selected keyword in edit control. */
              ShowEditKeyword( hWnd, (WORD) dwSelected );
            }
          }
          
          else if( HIWORD(lParam) == LBN_DBLCLK ) 
          {
            /* Select first topic. */
            SelectFirstTopic( hWnd );
          }

          /* Processed message. */
          return TRUE;
        }



        case IDC_SEARCHTOPIC:
        {
          switch( HIWORD(lParam) )
          {
            /* Getting focus. */
            case LBN_SETFOCUS:
            {
              /* Save handle to the window data. */
              hSearchWndData = ( HSEARCHWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 
        
              /* Lock window data. */
              fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );

              /* In the topic's list box by default. */
              fpSearchWndData->bInTopicList = TRUE;
              
              /* Unlock window data. */
              GlobalUnlock( hSearchWndData );

              break;
            }

            /* Losing focus. */
            case LBN_KILLFOCUS:
            {
              /* Save handle to the window data. */
              hSearchWndData = ( HSEARCHWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 
        
              /* No window data - probably exiting. */
              if( hSearchWndData == 0 ) 
              {
                /* Processed message. */
                return TRUE;
              }
          
              /* Lock window data. */
              fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );

              /* Not the topic's list box by default. */
              fpSearchWndData->bInTopicList = FALSE;
              
              /* Unlock window data. */
              GlobalUnlock( hSearchWndData );

              break;
            }

            case LBN_DBLCLK:
            {
              /* Goto topic. */
              PostMessage( hWnd, WM_COMMAND, IDC_SEARCHGOTO, 0L );
              
              break;
            }

            case LBN_SELCHANGE:
            {
              /* Enable goto button. */
              EnableWindow( GetDlgItem( hWnd, IDC_SEARCHGOTO ), TRUE );
              
              break;
            }
          }

          /* Processed message. */
          return TRUE;
        }


        case IDC_SEARCHGOTO:
        {
          dwSelected = SendMessage( GetDlgItem( hWnd, IDC_SEARCHTOPIC ), LB_GETCURSEL, 0, 0L );
          if( dwSelected != LB_ERR )
          {
            /* Save handle to the window data. */
            hSearchWndData = ( HSEARCHWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 
        
            /* Lock window data. */
            fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );
        
            /* Get topic's offset. */
            dwTopicOffset = GetKeywordTopicOffset( fpSearchWndData->hHelpFileInfo, fpSearchWndData->dwKWDATAOffset, dwSelected );

            /* Goto topic. */
            bSuccess = GotoSearchTopic( fpSearchWndData, dwTopicOffset );
            
            /* Unlock window data. */
            GlobalUnlock( hSearchWndData );

            /* If move to new topic is OK. */
            if( bSuccess )
            {
              /* Perform closing operations. */
              CloseUpDialog( hWnd );
        
              /* Close dialog box window. */
              EndDialog( hWnd, TRUE );
            }
          }

          /* Processed message. */
          return TRUE;
        }
      }
      
      /* Processed message. */
      return TRUE;
    }
  }

  /* Message not processed. */
  return FALSE;
}


static void __far __pascal SelectFirstTopic( HWND hWnd )
{
  DWORD dwReturn;
  
  /* Select first topic associated with keyword. */
  dwReturn = SendMessage( GetDlgItem( hWnd, IDC_SEARCHTOPIC ), LB_SETCURSEL, (WPARAM) 0, 0L );

  /* No items in the list box. */
  if( dwReturn != LB_ERR )
  {  
    /* Enable goto button. */
    EnableWindow( GetDlgItem( hWnd, IDC_SEARCHGOTO ), TRUE );
    
    /* SetFocus to list box. */
    SetFocus( GetDlgItem( hWnd, IDC_SEARCHTOPIC ) );
  }
}


LRESULT CALLBACK __export EditKeywordWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
  HGLOBAL hEditSearchWndData;
  EDITSEARCHWNDDATAPTR EditSearchWndDataPtr;
  WNDPROC OldEditWndProc;
  
  HWND hDlgWnd;
  
  LRESULT lResult;
  
  
  switch( Msg ) 
  {
    /* Receive all input. */
    case WM_GETDLGCODE:
    {
      return( DLGC_WANTCHARS );
    }
    
    /* Character input. */
    case WM_CHAR:
    {
      /* Get window of dialog box containing the edit control. */
      hEditSearchWndData = (HGLOBAL) GetProp( hWnd, EDITKEYWORDOLDPROC );
      EditSearchWndDataPtr = ( EDITSEARCHWNDDATAPTR ) GlobalLock( hEditSearchWndData );
      hDlgWnd = EditSearchWndDataPtr->hDlgWnd;
      OldEditWndProc = EditSearchWndDataPtr->OldWndProc;
      GlobalUnlock( hEditSearchWndData );

      /* Pass to edit control's old WndProc(). */
      lResult = CallWindowProc( OldEditWndProc, hWnd, Msg, wParam, lParam );
  
      /* Show selected keyword in edit control. */
      SelectClosestKeyword( hDlgWnd );
        
      /* Return returned value. */
      return( lResult );
    }
  }
 
  /* Get edit control's old WndProc(). */
  hEditSearchWndData = (HGLOBAL) GetProp( hWnd, EDITKEYWORDOLDPROC );
  EditSearchWndDataPtr = ( EDITSEARCHWNDDATAPTR ) GlobalLock( hEditSearchWndData );
  OldEditWndProc = EditSearchWndDataPtr->OldWndProc;
  GlobalUnlock( hEditSearchWndData );

  /* Pass to edit control's old WndProc(). */
  lResult = CallWindowProc( OldEditWndProc, hWnd, Msg, wParam, lParam );
  
  /* Return returned value. */
  return( lResult );
}                                                                                  


static void __far __pascal SelectClosestKeyword( HWND hWnd )
{
  char szKeyWord[MAX_KEYWORD_STRING];
  char szLBString[MAX_KEYWORD_STRING];
  DWORD dwCurrSel, dwReturn;
  short int nChars, nCompareVal;
  

  /* Get text from edit control. */
  SendMessage( GetDlgItem( hWnd, IDC_EDITKEYWORD ), WM_GETTEXT, MAX_KEYWORD_STRING, (LPARAM) szKeyWord );

  /* Default index. */
  dwCurrSel = 0;
  
  /* Get first text from list box. */
  dwReturn = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_GETTEXT, (WPARAM) dwCurrSel, (LPARAM) szLBString );
  
  /* No items in the list box. */
  if( dwReturn == LB_ERR ) return;

  /* Find correct item in list box to highlight. */
  while( dwReturn != LB_ERR )
  {
    /* Compare strings. */
    nChars = _fstrlen(szKeyWord) / sizeof(char);
    nCompareVal = _fstrnicmp( szLBString, szKeyWord, nChars );
  
    /* Found string or past where string could be. */
    if( nCompareVal == 0 || nCompareVal > 0 ) break; 
    
    /* Get next string in list box. */
    dwCurrSel++;
    
    /* Get next text from list box. */
    dwReturn = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_GETTEXT, (WPARAM) dwCurrSel, (LPARAM) szLBString );
  }
  
  /* Past the last selection - use last selection. */
  if( dwReturn == LB_ERR ) dwCurrSel--;
  
  /* Select keyword. */
  SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_SETCURSEL, (WPARAM) dwCurrSel, 0L );

  /* Scroll list so current selection is at top. */
  SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_SETTOPINDEX, (WPARAM) dwCurrSel, 0L );

  /* 
  ** Show the topics for the seelcted keyword. 
  ** Don't update the edit control.
  */
  ShowTopics( hWnd, dwCurrSel );
}


static BOOL __far __pascal ShowTopics( HWND hWnd, DWORD dwSelected )
{
  HSEARCHWNDDATA  hSearchWndData;
  FPSEARCHWNDDATA fpSearchWndData;

  BOOL bReturn;
  
  
  /* Save handle to the window data. */
  hSearchWndData = ( HSEARCHWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 
        
  /* Lock window data. */
  fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );

  /* Show the topics assoicated with the keyword. */
  bReturn = ShowSearchTopics( hWnd, fpSearchWndData->hHelpFileInfo, dwSelected, &(fpSearchWndData)->dwKWDATAOffset );

  /* Unlock window data. */
  GlobalUnlock( hSearchWndData );

  /* Return success or failure. */
  return( bReturn );
}


static void __far __pascal ShowEditKeyword( HWND hWnd, WORD wKeyWordIndex )
{
  char szKeyWord[MAX_KEYWORD_STRING];
  
  /* If there is a selection. */
  if( wKeyWordIndex != LB_ERR )
  {
    /* Get text from list box selection. */
    SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_GETTEXT, wKeyWordIndex, (LPARAM) szKeyWord );

    /* Set text in edit control. */
    SendMessage( GetDlgItem( hWnd, IDC_EDITKEYWORD ), WM_SETTEXT, 0, (LPARAM) szKeyWord );
  }
}


static BOOL __far __pascal ShowSearchTopics( HWND hWnd,  HGLOBAL hHelpFileInfo, DWORD dwKeyWordIndex, DWORD __far * dwKWDATAOffset )
{
  /* Clear items in the topics list box. */
  SendMessage( GetDlgItem( hWnd, IDC_SEARCHTOPIC ), LB_RESETCONTENT, 0, 0L );
                
  /* Show the topics for the selected keyword. */
  if( !ShowKeywordTopics( GetDlgItem( hWnd, IDC_SEARCHTOPIC ), dwKeyWordIndex, hHelpFileInfo, dwKWDATAOffset ) )
  {
    /* Failure. */
    return( FALSE );
  }
    
  /* Disable goto button. */
  EnableWindow( GetDlgItem( hWnd, IDC_SEARCHGOTO ), FALSE );

  /* Success. */
  return( TRUE );
}



static void __far __pascal CloseUpDialog( HWND hWnd )
{
  HSEARCHWNDDATA  hSearchWndData;
  FPSEARCHWNDDATA fpSearchWndData;

  HGLOBAL hEditSearchWndData;
  EDITSEARCHWNDDATAPTR EditSearchWndDataPtr;
  
  DWORD dwSelected;

  /* Save handle to the window data. */
  hSearchWndData = ( HSEARCHWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 

  /* Lock window data. */
  fpSearchWndData = ( FPSEARCHWNDDATA ) GlobalLock( hSearchWndData );
      
  /* If we are displaying the default keyword list. */
  if( fpSearchWndData->chKey == 'K' )
  {
    /* Get selected keyword. */
    dwSelected = SendMessage( GetDlgItem( hWnd, IDC_KEYWORDS ), LB_GETCURSEL, 0, 0L );

    if( dwSelected != LB_ERR )
    {
      /* Save last index. */
      SetLastKeywordIndex( fpSearchWndData->hHelpFileInfo, dwSelected );
    }
  }

  /* Unlock window data. */
  GlobalUnlock( hSearchWndData );
      
  /* Free window data. */
  GlobalFree( hSearchWndData );
      
  /* Save handle to the window data. */
  SetWindowLong( hWnd, DWL_USER, (long ) 0 ); 

  /* Restore the edit control's window procedure. */
  hEditSearchWndData = (HGLOBAL) GetProp( GetDlgItem( hWnd, IDC_EDITKEYWORD ), EDITKEYWORDOLDPROC );
  EditSearchWndDataPtr = ( EDITSEARCHWNDDATAPTR ) GlobalLock( hEditSearchWndData );
  SetWindowLong( GetDlgItem( hWnd, IDC_EDITKEYWORD ), GWL_WNDPROC, (LONG) EditSearchWndDataPtr->OldWndProc );
  GlobalUnlock( hEditSearchWndData );
  GlobalFree( hEditSearchWndData );
}


static BOOL __far __pascal GotoSearchTopic( FPSEARCHWNDDATA fpSearchWndData, DWORD dwTopicCharOffset )
{
  FPHLPFILEINFO fpHelpFileInfo;    /* Pointer to new help file info.      */
  HGLOBAL hTopicData;              /* Handle to the topic's elements list. */                
  HWND   hMainTopicWnd;            /* Window handle for the main topic window. */
  
  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( fpSearchWndData->hHelpFileInfo );

  /* 
  ** Get the topic's data. 
  */

  hTopicData = LockTopicData( fpSearchWndData->hDataWnd, fpHelpFileInfo->szFilePath, dwTopicCharOffset );
  if( hTopicData == NULL )
  {
    /* Unlock help file info. */
    GlobalUnlock( fpSearchWndData->hHelpFileInfo );

    /* Failure. */
    return FALSE;
  }

  /* Unlock help file info. */
  GlobalUnlock( fpSearchWndData->hHelpFileInfo );
  
  /* Get the window handle for the main topic window. */
  hMainTopicWnd = (HWND) WndData( fpSearchWndData->hDataWnd, WDM_GETMAINTOPICHWND, 0, NULL );

  /* Inform main topic window of new topic to load. */
  SendShowNewTopicMsg( hMainTopicWnd, hTopicData, TRUE, 0 ); 

  /* Success. */
  return TRUE;
}

