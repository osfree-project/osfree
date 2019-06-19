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
**                               HELP FILE OPENING ROUTINES
**
**                            TOPIC ALLOC. / TOPIC FREE ROUTINES
**
**                         HELP FILE BUFFER & TOPIC BUFFER ROUTINES
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
#include "filebuff.h"
#include "twhlprc.h"
#include "msgbox.h"
#include "globals.h"
#include "wnddata.h"
#include "winmem.h"
#include "elements.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* Maximum number of files in the file buffer. */
#define MAXFILEBUFF  3

/* Maximum number of topics buffered per file. */
#define MAXTOPICBUFF 15


/***********************************
**
**  Private functions
**
***********************************/

static void __far __pascal ClearFileBufferRec( FPHLPFILEBUFFREC fpRec );
static RETVAL __far __pascal GetFileBufferRec( HGLOBAL hHelpFileBuffer, char __far * szHelpFile, FPHLPFILEBUFFREC fpSaveRec );
static void __far __pascal CopyFileBufferRec( FPHLPFILEBUFFREC fpDest, FPHLPFILEBUFFREC fpSrc );
static short int __far __pascal FreeOldestFileBufferRec( HGLOBAL hHelpFileBuffer, char __far * szKeepFileName );
static BOOL __far __pascal InsertFileBufferRec( HGLOBAL hHelpFileBuffer, FPHLPFILEBUFFREC fpRecData );

static RETVAL __far __pascal AllocTopicBuffer( HWND hErrorWnd, HGLOBAL __far * hTopicBuffer );
static void __far __pascal FreeTopicBuffer( HGLOBAL hTopicBuffer );

void __far __pascal ClearTopicBufferRec( FPTOPICBUFFREC fpRec );
static void __far __pascal CopyTopicBuffRec( FPTOPICBUFFREC fpDest, FPTOPICBUFFREC fpSrc );
static RETVAL __far __pascal FillTopicBuffRec( HWND hErrorWnd, DWORD dwTopicCharOffset, FPHLPFILEBUFFREC fpHelpBuffRec, FPTOPICBUFFREC fpTopicBuffRec );
static RETVAL __far __pascal InsertTopicBufferRec( HGLOBAL hTopicBuffer, FPTOPICBUFFREC fpRecData );
static RETVAL __far __pascal GrabTopicBufferRec( HGLOBAL hTopicBuffer, DWORD dwTopicCharOffset, 
                                                 FPTOPICBUFFREC fpSaveRec );
static void __far __pascal ReleaseTopicBufferRec( HGLOBAL hTopicBuffer, DWORD dwTopicCharOffset );
static BOOL __far __pascal AllTopicsReleased( HGLOBAL hTopicBuffer );
static short int __far __pascal FreeOldestTopicBufferRec( HGLOBAL hTopicBuffer );


/***************************** TOPIC ALLOC. / TOPIC FREE ROUTINES ******************************/

/***************************************************
*
* Opens and loads data for a help file.  Saves file
* info. into the help file buffer.
*
* Returns a handle to the help file's info.
*
****************************************************/
HGLOBAL __far _pascal OpenHelpFile( HWND hMainWnd, char __far * szHelpFilePath )
{
  RETVAL         RetVal;            /* Custom return value from functions. */

  HGLOBAL        hHelpFileBuffer;   /* Handle to the help file buffer. */
  
  HLPFILEBUFFREC HelpFileBufferRec; /* File buffer record. */
  
  HCURSOR        hOldCursor;        /* Handle to the old cursor. */
  
  
  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return( NULL );

  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, szHelpFilePath, (FPHLPFILEBUFFREC) &HelpFileBufferRec );

  /* 
  ** If help file not in buffer,
  ** load the help file's info.
  */
  if( RetVal != NO_ERROR )
  {
    /* Loading the help file info may take a while. */
    hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
  
    /* Load the help file info. */
    RetVal = LoadHelpFileInfo( hMainWnd, szHelpFilePath, &(HelpFileBufferRec).hHelpFileInfo );

    /* Restore original cursor. */
    SetCursor( hOldCursor );
  
    /* File loading has failed. */
    if( RetVal != NO_ERROR )
    {
      /* If we've had memory problems, report them now. */
      if( RetVal == ERR_MEMORY )
      {
        MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
      }

      /* Failure. */
      return( NULL );
    }

    /* 
    ** File loading has NOT failed. 
    */
    
    /* Create a topic buffer. */
    RetVal = AllocTopicBuffer( hMainWnd, &(HelpFileBufferRec).hTopicBuffer );    

    /* Topic buffer creation has failed. */
    if( RetVal != NO_ERROR )
    {
      /* If we've had memory problems, report them now. */
      if( RetVal == ERR_MEMORY )
      {
        MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
      }

      /* Failure. */
      return( NULL );
    }

    /* Save the file's path. */  
    _fstrcpy( HelpFileBufferRec.szHelpFile, szHelpFilePath ); 
      
    /*
    ** File buffer record successfully created.  Add record to buffer. 
    */
    InsertFileBufferRec( hHelpFileBuffer, &HelpFileBufferRec );
  }

  /* Success. */
  return( HelpFileBufferRec.hHelpFileInfo );
}



