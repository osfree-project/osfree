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
**                              TOPIC ELEMENT LIST ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <stdio.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "twhlprc.h"
#include "msgbox.h"
#include "globals.h"
#include "wnddata.h"
#include "elements.h"
#include "portable.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* Initial number of elements allocated for a topic. */
#define INCREMENT_SIZE   100

typedef BYTE __far * LPBYTE;


/***********************************
**
**  Private functions
**
***********************************/

static BOOL __far __pascal AllocElementList( HGLOBAL __far * hElementList );
static BOOL __far __pascal ReAllocElementList( FPELEMENTLIST fpElementList );
static BOOL __far __pascal InsertNewElement( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpData );

static RETVAL __far __pascal ProcessTableData
( 
  HWND hErrorWnd,  
  FPHLPFILEINFO fpHelpFileInfo,
  FPPHRASEINFO fpPhraseInfo,
  FPELEMENTLIST fpElementList,
  HGLOBAL hText,
  DWORD dwTextSize, 
  HGLOBAL hCodes
);
static RETVAL __far __pascal ProcessGeneralData
( 
  HWND hErrorWnd,  
  FPHLPFILEINFO fpHelpFileInfo,
  FPPHRASEINFO fpPhraseInfo,
  FPELEMENTLIST fpElementList,
  HGLOBAL hText,
  DWORD dwTextSize, 
  HGLOBAL hCodes
);

static RETVAL __far __pascal AllocTABLEBEGINRecord
( 
  FPELEMENTLIST fpElementList, 
  FPTOPICELEMENT fpTopicElement, 
  LPBYTE __far * fpTopicCodes, 
  WORD __far * wNumColumns 
);

static RETVAL __far __pascal AllocTextRecord
( 
  FPELEMENTLIST fpElementList, 
  char __far * fpTopicText, 
  DWORD dwTextSize, 
  DWORD __far * dwCurPos,   
  FPPHRASEINFO fpPhraseInfo         
);

static RETVAL __far __pascal AllocParaDefRecords
( 
  HWND hErrorWnd,  
  FPELEMENTLIST fpElementList, 
  LPBYTE __far * fpTopicCodes, 
  FPHLPFILEINFO fpHelpFileInfo, 
  BOOL bTable
);

static RETVAL __far __pascal AllocBorderRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes );

static RETVAL __far __pascal AllocSLRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocLIRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocRIRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocFIRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocSARecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocSBRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocTXRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn );

static RETVAL __far __pascal AllocCodeRecord
( 
  HWND hErrorWnd, 
  FPELEMENTLIST fpElementList, 
  LPBYTE __far * fpTopicCodes, 
  FPHLPFILEINFO fpHelpFileInfo, 
  BOOL __far * bGetPARD 
);

static RETVAL __far __pascal AllocFontRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, FPHLPFILEINFO fpHelpFileInfo );

static RETVAL __far __pascal AllocHotSpotRecord
( 
  FPELEMENTLIST fpElementList, 
  BYTE HotSpotCode,                     /* Hotspot's file code. */
  FPTOPICELEMENT fpTopicElement, 
  LPBYTE __far * fpTopicCodes 
);

static long int __far __pascal FirstTWIPSConvert( BYTE Byte1, BYTE Byte2 );
static long int __far __pascal TabTWIPSConvert( BYTE Byte1, BYTE Byte2, BYTE Byte3 );

static RETVAL __far __pascal AllocPictRecord
( 
  FPELEMENTLIST fpElementList,
  HWND hErrorWnd,
  BYTE AlignType,
  FPTOPICELEMENT fpTopicElement, 
  LPBYTE __far * fpTopicCodes, 
  FPHLPFILEINFO fpHelpFileInfo 
);
static void __far __pascal FreePictRecord( FPTOPICELEMENT fpTopicElement );


static RETVAL __far __pascal AllocBitmapPictRec
(
  FPELEMENTLIST   fpElementList,
  HWND           hErrorWnd,
  BYTE __huge *  fpPictData,
  WORD           wNumHotSpots,
  BYTE           AlignType,
  FPTOPICELEMENT fpTopicElement 
);

static void __far __pascal FreeBitmapPictRec( FPTOPICELEMENT fpTopicElement );

static RETVAL __far __pascal AllocMetaFilePictRec
(
  FPELEMENTLIST  fpElementList,
  HWND           hErrorWnd,
  BYTE __far *   fpPictData,
  WORD           wNumHotSpots,
  BYTE           AlignType,
  FPTOPICELEMENT fpTopicElement 
);

static void __far __pascal FreeMetaFilePictRec( FPTOPICELEMENT fpTopicElement );

static RETVAL __far __pascal AllocSHEDHotSpots
(
  FPELEMENTLIST   fpElementList,
  HWND            hErrorWnd,
  WORD            wNumHotSpots,
  FPSHEDREC __far * fpSHEDRecs,
  BYTE __far *    fpHotSpotData
);

static RETVAL __far __pascal UncompressPict
(
  HWND hErrorWnd,
  BYTE __huge * fpCompressed,             /* Compressed data. */
  DWORD dwCompressSize,                   /* Size in bytes of compressed data. */
  DWORD dwUnCompressSize,                 /* Size in bytes of uncompressed data. 
                                             If 0 then value is unknown. */
  HGLOBAL __far * hOutBuffer,             /* Handle to use for uncompressed data buffer. */
  DWORD __far * dwBufferSize              /* Return value - size of uncompressed data bytes. */
);

static RETVAL __far __pascal UncompressPict2
( 
  HWND hErrorWnd,
  BYTE __huge * CompressedBuffer, 
  DWORD dwCompressedSize, 
  DWORD dwUnCompressedSize,      /* If 0 then dynamically alloc. uncompressed data buffer. */ 
  HGLOBAL __far * hOutBuffer,    /* Handle to use for uncompressed data buffer. */
  DWORD __far * dwOutSize        /* Return value - size of uncompressed data bytes. */
); 
static WORD __far __pascal BytesToRead( BYTE BitMap ); 
static WORD __far __pascal BitSet( BYTE BitMap, WORD Bit ); 


/**************************** TOPIC ELEMENT LIST ROUTINES ********************************/


/***************************************************
*
* Builds a topic element list from the topic data.
*
****************************************************/
RETVAL __far __pascal CreateElementLists
( 
  HWND hErrorWnd,  
  DWORD dwTopicCharOffset,
  HGLOBAL __far * hNonScrollElementsPtr,
  HGLOBAL __far * hScrollElementsPtr,
  long int __far * PrevTopicCharOffsetPtr,
  long int __far * NextTopicCharOffsetPtr,
  HGLOBAL hHelpFileInfo
)
{
  RETVAL RetVal;                 /* Return value of functions called. */
  
  FPELEMENTLIST  fpElementList;      /* Pointer to list info. */

  FPHLPFILEINFO fpHelpFileInfo;   /* Pointer to new help file info.      */

  FPPHRASEINFO fpPhraseInfo;    /* Pointer to the file's phrases table. */

  BOOL bMoreData;
  
  TOPICSTREAMSTRUCT TopicStream;
  
  TOPICDATATYPE TopicDataType;

  HGLOBAL hCodes, hText;
  DWORD dwTextSize; 

  
  /* Lock help file info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Lock file's Phrases info. */
  if( fpHelpFileInfo->hPhrasesInfo != NULL )
  {
    fpPhraseInfo = ( FPPHRASEINFO ) GlobalLock( fpHelpFileInfo->hPhrasesInfo );
  }

  /* Open the topic data stream. */
  RetVal = OpenTopicDataStream( &TopicStream, hErrorWnd, dwTopicCharOffset, fpHelpFileInfo, &bMoreData, PrevTopicCharOffsetPtr, NextTopicCharOffsetPtr );

  /* Error? */
  if( RetVal != NO_ERROR )
  {
    /* Unlock file's Phrases info. */
    if( fpHelpFileInfo->hPhrasesInfo != NULL )
    {
      GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
    }

    /* Unlock help file info. */
    GlobalUnlock( hHelpFileInfo );
    
    /* Close the topic data stream. */
    CloseTopicDataStream( &TopicStream );
    
    return( RetVal );
  }
  
  /* If there is non-scrollable topic data. */
  if( bMoreData == TRUE )
  {
    /* Alloc. basic element list info. */
    if( ! AllocElementList( hNonScrollElementsPtr ) )
    {
      /* Unlock file's Phrases info. */
      if( fpHelpFileInfo->hPhrasesInfo != NULL )
      {
        GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
      }

      /* Unlock help file info. */
      GlobalUnlock( hHelpFileInfo );
      
      /* Close the topic data stream. */
      CloseTopicDataStream( &TopicStream );

      /* Failure. */
      return( ERR_MEMORY );
    }
  
    /* Lock the element list info. */
    fpElementList = ( FPELEMENTLIST ) GlobalLock( *hNonScrollElementsPtr );

    /* While there is non-scrollable topic data to process. */
    while( bMoreData == TRUE )
    {
      /* Read next section of topic's data. */
      RetVal = ReadTopicData( &TopicStream, hErrorWnd, &TopicDataType, &hText, &dwTextSize, &hCodes, &bMoreData );

      /* Error? */
      if( RetVal != NO_ERROR )
      {
        /* Unlock file's Phrases info. */
        if( fpHelpFileInfo->hPhrasesInfo != NULL )
        {
          GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
        }

        /* Unlock help file info. */
        GlobalUnlock( hHelpFileInfo );
        
        /* Close the topic data stream. */
        CloseTopicDataStream( &TopicStream );        
        
        /* Free the topic elements list. */
        DestroyElementList( *hNonScrollElementsPtr );

        return( RetVal );
      }
  
      /* Process the topic link data. */
      switch( TopicDataType )
      {
        /* General topic data. */
        case TL_GENERAL:
        {
          RetVal = ProcessGeneralData( hErrorWnd, fpHelpFileInfo, fpPhraseInfo, 
                                       fpElementList, hText, dwTextSize, hCodes );

          /* Error? */
          if( RetVal != NO_ERROR )
          {
            /* Unlock file's Phrases info. */
            if( fpHelpFileInfo->hPhrasesInfo != NULL )
            {
              GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
            }

            /* Unlock help file info. */
            GlobalUnlock( hHelpFileInfo );
            
            /* Close the topic data stream. */
            CloseTopicDataStream( &TopicStream );        
            
            /* 
            ** If we just had a bad command
            ** act as if nothing wrong happened
            ** and try to show the elements that
            ** we have so far.
            */
            if( RetVal == ERR_BADCODE )
            {
              return( NO_ERROR );
            }
          
            /* Otherwise. */
            else 
            {
              /* Free the topic elements list. */
              DestroyElementList( *hNonScrollElementsPtr );
    
              return( RetVal );
            }
          }

          break;
        }

        /* Table in topic data. */
        case TL_TABLE:
        {
          RetVal = ProcessTableData( hErrorWnd, fpHelpFileInfo, fpPhraseInfo,
                                     fpElementList, hText, dwTextSize, hCodes );

          /* Error? */
          if( RetVal != NO_ERROR )
          {
            /* Unlock file's Phrases info. */
            if( fpHelpFileInfo->hPhrasesInfo != NULL )
            {
              GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
            }

            /* Unlock help file info. */
            GlobalUnlock( hHelpFileInfo );
            
            /* Close the topic data stream. */
            CloseTopicDataStream( &TopicStream );        
            
            /* 
            ** If we just had a bad command
            ** act as if nothing wrong happened
            ** and try to show the elements that
            ** we have so far.
            */
            if( RetVal == ERR_BADCODE )
            {
              return( NO_ERROR );
            }
          
            /* Otherwise. */
            else 
            {
              /* Free the topic elements list. */
              DestroyElementList( *hNonScrollElementsPtr );
    
              return( RetVal );
            }
          }

          break;
        }
      }
    }

    /* Unlock the topic elements list. */
    GlobalUnlock( *hNonScrollElementsPtr );
  }

  /* No non-scrollable topic data. */
  else
  {
    *hNonScrollElementsPtr = NULL;
  }


  /* Alloc. basic element list info. */
  if( ! AllocElementList( hScrollElementsPtr ) )
  {
    /* Unlock file's Phrases info. */
    if( fpHelpFileInfo->hPhrasesInfo != NULL )
    {
      GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
    }

    /* Unlock help file info. */
    GlobalUnlock( hHelpFileInfo );
      
    /* Close the topic data stream. */
    CloseTopicDataStream( &TopicStream );

    /* Free the topic elements list. */
    DestroyElementList( *hNonScrollElementsPtr );

    /* Failure. */
    return( ERR_MEMORY );
  }
  
  /* Lock the element list info. */
  fpElementList = ( FPELEMENTLIST ) GlobalLock( *hScrollElementsPtr );

  /* While there is scrollable topic data to process. */
  bMoreData = TRUE;
  while( bMoreData == TRUE )
  {
    /* Read next section of topic's data. */
    RetVal = ReadTopicData( &TopicStream, hErrorWnd, &TopicDataType, &hText, &dwTextSize, &hCodes, &bMoreData );

    /* Error? */
    if( RetVal != NO_ERROR )
    {
      /* Unlock file's Phrases info. */
      if( fpHelpFileInfo->hPhrasesInfo != NULL )
      {
        GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
      }

      /* Unlock help file info. */
      GlobalUnlock( hHelpFileInfo );
        
      /* Close the topic data stream. */
      CloseTopicDataStream( &TopicStream );        
        
      /* Free the topic elements list. */
      DestroyElementList( *hNonScrollElementsPtr );
      DestroyElementList( *hScrollElementsPtr );

      return( RetVal );
    }
  
    if( bMoreData == TRUE )
    {
      /* Process the topic link data. */
      switch( TopicDataType )
      {
        /* General topic data. */
        case TL_GENERAL:
        {
          RetVal = ProcessGeneralData( hErrorWnd, fpHelpFileInfo, fpPhraseInfo,
                                       fpElementList, hText, dwTextSize, hCodes );
  
          /* Error? */
          if( RetVal != NO_ERROR )
          {
            /* Unlock file's Phrases info. */
            if( fpHelpFileInfo->hPhrasesInfo != NULL )
            {
              GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
            }
  
            /* Unlock help file info. */
            GlobalUnlock( hHelpFileInfo );
              
            /* Close the topic data stream. */
            CloseTopicDataStream( &TopicStream );        
              
            /* 
            ** If we just had a bad command
            ** act as if nothing wrong happened
            ** and try to show the elements that
            ** we have so far.
            */
            if( RetVal == ERR_BADCODE )
            {
              return( NO_ERROR );
            }
          
            /* Otherwise. */
            else 
            {
              /* Free the topic elements list. */
              DestroyElementList( *hNonScrollElementsPtr );
              DestroyElementList( *hScrollElementsPtr );
        
              return( RetVal );
            }
          }
  
          break;
        }
  
        /* Table in topic data. */
        case TL_TABLE:
        {
          RetVal = ProcessTableData( hErrorWnd, fpHelpFileInfo, fpPhraseInfo,
                                     fpElementList, hText, dwTextSize, hCodes );
  
          /* Error? */
          if( RetVal != NO_ERROR )
          {
            /* Unlock file's Phrases info. */
            if( fpHelpFileInfo->hPhrasesInfo != NULL )
            {
              GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
            }
  
            /* Unlock help file info. */
            GlobalUnlock( hHelpFileInfo );
              
            /* Close the topic data stream. */
            CloseTopicDataStream( &TopicStream );        
              
            /* 
            ** If we just had a bad command
            ** act as if nothing wrong happened
            ** and try to show the elements that
            ** we have so far.
            */
            if( RetVal == ERR_BADCODE )
            {
              return( NO_ERROR );
            }
          
            /* Otherwise. */
            else 
            {
              /* Free the topic elements list. */
              DestroyElementList( *hNonScrollElementsPtr );
              DestroyElementList( *hScrollElementsPtr );
      
              return( RetVal );
            }
          }
          
          break;
        }
      }
    }
  }

  /* Unlock the topic elements list. */
  GlobalUnlock( *hScrollElementsPtr );


  /* Unlock file's Phrases info. */
  if( fpHelpFileInfo->hPhrasesInfo != NULL )
  {
    GlobalUnlock( fpHelpFileInfo->hPhrasesInfo );
  }

  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );
    
  /* Close the topic data stream. */
  CloseTopicDataStream( &TopicStream );
      
  /* Success. */
  return( NO_ERROR );
}



