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
**                                   DISPLAY ROUTINES
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
#include "winmem.h"
#include "wnddata.h"
#include "filebuff.h"
#include "fontbuff.h"
#include "elements.h"
#include "display.h"
#include "inifile.h"
#include "globals.h"
#include "twinhelp.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* Default text color. */
#define COLOR_BLACK  257

/* Default left indention. */
#define DEFAULT_LEFT_MARGIN   120;    

/* Default right indention. */
#define DEFAULT_RIGHT_MARGIN  120;   

/* Space of border under font. */
#define BOTTOM_BORDER_OFFSET        30;
#define AFTER_BOTTOM_BORDER_OFFSET  15;
#define TOP_BORDER_OFFSET           15;
#define AFTER_TOP_BORDER_OFFSET     15;
#define LEFT_BORDER_OFFSET          45;
#define RIGHT_BORDER_OFFSET         45;

/* Types of justification. */
typedef enum
{
  LEFT_JUSTIFIED,
  RIGHT_JUSTIFIED,
  CENTER_JUSTIFIED
}
JUSTIFY_TYPE;

/* 
** Context data.
*/
typedef struct tagCONTEXTDATA
{
  /* Short & long integers are used for values that can be negative and positive. */
  
  /* Page output layout/formating position. */
  long int PageXPos;    /* Current vertical position for page formating. */
  long int PageYPos;    /* Current horizontal position for page formating. */

  /* Vertical line spacing. */
  long int VertSpace;            /* User-defined vertical line spacing. */
  long int SpaceBeforeParagraph; /* Vertical space before a paragraph. */
  long int SpaceAddedBeforeParagraph; /* Assigned when we add space before a paragraph
                                          without knowing if the next paragraph has space 
                                          before it. */
  long int SpaceAfterParagraph;  /* Vertical space after a paragraph. */
  BOOL bSBAlreadyAdded;           /* Already inserted space before a paragraph. */
  
  /* Margins. */
  long int TableLeftMargin;     /* Left margin for current column in table. */ 
  long int TableRightMargin;    /* Right margin for current column in table. */
  long int LeftBorderMargin;    /* Left margin for current line's border.*/
  long int RightBorderMargin;   /* Right margin for current line's border. */
  long int LeftPictMargin;      /* Left margin for current line's picture.*/
  long int RightPictMargin;     /* Right margin for current line's picture. */
  long int LeftTxtMargin;       /* Left margin for current line's text.*/
  long int RightTxtMargin;      /* Right margin for current line's text. */

  /* Text line indentions. */
  WORD wDefaultLeftIndent;       /* Default left indention. */
  WORD wDefaultRightIndent;      /* Default right indention. */
  long int LeftIndent;          /* User-defined left indention. */
  long int RightIndent;         /* User-defined right indention. */
  long int FirstLineIndent;     /* Indent at start of new paragraph. */
  BOOL bOnFirstLine;             /* Are we at the first line of a new paragraph. */

  /* Justification. */
  JUSTIFY_TYPE JustifyType;  /* Type of justification. */

  /* Text attributes. */
  BYTE CharSet;              /* Current character set. Used when creating a new font. */ 
  BOOL bSmallCaps;           /* Does font specify smallcaps. */ 
  BOOL bDoubleUnd;           /* Does font specify double underline. */ 
  COLORREF LastFGTextColor;  /* Last foreground color of text for resetting text
                                color after a hotspot. */
  /* Hot spot info. */
  COLORREF HotSpotColor;     /* Color to draw hotspot text. */
  BOOL bInHotSpot;           /* Are we currently in a hotspot. */
  HOTSPOTREC HotSpotInfo;    /* Information about current hotspot. */

  /* Tab info. */
  WORD wCustomTabCount;      /* Number of custom tab stops. */
  TABSTOPPTR TabStopPtr;     /* Buffer of custom tab stop positions. */

  /* Line's display record info. */
  WORD wFirstTxtDisplayRec;      /* Number of first text display record for a line. */
  WORD wLastTxtDisplayRec;       /* Number of last text display record for a line. */

  /* Word wrapping. */
  BOOL bWordWrap;                /* Wrap a line that can't fit in the window? */
  
  /* Selected font info. */
  TEXTMETRIC FontMetrics;        /* Info. about currently selected font. */

  /* Picture info. */
  WORD wUnformatPicts;          /* Number of unformatted justified pictures belonging
                                   to the paragraph. */
  WORD wNextUnformatRec;        /* Number of next justified picture waiting to be 
                                   formatted. */
  POSRECT PictPosRect;          /* Page position of a justfied picture. */
  BOOL bLeftPicture;            /* Are we currently displaying a left justified picture. */
  BOOL bRightPicture;           /* Are we currently displaying a right justified picture. */

  /* Border info. */
  BOOL      bBorder;            /* Is there a border for the paragraph? */
  BORDERREC BorderInfo;         /* Information about border. */  
  WORD      wBorderWidth;       /* Width of border in LP. */
  WORD      wLineWidth;         /* Width of the border's line in LP. */
  POSRECT   BorderPosRect;      /* Position of border. */
  long int  ParaLeftMargin;     /* First position relative to left margin that we have
                                   output in the paragraph.  Used to calculate a left border's
                                   line position if there is one. */
  
  BOOL      bShouldHaveBorder;  /* If there are multiple paragraphs (line breaks) in 
                                   in a bordered area, we will not necessarily get a
                                   TE_PARD between the paragraph breaks.  
                                   If we have a border and we complete a paragraph and 
                                   do not get a TE_PARD command before
                                   outputting new objects, we should move down the page and
                                   start another border with the new objects in them. */
  
  /* Table info. */
  BOOL bInTable;                /* Are we drawing inside of a table. */
  WORD wNumColumns;             /* Number of columns in table. */
  COLUMNPOSPTR ColumnPosPtr;    /* Buffer of table column positions. */
  WORD wCurrColumn;             /* Current table column we are laying out. */
  long int RowYPos;             /* Starting vertical position for table's row. */
  long int RowHeight;           /* Calculated height of table's row. */
}
CONTEXTDATA;
typedef CONTEXTDATA __far * FPCONTEXTDATA;


/* 
** Record containing display information
** for the topic's text and character positioned
** pictures.
*/
typedef struct tagTXTDISPLAYREC
{
  WORD    wRecType;          /* What type of data is in the text display record. */

  /* Only one of these two is used. */
  HGLOBAL hElementData;     /* Element data. */
  LPVOID  ElementDataPtr;   /* Element data pointer. */
  
  WORD    wDataOffset;      /* Starting character offset if is a string. */
  WORD    wNumChars;        /* Number of characters of data if is a string. */ 
                               
  POSRECT PosRect;          /* Position that data will be displayed on full page. */

  WORD    wBaseLineHeight;  /* Space from top to baseline.  For a picture, it is 
                               the same as the picture's height. */
  DWORD   dwLinesYPos;      /* Line position of the text before it was aligned with
                               other objects on the same line. */ 

  BOOL bIsHotSpot;          /* Is text/char picture a hotspot. */
  HOTSPOTREC HotSpotInfo;   /* Information about hotspot. */

  WORD    wNumSHEDHotSpots; /* If this is a char. picture, how many SHED hotspots
                               does it have. */
}
TXTDISPLAYREC;
typedef TXTDISPLAYREC __far * FPTXTDISPLAYREC;


/* 
** Record containing display information
** for the topic's borders.
*/
typedef struct tagBORDERDISPLAYREC
{
  BORDERREC BorderInfo;         /* Border areas and line type of border. */  
  WORD      wLineWidth;         /* Width of border's line in LP. */
  POSRECT   PosRect;            /* Position of border. */
}
BORDERDISPLAYREC;
typedef BORDERDISPLAYREC __far * FPBORDERDISPLAYREC;


/* 
** Record containing display information
** for the topic's left/right justified
** pictures.
*/
typedef struct tagPICTDISPLAYREC
{
  WORD    wPictType;        /* Picture type. */
  HGLOBAL hPictData;        /* Picture data. */
  POSRECT PosRect;          /* Position that picture will be displayed on full page. */

  BOOL bIsHotSpot;          /* Is picture a hotspot. */
  HOTSPOTREC HotSpotInfo;   /* Information about hotspot. */

  WORD    wNumSHEDHotSpots; /* How many SHED hotspots does it have. */
}
PICTDISPLAYREC;
typedef PICTDISPLAYREC __far * FPPICTDISPLAYREC;


/* Number of records initially in a display list. */
#define DISPLAY_LIST_NUM   100

/* Kinds of display lists. */
#define NO_DISPLAY_LIST     0
#define TXT_DISPLAY_LIST    1
#define PICT_DISPLAY_LIST   2
#define BORDER_DISPLAY_LIST 3

/***********************************
**
**  Private functions
**
***********************************/
static void __far __pascal ClearDisplaySystem( HWND hWnd, FPDISPLAYINFO fpDisplayInfo, BOOL bNonScrollWnd );
static BOOL __far __pascal LayoutPage( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, BOOL UseNonScrollable );
static void __far __pascal GetPageSize( HWND hWnd, FPDISPLAYINFO fpDisplayInfo );
static BOOL __far __pascal DrawPage( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, BOOL UseNonScrollable );


static BOOL __far __pascal LayoutText
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement 
);

static BOOL __far __pascal DrawTextObject
( 
  HDC hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  FPTXTDISPLAYREC fpTxtDisplayRec 
);

static WORD __far __pascal GetCharFitCount
( 
  HDC hDC,
  FPCONTEXTDATA fpContextData,
  FPDISPLAYINFO fpDisplayInfo, 
  char __far * fpString, 
  WORD wStrLen,
  FPPOSRECT fpPosRect 
);


static BOOL __far __pascal InsertPicture
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement
);
static BOOL __far __pascal LayoutPicture
( 
  HDC           hDC,
  HWND          hWnd,
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData 
);
static void __far __pascal PictFormatted
( 
  FPCONTEXTDATA fpContextData,          /* Context data. */
  FPPOSRECT fpPictPos,                  /* Picture's page position. */
  BOOL bLeftPict                        /* Is picture left justified? */
);


static BOOL __far __pascal LayoutLeftBitMap
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
);
static BOOL __far __pascal LayoutCharBitmap
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement 
);
static BOOL __far __pascal LayoutRightBitMap
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
);
static BOOL __far __pascal DrawBitmap
( 
  HDC hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  HGLOBAL hBitmapRec, 
  FPPOSRECT PosRect
);


static BOOL __far __pascal LayoutLeftMetaFile
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
);
static BOOL __far __pascal LayoutRightMetaFile
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
);
static BOOL __far __pascal LayoutCharMetaFile
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement 
);
static BOOL __far __pascal DrawMetaFile
( 
  HDC hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  HGLOBAL hMetaRec, 
  FPPOSRECT PosRect
);


static BOOL __far __pascal SetFontObject
( 
  HDC hDC,
  HWND hWnd,
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
);
static void __far __pascal HotSpotBegin
( 
  HDC hDC, 
  FPCONTEXTDATA fpContextData, 
  FPTOPICELEMENT fpTopicElement
);
static void __far __pascal HotSpotEnd( HDC hDC, FPCONTEXTDATA fpContextData );
static void __far __pascal DrawHotspotUnderline
( 
  HDC hDC, 
  FPTXTDISPLAYREC fpDisplayRec,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo,
  HPEN hSolidPen, 
  HPEN hDashedPen 
);                                 


static void __far __pascal SetSpaceAfterParagraph( FPTOPICELEMENT fpTopicElement, FPCONTEXTDATA fpContextData );
static void __far __pascal SetSpaceBeforeParagraph( FPTOPICELEMENT fpTopicElement, FPCONTEXTDATA fpContextData );
static void __far __pascal SetVertLineSpace( FPTOPICELEMENT fpTopicElement, FPCONTEXTDATA fpContextData );
static void __far __pascal WrapToNextLine( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData );
static void __far __pascal JustifyCurrentLine( FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData );
static WORD __far __pascal BaselineAlign( FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData );
static void __far __pascal ParagraphEnd( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData );
static BOOL __far __pascal AtNewParagraph( FPCONTEXTDATA fpContextData );
static void __far __pascal GotoNextLine( HDC hDC, FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData );
static void __far __pascal InitializeContextData
( 
  HDC hDC, 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
);
static void __far __pascal ResetPageData
( 
  HDC hDC, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData 
);

static void __far __pascal CalcMargins( FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData );
static void __far __pascal CalcTableMargins
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
);
static void __far __pascal CalcBorderMargins( FPCONTEXTDATA fpContextData );
static void __far __pascal CalcPictMargins( FPCONTEXTDATA fpContextData );
static void __far __pascal CalcLeftTxtMargin( FPCONTEXTDATA fpContextData );
static void __far __pascal CalcRightTxtMargin( FPCONTEXTDATA fpContextData );

static void __far __pascal SetLeftIndent
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
);

static void __far __pascal SetFirstLineIndent
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
);

static void __far __pascal SetRightIndent
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
);

static void __far __pascal SetJustification
( 
  FPCONTEXTDATA fpContextData,
  JUSTIFY_TYPE JustifyType
);

static void __far __pascal SetWordWrappingOff
( 
  FPCONTEXTDATA fpContextData
);


static void __far __pascal SetCustomTabStops
( 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
);
static void __far __pascal InsertTab( FPCONTEXTDATA fpContextData );

static void __far __pascal SetBorder
( 
  HDC hDC,
  FPTOPICELEMENT fpTopicElement,
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData
);
static void __far __pascal SetAnotherBorder
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData
);
static BOOL __far __pascal LayoutBorder
( 
  HWND hWnd,
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
);
static BOOL __far __pascal DrawBorder
( 
  HDC  hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  FPBORDERDISPLAYREC fpBorderDisplayRec
);


static void __far __pascal ShowScrollBars( HWND hWnd, FPDISPLAYINFO fpDisplayInfo );
static void __far __pascal HideScrollBars( HWND hWnd, FPSCROLLDATA fpScrollData );
static void __far __pascal SetScrollBarValues( HWND hWnd, FPDISPLAYINFO fpDisplayInfo ) ;
static void __far __pascal CalcPagePosOffset( FPDISPLAYINFO fpDisplayInfo );


static void __far __pascal AddPagesHotSpots( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo );


static void __far __pascal TableBegin
( 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
);
static void __far __pascal TableEnd
( 
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData
);
static void __far __pascal TableColumn
( 
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData
);


static HGLOBAL __far __pascal AllocDisplayList( HWND hWnd, WORD wRecSize );
static BOOL __far __pascal ReAllocDisplayList( HWND hWnd, FPDISPLAYLIST fpDisplayList, 
                                               WORD wRecSize );
static void __far __pascal FreeDisplayList( HGLOBAL hDisplayList );
static void __far __pascal ClearDisplayList( HGLOBAL hDisplayList );
static BOOL __far __pascal InsertTxtDisplayListRec
( 
  HWND hWnd,
  FPDISPLAYLIST fpDisplayList,
  FPTXTDISPLAYREC fpTxtDispRecData 
);
static BOOL __far __pascal UpdateTxtDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPTXTDISPLAYREC fpTxtDispRecData 
);
static BOOL __far __pascal GetTxtDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPTXTDISPLAYREC fpTxtDispRecData 
);
static BOOL __far __pascal MoveTxtDisplayRecHortPos
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wBeginTxtDispRec,
  WORD wEndTxtDispRec,
  WORD wSlideAmount 
);
static void __far __pascal CopyTxtDisplayListRec
( 
  FPTXTDISPLAYREC fpDstRec, 
  FPTXTDISPLAYREC fpSrcRec 
);


static BOOL __far __pascal InsertBorderDisplayListRec
( 
  HWND hWnd,
  FPDISPLAYLIST fpDisplayList,
  FPBORDERDISPLAYREC fpBorderDispRecData 
);
static BOOL __far __pascal GetBorderDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList, 
  WORD wRecordNum,
  FPBORDERDISPLAYREC fpBorderDispRecData 
);
static void __far __pascal CopyBorderDisplayListRec
( 
  FPBORDERDISPLAYREC fpDstRec, 
  FPBORDERDISPLAYREC fpSrcRec 
);


static BOOL __far __pascal InsertPictDisplayListRec
( 
  HWND hWnd,
  FPDISPLAYLIST fpDisplayList,
  FPPICTDISPLAYREC fpPictDispRecData 
);
static BOOL __far __pascal UpdatePictDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPPICTDISPLAYREC fpPictDispRecData 
);
static BOOL __far __pascal GetPictDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPPICTDISPLAYREC fpPictDispRecData 
);
static void __far __pascal CopyPictDisplayListRec
( 
  FPPICTDISPLAYREC fpDstRec, 
  FPPICTDISPLAYREC fpSrcRec 
);


/********************************** DISPLAY SYSTEM ROUTINES ************************************/


/***************************************************
*
* Start window's display system. 
* 
* Allocate the display info. structure. 
*
****************************************************/
BOOL __far __pascal StartDisplaySystem( HWND hWnd, BOOL bNonScrollWnd )
{
  HGLOBAL hDisplayInfo;           /* Window's display info. */
  FPDISPLAYINFO fpDisplayInfo;    /* Pointer to window's display info. */


  /* Allocate memory for the topic buffer. */
  if( !GlobalAllocMem( hWnd, &hDisplayInfo, sizeof(DISPLAYINFO) ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Create the window's font buffer. */
  fpDisplayInfo->hFontBuffer = AllocFontBuffer( hWnd );
  if( ! fpDisplayInfo->hFontBuffer )
  {
    /* Free the display info. */
    ClearDisplaySystem( hWnd, fpDisplayInfo, bNonScrollWnd );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
  
    /* Failure. */
    return( FALSE );
  }

  /* Create the text display list. */
  fpDisplayInfo->hTxtDisplayList = AllocDisplayList( hWnd, sizeof(TXTDISPLAYREC) );
  if( ! fpDisplayInfo->hTxtDisplayList )
  {
    /* Free the display info. */
    ClearDisplaySystem( hWnd, fpDisplayInfo, bNonScrollWnd );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    /* Failure. */
    return( FALSE );
  }

  /* Create the border display list. */
  fpDisplayInfo->hBorderDisplayList = AllocDisplayList( hWnd, sizeof(BORDERDISPLAYREC) );
  if( ! fpDisplayInfo->hBorderDisplayList )
  {
    /* Free the display info. */
    ClearDisplaySystem( hWnd, fpDisplayInfo, bNonScrollWnd );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    /* Failure. */
    return( FALSE );
  }

  /* Create the justified pictures' display list. */
  fpDisplayInfo->hPictDisplayList = AllocDisplayList( hWnd, sizeof(PICTDISPLAYREC) );
  if( ! fpDisplayInfo->hPictDisplayList )
  {
    /* Free the display info. */
    ClearDisplaySystem( hWnd, fpDisplayInfo, bNonScrollWnd );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    /* Failure. */
    return( FALSE );
  }

  /* Create the hotspot buffer. */
  fpDisplayInfo->hHotSpotBuffer = AllocHotSpotBuffer( hWnd );
  if( ! fpDisplayInfo->hHotSpotBuffer )
  {
    /* Free the display info. */
    ClearDisplaySystem( hWnd, fpDisplayInfo, bNonScrollWnd );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    /* Failure. */
    return( FALSE );
  }

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
  
  /* Store handle of display info. */
  WndData( hWnd, WDM_SETDISPLAYINFO, (DWORD) hDisplayInfo, NULL );

  /* Success. */
  return( TRUE );
}



/***************************************************
*
* Stop window's display system. 
* 
* Free the display info. structure.
*
****************************************************/
void __far __pascal StopDisplaySystem( HWND hWnd, BOOL bNonScrollWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Free the display info. */
  ClearDisplaySystem( hWnd, fpDisplayInfo, bNonScrollWnd );

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );

  /* Free the display info. */
  GlobalFree( hDisplayInfo );

  /* Clear the handle to the display info. in the window data. */
  WndData( hWnd, WDM_SETDISPLAYINFO, 0L, NULL );
}      


/***************************************************
*
* Frees dynamically allocated information inside of
* a display info. structure.
*
****************************************************/
static void __far __pascal ClearDisplaySystem( HWND hWnd, FPDISPLAYINFO fpDisplayInfo, BOOL bNonScrollWnd )
{
  HWND hDataWnd;

  /* 
  ** Free all information in the structure. 
  */

  /* Destroy the font buffer. */
  FreeFontBuffer( hWnd, fpDisplayInfo->hFontBuffer );
  fpDisplayInfo->hFontBuffer = NULL;

  /* Destroy the text display list. */
  FreeDisplayList( fpDisplayInfo->hTxtDisplayList );
  fpDisplayInfo->hTxtDisplayList = NULL;

  /* Destroy the border display list. */
  FreeDisplayList( fpDisplayInfo->hBorderDisplayList );
  fpDisplayInfo->hBorderDisplayList = NULL;

  /* Destroy the justified pictures' display list. */
  FreeDisplayList( fpDisplayInfo->hPictDisplayList );
  fpDisplayInfo->hPictDisplayList = NULL;

  /* Free the hotspot buffer. */
  FreeHotSpotBuffer( fpDisplayInfo->hHotSpotBuffer );
  fpDisplayInfo->hHotSpotBuffer = NULL;

  /* Release the current topic. */
  if( fpDisplayInfo->hTopicData != NULL && !bNonScrollWnd )
  {
    /* Get the handle of the data window. */
    hDataWnd = (HWND) WndData( hWnd, WDM_GETDATAHWND, 0L, NULL );

    /* Release the lock on the current topic's data. */
    UnlockTopicData( hDataWnd, fpDisplayInfo->hTopicData );

    fpDisplayInfo->hTopicData = NULL;
  }
}


/***************************************************
*
* Are we already displaying the same
* help file?
*
* Returns:
*
*    TRUE - Help file is same.
*
*    FALSE - Help file is not the same.
*
****************************************************/
BOOL __far __pascal SameHelpFile( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  
  FPTOPICDATA  fpOldTopicsData;
  char __far * fpOldHelpFilesPath;

  FPTOPICDATA  fpNewTopicsData;
  char __far * fpNewHelpFilesPath;
  
  BOOL bSameFile;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Not same file. */
  bSameFile = FALSE;
  
  /* 
  ** If a topic is already being displayed. 
  */
  if( fpDisplayInfo->hTopicData != NULL )
  {
    /* Lock the window's current topic data. */
    fpOldTopicsData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );
    
    /* Lock the window's new topic data. */
    fpNewTopicsData = (FPTOPICDATA) GlobalLock( NewTopicStructPtr->hNewTopicsData );
    
    /* Get the path of the current topic's help file. */
    fpOldHelpFilesPath = fpOldTopicsData->szHelpFile;
    
    /* Get the path of the new topic's help file. */
    fpNewHelpFilesPath = fpNewTopicsData->szHelpFile;
    
    /* 
    ** See if the new and current topics use the same help file. 
    */
    if( _fstrcmp( fpOldHelpFilesPath, fpNewHelpFilesPath ) == 0 )
    {
      /* Same topic. */
      bSameFile = TRUE;
    }
        
    /* Unlock the window's new topic data. */
    GlobalUnlock( NewTopicStructPtr->hNewTopicsData );
    
    /* Unlock the window's current topic data. */
    GlobalUnlock( fpDisplayInfo->hTopicData );
  }

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );

  /* Same file? */
  return( bSameFile );
}


/***************************************************
*
* Are we already displaying the topic in the
* topic window?
*
* Returns:
*
*    TRUE - New topic not being displayed.
*
*    FALSE - Topic already being displayed
*            and new instance was unlocked.
*
****************************************************/
BOOL __far __pascal UnlockSameTopic( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr )
{
  HWND hDataWnd;

  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  
  FPTOPICDATA  fpOldTopicsData;
  char __far * fpOldHelpFilesPath;

  FPTOPICDATA  fpNewTopicsData;
  char __far * fpNewHelpFilesPath;
  
  BOOL bSameTopic;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Not same topic. */
  bSameTopic = FALSE;
  
  /* 
  ** If a topic is already being displayed. 
  */
  if( fpDisplayInfo->hTopicData != NULL )
  {
    /* Lock the window's current topic data. */
    fpOldTopicsData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );
    
    /* Lock the window's new topic data. */
    fpNewTopicsData = (FPTOPICDATA) GlobalLock( NewTopicStructPtr->hNewTopicsData );
    
    /* Get the path of the current topic's help file. */
    fpOldHelpFilesPath = fpOldTopicsData->szHelpFile;
    
    /* Get the path of the new topic's help file. */
    fpNewHelpFilesPath = fpNewTopicsData->szHelpFile;
    
    /* 
    ** See if the new and current topics use the same help file. 
    */
    if( _fstrcmp( fpOldHelpFilesPath, fpNewHelpFilesPath ) == 0 )
    {
      /* Stop if we are already displaying the same topic. */
      if( fpOldTopicsData->dwTopicCharOffset == fpNewTopicsData->dwTopicCharOffset )
      {
        /* Same topic. */
        bSameTopic = TRUE;
      }
    }
        
    /* Unlock the window's new topic data. */
    GlobalUnlock( NewTopicStructPtr->hNewTopicsData );
    
    /* Unlock the window's current topic data. */
    GlobalUnlock( fpDisplayInfo->hTopicData );
  }

  /* If same topic then unlock the topic data. */
  if( bSameTopic == TRUE )
  {
    /* Get the handle of the data window. */
    hDataWnd = (HWND) WndData( hWnd, WDM_GETDATAHWND, 0L, NULL );
        
    /* Release the buffer lock on the current topic's data. */
    UnlockTopicData( hDataWnd, NewTopicStructPtr->hNewTopicsData );
  }
  
  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );

  /* New topic used. */
  return( bSameTopic );
}


/***************************************************
*
* Informs display system of the color being used
* to erase the topic window's background.
*
****************************************************/
void __far __pascal SetDisplayBGColor( HWND hWnd, COLORREF ColorRef )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );

  /* Set background color. */
  fpDisplayInfo->BGColor = ColorRef; 
  
  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
}
  

/***************************************************
*
* Gets the color being used to erase the topic 
* window's background.
*
****************************************************/
COLORREF __far __pascal GetDisplayBGColor( HWND hWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  COLORREF ColorRef;
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );

  /* Set background color. */
  ColorRef = fpDisplayInfo->BGColor; 
    
  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );

  /* Return color. */
  return( ColorRef );
}

  