/***************************************************
*
* Create and returns a handle to a TOPICDATA 
* structure containing data for the desired
* help file/topic.
*
* If the file's topic data is not in the buffer, 
* the information is loaded and placed into
* the buffer for future use.
*
* The topic record in the topic buffer is locked
* and will not be removed from the buffer until
* the TOPICDATA is freed using the UnlockTopicData()
* function.
*
****************************************************/
HGLOBAL __far _pascal LockTopicData( HWND hMainWnd, char __far * szHelpFilePath, DWORD dwTopicCharOffset )
{
  RETVAL         RetVal;             /* Custom return value from functions. */

  HGLOBAL        hHelpFileBuffer;    /* Handle to the help file buffer. */
  
  HLPFILEBUFFREC HelpFileBufferRec;  /* File buffer record. */
  TOPICBUFFREC   TopicDataRec;       /* Topic buffer record. */
  
  HGLOBAL        hTopicData;         /* Handle to the topic data. */
  FPTOPICDATA    fpTopicData;        /* Pointer to the topic data. */
  
  HCURSOR        hOldCursor;        /* Handle to the old cursor. */
  

  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return( NULL );

  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, szHelpFilePath, (FPHLPFILEBUFFREC) &HelpFileBufferRec );

  /* Finding file has failed. */
  if( RetVal != NO_ERROR ) return( NULL );


  /* Retrieve topics data. */
  RetVal = GrabTopicBufferRec( HelpFileBufferRec.hTopicBuffer, dwTopicCharOffset, &TopicDataRec );

  /* 
  ** If topic not in buffer,
  ** load the topics info.
  */
  if( RetVal != NO_ERROR )
  {
    /* Loading the topic's info may take a while. */
    hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
  
    /* Load the topic's info. */
    RetVal = FillTopicBuffRec( hMainWnd, dwTopicCharOffset, &HelpFileBufferRec, &TopicDataRec );

    /* Restore original cursor. */
    SetCursor( hOldCursor );
  
    /* Topic loading has failed. */
    if( RetVal != NO_ERROR )
    {
      /* If we've had memory problems, report them now. */
      if( RetVal == ERR_MEMORY )
      {
        MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
      }

      /* Failure. */
      return( NULL );
    }

    /* Save topic data to buffer. */
    RetVal = InsertTopicBufferRec( HelpFileBufferRec.hTopicBuffer, &TopicDataRec );

    /* Topic data could not be saved to buffer. */
    if( RetVal != NO_ERROR ) 
    {
      /* Display error. */
      MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MAXTOPICS, MB_ICONHAND | MB_OK );
      
      /* Clear any memory allocated for the record. */
      ClearTopicBufferRec( &TopicDataRec );
      
      /* Failure. */
      return( NULL );
    }
  }

  /* Create a TOPICDATA structure. */
  hTopicData = GlobalAlloc( GHND, sizeof(TOPICDATA) );

  /* Creating structure has failed. */
  if( hTopicData == NULL )
  {
    /* Display error message. */
    MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );

    /* Clear any memory allocated for the record. */
    ClearTopicBufferRec( &TopicDataRec );
      
    /* Failure. */
    return( NULL );
  }

  /* Lock topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( hTopicData );  
  
  /* Load data into TOPICDATA. */
  fpTopicData->dwTopicCharOffset  = dwTopicCharOffset;
  _fstrcpy( fpTopicData->szHelpFile, HelpFileBufferRec.szHelpFile ); 
  fpTopicData->hHelpFileInfo = HelpFileBufferRec.hHelpFileInfo;
  fpTopicData->hNonScrollElements  = TopicDataRec.hNonScrollElements;
  fpTopicData->hScrollElements  = TopicDataRec.hScrollElements;
  fpTopicData->NextTopicCharOffset = TopicDataRec.NextTopicCharOffset; 
  fpTopicData->PrevTopicCharOffset = TopicDataRec.PrevTopicCharOffset; 
  
  /* Unlock topic data. */
  GlobalUnlock( hTopicData );  

  /* Success. */
  return( hTopicData );
}


/***************************************************
*
* Free a TOPICDATA structure.  The data that the
* structure contains is not freed because it is
* only a copy of what is in the help buffer and topic
* buffer.
*
* The topic record in the topic buffer is locked
* and will not be removed from the buffer until
* the TOPICDATA is unlocked using this
* function.
*
****************************************************/
void __far _pascal UnlockTopicData( HWND hMainWnd, HGLOBAL hTopicData )
{
  RETVAL         RetVal;             /* Custom return value from functions. */
                                    
  HGLOBAL        hHelpFileBuffer;    /* Handle to the help file buffer. */
  HLPFILEBUFFREC HelpFileBufferRec;  /* File buffer record. */
  
  FPTOPICDATA    fpTopicData;        /* Pointer to the topic data. */


  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return;

  /* Lock topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( hTopicData );  
  
  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, fpTopicData->szHelpFile, (FPHLPFILEBUFFREC) &HelpFileBufferRec );

  /* 
  ** Help file not in buffer.
  */
  if( RetVal != NO_ERROR ) return;

  /* Release the topic record's use. */
  ReleaseTopicBufferRec( HelpFileBufferRec.hTopicBuffer, fpTopicData->dwTopicCharOffset );

  /* Unlock topic data. */
  GlobalUnlock( hTopicData );  

  /* Free the structure. */
  GlobalFree( hTopicData );
}





