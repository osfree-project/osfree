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
**                                 "BACK" BUTTON ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <string.h>
#include <stdlib.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "back.h"
#include "winmem.h"
#include "inifile.h"
#include "wnddata.h"
#include "btnbar.h"
#include "twinhelp.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* Handle to the top of the back list containing 
** BACKREC structures. 
*/
typedef HGLOBAL HBACKLIST;


/* Record kept for each entry in the back list. */
typedef struct tagBACKREC
{
  DWORD   dwTopicCharOffset;    /* Character offset for the topic's data in the
                                   help file's |TOPIC file. */
  HGLOBAL hHelpFilePath;        /* Help file containing the topic data. */
  WORD    wVertPos;             /* Initial vertical scroll position. */
  HGLOBAL hNext;                /* Next record in the list. */
  HGLOBAL hPrev;                /* Previous record in the list. */
}
BACKREC;
typedef BACKREC __far * FPBACKREC;
typedef HGLOBAL HBACKREC;


/* 
** Data stored for each back list
** managed by the DLL.
*/
typedef struct tagBACKBTNDATA
{
  char      szActiveHelpFilePath[ _MAX_PATH ];  /* Currently active help file. */
  HWND      hDataWnd;              /* Main help window. */
  HWND      hMainTopicWnd;         /* Main topic window if we need to send it a
                                         PRIVMSG_SHOWNEWTOPIC. */
  WORD      wMaxBack;              /* Maximum number of BACKREC in back list. */
  WORD      wBackCount;            /* Number of BACKREC records in the list. */
  HBACKLIST hBackList;             /* List of back records. */
}
BACKBTNDATA;
typedef BACKBTNDATA __far * FPBACKBTNDATA;
typedef HGLOBAL HBACKBTNDATA;


/***********************************
**
**  Private Function Prototypes
**
***********************************/

static void __far __pascal FreeBackList( HWND hDataWnd, HBACKLIST __far * hBackList, WORD __far * wBackCount );
static BOOL __far __pascal InsertBackRecord( HWND hDataWnd, HGLOBAL __far * hBackList, 
                                             WORD wMaxBack,
                                             WORD __far * wBackCount, 
                                             HTOPICDATA hTopicData );
static BOOL __far __pascal RemoveBackRecord
( 
  HGLOBAL __far * hBackList, 
  WORD __far * wBackCount, 
  char __far * fpHelpFilePath, 
  DWORD __far * dwTopicCharOffset, 
  WORD __far * wVertPosPtr               /* Topics scroll bar position. */
);
static BOOL __far __pascal RecycleBackRecord( HWND hDataWnd, HBACKREC hBackRec, char __far * fpHelpFilePath, DWORD dwTopicCharOffset );
static HGLOBAL __far __pascal AllocBackRecord( HWND hDataWnd, char __far * fpHelpFilePath, DWORD dwTopicCharOffset );
static void __far __pascal FreeBackRecord( HBACKREC hBackRec );
static void __far __pascal ClearBackRecord( HBACKREC hBackRec );
static BOOL __far __pascal SaveBackRecordData( HWND hDataWnd, HBACKREC hBackRec, char __far * fpHelpFilePath, DWORD dwTopicCharOffset );


/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/

/***********************************
**
**  Allocates back list data.
**
***********************************/

