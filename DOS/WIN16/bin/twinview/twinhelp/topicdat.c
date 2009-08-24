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
**                                 TOPIC DATA RETRIEVAL ROUTINES
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
#include "topicdata.h"
#include "twhlprc.h"
#include "hlpfile.h"
#include "filebuff.h"
#include "msgbox.h"
#include "globals.h"
#include "wnddata.h"


/***********************************
**
**  Private Defines
**
***********************************/


/***********************************
**
**  Private functions
**
***********************************/




/***************************************************
*
* Create and returns a handle to a TOPICDATA 
* structure containing data for the desired
* help file/topic.
*
* If the file's general info. or the the topic
* data is not in the buffer, the information is loaded
* and placed into the buffer for future use.
*
* The topic record in the topic buffer is locked
* and will not be removed from the buffer until
* the TOPICDATA is freed using the FreeTopicData()
* function.
*
****************************************************/
HTOPICDATA __far _pascal AllocTopicData( HWND hMainWnd, char __far * szHelpFilePath, DWORD dwBlockNum, WORD wBlockOffset )
{
  RETVAL       RetVal;            /* Custom return value from functions. */

  HGLOBAL      hHelpFileBuffer;   /* Handle to the help file buffer. */
  
  HLPFILEBUFFREC   HelpBufferRec;     /* File buffer record. */
  TOPICBUFFREC TopicDataRec;      /* Topic buffer record. */
  
  HGLOBAL      hTopicData;        /* Handle to the topic data. */
  FPTOPICDATA  fpTopicData;       /* Pointer to the topic data. */
  

  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return( NULL );

  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, szHelpFilePath, (FPHLPFILEBUFFREC) &HelpBufferRec );

  /* 
  ** If help file not in buffer,
  ** load the help file's info.
  */
  if( RetVal != NO_ERROR )
  {
    /* Load the help file info. */
    RetVal = LoadHelpFileInfo( hMainWnd, szHelpFilePath, &(HelpBufferRec).hHelpFileInfo );

    /* If we failed due to a memory error. */
    while( RetVal == ERR_MEMORY )
    {
      /* Free oldest file's data and try again. */
      if( FreeOldestFileBufferRec( hHelpFileBuffer, "\0" ) )
      {
        /* Load the help file info. */
        RetVal = LoadHelpFileInfo( hMainWnd, szHelpFilePath, &(HelpBufferRec).hHelpFileInfo );
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
    RetVal = AllocTopicBuffer( hMainWnd, &(HelpBufferRec).hHelpFileInfo );    

    /* If we failed due to a memory error. */
    while( RetVal == ERR_MEMORY )
    {
      /* Free oldest file's data (except the one we are trying to open) and try again. */
      if( FreeOldestFileBufferRec( hHelpFileBuffer, szHelpFilePath ) )
      {
        /* Load the help file info. */
        RetVal = AllocTopicBuffer( hMainWnd, &(HelpBufferRec).hHelpFileInfo );
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
    _fstrcpy( HelpBufferRec.szHelpFile, szHelpFilePath ); 
      
    /*
    ** File buffer record successfully created.  Add record to buffer. 
    */
    InsertFileBufferRec( hHelpFileBuffer, &HelpBufferRec );
  }


  /* Retrieve topics data. */
  RetVal = GrabTopicBufferRec( HelpBufferRec.hTopicBuffer, dwBlockNum, wBlockOffset,
                               &TopicDataRec );

  /* 
  ** If topic not in buffer,
  ** load the topics info.
  */
  if( RetVal != NO_ERROR )
  {
    /* Load the help file info. */
    RetVal = FillTopicBuffRec( hMainWnd, dwBlockNum, wBlockOffset, &HelpBufferRec, &TopicDataRec );

    /* If we failed due to a memory error. */
    if( RetVal == ERR_MEMORY )
    {
      /* Free oldest file's data (except the one we are trying to open) and try again. */
      while( FreeOldestFileBufferRec( hHelpFileBuffer, szHelpFilePath ) )
      {
        /* Load the help file info. */
        RetVal = FillTopicBuffRec( hMainWnd, dwBlockNum, wBlockOffset, &HelpBufferRec, &TopicDataRec );

        /* Still had a memory error? */
        if( RetVal != ERR_MEMORY ) break;
      }
    }
          
    /* Couldn't free anymore files - try freeing unused topics in current file. */
    if( RetVal == ERR_MEMORY )
    {
      /* Free oldest topic's data and try again. */
      while( FreeOldestTopicBufferRec( HelpBufferRec.hTopicBuffer ) )
      {
        /* Load the help file info. */
        RetVal = FillTopicBuffRec( hMainWnd, dwBlockNum, wBlockOffset, &HelpBufferRec, &TopicDataRec );
        
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
    RetVal = InsertTopicBufferRec( HelpBufferRec.hTopicBuffer, &TopicDataRec );

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

  /* Create a DISPLAYDATA structure. */
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
      while( FreeOldestTopicBufferRec( HelpBufferRec.hTopicBuffer ) )
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
  fpTopicData->dwBlockNum    = dwBlockNum;
  fpTopicData->wBlockOffset  = wBlockOffset;
  _fstrcpy( fpTopicData->szHelpFile, HelpBufferRec.szHelpFile ); 
  fpTopicData->hHelpFileInfo = HelpBufferRec.hHelpFileInfo;
  fpTopicData->hElementList  = TopicDataRec.hElementList;
  
  /* Unlock topic data. */
  GlobalUnlock( hTopicData );  

  /* Success. */
  return( hTopicData );
}


/***************************************************
*
* Free a TOPICDATA structure.  The data that the
* structure contains is not free because it is
* only a copy of what is in the help buffer and topic
* buffer.
*
* The topic record in the topic buffer is locked
* and will not be removed from the buffer until
* the TOPICDATA is freed using this
* function.
*
****************************************************/
void __far _pascal FreeTopicData( HWND hMainWnd, HTOPICDATA hTopicData )
{
  RETVAL     RetVal;            /* Custom return value from functions. */

  HGLOBAL    hHelpFileBuffer;   /* Handle to the help file buffer. */
  HLPFILEBUFFREC HelpBufferRec;     /* File buffer record. */
  
  FPTOPICDATA  fpTopicData;       /* Pointer to the topic data. */


  /* Get help file buffer handle from HWND of main window. */
  hHelpFileBuffer = (HGLOBAL) WndData( hMainWnd, WDM_GETFILEBUFFER, 0L, NULL );

  /* Error - No buffer. */
  if( hHelpFileBuffer == NULL ) return;

  /* Lock topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( hTopicData );  
  
  /* Find help file in file buffer. */
  RetVal = GetFileBufferRec( hHelpFileBuffer, fpTopicData->szHelpFile, (FPHLPFILEBUFFREC) &HelpBufferRec );

  /* 
  ** Help file not in buffer.
  */
  if( RetVal != NO_ERROR ) return;

  /* Release the topic record's use. */
  ReleaseTopicBufferRec( HelpBufferRec.hTopicBuffer, fpTopicData->dwBlockNum, fpTopicData->wBlockOffset );

  /* Unlock topic data. */
  GlobalUnlock( hTopicData );  

  /* Free the structure. */
  GlobalFree( hTopicData );
}