/******************************** FILE BUFFER ROUTINES **********************************/

BOOL __far __pascal AllocHelpFileBuffer( HWND hMainWnd )
{
  HGLOBAL hHelpFileBuffer;
                          
                          
  /* Allocate memory for the topic buffer. */
  if( !GlobalAllocMem( hMainWnd, &hHelpFileBuffer, sizeof(HLPFILEBUFFREC) * MAXFILEBUFF  ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Set the handle to the file buffer. */
  WndData( hMainWnd, WDM_SETFILEBUFFER, (DWORD) hHelpFileBuffer, NULL );
  
  /* Success. */
  return( TRUE );
}



void __far __pascal FreeHelpFileBuffer( HWND hMainWnd  )
{
  HGLOBAL hHelpFileBuffer;         /* Help file buffer. */
  FPHLPFILEBUFFREC fpRec;          /* A pointer to a record in the file buffer. */
  short int nCounter;              /* Simple counter variable. */


  /* Get the handle to the file buffer. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  if( hHelpFileBuffer == NULL ) return;
  
  /* 
  ** Free all information in the buffer. 
  */

  /* Lock first record. */
  fpRec = (FPHLPFILEBUFFREC) GlobalLock( hHelpFileBuffer );
  
  /* Find record that are being used. */
  for( nCounter = 0; nCounter < MAXFILEBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid != FALSE )
    {
      /* Free memory inside of record. */
      ClearFileBufferRec( fpRec ); 
    }

    /* Go to next record. */
    fpRec++;
  }    
    
  /* Unlock the buffer. */
  GlobalUnlock( hHelpFileBuffer );

  /* Free the buffer itself. */
  GlobalFree( hHelpFileBuffer );

  /* Clear the handle to the file buffer in the window data. */
  WndData( hMainWnd, WDM_SETFILEBUFFER, 0L, NULL );
}



static void __far __pascal ClearFileBufferRec( FPHLPFILEBUFFREC fpRec )
{
  /* Free the file's HLPFILEINFO structure. */
  if( fpRec->hHelpFileInfo != NULL ) FreeHelpFileInfo( fpRec->hHelpFileInfo );  

  /* Free the file's topic buffer. */
  if( fpRec->hTopicBuffer != NULL ) FreeTopicBuffer( fpRec->hTopicBuffer );    

  /* Record does not contain valid info. */
  fpRec->bValid = FALSE;
}



static RETVAL __far __pascal GetFileBufferRec( HGLOBAL hHelpFileBuffer, char __far * szHelpFile, FPHLPFILEBUFFREC fpSaveRec )
{
  FPHLPFILEBUFFREC fpRec;          /* A pointer to a record in the file buffer. */
  short int nCounter;              /* Simple counter variable. */
  short int nResult;               /* Result of string compare. */

  
  /* Lock first record. */
  fpRec = (FPHLPFILEBUFFREC) GlobalLock( hHelpFileBuffer );
  
  /* For each record in the buffer. */
  for( nCounter = 0; nCounter < MAXFILEBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* See if its the right file. */
      nResult = _fstricmp( fpRec->szHelpFile, szHelpFile );
      
      /* Found right record? */
      if( nResult == 0 )
      {
        /* Set record's last used time. */
        fpRec->dwTickCount = GetTickCount();

        /* Copy record's data. */
        CopyFileBufferRec( fpSaveRec, fpRec );
  
        /* Unlock buffer. */
        GlobalUnlock( hHelpFileBuffer );
  
        /* Success. */
        return( NO_ERROR );
      }
    }
    
    /* Go to next record. */
    fpRec++;
  }

  /* Unlock buffer. */
  GlobalUnlock( hHelpFileBuffer );

  /* Failure. */
  return( ERR_OTHER );
}


  
/***************************************************
*
* Copy the fields of a file buffer record from
* one record to another.
*
****************************************************/
static void __far __pascal CopyFileBufferRec( FPHLPFILEBUFFREC fpDest, FPHLPFILEBUFFREC fpSrc )
{
  fpDest->bValid        = fpSrc->bValid;

  fpDest->dwTickCount   = fpSrc->dwTickCount;
                       
  fpDest->hHelpFileInfo = fpSrc->hHelpFileInfo;       

  fpDest->hTopicBuffer  = fpSrc->hTopicBuffer;       

  _fstrcpy( fpDest->szHelpFile, fpSrc->szHelpFile );
} 