/***************************************************
*
* Sets topic data that is associated with a window.
*
* If a topic is already associated with the window,
* it is unlocked.
*
* If the old topic and new topic are from different
* help files, then the fontbuffer is cleared.
*
* Window's size data is reset.
*
****************************************************/
void __far __pascal SetNewDisplayTopic( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr )
{
  HWND hDataWnd;

  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  
  FPTOPICDATA  fpOldTopicsData;
  char __far * fpOldHelpFilesPath;

  FPTOPICDATA  fpNewTopicsData;
  char __far * fpNewHelpFilesPath;
  

  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* 
  ** If a topic is already being displayed. 
  */
  if( fpDisplayInfo->hTopicData != NULL )
  {
    /* Lock the window's current topic data. */
    fpOldTopicsData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );
    
    /* Lock the window's new topic data. */
    fpNewTopicsData = (FPTOPICDATA) GlobalLock( NewTopicStructPtr->hNewTopicsData );
    
    /* Get the path of the current topic's help file. */
    fpOldHelpFilesPath = fpOldTopicsData->szHelpFile;
    
    /* Get the path of the new topic's help file. */
    fpNewHelpFilesPath = fpNewTopicsData->szHelpFile;
    
    /* If we are not displaying the same help file. */
    if( _fstrcmp( fpOldHelpFilesPath, fpNewHelpFilesPath ) != 0 )
    {
      /* Clear the window's font buffer. */
      ClearFontBuffer( hWnd, fpDisplayInfo->hFontBuffer );
    } 
  
    /* Unlock the window's current topic data. */
    GlobalUnlock( fpDisplayInfo->hTopicData );
  
    /* Unlock the window's new topic data. */
    GlobalUnlock( NewTopicStructPtr->hNewTopicsData );

    /* Get the handle of the data window. */
    hDataWnd = (HWND) WndData( hWnd, WDM_GETDATAHWND, 0L, NULL );
  
    /* Release the buffer lock on the current topic's data. */
    UnlockTopicData( hDataWnd, fpDisplayInfo->hTopicData );
  }

  /* 
  ** Set initial dimensions of our display 
  ** area to bogus values so that we will
  ** think on the next DisplayTopic() call
  ** that the window size has been changed.
  */
  fpDisplayInfo->DisplaySize.x = 0;
  fpDisplayInfo->DisplaySize.y = 0;
  
  /* Save handle to the new topic's data. */
  fpDisplayInfo->hTopicData = NewTopicStructPtr->hNewTopicsData;

  /* Initially no top list. */
  fpDisplayInfo->wTopDisplayList = NO_DISPLAY_LIST;

  /* Initially, force the use of the current vertical scroll bar position. */
  fpDisplayInfo->bUseVertScrollPos = TRUE;
  fpDisplayInfo->ScrollData.wVertPos = NewTopicStructPtr->wVertPos;
  
  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
}


/***************************************************
*
* Sets topic data that is assoicated with a 
* non-scrollable window.
*
* Since a non-scrollable window shares its topic
* data with a scrollable window, it does not have to
* unlock the topic data.  The topic data will be unlocked
* by the scrollable window.
*
* Size of the window's size is also cleared.
*
****************************************************/
void __far __pascal SetNewDisplayTopic2( HWND hWnd, FPNEWTOPICSTRUCT NewTopicStructPtr )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  

  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* 
  ** Set initial dimensions of our display 
  ** area to bogus values so that we will
  ** think on the next DisplayTopic() call
  ** that the window size has been changed.
  */
  fpDisplayInfo->DisplaySize.x = 0;
  fpDisplayInfo->DisplaySize.y = 0;
  
  /* Clear the window's font buffer. */
  ClearFontBuffer( hWnd, fpDisplayInfo->hFontBuffer );

  /* Save handle to the new topic's data. */
  fpDisplayInfo->hTopicData = NewTopicStructPtr->hNewTopicsData;

  /* 
  ** Initially at top of page. 
  */
  fpDisplayInfo->wTopDisplayList = NO_DISPLAY_LIST;

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
}



/***************************************************
*
* Size and position a popup window.
*
****************************************************/
BOOL __far __pascal SizePopupWindow( HWND hWnd, RECT __far * HotSpotWndRect )
{
  FPTOPICDATA fpTopicData;
  BOOL UseNonScrollable;

  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  BOOL bReturn;

  RECT ClientRect;
  
  PAINTSTRUCT PaintStruct;
  HDC hDC;
  
  short int cxScreen,                      /* Size of screen in DP. */
            cyScreen;

  short int WindowWidth,                   /* Popup window sizes in DP. */
            WindowHeight, 
            ClientWidth, 
            ClientHeight, 
            WindowXPos, 
            WindowYPos;

  short int HotSpotXCenter;
  
  POINT Point;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Nothing to display. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    return( TRUE );
  }

  /* Get screen dimensions. */
  cxScreen = GetSystemMetrics( SM_CXSCREEN );
  cyScreen = GetSystemMetrics( SM_CYSCREEN );

  hDC = GetDC( hWnd ); 

  /* Update the stored window size values. */
  GetClientRect( hWnd, &ClientRect );
  DPtoLP( hDC, (LPPOINT) &ClientRect, 2 );
  fpDisplayInfo->DisplaySize.x = ClientRect.right;
  fpDisplayInfo->DisplaySize.y = ClientRect.bottom;
      
  /* Clear the text and picture display lists. */  
  ClearDisplayList( fpDisplayInfo->hTxtDisplayList );
  ClearDisplayList( fpDisplayInfo->hPictDisplayList );
  ClearDisplayList( fpDisplayInfo->hBorderDisplayList );
  
  /* Lock the different display layout lists. */
  fpDisplayInfo->fpTxtDisplayList = GlobalLock( fpDisplayInfo->hTxtDisplayList );
  fpDisplayInfo->fpBorderDisplayList = GlobalLock( fpDisplayInfo->hBorderDisplayList );
  fpDisplayInfo->fpPictDisplayList = GlobalLock( fpDisplayInfo->hPictDisplayList );

  /* Clear the hotspot pages information. */
  ClearPagesHotSpots( fpDisplayInfo->hHotSpotBuffer );

  /* Lock the topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );  

  /* 
  ** If there is non-scrollable part of the topic,
  ** layout that part of the page. 
  */
  if( fpTopicData->hNonScrollElements != NULL )
  {
    UseNonScrollable = TRUE;

  }
  else
  {
    UseNonScrollable = FALSE;
  }
 
  /* Unlock the topic data. */
  GlobalUnlock( fpDisplayInfo->hTopicData );  

  /* Fill the display list with display data. */
  bReturn = LayoutPage( hDC, hWnd, fpDisplayInfo, UseNonScrollable );
        
  /* Fill error? */
  if( !bReturn )
  {
    /* Start painting. */
    EndPaint( hWnd, &PaintStruct ); 

    /* Unlock the the different display layout lists. */
    GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
    GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
    GlobalUnlock( fpDisplayInfo->hPictDisplayList );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
          
    /* Failure. */
    return( FALSE );
  }

  /* Get the size of the formatted page. */
  GetPageSize( hWnd, fpDisplayInfo );

  /* Add the pages hotspots to the hotspot list. */
  AddPagesHotSpots( hDC, hWnd, fpDisplayInfo );      

  /* Add default right margain to the page width. */
  fpDisplayInfo->dwPageWidth += DEFAULT_RIGHT_MARGIN;
  
  /* Add space to page's height. */
  fpDisplayInfo->dwPageHeight += 120;

  /* 
  ** Convert the page size to DP - it should not be larger 
  ** than a short int since we used the screen size for
  ** the formatting.
  */
  Point.x = (short int) fpDisplayInfo->dwPageWidth;
  Point.y = (short int) fpDisplayInfo->dwPageHeight;
  LPtoDP( hDC, &Point, 1 );

  /* Client size of window equal to page size. */
  ClientWidth  = Point.x; 
  ClientHeight = Point.y; 

  /* 
  ** Make the window's client area big enough
  ** to hold the page info.
  */
  while( TRUE )
  {
    /* Convert new client size back to LP. */
    Point.x = (short int) ClientWidth;
    Point.y = (short int) ClientHeight;
    DPtoLP( hDC, &Point, 1 );

    /* If size is to small when converted back to LP. */
    if( Point.x < (short int) fpDisplayInfo->dwPageWidth || 
        Point.y < (short int) fpDisplayInfo->dwPageHeight )
    {
      if( Point.x < (short int) fpDisplayInfo->dwPageWidth )
      {
        ClientWidth += 1;
      }
      
      if( Point.y < (short int) fpDisplayInfo->dwPageHeight )
      {
        ClientHeight += 1;
      }
    }
    
    /* Size will work. */
    else break;
  }

  /* 
  ** Add window dressing pieces to get the window size.
  */
  WindowWidth  = ClientWidth + ( GetSystemMetrics( SM_CXDLGFRAME ) * 2 ); 
  WindowHeight = ClientHeight + ( GetSystemMetrics( SM_CYDLGFRAME ) * 2 ); 
  
  /* Window will be wider than the screen. */
  if( cxScreen < WindowWidth )
  {
    /* Make window as wide as screen. */
    WindowWidth = cxScreen;
  }

  /* Window will be taller than the screen. */
  if( cyScreen < WindowHeight )
  {
    /* Make window as tall as screen. */
    WindowHeight = cyScreen;
  }

  /* Convert client sizes to LP. */
  Point.x = ClientWidth;
  Point.y = ClientHeight;
  DPtoLP( hDC, &Point, 1 );

  /* Save the window's client sizes. */
  fpDisplayInfo->DisplaySize.x = Point.x;
  fpDisplayInfo->DisplaySize.y = Point.y;

  /* Position the window vertically under the hotspot? */
  if( WindowHeight <= ( cyScreen - (HotSpotWndRect->bottom + 1) ) )
  {
    WindowYPos = HotSpotWndRect->bottom + 1;
  }

  /* Position the window vertically above the hotspot? */
  else if( WindowHeight <= HotSpotWndRect->top - 1 )
  {
    WindowYPos = (HotSpotWndRect->top - 1) - WindowHeight;
  }

  /* Center window on screen vertically. */
  else
  {
    WindowYPos = ( cyScreen - WindowHeight ) / 2;
  }
  
  /* 
  ** Center window horizontally to hotspot. 
  ** Adjust position so that all of the window
  ** is on the screen. 
  */
  HotSpotXCenter = HotSpotWndRect->left + ( ( HotSpotWndRect->right - HotSpotWndRect->left ) / 2 );
  WindowXPos = HotSpotXCenter - ( WindowWidth / 2 );

  /* Left edge of window off the screen. */
  if( WindowXPos < 0 ) 
  {
    WindowXPos = 0;
  }
  /* Right edge of window off the screen. */
  if( WindowXPos + WindowWidth > cxScreen )
  {
    WindowXPos = WindowXPos - ( ( WindowXPos + WindowWidth ) - cxScreen );
  }
  
  /* Set window width, height, x, y position. */
  SetWindowPos( hWnd, NULL, WindowXPos, WindowYPos, WindowWidth, WindowHeight, 
                SWP_NOACTIVATE | SWP_NOZORDER );
  
  ReleaseDC( hWnd, hDC ); 

  /* Unlock the the different display layout lists. */
  GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
  GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
  GlobalUnlock( fpDisplayInfo->hPictDisplayList );

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
 
  /* Success. */
  return( TRUE );  
}


/***************************************************
*
* Show the window's current topic on the screen.
*
****************************************************/
BOOL __far __pascal DisplayPopupTopic( HWND hWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  BOOL bReturn;

  PAINTSTRUCT PaintStruct;
  HDC hDC;

  FPTOPICDATA fpTopicData;
  BOOL UseNonScrollable;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Nothing to display. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Get rid of the WM_PAINT message. */
    ValidateRect( hWnd, NULL ); 
  
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    return( TRUE );
  }

  /* Start painting. */
  hDC = BeginPaint( hWnd, &PaintStruct ); 
  
  /* Lock the topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );  

  /* 
  ** If there is non-scrollable part of the topic,
  ** draw that part of the page. 
  */
  if( fpTopicData->hNonScrollElements != NULL )
  {
    UseNonScrollable = TRUE;

  }
  else
  {
    UseNonScrollable = FALSE;
  }
 
  /* Unlock the topic data. */
  GlobalUnlock( fpDisplayInfo->hTopicData );  

  /* Lock the different display layout lists. */
  fpDisplayInfo->fpTxtDisplayList = GlobalLock( fpDisplayInfo->hTxtDisplayList );
  fpDisplayInfo->fpBorderDisplayList = GlobalLock( fpDisplayInfo->hBorderDisplayList );
  fpDisplayInfo->fpPictDisplayList = GlobalLock( fpDisplayInfo->hPictDisplayList );

  /* Display the a part of the topic. */
  bReturn = DrawPage( hDC, hWnd, fpDisplayInfo, UseNonScrollable );
  
  /* Unlock the the different display layout lists. */
  GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
  GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
  GlobalUnlock( fpDisplayInfo->hPictDisplayList );

  /* Stop painting. */
  EndPaint( hWnd, &PaintStruct ); 

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
 
  /* Return Success or Failure. */
  return( bReturn );  
}


/***************************************************
*
* Size and position a non-scrollable topic window.
*
****************************************************/
BOOL __far __pascal SizeNonScrollWindow( HWND hWnd, RECT __far * fpMaxSizeRect )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  BOOL bReturn;

  RECT WindowSizeRect,
       ClientRect;
  
  PAINTSTRUCT PaintStruct;
  HDC hDC;
  
  short int WindowHeight, 
            ClientHeight;

  POINT Point;
  

  /* Get the maximum position/size data. */
  WindowSizeRect = *fpMaxSizeRect; 

  /* Use all of space that we are told about. */
  MoveWindow( hWnd, 0, WindowSizeRect.top, 
              WindowSizeRect.right, 
              WindowSizeRect.bottom, TRUE ); 

  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Nothing to display. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    return( TRUE );
  }

  hDC = GetDC( hWnd ); 

  /* Update the stored window size values. */
  GetClientRect( hWnd, &ClientRect );
  DPtoLP( hDC, (LPPOINT) &ClientRect, 2 );
  fpDisplayInfo->DisplaySize.x = ClientRect.right;
  fpDisplayInfo->DisplaySize.y = ClientRect.bottom;
      
  /* Clear the text and picture display lists. */  
  ClearDisplayList( fpDisplayInfo->hTxtDisplayList );
  ClearDisplayList( fpDisplayInfo->hPictDisplayList );
  ClearDisplayList( fpDisplayInfo->hBorderDisplayList );
  
  /* Lock the different display layout lists. */
  fpDisplayInfo->fpTxtDisplayList = GlobalLock( fpDisplayInfo->hTxtDisplayList );
  fpDisplayInfo->fpBorderDisplayList = GlobalLock( fpDisplayInfo->hBorderDisplayList );
  fpDisplayInfo->fpPictDisplayList = GlobalLock( fpDisplayInfo->hPictDisplayList );
  
  /* Clear the hotspot pages information. */
  ClearPagesHotSpots( fpDisplayInfo->hHotSpotBuffer );

  /* Fill the display list with display data. */
  bReturn = LayoutPage( hDC, hWnd, fpDisplayInfo, TRUE );
        
  /* Get the size of the formatted page. */
  GetPageSize( hWnd, fpDisplayInfo );

  /* Fill error? */
  if( !bReturn )
  {
    /* Start painting. */
    EndPaint( hWnd, &PaintStruct ); 

    /* Unlock the the different display layout lists. */
    GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
    GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
    GlobalUnlock( fpDisplayInfo->hPictDisplayList );

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
          
    /* Failure. */
    return( FALSE );
  }

  /* Add the pages hotspots to the hotspot list. */
  AddPagesHotSpots( hDC, hWnd, fpDisplayInfo );      

  /* 
  ** Convert the page size to DP - it should not be larger 
  ** than a short int since we used the screen size for
  ** the formatting.
  */
  Point.x = 0;
  Point.y = (short int) fpDisplayInfo->dwPageHeight;
  LPtoDP( hDC, &Point, 1 );

  /* Client size of window equal to page size. */
  ClientHeight = Point.y; 

  /* 
  ** Make the window's client area big enough
  ** to hold the page info.
  */
  while( TRUE )
  {
    /* Convert new client size back to LP. */
    Point.x = 0;
    Point.y = (short int) ClientHeight;
    DPtoLP( hDC, &Point, 1 );

    /* If size is to small when converted back to LP. */
    if( Point.y < (short int) fpDisplayInfo->dwPageHeight )
    {
      ClientHeight += 1;
    }
    
    /* Size height will work. */
    else break;
  }

  ReleaseDC( hWnd, hDC ); 

  /* Add an extra pixel to height for separator line. */
  WindowHeight = ClientHeight + 1; 
  
  /* If the calc. window height is less than the current height. */
  if( ClientHeight < WindowSizeRect.bottom ) 
  {
    /* Make window as high as we need it. */
    WindowSizeRect.bottom = WindowHeight;
  }

  /* Use the space that we need. */
  MoveWindow( hWnd, 0, WindowSizeRect.top, WindowSizeRect.right, 
              WindowSizeRect.bottom, TRUE ); 

  /* Save client position for non-scrolling window. */
  WndData( hWnd, WDM_SETCLIENTPOS, 0L, &WindowSizeRect );

  /* Unlock the the different display layout lists. */
  GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
  GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
  GlobalUnlock( fpDisplayInfo->hPictDisplayList );

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
 
  /* Success. */
  return( TRUE );  
}


/***************************************************
*
* Show the window's current topic on the screen.
*
****************************************************/
BOOL __far __pascal DisplayNonScrollTopic( HWND hWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  BOOL bReturn;

  PAINTSTRUCT PaintStruct;
  HDC hDC;

  RECT ClientRect;
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Nothing to display. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Get rid of the WM_PAINT message. */
    ValidateRect( hWnd, NULL ); 
  
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    return( TRUE );
  }

  /* Start painting. */
  hDC = BeginPaint( hWnd, &PaintStruct ); 
  
  /* Lock the different display layout lists. */
  fpDisplayInfo->fpTxtDisplayList = GlobalLock( fpDisplayInfo->hTxtDisplayList );
  fpDisplayInfo->fpBorderDisplayList = GlobalLock( fpDisplayInfo->hBorderDisplayList );
  fpDisplayInfo->fpPictDisplayList = GlobalLock( fpDisplayInfo->hPictDisplayList );
  
  /* Display the a part of the topic. */
  bReturn = DrawPage( hDC, hWnd, fpDisplayInfo, TRUE );

  /* Unlock the the different display layout lists. */
  GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
  GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
  GlobalUnlock( fpDisplayInfo->hPictDisplayList );

  /* Get client size and convert to LP. */
  GetClientRect( hWnd, &ClientRect );
  ClientRect.bottom = ClientRect.bottom - 1;
  DPtoLP( hDC, (POINT __far *) &ClientRect, 2 );

  /* Draw the separator line. */
  MoveTo( hDC, 0, ClientRect.bottom ); 
  LineTo( hDC, ClientRect.right, ClientRect.bottom ); 
  
  /* Stop painting. */
  EndPaint( hWnd, &PaintStruct ); 

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
 
  /* Return Success or Failure. */
  return( bReturn );  
}


/***************************************************
*
* Show the scrollable window's current topic on the screen.
*
****************************************************/
BOOL __far __pascal DisplayTopic( HWND hWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  BOOL bReturn;

  RECT ClientRect;
  
  PAINTSTRUCT PaintStruct;
  HDC hDC;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Nothing to display. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Get rid of the WM_PAINT message. */
    ValidateRect( hWnd, NULL ); 

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    return( TRUE );
  }

  /* Start painting. */
  hDC = BeginPaint( hWnd, &PaintStruct ); 

  /* 
  ** Check to see if the window's size has changed since
  ** we last formatted the display data.
  */
  
  GetClientRect( hWnd, &ClientRect );
  DPtoLP( hDC, (LPPOINT) &ClientRect, 2 );
    
  if( fpDisplayInfo->DisplaySize.x != ClientRect.right ||
      fpDisplayInfo->DisplaySize.y != ClientRect.bottom  )
  {
    /* Clear the hotspot pages information. */
    ClearPagesHotSpots( fpDisplayInfo->hHotSpotBuffer );

    /* Hide the window's scroll bars. */
    HideScrollBars( hWnd, &(fpDisplayInfo)->ScrollData );
      
    while( fpDisplayInfo->DisplaySize.x != ClientRect.right ||
           fpDisplayInfo->DisplaySize.y != ClientRect.bottom  )
    {
      /* Update the stored window size values. */
      fpDisplayInfo->DisplaySize.x = ClientRect.right;
      fpDisplayInfo->DisplaySize.y = ClientRect.bottom;
        
      /* Clear the text and picture display lists. */  
      ClearDisplayList( fpDisplayInfo->hTxtDisplayList );
      ClearDisplayList( fpDisplayInfo->hPictDisplayList );
      ClearDisplayList( fpDisplayInfo->hBorderDisplayList );
    
      /* Lock the different display layout lists. */
      fpDisplayInfo->fpTxtDisplayList = GlobalLock( fpDisplayInfo->hTxtDisplayList );
      fpDisplayInfo->fpBorderDisplayList = GlobalLock( fpDisplayInfo->hBorderDisplayList );
      fpDisplayInfo->fpPictDisplayList = GlobalLock( fpDisplayInfo->hPictDisplayList );
  
      /* Fill the display list with display data. */
      bReturn = LayoutPage( hDC, hWnd, fpDisplayInfo, FALSE );
          
      /* Fill error? */
      if( !bReturn )
      {
        /* Start painting. */
        EndPaint( hWnd, &PaintStruct ); 
  
        /* Unlock the display info. */
        GlobalUnlock( hDisplayInfo );
            
        /* Failure. */
        return( FALSE );
      }
    
      /* Get the size of the formatted page. */
      GetPageSize( hWnd, fpDisplayInfo );

      /* Add scroll bars. */
      ShowScrollBars( hWnd, fpDisplayInfo );
    
      /* 
      ** Did window size change due to adding scroll bars and we 
      ** need to reformat display data? 
      */
      GetClientRect( hWnd, &ClientRect );
      DPtoLP( hDC, (LPPOINT) &ClientRect, 2 );
    }
  
    /* 
    ** Now that we have formatted the page and have set
    ** the appropriate scroll bars.
    */
    SetScrollBarValues( hWnd, fpDisplayInfo );
      
    /* Add the pages hotspots to the hotspot list. */
    AddPagesHotSpots( hDC, hWnd, fpDisplayInfo );      

    /* Unlock the the different display layout lists. */
    GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
    GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
    GlobalUnlock( fpDisplayInfo->hPictDisplayList );

  }
  
  /* Lock the different display layout lists. */
  fpDisplayInfo->fpTxtDisplayList = GlobalLock( fpDisplayInfo->hTxtDisplayList );
  fpDisplayInfo->fpBorderDisplayList = GlobalLock( fpDisplayInfo->hBorderDisplayList );
  fpDisplayInfo->fpPictDisplayList = GlobalLock( fpDisplayInfo->hPictDisplayList );
  
  /* Display the topic data. */
  bReturn = DrawPage( hDC, hWnd, fpDisplayInfo, FALSE );
  
  /* Unlock the the different display layout lists. */
  GlobalUnlock( fpDisplayInfo->hTxtDisplayList );
  GlobalUnlock( fpDisplayInfo->hBorderDisplayList );
  GlobalUnlock( fpDisplayInfo->hPictDisplayList );

  /* Stop painting. */
  EndPaint( hWnd, &PaintStruct ); 

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
 
  /* Return Success or Failure. */
  return( bReturn );  
}


/***************************************************
*
* Does the topic assigned to the window have 
* non-scrollable data?
*
****************************************************/
BOOL __far __pascal HasNonScrollData( HWND hWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;

  FPTOPICDATA fpTopicData;

  BOOL bReturn;
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Nothing to display. */
  if( fpDisplayInfo->hTopicData == NULL ) 
  {
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    return( FALSE );
  }

  /* Lock the topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );  

  /* 
  ** If there is non-scrollable part of the topic,
  ** draw that part of the page. 
  */
  if( fpTopicData->hNonScrollElements != NULL )
  {
    bReturn = TRUE;

  }
  else
  {
    bReturn = FALSE;
  }
 
  /* Unlock the topic data. */
  GlobalUnlock( fpDisplayInfo->hTopicData );  

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
 
  /* Return yes or no. */
  return( bReturn );  
}



/********************************** PAGE ROUTINES ************************************/