BOOL __far __pascal StartBackSystem( HWND hDataWnd )
{
  HBACKBTNDATA  hBackBtnData;    /* Handle for the back system data. */
  FPBACKBTNDATA fpBackBtnData;   /* Pointer for the back system data. */
  WORD          wMaxBack;        /* Maximum number of back records. */
  

  /* Get max back from .ini file. */
  ReadINIMaxBack( &wMaxBack );
 
  /* Allocate memory for the data. */
  if( !GlobalAllocMem( hDataWnd, &hBackBtnData, sizeof(BACKBTNDATA) ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Lock back data. */
  fpBackBtnData = ( FPBACKBTNDATA ) GlobalLock( hBackBtnData );
      
  /* Save data passed in. */
  fpBackBtnData->hDataWnd   = hDataWnd;
  fpBackBtnData->wMaxBack   = wMaxBack;
  fpBackBtnData->wBackCount = 0;
  
  /* Unlock back data. */
  GlobalUnlock( hBackBtnData );
      
  /* Save the back data's handle. */
  WndData( hDataWnd, WDM_SETBACKDATA, (DWORD) hBackBtnData, NULL );

  /* Return success. */
  return( TRUE );
}


/***********************************
**
**  Destroys the back back. 
**
***********************************/
void __far __pascal StopBackSystem( HWND hDataWnd )
{
  HBACKBTNDATA  hBackBtnData;    /* Handle for the back system data. */
  FPBACKBTNDATA fpBackBtnData;   /* Pointer for the back system data. */


  /* Get the back data. */
  hBackBtnData = ( HBACKBTNDATA ) WndData( hDataWnd, WDM_GETBACKDATA, 0L, NULL );

  /* Lock back data. */
  fpBackBtnData = ( FPBACKBTNDATA ) GlobalLock( hBackBtnData );
      
  /* Free back list. */
  FreeBackList( hDataWnd, &(fpBackBtnData)->hBackList, &(fpBackBtnData)->wBackCount );
  
  /* Unlock back data. */
  GlobalUnlock( hBackBtnData );
      
  /* Free back data. */
  GlobalFree( hBackBtnData );
      
  /* Save the back data's handle. */
  WndData( hDataWnd, WDM_SETBACKDATA, (DWORD) 0, NULL );
}



BOOL __far __pascal InsertBackTopic( HWND hDataWnd, HTOPICDATA hTopicData  )
{
  HBACKBTNDATA  hBackBtnData;    /* Handle for the back system data. */
  FPBACKBTNDATA fpBackBtnData;   /* Pointer for the back system data. */

  HWND  hButtonBarWnd;

                   
  /* No topic data. */
  if( hTopicData == NULL ) return FALSE;

  /* Get the back data. */
  hBackBtnData = ( HBACKBTNDATA ) WndData( hDataWnd, WDM_GETBACKDATA, 0L, NULL );

  /* Lock back data. */
  fpBackBtnData = ( FPBACKBTNDATA ) GlobalLock( hBackBtnData );
      
  /* Insert back information into the list. */
  if( !InsertBackRecord( hDataWnd, &(fpBackBtnData)->hBackList, fpBackBtnData->wMaxBack, 
                         &(fpBackBtnData)->wBackCount, hTopicData ) )
  {
    /* Unlock back data. */
    GlobalUnlock( hBackBtnData );
  
    /* Failure. */
    return( FALSE );
  }

  /* Enable the "Back" button if we can go back. */
  if( fpBackBtnData->wBackCount > 1 )
  {
    hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0L, NULL );
    EnableWindow( GetButtonWindow( hButtonBarWnd, BACK_BUTTON_ID), TRUE );
  }

  /* Unlock back data. */
  GlobalUnlock( hBackBtnData );

  /* Success. */
  return( TRUE );
}


void __far __pascal SetLastBackVertPos( HWND hDataWnd, WORD wVertPos )
{
  HBACKBTNDATA  hBackBtnData;    /* Handle for the back system data. */
  FPBACKBTNDATA fpBackBtnData;   /* Pointer for the back system data. */
  FPBACKREC fpTopRec;
   
                   
  /* Get the back data. */
  hBackBtnData = ( HBACKBTNDATA ) WndData( hDataWnd, WDM_GETBACKDATA, 0L, NULL );

  /* Lock back data. */
  fpBackBtnData = ( FPBACKBTNDATA ) GlobalLock( hBackBtnData );
      
  /* Empty list? */
  if( fpBackBtnData->wBackCount == 0 )
  {
    /* Unlock back data. */
    GlobalUnlock( hBackBtnData );
  
    return;
  }

  /* Lock the top record. */
  fpTopRec = ( FPBACKREC ) GlobalLock( fpBackBtnData->hBackList );
  
  /* Update position. */
  fpTopRec->wVertPos = wVertPos;
  
  /* Unlock the record. */
  GlobalLock( fpBackBtnData->hBackList );

  /* Unlock back data. */
  GlobalUnlock( hBackBtnData );
}


void __far __pascal ClearAllBackVertPos( HWND hDataWnd )
{
  HBACKBTNDATA  hBackBtnData;    /* Handle for the back system data. */
  FPBACKBTNDATA fpBackBtnData;   /* Pointer for the back system data. */

  HBACKREC  hNextRec;          
  HBACKREC  hBackRec;
  FPBACKREC fpBackRec;

  
  /* Get the back data. */
  hBackBtnData = ( HBACKBTNDATA ) WndData( hDataWnd, WDM_GETBACKDATA, 0L, NULL );

  /* No back data. */
  if( hBackBtnData == NULL ) return; 

  /* Lock back data. */
  fpBackBtnData = ( FPBACKBTNDATA ) GlobalLock( hBackBtnData );
      
  /* Empty list? */
  if( fpBackBtnData->wBackCount == 0 )
  {
    /* Unlock back data. */
    GlobalUnlock( hBackBtnData );
  
    return;
  }

  /* Get our first record's handle. */
  hBackRec = fpBackBtnData->hBackList;
  
  /* While we have unprocessed records. */
  do
  {
    /* Lock the current record. */
    fpBackRec = (FPBACKREC) GlobalLock( hBackRec );

    /* Reset the vertical position value. */
    fpBackRec->wVertPos = 0;

    /* Get the next record handle. */
    hNextRec = fpBackRec->hNext;
    
    /* Unlock the current record. */
    GlobalUnlock( hBackRec );
  
    /* Use next record. */
    hBackRec = hNextRec; 
  }
  while( hBackRec != fpBackBtnData->hBackList );

  /* Unlock back data. */
  GlobalUnlock( hBackBtnData );
}



BOOL __far __pascal RemoveBackTopic
( 
  HWND hDataWnd, 
  char __far * szHelpFilePath, 
  DWORD __far * dwTopicCharOffset,
  WORD __far * wVertPosPtr               /* Topics scroll bar position. */
)
{
  BOOL          bReturn;         /* Success of getting last record's data. */
  HBACKBTNDATA  hBackBtnData;    /* Handle for the back system data. */
  FPBACKBTNDATA fpBackBtnData;   /* Pointer for the back system data. */

  HWND      hButtonBarWnd;


  /* Get the back data. */
  hBackBtnData = ( HBACKBTNDATA ) WndData( hDataWnd, WDM_GETBACKDATA, 0L, NULL );

  /* Lock back data. */
  fpBackBtnData = ( FPBACKBTNDATA ) GlobalLock( hBackBtnData );
      
  /* 
  ** Should always have at least one record in the list.
  ** We should not be trying to remove a record. 
  */
  if( fpBackBtnData->wBackCount == 1 )
  {
    /* Unlock back data. */
    GlobalUnlock( hBackBtnData );
  
    /* Error. */
    return( FALSE );
  }

  /* Remove the last back record's information from the list. */
  bReturn = RemoveBackRecord( &(fpBackBtnData)->hBackList, &(fpBackBtnData)->wBackCount, 
                              szHelpFilePath, dwTopicCharOffset, wVertPosPtr );

  /* No more back records, disable button bar. */
  if( fpBackBtnData->wBackCount == 1 )
  {
    hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0L, NULL );
    EnableWindow( GetButtonWindow( hButtonBarWnd, BACK_BUTTON_ID), FALSE );
  }

  /* Unlock back data. */
  GlobalUnlock( hBackBtnData );

  /* Success? */
  return( bReturn );
}



