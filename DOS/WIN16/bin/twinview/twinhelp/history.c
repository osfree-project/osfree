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


/***********************************
**
**  App. Includes
**
***********************************/
#include "history.h"
#include "winmem.h"
#include "globals.h"
#include "msgbox.h"
#include "twhlprc.h"
#include "twinhelp.h"
#include "inifile.h"
#include "filebuff.h"
#include "hlpfile.h"
#include "wnddata.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* Handle to the top of the history list containing 
** HISTORYREC structures. 
*/
typedef HGLOBAL HHISTORYLIST;


/* Record kept for each entry in the history listbox. */
typedef struct tagHISTORYREC
{
  HGLOBAL hTopicTitle;          /* Topic's title. */
  HGLOBAL hHelpFilePath;        /* Help file containing the topic data. */
  DWORD   dwTopicCharOffset;    /* Character offset for the topic's data in the
                                   help file's |TOPIC file. */
  WORD    wVertPos;             /* Initial vertical scroll position. */
  HGLOBAL hNext;                /* Next record in the list. */
  HGLOBAL hPrev;                /* Previous record in the list. */
}
HISTORYREC;
typedef HISTORYREC __far * FPHISTORYREC;
typedef HGLOBAL HHISTORYREC;


/* 
** Data stored for each history window
** managed by the DLL.
*/
typedef struct tagHISTWNDDATA
{
  HWND         hMainWnd;              /* Main help window. */
  HWND         hMainTopicWnd;         /* Main topic window if we need to send it a
                                         PRIVMSG_SHOWNEWTOPIC. */
  WORD         wMaxHistory;           /* Maximum number of HISTORYREC in history list. */
  WORD         wHistoryCount;         /* Number of history records in the list. */
  HHISTORYLIST hHistoryList;          /* List of history records. */
}
HISTWNDDATA;
typedef HISTWNDDATA __far * FPHISTWNDDATA;
typedef HGLOBAL HHISTWNDDATA;


/***********************************
**
**  Private Function Prototypes
**
***********************************/

BOOL CALLBACK __export HistoryDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
static void __far __pascal FreeHistoryList( HHISTORYLIST hHistoryList );
static void __far __pascal InsertHistoryRecord( HWND hWnd, HGLOBAL __far * hHistoryList, WORD wMaxHistory, WORD __far * wHistoryCount, HTOPICDATA hTopicData );
static HGLOBAL __far __pascal AllocHistoryRecord( HWND hWnd, char __far * szTopicTitle, char __far * fpHelpFilePath, DWORD dwTopicCharOffset );
static BOOL __far __pascal RecycleHistoryRecord( HWND hWnd, HHISTORYREC hHistoryRec, char __far * szTopicTitle, char __far * fpHelpFilePath, DWORD dwTopicCharOffset );
static void __far __pascal FreeHistoryRecord( HHISTORYREC hHistoryRec );
static void __far __pascal ClearHistoryRecord( HHISTORYREC hHistoryRec );
static BOOL __far __pascal SaveHistoryRecordData( HWND hWnd, HHISTORYREC hHistoryRec, char __far * szTopicTitle, char __far * fpHelpFilePath, DWORD dwTopicCharOffset );
static void __far __pascal ShowHistoryTitles( HWND hListBoxWnd, HGLOBAL hHistoryList );
static char __far * __far __pascal MoveToFileName( char __far * fpFilePath );
static void __far __pascal GotoHistoryTopic( HWND hListBoxWnd, HGLOBAL hHistoryList, HWND hMainWnd, HWND hMainTopicWnd );
static void __far __pascal UpdateHistoryRecordPos( HGLOBAL hHistoryList, WORD wHistoryCount, WORD wVertPos );
static void __far __pascal ClearHistoryRecordPos( HHISTORYLIST hHistoryList, WORD wHistoryCount );


/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/

/***********************************
**
**  Creates the history window.
**
***********************************/