static short int __far __pascal FreeOldestFileBufferRec( HGLOBAL hHelpFileBuffer, char __far * szKeepFileName )
{
  FPHLPFILEBUFFREC fpRec, fpStart;          /* A pointer to a record in the file buffer. */
                                     
  short int nCounter;                       /* Simple counter variable. */
   
  DWORD dwOldestTick;                       /* What's the tick count of the oldest file. */
  short int nOldestRecNum;                  /* What's the record number of the oldest file. */
  
  short int nResult;                        /* Result of string compare. */

  BOOL BFoundOne;
  
  
  /* Have not found a file to free. */
  BFoundOne = FALSE;
  
  /* Lock first record. */
  fpStart = fpRec = (FPHLPFILEBUFFREC) GlobalLock( hHelpFileBuffer );
  
  /* Find first record that we can use as initial oldest record. */
  for( nCounter = 0; nCounter < MAXFILEBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* See if its a file to ignore. */
      nResult = _fstricmp( fpRec->szHelpFile, szKeepFileName );
      
      /* Should we consider releasing this file? */
      if( nResult != 0 )
      {
        /* 
        ** Verify that the topics in the file's
        ** topic buffer are not locked.
        */
        if( AllTopicsReleased( fpRec->hTopicBuffer ) )
        {
          /* Found initial record to free. */
          dwOldestTick = fpRec->dwTickCount;
          nOldestRecNum = nCounter;
          BFoundOne = TRUE;
        }
      }
    }

    /* Go to next record. */
    fpRec++;
  }    
    
  /* Have not found a file to free. */
  if( !BFoundOne )
  {
    /* Unlock buffer. */
    GlobalUnlock( hHelpFileBuffer );
  
    /* Failure. */
    return( -1 );
  }
  
  /* Now compare the rest of the records with our current file to be freed. */
  for( ; nCounter < MAXFILEBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* See if its a file to ignore. */
      nResult = _fstricmp( fpRec->szHelpFile, szKeepFileName );
      
      /* Should consider use file? */
      if( nResult != 0 )
      {
        /* 
        ** Verify that the topics in the file's
        ** topic buffer are not locked.
        */
        if( AllTopicsReleased( fpRec->hTopicBuffer ) )
        {
          /* Older? */
          if( fpRec->dwTickCount < dwOldestTick )
          {
            /* Found initial record to free. */
            dwOldestTick = fpRec->dwTickCount;
            nOldestRecNum = nCounter;
          }
        }
      }
    }
      
    /* Go to next record. */
    fpRec++;
  }


  /* 
  ** We DID find a record to free - lets free it. 
  */
  
  /* Goto record. */
  fpRec = fpStart + nOldestRecNum;
    
  /* Free information inside of the record. */
  ClearFileBufferRec( fpRec );
  
  /* Unlock buffer. */
  GlobalUnlock( hHelpFileBuffer );

  /* Success. */
  return( nOldestRecNum );
}



static BOOL __far __pascal InsertFileBufferRec( HGLOBAL hHelpFileBuffer, FPHLPFILEBUFFREC fpRecData )
{
  FPHLPFILEBUFFREC fpRec, fpStart;    /* A pointer to a record in the file buffer. */
  short int nCounter;                 /* Simple counter variable. */
  short int nFreeRecord;              /* Record number of freed record. */
  
   
  /* Lock first record. */
  fpStart = fpRec = (FPHLPFILEBUFFREC) GlobalLock( hHelpFileBuffer );
  
  /* Find first record that is not used. */
  for( nCounter = 0; nCounter < MAXFILEBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == FALSE )
    {
      break; 
    }

    /* Go to next record. */
    fpRec++;
  }    
    
  /* No unused records. */
  if( nCounter == MAXFILEBUFF )
  { 
    /* Free the oldest file record in the buffer. */
    nFreeRecord = FreeOldestFileBufferRec( hHelpFileBuffer, "\0" ); 

    /* Can't free a record. */
    if( nFreeRecord == -1 )
    {
      /* Unlock buffer. */
      GlobalLock( hHelpFileBuffer );
      
      /* Failure. */
      return( FALSE );
    }
    
    /* Get the first record. */
    fpRec = fpStart + nFreeRecord;
  
  }
  
  /* 
  ** Now that we have a record - lets save the data. 
  */
  
  /* Copy fields. */
  CopyFileBufferRec( fpRec, fpRecData );

  /* Record now has data. */
  fpRec->bValid = TRUE;
  
  /* Set record's insertion time. */
  fpRec->dwTickCount = GetTickCount();

  /* Unlock buffer. */
  GlobalLock( hHelpFileBuffer );
      
  /* Success. */
  return( TRUE );
}



/******************************** TOPIC BUFFER ROUTINES **********************************/

static RETVAL __far __pascal AllocTopicBuffer( HWND hErrorWnd, HGLOBAL __far * hTopicBuffer )
{
  /* Allocate memory for the topic buffer. */
  *hTopicBuffer = GlobalAlloc( GHND, sizeof(TOPICBUFFREC) * MAXTOPICBUFF );
  
  /* Error? */
  if( *hTopicBuffer == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }
  
  /* Success. */
  return( NO_ERROR );
}