static void __far __pascal FreeBackList( HWND hDataWnd, HBACKLIST __far * hBackList, WORD __far * wBackCount )
{
  HBACKREC  hNextRec;          
  HBACKREC  hBackRec;
  FPBACKREC fpBackRec;

  HWND      hButtonBarWnd;

  
  /* If empty list. */
  if( *hBackList == NULL ) return;
  
  /* Get our first record's handle. */
  hBackRec = *hBackList;
  
  /* While we have unprocessed records. */
  do
  {
    /* Lock the current record. */
    fpBackRec = (FPBACKREC) GlobalLock( hBackRec );

    /* Get the next record handle. */
    hNextRec = fpBackRec->hNext;
    
    /* Unlock the current record. */
    GlobalUnlock( hBackRec );
  
    /* Free current record. */
    FreeBackRecord( hBackRec );

    /* Use next record. */
    hBackRec = hNextRec; 
  }
  while( hBackRec != *hBackList );

  /* Init. the list handle. */
  *hBackList = NULL;
  
  /* No record in the list. */
  *wBackCount = 0;

  /* No more back records, disable button bar. */
  hButtonBarWnd = (HWND) WndData( hDataWnd, WDM_GETBUTTONBARHWND, 0L, NULL );
  EnableWindow( GetButtonWindow( hButtonBarWnd, BACK_BUTTON_ID), FALSE );
}