/***************************************************
*
* Process a TL_GENERAL topic link record's topic
* data.
*
****************************************************/
static RETVAL __far __pascal ProcessTableData
( 
  HWND hErrorWnd,  
  FPHLPFILEINFO fpHelpFileInfo,
  FPPHRASEINFO fpPhraseInfo,
  FPELEMENTLIST fpElementList,
  HGLOBAL hText,
  DWORD dwTextSize, 
  HGLOBAL hCodes
)
{
  char __far * fpTopicText; 
  BYTE __far * fpTopicCodes;

  DWORD dwCurPos;                /* Our current position in the Topic text buffer. */
  BYTE  CurChar;                 /* Character in the Topic text buffer. */

  TOPICELEMENT TopicElement;     /* New element data to add to the element list. */
  RETVAL RetVal;                 /* No error initially. */
  
  BOOL bGetPARD;                 /* Get paragraph formatting information. */
  BOOL bTableDone;               /* Not at end of table */
  
  WORD wCounter,                 /* Dummy counter var. */
       wNumColumns,              /* Number of columns in the table. */
       wColumnNumber;            /* Number of the next column number in the codes buffer. */
       
  
  /* Lock topic code buffers. */
  fpTopicText = ( char __far * ) GlobalLock( hText ) ;
  fpTopicCodes = ( BYTE __far * ) GlobalLock( hCodes ) ;

  /* Process general table header data. */
  RetVal = AllocTABLEBEGINRecord( fpElementList, &TopicElement, &fpTopicCodes, &wNumColumns );

  /* Error? */
  if( RetVal != NO_ERROR )
  {
    /* Unlock topic code buffers. */
    GlobalUnlock( hText );
    GlobalUnlock( hCodes );

    /* Failure. */
    return( RetVal );
  }

  /* Insert a TABLEBEGIN record into the element list. */
  if( ! InsertNewElement( fpElementList, &TopicElement ) )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }
            
  /* At start of topic text. */
  dwCurPos = 0;
  
  /* Get paragraph formatting information. */
  bGetPARD = TRUE;
  
  /* Not at end of table */
  bTableDone = FALSE;

  /* For each column. */
  for( wCounter = 0; wCounter < wNumColumns && !bTableDone; wCounter++ ) 
  {
    /* Skip over column number. */
    fpTopicCodes = fpTopicCodes + sizeof( WORD );
  
    /* Allocate a TE_TABLECOLUMN record. */
    TopicElement.wRecType = TE_TABLECOLUMN;
    
    /* Insert a TABLEBEGIN record into the element list. */
    if( ! InsertNewElement( fpElementList, &TopicElement ) )
    {
      /* Failure. */
      return( ERR_MEMORY );
    }
              
    /* Go through each character in the text. */
    while( dwCurPos < dwTextSize )
    {
      /* 
      ** Get the integer value of the character at our current position 
      ** in the topic text buffer. 
      */
      CurChar = (BYTE) *( fpTopicText + dwCurPos );
      
      /* If we need to get paragraph formatting information. */
      if( bGetPARD )
      {
        /* Process code. */
        RetVal = AllocParaDefRecords( hErrorWnd, fpElementList, &fpTopicCodes, fpHelpFileInfo, TRUE );
        
        /* Error? */
        if( RetVal != NO_ERROR )
        {
          /* Unlock topic code buffers. */
          GlobalUnlock( hText );
          GlobalUnlock( hCodes );
  
          /* 
          ** If we just had a bad command
          ** act as if nothing wrong happened
          ** and try to show the elements that
          ** we have so far.
          */
          if( RetVal == ERR_BADCODE )
          {
            return( NO_ERROR );
          }
          
          /* Otherwise. */
          else 
          {
            /* Failure. */
            return( RetVal );
          }
        }
  
        /* Finished processing paragraph formatting information. */
        bGetPARD = FALSE;
      }
      
      
      /* 
      ** CODE is value 0. 
      */
      else if( CurChar == 0 ) 
      {
        /* Goto next position in the topic text buffer. */
        dwCurPos++;
  
        /* Process code. */
        RetVal = AllocCodeRecord( hErrorWnd, fpElementList, &fpTopicCodes, fpHelpFileInfo, &bGetPARD );
        
        /* Error? */
        if( RetVal != NO_ERROR )
        {
          /* Unlock topic code buffers. */
          GlobalUnlock( hText );
          GlobalUnlock( hCodes );
  
          /* 
          ** If we just had a bad command
          ** act as if nothing wrong happened
          ** and try to show the elements that
          ** we have so far.
          */
          if( RetVal == ERR_BADCODE )
          {
            return( NO_ERROR );
          }
          
          /* Otherwise. */
          else 
          {
            /* Failure. */
            return( RetVal );
          }
        }
        
        /* 
        ** If we are at the end of the current paragraph, 
        ** check to see if we are starting a new column or
        ** at the end of the table.
        */
        if( bGetPARD == TRUE )
        {
          /* Try to read a column number. */
          wColumnNumber = ReadWORD( fpTopicCodes );
        
          /* At the next column? */
          if( wColumnNumber == wCounter + 1 )
          {
            /* New column. */
            break;
          }
          
          /* End of table? */
          else if( wColumnNumber == 0xFFFF )
          {
            /* Done. */
            bTableDone = TRUE;
          }
        }
      }
  
  
      /* 
      ** PHRASE or TEXT string. 
      */
      else
      {
        /* Allocate a TE_TEXT record. */
        RetVal = AllocTextRecord( fpElementList, fpTopicText, dwTextSize, 
                                  &dwCurPos, fpPhraseInfo );
  
        /* Error? */
        if( RetVal != NO_ERROR )
        {
          /* Unlock topic code buffers. */
          GlobalUnlock( hText );
          GlobalUnlock( hCodes );
        
          /* Failure. */
          return( RetVal );
        }
      }
    }
  }
  
  /* Unlock topic code buffers. */
  GlobalUnlock( hText );
  GlobalUnlock( hCodes );
      
  /* Allocate a TE_TABLEEND record. */
  TopicElement.wRecType = TE_TABLEEND;
    
  /* Insert a TABLEBEGIN record into the element list. */
  if( ! InsertNewElement( fpElementList, &TopicElement ) )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }
              
  /* Success. */
  return( NO_ERROR );
}