static void __far __pascal FreeTopicBuffer( HGLOBAL hTopicBuffer )
{
  FPTOPICBUFFREC fpRec;            /* A pointer to a record in the topic buffer. */
  short int nCounter;              /* Simple counter variable. */

  /* 
  ** Free all information in the buffer. 
  */

  /* Lock first record. */
  fpRec = (FPTOPICBUFFREC) GlobalLock( hTopicBuffer );
  
  /* Find record that are being used. */
  for( nCounter = 0; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid != FALSE )
    {
      /* Free memory inside of record. */
      ClearTopicBufferRec( fpRec ); 
    }

    /* Go to next record. */
    fpRec++;
  }    
    
  /* Unlock the buffer. */
  GlobalUnlock( hTopicBuffer );

  /* Free the buffer itself. */
  GlobalFree( hTopicBuffer );
}


/***************************************************
*
* Clears any memory INSIDE of a TOPICBUFFREC structure. 
*
****************************************************/
static void __far __pascal ClearTopicBufferRec( FPTOPICBUFFREC fpRec )
{
  /* Free the nonscrollable element list's information. */
  DestroyElementList( &(fpRec)->hNonScrollElements );

  /* Free the scrollable element list's information. */
  DestroyElementList( &(fpRec)->hScrollElements );

  /* Record does not contain valid info. */
  fpRec->bValid = FALSE;
}


/***************************************************
*
* Copy the fields of a topic buffer record from
* one record to another.
*
****************************************************/
static void __far __pascal CopyTopicBuffRec( FPTOPICBUFFREC fpDest, FPTOPICBUFFREC fpSrc )
{
  fpDest->bValid       = fpSrc->bValid;

  fpDest->dwTickCount  = fpSrc->dwTickCount;
                       
  fpDest->dwUsage      = fpSrc->dwUsage;       

  fpDest->dwTopicCharOffset = fpSrc->dwTopicCharOffset;       
  
  fpDest->hNonScrollElements = fpSrc->hNonScrollElements;
  
  fpDest->hScrollElements = fpSrc->hScrollElements;
  
  fpDest->NextTopicCharOffset = fpSrc->NextTopicCharOffset;
  
  fpDest->PrevTopicCharOffset = fpSrc->PrevTopicCharOffset;
} 


/***************************************************
*
* Load topic data for a topic buffer record.
*
****************************************************/
static RETVAL __far __pascal FillTopicBuffRec( HWND hErrorWnd, DWORD dwTopicCharOffset, FPHLPFILEBUFFREC fpHelpBuffRec, FPTOPICBUFFREC fpTopicBuffRec )
{
  RETVAL        RetVal;           /* Custom return value. */

  RetVal = CreateElementLists( hErrorWnd, 
                               dwTopicCharOffset,
                               &(fpTopicBuffRec)->hNonScrollElements,
                               &(fpTopicBuffRec)->hScrollElements,
                               &(fpTopicBuffRec)->PrevTopicCharOffset,
                               &(fpTopicBuffRec)->NextTopicCharOffset,
                               fpHelpBuffRec->hHelpFileInfo );

  /* Save block/offset information to record. */
  fpTopicBuffRec->dwTopicCharOffset = dwTopicCharOffset;
        
  /* Return the error that we got. */
  return( RetVal );
}


/***************************************************
*
* Inserts the a topic record's data into the topic buffer.
*
****************************************************/
static RETVAL __far __pascal InsertTopicBufferRec( HGLOBAL hTopicBuffer, FPTOPICBUFFREC fpRecData )
{
  FPTOPICBUFFREC fpRec, fpStart;    /* A pointer to a record in the topic buffer. */
  short int nCounter;               /* Simple counter variable. */
  short int nFreeRecord;            /* Record number of freed record. */

   
  /* Lock first record. */
  fpStart = fpRec = (FPTOPICBUFFREC) GlobalLock( hTopicBuffer );
  
  /* Find first record that is not used. */
  for( nCounter = 0; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == FALSE )
    {
      break; 
    }

    /* Go to next record. */
    fpRec++;
  }    
    
  /* No unused records. */
  if( nCounter == MAXTOPICBUFF )
  { 
    /* Free the oldest file record in the buffer. */
    nFreeRecord = FreeOldestTopicBufferRec( hTopicBuffer ); 
    
    if( nFreeRecord == -1 )
    {
      /* Unlock buffer. */
      GlobalLock( hTopicBuffer );

      return( ERR_OTHER );
    }

    /* Get the first record. */
    fpRec = fpStart + nFreeRecord;
  }
  
  /* 
  ** Now that we have a record - lets save the data. 
  */
  
  /* Copy fields. */
  CopyTopicBuffRec( fpRec, fpRecData );

  /* Record now has data. */
  fpRec->bValid = TRUE;
  
  /* Set record's insertion time. */
  fpRec->dwTickCount = GetTickCount();

  /* Assume that on insertion we are using the topic data. */
  fpRec->dwUsage = 1;       
  
  /* Unlock buffer. */
  GlobalLock( hTopicBuffer );

  return( NO_ERROR );
}