static BOOL __far __pascal InsertBackRecord
( 
  HWND hDataWnd, 
  HGLOBAL __far * hBackList, 
  WORD wMaxBack, 
  WORD __far * wBackCount, 
  HTOPICDATA hTopicData 
)
{
  FPTOPICDATA fpTopicData;       /* Topic's data. */
  
  HBACKREC  hNewTopRec;
  FPBACKREC fpNewTopRec;
  FPBACKREC fpOldTopRec;
  FPBACKREC fpEndRec;


  /* Lock the topic data. */
  fpTopicData = ( FPTOPICDATA ) GlobalLock( hTopicData );
                   
#if 0
  char __far * lpTopFilePath;  
                   
  /*
  ** Unlike the real WinHelp program, we are not going to
  ** insert the same record two times in a row. 
  */
  if( *wBackCount != 0 )
  {
    /* Lock old top record. */
    fpOldTopRec = ( FPBACKREC ) GlobalLock( *hBackList );
    
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
        GlobalUnlock( *hBackList );
        
        /* Unlock topic's data. */
        GlobalUnlock( hTopicData );

        /* Done all the work that we need to. */
        return( TRUE );
      } 
    }

    /* Unlock the top record's file path. */
    GlobalUnlock( fpOldTopRec->hHelpFilePath );

    /* Unlock current top record. */
    GlobalUnlock( *hBackList );
  }
#endif

  /* 
  ** If we've not reached our list limit
  ** insert a new record into the back list.
  */
  if( *wBackCount != wMaxBack)
  {
    /* Make new record. */
    hNewTopRec = AllocBackRecord( hDataWnd, fpTopicData->szHelpFile, fpTopicData->dwTopicCharOffset );

    /* Allocation error? */
    if( hNewTopRec == NULL ) 
    {
      /* Unlock topic's data. */
      GlobalUnlock( hTopicData );

      return FALSE;
    }
    
    /* If first record. */
    if( *wBackCount == 0 )
    {
      /* Lock new top record. */
      fpNewTopRec = ( FPBACKREC ) GlobalLock( hNewTopRec );

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
      fpNewTopRec = ( FPBACKREC ) GlobalLock( hNewTopRec );

      /* Lock old top record. */
      fpOldTopRec = ( FPBACKREC ) GlobalLock( *hBackList );
  
      /* Lock end record. */
      fpEndRec = ( FPBACKREC ) GlobalLock( fpOldTopRec->hPrev );
  
      /* End record's hNext should now point to new top. */
      fpEndRec->hNext = hNewTopRec;
      
      /* Unlock end record. */
      GlobalUnlock( fpOldTopRec->hPrev );
  
      /* New top's hPrev points to end record. */
      fpNewTopRec->hPrev = fpOldTopRec->hPrev;
  
      /* New top's hNext points to old top record. */
      fpNewTopRec->hNext = *hBackList;
  
      /* Old top record's hPrev points to new top record. */
      fpOldTopRec->hPrev = hNewTopRec;
  
      /* Unlock old top record. */
      GlobalUnlock( *hBackList );
  
      /* Unlock new top record. */
      GlobalUnlock( hNewTopRec );
    }

    /* Increment record count. */
    *wBackCount = *wBackCount + 1;
  }
     
     
  /* 
  ** Otherwise, make the last record the new
  ** top record and change its contents.
  */
  else
  {  
    /* Lock old top record. */
    fpOldTopRec = ( FPBACKREC ) GlobalLock( *hBackList );

    /* 
    ** Get the record preceding the top record 
    ** (e.g. the end record).
    */
    hNewTopRec = fpOldTopRec->hPrev;
    
    /* Unlock old top record. */
    GlobalUnlock( *hBackList );

    /* Recycled the record's data. */
    if( ! RecycleBackRecord( hDataWnd, hNewTopRec, fpTopicData->szHelpFile, fpTopicData->dwTopicCharOffset ) )
    {
      /* Unlock topic's data. */
      GlobalUnlock( hTopicData );
    
      /* Success. */
      return( FALSE );
    }
  }
  
  /* Change the value of the handle list to the new top record. */
  *hBackList = hNewTopRec;

  /* Unlock topic's data. */
  GlobalUnlock( hTopicData );

  /* Success. */
  return( TRUE );
}


