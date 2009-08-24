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
#ifndef FILEBUFF_H
#define FILEBUFF_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>
#include <stdlib.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "hlpfile.h"


/***********************************
**
**  Shared Defines
**
***********************************/

/* Record in the help file data buffer. */
typedef struct tagHLPFILEBUFFREC
{
  BOOL     bValid;           /* Is data in record valid. */

  DWORD    dwTickCount;     /* When file was opened. Used to removed the oldest
                               help file from the help file buffer. */
                               
  /* Key data for record - complete path to the help file. */
  char     szHelpFile[ _MAX_PATH ]; 

  HGLOBAL  hHelpFileInfo;   /* Handle to a HLPFILEINFO structure. */

  HGLOBAL  hTopicBuffer;    /* Handle to a buffer of TOPICBUFFREC structures. */
  
}
HLPFILEBUFFREC;
typedef HLPFILEBUFFREC __far * FPHLPFILEBUFFREC;


/* 
** Record in hTopicBuffer buffer. 
**
** Holds processed data for one 
** topic in a help file. 
*/
typedef struct tagTOPICBUFFREC
{
  BOOL          bValid;           /* Is data in record valid. */

  DWORD         dwUsage;          /* How many windows are using this topic. */
  
  DWORD         dwTickCount;      /* When topic was last used. Used to removed the oldest
                                     topic from the buffer. */

  /* dwTopicCharOffset is key data unique for each record. */
  DWORD         dwTopicCharOffset; /* Character offset for the topic's data in the
                                      |TOPIC file. */

  HGLOBAL       hScrollElements;      /* Display element data for the non-scrollable part of 
                                         topic. */
  HGLOBAL       hNonScrollElements;   /* Display element data for the non-scrollable part of 
                                         topic. */
  
  long          NextTopicCharOffset; /* Character offset for the next browse topic in the
                                        |TOPIC file. */
  
  long          PrevTopicCharOffset; /* Character offset for the prev. browse topic in the
                                        |TOPIC file. */
}
TOPICBUFFREC;
typedef TOPICBUFFREC __far * FPTOPICBUFFREC;


/* 
** All available topic information.
*/
typedef struct tagTOPICDATA
{
  /* 
  ** The dwBlock, wOffset, and the hHelpFileInfo->szFileName 
  ** can be used to identify which file and topic's data is stored
  ** in this structure.
  */
  DWORD     dwTopicCharOffset;    /* Character offset for the topic's data in the
                                     |TOPIC file. */

  char      szHelpFile[ _MAX_PATH ];  /* Name of help file. */

  HGLOBAL   hHelpFileInfo;       /* Handle to a HLPFILEINFO structure.     */
  HGLOBAL   hNonScrollElements;  /* Nonscrollable elements for the topic.  */ 
  HGLOBAL   hScrollElements;     /* Scrollable elements for the topic.     */ 
            
  long int  NextTopicCharOffset; /* Character offset for the next browse topic in the
                                    |TOPIC file. */
  
  long int  PrevTopicCharOffset; /* Character offset for the prev. browse topic in the
                                    |TOPIC file. */
}
TOPICDATA;
typedef TOPICDATA __far * FPTOPICDATA;
typedef HGLOBAL HTOPICDATA;               


/**********************************
**
**  Shared Function Prototypes
**
***********************************/

/***********************************
**
**  Shared Function Prototypes
**
***********************************/

HGLOBAL __far _pascal OpenHelpFile( HWND hMainWnd, char __far * szHelpFilePath );

BOOL __far __pascal AllocHelpFileBuffer( HWND hMainWnd );
void __far __pascal FreeHelpFileBuffer( HWND hMainWnd  );

HGLOBAL __far _pascal LockTopicData( HWND hMainWnd, char __far * szHelpFilePath, DWORD dwTopicCharOffset );
void __far _pascal UnlockTopicData( HWND hMainWnd, HGLOBAL hTopicData );

#endif