HWND __far __pascal StartHistorySystem( HWND hMainWnd, HWND hMainTopicWnd )
{
  HWND hHistoryWnd;         /* Handle to the new history window. */
  HISTWNDDATA HistWndData;  /* Window data for the history window. */
  

  /* Initialize window data. */
  HistWndData.hMainWnd = hMainWnd;
  HistWndData.hMainTopicWnd = hMainTopicWnd;
      
  /* Get max history from .ini file. */
  ReadINIMaxHistory( &(HistWndData).wMaxHistory );

  /* Create a history window. */
  hHistoryWnd = CreateDialogParam( GetLibInst(), "DLG_HISTORY", hMainWnd, HistoryDlgProc, (LPARAM) ( FPHISTWNDDATA ) &HistWndData ); 

  /* Error? */
  if( hHistoryWnd == NULL )
  {
    MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_HISTORYWND, MB_ICONHAND | MB_OK );
    return( NULL );
  }

  /* Save the history window's handle. */
  WndData( hMainWnd, WDM_SETHISTORYHWND, (DWORD) hHistoryWnd, NULL );

  /* Return handle to history window. */
  return( hHistoryWnd );
}


/***********************************
**
**  Destroys the history window. 
**
***********************************/
void __far __pascal StopHistorySystem( HWND hMainWnd )
{
  HWND hHistoryWnd;
  
  /* Save the history window's handle. */
  hHistoryWnd = ( HWND ) WndData( hMainWnd, WDM_GETHISTORYHWND, 0L, NULL );

  /* Destroy history window.*/
  DestroyWindow( hHistoryWnd );
}


/***********************************************************************************
**
**                              Private Functions
**
************************************************************************************/

