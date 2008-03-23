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
#ifndef TOPICDAT_H
#define TOPICDAT_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>
#include <stdlib.h>


/***********************************
**
**  Shared Defines
**
***********************************/

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
  DWORD       dwBlockNum;       /* Topic's block number. */
  WORD        wBlockOffset;     /* Topic's offset within its block number. */

  char        szHelpFile[ _MAX_PATH ];  /* Name of help file. */

  HGLOBAL     hHelpFileInfo;    /* Handle to a HLPFILEINFO structure. */
  HGLOBAL     hElementList;     /* Element list for the topic.        */ 
}
TOPICDATA;
typedef TOPICDATA __far * FPTOPICDATA;


/**********************************
**
**  Shared Function Prototypes
**
***********************************/

HGLOBAL __far _pascal AllocTopicData( HWND hMainWnd, char __far * szHelpFilePath, DWORD dwBlockNum, WORD wBlockOffset );
void __far _pascal FreeTopicData( HWND hMainWnd, HGLOBAL hTopicData );

#endif