/***************************************************
*
* Fills text and bitmap display lists with display 
* information for each of the topic's elements.
*
****************************************************/
static BOOL __far __pascal LayoutPage( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, BOOL UseNonScrollable )
{
  FPTOPICDATA fpTopicData;

  TOPICELEMENT ElementData;
  WORD wCurrentElement;
  
  CONTEXTDATA ContextData;       /* Context data used while drawing. */
  
  HGLOBAL       hElementList;
  FPELEMENTLIST fpElementList;
  
  
  /* Initialize context data. */
  InitializeContextData( hDC, fpDisplayInfo, &ContextData );

  /* Lock the topic's topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );  

  /* Use the desired element list for the topic. */
  if( UseNonScrollable )
  {
    hElementList = fpTopicData->hNonScrollElements;
  }
  else
  {
    hElementList = fpTopicData->hScrollElements;
  }

  /* Lock the topic's element list. */
  fpElementList = (FPELEMENTLIST) GlobalLock( hElementList );  

  /* At the first element record - uses starting index of 0. */
  wCurrentElement = 0;

  /* For each topic element. */
  while( RetrieveElementListRec( fpElementList, &ElementData, wCurrentElement ) )
  {
    /* Process the current element record. */
    switch( ElementData.wRecType )
    {
      /* Starting output to a table. */
      case TE_TABLEBEGIN:
      {
        TableBegin( &ElementData, &ContextData );
        break;
      }


      /* Ending output to a table. */
      case TE_TABLEEND:
      {
        TableEnd( fpDisplayInfo, &ContextData );
        break;
      }


      /* Ending output to a table. */
      case TE_TABLECOLUMN:
      {
        TableColumn( fpDisplayInfo, &ContextData );
        break;
      }


      /* Text element. */
      case TE_TEXT:
      {
        /* Set a border if we should have one. */
        if( ContextData.bShouldHaveBorder && !ContextData.bBorder )
        {
          SetAnotherBorder( fpDisplayInfo, &ContextData );
        }

        if( ! LayoutText( hDC, hWnd, &ContextData, fpDisplayInfo, &ElementData ) )
        {
          /* Unlock the topic's element list. */
          GlobalUnlock( hElementList );  

          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
  
          /* Failure. */
          return( FALSE );
        }  
        
        break;
      }


      /* Character bitmap. */
      case TE_CHARBITMAP:
      {
        /* Set a border if we should have one. */
        if( ContextData.bShouldHaveBorder && !ContextData.bBorder )
        {
          SetAnotherBorder( fpDisplayInfo, &ContextData );
        }

        if( ! LayoutCharBitmap( hDC, hWnd, &ContextData, fpDisplayInfo, &ElementData ) )
        {
          /* Unlock the topic's element list. */
          GlobalUnlock( hElementList );  

          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
  
          /* Failure. */
          return( FALSE );
        }  

        break;
      }


      /* Character metafile. */
      case TE_CHARMETA:
      {
        /* Set a border if we should have one. */
        if( ContextData.bShouldHaveBorder && !ContextData.bBorder )
        {
          SetAnotherBorder( fpDisplayInfo, &ContextData );
        }

        if( ! LayoutCharMetaFile( hDC, hWnd, &ContextData, fpDisplayInfo, &ElementData ) )
        {
          /* Unlock the topic's element list. */
          GlobalUnlock( hElementList );  

          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
  
          /* Failure. */
          return( FALSE );
        }  

        break;
      }


      /* Left justified picture. */
      case TE_LEFTMETA:
      case TE_LEFTBITMAP:
      {
        if( ! InsertPicture( hDC, hWnd, &ContextData, fpDisplayInfo, &ElementData ) )
        {
          /* Unlock the topic's element list. */
          GlobalUnlock( hElementList );  

          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
    
          /* Failure. */
          return( FALSE );
        }                                                


        /* 
        ** If we are at the start of a paragraph and there's no right
        ** justified picture, position the left justified picture at 
        ** the start of the current line. 
        */
        if( AtNewParagraph( &ContextData ) && !(ContextData.bRightPicture) )
        {
          /* Set a border if we should have one. */
          if( ContextData.bShouldHaveBorder && !ContextData.bBorder )
          {
            SetAnotherBorder( fpDisplayInfo, &ContextData );
          }

          if( ! LayoutPicture( hDC, hWnd, fpDisplayInfo, &ContextData ) )
          {
            /* Unlock the topic's element list. */
            GlobalUnlock( hElementList );  

            /* Unlock the topic's data. */
            GlobalUnlock( fpDisplayInfo->hTopicData );
    
            /* Failure. */
            return( FALSE );
          }                                                
        
          /* Calculate the new line's left margin based on picture insertion. */
          CalcMargins( fpDisplayInfo, &ContextData );
        
          /* Set current horizontal position to new left margin. */
          ContextData.PageXPos = ContextData.LeftTxtMargin;             
        }

        break;
      }


      /* Right justified picture. */
      case TE_RIGHTMETA:
      case TE_RIGHTBITMAP:
      {
        if( ! InsertPicture( hDC, hWnd, &ContextData, fpDisplayInfo, &ElementData ) )
        {
          /* Unlock the topic's element list. */
          GlobalUnlock( hElementList );  

          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
    
          /* Failure. */
          return( FALSE );
        }                                                


        /* 
        ** If we are at the start of a paragraph, position the 
        ** picture at the end of the current line. 
        */
        if( AtNewParagraph( &ContextData ) && !(ContextData.bLeftPicture) )
        {
          /* Set a border if we should have one. */
          if( ContextData.bShouldHaveBorder && !ContextData.bBorder )
          {
            SetAnotherBorder( fpDisplayInfo, &ContextData );
          }

          if( ! LayoutPicture( hDC, hWnd, fpDisplayInfo, &ContextData ) )
          {
            /* Unlock the topic's element list. */
            GlobalUnlock( hElementList );  

            /* Unlock the topic's data. */
            GlobalUnlock( fpDisplayInfo->hTopicData );
    
            /* Failure. */
            return( FALSE );
          }                                                
        
          /* Calculate the new line's margins. */
          CalcMargins( fpDisplayInfo, &ContextData );
        }

        break;
      }


      /* Font element. */
      case TE_FONT:
      {
        if( ! SetFontObject( hDC, hWnd, fpDisplayInfo, &ElementData, &ContextData ) )
        {
          /* Unlock the topic's element list. */
          GlobalUnlock( hElementList );  

          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
  
          /* Failure. */
          return( FALSE );
        }  
        break;
      }


      /* End of the paragraph. */
      case TE_PAR:
      {
        /* Goto to a new line. */
        ParagraphEnd( hDC, hWnd, fpDisplayInfo, &ContextData );
        break;
      }
      

      /* Go to next tab stop. */
      case TE_TAB:
      {
        InsertTab( &ContextData );
        break;
      }


      /* Set custom tab stop positions. */
      case TE_TX:
      {
        SetCustomTabStops( &ElementData, &ContextData );
        break;
      }


      /* Reset paragraph formatting information to default. */
      case TE_PARD:
      {
        ResetPageData( hDC, fpDisplayInfo, &ContextData );
        break;
      }


      /* Vertical line spacing within a paragraph. */
      case TE_SL:
      {
        SetVertLineSpace( &ElementData, &ContextData );
        break;
      }


      /* Vertical line spacing after a paragraph. */
      case TE_SA:
      {
        SetSpaceAfterParagraph( &ElementData, &ContextData );
        break;
      }


      /* Vertical line spacing before a paragraph. */
      case TE_SB:
      {
        SetSpaceBeforeParagraph( &ElementData, &ContextData );
        break;
      }


      /* Left indention. */
      case TE_LI:
      {
        SetLeftIndent( fpDisplayInfo, &ElementData, &ContextData );
        break;
      }


      /* Right indention. */
      case TE_RI:
      {
        SetRightIndent( fpDisplayInfo, &ElementData, &ContextData );
        break;
      }


      /* First line indention. */
      case TE_FI:
      {
        SetFirstLineIndent( fpDisplayInfo, &ElementData, &ContextData );
        break;
      }

      /* Right justified. */
      case TE_QR:
      {
        SetJustification( &ContextData, RIGHT_JUSTIFIED );
        break;
      }

      /* Center justified. */
      case TE_QC:
      {
        SetJustification( &ContextData, CENTER_JUSTIFIED );
        break;
      }

      /* Turn word wrapping off. */
      case TE_KEEP:
      {
        SetWordWrappingOff( &ContextData );
        break;
      }


      /* Paragraph has border lines. */
      case TE_BORDER:
      {
        SetBorder( hDC, &ElementData, fpDisplayInfo, &ContextData );
        break;
      }

      /* Start of hotspot. */
      case TE_HOTBEGIN:
      {
        HotSpotBegin( hDC, &ContextData, &ElementData );
        break;
      }

      /* End of hotspot. */
      case TE_HOTEND:
      {
        /* Set color and flags. */
        HotSpotEnd( hDC, &ContextData );
        break;
      }
    
      /* New line, but not end of paragraph. */
      case TE_LINE:
      {
        WrapToNextLine( hDC, hWnd, fpDisplayInfo, &ContextData );
        break;
      }
    }
    
    /* Goto next topic element. */
    wCurrentElement++;
  }
  
  /* Unlock the topic's element list. */
  GlobalUnlock( hElementList );  

  /* Unlock the topic's data. */
  GlobalUnlock( fpDisplayInfo->hTopicData );

  /* Success. */
  return( TRUE );
}
  

/*********************************** TABLE ROUTINES ****************************************/

/***************************************************
*
* Set beginning data for a table's row.
*
* Margins for the table's first column will be
* set when a TE_TABLECOLUMN record is received. 
*
****************************************************/
static void __far __pascal TableBegin
( 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPTABLEREC TableRectPtr;

  /* Lock the record. */
  TableRectPtr = (FPTABLEREC) fpTopicElement->RecDataPtr;

  /* Set context data. */
  fpContextData->bInTable = TRUE;
  fpContextData->wNumColumns = TableRectPtr->wNumColumns;
  fpContextData->ColumnPosPtr = TableRectPtr->ColumnPosPtr;
  fpContextData->wCurrColumn = 0;
  fpContextData->RowYPos = fpContextData->PageYPos;
  fpContextData->RowHeight = 0;
  fpContextData->TableLeftMargin = 0;
  fpContextData->TableRightMargin = 0;
}


/***************************************************
*
* End of a table's row.
*
* Move to an vertical output position past the table. 
*
****************************************************/
static void __far __pascal TableEnd
( 
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData
)
{
  long int LastHeight;
  
  
  /* No in table any longer. */
  fpContextData->bInTable = FALSE;

  /* 
  ** Get height of the last cell in the table's row
  ** that we just layed out. 
  */
  LastHeight = fpContextData->PageYPos - fpContextData->RowYPos;
  
  /* Last column was the heighest so far. */
  if( LastHeight > fpContextData->RowHeight )
  {
    /* Save height. */
    fpContextData->RowHeight = LastHeight;
  }
  
  /* Move past table's row. */
  fpContextData->PageYPos = fpContextData->RowYPos + fpContextData->RowHeight;

  /* Reset the table margins bac to zero. */
  fpContextData->TableLeftMargin = 0;
  fpContextData->TableRightMargin = 0;
  
  /* Calc. table margins for current column. */
  CalcTableMargins( fpDisplayInfo, fpContextData );

  /* Set current horizontal position to left text margin. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;             
}


/***************************************************
*
* Position output for table's next column.
*
* Move to an vertical output position past the table. 
*
****************************************************/
static void __far __pascal TableColumn
( 
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData
)
{
  long int LastHeight;
  
  
  /* Calc. table margins for current column. */
  CalcTableMargins( fpDisplayInfo, fpContextData );
  
  /* Next time, use the next column's position info. */
  fpContextData->wCurrColumn = fpContextData->wCurrColumn + 1;

  /* Calculate the page's margins with a table. */
  CalcMargins( fpDisplayInfo, fpContextData );

  /* 
  ** Get height of the last cell in the table's row
  ** that we just layed out. 
  */
  LastHeight = fpContextData->PageYPos - fpContextData->RowYPos;
  
  /* Last column was the heighest so far. */
  if( LastHeight > fpContextData->RowHeight )
  {
    /* Save height. */
    fpContextData->RowHeight = LastHeight;
  }
  
  /* Move to next cell's top vertical position. */
  fpContextData->PageYPos = fpContextData->RowYPos;

  /* Set current horizontal position to left text margin. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;             
}



/***************************************************
*
* Gets page size.
*
****************************************************/
static void __far __pascal GetPageSize( HWND hWnd, FPDISPLAYINFO fpDisplayInfo )
{
  PICTDISPLAYREC PictDispRecData;     /* Display information for a picture display record. */
  TXTDISPLAYREC TxtDispRecData;       /* Display information for a text display record. */
  BORDERDISPLAYREC BorderDispRecData; /* Display information for a border display record. */
  WORD wCurrDispRec;

  BOOL bReturn;
  
  BOOL bPictureIsLast;
  

  /* Initialize page values. */
  fpDisplayInfo->dwTopDisplayRecVertPos = 0;
  fpDisplayInfo->dwPageWidth = 0; 
  fpDisplayInfo->dwPageHeight = 0;

  /* Is a char. picture object last on the page? */
  bPictureIsLast = FALSE;


  /* Retrieve first text and "char picture" display record. */
  wCurrDispRec = 0;
  bReturn = GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDispRec, &TxtDispRecData );
  
  /* For each text display record. */
  while( bReturn )
  {
    /* 
    ** Process the current display record's position info. 
    */
    
    /* Width of page. */
    if( fpDisplayInfo->dwPageWidth < (DWORD) TxtDispRecData.PosRect.right )
    {
       fpDisplayInfo->dwPageWidth = (DWORD) TxtDispRecData.PosRect.right;
    }
    
    /* Height of page. */
    if( fpDisplayInfo->dwPageHeight < (DWORD) TxtDispRecData.PosRect.bottom )
    {
       /* If its a char. picture. */
       if( TxtDispRecData.wRecType == TE_CHARBITMAP || TxtDispRecData.wRecType == TE_CHARMETA )
       {
         bPictureIsLast = TRUE;
       }
       
       fpDisplayInfo->dwPageHeight = (DWORD) TxtDispRecData.PosRect.bottom;
    }

    /* Get next display record. */
    wCurrDispRec++;
    bReturn = GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDispRec, &TxtDispRecData );
  }
  

  /* Retrieve first justified picture's display record. */
  wCurrDispRec = 0;
  bReturn = GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, wCurrDispRec, &PictDispRecData );
  
  /* For each justified picture display record. */
  while( bReturn )
  {
    /* 
    ** Process the current display 
    ** record's position info. 
    */
    
    /* Width of page. */
    if( fpDisplayInfo->dwPageWidth < (DWORD) PictDispRecData.PosRect.right )
    {
       fpDisplayInfo->dwPageWidth = (DWORD) PictDispRecData.PosRect.right;
    }
    
    /* Height of page. */
    if( fpDisplayInfo->dwPageHeight < (DWORD) PictDispRecData.PosRect.bottom )
    {
       fpDisplayInfo->dwPageHeight = (DWORD) PictDispRecData.PosRect.bottom;
    }

    /* Get next picture display record. */
    wCurrDispRec++;
    bReturn = GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, wCurrDispRec, &PictDispRecData );
  }


  /* Retrieve first border's display record. */
  wCurrDispRec = 0;
  bReturn = GetBorderDisplayListRec( fpDisplayInfo->fpBorderDisplayList, wCurrDispRec, &BorderDispRecData );
  
  /* For each border display record. */
  while( bReturn )
  {
    /* 
    ** Process the current display 
    ** record's position info. 
    */
    
    /* Width of page. */
    if( fpDisplayInfo->dwPageWidth < (DWORD) BorderDispRecData.PosRect.right )
    {
       fpDisplayInfo->dwPageWidth = (DWORD) BorderDispRecData.PosRect.right;
    }
    
    /* Height of page. */
    if( fpDisplayInfo->dwPageHeight < (DWORD) BorderDispRecData.PosRect.bottom )
    {
       fpDisplayInfo->dwPageHeight = (DWORD) BorderDispRecData.PosRect.bottom;
    }
    
    /* Get next picture display record. */
    wCurrDispRec++;
    bReturn = GetBorderDisplayListRec( fpDisplayInfo->fpBorderDisplayList, wCurrDispRec, &BorderDispRecData );
  }



  /* 
  ** Picture is last object on the screen. Add some space. 
  */
  if( bPictureIsLast )                                                 
  {
    fpDisplayInfo->dwPageHeight = fpDisplayInfo->dwPageHeight + 60;             
  }

  /* 
  ** Get new position for the object that was previously displayed 
  ** at the top of the window.
  */
  if( fpDisplayInfo->wTopDisplayList == NO_DISPLAY_LIST )
  {
    /* First time displaying the page - default to top position. */
    fpDisplayInfo->dwTopDisplayRecVertPos = 0;
  }
  else if( fpDisplayInfo->wTopDisplayList == TXT_DISPLAY_LIST )
  {
    /* Is text - use text display list. */    
    if( GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, fpDisplayInfo->wTopDisplayRec, &TxtDispRecData ) )
    {
      fpDisplayInfo->dwTopDisplayRecVertPos = TxtDispRecData.dwLinesYPos;
    }
  }
  else if( fpDisplayInfo->wTopDisplayList == PICT_DISPLAY_LIST )
  {
    /* Is a picture - use picture display list. */    
    if( GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, fpDisplayInfo->wTopDisplayRec, &PictDispRecData ) )
    {
      fpDisplayInfo->dwTopDisplayRecVertPos = (DWORD) PictDispRecData.PosRect.top;
    }
  }
  else /* Border. */
  {
    /* Is a border - use border display list. */    
    if( GetBorderDisplayListRec( fpDisplayInfo->fpBorderDisplayList, fpDisplayInfo->wTopDisplayRec, &BorderDispRecData ) )
    {
      fpDisplayInfo->dwTopDisplayRecVertPos = (DWORD) BorderDispRecData.PosRect.top;
    }
  }
}



/***************************************************
*
* Draw the topic using the display lists.
*
****************************************************/
static BOOL __far __pascal DrawPage( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, BOOL UseNonScrollable )
{
  FPTOPICDATA fpTopicData;

  HGLOBAL hElementList;                 /* The element list we are using. */
  FPELEMENTLIST fpElementList;

  WORD wCurrentElement;                 /* Number of current element record. */
  TOPICELEMENT ElementData;             /* A piece of data about the page. */
  
  WORD wCurrDisplayRec;                 /* Number of current display record. */

  TXTDISPLAYREC TxtDispRecData;         /* Text display record data holding formatting
                                           information for text and character positioned
                                           pictures. */

  PICTDISPLAYREC PictDispRecData;       /* Picture display record data holding formatting
                                           information for pictures that are right or
                                           left justified. */

  BORDERDISPLAYREC BorderDispRecData;   /* Border drawing information. */


  CONTEXTDATA ContextData;              /* Context data used while drawing. */
  
  BOOL bFoundFirst;                     /* Have we found the first object that is
                                           displayed. */
  
  HPEN hSolidPen, hDashedPen;           /* Pens for hotspot underlining. */
  
  DWORD dwWndBottomPos;                 /* Last page position visible at bottom of window. */
  
  long int TopPosX, TopPosY;            /* Position of first displayed object in window. */

  BOOL bReturn;                         /* Return value. */
  
  BOOL bUseSameElement;                 /* Keep using same element record. */
  

  /* Initialize context data. */                                
  InitializeContextData( hDC, fpDisplayInfo, &ContextData );

  /* Set background drawing color. */
  SetBkColor( hDC, fpDisplayInfo->BGColor );
  SetBkMode( hDC, TRANSPARENT );
  
  /* Get pointer to the topic data. */
  fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );  

  /* Calculate the last page position visible in the window. */
  dwWndBottomPos = (DWORD) fpDisplayInfo->PageYPosOffset + (DWORD) fpDisplayInfo->DisplaySize.y;


  /*************************************
  **
  **         Display borders
  **
  **************************************/

  /* Using first display record. */
  wCurrDisplayRec = 0;

  /* 
  ** Have we found first displayable object to
  ** appear in the window?
  */
  bFoundFirst = FALSE;
  
  /* For each topic element. */
  while( GetBorderDisplayListRec( fpDisplayInfo->fpBorderDisplayList, wCurrDisplayRec, &BorderDispRecData ) )
  {
    /* If we have not found the first visible object. */
    if( ! bFoundFirst )
    {
      /* 
      ** See if at least a bottom part of the object 
      ** will appear in the window. 
      */
      if( (BorderDispRecData.PosRect.bottom - fpDisplayInfo->PageYPosOffset) > 0 )
      {
        /* Object appeared in page before object currently stored as top. */
        if( BorderDispRecData.PosRect.top < TopPosY ||
            ( BorderDispRecData.PosRect.top == TopPosY && 
              BorderDispRecData.PosRect.left < TopPosX )
          )
        {
          /* Save page position of object. */
          TopPosX = BorderDispRecData.PosRect.left;
          TopPosY = BorderDispRecData.PosRect.top;
  
          /* Save first visible object's record number. */
          fpDisplayInfo->wTopDisplayRec = wCurrDisplayRec;
          fpDisplayInfo->wTopDisplayList = BORDER_DISPLAY_LIST;
        }
        
        /* Have found first displayed object. */
        bFoundFirst = TRUE;
      }
    }
  
    /* Use next record. */
    wCurrDisplayRec++;
  
    /* First visible object found. */
    if( bFoundFirst )
    {
      /* 
      ** Verify that at least a part of the top of the object 
      ** will appear in the window. 
      */
      if( (DWORD) BorderDispRecData.PosRect.top < dwWndBottomPos )
      {
        bReturn = DrawBorder( hDC, fpDisplayInfo, &BorderDispRecData );
              
        /* Error? */
        if( !bReturn )
        {
          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
        
          /* Failure. */
          return( FALSE );
        }
      }  
    }
  }


  /*************************************
  **
  **         Display text
  **
  **************************************/

  /* Create hotspot underlining pens. */
  hSolidPen = CreatePen( PS_SOLID, 1, ContextData.HotSpotColor );
  hDashedPen = CreatePen( PS_DOT, 1, ContextData.HotSpotColor );

  /* Using first topic element. */
  wCurrentElement = 0;

  /* Using first display record. */
  wCurrDisplayRec = 0;

  /* 
  ** Have we found first displayable object to
  ** appear in the window?
  */
  bFoundFirst = FALSE;

  /* Use the desired element list for the topic. */
  if( UseNonScrollable )
  {
    hElementList = fpTopicData->hNonScrollElements;
  }
  else
  {
    hElementList = fpTopicData->hScrollElements;
  }
  /* Lock the topic's element list. */
  fpElementList = (FPELEMENTLIST) GlobalLock( hElementList );  

  /* For each topic element. */
  while( RetrieveElementListRec( fpElementList, &ElementData, wCurrentElement ) )
  {
    /* Process the current element record. */
    switch( ElementData.wRecType )
    {
      /* Text element. */
      case TE_TEXT:
      {
        /* Use same element record until all of its text is shown. */
        bUseSameElement = TRUE;
          
        /* Get the next text display record. */
        if( !GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDisplayRec, &TxtDispRecData ) )
        {
          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
    
          /* Delete hotspot underlining pens. */
          DeleteObject( hSolidPen );
          DeleteObject( hDashedPen );
  
          /* Failure. */
          return( FALSE );
        }
  
        /* While we haven't shown all of element record's text. */
        while( bUseSameElement )
        {
          /* If we have not found the first visible object. */
          if( ! bFoundFirst )
          {
            /* 
            ** See if at least a bottom part of the object 
            ** will appear in the window. 
            */
            if( ( TxtDispRecData.PosRect.bottom - fpDisplayInfo->PageYPosOffset) > 0 )
            {
              /* Save page position of object. */
              TopPosX = TxtDispRecData.PosRect.left;
              TopPosY = TxtDispRecData.PosRect.top;
              
              /* Save first visible object's record number. */
              fpDisplayInfo->wTopDisplayRec = wCurrDisplayRec;
              fpDisplayInfo->wTopDisplayList = TXT_DISPLAY_LIST;
      
              /* Have found first displayed object. */
              bFoundFirst = TRUE;
            }
          }
  
  
          /* First visible object found. */
          if( bFoundFirst )
          {
            /* 
            ** Verify that at least a part of the top of the object 
            ** will appear in the window. 
            */
            if( (DWORD) TxtDispRecData.PosRect.top < dwWndBottomPos )
            {
              if( ! DrawTextObject( hDC, fpDisplayInfo, &TxtDispRecData ) )
              {
                /* Unlock the topic's data. */
                GlobalUnlock( fpDisplayInfo->hTopicData );
        
                /* Delete hotspot underlining pens. */
                DeleteObject( hSolidPen );
                DeleteObject( hDashedPen );
      
                /* Failure. */
                return( FALSE );
              }  
      
              
              /* 
              ** Should we underline text.
              */
              if( ContextData.bInHotSpot == TRUE && 
                  ContextData.HotSpotInfo.bColorAndUnder == TRUE )
              {
                /* Draw the correct underlining. */
                DrawHotspotUnderline( hDC, &TxtDispRecData, &ContextData, fpDisplayInfo, hSolidPen, hDashedPen );
              }
            }  
          }  
          
          /* Use next record. */
          wCurrDisplayRec++;

          /* 
          ** Retrieve next display record. 
          */
          if( GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDisplayRec, &TxtDispRecData ) )
          {
            /* 
            ** If next display record is for the same topic element,
            ** then don't skip to the next topic element.
            */
            if( TxtDispRecData.ElementDataPtr != ElementData.RecDataPtr )
            {
              /* Next text record IS NOT for current element record. */
              bUseSameElement = FALSE;
            }
          }
          else
          {
            /* No more text records left for current element record. */
            bUseSameElement = FALSE;
          }
        }
        break;
      }


      /* Character positioned bitmap or metafile. */
      case TE_CHARBITMAP:
      case TE_CHARMETA:
      {
        if( !GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDisplayRec, &TxtDispRecData ) )
        {
          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
  
          /* Failure. */
          return( FALSE );
        }

        /* If we have not found the first visible object. */
        if( ! bFoundFirst )
        {
          /* 
          ** See if at least a bottom part of the object 
          ** will appear in the window. 
          */
          if( ( TxtDispRecData.PosRect.bottom - fpDisplayInfo->PageYPosOffset) > 0 )
          {
            /* Save page position of object. */
            TopPosX = TxtDispRecData.PosRect.left;
            TopPosY = TxtDispRecData.PosRect.top;
      
            /* Save first visible object's record number. */
            fpDisplayInfo->wTopDisplayRec = wCurrDisplayRec;
            fpDisplayInfo->wTopDisplayList = TXT_DISPLAY_LIST;
    
            /* Have found first displayed object. */
            bFoundFirst = TRUE;
          }
        }

        /* Use next record. */
        wCurrDisplayRec++;

        /* First visible object found. */
        if( bFoundFirst )
        {
          /* 
          ** Verify that at least a part of the top of the object 
          ** will appear in the window. 
          */
          if( (DWORD) TxtDispRecData.PosRect.top < dwWndBottomPos )
          {
            /* Bitmap. */
            if( ElementData.wRecType == TE_CHARBITMAP )
            {
              bReturn = DrawBitmap( hDC, fpDisplayInfo, TxtDispRecData.hElementData, &TxtDispRecData.PosRect );
            }
            
            /* Metafile. */
            else
            {
              bReturn = DrawMetaFile( hDC, fpDisplayInfo, TxtDispRecData.hElementData, &TxtDispRecData.PosRect );
            }
            
            /* Error? */
            if( ! bReturn )
            {
              /* Unlock the topic's data. */
              GlobalUnlock( fpDisplayInfo->hTopicData );
      
              /* Failure. */
              return( FALSE );
            }
          }  
        }
        
        break;
      }


      /* Font element. */
      case TE_FONT:
      {
        if( ! SetFontObject( hDC, hWnd, fpDisplayInfo, &ElementData, &ContextData ) )
        {
          /* Unlock the topic's data. */
          GlobalUnlock( fpDisplayInfo->hTopicData );
  
          /* Delete hotspot underlining pens. */
          DeleteObject( hSolidPen );
          DeleteObject( hDashedPen );

          /* Failure. */
          return( FALSE );
        }                                                      

        break;
      }


      /* Start of hotspot. */
      case TE_HOTBEGIN:
      {
        HotSpotBegin( hDC, &ContextData, &ElementData );
        break;
      }

      /* End of hotspot. */
      case TE_HOTEND:
      {
        /* Set color and flags. */
        HotSpotEnd( hDC, &ContextData );
        break;
      }
    }
    
    /* Goto next topic element. */
    wCurrentElement++;
  }
  
  /* Delete hotspot underlining pens. */
  DeleteObject( hSolidPen );
  DeleteObject( hDashedPen );

  /* Unlock the topic's data. */
  GlobalUnlock( fpDisplayInfo->hTopicData );

      

  /*************************************
  **
  **     Display justified pictures
  **
  **************************************/

  /* Using first display record. */
  wCurrDisplayRec = 0;

  /* 
  ** Have we found first displayable object to
  ** appear in the window?
  */
  bFoundFirst = FALSE;
  
  /* For each topic element. */
  while( GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, wCurrDisplayRec, &PictDispRecData ) )
  {
    /* If we have not found the first visible object. */
    if( ! bFoundFirst )
    {
      /* 
      ** See if at least a bottom part of the object 
      ** will appear in the window. 
      */
      if( (PictDispRecData.PosRect.bottom - fpDisplayInfo->PageYPosOffset) > 0 )
      {
        /* Object appeared in page before object currently stored as top. */
        if( PictDispRecData.PosRect.top < TopPosY ||
            ( PictDispRecData.PosRect.top == TopPosY && 
              PictDispRecData.PosRect.left < TopPosX )
          )
        {
          /* Save page position of object. */
          TopPosX = PictDispRecData.PosRect.left;
          TopPosY = PictDispRecData.PosRect.top;
      
          /* Save first visible object's record number. */
          fpDisplayInfo->wTopDisplayRec = wCurrDisplayRec;
          fpDisplayInfo->wTopDisplayList = PICT_DISPLAY_LIST;
        }

        /* Have found first displayed object. */
        bFoundFirst = TRUE;
      }
    }

    /* Use next record. */
    wCurrDisplayRec++;

    /* First visible object found. */
    if( bFoundFirst )
    {
      /* 
      ** Verify that at least a part of the top of the object 
      ** will appear in the window. 
      */
      if( (DWORD) PictDispRecData.PosRect.top < dwWndBottomPos )
      {
        /* Bitmap. */
        if( PictDispRecData.wPictType == TE_RIGHTBITMAP || PictDispRecData.wPictType == TE_LEFTBITMAP)
        {
          bReturn = DrawBitmap( hDC, fpDisplayInfo, PictDispRecData.hPictData, &PictDispRecData.PosRect );
        }
            
        /* Metafile. */
        else
        {
          bReturn = DrawMetaFile( hDC, fpDisplayInfo, PictDispRecData.hPictData, &PictDispRecData.PosRect );
        }
            
        /* Error? */
        if( !bReturn )
        {
          /* Failure. */
          return( FALSE );
        }
      }  
    }
  }     

  /* Success. */
  return( TRUE );
}

  

/*************************** NON-CHAR POSITIONED PICTURE ROUTINES ****************************/