BOOL CALLBACK __export HistoryDlgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
  RECT ClientWndRect;
  
  HHISTWNDDATA hHistWndData;
  FPHISTWNDDATA fpHistWndData;
  
  FPHISTWNDDATA fpWndDataIn;
  

  switch( Msg ) 
  {
    /* Window initialization. */
    case WM_INITDIALOG:
    {  
      /* Allocate memory for window data. */
      if( !GlobalAllocMem( hWnd, &hHistWndData, sizeof(HISTWNDDATA) ) )
      {
        /* Failure. */
        return( -1 );
      }
  
      /* Lock window data. */
      fpHistWndData = ( FPHISTWNDDATA ) GlobalLock( hHistWndData );
      
      /* Save topic window data passed in. */
      fpWndDataIn = ( FPHISTWNDDATA ) lParam;
      fpHistWndData->hMainWnd      = fpWndDataIn->hMainWnd;
      fpHistWndData->hMainTopicWnd = fpWndDataIn->hMainTopicWnd;
      fpHistWndData->wMaxHistory   = fpWndDataIn->wMaxHistory;
      fpHistWndData->wHistoryCount = 0;
      
      /* Unlock window data. */
      GlobalUnlock( hHistWndData );
      
      /* Save handle to the window data. */
      SetWindowLong( hWnd, DWL_USER, (long ) hHistWndData ); 

      return( TRUE );
    }
    

    /* Window closing - only hide it. */
    case WM_SYSCOMMAND:
    {
      if ( wParam == SC_CLOSE ) 
      {
        ShowWindow( hWnd, SW_HIDE );
        return (TRUE);
      }
      break;
    }
    

    /* Window being destroyed. */
    case WM_DESTROY:
    {
      /* Save handle to the window data. */
      hHistWndData = ( HHISTWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 

      /* Lock window data. */
      fpHistWndData = ( FPHISTWNDDATA ) GlobalLock( hHistWndData );
      
      /* Free history list. */
      FreeHistoryList( fpHistWndData->hHistoryList );
  
      /* Unlock window data. */
      GlobalUnlock( hHistWndData );
      
      /* Free window data. */
      GlobalFree( hHistWndData );
      
      /* Save handle to the window data. */
      SetWindowLong( hWnd, DWL_USER, (long ) 0 ); 

      return (TRUE);
    }
    

    /* Window being sized - resize the list box. */
    case WM_SIZE:
    {  
      /* Get window size. */
      GetClientRect( hWnd, &ClientWndRect );

      /* Resize the list box to fill the window. */
      MoveWindow( GetDlgItem( hWnd, IDC_HISTORY ), -1, -1, ClientWndRect.right + 2, 
                  ClientWndRect.bottom + 2, TRUE );
      
      return (TRUE);
    }
    

    /* Add a new item to the history list. */
    case PRIVMSG_ADDHISTTOPIC:
    {
      /* Get handle of history list. */
      hHistWndData = ( HHISTWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 

      /* Lock window data. */
      fpHistWndData = ( FPHISTWNDDATA ) GlobalLock( hHistWndData );
      
      /* Insert history information into the list. */
      InsertHistoryRecord( hWnd, &(fpHistWndData)->hHistoryList, fpHistWndData->wMaxHistory, &(fpHistWndData)->wHistoryCount, (HTOPICDATA) wParam );
          
      /* Unlock window data. */
      GlobalUnlock( hHistWndData );

      return TRUE;
    }
      

    /* Update scroll bar position of top item in the history list. */
    case PRIVMSG_UPDATEHISTPOS:
    {
      /* Get handle of history list. */
      hHistWndData = ( HHISTWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 

      /* Lock window data. */
      fpHistWndData = ( FPHISTWNDDATA ) GlobalLock( hHistWndData );
      
      /* Update scroll bar position of top item in the history list. */
      UpdateHistoryRecordPos( fpHistWndData->hHistoryList, fpHistWndData->wHistoryCount, (WORD) wParam );
          
      /* Unlock window data. */
      GlobalUnlock( hHistWndData );

      return TRUE;
    }
      
      
    /* Clear scroll bar positions of all items in the history list. */
    case PRIVMSG_CLEARHISTPOS:
    {
      /* Get handle of history list. */
      hHistWndData = ( HHISTWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 

      /* Lock window data. */
      fpHistWndData = ( FPHISTWNDDATA ) GlobalLock( hHistWndData );
      
      /* Clear scroll bar positions of all items in the history list. */
      ClearHistoryRecordPos( fpHistWndData->hHistoryList, fpHistWndData->wHistoryCount );
          
      /* Unlock window data. */
      GlobalUnlock( hHistWndData );

      return TRUE;
    }
      
      
    case WM_COMMAND:
    {
      switch( wParam ) 
      {
        case IDC_HISTORY:
        {
          if ( HIWORD(lParam) == LBN_DBLCLK ) 
          {
            /* Save handle to the window data. */
            hHistWndData = ( HHISTWNDDATA ) GetWindowLong( hWnd, DWL_USER ); 
            
            /* Lock window data. */
            fpHistWndData = ( FPHISTWNDDATA ) GlobalLock( hHistWndData );
                  
            /* Go to the selected history topic. */
            GotoHistoryTopic( GetDlgItem( hWnd, IDC_HISTORY ), 
                              fpHistWndData->hHistoryList, 
                              fpHistWndData->hMainWnd,
                              fpHistWndData->hMainTopicWnd );

            /* Unlock window data. */
            GlobalUnlock( hHistWndData );
          }

          /* Process LBN_DBLCLK message. */
          return TRUE;
        }
      
        /* Process all wanted IDC_HISTORY messages. */
        return( TRUE );
      }       

      /* Process all wanted WM_COMMAND messages. */
      return TRUE;
    }
  }

  /* Message not processed. */
  return FALSE;
}



static void __far __pascal FreeHistoryList( HHISTORYLIST hHistoryList )
{
  HHISTORYREC  hNextRec;          
  HHISTORYREC  hHistoryRec;
  FPHISTORYREC fpHistoryRec;

  
  /* If empty list. */
  if( hHistoryList == NULL ) return;
  
  /* Get our first record's handle. */
  hHistoryRec = hHistoryList;
  
  /* While we have unprocessed records. */
  do
  {
    /* Lock the current record. */
    fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

    /* Get the next record handle. */
    hNextRec = fpHistoryRec->hNext;
    
    /* Unlock the current record. */
    GlobalUnlock( hHistoryRec );
  
    /* Free current record. */
    FreeHistoryRecord( hHistoryRec );

    /* Use next record. */
    hHistoryRec = hNextRec; 
  }
  while( hHistoryRec != hHistoryList );
}



static void __far __pascal InsertHistoryRecord( HWND hWnd, HGLOBAL __far * hHistoryList, WORD wMaxHistory, WORD __far * wHistoryCount, HTOPICDATA hTopicData )
{
  HHISTORYREC  hNewTopRec;
  FPHISTORYREC fpNewTopRec;
  FPHISTORYREC fpOldTopRec;
  FPHISTORYREC fpEndRec;

  FPTOPICDATA fpTopicData;
  
  BOOL bReturn;

  char szTopicTitle[MAX_TOPIC_TITLE];
  
                   
  /* Lock topic's data. */
  if( hTopicData == NULL ) return;
  fpTopicData = ( FPTOPICDATA ) GlobalLock( hTopicData );
  
#if 0
  char __far * lpTopFilePath;  
                   
  /*
  ** Unlike the real WinHelp program, we are not going to
  ** insert the same record two times in a row. 
  */
  /* If not first record. */
  if( *wHistoryCount != 0 )
  {
    /* Lock current top record. */
    fpOldTopRec = ( FPHISTORYREC ) GlobalLock( *hHistoryList );
  
    /* Lock the top record's file path. */
    lpTopFilePath = ( char __far * ) GlobalLock( fpOldTopRec->hHelpFilePath );

    /* Is it the same filename? */
    if( _fstrcmp( fpTopicData->szHelpFile, lpTopFilePath ) == 0 )
    {
      /* Unlock the top record's file path. */
      GlobalUnlock( fpOldTopRec->hHelpFilePath );

      /* Is it the same topic? */
      if( fpOldTopRec->dwTopicCharOffset == fpTopicData->dwTopicCharOffset )
      {
        /* Unlock current top record. */
        GlobalUnlock( *hHistoryList );
        
        /* Unlock topic's data. */
        GlobalUnlock( hTopicData );

        return;
      } 
    }

    /* Unlock the top record's file path. */
    GlobalUnlock( fpOldTopRec->hHelpFilePath );

    /* Unlock current top record. */
    GlobalUnlock( *hHistoryList );
  }
#endif

  /* Get the topic title data. */
  GetTopicTitle( fpTopicData->hHelpFileInfo, fpTopicData->dwTopicCharOffset, szTopicTitle );


  /* 
  ** If we've not reached our list limit
  ** insert a new record into the history list.
  */
  if( *wHistoryCount != wMaxHistory)
  {
    /* Make new record. */
    hNewTopRec = AllocHistoryRecord( hWnd, szTopicTitle, fpTopicData->szHelpFile, fpTopicData->dwTopicCharOffset );

    /* Allocation error? */
    if( hNewTopRec == NULL ) 
    {
      /* Unlock topic's data. */
      GlobalUnlock( hTopicData );

      return;
    }
    
    /* If first record. */
    if( *wHistoryCount == 0 )
    {
      /* Lock new top record. */
      fpNewTopRec = ( FPHISTORYREC ) GlobalLock( hNewTopRec );

      /* All pointers point to itself. */
      fpNewTopRec->hNext = hNewTopRec;
      fpNewTopRec->hPrev = hNewTopRec;

      /* Unlock new top record. */
      GlobalUnlock( hNewTopRec );
    }
    
    /* Not first record. */
    else
    {
      /* Lock new top record. */
      fpNewTopRec = ( FPHISTORYREC ) GlobalLock( hNewTopRec );

      /* Lock old top record. */
      fpOldTopRec = ( FPHISTORYREC ) GlobalLock( *hHistoryList );
  
      /* Lock end record. */
      fpEndRec = ( FPHISTORYREC ) GlobalLock( fpOldTopRec->hPrev );
  
      /* End record's hNext should now point to new top. */
      fpEndRec->hNext = hNewTopRec;
      
      /* Unlock end record. */
      GlobalUnlock( fpOldTopRec->hPrev );
  
      /* New top's hPrev points to end record. */
      fpNewTopRec->hPrev = fpOldTopRec->hPrev;
  
      /* New top's hNext points to old top record. */
      fpNewTopRec->hNext = *hHistoryList;
  
      /* Old top record's hPrev points to new top record. */
      fpOldTopRec->hPrev = hNewTopRec;
  
      /* Unlock old top record. */
      GlobalUnlock( *hHistoryList );
  
      /* Unlock new top record. */
      GlobalUnlock( hNewTopRec );
    }
  
    /* Increment the list count. */
    *wHistoryCount = *wHistoryCount + 1;
  }
     
     
  /* 
  ** Otherwise, make the last record the new
  ** top record and change its contents.
  */
  else
  {  
    /* Lock old top record. */
    fpOldTopRec = ( FPHISTORYREC ) GlobalLock( *hHistoryList );

    /* 
    ** Get the record preceding the top record 
    ** (e.g. the end record).
    */
    hNewTopRec = fpOldTopRec->hPrev;
    
    /* Unlock old top record. */
    GlobalUnlock( *hHistoryList );

    /* Recycled the record. */
    bReturn = RecycleHistoryRecord( hWnd, hNewTopRec, szTopicTitle, fpTopicData->szHelpFile, fpTopicData->dwTopicCharOffset );

    /* Recycle error? */
    if( bReturn == FALSE ) 
    {
      /* Unlock topic's data. */
      GlobalUnlock( hTopicData );

      return;
    }
    
  }
  
  /* Unlock topic's data. */
  GlobalUnlock( hTopicData );

  /* Change the value of the handle list to the new top record. */
  *hHistoryList = hNewTopRec;

  /* Update history window's list box. */
  ShowHistoryTitles( GetDlgItem( hWnd, IDC_HISTORY ), *hHistoryList );
}


/* Update scroll bar position of top item in the history list. */
static void __far __pascal UpdateHistoryRecordPos( HGLOBAL hHistoryList, WORD wHistoryCount, WORD wVertPos )
{
  FPHISTORYREC fpHistoryRec;

  /* No records in list. */
  if( wHistoryCount == 0 ) 
  {
    return;
  }

  /* Lock the top record. */
  fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryList );

  /* Update position information. */
  fpHistoryRec->wVertPos = wVertPos;
    
  /* Unlock the current record. */
  GlobalUnlock( hHistoryList );
}


/* Clear scroll bar positions of all items in the history list. */
static void __far __pascal ClearHistoryRecordPos( HHISTORYLIST hHistoryList, WORD wHistoryCount )
{
  HHISTORYREC  hNextRec;          
  HHISTORYREC  hHistoryRec;
  FPHISTORYREC fpHistoryRec;

  
  /* If empty list. */
  if( wHistoryCount == 0 ) 
  {
    return;
  }
  
  /* Get our first record's handle. */
  hHistoryRec = hHistoryList;
  
  /* While we have unprocessed records. */
  do
  {
    /* Lock the current record. */
    fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

    /* Update position information. */
    fpHistoryRec->wVertPos = 0;
    
    /* Get the next record handle. */
    hNextRec = fpHistoryRec->hNext;
    
    /* Unlock the current record. */
    GlobalUnlock( hHistoryRec );
  
    /* Use next record. */
    hHistoryRec = hNextRec; 
  }
  while( hHistoryRec != hHistoryList );
}


          
static void __far __pascal GotoHistoryTopic( HWND hListBoxWnd, HGLOBAL hHistoryList, HWND hMainWnd, HWND hMainTopicWnd )
{
  HHISTORYREC  hNextRec;          
  HHISTORYREC  hHistoryRec;
  FPHISTORYREC fpHistoryRec;

  WORD         Counter;
  DWORD        dwListBoxIndex;

  HGLOBAL hTopicData;           /* Handle to the topic's elements list. */                

  HGLOBAL hHelpFileInfo;        /* Handle to the help file's general info. */

  char __far * fpFilePath;

  
  /* Get selected entry in the list box. */
  dwListBoxIndex = SendMessage( hListBoxWnd, LB_GETCURSEL, 0, 0L );

  /* No records in list. */
  if( hHistoryList == NULL ) return;

  /* Get our first record's handle. */
  hHistoryRec = hHistoryList;
  
  /* Go to record holding list box item's data. */
  for( Counter = 0; ( DWORD ) Counter < dwListBoxIndex; Counter++ )
  {
    /* Lock the current record. */
    fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

    /* Get the next record handle. */
    hNextRec = fpHistoryRec->hNext;
    
    /* Unlock the current record. */
    GlobalUnlock( hHistoryRec );
  
    /* Use next record. */
    hHistoryRec = hNextRec; 
  }

  /* 
  ** Lock the current record containing the information 
  ** about the selected list box item. 
  */
  fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

  /* 
  ** Open and load the help file's info. 
  */

  /* Lock the record's file path data. */
  fpFilePath = (char __far *) GlobalLock( fpHistoryRec->hHelpFilePath );
    
  /* Get file info. */
  hHelpFileInfo = OpenHelpFile( hMainWnd, fpFilePath );
  if( hHelpFileInfo == NULL )
  {
    /* Unlock the record's file path data. */
    GlobalUnlock( fpHistoryRec->hHelpFilePath );

    /* Unlock the current record. */
    GlobalUnlock( hHistoryRec );

    /* Failure. */
    return;
  }


  /* 
  ** Get the topic's data. 
  */

  hTopicData = LockTopicData( hMainWnd, fpFilePath, fpHistoryRec->dwTopicCharOffset );
  if( hTopicData == NULL )
  {
    /* Unlock the record's file path data. */
    GlobalUnlock( fpHistoryRec->hHelpFilePath );

    /* Unlock the current record. */
    GlobalUnlock( hHistoryRec );

    /* Failure. */
    return;
  }

  /* Unlock the record's file path data. */
  GlobalUnlock( fpHistoryRec->hHelpFilePath );

  /* Inform main topic window of new topic to load. */
  SendShowNewTopicMsg( hMainTopicWnd, hTopicData, TRUE, fpHistoryRec->wVertPos ); 

  /* Unlock the current record. */
  GlobalUnlock( hHistoryRec );
}



static void __far __pascal ShowHistoryTitles( HWND hListBoxWnd, HGLOBAL hHistoryList )
{
  HHISTORYREC  hNextRec;          
  HHISTORYREC  hHistoryRec;

  FPHISTORYREC fpHistoryRec;
  char __far * fpTopicTitle;
  char __far * fpFilePath;

  FPHISTORYREC fpFirstHistoryRec;  
  char __far * fpFirstFilePath;

  #define BUFF_SIZE ( MAX_TOPIC_TITLE + _MAX_PATH - 1 )

  char StringBuffer[ BUFF_SIZE + 1 ];
  WORD wBuffIndex;
  
  WORD wListBoxIndex;
  
  
  /* Get selected entry in the list box. */
  wListBoxIndex = ( WORD ) SendMessage( hListBoxWnd, LB_GETCURSEL, 0, 0L );

  /* Clear list box. */
  SendMessage( hListBoxWnd, LB_RESETCONTENT, 0, 0L ); 

  /* No records in list. */
  if( hHistoryList == NULL ) return;

  /* Get our first record's handle. */
  hHistoryRec = hHistoryList;
  
  /* Lock the first record. */
  fpFirstHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryList );

  /* Lock the first record's file path. */
  fpFirstFilePath = (char __far *) GlobalLock( fpFirstHistoryRec->hHelpFilePath );

  /* While we have unprocessed records. */
  do
  {
    /* Lock the current record. */
    fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

    /* Get the next record handle. */
    hNextRec = fpHistoryRec->hNext;
    
    /* Lock the record's title data. */
    fpTopicTitle = ( char __far * ) GlobalLock( fpHistoryRec->hTopicTitle ); 
    
    /* Lock the record's file path data. */
    fpFilePath = (char __far *) GlobalLock( fpHistoryRec->hHelpFilePath );
    
    /* If we are showing a topic that's in the currect file. */
    if( _fstricmp( fpFilePath, fpFirstFilePath ) == 0 )
    {
      /* Insert the topic title at the end of the history list box. */
      SendMessage( hListBoxWnd, LB_ADDSTRING, 0, (LPARAM) fpTopicTitle );
    }
    
    /* Otherwise, build a string containing the .hlp filename and topic title. */
    else
    {
      fpFilePath = MoveToFileName( fpFilePath );
      
      /* 
      ** Build list box entry. 
      */
      
      wBuffIndex = 0;
      
      /* Add filename (without extension) to buffer. */
      while( ( *fpFilePath != '\0' ) && ( *fpFilePath != EXT_CHAR ) && ( wBuffIndex < BUFF_SIZE ) )
      {
        /* Save character to buffer. */
        StringBuffer[ wBuffIndex++ ] = *fpFilePath;
        
        /* Go to next filename character. */
        fpFilePath++;
      }
      StringBuffer[ wBuffIndex ] = '\0';
      
      /* Make the filename upper case. */
      _strupr( StringBuffer );
      
      /* Add colon. */
      _fstrcat( StringBuffer, ":" );
      
      /* Add topic text. */
      _fstrcat( StringBuffer, fpTopicTitle );
      
      /* Insert the entry at the end of the history list box. */
      SendMessage( hListBoxWnd, LB_ADDSTRING, 0, (LPARAM) ( char __far * ) StringBuffer );
    }
    
    /* Unlock the record's title data. */
    GlobalUnlock( fpHistoryRec->hTopicTitle ); 

    /* Unlock the record's file path data. */
    GlobalUnlock( fpHistoryRec->hHelpFilePath );

    /* Unlock the current record. */
    GlobalUnlock( hHistoryRec );
  
    /* Use next record. */
    hHistoryRec = hNextRec; 
  }
  while( hHistoryRec != hHistoryList );

  /* Lock the first record. */
  fpFirstHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryList );

  /* Lock the first record's file path. */
  fpFirstFilePath = (char __far *) GlobalLock( fpFirstHistoryRec->hHelpFilePath );

  /* Select index that was previously selected. */
  if( wListBoxIndex != LB_ERR ) 
  {
    SendMessage( hListBoxWnd, LB_SETCURSEL, wListBoxIndex, 0L );
  }
}