static BOOL __far __pascal RemoveBackRecord
( 
  HGLOBAL __far * hBackList, 
  WORD __far * wBackCount, 
  char __far * fpHelpFilePath, 
  DWORD __far * dwTopicCharOffset, 
  WORD __far * wVertPosPtr               /* Topics scroll bar position. */
)
{
  HBACKREC  hOldTopRec;
  HBACKREC  hNewTopRec;
  FPBACKREC fpNewTopRec;
  FPBACKREC fpOldTopRec;
  FPBACKREC fpEndRec;

  char __far * fpTopFilePath;
   
                   
  /* Empty list? */
  if( *wBackCount == 0 ) return FALSE;

  /*
  **  Remove the last record.
  */ 

  /* Get current top. */
  hOldTopRec = *hBackList;
    
  /* Lock old top record. */
  fpOldTopRec = ( FPBACKREC ) GlobalLock( hOldTopRec );
  
  /* Get handle to the new topic record. */
  hNewTopRec = fpOldTopRec->hNext;
  

  /* Lock last record. */
  fpEndRec = ( FPBACKREC ) GlobalLock( fpOldTopRec->hPrev );
  
  /* Set last record to point to new top record. */
  fpEndRec->hNext = hNewTopRec;
    
  /* Unlock last record. */
  GlobalUnlock( fpOldTopRec->hPrev );


  /* Lock new top record. */
  fpNewTopRec = ( FPBACKREC ) GlobalLock( hNewTopRec );
  
  /* Set new top record to point to end record. */
  fpNewTopRec->hPrev = fpOldTopRec->hPrev;
    
  /* Get new top record's topic offset. */
  *dwTopicCharOffset = fpNewTopRec->dwTopicCharOffset;

  /* Lock the file path. */
  fpTopFilePath = GlobalLock( fpNewTopRec->hHelpFilePath );

  /* Get new top record's file path. */
  _fstrcpy( fpHelpFilePath, fpTopFilePath );

  /* Unlock the file path. */
  GlobalUnlock( fpNewTopRec->hHelpFilePath );

  /* Get new top record's vert. scroll position. */
  *wVertPosPtr = fpNewTopRec->wVertPos;

  /* Unlock new top record. */
  GlobalUnlock( hNewTopRec );


  /* Unlock old top record. */
  GlobalUnlock( hOldTopRec );
  
  /* Free old top record. */
  FreeBackRecord( hOldTopRec );
    
  /* Save handle of new top record. */
  *hBackList = hNewTopRec;

  /* Decrement the record count. */
  *wBackCount = *wBackCount - 1;

  /* Success. */
  return( TRUE );
}



static HGLOBAL __far __pascal AllocBackRecord( HWND hDataWnd, char __far * fpHelpFilePath, DWORD dwTopicCharOffset )
{
  HBACKREC  hBackRec;

                   
  /* Allocate record structure memory. */
  if( !GlobalAllocMem( hDataWnd, &hBackRec, sizeof(BACKREC) ) )
  {
    /* Failure. */
    return( NULL );
  }
  
  /* Store the info. in the new record. */
  if( ! SaveBackRecordData( hDataWnd, hBackRec, fpHelpFilePath, dwTopicCharOffset ) )
  {
    /* Error. */
    return( NULL );
  }

  /* Success. */
  return( hBackRec );
}


static BOOL __far __pascal RecycleBackRecord( HWND hDataWnd, HBACKREC hBackRec, char __far * fpHelpFilePath, DWORD dwTopicCharOffset )
{
  /* Free data allocated for data in the record. */
  ClearBackRecord( hBackRec );
  
  if( ! SaveBackRecordData( hDataWnd, hBackRec, fpHelpFilePath, dwTopicCharOffset ) )
  {
    /* Failure. */
    return( FALSE );
  }

  /* Success. */
  return( TRUE );
}


static void __far __pascal FreeBackRecord( HBACKREC hBackRec )
{
  /* Free data allocated for data in the record. */
  ClearBackRecord( hBackRec ); 
 
  /* Free the record structure. */
  GlobalFree( hBackRec );
}


static void __far __pascal ClearBackRecord( HBACKREC hBackRec )
{
  FPBACKREC fpBackRec;

  /* Lock the record. */
  fpBackRec = (FPBACKREC) GlobalLock( hBackRec );

  /* Free data allocated for the filepath. */
  GlobalFree( fpBackRec->hHelpFilePath );

  /* Unlock the record. */
  GlobalUnlock( hBackRec );
}


static BOOL __far __pascal SaveBackRecordData( HWND hDataWnd, HBACKREC hBackRec, char __far * fpHelpFilePath, DWORD dwTopicCharOffset )
{
  FPBACKREC fpBackRec;


  /* Lock the record. */
  fpBackRec = (FPBACKREC) GlobalLock( hBackRec );

  /*
  **  Save file's path. 
  */
  fpBackRec->hHelpFilePath = CopyString( hDataWnd, fpHelpFilePath );
  
  /* Copy error? */
  if( fpBackRec->hHelpFilePath == NULL )
  {
    /* Unlock the record. */
    GlobalUnlock( hBackRec );

    /* Failure. */
    return( FALSE );
  }
  

  /*
  **  Save topic offset. 
  */
  fpBackRec->dwTopicCharOffset = dwTopicCharOffset;

  /* Unlock the record. */
  GlobalUnlock( hBackRec );

  /* Success. */
  return( TRUE );
}