/***************************************************
*
* Inserts a picture into the picture display list.
*
****************************************************/
static BOOL __far __pascal InsertPicture
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement
)
{
  PICTDISPLAYREC PictDisplayRec;

  /* Save element info. to a picture display list record. */
  PictDisplayRec.wPictType = fpTopicElement->wRecType; 
  PictDisplayRec.hPictData = fpTopicElement->hRecData; 
  
  /* 
  ** Are we drawing a picture for a hotspot.
  */
  if( fpContextData->bInHotSpot == TRUE )
  {
    /* Is it a hotspot? */
    PictDisplayRec.bIsHotSpot = TRUE;           

    /* Info for the hotspot. */
    PictDisplayRec.HotSpotInfo = fpContextData->HotSpotInfo;

  }
  else
  {
    PictDisplayRec.bIsHotSpot = FALSE; 
  }

  /* Insert record into display list for left justified pictures. */
  if( !InsertPictDisplayListRec( hWnd, fpDisplayInfo->fpPictDisplayList, &PictDisplayRec ) )  
  {
    /* Error. */
    return( FALSE );
  }
      
  /* 
  ** One more picture waiting to be formatted for the paragraph.
  */
  fpContextData->wUnformatPicts++;

  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Formats the next unformatted picture in the 
* picture display list.
*
****************************************************/
static BOOL __far __pascal LayoutPicture
( 
  HDC           hDC,
  HWND          hWnd,
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData 
)
{
  PICTDISPLAYREC PictDisplayRec;

  /* Get record for next unformatted picture from display list. */
  if( ! GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                               fpContextData->wNextUnformatRec, &PictDisplayRec ) )  
  {
    /* Error. */
    return( FALSE );
  }

  /* Process the current element record. */
  switch( PictDisplayRec.wPictType )
  {
    /* Left justified bitmap. */
    case TE_LEFTBITMAP:
    {
      return( LayoutLeftBitMap( hDC, hWnd, fpContextData, fpDisplayInfo ) );
    }

    /* Right justified bitmap. */
    case TE_RIGHTBITMAP:
    {
      return( LayoutRightBitMap( hDC, hWnd, fpContextData, fpDisplayInfo ) );
    }

    /* Left justified metafile. */
    case TE_LEFTMETA:
    {
      return( LayoutLeftMetaFile( hDC, hWnd, fpContextData, fpDisplayInfo ) );
    }

    /* Right justified metafile. */
    case TE_RIGHTMETA:
    {
      return( LayoutRightMetaFile( hDC, hWnd, fpContextData, fpDisplayInfo ) );
    }

    default:
    {
      /* Failure. */
      return( FALSE );
    }
  }

  /* Failure. */
  return( FALSE );
}


/***************************************************
*
* Sets context data to reflect that a left picture
* has been formatted.
*
****************************************************/
static void __far __pascal PictFormatted
( 
  FPCONTEXTDATA fpContextData,          /* Context data. */
  FPPOSRECT fpPictPos,                  /* Picture's page position. */
  BOOL bLeftPict                        /* Is picture left justified? */
)
{ 
  /* Update the left picture position information. */
  fpContextData->PictPosRect.left   = fpPictPos->left;          
  fpContextData->PictPosRect.right  = fpPictPos->right;           
  fpContextData->PictPosRect.top    = fpPictPos->top;       
  fpContextData->PictPosRect.bottom = fpPictPos->bottom;            
  
  if( bLeftPict )
  {
    /* New value for paragraph's left margin ? */
    if( fpContextData->ParaLeftMargin < fpContextData->PictPosRect.left )
    {
      /* New value. */
      fpContextData->ParaLeftMargin = fpContextData->PictPosRect.left;
    }

    /* We are currently displaying a left justfied picture. */
    fpContextData->bLeftPicture = TRUE;
  }
  else
  {
    /* We are currently displaying a right justfied picture. */
    fpContextData->bRightPicture = TRUE;
  }

  /* Next picture display record is awaiting formatting. */
  fpContextData->wNextUnformatRec++;

  /* 
  ** One less picture waiting to be formatted for the paragraph.
  */
  fpContextData->wUnformatPicts--;
}


/********************************** BITMAP ROUTINES ************************************/

/***************************************************
*
* Position a left margin bitmap in page.
*
****************************************************/
static BOOL __far __pascal LayoutLeftBitMap
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
)
{
  PICTDISPLAYREC PictDisplayRec;

  FPBITMAPREC fpBitmapRec;

  LPBITMAPINFO PtrBmi;
  POINT Point;
  
  short int Width,
            Height;
            
            
  /* Get record for next unformatted picture from display list. */
  if( ! GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                               fpContextData->wNextUnformatRec, &PictDisplayRec ) )
  {
    return( FALSE );
  }  

  /* Lock the record's  data. */
  fpBitmapRec = (FPBITMAPREC) GlobalLock( PictDisplayRec.hPictData );
  
  /* Lock bitmap info. buffer. */
  PtrBmi = (LPBITMAPINFO) GlobalLock( fpBitmapRec->hBitmapInfo );
    
  /* Get the size of the bitmap in TWIPS. */
  Point.x = ( short int ) PtrBmi->bmiHeader.biWidth;
  Point.y = ( short int ) PtrBmi->bmiHeader.biHeight;
  DPtoLP( hDC, &Point, 1 );
  Width  = Point.x;
  Height = Point.y;

  /* Unlock bitmap info. buffer. */
  GlobalUnlock( fpBitmapRec->hBitmapInfo );

  /* Save position data. */
  PictDisplayRec.PosRect.left   = fpContextData->PageXPos;          
  PictDisplayRec.PosRect.right  = fpContextData->PageXPos + Width;           
  PictDisplayRec.PosRect.top    = fpContextData->PageYPos;       
  PictDisplayRec.PosRect.bottom = fpContextData->PageYPos + Height;            

  /* Are there any SHED hotspots for the bitmap? */
  PictDisplayRec.wNumSHEDHotSpots = fpBitmapRec->wNumSHEDRecs;
      
  /* Insert record into display list. */
  if( !UpdatePictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                                 fpContextData->wNextUnformatRec, &PictDisplayRec ) )  
  {
    /* Unlock the bitmap record. */
    GlobalUnlock( PictDisplayRec.hPictData );
        
    /* Error. */
    return( FALSE );
        
  }
    
  /* Unlock the record's data. */
  GlobalUnlock( PictDisplayRec.hPictData );

  /* Set picture has been formatted. */
  PictFormatted( fpContextData, &(PictDisplayRec).PosRect, TRUE );

  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Position a right margin bitmap in page.
*
****************************************************/
static BOOL __far __pascal LayoutRightBitMap
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
)
{
  PICTDISPLAYREC PictDisplayRec;

  FPBITMAPREC fpBitmapRec;

  LPBITMAPINFO PtrBmi;
  POINT Point;
  
  short int Width,
            Height;
            
            
  /* Get record for next unformatted picture from display list. */
  if( ! GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                               fpContextData->wNextUnformatRec, &PictDisplayRec ) )
  {
    return( FALSE );
  }  

  /* Lock the record's  data. */
  fpBitmapRec = (FPBITMAPREC) GlobalLock( PictDisplayRec.hPictData );
  
  /* Lock bitmap info. buffer. */
  PtrBmi = (LPBITMAPINFO) GlobalLock( fpBitmapRec->hBitmapInfo );
    
  /* Get the size of the bitmap in TWIPS. */
  Point.x = ( short int ) PtrBmi->bmiHeader.biWidth;
  Point.y = ( short int ) PtrBmi->bmiHeader.biHeight;
  DPtoLP( hDC, &Point, 1 );
  Width  = Point.x;
  Height = Point.y;

  /* Unlock bitmap info. buffer. */
  GlobalUnlock( fpBitmapRec->hBitmapInfo );

  /* Save position data. */
  PictDisplayRec.PosRect.left   = fpDisplayInfo->DisplaySize.x - 
                                  fpContextData->RightPictMargin -           
                                  Width;
  PictDisplayRec.PosRect.right  = PictDisplayRec.PosRect.left + Width;           
  PictDisplayRec.PosRect.top    = fpContextData->PageYPos;       
  PictDisplayRec.PosRect.bottom = fpContextData->PageYPos + Height;            

  /* Are there any SHED hotspots for the bitmap? */
  PictDisplayRec.wNumSHEDHotSpots = fpBitmapRec->wNumSHEDRecs;

  /* Insert record into display list. */
  if( !UpdatePictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                                 fpContextData->wNextUnformatRec, &PictDisplayRec ) )  
  {
    /* Unlock the bitmap record. */
    GlobalUnlock( PictDisplayRec.hPictData );
        
    /* Error. */
    return( FALSE );
        
  }
    
  /* Unlock the record's data. */
  GlobalUnlock( PictDisplayRec.hPictData );

  /* Set picture has been formatted. */
  PictFormatted( fpContextData, &(PictDisplayRec).PosRect, FALSE );

  /* Successs. */
  return( TRUE );
}