static char __far * __far __pascal MoveToFileName( char __far * fpFilePath )
{
  char __far * fpSavePtr = fpFilePath;
  
  /* Move pointer to last "\" in file path. */
  fpFilePath = _fstrrchr( fpFilePath, DIR_SEP_CHAR );
  
  /* If we found a "\" in path. */
  if( fpFilePath != NULL )
  {
    /* Move to first character of the file's name. */
    fpFilePath++;
    
    /* Return pointer to the file's name. */
    return( fpFilePath );
  }

  /* 
  ** Otherwise assume that only the file's name is
  ** in the path and no drive or directory information exists.
  */
  return( fpSavePtr );
}
      

static HGLOBAL __far __pascal AllocHistoryRecord( HWND hWnd, char __far * szTopicTitle, char __far * fpHelpFilePath, DWORD dwTopicCharOffset )
{
  HHISTORYREC  hHistoryRec;

                   
  /* Allocate record structure memory. */
  if( !GlobalAllocMem( hWnd, &hHistoryRec, sizeof(HISTORYREC) ) )
  {
    /* Failure. */
    return( NULL );
  }
  
  /* Store the info. in the new record. */
  if( ! SaveHistoryRecordData( hWnd, hHistoryRec, szTopicTitle, fpHelpFilePath, dwTopicCharOffset ) )
  {
    /* Free record. */
    GlobalFree( hHistoryRec );
    
    /* Failure. */
    return( NULL );
  }

  /* Success. */
  return( hHistoryRec );
}