/***************************************************
*
* Grab the information for a topic from the
* topic buffer. 
*
* Increments the usage count for the topic.
*
****************************************************/
static RETVAL __far __pascal GrabTopicBufferRec
( 
  HGLOBAL hTopicBuffer,  
  DWORD dwTopicCharOffset, 
  FPTOPICBUFFREC fpSaveRec 
)
{
  FPTOPICBUFFREC fpRec;            /* A pointer to a record in the topic buffer. */
  short int nCounter;              /* Simple counter variable. */
   
  
  /* Lock first record. */
  fpRec = (FPTOPICBUFFREC) GlobalLock( hTopicBuffer );
  
  /* For each record in the buffer. */
  for( nCounter = 0; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* Found right record? */
      if( fpRec->dwTopicCharOffset == dwTopicCharOffset )
      {
        /* Increment topic usage count. */
        fpRec->dwUsage = fpRec->dwUsage + 1;
        
        /* Set record's last used time. */
        fpRec->dwTickCount = GetTickCount();

        /* Copy record's data. */
        CopyTopicBuffRec( fpSaveRec, fpRec );
  
        /* Unlock buffer. */
        GlobalUnlock( hTopicBuffer );
  
        /* Success. */
        return( NO_ERROR );
      }
    }
    
    /* Go to next record. */
    fpRec++;
  }

  /* Unlock buffer. */
  GlobalUnlock( hTopicBuffer );

  /* Failure. */
  return( ERR_OTHER );
}


/***************************************************
*
* Release a topic in the topic buffer by
* finding it and decrementing its usage count.
*
****************************************************/
static void __far __pascal ReleaseTopicBufferRec
( 
  HGLOBAL hTopicBuffer,  
  DWORD dwTopicCharOffset
)
{
  FPTOPICBUFFREC fpRec;            /* A pointer to a record in the topic buffer. */
  short int nCounter;              /* Simple counter variable. */
   
  
  /* Lock first record. */
  fpRec = (FPTOPICBUFFREC) GlobalLock( hTopicBuffer );
  
  /* For each record in the buffer. */
  for( nCounter = 0; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* Found right record? */
      if( fpRec->dwTopicCharOffset == dwTopicCharOffset ) 
      {
        /* Decrement topic usage count. */
        if( fpRec->dwUsage > 0 ) 
        {
          fpRec->dwUsage = fpRec->dwUsage - 1;
        }
        
        /* Unlock buffer. */
        GlobalUnlock( hTopicBuffer );
  
        /* Success. */
        return;
      }
    }
    
    /* Go to next record. */
    fpRec++;
  }

  /* Unlock buffer. */
  GlobalUnlock( hTopicBuffer );
}


/***************************************************
*
* Tells if all topics in a topic buffer are currently
* released ( not being used ).
*
* Returns: BOOL
*
*    TRUE:  All topics in the buffer are released.
*    FALSE: One or more topics in the buffe are currently
*           being used (grabbed).
*
****************************************************/
static BOOL __far __pascal AllTopicsReleased( HGLOBAL hTopicBuffer )
{
  FPTOPICBUFFREC fpRec;            /* A pointer to a record in the topic buffer. */
  short int nCounter;              /* Simple counter variable. */
   
  
  /* Lock first record. */
  fpRec = (FPTOPICBUFFREC) GlobalLock( hTopicBuffer );
  
  /* For each record in the buffer. */
  for( nCounter = 0; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* Decrement topic usage count. */
      if( fpRec->dwUsage > 0 ) 
      {
        /* Unlock buffer. */
        GlobalUnlock( hTopicBuffer );
  
        /* Found a topic being used. */
        return( FALSE );
      }
    }
    
    /* Go to next record. */
    fpRec++;
  }

  /* Unlock buffer. */
  GlobalUnlock( hTopicBuffer );

  /* Found no topics being used. */
  return( TRUE );
}


/***************************************************
*
* Free the oldest topic record from the topic buffer.
*
****************************************************/
static short int __far __pascal FreeOldestTopicBufferRec( HGLOBAL hTopicBuffer )
{
  FPTOPICBUFFREC fpRec, fpStart;            /* A pointer to a record in the topic buffer. */
                                     
  short int nCounter;                       /* Simple counter variable. */
   
  DWORD dwOldestTick;                       /* What's the tick count of the oldest topic. */
  short int nOldestRecNum;                  /* What's the record number of the oldest topic. */
  BOOL bFoundIt;
    
  /* Haven't found a topic to free. */
  bFoundIt = FALSE;
  
  /* Lock first record. */
  fpStart = fpRec = (FPTOPICBUFFREC) GlobalLock( hTopicBuffer );
  
  /* Find first record that we can use as initial oldest record. */
  for( nCounter = 0; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* 
      ** Should we can consider this topic? 
      ** Only if its usage count is zero.
      */
      if( fpRec->dwUsage == 0 )
      {
        /* Found initial record to free. */
        dwOldestTick = fpRec->dwTickCount;
        nOldestRecNum = nCounter;
        bFoundIt = TRUE;
      }
    }

    /* Go to next record. */
    fpRec++;
  }    
    
  /* Haven't found a topic to free. */
  if( !bFoundIt )
  {
    /* Unlock buffer. */
    GlobalUnlock( hTopicBuffer );
  
    /* Failure. */
    return( -1 );
  }
  

  /* Now compare the rest of the records with our current file to be freed. */
  for( ; nCounter < MAXTOPICBUFF; nCounter++ )
  {
    /* Does record contain valid info? */
    if( fpRec->bValid == TRUE )
    {
      /* 
      ** Should we can consider this file? 
      ** Only if its usage count is zero.
      */
      if( fpRec->dwUsage == 0 )
      {
        /* Found an older topic. */
        if( fpRec->dwTickCount < dwOldestTick )
        {
          /* Assign curent record as one to free. */
          dwOldestTick = fpRec->dwTickCount;
          nOldestRecNum = nCounter;
        }
      }
    }
      
    /* Go to next record. */
    fpRec++;
  }

  /* 
  ** We DID find a record to free - lets free it. 
  */
  
  /* Goto record. */
  fpRec = fpStart + nOldestRecNum;
    
  /* Free information inside of the record. */
  ClearTopicBufferRec( fpRec );
  
  /* Unlock buffer. */
  GlobalUnlock( hTopicBuffer );

  /* Success. */
  return( nOldestRecNum );
}