/***************************************************
*
* Saves the data for a TABLEBEGIN record.
*
****************************************************/
static RETVAL __far __pascal AllocTABLEBEGINRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, WORD __far * wNumColumns )
{
  FPTABLEREC fpTableRec;
  COLUMNPOSPTR ColumnPosPtr;
  WORD wCounter;
  

  /* Save record type. */
  fpTopicElement->wRecType = TE_TABLEBEGIN;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(TABLEREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpTableRec = (FPTABLEREC) fpTopicElement->RecDataPtr;

/************************************
          KLUDGE ZONE
*************************************/
  /* 
  ** I have no idea what this data
  ** is used for but it doesn't seem
  ** to be important.
  */

  /* Skip the first 2 bytes. */
  *fpTopicCodes = *fpTopicCodes + 2;

  /* Skip one or two depending on value of next byte. */ 
  if( **fpTopicCodes & 0x01 )
  {
    /* Skip next 2 bytes. */
    *fpTopicCodes = *fpTopicCodes + 2;
  }
  else
  {
    /* Skip next 1 bytes. */
    *fpTopicCodes = *fpTopicCodes + 1;
  }
/************************************
          KLUDGE ZONE
*************************************/

  /* Get the next byte of data. */
  fpTableRec->wNumColumns = **fpTopicCodes;
    
  /* Goto next byte. */
  *fpTopicCodes = *fpTopicCodes + 1;
    
  /* Get the next byte of data. */
  fpTableRec->bFitColumns = (BOOL) **fpTopicCodes;
    
  /* Goto next byte. */
  *fpTopicCodes = *fpTopicCodes + 1;
    
  /* Allocate space for the column information. */
  fpTableRec->ColumnPosPtr = (COLUMNPOSPTR) MyAlloc( fpElementList->fpMemoryList, 
                               sizeof(COLUMNPOSREC) * fpTableRec->wNumColumns );
  if( fpTableRec->ColumnPosPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Pointer to first record. */
  ColumnPosPtr = fpTableRec->ColumnPosPtr;
  
  /* Get column position data. */
  for( wCounter = 0; wCounter < fpTableRec->wNumColumns; wCounter++ )
  {
    /* Get the width of column. */
    ColumnPosPtr->Width = ReadWORD( *fpTopicCodes );
    ColumnPosPtr->Width = ColumnPosPtr->Width * 10;
      
    /* Goto next word. */
    *fpTopicCodes = *fpTopicCodes + sizeof(WORD);
  
    /* Get the width of column. */
    ColumnPosPtr->SpaceBetween = ReadWORD( *fpTopicCodes );
    ColumnPosPtr->SpaceBetween = ColumnPosPtr->SpaceBetween * 10;

    /* Goto next word. */
    *fpTopicCodes = *fpTopicCodes + sizeof(WORD);
  
    /* Next column's data. */
    ColumnPosPtr = ColumnPosPtr + 1;
  }
  
  /* Save number of columns. */
  *wNumColumns = fpTableRec->wNumColumns;

  /* Success. */
  return( NO_ERROR );
}



/***************************************************
*
* Process a TL_GENERAL topic link record's topic
* data.
*
****************************************************/
static RETVAL __far __pascal ProcessGeneralData
( 
  HWND hErrorWnd,  
  FPHLPFILEINFO fpHelpFileInfo,
  FPPHRASEINFO fpPhraseInfo,
  FPELEMENTLIST fpElementList,
  HGLOBAL hText,
  DWORD dwTextSize, 
  HGLOBAL hCodes
)
{
  char __far * fpTopicText; 
  BYTE __far * fpTopicCodes;

  DWORD dwCurPos;                /* Our current position in the Topic text buffer. */
  BYTE  CurChar;                 /* Character in the Topic text buffer. */

  RETVAL RetVal;                 /* Return value of functions called. */
  
  BOOL bGetPARD;                 /* Get paragraph formatting information. */
  

  /* At start of topic text. */
  dwCurPos = 0;

  /* Get paragraph formatting information. */
  bGetPARD = TRUE;

  /* Lock topic code buffers. */
  fpTopicText = ( char __far * ) GlobalLock( hText ) ;
  fpTopicCodes = ( BYTE __far * ) GlobalLock( hCodes ) ;

  /* Go through each character in the text. */
  while( dwCurPos < dwTextSize )
  {
    /* 
    ** Get the integer value of the character at our current position 
    ** in the topic text buffer. 
    */
    CurChar = (BYTE) *( fpTopicText + dwCurPos );
    
    /* If we need to get paragraph formatting information. */
    if( bGetPARD )
    {
      /* Process code. */
      RetVal = AllocParaDefRecords( hErrorWnd, fpElementList, &fpTopicCodes, fpHelpFileInfo, FALSE );
      
      /* Error? */
      if( RetVal != NO_ERROR )
      {
        /* Unlock topic code buffers. */
        GlobalUnlock( hText );
        GlobalUnlock( hCodes );

        /* 
        ** If we just had a bad command
        ** act as if nothing wrong happened
        ** and try to show the elements that
        ** we have so far.
        */
        if( RetVal == ERR_BADCODE )
        {
          return( NO_ERROR );
        }
        
        /* Otherwise. */
        else 
        {
          /* Failure. */
          return( RetVal );
        }
      }

      /* Finished processing paragraph formatting information. */
      bGetPARD = FALSE;
    }
    
    
    /* 
    ** CODE is value 0. 
    */
    else if( CurChar == 0 ) 
    {
      /* Goto next position in the topic text buffer. */
      dwCurPos++;

      /* Process code. */
      RetVal = AllocCodeRecord( hErrorWnd, fpElementList, &fpTopicCodes, fpHelpFileInfo, &bGetPARD );
      
      /* Error? */
      if( RetVal != NO_ERROR )
      {
        /* Unlock topic code buffers. */
        GlobalUnlock( hText );
        GlobalUnlock( hCodes );

        /* 
        ** If we just had a bad command
        ** act as if nothing wrong happened
        ** and try to show the elements that
        ** we have so far.
        */
        if( RetVal == ERR_BADCODE )
        {
          return( NO_ERROR );
        }
        
        /* Otherwise. */
        else 
        {
          /* Failure. */
          return( RetVal );
        }
      }
    }


    /* 
    ** PHRASE or TEXT string. 
    */
    else
    {
      /* Allocate a TE_TEXT record. */
      RetVal = AllocTextRecord( fpElementList, fpTopicText, dwTextSize, 
                                &dwCurPos, fpPhraseInfo );

      /* Error? */
      if( RetVal != NO_ERROR )
      {
        /* Unlock topic code buffers. */
        GlobalUnlock( hText );
        GlobalUnlock( hCodes );
      
        /* Failure. */
        return( RetVal );
      }
    }
  }

  /* Unlock topic code buffers. */
  GlobalUnlock( hText );
  GlobalUnlock( hCodes );
      
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Save new page (paragraph) information into the 
* element list.
*
****************************************************/
static RETVAL __far __pascal AllocParaDefRecords
( 
  HWND hErrorWnd,  
  FPELEMENTLIST fpElementList, 
  LPBYTE __far * fpTopicCodes, 
  FPHLPFILEINFO fpHelpFileInfo, 
  BOOL bTable
)
{
  TOPICELEMENT TopicElement;      /* New element data to add to the element list. */
  RETVAL RetVal;                  /* No error initially. */

  BYTE FlagByte1, FlagByte2;              /* Bytes holding flags for default paragraph settings. */
  WORD wFlagByteNum, wFlagBitNum;         /* The Byte and Bit we are processing. */
  BOOL bFlagsDone;                /* Done processing two byte flags? */
                          
                          
/************************************
          KLUDGE ZONE
*************************************/
  
  /* 
  ** I have no idea what this data
  ** is used for but it doesn't seem
  ** to be important.
  */ 

  /* Skip the first 3. */
  *fpTopicCodes = *fpTopicCodes + 3;

  if( bTable )
  {
    if( **fpTopicCodes != 0x80 )
    {
      /* Skip next 4. */
      *fpTopicCodes = *fpTopicCodes + 4;
    }
    else
    {
      /* Skip next 5. */
      *fpTopicCodes = *fpTopicCodes + 5;
    }
  }
  
  else
  {
    if( **fpTopicCodes == 0x00 )
    {
      /* Skip next 4. */
      *fpTopicCodes = *fpTopicCodes + 4;
    }
    else
    {
      /* Skip next 5. */
      *fpTopicCodes = *fpTopicCodes + 5;
    }
  }
  
/************************************
          KLUDGE ZONE
*************************************/


  /* 
  ** Process other default flags stored in next two bytes. 
  */
  
  /* No error. */
  RetVal = NO_ERROR;
 
  /* Get first flag's byte. */
  FlagByte1 = (BYTE) (**fpTopicCodes);

  /* Increment code buffer position. */
  *fpTopicCodes = *fpTopicCodes + 1;

  /* Get second flag's byte. */
  FlagByte2 = (BYTE) (**fpTopicCodes);

  /* Increment code buffer position. */
  *fpTopicCodes = *fpTopicCodes + 1;

  /* Starting with first bit of the first flag's byte. */
  wFlagByteNum = wFlagBitNum = 1;

  /* While not done processing two bytes. */
  bFlagsDone = FALSE;
  while( ! bFlagsDone )
  {
    /* Process byte. */
    switch( wFlagByteNum )
    {
      /* Process first byte. */
      case 1:
      {
        /* Process bit in byte. */
        switch( wFlagBitNum )
        {
          /* Byte 1, Bit 1: Not identified. */
          case 1:
          {
            if( FlagByte1 & 0x01 )
            {
              /* Set error value. */
              RetVal = ERR_BADCODE;
            
              /* Done processing flags. */
              bFlagsDone = TRUE;
            }
            break;
          }
  
          /* Byte 1, Bit 2: \sb - Vertical spacing before a new paragraph. */
          case 2:
          {
            if( FlagByte1 & 0x02 )
            {
              RetVal = AllocSBRecord( fpElementList, &TopicElement, fpTopicCodes, TRUE ); 
              
            }
            else
            {
              RetVal = AllocSBRecord( fpElementList, &TopicElement, fpTopicCodes, FALSE ); 
            }  

            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }
            
            break;
          }
  
          /* Byte 1, Bit 3: \sa - Vertical spacing after a new paragraph. */
          case 3:
          {
            if( FlagByte1 & 0x04 )
            {
              RetVal = AllocSARecord( fpElementList, &TopicElement, fpTopicCodes, TRUE ); 
            }
            else
            {
              RetVal = AllocSARecord( fpElementList, &TopicElement, fpTopicCodes, FALSE ); 
            }            

            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }

            break;
          }
  
          /* Byte 1, Bit 4: \sl - Vertical spacing between lines is a paragraph. */
          case 4:
          {
            if( FlagByte1 & 0x08 )
            {
              RetVal = AllocSLRecord( fpElementList, &TopicElement, fpTopicCodes, TRUE ); 
            }
            else
            {
              RetVal = AllocSLRecord( fpElementList, &TopicElement, fpTopicCodes, FALSE ); 
            }  

            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }

            break;
          }
  
          /* Byte 1, Bit 5: \li - Additional left indent for a paragraph. */
          case 5:
          {
            if( FlagByte1 & 0x10 )
            {
              RetVal = AllocLIRecord( fpElementList, &TopicElement, fpTopicCodes, TRUE ); 
            }
            else
            {
              RetVal = AllocLIRecord( fpElementList, &TopicElement, fpTopicCodes, FALSE ); 
            }
              
            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }

            break;
          }
  
          /* Byte 1, Bit 6: \ri - Additional right indent for a paragraph. */
          case 6:
          {
            if( FlagByte1 & 0x20 )
            {
              RetVal = AllocRIRecord( fpElementList, &TopicElement, fpTopicCodes, TRUE ); 
            }
            else
            {
              RetVal = AllocRIRecord( fpElementList, &TopicElement, fpTopicCodes, FALSE ); 
            }
              
            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }

            break;
          }

  
          /* Byte 1, Bit 7: \fi - First line indent for a paragraph. */
          case 7:
          {
            if( FlagByte1 & 0x40 )
            {
              RetVal = AllocFIRecord( fpElementList, &TopicElement, fpTopicCodes, TRUE );
            }
            else
            {
              RetVal = AllocFIRecord( fpElementList, &TopicElement, fpTopicCodes, FALSE );
            } 
              
            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }

            break;
          }
  
          /* Byte 1, Bit 8: Not identified. */
          case 8:
          {
            if( FlagByte1 & 0x80 )
            {
              /* Set error value. */
              RetVal = ERR_BADCODE;
            
              /* Done processinf flags. */
              bFlagsDone = TRUE;
            }

            /* Done with first byte. */
            wFlagByteNum = 2;
            wFlagBitNum = 1;
            
            continue;
          }
        }
        
        /* Process next bit. */
        wFlagBitNum = wFlagBitNum + 1;
        
        break;
      }
  
   
      /* Process second byte. */
      case 2:
      {
        /* Process bit in byte. */
        switch( wFlagBitNum )
        {
          /* Byte 2, Bit 1: Border drawing info. */
          case 1:
          {
            if( FlagByte2 & 0x01 )
            {
              RetVal = AllocBorderRecord( fpElementList, &TopicElement, fpTopicCodes ); 
              
              /* Error? */
              if( RetVal != NO_ERROR )
              {
                return( RetVal );
              }

              /* Insert element record's new information into the list. */
              if( ! InsertNewElement( fpElementList, &TopicElement ) )
              {
                /* Failure. */
                return( ERR_MEMORY );
              }
            }
            
            break;
          }
  
          /* Byte 2, Bit 2: \tx - Position of a tab stop. */
          case 2:
          {
            if( FlagByte2 & 0x02 )
            {
              RetVal = AllocTXRecord( fpElementList, &TopicElement, fpTopicCodes, TRUE ); 
            }
            else
            {
              RetVal = AllocTXRecord( fpElementList, &TopicElement, fpTopicCodes, FALSE ); 
            }
              
            /* Error? */
            if( RetVal != NO_ERROR )
            {
              return( RetVal );
            }
              
            /* Insert element record's new information into the list. */
            if( ! InsertNewElement( fpElementList, &TopicElement ) )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }
            
            break;
          }
  
          /* Byte 2, Bit 3: \qr - Align text along the right indent. */
          case 3:
          {
            if( FlagByte2 & 0x04 )
            {
              /* Save record type. */
              TopicElement.wRecType = TE_QR;
                
              /* Insert element record's new information into the list. */
              if( ! InsertNewElement( fpElementList, &TopicElement ) )
              {
                /* Failure. */
                return( ERR_MEMORY );
              }
            }
            
            break;
          }
  
          /* Byte 2, Bit 4: \qc - Centers text between the left and right indents. */
          case 4:
          {
            if( FlagByte2 & 0x08 )
            {
              /* Save record type. */
              TopicElement.wRecType = TE_QC;
                
              /* Insert element record's new information into the list. */
              if( ! InsertNewElement( fpElementList, &TopicElement ) )
              {
                /* Failure. */
                return( ERR_MEMORY );
              }
            }
            
            break;
          }
  
          /* Byte 2, Bit 5: \keep - No text wrapping. */
          case 5:
          {
            if( FlagByte2 & 0x10 )
            {
              /* Save record type. */
              TopicElement.wRecType = TE_KEEP;
                
              /* Insert element record's new information into the list. */
              if( ! InsertNewElement( fpElementList, &TopicElement ) )
              {
                /* Failure. */
                return( ERR_MEMORY );
              }
            }
            break;
          }
  
          /* Byte 2, Bit 6: Not identified. */
          case 6:
          {
            if( FlagByte2 & 0x20 )
            {
              /* Set error value. */
              RetVal = ERR_BADCODE;
            
              /* Done processinf flags. */
              bFlagsDone = TRUE;
            }
            break;
          }
  
          /* Byte 2, Bit 7: Not identified. */
          case 7:
          {
            if( FlagByte2 & 0x40 )
            {
              /* Set error value. */
              RetVal = ERR_BADCODE;
            
              /* Done processinf flags. */
              bFlagsDone = TRUE;
            }
            break;
          }
  
          /* Byte 2, Bit 8: Not identified. */
          case 8:
          {
            if( FlagByte2 & 0x80 )
            {
              /* Set error value. */
              RetVal = ERR_BADCODE;
            }

            /* Done processinf flags. */
            bFlagsDone = TRUE;
            
            break;
          }
        }

        /* Process next bit. */
        wFlagBitNum = wFlagBitNum + 1;
        
        break;
      }
    }
  }

  if( RetVal == ERR_BADCODE )
  {
    /* If we get down to here then the command is not recognized. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFORMATCODE, MB_ICONHAND | MB_OK );
  }

  /* Success/Failure. */
  return( RetVal );
}


/***************************************************
*
* Reads border line data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocBorderRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes )
{
  FPBORDERREC fpBorderRec;
  BYTE    Byte1;
    

  /* Save record type. */
  fpTopicElement->wRecType = TE_BORDER;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(BORDERREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpBorderRec = (FPBORDERREC) fpTopicElement->RecDataPtr;

  /* Get the first byte of data. */
  Byte1 = **fpTopicCodes;
    
  /* Skip the next two bytes of data. */
  *fpTopicCodes = *fpTopicCodes + 3;
    
  /* Box setting - All broder sides drawn. */
  if( Byte1 & 0x01 )
  {
    fpBorderRec->bTop = TRUE;
    fpBorderRec->bLeft = TRUE;
    fpBorderRec->bBottom = TRUE;
    fpBorderRec->bRight = TRUE;
  }
    
  /* Only some line borders drawn. */
  else
  {
    /* Top line setting. */
    if( Byte1 & 0x02 )
    {
      fpBorderRec->bTop = TRUE;
    }
    else
    {
      fpBorderRec->bTop = FALSE;
    }
    
    /* Left line setting. */
    if( Byte1 & 0x04 )
    {
      fpBorderRec->bLeft = TRUE;
    }
    else
    {
      fpBorderRec->bLeft = FALSE;
    }
    
    /* Bottom line setting. */
    if( Byte1 & 0x08 )
    {
      fpBorderRec->bBottom = TRUE;
    }
    else
    {
      fpBorderRec->bBottom = FALSE;
    }
    
    /* Right line setting. */
    if( Byte1 & 0x10 )
    {
      fpBorderRec->bRight = TRUE;
    }
    else
    {
      fpBorderRec->bRight = FALSE;
    }
  }
  
  
  /* Thick line setting. */
  if( Byte1 & 0x20 )
  {
    fpBorderRec->bThickLine = TRUE;
  }
  else
  {
    fpBorderRec->bThickLine = FALSE;
  }
  
  /* Thick line setting. */
  if( Byte1 & 0x40 )
  {
    fpBorderRec->LineType = DOUBLE_LINE;
  }
#if 0
  else if( Byte1 & 0x60 )
  {
    fpBorderRec->LineType = SHADOW_LINE;
  }
#endif
  else if( Byte1 & 0x80 )
  {
    fpBorderRec->LineType = DOTTED_LINE;
  }
  else
  {
    fpBorderRec->LineType = SINGLE_LINE;
  }

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads \sl .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocSLRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPSLREC fpSLRec;
  BYTE    Byte1, Byte2;

  /* Save record type. */
  fpTopicElement->wRecType = TE_SL;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(SLREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpSLRec = (FPSLREC) fpTopicElement->RecDataPtr;

  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get the first byte of data. */
    Byte1 = **fpTopicCodes;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Is there another byte of data? */
    Byte2 = 0;
    if( Byte1 & 0x01 )
    {
      /* Get the next byte of data. */
      Byte2 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    }
    
    /* Convert value to a TWIPS value. */
    fpSLRec->VertLineSpace = FirstTWIPSConvert( Byte1, Byte2 );
  }
  
  else
  {
    /* Set to the default value. */
    fpSLRec->VertLineSpace = 0;
  }
  

  /* Success. */
  return( NO_ERROR );
}