static BOOL __far __pascal RecycleHistoryRecord( HWND hWnd, HHISTORYREC hHistoryRec, char __far * szTopicTitle, char __far * fpHelpFilePath, DWORD dwTopicCharOffset )
{
  /* Free data allocated for data in the record. */
  ClearHistoryRecord( hHistoryRec );
  
  if( ! SaveHistoryRecordData( hWnd, hHistoryRec, szTopicTitle, fpHelpFilePath, dwTopicCharOffset ) )
  {
    /* Failure. */
    return( FALSE );
  }

  /* Success. */
  return( TRUE );
}


static void __far __pascal FreeHistoryRecord( HHISTORYREC hHistoryRec )
{
  /* Free data allocated for data in the record. */
  ClearHistoryRecord( hHistoryRec ); 
 
  /* Free the record structure. */
  GlobalFree( hHistoryRec );
}


static void __far __pascal ClearHistoryRecord( HHISTORYREC hHistoryRec )
{
  FPHISTORYREC fpHistoryRec;


  /* Lock the record. */
  fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

  /* Free data allocated for the filepath. */
  GlobalFree( fpHistoryRec->hHelpFilePath );

  /* Free data allocated for the topic title. */
  GlobalFree( fpHistoryRec->hTopicTitle );

  /* Unlock the record. */
  GlobalUnlock( hHistoryRec );
}


