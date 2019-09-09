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
#ifndef DISPLAY_H
#define DISPLAY_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "fontbuff.h"
#include "hotspots.h"


/***********************************
**
**  Shared Defines
**
***********************************/

typedef char __far * CHARPTR;
typedef CHARPTR __far * CHARDBLPTR;


/* Defines for INCHES. */
#define ONE_INCH         1440
#define HALF_INCH         720
#define EIGTH_INCH        180
#define SIXTEENTH_INCH     90
#define THIRTYSEC_INCH     45


/* 
** Scroll bar data.
*/
typedef struct tagSCROLLDATA
{
  BOOL bVertScroll; /* Is there a vertical scroll bar? */
  BOOL bHorzScroll; /* Is there a horizontal scroll bar? */

  WORD wVertPos;    /* Current vertical offset from 0 position. */
  WORD wMaxVertPos; /* The maximum vertical offset from 0 position. */

  WORD wHorzPos;    /* Current horzontal offset from 0 position. */
  WORD wMaxHorzPos; /* The maximum horzontal offset from 0 position. */

  WORD wVertLine;  /* How many pixels we move vertically on a single up/down. */
  WORD wVertPage;  /* How many pixels we move vertically on a single pg up/pg down. */

  WORD wHorzLine;  /* How many pixels we move horizontally on a single left/right. */
  WORD wHorzPage;  /* How many pixels we move horizontally on a single pg left/pg right. */
}
SCROLLDATA;
typedef SCROLLDATA __far * FPSCROLLDATA;


/* Text/Picture/Border display list and list info. */
typedef struct tagDISPLAYLIST
{
  WORD    wNumRecords;      /* Number of used records in the list. */
  WORD    wMaxRecords;      /* Number of total (used/unused) records in the list. */
  HGLOBAL hRecordList;      /* List of display records ( different record types ). */
}
DISPLAYLIST;
typedef DISPLAYLIST __far * FPDISPLAYLIST;


/* 
** A window's display information.
*/
typedef struct tagDISPLAYINFO
{
  /* Topic data. */
  HGLOBAL hTopicData;            /* Topic's element list and file info. */

  /* Page layout lists. */
  HGLOBAL hTxtDisplayList;       /* List of TXTDISPLAYREC structures. */
  HGLOBAL hBorderDisplayList;    /* List of BORDERDISPLAYREC structures. */
  HGLOBAL hPictDisplayList;      /* List of PICTDISPLAYREC structures for justified picts. */
  FPDISPLAYLIST fpTxtDisplayList;
  FPDISPLAYLIST fpBorderDisplayList;
  FPDISPLAYLIST fpPictDisplayList;

  HHOTSPOTBUFFER hHotSpotBuffer; /* Hotspots in the display. */
  HFONTBUFFER hFontBuffer;       /* Window's font buffer. */
  SCROLLDATA ScrollData;         /* Scroll bar data. */
  POINT DisplaySize;             /* Size of window's display area (client area). */
                                
  WORD  wTopDisplayRec;           /* Number of display record displayed at the top of window. */
  WORD  wTopDisplayList;          /* The type of display list that the top record is in. */
  DWORD dwTopDisplayRecVertPos;   /* Vertical Page position of the top display record. */

  BOOL bUseVertScrollPos;        /* Force the use of the current vertical scroll bar position. */

  /* 
  ** Page offsets from top left corner of page 
  ** to part of page shown in window. 
  */
  long int PageXPosOffset;    /* Horzontal offset to visible part of page. */
  long int PageYPosOffset;    /* Vertical offset to visible part of page. */

  DWORD dwPageWidth;          /* Width of topic's page. */
  DWORD dwPageHeight;         /* Height of topic's page. */

  COLORREF BGColor;          /* Color of window's background. */
} 
DISPLAYINFO;
typedef DISPLAYINFO __far * FPDISPLAYINFO;


/***********************************
**
**  Shared Function Prototypes
**
***********************************/

BOOL __far __pascal StartDisplaySystem( HWND hWnd, BOOL bNonScrollWnd );
void __far __pascal StopDisplaySystem( HWND hWnd, BOOL bNonScrollWnd );

BOOL __far __pascal UnlockSameTopic( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr );
BOOL __far __pascal SameHelpFile( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr );
void __far __pascal SetNewDisplayTopic( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr );
void __far __pascal SetNewDisplayTopic2( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr );

void __far __pascal SetDisplayBGColor( HWND hWnd, COLORREF ColorRef );
COLORREF __far __pascal GetDisplayBGColor( HWND hWnd );

BOOL __far __pascal SizeNonScrollWindow( HWND hWnd, RECT __far * fpMaxSizeRect );
BOOL __far __pascal DisplayNonScrollTopic( HWND hWnd );
BOOL __far __pascal HasNonScrollData( HWND hWnd );

BOOL __far __pascal SizePopupWindow( HWND hWnd, RECT __far * HotSpotPosRect );
BOOL __far __pascal DisplayPopupTopic( HWND hWnd );

BOOL __far __pascal DisplayTopic( HWND hWnd );

void __far __pascal UpdateScrollBar( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
WORD __far __pascal GetVertScrollPos( HWND hWnd );

void __far __pascal SetCorrectCursor( HWND hWnd, POINT WndPoint );

BOOL __far __pascal GetHotSpotTopicInfo
( 
  HWND hWnd,                           /* Topic window. */
  POINT WndPoint,                      /* Mouse click window position point. */
  HTOPICDATA __far * hNewTopicsData,   /* Hot spots data. */
  HOTSPOTTYPE __far * HotSpotType,     /* Type of hotspot. */
  RECT __far * WndPosRect,             /* Position of hotspot. */
  WORD __far * wSecWndNumPtr,          /* Secondary window's number. */
  CHARDBLPTR MacroDblPtr               /* Macro string - If hotspot is linked to a macro. */
);


#endif