/***************************************************
*
* Reads \li .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocLIRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPLIREC fpLIRec;
  BYTE    Byte1, Byte2;


  /* Save record type. */
  fpTopicElement->wRecType = TE_LI;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(LIREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpLIRec = (FPLIREC) fpTopicElement->RecDataPtr;


  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get the first byte of data. */
    Byte1 = **fpTopicCodes;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Is there another byte of data? */
    Byte2 = 0;
    if( Byte1 & 0x01 )
    {
      /* Get the next byte of data. */
      Byte2 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    }
    
    /* Convert to a TWIPS value. */
    fpLIRec->LeftIndent = FirstTWIPSConvert( Byte1, Byte2 );
  }
  
  else
  {
    /* Set to the default value. */
    fpLIRec->LeftIndent = 0;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads \ri .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocRIRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPRIREC fpRIRec;
  BYTE    Byte1, Byte2;


  /* Save record type. */
  fpTopicElement->wRecType = TE_RI;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(RIREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpRIRec = (FPRIREC) fpTopicElement->RecDataPtr;

  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get the first byte of data. */
    Byte1 = **fpTopicCodes;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Is there another byte of data? */
    Byte2 = 0;
    if( Byte1 & 0x01 )
    {
      /* Get the next byte of data. */
      Byte2 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    }
    
    /* Convert to a TWIPS value. */
    fpRIRec->RightIndent = FirstTWIPSConvert( Byte1, Byte2 );
  }
  
  else
  {
    /* Set to the default value. */
    fpRIRec->RightIndent = 0;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads \sa .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocSARecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPSAREC fpSARec;
  BYTE    Byte1, Byte2;


  /* Save record type. */
  fpTopicElement->wRecType = TE_SA;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(SAREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpSARec = (FPSAREC) fpTopicElement->RecDataPtr;

  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get the first byte of data. */
    Byte1 = **fpTopicCodes;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Is there another byte of data? */
    Byte2 = 0;
    if( Byte1 & 0x01 )
    {
      /* Get the next byte of data. */
      Byte2 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    }
    
    /* Convert to a TWIPS value. */
    fpSARec->SpaceAfterParagraph = FirstTWIPSConvert( Byte1, Byte2 );
  }
  
  else
  {
    /* Set to the default value. */
    fpSARec->SpaceAfterParagraph = 0;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads \sb .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocSBRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPSBREC fpSBRec;
  BYTE    Byte1, Byte2;


  /* Save record type. */
  fpTopicElement->wRecType = TE_SB;

  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(SBREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpSBRec = (FPSBREC) fpTopicElement->RecDataPtr;

  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get the first byte of data. */
    Byte1 = **fpTopicCodes;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Is there another byte of data? */
    Byte2 = 0;
    if( Byte1 & 0x01 )
    {
      /* Get the next byte of data. */
      Byte2 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    }
    
    /* Convert to a TWIPS value. */
    fpSBRec->SpaceBeforeParagraph = FirstTWIPSConvert( Byte1, Byte2 );
  }
  
  else
  {
    /* Set to the default value. */
    fpSBRec->SpaceBeforeParagraph = 0;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads \tx .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocTXRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPTXREC fpTXRec;
  BYTE    Byte1, Byte2, Byte3;
  WORD    wCounter;
  TABSTOPPTR TabStopPtr;
    

  /* Save record type. */
  fpTopicElement->wRecType = TE_TX;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(TXREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpTXRec = (FPTXREC) fpTopicElement->RecDataPtr;

  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get number of tabs - 0x82 means 1 tabs, 0x84 means 2 tabs... */
    fpTXRec->wTabCount = ( **fpTopicCodes - 0x80 ) / 2;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Allocate space for tab. values. */
    fpTXRec->TabStopsPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(long int) * fpTXRec->wTabCount );
    if( fpTXRec->TabStopsPtr == NULL )
    {
      /* Failure. */
      return( ERR_MEMORY );
    }
  
    /* Get first tab stop. */
    TabStopPtr = fpTXRec->TabStopsPtr;
    
    /* Get the file's value for each tab. */
    for( wCounter = 0; wCounter < fpTXRec->wTabCount; wCounter++ )
    {
      /* Get the first byte of data. */
      Byte1 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    
      /* Init. */
      Byte2 = 0;
      Byte3 = 0;

      /* Is there another byte of data? Yes if Byte1 is an odd value. */
      if( Byte1 & 0x01 )
      {
        /* Get the next byte of data. */
        Byte2 = **fpTopicCodes;
      
        /* Goto next byte. */
        *fpTopicCodes = *fpTopicCodes + 1;
      
        /* If 0x80, then there is another byte of data. */
        if( Byte2 & 0x80 )
        {
          /* Get the next byte of data. */
          Byte3 = **fpTopicCodes;
        
          /* Goto next byte. */
          *fpTopicCodes = *fpTopicCodes + 1;
        }
      }
      
      /* Convert file's value to a TWIPS tab value. */
      *TabStopPtr = TabTWIPSConvert( Byte1, Byte2, Byte3 );
  
      /* Move to next tab value. */
      TabStopPtr++;
    }
  }
  
  else
  {
    /* No tab info. */
    fpTXRec->TabStopsPtr = NULL;
    
    /* Set to the default value. */
    fpTXRec->wTabCount = 0;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads \fi .RTF code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocFIRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, BOOL bSettingOn )
{
  FPFIREC fpFIRec;
  BYTE    Byte1, Byte2;


  /* Save record type. */
  fpTopicElement->wRecType = TE_FI;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(FIREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpFIRec = (FPFIREC) fpTopicElement->RecDataPtr;

  /* If we have information for this setting. */
  if( bSettingOn )
  {
    /* Get the first byte of data. */
    Byte1 = **fpTopicCodes;
  
    /* Goto next byte. */
    *fpTopicCodes = *fpTopicCodes + 1;
  
    /* Is there another byte of data? */
    Byte2 = 0;
    if( Byte1 & 0x01 )
    {
      /* Get the next byte of data. */
      Byte2 = **fpTopicCodes;
    
      /* Goto next byte. */
      *fpTopicCodes = *fpTopicCodes + 1;
    }
    
    /* Convert to a TWIPS value. */
    fpFIRec->FirstLineIndent = FirstTWIPSConvert( Byte1, Byte2 );
  }
  
  else
  {
    /* Set to the default value. */
    fpFIRec->FirstLineIndent = 0;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Converts value into a useable TWIPS display
* value.
*
****************************************************/
static long int __far __pascal FirstTWIPSConvert( BYTE Byte1, BYTE Byte2 )
{
  long int TWIPValue;
  BOOL bIsNegative = FALSE;

  /* Initialize. */
  TWIPValue = 0;
  
  /* Using both bytes of data - TWIPValue will be > (+/-)640. */
  if( Byte1 & 0x01 )
  {
    /* Should be a negative TWIP value. */
    if( !(Byte2 & 0x80) )
    {
      short int Both;
      
      /* Should be a negative TWIP value. */
      bIsNegative = TRUE;
      
      /* Make a combined value. */
      Both = Byte1;
      Both |= Byte2 << 8;

      /* Take inverse. */
      Both -= 2;
      Both = -Both + 2;
    
      /* Break values apart. */
      Byte1 = Both & 0x00FF;
      Byte2 = (Both >> 8) & 0x00FF;
    }

    /* Calculate TWIPS value. */
    TWIPValue = ( (Byte1 - 1) / 2 ) * 10;
    
    /* 
    ** long int cast used because when code is optimized I got
    ** an overflow error.
    */
    TWIPValue = TWIPValue + ((long int)(Byte2 - 128 ) * 1280);
  }
 

  /* Using one byte of data - TWIPValue will be < (+/-)640. */
  else
  {
    /* Should be a negative TWIP value. */
    if( !(Byte1 & 0x80) )
    {
      short int Both;
      
      /* Should be a negative TWIP value. */
      bIsNegative = TRUE;
      
      /* Make a combined value. */
      Both = Byte1;
      Both |= 0 << 8;

      /* Take inverse. */
      Both = -Both + 2;
    
      /* Break values apart. */
      Byte1 = Both & 0x00FF;
    }

    /* Calculate TWIPS value. */
    TWIPValue = ( (Byte1 - 128) / 2 ) * 10;
  
  }

  /* If it should be a negative TWIP value. */
  if( bIsNegative )
  {
  /* Make TWIP value negative. */
    TWIPValue = -TWIPValue;
  }

  /* Return TWIPS value. */
  return( TWIPValue );
}                 


/***************************************************
*
* Converts value into a useable TWIPS display
* tab value.
*
****************************************************/
static long int __far __pascal TabTWIPSConvert( BYTE Byte1, BYTE Byte2, BYTE Byte3 )
{
  short int TabTWIPSValue = 0;

  /* Using both bytes of data - value will be (x > 640 or x <-640). */
  if( Byte1 & 0x01 )
  {
    /* Calculate TWIPS value. */
    TabTWIPSValue = ( (Byte1 - 1) / 2 ) * 10;

    /* If 0x80 for byte2. */
    if( Byte2 & 0x80 )
    {
      /*
      **  I found a tab entry 0x86 0x00 0x49 0x80 0x02 0x5C.
      **  There should be 3 entries then.
      **
      **  The last byte value - 0x5C - is a valid tab value.
      **
      **  That left me with four bytes (0x00 0x49 0x80 0x02)
      **  that define 2 more tab values.
      **
      **  In alot of places there was a 0x86 0x00 entry to
      **  (I guess) specify one tab at the 0 x-coord. position.
      **  I assumed then that the first 0x00 byte was a real
      **  tab value.
      **
      **  That left me with three bytes (0x49 0x80 0x02)
      **  to define one more tab value. Thus, I embarked on the
      **  addition of the 3rd byte value.  I think the 0x80 signifies
      **  that another byte follows.  I guessed that adding the 2nd and 3rd
      **  byte together and subtracting from the TabTWIPSValue.
      **  The value I was shooting for was 230. 
      **  The 0x49 processed becomes the TabTWIPSValue 360.
      **  0x80 + 0x02 = 130.
      **  360 - 130 = 230.
      **
      **  Just a bunch of guesses.
      **
      */
      TabTWIPSValue = TabTWIPSValue - ( Byte2 + Byte3 );
    }
    else
    {
      TabTWIPSValue = TabTWIPSValue + (Byte2 * 1280);
    }
  }
 

  /* Using one byte of data - value will be (-640 < x < 640). */
  else
  {
    /* Calculate TWIPS value. */
    TabTWIPSValue = ( Byte1 / 2 ) * 10;
  }

  /* Return TWIPS value. */
  return( TabTWIPSValue );
}                 



/***************************************************
*
* Save text information into the TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocTextRecord
( 
  FPELEMENTLIST fpElementList,
  char __far * fpTopicText, 
  DWORD dwTextSize, 
  DWORD __far * dwCurPos,   
  FPPHRASEINFO fpPhraseInfo   /* Pointer to the file's phrases table. */
)
{
  #define MORE_SPACE  300

  TOPICELEMENT TopicElement;   /* TOPICELEMENT record for a new TE_TEXT record. */
  FPTEXTREC fpTextRec;         /* Pointer to a TE_TEXT record. */

  DWORD dwBeginPos;            /* Beginning offset position in the Topic text buffer. */
  BYTE  CurChar;               /* Character in the Topic text buffer. */
  BYTE Byte1, Byte2;           /* Byte values in the Topic text buffer. */
                        
  WORD  wPhraseIndex;          /* Phrase index number. */
  BOOL  bSpaceAfter;           /* Is there a space after the phrase. */
  char __far * fpPhrase;       /* Pointer to the phrase in the Phrases info. */
  WORD wPhraseSize;            /* Size of the phrase. */

  char __far * fpStringSave;   /* Buffer in the TE_TEXT record for the string. */
  WORD wStringSavePos;         /* Current position in TE_TEXT record's buffer. */
  WORD wStringSaveSize;        /* Size of TE_TEXT record's buffer. */
  
  WORD wStringSize;            /* Size of a string in the topic text buffer. */
  BOOL bText;

  HGLOBAL hText;               /* Temporary space for text. */
  
  
  /* The topic element is a text record type. */
  TopicElement.wRecType = TE_TEXT;
          
  /* Allocate record's data. */
  TopicElement.RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(TEXTREC) );
  if( TopicElement.RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpTextRec = (FPTEXTREC) TopicElement.RecDataPtr;

  /* Allocate initial space for the text string. */
  hText = GlobalAlloc( GHND, sizeof(char) * MORE_SPACE );
  if( hText == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }
  
  /* Lock the handle to the string saving space. */
  fpStringSave = ( char __far * ) GlobalLock( hText );

  /* Size and intial position. */
  wStringSaveSize = MORE_SPACE;
  wStringSavePos = 0;
  
  /* Get the first byte in the topic text buffer. */
  CurChar = (BYTE) *( fpTopicText + *dwCurPos );

  /* While next byte is not a CODE and we are not at the end. */
  while( CurChar != 0 && *dwCurPos < dwTextSize )
  {
    /* Text. */
    if( CurChar > 31 ) 
    {
      /* Copying text. */
      bText = TRUE;
      
      /* First position in beginning position of string. */
      dwBeginPos = *dwCurPos;

      /* We have one character in the string so far. */
      wStringSize = 1;
          
      /* Goto next position in the topic text buffer. */
      *dwCurPos = *dwCurPos + 1;

      /* Go through each character in the text buffer. */
      while( *dwCurPos < dwTextSize )
      {
        /* 
        ** Get the integer value of the character at our current position 
        ** in the topic text buffer. 
        */
        CurChar = (BYTE) *( fpTopicText + *dwCurPos );
    
        /* Is it another character? */
        if( CurChar > 31 ) 
        {
          /* Goto next position in the topic text buffer. */
          *dwCurPos = *dwCurPos + 1;
    
          /* Increment the string length. */
          wStringSize++;
        }
          
        /* Past end of string. */
        else 
        {  
          /* Stop looking at characters. */
          break;
        }
      }                  
    }


    /* Phrase. */
    else
    {
      /* Copying phrase. */
      bText = FALSE;

      /* Get first byte. */
      Byte1 = CurChar;

      /* Goto next position in the topic text buffer. */
      *dwCurPos = *dwCurPos + 1;
      
      /* Get second byte. */
      Byte2 = (BYTE) *( fpTopicText + *dwCurPos );
      
      /* Goto next position in the topic text buffer. */
      *dwCurPos = *dwCurPos + 1;

      /* Calculate phrase index number (almost). */
      wPhraseIndex = ( ( 256 * (Byte1 - 1) ) + Byte2 );
          
      /* Is there a space after the phrase. */
      if( wPhraseIndex % 2 ) 
      {
        bSpaceAfter = TRUE;
      }
      else bSpaceAfter = FALSE;
          
      /* Divide Phrase number by 2 to get the REAL phrase index number. */
      wPhraseIndex = wPhraseIndex / 2;
          
      /* Get the size of the phrase and a pointer to it. */
      fpPhrase = GetPhrasePtr( fpPhraseInfo, wPhraseIndex, &wPhraseSize );

      /*
      ** Allocate space for the string. 
      ** 1 additional character added for ending NULL char.
      */
      if( bSpaceAfter )
      {
        /* Save the size of the string including the additional space character. */
        wStringSize =  wPhraseSize + 1;
      }
      else
      {
        /* Save the size of the string including the additional space character. */
        wStringSize =  wPhraseSize;
      }
    }
    
    /* Need more space? */
    while( wStringSavePos + wStringSize > wStringSaveSize )
    {
      /* Unlock the handle to the string saving space. */
      GlobalUnlock( hText );

      /* Increase size of buffer. */
      wStringSaveSize += MORE_SPACE;

      /* Allocate bigger buffer. */
      hText = GlobalReAlloc( hText, wStringSaveSize, LMEM_MOVEABLE | GMEM_ZEROINIT );
      
      if( hText == NULL )
      {
        /* Failure. */
        return( ERR_MEMORY );
      }
  
      /* Lock the handle to the string saving space. */
      fpStringSave = ( char __far * ) GlobalLock( hText );
    }
      
    /* Copying text. */
    if( bText )
    {
      /* Copy text to the save buffer. */
      _fmemcpy( fpStringSave + wStringSavePos, fpTopicText + dwBeginPos, wStringSize );
    }
    
    /* Copying phrase. */
    else
    {
      /* Phrase should have a space after it. */
      _fmemcpy( fpStringSave + wStringSavePos, fpPhrase, wPhraseSize );
          
      /* Add space at end if if should be there. */
      if( bSpaceAfter )
      {
        *( fpStringSave + wStringSavePos + wPhraseSize ) = ' ';
      }
    }
    
    /* Increment our save buffer position. */
    wStringSavePos += wStringSize;

    /* 
    ** Get the integer value of the character at our current position 
    ** in the topic text buffer. 
    */
    CurChar = (BYTE) *( fpTopicText + *dwCurPos );
  }

  /* Add NULL. */
  *(fpStringSave + wStringSavePos) = '\0';

  /* Allocate permanent location for string. */
  fpTextRec->TextPtr = (char __far * ) MyAlloc( fpElementList->fpMemoryList, wStringSavePos + 1 );
  if( fpTextRec->TextPtr == NULL )
  {
    /* Free temporary string. */
    GlobalUnlock( hText );
    GlobalFree( hText );

    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Copy temp. string. */
  _fstrcpy( fpTextRec->TextPtr, fpStringSave );
  
  /* Free temporary string. */
  GlobalUnlock( hText );
  GlobalFree( hText );
                                                                                    
  /* 
  ** Insert element record's new information into the list. 
  */
  if( ! InsertNewElement( fpElementList, &TopicElement ) )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Save code information into the TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocCodeRecord
( 
  HWND hErrorWnd, 
  FPELEMENTLIST fpElementList, 
  LPBYTE __far * fpTopicCodes, 
  FPHLPFILEINFO fpHelpFileInfo, 
  BOOL __far * bGetPARD 
)
{
  TOPICELEMENT TopicElement;      /* Data for a new record in the element list. */
  BYTE   CodeType;                /* Type of code to process. */
  BOOL   bKnowCode;               /* Code is recognized. */
  RETVAL RetVal = NO_ERROR;       /* No error initially. */
  
  
  /* Get code type. */
  CodeType = **fpTopicCodes;
  
  /* Increment code buffer position. */
  *fpTopicCodes = *fpTopicCodes + 1;
  
  /* Process code type. */
  switch( CodeType )
  {
    /* Picture element. */
    case TC_RIGHTPICT:
    case TC_LEFTPICT:
    case TC_CHARPICT:
    {
      bKnowCode = TRUE;
      RetVal = AllocPictRecord( fpElementList, hErrorWnd, CodeType, &TopicElement, fpTopicCodes, fpHelpFileInfo );
      break;
    }
    
    /* Font element. */
    case TC_FONT:
    {
      bKnowCode = TRUE;
      RetVal = AllocFontRecord( fpElementList, &TopicElement, fpTopicCodes, fpHelpFileInfo );
      break;
    }
    
    /* End of paragraph. */
    case TC_PAR:
    {
      bKnowCode = TRUE;
      TopicElement.wRecType = TE_PAR;
      break;
    }
  
    /* Tab element. */
    case TC_TAB:
    {
      bKnowCode = TRUE;
      TopicElement.wRecType = TE_TAB;
      break;
    }
                                                        
    /* Hotspot beginning. */
    case TC_HOTBEG_2:
    case TC_HOTBEG_3:
    case TC_HOTBEG_6:
    case TC_HOTBEG_7:
    case TC_HOTBEG_A:
    case TC_HOTBEG_B:
    case TC_HOTBEG_C8:
    {
      RetVal = AllocHotSpotRecord( fpElementList, CodeType, &TopicElement, fpTopicCodes );
      if( RetVal == ERR_BADCODE )
      {
        bKnowCode = FALSE;
      }
      else
      {
        bKnowCode = TRUE;
      }
      break;
    }

    /* Hotspot ending element. */
    case TC_HOTEND:
    {
      bKnowCode = TRUE;
      TopicElement.wRecType = TE_HOTEND;
      break;
    }
  
    /* Need to process paragraph formatting information. */
    case TC_PARD:
    {
      bKnowCode = TRUE;
      TopicElement.wRecType = TE_PARD;
      *bGetPARD = TRUE;
      break;
    }

    /* New line, but not a new paragraph. */
    case TC_LINE:
    {
      bKnowCode = TRUE;
      TopicElement.wRecType = TE_LINE;
      break;
    }

    default:
    {
      bKnowCode = FALSE;
    }
  }

  /* Unknown code. */
  if( bKnowCode == FALSE )
  {
    /* If we get down to here then the command is not recognized. */ 
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFORMATCODE, MB_ICONHAND | MB_OK );

    /* Set error value. */
    RetVal = ERR_BADCODE;
  }

  /* If we have a record to save. */
  if( bKnowCode && RetVal == NO_ERROR )
  {
    /* Insert the record into the element list. */
    if( ! InsertNewElement( fpElementList, &TopicElement ) )
    {
      /* Free picture record type since it is GlobalAlloc'd. */
      switch( CodeType )
      {
        /* Picture element. */
        case TC_RIGHTPICT:
        case TC_LEFTPICT:
        case TC_CHARPICT:
        {
          FreePictRecord( &TopicElement );
          break;
        }
      }

      /* Failure. */
      return( ERR_MEMORY );
    }
  }

  /* Return success/failure. */
  return( RetVal );
}


/***************************************************
*
* Free allocated record data for a picture record.
*
****************************************************/
static void __far __pascal FreePictRecord( FPTOPICELEMENT fpTopicElement )
{
  switch( fpTopicElement->wRecType )
  {
    case TE_CHARBITMAP:
    case TE_LEFTBITMAP:
    case TE_RIGHTBITMAP:
    {
      FreeBitmapPictRec( fpTopicElement );
      break;
    }

    case TE_CHARMETA:
    case TE_LEFTMETA:
    case TE_RIGHTMETA:
    {
      FreeMetaFilePictRec( fpTopicElement );
      break;
    }
  }    
}



/***************************************************
*
* Determines if a bitmap or metafile's data is 
* in the TopicCodes buffer and calls the correct
* allocation function.
*
****************************************************/
static RETVAL __far __pascal AllocPictRecord
( 
  FPELEMENTLIST fpElementList,
  HWND hErrorWnd,
  BYTE AlignType,
  FPTOPICELEMENT fpTopicElement, 
  LPBYTE __far * fpTopicCodes, 
  FPHLPFILEINFO fpHelpFileInfo 
)
{
  RETVAL RetVal;

  WORD wNumHotSpots;
  WORD wPictFileNum;
  
  HGLOBAL hPictData;
  BYTE __huge * fpPictData;
  
  
  /* Move to position of the hotspot number. */
  *fpTopicCodes = *fpTopicCodes + 3;  

  /* 
  ** Get the number of hotspots 
  ** from the codes buffer and adjust
  ** value. 
  */
  wNumHotSpots = ReadWORD( *fpTopicCodes );
  *fpTopicCodes = *fpTopicCodes + sizeof(WORD);
  wNumHotSpots = ( wNumHotSpots / 2 ) - 1;
  
  /* Move to position of the picture file number. */
  *fpTopicCodes = *fpTopicCodes + 1;  

  /* 
  ** Get the number of the picture 
  ** file from the codes buffer. 
  */
  wPictFileNum = ReadWORD( *fpTopicCodes );
  *fpTopicCodes = *fpTopicCodes + sizeof(WORD);
  
  /* Load picture file into memory from .HLP file. */
  RetVal = LoadPictFile( hErrorWnd, fpHelpFileInfo,  &hPictData, wPictFileNum );
  if( RetVal != NO_ERROR )
  {
    return( RetVal ); 
  }
 
  /* Lock picture data. */
  fpPictData = ( BYTE __huge * ) GlobalLock( hPictData );
  
  /* Determine the type of picture that it is. */
  switch( *(fpPictData + 8) )
  {
    /* Bitmap picture. */
    case 0x06:
    {
      RetVal = AllocBitmapPictRec( fpElementList, hErrorWnd, fpPictData, wNumHotSpots, AlignType, fpTopicElement );
      break;
    }
  
    /* Metafile picture. */
    case 0x08:
    {
      RetVal = AllocMetaFilePictRec( fpElementList, hErrorWnd, fpPictData, wNumHotSpots, AlignType, fpTopicElement );
      break;
    }
  }

  /* Unlock picture's raw data. */
  GlobalUnlock( hPictData );
  
  /* Free picture's raw data. */
  GlobalFree( hPictData );
  
  /* Return result. */
  return( RetVal );
}  
  

/***************************************************
*
* Saves the picture's SHED hotspot information in
* a buffer.
*
****************************************************/
static RETVAL __far __pascal AllocSHEDHotSpots
(
  FPELEMENTLIST   fpElementList,
  HWND            hErrorWnd,
  WORD            wNumHotSpots,
  FPSHEDREC __far * fpSHEDRecs,
  BYTE __far *    fpHotSpotData
)
{
  BYTE  HotSpotCode;
  WORD  wInvisible;
  
  WORD  wCounter;
  WORD  wBytes;
  FPSHEDREC fpCurrShedRec;
  

  /* BYTE 0 - Verify version. */
  if( *fpHotSpotData != 0x01 ) return( ERR_OTHER );
  fpHotSpotData = fpHotSpotData + sizeof( BYTE );
  
  /* Allocate record's data. */
  *fpSHEDRecs = ( FPSHEDREC ) MyAlloc( fpElementList->fpMemoryList, sizeof(SHEDREC) * wNumHotSpots );
  if( *fpSHEDRecs == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Get pointer to shed record space. */
  fpCurrShedRec = *fpSHEDRecs;


  /* BYTE 1 - Skip next WORD - the number of hotspots. */
  fpHotSpotData = fpHotSpotData + sizeof( WORD );

  /* BYTE 3 - Skip next DWORD - Size used by all of the macros' strings (include ending NULLS). */
  fpHotSpotData = fpHotSpotData + sizeof( DWORD );

  /* For each hotspot. */
  for( wCounter = 0; wCounter < wNumHotSpots; wCounter++ )
  {
    /* Read hotspot's code type BYTE. */
    HotSpotCode = *fpHotSpotData;
    fpHotSpotData = fpHotSpotData + sizeof( BYTE );

    /* Verify code is known and save info. */
    switch( HotSpotCode )
    {
      /* Hotspot code recognized. */
      case 0xE2:  /* Popup, Invisible. */   
      case 0xE6:  /* Popup, Visible. */
      {
        fpCurrShedRec->HotSpotRec.HotSpotType = HOTSPOT_POPUP;
        break;
      }

      case 0xE3:  /* Jump, Invisible. */ 
      case 0xE7:  /* Jump, Visible. */
      {
        fpCurrShedRec->HotSpotRec.HotSpotType = HOTSPOT_JUMP;
        break;
      }

      case 0xCC:  /* Macro, Invisible. */ 
      case 0xC8:  /* Macro, Visible. */
      {
        fpCurrShedRec->HotSpotRec.HotSpotType = HOTSPOT_MACRO;
        break;
      }
      
      /* Not recognized. */
      default:
      {
        /* If we get down to here then the command is not recognized. */ 
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADSHED, MB_ICONHAND | MB_OK );

        /* Failure. */
        return( ERR_BADCODE );
      }
    }
    
    /* Read invisible/visible WORD. */
    wInvisible = ReadWORD( fpHotSpotData );  
    fpHotSpotData = fpHotSpotData + sizeof( WORD );

    /* Verify code is known and save info. */
    switch( wInvisible )
    {
      /* Hotspot rectangle visible. */
      case 0x00:
      {
        fpCurrShedRec->HotSpotRec.bVisible = TRUE;
        break;
      }

      /* Hotspot rectangle invisible. */
      case 0x04:
      {
        fpCurrShedRec->HotSpotRec.bVisible = FALSE;
        break;
      }
    }    
    
    /* Read left position WORD. */
    fpCurrShedRec->PosInBitmap.left = (WORD) ReadWORD( fpHotSpotData );  
    fpHotSpotData = fpHotSpotData + sizeof( WORD );
    
    /* Read top position WORD. */
    fpCurrShedRec->PosInBitmap.top = (WORD) ReadWORD( fpHotSpotData );  
    fpHotSpotData = fpHotSpotData + sizeof( WORD );
    
    /* Read width WORD. */
    fpCurrShedRec->PosInBitmap.right = (WORD) ReadWORD( fpHotSpotData );  
    fpHotSpotData = fpHotSpotData + sizeof( WORD );

    /* Covert width to right position. */
    fpCurrShedRec->PosInBitmap.right = fpCurrShedRec->PosInBitmap.left + 
                                       fpCurrShedRec->PosInBitmap.right;    

    /* Read height WORD. */
    fpCurrShedRec->PosInBitmap.bottom = (WORD) ReadWORD( fpHotSpotData );  
    fpHotSpotData = fpHotSpotData + sizeof( WORD );

    /* Covert height to bottom position. */
    fpCurrShedRec->PosInBitmap.bottom = fpCurrShedRec->PosInBitmap.top + 
                                        fpCurrShedRec->PosInBitmap.bottom;

    /* Read hash value. */
    fpCurrShedRec->HotSpotRec.dwHotSpotContext = ReadDWORD( fpHotSpotData );
    fpHotSpotData = fpHotSpotData + sizeof( DWORD );
          
    /* No color attributes for a SHED hotspot. */
    fpCurrShedRec->HotSpotRec.bColorAndUnder = FALSE;

    /* No different help file for a SHED hotspot. */
    fpCurrShedRec->HotSpotRec.HelpFilePathPtr = NULL;
    
    /* For now, there's no macro. */
    fpCurrShedRec->HotSpotRec.MacroPtr = NULL;
    
    /* Go to next shed record. */
    fpCurrShedRec = fpCurrShedRec + 1;
  }

  /* Point back to first shed record. */
  fpCurrShedRec = *fpSHEDRecs;

  /* For each hotspot. */
  for( wCounter = 0; wCounter < wNumHotSpots; wCounter++ )
  {
    /* If its a macro hotspot. */
    if( fpCurrShedRec->HotSpotRec.HotSpotType == HOTSPOT_MACRO )
    {
      /* Get number of bytes of macro string ( not includ. NULL ). */
      wBytes = (WORD) _fstrlen( fpHotSpotData ); 
      
      /* Add in space for NULL. */
      wBytes = wBytes + sizeof( char );
      
      /* Allocate space for macro. */
      fpCurrShedRec->HotSpotRec.MacroPtr = (char __far *) MyAlloc( fpElementList->fpMemoryList, wBytes );
      if( fpCurrShedRec->HotSpotRec.MacroPtr == NULL )
      {
        /* Failure. */
        return( ERR_MEMORY );
      }
      
      /* Copy the macro string. */
      _fstrcpy( fpCurrShedRec->HotSpotRec.MacroPtr, fpHotSpotData );

      /* Move to next macro string. */
      fpHotSpotData = fpHotSpotData + wBytes;
    }
  
    /* Go to next shed record. */
    fpCurrShedRec = fpCurrShedRec + 1;
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads bitmap picture data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocBitmapPictRec
(
  FPELEMENTLIST   fpElementList,
  HWND           hErrorWnd,
  BYTE __huge *  fpPictData,
  WORD           wNumHotSpots,
  BYTE           AlignType,
  FPTOPICELEMENT fpTopicElement 
)
{
  /* Size of header data. */
  #define BITMAP_HEAD_SIZE 36

  FPBITMAPREC fpBitmapRec;

  WORD wNumColors;
  PBITMAPINFO PtrBmi;
    
  long int biWidth;
  long int biHeight;
  WORD     biPlanes;
  WORD     biBitCount;

  BYTE      Compressed;
  DWORD     dwUncompressSize;
  long int  nLineWidth;

  DWORD    dwBitmapDataSize;   
  DWORD    dwResultSize;
  BYTE __huge * fpBitMapData;  

  RETVAL RetVal;
  
  WORD biCompression;

  
  /* What type of bitmap record type is it. */
  switch( AlignType )
  {
    case 0x86:
    {
      fpTopicElement->wRecType = TE_CHARBITMAP;
      break;
    }
    
    case 0x87:
    {
      fpTopicElement->wRecType = TE_LEFTBITMAP;
      break;
    }
    
    case 0x88:
    {
      fpTopicElement->wRecType = TE_RIGHTBITMAP;
      break;
    }
  }

  /* Allocate record's data. */
  fpTopicElement->hRecData = GlobalAlloc( GHND, sizeof(BITMAPREC) );
  if( fpTopicElement->hRecData == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Lock the record's  data. */
  fpBitmapRec = (FPBITMAPREC) GlobalLock( fpTopicElement->hRecData );
  

  /* 
  ** Get bitmap info. values. 
  */

  /* Is it compressed. */
  Compressed = *(fpPictData + 9);

  /* Get the number of planes. */
  biPlanes = *(fpPictData + 14);
  biPlanes = biPlanes / 2;

  /* Get the number of bits-per-pixel. */
  biBitCount = *(fpPictData + 15);
  biBitCount = biBitCount / 2;

  /* Get the picture width. */
  biWidth = ReadWORD( fpPictData + 16 );
  biWidth = biWidth / 2;

  /* Get the picture height. */
  biHeight = ReadWORD( fpPictData + 18 );
  biHeight = biHeight / 2;

  /* Get the number of colors in the palette. */
  wNumColors = ReadWORD( fpPictData + 20 );
  wNumColors = wNumColors / 2;

  /* Compressed bitmap data size ( not including palette ). */
  dwBitmapDataSize = (DWORD) ReadWORD( fpPictData + 24 );
  dwBitmapDataSize = dwBitmapDataSize / 2;

  /* Default is uncompressed data. */
  biCompression = BI_RGB;

  /* 
  ** Microsoft uses a non-documented compression
  ** routines to compress the bitmap data.
  */

  /* If compressed, uncompress it. */
  if( Compressed != 0 )
  {
    /* 
    ** Calculate uncompressed size 
    ** of bitmap data. 
    */
    
    /* Get number of bits per scan line. */
    nLineWidth = CalcBitmapByteWidth( biWidth, biBitCount );
    
    /* Unsupported bitmap - bits per pixel value not supported. */
    if( nLineWidth == 0 ) 
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
      
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
        
      /* Unsupported bitmap. */ 
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_UNSUPPORTPICT, MB_ICONHAND | MB_OK );

      return( ERR_OTHER );
    }

    /* Uncompressed image size. */
    dwUncompressSize = (DWORD) ( nLineWidth * biHeight );
    

    /* 
    ** Uncompress the image. 
    */

    /* PCX style compression used when .HLP not compressed. */
    if( Compressed == 1 )
    {
      /* Uncompress it. */
      RetVal = UncompressPict( hErrorWnd, fpPictData + BITMAP_HEAD_SIZE + 
                               (wNumColors * sizeof(RGBQUAD)), 
                               dwBitmapDataSize, dwUncompressSize, 
                               &(fpBitmapRec)->hBitmapData, &dwResultSize );
      
      /* Uncompress failure? */
      if( RetVal != NO_ERROR ) 
      {
        /* Unlock the record's  data. */
        GlobalUnlock( fpTopicElement->hRecData );
      
        /* Free the record's data. */
        GlobalFree( fpTopicElement->hRecData );
        
        return( RetVal );
      }
    }


    /* Compression used when .HLP is compressed. */
    else if ( Compressed == 2 )
    {
      /* Allocate the bitmap data buffer. */         
      fpBitmapRec->hBitmapData = GlobalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE, dwUncompressSize );
      if( fpBitmapRec->hBitmapData == NULL )
      {
        /* Unlock the record's  data. */
        GlobalUnlock( fpTopicElement->hRecData );
      
        /* Free the record's data. */
        GlobalFree( fpTopicElement->hRecData );
    
        /* Failure. */
        return( ERR_MEMORY );
      }
    
      /* UncompressPict2 the data. */
      UncompressPict2( hErrorWnd, fpPictData + BITMAP_HEAD_SIZE + 
                       (wNumColors * sizeof(RGBQUAD)), 
                       dwBitmapDataSize, dwUncompressSize, &(fpBitmapRec)->hBitmapData, 
                       &dwResultSize ) ;
    }
  }

  /* If not compressed, copy it as is. */
  else
  {
    /* Allocate the bitmap data buffer. */         
    fpBitmapRec->hBitmapData = GlobalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE, dwBitmapDataSize );
    if( fpBitmapRec->hBitmapData == NULL )
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
    
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
  
      /* Failure. */
      return( ERR_MEMORY );
    }
  
    /* Lock the bitmap data buffer. */
    fpBitMapData = (BYTE __huge *) GlobalLock( fpBitmapRec->hBitmapData );
    
    /* Copy the bitmap data. */
    _fmemcpy( fpBitMapData, fpPictData + BITMAP_HEAD_SIZE + (wNumColors * sizeof(RGBQUAD)),
              (size_t) dwBitmapDataSize );
  
    /* Unlock the bitmap data buffer. */
    GlobalUnlock( fpBitmapRec->hBitmapData );
  }
  
  /* Allocate bitmap header and palette. */
  fpBitmapRec->hBitmapInfo = GlobalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE,
                             sizeof(BITMAPINFOHEADER) + (wNumColors * sizeof(RGBQUAD)) ); 
  if( fpBitmapRec->hBitmapInfo == NULL )
  {
    /* Free bitmap data buffer. */
    GlobalFree( fpBitmapRec->hBitmapData );
  
    /* Unlock the record's  data. */
    GlobalUnlock( fpTopicElement->hRecData );
  
    /* Free the record's data. */
    GlobalFree( fpTopicElement->hRecData );

    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Lock bitmap info. buffer. */
  PtrBmi = (PBITMAPINFO) GlobalLock( fpBitmapRec->hBitmapInfo );
  
  /* Save the palette data. */
  if( wNumColors )
  {
    _fmemcpy( PtrBmi->bmiColors, fpPictData + BITMAP_HEAD_SIZE, 
              wNumColors * sizeof(RGBQUAD) );
  }

  /* Save bitmap header data. */
  PtrBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  PtrBmi->bmiHeader.biWidth = biWidth;
  PtrBmi->bmiHeader.biHeight = biHeight;
  PtrBmi->bmiHeader.biPlanes = biPlanes;
  PtrBmi->bmiHeader.biBitCount = biBitCount;
  PtrBmi->bmiHeader.biCompression = biCompression;

  /* Unlock bitmap info. buffer. */
  GlobalUnlock( fpBitmapRec->hBitmapInfo );

  /* If there are SHED hotspots for the bitmap. */
  if( wNumHotSpots > 0 )
  {
    /* Capture SHED hotspot information. */
    RetVal = AllocSHEDHotSpots( fpElementList, hErrorWnd, wNumHotSpots, 
                                &(fpBitmapRec)->fpSHEDRecs,
                                fpPictData + BITMAP_HEAD_SIZE + 
                                (wNumColors * sizeof(RGBQUAD)) + 
                                dwBitmapDataSize );

    /* If no error. */
    if( RetVal == NO_ERROR )
    {
      /* Save number of hotspots. */
      fpBitmapRec->wNumSHEDRecs = wNumHotSpots;
    }
    /* If memory error, return that error. */
    else if( RetVal == ERR_MEMORY )
    {
      /* Free the bitmap info. */
      GlobalFree( fpBitmapRec->hBitmapInfo );
    
      /* Free the image data. */
      GlobalFree( fpBitmapRec->hBitmapData );
    
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
    
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
    
      /* Failure. */
      return( ERR_MEMORY );
    }
    /* Just don't have any hotspots. */
    else
    {
      fpBitmapRec->wNumSHEDRecs = 0;
    }
  }

  /* No SHED hotspots. */
  else
  {
    fpBitmapRec->wNumSHEDRecs = 0;
  }
  
  /* Unlock the record's  data. */
  GlobalUnlock( fpTopicElement->hRecData );
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Calculates the number of bits in a bitmap's
* single scanline.
*
****************************************************/
long int __far __pascal CalcBitmapByteWidth
(
  long int nBitWidth,
  WORD wBitsPerPixel
)
{
  long int nResult = 0;
  
  switch( wBitsPerPixel )
  {
    case 1:
    {
      nResult = ( (nBitWidth + 31) / 32 ) << 2;
      break;
    }

    case 4:
    {
      nResult = ( (nBitWidth + 7) / 8 ) << 2;
      break;
    }
    
    case 8:
    {
      nResult = ( (nBitWidth + 3) / 4 ) << 2;
      break;
    }
    
    case 16:
    {
      nResult = ( (nBitWidth + 1) & ~1 ) * 2;
      break;
    }
  }
  
  return( nResult );
}




/***************************************************
*
* Frees bitmap picture data in a TOPICELEMENT record.
*
****************************************************/
static void __far __pascal FreeBitmapPictRec( FPTOPICELEMENT fpTopicElement )
{
  FPBITMAPREC fpBitmapRec;

  /* Lock the record's  data. */
  fpBitmapRec = (FPBITMAPREC) GlobalLock( fpTopicElement->hRecData );
  
  /* Free the bitmap info. */
  GlobalFree( fpBitmapRec->hBitmapInfo );

  /* Free the image data. */
  GlobalFree( fpBitmapRec->hBitmapData );

  /* Unlock the record's  data. */
  GlobalUnlock( fpTopicElement->hRecData );

  /* Free the record's data. */
  GlobalFree( fpTopicElement->hRecData );
}


/***************************************************
*
* Reads metafile data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocMetaFilePictRec
(
  FPELEMENTLIST  fpElementList,
  HWND           hErrorWnd,
  BYTE __far *   fpPictData,
  WORD           wNumHotSpots,
  BYTE           AlignType,
  FPTOPICELEMENT fpTopicElement 
)
{
  /* Size of header data. */
  #define METAFILE_HEAD_SIZE 29
  
  FPMETAREC fpMetaRec;
  DWORD  dwMetaDataSize;   
  BYTE   Compressed;
  RETVAL RetVal;

  HFILE   hFile;
  HGLOBAL hFileData;
  BYTE __huge * fpFileData;
  DWORD   dwFileSize;
  

  /* What type of bitmap record type is it. */
  switch( AlignType )
  {
    case 0x86:
    {
      fpTopicElement->wRecType = TE_CHARMETA;
      break;
    }
    
    case 0x87:
    {
      fpTopicElement->wRecType = TE_LEFTMETA;
      break;
    }
    
    case 0x88:
    {
      fpTopicElement->wRecType = TE_RIGHTMETA;
      break;
    }
  }
  
/****** DISABLED BY SPECIFYING UNKNOWN TYPE ******/

//fpTopicElement->wRecType = 0;
//return NO_ERROR;

/****************************************/

  /* Allocate record's data. */
  fpTopicElement->hRecData = GlobalAlloc( GHND, sizeof(METAREC) );
  if( fpTopicElement->hRecData == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Lock the record's  data. */
  fpMetaRec = (FPMETAREC) GlobalLock( fpTopicElement->hRecData );
  
  /* Get a temporary filename. */
  GetTempFileName( 0, "hlp", 0, fpMetaRec->szMetaFile );

  /* 
  ** Get bitmap info. values. 
  */

  /* Is it compressed. */
  Compressed = *(fpPictData + 9);

  /* Get the picture width. */
  fpMetaRec->wWidth = ReadWORD( fpPictData + 11 );
  fpMetaRec->wWidth = fpMetaRec->wWidth / 2;

  /* Get the picture height. */
  fpMetaRec->wHeight = ReadWORD( fpPictData + 13 );
  fpMetaRec->wHeight = fpMetaRec->wHeight / 2;

  /* Metafile data size ( size of |bm# not including |bm# file header ). */
  dwMetaDataSize = ReadDWORD( fpPictData + 17 );
  dwMetaDataSize = dwMetaDataSize / 2;

  /* 
  ** Microsoft uses a non-documented compression
  ** routine to compress the metafile's data.
  */

  /* If compressed, uncompress it. */
  if( Compressed != 0 )
  {
    /* 
    ** Uncompress the image. 
    */

    /* PCX style compression used when .HLP file not compressed. */
    if( Compressed == 1 )
    {
      /* Uncompress it. */
      RetVal = UncompressPict( hErrorWnd, fpPictData + METAFILE_HEAD_SIZE, 
                               dwMetaDataSize, 0, &hFileData, &dwFileSize );
      
    }

    /* Compression used when .HLP is compressed. */
    else if ( Compressed == 2 )
    {
      /* UncompressPict2 the data. */
      RetVal = UncompressPict2( hErrorWnd, fpPictData + METAFILE_HEAD_SIZE, 
                                dwMetaDataSize, 0, &hFileData, &dwFileSize ) ;
    }

    /* Uncompress failure? */
    if( RetVal != NO_ERROR ) 
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
      
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
        
      return( RetVal );
    }

    /* Open the metafile's file. */
    hFile = _lopen( fpMetaRec->szMetaFile, WRITE ); 
    if( hFile == HFILE_ERROR )
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
      
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
        
      /* Free the file data. */
      GlobalFree( hFileData );

      return( ERR_OTHER );
    }

    /* Lock the metafile's file data. */
    fpFileData = (BYTE __huge *) GlobalLock( hFileData );
  
    /* Write the metafile's file data. */
    if( (long) _hwrite( hFile, fpFileData, dwFileSize ) != (long) dwFileSize )
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
      
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
        
      /* Unlock the file data. */
      GlobalUnlock( hFileData );
      
      /* Free the file data. */
      GlobalFree( hFileData );

      /* Close the metafile's file. */
      _lclose( hFile );
      
      /* Delete the temporary file. */
      remove( fpMetaRec->szMetaFile );

      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }

    /* Close the metafile's file. */
    _lclose( hFile );
      
    /* Unlock the file data. */
    GlobalUnlock( hFileData );

    /* Free the file data. */
    GlobalFree( hFileData );
  }

  /* If not compressed, copy it as is. */
  else
  {
    /* Open the metafile's file. */
    hFile = _lopen( fpMetaRec->szMetaFile, WRITE ); 
    if( hFile == HFILE_ERROR )
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
      
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
        
      /* Free the file data. */
      GlobalFree( hFileData );

      return( ERR_OTHER );
    }

    /* Write the metafile's file data. */
    if( (long)_hwrite( hFile, fpPictData + METAFILE_HEAD_SIZE, dwMetaDataSize ) 
        != (long) dwMetaDataSize )
    {
      /* Unlock the record's  data. */
      GlobalUnlock( fpTopicElement->hRecData );
      
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
        
      /* Close the metafile's file. */
      _lclose( hFile );
      
      /* Delete the temporary file. */
      remove( fpMetaRec->szMetaFile );

      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }

    /* Close the metafile's file. */
    _lclose( hFile );
  }

  /* Get handle to the metafile file's data. */
  fpMetaRec->hMetaFile = GetMetaFile( fpMetaRec->szMetaFile );

  /* If there are SHED hotspots for the metafile. */
  if( wNumHotSpots > 0 )
  {
    /* Capture SHED hotspot information. */
    RetVal = AllocSHEDHotSpots( fpElementList, hErrorWnd, 
                                wNumHotSpots, &(fpMetaRec)->fpSHEDRecs,
                                fpPictData + METAFILE_HEAD_SIZE + 
                                dwMetaDataSize );
  
    /* If no error. */
    if( RetVal == NO_ERROR )
    {
      /* Save number of hotspots. */
      fpMetaRec->wNumSHEDRecs = wNumHotSpots;
    }
    /* If memory error, return that error. */
    else if( RetVal == ERR_MEMORY )
    {
      /* Free the metafile handle. */
      DeleteMetaFile( fpMetaRec->hMetaFile );
    
      /* Delete the temporary file. */
      remove( fpMetaRec->szMetaFile );
  
      /* Unlock the record's data. */
      GlobalUnlock( fpTopicElement->hRecData );
  
      /* Free the record's data. */
      GlobalFree( fpTopicElement->hRecData );
    
      /* Failure. */
      return( ERR_MEMORY );
    }
    /* Just don't have any hotspots. */
    else
    {
      fpMetaRec->wNumSHEDRecs = 0;
    }
  }

  /* No SHED hotspots. */
  else
  {
    fpMetaRec->wNumSHEDRecs = 0;
  }
  
  /* Unlock the record's data. */
  GlobalUnlock( fpTopicElement->hRecData );
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Frees metafile data in a TOPICELEMENT record.
*
****************************************************/
static void __far __pascal FreeMetaFilePictRec( FPTOPICELEMENT fpTopicElement )
{
  FPMETAREC fpMetaRec;

  /* Lock the record's  data. */
  fpMetaRec = (FPMETAREC) GlobalLock( fpTopicElement->hRecData );
  
  /* Free the metafile handle. */
  DeleteMetaFile( fpMetaRec->hMetaFile );

  /* Delete the temporary file. */
  remove( fpMetaRec->szMetaFile );

  /* Free the record's data. */
  GlobalFree( fpTopicElement->hRecData );
}


/******************************** COMPRESSION ROUTINES **************************************/

/***************************************************
*
* Uncompresses a picture's data.  The
* algorithm is a variation on the .PCX file 
* compression algorithm.
*
****************************************************/
static RETVAL __far __pascal UncompressPict
(
  HWND hErrorWnd,
  BYTE __huge * fpCompressed,             /* Compressed data. */
  DWORD dwCompressSize,                   /* Size in bytes of compressed data. */
  DWORD dwUnCompressSize,                 /* Size in bytes of uncompressed data. 
                                             If 0 then value is unknown. */
  HGLOBAL __far * hOutBuffer,             /* Handle to use for uncompressed data buffer. */
  DWORD __far * dwBufferSize              /* Return value - size of uncompressed data bytes. */
)
{
  #define INCREASE_SIZE  1024
  
  BYTE __huge * fpOutBuffer;               /* Pointer to the output buffer. */
  DWORD dwOutBufferPos;                   /* Current position in the output buffer. */

  DWORD dwBytesLeft;                      /* Compressed bytes left to process. */
  
  BOOL bReadCode;                         /* Is the compression code to read or duplicate. */
  WORD wCount;                            /* Bytes to process for a compression code. */
  
  BOOL bDynamicReAlloc;                   /* Dynamically realloc uncompressed data's buffer. */
  
  
  /* At first position in the out buffer. */
  dwOutBufferPos = 0;
  
  /* Bytes left to uncompress. */
  dwBytesLeft = dwCompressSize;

  /* No size for uncompressed data's buffer. */
  if( dwUnCompressSize == 0 ) 
  {
    bDynamicReAlloc = TRUE; 
    dwUnCompressSize = INCREASE_SIZE;
  }
  else 
  {
    bDynamicReAlloc = FALSE;
  }
  
  /* Allocate starting size for the output data buffer. */         
  *hOutBuffer = GlobalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE, dwUnCompressSize );
  if( *hOutBuffer == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }
  
  /* Lock the output data buffer. */
  fpOutBuffer = ( BYTE __huge * ) GlobalLock( *hOutBuffer );
  
  /* While there are bytes left to uncompress. */
  while( dwBytesLeft > 0 )
  {
    
    /* READ code. */
    if( *fpCompressed & 0x80 )
    {
      /* 8th bit set - READ compressed data. */
      bReadCode = TRUE;
      
      /* Strip off 8th bit. */
      wCount = *fpCompressed & 0x7F;
    }
  
    /* Duplication code. */
    else
    {
      /* 8th bit not set - DUPLICATE next byte. */
      bReadCode = FALSE;

      wCount = *fpCompressed;
    }

    /* One less byte to process. */
    fpCompressed++;
    dwBytesLeft--;

    /* If our output buffer is not big enough for next data. */  
    while( dwOutBufferPos + wCount > dwUnCompressSize )
    {
      /* Can't dynamically realloc the size. */
      if( ! bDynamicReAlloc )
      {
        /* Unlock the output data buffer. */
        GlobalUnlock( *hOutBuffer );
    
        /* Free the output data buffer. */
        GlobalFree( *hOutBuffer );
    
        /* Decompression error. */ 
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADPICTCOMPRESS, MB_ICONHAND | MB_OK );
  
        /* Failure. */
        return( ERR_OTHER );
      }
      
      /* Can dynamically realloc the size. */
      else
      {
        /* Unlock the output data buffer. */
        GlobalUnlock( *hOutBuffer );
      
        /* Increase size of buffer. */
        dwUnCompressSize += INCREASE_SIZE;

        /* Allocate bigger buffer. */
        *hOutBuffer = GlobalReAlloc( *hOutBuffer, dwUnCompressSize, LMEM_MOVEABLE | GMEM_ZEROINIT );
      
        if( *hOutBuffer == NULL )
        {
          /* Failure. */
          return( ERR_MEMORY );
        }
  
        /* Lock the output data buffer. */
        fpOutBuffer = ( BYTE __huge * ) GlobalLock( *hOutBuffer );
      }
    }

    /* Perform action based on code. */
    if( bReadCode )
    {
      /* 
      ** Next wCount bytes can be read and written 
      ** into the output buffer. 
      */
      _fmemcpy( fpOutBuffer + dwOutBufferPos, fpCompressed, (size_t) wCount );
          
      /* Count less bytes to process. */
      fpCompressed = fpCompressed + wCount;
      dwBytesLeft = dwBytesLeft - wCount;

      /* Move to new output buffer position. */
      dwOutBufferPos = dwOutBufferPos + wCount;
    }

    else
    {
      /* Duplicate next byte wCount times to the output buffer. */
      _fmemset( fpOutBuffer + dwOutBufferPos, *fpCompressed, (size_t) wCount );

      /* One less byte to process. */
      fpCompressed++;
      dwBytesLeft--;

      /* Move to new output buffer position. */
      dwOutBufferPos = dwOutBufferPos + wCount;
    }
  }

  /* Unlock the output data buffer. */
  GlobalUnlock( *hOutBuffer );
  
  /* Return output data buffer size. */
  *dwBufferSize = dwOutBufferPos;

  /* Success. */
  return( NO_ERROR );
} 