static BOOL __far __pascal SaveHistoryRecordData( HWND hWnd, HHISTORYREC hHistoryRec, char __far * szTopicTitle, char __far * fpHelpFilePath, DWORD dwTopicCharOffset )
{
  FPHISTORYREC fpHistoryRec;


  /* Lock the record. */
  fpHistoryRec = (FPHISTORYREC) GlobalLock( hHistoryRec );

  /*
  **  Save file's path. 
  */
  fpHistoryRec->hHelpFilePath = CopyString( hWnd, fpHelpFilePath );
  
  /* Copy error? */
  if( fpHistoryRec->hHelpFilePath == NULL )
  {
    /* Unlock the record. */
    GlobalUnlock( hHistoryRec );

    /* Failure. */
    return( FALSE );
  }
  

  /*
  **  Save topic's title. 
  */
  fpHistoryRec->hTopicTitle = CopyString( hWnd, szTopicTitle );

  /* Copy error? */
  if( fpHistoryRec->hTopicTitle == NULL )
  {
    /* Unlock the record. */
    GlobalUnlock( hHistoryRec );

    /* Failure. */
    return( FALSE );
  }
  

  /*
  **  Save topic offset. 
  */
  fpHistoryRec->dwTopicCharOffset = dwTopicCharOffset;

  /* Unlock the record. */
  GlobalUnlock( hHistoryRec );

  /* Success. */
  return( TRUE );
}