#if 0

THESE TWO FUNCTIONS ARE THE ORIGINALS OF THOSE AT THE TOP OF THE FILE
EXCEPT THAT THESE ATTEMPT TO FREE DATA (MEMORY) USED IN THE TOPIC/FILE BUFFER
WHEN A MEMORY ALLOCATION ERROR OCCURS..

/***************************************************
*
* Opens and loads data for a help file.  Saves file
* info. into the help file buffer.
*
* Returns a handle to the help file's info.
*
****************************************************/
HGLOBAL __far _pascal OpenHelpFile( HWND hMainWnd, char __far * szHelpFilePath )
{
  RETVAL         RetVal;            /* Custom return value from functions. */

  HGLOBAL        hHelpFileBuffer;   /* Handle to the help file buffer. */
  
  HLPFILEBUFFREC HelpFileBufferRec; /* File buffer record. */
  

  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return( NULL );

  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, szHelpFilePath, (FPHLPFILEBUFFREC) &HelpFileBufferRec );

  /* 
  ** If help file not in buffer,
  ** load the help file's info.
  */
  if( RetVal != NO_ERROR )
  {
    /* Load the help file info. */
    RetVal = LoadHelpFileInfo( hMainWnd, szHelpFilePath, &(HelpFileBufferRec).hHelpFileInfo );

    /* If we failed due to a memory error. */
    while( RetVal == ERR_MEMORY )
    {
      /* Free oldest file's data and try again. */
      if( FreeOldestFileBufferRec( hHelpFileBuffer, "\0" ) )
      {
        /* Load the help file info. */
        RetVal = LoadHelpFileInfo( hMainWnd, szHelpFilePath, &(HelpFileBufferRec).hHelpFileInfo );
      }
          
      /* Couldn't free memory. */
      else
      {
        MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
        return( NULL );
      }
    }

    /* File loading has failed. */
    if( RetVal != NO_ERROR ) return( NULL );

    /* 
    ** File loading has NOT failed. 
    */
    
    /* Create a topic buffer. */
    RetVal = AllocTopicBuffer( hMainWnd, &(HelpFileBufferRec).hTopicBuffer );    

    /* If we failed due to a memory error. */
    while( RetVal == ERR_MEMORY )
    {
      /* Free oldest file's data (except the one we are trying to open) and try again. */
      if( FreeOldestFileBufferRec( hHelpFileBuffer, szHelpFilePath ) )
      {
        /* Load the help file info. */
        RetVal = AllocTopicBuffer( hMainWnd, &(HelpFileBufferRec).hTopicBuffer );
      }
          
      /* Couldn't free memory. */
      else
      {
        MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
        return( NULL );
      }
    }

    /* Topic buffer creation has failed. */
    if( RetVal != NO_ERROR ) return( NULL );

    /* Save the file's path. */  
    _fstrcpy( HelpFileBufferRec.szHelpFile, szHelpFilePath ); 
      
    /*
    ** File buffer record successfully created.  Add record to buffer. 
    */
    InsertFileBufferRec( hHelpFileBuffer, &HelpFileBufferRec );
  }

  /* Success. */
  return( HelpFileBufferRec.hHelpFileInfo );
}