/***************************************************
*
* Decompresses the data using Microsoft's LZ77
* derivative.    
*
* This routine reads the compressed data
* from a buffer and decompressed the data.
*
* There is another version of the routine in the
* HLPFILE.C file that reads the compressed data
* from a file and decompressed the data.  The
* function is called Decompress().
*
* This routine and the HLPFILE.C version
* used the same algorithm - just different
* sources.
*
****************************************************/
static RETVAL __far __pascal UncompressPict2
( 
  HWND hErrorWnd,
  BYTE __huge * CompressedBuffer, 
  DWORD dwCompressedSize, 
  DWORD dwUnCompressedSize,      /* If 0 then dynamically alloc. uncompressed data buffer. */ 
  HGLOBAL __far * hOutBuffer,    /* Handle to use for uncompressed data buffer. */
  DWORD __far * dwOutSize        /* Return value - size of uncompressed data bytes. */
) 
{
  #define SET_SIZE 16
  #define INCREASE_SIZE2  1024

  DWORD dwInBytes = 0;         /* How many bytes read in.                    */
  BYTE  BitMap,                /* Bitmap and bytes associated with it.       */
        Set[SET_SIZE];         
  WORD  wNumToRead;            /* Number of bytes to read in the Set array.  */
  WORD  wBitCounter,           /* Going through next 8-16 codes or chars.    */
        wIndex;   
  WORD  wLength,               /* Code length and distance back in 'window'. */
        wDistance;     
  BYTE __huge * UncompressPos; /* Where we are at any given moment in the 
                                  uncompressed buffer. */
  BYTE __huge * CodePtr;        /* Pointer to back-up in LZ77 'window'.       */
  WORD wSetCounter;   
  
  BOOL bDynamicReAlloc;   /* Dynamically realloc uncompressed data's buffer. */
  BOOL bDone = FALSE;     /* Done uncompressing? */
 
 
  /* No size for uncompressed data's buffer. */
  if( dwUnCompressedSize == 0 ) 
  {
    bDynamicReAlloc = TRUE; 
    dwUnCompressedSize = INCREASE_SIZE2;

    /* Allocate starting size for the output data buffer. */         
    *hOutBuffer = GlobalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE, dwUnCompressedSize );
    if( *hOutBuffer == NULL )
    {
      /* Failure. */
      return( ERR_MEMORY );
    }
  }
  else 
  {
    bDynamicReAlloc = FALSE;
  }
  
  /* Get the current position in the buffer. */
  UncompressPos = ( BYTE __huge * ) GlobalLock( *hOutBuffer );
     
  /* How many bytes written out. */
  *dwOutSize = 0;
  
  /* While we haven't read everything. */
  while ( dwInBytes < dwCompressedSize && !bDone )
  {
    /* Read bitmap. */
    BitMap = *CompressedBuffer;
    CompressedBuffer++;
    dwInBytes = dwInBytes + 1;
    
    /* Get number to read. */
    wNumToRead = (WORD) BytesToRead( BitMap );
  
    /* If we are being asked to read more than we should. */
    if( ( dwCompressedSize - dwInBytes) < wNumToRead )
    { 
      /* Only read what we have left. */
      wNumToRead = (WORD) (dwCompressedSize - dwInBytes);   
    }
        
    /* Read in the compressed data. */
    for( wSetCounter = 0; wSetCounter < wNumToRead; wSetCounter++ )
    {
      Set[ wSetCounter ] = *CompressedBuffer;
      CompressedBuffer++;
    }
    dwInBytes = dwInBytes + wNumToRead;
  
    /* Go through the compressed data and decode it. */
    for( wBitCounter = 0, wIndex = 0; wBitCounter < 8 && !bDone; wBitCounter++ ) 
    {
      /* It's a code, so decode it and copy the data. */
      if( BitSet( BitMap, wBitCounter ) ) 
      {
        wLength = ( (Set[wIndex+1] & 0xF0) >> 4 ) + 3;
        wDistance = ( 256 * (Set[wIndex+1] & 0x0F) ) + Set[wIndex] + 1;
            
        /* Pointer into decompress window. */
        CodePtr = UncompressPos - wDistance;   
            
        /* Verify that we have the space. */
        while( *dwOutSize + wLength > dwUnCompressedSize )
        {
          /* If realloc uncompressed buffer on the fly. */
          if( bDynamicReAlloc )
          {
            /* Unlock the output data buffer. */
            GlobalUnlock( *hOutBuffer );
            
            /* Increase size of buffer. */
            dwUnCompressedSize += INCREASE_SIZE2;
      
            /* Allocate bigger buffer. */
            *hOutBuffer = GlobalReAlloc( *hOutBuffer, dwUnCompressedSize, LMEM_MOVEABLE | GMEM_ZEROINIT );
            
            if( *hOutBuffer == NULL )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }
        
            /* Lock the buffer. */
            UncompressPos = ( BYTE __huge * ) GlobalLock( *hOutBuffer );
              
            /* Move to the new output position in the buffer. */
            UncompressPos = UncompressPos + *dwOutSize;
          }
            
          /* Only copy what we have to. */
          else
          {
            wLength = (WORD)( dwUnCompressedSize - *dwOutSize );
            bDone = TRUE;
          }
        }
        

        while( wLength )
        { 
          *UncompressPos++ = *CodePtr++; 
          *dwOutSize = *dwOutSize + 1; 
          wLength--; 
        } 
        wIndex += 2;  /* codes are 2 bytes */
      }
          
      /* Not a code. */
      else 
      { 
        /* Verify that we have the space. */
        if( *dwOutSize == dwUnCompressedSize )
        {
          /* If realloc uncompressed buffer on the fly. */
          if( bDynamicReAlloc )
          {
            /* Unlock the output data buffer. */
            GlobalUnlock( *hOutBuffer );
            
            /* Increase size of buffer. */
            dwUnCompressedSize += INCREASE_SIZE2;
      
            /* Allocate bigger buffer. */
            *hOutBuffer = GlobalReAlloc( *hOutBuffer, dwUnCompressedSize, LMEM_MOVEABLE | GMEM_ZEROINIT );
            
            if( *hOutBuffer == NULL )
            {
              /* Failure. */
              return( ERR_MEMORY );
            }
        
            /* Lock the buffer. */
            UncompressPos = ( BYTE __huge * ) GlobalLock( *hOutBuffer );
              
            /* Move to the new output position in the buffer. */
            UncompressPos = UncompressPos + *dwOutSize;
          }
            
          /* No realloc of uncompressed buffer on the fly. */
          else
          {
            /* No more space in the uncompressed buffer. */
            bDone = TRUE;
            continue;
          }
        }

        /* Copy the one byte to the output buffer. */
        *UncompressPos++ = Set[wIndex++]; 
        *dwOutSize = *dwOutSize + 1; 
      }
    }
  }
  
  /* Unlock the uncompressed data buffer. */
  GlobalUnlock( *hOutBuffer );

  /* Return the decompression size. */
  return( NO_ERROR );
} 