/***************************************************
*
* Insert a character bitmap in page.
*
****************************************************/
static BOOL __far __pascal LayoutCharBitmap
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement 
)
{
  FPBITMAPREC fpBitmapRec;

  TXTDISPLAYREC TextDisplayRec;

  LPBITMAPINFO PtrBmi;
  POINT Point;
  
  short int Width,
            Height;
            
            
  /* Lock the record's  data. */
  fpBitmapRec = (FPBITMAPREC) GlobalLock( fpTopicElement->hRecData );
  
  /* Lock bitmap info. buffer. */
  PtrBmi = (LPBITMAPINFO) GlobalLock( fpBitmapRec->hBitmapInfo );
    
  /* Get the size of the bitmap in TWIPS. */
  Point.x = ( short int ) PtrBmi->bmiHeader.biWidth;
  Point.y = ( short int ) PtrBmi->bmiHeader.biHeight;
  DPtoLP( hDC, &Point, 1 );
  Width  = Point.x;
  Height = Point.y;

  /* Unlock bitmap info. buffer. */
  GlobalUnlock( fpBitmapRec->hBitmapInfo );
  
  /* What type of data is in the text display record. */
  TextDisplayRec.wRecType = TE_CHARBITMAP;

  /* Save formatting info. to a display list record. */
  TextDisplayRec.hElementData    = fpTopicElement->hRecData; 
  TextDisplayRec.wDataOffset     = 0;       
  TextDisplayRec.wNumChars       = 0;       
  TextDisplayRec.wBaseLineHeight = Height;
  TextDisplayRec.dwLinesYPos     = (DWORD) fpContextData->PageYPos; 

  /* Are there any SHED hotspots for char. bitmap? */
  TextDisplayRec.wNumSHEDHotSpots = fpBitmapRec->wNumSHEDRecs;

  /* 
  ** Fits on display's line?
  ** This comparison is done because Width could be
  ** a value that is VERY large ( larger than SHORT INT can hold ). */
  if( Width <= (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
  {
    /* 
    ** String fits on display's line even when considering current output position. 
    ** This comparison assumes that the value of wTextWidth is small enough for a SHORT INT
    ** to hold.
    */
    if( (fpContextData->PageXPos + Width) > (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
    {
      /* Goto the beginning of the next display next line. */
      WrapToNextLine( hDC, hWnd, fpDisplayInfo, fpContextData );
    }     
  }
  else
  {
    /* Goto the beginning of the next display next line. */
    WrapToNextLine( hDC, hWnd, fpDisplayInfo, fpContextData );
  }     

  /* Save position data. */
  TextDisplayRec.PosRect.left   = fpContextData->PageXPos;          
  TextDisplayRec.PosRect.right  = fpContextData->PageXPos + Width;           
  TextDisplayRec.PosRect.top    = fpContextData->PageYPos;       
  TextDisplayRec.PosRect.bottom = fpContextData->PageYPos + Height;            
      
  /* Picture is a hotspot? */
  if( fpContextData->bInHotSpot == TRUE )
  {
    /* Is it a hotspot? */
    TextDisplayRec.bIsHotSpot = TRUE;           

    /* Info for the hotspot. */
    TextDisplayRec.HotSpotInfo = fpContextData->HotSpotInfo;

  }
  else
  {
    TextDisplayRec.bIsHotSpot = FALSE; 
  }

  /* Insert record into display list. */
  if( !InsertTxtDisplayListRec( hWnd, fpDisplayInfo->fpTxtDisplayList, &TextDisplayRec ) )  
  {
    /* Unlock the text record. */
    GlobalUnlock( fpTopicElement->hRecData );
        
    /* Error. */
    return( FALSE );
        
  }

  /* Go to next display record. */
  fpContextData->wLastTxtDisplayRec = fpContextData->wLastTxtDisplayRec + 1;
        
  /* Update the horizontal position. */
  fpContextData->PageXPos = TextDisplayRec.PosRect.right; 
  
  /* Unlock the record's data. */
  GlobalUnlock( fpTopicElement->hRecData );

  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Draws a bitmap on the page.
*
****************************************************/
static BOOL __far __pascal DrawBitmap
( 
  HDC hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  HGLOBAL hBitmapRec, 
  FPPOSRECT PosRect
)
{
  FPBITMAPREC fpBitmapRec;
  LPBITMAPINFO PtrBmi;
  BYTE __huge * fpBitMapData;  
  
  
  /* Lock the record's data. */
  fpBitmapRec = (FPBITMAPREC) GlobalLock( hBitmapRec );
  
  /* Lock bitmap info. buffer. */
  PtrBmi = (LPBITMAPINFO) GlobalLock( fpBitmapRec->hBitmapInfo );
    
  /* Lock the bitmap data buffer. */
  fpBitMapData = (BYTE __huge *) GlobalLock( fpBitmapRec->hBitmapData );
    
  SetDIBitsToDevice( hDC, (short int) (PosRect->left - fpDisplayInfo->PageXPosOffset), 
                     (short int)( PosRect->top - fpDisplayInfo->PageYPosOffset ), 
                     (short int) PtrBmi->bmiHeader.biWidth, (short int) PtrBmi->bmiHeader.biHeight,
                     0, 0, 0, (short int) PtrBmi->bmiHeader.biHeight, fpBitMapData, PtrBmi, 
                     DIB_RGB_COLORS ); 

  /* Unlock the bitmap data buffer. */
  GlobalUnlock( fpBitmapRec->hBitmapData );
    
  /* Unlock bitmap info. buffer. */
  GlobalUnlock( fpBitmapRec->hBitmapInfo );

  /* Unlock the text record. */
  GlobalUnlock( hBitmapRec );
    
  /* Success. */
  return( TRUE );
}


  
/********************************** METAFILE ROUTINES ************************************/

/***************************************************
*
* Position a left margin metafile in page.
*
****************************************************/
static BOOL __far __pascal LayoutLeftMetaFile
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
)
{
  PICTDISPLAYREC PictDisplayRec;

  FPMETAREC fpMetaFileRec;

            
  /* Get record for next unformatted picture from display list. */
  if( ! GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                               fpContextData->wNextUnformatRec, &PictDisplayRec ) )
  {
    return( FALSE );
  }  

  /* Lock the record's  data. */
  fpMetaFileRec = (FPMETAREC) GlobalLock( PictDisplayRec.hPictData );
  
  /* Save position data. */
  PictDisplayRec.PosRect.left   = fpContextData->PageXPos;          
  PictDisplayRec.PosRect.right  = fpContextData->PageXPos + fpMetaFileRec->wWidth;           
  PictDisplayRec.PosRect.top    = fpContextData->PageYPos;       
  PictDisplayRec.PosRect.bottom = fpContextData->PageYPos + fpMetaFileRec->wHeight;            

  /* Are there any SHED hotspots for the metafile? */
  PictDisplayRec.wNumSHEDHotSpots = fpMetaFileRec->wNumSHEDRecs;

  /* Insert record into display list. */
  if( !UpdatePictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                                 fpContextData->wNextUnformatRec, &PictDisplayRec ) )  
  {
    /* Unlock the bitmap record. */
    GlobalUnlock( PictDisplayRec.hPictData );
        
    /* Error. */
    return( FALSE );
        
  }
    
  /* Unlock the record's data. */
  GlobalUnlock( PictDisplayRec.hPictData );

  /* Set picture has been formatted. */
  PictFormatted( fpContextData, &(PictDisplayRec).PosRect, TRUE );

  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Position a right margin metafile in page.
*
****************************************************/
static BOOL __far __pascal LayoutRightMetaFile
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo 
)
{
  PICTDISPLAYREC PictDisplayRec;

  FPMETAREC fpMetaFileRec;

            
  /* Get record for next unformatted picture from display list. */
  if( ! GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                               fpContextData->wNextUnformatRec, &PictDisplayRec ) )
  {
    return( FALSE );
  }  

  /* Lock the record's  data. */
  fpMetaFileRec = (FPMETAREC) GlobalLock( PictDisplayRec.hPictData );
  
  /* Save position data. */
  PictDisplayRec.PosRect.left   = fpDisplayInfo->DisplaySize.x - 
                                  fpContextData->RightPictMargin -           
                                  fpMetaFileRec->wWidth;
  PictDisplayRec.PosRect.right  = PictDisplayRec.PosRect.left + fpMetaFileRec->wWidth;           
  PictDisplayRec.PosRect.top    = fpContextData->PageYPos;       
  PictDisplayRec.PosRect.bottom = fpContextData->PageYPos + fpMetaFileRec->wHeight;            

  /* Are there any SHED hotspots for the metafile? */
  PictDisplayRec.wNumSHEDHotSpots = fpMetaFileRec->wNumSHEDRecs;

  /* Insert record into display list. */
  if( !UpdatePictDisplayListRec( fpDisplayInfo->fpPictDisplayList, 
                                 fpContextData->wNextUnformatRec, &PictDisplayRec ) )  
  {
    /* Unlock the bitmap record. */
    GlobalUnlock( PictDisplayRec.hPictData );
        
    /* Error. */
    return( FALSE );
        
  }
    
  /* Unlock the record's data. */
  GlobalUnlock( PictDisplayRec.hPictData );

  /* Set picture has been formatted. */
  PictFormatted( fpContextData, &(PictDisplayRec).PosRect, FALSE );

  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Insert a character metafile in page.
*
****************************************************/
static BOOL __far __pascal LayoutCharMetaFile
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement 
)
{
  FPMETAREC fpMetaFileRec;

  TXTDISPLAYREC TextDisplayRec;

            
  /* Lock the record's  data. */
  fpMetaFileRec = (FPMETAREC) GlobalLock( fpTopicElement->hRecData );
  
  /* What type of data is in the text display record. */
  TextDisplayRec.wRecType = TE_CHARMETA;

  /* Save formatting info. to a display list record. */
  TextDisplayRec.hElementData    = fpTopicElement->hRecData; 
  TextDisplayRec.wDataOffset     = 0;       
  TextDisplayRec.wNumChars       = 0;       
  TextDisplayRec.wBaseLineHeight = fpMetaFileRec->wHeight;
  TextDisplayRec.dwLinesYPos     = (DWORD) fpContextData->PageYPos; 

  /* Are there any SHED hotspots for the char. metafile? */
  TextDisplayRec.wNumSHEDHotSpots = fpMetaFileRec->wNumSHEDRecs;
      
  /* 
  ** Fits on display's line?
  ** This comparison is done because Width could be
  ** a value that is VERY large ( larger than SHORT INT can hold ). */
  if( fpMetaFileRec->wWidth <= (WORD)(fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
  {
    /* 
    ** String fits on display's line even when considering current output position. 
    ** This comparison assumes that the value of wTextWidth is small enough for a SHORT INT
    ** to hold.
    */
    if( (fpContextData->PageXPos + fpMetaFileRec->wWidth) > (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
    {
      /* Goto the beginning of the next display next line. */
      WrapToNextLine( hDC, hWnd, fpDisplayInfo, fpContextData );
    }     
  }
  else
  {
    /* Goto the beginning of the next display next line. */
    WrapToNextLine( hDC, hWnd, fpDisplayInfo, fpContextData );
  }     

  /* Save position data. */
  TextDisplayRec.PosRect.left   = fpContextData->PageXPos;          
  TextDisplayRec.PosRect.right  = fpContextData->PageXPos + fpMetaFileRec->wWidth;           
  TextDisplayRec.PosRect.top    = fpContextData->PageYPos;       
  TextDisplayRec.PosRect.bottom = fpContextData->PageYPos + fpMetaFileRec->wHeight;            
      
  /* Picture is a hotspot? */
  if( fpContextData->bInHotSpot == TRUE )
  {
    /* Is it a hotspot? */
    TextDisplayRec.bIsHotSpot = TRUE;           

    /* Info for the hotspot. */
    TextDisplayRec.HotSpotInfo = fpContextData->HotSpotInfo;

  }
  else
  {
    TextDisplayRec.bIsHotSpot = FALSE; 
  }


  /* Insert record into display list. */
  if( !InsertTxtDisplayListRec( hWnd, fpDisplayInfo->fpTxtDisplayList, &TextDisplayRec ) )  
  {
    /* Unlock the text record. */
    GlobalUnlock( fpTopicElement->hRecData );
        
    /* Error. */
    return( FALSE );
        
  }
    
  /* Go to next display record. */
  fpContextData->wLastTxtDisplayRec = fpContextData->wLastTxtDisplayRec + 1;
        
  /* Update the horizontal position. */
  fpContextData->PageXPos = TextDisplayRec.PosRect.right; 
  
  /* Unlock the record's data. */
  GlobalUnlock( fpTopicElement->hRecData );

  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Draws a metafile on the page.
*
****************************************************/
static BOOL __far __pascal DrawMetaFile
( 
  HDC hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  HGLOBAL hMetaRec, 
  FPPOSRECT PosRect
)
{
  FPMETAREC fpMetaFileRec;
  DWORD     dwOldOrg;
  POINT     OrgPoint;
  
  
  /* Lock the record's data. */
  fpMetaFileRec = (FPMETAREC) GlobalLock( hMetaRec );
    
  /* 
  ** Convert the page position to a window position.
  */
  OrgPoint.x = (short int)(PosRect->left - fpDisplayInfo->PageXPosOffset);
  OrgPoint.y = (short int)(PosRect->top - fpDisplayInfo->PageYPosOffset);
  
  LPtoDP( hDC, &OrgPoint, 1 );
  
  /* Set the device drawing origin to the correct position. */
  dwOldOrg = SetViewportOrg( hDC, OrgPoint.x, OrgPoint.y );
  
  /* Set the scaling. */
  ScaleViewportExt( hDC, 1, 3, 1, 3);

  /* Play the metafile. */
  PlayMetaFile( hDC, fpMetaFileRec->hMetaFile ); 

  /* Unlock the record's record. */
  GlobalUnlock( hMetaRec );
    
  /* Remove the scaling. */
  ScaleViewportExt( hDC, 1, 1, 1, 1);

  /* Restore origin. */
  SetViewportOrg( hDC, LOWORD( dwOldOrg ), HIWORD( dwOldOrg ) );
  
  /* In case metafile didn't, restore original mapping mode and extents. */
  SetMapMode( hDC, MM_ANISOTROPIC );
  SetWindowExt( hDC, ONE_INCH, ONE_INCH );
  SetViewportExt( hDC, GetDeviceCaps( hDC, LOGPIXELSX ),
                       GetDeviceCaps( hDC, LOGPIXELSY ) );

  /* Success. */
  return( TRUE );
}

  
/********************************** TEXT ROUTINES ********************************/

/***************************************************
*
* Creates a text display record for a topic's text element
* and character positioned picture element.
*
****************************************************/
static BOOL __far __pascal LayoutText
( 
  HDC hDC,
  HWND hWnd,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement 
)
{
  TXTDISPLAYREC TextDisplayRec;

  FPTEXTREC fpTextRec;      /* Pointer to a TEXT record's data. */

  char __far * fpString;    /* Pointer to the text string to be output. */

  WORD wCharsOnLine;

  WORD wCurrStringPos;
  WORD wCharsLeft;

  WORD wCurrentRec;
  BOOL bBlankLine;


  /* Save the next text record number. */
  wCurrentRec = fpContextData->wLastTxtDisplayRec;
  bBlankLine = FALSE;
  
  /* Lock the text record. */
  fpTextRec = (FPTEXTREC) fpTopicElement->RecDataPtr;

  /* Get the record's string. */
  fpString = fpTextRec->TextPtr;

  /* Our current position in the string. */
  wCurrStringPos = 0;
  
  /* Number of characters left to process in the string. */
  wCharsLeft = ( _fstrlen( fpString ) / sizeof(char) ); 
  
  /* Format all of the text in the string. */
  do
  {   
    /* 
    ** Get maximum number of characters in the string
    ** that will fit on the current line. 
    */
    wCharsOnLine = GetCharFitCount( hDC, fpContextData, fpDisplayInfo, fpString + wCurrStringPos, wCharsLeft, &(TextDisplayRec).PosRect );
      
    /* If none of the line fits, go to next display line. */
    if( wCharsOnLine == 0 )
    {
      /* Skip white space that should not be at the beginning of the line. */
      while( *(fpString + wCurrStringPos) == ' ' ) 
      {
        /* Goto next char. in string. */
        wCurrStringPos++;
          
        /* We have one less character to process. */
        wCharsLeft--;
      }
        
      /* If we had only spaces left to display on the new line, 
      ** then they would now be gone now.  So verify
      ** that we still have something left in the string that we
      ** want to display on the new line. 
      */
      if( wCharsLeft )
      {
        /* Goto the beginning of the next display next line. */
        WrapToNextLine( hDC, hWnd, fpDisplayInfo, fpContextData );
        
        /* 
        ** Get maximum number of characters in the string
        ** that will fit on the current line. 
        */
        wCharsOnLine = GetCharFitCount( hDC, fpContextData, fpDisplayInfo, fpString + wCurrStringPos, wCharsLeft, &(TextDisplayRec).PosRect );
      }
        
      /* No characters left to process. */
      else
      { 
        /* No characters remaining in string to display. */
        wCharsOnLine = 0;
        
        /* 
        ** If we're still using the beginning record number then
        ** all of string is a blank line.
        */
        if( wCurrentRec == fpContextData->wLastTxtDisplayRec )
        {
          /* All of string was a blank line. */
          bBlankLine = TRUE;

          /* Set position values to 0 space used. */ 
          TextDisplayRec.PosRect.left   = fpContextData->PageXPos;
          TextDisplayRec.PosRect.right  = fpContextData->PageXPos;
          TextDisplayRec.PosRect.top    = fpContextData->PageYPos;
          TextDisplayRec.PosRect.bottom = fpContextData->PageYPos;
        }
      }
    }
  
    /* If we found something we could to display on the line 
    ** or all of string was a total blank line. */
    if( wCharsOnLine > 0 || bBlankLine )
    {
      /* What type of data is in the text display record. */
      TextDisplayRec.wRecType = TE_TEXT;

      /* Save formatting info. to a display list record. */
      TextDisplayRec.ElementDataPtr     = fpTopicElement->RecDataPtr;
      TextDisplayRec.wDataOffset        = wCurrStringPos;       
      TextDisplayRec.wNumChars          = wCharsOnLine;       
      TextDisplayRec.wBaseLineHeight    = (WORD) fpContextData->FontMetrics.tmAscent;
      TextDisplayRec.dwLinesYPos        = (DWORD) fpContextData->PageYPos; 

      /* Text is a hotspot? */
      if( fpContextData->bInHotSpot == TRUE )
      {
        /* Is it a hotspot? */
        TextDisplayRec.bIsHotSpot = TRUE;           
    
        /* Info for the hotspot. */
        TextDisplayRec.HotSpotInfo = fpContextData->HotSpotInfo;
    
      }
      else
      {
        TextDisplayRec.bIsHotSpot = FALSE; 
      }

      /* Text will not have any SHED hotspots. */
      TextDisplayRec.wNumSHEDHotSpots = 0;

      /* Insert record into display list. */
      if( !InsertTxtDisplayListRec( hWnd, fpDisplayInfo->fpTxtDisplayList, &TextDisplayRec ) )  
      {
        /* Error. */
        return( FALSE );
        
      }
    
      /* Go to next display record. */
      fpContextData->wLastTxtDisplayRec = fpContextData->wLastTxtDisplayRec + 1;
        
      /* Update the horizontal position. */
      fpContextData->PageXPos = TextDisplayRec.PosRect.right; 
  
      /* Adjust number of characters left to process. */
      wCharsLeft = wCharsLeft - wCharsOnLine; 
    
      /* Adjust where we are in the string. */
      wCurrStringPos = wCurrStringPos + wCharsOnLine;
    }
  }
  while( wCharsLeft > 0 );
  
  /* Success. */
  return( TRUE );
}
  

/***************************************************
*
* The determines the number of characters that
* can fit on the current line given the currently
* selected font and the current display size.
*
****************************************************/
static WORD __far __pascal GetCharFitCount
( 
  HDC hDC,
  FPCONTEXTDATA fpContextData,
  FPDISPLAYINFO fpDisplayInfo, 
  char __far * fpString, 
  WORD wStrLen,
  FPPOSRECT fpPosRect 
)
{
  DWORD dwExtent;
  WORD  wTextWidth, 
        wTextHeight;
  
  WORD  wCharsAllowed;
  WORD  wCharsTrimmed;
  WORD  wTempCharsTrimmed;
  
  char __far * fpSringEndPtr;
  
    
  /* If word wrapping. */
  if( fpContextData->bWordWrap )
  {
    /* If string is longer than 10 characters. */
    if( wStrLen > 10 )
    {
      /* 
      ** See how long the string can be before we fail.
      */ 
      wCharsAllowed = 10;
      
      /* While we haven't add the entire string. */
      while( wCharsAllowed != wStrLen )
      {
        /* Get string's width in pixels. */
        dwExtent = GetTextExtent( hDC, fpString, wCharsAllowed );
        wTextWidth = LOWORD(dwExtent);  

        /* 
        ** String fits on display's line?
        ** This comparison is done because wTextWidth could be
        ** a value that is VERY large ( larger than SHORT INT can hold ). */
        if( wTextWidth <= (WORD) (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
        {
          /* 
          ** String fits on display's line even when considering current output position. 
          ** This comparison assumes that the value of wTextWidth is small enough for a SHORT INT
          ** to hold.
          */
          if( (short int) (fpContextData->PageXPos + (long int) wTextWidth) > (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
          {
            /* String width that many characters is too long. */
            break;
          }
        }
        else
        {
          /* String width that many characters is too long. */
          break;
        }
        
        /* Add ten more characters to string. */
        wCharsAllowed = min( wStrLen, wCharsAllowed + 10 );
      } 
    
      /* Now start trimming at point of failure. */ 
      wCharsTrimmed = wStrLen - wCharsAllowed;
    }
    
    /* No preliminary work. */
    else
    {
      wCharsTrimmed = 0;
    }


    /* 
    ** While we haven't trimmed all the characters from 
    ** the string to make it fit. 
    */
    while( wCharsTrimmed != wStrLen )
    {
      /* Get string's width in pixels. */
      dwExtent = GetTextExtent( hDC, fpString, wStrLen - wCharsTrimmed );
      wTextWidth = LOWORD(dwExtent);  
      wTextHeight = HIWORD(dwExtent);  
      
      /* 
      ** String fits on display's line?
      ** This comparison is done because wTextWidth could be
      ** a value that is VERY large ( larger than SHORT INT can hold ). */
      if( wTextWidth <= (WORD) (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
      {
        /* 
        ** String fits on display's line even when considering current output position. 
        ** This comparison assumes that the value of wTextWidth is small enough for a SHORT INT
        ** to hold.
        */
        if( (short int) (fpContextData->PageXPos + (long int) wTextWidth) <= (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
        {
          /* Success. */
            
          /* Save position data. */
          fpPosRect->left   = fpContextData->PageXPos;          
          fpPosRect->right  = fpContextData->PageXPos + wTextWidth;           
          fpPosRect->top    = fpContextData->PageYPos;       
          fpPosRect->bottom = fpContextData->PageYPos + wTextHeight;
      
          /* Return length of string. */
          break;
        }
      }
          
      /* 
      ** String too long - trim off a word. 
      */
      
      /* Point to end of string. */
      fpSringEndPtr = fpString + ( wStrLen - wCharsTrimmed ) - 1;
      
      /* Skip backwards until a white space ( e.g. a word break ) */
      while( (wCharsTrimmed != wStrLen) && (*fpSringEndPtr != ' ') ) 
      {
        wCharsTrimmed++;
        fpSringEndPtr--;
      }
          
      /* Skip backwards until a non white space */
      while( (wCharsTrimmed != wStrLen) && (*fpSringEndPtr == ' ') ) 
      {
        wCharsTrimmed++;
        fpSringEndPtr--;
      }
    }
  }
  
  /* No word wrapping. */
  else
  {
    /* 
    ** All of the string's characters fit on the line. 
    */ 
    wCharsTrimmed = 0;

    /* Get string's width in pixels. */
    dwExtent = GetTextExtent( hDC, fpString, wStrLen );
    wTextWidth = LOWORD(dwExtent);  
    wTextHeight = HIWORD(dwExtent);  
    
    /* Save position data. */
    fpPosRect->left   = fpContextData->PageXPos;          
    fpPosRect->right  = fpContextData->PageXPos + wTextWidth;           
    fpPosRect->top    = fpContextData->PageYPos;       
    fpPosRect->bottom = fpContextData->PageYPos + wTextHeight;            
  }

    
  /* 
  ** If we found no characters to fit,
  ** we probably have so small a window that
  ** not even one word can be drawn on a line.
  ** In this case, return first word of text.
  */
  if( (wStrLen - wCharsTrimmed) == 0 )
  {
    wTempCharsTrimmed = wCharsTrimmed;
    fpSringEndPtr = fpString;
    
    /* Skip forward until a white space to get first word.*/
    while( (wTempCharsTrimmed != 0) && (*fpSringEndPtr != ' ') ) 
    {
      wTempCharsTrimmed--;
      fpSringEndPtr++;
    }

    /* Get text's width in pixels. */
    dwExtent = GetTextExtent( hDC, fpString, wStrLen - wTempCharsTrimmed );
    wTextWidth = LOWORD(dwExtent);  
    wTextHeight = HIWORD(dwExtent);  
  
    /* Even a word would not fit because window is too small, output the word anyway. */
    if( fpContextData->LeftTxtMargin + wTextWidth > fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin )
    {
      /* Success. */
      
      /* Save position data. */
      fpPosRect->left   = fpContextData->PageXPos;          
      fpPosRect->right  = fpContextData->PageXPos + wTextWidth;           
      fpPosRect->top    = fpContextData->PageYPos;       
      fpPosRect->bottom = fpContextData->PageYPos + wTextHeight;            

      /* Return string length of word only. */
      wCharsTrimmed = wTempCharsTrimmed;
    }
  }
  
  /* 
  ** Return the number of characters 
  ** that fits on the line. 
  */
  return( wStrLen - wCharsTrimmed );
}



/***************************************************
*
* Shows a text display record on the screen.
*
****************************************************/
static BOOL __far __pascal DrawTextObject
( 
  HDC hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  FPTXTDISPLAYREC fpTxtDisplayRec 
)
{
  FPTEXTREC fpTextRec;      /* Pointer to a TEXT record's data. */
  char __far * fpString;    /* Pointor to the text string to be output. */

  /* If there are characters to show. */
  if( fpTxtDisplayRec->wNumChars > 0 )
  {
    /* Lock the text record. */
    fpTextRec = (FPTEXTREC) fpTxtDisplayRec->ElementDataPtr;
  
    /* Lock the record's string. */
    fpString = fpTextRec->TextPtr;
  
    /* Move to starting position in the string. */
    fpString = fpString + fpTxtDisplayRec->wDataOffset;

    /* Output string. */
    if( ! TextOut( hDC, (short int) (fpTxtDisplayRec->PosRect.left - fpDisplayInfo->PageXPosOffset), 
                   (short int) (fpTxtDisplayRec->PosRect.top - fpDisplayInfo->PageYPosOffset), 
                   fpString, fpTxtDisplayRec->wNumChars ) )
    {
      /* Failure. */
      return( FALSE );
    }
  }
    
  /* Success. */
  return( TRUE );
}


/************************************** FONT ROUTINES ********************************/

static BOOL __far __pascal SetFontObject
( 
  HDC hDC,
  HWND hWnd,
  FPDISPLAYINFO fpDisplayInfo, 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPFONTREC fpFontRec;

  HFONT   hFont;
  LOGFONT lf;
  

  /* Lock the font record. */
  fpFontRec = (FPFONTREC) fpTopicElement->RecDataPtr;

  /* Get font from the font buffer. */
  hFont = FindFont( fpDisplayInfo->hFontBuffer, fpFontRec->wFontIndex );

  /* 
  ** Did not find an existing font. 
  ** Make a new font.
  */
  if( hFont == NULL )
  {
    /* Set font features. */
    lf.lfWidth =  0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfCharSet = fpContextData->CharSet;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;

    /* Font face name. */
    _fstrcpy( lf.lfFaceName, fpFontRec->FontInfo.szFontName );
  
    /* Family. */
    switch (fpFontRec->FontInfo.FontFamily) 
    {
      case FAM_MODERN: 
      {
        lf.lfPitchAndFamily = (FF_MODERN | DEFAULT_PITCH);
        break;
      }
      
      case FAM_ROMAN:  
      {
        lf.lfPitchAndFamily = (FF_ROMAN | DEFAULT_PITCH);
        break;
      }
      
      case FAM_SWISS:  
      {
        lf.lfPitchAndFamily = (FF_SWISS | DEFAULT_PITCH);
        break;
      }
      
      case FAM_SCRIPT: 
      {
        lf.lfPitchAndFamily = (FF_SCRIPT | DEFAULT_PITCH);
        break;
      }
      
      case FAM_DECOR:  
      {
        lf.lfPitchAndFamily = (FF_DECORATIVE | DEFAULT_PITCH);
        break;
      }
      
      default:         
      {
        lf.lfPitchAndFamily = (FF_DONTCARE | DEFAULT_PITCH);
        break;
      }
    } 

    /* Font height. */
    lf.lfHeight = -1 * fpFontRec->FontInfo.PointSize * 20;
  
    /* 
    ** Font attributes.  
    */
    if( fpFontRec->FontInfo.Attributes & FONT_BOLD ) 
    {
      lf.lfWeight = FW_BOLD;
    }
    else
    {
      lf.lfWeight = FW_NORMAL;
    }
    lf.lfItalic    = ( fpFontRec->FontInfo.Attributes & FONT_ITAL );
    lf.lfStrikeOut = ( fpFontRec->FontInfo.Attributes & FONT_STRK );
    lf.lfUnderline = ( fpFontRec->FontInfo.Attributes & FONT_UNDR );

    /* 
    ** Set context data for other Double underline and smallcaps font attributes.
    */

    /* Does font specify smallcaps. */
    fpContextData->bSmallCaps = ( fpFontRec->FontInfo.Attributes & FONT_SMCP );          

    /* Does font specify double underline. */ 
    fpContextData->bDoubleUnd = ( fpFontRec->FontInfo.Attributes & FONT_DBUN );         

    /* Create the font. */                                         
    hFont = CreateFontIndirect( &lf );

    /* 
    ** Put the new font into the font cache.
    */
    if( ! AddFont( hWnd, fpDisplayInfo->hFontBuffer, hFont, fpFontRec->wFontIndex ) )
    {
      /* Failure. */
      return( FALSE );
    }
  }

  /* 
  ** If we are not in a hotspot, change text foreground color
  ** to color specified in font information.
  */ 
  if( fpContextData->bInHotSpot != TRUE )
  {
    fpContextData->LastFGTextColor = fpFontRec->FontInfo.fgTextColor;         
    SetTextColor( hDC, fpContextData->LastFGTextColor );                                
  }
  
  /* Select the font that we want. */
  SelectObject( hDC, hFont);

  /* Save the font's metrics. */
  GetTextMetrics( hDC, &(fpContextData)->FontMetrics );

  /* Success. */
  return( TRUE );
}


/********************************** BORDER ROUTINES ***********************************/

/***************************************************
*
* Sets paragraph's border information.
*
****************************************************/
static void __far __pascal SetBorder
( 
  HDC hDC,
  FPTOPICELEMENT fpTopicElement,
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData
)
{
  FPBORDERREC fpBorderRec;
  short int nNumPixels;
  POINT Point;


  /* There is a border around the paragraph. */
  fpContextData->bBorder = TRUE;
  fpContextData->bShouldHaveBorder = TRUE;

  /* Lock the record's  data. */
  fpBorderRec = (FPBORDERREC) fpTopicElement->RecDataPtr;
    
  /* Get the paragraph's broder information. */
  fpContextData->BorderInfo.bTop        = fpBorderRec->bTop;
  fpContextData->BorderInfo.bLeft       = fpBorderRec->bLeft;
  fpContextData->BorderInfo.bRight      = fpBorderRec->bRight;
  fpContextData->BorderInfo.bBottom     = fpBorderRec->bBottom;
  fpContextData->BorderInfo.bThickLine  = fpBorderRec->bThickLine;
  fpContextData->BorderInfo.LineType    = fpBorderRec->LineType;

  /* Line thickness. */
  if( fpContextData->BorderInfo.bThickLine == TRUE )
  {
    /* Dotted line type - always one wide.  */
    if( fpContextData->BorderInfo.LineType == DOTTED_LINE || 
        fpContextData->BorderInfo.LineType == SHADOW_LINE
      )
    {
      nNumPixels = 1;
    }
    else nNumPixels = 2;
  }
  else
  {
    nNumPixels = 1;
  }
    
  /* Convert pixels to inches. */
  Point.x = nNumPixels;
  Point.y = 0;
  DPtoLP( hDC, &Point, 1 );
    
  /* Save line thickness. */
  fpContextData->wLineWidth = (WORD) Point.x;

  /* Double line.  */
  if( fpContextData->BorderInfo.LineType == DOUBLE_LINE )
  {
    /* Border margin is width of border. */
    fpContextData->wBorderWidth = (WORD) Point.x * 3;
  }      
  
  /* Single line. */
  else
  {
    /* Border margin is width of border. */
    fpContextData->wBorderWidth = (WORD) Point.x;
  }

  /* Calculate the page's margins. */
  CalcMargins( fpDisplayInfo, fpContextData );

  /* If we are at the top of the page, skip down a little. */
  if( fpContextData->PageYPos == 0 )
  {
    fpContextData->PageYPos = TOP_BORDER_OFFSET;
  }

  /* Set border's top and right positions. */
  fpContextData->BorderPosRect.top = fpContextData->PageYPos;
  fpContextData->BorderPosRect.right = fpDisplayInfo->DisplaySize.x - 
                                       fpContextData->RightBorderMargin;


  /* If there is a top border, move past it's vertical position. */
  if( fpContextData->BorderInfo.bTop )
  {
    fpContextData->PageYPos = fpContextData->BorderPosRect.top + fpContextData->wBorderWidth +
                          AFTER_TOP_BORDER_OFFSET;  
  }

  /* Save paragraph's left margin value. */ 
  fpContextData->ParaLeftMargin = fpContextData->LeftTxtMargin;

  /* Set new X position. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;
}


/***************************************************
*
* Continues using paragraph's border information
* to draw another border enclosed paragraph. 
*
****************************************************/
static void __far __pascal SetAnotherBorder
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData
)
{
  /* There is a border around the paragraph. */
  fpContextData->bBorder = TRUE;

  /* Calculate the page's margins. */
  CalcMargins( fpDisplayInfo, fpContextData );

  /* Set border's top and right positions. */
  fpContextData->BorderPosRect.top = fpContextData->PageYPos;

  /* If there is a top border, move past it's vertical position. */
  if( fpContextData->BorderInfo.bTop )
  {
    fpContextData->PageYPos = fpContextData->BorderPosRect.top + fpContextData->wBorderWidth +
                          AFTER_TOP_BORDER_OFFSET;  
  }

  /* Save paragraph's left margin value. */ 
  fpContextData->ParaLeftMargin = fpContextData->LeftTxtMargin;

  /* Set new X position. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;
}


/***************************************************
*
* Layout a border on the display page.
*
****************************************************/
static BOOL __far __pascal LayoutBorder
( 
  HWND hWnd,
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
)
{
  BORDERDISPLAYREC BorderDisplayRec;


  /* Set bottom position of border. */
  fpContextData->BorderPosRect.bottom = fpContextData->PageYPos + fpContextData->wBorderWidth +
                                        BOTTOM_BORDER_OFFSET;

  /* Increment y position past border. */
  fpContextData->PageYPos = fpContextData->BorderPosRect.bottom + AFTER_BOTTOM_BORDER_OFFSET;

  /* Set left position of border. */
  if( fpContextData->BorderInfo.bLeft )
  {
    /* 
    ** If there's a left border, move to the left of the margin so
    ** there will be room to draw the left border's line.
    */
    fpContextData->BorderPosRect.left = fpContextData->ParaLeftMargin -
                                        fpContextData->wBorderWidth - 
                                        LEFT_BORDER_OFFSET;
  }
  else
  {
    /* 
    ** No left border line, left edge is smallest left margin in
    ** the paragraph.
    */
    fpContextData->BorderPosRect.left = fpContextData->ParaLeftMargin;
  }                                        

  /* Save formatting info. to a display list record. */
  BorderDisplayRec.BorderInfo   = fpContextData->BorderInfo; 
  BorderDisplayRec.wLineWidth   = fpContextData->wLineWidth; 
  BorderDisplayRec.PosRect      = fpContextData->BorderPosRect; 

  /* Insert record into display list. */
  if( !InsertBorderDisplayListRec( hWnd, fpDisplayInfo->fpBorderDisplayList,
                                   &BorderDisplayRec ) )  
  {
    /* Error. */
    return( FALSE );
  }
    
  /* No more border. */
  fpContextData->bBorder = FALSE;
        
  /* Successs. */
  return( TRUE );
}


/***************************************************
*
* Draw a border on the display page.
*
****************************************************/
static BOOL __far __pascal DrawBorder
( 
  HDC  hDC,
  FPDISPLAYINFO fpDisplayInfo, 
  FPBORDERDISPLAYREC fpBorderDisplayRec
)
{
  HPEN hPen, hOldPen;
  short int nPenStyle;
  COLORREF  ColorRef;
  
  POSRECT DblRect;
  short int nDblOffset;
  

  /* Double line.  */
  if( fpBorderDisplayRec->BorderInfo.LineType == DOUBLE_LINE )
  {
    /* Offset of 2nd line. */
    nDblOffset = (short int) 2 * fpBorderDisplayRec->wLineWidth;
    
    /* 
    ** Position of 2nd lines. 
    */

    /* If there's a left line. */
    if( fpBorderDisplayRec->BorderInfo.bLeft )
    {
      /* Start top and bottom border to the left of existing left border. */
      DblRect.left = fpBorderDisplayRec->PosRect.left + nDblOffset;
    }
    else
    {
      /* Same. */
      DblRect.left = fpBorderDisplayRec->PosRect.left;
    }
    
    /* If there's a right line. */
    if( fpBorderDisplayRec->BorderInfo.bRight )
    {
      /* Stop top and bottom border to the right of existing right border. */
      DblRect.right = fpBorderDisplayRec->PosRect.right - nDblOffset;
    }
    else
    {
      /* Same. */
      DblRect.right = fpBorderDisplayRec->PosRect.right;
    }
    
    /* If there's a top line. */
    if( fpBorderDisplayRec->BorderInfo.bTop )
    {
      /* Adjust left and right side line's top position. */
      DblRect.top = fpBorderDisplayRec->PosRect.top + nDblOffset;
    }
    else
    {
      /* Same. */
      DblRect.top = fpBorderDisplayRec->PosRect.top;
    }
    
    /* If there's a bottom line. */
    if( fpBorderDisplayRec->BorderInfo.bBottom )
    {
      /* Adjust left and right side line's bottom position. */
      DblRect.bottom = fpBorderDisplayRec->PosRect.bottom - nDblOffset;
    }
    else
    {
      /* Same. */
      DblRect.bottom = fpBorderDisplayRec->PosRect.bottom;
    }
  }


  /* Line type. */
  if( fpBorderDisplayRec->BorderInfo.LineType == DOTTED_LINE )
  {
    nPenStyle = PS_DOT;
  }
  else                    
  {
    nPenStyle = PS_SOLID;
  }

  /* Line color. */
  if( fpBorderDisplayRec->BorderInfo.LineType == SHADOW_LINE )
  {
    /* Gray. */
    ColorRef = RGB( 128, 128, 128 );
  }
  else
  {
    /* Black. */
    ColorRef = RGB( 0, 0, 0 );
  }

  /* Create the pen. */
  hPen = CreatePen( nPenStyle, fpBorderDisplayRec->wLineWidth, ColorRef );
  
  /* Pen creation error. */
  if( hPen == NULL )
  {
    return FALSE;
  }
  
  hOldPen = SelectObject( hDC, hPen );

  /* 
  ** Draw top line. 
  */
  if( fpBorderDisplayRec->BorderInfo.bTop )
  {
    /* Go to beginning position of line. */
    MoveTo( hDC, (short int) (fpBorderDisplayRec->PosRect.left - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.top - fpDisplayInfo->PageYPosOffset) );
    
    /* Draw the line. */
    LineTo( hDC, (short int) (fpBorderDisplayRec->PosRect.right - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.top - fpDisplayInfo->PageYPosOffset) );

    /* Double line.  */
    if( fpBorderDisplayRec->BorderInfo.LineType == DOUBLE_LINE )
    {
      /* Go to beginning position of line. */
      MoveTo( hDC, (short int) (DblRect.left - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.top - fpDisplayInfo->PageYPosOffset) );
      
      /* Draw the line. */
      LineTo( hDC, (short int) (DblRect.right - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.top - fpDisplayInfo->PageYPosOffset) );
    }
  }
  
  /* 
  ** Draw left line. 
  */
  if( fpBorderDisplayRec->BorderInfo.bLeft )
  {
    /* Go to beginning position of line. */
    MoveTo( hDC, (short int) (fpBorderDisplayRec->PosRect.left - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.top - fpDisplayInfo->PageYPosOffset) );
    
    /* Draw the line. */
    LineTo( hDC, (short int) (fpBorderDisplayRec->PosRect.left - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.bottom - fpDisplayInfo->PageYPosOffset) );

    /* Double line.  */
    if( fpBorderDisplayRec->BorderInfo.LineType == DOUBLE_LINE )
    {
      /* Go to beginning position of line. */
      MoveTo( hDC, (short int) (DblRect.left - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.top - fpDisplayInfo->PageYPosOffset) );
      
      /* Draw the line. */
      LineTo( hDC, (short int) (DblRect.left - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.bottom - fpDisplayInfo->PageYPosOffset) );
    }
  }

  /* 
  ** Draw right line. 
  */
  if( fpBorderDisplayRec->BorderInfo.bRight )
  {
    /* Go to beginning position of line. */
    MoveTo( hDC, (short int) (fpBorderDisplayRec->PosRect.right - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.top - fpDisplayInfo->PageYPosOffset) );
    
    /* Draw the line. */
    LineTo( hDC, (short int) (fpBorderDisplayRec->PosRect.right - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.bottom - fpDisplayInfo->PageYPosOffset) );

    /* Double line.  */
    if( fpBorderDisplayRec->BorderInfo.LineType == DOUBLE_LINE )
    {
      /* Go to beginning position of line. */
      MoveTo( hDC, (short int) (DblRect.right - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.top - fpDisplayInfo->PageYPosOffset) );
      
      /* Draw the line. */
      LineTo( hDC, (short int) (DblRect.right - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.bottom - fpDisplayInfo->PageYPosOffset) );
    }
  }

  /* 
  ** Draw bottom line. 
  */
  if( fpBorderDisplayRec->BorderInfo.bBottom )
  {
    /* Go to beginning position of line. */
    MoveTo( hDC, (short int) (fpBorderDisplayRec->PosRect.left - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.bottom - fpDisplayInfo->PageYPosOffset) );
    
    /* Draw the line. */
    LineTo( hDC, (short int) (fpBorderDisplayRec->PosRect.right - fpDisplayInfo->PageXPosOffset), 
            (short int) (fpBorderDisplayRec->PosRect.bottom - fpDisplayInfo->PageYPosOffset) );

    /* Double line.  */
    if( fpBorderDisplayRec->BorderInfo.LineType == DOUBLE_LINE )
    {
      /* Go to beginning position of line. */
      MoveTo( hDC, (short int) (DblRect.left - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.bottom - fpDisplayInfo->PageYPosOffset) );
      
      /* Draw the line. */
      LineTo( hDC, (short int) (DblRect.right - fpDisplayInfo->PageXPosOffset), 
              (short int) (DblRect.bottom - fpDisplayInfo->PageYPosOffset) );
    }
  }
  

  /* Destroy the pen. */
  SelectObject( hDC, hOldPen );
  DeleteObject( hPen ); 
  
  /* Successs. */
  return( TRUE );
}


/********************************** CONTEXT DATA ROUTINES ********************************/

/***************************************************
*
* New line used when wrapping text.
*
****************************************************/
static void __far __pascal WrapToNextLine( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData )
{
  /* NOT on first line of a new paragraph. */
  fpContextData->bOnFirstLine = FALSE;

  /* Go to normal position of next line. */
  GotoNextLine( hDC, fpDisplayInfo, fpContextData );

  /* 
  ** If there was a justified picture,
  ** is the new line past it?
  */
  if( fpContextData->bLeftPicture || fpContextData->bRightPicture )
  {
    if( fpContextData->PageYPos > fpContextData->PictPosRect.bottom )
    {
      fpContextData->bRightPicture = FALSE;
      fpContextData->bLeftPicture = FALSE;
    }
  }

  /* 
  ** If there is not a left justified picture,
  ** is there one waiting to be formatted?
  */
  if( fpContextData->wUnformatPicts > 0 && 
      fpContextData->bLeftPicture == FALSE && 
      fpContextData->bRightPicture == FALSE )
  {
    /* Set current horizontal position. */
    fpContextData->PageXPos = fpContextData->LeftPictMargin;             

    /* For the next picture awaiting formatting. */
    LayoutPicture( hDC, hWnd, fpDisplayInfo, fpContextData );

    /* Calculate the new line's left margin including picture. */
    CalcMargins( fpDisplayInfo, fpContextData );

    /* Set current horizontal position to new left margin. */
    fpContextData->PageXPos = fpContextData->LeftTxtMargin;             
  }

  /* No picture. */
  else
  {
    /* Calculate the new line's margins. */
    CalcMargins( fpDisplayInfo, fpContextData );

    /* Set current horizontal position to new left margin. */
    fpContextData->PageXPos = fpContextData->LeftTxtMargin;
                 
    /* New value for paragraph's left margin ? */
    if( fpContextData->ParaLeftMargin < fpContextData->PageXPos )
    {
      /* New value. */
      fpContextData->ParaLeftMargin = fpContextData->PageXPos;
    }
  }
}


/***************************************************
*
* End of the current paragraph.
*
****************************************************/
static void __far __pascal ParagraphEnd( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData )
{
  /* Go to normal position of next line. */
  GotoNextLine( hDC, fpDisplayInfo, fpContextData );

  /* On first line of a new paragraph. */
  fpContextData->bOnFirstLine = TRUE;

  /* If there is a justified picture, go past it. */
  if( fpContextData->bLeftPicture || fpContextData->bRightPicture )
  {
    /* 
    ** If the new paragraph's line is not past the bitmap for the
    ** last paragraph, start the paragraph after the bitmap. 
    */
    if( fpContextData->PageYPos < fpContextData->PictPosRect.bottom )
    {
      fpContextData->PageYPos = fpContextData->PictPosRect.bottom;
    }
    fpContextData->bLeftPicture = FALSE;
    fpContextData->bRightPicture = FALSE;
  }


  /* 
  ** If we have any unformatted pictures,
  ** format all of them.
  */
  if( fpContextData->wUnformatPicts > 0 )
  {
    /* Set current horizontal position. */
    fpContextData->PageXPos = fpContextData->LeftPictMargin;             

    /* While we have unformatted pictures to position. */
    while( fpContextData->wUnformatPicts > 0 )
    {
      /* For the next picture awaiting formatting. */
      LayoutPicture( hDC, hWnd, fpDisplayInfo, fpContextData );
  
      /* Go past left justified picture that we just formatted. */
      fpContextData->PageYPos = fpContextData->PictPosRect.bottom;
    }

    /* No more pictures. */
    fpContextData->bLeftPicture = FALSE;
    fpContextData->bRightPicture = FALSE;
  }

  /* If there is a border around the paragraph. */
  if( fpContextData->bBorder )
  {
    /* Save the border format data. */
    LayoutBorder( hWnd, fpDisplayInfo, fpContextData );
  }
  
  /* Add vertical spacing after paragraph. */
  fpContextData->PageYPos = fpContextData->PageYPos + fpContextData->SpaceAfterParagraph;
  
  /* Add vertical spacing before next paragraph. */
  fpContextData->PageYPos = fpContextData->PageYPos + fpContextData->SpaceBeforeParagraph;
  fpContextData->SpaceAddedBeforeParagraph = fpContextData->SpaceBeforeParagraph;
  
  /* Calculate the margins of the new paragraph's first line. */
  CalcMargins( fpDisplayInfo, fpContextData );

  /* Set current horizontal position to new left margin. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;             
}


/***************************************************
*
* At new paragraph?
*
****************************************************/
static BOOL __far __pascal AtNewParagraph( FPCONTEXTDATA fpContextData ) 
{
  /* On first line of a new paragraph. */
  if( fpContextData->bOnFirstLine == TRUE )
  {
    return( fpContextData->PageXPos == fpContextData->LeftTxtMargin );
  }
  
  /* No. */
  return( FALSE );
}



/***************************************************
*
* Default way to calculate vertical starting
* position of next output line.
*
****************************************************/
static void __far __pascal GotoNextLine( HDC hDC, FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData )
{
  WORD wLinesHeight;

  /* 
  ** Justify current line ( move's line's display records horizontally ). 
  */
  JustifyCurrentLine( fpDisplayInfo, fpContextData );

  /* 
  ** Align each element in the current line to the same baseline 
  ** ( move's line's display records vertically ). 
  */
  wLinesHeight = BaselineAlign( fpDisplayInfo, fpContextData );


  /* Set next line's first text display record number. */
  fpContextData->wFirstTxtDisplayRec = fpContextData->wLastTxtDisplayRec;

  /* 
  ** Set current horizontal position in topic's page to next line. 
  */
  
  /* If the vertical spacing value is negative. */
  if( fpContextData->VertSpace < 0 )
  {
    fpContextData->PageYPos = fpContextData->PageYPos + ( -1 * fpContextData->VertSpace );
  }
  
  /* Vertical spacing value is not negative. */
  else
  {
    /* 
    ** If we have no height value and have no vertical 
    ** spacing value, use selected font's height. 
    */
    if( wLinesHeight == 0 && fpContextData->VertSpace == 0 )
    {
      /* Use height of the selected font. */
      fpContextData->PageYPos = fpContextData->PageYPos + (fpContextData->FontMetrics.tmHeight + 
                            fpContextData->FontMetrics.tmExternalLeading);
    }

    /* If we have a vertical spacing value is greater than our last output line. */
    else if( (WORD) fpContextData->VertSpace > wLinesHeight )
    {
      fpContextData->PageYPos = fpContextData->PageYPos + fpContextData->VertSpace;
    }
    
    /* Otherwise use last output line's height. */
    else
    {
      fpContextData->PageYPos = fpContextData->PageYPos + wLinesHeight;
    }
  }
}


/***************************************************
*
* Move the horizontal position of the line's display 
* records so that they will be draw at positions that 
* are correct for the type of justification that is 
* set.
*
****************************************************/
static void __far __pascal JustifyCurrentLine( FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData )
{
  TXTDISPLAYREC TxtDispRecData;
  WORD wSlideAmount;

  if( fpContextData->JustifyType == CENTER_JUSTIFIED || fpContextData->JustifyType == RIGHT_JUSTIFIED )
  {
    /* Nothing displayed on the line. */
    if( fpContextData->wFirstTxtDisplayRec == fpContextData->wLastTxtDisplayRec ) return;
  
    /* Get the position of the last display record in the current line. */
    if( ! GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, fpContextData->wLastTxtDisplayRec - 1, 
                            &TxtDispRecData ) ) return;
    
    /* If there is unused horizontal space, justify line's output using it. */
    if( TxtDispRecData.PosRect.right < (fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin) )
    {
      /* Calculate unused space in the window. */
      wSlideAmount = (WORD) ( fpDisplayInfo->DisplaySize.x - fpContextData->RightTxtMargin - TxtDispRecData.PosRect.right);
    
      /* If centering line, cut slide amount in half. */
      if( fpContextData->JustifyType == CENTER_JUSTIFIED )
      {
        wSlideAmount = wSlideAmount / 2;
      }
  
      /* Increment the position of all display records for that line. */
      MoveTxtDisplayRecHortPos( fpDisplayInfo->fpTxtDisplayList, fpContextData->wFirstTxtDisplayRec,
                             fpContextData->wLastTxtDisplayRec, wSlideAmount );
    }
  }
}
  

/***************************************************
*
* Move the vertical position of the line's 
* display records so that they will be drawn on the
* same baseline.
*
****************************************************/
static WORD __far __pascal BaselineAlign( FPDISPLAYINFO fpDisplayInfo, FPCONTEXTDATA fpContextData )
{
  TXTDISPLAYREC TextDisplayRec;
  WORD wCurrentRec;

  WORD wBaseLineHeight;     /* Tallest baseline height value. */
  WORD wLineHeight;         /* Height of line. */
  
  WORD wAdjustment;         /* Amount to adjust the vertical position of a
                               display record for the new baseline. */
  
  WORD wRecordHeight;       /* Height of display record after baseline adjustment. */
  

  /* Nothing displayed on the line. */
  if( fpContextData->wFirstTxtDisplayRec == fpContextData->wLastTxtDisplayRec )
  {
    /* 0 height for line. */
    return 0;
  }
  
  /* Init. */
  wBaseLineHeight = 0;
  wLineHeight     = 0;
  
  /* Find display record in line with the tallest ascent. */
  for( wCurrentRec = fpContextData->wFirstTxtDisplayRec; wCurrentRec < fpContextData->wLastTxtDisplayRec; wCurrentRec++ )
  {
    /* Retrieve display record in the current line. */
    if( ! GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrentRec, &TextDisplayRec ) ) 
    {
      return( wLineHeight );
    }                       
    
    /* New tallest ascent? */
    if( TextDisplayRec.wBaseLineHeight > wBaseLineHeight ) 
    {
      wBaseLineHeight = TextDisplayRec.wBaseLineHeight;
    }
  }
  

  /* Calculate line's tallest place and adjust display records to baseline. */
  for( wCurrentRec = fpContextData->wFirstTxtDisplayRec; wCurrentRec < fpContextData->wLastTxtDisplayRec; wCurrentRec++ )
  {
    /* Retrieve display record in the current line. */
    if( ! GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrentRec, &TextDisplayRec ) ) 
    {
      return( wLineHeight );
    }                       
    
    /* If it needs adjustment to new baseline. */
    if( TextDisplayRec.wBaseLineHeight < wBaseLineHeight ) 
    {
      /* Amount to move it down to baseline. */
      wAdjustment = wBaseLineHeight - TextDisplayRec.wBaseLineHeight;

      /* Move it down. */
      TextDisplayRec.PosRect.top    = TextDisplayRec.PosRect.top + wAdjustment;
      TextDisplayRec.PosRect.bottom = TextDisplayRec.PosRect.bottom + wAdjustment;

      /* 
      ** Update the record's information. 
      */
      if( !UpdateTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrentRec, &TextDisplayRec ) )  
      {
        return( wLineHeight );
      }                       
    }

    /* Display record's height in line ( includes tmDecent ). */
    wRecordHeight = (WORD) (TextDisplayRec.PosRect.bottom - fpContextData->PageYPos);
    
    /* Update line's height. */
    if( wRecordHeight > wLineHeight )
    {
      wLineHeight = wRecordHeight;
    }
  }

  /* Return height of line's tallest display record. */
  return( wLineHeight );
}



/***************************************************
*
* Initializes the context data.
*
****************************************************/
static void __far __pascal InitializeContextData
( 
  HDC hDC, 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
)
{
  /* Current line's display record info. */
  fpContextData->wFirstTxtDisplayRec = 0;
  fpContextData->wLastTxtDisplayRec = 0;

  /* Picture info. */
  fpContextData->wUnformatPicts = 0;
  fpContextData->wNextUnformatRec = 0;
  fpContextData->bLeftPicture = FALSE;
  fpContextData->bRightPicture = FALSE;

  /* Border info. */
  fpContextData->ParaLeftMargin = 0;
  
  /* Space already added before next paragraph. */
  fpContextData->SpaceAddedBeforeParagraph = 0;
  
  /* We are not in a table. */
  fpContextData->bInTable = FALSE;

  /* Reset paragraph oriented context data. */
  ResetPageData( hDC, fpDisplayInfo, fpContextData );

  /* Initial x/y formatting position. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;           
  fpContextData->PageYPos = 0;                  
}



/***************************************************
*
* Resets page oriented context data back to
* the defaults settings.
*
* Called when context data is initialized and 
* after a TE_PARD record has been encountered.
*
* A TE_PARD record does not signify the end of
* a topic and there can be multiple TE_PARD records
* in a topic.
*
****************************************************/
static void __far __pascal ResetPageData
( 
  HDC hDC, 
  FPDISPLAYINFO fpDisplayInfo, 
  FPCONTEXTDATA fpContextData 
)
{
  /* Vert. line spacing. */
  fpContextData->VertSpace = 0;             /* No user-defined spacing. */
  fpContextData->SpaceBeforeParagraph = 0;  /* Vertical space before a paragraph. */
  fpContextData->SpaceAfterParagraph = 0;   /* Vertical space after a paragraph. */
  fpContextData->bSBAlreadyAdded = FALSE;   /* Not added space before amount. */
  
  /* On first line of a new paragraph. */
  fpContextData->FirstLineIndent = 0;       /* Indent at start of new paragraph. */
  fpContextData->bOnFirstLine = TRUE;

  /* Calculated left text margin. */
  fpContextData->wDefaultLeftIndent = DEFAULT_LEFT_MARGIN;    /* Default left indention. */
  fpContextData->LeftIndent  = 0;             /* No user-defined left indention. */

  /* Calculated right text margin. */
  fpContextData->wDefaultRightIndent = DEFAULT_RIGHT_MARGIN;   /* Default right indention. */
  fpContextData->RightIndent = 0;             /* No user-defined right indention. */

  /* Text attributes. */
  fpContextData->bSmallCaps = FALSE;         /* Font does not specify smallcaps. */ 
  fpContextData->bDoubleUnd = FALSE;         /* Font does not specify double underline. */ 
  fpContextData->CharSet = ANSI_CHARSET;     /* Documentation. says Windows character set is
                                                used by default, but I'm not sure
                                                what that is... */ 
  fpContextData->LastFGTextColor = COLOR_BLACK; /* Default text color. */

  /* Hotspot info. */
  ReadINIHotSpotColor( &(fpContextData)->HotSpotColor ); /* Color to draw hotspot text. */
  fpContextData->bInHotSpot = FALSE;                     /* Are we currently in a hotspot. */

  /* Tab info. */
  fpContextData->wCustomTabCount = 0;              /* No custom tab stops. */

  /* Justification type. */
  fpContextData->JustifyType = LEFT_JUSTIFIED;     /* Displayed text/in-line bitmaps are 
                                                      left justified. */
  /* Line wrapping. */
  fpContextData->bWordWrap = TRUE;       /* Wrap lines too long for window. */

  /* Drawing border. */
  fpContextData->wBorderWidth = 0;           /* No border around paragraph. */
  fpContextData->bBorder = FALSE;            /* No border around paragraph. */
  fpContextData->bShouldHaveBorder = FALSE;  /* No border around paragraph. */

  /* Calculate the new line's margins. */
  CalcMargins( fpDisplayInfo, fpContextData );
}


/***************************************************
*
* Line wrapping.
*
****************************************************/
static void __far __pascal SetWordWrappingOff
( 
  FPCONTEXTDATA fpContextData
)
{
  /* Set wrapping off. */
  fpContextData->bWordWrap = FALSE;             
}


/***************************************************
*
* Set the left indention value and calculates the
* left margin.
*
****************************************************/
static void __far __pascal SetLeftIndent
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPLIREC fpLIRec;

  /* Lock the record. */
  fpLIRec = (FPLIREC) fpTopicElement->RecDataPtr;

  /* Set value. */
  fpContextData->LeftIndent = fpLIRec->LeftIndent;

  /* Calculate the new line's margins. */
  CalcMargins( fpDisplayInfo, fpContextData );
  
  /* Set x position to left margin. */
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;             
}


/***************************************************
*
* Set the indention value for first line of
* a new paragraph.
*
****************************************************/
static void __far __pascal SetFirstLineIndent
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPFIREC fpFIRec;

  /* Lock the record. */
  fpFIRec = (FPFIREC) fpTopicElement->RecDataPtr;

  /* Set value. */
  fpContextData->FirstLineIndent = fpFIRec->FirstLineIndent;

  /* Calculate the new line's margins. */
  CalcMargins( fpDisplayInfo, fpContextData );
  
  /* Set current x position. */                                                   
  fpContextData->PageXPos = fpContextData->LeftTxtMargin;             
}


/***************************************************
*
* Calculates the pages margins given the
* known objects on the page
*
****************************************************/
static void __far __pascal CalcMargins
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
)
{
  /* 
  ** Calculate the margins from the 
  ** outside edges of the page inward.
  */

  /* 
  ** Margins for tables are only calculated
  ** when we get a TABLECOLUMN element record.
  */
  
  /* Calc. border margins. */
  CalcBorderMargins( fpContextData );
  
  /* Calc. picture margins. */
  CalcPictMargins( fpContextData );
  
  /* Calc. text margins. */
  CalcLeftTxtMargin( fpContextData );
  CalcRightTxtMargin( fpContextData );
}


/***************************************************
*
* Calculates the table margins.
*
****************************************************/
static void __far __pascal CalcTableMargins
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPCONTEXTDATA fpContextData 
)
{
  COLUMNPOSPTR ColumnPosPtr;
  long int LastWidth, RightPos;
  

  /* In a table. */
  if( fpContextData->bInTable )
  {
    /* Get table's column positions. */
    ColumnPosPtr = fpContextData->ColumnPosPtr; 
    
    /* If there was a previous column. */
    if( fpContextData->wCurrColumn > 0 )
    {
      /* 
      ** Move to last column's position data. 
      */
      ColumnPosPtr = ColumnPosPtr + (fpContextData->wCurrColumn - 1);
      
      /* Get last column's width. */
      LastWidth = ColumnPosPtr->Width;
    }
    else
    {
      LastWidth = 0;
    }
    
    /* Get table's column positions. */
    ColumnPosPtr = fpContextData->ColumnPosPtr; 
    
    /* 
    ** Move to current columns position data. 
    */
    ColumnPosPtr = ColumnPosPtr + fpContextData->wCurrColumn;

    /* Calculate left margin for new column using previous column's right margin. */
    fpContextData->TableLeftMargin = fpContextData->TableLeftMargin +
                                     LastWidth +
                                     ColumnPosPtr->SpaceBetween;
    
    /* 
    ** If left column margin is less than the 
    ** default left margin, use the default left margin. 
    */
    if( (long) fpContextData->wDefaultLeftIndent > fpContextData->TableLeftMargin )
    {
      fpContextData->TableLeftMargin = (long) fpContextData->wDefaultLeftIndent;
    }

    /* 
    ** Get the distance of the column's right edge 
    ** from the page's left edge. 
    */
    RightPos = fpContextData->TableLeftMargin + ColumnPosPtr->Width;
    
    /* 
    ** Calculate the margin from the page's right edge 
    ** to the column's right edge. 
    */
    fpContextData->TableRightMargin = fpDisplayInfo->DisplaySize.x - RightPos;
  }
  
  /* Not in a table. */
  else
  {
    fpContextData->TableLeftMargin = 0;
    fpContextData->TableRightMargin = 0;
  }
}



/***************************************************
*
* Calculates the border margins.
*
****************************************************/
static void __far __pascal CalcBorderMargins
( 
  FPCONTEXTDATA fpContextData 
)
{
  /* In a table. */
  if( fpContextData->bInTable )
  {
    /* Borders start at the left and right margins of the table's current column. */
    fpContextData->LeftBorderMargin = fpContextData->TableLeftMargin;
    fpContextData->RightBorderMargin = fpContextData->TableRightMargin;
  }

  /* Not in a table. */
  else
  {
    /* Borders start at the default margins. */
    fpContextData->LeftBorderMargin = fpContextData->wDefaultLeftIndent;
    fpContextData->RightBorderMargin = fpContextData->wDefaultRightIndent;
  }
}



/***************************************************
*
* Calculates the picture margins.
*
****************************************************/
static void __far __pascal CalcPictMargins
( 
  FPCONTEXTDATA fpContextData 
)
{
  /* If there is a left border. */
  if( fpContextData->bBorder && fpContextData->BorderInfo.bLeft )
  {
    /* 
    ** Assign an offset value so that there will be space 
    ** between the left border and any object next to it.
    */
    fpContextData->LeftPictMargin = fpContextData->LeftBorderMargin + 
                                    fpContextData->wBorderWidth + 
                                    LEFT_BORDER_OFFSET;
  }

  /* No left border. */
  else
  {
    /* In a table. */
    if( fpContextData->bInTable )
    {
      /* Pictures start at the left margin of the table's current column. */
      fpContextData->LeftPictMargin = fpContextData->TableLeftMargin;
    }
    else
    {
      /* Pictures start at the default left margin. */
      fpContextData->LeftPictMargin = fpContextData->wDefaultLeftIndent;
    }
  }
      

  /* If there is a right border. */
  if( fpContextData->bBorder && fpContextData->BorderInfo.bRight )
  {
    /* 
    ** Assign an offset value so that there will be space
    ** between the right border and any object next to it.
    */
    fpContextData->RightPictMargin = fpContextData->RightBorderMargin +
                                     fpContextData->wBorderWidth + 
                                     RIGHT_BORDER_OFFSET;
  }

  /* No right border. */
  else
  {
    /* In a table. */
    if( fpContextData->bInTable )
    {
      /* Pictures start at the right margin of the table's current column. */
      fpContextData->RightPictMargin = fpContextData->TableRightMargin;
    }
    /* Not in a table. */
    else
    {
      /* Pictures start at the default right margin. */
      fpContextData->RightPictMargin = fpContextData->wDefaultRightIndent;
    }
  }
}


/***************************************************
*
* Calculates the left margin for text.
*
****************************************************/
static void __far __pascal CalcLeftTxtMargin
( 
  FPCONTEXTDATA fpContextData
)
{
  short int nPictWidth;

  /* 
  ** If there is a left justified picture,
  ** adjust margin appropriately.
  */
  if( fpContextData->bLeftPicture == TRUE )
  {
    /* Get picture width + its offset from page edge. */
    nPictWidth = (short int)(fpContextData->PictPosRect.right - fpContextData->PictPosRect.left);

    /* Calculated left margin. */
    fpContextData->LeftTxtMargin = fpContextData->LeftPictMargin +
                                   nPictWidth +
                                   fpContextData->wDefaultLeftIndent +
                                   fpContextData->LeftIndent;
  }


  /* No left picture. */
  else
  {
    /* If there is a left border. */
    if( fpContextData->bBorder && fpContextData->BorderInfo.bLeft )
    {
      /* Calculated left margin including border margin. */
      fpContextData->LeftTxtMargin = fpContextData->LeftBorderMargin + 
                                     fpContextData->wBorderWidth + 
                                     LEFT_BORDER_OFFSET +
                                     fpContextData->LeftIndent;
    }
  
    /* If there is no left border. */
    else
    {
      /* In a table. */
      if( fpContextData->bInTable )
      {
        /* 
        ** Calculated left margin using the left margin of 
        ** the table's current column. 
        */
        fpContextData->LeftTxtMargin = fpContextData->TableLeftMargin + 
                                       fpContextData->LeftIndent;
      }
      /* Not in a table. */
      else
      {
        /* Calculated left margin. */
        fpContextData->LeftTxtMargin = fpContextData->wDefaultLeftIndent + 
                                       fpContextData->LeftIndent;
      }
    }
  }
  

  /* 
  ** If on first line of paragraph,
  ** add in first line indention.
  */
  if( fpContextData->bOnFirstLine )
  {
    fpContextData->LeftTxtMargin = fpContextData->LeftTxtMargin + 
                                   fpContextData->FirstLineIndent;
  }
}


/***************************************************
*
* Calculates the right margin for text.
*
****************************************************/
static void __far __pascal CalcRightTxtMargin
( 
  FPCONTEXTDATA fpContextData
)
{
  short int nPictWidth;

  /* 
  ** If there is a right justified picture,
  ** adjust margin appropriately.
  */
  if( fpContextData->bRightPicture == TRUE )
  {
    /* Get picture width + its offset from page edge. */
    nPictWidth = (short int) (fpContextData->PictPosRect.right - 
                 fpContextData->PictPosRect.left);

    /* Calculated right margin. */
    fpContextData->RightTxtMargin = fpContextData->RightPictMargin +
                                    nPictWidth +
                                    fpContextData->wDefaultRightIndent +
                                    fpContextData->RightIndent;
  }


  /* No right picture. */
  else
  {
    /* If there is a left border. */
    if( fpContextData->bBorder && fpContextData->BorderInfo.bRight )
    {
      /* Calculated right margin including border margin. */
      fpContextData->RightTxtMargin = fpContextData->RightBorderMargin + 
                                      fpContextData->wBorderWidth + 
                                      LEFT_BORDER_OFFSET +
                                      fpContextData->RightIndent;
    }
  
    /* If there is no border. */
    else
    {
      /* In a table. */
      if( fpContextData->bInTable )
      {
        /* 
        ** Calculated left margin using the left margin of 
        ** the table's current column. 
        */
        fpContextData->RightTxtMargin = fpContextData->TableRightMargin + 
                                        fpContextData->RightIndent;
      }
      /* Not in a table. */
      else
      {
        /* Calculated right margin. */
        fpContextData->RightTxtMargin = fpContextData->wDefaultRightIndent + 
                                        fpContextData->RightIndent;
      }
    }
  }
}


/***************************************************
*
* Set the right indention value and calculates the
* right margin.
*
****************************************************/
static void __far __pascal SetRightIndent
( 
  FPDISPLAYINFO fpDisplayInfo,
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPRIREC fpRIRec;

  /* Lock the record. */
  fpRIRec = (FPRIREC) fpTopicElement->RecDataPtr;

  /* Set value. */
  fpContextData->RightIndent = fpRIRec->RightIndent;

  /* Calculate the new line's margins. */
  CalcMargins( fpDisplayInfo, fpContextData );
}


/***************************************************
*
* Set the value of the vertical spacing after a new 
* paragraph.
*
****************************************************/
static void __far __pascal SetSpaceAfterParagraph
( 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPSAREC fpSARec;

  /* Lock the record. */
  fpSARec = (FPSAREC) fpTopicElement->RecDataPtr;

  /* Set line spacing. */
  fpContextData->SpaceAfterParagraph = fpSARec->SpaceAfterParagraph;
}




/***************************************************
*
* Set the value of the vertical spacing before a new 
* paragraph.
*
****************************************************/
static void __far __pascal SetSpaceBeforeParagraph
( 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPSBREC fpSBRec;

  /* Lock the record. */
  fpSBRec = (FPSBREC) fpTopicElement->RecDataPtr;

  /* If we are setting "space before" value and SpaceAddedBeforeParagraph
  ** value is not zero than we'll assume:
  ** 
  ** 1. There is already a paragraph before the next paragraph. 
  ** 2. We have already added SpaceAddedBeforeParagraph value to the
  **    y page coordinate. 
  **
  ** If this is the case, we should remove the SpaceAddedBeforeParagraph
  ** added after ending the last paragraph and use the new
  ** "space before" value instead.
  */ 
  if( fpContextData->SpaceAddedBeforeParagraph != 0 )
  {
    /* Decrement the vertical position. */
    fpContextData->PageYPos = fpContextData->PageYPos - fpContextData->SpaceAddedBeforeParagraph;
  }
  
  /* Set line spacing. */
  fpContextData->SpaceBeforeParagraph = fpSBRec->SpaceBeforeParagraph;
  
  /* Increment the vertical position now for current paragraph. */
  fpContextData->PageYPos = fpContextData->PageYPos + fpContextData->SpaceBeforeParagraph;
}


/***************************************************
*
* Sets vertical line spacing within a paragraph.
*
****************************************************/
static void __far __pascal SetVertLineSpace( FPTOPICELEMENT fpTopicElement, FPCONTEXTDATA fpContextData )
{
  FPSLREC fpSLRec;

  /* Lock the record's  data. */
  fpSLRec = (FPSLREC) fpTopicElement->RecDataPtr;
    
  /* Set line spacing. */
  fpContextData->VertSpace = fpSLRec->VertLineSpace;
}


/***************************************************
*
* Moves output position one tab space.
*
****************************************************/
static void __far __pascal InsertTab( FPCONTEXTDATA fpContextData )
{
  WORD wCounter;
  long int OldPosition;
  long int NewPosition;
  TABSTOPPTR TabStopPtr;       /* Pointer to buffer of custom tab stop positions. */  
  BOOL bFoundNextTab;

  /* If there are custom tab stops defined. */
  if( fpContextData->wCustomTabCount > 0 )
  {
    /* If our current position is positive, use it. */
    if( fpContextData->PageXPos >= 0 )
    {
      OldPosition = fpContextData->PageXPos;
    }
    /* Otherwise, act like we are at zero. */
    else 
    {
      OldPosition = 0;
    }
    
    /* Haven't found a tab. */
    bFoundNextTab = FALSE;
    
    /* Get first tab stop. */
    TabStopPtr = fpContextData->TabStopPtr;
    
    /* See if the next tab stop is a custom tab stop. */
    for( wCounter = 0; wCounter < fpContextData->wCustomTabCount; wCounter++ )
    {
      /* Calculate position of custom tab stop. */
      NewPosition = *TabStopPtr + fpContextData->wDefaultLeftIndent;
        
      /* Found next position. */
      if( NewPosition > OldPosition )
      {
        /* Found a tab. */
        bFoundNextTab = TRUE;
        break;
      }
      
      /* Goto next custom tab stop in buffer. */
      TabStopPtr++;
    }
  
    /* If we have a new position. */
    if( bFoundNextTab )
    {
      /* Move to that position. */
      fpContextData->PageXPos = NewPosition;
      return;
    }
  }

  /* 
  ** Default - If current output position is negative.
  */
  if( fpContextData->PageXPos < 0 )
  {
    /* If the current position is less than -1 inch. */
    if( fpContextData->PageXPos < -(ONE_INCH) )
    {
      /* Tab position is always -1 inch. */
      NewPosition = -(ONE_INCH) + fpContextData->wDefaultLeftIndent;
    }

    /* Else, tab position is positive 1/2 inch. */
    else
    {
      NewPosition = HALF_INCH + fpContextData->wDefaultLeftIndent;
    }
  }
  
  /* 
  ** Default - If current output position is positive.
  */
  else
  {
    /* Init. */
    NewPosition = 0;
    
    /* Find the next HALF_INCH default tab position. */
    for( wCounter = 1; NewPosition <= fpContextData->PageXPos; wCounter++ )
    {
      NewPosition = ( HALF_INCH * wCounter ) + fpContextData->wDefaultLeftIndent;
    }
  }

  /* Update the horizontal position. */
  fpContextData->PageXPos = NewPosition; 
}


/***************************************************
*
* Sets the current custom tab stops.
*
****************************************************/
static void __far __pascal SetCustomTabStops
( 
  FPTOPICELEMENT fpTopicElement,
  FPCONTEXTDATA fpContextData
)
{
  FPTXREC fpTXRec;

  /* Lock the record's  data. */
  fpTXRec = (FPTXREC) fpTopicElement->RecDataPtr;
    
  /* Save tab stop information to context data. */
  fpContextData->wCustomTabCount = fpTXRec->wTabCount;
  fpContextData->TabStopPtr      = fpTXRec->TabStopsPtr;
}



/***************************************************
*
* Sets the current justification.
*
****************************************************/
static void __far __pascal SetJustification
( 
  FPCONTEXTDATA fpContextData,
  JUSTIFY_TYPE JustifyType
)
{
  /* Save the type. */
  fpContextData->JustifyType = JustifyType;
}



/********************************** SCROLL BAR ROUTINES ***********************************/

/***************************************************
*
* Add a scroll bar to the window if needed
* and update the scroll bar data structure's info.
*
* This function is called each time the display 
* page is reformatted to fit in the window.
*
****************************************************/
static void __far __pascal ShowScrollBars( HWND hWnd, FPDISPLAYINFO fpDisplayInfo )
{
  /* 
  ** Is there are no scroll bars, 
  ** see if we need to add any. 
  */
  if( fpDisplayInfo->ScrollData.bVertScroll == FALSE &&
      fpDisplayInfo->ScrollData.bHorzScroll == FALSE )
  {
    /* If page is wider than the screen. */
    if( fpDisplayInfo->dwPageWidth > (DWORD) fpDisplayInfo->DisplaySize.x )
    {
      /* There is a horizontal scroll bar. */
      fpDisplayInfo->ScrollData.bHorzScroll = TRUE;   

      /* Show scroll bar. */
      ShowScrollBar( hWnd, SB_HORZ, TRUE );

      /* 
      ** Stop for now and see how the addition 
      ** of the scroll bar impacts page formatting. 
      */
      return;
    }
   
    /* If page is longer than the screen. */
    if( fpDisplayInfo->dwPageHeight > (DWORD) fpDisplayInfo->DisplaySize.y )
    {
      /* There is a vertical scroll bar. */
      fpDisplayInfo->ScrollData.bVertScroll = TRUE;   

      /* Show scroll bar. */
      ShowScrollBar( hWnd, SB_VERT, TRUE );

      /* 
      ** Stop for now and see how the addition 
      ** of the scroll bar impacts page formatting. 
      */
      return;
    }
  }

  /* 
  ** If we added a horizontal scroll bar, 
  ** its addition may force us to add a
  ** vertical scroll bar.
  */
  if( fpDisplayInfo->ScrollData.bHorzScroll == TRUE )
  {
    /* 
    ** If we don't already have the vertical scroll bar
    ** see if we need to add it. 
    */
    if( fpDisplayInfo->ScrollData.bVertScroll == FALSE )
    {
      /* If page is longer than the screen. */
      if( fpDisplayInfo->dwPageHeight > (DWORD) fpDisplayInfo->DisplaySize.y )
      {
        /* There is a vertical scroll bar. */
        fpDisplayInfo->ScrollData.bVertScroll = TRUE;   
  
        /* Show scroll bar. */
        ShowScrollBar( hWnd, SB_VERT, TRUE );
  
        /* 
        ** Stop for now and see how the addition 
        ** of the scroll bar impacts page formatting. 
        */
        return;
      }
    }
  }


  /* 
  ** If we added a vertical scroll bar, 
  ** its addition may force us to add a
  ** horizontal scroll bar.
  */
  if( fpDisplayInfo->ScrollData.bVertScroll == TRUE )
  {                             
    /* 
    ** If we don't already have the horizontal scroll bar
    ** see if we need to add it. 
    */
    if( fpDisplayInfo->ScrollData.bHorzScroll == FALSE )
    {
      /* If page is longer than the screen. */
      if( fpDisplayInfo->dwPageWidth > (DWORD) fpDisplayInfo->DisplaySize.x )
      {
        /* There is a vertical scroll bar. */
        fpDisplayInfo->ScrollData.bHorzScroll = TRUE;   
  
        /* Show scroll bar. */
        ShowScrollBar( hWnd, SB_HORZ, TRUE );
  
        /* 
        ** Stop for now and see how the addition 
        ** of the scroll bar impacts page formatting. 
        */
        return;
      }
    }
  }

  /* 
  ** If we got down to here then we've added
  ** all of the scroll bars that we need to add.
  */
}


/***************************************************
*
* Hides the window's scroll bars and updates
* the scroll bar data structure's info.
*
* This function is called the first time that
* the window is painted and each time that the window 
* is resized.
*
****************************************************/
static void __far __pascal HideScrollBars( HWND hWnd, FPSCROLLDATA fpScrollData )
{
  /* No vertical scroll bar. */
  fpScrollData->bVertScroll = FALSE;   

  /* No horizontal scroll bar. */
  fpScrollData->bHorzScroll = FALSE;  

  /* Hide both scroll bars. */
  ShowScrollBar( hWnd, SB_BOTH, FALSE );
}


/***************************************************
*
* Sets the window's scroll bar ranges and updates
* the scroll bar data structure's info.  
*
* This function is called the first time that
* the window is painted and each time that the window 
* is resized.
*
****************************************************/
static void __far __pascal SetScrollBarValues( HWND hWnd, FPDISPLAYINFO fpDisplayInfo ) 
{
  /* Define how much space we skip on up/down line. */
  #define VERT_LINE  240

  /* If there is a vertical scroll bar. */
  if( fpDisplayInfo->ScrollData.bVertScroll == TRUE )
  {
    /* 
    ** Use constant value for how much to skip on one 
    ** line up/down.
    **
    ** The exception to this is if the height of the 
    ** window is smaller than the constant value. 
    */
    if( (WORD) fpDisplayInfo->DisplaySize.y < VERT_LINE )
    {
      fpDisplayInfo->ScrollData.wVertLine = fpDisplayInfo->DisplaySize.y;
    }
    else
    {
      fpDisplayInfo->ScrollData.wVertLine = VERT_LINE;
    }

    /* 
    ** When we page down, leave some of the previous page
    ** visible so the reference won't be lost. 
    */
    fpDisplayInfo->ScrollData.wVertPage = (WORD) ( fpDisplayInfo->DisplaySize.y * 2/3 )
                                          / fpDisplayInfo->ScrollData.wVertLine;
    /* 
    ** Allow a half screen of blank space in the window after the
    ** last page.
    */
    fpDisplayInfo->ScrollData.wMaxVertPos = (WORD)
        (fpDisplayInfo->dwPageHeight / fpDisplayInfo->ScrollData.wVertLine )
        - ( (fpDisplayInfo->DisplaySize.y / 2) / fpDisplayInfo->ScrollData.wVertLine );


    /* If forced to use current scroll position value. */
    if( fpDisplayInfo->bUseVertScrollPos )
    {
      /* No more forcing. */
      fpDisplayInfo->bUseVertScrollPos = FALSE;
    }
    
    else
    {
      /* 
      ** If last vertical position was not zero. 
      */
      if( fpDisplayInfo->ScrollData.wVertPos != 0 )
      {
        /* Set position on current screen. */
        fpDisplayInfo->ScrollData.wVertPos = (WORD) (fpDisplayInfo->dwTopDisplayRecVertPos / 
                                             fpDisplayInfo->ScrollData.wVertLine);
      }
      else
      {
        /* At the top. */
        fpDisplayInfo->ScrollData.wVertPos = 0;
      }
    }
    
    /* Set scroll range and position. */
    SetScrollRange( hWnd, SB_VERT, 0, fpDisplayInfo->ScrollData.wMaxVertPos, FALSE );
    SetScrollPos( hWnd, SB_VERT, fpDisplayInfo->ScrollData.wVertPos, TRUE );
  }
  
  /* No vertical scroll bar. */
  else
  {
    /* At top of the page. */
    fpDisplayInfo->ScrollData.wVertPos = 0;
  }
  

  /* Is there a horizontal scroll bar? */
  if( fpDisplayInfo->ScrollData.bHorzScroll == TRUE )
  {
    /* 
    ** Horizontal position on the page doesn't change 
    ** from one resize to the next.  Therefore, no change
    ** to fpDisplayInfo->ScrollData.wHorzPos.
    */
    
    /* Scroll right/left 1/10 of display width. */
    fpDisplayInfo->ScrollData.wHorzLine = fpDisplayInfo->DisplaySize.x / 10;

    /* Scroll right pg./left pg. 1/3 of display width. */
    fpDisplayInfo->ScrollData.wHorzPage = (WORD) (fpDisplayInfo->DisplaySize.x / 3 )
                                          / fpDisplayInfo->ScrollData.wHorzLine;
                                          
    /* Leave 1/2 of screen blank space on the right side. */
    fpDisplayInfo->ScrollData.wMaxHorzPos = (WORD)
      (fpDisplayInfo->dwPageWidth / fpDisplayInfo->ScrollData.wHorzLine) - 5;

    /* Set scroll range and position. */
    SetScrollRange( hWnd, SB_HORZ, 0, fpDisplayInfo->ScrollData.wMaxHorzPos, TRUE );
  }
  
  /* No horizontal scroll bar. */
  else
  {
    /* At top of the page. */
    fpDisplayInfo->ScrollData.wHorzPos = 0;
  }

  /* Set the page offset position. */
  CalcPagePosOffset( fpDisplayInfo );
}


/***************************************************
*
* Updates the window's scroll bar position and
* update the scroll bar data structure's info.
*
****************************************************/
void __far __pascal UpdateScrollBar( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam ) 
{
  HGLOBAL hDisplayInfo;           /* Window's display info. */
  FPDISPLAYINFO fpDisplayInfo;    /* Pointer to window's display info. */
  
  BOOL bUpdated;                  /* Has position been updated. */

  short int nYScrollWnd, nXScrollWnd;  /* Amount to scroll the window. */
   
  WORD wLastPos;                  /* Vert. or Horz. position before scroll. */
  
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* Position not updated. */
  bUpdated = FALSE;

  /* Scroll window. */
  nXScrollWnd = 0;
  nYScrollWnd = 0;


  switch( Msg )
  {
    /* Vertical scroll bar message. */
    case WM_VSCROLL:
    {
      /* Save position. */
      wLastPos = fpDisplayInfo->ScrollData.wVertPos;
            
      switch (wParam) 
      {
        /* Show top of page. */
        case SB_TOP:
        {
          /* If not already at top. */
          if( fpDisplayInfo->ScrollData.wVertPos != 0 )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wVertPos = 0;
  
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }
          
        /* Show bottom of page. */
        case SB_BOTTOM:
        {
          /* If not already at bottom. */
          if( fpDisplayInfo->ScrollData.wVertPos != fpDisplayInfo->ScrollData.wMaxVertPos )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wVertPos = fpDisplayInfo->ScrollData.wMaxVertPos;
  
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Show page one line up. */
        case SB_LINEUP:
        {
          /* If not already at top. */
          if( fpDisplayInfo->ScrollData.wVertPos != 0 )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wVertPos = max( 0, (short int)(fpDisplayInfo->ScrollData.wVertPos - 1) );
  
            /* Set the scroll window value. */
            nYScrollWnd = (wLastPos - fpDisplayInfo->ScrollData.wVertPos) *
                          fpDisplayInfo->ScrollData.wVertLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Show page one line down. */
        case SB_LINEDOWN:
        {
          /* If not already at bottom. */
          if( fpDisplayInfo->ScrollData.wVertPos != fpDisplayInfo->ScrollData.wMaxVertPos )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wVertPos = min( fpDisplayInfo->ScrollData.wMaxVertPos, fpDisplayInfo->ScrollData.wVertPos + 1 );
  
            /* Set the scroll window value. */
            nYScrollWnd = -1 * ( fpDisplayInfo->ScrollData.wVertPos - wLastPos ) *
                          fpDisplayInfo->ScrollData.wVertLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }
          
        /* Show page one page up. */
        case SB_PAGEUP:
        {
          /* If not already at top. */
          if( fpDisplayInfo->ScrollData.wVertPos != 0 )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wVertPos = max( 0, (short int) (fpDisplayInfo->ScrollData.wVertPos - fpDisplayInfo->ScrollData.wVertPage) );
  
            /* Set the scroll window value. */
            nYScrollWnd = ( wLastPos - fpDisplayInfo->ScrollData.wVertPos ) *
                          fpDisplayInfo->ScrollData.wVertLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Show page one page down. */
        case SB_PAGEDOWN:
        {
          /* If not already at bottom. */
          if( fpDisplayInfo->ScrollData.wVertPos != fpDisplayInfo->ScrollData.wMaxVertPos )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wVertPos = min( fpDisplayInfo->ScrollData.wMaxVertPos, fpDisplayInfo->ScrollData.wVertPos + fpDisplayInfo->ScrollData.wVertPage );
  
            /* Set the scroll window value. */
            nYScrollWnd = -1 * ( fpDisplayInfo->ScrollData.wVertPos - wLastPos ) *
                          fpDisplayInfo->ScrollData.wVertLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Go to vertical page position. */
        case SB_THUMBPOSITION:
        {
          /* If position has changed. */
          if( fpDisplayInfo->ScrollData.wVertPos != LOWORD( lParam ) )
          {
            /* Get new position. */
            fpDisplayInfo->ScrollData.wVertPos = LOWORD( lParam );
  
            /* Set the scroll window value. */
            if( wLastPos < fpDisplayInfo->ScrollData.wVertPos )
            {
              /* Set the scroll window value. */
              nYScrollWnd = -1 * ( fpDisplayInfo->ScrollData.wVertPos - wLastPos ) * 
                            fpDisplayInfo->ScrollData.wVertLine;
            }
            else
            {
              nYScrollWnd = (wLastPos - fpDisplayInfo->ScrollData.wVertPos ) *
                            fpDisplayInfo->ScrollData.wVertLine;
            }            
            
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }
      }

  
      /* 
      ** Processed any vertical scroll message
      ** that we wanted to.  Do we update the position?
      */

      if( bUpdated )
      {
        /* Repaint scroll bar. */
        SetScrollPos( hWnd, SB_VERT, fpDisplayInfo->ScrollData.wVertPos, TRUE );
      }

      break;
    }


    /* Horizontal scroll bar message. */
    case WM_HSCROLL:
    {
      /* Save position. */
      wLastPos = fpDisplayInfo->ScrollData.wHorzPos;
            
      switch (wParam) 
      {
        /* Move page one horz. line position left. */
        case SB_LINEUP:
        {
          /* If not already at far left. */
          if(  fpDisplayInfo->ScrollData.wHorzPos != 0 )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wHorzPos = max( 0, (short int) (fpDisplayInfo->ScrollData.wHorzPos - 1) );
  
            /* Set the scroll window value. */
            nXScrollWnd = ( wLastPos - fpDisplayInfo->ScrollData.wHorzPos ) *
                          fpDisplayInfo->ScrollData.wHorzLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Move page one horz. line position right. */
        case SB_LINEDOWN:
        {
          /* If not already at far right. */
          if(  fpDisplayInfo->ScrollData.wHorzPos != fpDisplayInfo->ScrollData.wMaxHorzPos )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wHorzPos = min( fpDisplayInfo->ScrollData.wMaxHorzPos, fpDisplayInfo->ScrollData.wHorzPos + 1 );
  
            /* Set the scroll window value. */
            nXScrollWnd = -1 * ( fpDisplayInfo->ScrollData.wHorzPos - wLastPos ) *
                          fpDisplayInfo->ScrollData.wHorzLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Move page one horz. page position left. */
        case SB_PAGEUP:
        {
          /* If not already at far left. */
          if(  fpDisplayInfo->ScrollData.wHorzPos != 0 )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wHorzPos = max( 0, (short int) (fpDisplayInfo->ScrollData.wHorzPos - fpDisplayInfo->ScrollData.wHorzPage) );
  
            /* Set the scroll window value. */
            nXScrollWnd = ( wLastPos - fpDisplayInfo->ScrollData.wHorzPos ) *
                          fpDisplayInfo->ScrollData.wHorzLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Move page one horz. page position right. */
        case SB_PAGEDOWN:
        {
          /* If not already at far right. */
          if(  fpDisplayInfo->ScrollData.wHorzPos != fpDisplayInfo->ScrollData.wMaxHorzPos )
          {
            /* Change position. */
            fpDisplayInfo->ScrollData.wHorzPos = min( fpDisplayInfo->ScrollData.wMaxHorzPos, fpDisplayInfo->ScrollData.wHorzPos + fpDisplayInfo->ScrollData.wHorzPage );
  
            /* Set the scroll window value. */
            nXScrollWnd = -1 * ( fpDisplayInfo->ScrollData.wHorzPos - wLastPos ) *
                          fpDisplayInfo->ScrollData.wHorzLine;
 
            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }

        /* Go to horz. position in page. */
        case SB_THUMBPOSITION:
        {
          /* If position is changing. */
          if(  fpDisplayInfo->ScrollData.wHorzPos != LOWORD( lParam ) )
          {
            /* Get new position. */
            fpDisplayInfo->ScrollData.wHorzPos = LOWORD( lParam );
  
            /* Set the scroll window value. */
            if( wLastPos < fpDisplayInfo->ScrollData.wHorzPos )
            {
              nXScrollWnd = -1 * ( fpDisplayInfo->ScrollData.wHorzPos - wLastPos ) *
                            fpDisplayInfo->ScrollData.wHorzLine;

            }
            else
            {
              nXScrollWnd = ( wLastPos - fpDisplayInfo->ScrollData.wHorzPos ) *
                            fpDisplayInfo->ScrollData.wHorzLine;
            }

            /* Position updated. */
            bUpdated = TRUE;
          }
          break;
        }
      }

      /* 
      ** Processed any horizontal scroll message
      ** that we wanted to.  Do we update the position?
      */

      if( bUpdated )
      {
        /* Repaint scroll bar. */
        SetScrollPos( hWnd, SB_HORZ, fpDisplayInfo->ScrollData.wHorzPos, TRUE );
      }

      break;
    }  
  } 

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );


  /* If we have changed a scroll position. */
  if( bUpdated )
  {
    /* If we should just scroll the window. */
    if( nXScrollWnd != 0 | nYScrollWnd != 0 )
    {
      ScrollWindow( hWnd, nXScrollWnd, nYScrollWnd, NULL, NULL );
    }

    /* Erase entire window. */
    else
    {
      InvalidateRect( hWnd, NULL, TRUE );
    }
  }

  /* Set the page offset position. */
  CalcPagePosOffset( fpDisplayInfo );
}


/***************************************************
*
* Calculate page drawing offset.
*
****************************************************/
static void __far __pascal CalcPagePosOffset
( 
  FPDISPLAYINFO fpDisplayInfo
)
{
  /* Horzontal offset to visible part of page. */
  fpDisplayInfo->PageXPosOffset = (long int) fpDisplayInfo->ScrollData.wHorzPos * 
                                  (long int) fpDisplayInfo->ScrollData.wHorzLine;
  
  /* Vertical offset to visible part of page. */
  fpDisplayInfo->PageYPosOffset = (long int) fpDisplayInfo->ScrollData.wVertPos  * 
                                  (long int) fpDisplayInfo->ScrollData.wVertLine;
}


/***************************************************
*
* Get the current vertical scroll position.
*
****************************************************/
WORD __far __pascal GetVertScrollPos( HWND hWnd )
{
  HGLOBAL hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  
  WORD  wVertPos;
  

  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );

  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );
  
  /* 
  ** If a topic is already being displayed. 
  */
  if( fpDisplayInfo->hTopicData != NULL )
  {
    wVertPos = fpDisplayInfo->ScrollData.wVertPos;
  }
  else
  {
    wVertPos = 0;
  } 
    
  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );

  /* Current vertical scroll position */
  return( wVertPos );
}



/********************************** HOTSPOT OBJECT ROUTINES ***********************************/


/***************************************************
*
* Adds hotspots for the page and calc. page size.
*
****************************************************/
static void __far __pascal AddPagesHotSpots( HDC hDC, HWND hWnd, FPDISPLAYINFO fpDisplayInfo )
{
  PICTDISPLAYREC PictDispRecData;     /* Display information for a picture display record. */
  TXTDISPLAYREC TxtDispRecData;       /* Display information for a text display record. */
  WORD wCurrDispRec;

  FPBITMAPREC fpBitmapRec;            /* Pointer to a bitmap record. */
  FPMETAREC fpMetaFileRec;            /* Pointer to a metafile record. */
  FPSHEDREC fpShedRec;                /* SHED hotspot record. */

  RECT    Rect;                       /* Size of hotspot. */
  POSRECT PosRect;                    /* Position of hotspot on page. */

  WORD wCounter;

  BOOL bReturn;
  

  /* Retrieve first text and "char picture" display record. */
  wCurrDispRec = 0;
  bReturn = GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDispRec, &TxtDispRecData );
  
  /* For each text display record. */
  while( bReturn )
  {
    /* 
    ** Is the text is a hotspot.
    */
    if( TxtDispRecData.bIsHotSpot == TRUE )
    {
      /* Add the hotspot information to the hotspot buffer. */
      bReturn = AddHotSpot( hWnd, fpDisplayInfo->hHotSpotBuffer, 
                            &(TxtDispRecData).HotSpotInfo, &(TxtDispRecData).PosRect );
      
      /* Hotspot adding error? */
      if( ! bReturn ) 
      {
        return;
      }
    }

    /* Does the text display list record store info. for a char. picture? */
    else if( TxtDispRecData.wNumSHEDHotSpots > 0  && 
             ( TxtDispRecData.wRecType == TE_CHARBITMAP || 
             TxtDispRecData.wRecType == TE_CHARMETA )
           )
    {
      /* Character bitmap. */
      if( TxtDispRecData.wRecType == TE_CHARBITMAP )
      {
        /* Lock the display record's data. */
        fpBitmapRec = (FPBITMAPREC) GlobalLock( TxtDispRecData.hElementData );
        
        /* Get pointer to the first SHED record. */
        fpShedRec = fpBitmapRec->fpSHEDRecs;
      }
      
      /* Character metafile. */
      else
      {            
        /* Lock the display record's data. */
        fpMetaFileRec = (FPMETAREC) GlobalLock( TxtDispRecData.hElementData );
        
        /* Get pointer to the first SHED record. */
        fpShedRec = fpMetaFileRec->fpSHEDRecs;
      }

      /* For each SHED record. */
      for( wCounter = 0; wCounter < TxtDispRecData.wNumSHEDHotSpots; wCounter++ )
      {
        /* 
        ** Get the hotspot position in the picture. 
        */
        Rect.left   = fpShedRec->PosInBitmap.left;
        Rect.top    = fpShedRec->PosInBitmap.top;
        Rect.right  = fpShedRec->PosInBitmap.right;
        Rect.bottom = fpShedRec->PosInBitmap.bottom;
        
        /* Convert the picture position to TWIPS. */
        DPtoLP( hDC, (LPPOINT) &Rect, 2 );
      
        /* 
        ** Move the position of the hotspot relative to the 
        ** picture's page position. 
        */
        PosRect.left   = TxtDispRecData.PosRect.left + Rect.left;
        PosRect.right  = TxtDispRecData.PosRect.left + Rect.right;
        PosRect.top    = TxtDispRecData.PosRect.top + Rect.top;
        PosRect.bottom = TxtDispRecData.PosRect.top + Rect.bottom;

        /* Add the SHED hotspot information to the hotspot buffer. */
        bReturn = AddHotSpot( hWnd, fpDisplayInfo->hHotSpotBuffer, 
                              &(fpShedRec)->HotSpotRec, &PosRect );

        /* Hotspot adding error? */
        if( ! bReturn ) 
        {
          /* Unlock the display record's data. */
          GlobalUnlock( TxtDispRecData.hElementData );

          return;
        }

        /* Goto next SHED record. */
        fpShedRec++;
      }

      /* Unlock the display record's element data. */
      GlobalUnlock( TxtDispRecData.hElementData );
    }
    
    /* Get next display record. */
    wCurrDispRec++;
    bReturn = GetTxtDisplayListRec( fpDisplayInfo->fpTxtDisplayList, wCurrDispRec, &TxtDispRecData );
  }

  /* Retrieve first justified picture's display record. */
  wCurrDispRec = 0;
  bReturn = GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, wCurrDispRec, &PictDispRecData );
  
  /* For each justified picture display record. */
  while( bReturn )
  {
    /* 
    ** Is the picture a hotspot.
    */
    if( PictDispRecData.bIsHotSpot == TRUE )
    {
      /* Add the hotspot information to the hotspot buffer. */
      bReturn = AddHotSpot( hWnd, fpDisplayInfo->hHotSpotBuffer, 
                            &(PictDispRecData).HotSpotInfo, &(PictDispRecData).PosRect );

      /* Hotspot adding error? */
      if( ! bReturn ) 
      {
        return;
      }
    }

    /* Does the text display list record store info. for a char. picture? */
    else if( PictDispRecData.wNumSHEDHotSpots > 0 )
    {
      switch( PictDispRecData.wPictType )
      {
        /* Bitmap. */
        case TE_RIGHTBITMAP:
        case TE_LEFTBITMAP:
        {
          /* Lock the display record's data. */
          fpBitmapRec = (FPBITMAPREC) GlobalLock( PictDispRecData.hPictData );
          
          /* Get pointer to the first SHED record. */
          fpShedRec = fpBitmapRec->fpSHEDRecs;
          
          break;
        }
      
        /* Metafile. */
        case TE_LEFTMETA:
        case TE_RIGHTMETA:
        {            
          /* Lock the display record's data. */
          fpMetaFileRec = (FPMETAREC) GlobalLock( PictDispRecData.hPictData );
          
          /* Get pointer to the first SHED record. */
          fpShedRec = fpMetaFileRec->fpSHEDRecs;
          
          break;
        }
      }

      /* For each SHED record. */
      for( wCounter = 0; wCounter < PictDispRecData.wNumSHEDHotSpots; wCounter++ )
      {
        /* 
        ** Get the hotspot position in the picture. 
        */
        Rect.left   = fpShedRec->PosInBitmap.left;
        Rect.top    = fpShedRec->PosInBitmap.top;
        Rect.right  = fpShedRec->PosInBitmap.right;
        Rect.bottom = fpShedRec->PosInBitmap.bottom;
        
        /* Convert the position position to TWIPS. */
        DPtoLP( hDC, (LPPOINT) &Rect, 2 );
      
        /* 
        ** Move the position of the hotspot relative to the 
        ** picture's page position. 
        */
        PosRect.left   = PictDispRecData.PosRect.left + Rect.left;
        PosRect.right  = PictDispRecData.PosRect.left + Rect.right;
        PosRect.top    = PictDispRecData.PosRect.top + Rect.top;
        PosRect.bottom = PictDispRecData.PosRect.top + Rect.bottom;
        
        /* Add the SHED hotspot information to the hotspot buffer. */
        bReturn = AddHotSpot( hWnd, fpDisplayInfo->hHotSpotBuffer, 
                              &(fpShedRec)->HotSpotRec, &PosRect );

        /* Hotspot adding error? */
        if( ! bReturn )
        {
          /* Unlock the display record's data. */
          GlobalUnlock( PictDispRecData.hPictData );

          return;
        }

        /* Goto next SHED record. */
        fpShedRec++;
      }

      /* Unlock the display record's data. */
      GlobalUnlock( PictDispRecData.hPictData );
    }

    /* Get next picture display record. */
    wCurrDispRec++;
    bReturn = GetPictDisplayListRec( fpDisplayInfo->fpPictDisplayList, wCurrDispRec, &PictDispRecData );
  }
}


/***************************************************
*
* Sets state variable and text color for a hotspot.
*
****************************************************/
static void __far __pascal HotSpotBegin
( 
  HDC hDC, 
  FPCONTEXTDATA fpContextData, 
  FPTOPICELEMENT fpTopicElement
)
{
  FPHOTSPOTREC fpHotSpotRec;
  
  /* We are in a hotspot. */
  fpContextData->bInHotSpot = TRUE;

  /* Lock the hotspot record. */
  fpHotSpotRec = (FPHOTSPOTREC) fpTopicElement->RecDataPtr;

  /* Save hotspot info. */
  fpContextData->HotSpotInfo = *fpHotSpotRec;

  /* Should we set the hotspot color. */
  if( fpContextData->HotSpotInfo.bColorAndUnder == TRUE )
  {
    /* Set color to hotspot color. */
    SetTextColor( hDC, fpContextData->HotSpotColor );                                
  }
}

/***************************************************
*
* Resets state variable and text color when a hotspot
* is ending.
*
****************************************************/
static void __far __pascal HotSpotEnd
( 
  HDC hDC, 
  FPCONTEXTDATA fpContextData 
)
{
  /* Set color to last non-hotspot text color. */
  SetTextColor( hDC, fpContextData->LastFGTextColor );                                
  
  /* We are out of any hotspot. */
  fpContextData->bInHotSpot = FALSE;
}


/***************************************************
*
* Draws hotspot underlining below hotspot text.
*
****************************************************/
static void __far __pascal DrawHotspotUnderline
( 
  HDC hDC, 
  FPTXTDISPLAYREC fpDisplayRec,
  FPCONTEXTDATA fpContextData, 
  FPDISPLAYINFO fpDisplayInfo,
  HPEN hSolidPen, 
  HPEN hDashedPen 
)                                 
{
  HPEN hPrevPen;                       
  TEXTMETRIC tm;
  
  POINT Point;
    
  /* Get the current font's metrics. */
  GetTextMetrics( hDC, &tm );
        
  /* 
  ** If we are drawing for a hotspot whose
  ** topic is shown in the main window.
  */
  if( fpContextData->HotSpotInfo.HotSpotType == HOTSPOT_POPUP  )
  {
    /* Select the new pen. */
    hPrevPen = SelectObject( hDC, hDashedPen );
  }  

  /* 
  ** If we are drawing for a hotspot whose
  ** topic is shown in a popup window.
  */                                                         
  else                                                       
  {
    /* Select the new pen. */
    hPrevPen = SelectObject( hDC, hSolidPen );
  }        

  /* Get the value of one pixel. */
  Point.x = 0;
  Point.y = 1;                                  
  DPtoLP( hDC, &Point, 1 );
                                                

  /* Go to beginning window output position for word. */
  MoveTo( hDC, (short int) (fpDisplayRec->PosRect.left - fpDisplayInfo->PageXPosOffset), 
          (short int) (fpDisplayRec->PosRect.bottom - tm.tmDescent + Point.y - fpDisplayInfo->PageYPosOffset) );

  /* Draw the underline. */
  LineTo( hDC, (short int) (fpDisplayRec->PosRect.right - fpDisplayInfo->PageXPosOffset), 
          (short int) (fpDisplayRec->PosRect.bottom - tm.tmDescent + Point.y - fpDisplayInfo->PageYPosOffset) );

  /* Select the previous pen. */
  hPrevPen = SelectObject( hDC, hPrevPen );
}


/***************************************************
*
* Set the cursor to the correct cursor.
*
****************************************************/
void __far __pascal SetCorrectCursor( HWND hWnd, POINT WndPoint )
{
  HGLOBAL       hDisplayInfo;
  FPDISPLAYINFO fpDisplayInfo;
  POSPOINT      PagePosPoint;

  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );
        
  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );

  /* 
  ** Adjust the point to take into consideration that we have scroll
  ** bars.
  */
  PagePosPoint.x = WndPoint.x + fpDisplayInfo->PageXPosOffset;
  PagePosPoint.y = WndPoint.y + fpDisplayInfo->PageYPosOffset;

  /* If in a hotspot, set cursor to hand. */
  if( IsPointInHotSpot( fpDisplayInfo->hHotSpotBuffer, PagePosPoint ) )
  {
    SetCursor( GetHandCursor() );
  } 

  /* Otherwise, set cursor to arow. */
  else 
  {
    SetCursor( LoadCursor( (HINSTANCE)NULL, IDC_ARROW ) );
  }

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );
}


/***************************************************
*
* Loads the topic data for a hotspot.
*
****************************************************/
BOOL __far __pascal GetHotSpotTopicInfo
( 
  HWND hWnd,                           /* Topic window. */
  POINT WndPoint,                      /* Mouse click window position point. */
  HTOPICDATA __far * hNewTopicsData,   /* Hot spots data. */
  HOTSPOTTYPE __far * HotSpotType,     /* Type of hotspot. */
  RECT __far * WndPosRect,             /* Position of hotspot. */
  WORD __far * wSecWndNumPtr,          /* Secondary window's number. */
  CHARDBLPTR MacroDblPtr               /* Macro string - If hotspot is linked to a macro. */
)
{
  HGLOBAL       hHelpFileInfo;     /* Handle to the help file's general info. */

  HOTSPOTREC    HotSpotInfo;       /* Info about the hotspot. */
  char __far *  fpHelpFilePath;    /* Pointer to the help file's path. */
  DWORD         dwTopicCharOffset; /* Hotspot's topic offset in the |TOPIC file. */

  HWND          hDataWnd;       /* Handle to the data window. */
  HGLOBAL       hDisplayInfo;   /* Window's display information. */
  FPDISPLAYINFO fpDisplayInfo;  /* Pointer to the window's display information. */
  FPTOPICDATA   fpTopicData;    /* Pointer to the window current topic. */
  
  HDC           hDC;
  
  POSRECT       PagePosRect;
  POSPOINT      PagePosPoint;
  
  /* Get the handle display info. from the window data. */
  hDisplayInfo = (HGLOBAL) WndData( hWnd, WDM_GETDISPLAYINFO, 0L, NULL );
        
  /* Lock the display info. */
  fpDisplayInfo = (FPDISPLAYINFO) GlobalLock( hDisplayInfo );

  /* 
  ** Adjust the point to take into consideration that we have scroll
  ** bars.
  */
  PagePosPoint.x = WndPoint.x + fpDisplayInfo->PageXPosOffset;
  PagePosPoint.y = WndPoint.y + fpDisplayInfo->PageYPosOffset;


  /* If the point is in a hotspot, get the hotspot's topic info. */
  if( ! GetHotSpot( fpDisplayInfo->hHotSpotBuffer, PagePosPoint, &HotSpotInfo, &PagePosRect ) ) 
  {
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );

    /* Not in a hotspot. */
    return( FALSE );
  }

  /* Save the hotspot's type. */
  *HotSpotType = HotSpotInfo.HotSpotType;

  /* 
  ** Hotspot is a macro. 
  */
  if( HotSpotInfo.HotSpotType == HOTSPOT_MACRO )
  {
    /* Assign the secondary window number. */
    *MacroDblPtr = HotSpotInfo.MacroPtr;
  
    /* Success. */
    return( TRUE );
  }

  /* 
  ** Returned hotspot position is relative to the topic's entire page. Make the 
  ** position of the hotspot relative to the topic window. 
  */
  PagePosRect.top    -= fpDisplayInfo->PageYPosOffset;
  PagePosRect.bottom -= fpDisplayInfo->PageYPosOffset;
  PagePosRect.left   -= fpDisplayInfo->PageXPosOffset;
  PagePosRect.right  -= fpDisplayInfo->PageXPosOffset;
  
  /* Save the window position of the hot spot. */
  WndPosRect->top    = (short int) PagePosRect.top;
  WndPosRect->bottom = (short int) PagePosRect.bottom;
  WndPosRect->left   = (short int) PagePosRect.left;
  WndPosRect->right  = (short int) PagePosRect.right;
  
  /* Convert hotspot window position to screen position. */
  hDC = GetDC( hWnd );
  LPtoDP( hDC, (LPPOINT) WndPosRect, 2 );
  ReleaseDC( hWnd, hDC );
  MapWindowPoints( hWnd, NULL, (POINT __far *) WndPosRect, 2 );

  /* Get the handle of the data window. */
  hDataWnd = (HWND) WndData( hWnd, WDM_GETDATAHWND, 0L, NULL );
        
  /* Get file info. */
  if( HotSpotInfo.HelpFilePathPtr != NULL )
  {
    /* Use help file specified. */
    fpHelpFilePath = HotSpotInfo.HelpFilePathPtr;
  }
  else
  {  
    /* 
    ** Lock the window's current topic data to get 
    ** the current help file name. 
    */
    fpTopicData = (FPTOPICDATA) GlobalLock( fpDisplayInfo->hTopicData );

    /* No help file specified - Use currently open help file. */
    fpHelpFilePath = fpTopicData->szHelpFile;
  }

  /* Get file info. */
  hHelpFileInfo = OpenHelpFile( hDataWnd, fpHelpFilePath );
  if( hHelpFileInfo == NULL )
  {
    /* Unlock file path buffer. */
    if( HotSpotInfo.HelpFilePathPtr == NULL )
    {
      /* Unlock the window's current topic data. */
      GlobalUnlock( fpDisplayInfo->hTopicData );
    }
    
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
  
    /* Failure. */
    return( FALSE );
  }

  /* Get the topic's char. offset from the HASH value specified. */
  if( ! GetContextOffset( hHelpFileInfo, HotSpotInfo.dwHotSpotContext, &dwTopicCharOffset ) )
  {
    /* Unlock file path buffer. */
    if( HotSpotInfo.HelpFilePathPtr == NULL )
    {
      /* Unlock the window's current topic data. */
      GlobalUnlock( fpDisplayInfo->hTopicData );
    }
      
    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
    
    /* Failure. */
    return( FALSE );
  }

  /* Get the new topic's data. */
  *hNewTopicsData = LockTopicData( hDataWnd, fpHelpFilePath, dwTopicCharOffset );
  if( *hNewTopicsData == NULL )
  {
    /* Unlock file path buffer. */
    if( HotSpotInfo.HelpFilePathPtr == NULL )
    {
      /* Unlock the window's current topic data. */
      GlobalUnlock( fpDisplayInfo->hTopicData );
    }

    /* Unlock the display info. */
    GlobalUnlock( hDisplayInfo );
  
    /* Failure. */
    return( FALSE );
  }

  /* Unlock file path buffer. */
  if( HotSpotInfo.HelpFilePathPtr == NULL )
  {
    /* Unlock the window's current topic data. */
    GlobalUnlock( fpDisplayInfo->hTopicData );
  }

  /* Unlock the display info. */
  GlobalUnlock( hDisplayInfo );

  /* Assign the secondary window number. */
  *wSecWndNumPtr = HotSpotInfo.wSecWndNum;

  /* Success. */
  return( TRUE );
}



/********************************* GENERAL DISPLAY LIST ROUTINES ********************************/

/***************************************************
*
* Allocate memory for a display list.
*
****************************************************/
static HGLOBAL __far __pascal AllocDisplayList( HWND hWnd, WORD wRecSize )
{
  HGLOBAL hDisplayList;
  FPDISPLAYLIST fpDisplayList;
                          
  /* 
  ** Allocate memory for the display list info structure. 
  ** Extra record allocated to hold record count.
  */
  if( !GlobalAllocMem( hWnd, &hDisplayList, sizeof(DISPLAYLIST) ) )
  {
    /* Failure. */
    return( NULL );
  }

  /* Lock display list. */
  fpDisplayList = (FPDISPLAYLIST) GlobalLock( hDisplayList );  
  
  /* 
  ** Allocate memory for the record list. 
  */
  if( !GlobalAllocMem( hWnd, &(fpDisplayList)->hRecordList, 
                       wRecSize * DISPLAY_LIST_NUM ) )
  {
    /* Unlock display list. */
    GlobalUnlock( hDisplayList );  

    /* Free the dispay list. */
    GlobalFree( hDisplayList );  

    /* Failure. */
    return( NULL );
  }
  
  /* Set the record counts. */
  fpDisplayList->wNumRecords = 0;
  fpDisplayList->wMaxRecords = DISPLAY_LIST_NUM;
  
  /* Unlock display list. */
  GlobalUnlock( hDisplayList );  

  /* Success. */
  return( hDisplayList );
}


/***************************************************
*
* Increse size of a text/picture display list.
*
****************************************************/
static BOOL __far __pascal ReAllocDisplayList( HWND hWnd, FPDISPLAYLIST fpDisplayList, WORD wRecSize )
{
  /* 
  ** Allocate memory for the record list. 
  ** Extra record allocated to hold record count.
  */
  if( !GlobalReAllocMem( hWnd, &(fpDisplayList)->hRecordList, 
                         wRecSize * ( fpDisplayList->wNumRecords + DISPLAY_LIST_NUM ) ) )
  {
    /* Failure. */
    return( FALSE );
  }
  
  /* Set the new record count in the first record. */
  fpDisplayList->wMaxRecords = fpDisplayList->wNumRecords + DISPLAY_LIST_NUM;
  
  /* Success. */
  return( TRUE );
}



/***************************************************
*
* Free memory for a text/picture display list.
*
****************************************************/
static void __far __pascal FreeDisplayList( HGLOBAL hDisplayList )
{
  FPDISPLAYLIST fpDisplayList;

  
  /* Lock display list. */
  if( hDisplayList == NULL ) return;  
  
  /* Lock display list. */
  fpDisplayList = (FPDISPLAYLIST) GlobalLock( hDisplayList );  
  
  /* Free the buffer itself. */
  if( fpDisplayList->hRecordList != NULL ) 
  {
    GlobalFree( fpDisplayList->hRecordList );
  }

  /* Unlock display list. */
  GlobalUnlock( hDisplayList );  

  /* Free the display list. */
  GlobalFree( hDisplayList );
}


/***************************************************
*
* Reset used record count for a text/picture display
* list.
*
****************************************************/
static void __far __pascal ClearDisplayList( HGLOBAL hDisplayList )
{
  FPDISPLAYLIST fpDisplayList;

  
  /* Lock display list. */
  if( hDisplayList == NULL ) return;  
  
  /* Lock display list. */
  fpDisplayList = (FPDISPLAYLIST) GlobalLock( hDisplayList );  
  
  /* Set the new record count in the first record. */
  fpDisplayList->wNumRecords = 0;
  
  /* Unlock display list. */
  GlobalUnlock( hDisplayList );  
}



/****************************** TEXT DISPLAY LIST RECORD ROUTINES *******************************/

/***************************************************
*
* Inserts information into a text display list's record.
*
****************************************************/
static BOOL __far __pascal InsertTxtDisplayListRec
( 
  HWND hWnd,
  FPDISPLAYLIST fpDisplayList,
  FPTXTDISPLAYREC fpTxtDispRecData 
)
{
  FPTXTDISPLAYREC fpTxtDisplayRec;
   
  
  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Do we need to increase the size of the list? */
  if( fpDisplayList->wNumRecords == fpDisplayList->wMaxRecords )
  {
    /* Realloc the list. */
    if( ! ReAllocDisplayList( hWnd, fpDisplayList, sizeof(TXTDISPLAYREC) ) )
    {
      /* Failure. */
      return FALSE;
    }
  }
  
  /* Lock the record list. */
  fpTxtDisplayRec = (FPTXTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpTxtDisplayRec = fpTxtDisplayRec + fpDisplayList->wNumRecords;
  
  /* Save data to record. */
  CopyTxtDisplayListRec( fpTxtDisplayRec, fpTxtDispRecData ); 

  /* Increment the new record count. */
  fpDisplayList->wNumRecords = fpDisplayList->wNumRecords + 1;
  
  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Updates the information in an existing text 
* display list's record.
*
****************************************************/
static BOOL __far __pascal UpdateTxtDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPTXTDISPLAYREC fpTxtDispRecData 
)
{
  FPTXTDISPLAYREC fpTxtDisplayRec;
   
  
  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Do we need to increase the size of the list? */
  if( wRecordNum >= fpDisplayList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Lock the record list. */
  fpTxtDisplayRec = (FPTXTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpTxtDisplayRec = fpTxtDisplayRec + wRecordNum;
  
  /* Save data to record. */
  CopyTxtDisplayListRec( fpTxtDisplayRec, fpTxtDispRecData ); 

  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Retrieves information from a text display list's 
* record.
*
****************************************************/
static BOOL __far __pascal GetTxtDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPTXTDISPLAYREC fpTxtDispRecData 
)
{
  FPTXTDISPLAYREC fpTxtDisplayRec;

  
  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Are we past the end of the list? */
  if( wRecordNum >= fpDisplayList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Lock the record list. */
  fpTxtDisplayRec = (FPTXTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpTxtDisplayRec = fpTxtDisplayRec + wRecordNum;
  
  /* Retrieve data from record. */
  CopyTxtDisplayListRec( fpTxtDispRecData, fpTxtDisplayRec ); 

  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}




/***************************************************
*
* Moves the horizontal position of each text display
* record in a display list.
*
****************************************************/
static BOOL __far __pascal MoveTxtDisplayRecHortPos
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wBeginTxtDispRec,
  WORD wEndTxtDispRec,
  WORD wSlideAmount 
)
{
  FPTXTDISPLAYREC fpTopRecord;
  FPTXTDISPLAYREC fpTxtDisplayRec;
  WORD wCounter;
  

  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* We will pass the end of the list? */
  if( wEndTxtDispRec >= fpDisplayList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Lock the record list. */
  fpTopRecord = (FPTXTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Move each record in the range. */
  for( wCounter = wBeginTxtDispRec; wCounter < wEndTxtDispRec; wCounter++ )
  {
    /* Goto to the first record to be adjusted. */
    fpTxtDisplayRec = fpTopRecord + wCounter;
  
    /* Move record's horizontal position. */
    fpTxtDisplayRec->PosRect.left  = fpTxtDisplayRec->PosRect.left + ( long int ) wSlideAmount;          
    fpTxtDisplayRec->PosRect.right = fpTxtDisplayRec->PosRect.right + ( long int ) wSlideAmount;           
  }

  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}



/***************************************************
*
* Copy the data in a text display list record to another
* record.
*
****************************************************/
static void __far __pascal CopyTxtDisplayListRec( FPTXTDISPLAYREC fpDstRec, FPTXTDISPLAYREC fpSrcRec )
{
  fpDstRec->wRecType        = fpSrcRec->wRecType; 

  fpDstRec->hElementData    = fpSrcRec->hElementData;    /* One of these two is used. */
  fpDstRec->ElementDataPtr  = fpSrcRec->ElementDataPtr;  /* One of these two is used. */
  fpDstRec->wDataOffset     = fpSrcRec->wDataOffset;       
  fpDstRec->wNumChars       = fpSrcRec->wNumChars;

  fpDstRec->PosRect.top     = fpSrcRec->PosRect.top;       
  fpDstRec->PosRect.bottom  = fpSrcRec->PosRect.bottom;            
  fpDstRec->PosRect.left    = fpSrcRec->PosRect.left;          
  fpDstRec->PosRect.right   = fpSrcRec->PosRect.right;           

  fpDstRec->wBaseLineHeight = fpSrcRec->wBaseLineHeight;           

  fpDstRec->dwLinesYPos     = fpSrcRec->dwLinesYPos; 

  fpDstRec->bIsHotSpot      = fpSrcRec->bIsHotSpot;
  fpDstRec->HotSpotInfo     = fpSrcRec->HotSpotInfo; 
  fpDstRec->wNumSHEDHotSpots  = fpSrcRec->wNumSHEDHotSpots;
}



/****************************** PICTURE DISPLAY LIST RECORD ROUTINES *******************************/

/***************************************************
*
* Inserts information into a picture display list's
* record.
*
****************************************************/
static BOOL __far __pascal InsertPictDisplayListRec
( 
  HWND hWnd,
  FPDISPLAYLIST fpDisplayList,
  FPPICTDISPLAYREC fpPictDispRecData 
)
{
  FPPICTDISPLAYREC fpPictDisplayRec;
   
  
  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Do we need to increase the size of the list? */
  if( fpDisplayList->wNumRecords == fpDisplayList->wMaxRecords )
  {
    /* Realloc the list. */
    if( ! ReAllocDisplayList( hWnd, fpDisplayList, sizeof(PICTDISPLAYREC) ) )
    {
      /* Failure. */
      return FALSE;
    }
  }
  
  /* Lock the record list. */
  fpPictDisplayRec = (FPPICTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpPictDisplayRec = fpPictDisplayRec + fpDisplayList->wNumRecords;
  
  /* Save data to record. */
  CopyPictDisplayListRec( fpPictDisplayRec, fpPictDispRecData ); 

  /* Increment the new record count. */
  fpDisplayList->wNumRecords = fpDisplayList->wNumRecords + 1;
  
  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Updates information in an existing picture display
* list's record.
*
****************************************************/
static BOOL __far __pascal UpdatePictDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPPICTDISPLAYREC fpPictDispRecData 
)
{
  FPPICTDISPLAYREC fpPictDisplayRec;
   
  
  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Do we need to increase the size of the list? */
  if( wRecordNum >= fpDisplayList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Lock the record list. */
  fpPictDisplayRec = (FPPICTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpPictDisplayRec = fpPictDisplayRec + wRecordNum;
  
  /* Save data to record. */
  CopyPictDisplayListRec( fpPictDisplayRec, fpPictDispRecData ); 

  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Retrieves information from a picture display list's 
* record.
*
****************************************************/
static BOOL __far __pascal GetPictDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList,
  WORD wRecordNum,
  FPPICTDISPLAYREC fpPictDispRecData 
)
{
  FPPICTDISPLAYREC fpPictDisplayRec;

  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Are we at the end of the list? */
  if( wRecordNum >= fpDisplayList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Lock the record list. */
  fpPictDisplayRec = (FPPICTDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpPictDisplayRec = fpPictDisplayRec + wRecordNum;
  
  /* Retrieve data from record. */
  CopyPictDisplayListRec( fpPictDispRecData, fpPictDisplayRec ); 

  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Copy the data in a picture display list record to
* another record.
*
****************************************************/
static void __far __pascal CopyPictDisplayListRec( FPPICTDISPLAYREC fpDstRec, FPPICTDISPLAYREC fpSrcRec )
{
  fpDstRec->wPictType    = fpSrcRec->wPictType;
  fpDstRec->hPictData    = fpSrcRec->hPictData;
  fpDstRec->PosRect      = fpSrcRec->PosRect; 
  fpDstRec->bIsHotSpot   = fpSrcRec->bIsHotSpot;
  fpDstRec->HotSpotInfo  = fpSrcRec->HotSpotInfo;
  fpDstRec->wNumSHEDHotSpots  = fpSrcRec->wNumSHEDHotSpots;
}



/****************************** BORDER DISPLAY LIST RECORD ROUTINES *****************************/

/***************************************************
*
* Inserts information into a border display list's
* record.
*
****************************************************/
static BOOL __far __pascal InsertBorderDisplayListRec
( 
  HWND hWnd,
  FPDISPLAYLIST fpDisplayList,
  FPBORDERDISPLAYREC fpBorderDispRecData 
)
{
  FPBORDERDISPLAYREC fpBorderDisplayRec;
   
  
  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Do we need to increase the size of the list? */
  if( fpDisplayList->wNumRecords == fpDisplayList->wMaxRecords )
  {
    /* Realloc the list. */
    if( ! ReAllocDisplayList( hWnd, fpDisplayList, sizeof(BORDERDISPLAYREC) ) )
    {
      /* Failure. */
      return FALSE;
    }
  }
  
  /* Lock the record list. */
  fpBorderDisplayRec = (FPBORDERDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpBorderDisplayRec = fpBorderDisplayRec + fpDisplayList->wNumRecords;
  
  /* Save data to record. */
  CopyBorderDisplayListRec( fpBorderDisplayRec, fpBorderDispRecData ); 

  /* Increment the new record count. */
  fpDisplayList->wNumRecords = fpDisplayList->wNumRecords + 1;
  
  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}



/***************************************************
*
* Retrieves information from a picture display list's 
* record.
*
****************************************************/
static BOOL __far __pascal GetBorderDisplayListRec
( 
  FPDISPLAYLIST fpDisplayList, 
  WORD wRecordNum,
  FPBORDERDISPLAYREC fpBorderDispRecData 
)
{
  FPBORDERDISPLAYREC fpBorderDisplayRec;

  /* No list. */
  if( fpDisplayList->hRecordList == NULL ) return FALSE;
  
  /* Are we at the end of the list? */
  if( wRecordNum >= fpDisplayList->wNumRecords )
  {
    /* Failure. */
    return FALSE;
  }
  
  /* Lock the record list. */
  fpBorderDisplayRec = (FPBORDERDISPLAYREC) GlobalLock( fpDisplayList->hRecordList );  
  
  /* Goto to the record where info will be saved. */
  fpBorderDisplayRec = fpBorderDisplayRec + wRecordNum;
  
  /* Retrieve data from record. */
  CopyBorderDisplayListRec( fpBorderDispRecData, fpBorderDisplayRec ); 

  /* Unlock the record list. */
  GlobalUnlock( fpDisplayList->hRecordList );  

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Copy the data in a border display list record to
* another record.
*
****************************************************/
static void __far __pascal CopyBorderDisplayListRec( FPBORDERDISPLAYREC fpDstRec, FPBORDERDISPLAYREC fpSrcRec )
{
  fpDstRec->BorderInfo = fpSrcRec->BorderInfo;
  fpDstRec->wLineWidth = fpSrcRec->wLineWidth;
  fpDstRec->PosRect    = fpSrcRec->PosRect;
}