/***************************************************
*
* Create and returns a handle to a TOPICDATA 
* structure containing data for the desired
* help file/topic.
*
* If the file's topic data is not in the buffer, 
* the information is loaded and placed into
* the buffer for future use.
*
* The topic record in the topic buffer is locked
* and will not be removed from the buffer until
* the TOPICDATA is freed using the UnlockTopicData()
* function.
*
****************************************************/
HGLOBAL __far _pascal LockTopicData( HWND hMainWnd, char __far * szHelpFilePath, DWORD dwTopicCharOffset )
{
  RETVAL         RetVal;             /* Custom return value from functions. */

  HGLOBAL        hHelpFileBuffer;    /* Handle to the help file buffer. */
  
  HLPFILEBUFFREC HelpFileBufferRec;  /* File buffer record. */
  TOPICBUFFREC   TopicDataRec;       /* Topic buffer record. */
  
  HGLOBAL        hTopicData;         /* Handle to the topic data. */
  FPTOPICDATA    fpTopicData;        /* Pointer to the topic data. */
  

  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return( NULL );

  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, szHelpFilePath, (FPHLPFILEBUFFREC) &HelpFileBufferRec );

  /* Finding file has failed. */
  if( RetVal != NO_ERROR ) return( NULL );


  /* Retrieve topics data. */
  RetVal = GrabTopicBufferRec( HelpFileBufferRec.hTopicBuffer, dwTopicCharOffset, &TopicDataRec );

  /* 
  ** If topic not in buffer,
  ** load the topics info.
  */
  if( RetVal != NO_ERROR )
  {
    /* Load the topic's info. */
    RetVal = FillTopicBuffRec( hMainWnd, dwTopicCharOffset, &HelpFileBufferRec, &TopicDataRec );

    /* If we failed due to a memory error. */
    if( RetVal == ERR_MEMORY )
    {
      /* Free oldest file's data (except the one we are trying to open) and try again. */
      while( FreeOldestFileBufferRec( hHelpFileBuffer, szHelpFilePath ) )
      {
        /* Load the help file info. */
        RetVal = FillTopicBuffRec( hMainWnd, dwTopicCharOffset, &HelpFileBufferRec, &TopicDataRec );

        /* Still had a memory error? */
        if( RetVal != ERR_MEMORY ) break;
      }
    }
          
    /* Couldn't free anymore files - try freeing unused topics in current file. */
    if( RetVal == ERR_MEMORY )
    {
      /* Free oldest topic's data and try again. */
      while( FreeOldestTopicBufferRec( HelpFileBufferRec.hTopicBuffer ) )
      {
        /* Load the help file info. */
        RetVal = FillTopicBuffRec( hMainWnd, dwTopicCharOffset, &HelpFileBufferRec, &TopicDataRec );
        
        /* Still had a memory error? */
        if( RetVal != ERR_MEMORY ) break;
      }
    }
          
    /* Topic loading has failed. */
    if( RetVal != NO_ERROR )
    {
      /* If we've had memory problems, report them now. */
      if( RetVal == ERR_MEMORY )
      {
        MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
      }

      /* Failure. */
      return( NULL );
    }

    /* Save topic data to buffer. */
    RetVal = InsertTopicBufferRec( HelpFileBufferRec.hTopicBuffer, &TopicDataRec );

    /* Topic data could not be saved to buffer. */
    if( RetVal != NO_ERROR ) 
    {
      /* Display error. */
      MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MAXTOPICS, MB_ICONHAND | MB_OK );
      
      /* Clear any memory allocated for the record. */
      ClearTopicBufferRec( &TopicDataRec );
      
      /* Failure. */
      return( NULL );
    }
  }

  /* Create a TOPICDATA structure. */
  hTopicData = GlobalAlloc( GHND, sizeof(TOPICDATA) );

  /* Allocation error? */
  if( hTopicData == NULL )
  {
    /* Free oldest file's data (except the one we are trying to open) and try again. */
    while( FreeOldestFileBufferRec( hHelpFileBuffer, szHelpFilePath ) )
    {
      /* Load the help file info. */
      hTopicData = GlobalAlloc( GHND, sizeof(TOPICDATA) );

      /* Still had a memory error? */
      if( hTopicData != NULL ) break;
    }
          
    /* Couldn't free anymore files - try freeing unused topics in current file. */
    if( hTopicData == NULL )
    {
      /* Free oldest topic's data and try again. */
      while( FreeOldestTopicBufferRec( HelpFileBufferRec.hTopicBuffer ) )
      {
        /* Load the help file info. */
        hTopicData = GlobalAlloc( GHND, sizeof(TOPICDATA) );
        
        /* Still had a memory error? */
        if( hTopicData != NULL ) break;
      }
    }
          
    /* Creating structure has failed. */
    if( hTopicData == NULL )
    {
      /* Display error message. */
      MsgBox( GetLibInst(), hMainWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );

      /* Clear any memory allocated for the record. */
      ClearTopicBufferRec( &TopicDataRec );
      
      /* Failure. */
      return( NULL );
    }
  }

  /* Lock topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( hTopicData );  
  
  /* Load data into TOPICDATA. */
  fpTopicData->dwTopicCharOffset  = dwTopicCharOffset;
  _fstrcpy( fpTopicData->szHelpFile, HelpFileBufferRec.szHelpFile ); 
  fpTopicData->hHelpFileInfo = HelpFileBufferRec.hHelpFileInfo;
  fpTopicData->hNonScrollElements  = TopicDataRec.hNonScrollElements;
  fpTopicData->hScrollElements  = TopicDataRec.hScrollElements;
  fpTopicData->NextTopicCharOffset = TopicDataRec.NextTopicCharOffset; 
  fpTopicData->PrevTopicCharOffset = TopicDataRec.PrevTopicCharOffset; 
  
  /* Unlock topic data. */
  GlobalUnlock( hTopicData );  

  /* Success. */
  return( hTopicData );
}
#endif