/***************************************************
*
* Decides how many bytes to read, depending on the
* number of bits set in the Bitmap.
*
****************************************************/
static WORD __far __pascal BytesToRead( BYTE BitMap ) 
{
  WORD wTempSum,
       wCounter;

  wTempSum = 8;
  
  for( wCounter = 0; wCounter < 8; wCounter ++ )
  {
    wTempSum += BitSet( BitMap, wCounter );
  }
  
  return wTempSum;
}


/***************************************************
*
* Returns:
*
*    1 = bit is set.
*    0 = bit not set.
*
****************************************************/
static WORD __far __pascal BitSet( BYTE BitMap, WORD wBit ) 
{
  if( BitMap & ( 1 << wBit ) ) return 1;
  else return 0;
}


/***************************************************
*
* Reads font code data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocFontRecord( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpTopicElement, LPBYTE __far * fpTopicCodes, FPHLPFILEINFO fpHelpFileInfo )
{
  FPFONTREC fpFontRec;
  

  /* Save record type. */
  fpTopicElement->wRecType = TE_FONT;
      
  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(FONTREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpFontRec = (FPFONTREC) fpTopicElement->RecDataPtr;

  /* Save the font index. */
  fpFontRec->wFontIndex = ReadWORD( *fpTopicCodes );
  *fpTopicCodes = *fpTopicCodes + sizeof( WORD );
  
  /* Fill font info. */
  if( ! GetFontListRec( fpHelpFileInfo, fpFontRec->wFontIndex, &(fpFontRec)->FontInfo ) )
  {
    /* Failure. */
    return( ERR_OTHER );
  }
  
  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Reads hotspot data from the TopicCodes buffer and
* and saves the data in a TOPICELEMENT record.
*
****************************************************/
static RETVAL __far __pascal AllocHotSpotRecord
( 
  FPELEMENTLIST fpElementList, 
  BYTE HotSpotCode,                     /* Hotspot's file code. */
  FPTOPICELEMENT fpTopicElement, 
  LPBYTE __far * fpTopicCodes 
)
{
  #define  SEC_WND  0x01  
  
  FPHOTSPOTREC fpHotSpotRec;

  WORD         wDataLength;


  /* Beginning of hotspot. */
  fpTopicElement->wRecType = TE_HOTBEGIN;

  /* Allocate record's data. */
  fpTopicElement->RecDataPtr = MyAlloc( fpElementList->fpMemoryList, sizeof(HOTSPOTREC) );
  if( fpTopicElement->RecDataPtr == NULL )
  {
    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Cast pointer. */
  fpHotSpotRec = (FPHOTSPOTREC) fpTopicElement->RecDataPtr;

  /* Init. pointers */
  fpHotSpotRec->HelpFilePathPtr = NULL;
  fpHotSpotRec->MacroPtr = NULL;

  /* Where is the hotspot displayed? */
  switch( HotSpotCode )
  {
    /* Popup hotspot. */
    case TC_HOTBEG_2:
    case TC_HOTBEG_6:
    case TC_HOTBEG_A:
    {
      fpHotSpotRec->HotSpotType = HOTSPOT_POPUP;
      break;
    }
  
    /* Jump hotspot. */
    case TC_HOTBEG_3:
    case TC_HOTBEG_7:
    case TC_HOTBEG_B:
    {
      fpHotSpotRec->HotSpotType = HOTSPOT_JUMP;
      break;
    }

    case TC_HOTBEG_C8:
    {
      fpHotSpotRec->HotSpotType = HOTSPOT_MACRO;
      break;
    }

    default: 
    {
      /* Error. */
      return( ERR_BADCODE );
    }
  }


  /* Underline and color hotspot text? */
  switch( HotSpotCode )
  {
    /* Use underlining and color. */
    case TC_HOTBEG_2:
    case TC_HOTBEG_3:
    case TC_HOTBEG_A:
    case TC_HOTBEG_B:
    case TC_HOTBEG_C8:
    {
      fpHotSpotRec->bColorAndUnder = TRUE;
      break;
    }
      
    /* No underlining or color. */
    case TC_HOTBEG_6:
    case TC_HOTBEG_7:
    {
      fpHotSpotRec->bColorAndUnder = FALSE;
      break;
    }
  }
    
    
  /* Get hotspot's topic data. */
  switch( HotSpotCode )
  {
    /* Hotspot's topic is in the same help file. */
    case TC_HOTBEG_2:
    case TC_HOTBEG_3:
    case TC_HOTBEG_6:
    case TC_HOTBEG_7:
    {
      /* Get the hash value. */
      fpHotSpotRec->dwHotSpotContext = ReadDWORD( *fpTopicCodes );
      *fpTopicCodes = *fpTopicCodes + sizeof( DWORD );
          
      /* Save the hotspot info. */
      fpHotSpotRec->HelpFilePathPtr = NULL; 
      
      break;
    }
        

    /* 
    ** Hotspot's topic is in another help file that is specified.
    */
    case TC_HOTBEG_A:
    {
      /* Get the length of string. */
      wDataLength = ReadWORD( *fpTopicCodes );
      *fpTopicCodes = *fpTopicCodes + sizeof( WORD );
      wDataLength = wDataLength - sizeof( BYTE ) - sizeof( DWORD );
            
      /* Skip next byte - unknown data.  Has been value 0x04. */
      *fpTopicCodes = *fpTopicCodes + sizeof( BYTE );
          
      /* Get the hash value. */
      fpHotSpotRec->dwHotSpotContext = ReadDWORD( *fpTopicCodes );
      *fpTopicCodes = *fpTopicCodes + sizeof( DWORD );
          
      /* Allocate buffer for help file name. */
      fpHotSpotRec->HelpFilePathPtr = (char __far *) MyAlloc( fpElementList->fpMemoryList, wDataLength * sizeof( BYTE ) );
      if( fpHotSpotRec->HelpFilePathPtr == NULL )
      {
        /* Failure. */
        return( ERR_MEMORY );
      }
          
      /* Copy filename. */
      _fstrcpy( fpHotSpotRec->HelpFilePathPtr, *fpTopicCodes );
      
      /* Skip to next data position. */
      *fpTopicCodes = *fpTopicCodes + wDataLength;

      break;
    }

    case TC_HOTBEG_B:
    {
      /* Get the length of data ( length of string ). */
      wDataLength = ReadWORD( *fpTopicCodes );
      *fpTopicCodes = *fpTopicCodes + sizeof( WORD );
      wDataLength = wDataLength - sizeof( BYTE ) - sizeof( DWORD );
            
      /* Display topic in a secondary window. */
      if( **fpTopicCodes == SEC_WND )
      {
        fpHotSpotRec->HotSpotType = HOTSPOT_SEC;
      }
      *fpTopicCodes = *fpTopicCodes + sizeof( BYTE );
         
      /* Get the hash value. */
      fpHotSpotRec->dwHotSpotContext = ReadDWORD( *fpTopicCodes );
      *fpTopicCodes = *fpTopicCodes + sizeof( DWORD );
          
      /* If secondary window. */
      if( fpHotSpotRec->HotSpotType == HOTSPOT_SEC )
      { 
        /* Get secondary window's number. */
        fpHotSpotRec->wSecWndNum = (WORD) **fpTopicCodes;
        *fpTopicCodes = *fpTopicCodes + sizeof( BYTE );
      }
      
      /* If NOT secondary window. */
      else
      {
        /* Allocate buffer for help file name. */
        fpHotSpotRec->HelpFilePathPtr = (char __far *) MyAlloc( fpElementList->fpMemoryList, wDataLength * sizeof( BYTE ) );
        if( fpHotSpotRec->HelpFilePathPtr == NULL )
        {
          /* Failure. */
          return( ERR_MEMORY );
        }
            
        /* Copy filename. */
        _fstrcpy( fpHotSpotRec->HelpFilePathPtr, *fpTopicCodes );
        
        /* Skip to next data position. */
        *fpTopicCodes = *fpTopicCodes + wDataLength;
      }
      break;
    }


    case TC_HOTBEG_C8:
    {
      /* Get the length of data ( length of string ). */
      wDataLength = ReadWORD( *fpTopicCodes );
      *fpTopicCodes = *fpTopicCodes + sizeof( WORD );
            
      /* Allocate buffer for help file name. */
      fpHotSpotRec->MacroPtr = (char __far *) MyAlloc( fpElementList->fpMemoryList, wDataLength * sizeof( BYTE ) );
      if( fpHotSpotRec->MacroPtr == NULL )
      {
        /* Failure. */
        return( ERR_MEMORY );
      }
            
      /* Copy filename. */
      _fstrcpy( fpHotSpotRec->MacroPtr, *fpTopicCodes );
        
      /* Skip to next data position. */
      *fpTopicCodes = *fpTopicCodes + wDataLength;

      break;
    }
  }

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Allocate memory for the element list info.
* strucuture and an initial number of empty element
* records.
*
****************************************************/
static BOOL __far __pascal AllocElementList( HGLOBAL __far * hElementList )
{
  FPELEMENTLIST  fpElementList;   /* Pointer to list info. */

  /* 
  ** First time allocation. 
  */

  /*
  ** Allocate the element list structure. 
  */
  *hElementList =  GlobalAlloc( GHND, sizeof(ELEMENTLIST) );
  if( *hElementList == NULL )
  {
    /* Failure. */
    return( FALSE );
  }
    
  /* Lock the element list info. */
  fpElementList = ( FPELEMENTLIST ) GlobalLock( *hElementList );
    
  /* 
  ** Allocate memory page list that will be used to alloc. memory for
  ** the element list and most of its contents.
  */
  if( ! AllocMemoryList( &(fpElementList)->hMemoryList ) )
  {
    /* Unlock the element list info. */
    GlobalUnlock( *hElementList );

    /* Free the element list info. */
    GlobalFree( *hElementList );

    /* Failure. */
    return( ERR_MEMORY );
  }

  /* Lock the memory page list info. */
  fpElementList->fpMemoryList = ( FPMEMORYLIST ) GlobalLock( fpElementList->hMemoryList );

  /*
  ** Allocate an initial size for the record list. 
  */
  fpElementList->hRecordList = GlobalAlloc( GHND, INCREMENT_SIZE * sizeof(TOPICELEMENT) );
  if( fpElementList->hRecordList == NULL )
  {
    /* Free the memory page list info. */
    GlobalUnlock( fpElementList->hMemoryList );
    FreeMemoryList( &(fpElementList)->hMemoryList );

    /* Free the element list info. */
    GlobalUnlock( *hElementList );
    GlobalFree( *hElementList );

    /* Failure. */
    return( ERR_MEMORY );
  }
  
  /* Set record counts. */
  fpElementList->wNumRecords = 0;
  fpElementList->wMaxRecords = INCREMENT_SIZE;

  /* Unlock the element list info. */
  GlobalUnlock( *hElementList );

  /* Success. */
  return( TRUE );
}



/***************************************************
*
* Allocate memory for the current number of element
* records in the list and for an additional number
* of new blank records.
*
****************************************************/
static BOOL __far __pascal ReAllocElementList( FPELEMENTLIST fpElementList )
{
  /*
  ** Reallocate the size for the element list. 
  */
  fpElementList->hRecordList = GlobalReAlloc( fpElementList->hRecordList, 
                               (fpElementList->wMaxRecords + INCREMENT_SIZE) * sizeof(TOPICELEMENT),
                               GMEM_MOVEABLE | GMEM_ZEROINIT );
  
  /* Error? */
  if( fpElementList->hRecordList == NULL )
  {
    /* Failure. */
    return( FALSE );
  }
      
  /* Increase the count of our list size. */
  fpElementList->wMaxRecords = fpElementList->wMaxRecords + INCREMENT_SIZE;

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Insert information into the next epmty element
* record in the list.
*
****************************************************/
static BOOL __far __pascal InsertNewElement( FPELEMENTLIST fpElementList, FPTOPICELEMENT fpData )
{
  FPTOPICELEMENT fpTopicElement;  /* Pointer to a record in the element list. */


  /* Need more space. */
  if( fpElementList->wNumRecords == fpElementList->wMaxRecords )
  {
    if( ! ReAllocElementList( fpElementList ) )
    {
      /* Failure. */
      return( FALSE );
    }
  }

  /* Lock the topic elements list and point to the first record. */
  fpTopicElement = ( FPTOPICELEMENT ) GlobalLock( fpElementList->hRecordList );

  /* Go to the insertion point. */
  fpTopicElement = fpTopicElement + fpElementList->wNumRecords;
  
  /* Save record data. */
  fpTopicElement->wRecType   = fpData->wRecType;
  fpTopicElement->hRecData   = fpData->hRecData; 
  fpTopicElement->RecDataPtr = fpData->RecDataPtr;
  
  /* Increment record count. */
  fpElementList->wNumRecords = fpElementList->wNumRecords + 1;

  /* Unlock the topic elements list. */
  GlobalUnlock( fpElementList->hRecordList );

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Frees any memory associated with a element 
* info. strucuture.
*
****************************************************/
void __far __pascal DestroyElementList( HGLOBAL __far * hElementList )
{
  FPELEMENTLIST  fpElementList;   /* Pointer to list info. */
  FPTOPICELEMENT fpTopicElement;  /* Pointer to a record in the element list. */
  WORD wCounter;                  /* A simple counter varaible.  */


  /* Empty list. */
  if( *hElementList == NULL ) return;
  
  /* Lock the element list info. */
  fpElementList = ( FPELEMENTLIST ) GlobalLock( *hElementList );

  /* Lock the topic elements list and point to the first record. */
  fpTopicElement = ( FPTOPICELEMENT ) GlobalLock( fpElementList->hRecordList );
  
  /* Go through each used record in the elements list. */
  for( wCounter = 0;  wCounter < fpElementList->wNumRecords; wCounter++ )
  {
    /* 
    ** Free memory used in picture record's since it is GlobalAlloc'd. 
    ** All of the other memory is alloc'd in the memory page list and
    ** will be freed at one time.
    */
    switch( fpTopicElement->wRecType )
    {
      /* Picture element. */
      case TE_CHARBITMAP:
      case TE_LEFTBITMAP:
      case TE_RIGHTBITMAP:
      case TE_CHARMETA:
      case TE_LEFTMETA:
      case TE_RIGHTMETA:
      {
        FreePictRecord( fpTopicElement );
        break;
      }
    }
   
    /* Goto next record. */
    fpTopicElement++;
  }
  
  /* 
  ** Free the memory page list info. 
  **
  ** This will also free all alloc'd space
  ** used by the element list records
  ** (other than picture records).
  */
  GlobalUnlock( fpElementList->hMemoryList );
  FreeMemoryList( &(fpElementList)->hMemoryList );

  /* Unlock the topic elements list. */
  GlobalUnlock( fpElementList->hRecordList );

  /* Free the topic elements list. */
  GlobalFree( fpElementList->hRecordList );

  /* Unlock the element list info. */
  GlobalUnlock( *hElementList );

  /* Free the element list info. */
  GlobalFree( *hElementList );

  *hElementList = NULL;
}


/***************************************************
*
* Fills a display list with display information for
* each of the topic's elements. 
*
* ElementNum = 0 gets first record's information.
*
****************************************************/
BOOL __far __pascal RetrieveElementListRec
( 
  FPELEMENTLIST fpElementList, 
  FPTOPICELEMENT fpData, 
  WORD wElementNum 
)
{
  FPTOPICELEMENT fpTopicElement;  /* Pointer to a record in the element list. */
  
  
  /* Are we past the end of the list? */
  if( wElementNum >= fpElementList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }

  /* Lock the topic elements list and point to the first record. */
  fpTopicElement = ( FPTOPICELEMENT ) GlobalLock( fpElementList->hRecordList );
  
  /* 
  ** Goto to the record where info will be retrieved. 
  */
  fpTopicElement = fpTopicElement + wElementNum;
  
  /* Copy data in record. */
  fpData->wRecType = fpTopicElement->wRecType;
  fpData->hRecData = fpTopicElement->hRecData;
  fpData->RecDataPtr = fpTopicElement->RecDataPtr;
 
  /* Unlock the topic elements list. */
  GlobalUnlock( fpElementList->hRecordList );

  /* Success. */
  return( TRUE );
}




